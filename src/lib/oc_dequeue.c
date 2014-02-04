/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Removes and order index entry for a queue.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/17/93    |  tjt  Added to mfc.
 *  7/23/93    |  tjt  Rewritten.
 *  8/24/93    |  tjt  Queue added.
 *-------------------------------------------------------------------------*/
static char oc_dequeue_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"

oc_dequeue(block)
register long block;
{
  register struct oi_item *p;
  register struct oc_entry *c;
  
  if (oc_fd <= 0) krash("oc_dequeue", "oc not open", 1);
    
  if (block < 1 || block > oc->of_size)
  {
    return krash("oc_dequeue", "block out of range", 1);
  }
  p = &oc->oi_tab[block - 1];             /* point to index entry            */

  if (p->oi_pl < 1 || p->oi_pl > PicklineMax + SegmentMax)
  {
    return krash("oc_dequeue", "pl out of range", 1);
  }
  if (p->oi_queue > 6) return krash("oc_dequeue", "bad queue", 1);

  c = &oc->oc_tab[p->oi_pl - 1].oc_queue[p->oi_queue];
  
/*-------------------------------------------------------------------------*
 * Update Next of Previous or New First in Queue
 *-------------------------------------------------------------------------*/

  if (p->oi_blink)
  {
    oc->oi_tab[p->oi_blink - 1].oi_flink = p->oi_flink;
  }
  else  c->oc_first = p->oi_flink;

/*-------------------------------------------------------------------------*
 *  Update Previous of Next or New Last in Queue
 *-------------------------------------------------------------------------*/

  if (p->oi_flink)
  {
    oc->oi_tab[p->oi_flink - 1].oi_blink = p->oi_blink;
  }
  else c->oc_last = p->oi_blink;

  p->oi_blink = p->oi_flink = 0;          /* clear links                     */
  c->oc_count--;                          /* update count                    */

  return 0;
}

/* end of oc_dequeue.c */
