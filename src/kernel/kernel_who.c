/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Obtain List of Current Kernel Tasks
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/21/94   |  tjt  Original Implementation
 *-------------------------------------------------------------------------*/
static char kernel_who[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "message.h"
#include "./kernel.h"
#include "./kernel_types.h"

main(argc, argv)
long argc;
char **argv;
{
  extern leave();
  long who, type, len;
  char name[40];
  
  message_open();
  
  message_put(KernelDestination, KernelWho, 0, 0);

  signal(SIGALRM, leave);

  while (1)
  {
    alarm(3);
    message_get(&who, &type, name, &len);

    name[len] = 0;
    printf("%2d: %s\n", who, name);
  }
  message_close();
  exit(0);
}
leave()
{
  message_close();
  exit(0);
}

/* end of kernel_who.c */
