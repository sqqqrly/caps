/* #define DEBUG */
/* #define DEBUG2 */
#define DEBUG3
#define DEBUG4
#define DALLAS
/* #define CONROE */
#define SHIV
#define TOTE_SCAN
/*-------------------------------------------------------------------------*
 *  Custom Code:    DALLAS - Order is in all configured picklines.
 *                           All manifests for an order printed only
 *                           when all picklines are completed.
 *                  CONROE - Order is in 2 picklines. Each printed when
 *                           the order is complete in the pickline.
 *                  SHIV   - Prevents duplicate manifests from printing by 
 *                           setting TOTE_LABEL flag on each order printed.
 *               TOTE_SCAN - Used if scanning tote labels.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Packing list printer daemon.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/27/93    |  tjt  Added to mfc.
 * 08/09/94    |  tjt  Added putenv and chdir.
 * 11/26/94    |  tjt  Added order and print count.
 * 02/02/95    |  tjt  Remove UNOS queues.
 * 03/07/95    |  tjt  Fix printer name usage.
 * 07/22/95    |  tjt  Special for eckerd.
 * 11/11/96    |  tjt  Modified for both Dallas and Houston.
 * 09/29/97    |  phl  Modified for ModIII 14 Picklines
 * 12/17/99    |  aha  Turned off #define DEBUG
 * 10/08/01    |  aha  Modified for Eckerd's Tote Integrity.
 * 02/27/03    |  aha  Added delay to ensure picks are all updated.
 *-------------------------------------------------------------------------*/
static char tote_packing_list_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/*-------------------------------------------------------------------------*
 *  Prints Packing Lists
 *
 *  To Execute:              packing_list <printer>  &
 *
 *  To Test Report Layout:   packing_list -d [-copies=n] [-picks=m] [-value=x]
 *                              
 *                             -d indicates report debug mode
 *                             -copies=n indicates 'n' copies
 *                             -picks=m  indicates 'm' picks
 *                             -value=x  indicates 'x' as alpha fill byte
 *-------------------------------------------------------------------------*/
#include <stdio.h>
#include <signal.h>
#include "file_names.h"
#include "ss.h"
#include "of.h"
#include "co.h"
#include "st.h"
#include "box.h"

#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"

#define DELAY              15
#define CLENGTH            12
#define MAX_ORDERS         256

char HOF = 0x0c;
/*------------------------------------------------------------------------*
 *  Specialized Header Files For This Packing List
 *------------------------------------------------------------------------*/

#include "remarks_text.h"
#include "pick_text.h"

/*------------------------------------------------------------------------*
 *  Input Queue Of Print Requests
 *------------------------------------------------------------------------*/

paper_item qi;

long L_OrderNo[MAX_ORDERS][2],
     k1,
     k2,
     block1;
struct oi_item *o;
int L_Count = 0;

#ifdef SHIV
long block2;
#endif

/*-------------------------------------------------------------------------*
 *  Pick Module And Product File Database Files
 *-------------------------------------------------------------------------*/

prodfile_item  sku_rec;                   /* SKU record definition           */
pmfile_item    pkm_rec;                   /* Pick module record definition   */

/*-------------------------------------------------------------------------*
 *  Various Size Letters For Printing 
 *-------------------------------------------------------------------------*/

typedef struct
{
  short letter_size;                      /* letter height  = 1, 5, 7, 9     */
  short letter_width;                     /* letter width   = 1, 5, 5, 7     */
  char  *letter_image;                    /* letter image table              */

} letter_item;

letter_item *letter1 = 0;                 /* 1x1 letters                     */
letter_item *letter5 = 0;                 /* 5x5 letters                     */
letter_item *letter7 = 0;                 /* 7x5 letters                     */
letter_item *letter9 = 0;                 /* 9x7 letters                     */
letter_item *current_letter = 0;          /* current letter in use           */

/*-------------------------------------------------------------------------*
 *  Global Print Variables
 *-------------------------------------------------------------------------*/

long print_flag;                          /* print / no print flag           */
long base_row;                            /* row offset in report            */
long base_col;                            /* col offset in report            */
long copies;                              /* number of copies to print       */
long page;                                /* current page number             */
long last_page;                           /* total pages in one copy         */
long picks_to_go;                         /* picks in the order              */
long base_order;                          /* base number xx?xxx              */
char *image;                              /* print format area               */

long short_picks;                         /* short count in order            */
long short_page;                          /* number of short pages           */
long last_short_page;                     /* last short page                 */

/*-------------------------------------------------------------------------*
 *  Order File Pointers
 *-------------------------------------------------------------------------*/

long block;                               /* order block number              */
long pick_no;                             /* current pick subscript 0 ..     */
long line;                                /* current line in repeat 0 ..     */
remarks_text_item    *rmks;               /* order remarks                   */
pick_text_item       *ptext;              /* pick text                       */
struct of_pick_info  *pick;               /* pointer to current pick         */

struct of_pick_item  *tab;                /* pointer to picks table          */

typedef struct 
{
  char txt[25];
} desc_item;

desc_item            *desc;               /* pointer to descriptions       */

#ifdef TOTE_SCAN
#define BOX_ARRAY_SIZE    100
long box_num[BOX_ARRAY_SIZE];             /* box numbers for one order       */
long sorted_box_num[BOX_ARRAY_SIZE];      /* sorted box numbers, ascending   */
struct of_pick_item *box_tab;
desc_item *box_desc;
long box_picks_to_go,
     box_number,
     box_short_picks;

