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
 *-------------------------------------------------------------------------*/
static char Kernel_Get_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"

extern long k_in;                         /* message input queue             */

/*-------------------------------------------------------------------------*
 *  Reads a single message from a task on the common input queue to the
 *  kernel.
 *  return -1 is error; return 0 is ok.
 *-------------------------------------------------------------------------*/

kernel_get(buf)
register TMessageItem *buf;
{
  register long ret;
  char text[100];
  
  while (1)
  {
    ret = msgrcv(k_in, buf, sizeof(TMessageItem), 0, 0);

    if (ret < 0)
    {
      if (errno == EINTR) continue;       /* interrupted by signal           */

      return krash("kernel_get", "System Error", 1);
    }
    break;
  }
  if (ret != buf->m_length)
  {
    sprintf(text, "Message Wrong Length = %d [%02x %02x %02x %02x]",
      ret, buf->m_sender, buf->m_destination, buf->m_type, buf->m_length);
    
    krash("kernel_get", text, 0);
    
    Bdump(buf, ret > 256 ? 256 : ret);
    
    return -1;
  }
  return 0;
}

/* end of Kernel_Get.c */
