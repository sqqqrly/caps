/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Insert PM Screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   10/3/93   |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char ins_pm_screen_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      ins_pm_screen.c         screen 7.20                             */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "ins_pm_screen.t"
#include "Bard.h"
#include "bard/pmfile.h"

#define BUF_SIZE        6

pmfile_item pm;

short LMOD;

static struct fld_parms fld = 
  {7,55,20,1,&LMOD,"Before Module Number",'n'};

char text[80];

main()
{
  short rm;
  short ret;
  unsigned char t;
  char buf[BUF_SIZE];

  short i;
  short n;

  putenv("_=ins_pm_screen");
  chdir(getenv("HOME"));

  open_all();
        
        /* set lengths into field structures */

  LMOD = rf->rf_mod;

  fix(ins_pm_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(ins_pm_screen);
  sd_screen_on();

  while(1)                                /* begin massive loop              */
  {
    memset(buf, 0, BUF_SIZE);

    sd_prompt(&fld, 0);

    while(1)
    {
      t = sd_input(&fld, 0, 0, buf, 0);

      if(t == EXIT) leave();

      if (sp->sp_running_status == 'y')
      {
        eh_post(ERR_IS_CONFIG, 0);
        continue;
      }
      if(!(*buf))                        /* no data entered                 */
      {
        eh_post(ERR_PM_INV,"");
        continue;
      }
      if (t != RETURN) continue;
      
      sd_wait();
      
      pm.p_pmodno = atol(buf);
      pmfile_startkey(&pm);
      pm.p_pmodno = 50000;
      pmfile_stopkey(&pm);
      
      begin_work();
      while (!pmfile_next(&pm, LOCK))
      {
        pm.p_pmodno += 50001;
        pmfile_update(&pm);
        commit_work();
        begin_work();
      }
      commit_work();
      
      pm.p_pmodno = 50000;
      pmfile_startkey(&pm);
      pm.p_pmodno = 99999;
      pmfile_stopkey(&pm);
      
      begin_work();
      while (!pmfile_next(&pm, LOCK))
      {
        pm.p_pmodno -= 50000;
        pmfile_update(&pm);
        commit_work();
        begin_work();
      }
      commit_work();
      
      sprintf(text, "Insert module %s", buf);
      log_entry(text);
      
      eh_post(ERR_CONFIRM, "Insert");
    }
  }                                       /* end massive while(1)loop        */
}
leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
}

open_all()
{
  database_open();
  pmfile_open(AUTOLOCK);
  pmfile_setkey(1);
  log_open(AUTOLOCK);
  ss_open();
  sd_open(leave);
}

close_all()
{
  sd_close();
  ss_close();
  pmfile_close();
  log_close();
  database_close();
}


/* end of ins_pm_screen.c */
