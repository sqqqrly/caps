/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Find task messages and mark deleted.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/17/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Flush_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Finds messages in the queue for the task and marks deleted.
 *  return -1 is error; return 0 is ok.
 *-------------------------------------------------------------------------*/

kernel_flush(k)
register long k;                          /* task number 1 ..32              */
{
  register TTaskItem *t;
  register long j, byte, offset;
  register unsigned char bit;
  register TMessageQueueItem *q;

  if (k <= 0)             return -1;      /* invalid task number             */
  if (k >  MessageTasks)  return -1;      /* invalid task number             */

  t = &t_task[k - 1];                     /* point to task                   */
  if (!t->t_pid)          return -1;      /* task is not active              */
   
  byte = Byte[k - 1];                     /* byte in selector mask           */
  bit  = ~Bit[k - 1];                     /* bit  in selector mask           */

  offset = 0;                             /* message queue offset            */

  while (offset < tq_end)                 /* search to end of queue          */
  {
    q = (TMessageQueueItem *)(tq + offset);/* pointer to queue               */

    q->tq_tasks[byte] &= bit;             /* remove bit, if any              */

    offset += (q->tq_length + KernelQueueMin);
  }
  for (j = 0; j < MessageTypes; j++)      /* message selection table         */
  {
    ms[j][byte] &= bit;                   /* clear task selection            */
  }
  return 0;
}

/* end of Kernel_FLush.c */
