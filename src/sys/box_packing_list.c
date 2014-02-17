/*-------------------------------------------------------------------------*
 *  Custom Versions: CANTON and all others
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Packing list daemon for boxes.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/27/93   |  tjt  Add to mfc.
 *  08/05/94   |  tjt  Fix when only boxid is given. (from shf fixes).
 *  08/09/94   |  tjt  Add putenv and chdir.
 *  11/24/94   |  tjt  Add order and print count.
 *  02/02/95   |  tjt  Remove UNOS queues. 
 *  06/14/95   |  tjt  Fix aplace copy.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  04/19/06   |  tjt  Add printed count.
 *  10/08/01   |  aha  Modified for Eckerd's Tote Integrity.
 *-------------------------------------------------------------------------*/
static char box_packing_list_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/*--------------------------------------------------------------------------*
 *  Prints Packing Lists by Box
 *
 *  To Execute:      box_packing_list <printer>  &
 *
 *  To Test Report:  box_packing_list -d [-copies=n] [-picks=m] [-value=x]
 *                              
 *                   -d indicates report debug mode
 *                   -copies=n indicates 'n' copies
 *                   -picks=m  indicates 'm' picks
 *                   -value=x  indicates 'x' as alpha fill byte
 *-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "boxes.h"
#ifdef CANTON
#include "bard/picker.h"
#endif

#define  DELAY   15

/*------------------------------------------------------------------------*
 *  Specialized Header Files For This Box Packing List 
 *------------------------------------------------------------------------*/

#ifdef CANTON
#include "custom/kmart/remarks_text.h"
#include "custom/kmart/pick_text.h"
#else
#include "custom/noxell/remarks_text.h"
#include "custom/noxell/pick_text.h"
#endif

/*------------------------------------------------------------------------*
 *  Input Queue Of Print Requests
 *------------------------------------------------------------------------*/
box_paper_item qi;                        /* print request                   */

/*-------------------------------------------------------------------------*
 *  Pick Module And Product File Database Files
 *-------------------------------------------------------------------------*/

prodfile_item  sku_rec;                   /* SKU record definition           */
pmfile_item    pkm_rec;                   /* pick module record definition   */
boxes_item     box;                       /* box order information           */

#ifdef CANTON
picker_item    picker;                    /* picker information              */
#endif
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

char *image;                              /* print format area               */

/*-------------------------------------------------------------------------*
 *  Order File Pointers
 *-------------------------------------------------------------------------*/

long block;                               /* order block number              */
long pick_no;                             /* current pick subscript 0 ..     */
long line;                                /* current line in repeat 0 ..     */
remarks_text_item    *rmks;               /* order remarks                   */
pick_text_item       *ptext;              /* pick text                       */
struct of_pick_info  *pick;               /* pointer to current pick         */

/*-------------------------------------------------------------------------*
 *  Output File And Misc Items
 *-------------------------------------------------------------------------*/
 
FILE *fd;                                 /* output file descriptor          */
char fd_name[40];                         /* output file name                */
char printer[40];                         /* system printer name             */
char printer_env[40];                     /* as PRINTER=xxxxx                */
long daemon = 0;                          /* printer daemon flag             */
char command[60];                         /* lpr command                     */

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
 *  Specialized Header Files For This Box Packing List
 *------------------------------------------------------------------------*/

#ifdef CANTON
#include "custom/kmart/box_packing_list.h"
#else
#include "custom/noxell/box_packing_list.h"
#endif

void leave(int signum); // Signal handler

/*-------------------------------------------------------------------------*
 *   M A I N
 *-------------------------------------------------------------------------*/

main(argc, argv)
long  argc;
char  **argv;
{
  register long k;
  register char *p;

  putenv("_=box_packing_list");
  chdir(getenv("HOME"));

  database_open();
  
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
    else if (strncmp(fd_name, "/dev/", 5) == 0) strcpy(fd_name, argv[k]);
    else
    {
      strcpy(printer, argv[k]);           /* is a printer daemon             */
      daemon = 1;                         /* use printer daemon              */
    }
  }
