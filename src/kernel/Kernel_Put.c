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
 *-------------------------------------------------------------------------*/
static char Kernel_Put_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"
#include "./kernel.h"

extern long k_out[MessageTasks];          /* message output queue            */

/*-------------------------------------------------------------------------*
 *  Writes a message to a task queue.
 *-------------------------------------------------------------------------*/

kernel_put(buf)
register TMessageItem *buf;
{
  register long ret;                      /* return value                    */
  long now;                               /* current time                    */
   
  if (buf->m_destination < 1 || buf->m_destination > MessageTasks)
  {
    kernel_error(buf);
    krash("kernel_put", "Invalid Destination", 0);
    return - 1;
  }
  if (k_out[buf->m_destination - 1] <= 0)
  {
    krash("kernel_put", "Queue Not Open", 0);
    return - 1;
  }
  if (buf->m_length > MessageText)
  {
    krash("kernel_put", "Message Too Long", 0);
    return -1;
  }
  while (1)
  {
    ret = msgsnd(k_out[buf->m_destination - 1], buf, buf->m_length, 0);
   
    if (ret < 0)
    {
      if (errno == EINTR) continue;       /* interrupted by signal           */
      return krash("kernel_put", "System Error", 1);
    }
    break;
  }
  if (fd)                                 /* logging is on                   */
  {
    now = 0;                              /* zero is kernel send             */

    fwrite(&now, 4, 1, fd);
    fwrite(buf, buf->m_length + 4, 1, fd);
    fflush(fd);
  }
  return 0;
}

/* end of Kernel_Put.c */
