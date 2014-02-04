/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open links to kernel.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/14/93    |  tjt  Original implementation
 *-------------------------------------------------------------------------*/
static char message_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <string.h>
#include "message.h"
#include "message_types.h"
#include "kernel_types.h"

message_open()
{
  static unsigned char defaults[] = {ShutdownEvent, 0};
  unsigned short pid;
  register char  *p;
  long ret, type;

  if (msgin > 0 || msgout > 0) return 0;  /* already open                    */

/* ----------------- fails if kernel queue not open ------------------------ */

  if (Message_Open_Out() < 0) return -1;  /* open output to kernel           */
  if (Message_Open_In() < 0)  return -1;  /* open input from kernel          */

  pid = getpid();                         /* get pid of task                 */

  if (message_put(KernelDestination, KernelLogIn, &pid, 2) < 0) return -1;
  
/* -----------------  hangs forever if kernel has died --------------------- */

  if (message_get(0, &type, 0, 0) < 0) return -1;
  
  if (type != KernelAck) 
  {
    return krash("message_open", "Expected Ack", 1);
  }
  if (message_put(KernelDestination, KernelSelect, defaults, 1) < 0) return -1;

  p   = (char *)getenv("_");              /* get name of task                */
  if (p)
  {
    ret = message_put(KernelDestination, KernelTaskName, p, strlen(p));
    if (ret < 0) return - 1;
  }
  return 0;
}

/* end of message_open.c */