/*
 *      Open Database Files
 */
  prodfile_open( READONLY );
  prodfile_setkey( 1 );

  pmfile_open( READONLY );
  pmfile_setkey( 1 );

  boxes_open( AUTOLOCK );
#ifdef CANTON
  boxes_setkey( 3 );                      /* box number only                 */
#else
  boxes_setkey( 2 );                      /* pl + order + box number         */
#endif

#ifdef CANTON
  picker_open( AUTOLOCK );
  picker_setkey( 1 );
#endif

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
  pick_setkey( 2 );
  
/*
 * Setup Pointers
 */

  rmks  = (remarks_text_item *)   or_rec->rmks_text;
  ptext = (pick_text_item *)      op_rec->pi_pick_text;
  pick  = (struct of_pick_info *)&op_rec->pi_mod;

/*
 * Open Print Request Queue
 */

  box_list_open(AUTOLOCK);
  box_list_setkey( 1 );

/*-------------------------------------------------------------------------*
 *   M A I N   L O O P
 *-------------------------------------------------------------------------*/
  signal(SIGTERM, leave);                 /* terminate on signal             */

  set_size(1);                            /* setup default letter size       */

  image = (char *)malloc(LENGTH * WIDTH); /* print page work image           */
  memset(image, 0x20, WIDTH * LENGTH);    /* clear image initially           */
   
  if (debug) {do_debug(); leave(0);} // leave is a signal handler, passing it zero!

  while (1)
  {
    begin_work();
    if (box_list_next(&qi, LOCK)) 
    {
      commit_work();
      sleep(DELAY);                        /* wait - queue is empty          */
      box_list_setkey(1);                  /* reset keys                     */
      continue;
    }
    box_list_delete();
    sp->sp_bpl_printed += 1;
    
#ifdef DEBUG
    fprintf(stderr, "Pickline=%d Copies=%d Order=%d Box=%d\n",
    qi.paper_pl, qi.paper_copies, qi.paper_order, qi.paper_box_number);
#endif

    print_flag = 1;                       /* set print flag is true          */
    copies = 1;                           /* default copies                  */

    if (qi.paper_copies) copies = qi.paper_copies;

    box.b_box_pl     = qi.paper_pl;
    box.b_box_on     = qi.paper_order;
    box.b_box_number = qi.paper_box_number;

    if (boxes_read(&box, LOCK)) 
    {
      commit_work();
      continue;
    }
    box.b_box_status[0] = BOX_PRINTED;
    box.b_box_status[1] = '\0';
    boxes_update(&box);
    commit_work();
    
    block = oc_find(box.b_box_pl, box.b_box_on);

    if (block <= 0) continue;             /* no order record found ???       */
      
    of_rec->of_pl = box.b_box_pl;
    of_rec->of_on = box.b_box_on;

    if (order_read(of_rec, NOLOCK)) continue;
      
    sp->sp_pl_print = of_rec->of_on;  /* current order                    */

    or_rec->rmks_pl = box.b_box_pl;
    or_rec->rmks_on = box.b_box_on;
    if (remarks_read(or_rec, NOLOCK)) memset(rmks, 0x20, rf->rf_rmks);
    
    format_report();
  }
  return;
}
/*------------------------------------------------------------------------*
 *  Terminate Processing On Shutdown
 *------------------------------------------------------------------------*/
void leave(int signum)
{
  printf("\rBox Packing List Stopping On Shutdown\n");

  ss_close();
  co_close();
  oc_close();
  od_close();
  box_list_close();
  prodfile_close();
  pmfile_close();
  boxes_close();
#ifdef CANTON
  picker_close();
#endif
  if (!daemon) fclose(fd);
  database_close();
  exit(0);
}
/*------------------------------------------------------------------------*
 *  Format Report - Do All Copies For One Queue Request
 *------------------------------------------------------------------------*/
