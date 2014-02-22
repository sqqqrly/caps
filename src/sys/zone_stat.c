/*------------------------------------------------------------------------*
 *  Custome Code:   GTE - show order number and box number
 *------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Display Zone Status.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/13/93   |  tjt  Added to mfc.
 *  05/13/94   |  tjt  Fix order number 1 .. 6 digits right justified.
 *  05/20/94   |  tjt  Fix only count info if picks.
 *  06/06/94   |  tjt  Fix completeted counts to pickline counts.
 *  05/05/95   |  tjt  Fix display forward/backward.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  06/04/95   |  tjt  Add sp_pl_by_name display.
 *  06/20/95   |  tjt  Fix table allocation.
 *  06/20/95   |  tjt  Fix display of totals.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  06/05/98   |  tjt  Fix remove Box Full flags (ff only).
 *  06/07/98   |  tjt  Fix oi_con length.
 *-------------------------------------------------------------------------*/
static char zone_stat_c[] = "%Z% %M% %I% (%G% - %U%)";

/****************************************************************************/
/*                                                                          */
/*                            Zone Status Screen                            */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>
#include "global_types.h"
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "eh_nos.h"
#include "message_types.h"
#include "caps_messages.h"
#include "zone_stat1.t"
#include "zone_stat2.t"
#include "Bard.h"

void refresh(int signum); // Signal handler
extern catcher();
extern leave();

/*
 * Global Variables
 */

#define WIDTH    80
#define LINES    15
#define LENGTH   ZoneMax

char graph[18][80];                       /* pretty picture                  */
long graph_flag = 0;                      /* do picture                      */
long activity_flag = 0;                   /* picks have occurred             */

long queued[ZoneMax + 4];                 /* queued orders by zone           */
char *tab;                                /* data table                      */
char temp[1921];                          /* copy of screen                  */

#define BUF_SIZE        10

short ONE = 1;
short LBUF = BUF_SIZE - 1;


struct fld_parms fld[] ={
  {10,48,28,1,&LBUF,"Enter Zone Range",'a'},
  {23,15,1,1,&ONE,"More? (y/n)",'a'}
};
char selection = 0;
long low_zone, high_zone;
long pos = 0;
long max = 0;

long total_queued, total_complete, total_active;

/*-------------------------------------------------------------------------*
 *  Main Program
 *-------------------------------------------------------------------------*/
main()
{
  putenv("_=zone_stat");
  chdir(getenv("HOME"));

  open_all();                             /* open all files                  */

  strcpy(temp, zone_stat2);

  tab = (char *)malloc((coh->co_zone_cnt + 5) * WIDTH);  /* F062095 */
  if (!tab) krash("main", "malloc tab", 1);

  fix(temp);
  sd_screen_off();
  sd_clear_screen();
  sd_text(temp);
  sd_screen_on();
  
  while(1)
  {
    zone_prompt();                        /* prompt for zone range           */

    pos = 0;                              /* reset display                   */
    activity_flag = 1;                    /* force counts                    */
    refresh(0);        
    more_prompt();
    alarm(0);                             /* stop refresh                    */
  
    strcpy(temp, zone_stat2);
    fix(temp);
    sd_cursor(0, 1, 1);
    sd_text(temp);
  }
}
/*-------------------------------------------------------------------------*
 *  prompt for zone range
 *-------------------------------------------------------------------------*/
