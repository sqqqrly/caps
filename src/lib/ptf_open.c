/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open product transaction file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char ptf_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"

FILE *ptf_fd;

long ptf_open(parm)
char *parm;                               /* open parameters                 */
{
  ptf_fd = fopen(ptf_name, parm);        /* Open transaction file           */
  if (ptf_fd == 0)
  {
    return krash("ptf_open", "failed on ptf", 1);
  }
  return 0;
}

/*  end of ptf_open.c  */      
