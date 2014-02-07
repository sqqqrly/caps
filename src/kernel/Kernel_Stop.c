/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Kernel Stop All Tasks.
 *
 *                  1. Sends ShutdownRequest
 *                  2. Waits for message activity to stop (12 seconds).
 *                  3. Stops when no more tasks, or --
 *                  4. Stops when 60 seconds total has elapsed.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/04/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Stop_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <signal.h>
#include "message.h"
#include "./kernel.h"
#include "./kernel_types.h"
#include "message_types.h"

/*-------------------------------------------------------------------------*
 *  Kernel Stop Processing
 *-------------------------------------------------------------------------*/

kernel_stop(buf)
register TMessageItem *buf;
{
  extern long kernel_watchdog();
  TTaskItem *t;
  long k, task_count;
  static long requestor = 0;
  static long empty_count = 0;
  static long shutdown_count = 0;
  static unsigned char shut1[4] = {KernelDestination, 0, ShutdownRequest, 0};
  static unsigned char shut2[4] = {KernelDestination, 0, ShutdownEvent, 0};

  if (!kernelshutdown)                    /* first call for Shutdown         */
  {
    alarm(0);                             /* stop watchdog timer             */
    requestor = t_task[buf->m_sender - 1].t_pid;
    process_task_message(shut1);          /* shutdown request                */
    empty_count = 0;                      /* clear count                     */
    shutdown_count = 0;                   /* clear attempts                  */
    kernelshutdown = 1;                   /* set the shutdown flag           */
    kerneltimer    = ShutdownTimeout;     /* set faster timeout              */
    signal(SIGALRM, kernel_watchdog);     /* start watchdog again            */
    alarm(kerneltimer);
    return 0;
  }
  shutdown_count++;                       /* count number of timeouts        */
  
  kernel_pack();                          /* repack queue space for empty    */
    
  if (!tq_end) empty_count++;             /* count times queue is empty      */
  else empty_count = 0;                   /* reset times found empty         */
    
  if (empty_count >= 2 && kernelshutdown == 1)/* shutdown request done       */
  {
    kernelshutdown = 2;
    process_task_message(shut2);          /* shutdown demand event           */
    return 0;
  }
  if (kernelshutdown == 2)
  {
    task_count = 0;                       /* clear count of login task       */

    for (k = 1, t = t_task; k <= MessageTasks; k++, t++)
    {
      if (!t->t_pid) continue;
      task_count++;                       /* some task is running            */
    }
    if (!task_count || shutdown_count > ShutdownCount)
    {
      kernel_shutdown();
      kill(requestor, SIGUSR1);           /* signal requestor                */
      exit(0);                            /* kernel dies now                 */
    }
  }
  return 0;
}

/* end of Kernel_Stop.c */