int sort_sequence(const long int *,
                  const long int,
                  long int *);
#endif

/*-------------------------------------------------------------------------*
 *  Output File And Misc Items
 *-------------------------------------------------------------------------*/
 
FILE *fd;                                 /* output file descriptor          */
char fd_name[40];                         /* output file name                */
char printer[40];                         /* system printer name             */
long daemon = 0;                          /* printer daemon flag             */
char command[64];                         /* lpr command                     */

/*-------------------------------------------------------------------------*
 *  Dates In Three Formats
 *-------------------------------------------------------------------------*/

char udate[9];                            /* date mo/da/yr                   */
char edate[9];                            /* date da/mo/yr                   */
char mdate[13];                           /* date mon da, yyyy               */

/*-------------------------------------------------------------------------*
 *  Debug Mode Paramters
 *-------------------------------------------------------------------------*/
 
long debug     = 0;                       /* debug mode flag                 */
long dcopies   = 1;                       /* debug copies                    */
long dpicks    = 0;                       /* one full set of picks           */
char dvalue    = 'x';                     /* debug data value                */

/*------------------------------------------------------------------------*
 *  Specialized Header Files For This Packing List
 *------------------------------------------------------------------------*/
#ifdef TOTE_SCAN
#include "packing_list_tote.h"
#else
#include "packing_list_eckerd.h"
#endif

/*------------------------------------------------------------------------*/

extern leave();

/*-------------------------------------------------------------------------*
 *   M A I N
 *-------------------------------------------------------------------------*/
