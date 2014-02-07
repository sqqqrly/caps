/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Kernel acknowledge watchdog.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_WDog_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <signal.h>
#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Kernel Watchdog Routine - Check Acks Of Messages
 *-------------------------------------------------------------------------*/

kernel_watchdog()
{
  register TTaskItem *t;
  register long k, now;
  unsigned char message[128];
   
  now = time(0);
   
  for (k = 1, t = t_task; k <= MessageTasks; k++, t++)
  {
    if (!t->t_pid)    continue;
    if (!t->t_count)  continue;
   
    if (now - t->t_snd_time > KernelTimeout)
    {
      sprintf(message, "KERNEL: Sender:%d (%s) Watchdog Timeout",
      k, t->t_name);
      errlog(message);                    /* name of unresponsive task       */

      krash("kernel_watchdog", "Task Suspended", 0);

      kill(t->t_pid, SIGTRAP);            /* attempt to suspend task         */

      kernel_close_out(k);                /* close queue                     */
      kernel_flush(k);                    /* flush all messages to task      */
      memset(t, 0, sizeof(TTaskItem));    /* clear task entry                */
    }
  }
  if (kernelshutdown) kernel_stop(0);     /* check shutdown progress         */

  signal(SIGALRM, kernel_watchdog);       /* catch timeouts                  */
  alarm(kerneltimer);                     /* reset timer                     */
  return 0;
}

/* end of Kernel_WDog.c */
