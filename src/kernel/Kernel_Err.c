/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Produce error information to errlog.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Err_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Kernel Error Message Routine
 *-------------------------------------------------------------------------*/

kernel_error(buf)
register TMessageItem   *buf;
{
  unsigned char sname[32], dname[32], message[128];

  *sname = *dname = 0;

  if (buf->m_sender > 0 && buf->m_sender <= MessageTasks)
  {
    if (t_task[buf->m_sender - 1].t_pid)
    {
      strcpy(sname, t_task[buf->m_sender - 1].t_name);
    }
  }
  if (buf->m_destination > 0 && buf->m_destination <= MessageTasks)
  {
    if (t_task[buf->m_destination - 1].t_pid)
    {
      strcpy(dname, t_task[buf->m_destination - 1].t_name);
    }
  }
  sprintf(message, "Kernel: Sender:%d (%s) Dest:%d (%s) Type:%d Len:%d",
  buf->m_sender, sname, buf->m_destination, dname,
  buf->m_type, buf->m_length);

  errlog(message);
}

/* end of Kernel_Err.c */
