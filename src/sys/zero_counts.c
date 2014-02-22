/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/11/93   |  tjt  Added to mfc.
 *  06/08/94   |  tjt  Added clear zone counts.
 *  09/13/94   |  tjt  Added correct remaining counts.
 *  09/27/94   |  tjt  Fix clear remaining counts for speed. 
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  04/19/96   |  tjt  Zero all label counts.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  12/02/96   |  tjt  Add units and lines to location.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char zero_counts_c[] = "%Z% %M% %I% (%G% - %U%)";

/****************************************************************************/
/*                                                                          */
/*                                Zero Counts                               */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "zero_counts.t"
#include "ss.h"
#include "eh_nos.h"
#include "co.h"
#include "of.h"
#include "language.h"

#include "Bard.h"

#define NUM_PROMPTS  4
#define BUF_SIZE     9

/*
 *Global Variables
 */
short ONE = 1;
short LPL = 8;

struct fld_parms fld[] ={
  {6,54,15,1,&ONE,"Zero Shorts Printed Counts? (y/n)",'a'},
  {7,54,15,1,&ONE,"Zero Completed Count? (y/n)",'a'},
  {8,54,15,1,&ONE,"Zero Remaining Count? (y/n)",'a'},
  {10,54,15,1,&LPL,"Enter Pickline",'a'},
};

short rm,ret,i,pickline,j;
unsigned char t;
char buf[NUM_PROMPTS][BUF_SIZE];

