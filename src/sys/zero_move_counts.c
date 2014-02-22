/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear current/cummulate counts in pmfile.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/16/93   |  tjt  Added to mfc.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char zero_move_count_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      zero_move_counts.c      screen 7.15                             */
/*                                                                      */
/*      this routine clears the current and possibly cumulative         */
/*      movement history data from the pmfile database. if data is      */
/*      cleared, the appropriate start times in the sys/imt file        */
/*      are set to the current time.                                    */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "getparms.h"
#include "imt.h"
#include "zero_move_counts.t"
#include "Bard.h"
#include "bard/pmfile.h"
#include "language.h"

extern leave();

pmfile_item pm;

short ONE = 1;
short LPL = 8;

char buf[9];

struct fld_parms fld[] ={

  {6,40,7,1,&LPL,"Enter Pickline",'a'},  
  {7,69,7,1,&ONE,"Purge Current Movement History Counts? (y/n)",'a'},
  {8,69,7,1,&ONE,"Purge Current and Cumulative History Counts? (y/n)",'a'}
};

main()
{
  register struct hw_item   *h;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k;
  unsigned char t;                        /* terminating character           */
  unsigned long pickline;                 /* numeric pickline                */
  short purge_current;                    /* clear current counts            */
  short purge_cumulative;                 /* clear current and cumul. counts */
  long now;
  
  putenv("_=zero_move_counts");
  chdir(getenv("HOME"));
  
  open_all();                             
  
  fix(zero_move_counts);
  sd_screen_off();
  sd_clear_screen();
  sd_text(zero_move_counts);
  sd_screen_on();
 
  if (IS_ONE_PICKLINE) pickline = op_pl;  
  else pickline = op_pl;

  while(1)
  {
    sd_prompt(&fld[1], 0);
    sd_prompt(&fld[2], 0);

    purge_current=0;                      /* clear flags                     */
    purge_cumulative=0;

    while (1)                             /* gather parms                  */
    {
      if (SUPER_OP)
      {
        sprintf(buf, "%d", op_pl);
        sd_prompt(&fld[0], 0);
        t = sd_input(&fld[0], 0, 0, buf, 0);
        if (t == EXIT) leave();
        else if(t == UP_CURSOR) continue;
        pickline = pl_lookup(buf, 0);
        if (pickline < 0)
        {
          eh_post(ERR_PL, buf);
          continue;
        }
        if (pickline > 0) 
        {
          sprintf(buf, "%d", pickline);
          chng_pkln(buf);                  
        }
      }
      while (1)
      {
        memset(buf, 0, 2);
        sd_prompt(&fld[1], 0);
        t = sd_input(&fld[1], 0, 0, buf, 0);
        if (t == EXIT) leave();
        else if(t == UP_CURSOR) break;
        if (code_to_caps(*buf) == 'y') purge_current = 1;
        else if (code_to_caps(*buf) != 'n')
        {
          eh_post(ERR_YN, 0);
          continue;
        }
        if (t == RETURN) break;
        
        while (1)
        {
          memset(buf, 0, 2);
          sd_prompt(&fld[2], 0);
          t = sd_input(&fld[2], 0, 0, buf, 0);
          if (t == EXIT) leave();
          else if(t == UP_CURSOR) break;
          if (code_to_caps(*buf) == 'y') purge_cumulative = 1;
          else if (code_to_caps(*buf) != 'n')
          {
            eh_post(ERR_YN, 0);
            continue;
          }
          if (t == RETURN) break;
        }
        if (t == RETURN) break;
      }
      if (!purge_current && !purge_cumulative) continue;
      if (t == UP_CURSOR) continue;
      if (t == RETURN) break;
    }
    begin_work();

    pmfile_setkey(1);

    while (!pmfile_next(&pm, LOCK))
    {
      if (pickline > 0)
      {
        if (pm.p_pmodno > coh->co_prod_cnt) continue;
        k = pw[pm.p_pmodno - 1].pw_ptr;
        h = &hw[k - 1];
        if (!h->hw_bay) continue;
        b = &bay[h->hw_bay - 1];
        if (!b->bay_zone) continue;
        z = &zone[b->bay_zone - 1];
        if (z->zt_pl != pickline) continue;
      }
      if (purge_cumulative)
      {
        pm.p_cmunits = 0;
        pm.p_cmlines = 0;
        pm.p_cmrecpt = 0;
      }
      if (purge_current || purge_cumulative)  
      {
        pm.p_cuunits = 0;
        pm.p_culines = 0;
        pm.p_curecpt = 0;
     }   
     pmfile_replace(&pm);
     commit_work();
     begin_work();
    }
    commit_work();
    
    now = time(0);
    
    imt_load();
    
    for (k = 0; k < PicklineMax; k++)
    {
      if(pickline && pickline != k+1) continue;
      if (purge_cumulative) imt[k].imt_cum = now;
      imt[k].imt_cur = now;
    }
    imt_unload();
    eh_post(ERR_CONFIRM, "Zero Movement");
  }                                     /* END MAIN LOOP FOR REPEATING SCREEN*/
}                                         /* end prog                        */

open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  co_open();
  pmfile_open(AUTOLOCK);
  getparms(0);
}

close_all()
{
  pmfile_close();
  sd_close();
  co_close();
  ss_close();
  database_close();
}

leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
}

/* end of zero_move_counts.c */
