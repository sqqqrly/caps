#define DATETIME
/*-------------------------------------------------------------------------*
 *  Custom Code:    DAYTIMER - Allows change of quantity
 *                  DATETIME - Show last action time
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order status screen 2.6.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/13/93   |  tjt  Added to mfc.
 *  05/12/94   |  tjt  Fix order number as 1 .. 6 digits left justified.
 *  05/19/94   |  tjt  Fix gather pick info only when picks of order.
 *  07/27/94   |  tjt  Fix order number as 1 .. 7 digits left justified.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  06/27/95   |  tjt  Fix case on yn response. 
 *  06/29/95   |  tjt  Add customer order number.
 *  07/02/95   |  tjt  Add orphan picks note.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  09/08/95   |  tjt  Add display canceled.
 *  03/08/96   |  tjt  Add datetime of last action option.
 *  07/05/96   |  tjt  Set LORDER to rf_on as default.
 *  07/15/96   |  tjt  Fix change zone pick refresh logic.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char order_stat_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "global_types.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "co.h"
#include "of.h"
#include "language.h"
#include "order_stat.t"
#include "message_types.h"
#include "caps_messages.h"
#include "getparms.h"

#include "Bard.h"

extern leave();

#define NUM_PROMPTS     3
#define BUF_SIZE        (CustomerNoLength + 2)
FILE *fd;

extern char sd_pr_flag;

short LPL    = 8;
short LORDER = 5;
short ONE    = 1;

struct fld_parms fld[] = {

  {6,27,1,1, &LPL,   "Enter Pickline",'a'},
  {7,27,1,1, &LORDER,"Enter Order Number",'a'},
  {23,26,1,1,&ONE,   "Display Picks? (y/n)",'a'},
};

short ref_flag = 0;

char temp1[1920],temp2[10];
long rm,pickline,cnt,row,done,icol,gototop,block,index,flag,last,more;
long first_time = 0;
long last_entry = 0;
unsigned char t;
char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */
char temp[BUF_SIZE];
short i,j,k,n,ret;
char code;
char fmt[5];
char a[6],status[2],priority[2];
struct oc_item *z;

long x,y;
struct text_spec_item *rmks;
long rmks_count = 0;
char r_text[61];
char *rptr;

main()
{
  putenv("_=order_stat");
  chdir(getenv("HOME"));
  
  open_all();

  LORDER = rf->rf_on;                     /* F070596 */
  
  if (sp->sp_use_con == 'y' || sp->sp_use_con == 'b') 
  {
    if (rf->rf_con >= LORDER) LORDER = rf->rf_con + 1;
  }
  strcpy(temp1, order_stat);
  fix(temp1);
  sd_screen_off();
  sd_clear_screen();
  sd_text(temp1);
  sd_screen_on();
  
  while(1)
  {
    get_parms();
  }
}
/*-------------------------------------------------------------------------*
 *  Get Pickline And Order Number
 *-------------------------------------------------------------------------*/
get_parms()
{
  register long order;
  unsigned char t;

  last_entry = -1;                       /* force display - ignore picks     */

  memset(buf[0], 0, BUF_SIZE * NUM_PROMPTS);

  sd_prompt(&fld[1], 0);
  
  while(1)
  {
    if (IS_ONE_PICKLINE) pickline = op_pl;
    else if (!SUPER_OP)  pickline = op_pl;
    else
    {
      sd_prompt(&fld[0], 0);

      while(1)
      {
        t = sd_input(&fld[0], 0, &rm, buf[0], 0);
        if (t == EXIT) leave();

        pickline = pl_lookup(buf[0], op_pl);

        if (pickline <= 0)
        {
          eh_post(ERR_PL, buf[0]);
          continue;
        }
        sprintf(buf[0], "%d", pickline);
        chng_pkln(buf[0]);
        break;
      }                                  /* end of get pickline    */          
    }                                    /* end SUPER OP           */
    t = sd_input(&fld[1], 0, &rm, buf[1], 0);
    if (t == EXIT) leave();
 
    if (*buf[1] == '#') 
    {
      if (sp->sp_use_con == 'n') block = check_con(pickline, buf[1] + 1);
      else                       block = check_parms(pickline, buf[1] + 1);
    }
    else if (*buf[1] == '-' || !*buf[1]) 
    {
      block = check_parms(pickline, buf[1]);
    }
    else
    {
      if (sp->sp_use_con == 'n') block = check_parms(pickline, buf[1]);
      else                       block = check_con(pickline, buf[1]);
    }
    if (block > 0) 
    {
      show_order();
      sd_cursor(0,6,1);
      sd_clear_rest();
    }
    return 0;
  }                                        /* end of get_parms     */
}
/*-------------------------------------------------------------------------*
 *  Check Pickline And Order Paramaters
 *-------------------------------------------------------------------------*/
