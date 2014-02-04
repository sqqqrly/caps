/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Read order header, status, and picks.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/23/93   |  tjt  Original implementation
 *  11/07/95   |  tjt  Ignore orphans.
 *-------------------------------------------------------------------------*/
static char od_status_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <string.h>
#include "co.h"
#include "of.h"
#include "Bard.h"
#include "zone_status.h"

od_status(block, status)
register long block;
register unsigned char *status;
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct zone_item *z;
  register struct bay_item  *b;
  register long j, k, min, queue;
  struct of_pick_item x;
  
  pick_setkey(1);
  
  queue    = oc->oi_tab[block - 1].oi_queue;
  min      = oc->oi_tab[block - 1].oi_entry_zone - 1;
  x.pi_pl  = oc->oi_tab[block - 1].oi_pl;
  x.pi_on  = oc->oi_tab[block - 1].oi_on;
  x.pi_mod = 1;
  pick_startkey(&x);
   
  x.pi_mod = ProductMax;
  pick_stopkey(&x);
   
  memset(status, 0x20, ZoneMax);
          
  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
     status[k] = OS_NOPICKS;
  }
  while (!pick_next(&x, NOLOCK))
  {
    if (x.pi_flags & PICKED)  continue;
    if (x.pi_flags & NO_PICK) continue;    
    
    i = &pw[x.pi_mod - 1];
    h = &hw[i->pw_ptr - 1];
    b = &bay[h->hw_bay - 1];
    j = b->bay_zone - 1;

    status[j] = OS_PICKS;
  }
  for (k = 0; k < coh->co_zone_cnt; k++)
  {
    if (queue == OC_UW && k < min && !(zone[k].zt_flags & IsSegmented))
    {
      if (status[k] == OS_NOPICKS) status[k] = OS_COMPLETE;
    }
    if (zone[k].zt_order == block)
    {
      if (zone[k].zt_status == ZS_UNDERWAY)
      {
        if (status[k] == OS_PICKS) status[k] = OS_UW;
        else status[k] = OS_UW_NP;
        continue;
      }
      else if (zone[k].zt_status == ZS_EARLY)
      {
        if (status[k] == OS_PICKS) status[k] = OS_EARLY;
        else status[k] = OS_EARLY_NP;
        continue;
      }
      else if (zone[k].zt_status == ZS_LATE)
      {
        if (status[k] == OS_PICKS) status[k] = OS_LATE;
        else status[k] = OS_LATE_NP;
        continue;
      }
      else if (zone[k].zt_status == ZS_AHEAD)
      {
        if (status[k] == OS_PICKS) status[k] = OS_AHEAD;
        else status[k] = OS_AHEAD_NP;
        continue;
      }
    }
  }
  return 0;
}

/* end of od_status.c */
