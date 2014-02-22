/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Zone Balancing Input Screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   11/28/93  | tjt  Added to mfc.
 *   03/31/94  | tjt  Allow zero zones for automatic selection.
 *   06/03/95  | tjt  Add pickline input by name.
 *   07/07/95  | tjt  Fix cummulative prompt.
 *   04/18/97  | tjt  Add language.h and code_to_caps.
 *   07/24/01  | aha  Modified to use sections per Eckerd's MP & ZB spec.
 *   11/12/01  | aha  Modified to use 0 for all picklines.
 *-------------------------------------------------------------------------*/
static char zone_bal_input_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/*****************************************************************************
 *                                                                           *
 *                        zone_bal_input                                     *
 *                                                                           *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "file_names.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "getparms.h"
#include "zone_bal_input.t"
#include "language.h"

extern leave();

#define NUM_PROMPTS     8
#define BUF_SIZE        33

/*
 * Global Variables
 */

short ONE = 1;
short TWO = 2;
short FIVE = 5;
short L32 = 32;
short LPL = 8;

struct fld_parms fld[] ={
  { 6,48,3,1,&LPL,"Enter Pickline:",'a'},
  { 7,48,3,1,&ONE,"Balancing Type: enter Lines or Units (L/U):",'a'},
  { 8,48,3,1,&FIVE,"Number of Orders:",'n'},
  { 9,48,3,1,&TWO,"Number of Zones (pickers):",'n'},
  {10,48,3,1,&L32,"Enter output configuration name:",'a'},
  {11,48,3,1,&ONE,"Use Current Productivity Rates (y/n) ?",'a'},
  {12,48,3,1,&ONE,"Use Cummulative Productivity Rates (y/n) ?",'a'},
  {13,48,3,1,&L32,"Enter Pick Rate File:",'a'}
};

long i,j,n,pickline,index0,si,rm,zones;
long orders, zones;
unsigned char t;
char rates[50], text[50];
char buf[NUM_PROMPTS][BUF_SIZE];
char yn0[2];
FILE *fd;
char temp1[10], temp2[10], temp3[10], temp4[10], junk[50];

main()
{
  putenv("_=zone_bal_input");
  chdir(getenv("HOME"));

  open_all();

  fix(zone_bal_input);
  sd_screen_off();
  sd_clear_screen();                       /* clear screen                   */
  sd_text(zone_bal_input);
  sd_screen_on();

  for(i = 0; i < NUM_PROMPTS; i++)
  for(n = 0; n < BUF_SIZE; n++)
  buf[i][n] = 0;

  
  if(IS_ONE_PICKLINE || !SUPER_OP) 
  {
    pickline = op_pl; 
    sprintf(buf[0], "%d", pickline);
    si = 1;
  }
  else si = 0;
  
  for (j = si; j < 5; j++) 
  {  
    sd_prompt(&fld[j], 0);
  }
  index0 = si;

  while(1)
  {
    t = sd_input(&fld[index0], 0, 0, buf[index0], 0);

    if (t == EXIT) leave();
    else if(t == UP_CURSOR)
    {
      if (index0 > si) index0--;
      continue;
    }
    else if (t == DOWN_CURSOR || t == TAB)
    {
      if (index0 < 4) index0++;
      continue;
    }
    if (!si)
    {
      if (atol(buf[0]) == 0L)
         {
           pickline = 0L;
         }
      else
         {
           pickline = pl_lookup(buf[0], op_pl);
           if (pickline < 1)
              {
                eh_post(ERR_PL,buf[0]);
                index0 = 0;
                continue;
              }
         }

      sprintf(buf[0], "%d", pickline);
    }                              
    *buf[1] = tolower(*buf[1]);
    if (*buf[1] != 'l' && *buf[1] != 'u')
    {
      eh_post(ERR_CODE, buf[1]);
      index0 = 1;
      continue;
    }
/*
 * process number of orders prompt
 */
    orders = atol(buf[2]);
    sprintf(buf[2], "%d", orders);
/*
 * process number of zones prompt
 */
    zones = atol(buf[3]);
    if(zones < 0 || zones > coh->co_zones)
    {
      eh_post(ERR_RANGE, 0);
      index0 = 3;
      continue;
    }
    sprintf(buf[3], "%d", zones);

    if(!*buf[4])                          /* output config file              */
    {
      eh_post(ERR_FILE_INVALID, 0);
      index0 = 4;
      continue;
    }
    sprintf(junk, "config/%s", buf[4]);

    fd = fopen(junk, "r");

    if (fd > 0)                           /* file already exists             */
    {
      eh_post(ERR_REUSE, buf[4]);
      fclose(fd);
    }
/*
 * determine which rates to use
 */
    memset(yn0, 0, 2);
    while(1)
    {
      sd_prompt(&fld[5], 0);
      t = sd_input(&fld[5], 0, 0,yn0, 0);
        
      if(t == EXIT) leave();
      *buf[5] = code_to_caps(*yn0);
      
      if (*buf[5] == 'y') 
      {
        strcpy(rates,"current");
        report();                           /* go to report screen          */
      }
      if (*buf[5] != 'n')
      {
        eh_post(ERR_YN,0);
        continue;
      }
      break;
    }
/* 
 * prompt for cumulative rates
 */
    memset(yn0, 0, 2);
    while(1)
    {
      sd_prompt(&fld[6], 0);
      t = sd_input(&fld[6], 0, 0, yn0, 0);
        
      if(t == EXIT) leave();
      *buf[6] = code_to_caps(*yn0);
      if (*buf[6] == 'y') 
      {
        strcpy(rates,"cumulative");
        report();                           /* go to report screen          */
      }
      if (*buf[6] != 'n')
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
      sd_prompt(&fld[7], 0);
      t = sd_input(&fld[7], 0, 0, buf[7], 0);
        
      if(t == EXIT) leave();
    
      if (!*buf[7])
      {
        eh_post(ERR_FILE_INVALID, "pick rate");
        continue;
      }
      sprintf(text, "%s/%s", pick_rate_text, buf[7]);

      fd = fopen(text, "r");
      if(fd == 0)
      {
        eh_post(ERR_FILE_INVALID, buf[7]);
        buf[7][0] = 0;
        continue;
      }
      strcpy(rates, buf[7]);
      fclose(fd);
      report();
    }
  }
}
/*
 * transfer control to report screen
 */
report()
{
  close_all();
  execlp("zone_balancing", "zone_balancing", 
    buf[0], buf[1], buf[2], buf[3], buf[4], rates, 0);
  krash("report", "zone_balancing load", 1);
}
/*
 *open_all_files
 */
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
  sd_close();
  ss_close();
  co_close();
}
/*
 * transfer control back to calling program
 */
leave()
{
  close_all();
  execlp("pnmm", "pnmm", 0);
  krash("leave", "pnmm load", 1);
}

/* end of zone_bal_input.c */
