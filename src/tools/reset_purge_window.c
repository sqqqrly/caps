/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Resets order purge window in sp .
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
  putenv("_=reset_purge_window");
  chdir(getenv("HOME"));
  
  ss_open();
  co_open();
  
  printf( "Purge Window Was = %d \n ", sp->sp_purge_window);
/*  sp->sp_purge_window=  28800; */
  sp->sp_purge_window=  5578800;
/*  sp->sp_to_pick_event = 'n' ;    	as per carl */
/*  sp->sp_inventory = 'n' ;		as per carl */
  printf( "Purge Window Is now = %d \n ", sp->sp_purge_window);

  ss_close();
  co_close();
  
}


/* end of clear_comm.c */
  
