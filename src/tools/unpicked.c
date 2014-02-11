/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Show Unpicked Items.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/24/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char unpicked_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"

#include "Bard.h"
#include "bard/picks.h"

long fd;
picks_item x;

main()
{
  long block;
  
  putenv("_=unpicked");
  chdir(getenv("HOME"));

  oc_open();

  pick_open(READONLY);
  pick_setkey(1);
  
  while (!pick_next(&x, NOLOCK))
  {
    if (x.p_pi_flags & PICKED) continue;
    
    block = oc_find(x.p_pi_pl, x.p_pi_on);
    
    if (block < 1) continue;
    
    if (oc->oi_tab[block - 1].oi_queue != OC_COMPLETE) continue;
  
    printf("pl=%2d  Order: %5d  Mod: %d\n",
      x.p_pi_pl, x.p_pi_on, x.p_pi_module);
  
  }
  pick_close();
  oc_close();
}

/* end of unpicked.c */


