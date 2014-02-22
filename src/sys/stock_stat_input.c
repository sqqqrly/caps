/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Stock Status Report Input Screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/28/93   |  tjt  Added to mfc.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char stock_stat_input_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      stock_stat_input.c              screen 7.1                      */
/*                                                                      */
/*      all input is checked against the database. in addition,         */
/*      if sorting is by pick module, entered pick module(s) are        */
/*      checked against the configuration.                              */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "getparms.h"
#include "language.h"
#include "stock_stat_input.t"

extern leave();

#define NUM_PROMPTS     4
#define BUF_SIZE        32

short ONE = 1;
short LPL = 8;
short LKEY = 31;

static struct fld_parms fld[] = {

  {6,40,10,1,&LPL,"Enter Pickline",'a'},  
  {10,40,10,1,&ONE,"Enter Sorted By",'a'},
  {11,40,10,1,&LKEY,"Enter Key Range",'a'},    
  {13,40,10,1,&ONE,"Print? (y/n)",'a'}  

};

main()
{
  register long i,j,k,n;                  /* general purpose                 */
  register char *p;
  
  char range1[16];                        /* holds entered range items       */
  char range2[16];
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];
  long PM_num;                            /* for pmfile access               */

  unsigned char pickline=0;               /* operator pickline number        */

  open_all();                             /* open shared segments            */

        /* initialize and show screen display */

  fix(stock_stat_input);
  sd_screen_off();
  sd_clear_screen();                      /* clear entire screen             */
  sd_text(stock_stat_input);
  sd_screen_on();
  
  for(i=0;i<NUM_PROMPTS;i++)
  for(n=0;n<BUF_SIZE;n++)
  buf[i][n] = 0;                          /* clear buffers                   */

        /* main loops to gather input */

  sd_cursor(0, 8, 10);                   /* display menu                    */
  sd_text("L = CAPS Stock Location   P = Pick Module No.   S = SKU");

  if((SUPER_OP) && (!(IS_ONE_PICKLINE))) 
  {
    sd_prompt(&fld[0], 0);   
  }
  sd_prompt(&fld[1], 0);     
  sd_prompt(&fld[2], 0);
  sd_prompt(&fld[3], 0);

  while (1)
  {
    if (IS_ONE_PICKLINE || !SUPER_OP)  
    {
      pickline = op_pl;
      sprintf(buf[0], "%d", pickline);
    }
    else
    {
      t = sd_input(&fld[0], 0, 0,buf[0], 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) continue;

      pickline = pl_lookup(buf[0], 0);       /* numeric of pickline        */
      if (pickline < 0)
      {
        eh_post(ERR_PL, buf[0]);
        continue;
      }
      sprintf(buf[0], "%d", pickline);

      if (pickline > 0) chng_pkln(buf[0]);
    }
    while (1)
    {
      t = sd_input(&fld[1], 0, 0, buf[1], 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;

      *buf[1] = tolower(*buf[1]);         /* lower case                      */

      if (*buf[1] != 'l' && *buf[1] != 'p' && *buf[1] != 's')
      {
        eh_post(ERR_CODE,buf[1]); 
        continue;
      }
      while (1)
      {
        t = sd_input(&fld[2], 0, 0,buf[2], 0);
        if (t==EXIT) leave();
        if (t==UP_CURSOR) break;
      
        strcpy(range1, "0");
        strcpy(range2, "0");
        
        if (*buf[2])                        /* has some data                 */
        {
          p = (char *)memchr(buf[2], '-', 31);
          if (p) *p++ = 0;
        
          switch (*buf[1])
          {
            case 'l': sprintf(range1, "%6.6s", buf[2]);
                      if (p) sprintf(range2, "%6.6s", p);
                      break;
                       
            case 'p': strcpy(range1, buf[2]);
                      if (p) strcpy(range2, p);
                      break;

            case 's': sprintf(range1, "%*.*s", rf->rf_sku, rf->rf_sku, buf[2]);
                      sprintf(range2, "%*.*s", rf->rf_sku, rf->rf_sku, p);
                      break;
          }
        }
        while (1)
        {
          t = sd_input(&fld[3], 0, 0, buf[3], 0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;

          switch(sd_early(t, code_to_caps(*buf[3])))  /* F041897 */
          {
            case (0):  leave();

            case (4):
              
              sd_wait();
              close_all();
              execlp("stock_stat_create", "stock_stat_create",
                buf[0], buf[1], " ", range1, range2, "print", 0);
              krash("main", "stock_stat_create load", 1);
                        
            case (5):

              sd_wait();
              close_all();
              execlp("stock_stat_create", "stock_stat_create",
                buf[0], buf[1], " ", range1, range2, "report", 0);
              krash("main", "stock_stat_create load", 1);

            case (6):
              eh_post(ERR_YN,0);
              break;
          }
        }                                  /* fld[3]                         */
      }                                    /* fld[2]                         */
    }                                      /* fld[1]                         */
  }                                        /* fld[0]                         */
}
open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  getparms(0);
}

close_all()
{
  sd_close();
  co_close();
  ss_close();
}

leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
}

/* end of stock_stat_input.c */

