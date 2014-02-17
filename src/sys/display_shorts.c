/*#define DEBUG */
/* #define CANTON */
/*-------------------------------------------------------------------------*
 *  Custom Versions: CANTON
 *                   HANES
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Display order shorts.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  01/23/94   |  tjt Original Implementation.
 *  05/13/94   |  tjt Fix order numbers 1..6 digits right justified.
 *  05/25/94   |  tjt Add ignore pick text.
 *  07/13/94   |  tjt Fix bug in Hanes version (sort == 'y').
 *  07/28/94   |  tjt Fix order numbers 1..7 digits right justified.
 *  09/08/94   |  tjt Fix bugs in 'x' and 'y' options.
 *  11/18/94   |  tjt Add hanes default to 'y' option.
 *  01/23/95   |  tjt Add  new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt Add pickline input by name.
 *  06/04/95   |  tjt Add sp_pl_by_name.
 *  07/21/95   |  tjt Revise Bard calls.
 *  08/23/96   |  tjt Add begin and commit work.
 *  05/17/99   |  aha Modified system sort command call to include the "-t"
 *             |      parameter as used at Eckerd-Charlotte RX site to fix
 *             |      sorting.  Also added local flag variable, fd_name_flag,
 *             |      to the get_picks() function to keep track of which
 *             |      file has the final sort results.
 *  05/24/99   | aha  Restored file to MFC Version 6.3.2 by eliminating
 *             |      tjt's 04/18/97 fixes of "include language.h" and use of
 *             |      code_to_caps() function.
 *-------------------------------------------------------------------------*/
#ident "(C) Kingway Material Handling Company %M% %H%"
static char display_shorts_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global_types.h"
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "display_shorts.t"
#include "eh_nos.h"
#include "co.h"
#include "of.h"
#include "st.h"

#include "Bard.h"
#include "bard/pmfile.h"

pmfile_item pm;

#define WIDTH 80

#define PLEN 8

short LPL  = PLEN;
short ONE  = 1;
short LMOD = 5;
short LSKU = 15;

struct fld_parms fld1 = { 6,26,1,1, &LPL,  "Enter Pickline",'a'};
struct fld_parms fld2 = { 7,26,1,1, &LSKU, "Enter SKU", 'a'};
struct fld_parms fld3 = { 8,26,1,1, &LMOD, "Enter Module", 'n'};
struct fld_parms fld4 = { 9,26,1,1, &ONE,  "Sort By (s,m,o)", 'a'};
struct fld_parms fld5 = {23,16,1,1, &ONE,  "Print? (y/n)",'a'};

char buf1[PLEN +1]  = {0};
char buf2[16] = {0};
char buf3[6]  = {0};
char buf4[2]  = {0};
char buf5[2]  = {0};

long pickline = 0;

FILE *fp = 0;
char ret_name[40];
long delete = 1;                          /* delete work file                */
char temp_file[16],print_file[16];        /* temporary file name             */
char work_file[16];
#ifdef DEBUG
FILE *debug;
#endif

long savefp = 0;

main(argc,argv)
short argc;
char **argv;
{
  extern leave();
  extern unsigned short get_parms();

  putenv("_=display_shorts");
  chdir(getenv("HOME"));

#ifdef DEBUG
   debug = fopen ( "/u/mfc/debug/display_shorts", "w");
#endif
  open_all();

  tmp_name(temp_file);                    /* get a temporary file name       */
  tmp_name(work_file);

  LSKU = rf->rf_sku;
  LMOD = rf->rf_mod;

  fix(display_shorts);
  sd_screen_off();
  sd_clear_screen();
  sd_text(display_shorts);
  sd_screen_on();

  while(1)
  {
    get_parms();
    get_picks();
    show_picks();
  }
}
/*-------------------------------------------------------------------------*
 *  Get Pickline and Order Number
 *-------------------------------------------------------------------------*/
