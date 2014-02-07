/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Find next task message in queue.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/17/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Find_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Finds and sends the next message in the queue for the task.
 *  return -1 is error; return 0 is none found; return 1 is found.
 *-------------------------------------------------------------------------*/

kernel_find(k)
register long k;                          /* task number 1 ..32              */
{
  register TTaskItem *t;
  register long byte;
  register unsigned char bit;
  register TMessageQueueItem *q;
  TMessageItem out;
   
  if (k <= 0)             return -1;      /* invalid task number             */
  if (k >  MessageTasks)  return -1;      /* invalid task number             */

  t = &t_task[k - 1];                     /* point to task                   */
  if (!t->t_pid)          return -1;      /* task is not active              */
   
  byte = Byte[k - 1];                     /* byte in selector mask           */
  bit  = Bit[k - 1];                      /* bit  in selector mask           */

  while (t->t_offset < tq_end)            /* search to end of queue          */
  {
    q = (TMessageQueueItem *)(tq + t->t_offset);/* pointer to queue          */

    t->t_offset += (q->tq_length + KernelQueueMin);

    if (q->tq_tasks[byte] & bit)          /* found one                       */
    {
      out.m_sender      = q->tq_sender;
      out.m_destination = k;
      out.m_type        = q->tq_type;
      out.m_length      = q->tq_length;

      if (out.m_length) memcpy(&out.m_message, q->tq_text, out.m_length);
         
      q->tq_tasks[byte] ^= bit;           /* turn off bit                    */

      if (kernel_put(&out) < 0) return -1;/* exit if failed                  */

      t->t_count++;                       /* ack count                       */
      t->t_snd_count++;                   /* send count                      */
      t->t_snd_time = time(0);            /* send time                       */

      if (t->t_signal) kill(t->t_pid, t->t_signal);

      return 1;
    }
  }
  return 0;                               /* nothing found                   */
}

/* end of Kernel_Find.c */
