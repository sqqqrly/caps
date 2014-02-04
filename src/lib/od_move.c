/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Reconfigure orders and picks to another pickline.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/25/95   |  tjt  Original implementation from od_config.
 *  05/25/96   |  tjt  Add pi_zone value.
 *  12/02/96   |  tjt  Add units and lines to location.
 *-------------------------------------------------------------------------*/
static char od_move_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"
#include "co.h"
#include "ss.h"
#include "st.h"
#include "Bard.h"

od_move(block, new_pickline)
register long block, new_pickline;
{
  register struct oi_item   *o;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register struct pl_item   *p, *q;
  register struct st_item   *s;
  register long k, offset;
  
  if (block < 1 || block > oc->of_size) 
  {
    krash("od_config", "invalid block", 1);
  }
  o = &oc->oi_tab[block - 1];
  p = &pl[o->oi_pl - 1];                     /* point to pickline            */
  q = &pl[new_pickline - 1];                 /* new pickline                 */
  
  z = &zone[p->pl_first_zone - 1];
  b = &bay[z->zt_first_bay - 1];
  
  offset = b->bay_prod_first;

  z = &zone[pl[new_pickline - 1].pl_first_zone - 1];
  b = &bay[z->zt_first_bay - 1];
  
  offset -= b->bay_prod_first;               /* offset to new pickline       */

  o->oi_flags = 0;                           /* clear flags                  */

  o->oi_entry_zone = ZoneMax;                /* extreme values               */
  o->oi_exit_zone  = 0;

  /*
  op_rec = (struct of_pick_item *)malloc(oc->op_rec_size);
  if (!op_rec) krash("od_move", "malloc op_rec", 1);
 */

  op_rec->pi_pl  = o->oi_pl;                 /* setup key ranges             */
  op_rec->pi_on  = o->oi_on;
  op_rec->pi_mod = 0;                        /* also gets orphans            */
  pick_startkey(op_rec);
 
  op_rec->pi_mod = ProductMax;
  pick_stopkey(op_rec);
  
  while (!pick_next(op_rec, LOCK))          /* find all picks for order      */
  {
    if (op_rec->pi_flags & PICKED)  continue;
    if (op_rec->pi_flags & NO_PICK) continue;
    
    if (op_rec->pi_mod > 0)                 /* clear remaining picks         */
    {
      p->pl_units_to_go -= op_rec->pi_ordered;
      p->pl_lines_to_go -= 1;
      
      i = &pw[op_rec->pi_mod - 1];

      i->pw_units_to_go -= op_rec->pi_ordered;
      i->pw_lines_to_go -= 1;
    }
    op_rec->pi_pl = new_pickline;           /* change pickline               */

    op_rec->pi_flags = 0;                   /* initially mark invalid        */
    op_rec->pi_zone  = 0;                   /* clear zone assignement        */
    
    if (sp->sp_sku_support == 'y')          /* lookup only with sku's        */
    {
      s = sku_lookup(op_rec->pi_pl, op_rec->pi_sku);

      if (s) op_rec->pi_mod = s->st_mod;    /* now have a good module        */
      else                                  /* sku is undefined              */
      {
        o->oi_flags |= ORPHANS;             /* flag order has orphans        */
        op_rec->pi_mod = 0;                 /* invalid module                */
        pick_update(op_rec);                /* not validated                 */
        continue;                           /* cannot use this pick          */
      }
    }
    else
    {
      op_rec->pi_mod -= offset;             /* move module                   */
    }
    if (op_rec->pi_mod >= 1 && op_rec->pi_mod <= coh->co_prod_cnt)
    {
      i = &pw[op_rec->pi_mod - 1];
      k = i->pw_ptr;

      if (k > 0)
      {
        h = &hw[k - 1];
        if (h->hw_bay > 0)
        {
          b = &bay[h->hw_bay - 1];
          if (b->bay_zone > 0)
          {
            z = &zone[b->bay_zone - 1];
            if (z->zt_pl == op_rec->pi_pl) op_rec->pi_flags |= VALIDATED;
          }
        }
      }
    }
    if (!(op_rec->pi_flags & VALIDATED))
    {
      o->oi_flags   |= ORPHANS;
      op_rec->pi_mod = 0;
      pick_update(op_rec);   
      continue;
    }
    if (i->pw_flags & PicksInhibited) o->oi_flags |= INHIBITED;

    i->pw_lines_to_go += 1;
    i->pw_units_to_go += op_rec->pi_ordered;
    
    q->pl_lines_to_go += 1;
    q->pl_units_to_go += op_rec->pi_ordered;

    if (z->zt_zone < o->oi_entry_zone) o->oi_entry_zone = z->zt_zone;
    if (z->zt_zone > o->oi_exit_zone)  o->oi_exit_zone  = z->zt_zone;

    op_rec->pi_zone = z->zt_zone;           /* assign pick to zone           */

    pick_update(op_rec);
  }
  pick_unlock();                            /* unlock in case                */

  o->oi_pl = new_pickline;                  /* update pickline               */

  if(o->oi_entry_zone < q->pl_first_zone || o->oi_entry_zone > q->pl_last_zone)
  {
    o->oi_entry_zone = q->pl_first_zone;
  }
  if (o->oi_entry_zone > o->oi_exit_zone) o->oi_exit_zone = o->oi_entry_zone;
  if (o->oi_exit_zone  > q->pl_last_zone) o->oi_exit_zone = q->pl_last_zone;
  
  if (o->oi_flags & ORPHANS) return 1;
/*
  if (op_rec) free(op_rec);
  op_rec = 0;
*/

  return 0;
}

/* end of od_move.c */
