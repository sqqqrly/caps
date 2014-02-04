/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Close batch receipts file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char brf_close_c[] = "%Z% %M% %I% (%G% - %U%)";

/*  
 *   Close Batch Receipts File
 */
#include <stdio.h>

extern FILE *brf_fd;                      /* Batch Receipts File             */

long brf_close()
{
  long ret;
 
  if (brf_fd > 0)                         /* check file is open ok           */
  {
    fclose(brf_fd);                       /* close file                      */
    brf_fd = 0;
  }
  return 0;
}

/* end of brf_close.c */