check_parms(pickline, order)
register long pickline;
register char *order;
{
  register long n;
  register char *p, code;

  n = 0; p = order; code = 0;
  
  if (!*p) code = 'u';

  else if (*p == '-')
  {
    p++;
    code = tolower(*p++);
  }
  else
  {
    while (*p >= '0' && *p <= '9')
    {
      n = 10 * n + (*p - '0');
      p++;
    }
    if (*p) code = tolower(*p++); 
  }
  if (*p)
  {
    eh_post(ERR_CODE, order);
    return 0;
  }
  switch (code)
  {
    case 0:    block = oc_find(pickline, n);
               if (!block)
               {
                 eh_post(ERR_NO_ORDER, order);
               }
               return block;
               
    case 'c':  if (!n) block = oc->oc_tab[pickline - 1].oc_comp.oc_last;
               else block = oc->oc_tab[pickline - 1].oc_comp.oc_first;

               if (oc->oc_tab[pickline - 1].oc_comp.oc_count < n) block = 0;

               while (block && n > 1)
               {
                 block = oc->oi_tab[block - 1].oi_flink;
                 n--;
               }
               if (!block) eh_post(ERR_NO_ORDER, "Completed");
               return block;
               
    case 'q':  if (!n) 
               {
                 block = oc->oc_tab[pickline - 1].oc_low.oc_last;
                 if (!block)
                 {
                   block = oc->oc_tab[pickline - 1].oc_med.oc_last;
                   if (!block)
                   {
                     block = oc->oc_tab[pickline - 1].oc_high.oc_last;
                   }
                 }  
                 if (!block) eh_post(ERR_NO_ORDER, "Queued");
                 return block;
               }
               if (oc->oc_tab[pickline - 1].oc_high.oc_count < n)
               {
						n -= oc->oc_tab[pickline - 1].oc_high.oc_count;

                  if (oc->oc_tab[pickline - 1].oc_med.oc_count < n)
                  {
						  n -= oc->oc_tab[pickline - 1].oc_med.oc_count;

                    if (oc->oc_tab[pickline - 1].oc_low.oc_count < n)
                    {
						    block = 0;
                    }
                    else block = oc->oc_tab[pickline - 1].oc_low.oc_first;
                 }
                 else block = oc->oc_tab[pickline - 1].oc_med.oc_first;
               }
               else block = oc->oc_tab[pickline - 1].oc_high.oc_first;

               while (block && n > 1)
               {
                 block = oc->oi_tab[block - 1].oi_flink;
                 n--;
               }
               if (!block) eh_post(ERR_NO_ORDER, "Queued");
               return block;

    case 'u':  if (!n) block = oc->oc_tab[pickline - 1].oc_uw.oc_last;
               else block = oc->oc_tab[pickline - 1].oc_uw.oc_first;

               if (oc->oc_tab[pickline - 1].oc_uw.oc_count < n) block = 0;

               while (block && n > 1)
               {
                 block = oc->oi_tab[block - 1].oi_flink;
                 n--;
               }
               if (!block) eh_post(ERR_NO_ORDER, "Underway");
               return block;
  }
}
/*-------------------------------------------------------------------------*
 *  Check Customer Order Number
 *-------------------------------------------------------------------------*/
