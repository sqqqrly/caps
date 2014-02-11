/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Test Signal Catcher.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  01/22/97   |  tjt  Original Implementation.
 *-------------------------------------------------------------------------*/
static char test_catcher_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  
  signal_catcher(1);
  
  for (k = 1; k <= 17; k++)
  {
    if (k != 9) kill(getpid(), k);
  }
  exit(1);

}

/* end of test_catcher.c */

