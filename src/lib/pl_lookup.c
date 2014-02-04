/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Find a pickline name or number in the pickline table.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/31/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char pl_lookup_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "co.h"

long pl_lookup(name, dflt)
register char *name;
register long dflt;
{
  register struct pl_item *p;
  register long j, k, n;
  
  if (*name && inanycase("ALL", name) == 0) return 0;

  if (!co) return krash("pl_lookup", "co not open", 1);

  if (*name)
  {
    for (k = n = 0; k < strlen(name); k++)
    {
      if (name[k] < '0' || name[k] > '9') 
      {
        for (j = 0, p = pl; j < coh->co_pl_cnt; j++, p++)
        {
          if (!p->pl_pl) continue;
          if (!p->pl_name[0]) continue;
    
          if (inanycase(name, p->pl_name) == 0) return j + 1;
        }
        return - 1;
      }
      n = 10 * n + (name[k] - '0');
    }
  }
  else n = dflt;                           /* default pickline number        */
  
  if (n == 0) return 0;                    /* pickline zero is ALL           */
  else if (n >= 1 && n <= coh->co_pl_cnt && pl[n - 1].pl_pl) return n;
  return -1;
}

/* end of pl_lookup.c */ 
