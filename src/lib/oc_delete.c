/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Remove order from index and frees space.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/17/93    |  tjt  Added to mfc.
 *  7/23/93    |  tjt  Rewritten.
 *-------------------------------------------------------------------------*/
static char oc_delete_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"

oc_delete(block)
register long block;
{
  if (oc_fd <= 0) krash("oc_delete", "oc not open", 1);

  if (block < 1 || block > oc->of_size)
  {
    return krash("oc_delete", "block out of range", 1);
  }
  memset(&oc->oi_tab[block - 1], 0, sizeof(struct oi_item));

  return 0;
}

/* end of oc_delete.c */ 
 
