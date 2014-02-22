/*-----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     System status display without remaining picks.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/27/93   |  tjt  Added to mfc.
 *  05/13/94   |  tjt  Fix order numbers 1..6 right justified.
 *  11/01/94   |  tjt  Fix pending max order number.
 *  05/07/95   |  tjt  Fix backward display.
 *  06/03/95   |  tjt  Add pickline input by names.
 *  06/04/95   |  tjt  Add sp_pl_by_name display.
 *  06/27/95   |  tjt  Fix pnd_group as GroupLength.
 *  06/28/95   |  tjt  Add show disabled port/pickline.
 *  07/01/95   |  tjt  Fix symbolic queues.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  04/19/96   |  tjt  New short count names.
 *  04/19/96   |  tjt  Add sp_to_xmit.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  01/11/97   |  tjt  Add last purge time.
 *  04/18/97   |  tjt  Add language.h and code xlate.
 *-------------------------------------------------------------------------*/
static char sys_stat_c[] = "%Z% %M% %I% (%G% - %U%)";

/****************************************************************************/
/*                                                                          */
/*                          System Status Screen                            */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <signal.h>
#include "Bard.h"
#include "global_types.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "language.h"
#include "sys_stat.t"
#include "display_pending.t"
#include "eh_nos.h"
#include "co.h"
#include "of.h"
#include "getparms.h"

#define BUF             9
#define COLS            7
#define WIDTH           8
#define LENGTH          11

short ONE = 1;
short LPL = 8;
short rm  = 0;

char buf[BUF] = {0};

struct fld_parms fld[] = {

  {23,30,2,1,&LPL,"Enter Pickline",'a'},
  {23,30,2,1,&ONE,"More? (y/n)",'a'},
  {23,30,2,1,&ONE,"Display Pending? (y/n)",'a'}
};
unsigned char t;
long pickline = -1;

char lines[LENGTH][PicklineMax][WIDTH];
long data  = 0;
long where = 0;

/*-------------------------------------------------------------------------*
 *  Main Program
 *-------------------------------------------------------------------------*/
