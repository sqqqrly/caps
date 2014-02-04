/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open transaction file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char xt_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "oracle_defines.h"
#include "Bard.h"

long xt_open()
{
  if (sp->sp_to_flag == 'y' || sp->sp_to_flag == 'b')
  {
    transaction_open(AUTOLOCK);
  }
  return 0;
}

/*  end of xt_open.c  */      
