/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Compare new and saved hardware maps
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/07/94   |  tjt  Original implementation.
 *  08/06/94   |  tjt  Ignore simulated.
 *  08/22/94   |  tjt  Show All basic function errors.
 *  03/22/95   |  tjt  Fix ss_open missing.
 *  05/20/96   |  tjt  Add box full module.
 *  05/26/98   |  tjt  Add IO module.
 *  05/28/98   |  tjt  Add full function.
 *-------------------------------------------------------------------------*/
static char compare_hw_maps_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"
#include "message_types.h"
#include "ss.h"
#include "co.h"

extern leave();


unsigned long mask = IsFullFunction | IsBasicFunction | IsTotalFunction;

char *hname[] = 
  {"??", "BL", "ZC", "PM", "PI", "ZC2", "PM2", "PM4", "PM6", "BF", "IO"};

long k, osize, nsize;
char md_name[40], sd_name[40], fd_name[16];
FILE *md, *sd, *fd;

long send_errors = 0;
long errors = 0;

main(argc, argv)
long argc;
char **argv;
{
  char text[80];

  putenv("_=compare_hw_maps");
  chdir(getenv("HOME"));
  
  if (argc > 1)
  {
    if (strcmp(argv[1], "-m") == 0)
    {
      send_errors = 1;
      message_open();
    }
  }
  tmp_name(fd_name);
  fd = fopen(fd_name, "w");
  if (fd == 0) krash("main", "open tmp file", 1);
  
  co_open();
  ss_open();                            /* F032295 */
  
  for (k = 0; k < coh->co_ports; k++)
  {
    if (!(po[k].po_flags & mask)) continue;
  
    sprintf(md_name, "%s.%s", hw_name, 		basename(po[k].po_name));
    sprintf(sd_name, "%s.%s", hw_save_name,	basename(po[k].po_name));

    md = fopen(md_name, "r");
    if (md == 0) krash("main - open", md_name, 1);
    
    sd = fopen(sd_name, "r");
    if (sd == 0)
    {
      sprintf(text, "Port: %s Has No Prior Initialization", po[k].po_name);
      message(text, 1);
      continue;
    }
    fseek(md, 0, 2);
    nsize = ftell(md);
    fseek(md, 0, 0);

    fseek(sd, 0, 2);
    osize = ftell(sd);
    fseek(sd, 0, 0);

    if (po[k].po_flags & IsBasicFunction)			bf_compare();
    else if (po[k].po_flags & IsTotalFunction)  ac_compare();
    else if (po[k].po_flags & IsFullFunction)   ff_compare(); /* F052898 */

    fclose(md);
    fclose(sd);
  }
  leave();
}
/*-------------------------------------------------------------------------*
 *  Compare Two Full Function Maps
 *-------------------------------------------------------------------------*/
ff_compare()
{
  unsigned char otype, ntype, text[80];
  register long olen, nlen, count;
  
  if (osize != nsize)
  {
    sprintf(text, "Port: %s - Had %d Modules Now Has %d Modules",
      po[k].po_name, osize, nsize);
    message(text, 1);
  }
  count = 0;

  while (1)
  {
    count++;

    olen = fread(&otype, 1, 1, sd);
    nlen = fread(&ntype, 1, 1, md);
  
    if (olen != 1 || nlen != 1) break; 
    if (otype == ntype) continue;

    sprintf(text, "Port: %s - HWIX %d Was %s Now Is %s", 
      po[k].po_name, count, hname[otype], hname[ntype]);
    
    message(text, 1);
    return 1;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Compare Two Basic Function Maps
 *-------------------------------------------------------------------------*/
bf_compare()
{
  unsigned char opi, npi, text[80];
  register long olen, nlen, count, ret;

  ret = count = 0;

  if (osize != nsize)
  {
    sprintf(text, "Port: %s Had %d TC's Now Has %d TC's",
      po[k].po_name, osize, nsize);
    message(text, 1);
    ret = 1;
  }
  while (1)
  {
    count++;

    olen = fread(&opi, 1, 1, sd);
    nlen = fread(&npi, 1, 1, md);
  
    if (olen != 1 || nlen != 1) break; 
    if (opi == npi) continue;

    sprintf(text, "Port: %s - TC %d Had %d PI's Now Has %d PI's", 
      po[k].po_name, count, opi, npi);
    
    message(text, 1);
    ret = 1;
  }
  return ret;
}
/*-------------------------------------------------------------------------*
 *  Compare Two Total Function Maps
 *-------------------------------------------------------------------------*/
ac_compare()
{
  register long olen, nlen, count;
  char text[128];
  
  typedef struct
  {
    unsigned char ac;
    unsigned char mod;
    unsigned char type;
  }  ac_item;
  
  ac_item old, new;
  
  if (osize != nsize)
  {
    sprintf(text, "Port: %s - Had %d Modules Now Has %d Modules",
      po[k].po_name, osize / 3 , nsize / 3);
    message(text, 1);
  }
  count = 0;

  while (1)
  {
    olen = fread(&old, 3, 1, sd);
    nlen = fread(&new, 3, 1, md);
  
    if (olen != 1 || nlen != 1) break; 

    if (memcmp(&old, &new, 3) == 0) continue;

    if (count >= 5)
    {
      message("More Than 5 Differences", 0);
      return 1;
    }
    sprintf(text, "Port: %s - Was CM%d Mod%d %s - Now CM%d Mod%d %s",
      po[k].po_name, old.ac, old.mod + 1, hname[old.type],
      new.ac, new.mod + 1, hname[new.type]);
    
    message(text, 1);

    count++;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Message Output
 *-------------------------------------------------------------------------*/
message(p, n)
register char *p;
register long n;
{
  char text[80];

  sprintf(text, "%s\n", p);
  fprintf(fd, "%s", text);

  errors += n;
  
  if (send_errors)
  {
    message_put(0, InitErrorMessageEvent, text, strlen(text));
  }
}

/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  char prt_name[16];
  long stat;
  
  ss_close();
  co_close();

  fclose(fd);

  if (send_errors) message_close();
  
  if (errors)
  {
    if (fork() == 0)
    {
      execlp("prft", "prft", fd_name, tmp_name(prt_name),
        "sys/report/init_report.h", 0);
      krash("leave", "prft load", 1);
    }
    wait(&stat);
    exit(1);
  }
  else unlink(fd_name);
  
  exit(0);
}
/* end of compare_hw_maps.c */
