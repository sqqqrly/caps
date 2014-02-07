/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Process task name message.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Name_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Process Kernel Task Name Message
 *-------------------------------------------------------------------------*/

kernel_task_name(buf)
register TMessageItem *buf;
{
  register TTaskItem *t;

  t = &t_task[buf->m_sender - 1];         /* point to task table             */

  if (!t->t_pid)
  {
    krash("kernel_task_name", "Not Logged In", 0);
    return 0;
  }
  strncpy(t->t_name, buf->m_message.m_text, buf->m_length);
  t->t_name[buf->m_length] = 0;
  return 0;
}

/* end of Kernel_Name.c */


