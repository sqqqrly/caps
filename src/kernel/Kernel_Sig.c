/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Process task signal message.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Sig_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Process Kernel Signal Message
 *-------------------------------------------------------------------------*/

kernel_signal(buf)
register TMessageItem *buf;
{
  register TTaskItem *t;
   
  t = &t_task[buf->m_sender - 1];         /* point to task table             */

  if (!t->t_pid)
  {
    krash("kernel_signal", "Not Logged In", 0);
    return 0;
  }
  t->t_signal = buf->m_message.m_signal;  /* update signal value             */

  return 0;
}

/* end of Kernel_Sig.c */
