/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Close transaction file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char xt_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "Bard.h"

long xt_close()
{
  transaction_close();
  return 0;
}

/* end of xt_close.c */
