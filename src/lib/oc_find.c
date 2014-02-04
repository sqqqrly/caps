/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Search the entire index for an order.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  07/23/93   |  tjt  Rewritten.
 *  04/20/95   |  tjt  Added hashing.
 *  06/28/95   |  tjt  Add long block.
 *-------------------------------------------------------------------------*/
static char oc_find_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"

oc_find(pl, order)
register TPickline pl;
register TOrder    order;
{
  register struct oi_item *p;
  register long  k, key;

  if (oc_fd <= 0) krash("oc_find", "oc not open", 1);
  
  key = ((pl << 24) + order) % oc->of_size;/* hash key is modulo size        */
  
  for (k = key, p = &oc->oi_tab[key]; k < oc->of_size; k++, p++)
  {
    if (p->oi_pl == pl && p->oi_on == order) return (k + 1);
  }
  for (k = 0, p = oc->oi_tab; k < key; k++, p++)
  {
    if (p->oi_pl == pl && p->oi_on == order) return (k + 1);
  }
  return 0;                               /* failed !!!                      */
}

/* end of oc_find.c */