check_con(pickline, con)
register long pickline;
register char *con;
{
  strip_space(con, CustomerNoLength);     /* move any spaces                 */

  if (!*con) return 0;                    /* nothing entered                 */

  order_setkey(2);                        /* pickline + con                  */
  
  of_rec->of_pl = pickline;
  memcpy(of_rec->of_con, con, CustomerNoLength);
  space_fill(of_rec->of_con, CustomerNoLength);  /* field is space filled    */
  
  begin_work();
  if (!order_read(of_rec, NOLOCK))
  {
    block = oc_find(of_rec->of_pl, of_rec->of_on);
    if (block) 
    {
      commit_work();
      return block;
    }
  }
  commit_work();
  eh_post(ERR_NO_ORDER, con);
  return -1;
}
/*-------------------------------------------------------------------------*
 *  Show Order Information
 *-------------------------------------------------------------------------*/
show_order()
{
  register long n;
  
  while(1)                              /*loop for brousing order file     */
  {
    last_entry = -1;                    /* force display - ignore picks    */
    first_time = 1;                     /* clear when complete event       */
    
    sd_cursor(0,6,1);
    sd_clear_rest();
      
    if (!block) return;

    if (block < 1 || block > oc->of_size) return;
   
    order_setkey(1);                    /* pickline + order number         */

    of_rec->of_pl = oc->oi_tab[block - 1].oi_pl;
    of_rec->of_on = oc->oi_tab[block - 1].oi_on;
   
    begin_work();
    if (order_read(of_rec, NOLOCK)) {commit_work(); return;}
    commit_work();
    refresh();                          /* display the order info          */

    sd_cursor(0,23,30);
    sd_text("(Exit, Forward, or Backward)");
    sd_prompt(&fld[2],0);

    while(1)
    {
      t = sd_input(&fld[2],0, 0,buf[2], 0);
      gototop = 0;
      
      alarm(0);
 
      *buf[2] = tolower(*buf[2]);
      n = sd_more(t, code_to_caps(*buf[2]));  /* F041897 */

      switch(n)
      {
        case(0) : leave();

        case(1) : if (*buf[2] == 'y')
                  {
                    close_all();
                    sprintf(temp, "%d", pickline);
                    sprintf(temp2, "%d",  block);
#ifdef DAYTIMER
                    execlp("order_picks2","order_picks2", temp, temp2, 0); 
#else
                    execlp("order_picks","order_picks", temp, temp2, 0);
#endif
                    krash("main", "order_picks load", 1);
                  }
                  block = get_next_order();
                  break;

        case(2) : block = get_prev_order();
                  break;
                  
        case(3) : gototop = 1;
                  break;

        case(6) : eh_post(ERR_YN,0);
                  break;
      }
      if (gototop) return;
      break;
    }
  }
}
/*--------------------------------------------------------------------------*
 *  Get Next Order
 *--------------------------------------------------------------------------*/
get_next_order()
{
  register long n;

  n = oc->oi_tab[block - 1].oi_queue;

  block = oc->oi_tab[block - 1].oi_flink;
  if (block) return block;
  
  for (n++; n <= OC_LOW; n++)
  {
    block = oc->oc_tab[pickline - 1].oc_queue[n].oc_first;
    if (block) return block;
  }
  return 0;
}
/*--------------------------------------------------------------------------*
 *  Get Previous Order
 *--------------------------------------------------------------------------*/