main()
{
  register struct oi_item *o;
  register struct zone_item *z;
  register struct bay_item  *b;
  register struct hw_item   *h;
  register struct pw_item   *p;
  
  extern leave();

  open_all();                             /*open all files                   */

  for(i = 0;i < NUM_PROMPTS;i++)          /*clear the buffers                */
  for(j = 0;j < BUF_SIZE;j++)
  buf[i][j] = 0;

  fix(zero_counts);
  sd_screen_off();
  sd_clear_screen();
  sd_text(zero_counts);
  sd_screen_on();
  
  while(1)
  {
    t = sd_input(&fld[0],sd_prompt(&fld[0],0),&rm,buf[0],0);
    if(t == EXIT) leave();
   
    if(code_to_caps(*buf[0]) == 'y')      /* zero the shorts counts          */
    {
      sp->sp_sh_printed = 0;
      sp->sp_sh_count = 0;
      sp->sp_rs_printed = 0;
      sp->sp_rs_count = 0;
      sp->sp_tl_printed = 0;
      sp->sp_tl_count = 0;
      sp->sp_sl_printed = 0;
      sp->sp_sl_count = 0;
      sp->sp_pl_printed = 0;
      sp->sp_pl_count = 0;
      sp->sp_bpl_printed = 0;
      sp->sp_bpl_count = 0;
      
      sp->sp_pl_order = 0;
      sp->sp_pl_print = 0;
      sp->sp_sl_order = 0;
      sp->sp_sl_print = 0;
      sp->sp_tl_order = 0;
      sp->sp_tl_print = 0;

      eh_post(ERR_CONFIRM,"Zero Counts");
      break;
    }
    else if(code_to_caps(*buf[0]) == 'n') break;

    else
    {
      eh_post(ERR_YN,0);
    }
  }
  if(sp->sp_config_status != 'y') leave();
    
  while(1)
  {
    while (1)
    {
      t = sd_input(&fld[1],sd_prompt(&fld[1],0),&rm,buf[1],0);
      if(t == EXIT) leave();
   
      if(code_to_caps(*buf[1]) == 'n' || code_to_caps(*buf[1]) == 'y') break;
 
      eh_post(ERR_YN,0);
    }
    while (1)
    {
      t = sd_input(&fld[2],sd_prompt(&fld[2],0),&rm,buf[2],0);
      if(t == EXIT) leave();
 
      if(code_to_caps(*buf[2]) == 'n' || code_to_caps(*buf[2]) == 'y') break;
 
      eh_post(ERR_YN,0);
    }
    if (code_to_caps(*buf[1]) == 'n' && code_to_caps(*buf[2]) == 'n') leave();

    if(IS_ONE_PICKLINE) pickline = op_pl;
    else
    {
      if(SUPER_OP) pickline_prompt();
      else pickline = op_pl;
    }
    if (pickline == 0 && code_to_caps(*buf[1]) == 'y')
    {
      for (i = 0; i < coh->co_pl_cnt; i++) /*for all picklines               */
      {
        pl[i].pl_complete = 0;
      }
      for (i = 0; i < coh->co_zone_cnt; i++)
      {
        zone[i].zt_count = 0;
      }
      eh_post(ERR_CONFIRM,"Zero counts");
    }
    else if (code_to_caps(*buf[1]) == 'y')
    {
      pl[pickline - 1].pl_complete = 0;

      for (i = 0; i < coh->co_zone_cnt; i++)
      {
        if (pickline == zone[i].zt_pl) zone[i].zt_count = 0;
      }
      eh_post(ERR_CONFIRM,"Zero counts");
    }
    if (code_to_caps(*buf[2]) == 'y')
    {
      sd_wait();
      
      for (i = 0; i < coh->co_pl_cnt; i++)
      {
        if (pickline && pl[i].pl_pl != pickline) continue;

        pl[i].pl_lines_to_go = 0;
        pl[i].pl_units_to_go = 0;
      }
      for (j = 0, p = pw; j < coh->co_prod_cnt; j++, p++)
      {
        h = &hw[p->pw_ptr - 1];
        if (h->hw_bay)
        {
          b = &bay[h->hw_bay - 1];
          if (b->bay_zone)
          {
            z = &zone[b->bay_zone - 1];
         
            if (pickline && z->zt_pl != pickline) continue;
            
            p->pw_units_to_go = 0;
            p->pw_lines_to_go = 0;
          }
        }
      }
      for (i = 0, o = oc->oi_tab; i < oc->of_size; i++, o++)
      {
        if (!o->oi_pl || !o->oi_on) continue;

        if (pickline && o->oi_pl != pickline) continue;
        if (o->oi_queue == OC_COMPLETE) continue;
        
        op_rec->pi_pl  = o->oi_pl;
        op_rec->pi_on  = o->oi_on;
        op_rec->pi_mod = 1;
        pick_startkey(op_rec);
      
        op_rec->pi_mod = coh->co_prod_cnt;
        pick_stopkey(op_rec);

        begin_work();
        while (!pick_next(op_rec, NOLOCK))
        {
          if (op_rec->pi_flags & PICKED)  continue;
          if (op_rec->pi_flags & NO_PICK) continue;
        
          pl[op_rec->pi_pl - 1].pl_units_to_go += op_rec->pi_ordered;
          pl[op_rec->pi_pl - 1].pl_lines_to_go += 1;
          
          pw[op_rec->pi_mod - 1].pw_units_to_go += op_rec->pi_ordered;
          pw[op_rec->pi_mod - 1].pw_lines_to_go += 1;
        }
        commit_work();
      }
      eh_post(ERR_CONFIRM, "Zero Remaining:");
    }
    if (SUPER_OP && pickline != 0 && (!(IS_ONE_PICKLINE)))
    {
      sd_cursor(0,9,1);
      sd_clear_line();
      sd_cursor(0,10,1);
      sd_clear_line();
      continue;
    }
    leave();
  }
}
/*
 *prompt and check the pickline
 */
pickline_prompt()
{
  while(1)
  {
    for(i = 0; i < BUF_SIZE;i++) buf[3][i] = 0;

    t = sd_input(&fld[3],sd_prompt(&fld[3],0),&rm,buf[3],0);
    if(t == EXIT) leave();

    pickline = pl_lookup(buf[3], 0);
  
    if (!pickline) return;

    if(pickline < 0)
    {
      eh_post(ERR_PL,buf[3]);
      continue;
    }
    return;
  }
}
leave()
{
  close_all();
  execlp("operm", "operm", 0);
  krash("leave", "operm load", 1);
}
/*
 *open all files
 */
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  co_open();
  oc_open();
  od_open();
  getparms(0);
}
close_all()
{
  sd_close();
  ss_close();
  co_close();
  oc_close();
  od_close();
  database_close();
}
/*
 *close all files and transfer control back to calling program
 */

/* end of zero_counts.c */
