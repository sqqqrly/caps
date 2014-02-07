/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Add message selections or clears all but shutdown.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Selct_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "message_types.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Process Kernel Select Message
 *-------------------------------------------------------------------------*/

kernel_select(buf)
register TMessageItem *buf;
{
  register TTaskItem *t;
  register unsigned char *p, bit;
  register long k, byte;

  t = &t_task[buf->m_sender - 1];         /* point to task table             */

  if (!t->t_pid)
  {
    krash("kernel_select", "Not Logged In", 0);
    return 0;
  }
  p = buf->m_message.m_select;

  if (buf->m_length < 1)                  /* no selects clears all           */
  {
    kernel_flush(buf->m_sender);          /* wipeout all messages            */
    *p = ShutdownEvent;                   /* include shutdown always         */
    buf->m_length = 1;
  }
  byte = Byte[buf->m_sender - 1];
  bit  = Bit[buf->m_sender - 1];

  for (k = 0; k < buf->m_length; k++, p++)
  {
    ms[*p][byte] |= bit;                  /* mark message selections         */
  }
  return 0;
}

/* end of Kernel_Selct.c */


