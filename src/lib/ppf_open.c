/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open pending product file actions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modidied for UNIX.
 *-------------------------------------------------------------------------*/
static char ppf_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"

FILE *ppf_fd;

long ppf_open(parm)
char *parm;                               /* open parameters                 */
{
  ppf_fd = fopen(ppf_name, parm);        /* Open transaction file           */
  if (ppf_fd == 0)
  {
    return krash("ppf_open", "failed on ppf", 1);
  }
  return 0;
}

/*  end of ppf_open.c  */      
