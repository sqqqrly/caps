/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Search the entire index for an order by pl + con.
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
 *  02/24/97   |  tjt  Modified for CON lookup.
 *-------------------------------------------------------------------------*/
static char oc_find_con_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"
#include "ss.h"

oc_find_con(pickline, con)
register TPickline pickline;
register char *con;
{
  register struct oi_item *o;
  register long k, block;

  if (oc_fd <= 0) krash("oc_find_con", "oc not open", 1);
  if (ss_fd <= 0) krash("oc_find_con", "ss not open", 1);
  
  for (k = OC_COMPLETE; k <= OC_HOLD; k++)
  {
    block = oc->oc_tab[pickline - 1].oc_queue[k].oc_first;
    
    while (block > 0)
    {
      o = &oc->oi_tab[block - 1];
      
      if (memcmp(o->oi_con, con, rf->rf_con) == 0) return block;

      block = o->oi_flink;
    }
  }
  return 0;                               /* failed !!!                      */
}

/* end of oc_find_con.c */
