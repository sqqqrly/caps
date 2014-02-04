/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:   Removes Task Input Queue
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/10/93   |  tjt  Original implementation.
 *             |
 *-------------------------------------------------------------------------*
 *  Sends LogOut message to the kernel and deletes message ids.  It is safe
 *  to reopen the messages queues immediately; however, messages to the
 *  task may be lost during time of closure.
 *-------------------------------------------------------------------------*/
static char message_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "kernel_types.h"
#include "message.h"

message_close()
{
  register long ret;

  if (msgsig) signal(msgsig, SIG_IGN);    /* ignore signal                   */
  msgsig = 0;                             /* delete signal                   */

  if (msgout > 0)                         /* check queue is open             */
  {
    ret = message_put(KernelDestination, KernelLogOut, 0, 0);
  }
  msgin = msgout = msgtask = 0;           /* delete id's                     */

  return ret;
}

/* end of message_clos.c */
