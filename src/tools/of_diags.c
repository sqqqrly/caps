/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Check and fix order index.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/01/93    |  tjt  Added to mfc.
 * 11/19/94    |  tjt  Add segmented picklines.
 * 06/21/95    |  tjt  Add clear work queue in repair queue.
 * 07/01/95    |  tjt  Add symbolic queue names.
 * 07/21/95    |  tjt  Revise Bard calls.
 *-------------------------------------------------------------------------*/
static char of_diags_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  of_diags.c
 *
 *  Checks structure of order file index.
 *
 *  of_diags     -uvrti
 *
 *  v = verbose output
 *  r = repair all queues
 *  u = sort underway queues
 *  t = dump working tables
 *  i = input to clobber order indices
 */
#include <stdio.h>
#include <signal.h>
#include "Bard.h"
#include "global_types.h"
#include "of.h"

#define NULL            0x01              /* valid null index entry          */
#define BAD             0x02              /* order is marked bad             */
#define HEAD            0x04              /* blink invalid or null           */
#define TAIL            0x08              /* flink invalid or null           */

#define BACK            0x10              /* backward used                   */
#define FWD             0x20              /* forward used                    */
#define VISITED         0x40              /* visited                         */
#define MARKED          0x80              /* visited                         */

#define Max             PicklineMax

unsigned char *flag;                      /* status of index entry           */
long total_valid  = 0;                    /* total useful orders             */
long total_orders = 0;                    /* orders in index                 */
long queue_orders = 0;                    /* orders in queues                */

long          qcount[Max][6] = {0};      /* orders in each queue            */
unsigned char qflag[Max][6]  = {0};      /* errors in each queue            */

char *qname[7] = {"Complete", "Underway",
  "High", "Medium", "Low", "Hold", "Work"};

long vflag = 0;                           /* verbose reporting               */
long rflag = 0;                           /* attempt repair                  */
long uflag = 0;                           /* sort underway queues            */
long tflag = 0;                           /* test mode                       */
long iflag = 0;                           /* input mode                      */

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  register char *p;

  putenv("_=of_diags");
  chdir(getenv("HOME"));

  database_open();

  oc_open();                              /* open index file                 */
  od_open();

  for (k = 0; k < argc; k++)
  {
    p = argv[k];

    if (*p != '-') continue;
    p++;
                
    while (*p)
    {
      if (*p == 'v')          vflag = 1;  /* verbose messages                */
      else if (*p == 'u')     uflag = 1;  /* sort underway queues            */
      else if (*p == 'r')     rflag = 1;  /* repair order file               */
      else if (*p == 't')     tflag = 1;  /* show tables                     */
      else if (*p == 'i')     iflag = 1;  /* input bugs                      */
      else
      {
        printf("*** Unrecognized Parameter %s\n", argv[k]);
        exit(1);
      }
      p++;
    }
  }
  printf("Order Index File Diagnostics\n\n");
        
  flag = (unsigned char *)malloc(oc->of_size);
  if (!flag)
  {
    printf("*** Insufficient Memory To Alloc %d Bytes\n", oc->of_size);
    leave(1);
  }
  memset(flag, 0, oc->of_size);

  lock_order_index();                     /* lock entire order index         */
        
  if (iflag) get_info();

  check_index_entries();
  check_status();
  check_raw_linkages();
  check_queues();
  if (rflag) repair_queues();
  if (uflag) repair_uw();
        
  leave(0);
}
/*-------------------------------------------------------------------------*
 *  Program Termination
 *-------------------------------------------------------------------------*/
leave(ret)
long ret;
{
  oc_unlock();
  od_close();
  oc_close_save();
  database_close();
  exit(ret);
}
/*-------------------------------------------------------------------------*
 *  Repair Underway 
 *-------------------------------------------------------------------------*/
