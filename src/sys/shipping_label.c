/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Shipping list printer daemon.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/27/93   |  tjt  Added to mfc.
 *  08/09/94   |  tjt  Added putenv and chdir.
 *  11/24/94   |  tjt  Added order and print count.
 *  02/02/95   |  tjt  Remove UNOS queues.
 *  06/14/95   |  tjt  Fix aplace copy.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  04/19/96   |  tjt  Add printed count.
 *-------------------------------------------------------------------------*/
static char shipping_label_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Prints Shipping Labels
 *
 *  To Execute:            shipping_label <printer>  &
 *
 *  To Test Report Layout: shipping_label -d [-copies=n] [-picks=m] [-value=x]
 *                              
 *                             -d indicates report debug mode
 *                             -copies=n indicates 'n' copies
 *                             -picks=m  indicates 'm' picks
 *                             -value=x  indicates 'x' as alpha fill byte
 *-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include "file_names.h"
#include "ss.h"
#include "of.h"
#include "box.h"
#include "Bard.h"

#define DELAY  15

/*------------------------------------------------------------------------*
 *  Specialized Header Files For Shipping Labels
 *------------------------------------------------------------------------*/

#include "custom/labels/remarks_text.h"
#include "custom/labels/shipping_defines.h"

/*------------------------------------------------------------------------*
 *  Input Queue Of Print Requests
 *------------------------------------------------------------------------*/

paper_item qi;                            /* queue request item              */

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

/*-------------------------------------------------------------------------*
 *  Output File And Misc Items
 *-------------------------------------------------------------------------*/
 
FILE *fd;                                 /* output file descriptor          */
char fd_name[40];                         /* output file name                */
char printer[40];                         /* system printer name             */
char printer_env[64];                     /* system printer environemnt      */
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

/*-------------------------------------------------------------------------*
 *  Specialized Header File For Shipping Labels
 *-------------------------------------------------------------------------*/
 
#include "custom/labels/shipping_label.h"

/*-------------------------------------------------------------------------*
 *   M A I N
 *-------------------------------------------------------------------------*/
main(argc, argv)
long  argc;
char  **argv;
{
  void leave(int signum); // Signal handler
  register long k;
  register char *p;

  putenv("_=shipping_label");
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
      strcpy(printer, argv[k]);           /* is a printer daemon             */
      sprintf(printer_env, "PRINTER=%s", printer);
      putenv(printer_env);
      daemon = 1;                         /* use printer daemon              */
    }
  }
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
  database_open();
  ss_open();
  oc_open();
  od_open();
  co_open();
/*
 * Setup Pointers
 */

  rmks   = (remarks_text_item *)  or_rec->rmks_text;

/*
 * Open Print Request Queue
 */

  ship_label_open(AUTOLOCK);
  ship_label_setkey(1);

/*-------------------------------------------------------------------------*
 *   M A I N   L O O P
 *-------------------------------------------------------------------------*/
  signal(SIGTERM, leave);                 /* terminate on shutdown           */

  set_size(1);                            /* setup default letter size       */

  image = (char *)malloc(LENGTH * WIDTH); /* print page work image           */
  memset(image, 0x20, WIDTH * LENGTH);    /* clear image initially           */
   
  if (debug) {do_debug(); leave(0);}

  while (1)
  {
    begin_work();
    if (ship_label_next(&qi, LOCK))
    {
      commit_work();
      sleep(DELAY);                       /* empty - wait and try again      */
      ship_label_setkey(1);
      continue;
    }
    ship_label_delete();
    commit_work();
    
    print_flag = 1;                       /* set print flag is true          */
    copies = 1;                           /* default copies                  */

    if (qi.paper_copies) copies = qi.paper_copies;

    if (qi.paper_pl == 0)                 /* marks end of a batch            */
    {
      print_all();                        /* blank label                     */
      continue;
    }
    block = oc_find(qi.paper_pl, qi.paper_order);

    if (block <= 0) continue;             /* no order record found ???       */
         
    of_rec->of_pl = qi.paper_pl;
    of_rec->of_on = qi.paper_order;
    if (order_read(of_rec, NOLOCK)) continue;
      
    if (remarks_fd)
    {
      or_rec->rmks_pl = qi.paper_pl;
      or_rec->rmks_on = qi.paper_order;
      if (remarks_read(or_rec, NOLOCK)) memset(rmks, 0x20, rf->rf_rmks);
    }
    sp->sp_sl_print    = qi.paper_order;
    sp->sp_sl_printed += 1;

    format_report();
  }
  return;
}
/*------------------------------------------------------------------------*
 *  Terminate On Shutdown
 *------------------------------------------------------------------------*/
void leave(int signum)
{
  printf("\rShipping Labels Stopping On Shutdown\n");
  ss_close();
  oc_close();
  od_close();
  co_close();
  ship_label_close();
  if (!daemon) fclose(fd);
  database_close();
  exit(0);
}
/*------------------------------------------------------------------------*
 *  Format Report - Do All Copies For One Queue Request
 *------------------------------------------------------------------------*/
format_report()
{
  register long picks_to_go;

  if (daemon)
  {
    tmp_name(fd_name);                    /* get a file name                 */
    fd = fopen(fd_name, "w");
  }
  do_dates();                             /* build date fields               */

  before_request();                       /* any preliminary processing      */

  while(copies > 0 && print_flag)         /* do until all copies printed     */
  {
    set_base(1, 1);                       /* default to top corner           */

    before_each_page();                   /* before picks formatted          */
    print_all();
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
 *  Format three dates
 *-------------------------------------------------------------------------*/
do_dates()
{
  static char month[12][3] = {"Jan", "Feb", "Mar",
    "Apr", "May", "Jun",
    "Jul", "Aug", "Sep",
    "Oct", "Nov", "Dec"};
  long now;
  long *t;                                   /* t[4] = mon (0-11)            */
                                             /* t[3] = day (1-31)            */
                                             /* t[5] = year (70-??)          */

  time(&now);                             /* get time and break apart        */
  t = (long *)localtime(&now);
  
  sprintf(udate, "%02d/%02d/%02d", t[4]+1, t[3], t[5]);     /* mm/dd/yy      */
  sprintf(edate, "%02d/%02d/%02d", t[3], t[4]+1, t[5]);     /* dd/mm/yy      */
  sprintf(mdate, "%.3s %2d, %4d",                           /* mmm dd, yyyy  */
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
  of_rec->of_on        = 99999;
  of_rec->of_datetime  = time(0);
  of_rec->of_no_picks  = dpicks;
  of_rec->of_pl        = 1;
  of_rec->of_pri       = 'h';
  of_rec->of_status    = 'c';
  memset(of_rec->of_grp, dvalue, GroupLength + CustomerNoLength);
   
  qi.paper_copies   = dcopies;
  qi.paper_pl       = 1;
  qi.paper_order    = 99999;
   
  print_flag = 1;
  copies = dcopies;                       /* debug copies                    */

  do_dates();                             /* build date fields               */

  before_request();                       /* preliminaries                   */

  while(copies > 0 && print_flag)         /* do until all copies printed     */
  {
    set_base(1, 1);                       /* default to top corner           */
    before_each_page();                   /* before picks formatted          */
    print_all();
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

/* end of shipping_list.c */
