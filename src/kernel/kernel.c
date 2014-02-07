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
 *  06/01/93   | tjt - original implementation.
 *-------------------------------------------------------------------------*/
static char kernel_c[] = "%Z% %M% %I% (%G% - %U%)";

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
#include "./kernel.h"
#include "./kernel_types.h"
#include "./kernel_data.h"

extern   long kernel_watchdog();          /* watchdog routine                */
extern   long kernel_shutdown();          /* terminate everything !!!        */

long KernelTimeout = 600;                 /* default response timeout seconds*/

main(argc, argv)
long argc;
char **argv;
{
  TMessageItem buf;                       /* input message buffer            */
  register long j, k;                     /* working values                  */
  long now;                               /* message arrival time            */
   
  putenv("_=kernel");                     /* store name of self              */
  chdir(getenv("HOME"));                  /* to home directory               */
   
  setpgrp();                              /* separate task                   */
   
  signal_catcher(0);                      /* allow krash                     */

  for (k = MessageKeyOut; k <= MessageKeyMax; k++)
  {
    j = msgget(k, 0666);                  /* delete all queues               */
    if (j > 0) msgctl(j, IPC_RMID, 0);
  }
  tq_size = KPage;                        /* small queue at start            */
  tq = (unsigned char *)malloc(tq_size);  /* allocated queue space           */

  kernel_open_in();                       /* open input queue                */

  if (argc > 1)                           /* check kernel timeout            */
  {
    if (memcmp(argv[1], "-timeout=", 9) == 0)
    {
      now = atol(&argv[1][9]);
      if (now > 60) KernelTimeout = now;
    }
  }
  signal(SIGALRM, kernel_watchdog);       /* catch timeouts                  */
  alarm(kerneltimer);

#ifdef DEBUG
  buf.m_length = 0;
  kernel_start_log(&buf);
#endif
/*  -------------------- end of kernel initialization -------------------- */

/*-------------------------------------------------------------------------*
 * M A I N   L O O P   -   W A I T   A N D   P R O C E S S   M E S S A G E
 *-------------------------------------------------------------------------*/
  while (1)
  {
    if (kernel_get(&buf) < 0) continue;
      
    if (fd)                               /* logging is on                   */
    {
      now = time(0);                      /* current clock time              */

      fwrite(&now, 4, 1, fd);
      fwrite(&buf, buf.m_length + 4, 1, fd);
      fflush(fd);
    }
    if (buf.m_sender > MessageTasks)
    {
      kernel_error(&buf);
      krash("kernel main", "Invalid Sender", 0);
      continue;
    }
    if (buf.m_length > MessageText)
    {
      kernel_error(&buf);
      krash("kernel main", "Invalid Message Length", 0);
      continue;
    }
    if (buf.m_destination == KernelDestination)
    {
      switch (buf.m_type)
      {
        case KernelLogIn:    kernel_log_in(&buf); break;
      
        case KernelLogOut:   kernel_log_out(&buf);   break;
                           
        case KernelSelect:   kernel_select(&buf); break;
                           
        case KernelTaskName: kernel_task_name(&buf); break;

        case KernelAck:      kernel_ack(&buf); break;

        case KernelSignal:   kernel_signal(&buf); break;
      
        case KernelStartLog: kernel_start_log(&buf); break;
      
        case KernelStopLog:  kernel_stop_log(&buf); break;
      
        case KernelShutdown: kernel_stop(&buf); break;

/*
        case KernelWho:      kernel_who(&buf); break;
*/

        default: krash("process_kernel_message", "Bad Type", 0);
      }
      continue;
    }
    if (buf.m_destination > MessageTasks)
    {
      kernel_error(&buf);
      krash("kernel main", "Invalid Destination", 0);
      continue;
    }
    process_task_message(&buf);
  }
}
 
/* end of kernel.c */
