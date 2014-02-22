/* #define SKIP_LAST_ZONE */
/*-------------------------------------------------------------------------*
 *  Custom Code:    SKIP_LAST_ZONE - Do not use last zone in each pickline.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Productivity reports.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/12/93   |  tjt  Added mfc.
 *  11/07/94   |  tjt  Add average productivity.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  03/02/95   |  tjt  Fix bug in pickline prompt.
 *  03/02/95   |  tjt  Fix elapsed less than zero.
 *  04/18/95   |  tjt  Revised extensively.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char productivity_c[] = "%Z% %M% %I% (%G% - %U%)";

/****************************************************************************/
/*                                                                          */
/*                          Productivity Screen                             */
/*                                                                          */
/****************************************************************************/
                           
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "productivity.t"
#include "pr.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "language.h"

extern leave();

#define NUM_PROMPTS   4
#define BUF_SIZE      9
#define WIDTH         80

char buf[NUM_PROMPTS][BUF_SIZE];

/*
 * Global Variables
 */
short ONE = 1;
short TWO = 2;
short LPL = 8;

struct fld_parms fld[] ={
  {6, 38,20,1,&ONE,"Enter Code           (C = Current, M = Cumulative)",'a'},
  {7, 38,20,1,&ONE,"Summary? (y/n)",'a'},
  {8, 38,20,1,&LPL,"Enter Pickline",'a'},
  {23,17,1,1,&ONE, "Print? (y/n)       (Exit, Forward, or Backward)",'a'}
};

FILE *fp;
char temp_file[16];                       /* print work file                 */

long ahead1,  ahead2,  ahead3;
long active1, active2, active3;
long order_cnt, lines_cnt, units_cnt, ahead_time, active_time, pickline;
long elapsed, elapsed1, completed;
long temp1, temp2, temp3;
long summary, current;
main()
{
  putenv("_=productivity");
  chdir(getenv("HOME"));
  
  getparms(0);
  
  open_all();

  while(1)
  {
    prompt();                             /* get parameters                  */

    if (!store_all()) continue;           /* get all data                    */

    print_prompt();                       /* show or print data              */
  }
}                                         /*end of main                      */
/*-------------------------------------------------------------------------*
 *  Prompt for first 3 questions
 *-------------------------------------------------------------------------*/
prompt()
{
  register long i, max;
  register unsigned char t;
  
  memset(buf, 0, NUM_PROMPTS * BUF_SIZE);

  fix(productivity);
  sd_screen_off();
  sd_clear_screen();
  sd_text(productivity);
  sd_screen_on();

  i = 0; max = 1;

  sd_prompt(&fld[0], 0);
  sd_prompt(&fld[1], 0);
  if (SUPER_OP && !IS_ONE_PICKLINE) 
  {
    sd_prompt(&fld[2], 0);
    max = 2;
  }
  while(1)
  {
    t = sd_input(&fld[i], 0, 0, buf[i], 0);

    if(t ==  EXIT) leave();
    if(t == UP_CURSOR && i > 0)     i--;
    if(t == DOWN_CURSOR && i < max) i++;
    else if(t == RETURN)
    {
      *buf[0] = tolower(*buf[0]);
      if (*buf[0] == 'c')     current = 1;
      else if(*buf[0] == 'm') current = 0;
      else
      {
        eh_post(ERR_CODE,buf[0]);
        i = 0;
        continue;
      }
      if (code_to_caps(*buf[1]) == 'y')      summary = 1;
      else if (code_to_caps(*buf[1]) == 'n') summary = 0;
      else
      {
        eh_post(ERR_YN,0);
        i = 1;
        continue;
      }
      if (SUPER_OP)
      { 
        if (check_pkln()) return 1;
        i = 2;
        continue;
      }
      pickline = op_pl;
      return 1;
    }
  }
}
/*-------------------------------------------------------------------------*
 *check the pickline
 *-------------------------------------------------------------------------*/
