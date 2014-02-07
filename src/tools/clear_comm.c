/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear Comm Locked Flags.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/28/94   |  tjt  Original Implememtation.
 *-------------------------------------------------------------------------*/
static char clear_comm_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"

main()
{
  putenv("_=clear_comm");
  chdir(getenv("HOME"));
  
  ss_open();
  
  sp->sp_oi_mode = 0x20;
  sp->sp_to_mode = 0x20;
/*
  sp->sp_remaining_picks = 'u';
  sp->sp_order_selection = 'y';
*/
  ss_close();
  
}


/* end of clear_comm.c */
  
