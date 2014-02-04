/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Open a message queue for a user task.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/14/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*
 *  Opens the common output queue and searches for an available input
 *  queue.  The number of the queue becomes the number of the task.
 *-------------------------------------------------------------------------*/
static char Message_Open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "message.h"

/*-------------------------------------------------------------------------*
 *  Attempt all possible queues until can create a new one.
 *  Each task has its own, exclusive input queue.
 *-------------------------------------------------------------------------*/
Message_Open_In()
{
  register long k, ret;

  if (msgin > 0) return 0;                /* already open                    */

  for (msgtask = 1, k = MessageKeyMin; k <= MessageKeyMax; k++, msgtask++)
  {
    msgin = msgget(k, 0666 | IPC_CREAT | IPC_EXCL);
   
    if (msgin < 0)                        /* an error has occured            */
    {
      if (errno == EEXIST) continue;      /* try again - already in use      */

      return krash("Message_Open", "Open Input Queue", 1);
    }
    else break;
  }
  if (k > MessageKeyMax) return krash("Message_Open", "Too Many Queues", 1);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  All tasks share to same output queue.
 *-------------------------------------------------------------------------*/
Message_Open_Out()
{
  if (msgout > 0) return 0;               /* already open                    */

  msgout = msgget(MessageKeyOut, 0666);   /* output of task                  */

  if (msgout < 0) return krash("Message_Open", "Open Output Queue", 1);
  return 0;
}

/* end of Message_Open.c */
