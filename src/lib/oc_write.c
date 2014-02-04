/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Add an order to the index file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/20/95   |  tjt  Added hashing.
 *  06/28/95   |  tjt  Add block as long.
 *-------------------------------------------------------------------------*/
static char oc_write_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"

long oc_hole();

oc_write(pl, on)
register TPickline pl;
register TOrder    on;
{
  register struct oi_item *p;
  register long k;

  if (oc_fd <= 0) krash("oc_write", "oc not open", 1);
  
/*------------------------------------------------------------------------*
 *  Find Empty Hole In Index
 *------------------------------------------------------------------------*/

  k = oc_hole(pl, on);                    /* find a hole in the heap         */

  if (k <= 0) return 0;                   /* not enough space !!!            */

/*-------------------------------------------------------------------------*
 * Build Index Entry
 *-------------------------------------------------------------------------*/
  p = &oc->oi_tab[k - 1];                 /* point to entry                  */

  memset(p, 0, sizeof(struct oi_item));   /* insure all is clear             */

  p->oi_pl = pl;                          /* pickline                        */
  p->oi_on = on;                          /* order number                    */

  return k;                               /* return block number             */
}
/*-------------------------------------------------------------------------*
 *  Find An Empty Hole
 *-------------------------------------------------------------------------*/
long oc_hole(pl, on)
register TPickline pl;
register TOrder    on;
{
  register struct oi_item *p;
  register long  k, key;

  key = ((pl << 24) + on) % oc->of_size;  /* hash key is modulo size         */
  
  for (k = key, p = &oc->oi_tab[key]; k < oc->of_size; k++, p++)
  {
    if (!p->oi_pl) return (k + 1);        /* found a hole                    */
  }
  for (k = 0, p = oc->oi_tab; k < key; k++, p++)
  {
    if (!p->oi_pl) return (k + 1);        /* found hole                      */
  }
  return 0;                               /* failed !!!                      */
}

/* end of oc_write.c */
