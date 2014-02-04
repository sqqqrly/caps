/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Update order header and status.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/24/93   |  tjt  Originial implementation.
 *  07/21/95   |  tjt  Revise Bard calls.
 *-------------------------------------------------------------------------*/
static char od_update_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "Bard.h"
#include "of.h"

od_update()
{
  order_replace(of_rec);
  return 0;
}

/* end of od_update.c */


