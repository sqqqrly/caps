/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Close product pending file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char ppf_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

extern FILE *ppf_fd;                      /* Product Pending File            */

long ppf_close()
{
  if (ppf_fd > 0)                         /* check file is open ok           */
  {
    fclose(ppf_fd);                       /* close file                      */
    ppf_fd = 0;
  }
  return 0;
}

/* end of ppf_close.c */
