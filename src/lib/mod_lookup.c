/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Find a SKU in the SKU table by module number
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   9/24/93   |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char mod_lookup_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "co.h"
#include "st.h"

struct st_item *mod_lookup(mod)
register long mod;
{
  register long k;
  register struct st_item *s;

  if (!co)  return krash("mod_lookup", "co not open", 1);

  if (coh->co_st_cnt <= 0) return 0;     /* table is empty               */

  for (k = 0, s = st; k < coh->co_st_cnt; k++, s++)
  {
    if (s->st_mod == mod) return s;
  }
  return 0;
}

/* end of mod_lookup.c */ 
