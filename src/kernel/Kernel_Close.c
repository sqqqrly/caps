/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Close messages queue for the kernel.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/14/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"

extern long k_in;                         /* input queue to kernel           */
extern long k_out[MessageTasks];          /* output queue from kernel        */

/*-------------------------------------------------------------------------*
 *  Closes and deletes queues.  Note: The kernel must delete the output
 *  queue(s).  System errors will occur if the task deletes this queue 
 *  since the queue may be deleted before the kernel knows its gone.
 *-------------------------------------------------------------------------*/

kernel_close_in()
{
  register long ret;

/*-------------------------------------------------------------------------*
 *  There is only one input queue for all tasks. Closed only at Shutdown.
 *-------------------------------------------------------------------------*/

  ret  = msgctl(k_in, IPC_RMID, 0);

  if (ret < 0)
  {
    krash("kernel_close_in", "Close Input Queue", 0);
  }
  k_in = 0;
   
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Each task has its own, unique input queue.  Closed at KernelLogOut.
 *-------------------------------------------------------------------------*/
 
kernel_close_out(k)
register long k;                          /* task id 1 .. 32                 */
{
  register long ret;

  if (k_out[k - 1] <= 0) return 0;        /* is not open                     */

  ret  = msgctl(k_out[k - 1], IPC_RMID, 0);

  if (ret < 0)
  {
    krash("kernel_close_out", "Close Output Queue", 0);
  }
  k_out[k - 1] = 0;                       /* delete id from list             */

  return 0;
}

/* end of Kernel_Close.c */
