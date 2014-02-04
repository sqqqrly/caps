/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Kernel message get for a task.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/14/94   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char Message_GNW_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"

/*-------------------------------------------------------------------------*
 *  Reads one message with no wait from the input queue to the task.
 *  returns 0 if ok; return 1 if no message; returns -1 if error.
 *-------------------------------------------------------------------------*/
Message_GNW(buf)
register TMessageItem *buf;
{
  register long ret;

  if (msgin <= 0)                         /* check queue is open             */
  {
    return krash("Message_GNW", "Queue Not Open", 1);
  }
  while (1)
  {
    ret = msgrcv(msgin, buf, sizeof(TMessageItem), 0, IPC_NOWAIT);

    if (ret < 0)
    {
      if (errno == EINTR) continue;       /* interrupted by signal           */
      if (errno == ENOMSG) return 1;      /* no message available            */

      return krash("Message_GNW", "System Error", 1);
    }
    break;
  }
  if (ret != buf->m_length)
  {
    return krash("Message_GNW", "Message Wrong Length", 1);
  }
  return 0;
}

/* end of Message_GNW.c */
