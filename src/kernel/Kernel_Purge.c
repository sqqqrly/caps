/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Kernel dead task purge.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/14/96   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Purge_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "message.h"
#include "./kernel.h"

#define MAX 200

/*-------------------------------------------------------------------------*
 *  Kernel Watchdog Routine - Check For Dead Tasks
 *-------------------------------------------------------------------------*/

kernel_purge()
{
  FILE *fd;
  register TTaskItem *t;
  register long max, j, k;
  unsigned char buf[8], message[128];
  long tlist[MAX];
  
  system("ps -ef | cut -c 10-14 >tmp/KPURGE");  /* get all pids              */
  
  fd = fopen("tmp/KPURGE", "r");
  if (fd == 0) return 0;
  
  for (max = 0; max < MAX; max++)
  {
    if (!fgets(buf, 8, fd)) break;
    tlist[max] = atol(buf);
  }
  fclose(fd);
   
  for (k = 1, t = t_task; k <= MessageTasks; k++, t++)
  {
    if (!t->t_pid) continue;
    
    for (j = 0; j < max; j++)
    {
      if (tlist[j] == t->t_pid) break;
    }
    if (j < max) continue;                /* task is alive                   */
    {
      sprintf(message, "KERNEL: %d Pid %d (%s) Is Dead",
        k, t->t_pid, t->t_name);

      errlog(message);                    /* name of unresponsive task       */

      kernel_close_out(k);                /* close queue                     */
      kernel_flush(k);                    /* flush all messages to task      */
      memset(t, 0, sizeof(TTaskItem));    /* clear task entry                */
    }
  }
  if (kernelshutdown) kernel_stop(0);     /* check shutdown progress         */

  signal(SIGALRM, kernel_purge);          /* catch timeouts                  */
  alarm(KernelTimeout);                   /* reset timer                     */
  return 0;
}

/* end of Kernel_Purge.c */
