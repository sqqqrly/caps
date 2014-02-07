/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Process kernel task log in message.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *             |
 *-------------------------------------------------------------------------*/
static char Kernel_Log_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "./kernel.h"
#include "./kernel_data.h"

/*-------------------------------------------------------------------------*
 *  Process Kernel Log In Message
 *-------------------------------------------------------------------------*/

kernel_log_in(buf)
register TMessageItem *buf;
{
  register TTaskItem *t;
   
  t = &t_task[buf->m_sender - 1];         /* point to task table             */

  if (t->t_pid)
  {
    kernel_error(buf);
    krash("kernel_log_in", "Already Logged In", 0);
    return 0;
  }
  kernel_open_out(buf->m_sender);
  memset(t, 0, sizeof(TTaskItem));
  t->t_pid = buf->m_message.m_pid;

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Kernel Log Out Message
 *-------------------------------------------------------------------------*/

kernel_log_out(buf)
register TMessageItem *buf;
{
  register TTaskItem *t;
   
  t = &t_task[buf->m_sender - 1];         /* point to task table             */

  if (!t->t_pid)
  {
    krash("kernel_log_out", "Not Logged In", 0);
    return 0;
  }
  kernel_flush(buf->m_sender);            /* remove all messages             */
  kernel_close_out(buf->m_sender);        /* close queue to task             */
   
  memset(t, 0, sizeof(TTaskItem));        /* clear entire entry              */

  return 0;
}

/* end of Kernel_Log.c */