unsigned short get_parms()
{
  unsigned char t;

  sd_cursor(0, 6, 1);
  sd_clear_rest();

  memset(buf1, 0, sizeof(buf1));          /* clear pickline                  */
  memset(buf2, 0, sizeof(buf2));          /* clear sku number                */
  memset(buf3, 0, sizeof(buf3));          /* clear module                    */
  memset(buf4, 0, sizeof(buf4));          /* sort order                      */

#ifdef HANES
  *buf4 = 'y';
#endif

  if (rf->rf_sku) sd_prompt(&fld2, 0);    /* prompt for sku                  */
  sd_prompt(&fld3, 0);                    /* prompt for module               */
  sd_prompt(&fld4, 0);                    /* prompt for sort order           */
  sd_cursor(0, 9, 40);
#ifdef CANTON
  sd_text("(Sku, Module, Order#, else by ModSlot)");
#else
  sd_text("(S=Sku  M=Module  O=Order  L=StkLoc)");
#endif
  sd_cursor(0, 10, 40);
  sd_text("(X=Subtotal by Module)");
  sd_cursor(0, 11, 40);
  sd_text("(Y=Summary  by Module)");

  while(1)
  {
    if (IS_ONE_PICKLINE) pickline = op_pl;
    else if (!SUPER_OP)  pickline = op_pl;
    else
    {
      sd_prompt(&fld1, 0);

      while(1)
      {
        t = sd_input(&fld1,0, 0, buf1, 0);
        if(t == EXIT) leave();
        if (*buf1)
        {
          pickline = pl_lookup(buf1, 0);

          if (!pickline) break;

          if (pickline < 0)
          {
            eh_post(ERR_PL, buf1);
            continue;
          }
          sprintf(buf1, "%d", pickline);
          chng_pkln(buf1);               /* change pickline on the screen   */
        }
        break;
      }                                   /* end while(1)pickline            */
    }                                     /* end SUPER OP                    */
    while(1)
    {
      if (rf->rf_sku)
      {
        t = sd_input(&fld2, 0, 0, buf2,0);  /* sku                           */
        if (t == EXIT) leave();
        if (t == UP_CURSOR) break;
      }
      while(1)
      {
        t = sd_input(&fld3, 0, 0, buf3, 0);  /* module                       */
        if (t == EXIT) leave();
        if (t == UP_CURSOR) break;

        while(1)
        {
          t = sd_input(&fld4, 0, 0, buf4,0);  /* sort order                  */
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;

          *buf4 = tolower(*buf4);
          if (!*buf4) *buf4 = 'l';

          if (*buf4 != 's' && *buf4 != 'm' && *buf4 != 'o' &&
              *buf4 != 'x' && *buf4 != 'y' && *buf4 != 'l')
          {
            eh_post(ERR_CODE, buf4);
            continue;
          }
          if (t == RETURN) return 0;
        }                                 /* end while(1)sort                */
      }                                   /* end while(1)module              */
    }                                     /* end while(1)sku number          */
  }                                       /* end while(1)parms               */
}
/*-------------------------------------------------------------------------*
 *  Get Short Pick Information
 *-------------------------------------------------------------------------*/
