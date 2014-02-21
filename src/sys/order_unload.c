/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Custom Code:    DAYTIMER - special report.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Unloads orders, picks, and remarks.
 *
 *  Execution:      order_unload [file_name] [-p=pickline] [-c]
 *                  -c unloads on changed picks
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/31/95   |  tjt  Original implementation.
 *  08/03/95   |  tjt  Modified for Day-Timers
 *  08/31/95   |  tjt  Add unload changed items
 *  04/17/96   |  tjt  Revise to_go.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  12/02/96   |  tjt  Add units and lines to location.
 *-------------------------------------------------------------------------*/
static char order_unload_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss.h"
#include "st.h"
#include "co.h"
#include "of.h"
#include "Bard.h"
#include "bard/pmfile.h"
#include "bard/prodfile.h"

FILE *fd;
char fd_name[64];

FILE *rd;
char rd_name[16];

long pickline = 0;
long changed  = 0;

typedef struct
{
  char  tstkloc[6];
  char  tdescr[25];
  char  tsku[SkuLength];
  short tordered;
  short tmove;

} titem;

#define TMAX 500

titem tab[TMAX + 1];
long tmax, ton, count;
char tcon[12];

main (argc, argv)
long argc;
char **argv;
{
  register long k;

  putenv("_=order_unload");
  chdir(getenv("HOME"));
  
  if (argc < 2) krash("main", "file name missing", 1);
  
  strcpy(fd_name, argv[1]);
  fd = fopen(fd_name, "w");
  if (fd == 0) krash("main", "file open", 1);
  
  for (k = 1; k < argc; k++)
  {
    if (memcmp(argv[k], "-p=", 3) == 0)
    {
      pickline = atol(argv[k] + 3);
    }
    else if (memcmp(argv[k], "-c", 2) == 0) changed = 1;
  }
  database_open();
  
  ss_open();
  co_open();
  oc_open();
  od_open();
  
  pmfile_open(READONLY);
  pmfile_setkey(2);
  
  prodfile_open(READONLY);
  prodfile_setkey(1);
  
#ifdef DAYTIMER
  tmp_name(rd_name);
  rd = fopen(rd_name, "w");
#endif

  if (pickline > 0) unload_one_pickline(pickline);
  else unload_all_picklines();
  
  fprintf(fd, "%c\n", rf->rf_eof);
  fclose(fd);

  leave(0);
}
/*-------------------------------------------------------------------------*
 *  Process All Picklines
 *-------------------------------------------------------------------------*/
