/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Find a SKU in the SKU table - Uses binary search.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/22/96   |  tjt  Change length to rf->rf_sku.
 *-------------------------------------------------------------------------*/
static char sku_lookup_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "co.h"
#include "st.h"

struct st_item *sku_lookup(pickline, sku)
register long pickline;
register char *sku;
{
  extern sku_cmp();
  struct st_item work, *s;

  if (!ssi) return (long)krash("sku_lookup", "ss not open", 1);
  if (!co)  return (long)krash("sku_lookup", "co not open", 1);

  if (coh->co_st_cnt <= 0) return 0;     /* table is empty               */
  if (rf->rf_sku < 1)      return 0;     /* no sku length                */
  
  work.st_pl = pickline;
  memcpy(work.st_sku, sku, rf->rf_sku);
  strip_space(work.st_sku, rf->rf_sku);

  s = (struct st_item *)bsearch(&work, st, coh->co_st_cnt, 
                                sizeof(struct st_item), sku_cmp);
  if (!s || sp->sp_mirroring != 'y') return s;

  for (; s > st; s--)
  {
    if (sku_cmp(s, s - 1)) break;
  }
  return s;
}

sku_cmp(p, q)
register struct st_item *p, *q;
{
  return memcmp(p, q, rf->rf_sku + 1);
}

/* end of sku_lookup.c */ 
