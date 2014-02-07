/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Open a message queue for the kernel.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/14/93   |  tjt  Original implementation.
 *  05/14/96   |  tjt  Ignore ands ipc queue id of zero.
 *-------------------------------------------------------------------------*/
static char Kernel_Open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "message.h"
#include "./kernel_types.h"

long k_in;                                /* input queue to kernel           */
long k_out[MessageTasks];                 /* output queue from kernel        */

/*-------------------------------------------------------------------------*
 *  There is only one input queue for all tasks.  Opened at system startup.
 *  The queue is removed, if exists, and is reopened to flush any trash.
 *-------------------------------------------------------------------------*/

kernel_open_in()
{
  k_in = msgget(MessageKeyOut, 0666 | IPC_CREAT);

  if (k_in >= 0)                          /* exists - delete and create      */
  {
    msgctl(k_in, IPC_RMID, 0);
    k_in = msgget(MessageKeyOut, 0666 | IPC_CREAT);
  }
  if (k_in <= 0)
  {
    return krash("kernel_open_in", "Open Input Queue", 1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Each task has its own, unique input queue. Openned at KernelLogIn.
 *-------------------------------------------------------------------------*/
 
kernel_open_out(k)
register long k;                          /* task id 1 .. 32                 */
{
  TMessageItem buf;

  if (k < 1 || k > MessageTasks) return -1;

  k_out[k - 1] = msgget(MessageKeyMin + k - 1, 0666);

  if (k_out[k - 1] < 0)
  {
    return krash("kernel_open_out", "Open Output Queue", 1);
  }
  buf.m_sender      = KernelDestination;
  buf.m_destination = k;
  buf.m_type        = KernelAck;
  buf.m_length      = 0;
   
  kernel_put(&buf);                       /* ack to task                     */

  return 0;
}

/* end of Kernel_Open.c */
