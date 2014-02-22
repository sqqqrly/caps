/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Initialize all ports from hardware map.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/19/93   |  tjt  Original implementation.
 *  07/07/94   |  tjt  Added error messages (-m option).
 *  02/24/95   |  tjt  Improved basic function message.
 *  03/16/95   |  tjt  Added PM4 in hname list.
 *  04/28/05   |  tjt  Added dummy port type.
 *  09/24/95   |  tjt  Added box full mask and test.
 *  05/20/96   |  tjt  Added BF  in hname list.
 *  01/07/97   |  tjt  Added PM6 in hname list..
 *  05/26/98   |  tjt  Added IO Module.
 *  05/26/98   |  tjt  Allow both total and full function flags.
 *  05/18/01   |  aha  Added fix for disabled ports.
 *-------------------------------------------------------------------------*/
static char rp_init_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include "file_names.h"
#include "message_types.h"
#include "ss.h"
#include "co.h"
#include "plc.h"

FILE *fd;                                 /* hardware map descriptor         */
char fd_name[40];                         /* hardware map file               */

FILE *ed;                                 /* error file name                 */
char ed_name[16];                         /* error file name                 */

char *hname[] = 
  {"??", "BL", "ZC", "PM", "PI", "ZC2", "PM2", "PM4", "PM6", "BF", "IO"}; 

long errors = 0;                          /* error count                     */
long total  = 0;
long bl_mod = 0;
long zc_mod = 0;
long pm_mod = 0;

long send_errors = 0;

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  char text[80];
  
  setpgrp();                              /* krash only this task            */
  putenv("_=rp_init");                    /* name to environment             */
  chdir(getenv("HOME"));                  /* to home directory               */

  ss_open();
  co_open();
  
  if (sp->sp_full_function  == 's') leave(0);
  if (sp->sp_basic_function == 's') leave(0);
  if (sp->sp_total_function == 's') leave(0);
  
  if (argc > 1)
  {
    if (strcmp(argv[1], "-m") == 0)
    {
      send_errors = 1;
      message_open();
    }
  }
  tmp_name(ed_name);
  ed = fopen(ed_name, "w");               /* open error file                 */
  if (ed == 0)
  {
    krash("rp_init", "tmp file", 0);
    exit(1);
  }

  for (k = 0; k < coh->co_ports; k++)
  {
    if (po[k].po_flags & IsCarousel) continue;

    sprintf(fd_name, "%s.%s", hw_name, basename(po[k].po_name));
    fd = fopen(fd_name, "r");
    if (fd == 0)                         /* aha 051801 */
       {  
          total += po[k].po_lights;      /* Add disabled port's lights */
          continue;
       }

    if      (po[k].po_flags & IsBasicFunction) bf_restoreplace(k);
    else if (po[k].po_flags & IsTotalFunction) ac_restoreplace(k);
    else if (po[k].po_flags & IsDummy)         df_restoreplace(k);
    else if (po[k].po_flags & IsFullFunction)  ff_restoreplace(k); 

    fclose(fd);
  }
  if (total != coh->co_light_cnt)
  {
    sprintf(text, "Total System Had %d Modules - Now Has %d Modules",
      coh->co_light_cnt, total);
    message(text, 1);
    leave(1);
  }
  leave(0);
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave(x)
register long x;
{
  long pid, stat;
  char text[80], prt_name[16];

  ss_close();
  co_close_save();

  if (send_errors) message_close();

  if (ed) fclose(ed);
  if (fd) fclose(fd);
  
  if (errors)
  {
    if ((pid = fork()) == 0)
    {
      execlp("prft", "prft", ed_name, tmp_name(prt_name),
        "sys/report/rp_report.h", 0);
      krash("leave", "prft load", 0);
      exit(1);
    }
    pid = wait(&stat);
  }
  else unlink(ed_name);

  exit(x);
}
/*-------------------------------------------------------------------------*
 *  Write Error Message
 *-------------------------------------------------------------------------*/
message(p, n)
register char *p;
register long n;
{
  char text[80];

  sprintf(text, "%s\n", p);
  fprintf(ed, "%s", text);

#ifdef DEBUG
  fprintf(stderr, "%s\n", p);
#endif

  errors += n;
  
  if (send_errors)
  {
    message_put(0, InitErrorMessageEvent, text, strlen(text));
  }
}
/*-------------------------------------------------------------------------*
 *  Restore Full Function Port
 *-------------------------------------------------------------------------*/
