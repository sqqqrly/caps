/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Find a stkloc in the SKU table - Uses serial search.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/22/96   |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char stkloc_lookup_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "co.h"
#include "st.h"

struct st_item *stkloc_lookup(stkloc)
register char *stkloc;
{
  register struct st_item *s;
  register long k;
  char work[StklocLength];
  
  if (!ssi) return (long)krash("stkloc_lookup", "ss not open", 1);
  if (!co)  return (long)krash("stkloc_lookup", "co not open", 1);

  if (coh->co_st_cnt <= 0) return 0;     /* table is empty               */
  if (rf->rf_stkloc < 1)   return 0;     /* no stkloc length             */
  
  memcpy(work, stkloc, rf->rf_stkloc);
  strip_space(work, rf->rf_stkloc);
  
  for (k = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    if (memcmp(s->st_stkloc, work, rf->rf_stkloc) == 0) return s;
  }
  return 0;
}

/* end of stkloc_lookup.c */ 
