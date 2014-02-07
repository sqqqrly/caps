/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Process acknowledge message from task.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *             |
 *-------------------------------------------------------------------------*/
static char Kernel_Ack_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Process Kernel Acknowledge Message
 *-------------------------------------------------------------------------*/

kernel_ack(buf)
register TMessageItem *buf;
{
  register TTaskItem *t;
  register TMessageQueueItem *q;
  TMessageItem   out;

  t = &t_task[buf->m_sender - 1];         /* point to task table             */

  if (!t->t_pid)
  {
    krash("kernel_ack", "Not Logged In", 0);
    return 0;
  }
  if (t->t_count > 0)
  {
    t->t_count--;                         /* reduce ack count                */
    kernel_find(buf->m_sender);           /* find and send message           */
  }
  return 0;
}

/* end of Kernel_Ack.c */