check_pkln()
{
  if (IS_ONE_PICKLINE) pickline = op_pl;
  else
  {
    pickline = pl_lookup(buf[2], 0);
  
    if (!pickline) return 1;

    if (pickline < 0)
    {
      eh_post(ERR_PL, buf[2]);
      return 0;
    }
    sprintf(buf[2], "%d", pickline);
    chng_pkln(buf[2]);
  }                             
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Get All DAta
 *-------------------------------------------------------------------------*/
store_all()
{
  register long i, found;

  tmp_name(temp_file);                    /* get a temporary file name       */
  fp = fopen(temp_file, "w+");

  if (fp == 0)
  {
    krash("store_data", "open tmp", 1);
  }
  found = 0;

  for (i = 0; i < pr->pr_picklines; i++)
  {
    if (!pl[i].pl_pl) continue;
    if (pickline && pickline != i+1) continue;
    
    if (found)                             /* after first pickline           */
    {
      fprintf(fp, "%79c\n", 0x20);
      if (!summary)
      {
        fprintf(fp, "%79c\n", 0x20);
        fprintf(fp, "%79c\n", 0x20);
        fprintf(fp, "%79c\n", 0x20);
      }
    }
    store_data(i);                         /* store one pickline             */
    found = 1;
  }
  if (!found) eh_post(ERR_PROD, 0);

  return found;
}
/*-------------------------------------------------------------------------*
 * Store Data For One Pickline
 *-------------------------------------------------------------------------*/
store_data(i)
register long i;
{
  register long j;
  
  completed = lines_cnt = units_cnt = ahead_time = active_time = 0;

  for (j = 0; j < pr->pr_zones; j++)        /* for all zones                 */
  {
    if (zone[j].zt_pl != i + 1) continue;

#ifdef SKIP_LAST_ZONE
    if (!zone[j].zt_feeding) continue;
#endif

    if(current) store_current_data(i, j);
    else store_cumulative_data(i, j);
  }
  active1 = active_time / 3600;
  active2 = (active_time - active1 * 3600) / 60;
  active3 = active_time  - active1 * 3600 - active2 * 60;
  
  if (active_time > 0)
  {
    temp1 = ((3600.0 * completed) / active_time) + 0.5;
    temp2 = ((3600.0 * lines_cnt) / active_time) + 0.5;
    temp3 = ((3600.0 * units_cnt) / active_time) + 0.5;  
  }
  else temp1 = temp2 = temp3 = 0;
  
  fprintf(fp,
 "  All Zones%7d    %8d   %8d                      %02d:%02d:%02d        \n",
    temp1, temp2, temp3, active1, active2, active3);

  active1 = elapsed / 3600;
  active2 = (elapsed - active1 * 3600) / 60;
  active3 = elapsed  - active1 * 3600 - active2 * 60;

  fprintf(fp,
 "  %-8.8s%8d    %8d   %8d                      %02d:%02d:%02d        \n",
    pl[i].pl_name, order_cnt, lines_cnt, units_cnt, active1, active2, active3);

  if (elapsed > 0)
  {
    temp1 = ((3600.0 * order_cnt) / elapsed) + 0.5;
    temp2 = ((3600.0 * lines_cnt) / elapsed) + 0.5;
    temp3 = ((3600.0 * units_cnt) / elapsed) + 0.5;  
  }
  else temp1 = temp2 = temp3 = 0;

  fprintf(fp,"  Pickline%8d    %8d   %8d%38c\n",
    temp1, temp2, temp3, ' ');

  return 0;
}                                         /* end store_data                  */
/*-------------------------------------------------------------------------*
 * Store Current Data
 *-------------------------------------------------------------------------*/
store_current_data(i, j)
register long i, j;
{
  elapsed  = pp[i].pr_pl_cur_elapsed;
  elapsed1 = pz[j].pr_zone_cur_active;

  if (elapsed1 < 0) elapsed1 = 0;

  active_time += elapsed1;
  ahead_time  += pz[j].pr_zone_cur_ahead;
  
  order_cnt = pp[i].pr_pl_cur_completed;

  completed   += pz[j].pr_zone_cur_orders;
  lines_cnt   += pz[j].pr_zone_cur_lines;
  units_cnt   += pz[j].pr_zone_cur_units;

  if (summary) return 0;

  ahead1    = pz[j].pr_zone_cur_ahead / 3600;
  ahead2    = (pz[j].pr_zone_cur_ahead - ahead1 * 3600) / 60;
  ahead3    = pz[j].pr_zone_cur_ahead - ahead1 * 3600 - ahead2 * 60;

  active1   = elapsed1 / 3600;
  active2   = (elapsed1 - active1 * 3600) / 60;
  active3   = elapsed1  - active1 * 3600 - active2 * 60;

/*       1         2         3         4         5         6         7
1234567890123456789012345678901234567890123456789012345678901234567890123456789
  Zone      Orders       Lines      Units
           Orders/hr    Lines/hr   Units/hr  Ahead Time   Active Picking Time
  xxx     xxxxxxxx    xxxxxxxx   xxxxxxxx     xx:xx:xx         xx:xx:xx
..   ..... 5       .... 4      ... 3       ..... 5      ......... 9    ........
*/

  fprintf(fp,
 "  %3d     %8d    %8d   %8d     %02d:%02d:%02d         %02d:%02d:%02d%8c\n",
      j + 1, pz[j].pr_zone_cur_orders, pz[j].pr_zone_cur_lines,
      pz[j].pr_zone_cur_units,
      ahead1, ahead2, ahead3, active1, active2, active3, 0x20);

  if(elapsed1 > 0)
  {
    temp1 = ((3600.0 * pz[j].pr_zone_cur_orders) / elapsed1) + 0.5;
    temp2 = ((3600.0 * pz[j].pr_zone_cur_lines)  / elapsed1) + 0.5;
    temp3 = ((3600.0 * pz[j].pr_zone_cur_units)  / elapsed1) + 0.5;
  }
  else  temp1 = temp2 = temp3 = 0;

  fprintf(fp,"%10c%8d    %8d   %8d%38c\n%79c\n",
    ' ', temp1, temp2, temp3, ' ', ' ');

  return 0;
}                                         /* end store current data          */
/*-------------------------------------------------------------------------*
 * Store Cumulative Data 
 *-------------------------------------------------------------------------*/
store_cumulative_data(i, j)
register long i, j;
{
  elapsed  = pp[i].pr_pl_cum_elapsed;
  elapsed1 = pz[j].pr_zone_cum_active;

  if (elapsed1 < 0) elapsed1 = 0;

  active_time += elapsed1;
  ahead_time  += pz[j].pr_zone_cum_ahead;
  
  order_cnt = pp[i].pr_pl_cum_completed;

  completed   += pz[j].pr_zone_cum_orders;
  lines_cnt   += pz[j].pr_zone_cum_lines;
  units_cnt   += pz[j].pr_zone_cum_units;

  if (summary) return 0;

  ahead1    = pz[j].pr_zone_cum_ahead / 3600;
  ahead2    = (pz[j].pr_zone_cum_ahead - ahead1 * 3600) / 60;
  ahead3    = pz[j].pr_zone_cum_ahead - ahead1 * 3600 - ahead2 * 60;

  active1   = elapsed1 / 3600;
  active2   = (elapsed1 - active1 * 3600) / 60;
  active3   = elapsed1  - active1 * 3600 - active2 * 60;

/*       1         2         3         4         5         6         7
1234567890123456789012345678901234567890123456789012345678901234567890123456789
  Zone      Orders       Lines      Units
           Orders/hr    Lines/hr   Units/hr  Ahead Time   Active Picking Time
  xxx     xxxxxxxx    xxxxxxxx   xxxxxxxx     xx:xx:xx         xx:xx:xx
..   ..... 5      .... 4      ... 3       ..... 5      ......... 9    ........
..........xxxxxxxx....xxxxxxxx...xxxxxxxx
*/

  fprintf(fp,
 "  %3d     %8d    %8d   %8d     %02d:%02d:%02d         %02d:%02d:%02d%8c\n",
      j + 1, pz[j].pr_zone_cum_orders, pz[j].pr_zone_cum_lines,
      pz[j].pr_zone_cum_units,
      ahead1, ahead2, ahead3, active1, active2, active3, 0x20);

  if(elapsed1 > 0)
  {
    temp1 = ((3600.0 * pz[j].pr_zone_cum_orders) / elapsed1) + 0.5;
    temp2 = ((3600.0 * pz[j].pr_zone_cum_lines)  / elapsed1) + 0.5;
    temp3 = ((3600.0 * pz[j].pr_zone_cum_units)  / elapsed1) + 0.5;
  }
  else  temp1 = temp2 = temp3 = 0;

  fprintf(fp,"%10c%8d    %8d   %8d%38c\n%79c\n",
    ' ', temp1, temp2, temp3, ' ', ' ');

  return 0;
}                                         /* end store cumulative data       */
/*
**store the heading in the file
*/
store_heading()
{
char text[128];

/*       1         2         3         4         5         6         7
1234567890123456789012345678901234567890123456789012345678901234567890123456789
  Zone      Orders       Lines      Units
           Orders/hr   Lines/hr   Units/hr   Ahead Time   Active Picking Time
  xxx     xxxxxxxx   xxxxxxxx   xxxxxxxx      xx:xx:xx        xx:xx:xx
*/

  sprintf(text, "  Zone      Orders       Lines      Units%38c", 0x20);
  sd_cursor(0, 6, 1);
  sd_text(text);
  
  sprintf(text,
  "%11cOrders/hr   Lines/hr   Units/hr   Ahead Time   Active Picking Time",
    0x20);
    
  sd_cursor(0, 7, 1);
  sd_text(text);
  
  return 0;
}
/*-------------------------------------------------------------------------*
 * Display Print Prompt
 *-------------------------------------------------------------------------*/
print_prompt()
{
  unsigned char t;

  store_heading();
  fseek(fp, 0, 0);
  sd_cursor(0, 8, 1);
  show(fp,15,1);

  while(1)
  {
    sd_prompt(&fld[3], 0);
    t = sd_input(&fld[3], 0, 0, buf[3], 0);

    switch(sd_print(t, code_to_caps(*buf[3])))  /* F041897 */
    {
      case(0): leave();

      case(1): sd_cursor(0,8,1);
               show(fp,15,1);
               break;

      case(2): sd_cursor(0,8,1);
               show(fp,15,2);
               break;
               
      case(3): fclose(fp); fp = 0;
               unlink(temp_file);
               return;

      case(4): fclose(fp);
               print_all();
               return;

      case(6): eh_post(ERR_YN,0);
               break;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Print The Information
 *-------------------------------------------------------------------------*/
print_all()
{
  long pid, stat;
  char print_file[16];
  
  if (fork() == 0)                      /* if child process                */
  {
    close_all();

    if (current)
    {
      execlp("prft", "prft", temp_file,
        tmp_name(print_file),"sys/report/productivity_rpt.h",
        "Current Period",0);
      krash("main", "prft load", 0);
      exit(1);
    }
    else
    {
      execlp("prft","prft",temp_file,
        tmp_name(print_file),"sys/report/productivity_rpt.h",
        "Cummulative Period",0);
      krash("main", "prft load", 0);
      exit(1);
    }
  }
  pid = wait(&stat);
  if (pid < 0 || stat) krash("print_all", "prft failed", 1);
  return 0;
}
/*
 *function to display x number of lines of data on the screen               
 * Arguments:                                                               
 *           fp : the data file pointer.                                    
 *           lines : the number of lines to be displayed.                   
 *           i : the indicator of either going forward or                   
 *           backward on the file.                                          
 *                                                                          
 */
show (fp, lines, index)
FILE *fp;
short lines,index;
{
  register long pos, size;
  static long savefp;
  char str[1920];

  memset(str, 0, 1920);
  pos = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);
  
#ifdef DEBUG
  fprintf(stderr, "show(%x, %d, %d)\n", fp, lines, index);
  fprintf(stderr, "pos=%d  size=%d\n", pos, size);
#endif

  if (index == 2)
  {
    pos = savefp - lines * WIDTH;
    if (pos < 0) pos = 0;
    savefp = pos;

    fseek(fp, pos, 0);                    /* position the file pointer       */
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
#ifdef DEBUG
    fprintf(stderr, "pos=%d savefp=%d\n", pos, savefp);
    Bdump(str, 1920);
#endif
    return;
  }
  else if (index == 1)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;
  
    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
#ifdef DEBUG
    fprintf(stderr, "pos=%d savefp=%d\n", pos, savefp);
    Bdump(str, 1920);
#endif
    return;
  }
}                                         /*end of show                      */
/*
 ** open all files
 */
open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  pr_open();
}
/*
**close all files
*/
close_all()
{
  ss_close();
  co_close();
  pr_close();
}
/*
**transfer control back to calling program
*/
leave()
{
  close_all();
  if (fp)
  {
    fclose(fp);
    unlink(temp_file);
  }
  sd_close();
  execlp("pnmm", "pnmm", 0);
  krash("pnmm failed to load");
}


/* end of productivity.c */