main(argc, argv)
long  argc;
char  **argv;
{
  register long k;
  register char *p;

  long x = 0L;
  int results = 0;

  char L_CustomerOrderNo[CLENGTH + 1];

  putenv("_=packing_list"); 
  chdir(getenv("HOME"));
  
  for (k = 1; k < argc; k++)              /* look at all arguments           */
  {
    if (strcmp(argv[k], "-d") == 0)
    debug = 1;

    else if (strncmp(argv[k], "-value=", 7) == 0)
    dvalue = argv[k][7];

    else if (strncmp(argv[k], "-copies=", 8) == 0)
    sscanf(&argv[k][8], "%d", &dcopies);

    else if (strncmp(argv[k], "-picks=", 7) == 0)
    sscanf(&argv[k][7], "%d", &dpicks);

    else if (strncmp(argv[k], "-f=", 3) == 0)
    strcpy(fd_name, &argv[k][3]);

    else if (strncmp(argv[k], "/dev/", 5) == 0)
    strcpy(fd_name, argv[k]);
    else
    {
      sprintf(printer, "%s", argv[k]);  /* F030795                   */
      daemon = 1;                         /* use printer daemon              */
    }
  }
/*
 *      Open Database Files
 */
  database_open();

  prodfile_open( AUTOLOCK );
  prodfile_setkey( 1 );

  pmfile_open( AUTOLOCK );
  pmfile_setkey( 1 );

/*
 *  Open printers
 */
  if (!daemon)                            /* is a real device                */
  {
    fd = fopen(fd_name, "w");
    if (fd == 0)
    {
      printf("Can't open %s\n\n", fd_name);
    }
  }
/* 
 *  Open System Segments
 */
  ss_open();
  oc_open();
  od_open();
  co_open();
/*
 * Setup Pointers
 */
  rmks  = (remarks_text_item *)  or_rec->rmks_text;
  ptext = (pick_text_item *)     op_rec->pi_pick_text;
  pick  = (struct of_pick_info *)&op_rec->pi_mod;

  tab   = (struct of_pick_item *)malloc(oc->op_rec_size * oc->of_max_picks);
  desc  = (desc_item *)malloc(25 * oc->of_max_picks);
#ifdef TOTE_SCAN
  box_tab  = (struct of_pick_item *)malloc(oc->op_rec_size * oc->of_max_picks);
  box_desc = (desc_item *)malloc(25 * oc->of_max_picks);
#endif
/*
 * Open Print Request Queue
 */

  packing_list_open(AUTOLOCK);
  packing_list_setkey(1);

/*-------------------------------------------------------------------------*
 *   M A I N   L O O P
 *-------------------------------------------------------------------------*/

  set_size(1);                            /* setup default letter size       */

  image = (char *)malloc(LENGTH * WIDTH); /* print page work image           */
  memset(image, 0x20, WIDTH * LENGTH);    /* clear image initially           */

#ifdef DEBUG
  fprintf(stderr, "Packing List Started...\n");
#endif
   
  if (debug)
     {
       do_debug();
       leave();
     }

  while (1)
  {
    begin_work();
    if (packing_list_next(&qi, LOCK))
       {
         commit_work();
         sleep(DELAY);                  /* empty - wait and try again      */
         packing_list_setkey(1);
         continue;
       }

    if (qi.paper_pl > coh->co_pl_cnt)
       {
         commit_work();
         continue;
       }
    
    packing_list_delete();
    commit_work();
    sleep(8);                           /* time delay to ensure all picks    */
                                        /* have been updated                 */
    
    print_flag = 1;                       /* set print flag is true          */

#ifdef DEBUG
    fprintf(stderr, "packing_list() pl:%d on:%d\n",
            qi.paper_pl, qi.paper_order);
    fflush(stderr);
#endif

#ifdef DALLAS
    base_order  = qi.paper_order;
    base_order -= (qi.paper_order % 10000);
    base_order += (qi.paper_order % 1000);

    block = oc_find(qi.paper_pl, qi.paper_order);

    strncpy(L_CustomerOrderNo, oc->oi_tab[block -1].oi_con, CLENGTH);
    L_CustomerOrderNo[CLENGTH] = '\0';


#ifdef DEBUG
    fprintf(stderr, "Order=%d pl = %d con = %s\n", qi.paper_order,
                     qi.paper_pl, L_CustomerOrderNo);
    fprintf(stderr, "Block=%d\n", block);
    fflush(stderr);
#endif

    /* Initialize the order array to 0 */
    for (k2 = 0; k2 < MAX_ORDERS; k2++)
        {
          L_OrderNo[k2][0] = 0L;
          L_OrderNo[k2][1] = 0L;
        }

    L_Count = 0;
    x       = 0L;

    /* loop for all the configured picklines. */
    for (k2 = 0; k2 < coh->co_pl_cnt; k2++)
        {
          /* Search for orders in the queue irrespective of their status */
          for (k1 = OC_COMPLETE; k1 <= OC_HOLD; k1++)
              {
                /* Get the block for a pickline and any status */
                block1 = oc->oc_tab[k2].oc_queue[k1].oc_first;
#ifdef DEBUG
                fprintf(stderr, "Block1 = %d, queue = %d, pl = %d\n",
                        block1, k1, k2);
                fprintf(stderr, "con = %s\n", L_CustomerOrderNo);
                fflush(stderr); 
#endif
                while (block1 > 0)
                      {
                        /* Get the structure holding the order */
                        o = &oc->oi_tab[block1 - 1];
#ifdef DEBUG
                        fprintf(stderr, "Block1 = %d, o->oi_con = %s\n",
                        block1, o->oi_con);
                        fflush(stderr);
#endif
                        /* Check if the record found is correct or not */
                        if (memcmp(o->oi_con, L_CustomerOrderNo, CLENGTH) == 0)
                           {
#ifdef DEBUG
                             fprintf(stderr, "Block1=%d pl = %d con = %s\n",
                                     block1, k2+1, L_CustomerOrderNo);
                             fflush(stderr);
#endif
                             /* Check if the order for pickline has been picked 
                             ** If not, set the flag else store the pickline no
                             ** and order no in the global array 
                             */
#ifdef SHIV
                             if ((oc->oi_tab[block1 - 1].oi_queue !=
                                  OC_COMPLETE)                    || 
                                 (oc->oi_tab[block1 - 1].oi_flags &
                                  TOTE_LABEL)                       )
#else
                             if (oc->oi_tab[block1 - 1].oi_queue !=
                                 OC_COMPLETE                        ) 
#endif
                                {
                                  L_Count = 1;
                                }
                             else
                                {
#ifdef DEBUG
                                  fprintf(stderr,"Transaction = %d\n",
                                          oc->oi_tab[block1-1].oi_queue);
                                  fflush(stderr);
#endif
                                  L_OrderNo[x][0] =
                                    oc->oi_tab[block1 - 1].oi_pl;
                                  L_OrderNo[x][1] =
                                    oc->oi_tab[block1 - 1].oi_on;
                                  x++;
                                }
                           } /* End find if order is complete */

                        if (L_Count == 1)
                           {
#ifdef DEBUG
                             fprintf(stderr,"L_Count = %d\n",L_Count);
#endif
                             block1 = 0;
                             k1     = 6;
                             k2     = coh->co_pl_cnt;
                           }
                        else
                           {
                             block1 = o->oi_flink;
                           }
                      } /* End of while loop for one order queue */
              } /* End of for loop to search all queues for one pickline */
        } /* End of for loop to search all picklines */

    if (L_Count == 1)
       {
         continue;
       }

    if (daemon)
       {
         memset(fd_name, 0x20, 40);         /* added on 01/24/97   */
         tmp_name(fd_name);                 /* get a file name     */
         fd = fopen(fd_name, "w");
       }

    do_dates();                             /* build date fields             */

    for (k1 = 0 ; k1 < x; k1++)
        {
          qi.paper_pl    = L_OrderNo[k1][0]; 
          qi.paper_order = L_OrderNo[k1][1]; 
#ifdef SHIV
          block2 = oc_find(qi.paper_pl, qi.paper_order);
          oc_lock();
          oc->oi_tab[block2 - 1].oi_flags |= TOTE_LABEL;
          oc_unlock();
#endif
#ifdef DEBUG
          fprintf(stderr, "k1 = %d\n", k1);
          fflush(stderr);
#endif
          fetch_order();
        }

    sleep(DELAY);  

    /* fprintf(fd, "%c", HOF); */      /* prints blank page betw stores */
#endif

#ifdef CONROE
    block = oc_find(qi.paper_pl, qi.paper_order);
    do_dates();
    fetch_order();
#endif

    if (daemon && fd)
       {
#ifdef DEBUG
         fprintf(stderr, "Inside Printing Call...\n");
         fflush(stderr);
#endif
         fclose(fd);
         fd = 0;

         if (print_flag)
            {
#ifdef DEBUG3
              fprintf(stderr, "Printing to spool... File=%s\n", fd_name);
              fflush(stderr);
#endif
              sprintf(command, "cp -f %s  tmp/last_manifest", fd_name);
              results = system(command);
#ifdef DEBUG4 
              sprintf(command,"cp -f %s tmp/mani",fd_name);
              results = system(command);
#endif
/*  Use if want mail notification that print job succeeded.
              sprintf(command,"lpr -m -d %s -R %s",printer,fd_name);
*/
              sprintf(command,"lpr -d %s -R %s",printer,fd_name);
              results = system(command);
#ifdef DEBUG3
              fprintf(stderr, "command= %s\n", command);
              fprintf(stderr, "File printed was for order = %d,\n", 
                      qi.paper_order);
              fprintf(stderr, "   con = %s, results = %d\n",
                      L_CustomerOrderNo, results);
              fflush(stderr);
#endif
            }
       }
  } /* End of main while loop */
} /* End of main() */


