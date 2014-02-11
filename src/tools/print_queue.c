/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Print Order Queues
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/06/94   |  tjt  Original implementation.
 *  07/21/95   |  tjt  Revise Bard calls.
 *-------------------------------------------------------------------------*/
static char print_queue_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

#include "co.h"
#include "of.h"
#include "Bard.h"

main(argc, argv)
long argc;
char **argv;
{
  register long pl, num, block, low_mod, hi_mod, low_zone, hi_zone;
  register struct oi_item *o;
  register struct oc_entry *q;
  register struct hw_item *h;
  register struct bay_item *b;
  char work[8];
  
  putenv("_=print_queue");
  chdir(getenv("HOME"));

  database_open();

  printf("Print Order Queue Information\n\n");
  printf("Queues:\n");
  printf("  0 = Hold\n");
  printf("  1 = Complete\n");
  printf("  2 = Underway\n");
  printf("  3 = High\n");
  printf("  4 = Medium\n");
  printf("  5 = Low\n\n");
  
  printf("Enter Pickline --> ");
  gets(work); pl = atol(work);
  printf("Enter Queue    --> ");
  gets(work); num = atol(work);

  co_open();
  oc_open();
  od_open();

  q = &oc->oc_tab[pl - 1].oc_queue[num];
  
  block = q->oc_first;
  
  fprintf(stderr, "Block PL  Order  Grp  Entry Exit  First Zone Last  Zone\n");
  fprintf(stderr, "----- -- ------- ---- ----- ----- ----- ---- ----- ----\n");

  while (block)
  {
    o = &oc->oi_tab[block - 1];
  
    op_rec->pi_pl  = o->oi_pl; 
    op_rec->pi_on  = o->oi_on;
    op_rec->pi_mod = 1;

    pick_startkey(op_rec);
    
    op_rec->pi_mod = 9999;
    pick_stopkey(op_rec);
    
    low_mod = 9999;
    hi_mod  = 0;
    
    while (!pick_next(op_rec, NOLOCK))
    {
      if (op_rec->pi_mod < low_mod) low_mod = op_rec->pi_mod;
      if (op_rec->pi_mod > hi_mod)  hi_mod  = op_rec->pi_mod;
    }
    h = &hw[mh[low_mod - 1].mh_ptr - 1];
    b = &bay[h->hw_bay - 1];
    low_zone = b->bay_zone;
    
    h = &hw[mh[hi_mod - 1].mh_ptr - 1];
    b = &bay[h->hw_bay - 1];
    hi_zone = b->bay_zone;
    
    fprintf(stderr, "%5d %2d %7.5d %4.4s %5d %5d%c%5d %4d %5d %4d\n",
      block, o->oi_pl, o->oi_on, o->oi_grp,
      o->oi_entry_zone,
      o->oi_exit_zone,  (o->oi_exit_zone < hi_zone) ? '*' : 0x20,
      low_mod, low_zone, hi_mod, hi_zone);
  
    block = o->oi_flink;

  }
  co_close();
  oc_close();
  od_close();
  database_close();
  exit(0);
}

/* end of print_queue.c */