format_report()
{
  register unsigned short who;
  register struct st_item *s;
  register long picks_to_go;

  if (daemon)
  {
    tmp_name(fd_name);                    /* get a file name                 */
    fd = fopen(fd_name, "w");
  }
  do_dates();                             /* build date fields               */

  before_request();                       /* any preliminary processing      */

  picks_to_go = box.b_box_lines;

  last_page = (picks_to_go + REPEAT - 1) / REPEAT;
  if (last_page < 1) last_page = 1;       /* always one page                 */
      
  while(copies > 0 && print_flag)         /* do until all copies printed     */
  {
    op_rec->pi_pl         = box.b_box_pl;
    op_rec->pi_on         = box.b_box_on;
    op_rec->pi_box_number = box.b_box_number;

    pick_startkey(op_rec);

    set_base(1, 1);                       /* default to top corner           */
    before_first_page();

    for (page = 1; page <= last_page; page++)
    {
      set_base(1,1);
      before_each_page();                 /* before picks formatted          */

      for (line = 0; line < REPEAT; line++)
      {
        if (pick_next(op_rec, NOLOCK)) break;

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
  if (daemon && fd)
  {
    fclose(fd); fd = 0;
    if (qi.paper_printer[0] > 0x20)
    {
      sprintf(printer_env, "PRINTER=%-8.8s", qi.paper_printer);
    }
    else sprintf(printer_env, "PRINTER=%-8.8s", printer);
    putenv(printer_env);

    if (print_flag)
    {
      sprintf(command, "%s %s", getenv("LPR"), fd_name);

#ifdef DEBUG
  fprintf(stderr, "Command: %s\n", command);
#endif
      system(command);
    }
  }

  return;
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

  pkm_rec.p_pmodno = op_rec->pi_mod;

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
  static char month[12][3] = 
    {"Jan", "Feb", "Mar",  "Apr", "May", "Jun",
     "Jul", "Aug", "Sep",  "Oct", "Nov", "Dec"};
  long now;
  long *t;                                /* t[4] = mon (0-11)               */
                                          /* t[3] = day (1-31)               */
                                          /* t[5] = year (70-???)            */

  time(&now);                            /* get time and break apart        */
  t = (long *)localtime(&now);
  
  sprintf(udate, "%02d/%02d/%02d", t[4]+1, t[3], t[5]);  /* mm/dd/yy         */
  sprintf(edate, "%02d/%02d/%02d", t[3], t[4]+1, t[5]);  /* dd/mm/yy         */
  sprintf(mdate, "%.3s %2d, %4d",                        /* mmm dd, yyyy     */
    month[t[4]], t[3], t[5]+1900); 

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
  memset(of_rec->of_grp , dvalue, GroupLength + CustomerNoLength);
   
  pick->of_mod      = 1;
  pick->of_quan     = 99;
  pick->of_short    = 0;

  memset(&sku_rec, dvalue, sizeof(prodfile_item));
  sku_rec.p_ipqty = 99;
  sku_rec.p_cpack = 99;

  memset(&pkm_rec, 0, sizeof(pmfile_item));
  pkm_rec.p_pmodno = 1;
  memset(pkm_rec.p_pmsku, dvalue, 15);
  memset(pkm_rec.p_stkloc, dvalue, 6);
      
  qi.paper_copies      = dcopies;
  qi.paper_pl          = 1;
  qi.paper_order       = 99999;
  qi.paper_box_number  = 9999;
   
  print_flag = 1;
  copies = dcopies;                       /* debug copies                    */

  do_dates();                             /* build date fields               */

  picks_to_go = dpicks;
  last_page = (picks_to_go + REPEAT - 1) / REPEAT;
  pick_no = 0;
   
  if (last_page < 1) last_page = 1;       /* always one page                 */

  before_request();                       /* preliminaries                   */

  while(copies > 0 && print_flag)         /* do until all copies printed     */
  {
    pkm_rec.p_pmodno = 1;

    for (page = 1, pick_no = 0; page <= last_page; page++)
    {
      set_base(1, 1);                     /* default to top corner           */

      if (page == 1) before_first_page();
      before_each_page();                 /* before picks formatted          */

      for (line = 0;line < REPEAT && pick_no < picks_to_go; line++, pick_no++)
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
    copies--;
  }
  if (daemon && fd)
  {
    fclose(fd); fd = 0;
    if (print_flag)
    {
      sprintf(command, "%s %s", getenv("LPR"), fd_name);
      system(command);
    }
  }
}

/* end of box_packing_list.c */