/*-------------------------------------------------------------------------*
 *  Fetch Order and Remarks
 *-------------------------------------------------------------------------*/
fetch_order()
{
#ifdef TOTE_SCAN
  long count             = 0L,
       save_copies       = 0L,
       index             = 0L,
       index1            = 0L;
  short int box_add_flag = 0;

  memset(box_num, 0L, BOX_ARRAY_SIZE);
  memset(sorted_box_num, 0L, BOX_ARRAY_SIZE);
#endif

  copies = 1L;                           /* default copies                  */
  /* if (qi.paper_copies) copies = qi.paper_copies; */

  block = oc_find(qi.paper_pl, qi.paper_order);

  if (block <= 0) return;                 /* no order record found ???       */
         
  of_rec->of_pl = qi.paper_pl;
  of_rec->of_on = qi.paper_order;
  if (order_read(of_rec, NOLOCK)) return;
      
  sp->sp_pl_order = of_rec->of_on;       /* current order                   */

  if (remarks_fd && rf->rf_rmks > 0)
  {
    or_rec->rmks_pl = qi.paper_pl;
    or_rec->rmks_on = qi.paper_order;
    if (remarks_read(or_rec, NOLOCK)) memset(rmks, 0x20, rf->rf_rmks);
  }
  sp->sp_pl_order = qi.paper_order;
  sp->sp_pl_print += 1; 

  op_rec->pi_pl  = qi.paper_pl;
  op_rec->pi_on  = qi.paper_order;
  op_rec->pi_mod = 1;
  pick_startkey(op_rec);
    
  op_rec->pi_mod = sp->sp_products;
  pick_stopkey(op_rec);

  short_picks = 0;
  picks_to_go = 0;
  
  while (!pick_next(op_rec, NOLOCK))
  {
    if (op_rec->pi_ordered > op_rec->pi_picked) short_picks++;
    
#ifdef DEBUG
    fprintf(stderr, "FetchOrders()pl=%d on=%d ordered=%d, picked=%d,\n", 
            op_rec->pi_pl, op_rec->pi_on,
            op_rec->pi_ordered,
            op_rec->pi_picked);
    fprintf(stderr, "   box_number=%d.\n", op_rec->pi_box_number);
    fflush(stderr);
#endif
    get_pm_sku();

#ifdef TOTE_SCAN
    if (!count)
       {
         box_num[0] = op_rec->pi_box_number;
         count      = 1L;
       }
    else
       {
         box_add_flag = 0;
         for (index = 0L; index < count; index++)
             {
               if (op_rec->pi_box_number == box_num[index])
                  {
                    box_add_flag = 0;
                    break;
                  }
               else
                  {
                    box_add_flag = 1;
                  }
             }

          if (box_add_flag)
             {
               box_num[count] = op_rec->pi_box_number;
               count++;
             }
       }
#endif

    memcpy(&tab[picks_to_go], op_rec, oc->op_rec_size);
    memcpy(&desc[picks_to_go], sku_rec.p_descr, 25);
#ifdef DEBUG2
    Bdump(&tab[picks_to_go], sizeof(struct of_pick_item));
#endif

    picks_to_go++;
  } /* end of while loop for fetching picks */

#ifdef DEBUG
  fprintf(stderr, "picks=%d  short=%d\n", picks_to_go, short_picks);
  fflush(stderr);
#endif
#ifdef TOTE_SCAN
  if (count > 1L)
     {
       sort_sequence(box_num, count, sorted_box_num);
     }
  else
     {
       sorted_box_num[0] = box_num[0];
     }

  for (index = 0L; index < count; index++)
      {
        box_number = sorted_box_num[index];
#ifdef DEBUG
        fprintf(stderr, "Sorted Boxes: box=%d, index=%d, count=%d\n",
                box_number, index, count);
        fflush(stderr);
#endif
        box_picks_to_go = 0L;

        for (index1 = 0L; index1 < picks_to_go; index1++)
            {
              memcpy(op_rec, &tab[index1], oc->op_rec_size);
              if (op_rec->pi_box_number == box_number)
                 {
                   memcpy(&box_tab[box_picks_to_go],
                          &tab[index1],
                          oc->op_rec_size);
                   memcpy(&box_desc[box_picks_to_go],
                          &desc[index1],
                          25);
                   box_picks_to_go++;

                   if (op_rec->pi_ordered > op_rec->pi_picked)
                      {
                        box_short_picks++;
                      }
                 }
            }

        box_sort();

        save_copies = copies;
        box_format_report();
        copies = save_copies;
      }
#else
  sort();
  
  format_report();
#endif
  return 0;
}  /* End of fetch_order() */


/*------------------------------------------------------------------------*
 *  Sort Pick Table
 *------------------------------------------------------------------------*/
