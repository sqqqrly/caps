/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Close product transaction file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char ptf_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

extern FILE *ptf_fd;                      /* Product Transaction File        */

long ptf_close()
{
  long ret;
 
  if (ptf_fd > 0)                         /* check file is open ok           */
  {
    fclose(ptf_fd);                       /* close file                      */
    ptf_fd = 0;
  }
  return 0;
}

/* end of ptf_close.c */