get_picks()
{
  FILE *fd;
  register long mod, lines, items;
  long otot, ptot, stot;
  long pid, stat, work1, work2, work3;
  char ibuf[80], last[80], work[8], fd_name[16], command[80];
  unsigned short int fd_name_flag;

  sd_wait();

  lines = items = 0;

  fd_name_flag = 0;    /* aha - means work_file is the default */
                       /*       results file unless fd_name is */
                       /*       used further down.             */

  if (fp) fclose(fp);

  fp = fopen(temp_file,"w+");

  if (fp == 0) krash("get_picks", "tmp file", 1);

  mod = atol(buf3);                          /* F071394 */

  pick_setkey(0);                         /* read in physical order          */
  if (rf->rf_sku > 0) pmfile_setkey(1);
  order_setkey(1);

  memset(&pm, 0x20, sizeof(pmfile_item));

  begin_work();
  while (!pick_next(op_rec, NOLOCK))
  {
    if (pickline && op_rec->pi_pl != pickline) continue;
    if (!(op_rec->pi_flags & PICKED)) continue;
    if (op_rec->pi_ordered == op_rec->pi_picked) continue;

    if (*buf2)
    {
      if (memcmp(buf2, op_rec->pi_sku, rf->rf_sku) != 0) continue;
    }
    if (mod && mod != op_rec->pi_mod) continue;

    of_rec->of_pl = op_rec->pi_pl;
    of_rec->of_on = op_rec->pi_on;

    if (order_read(of_rec, NOLOCK))
    {
      commit_work();
      begin_work();
      continue;
    }
    if (rf->rf_sku > 0)
    {
      pm.p_pmodno = op_rec->pi_mod;
      if (pmfile_read(&pm, NOLOCK)) continue;
    }
    else memset(&pm, 0x20, sizeof(pm));

    lines += 1;
    items += (op_rec->pi_ordered - op_rec->pi_picked);

/*-------------------------------------------------------------------------*
         1         2         3         4         5         6         7
1...5....0....5....0....5....0....5....0....5....0....5....0....5....0....5...
Pickline  Order  SKU             Module  Stkloc  Ordered  Picked   Short  Load
--------  -----  --------------- ------  ------  -------  ------   -----  ----
   xx   xxxxxxx  xxxxxxxxxxxxxxx xxxxx   xxxxxx     xxxx    xxxx    xxxx  xxxx
 *-------------------------------------------------------------------------*/

    if (sp->sp_pl_by_name == 'y' && pl[op_rec->pi_pl - 1].pl_pl)
    {
      fprintf(fp, "%-8.8s%7.*d  %-15.15s %5d   %6.6s     ",
        pl[op_rec->pi_pl - 1].pl_name, rf->rf_on, op_rec->pi_on,
        op_rec->pi_sku, op_rec->pi_mod, pm.p_stkloc);
    }
    else
    {
      fprintf(fp, "   %2d   %7.*d  %-15.15s %5d   %6.6s     ",
        op_rec->pi_pl, rf->rf_on, op_rec->pi_on,
        op_rec->pi_sku, op_rec->pi_mod, pm.p_stkloc);
    }

#ifdef CANTON
    fprintf(fp, "%4d    %4d    %4d  %4.4s \n",
      op_rec->pi_ordered, op_rec->pi_picked,
      op_rec->pi_ordered - op_rec->pi_picked, of_rec->of_con);
#else
    fprintf(fp, "%4d    %4d    %4d       \n",
      op_rec->pi_ordered, op_rec->pi_picked,
      op_rec->pi_ordered - op_rec->pi_picked);
#endif
    commit_work();
    begin_work();
  }
  commit_work();
  fclose(fp);
#ifdef DEBUG
  fprintf(debug, "value of *buf is %c\n", *buf4);
  fflush(debug);
#endif

  if (*buf4 == 's')                     /* sort by sku, then pl + order */
  {
    sprintf(command, "sort -t*  +0.17 -0.33 +0.0 -0.15 -o %s %s",
      work_file, temp_file);
  }
  else if (*buf4 == 'm' ||              /* sort by mod, then pl + order */
           *buf4 == 'x' || *buf4 == 'y')
  {
    sprintf(command, "sort -t* +0.33 -0.38 +0.0 -0.15 -o %s %s",
      work_file, temp_file);
  }
  else if (*buf4 == 'o')                /* sort order + sku */
  {
    sprintf(command, "sort -t* +0.8 -0.15 +0.17 -0.33 -o %s %s",
      work_file, temp_file);
  }
  else                                   /* sort by stkloc + pl + order */
  {
    sprintf(command, "sort -t* +0.41 -0.48 +0.0 -0.15 -o %s %s",
      work_file, temp_file);
  }
#ifdef DEBUG
  fprintf(debug, "command = %s\n", command);
  fflush(debug);
#endif
  if ( system(command))
  {
#ifdef DEBUG
     fprintf(debug, "error in executing sort\n");
     fflush(debug);
#endif
  }
  if ((*buf4 == 'x' || *buf4 == 'y') && lines > 1)
  {
    tmp_name(fd_name);

    fd_name_flag = 1;      /* aha - means using fd_name file as the */
                           /*       results file, otherwise its     */
                           /*       still work_file.                */

    fd = fopen(fd_name, "w");
    fp = fopen(work_file, "r");

    fread(last, 80, 1, fp);
    if (*buf4 == 'x') fwrite(last, 80, 1, fd);
    lines = 0;

    sscanf(last + 52, "%d", &otot);                     /* F090894  */
    sscanf(last + 60, "%d", &ptot);
    sscanf(last + 68, "%d", &stot);

    while (fread(ibuf, 80, 1, fp) == 1)
    {
      if (memcmp(ibuf + 33, last + 33, 5) != 0)         /* F090894  */
      {
        if (lines || *buf4 == 'y')
        {
          if (*buf4 == 'x') memset(last, 0x20, 50);     /* F080994   */
          else memset(last, 0x20, 15);
          fwrite(last, 80, 1, fd);
        }
        if (*buf4 == 'x') fwrite(ibuf, 80, 1, fd);
        memcpy(last, ibuf, 80);
        sscanf(last + 52, "%d", &work1);                 /* F090894  */
        sscanf(last + 60, "%d", &work2);
        sscanf(last + 68, "%d", &work3);
        otot += work1;
        ptot += work2;
        stot += work3;
        lines = 0;
        continue;
      }
      if (*buf4 == 'x') fwrite(ibuf, 80, 1, fd);
      lines++;

      sscanf(ibuf + 52, "%d", &work1);
      sscanf(last + 52, "%d", &work2);
      sprintf(work, "%6d", work1 + work2);
      memcpy(last + 50, work, 6);
      otot += work1;

      sscanf(ibuf + 60, "%d", &work1);
      sscanf(last + 60, "%d", &work2);
      sprintf(work, "%6d", work1 + work2);
      memcpy(last + 58, work, 6);
      ptot += work1;

      sscanf(ibuf + 68, "%d", &work1);
      sscanf(last + 68, "%d", &work2);
      sprintf(work, "%6d", work1 + work2);
      memcpy(last + 66, work, 6);
      stot += work1;
    }
    if (lines || *buf4 == 'y')
    {
      if (*buf4 == 'x') memset(last, 0x20, 50);    /* F090894 */
      else memset(last, 0x20, 15);
      fwrite(last, 80, 1, fd);
    }
    fprintf(fd, "Grand Total:%38c%6d%8d%8d        \n",0x20, otot, ptot, stot);
    fclose(fp);
    fclose(fd);
    unlink(temp_file);
    unlink(work_file);
  }

  /* Find out which file has the final results in it and have */
  /* temp_file point to it to carry over over to show_picks().*/
  if (fd_name_flag == 1)
     {
       strcpy(temp_file, fd_name);
     }
  else
     {
       strcpy(temp_file, work_file);
     }

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Show Picks On Screen
 *-------------------------------------------------------------------------*/
show_picks()
{
  unsigned char t;
  long pid, stat;

  sd_cursor(0, 6, 1);
  sd_clear_rest();

  sd_cursor(0, 6,  1);
#ifdef CANTON
  sd_text("Pickline  Order  SKU             Module  ModSlot");
  sd_cursor(0, 6, 50);
  sd_text("Ordered  Picked   Short  Load");

  sd_cursor(0, 7, 1);
  sd_text("--------  -----  --------------- ------  -------");
  sd_cursor(0, 7, 50);
  sd_text("-------  ------   -----  ----");

#else
  sd_text("Pickline  Order  SKU             Module  StkLoc ");
  sd_cursor(0, 6, 50);
  sd_text("Ordered  Picked   Short");

  sd_cursor(0, 7, 1);
  sd_text("--------  -----  --------------- ------  -------");
  sd_cursor(0, 7, 50);
  sd_text("-------  ------   -----");
#endif

  sd_cursor(0,8,1);

  fp = fopen(temp_file, "r");
  show(fp,15,2);                          /* display data     */
#ifdef DEBUG
  fprintf(debug,"\n right now the values for work :%s and temp :%s",work_file,te
mp_file);
#endif
  memset(buf4, 0, 2);

  while(1)
  {
    sd_cursor(0,23,30);
    sd_text("(Exit, Forward, or Backward)");
    t = sd_input(&fld5, sd_prompt(&fld5,0), 0, buf5, 0);
    switch(sd_print(t, *buf5))
    {
      case(0) : leave();
      break;
      case(1) : sd_cursor(0,8,1);
      show(fp,15,1);                      /* show the next set of data       */
      break;
      case(2) : sd_cursor(0,8,1);
      show(fp,15,2);                      /* show the previous set of data   */
      break;
      case(3) : return;
      break;
      case(4) :                           /*   Print the data and leave      */
      if(fork() == 0)                     /* if child pprocess               */
      {
#ifdef CANTON
        execlp("prft", "prft", temp_file,
          tmp_name(print_file), "sys/report/display_load.h", 0);
#else
        execlp("prft", "prft", temp_file,
          tmp_name(print_file), "sys/report/display_shorts.h", 0);
#endif
        krash("show_picks", "prft load", 1);
        exit(1);
      }
      pid = wait(&stat);
      if (pid < 0 || stat) krash("show_picks", "prft failed", 1);
      delete = 0;
      leave();

      case(6) : eh_post(ERR_YN,0);
      break;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  if (fp)
  {
    fclose(fp);
    if (delete) unlink(temp_file);
  }
#ifdef DEBUG
  if (debug)
  {
    fclose(debug);
  }
#endif
  close_all();
  sd_close();
  execlp("operm",  "operm", 0);
  krash("leave", "load operm", 1);
}
/*--------------------------------------------------------------------------*
 * Open / Close All Files
 *--------------------------------------------------------------------------*/
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  oc_open();
  od_open();
  co_open();
  if (rf->rf_sku > 0) pmfile_open(READONLY);
  getparms(0);
}
close_all()
{
  if (rf->rf_sku > 0) pmfile_close();
  co_close();
  ss_close();
  oc_close();
  od_close();
  database_close();

  return 0;
}
/****************************************************************************/
/*function to display x number of lines of data on the screen               */
/* Arguments:                                                               */
/*           fp : the data file pointer.                                    */
/*           lines : the number of lines to be displayed.                   */
/*           i : the indicator of either going forward or                   */
/*           backward on the file.                                          */
/*                                                                          */
/* returns : 1 if successfull                                               */
/*           0 if failed                                                    */
/****************************************************************************/

show(fp, lines, index)
FILE *fp;
short lines,index;
{
  long pos, size;
  char str[1920];
  short j;

  memset(str, 0, 1920);

  pos = ftell(fp);
  fseek(fp, 0, 2);
  size  = ftell(fp);

  if(index == 2)
  {
    pos = savefp - lines * WIDTH;
    if(pos < 0) pos = 0;
    savefp = pos;

    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
  }
  else if(index == 1)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;

    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
  }
  else return(0);
}
/*--------------------------------------------------------------------------*
 * print the module in the file
 *--------------------------------------------------------------------------*/
print(module, zne)
long module, zne;
{
  static long last_zone = 9999;
  static long n = 0;
  static char p_text[3][PickTextLength + 1];

  if(zne != last_zone || n == 3)
  {
    switch (n)
    {
      case 1: fprintf(fp,"%50c\n", ' ');  break;
      case 2: fprintf(fp,"%25c\n", ' ');  break;
      case 3: fprintf(fp,"\n");         break;
    }
    if (rf->rf_pick_text && n && rf->rf_ignore_pick_text != 'y')
    {
      fprintf(fp, "     %-24.24s %-24.24s %-24.24s\n",
      p_text[0], p_text[1], p_text[2]);

      p_text[0][0] = p_text[1][0] = p_text[2][0] = 0;
    }
    n = 0; last_zone = 9999;

    if (module == -1 && zne == -1) return;/* last call                       */

    if(zne > 0) fprintf(fp,"%3d ", zne);   /* write the zone                 */
    else fprintf(fp,"  ? ");               /* write ?                        */
  }
  if (rf->rf_sku)
  {
    fprintf(fp, " %-15.15s", op_rec->pi_sku);
  }
  else
  {
    fprintf(fp,"%5d           ", op_rec->pi_mod);
  }
  if (op_rec->pi_flags & PICKED)
  {
    fprintf(fp, "%4d%c%4d", op_rec->pi_ordered,
      (op_rec->pi_flags & NO_PICK) ? '*' : ' ',
      op_rec->pi_ordered - op_rec->pi_picked);
  }
  else
  {
    fprintf(fp, "%4d%c    ", op_rec->pi_ordered,
      (op_rec->pi_flags & NO_PICK) ? '*' : ' ');
  }
  if (rf->rf_pick_text && rf->rf_ignore_pick_text != 'y')
  {
    memcpy(p_text[n], op_rec->pi_pick_text, rf->rf_pick_text);
    p_text[n][rf->rf_pick_text] = 0;
  }
  n++;
  last_zone = zne;
}

/* end of display_shorts.c */
