/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Delete PM screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/03/93   |  tjt  Added to mfc.
 *  07/21/95   |  tjt  Revise Bard calls.
 *-------------------------------------------------------------------------*/
static char delete_pm_screen_c[] = "%Z% %M% %I% (%G% - %U%)";
/************************************************************************/
/*                                                                      */
/*      delete_pm_screen.c              screen 7.21                     */
/*                                                                      */
/*      pick modules may be marked for deletion if:                     */
/*                                                                      */
/*              pick module NOT in UNIFY...     ||                      */
/*              pick module in UNIFY and deassigned.                    */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include "iodefs.h"
#include "ss.h"
#include "sd.h"
#include "eh_nos.h"
#include "xf_defs.h"
#include "delete_pm_screen.t"
#include "Bard.h"
#include "bard/pmfile.h"

#define BUF_SIZE        6

short LMOD;

static struct fld_parms fld =
  {6,52,12,1,&LMOD,"Enter Module Number to Be Deleted",'n'};

pmfile_item pm;

char text[80];

main()
{
  short rm,ret,i,n;
  unsigned char t;
  char buf[BUF_SIZE];

  putenv("_=delete_pm_screen");
  chdir(getenv("HOME"));

  open_all();
 
        /* set lengths into field structures */

  LMOD = rf->rf_mod;

  fix(delete_pm_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(delete_pm_screen);
  sd_screen_on();  

  while(1)                                /* begin massive loop              */
  {
                /* clear input buffers */

    memset(buf, 0, BUF_SIZE);

    sd_prompt(&fld,0);

                /* main loop to gather input */

    while(1)
    {
      t = sd_input(&fld,0,&rm,buf,0);

                        /* erase previous SKU */

      sd_cursor(0,7,10);
      sd_clear_rest();

      if (t == EXIT) leave();
      if (sp->sp_running_status == 'y')
      {
        eh_post(ERR_IS_CONFIG, 0);
        continue;
      }
      if (t != RETURN) continue;

      if(!(*buf))                       /* no data entered                 */
      {
        eh_post(ERR_PM_INV,"");
        continue;
      }
      pm.p_pmodno = atol(buf);          /* cnvrt PM to int                 */
        
      begin_work();
      if (!pmfile_read(&pm, LOCK))
      {
        if (pm.p_pmsku[0] > 0x20)
        {
          sd_cursor(0,7,12);
          sd_text("SKU: ");
          sd_text_2(pm.p_pmsku, rf->rf_sku);
          sd_text("   Stkloc: ");
          sd_text_2(pm.p_stkloc, 6);
        
          eh_post(ERR_PM_DEL,0);
          commit_work();
          continue;
        }
        pmfile_delete();
      }
      commit_work();
                                /* everything OK. to delete.  */

      sd_wait();
      
      pm.p_pmodno = atol(buf);
      pmfile_startkey(&pm);
      pm.p_pmodno = 0x7fff;
      pmfile_stopkey(&pm);

      begin_work();
      while (!pmfile_next(&pm, LOCK))
      {
        pm.p_pmodno -= 1;
        pmfile_update(&pm);
        commit_work();
        begin_work();
      }
      commit_work();
      eh_post(ERR_CONFIRM, "Module delete");
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
  sd_open(leave);
  ss_open();
  database_open();
  pmfile_open(AUTOLOCK);
  pmfile_setkey(1);
  log_open(AUTOLOCK);
}
close_all()
{
  ss_close();
  pmfile_close();
  log_close();
  database_close();
  sd_close();
}


/* end of delete_pm_screen.c */