main()
{
  register char *p;
  register long n, flag;
  extern refresh();
  extern leave();

  putenv("_=sys_stat");
  chdir(getenv("HOME"));
  
  open_all();
  
  sd_screen_off();
  sd_clear_screen();
  show_top(1920);
  sd_screen_on();

/*
 *prompt for pickline if super_op
 */
  while(1)
  {
    if(IS_ONE_PICKLINE) pickline = op_pl;
    else
    {
      if(SUPER_OP)
      {
        sd_cursor(0, 23, 1);
        sd_clear_line();
        sd_prompt(&fld[0], rm);

        while(1)
        {
          memset(buf, 0, BUF);

          t = sd_input(&fld[0], 0, &rm, buf, 0);
          if(t == EXIT) leave();

          pickline = pl_lookup(buf, 0);
          if (!pickline) break;               /* all picklines          */

          if (pickline < 0)
          {
            eh_post(ERR_PL, buf);
            continue;
          }
          sprintf(buf, "%d", pickline);
          chng_pkln(buf);
          break;
        }
      }
      else pickline = op_pl;
    }
    alarm(0);
    refresh();

    if(pickline == 0 || sp->sp_pending_ops == 'n')
    {
      sd_cursor(0, 23, 1);
      sd_clear_line();
      sd_prompt(&fld[1], 0);

      while(1)
      {
        sd_cursor(0, 23, 32);
        sd_text("(Exit, Forward, or Backward)");
        memset(buf, 0, BUF);

        t = sd_input(&fld[1], 0, &rm, buf, 0);
        alarm(0);

        if (t == EXIT) leave();
        n = sd_more(t, code_to_caps(*buf)); /* F041897 */
 
        if (!n) leave();
        if (n == 1)
        {
          where += COLS;
          if (where + COLS > data) where = data - COLS;
          if (where < 0) where = 0;
        }
        else if (n == 2)
        {
          where -= COLS;
          if (where < 0) where = 0;
        }
        else if (n == 3) break;
        else
        {
          eh_post(ERR_YN, 0);
          continue;
        }
        refresh();
      }
      where = data = 0;                 /* stop display                   */
      alarm(0);                         /* stop refresh                   */

      if (t == UP_CURSOR) continue;
    }
    sd_cursor(0, 23, 1);
    sd_clear_line();
    flag = 0;

    while(sp->sp_pending_ops == 'y')
    {
      memset(buf, 0, BUF);
      if (flag) sd_prompt(&fld[1], 0);
      else sd_prompt(&fld[2], 0);
      t = sd_input(&fld[2], 0, &rm, buf, 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;
      *buf = code_to_caps(*buf);              /* to lower case F041897       */

      if(t == RETURN)
      {
        if(*buf == 'y') 
        {
          where = data = 0;                   /* stop display                */
          alarm(0);                           /* stop refresh                */
          flag = 1; 
          display_pend();
        }
        else if(*buf == 'n') break;
        else eh_post(ERR_YN,0);
      }
    }
    if (flag) 
    {
      sd_clear_screen();
      show_top(1920); 
    }
    refresh();
  }
}
/*-------------------------------------------------------------------------*
 *  Show Top Of Screen
 *-------------------------------------------------------------------------*/
show_top(n)
register long n;
{
  register long i, comp_count, uw_count, size;
  char temp[24][80], buffer[16];

  memset(temp, 0, 1920);
  memcpy(temp, sys_stat, sizeof(sys_stat));
  fix(temp);

  uw_count = comp_count = 0;

  if (sp->sp_config_status == 'y')
  {
    for(i = 0; i < coh->co_pl_cnt ; i++)
    {
      if(pl[i].pl_pl) comp_count += pl[i].pl_complete;
    }
  }
  for (i = 0; i < PicklineMax; i++) 
  {
    uw_count += oc->oc_tab[i].oc_uw.oc_count;
  }
  sprintf(buffer, " %6d", uw_count);      /* active orders count             */
  memcpy(&temp[5][22], buffer, strlen(buffer));

  temp[5][77] =  sp->sp_oi_mode;          /* order_input status              */

  sprintf(buffer, " %6d", comp_count);	   /* completed orders count          */
  memcpy(&temp[6][22], buffer, strlen(buffer));

  temp[6][77] = sp->sp_to_mode;           /* transaction output status       */

  sprintf(buffer, " %6d", sp->sp_sh_printed);    /* shorts printed count     */
  memcpy(&temp[7][22], buffer, strlen(buffer));

  temp[7][77] = sp->sp_sp_flag;           /* shorts printerd flag            */

  sprintf(buffer, " %6d", sp->sp_sh_count); /* short queued for printing     */
  memcpy(&temp[8][22], buffer, strlen(buffer));

  temp[8][77] = sp->sp_to_flag;           /* transaction accum flag          */

  temp[9][28] = sp->sp_running_status;    /* system configured               */

  sprintf(buffer, " %6d", sp->sp_to_count - sp->sp_to_xmit); /* transactions */
  memcpy(&temp[9][71], buffer, strlen(buffer));

  memcpy(&temp[10][22], ctime(&oc->of_last_purge), 24);
  
  sd_cursor(0, 1, 1);
  sd_text_2(temp, n);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  store the data in a table
 *-------------------------------------------------------------------------*/
store_data()
{
  register long j;
  
  data = 0;                               /* no data                         */
        
  memset(lines, 0, LENGTH * PicklineMax * WIDTH);

  if (pickline < 0) return 0;

  if (pickline == 0)
  {
    for (j = 1; j <= coh->co_picklines; j++) data += store_pickline(j);
  }
  else data = store_pickline(pickline);

  if (where > data) where = 0;

  return 0;
}

/*-------------------------------------------------------------------------*
 *  Store One Pickline
 *-------------------------------------------------------------------------*/
store_pickline(j)
register long j;
{
  register long flag, last_queued, last_underway, comp_count, pend_count;
  register long queue_count, active_count, hold_count;
  register struct oc_item   *c;
  register struct zone_item *z;
  register struct bay_item  *b;
  char lock_flag, pickline_inhibit, last_group[GroupLength + 1];
  char work[16];
  
  if (j < 1 || j > coh->co_picklines) return 0;
  j--;

  flag = last_queued = last_underway = comp_count = pend_count = 0;
  queue_count = active_count = hold_count = 0;
  memset(last_group, 0, GroupLength + 1);
  
  if (sp->sp_config_status == 'y')
  {
    if (pl[j].pl_pl) flag = 1;
  }
  c = &oc->oc_tab[j];

  queue_count  = c->oc_high.oc_count + c->oc_med.oc_count + c->oc_low.oc_count;
  active_count = c->oc_uw.oc_count;
  hold_count   = c->oc_hold.oc_count;
  
  if (flag) comp_count = pl[j].pl_complete;

  if (!flag && !queue_count && !active_count && !hold_count) return 0;

  if (c->oc_low.oc_last)
  {
    last_queued = c->oc_low.oc_last;
  }
  else if (c->oc_med.oc_last)
  {
    last_queued = c->oc_med.oc_last;
  }
  else if (c->oc_high.oc_last)
  {
    last_queued = c->oc_high.oc_last;
  }
/*
 * find the last underway in the pickline
 */
  if (c->oc_uw.oc_last)
  {
    last_underway = c->oc_uw.oc_last;
    if (rf->rf_grp)
    {
      memcpy(last_group, oc->oi_tab[last_underway - 1].oi_grp, rf->rf_grp);
    }
  }
/*
 * pickline lock flag
 */
  lock_flag = pickline_inhibit = 0x20;
  
  if (flag)
  {
    if(pl[j].pl_flags & OrdersLocked) lock_flag = 'y';
    else  lock_flag = 'n';

    lock_flag = caps_to_code(lock_flag);  /* F041897 */
    
    if(pl[j].pl_flags &  SwitchesDisabled) pickline_inhibit = 'y';
    else  pickline_inhibit = 'n';
 
    z = &zone[pl[j].pl_first_zone - 1];
    if (z->zt_flags & SwitchesDisabled) pickline_inhibit = 'y';
    
    b = &bay[z->zt_first_bay - 1];
    if (po[b->bay_port - 1].po_disabled == 'y') pickline_inhibit = 'd';

    pickline_inhibit = caps_to_code(pickline_inhibit); /* F041897 */
  }
  pend_count = count_pending(j + 1);

  if (sp->sp_pl_by_name == 'y')
  {
    sprintf(lines[0][data], "%8.8s", pl[j].pl_name);
  }
  else sprintf(lines[0][data], "  %6d", j + 1);

  sprintf(lines[1][data], "       %c", pickline_inhibit);
  sprintf(lines[2][data], "  %6d",    queue_count);
  sprintf(lines[3][data], "  %6d",    hold_count);
  sprintf(lines[4][data], "  %6d",    active_count);
  sprintf(lines[5][data], "  %6d",    comp_count);

  if (last_underway) 
  {
    sprintf(lines[6][data], "  %6.6s", oc->oi_tab[last_underway - 1].oi_con);
  }
  else memset(lines[6][data], 0x20, 8);

  sprintf(lines[7][data], "%8.8s", last_group);

  if (last_queued) 
  {
    sprintf(lines[8][data], "  %6.6s", oc->oi_tab[last_queued - 1].oi_con);
  }
  else memset(lines[8][data], 0x20, 8);
  
  sprintf(lines[9][data], "       %c", lock_flag);
  sprintf(lines[10][data], "  %6d",   pend_count);

  return 1;
}
/*-------------------------------------------------------------------------*
 * display and modify all system status
 *-------------------------------------------------------------------------*/
display()
{
  store_data();                           /* store new information           */
  show_top(800);                          /* renew top of screen             */
  show_stat(where);                       /* show or clear data              */
  
  return 0;
}
/*-------------------------------------------------------------------------*
 * function to refresh the screen
 *-------------------------------------------------------------------------*/
refresh()
{
  display();
  signal(SIGALRM, refresh);
  alarm(op_refresh);
}
/*-------------------------------------------------------------------------*
 * show the status
 *-------------------------------------------------------------------------*/
show_stat(n)
register long n;
{
  register long k;
    
  if (!data) return 0;

  for (k = 0; k < LENGTH; k++)
  {
    sd_cursor(0, 12 + k, 23);
    sd_text_2(lines[k][n], 56);
    sd_clear_line();
  }
}
/*-------------------------------------------------------------------------*
 * count pending operations
 *-------------------------------------------------------------------------*/
count_pending(j)
register long j;
{
  register long count;
  struct pending_item x;
  
  pending_setkey(0);

  count = 0;

  begin_work();
  
  while (!pending_next(&x, NOLOCK))
  {
    if (x.pnd_pl == j) count++;
  }
  commit_work();
  return count;
}
/*-------------------------------------------------------------------------*
 * display pending operations
 *-------------------------------------------------------------------------*/
display_pend()
{
  register long j, k, n, flag;
  char work[12], temp[1924];
  struct pending_item x;
  
  memset(temp, 0, 1920);
  memcpy(temp, display_pending, sizeof(display_pending));
  fix(temp);
  sd_clear_screen();
  sd_cursor(0,1,1);
  sd_text(temp);

  k = 0;
  j = 1;
  
  begin_work();
  
  pending_setkey(1);
  x.pnd_pl = pickline ? pickline : 0;
  x.pnd_on = 0;
  pending_startkey(&x);

  x.pnd_pl = pickline ? pickline : PicklineMax;
  x.pnd_on = OrderMax;
  pending_stopkey(&x);

  flag = 0;

  sd_cursor(0, 6 + k, 4);

  while (!pending_next(&x, NOLOCK))
  {
    if ((x.pnd_flags & PENDING_HOLD) && x.pnd_on)
    {
      if (k > 15) break;
      if (!flag)
      {
        sd_text("Pending Hold Orders:");
        j = 30;
        flag = 1;
      }
      sprintf(work, "%d/%0*d", x.pnd_pl, rf->rf_on, x.pnd_on);
      n = strlen(work) + 2;
      if ((n + j) > 80) {k++; j = 1;}
      sd_cursor(0, 6 + k, j);
      sd_text(work);
      j += n;
    }
  }
  pending_setkey(1);
  x.pnd_pl = pickline ? pickline : 0;
  x.pnd_on = 0;
  pending_startkey(&x);

  x.pnd_pl = pickline ? pickline : PicklineMax;
  x.pnd_on = OrderMax;
  pending_stopkey(&x);

  if (flag)
  {
    k++; j = 1;
    sd_cursor(0, 6 + k, 4);
  }
  flag = 0;

  while (!pending_next(&x, NOLOCK))
  {
    if ((x.pnd_flags & PENDING_CANCEL) && x.pnd_on)
    {
      if (k > 15) break;
      if (!flag)
      {
        sd_text("Pending Cancel Orders:");
        j = 30;
        flag = 1;
      }
      sprintf(work, "%d/%0*d", x.pnd_pl, rf->rf_on, x.pnd_on);
      n = strlen(work) + 2;
      if ((n + j) > 80) {k++; j = 1;}
      sd_cursor(0, 6 + k, j);
      sd_text(work);
      j += n;
    }
  }
  pending_setkey(2);
  x.pnd_pl = pickline ? pickline : 0;
  memset(x.pnd_group, 0x20, GroupLength);
  pending_startkey(&x);

  x.pnd_pl = pickline ? pickline : PicklineMax;
  memset(x.pnd_group, 'z', GroupLength);
  pending_stopkey(&x);

  if (flag)
  {
    k++; j = 1;
    sd_cursor(0, 6 + k, 4);
  }
  flag = 0;

  while (!pending_next(&x, NOLOCK))
  {
    if ((x.pnd_flags & PENDING_HOLD) && !x.pnd_on)
    {
      if (k > 15) break;
      if (!flag)
      {
        sd_text("Pending Hold Groups:");
        j = 30;
        flag = 1;
      }
      sprintf(work, "%d/%*.*s", x.pnd_pl, rf->rf_grp, rf->rf_grp, x.pnd_group);
      n = strlen(work) + 2;
      if ((n + j) > 80) {k++; j = 1;}
      sd_cursor(0, 6 + k, j);
      sd_text(work);
      j += n;
    }
  }
  pending_setkey(2);
  x.pnd_pl = pickline ? pickline : 0;
  memset(x.pnd_group, 0x20, GroupLength);
  pending_startkey(&x);

  x.pnd_pl = pickline ? pickline : PicklineMax;
  memset(x.pnd_group, 'z', GroupLength);
  pending_stopkey(&x);

  if (flag)
  {
    k++; j = 1;
    sd_cursor(0, 6 + k, 4);
  }
  flag = 0;

  while (!pending_next(&x, NOLOCK))
  {
    if ((x.pnd_flags & PENDING_CANCEL) && !x.pnd_on)
    {
      if (k > 15) break;
      if (!flag)
      {
        sd_text("Pending Cancel Groups:");
        j = 30;
        flag = 1;
      }
      sprintf(work, "%d/%*.*s", x.pnd_pl, rf->rf_grp, rf->rf_grp, x.pnd_group);
      n = strlen(work) + 2;
      if ((n + j) > 80) {k++; j = 1;}
      sd_cursor(0, 6 + k, j);
      sd_text(work);
      j += n;
    }
  }
  commit_work();
  
  if (k == 0 && j == 1) eh_post(ERR_NO_PENDING, 0);
  return 0;
}
leave()
{
  close_all();
  execlp("operm", "operm", 0);
  krash("leave", "load operm", 0);
}

/*-------------------------------------------------------------------------*
 * openm / close all files 
 *-------------------------------------------------------------------------*/
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  oc_open();
  co_open();
  getparms(0);
  pending_open(READONLY);
}
close_all()
{
  alarm(0);
  pending_close();
  oc_close();
  co_close();
  ss_close();
  sd_close();
  database_close();
}
/*-------------------------------------------------------------------------*
 * transfer control back to calling program
 *--------------------------------------------------------------------------*/
/* end of sys_stat.c */

