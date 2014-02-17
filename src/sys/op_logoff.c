/*-------------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Logoff caps system.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/9/93    |  tjt  Original impletmentation
 *-------------------------------------------------------------------------*/
static char logoff_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "iodefs.h"
#include "sd.h"

extern leave();                      /* graceful exit routine           */

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=logoff");                     /* name to environemnt             */
  chdir(getenv("HOME"));
  
  signal(SIGQUIT, leave);                 /* log off on any error            */
  
  sd_open(leave);                         /* open messages to kernel         */
  sd_clear(0);
  leave();
}
/*-------------------------------------------------------------------------*
 *  Logoff or Error Exit From CAPS
 *-------------------------------------------------------------------------*/
leave()
{
  sd_close();                             /* close tty server                */

  signal(SIGTERM, SIG_IGN);
  kill(0, SIGTERM);
  
  system("stty sane echo");

  printf("\r\n\n Goodbye ...\r\n\n");

  exit(1);                               /* server dies on hangup signal    */
}

/* end of logoff.c */
