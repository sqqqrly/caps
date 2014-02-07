/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Delete Shared Segment ID's.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/14/94   |  tjt  Original Implememtation.
 *-------------------------------------------------------------------------*/
static char clear_segs_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

main()
{
  putenv("_=clear_segs");
  chdir(getenv("HOME"));
  
  ss_remove();
  co_remove();
  oc_remove();
  pr_remove();
}

/* end of clear_segs */
  