repair_uw()
{
  register struct oi_item *o;
  register struct oc_entry *y, *z;
  register long k, block, found, max;
        
  if (vflag) printf("Phase 6: All uw queues are sorted by entry zone.\n\n");

  printf("Repairing UW Queues\n");

  for (k = 0; k < PicklineMax; k++)       /* for all picklines               */
  {
    y = &oc->oc_tab[k].oc_uw;
    z = &oc->oc_tab[k].oc_work;

    z->oc_first = z->oc_last = z->oc_count = 0;
    z->oc_first = z->oc_last = z->oc_count = 0;

    z = &oc->oc_tab[k].oc_work;

    if (y->oc_count <= 0) continue;       /* nothing underway                */

    block = y->oc_first;
    max = ZoneMax;
    
    while (block)
    {
      o = &oc->oi_tab[block - 1];
      if (o->oi_entry_zone > max) break;
      max = o->oi_entry_zone;
      block = o->oi_flink;
    }
    if (block) 
    {
      printf(" .. Pickline %d Underway Queue Is Out Of Order\n", k + 1);
    }
    else
    {
      printf(" .. Pickline %d Underway Is Valid\n", k + 1);
      continue;
    }
    z->oc_first = 0;
    z->oc_last  = 0;
    z->oc_count = 0;

    while (y->oc_first)
    {
      found = block = y->oc_first;
      max = 0;
      
      while (block)
      {
        o = &oc->oi_tab[block - 1];
        if (o->oi_entry_zone > max)
        {
          max   = o->oi_entry_zone;
          found = block;
        }
        block = o->oi_flink;
      }
      oc_dequeue(block);
      oc_enqueue(block, OC_LAST, OC_WORK);
    }
    y->oc_first = z->oc_first;
    y->oc_last  = z->oc_last;
    y->oc_count = z->oc_count;

    z->oc_first = 0;
    z->oc_last  = 0;
    z->oc_count = 0;
  }
  printf("\n");
}
/*-------------------------------------------------------------------------*
 *  Repair Queues
 *-------------------------------------------------------------------------*/
repair_queues()
{
  register struct oi_item *x;
  register struct oc_entry *y, *z;
  register long i, j, k, m, n, change, goal;
        
  if (vflag) printf("Phase 5: All queues are repaired.\n\n");

  printf("Repairing Queues\n");

  for (k = 0; k < PicklineMax; k++)       /* for all picklines               */
  {
    z = &oc->oc_tab[k].oc_work;
    z->oc_first = z->oc_last = z->oc_count = 0;   /* clear work queue        */

    y = &oc->oc_tab[k].oc_queue[0];

    for (j = 0; j < 6; j++, y++)          /* for all queues                  */
    {
      change = 0;

      if (!qcount[k][j])                  /* no orders for this queue        */
      {
        if (y->oc_count)
        {
          printf(" .. Pickline %d %s Queue Had %d Orders - Now Empty\n",
            k + 1, qname[j], y->oc_count);
        }
        y->oc_count = 0;
        y->oc_first = 0;
        y->oc_last  = 0;
        continue;
      }
      if (y->oc_count != qcount[k][j])    /* changed counts                  */
      {
        printf(" .. Pickline %d %s Queue Had %d Orders Now Has %d Orders\n",
          k + 1, qname[j], y->oc_count, qcount[k][j]);
        change = 1;
      }
      y->oc_count = 0;
/*
 *  Try to use old first chain of queue
 */
      if (y->oc_first)                    /* try to use previous first       */
      {
        m = y->oc_first - 1;
        x = &oc->oi_tab[m];
        if (x->oi_pl == k + 1 && x->oi_queue == j && (flag[m] & HEAD))
        {
          x->oi_blink = 0;

          while (1)
          {
            flag[m] |= MARKED;
            y->oc_count += 1;
            if (flag[m] & TAIL) break;
            m = x->oi_flink - 1;
            if (flag[m] & MARKED) break;
            x = &oc->oi_tab[m];
          }
          x->oi_flink = 0;
          y->oc_last  = m + 1;
        }
        else
        {
          y->oc_first = 0;                /* first no longer valid           */
          y->oc_last  = 0;                /* last no longer valid            */
        }
      }
      else
      {
        y->oc_first = 0;                  /* first no longer valid           */
        y->oc_last  = 0;                  /* last no longer valid            */
      }
/*
 *  Old oc_first failed - try to find previous first chain
 */
      if (!y->oc_first && y->oc_count < qcount[k][j])
      {
        for (m = 0, x = oc->oi_tab; m < oc->of_size; m++, x++)
        {
          if (x->oi_pl != k + 1)  continue;
          if (x->oi_queue != j)   continue;
          if (!(flag[m] & HEAD))  continue;
          if (flag[m] & MARKED)   continue;
          if (x->oi_blink)        continue;    /* likely old first */

          change = 1;

          n = m;

          while (1)
          {
            flag[n] |= MARKED;
            i = oc->oi_tab[n].oi_flink - 1;
            oc_enqueue(n + 1, OC_LAST, j);
            if (flag[n] & TAIL) break;
            if (flag[i] & MARKED) break;
            n = i;
          }
          break;
        }
      }
/*
 *  Append Any others chains to queue
 */
      if (y->oc_count < qcount[k][j])     /* any chains left                 */
      {
        for (m = 0, x = oc->oi_tab; m < oc->of_size; m++, x++)
        {
          if (x->oi_pl != k + 1) continue;
          if (x->oi_queue != j)  continue;
          if (!(flag[m] & HEAD)) continue;
          if (flag[m] & MARKED)  continue;

          change = 1;

          n = m;

          while (1)
          {
            flag[n] |= MARKED;
            i = oc->oi_tab[n].oi_flink - 1;
            oc_enqueue(n + 1, OC_LAST, j);
            if (flag[n] & TAIL) break;
            if (flag[i] & MARKED) break;
            n = i;
          }
        }
      }
/*
 *  Recover all others in input sequence - loops have no effect here.
 */
      if (y->oc_count < qcount[k][j])     /* any orphans left                */
      {
        for (m = 0, x = oc->oi_tab; m < oc->of_size; m++, x++)
        {
          if (x->oi_pl != k + 1) continue;
          if (x->oi_queue != j)  continue;
          if (flag[m] & MARKED)  continue;

          change = 1;

          flag[m] |= MARKED;
          oc_enqueue(m + 1, OC_LAST, j);
        }
      }
      if (y->oc_count != qcount[k][j])
      {
        printf(" .. Pickline %d %s Queue - Repair Failed !!!\n",
        k + 1, qname[j]);
      }
      if (change)
      {
        printf(" .. Pickline %d %s Queue Has Been Repaired.\n",
        k + 1, qname[j]);
        printf("    Sequence of Orders May Have Been Changed.\n");
      }
      else 
      {
        printf(" .. Pickline %d %s Queue Was Valid.\n",
        k + 1, qname[j]);
      }
    }
  }
  printf("\n");
  if (tflag) dump_tables();
}
/*-------------------------------------------------------------------------*
 *  Check Valid Queues
 *-------------------------------------------------------------------------*/
