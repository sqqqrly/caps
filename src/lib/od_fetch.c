/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Get order file header and status
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/23/93   |  tjt Original implementation.
 *  07/21/95   |  tjt Revise Bard calls.
 *-------------------------------------------------------------------------*/
static char od_fetch_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"
#include "Bard.h"

od_fetch(block, lock)
register long block, lock;
{
  register long ret;
  register struct oi_item *q;
  char text[80];
  
  if (block < 1 || block > oc->of_size)
  {
    sprintf(text, "invalid block %d", block);
    krash("od_get", text, 1);
  }
  q = &oc->oi_tab[block - 1];
   
  of_rec->of_pl = q->oi_pl;
  of_rec->of_on = q->oi_on;
   
  order_setkey(1);

  ret = order_read(of_rec, lock);

  if (ret)
  {
    sprintf(text, "of not found - block = %d order (%d: %d)", 
      block, q->oi_pl, q->oi_on);
    krash("od_get", text, 1);
  }
  return 0;
}

/* end of od_fetch.c */

