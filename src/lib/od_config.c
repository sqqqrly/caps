/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Reconfigure orders and picks.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/18/93   |  tjt  Original implementation.
 *  09/07/94   |  tjt  Add picks to go to pickline item;
 *  06/30/95   |  tjt  Add sku lookup option.
 *  07/01/95   |  tjt  Add accumulate remaining option.
 *  07/02/95   |  tjt  Add check module range.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  07/31/95   |  tjt  Add MOVED flag.
 *  04/16/96   |  tjt  Revise hw an pw tables.
 *  12/02/96   |  tjt  Add units and lines to location.
 *-------------------------------------------------------------------------*/
static char od_config_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "of.h"
#include "co.h"
#include "ss.h"
#include "st.h"
#include "Bard.h"
#include "oracle_defines.h"

od_config(block, lookup, accum)
register long block, lookup, accum;
{
  register struct oi_item   *o;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register struct pl_item   *p;
  register struct st_item   *s;
  register long k, orphan, old_flag, new_flag;
  
  if (sp->sp_sku_support != 'y') lookup = 0; /* lookup only with sku's       */
  if (sp->sp_remaining_picks == 'n') accum  = 0; /* no remaining picks       */
  orphan = 0;                                /* clear flags                  */

  if (block < 1 || block > oc->of_size)
  {
    krash("od_config", "invalid block", 1);
  }
  o = &oc->oi_tab[block - 1];
  p = &pl[o->oi_pl - 1];                     /* point to pickline            */

  o->oi_flags &= ~(ORPHANS | INHIBITED | MOVED); /* clear flags              */

  o->oi_entry_zone = ZoneMax;                /* extreme values               */
  o->oi_exit_zone  = 0;
  
  pick_setkey(1);
  
  op_rec->pi_pl  = o->oi_pl;                 /* setup key ranges             */
  op_rec->pi_on  = o->oi_on;
  op_rec->pi_mod = 0;
  pick_startkey(op_rec);
 
  op_rec->pi_mod = ProductMax;
  pick_stopkey(op_rec);
  
  while (!pick_next(op_rec, LOCK))          /* find all picks for order      */
  {
    if (op_rec->pi_flags & PICKED)   continue;
    if (op_rec->pi_flags & NO_PICK)  continue;
    if (op_rec->pi_flags & OBSOLETE) continue;
    
    old_flag = op_rec->pi_flags & VALIDATED;/* save old flag                 */
    new_flag = 0;                           /* clear new flag                */
    
    op_rec->pi_flags &= ~VALIDATED;         /* initially mark invalid        */
    op_rec->pi_flags &= ~MOVED;             /* intiially mark not moved      */
    
    if (lookup)                             /* sku table has changed         */
    {
      s = sku_lookup(op_rec->pi_pl, op_rec->pi_sku);

      if (s)                                /* sku valid in pickline         */
      {
        if (op_rec->pi_mod != s->st_mod)    /* module has moved - flag OK    */
        {
          if (op_rec->pi_mod && op_rec->pi_mod != s->st_mod) 
          {
            o->oi_flags      |= MOVED;      /* mark order has moved picks    */
            op_rec->pi_flags |= MOVED;      /* mark this pick has moved      */
          }
          op_rec->pi_mod  = s->st_mod;      /* now have a good module        */
        }
      }  
      else                                  /* sku is undefined              */
      {
        o->oi_flags |= ORPHANS;             /* flag order has orphans        */
        orphan = 1;

        if (old_flag == VALIDATED)          /* was OK - now BAD              */
        {
          op_rec->pi_mod  = 0;              /* invalid module                */
          op_rec->pi_zone = 0;
          pick_update(op_rec);
        }
        continue;                           /* cannot use this pick          */
      }
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
            if (z->zt_pl == op_rec->pi_pl)
            {
              new_flag          = VALIDATED;
              op_rec->pi_flags |= VALIDATED;
              
              if (op_rec->pi_zone != z->zt_zone)
              {
                op_rec->pi_zone   = z->zt_zone;
                old_flag = -1;
              }
            }
          }
        }
      }
    }
    if (old_flag != new_flag) pick_update(op_rec);   

    if (new_flag != VALIDATED)
    {
      o->oi_flags |= ORPHANS;               /* flag orders has orphans       */
      orphan = 1;
      continue;
    }
    if (i->pw_flags & PicksInhibited) o->oi_flags |= INHIBITED;

    if (accum)
    {
      i->pw_lines_to_go += 1;
      i->pw_units_to_go += op_rec->pi_ordered;
    
      p->pl_lines_to_go += 1;
      p->pl_units_to_go += op_rec->pi_ordered;
    }
    if (z->zt_zone < o->oi_entry_zone) o->oi_entry_zone = z->zt_zone;
    if (z->zt_zone > o->oi_exit_zone)  o->oi_exit_zone  = z->zt_zone;
  }
  if(o->oi_entry_zone < p->pl_first_zone || o->oi_entry_zone > p->pl_last_zone)
  {
    o->oi_entry_zone = p->pl_first_zone;
  }
  if (o->oi_entry_zone > o->oi_exit_zone) o->oi_exit_zone = o->oi_entry_zone;
  if (o->oi_exit_zone  > p->pl_last_zone) o->oi_exit_zone = p->pl_last_zone;
  
  pick_unlock();

  return orphan;
}

/* end of od_config.c */