check_queues()
{
  register struct oi_item *x;
  register struct oc_entry *y;
  register long j, k, m, count, first, last, errors;
        
  if (vflag)
  {
    printf("Phase 4: Queues are checked now for valid contents\n");
    printf("         by following the index order chain.  Only\n");
    printf("         queues which have not already been flagged as\n");
    printf("         bad are checked here.\n\n");
  }
  printf("Checking Queues\n");
  errors = 0;
        
  for (k = 0; k < PicklineMax; k++)       /* for all picklines               */
  {
    y = &oc->oc_tab[k].oc_queue[0];

    for (j = 0; j < 6; j++, y++)          /* for all groups                  */
    {
      if (y->oc_count != qcount[k][j])
      {
        printf(" .. Pickline %d %s Queue Had %d Orders Now Has %d Orders\n",
          k + 1, qname[j], y->oc_count, qcount[k][j]);
        errors++;
        continue;
      }
      if (!y->oc_count) continue;
                        
      if (y->oc_first < 1 || y->oc_first > oc->of_size)
      {
        printf(" .. Pickline %d %s Queue Is Lost Completely\n",
          k + 1, qname[j]);
        errors++;
        continue;
      }
      if (qflag[k][j])
      {
        printf(" .. Pickline %d %s Queue Has Error(s)\n",
          k + 1, qname[j]);
        errors++;
        continue;
      }
      count = last = 0;

      first = y->oc_first;
      m = first - 1;
      x = &oc->oi_tab[m];

      while (1)
      {
        if (flag[m] & BAD)              break;
        if (flag[m] & VISITED)  break;
        if (flag[m] & NULL)             break;
                                
        count++;
        last = m + 1;
        flag[m] |= VISITED;
                                
        if (flag[m] & TAIL)             break;
                                
        m = x->oi_flink - 1;
        x = &oc->oi_tab[m];
        if (flag[m] & HEAD)             break;
      }
      if (count != y->oc_count || first != y->oc_first || last != y->oc_last)
      {
        printf(" .. Pickline %d %s Queue Has Error(s)\n",
          k + 1, qname[j]);
        errors++;
      }
    }
  }
  if (errors) printf(" .. %d Errors\n\n", errors);
  else printf(" .. No Errors\n\n");
}
/*-------------------------------------------------------------------------*
 *  Check Raw Linkages
 *-------------------------------------------------------------------------*/