sort()
{
  register long j, k;
  struct of_pick_item work;
  char text[25];
  
#ifdef DEBUG        
  fprintf(stderr, "Picks to go in sort function:%d\n", picks_to_go)	;
  fflush(stderr);
#endif
  for (j = 0; j < picks_to_go; j++)
  {
    for (k = j + 1; k < picks_to_go; k++)
    {
      if (memcmp(desc[k].txt, desc[j].txt, 25) < 0)
      {
        memcpy(text,        desc[j].txt, 25);
        memcpy(desc[j].txt, desc[k].txt, 25);
        memcpy(desc[k].txt, text,    25);

        memcpy(&work,   &tab[j], oc->op_rec_size);
        memcpy(&tab[j], &tab[k], oc->op_rec_size);
        memcpy(&tab[k], &work,   oc->op_rec_size);
      }
    }
  }
  return 0;
}


#ifdef TOTE_SCAN
/*------------------------------------------------------------------------*
 *  Sort Pick Table for a box
 *------------------------------------------------------------------------*/
box_sort()
{
  register long j, k;
  struct of_pick_item work;
  char text[25];
  
#ifdef DEBUG        
  fprintf(stderr, "Picks to go in box_sort function:%d\n", box_picks_to_go);
  fflush(stderr);
#endif
  for (j = 0; j < box_picks_to_go; j++)
      {
        for (k = j + 1; k < box_picks_to_go; k++)
            {
              if (memcmp(box_desc[k].txt, box_desc[j].txt, 25) < 0)
                 {
                   memcpy(text,            box_desc[j].txt, 25);
                   memcpy(box_desc[j].txt, box_desc[k].txt, 25);
                   memcpy(box_desc[k].txt, text,            25);
           
                   memcpy(&work,       &box_tab[j], oc->op_rec_size);
                   memcpy(&box_tab[j], &box_tab[k], oc->op_rec_size);
                   memcpy(&box_tab[k], &work,       oc->op_rec_size);
                 }
            }
      }

  return 0;
}  /* end of box_sort() */


/*------------------------------------------------------------------------*
 *  Format Report - Do All Copies For One Box 
 *------------------------------------------------------------------------*/
box_format_report()
{
  before_request();                       /* any preliminary processing      */

#ifdef DEBUG        
  fprintf(stderr, "Picks to go in box_format_report function:%d\n",
          box_picks_to_go);
  fflush(stderr);
#endif

  last_page = (box_picks_to_go + REPEAT - 1) / REPEAT;
  if (last_page < 1) last_page = 1;       /* always one page                 */
      
  last_short_page = 0;
  if (box_short_picks)
  {
    last_short_page = (box_short_picks + REPEAT - 1) / REPEAT;
    if (last_short_page < 1) last_short_page = 1;
  }

  while (copies > 0 && print_flag)        /* do until all copies printed     */
  {
    for (page = 1, pick_no = 0; page <= last_page; page++)
    {
      set_base(1, 1);                     /* default to top corner           */

      if (page == 1) before_first_page();

      before_each_page();                 /* before picks formatted          */

      for (line = 0;
           line < REPEAT && pick_no < box_picks_to_go;
           line++, pick_no++)
      {
        memcpy(op_rec, &box_tab[pick_no], oc->op_rec_size);

#ifdef DEBUG2        
        Bdump(op_rec, oc->op_rec_size);
#endif
        set_base(PICK_ROW + line * PICK_LINES, PICK_COL);
        get_pm_sku();
        each_pick();                      /* format this pick                */
      }
      set_base(1, 1);                     /* default to top corner           */
      after_each_page();                  /* after picks formatted           */
         
      if (page == last_page) after_last_page();
      print_all();
    }

    copies--;
  }

  return;
} /* end of box_format_report() */


/*-------------------------------------------------------------------------*
 *  Special Short Pages for Boxes
 *-------------------------------------------------------------------------*/
box_format_short_page()
{
  for (page = 1, pick_no = 0; page <= last_short_page; page++)
  {
    set_base(1, 1);                       /* default to top corner           */

    if (page == 1) before_first_short_page();

    before_each_short_page();           /* before picks formatted          */

    for (line = 0; line < REPEAT && pick_no < box_picks_to_go; pick_no++)
    {
      memcpy(op_rec, &box_tab[pick_no], oc->op_rec_size);

      if (op_rec->pi_ordered <= op_rec->pi_picked) continue;
      
      set_base(PICK_ROW + line * PICK_LINES, PICK_COL);
      get_pm_sku();
      each_short_pick();                  /* format this pick                */
      
      line++;
    }
    set_base(1, 1);                       /* default to top corner           */
    after_each_short_page();              /* after picks formatted           */
         
    if (page == last_short_page) after_last_short_page();
    print_all();
  }

  return;
} /* end of box_format_short_page() */
#endif


/*------------------------------------------------------------------------*
 *  Terminate On Shutdown
 *------------------------------------------------------------------------*/
leave()
{
  printf("\rPacking List Stopping On Shutdown\n");
  ss_close();
  oc_close();
  od_close();
  co_close();
  prodfile_close();
  pmfile_close();
  packing_list_close();
  if (!daemon) fclose(fd);
  database_close();
  exit(0);
}


/*------------------------------------------------------------------------*
 *  Format Report - Do All Copies For One Queue Request
 *------------------------------------------------------------------------*/
