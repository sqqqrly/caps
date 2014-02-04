/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Kernel message put from a task.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/14/94   |  tjt  Original implementation.
 *             |
 *-------------------------------------------------------------------------*/
static char Message_Put_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"

Message_Put(buf)
register TMessageItem *buf;
{
  register long ret;                      /* return value                    */

  if (msgout <= 0)
  {
    return krash("Message_Put", "Queue Not Open", 1);
  }
  if (buf->m_length > MessageText)
  {
    return krash("Message_Put", "Message Too Long", 1);
  }
  while (1)
  {
    ret = msgsnd(msgout, buf, buf->m_length, 0);
      
    if (ret < 0)
    {
      if (errno == EINTR) continue;       /* interrupted by signal           */
      
      return krash("Message_Put", "System Error", 1);
    }
    break;
  }
  return 0;
}

/* end of Message_Put.c */