zone_prompt()
{
  unsigned char t;
  char buf[BUF_SIZE];
  char *p;
  long low, high;
  
  if (!SUPER_OP)
  {
    low  = pl[op_pl - 1].pl_first_zone;
    high = pl[op_pl - 1].pl_last_zone;
  }
  else                          
  {
    low  = 1;
    high = coh->co_zone_cnt;
  }
  while(1)
  {
    memset(buf, 0, BUF_SIZE);

    low_zone  = low;
    high_zone = high;
    selection = 0;
    
    sd_prompt(&fld[0], 0);

    t = sd_input(&fld[0], 0 , 0, buf,0);
    if(t == EXIT) leave();
     
    if (*buf)
    {
      if (*buf == '*' || *buf == '+')
      {
        selection = *buf;
        low  = 1;
        high = coh->co_zone_cnt;
        break;
      }
      p = (char *)memchr(buf, '-', BUF_SIZE - 1);
      if (!p) p = buf + strlen(buf);
      *p++ = 0;
      
      low_zone = high_zone = atol(buf);

      if (*p) high_zone = atol(p);

      sprintf(buf, "%d-%d", low_zone, high_zone);

      if (low_zone < low || high_zone > high)
      {
        eh_post(ERR_RANGE, buf);
        continue;
      }
    }
    sprintf(buf, "%d-%d", low_zone, high_zone);
    sd_cursor(0, fld[0].irow, fld[0].icol);
    sd_text(buf);

    if (t == F_KEY_8) graph_flag = 1;
    else graph_flag = 0;

    return;
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  store data in a temporary file
 *-------------------------------------------------------------------------*/
store_data()
{
  register struct zone_item *z;
  register struct oi_item *o;
  register long i, j, k, block;
  
  char flags[20], *w, *p;
  char status[12];
  char number[CustomerNoLength];
  char grp[8];
  
  if (!activity_flag) return 0;
  activity_flag = 0;

  get_queued();                            /* find queued orders             */
  
  if (graph_flag) return make_graph();

  p = tab;

  for (i = low_zone, z = &zone[low_zone - 1]; i <= high_zone; i++, z++)
  {
    if (!z->zt_zone) continue;

    if (selection == '*')
    {
      if (!(z->zt_flags & (FirstZone | JumpZone))) continue;
    }
    if (selection == '+' && !queued[i - 1]) continue;

    memset(flags,  0x20, 20); w = flags;    /* zone type                    */
    memset(status, 0x20, 12);
    memset(number, 0x20, CustomerNoLength);
    memset(grp,  0x20, 8);
    
    if (z->zt_flags & FirstZone) {memcpy(w, "1ST", 3); w += 4;}
    if (z->zt_flags & LateEntry) {memcpy(w, "LE", 2);  w += 3;}
    if (z->zt_flags & JumpZone)  {memcpy(w, "JZ", 2);  w += 3;}
    if (z->zt_flags & EarlyExit) {memcpy(w, "EE", 2);  w += 3;}
    if (z->zt_flags & Steering)  {memcpy(w, "ST", 2);  w += 3;}
    if (z->zt_flags & IsMirror)  {memcpy(w, "MI", 2);  w += 3;}
/*  if (z->zt_flags & HasBoxFull){memcpy(w, "BX", 2);  w += 3;} F060598 */
    
    
    switch(z->zt_status)                /* zone status                    */
    {
      case 'A': memcpy(status, "Ahead", 5);

        if (z->zt_on && z->zt_order) get_order(grp, number, z->zt_order);
        break;
       
      case 'L': memcpy(status, "LateEntry", 9); 

        if (z->zt_on && z->zt_order) get_order(grp, number, z->zt_order);
        break;
       
      case 'E': memcpy(status, "EarlyExit", 9);

        if (z->zt_on && z->zt_order) get_order(grp, number, z->zt_order);
        break;
       
      case 'U': memcpy(status, "Underway", 8); 

        if (z->zt_on && z->zt_order) get_order(grp, number, z->zt_order);
        break;
               
      case 'F': memcpy(status, "Done", 4); break;

      case 'I':
      case 'W': memcpy(status, "Waiting", 7); break;

      case 'X': memcpy(status, "Locked", 6); break;

      case 'O': memcpy(status, "Login", 5); break;

      default:  memcpy(status, "Unknown", 7); break;
    }
/*
 * Pickline Zone   Entry    Status  Lines   Order         Group   Queued Picked
 * -------- ---- -------- --------- -----   -----         -----   ------ ------
 * xxxxxxxx xxx  xxxxxxxx xxxxxxxxx xxxxx xxxxxxxxxxxxxxx xxxxxx xxxxxx xxxxxx
 *         1         2         3         4         5         6         7
 *12345678901234567890123456789012345678901234567890123456789012345678901234567
 *
 */
    if (sp->sp_pl_by_name == 'y')
    {
      sprintf(p, " %-8.8s %3d  %8.8s %9.9s %5d %15.15s %6.6s%7d%7d",
        pl[z->zt_pl - 1].pl_name, z->zt_zone, flags, status, z->zt_lines, 
        number, grp, queued[z->zt_zone - 1], z->zt_count);
    }
    else
    {
      sprintf(p, "    %2d    %3d  %8.8s %9.9s %5d %15.15s %6.6s%7d%7d",
        z->zt_pl, z->zt_zone, flags, status, z->zt_lines, 
        number, grp, queued[z->zt_zone - 1], z->zt_count);
    }
    p += WIDTH;
  }
  sprintf(p, "%79c", 0x20);         /* F062095 */
  p += WIDTH;
  sprintf(p, "  Total Queued:   %5d%55c", total_queued, 0x20);
  p += WIDTH;
  sprintf(p, "  Total Active:   %5d%55c", total_active, 0x20);
  p += WIDTH;
  sprintf(p, "  Total Complete: %5d%55c", total_complete, 0x20);
  p += WIDTH;

  max = p - tab;                          /* end of table                    */
}
/*-------------------------------------------------------------------------*
 *  Get Group, Order Number, and Customer Order Number
 *-------------------------------------------------------------------------*/
get_order(grp, number, block)
char *grp;
char *number;
long block;
{
  char work[16];

  memcpy(grp, oc->oi_tab[block - 1].oi_grp, GroupLength);

#ifdef GTE
  sprintf(number, "%0*d/%*.*s", 
  				rf->rf_on, oc->oi_tab[block - 1].oi_on,
  				BoxNoLength, BoxNoLength, oc->oi_tab[block - 1].oi_box);
  return 0;
#endif
  
  if (sp->sp_use_con == 'n')
  {
    sprintf(work, "%7.*d", rf->rf_on, oc->oi_tab[block - 1].oi_on);
    memcpy(number, work, 7);
  }
  else
  {
    memcpy(number, oc->oi_tab[block - 1].oi_con, ConLength); /* F060798 */
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Make Workload Graph
 *-------------------------------------------------------------------------*/
make_graph()
{
  register long j, k, m, max, each;

  memset(graph, 0x20, sizeof(graph));
  for (k = 0; k < 16; k++) graph[k][0] = VERT;
  for (k = 0; k < 80; k++) graph[16][k] = HORT;
  graph[16][0] = LLC;

  max = 0;
  
  if (high_zone - low_zone > 75) high_zone = low_zone + 74;
  
  for (k = low_zone; k <= high_zone; k++)
  {
    if (queued[k - 1] > max) max = queued[k - 1];
  }
  each = (max + 15) / 16;
  
  for (k = 0; k <= high_zone - low_zone; k++)
  {
    j = 16 - ((queued[k + low_zone - 1] + each - 1) / each);
    
    for(; j < 16; j++)
    {
      if (graph[j][k + 2] == 0x20) graph[j][k + 1] = REVDIM;
      graph[j][k + 2] = 'X';
      graph[j][k + 3] = NORMAL;
    }
  }
  for (k = 0; k < 16; k++) graph[k][high_zone - low_zone + 3];
}

/*-------------------------------------------------------------------------*
 *  count orders in a single queue
 *-------------------------------------------------------------------------*/
count_queued(q, uwflag)
register struct oc_entry *q;
register long uwflag;
{
  register struct pl_item *p;
  register struct seg_item *s;
  register struct oi_item *o;
  register long j, k, entry, block;
   
  block = q->oc_first;
   
  while (block)
  {
    o = &oc->oi_tab[block - 1];
    block = o->oi_flink;
      
    k = o->oi_entry_zone;
    if (!uwflag) 
    {
      k = zone[k - 1].zt_start_section; 

      p = &pl[o->oi_pl - 1];

      if (p->pl_flags & IsSegmented)
      {
        entry = sg[zone[k - 1].zt_segment - 1].sg_snode;

        s = &sg[p->pl_first_segment - 1];
        
        for (j = p->pl_first_segment; j <= p->pl_last_segment; j++, s++)
        {
          if (s->sg_snode != entry) continue;
          queued[s->sg_first_zone - 1] += 1;
        }
        continue;
      }
    }
    queued[k - 1] += 1;
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  count orders in a segmented pickline
 *-------------------------------------------------------------------------*/
count_segmented(p)
register struct pl_item *p;
{
   register struct seg_item *s;
   register struct oi_item *o;
   register long j, k, block;
   
   s = &sg[p->pl_first_segment - 1];

   for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, s++)
   {
     block = oc->oc_tab[PicklineMax + k - 1].oc_uw.oc_first;
   
     while (block)
     {
        o = &oc->oi_tab[block - 1];
        block = o->oi_flink;
      
        j = o->oi_entry_zone;
        if (!j) 
        {
          if (s->sg_enode == 99) continue;   /* this seg is done on order    */
          
          j = s->sg_last_zone;
        }
        queued[j - 1] += 1;
     }
   }
   return;
}
/*-------------------------------------------------------------------------*
 *  count queued order over all picklines, zones, and queued 
 *-------------------------------------------------------------------------*/
get_queued()
{
   register struct pl_item *p;
   register struct oc_entry *q;
   register long j, k;
   
   for (k = 0; k < ZoneMax + 4; k++) queued[k] = 0;
   
   total_queued = total_active = total_complete = 0;

   for (k = 0, p = pl; k < coh->co_pl_cnt; k++, p++)
   {
      q = &oc->oc_tab[k].oc_comp;

      total_complete += pl[k].pl_complete;

      q = &oc->oc_tab[k].oc_uw;
      if (q->oc_first) 
      {
        if (p->pl_flags & IsSegmented) count_segmented(p);
        else count_queued(q, 1);
      }
      total_active += q->oc_count;

      q = &oc->oc_tab[k].oc_high;
      if (q->oc_first) count_queued(q, 0);
      
      total_queued += q->oc_count;

      q = &oc->oc_tab[k].oc_med;
      if (q->oc_first) count_queued(q, 0);
      
      total_queued += q->oc_count;

      q = &oc->oc_tab[k].oc_low;
      if (q->oc_first) count_queued(q, 0);

      total_queued += q->oc_count;
   }
   return;
}
/*-------------------------------------------------------------------------*
 *  more_prompt fuction
 *-------------------------------------------------------------------------*/
more_prompt()
{
  unsigned char t;
  char buf[2];
  
  sd_cursor(0,23,20);
  sd_text("(Exit, Forward, or Backward)");
  sd_prompt(&fld[1],0);
  memset(buf, 0, 2);

  while(1)
  {
    t = sd_input(&fld[1],0, 0, buf, 0);

    activity_flag = 0;                     /* F052094 stop counting          */
    
    switch(sd_more(t, *buf))
    {
      case(0) : leave();
      case(1) : show(LINES, 1); break;
      case(2) : show(2 * LINES, 2); break;
      case(3) : return;
      case(6) : eh_post(ERR_YN,0);
    }
    activity_flag = 1;                    /* F052094 force counts            */
  }
}
/*-------------------------------------------------------------------------*
 * function to display x number of lines of data on the screen             
 *-------------------------------------------------------------------------*/
show(lines, i)
register long  lines, i;
{
  register long j;

  if (graph_flag)
  {
    sd_cursor(0, 6, 1);
    sd_text_2(graph, sizeof(graph));
    return 0;
  }
  if (i == 2)                             /* go backward in the file         */
  {
    pos = pos - lines * WIDTH;
    if(pos < 0) pos = 0;
  }
  else
  {
    if (pos + LINES * WIDTH > max) pos = max - LINES * WIDTH;
    if (pos < 0) pos = 0;
  }
  for (j = 0; j < LINES; j++)
  {
    sd_cursor(0, 8 + j, 1);
    if (pos < max)
    {
      sd_text(tab + pos);
      pos += WIDTH;
    }
    else sd_clear_line();
  }
}
/*-------------------------------------------------------------------------*
 *  display
 *-------------------------------------------------------------------------*/
display()
{
  strcpy(temp, zone_stat1);
  fix(temp);
  sd_cursor(0, 1, 1);
  sd_text_2(temp, 560);                  /* first 7 lines only               */
  store_data();
  return 0;
}

/*-------------------------------------------------------------------------*
 * refresh the screen 
 *-------------------------------------------------------------------------*/
void refresh(int signum) // Signal handler
{
  display();
  show(LINES, 2);
  signal(SIGALRM, refresh);
  alarm(op_refresh);
}
/*-------------------------------------------------------------------------*
 *  open all files
 *-------------------------------------------------------------------------*/
open_all()
{
  static unsigned char list[] = {ZoneStartEvent};

  sd_open(catcher);
  message_select(list, 1);
  ss_open();
  co_open();
  oc_open();
  getparms(0);
}
/*-------------------------------------------------------------------------*
 *  Message Catcher
 *-------------------------------------------------------------------------*/
catcher(who, type, buf, len)
register long who, type, len;
register TZoneMessage *buf;
{
  if (type == ShutdownRequest) leave();
  
  if (type == ZoneStartEvent) activity_flag = 1;
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  transfer control back to the calling program
 *-------------------------------------------------------------------------*/
leave()
{
  alarm(0);
  sd_close();
  oc_close();
  od_close();
  ss_close();
  
  execlp("operm", "operm", 0);
  krash("leave", "operm load", 1);
}

/* end of zone_stat.c */