unload_all_picklines()
{
  register long k, n, m, next, block, blk, on;

#ifdef DEBUG
  fprintf(stderr, "unload_all_picklines\n");
#endif

  for (n = 1; n <= coh->co_pl_cnt; n++)
  {
    if (!pl[n - 1].pl_pl) continue;

#ifdef DEBUG
  fprintf(stderr, "pickline=%d\n", n);
#endif

    for (k = OC_HOLD; k > OC_HIGH; k--)
    {
#ifdef DEBUG
  fprintf(stderr, "queue=%d first=%d\n", k,
    oc->oc_tab[n - 1].oc_queue[k].oc_first);
#endif
      next = oc->oc_tab[n - 1].oc_queue[k].oc_first;

      while (next > 0)
      {
        block = next;
        next  = oc->oi_tab[block - 1].oi_flink;

        tmax = 0;
        
        on = oc->oi_tab[block - 1].oi_on;
      
#ifdef DEBUG
  fprintf(stderr, "pickline=%d order=%d block=%d\n", n, on, block);
#endif
        if (changed)
        {
          if (!check_order(n, on)) continue;
        }
        oc_dequeue(block);
        
        if (sp->sp_pickline_zero == 'n') pickline = n;
        
        if (unload_header(n, on, block)) continue;
        unload_picks(n, on);

        if (sp->sp_pickline_zero != 'n')
        {
          for (m = n + 1; m <= coh->co_pl_cnt; m++)
          {
            if (!pl[m - 1].pl_pl) continue;

#ifdef DEBUG
  fprintf(stderr, "pickline=%d order=%d\n", m, on);
#endif
            blk = oc_find(m, on);
            if (!blk) continue;

            oc_dequeue(blk);
            oc_enqueue(blk, OC_LAST, OC_COMPLETE);

            of_rec->of_pl = m;
            of_rec->of_on = on;
            begin_work();
            if (!order_read(of_rec, LOCK)) 
            {
              of_rec->of_status = 'x';
              of_rec->of_datetime = time(0);
              order_replace(of_rec);
            }
            if (rf->rf_rmks > 0)
            {
              or_rec->rmks_pl = m;
              or_rec->rmks_pl = on;
              if (!remarks_read(or_rec, LOCK)) remarks_delete();
            }
            commit_work();

            unload_picks(m, on);
          }                                 /* end other picklines           */
        }                                   /* end zero picklines            */
        fprintf(fd, "%c", rf->rf_rt);       /* end of order                  */
#ifdef DAYTIMER
        print_report();
#endif
      }                                     /* end of queue                  */
    }                                       /* end of all queues             */
  }                                         /* end of picklines              */
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Order File To Unload
 *-------------------------------------------------------------------------*/
unload_one_pickline(n)
register long n;
{
  register long k, next, block, on;

#ifdef DEBUG
  fprintf(stderr, "unload_one_pickline(%d)\n", pickline);
#endif

  for (k = OC_HIGH; k <= OC_HOLD; k++)
  {
    next = oc->oc_tab[n - 1].oc_queue[k].oc_first;

    while (next > 0)
    {
      block = next;
      next  = oc->oi_tab[block - 1].oi_flink;
      
      on = oc->oi_tab[block - 1].oi_on;
      
#ifdef DEBUG
  fprintf(stderr, "pickline=%d order=%d\n", n, on);
#endif

      if (changed)
      {
        if (!check_picks(n, on)) continue;
      }
      oc_dequeue(block);

      if (unload_header(n, on, block)) continue;
      unload_picks(n, on);

      fprintf(fd, "%c", rf->rf_rt);
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Unload Order Header File
 *-------------------------------------------------------------------------*/
unload_header(n, on, block)
register long n, on, block;
{
  of_rec->of_pl = n;
  of_rec->of_on = on;

  begin_work();
  if (order_read(of_rec, LOCK)) 
  {
    commit_work();
    return 1;
  }
  commit_work();
  ton = on;
  memcpy(tcon, of_rec->of_con, 12);

  fprintf(fd, "%c", rf->rf_rp);

  if (rf->rf_con > 0) 
  {
    fprintf(fd, "%*.*s", rf->rf_con, rf->rf_con, of_rec->of_con);
  }
  if (rf->rf_on > 0)
  {
    fprintf(fd, "%0*d", rf->rf_on, of_rec->of_on);
  }
  if (rf->rf_pri > 0)
  {
    if (of_rec->of_status == 'h') fprintf(fd, "k");
    else fprintf(fd, "%c", of_rec->of_pri);
  }
  if (rf->rf_grp > 0)
  {
    fprintf(fd, "%*.*s", rf->rf_grp, rf->rf_grp, of_rec->of_grp);
  }
  if (rf->rf_pl > 0)
  {
     fprintf(fd, "%0*d", rf->rf_pl, pickline);
  }
  if (rf->rf_rmks > 0)
  {
    or_rec->rmks_pl = n;
    or_rec->rmks_pl = on;
    if (!remarks_read(or_rec, LOCK))
    {
      fprintf(fd, "%*.*s", rf->rf_rmks, rf->rf_rmks, or_rec->rmks_text);
      remarks_delete();
    }
  }
  fprintf(fd, "\n");

  of_rec->of_status = 'x';
  of_rec->of_datetime = time(0);
  order_replace(of_rec);
  commit_work();
  oc_enqueue(block, OC_LAST, OC_COMPLETE);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Unload Picks
 *-------------------------------------------------------------------------*/
unload_picks(n, on)
register long n, on;
{
  register struct pw_item *i;
  register struct pl_item *p;
  register struct st_item *s;

#ifdef DEBUG
  fprintf(stderr, "unload_picks(pl=%d  on=%d)\n", n, on);
#endif
  
  op_rec->pi_pl  = n;
  op_rec->pi_on  = on;
  op_rec->pi_mod = 0;
  pick_startkey(op_rec);
  
  op_rec->pi_mod = ModuleMax;
  pick_stopkey(op_rec);
  
  begin_work();
  while (!pick_next(op_rec, LOCK))
  {
#ifdef DEBUG
  fprintf(stderr, "pl=%d mod=%d quan=%d sku=%.*s\n",
    op_rec->pi_pl, op_rec->pi_mod, op_rec->pi_ordered, op_rec->pi_sku);
#endif
    
    if (op_rec->pi_flags & MIRROR) continue;
    
    if (op_rec->pi_mod > 0)
    {
      p = &pl[op_rec->pi_pl - 1];
      
      i = &pw[op_rec->pi_mod - 1];
    
      p->pl_lines_to_go -= 1;
      p->pl_units_to_go -= op_rec->pi_ordered;
      
      i->pw_units_to_go -= op_rec->pi_ordered;
      i->pw_lines_to_go -= 1;
    }
    if (sp->sp_use_stkloc == 'y' && rf->rf_stkloc > 0)
    {
      s = mod_lookup(op_rec->pi_mod);
      fprintf(fd, "%*.*s", rf->rf_stkloc, rf->rf_stkloc, s->st_stkloc);
    }
    else if (rf->rf_sku > 0) 
    {
      fprintf(fd, "%*.*s", rf->rf_sku, rf->rf_sku, op_rec->pi_sku);
    }
    else fprintf(fd, "0*d", rf->rf_mod, op_rec->pi_mod);

    fprintf(fd, "%0*d", rf->rf_quan, op_rec->pi_ordered);
    
    if (rf->rf_pick_text > 0)
    {
      fprintf(fd, "%*.*s", 
        rf->rf_pick_text, rf->rf_pick_text, op_rec->pi_pick_text);
    }
    fprintf(fd, "\n");

#ifdef DAYTIMER
    s = sku_lookup(op_rec->pi_pl, op_rec->pi_sku);

    if (s)
    {
      if (s->st_mod != op_rec->pi_mod) 
      {
        tab[tmax].tmove = 1;
      }
    }
    else tab[tmax].tmove = 1;

    space_fill(op_rec->pi_sku, SkuLength);
    memcpy(tab[tmax].tsku, op_rec->pi_sku, SkuLength);
    tab[tmax].tordered = op_rec->pi_ordered;

#ifdef DEBUG
    fprintf(stderr, "[%15.15s %5d %d]\n", 
      tab[tmax].tsku, tab[tmax].tordered, tab[tmax].tmove);
#endif

    if (tmax < TMAX) tmax++;
#endif
  }
  commit_work();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Orders For Changed Picks
 *-------------------------------------------------------------------------*/
check_order(n , on)
register long n, on;
{
  register long m;
  
#ifdef DEBUG
  fprintf(stderr, "check_orders(%d, %d)\n", n, on);
#endif

  if (sp->sp_pickline_zero != 'n')          /* test checked in previous pl   */
  {
    for (m = 1; m < n; m++)
    {
      if (!pl[m - 1].pl_pl) continue;

      of_rec->of_pl = m;
      of_rec->of_on = on;
      if (!order_read(of_rec, LOCK)) return 0;
    }
  }
  if (check_picks(n, on)) return 1;         
  
  if (sp->sp_pickline_zero != 'n')
  {
    for (m = n + 1; m <= coh->co_pl_cnt; m++)
    {
      if (!pl[m - 1].pl_pl) continue;

      if (check_picks(m, on)) return 1;
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Changed Picks
 *-------------------------------------------------------------------------*/
check_picks(n, on)
register long n, on;
{
  register struct st_item *s;
  
#ifdef DEBUG
  fprintf(stderr, "check_picks(%d, %d)\n", n, on);
#endif

  op_rec->pi_pl = n;
  op_rec->pi_on = on;
  op_rec->pi_mod = 0;

  pick_startkey(op_rec);
  
  op_rec->pi_mod = ModuleMax;
  pick_stopkey(op_rec);
  
  begin_work();
  while (!pick_next(op_rec, NOLOCK))
  {
    s = sku_lookup(op_rec->pi_pl, op_rec->pi_sku);

    if (s)
    {
      if (s->st_mod == op_rec->pi_mod) continue;
    }
    commit_work();
    return 1;
  }
  commit_work();
  return 0;
}
#ifdef DAYTIMER
/*-------------------------------------------------------------------------*
 *  Print Report
 *-------------------------------------------------------------------------*/
print_report()
{
  extern compare();
  register long k;
  pmfile_item x;
  prodfile_item y;
  
  for (k = 0; k < tmax; k++)
  {
    memcpy(x.p_pmsku, tab[k].tsku, SkuLength);
    memcpy(y.p_pfsku, tab[k].tsku, SkuLength);

    if (pmfile_read(&x, NOLOCK))  memset(tab[k].tstkloc, 0x20, 6);
    else memcpy(tab[k].tstkloc, x.p_stkloc, 6);
    
    if (prodfile_read(&y, NOLOCK)) memset(tab[k].tdescr, 0x20, 25);
    else memcpy(tab[k].tdescr, y.p_descr, 25);
  }
  qsort(tab, tmax, sizeof(titem), compare);

  fprintf(rd, "  CAPS No: %05d\n", ton);
  fprintf(rd, " Order No: %12.12s\n\n", tcon);

  fprintf(rd, " Stkloc      SKU       Quan         Description\n");
  fprintf(rd, " ------  ------------  ----  --------------------------\n");

  for (k = 0; k < tmax; k++)
  {
    fprintf(rd, "%c%6.6s  %12.12s  %4d  %25.25s\n", 
      tab[k].tmove > 0 ? '*' : 0x20,
      tab[k].tstkloc, 
      tab[k].tsku,
      tab[k].tordered,
      tab[k].tdescr);
  }
  fprintf(rd, "%c", 0x0c);
  count += 1;
}
compare(p, q)
register titem *p, *q;
{
  return memcmp(p->tstkloc, q->tstkloc, 6);
}
#endif
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave(x)
register long x;
{
  char command[80];

  fclose(rd);
  ss_close();
  oc_close_save();
  od_close();
  co_close();
  pmfile_close();
  prodfile_close();
  
  database_close();

#ifdef DAYTIMER
  if (count > 0)
  {
    sprintf(command, "%s %s", getenv("LPR"), rd_name);
    system(command);
  }
  else unlink(rd_name);
#endif

  exit(0);
}

/* end of order_unload.c */
