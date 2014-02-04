/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order file database open.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/16/93   |  tjt  Original implementation.
 *  07/21/95   |  tjt  Revised Bard calls.
 *-------------------------------------------------------------------------*/
static char od_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "Bard.h"
#include "of.h"
#include "od_open.h"

od_open()
{
  register long size;

  if (!oc_fd) return krash("od_open", "oc not open", 1);

  order_open(AUTOLOCK);                   /* open order header part          */
  order_setkey(1);                        /* key by pickline + order number  */

  of_rec = (struct of_header *)malloc(sizeof(struct of_header));
  if (!of_rec) krash("od_open", "malloc of_rec", 1);

  pick_open(AUTOLOCK);                    /* open picks part                 */
  pick_setkey(1);                         /* key is pickline + order number  */
  
  op_rec = (struct of_pick_item *)malloc(oc->op_rec_size);
  if (!op_rec) krash("od_open", "malloc op_rec", 1);

  if (oc->or_rec_size > 0)                /* open remarks part               */
  {
    remarks_open(AUTOLOCK);
    remarks_setkey(1);

    or_rec = (struct of_rmks_item *)malloc(oc->or_rec_size + 8);
    if (!or_rec) krash("od_open", "malloc or_rec", 1);
  }
  else or_rec = 0;
  
  return 0;
}
  
/* end of od_open.c */