check_raw_linkages()
{
  register struct oi_item *x, *y, *z;
  register long k, m, n, errors, bad;

  if (vflag)
  {
    printf("Phase 3: Index Linkages are checked for logical consistency.\n");
    printf("         Long chains of orders are preserved for later repair.\n");
    printf("         Single unlinked orders are 'orphans'.  This phase\n");
    printf("         breaks all invalid links and may create orphans.\n\n");
  }
  printf("Checking Individual Order Linkages\n");
  errors = 0;
        
  for (x = oc->oi_tab, k = 0; k < oc->of_size; k++, x++)
  {
    if (flag[k] & BAD) continue;

    bad = 0;

    m = n = -1;
    y = z = 0;

    if (x->oi_blink)
    {
      m = x->oi_blink - 1;
      y = &oc->oi_tab[m];
    }
    else flag[k] |= HEAD;

    if (x->oi_flink)
    {
      n = x->oi_flink - 1;
      z = &oc->oi_tab[n];
    }
    else flag[k] |= TAIL;
                
    if (k == m)
    {
      printf(" .. Pickline %d Order %05d Back Linked To Self\n",
        x->oi_pl, x->oi_on);
      y = 0;
      flag[k] |= HEAD;
      bad++;
    }
    if (k == n)
    {
      printf(" .. Pickline %d Order %05d Forward Linked To Self\n",
        x->oi_pl, x->oi_on);
      z = 0;
      flag[k] |= TAIL;
      bad++;
    }
    if (y)
    {
      if (flag[m] & NULL)
      {
        printf(" .. Pickline %d Order %05d Invalid Back Link\n",
          x->oi_pl, x->oi_on);
        flag[k] |= HEAD;
        bad++;
      }
      else if (flag[m] & BAD)
      {
        printf(" .. Pickline %d Order %05d Back Link To Bad Order\n",
          x->oi_pl, x->oi_on);
        flag[k] |= HEAD;
        bad++;
      }
      else if (x->oi_pl != y->oi_pl)
      {
        printf(" .. Pickline %d Order %05 Back Link To Wrong Pickline\n",
        y->oi_pl, y->oi_on, x->oi_pl, x->oi_on);

        flag[k] |= HEAD;
        bad++;
      }
      else if (x->oi_queue != y->oi_queue)
      {
        printf(" .. Pickline %d Order %05d Back Link Wrong Status\n",
        x->oi_pl, x->oi_on);

        flag[k] |= HEAD;
        bad++;
      }
      else if (y->oi_flink != k + 1 || (flag[m] & FWD))
      {
        printf(" .. Pickline %d Order %05d Inconsistent Back Link\n",
        x->oi_pl, x->oi_on);
                                
        if (rflag && !(flag[m] & BACK)) y->oi_flink = k + 1;
        else flag[k] |= HEAD;
        bad++;
      }
      flag[m] |= FWD;
    }
    if (z)
    {
      if (flag[n] & NULL)
      {
        printf(" .. Pickline %d Order %05d Invalid Forward Link\n",
        x->oi_pl, x->oi_on);
        flag[k] |= TAIL;
        bad++;
      }
      else if (flag[n] & BAD)
      {
        printf(" .. Pickline %d Order %05d Forward Link To Bad Order\n",
        x->oi_pl, x->oi_on);
        flag[k] |= TAIL;
        bad++;
      }
      else if (x->oi_pl != z->oi_pl)
      {
        printf(" .. Pickline %d Order %05 Forward Link Wrong Pickline\n",
        x->oi_pl, x->oi_on, z->oi_pl, z->oi_on);

        flag[k] |= TAIL;
        bad++;
      }
      else if (x->oi_queue != z->oi_queue)
      {
        printf(" .. Pickline %d Order %05d Forward Link Wrong Status\n",
        x->oi_pl, x->oi_on);

        flag[k] |= TAIL;
        bad++;
      }
      else if (z->oi_blink != k + 1 || (flag[n] & BACK))
      {
        printf(" .. Pickline %d Order %05d Inconsistent Forward Link\n",
        x->oi_pl, x->oi_on);
                                
        if (rflag && !(flag[n] & BACK)) z->oi_blink = k + 1;
        else flag[k] |= TAIL;
                                
        bad++;
      }
      flag[n] |= BACK;
    }
    if (bad)
    {
      qflag[x->oi_pl - 1][x->oi_queue] = 1;
      errors++;
    }
  }
  if (errors) printf(" .. %d Errors\n\n", errors);
  else printf(" .. No Errors\n\n");

  if (tflag) dump_tables();
        
}
/*-------------------------------------------------------------------------*
 *  Fetch Order Status
 *-------------------------------------------------------------------------*/
