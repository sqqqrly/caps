/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Compress and/or reallocate message queue.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/17/93    |  tjt  Original implementation
 *-------------------------------------------------------------------------*/
static char Kernel_Pack_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Removes queued messages which have been sent and/or changes the size
 *  of the queue area.
 *-------------------------------------------------------------------------*/

kernel_pack()
{
  static   TMessageSelectorItem nil = {0};
  register TMessageQueueItem *q;
  register long j, k, n;
   
  j = k = n = 0;                          /* clear all values                */
   
  while (k < tq_end)                      /* over entire queue               */
  {
    q = (TMessageQueueItem *)(tq + k);    /* point to queue item             */

    n = q->tq_length + KernelQueueMin;    /* size of this item               */

    if (memcmp(q->tq_tasks, nil, KernelSelectorBytes) == 0)
    {
      k += n;                             /* step over a hole in queue       */
      continue;
    }
    if (j == k)                           /* no hole found yet               */
    {
      k += n;                             /* step over good item             */
      j = k;
      continue;
    }
    while (n > 0)                         /* copy by byte in case overlap    */
    {
      tq[j++] = tq[k++];
      n--;
    }
  }
  tq_end = j;                             /* new current end                 */

  for (k = 0; k < MessageTasks; k++)
  {
    t_task[k].t_offset = 0;               /* reset to head of queue          */
  }
  n = ((tq_end + sizeof(TMessageQueueItem)) / KPage + 1) * KPage;
  
  if (tq_size != n)
  {
    tq_size = n;
    tq = (unsigned char *)realloc(tq, tq_size);
    if (!tq) return krash("kernel_pack", "No Memory Space", 1);
  }
  return 0;
}

/* end of Kernel_Pack.c */
