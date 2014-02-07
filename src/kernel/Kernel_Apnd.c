/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Append a message to the queue.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/17/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Apnd_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Adds to the message queue if any destinations.  If the queue is full,
 *  Kernel_Pack is called to garbage collect empty space.
 *  return 0 is none appended.   return 1 is message appended.
 *-------------------------------------------------------------------------*/

kernel_append(buf)
register TMessageItem *buf;               /* message to be appended          */
{
  static   TMessageSelectorItem nil = {0};
  register TMessageQueueItem *q;

/*-------------------------------------------------------------------------*
 *  Check if any tasks wants this message
 *-------------------------------------------------------------------------*/

  if (buf->m_destination)
  {
    if (!t_task[buf->m_destination - 1].t_pid) return 0;
  }
  else if (memcmp(ms[buf->m_type], nil, KernelSelectorBytes) == 0) return 0;

/*-------------------------------------------------------------------------*
 *  Check if sufficient space is available in the message queue
 *-------------------------------------------------------------------------*/

  if (tq_end + buf->m_length + KernelQueueMin >= tq_size) kernel_pack();

/*-------------------------------------------------------------------------*
 *  Store message to message queue
 *-------------------------------------------------------------------------*/

  q = (TMessageQueueItem *)(tq + tq_end); /* point to new item               */

  if (buf->m_destination)                 /* single destination              */
  {
    memset(q->tq_tasks, 0, KernelSelectorBytes);
    q->tq_tasks[Byte[buf->m_destination - 1]] |= Bit[buf->m_destination - 1];
  }
  else memcpy(q->tq_tasks, ms[buf->m_type], KernelSelectorBytes);

  q->tq_sender = buf->m_sender;
  q->tq_type   = buf->m_type;
  q->tq_length = buf->m_length;

  if (buf->m_length) memcpy(q->tq_text, &buf->m_message, buf->m_length);

  tq_end += (buf->m_length + KernelQueueMin);

  return 1;
}

/* end of Kernel_Apnd.c */