check_status()
{
  register struct oi_item *x;
  register long k, errors, queue;
        
  if (vflag)
  {
    printf("Phase 2: Order index and order records are checked for\n");
    printf("         consistent pickline, order number, and valid\n");
    printf("         order status.  Invalid orders are not repaired.\n\n");
  }
  printf("Checking Order Status Codes\n");

  errors = 0;

  order_setkey(0);

  while (!order_next(of_rec, NOLOCK))
  {
    switch(of_rec->of_status)
    {
      case 'h': queue = OC_HOLD; break;

      case 'x':
      case 'c': queue = OC_COMPLETE; break;
                        
      case 'u': queue = OC_UW; break;
                        
      case 'q': switch (of_rec->of_pri)
                {
                  case 'h': queue = OC_HIGH; break;
                  case 'm': queue = OC_MED; break;
                  case 'l': queue = OC_LOW; break;
                }
                break;
                
      default: queue = OC_COMPLETE; break;
    }
    k = oc_find(of_rec->of_pl, of_rec->of_on) - 1;
    if (k < 0)
    {
      printf(" .. Pickline %02d Order %05d - Not Indexed\n",
        of_rec->of_pl, of_rec->of_on);
      errors++;

      if (rflag)
      {
        k = oc_write(of_rec->of_pl, of_rec->of_on) - 1;
        oc->oi_tab[k].oi_queue = queue;
        flag[k] |= BAD | MARKED;
      }
    }
    else if (queue != oc->oi_tab[k].oi_queue)
    {
      printf(" .. Pickline %02d Order %05d - Bad Status %c\n",
      of_rec->of_pl, of_rec->of_on, of_rec->of_status);

      if (rflag) oc->oi_tab[k].oi_queue = queue;

      flag[k] |= BAD | MARKED;
      errors++;
      total_valid--;
    }
    else flag[k] |= MARKED;

    qcount[of_rec->of_pl - 1][queue] += 1;  /* count in database      */
  }
  for (x = oc->oi_tab, k = 0; k < oc->of_size; k++, x++)
  {
    if (flag[k] & (NULL | BAD | MARKED))
    {
      flag[k] &= ~MARKED;
      continue;
    }
    printf(" .. Pickline %02d Order %05d - Indexed - No Record\n",
      x->oi_pl, x->oi_on);

    errors++;
    total_valid--;
    if (rflag) 
    {
      flag[k] = NULL;
      memset(x, 0, sizeof(struct oi_item));
    }
  }
  if (errors) printf(" .. %d Status Errors\n\n", errors);
  else printf(" .. No Errors\n\n");

  if (tflag) dump_tables();
}
/*-------------------------------------------------------------------------*
 *  Check Valid Order Block Sizes
 *
 *  Flag becomes either NULL, BAD, or zero.
 *-------------------------------------------------------------------------*/
check_index_entries()
{
  register struct oi_item *x;
  register long k, errors;

  if (vflag)
  {
    printf("Phase 1: Orders valid pickline range, and valid forward\n");
    printf("         backward link value range.\n");
    printf("         Invalid links may be repaired.\n\n");
  }
  printf("Checking Index Occupancy\n");

  errors = 0;

  for (x = oc->oi_tab, k = 0; k < oc->of_size; k++, x++)
  {
    if (!x->oi_on)                        /* no order number                 */
    {
      flag[k] |= NULL;
      continue;
    }
    if (x->oi_pl > PicklineMax && x->oi_pl <= PicklineMax+SegmentMax) 
    {
      flag[k] = NULL | BAD;
      continue;
    }
    total_orders++;                       /* count orders                    */

    if (x->oi_pl < 1 || x->oi_pl > PicklineMax)
    {
      printf(" .. Pickline %d Invalid For Order %05d\n", x->oi_pl, x->oi_on);
      flag[k] |= BAD;
      continue;
    }
    if (x->oi_blink > oc->of_size)
    {
      printf(" .. Pickline %d Order %05d - Back Link Out of Range\n",
      x->oi_pl, x->oi_on);
      if (rflag) x->oi_blink = 0;
      else flag[k] |= BAD;
    }
    if (x->oi_flink > oc->of_size)
    {
      printf(" .. Pickline %d Order %05d - Forward Link Out of Range\n",
      x->oi_pl, x->oi_on);
      if (rflag) x->oi_flink = 0;
      else flag[k] |= BAD;
    }
    if (flag[k] & BAD) errors++;
    else total_valid++;
  }
  if (vflag) printf("%d Orders In Index\n", total_orders);
  if (errors)     printf(" .. %d Errors\n\n", errors);
  else printf(" .. No Errors\n\n");

  if (tflag) dump_tables();
}
/*-------------------------------------------------------------------------*
 *  Attempt To Lock Order Index
 *-------------------------------------------------------------------------*/