format_report()
{
  before_request();                       /* any preliminary processing      */

#ifdef DEBUG        
  fprintf(stderr, "Picks to go in format_report function:%d\n", picks_to_go);
  fflush(stderr);
#endif

  last_page = (picks_to_go + REPEAT - 1) / REPEAT;
  if (last_page < 1) last_page = 1;       /* always one page                 */
      
  last_short_page = 0;
  if (short_picks)
  {
    last_short_page = (short_picks + REPEAT - 1) / REPEAT;
    if (last_short_page < 1) last_short_page = 1;
  }

  while(copies > 0 && print_flag)         /* do until all copies printed     */
  {
    for (page = 1, pick_no = 0; page <= last_page; page++)
    {
      set_base(1, 1);                     /* default to top corner           */

      if (page == 1) before_first_page();

      before_each_page();                 /* before picks formatted          */

      for (line = 0;line < REPEAT && pick_no < picks_to_go; line++, pick_no++)
      {
        memcpy(op_rec, &tab[pick_no], oc->op_rec_size);

#ifdef DEBUG2        
        Bdump(op_rec, oc->op_rec_size);
#endif
        set_base(PICK_ROW + line * PICK_LINES, PICK_COL);
        get_pm_sku();
        each_pick();                      /* format this pick                */
      }
      set_base(1, 1);                     /* default to top corner           */
      after_each_page();                  /* after picks formatted           */
         
      if (page == last_page) after_last_page();
      print_all();
    }

/* if (short_picks > 0) format_short_page(); */
    copies--;
/*     fprintf(fd, "%c", HOF);               print blank page betw stores */
  }
  return;
}


/*-------------------------------------------------------------------------*
 *  Special Short Pages
 *-------------------------------------------------------------------------*/
format_short_page()
{
  for (page = 1, pick_no = 0; page <= last_short_page; page++)
  {
    set_base(1, 1);                       /* default to top corner           */

    if (page == 1) before_first_short_page();

    before_each_short_page();           /* before picks formatted          */

    for (line = 0; line < REPEAT && pick_no < picks_to_go; pick_no++)
    {
      memcpy(op_rec, &tab[pick_no], oc->op_rec_size);

      if (op_rec->pi_ordered <= op_rec->pi_picked) continue;
      
      set_base(PICK_ROW + line * PICK_LINES, PICK_COL);
      get_pm_sku();
      each_short_pick();                  /* format this pick                */
      
      line++;
    }
    set_base(1, 1);                       /* default to top corner           */
    after_each_short_page();              /* after picks formatted           */
         
    if (page == last_short_page) after_last_short_page();
    print_all();
  }
}


/*-------------------------------------------------------------------------*
 *  Print Entire Image Area
 *-------------------------------------------------------------------------*/
print_all()
{
  register char *p;
  register long i;

  for (i = 0, p = image; i < LENGTH; i++, p += WIDTH)
  {
    fprintf(fd, "%*.*s\n", WIDTH, WIDTH, p);
  }
  memset(image, 0x20, LENGTH * WIDTH);
}
/*-------------------------------------------------------------------------*
 *  Get PM and SKU Record
 *-------------------------------------------------------------------------*/
get_pm_sku()
{
  register char *p;
  register long ret;
   
  if (!rf->rf_sku) return;

  pkm_rec.p_pmodno = pick->of_mod;

  ret = pmfile_read( &pkm_rec, NOLOCK );

  if (!ret)
  {
    memcpy( sku_rec.p_pfsku, pkm_rec.p_pmsku, 15 );
    ret = prodfile_read( &sku_rec, NOLOCK );
    if (!ret) return;
  }
  strncpy(pkm_rec.p_pmsku, "**ERROR**ERROR**", 15);
  strncpy(sku_rec.p_pfsku, "**ERROR**ERROR**", 15);
  strncpy(sku_rec.p_altid,  "**ERROR**ERROR**", 15);
  return;
}
/*-------------------------------------------------------------------------*
 *  Format three dates
 *-------------------------------------------------------------------------*/
do_dates()
{
  static char month[12][3] = {"Jan", "Feb", "Mar",
    "Apr", "May", "Jun",
    "Jul", "Aug", "Sep",
    "Oct", "Nov", "Dec"};
  long now;
  long *t;                                /* t[4] = mon (0-11)               */
                                          /* t[3] = day (1-31)               */
                                          /* t[5] = year (70-??)             */

  time(&now);                             /* get time and break apart        */
  t = (long *)localtime(&now);
  
  sprintf(udate, "%02d/%02d/%02d", t[4]+1, t[3], t[5] % 100);     /* mm/dd/yy      */
  sprintf(edate, "%02d/%02d/%02d", t[3], t[4]+1, t[5] % 100);     /* dd/mm/yy      */
  sprintf(mdate, "%.3s %2d, %4d",                           /* mmm dd, yyyy  */
    month[t[4]], t[3], t[5] + 1900);

  return;
}
/*-------------------------------------------------------------------------*
 *  Move Alpha Field To Image Area
 *-------------------------------------------------------------------------*/
