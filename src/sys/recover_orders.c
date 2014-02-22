/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Rationalizes the underway order file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/01/95   |  tjt  Constructed from reconfigure_orders.
 *  08/23/96   |  tjt  Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char recover_orders[] = "%Z% %M% %I% (%G% - %U%)";

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
  register struct hw_item *h;
  register long j, k, block, blink, max, prev, entry;
  register struct oi_item *o;
  
  putenv("_=recover_orders");
  chdir(getenv("HOME"));
  
  database_open();

  ss_open();
  co_open();
  oc_open();
  od_open();
  
  oc_lock();
  
  for (k = 0, p = pl; k < coh->co_pl_cnt; k++, p++)
  {
    if (!p->pl_pl) continue;

    block = oc->oc_tab[k].oc_queue[OC_UW].oc_first;
    prev  = p->pl_last_zone;
    
    while (block)
    {
      o = &oc->oi_tab[block - 1];

      entry = o->oi_entry_zone;           /* save current entry              */

      begin_work();
      od_config(block, 0, 0);             /* recalculate position            */
      commit_work();
      
      if (o->oi_entry_zone > entry && entry >= p->pl_first_zone) 
      {
        o->oi_entry_zone = entry;         /* use old entry zone              */
      }
      block = o->oi_flink;
    }
    if (p->pl_flags & IsSegmented) unload_uw(k);
    else repair_uw(k);                   
  }
  od_close();
  
  ss_close_save();
  co_close_save();
  oc_unlock();
  oc_close_save();
    
  database_close();

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
    oc_dequeue(block);
    oc_enqueue(block, OC_FIRST, OC_HIGH);
  }
  return 0;
}


/* end of recover_orders.c */
