/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear productivity counts.
 *
 *  zero_prod_counts [Y|Z]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   10/11/93  |  tjt  Added to mfc.
 *   06/08/94  |  tjt  Fix so don't clear error message line.
 *   01/27/95  |  tjt  Set all picklines to same value.
 *   01/27/95  |  tjt  Clear current time too - ofc will change it.
 *   06/03/95  |  tjt  Add pickline input by name.
 *   04/18/97  |  tjt  Add language.h and code_to_caps.
 *   06/05/01  |  aha  Modified for section productivity.
 *-------------------------------------------------------------------------*/
static char zero_prod_counts_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/****************************************************************************/
/*                                                                          */
/*                           Zero Productivity Counts                       */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <time.h>
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "zero_prod_counts.t"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "pr.h"
#include "Bard.h"
#include "language.h"
#include "sections.h"

#define NUM_PROMPTS     2
#define BUF_SIZE        9

/*
 *Global Variables
 */

short ONE = 1;
short LPL = 8;

struct fld_parms fld[] ={
  {6,62,5,1,&LPL,"Enter Pickline",'a'},
  {8,62,5,1,&ONE,"Zero Current Productivity Counts? (y/n)"
    ,'a'},
};

unsigned char t;
char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */
char bufx[NUM_PROMPTS][BUF_SIZE];         /* array of buffers                */

short i,j,n,rm,ret,j,pickline,current,cumulative;

main(argc, argv)
long argc;
char **argv;
{
  extern leave();
  char text[80];
  section_prod_log_item splog;
  unsigned long int now = 0L;
  time_t * nowptr = 0;
  char datemsg[20];
  
  datemsg[19] = '\0';

  memset(&splog, 0x0, sizeof(section_prod_log_item));

  text[79] = '\0';

  open_all();

  chdir(getenv("HOME"));
  
  fix(zero_prod_counts);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(zero_prod_counts);
  sd_screen_on();
 
  cumulative = 1;
  pickline = 0;
 
  while(1)
  {
    sd_cursor(0,6,1);
    sd_clear_line();
    sd_cursor(0,8,1);
    sd_clear_line();

    memset(buf, 0, BUF_SIZE * NUM_PROMPTS);
    memset(bufx, 0, BUF_SIZE * NUM_PROMPTS);

    sd_prompt(&fld[1], 0);
    
    while(1)
    {
      current = 0;
      t = sd_input(&fld[1],0,&rm,bufx[1],0);
      if(t == EXIT) leave();

      else if (t == RETURN)
      {
        *buf[1] = code_to_caps(*bufx[1]);

        if(*buf[1] == 'y')
        {
            current = 1;
            break;
        }
        else if(*buf[1] != 'n')
        {
          eh_post(ERR_YN,0);
          continue;
        }
        else
        {
          eh_post(ERR_YN,0);
          continue;
        }
      }
    }
    now = time(0);
    
    if(pickline == 0)                     /*all picklines                    */
    {
      for (i = 0; i < pr->pr_picklines; i++)
      {
        if(pp[i].pr_pl_cum_elapsed)       /*if pickline in the configuration */
        {
          if (current)
          {
            pp[i].pr_pl_cur_start = now;
            pp[i].pr_pl_current   = now;
            pp[i].pr_pl_cur_completed = 0;
            pp[i].pr_pl_cur_elapsed = 0;
            
            for (j = 0; j < pr->pr_zones; j++)
            {
              if(zone[j].zt_pl == i + 1)
              {
                pz[j].pr_zone_cur_orders = 0;
                pz[j].pr_zone_cur_lines  = 0;
                pz[j].pr_zone_cur_units  = 0;
                pz[j].pr_zone_cur_ah_cnt = 0;
                pz[j].pr_zone_cur_ahead  = 0;
                pz[j].pr_zone_cur_active = 0;
              }
            }
          }
          if (cumulative)
          {
            pp[i].pr_pl_cum_start = now;
            pp[i].pr_pl_current   = now;
            pp[i].pr_pl_cum_completed = 0;
            pp[i].pr_pl_cum_elapsed = 0;

            for (j = 0; j < pr->pr_zones; j++)
            {
              if(zone[j].zt_pl == i + 1)
              {
                pz[j].pr_zone_cum_orders = 0;
                pz[j].pr_zone_cum_lines  = 0;
                pz[j].pr_zone_cum_units  = 0;
                pz[j].pr_zone_cum_ah_cnt = 0;
                pz[j].pr_zone_cum_ahead  = 0;
                pz[j].pr_zone_cum_active = 0;
              }
            }
          }
        }
      }
    }
    
    sprintf(text, "dbaccess caps sql/copy_section_prod.sql %s",
            "1>/dev/null 2>&1");
    system(text);
   
    nowptr = (time_t *)&now;
    strftime(datemsg, 20, "%Y-%m-%d %T", localtime(nowptr));

    strcpy(splog.spl_record_date, datemsg);

    //database_open();
    //sec_prl_open(AUTOLOCK);
    //begin_work();
    //sec_prl_purge(&splog);
    //commit_work(); 
    //sec_prl_close();
    //database_close();

    sprintf(text, "dbaccess caps src/h/informix/section_prod.sql %s",
            "1>/dev/null 2>&1");
    system(text);  
    eh_post(ERR_CONFIRM,"Zero current counts");
    leave();
  }
}
leave()
{
  close_all();
  execlp("pnmm", "pnmm", 0);
  krash("leave", "pnmm load", 1);
}

/*
 *open all files
 */
open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  pr_open();
  getparms(0);
}
close_all()
{
  sd_close();
  ss_close();
  co_close();
  pr_close();
}
/*
 *close all files and transfer control back to calling program
 */

/* end of zero_prod_counts.c */
