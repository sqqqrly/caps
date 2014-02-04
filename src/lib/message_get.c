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
 *             |
 *-------------------------------------------------------------------------*
 *  Reads one message from the input queue to the task.
 *-------------------------------------------------------------------------*/
static char Message_Get_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"

Message_Get(buf)
register TMessageItem *buf;
{
  register long ret;

  if (msgin <= 0)                         /* check queue is open             */
  {
    krash("Message_Get", "Queue Not Open", 1);
  }
  while (1)
  {
    ret = msgrcv(msgin, buf, sizeof(TMessageItem), 0, 0);

    if (ret < 0)
    {
      if (errno == EINTR) continue;       /* interrupted by signal           */
      
      return krash("Message_Get", "System Error", 1);
    }
    break;
  }
  if (ret != buf->m_length)
  {
    return krash("Message_Get", "Message Wrong Length", 1);
  }
  return 0;
}

/* end of Message_Get.c */
