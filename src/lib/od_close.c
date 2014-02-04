/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order file database close.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  9/10/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char od_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "Bard.h"
#include "of.h"

od_close()
{
  order_close();
  pick_close();
  remarks_close();
 
  if (of_rec) free(of_rec);
  if (op_rec) free(op_rec);
  if (or_rec) free(or_rec);

  of_rec = 0;
  op_rec = 0;
  or_rec = 0;

  return 0;
}
  
/* end of od_close.c */
