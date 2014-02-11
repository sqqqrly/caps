/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  01/22/97   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char to_caps_errlog_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

main(argc, argv)\
long argc;
char **argv;
{
  long now;
  char text[128];
  
  putenv("_to_caps_errlog");
  chdir(getenv("HOME"));
   
  if (argc < 2)
  {
    printf("Usage:  to_caps_errlog \"any text\"\n\n");
    exit(1);
  }
  now = time(0);
  
  sprintf(text, "echo \"... %-47.47s %20.20s ........\" >dat/log/caps_errlog",
    argv[1], ctime(&now) + 4);
  
  exit(0);
}

/* end of to_caps_errlog.c */
