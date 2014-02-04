/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open batch receipts file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt   Revised to mfc.
 *  04/13/94   |  tjt   Modified ro UNX.
 *-------------------------------------------------------------------------*/
static char brf_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"

FILE *brf_fd;

long brf_open(parm)
char *parm;                               /* open parameters                 */
{
  brf_fd = fopen(brf_name, parm);         /* Open Batch Receipts file        */
  if (brf_fd == 0)
  {
    return krash("brf_open", "failed on brf", 1);
  }
  return 0;
}

/*  end of brf_open.c  */
