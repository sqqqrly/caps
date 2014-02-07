/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Process intertask message.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Msg_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"
#include "./kernel_types.h"

/*-------------------------------------------------------------------------*
 *  Process Task Message
 *-------------------------------------------------------------------------*/

process_task_message(buf)
register TMessageItem *buf;
{
  TTaskItem *t;
  TMessageQueueItem *q;
  register long k;
   
  if (buf->m_sender != KernelDestination)
  {
    t = &t_task[buf->m_sender - 1];       /* point to sender task            */

    if (!t->t_pid)
    {
      krash("process_task_message", "Not Logged In", 0);
      return 0;
    }
    t->t_rcv_time  =  time(0);
    t->t_rcv_count += 1;
  } 
  if (kernel_append(buf))
  {
    for (k = 1, t = t_task; k <= MessageTasks; k++, t++)
    {
      if (!t->t_pid)             continue;
      if (t->t_count)            continue;
      if (t->t_offset >= tq_end) continue;
         
      kernel_find(k);
    }
  }
  return 0;
}

/* end of Kernel_Msg.c */
