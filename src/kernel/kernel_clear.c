/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Multifunction CAPS Message Processor.
 *
 *  Called:   kernel  [-timeout=xxx]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  02/21/94   | tjt - original implementation.
 *-------------------------------------------------------------------------*/
static char kernel_clear_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *
 *  The kernel is the message passing distributor for CAPS.  All messages
 *  are sent to the kernel on its input ipc and all messages are sent to
 *  tasks on its output ipc.
 *-------------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "message.h"

main(argc, argv)
long argc;
char **argv;
{
  register long j, k;                     /* working values                  */
   
  putenv("_=kernel_clear");               /* store name of self              */
  chdir(getenv("HOME"));                  /* to home directory               */
   
  setpgrp();                              /* separate task                   */
   
  printf("Clearing IPC Queues\n\n");	
  for (k = MessageKeyOut; k <= MessageKeyMax; k++)
  {
    printf("Queue %d", k);
    
    j = msgget(k, 0666);                  /* delete all queues               */
    if (j > 0) 
    {
      printf(" Deleted\n");
      msgctl(j, IPC_RMID, 0);
    }
    else printf(" Gone\n");
  }
}
 
/* end of kernel_clear.c */
