/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear Transaction Count.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/20/94   | tjt  Original Implementation
 *-------------------------------------------------------------------------*/
static char clear_trans_count_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"

main()
{
  putenv("_=clear_trans_count");
  chdir(getenv("HOME"));
  
  ss_open();
  
  printf("Clear Transaction Count = %d\n\n", sp->sp_to_count);

  sp->sp_to_count = 0;
  
  printf("Clear Transaction Count = %d\n\n", sp->sp_to_count);
  
  ss_close_save();
}

/* end of clear_trans_count.c */
