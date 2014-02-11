/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Set Flag To Simulation
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/23/94   |  tjt  Original implementation
 *-------------------------------------------------------------------------*/
static char set_ff_simulate_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=set_ff_simulate");
  chdir(getenv("HOME"));

  ss_open();
  
  printf("sp_full_function = %c\n", sp->sp_full_function);
  
  sp->sp_full_function = 's';

  printf("sp_full_function = %c\n", sp->sp_full_function);

  ss_close_save();
  return 0;
}

/* end of set_ff_simulate.c */


