/* #define DEBUG */
#define TCBL
#define MIO
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
 *  07/07/94   |  tjt  Add error messages (-m option).
 *  03/16/94   |  tjt  Add type PM4.
 *  03/17/95   |  tjt  Add ZC2 and PM2 modules.
 *  04/28/95   |  tjt  Add dummy port type.
 *  05/20/96   |  tjt  Add box full module.
 *  01/06/97   |  tjt  Add PM6 module.
 *  01/07/96   |  sg   Add full function zone controller.
 *  05/26/98   |  tjt  Add IO module scanner.
 *  05/26/98   |  tjt  Remove full function ports.
 *-------------------------------------------------------------------------*/
static char hw_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <errno.h>
#include "file_names.h"
#include "message_types.h"
#include "ss.h"
#include "co.h"

extern leave();

FILE *fd;                                 /* hardware map descriptor         */
char fd_name[40];                         /* hardware map file               */

FILE *ed;                                 /* error file name                 */
char ed_name[16];                         /* error file name                 */

long errors = 0;                          /* error count                     */
long total  = 0;
long bl_mod = 0;
long zc_mod = 0;
long pm_mod = 0;
long bf_mod = 0;
long io_mod = 0;									/* F052698 - added					  */

#ifdef TCBL
long tb_mod = 0;
#endif
#ifdef MIO
long mio_mod = 0;
#endif
long send_errors = 0;

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  char text[80];
  
  setpgrp();                              /* krash only this task            */
  putenv("_=hw_init");                    /* name to environment             */
  chdir(getenv("HOME"));                  /* to home directory               */

  ss_open();
  co_open();

  if (argc > 1)
  {
    if (strcmp(argv[1], "-m") == 0)
    {
      send_errors = 1;
      message_open();
    }
  }
  coh->co_port_cnt = 0;                   /* old port count                  */

  memset(hw, 0, coh->co_lights * sizeof(struct hw_item));

  for (k = 0; k < coh->co_ports; k++)     /* clear old counts                */
  {
    if (po[k].po_flags) coh->co_port_cnt = k + 1;

    po[k].po_products    = 0;
    po[k].po_lights      = 0;
    po[k].po_controllers = 0;
    memset(po[k].po_count, 0, sizeof(po[k].po_count));
  }
  if (sp->sp_basic_function == 's' || sp->sp_total_function == 's')
  {
    leave(0);
  }
  tmp_name(ed_name);
  ed = fopen(ed_name, "w");               /* open error file                 */
  if (ed == 0)
  {
    krash("hw_init", "tmp file", 0);
    exit(1);
  }
  for (k = 0; k < coh->co_ports; k++)
  {
    if (!po[k].po_flags) continue;

    coh->co_port_cnt = k + 1;

    if (po[k].po_flags & IsCarousel) continue;
 
    sprintf(fd_name, "%s.%s", hw_name, basename(po[k].po_name));
    
    errno = 0;
    fd = fopen(fd_name, "r");
    
#ifdef DEBUG
    system("pwd");
    fprintf(stderr, "fd_name=[%s] fd=%x errno=%d\n", fd_name, fd, errno);
#endif

    if (fd == 0)
    {
      sprintf(text, "No Lights Found On Port %s", po[k].po_name);
      message(text, 1);
      leave(1);
    }
    if (po[k].po_flags & IsBasicFunction)      bf_initialize(k);
    else if (po[k].po_flags & IsTotalFunction) ac_initialize(k);
    else if (po[k].po_flags & IsDummy)         df_initialize(k);
    
    fclose(fd);
  }
  coh->co_light_cnt = total;
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
  /* if (fd) fclose(fd); */ /* for linux */

  if (errors)
  {
    if ((pid = fork()) == 0)
    {
      execlp("prft", "prft", ed_name, tmp_name(prt_name),
      "sys/report/init_report.h", 0);
      krash("leave", "prft", 0);
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
  errors += n;

  if (send_errors)
  {
    message_put(0, InitErrorMessageEvent, text, strlen(text));
  }
}
/*-------------------------------------------------------------------------*
 *  Initialize Basic Function Port
 *-------------------------------------------------------------------------*/
bf_initialize(n)
register long n;
{
  register long j, tc_count;
  unsigned char c;
  
  tc_count = 0;

  while (fread(&c, 1, 1, fd) == 1)
  {
    tc_count++;

    po[n].po_lights   += c;
    po[n].po_count[PI - 1] += c;
    
    for (j = 0; c > 0; c--, j++)
    {
      if (total >= coh->co_lights)
      {
        message("Too Many Lights In Hardware For Allocated Space", 1);
        leave(1);
      }
      if (pm_mod >= coh->co_modules)
      {
        message("Too Many Modules In Hardware For Allocated Space", 1);
        leave(1);
      }
      hw[total].hw_type  = PI;            /* pick indicator type             */
      hw[total].hw_mod   = ++pm_mod;      /* pick module/indicator           */
      hw[total].hw_controller = tc_count; /* terminal controller             */
      hw[total].hw_mod_address = j;       /* pi number on tc                 */
      total++;
    }
  }
  po[n].po_controllers = tc_count;
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Initialize Total Function Port
 *-------------------------------------------------------------------------*/
ac_initialize(n)
register long n;
{
  struct
  {
    unsigned char ac;
    unsigned char mod;
    unsigned char type;
  } ac_rec;
  
  while (fread(&ac_rec, 1, 3, fd) == 3)
  {
    switch(ac_rec.type)
    {
      case BL:

        if (sp->sp_pickline_view == 'y')
        {
          memset(blv[bl_mod].hw_display, 0x20, 1);
        }
        hw[total].hw_type        = BL;
        hw[total].hw_state       = 0;
        hw[total].hw_mod         = ++bl_mod;
        hw[total].hw_controller  = ac_rec.ac;
        hw[total].hw_mod_address = ac_rec.mod;

        po[n].po_count[BL - 1]  += 1;
        break;
        
      case ZC:										/* F052698 - full function only	  */

        if (sp->sp_pickline_view == 'y')
        {
          memset(zcv[zc_mod].hw_display, 0x20, 16);
        }
        hw[total].hw_type        = ZC;
        hw[total].hw_state       = 0;
        hw[total].hw_mod         = ++zc_mod;
        hw[total].hw_controller  = ac_rec.ac;
        hw[total].hw_mod_address = ac_rec.mod;

        po[n].po_count[ZC - 1]   += 1;		/* counted separately from ZC2	  */
        break;
        
      case ZC2:

        if (sp->sp_pickline_view == 'y')
        {
          memset(zcv[zc_mod].hw_display, 0x20, 16);
        }
        hw[total].hw_type        = ZC2;
        hw[total].hw_state       = 0;
        hw[total].hw_mod         = ++zc_mod;
        hw[total].hw_controller  = ac_rec.ac;
        hw[total].hw_mod_address = ac_rec.mod;

        po[n].po_count[ZC2 - 1] += 1;
        break;
        
      case PM2:
      case PM4:
      case PM6:
      
        if (pm_mod >= coh->co_modules)
        {
          message("Too Many Modules In Hardware For Allocated Space", 1);
          leave(1);
        }
        if (sp->sp_pickline_view == 'y')
        {
          memset(pmv[pm_mod].hw_display, 0x20, 6);
        }
        hw[total].hw_type        = ac_rec.type;
        hw[total].hw_state       = 0;
        hw[total].hw_mod         = ++pm_mod;
        hw[total].hw_controller  = ac_rec.ac;
        hw[total].hw_mod_address = ac_rec.mod;

        po[n].po_count[ac_rec.type - 1] += 1;
        break;

      case BF:

        hw[total].hw_type        = BF;
        hw[total].hw_state       = 0;
        hw[total].hw_mod         = ++bf_mod;
        hw[total].hw_controller  = ac_rec.ac;
        hw[total].hw_mod_address = ac_rec.mod;

        po[n].po_count[BF - 1] += 1;
        break;
      
      case IO:										/* F052698 - scanner					  */

        hw[total].hw_type        = IO;
        hw[total].hw_state       = 0;
        hw[total].hw_mod         = ++io_mod;
        hw[total].hw_controller  = ac_rec.ac;
        hw[total].hw_mod_address = ac_rec.mod;

        po[n].po_count[IO - 1] += 1;
        break;
#ifdef TCBL
      case TB:
        hw[total].hw_type        = TB;
        hw[total].hw_state       = 0;
        hw[total].hw_mod         = ++tb_mod;
        hw[total].hw_controller  = ac_rec.ac;
        hw[total].hw_mod_address = ac_rec.mod;

        po[n].po_count[TB - 1] += 1;
        break;
#endif
    }
    total++;
    po[n].po_lights += 1;                 /* total lights on this port       */
  }
  if (po[n].po_lights > 0) po[n].po_controllers = ac_rec.ac;
  return 0;
}

/*--------------------------------------------------------------------------*
 *  Initialize Dummy Function Port
 *--------------------------------------------------------------------------*/
df_initialize(n)
register long n;
{
  long count;
  
  if (fread(&count, 4, 1, fd) != 1)
  {
    message("No Map File For Dummy Port", 1);
    leave(1);
  }
  if (total + count >= coh->co_lights)
  {
    message("Too Many Lights In Hardware For Allocated Space", 1);
    leave(1);
  }
  if (pm_mod + count >= coh->co_modules)
  {
    message("Too Many Modules In Hardware For Allocated Space", 1);
    leave(1);
  }
  return 0;
}
                                                                               
/* end of hw_init.c */
