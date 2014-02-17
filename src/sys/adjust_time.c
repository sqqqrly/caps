/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Adjust System Clock By Signed Value.
 *                  Requires ADMIN authority.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   3/7/94    |  tjt  Original implemeatation.
 *   06/04/02  |  aha  Fixed by using asroot methodology.
 *-------------------------------------------------------------------------*/
static char adjust_time_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define COMMAND_SIZE     64
#define DATETIME_SIZE    16
#define DATETIME_FORMAT  "%Y%m%d%H%M.%S"

main(argc, argv)
long argc;
char **argv;
{
  short results         = 0;
  long adjust           = 0L;
  unsigned long int now = 0L;
  time_t * nowptr       = 0;
  char command[COMMAND_SIZE],
       datetimemsg[DATETIME_SIZE];

  putenv("_=adjust_time");
  chdir(getenv("HOME"));

  memset(datetimemsg, 0x0, DATETIME_SIZE);
  memset(command, 0x0, COMMAND_SIZE);

  if (argc < 2)
     {
       exit(1);
     }
  
  sscanf(argv[1], "%d", &adjust);

  now = time(0);
  now = now + adjust;

  nowptr = (time_t *)&now;

  results = strftime(datetimemsg,
                     DATETIME_SIZE,
                     DATETIME_FORMAT,
                     localtime(nowptr));

  if (results == 0)  
     {
       krash("main", "Cannot Change Time", 0);
     }

  sprintf(command,
          "/tcb/bin/asroot date -t %s 1>/dev/null 2>&1",
          datetimemsg);

  results = system(command);

  if (results != 0)  
     {
       krash("main", "Cannot Change Time", 0);
     }
  
  exit(0);
}

/* end of adjust_time.c */
