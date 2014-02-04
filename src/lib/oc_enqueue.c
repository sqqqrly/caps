/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Insert an order index entry into a queue.
 *
 *  Called:         oc_enqueue(block, after, control)
 *                  block is 1 .. of_size.
 *                  after is OC_FIRST, OC_LAST, or block.
 *                  control is OC_HELD, OC_UW, OC_HIGH, OC_MED, OC_LOW, or
 *                             OC_COMPLETE.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/17/93    |  tjt  Added to mfc
 *  7/23/93    |  tjt  Rewritten.
 *  8/24/93    |  tjt  Queue added.
 *-------------------------------------------------------------------------*/
static char oc_enqueue_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "global_types.h"
#include "of.h"
 
oc_enqueue(block, after, control)
register long block, after, control;
{
  register struct oi_item  *p;            /* order to be added               */
  register struct oi_item  *q;            /* after/before work pointer       */
  register struct oc_entry *c;            /* pointer to queue                */
  register long before;                   /* working value                   */

  if (oc_fd <= 0) krash("oc_enqueue", "oc not open", 1);
  
  if (block < 1 || block > oc->of_size)   /* check block number              */
  {
    return krash("oc_enqueue", "block out of range", 1);
  }
  p = &oc->oi_tab[block - 1];             /* point to order                  */

  if (p->oi_pl < 1 || p->oi_pl > PicklineMax + SegmentMax) /* check pickline */
  {
    return krash("oc_enqueue", "pl out of range", 1);
  }
  if (control < 0 || control > 6)         /* check queue number              */
  {
    return krash("oc_enqueue", "bad queue", 1);
  }
  c = &oc->oc_tab[p->oi_pl - 1].oc_queue[control];

  if (after < 0) after = c->oc_last;      /* after == -1 is queue last       */

  if (after)                              /* enqueue not first               */
  {
    q = &oc->oi_tab[after - 1];           /* point to after order            */
    if (q->oi_pl != p->oi_pl)             /* check after pickline            */
    {
      return krash("oc_enqueue", "after pl", 1);
    }
    if (q->oi_queue != control)           /* check after queue               */
    {
      return krash("oc_enqueue", "after queue", 1);
    }
    before      = q->oi_flink;            /* next order                      */
    q->oi_flink = block;                  /* update forward link             */
  }
  else                                    /* enqueue first                   */
  {
    before      = c->oc_first;            /* insert as first in queue        */
    c->oc_first = block;                  /* new first in queue              */
  }
  if (before)
  {
    q = &oc->oi_tab[before - 1];          /* point to next order             */
    q->oi_blink = block;                  /* back from next                  */
  }
  else c->oc_last = block;                /* new last in queue               */

  p->oi_blink = after;                    /* get previous                    */
  p->oi_flink = before;                   /* forward from order              */
  p->oi_queue = control;                  /* queue of entry                  */
  c->oc_count++;                          /* add one to queue count          */

  return 0;
}

/* end of oc_enqueue.c */
