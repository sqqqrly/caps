/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:.   Manpower requirements input screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/27/93   |  tjt  Added to mfc.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  07/05/95   |  tjt  Fix directory name.
 *  07/14/96   |  tjt  Fix display of Hrs:Min
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *  07/24/01   |  aha  Modified to use sections per Eckerd's MP & ZB spec.
 *-------------------------------------------------------------------------*/
static char manpower_input_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/****************************************************************************/
/*                                                                          */
/*                      Manpower Requirement Input Screen                   */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "file_names.h"
#include "manpower_input.t"
#include "eh_nos.h"
#include "language.h"

#define NUM_PROMPTS     7
#define BUF_SIZE        33

/*
 * Global Variables
 */

short ONE = 1;
short TWO = 2;
short FIVE = 5;
short THIRTY_TWO = 32;
short LPL = 8;

struct fld_parms fld[] ={
  { 5,48,3,1,&LPL, "Enter Pickline",'a'},
  { 6,48,3,1,&FIVE,"Enter Time Interval (hrs)",'a'},
  { 7,48,3,1,&FIVE,"Number of Orders",'n'},
  { 8,48,3,1,&ONE, "Use underway orders (y/n) ?",'a'},
  { 9,48,3,1,&ONE, "Use Current Productivity Rates (y/n) ?", 'a'},
  {10,48,3,1,&ONE, "Use Cummulative Productivity Rates (y/n) ?",'a'},
  {11,48,3,1,&THIRTY_TWO,"Enter Pick Rate File",'a'}
};
short i,j,n,pickline,index,si,rm,orders,underway;
unsigned char t;
long value,time_int;
char rates[50],*q;
char buf[NUM_PROMPTS][BUF_SIZE];
FILE *fd;
char temp2[10], temp3[10], temp4[10], temp5[10];

main()
{
  extern leave();
  extern char *atoi1();
  
  putenv("_=manpower_input");
  chdir(getenv("HOME"));
  
  open_all();

  fix(manpower_input);
  sd_screen_off();
  sd_clear_screen();                         /* clear screen                 */
  sd_text(manpower_input);
  sd_screen_on();

  for(i = 0; i < NUM_PROMPTS; i++)
  for(n = 0; n < BUF_SIZE; n++)
  buf[i][n] = 0;

  if(SUPER_OP)
  {
    rm = 1;
    index = 0;
  }
  else
  {
    rm = 0;
    index = 1;
  }
  for (j = index; j < 5; j++) sd_prompt(&fld[j],rm);

  sd_cursor(0,6 + rm, 57);                /* F071496 */
  sd_text("(Hrs:Min)");
   
  si = index;                             /* save index                      */

  while(1)
  {
    t = sd_input(&fld[index],rm,&rm,buf[index],0);

    if(t == EXIT) leave();

    else if(t == UP_CURSOR)
    {
      if(index > si)
      index--;
      else
      index = 4;
      continue;
    }
    else if(t == DOWN_CURSOR || t == TAB)
    {
      if (sp->sp_productivity == 'y' && index < 3) index++;
      else if (index < 4) index++;
      else index = si;
      continue;
    }
   
    if (SUPER_OP)
    {
      pickline = pl_lookup(buf[0], 0);
      if(pickline < 0)
      {
        eh_post(ERR_PL, buf[0]);
        index = 0;
        continue;
      }
    }
    else pickline = op_pl;

    time_int = 0;
    q = (char *)memchr(buf[1], ':', strlen(buf[1]));  /* find any colon      */
    if (q) *q++ = 0;
    
    value = atol(buf[1]);
    if (value > 99)
    {
      eh_postn(ERR_VALUE, value);
      index = 1;
      continue;
    }
    time_int = value * 3600;              /* hours                           */
    if (q)
    {
      value = atol(q);
      if (value > 59)
      {
        eh_postn(ERR_VALUE, value);
        index = 1;
        continue;
      }
      time_int += value * 60;
    }
    if(time_int <= 0)
    {
      eh_post(ERR_VALUE, buf[1]);
      index = 1;
      continue;
    }

    orders = atol(buf[2]);                /* number of orders                */
    if(orders == 0)
    {
      eh_post(ERR_RANGE,0);
      index = 2;
      continue;
    }
    
    if (code_to_caps(*buf[3]) == 'n')     underway = 0; /* dont use underway */
    else if(code_to_caps(*buf[3]) == 'y') underway = 1; /* use underway      */
    else
    {
      eh_post(ERR_YN,0);
      index = 3;
      continue;
    }
    if (sp->sp_productivity == 'y')
    {
      if(code_to_caps(*buf[4]) != 'n' && code_to_caps(*buf[4]) != 'y')
      {
        eh_post(ERR_YN,0);
        index = 4;
        continue;
      }
    }
    break;
  }
/*
 * determine which rates to use
 */
  if(sp->sp_productivity == 'y' && code_to_caps(*buf[4]) == 'y')
  {
    strcpy(rates, "current");
    report();                             /* go to report screen             */
  }
/* 
 * prompt for cumulative rates
 */
  while (sp->sp_productivity == 'y')
  {
    t = sd_input(&fld[5],sd_prompt(&fld[5],rm),&rm,buf[5],0);

    if(t == EXIT)
    leave();

    *buf[5] = tolower(*buf[5]);
    if(*buf[5] == 'y')
    {
      strcpy(rates, "cumulative");
      report();                           /* go to report screen             */
    }
    else if(*buf[5] != 'n')
    {
      eh_post(ERR_YN,0);
      continue;
    }
    break;
  }
/*
 * prompt for pick rate file name
 */
  while(1)
  {
    t = sd_input(&fld[6],sd_prompt(&fld[6],rm),&rm,buf[6],0);
        
    if(t == EXIT)
    leave();
/*
 * build pick rate file name
 */
    if (!buf[6][0])
    {
      eh_post(ERR_FILE_INVALID, "pick rate");
      continue;
    }
    sprintf(rates, "%s/%s", pick_rate_text, buf[6]);

    fd = fopen(rates, "r");
    if(fd == 0)
    {
      eh_post(ERR_FILE_INVALID,buf[6]);
      buf[6][0] = 0;
      continue;
    }
    fclose(fd);
    report();
  }
}
/*
 * transfer control to report screen
 */
report()
{
  close_all();
  sprintf(temp2, "%d", pickline);
  sprintf(temp3, "%d", time_int);
  sprintf(temp4, "%d", orders);
  sprintf(temp5, "%d", underway);
  execlp("manpower_rqmt", "manpower_rqmt", temp2,temp3,temp4,temp5,rates, 0);
  krash("report", "manpower_rqmt load", 1);
}
/*
 *open_all_files
 */
leave()
{
  close_all();
  execlp("pnmm", "pnmm", 0);
  krash("leave", "pnmm load", 1);
}
open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  getparms(0);
}
/* 
 * close all files
 */
close_all()
{
  ss_close();
  co_close();
  sd_close();
}
/*
 * transfer control back to calling program
 */

/* end of manpower_input.c */
