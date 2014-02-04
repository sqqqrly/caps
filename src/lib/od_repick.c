/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Setup picks and index for repicking.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/18/93   |  tjt  Original implementation.
 *  09/07/04   |  tjt  Add picks to go to pickline item.
 *  06/30/95   |  tjt  Add sku lookup.
 *  07/01/95   |  tjt  Add remaining picks always.
 *  07/02/95   |  tjt  Add flag orders with orphans.
 *  07/02/95   |  tjt  Add flag orders with inhibitted picks.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  12/02/96   |  tjt  Add units and lines to location.
 *-------------------------------------------------------------------------*/
static char od_repick_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "of.h"
#include "co.h"
#include "st.h"
#include "Bard.h"

od_repick(block, lookup)
register long block, lookup;
{
  register struct oi_item   *o;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register struct pl_item   *p;
  register struct st_item   *s;
  register long k;
  
  struct of_pick_item split;

  if (sp->sp_sku_support != 'y') lookup = 0;

  if (block < 1 || block > oc->of_size)
  {
    krash("od_repick", "invalid block", 1);
  }
  od_get(block);                         /* get order header record          */

  p = &pl[of_rec->of_pl - 1];            /* point to pickline                */

  of_rec->of_repick   = 'y';             /* clear various fields             */
  of_rec->of_picker   = 0;
  of_rec->of_elapsed  = 0;
  of_rec->of_datetime = time(0);
  
  od_update(block);                       /* update order header             */

  o = &oc->oi_tab[block - 1];

  o->oi_flags = 0;                        /* clear all index flags           */
  
  if (of_rec->of_no_picks < 1)            /* null order                      */
  {
    o->oi_entry_zone = p->pl_first_zone;
    o->oi_exit_zone  = p->pl_first_zone;
    return 0;
  }
  o->oi_entry_zone = ZoneMax;             /* clear entry/exit zones          */
  o->oi_exit_zone  = 0;

  op_rec->pi_pl  = o->oi_pl;
  op_rec->pi_on  = o->oi_on;
  op_rec->pi_mod = 0;
  pick_startkey(op_rec);
 
  op_rec->pi_mod = ProductMax;
  pick_stopkey(op_rec);
  
  split.pi_mod = 0;

  while (!pick_next(op_rec, LOCK))
  {
    if (op_rec->pi_flags & NO_PICK) continue;

    op_rec->pi_flags   &= ~VALIDATED;      /* initially mark invalid         */
    op_rec->pi_datetime = 0;               /* clear pick time                */
    
    if (lookup)
    {
      s = sku_lookup(op_rec->pi_pl, op_rec->pi_sku);
      if (s) op_rec->pi_mod = s->st_mod;
      else                                /* sku was not found              */
      {
        o->oi_flags |= ORPHANS;           /* mark has orphans               */
        op_rec->pi_mod  = 0;              /* clear module to zero           */
        op_rec->pi_zone = 0;
        pick_update(op_rec);
        continue;            
      }
    }
    if (op_rec->pi_mod >= 1 && op_rec->pi_mod <= coh->co_prod_cnt)
    {
      i = &pw[op_rec->pi_mod - 1];
      k = i->pw_ptr;
      
      if (k > 0)
      {
        h = &hw[k - 1];
        if (h->hw_bay > 0)                /* module is unassigned            */
        {
          b = &bay[h->hw_bay - 1];
          if (b->bay_zone > 0)            /* bay is unassigned               */
          {
            z = &zone[b->bay_zone - 1];
            if (z->zt_pl == op_rec->pi_pl) 
            {
              op_rec->pi_flags |= VALIDATED;
              op_rec->pi_zone   = z->zt_zone;
            }
          }
        }
      }
    }
    if (!(op_rec->pi_flags & VALIDATED))  /* module not defined              */
    {
      o->oi_flags |= ORPHANS;             /* mark order has orphans          */
      op_rec->pi_flags &= ~PICKED;        /* clear picked flag               */
      pick_update(op_rec);
      continue;
    }
    if (i->pw_flags & PicksInhibited) o->oi_flags |= INHIBITED;

    p->pl_units_to_go += op_rec->pi_ordered;
    i->pw_units_to_go += op_rec->pi_ordered;

    if (split.pi_mod)                      /* current split module           */
    {
      if (op_rec->pi_mod == split.pi_mod)  /* accumulate picks               */
      {
        split.pi_ordered += op_rec->pi_ordered;
        pick_delete();
        continue;
      }
      pick_write(&split);                  /* add accumulated module         */
      split.pi_mod = 0;                    /* mark end of splits             */
      continue;
    }
    p->pl_lines_to_go += 1;
    i->pw_lines_to_go += 1;

    if (z->zt_zone < o->oi_entry_zone) o->oi_entry_zone = z->zt_zone;
    if (z->zt_zone > o->oi_exit_zone)  o->oi_exit_zone  = z->zt_zone;
  
    op_rec->pi_picked     = 0;              /* clear picked data             */
    op_rec->pi_box_number = 0;

    if (op_rec->pi_flags & SPLIT_PICK)      /* save to accumulate splits     */
    {
      op_rec->pi_flags &= ~(PICKED | SPLIT_PICK);
      memcpy(&split, op_rec, sizeof(struct of_pick_item));
      pick_delete();
      continue;
    }
    op_rec->pi_flags &= ~PICKED;
    op_rec->pi_flags |= REPICK;
    
    pick_update(op_rec);
  }
  if (o->oi_exit_zone < o->oi_entry_zone) o->oi_exit_zone = o->oi_entry_zone;

  if (split.pi_mod) pick_write(&split);

  return 0;
}

/* end of od_repick.c */
