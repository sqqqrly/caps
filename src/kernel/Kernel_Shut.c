/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Process shutdown message.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_Shut_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <signal.h>

#include "message.h"
#include "./kernel.h"

/*-------------------------------------------------------------------------*
 *  Kernel Shutdown
 *-------------------------------------------------------------------------*/

kernel_shutdown()
{
  register TTaskItem *t;
  register long k;
   
  kernel_close_in();                      /* remove input queue              */

  for (k = 1, t = t_task; k <= MessageTasks; k++, t++)
  {
    if (!t->t_pid) continue;              /* no task here                    */
      
    kill(t->t_pid, SIGKILL);              /* kill any attached tasks         */

    kernel_close_out(k);                  /* remove queue                    */
  }
  return 0;
}

/* end of Kernel_Shut.c */