aplace(field, len, row, col)
register char *field;                     /* field to print                  */
register long len;                        /* field length                    */
register long row, col;                   /* output print location           */
{
  register char c;
  register long j, letter, frame;
  register char *p, *q;
   
#ifdef DEBUG2
  fprintf(stderr, "aplace(%x, %d, %d, %d)\n", field, len, row, col);
  fflush(stderr);
#endif
   
  row += base_row;                        /* make absolute & zero based      */
  col += base_col;

  if (current_letter->letter_size == 1)   /* normal print output             */
  {
    p = &image[row * WIDTH + col];
    for (; len > 0; len--)
    {
      if (*field < 0x20 || *field > 0x7d) return;
      *p++ = *field++;
    }
    return;
  }
  for (; len > 0; len--, col += current_letter->letter_width + 1, field++)
  {
    c = *field;
    if (!c) break;                        /* end of field                    */

    if (col + current_letter->letter_width > WIDTH)
    return;                               /* won't fit on line               */

    letter = 0;
    if (c >= '0' && c <= '9') letter = (c - '0') + 1;
    else if (c >= 'A' && c <= 'Z') letter = (c - 'A') + 11;
    else if (c >= 'a' && c <= 'z') letter = (c - 'a') + 11;

    frame = current_letter->letter_size * current_letter->letter_width;

    p = &current_letter->letter_image[letter * frame];

    q = &image[row * WIDTH + col];

    for (j = 0; j < current_letter->letter_size; j++)
    {
      memcpy(q, p, current_letter->letter_width);
      p += current_letter->letter_width;
      q += WIDTH;
    }
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Edit Numeric Field Using Mask
 *-------------------------------------------------------------------------*/
nplace(field, mask, row, col)
register long field;
register char *mask;
register row, col;
{
  register char *p, *q;
  register long m, n;
  char temp[64];
  char work[16];

#ifdef DEBUG2
  fprintf(stderr, "nplace(%x, %s, %d, %d)\n", field, mask, row, col);
#endif
   
  sprintf(work, "%12d", field);           /* convert to alpha                */
   
  m = n = strlen(mask);                   /* length of the mask              */
  memset(temp, 0x20, n);                  /* clear output area               */

  mask  += (n - 1);                       /* left of mask                    */
  q     =  temp + n - 1;                  /* left of numeric value           */
  p     =  work + 11;                     /* left of output                  */

  for (; n > 0; n--, mask--, q--)
  {
    switch (*mask)
    {
    case '9':   if (*p == 0x20 || *p == '-') *q = '0';
      else *q = *p--;
      break;
                     
    case 'Z':   if (*p == 0x20) *q = 0x20;
      else *q = *p--;
      break;

    case '.':   *q = '.';
      break;
                     
    case ',':   *q = 0x20;
      if (*(q + 1) >= '0' && *(q + 1) <= '9')
      {
        if (*p != 0x20 || *(mask - 1) == '9') *q = ',';
      }
      break;
         
      default:    *q = *mask;
      break;
    }
  }
  aplace(temp, m, row, col);              /* now put in print image area     */
  return;
}
/*-------------------------------------------------------------------------*
 *  Set Base Row And Column  1 <= x <= LENGTH; 1 <= y <= WIDTH
 *-------------------------------------------------------------------------*/
set_base(x, y)
register long x, y;
{
  if (x < 1 || x > LENGTH)   x = 1;       /* make bad values harmless        */
  if (y < 1 || y > WIDTH)    y = 1;
   
  base_row = x - 2;                       /* to convert to zero based        */
  base_col = y - 2;

  return;
}
/*-------------------------------------------------------------------------*
 *  Convert To Binary
 *-------------------------------------------------------------------------*/
cvrt(field, n)
register char *field;
register long n;
{
  register long x;
   
  x = 0;

  while (n > 0 && *field == 0x20) {n--; field++;}

  for(; n > 0; n--, field++)
  {
    if (*field < '0' || *field > '9') break;
    x = 10 * x + (*field - '0');
  }
  return x;
}
/*-------------------------------------------------------------------------*
 *  Set Current Printing Size (1, 5, 7, or 9)
 *
 *  Values are pointed to by global variable 'current_letter'
 *-------------------------------------------------------------------------*/
set_size(x)
register long x;
{
  register long space;
  FILE *ld;
  char letter_name[40];
   
  switch (x)
  {
    default:
  case 1:  current_letter = letter1;
    if (current_letter) return;           /* table is already loaded         */
    letter1 = (letter_item *)malloc(sizeof(letter_item));
    current_letter = letter1;
    current_letter->letter_size   = 1;
    current_letter->letter_width  = 1;
    current_letter->letter_image  = 0;
    return;
               
  case 5:  current_letter = letter5;
    if (current_letter) return;           /* table is already loaded         */
    letter5 = (letter_item *)malloc(sizeof(letter_item));
    current_letter = letter5;
    current_letter->letter_size   = 5;
    current_letter->letter_width  = 5;
    strcpy(letter_name, "sys/letter5.t");
    break;
      
  case 7:  current_letter = letter7;
    if (current_letter) return;           /* table is already loaded         */
    letter7 = (letter_item *)malloc(sizeof(letter_item));
    current_letter = letter7;
    current_letter->letter_size   = 7;
    current_letter->letter_width  = 5;
    strcpy(letter_name, "sys/letter7.t");
    break;
               
  case 9:  current_letter = letter9;
    if (current_letter) return;           /* table is already loaded         */
    letter9 = (letter_item *)malloc(sizeof(letter_item));
    current_letter = letter9;
    current_letter->letter_size   = 9;
    current_letter->letter_width  = 7;
    strcpy(letter_name, "sys/letter9.t");
    break;

  }
  ld = fopen(letter_name, "r");
  if (ld == 0)
  {
    printf("Can't Open %s\n\n", letter_name);
    current_letter = letter1;
    return;
  }
  fseek(ld, 0, 2);
  space = ftell(ld);
  fseek(ld, 0, 0);
   
  current_letter->letter_image = (char *)malloc(space);

  fread(current_letter->letter_image, space, 1, ld);
  fclose(ld);
}
/*-------------------------------------------------------------------------*
 *  Debug Form Layout
 *-------------------------------------------------------------------------*/
do_debug()
{
  register long picks_to_go;

  if (daemon)
  {
    tmp_name(fd_name);                    /* get a file name                 */
    fd = fopen(fd_name, "w");
  }
  if (rf->rf_rmks)                        /* dummy remarks text              */
  {
    rmks = (remarks_text_item *)malloc(rf->rf_rmks);
    memset(rmks, dvalue, rf->rf_rmks);
  }
  if (rf->rf_pick_text)                   /* dummy pick text                 */
  {
    ptext = (pick_text_item *)malloc(rf->rf_pick_text);
    memset(ptext, dvalue, rf->rf_pick_text);
  }
  if (dpicks < 1) dpicks = REPEAT;

  of_rec->of_on        = 99999;
  of_rec->of_datetime  = time(0);
  of_rec->of_no_picks  = dpicks;
  of_rec->of_pl        = 1;
  of_rec->of_pri       = 'h';
  of_rec->of_status    = 'c';
  memset(of_rec->of_grp, dvalue, 19);
   
  op_rec->pi_pl         =  1;
  op_rec->pi_on         =  99999;
  op_rec->pi_mod        =  1;
  op_rec->pi_ordered    =  99;
  op_rec->pi_picked     =  98;
  op_rec->pi_box_number =  999999;

  memset(op_rec->pi_sku, dvalue, 15);
  memset(op_rec->pi_pick_text, dvalue, 32);
  
  memset(&sku_rec, dvalue, sizeof(prodfile_item));
  sku_rec.p_ipqty = 99;
  sku_rec.p_cpack = 99;

  memset(&pkm_rec, 0, sizeof(pmfile_item));
  pkm_rec.p_pmodno = 1;
  memset(pkm_rec.p_pmsku, dvalue, 15);
  memset(pkm_rec.p_stkloc, dvalue, 6);
      
  qi.paper_copies   = dcopies;
  qi.paper_pl       = 1;
  qi.paper_order    = 99999;
   
  print_flag = 1;
  copies = dcopies;                       /* debug copies                    */

  do_dates();                             /* build date fields               */

  picks_to_go = dpicks;
  last_page = (picks_to_go + REPEAT - 1) / REPEAT;
  pick_no = 0;
   
  if (last_page < 1) last_page = 1;       /* always one page                 */

  short_picks = dpicks;
  last_short_page = last_page;

  before_request();                       /* preliminaries                   */

  while(copies > 0 && print_flag)         /* do until all copies printed     */
  {
    pkm_rec.p_pmodno = 1;

    for (page = 1, pick_no = 0; page <= last_page; page++)
    {
      set_base(1, 1);                     /* default to top corner           */

      if (page == 1) before_first_page();
      before_each_page();                 /* before picks formatted          */

      for (line = 0;line < REPEAT && pick_no < picks_to_go;line++,pick_no++)
      {
        set_base(PICK_ROW + line * PICK_LINES, PICK_COL);
        each_pick();                      /* format this pick                */
        pkm_rec.p_pmodno++;
      }
      set_base(1, 1);                     /* default to top corner           */
      after_each_page();                  /* after picks formatted           */
         
      if (page == last_page) after_last_page();
      print_all();
    }
    pkm_rec.p_pmodno = 1;

    for (page = 1, pick_no = 0; page <= last_page; page++)
    {
      set_base(1, 1);                     /* default to top corner           */

      if (page == 1) before_first_short_page();
      before_each_short_page();               

      for (line = 0;line < REPEAT && pick_no < picks_to_go; line++, pick_no++)
      {
        set_base(PICK_ROW + line * PICK_LINES, PICK_COL);
        each_short_pick();                /* format this pick                */
        pkm_rec.p_pmodno++;
      }
      set_base(1, 1);                     /* default to top corner           */
      after_each_short_page();            /* after picks formatted           */
         
      if (page == last_page) after_last_short_page();
      print_all();
    }
    copies--;
    fprintf(fd, "%c", HOF);
  }

  if (daemon && fd)
  {
    fclose(fd); fd = 0;
    if (print_flag)
    {
      sprintf(command, "%s %s %s %s", "lpr -d ", printer, "-R ", fd_name);
      system(command); 
    }
  }
}


#ifdef TOTE_SCAN
/*-------------------------------------------------------------------------*
 *  Sort Input Array in  Ascending Sequential Order 
 *-------------------------------------------------------------------------*/
int sort_sequence(const long int *input_array,
                  const long int size_of_input_array,
                  long int *sorted_array)
{
  long int diff_array[BOX_ARRAY_SIZE][BOX_ARRAY_SIZE + 1],
           index;

  long int i,
           j,
         max;

  i     = 0;
  j     = 0;
  index = 0;
  max   = size_of_input_array;

  for (i = 0; i < max; i++)
      {
         diff_array[i][max] = 0;
      }

  for (i = 0; i < max; i++)
      {
        for (j = 0; j < max; j++)
            {
               diff_array[i][j] = input_array[i] - input_array[j];

               if (diff_array[i][j] > 0)
                  {
                     diff_array[i][max] = diff_array[i][max] + 1;
                  }
            }
      }

  for (i = 0; i < max; i++)
      {
         index = diff_array[i][max];

         sorted_array[index] = input_array[i];
      }

  return 0;
}  /* End of sort_sequence() */
#endif

/* end of tote_packing_list.c */
