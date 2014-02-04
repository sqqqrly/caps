/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Delete order header, picks, and remarks.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/09/93   |  tjt  Original implementation.
 *  09/07/94   |  tjt  Add picks to go to pickline item.
 *  04/20/95   |  tjt  Removed od_get() call.
 *  07/21/95   |  tjt  Revised Bard calls.
 *  04/16/96   |  tjt  Revised hw and pw tables.
 *  12/02/96   |  tjt  Add units and lines to location.
 *-------------------------------------------------------------------------*/
static char od_delete_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "of.h"
#include "co.h"
#include "Bard.h"

od_delete()
{
  register struct pl_item *p;
  register struct hw_item *h;
  register struct pw_item *i;
  
  p = &pl[of_rec->of_pl - 1];             /* point to pickline record        */

  if (or_rec)                             /* delete remarks                  */
  {
    or_rec->rmks_pl = of_rec->of_pl;
    or_rec->rmks_on = of_rec->of_on;
    if (!remarks_read(or_rec, LOCK)) remarks_delete();
  }
  op_rec->pi_pl  = of_rec->of_pl;
  op_rec->pi_on  = of_rec->of_on;
  op_rec->pi_mod = 0;
  pick_startkey(op_rec);
   
  op_rec->pi_mod = ProductMax;
  pick_stopkey(op_rec);
   
  if (of_rec->of_status != 'c' && of_rec->of_status != 'x')
  {
    while (!pick_next(op_rec, LOCK))
    {
      if (op_rec->pi_flags & (NO_PICK | PICKED))
      {
        i = &pw[op_rec->pi_mod - 1];
        h = &hw[i->pw_ptr - 1];
        
        i->pw_lines_to_go -= 1;
        i->pw_units_to_go -= op_rec->pi_ordered;

        p->pl_lines_to_go -= 1;
        p->pl_units_to_go -= op_rec->pi_ordered;
      }
      pick_delete();
    }
  }
  else
  {
    while (!pick_next(op_rec, LOCK)) pick_delete();
  }
  order_delete();
  
  return 0;
}

/* end of od_delete.c */