get_prev_order()
{
  register long n;

  n = oc->oi_tab[block - 1].oi_queue;

  block = oc->oi_tab[block - 1].oi_blink;
  if (block) return block;
 
  for (n--; n >= 0; n--)
  {
    block = oc->oc_tab[pickline - 1].oc_queue[n].oc_last;
    if (block) return block;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 * display order number and its status
 *-------------------------------------------------------------------------*/
display_stat(block)
register long block;
{
  register struct pl_item *p;
  static char zones[ZoneMax];
  char work[16];
  
  strcpy(temp1, order_stat);
  fix(temp1);                             /* refresh the date and time       */
  sd_cursor(0,1,1);
  sd_text(temp1);

  sd_cursor(0,6,1);

  p = &pl[of_rec->of_pl - 1];

  if(sp->sp_config_status == 'y')
  {
    if(p->pl_pl) sd_text(p->pl_name);
    else sd_text("Pickline");
  }
  else sd_text("Pickline");

  sd_cursor(1,0,1);
  sprintf(a, "%d", p->pl_pl);
  sd_text(a);                             /* display pickline number         */

  sd_cursor(0, 6, 12);
  sd_text("Order Number: ");
  sprintf(a, "%0*d", rf->rf_on, of_rec->of_on);
  sd_text(a);                             /* display order number            */

  sd_cursor(0, 6, 35);
  sd_text("Status: ");
  switch(oc->oi_tab[block - 1].oi_queue)
  {
    case OC_HOLD:     sd_text("Hold    "); break;
    case OC_COMPLETE: if (first_time) order_read(of_rec, NOLOCK);
                      if (of_rec->of_status == 'x') sd_text("Canceled");
                      else sd_text("Complete"); 
                      break;
    case OC_UW:       sd_text("Underway"); break;
    case OC_HIGH:     sd_text("Queued  "); break;
    case OC_MED:      sd_text("Queued  "); break;
    case OC_LOW:      sd_text("Queued  "); break;
    default:          sd_text("Unknowm "); break;
  }
  sd_cursor(0, 6, 52);
  sd_text("Priority: ");
  sd_text_2(&of_rec->of_pri,1);        /* display priority                */

  if (rf->rf_grp > 0)
  {
    sd_cursor(0, 6, 64);
    sd_text("Group: ");
    sd_text_2(of_rec->of_grp, rf->rf_grp);/* display group number         */
  }
  if (rf->rf_con > 0)
  {
    sd_cursor(0, 7, 12);
    sd_text("Reference:    ");
    sd_text_2(of_rec->of_con, rf->rf_con);
  }
#ifdef DATETIME
  sd_cursor(0, 8, 12);
  sd_text_2(ctime(&of_rec->of_datetime), 24);
#endif

  if (oc->oi_tab[block - 1].oi_flags & ORPHANS)
  {
    sd_cursor(0, 7, 43);
    sd_text("*Orphans");
  }
  sd_cursor(0, 7, 52);
  sprintf(work, "Lines: %d", of_rec->of_no_picks);
  sd_text(work);
  
  sd_cursor(0, 7, 64);
  sprintf(work, "Units: %d", of_rec->of_no_units);
  sd_text(work);

  if (of_rec->of_status == 'c' && first_time)  /*clear lines                */
  {
    for(j = 9; j < 23; j++)
    {
      sd_cursor(0,j,1);
      sd_clear_line();
    }
    first_time = 0;
  }                                       /* F111186 added 'e' test          */
  if(of_rec->of_status != 'c')
  {
    if (last_entry != oc->oi_tab[block - 1].oi_entry_zone)
    {
      od_status(block, zones);
    }
    last_entry = oc->oi_tab[block - 1].oi_entry_zone;
    
    icol = 14;
    i    = p->pl_first_zone;
    k    = p->pl_last_zone;
    if (k > i + 80) k = i + 80;
    cnt  = 1;
    row  = 9;

    sd_cursor(0,row,1);
    sd_text("Zone");
    sd_cursor(0,row+1,1);
    sd_text("Status");
 
    while (i <= k)
    {
      if (cnt > 16)
      {
        cnt = 1;
        icol = 14;
        row += 3;
        sd_cursor(0,row,1);
        sd_text("Zone");
        sd_cursor(0,row+1,1);
        sd_text("Status");
      }
      sd_cursor(0, row, icol);
      sprintf(work, "%-3d", i);
      sd_text(work);

      sd_cursor(0, row+1, icol);
      sd_text_2(&zones[i - 1], 1);

      icol += 4;
      i++;
      cnt++;
    }
  }
}
/*
 * refresh the screen
 */
refresh()
{
  display_stat(block);
  signal(SIGALRM, refresh);
  
  alarm(op_refresh);

  return 0;
}
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  oc_open();
  od_open();
  co_open();
  getparms(0);
  return 0;
}
close_all()
{
  alarm(0);
  co_close();
  oc_close();
  od_close();
  ss_close();
  sd_close();
  database_close();
  return 0;
}
leave()
{
  close_all();
  execlp("operm", "operm", 0);
  krash("leave", "operm load");
}

/* end of order_stat.c */
