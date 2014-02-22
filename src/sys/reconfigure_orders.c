/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Rationalizes the order file for configuration.
 *
 *  Execution:      reconfigure_orders  [-c]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/15/93   |  tjt  Original implementation.
 *  06/23/94   |  tjt  Fix to leave uw in order when reconfiguring.
 *  10/27/94   |  tjt  Fix to leave uw in order always.
 *  03/22/95   |  tjt  Fix recovery on all picklines.
 *  06/30/95   |  tjt  Add sku lookup on reconfigure.
 *  07/01/95   |  tjt  Fix reconfigure HOLD queue too.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  10/17/95   |  tjt  Fix unload of segmented picklines of_status of 'q'.
 *  04/17/96   |  tjt  Revise to_go.
 *  12/02/96   |  tjt  Add units and lines to location.
 *-------------------------------------------------------------------------*/
static char reconfigure_orders[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "global_types.h"
#include "Bard.h"
#include "ss.h"
#include "co.h"
#include "of.h"

main(argc, argv)
long argc;
char **argv;
{
  register struct pl_item *p;
  register struct pw_item *i;
  register long j, k, block, prev;
  register long lookup, accum, orphans, pl_zero;
  register struct oi_item *o;
  long pid, status;
  
#ifdef DEBUG
  fprintf(stderr, "reconfigure_orders\n");
#endif

  putenv("_=reconfigure_orders");
  chdir(getenv("HOME"));
  
  ss_open();
  co_open();

  if (argc > 1)
  {
    if (memcmp(argv[1], "-c", 2) == 0) coh->co_st_changed = 1;
  }
  orphans = 0;                             /* any orphans                    */
  lookup  = 0;
  
  if (coh->co_st_changed == 1)             /* modules have changed           */
  {
    lookup = 1;

    if (fork() == 0)
    {
      execlp("bin/order_unload", "bin/order_unload", 
             "otext/ORDER.UNLOAD", "-p=0", "-c", 0);
         krash("main", "load order_unload", 1);
    }
    pid = wait(&status);
    if (pid < 0 || status) krash("main", "order_unload failed", 1);
  }
  database_open();

  oc_open();
  od_open();
  oc_lock();

  for (k = 0, p = pl; k < coh->co_pl_cnt; k++, p++)
  {
    p->pl_lines_to_go = p->pl_units_to_go = 0;  /* clear remaining picks     */
  }
  for (k = 0, i = pw; k < coh->co_prod_cnt; k++, i++)
  {
    i->pw_lines_to_go = i->pw_units_to_go = 0;  /* clear remaining picks     */
  }
  for (k = 0, p = pl; k < coh->co_pl_cnt; k++, p++)
  {
    if (!p->pl_pl) continue;
    prev = p->pl_last_zone;
    
    for (j = OC_UW; j <= OC_HOLD; j++)
    {
      block = oc->oc_tab[k].oc_queue[j].oc_first;
      
      while (block)
      {
        o = &oc->oi_tab[block - 1];

        begin_work();
        orphans += od_config(block, lookup, 1);  /* recalculate position     */
        commit_work();
        
#ifdef DEBUG
  printf("pl=%d order=%d orphans=%d flag=%d\n",
    o->oi_pl, o->oi_on, orphans, o->oi_flags);
#endif
        if (j == OC_UW)                   /* don't move uw orders F102794    */
        {
          if (o->oi_entry_zone > prev) o->oi_entry_zone = prev;
          prev = o->oi_entry_zone;
        }
        block = o->oi_flink;
      }
    }
    if (p->pl_flags & IsSegmented) unload_uw(k);
    else repair_uw(k);                   
  }
  if (lookup) coh->co_st_changed = 2;       /* sku's are updated             */

  od_close();
  ss_close_save();
  co_close_save();
  oc_unlock();
  oc_close_save();
  database_close();
  
  if (lookup)
  {
    if (fork() == 0)
    {
      execlp("bin/order_input", "bin/order_input", 
           "otext/ORDER.UNLOAD", 0);
      krash("main", "load order_input", 1);
    }
    pid = wait(&status);
  }
  if (orphans > 0)
  {
    if (fork() == 0)
    {
      execlp("orphan_picks", "orphans_picks", 0);
      krash("main", "load orphans_picks", 1);
    }
    pid = wait(&status);
    if (pid < 0) krash("main", "orphans_picks failed", 1);
    else if (status) exit(1);
  }
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Sort UW Queues Into Entry Zone Descending Order
 *-------------------------------------------------------------------------*/
repair_uw(k)
register long k;
{
  register struct oi_item *o;
  register struct oc_entry *y, *z;
  register block, found, max;
        
  y = &oc->oc_tab[k].oc_uw;
  z = &oc->oc_tab[k].oc_work;

  if (y->oc_count <= 0) return 0;         /* nothing underway                */

  block = y->oc_first;
  max = ZoneMax;
    
  while (block)                           /* scan and check order            */
  {
    o = &oc->oi_tab[block - 1];
    if (o->oi_entry_zone > max) break;
    max = o->oi_entry_zone;
    block = o->oi_flink;
  }
  if (!block) return 0;                   /* is already in order             */

  z->oc_first = 0;
  z->oc_last  = 0;
  z->oc_count = 0;

  while (y->oc_first)                     /* find first largest entry zone   */
  {
    found = block = y->oc_first;
    max = 0;
      
    while (block)                         /* sort in descending order        */
    {
      o = &oc->oi_tab[block - 1];
      if (o->oi_entry_zone > max)
      {
        max   = o->oi_entry_zone;
        found = block;
      }
      block = o->oi_flink;
    }
    oc_dequeue(found);
    oc_enqueue(found, OC_LAST, OC_WORK);
  }
  while (z->oc_first)                     /* copy sorted work to uw queue    */
  {
    block = z->oc_first;
    oc_dequeue(block);
    oc_enqueue(block, OC_LAST, OC_UW);
  }
  z->oc_first = 0;
  z->oc_last  = 0;
  z->oc_count = 0;

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Reset Segmented Pickline
 *-------------------------------------------------------------------------*/
unload_uw(pline)
register long pline;
{
  register struct pl_item *p;
  register struct oi_item *o;
  register long k, block;
  
#ifdef DEBUG
  fprintf(stderr, "unload_uw() PL=%d\n", pline+1);
#endif

  p = &pl[pline];                          /* pointer to pickline            */
  
  for (k = 0, o = oc->oi_tab; k < oc->of_size; k++, o++)
  {
    if (o->oi_pl < p->pl_first_segment + PicklineMax) continue;
    if (o->oi_pl > p->pl_last_segment  + PicklineMax) continue;
    memset(o, 0, sizeof(struct oi_item));
  }
  for (k = p->pl_first_segment; k <= p->pl_last_segment; k++)
  {
    memset(&oc->oc_tab[PicklineMax + k - 1], 0, sizeof(struct oc_item));
  }
  while (block = oc->oc_tab[pline].oc_uw.oc_last)
  {
    begin_work();
    od_get(block);
    of_rec->of_status = 'q';
    order_update(of_rec);
    oc_dequeue(block);
    oc_enqueue(block, OC_FIRST, OC_HIGH);
    commit_work();
  }
  return 0;
}


/* end of reconfigure_orders.c */