ff_restoreplace(n)
register long n;                           /* port number                    */
{
  register long count;
  unsigned char c;
  char text[80];

#ifdef DEBUG
  fprintf(stderr, "ff_restoreplace(%d)\n", n);
#endif

  count = 0;

  while (fread(&c, 1, 1, fd) == 1)
  {
    if (total >= coh->co_lights)
    {
      message("Too Many Lights In Hardware For Allocated Space", 1);
      leave(1);
    }
    if (hw[total].hw_type != c && (c != PM || hw[total].hw_type != BF))
    {
      sprintf(text, "Port %s HWIX %d Was %s Now Is %s",
      po[n].po_name, total+1, hname[hw[total].hw_type], hname[c]);
        
      message(text, 1);
      leave(1);
    }
    total++;
    count++;
  }
  if (count != po[n].po_lights)           /* check length of line            */
  {
    sprintf(text, "Port %s Was %d Modules - Now Has %d Modules",
      po[n].po_name, po[n].po_lights, count);
    message(text, 1);
    leave(1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Restore Basic Function Port
 *-------------------------------------------------------------------------*/
bf_restoreplace(n)
register long n;                           /* port number                    */
{
  register long j, tc_count, count;
  unsigned char c;
  char text[80];
  
#ifdef DEBUG
  fprintf(stderr, "bf_restoreplace(%d)\n", n);
#endif

  tc_count = count = 0;

  while (fread(&c, 1, 1, fd) == 1)
  {
    tc_count++;

#ifdef DEBUG
  fprintf(stderr, "port:%d  tc:%d  pi:%d\n", n, tc_count, c);
#endif
    
    if (total >= coh->co_light_cnt)
    {
      message("Too Many Lights In Hardware For Allocated Space", 1);
      leave(1);
    }
    for (j = 0; total < coh->co_light_cnt; total++)
    {
      if (hw[total].hw_controller != tc_count) break;
      if (hw[total].hw_type == PI) j++;
      count++;
    }
#ifdef DEBUG
  fprintf(stderr, "found pi:%d\n", j);
#endif

    if (j != c)                           /* different PI Count              */
    {
      sprintf(text, "Port %s TC %d - Had %d PI's Now Has %d PI's",
          po[n].po_name, tc_count, j, c);

      message(text, 1);
      leave(1);
    }
  }
  if (count != po[n].po_lights)           /* check length of line            */
  {
    sprintf(text, "Port %s Had %d Modules - Now Has %d Modules",
      po[n].po_name, po[n].po_lights, count);
    message(text, 1);
    leave(1);
  }
  if (tc_count != po[n].po_controllers)
  {
    sprintf(text, "Port %s Had %d Contollers - Now Has %d Controllers",
      po[n].po_name, po[n].po_controllers, tc_count);
    message(text, 1);
    leave(1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Restore Total Function Port
 *-------------------------------------------------------------------------*/
ac_restoreplace(n)
register long n;                           /* port number                    */
{
  register long count;
  char text[80];
  long last_ac;
  
  struct
  {
    unsigned char ac;
    unsigned char mod;
    unsigned char type;
  } ac_rec;

#ifdef DEBUG
  fprintf(stderr, "ac_restoreplace(%d)\n", n);
#endif

  count = 0;

  while (fread(&ac_rec, 1, 3, fd) == 3)
  {
    
#ifdef DEBUG
    fprintf(stderr, "ac=%d mod=%d type=%d last=%d\n",
      ac_rec.ac, ac_rec.mod, ac_rec.type, last_ac);
      
	fflush(stderr);
#endif       
    
    if (total >= coh->co_light_cnt)
    {
      message("Too Many Lights In Hardware For Allocated Space", 1);
      leave(1);
    }
    last_ac = ac_rec.ac;
    
    if (hw[total].hw_controller < ac_rec.ac)
    {
      sprintf(text, "Port %s CM %d - Module Bad/Removed/Break At Module %d",
        po[n].po_name, hw[total].hw_controller, hw[total].hw_mod_address+1);

      message(text, 1);
      leave(1);
    }
    else if (hw[total].hw_controller > ac_rec.ac)
    {
      sprintf(text, "Port %s CM %d - Module(s) Added At Module %d",
        po[n].po_name, ac_rec.ac, ac_rec.mod+1);

      message(text, 1);
      leave(1);
    }
    if (hw[total].hw_type != ac_rec.type)
    {
      sprintf(text, "Port %s CM %d - Module %d Was %s Now %s",
        po[n].po_name, hw[total].hw_controller, hw[total].hw_mod_address+1,
        hname[hw[total].hw_type], hname[ac_rec.type]);

      message(text, 1);
      leave(1);
    }
    if (hw[total].hw_mod_address != ac_rec.mod)
    {
      sprintf(text, "Port %d CM %d - Module %d Mismatch",
        po[n].po_name, hw[total].hw_controller, hw[total].hw_mod_address+1);
      
      message(text, 1);
      leave(1);
    }
    count++;
    total++;
  }
  if (count != po[n].po_lights)           /* check length of line            */
  {
    sprintf(text, "Port %s Had %d Modules - Now Has %d Modules",
      po[n].po_name, po[n].po_lights, count);
    message(text, 1);
    leave(1);
  }
/*
  if (last_ac.ac != po[n].po_controllers)
  {
    sprintf(text, "Port %d Had %d Contollers - Now Has %d Controllers",
      n, po[n].po_controllers, last_ac);
    message(text, 1);
    leave(1);
  }
*/
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Restore Dummy Function Port
 *-------------------------------------------------------------------------*/
df_restoreplace(n)
register long n;                           /* port number                    */
{
  char text[80];
  long count;

#ifdef DEBUG
  fprintf(stderr, "df_restoreplace(%d)\n", n);
#endif

  if (fread(&count, 4, 1, fd) != 1)
  {
    message("No Map File For Dummy Port", 1);
    leave(1);
  }
  if (count != po[n].po_lights)           /* check length of line            */
  {
    sprintf(text, "Port %s Was %d Modules - Now Has %d Modules",
      po[n].po_name, po[n].po_lights, count);
    message(text, 1);
    leave(1);
  }
  while (count > 0)
  {
    if (total >= coh->co_lights)
    {
      message("Too Many Lights In Hardware For Allocated Space", 1);
      leave(1);
    }
    if (hw[total].hw_type != PM)
    {
      sprintf(text, "Port %s HWIX %d Was %s Now Is PM",
      po[n].po_name, total+1, hname[hw[total].hw_type]);
        
      message(text, 1);
      leave(1);
    }
    total++;
    count--;
  }
  return 0;
}
                                                                               
/* end of rp_init.c */