lock_order_index()
{
  extern timeout();
  register long count, when;
        
  printf("Locking Entire Order Index\n");

  signal(SIGALRM, timeout);
  alarm(15);                              /* timeout in 15 seconds           */

  oc_lock();

  alarm(0);                               /* clear timeout                   */
  printf("Order File Is Now Locked!\n\n");
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Locking Alarm Timeout
 *-------------------------------------------------------------------------*/
timeout()
{
  printf("*** No Unlock In 15 Seconds\n\n");
  leave(1);
}
/*-------------------------------------------------------------------------*
 *  Dump Tables
 *-------------------------------------------------------------------------*/
dump_tables()
{
  register struct oi_item *x;
  register long k;
        
  printf("Current Tables\n");

  for (k = 0, x = oc->oi_tab; k < oc->of_size; k++, x++)
  {
    if (!x->oi_on) continue;
                
    printf("%4d: pl: %2d order: %05d blink: %4d flink: %4d",
    k + 1, x->oi_pl, x->oi_on, x->oi_blink, x->oi_flink);
                        
    printf(" queue: %d", x->oi_queue);
    if (flag[k] & NULL)   printf(" NULL");
    if (flag[k] & BAD)    printf(" BAD");
    if (flag[k] & HEAD)   printf(" HEAD");
    if (flag[k] & TAIL)   printf(" TAIL");
    if (flag[k] & BACK)   printf(" BACK");
    if (flag[k] & FWD)    printf(" FWD");
    if (flag[k] & MARKED) printf(" MARK");
    printf("\n");
  }
}
/*-------------------------------------------------------------------------*
 *  Test Entry  
 *-------------------------------------------------------------------------*/
get_info()
{
  register struct oi_item *x;
  register struct oc_entry *y;
  char ans[16];
  long j, k, m, n;
        
  while (1)
  {
    printf("Modify Pickline Control (y/n)? --> ");
    gets(ans);
    if (*ans != 'y') break;

    printf("Enter Pickline     --> ");
    gets(ans);
    sscanf(ans, "%d", &j);

    y = &oc->oc_tab[j - 1].oc_queue[0];

    printf("Enter Queue (1-6)  --> ");
    gets(ans);
    sscanf(ans, "%d", &k);
                
    y += (k - 1);

    printf("Enter New First    --> ");
    gets(ans);
    sscanf(ans, "%d", &m);
                
    y->oc_first = m;
  }
  while (1)
  {
    printf("Modify Order File (y/n)? --> ");
    gets(ans);
    if (*ans != 'y') return;

    printf("Enter Pickline     --> ");
    gets(ans);
    sscanf(ans, "%d", &j);

    printf("Enter Order Number --> ");
    gets(ans);
    sscanf(ans, "%d", &k);

    m = oc_find(j, k);
    if (!m)
    {
      printf("*** Order Not Found\n\n");
      continue;
    }
    x = &oc->oi_tab[m - 1];
                
    printf("Enter Back Link (%d)    --> ", x->oi_blink);
    gets(ans);
    if (*ans)
    {
      sscanf(ans, "%d", &n);
      x->oi_blink = n;
    }
    printf("Enter Forward Link (%d) --> ", x->oi_flink);
    gets(ans);
    if (*ans)
    {
      sscanf(ans, "%d", &n);
      x->oi_flink = n;
    }
    begin_work();
    od_get(m);
    printf("Enter Status (%c)       --> ", of_rec->of_status);
    gets(ans);
    if (*ans)
    {
      of_rec->of_status = *ans;
    }
    printf("Enter Priority (%c)     --> ", of_rec->of_pri);
    gets(ans);
    if (*ans)
    {
      of_rec->of_pri = *ans;
    }
    od_update(m);
    commit_work();
  }
}

/* end of of_diags.c */
