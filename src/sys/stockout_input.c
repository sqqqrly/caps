/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Gather parms for anticipated stockout report.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/10/93   |  tjt  Added to mfc.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char stockout_input_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "eh_nos.h"
#include "language.h"
#include "stockout_input.t"

extern leave();
extern shutdown();

#define NUM_PROMPTS     5
#define LAST_PROMPT     3                 /* if config change 3 to 4         */
#define BUF_SIZE        31

short ONE = 1;
short LPL = 8;
short FIVE = 5;
short LORDER = 5;
short L30 = 30;

/* row modifier is either 1 or 0 for prompts */

static struct fld_parms fld[] = {

  {5,60,10,1,&LPL,"Enter Pickline",'a'},  /* line 6                          */
  {6,60,10,1,&FIVE,"Enter No. of Orders to be considered",'n'},/* 6 or 7     */
  {7,60,10,1,&LORDER,"Enter Until Order No.",'n'},  /* 7 or 8                */
  {8,45,10,1,&L30,"Enter Configuration Name",'a'},  /* 8 or 9                */
  {10,60,10,1,&ONE,"Print? (y/n)",'a'}              /* 10 or 11              */

};

main()
{
  long pid, status;
  short rm,ret,i,si,n;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];
  long order_num=0;
  short pickline;
  short all_pl=0;

  putenv("_=stockout_input");
  chdir(getenv("HOME"));
  
  open_all();

        /* set order length into fld_parms structure */

  LORDER = rf->rf_on;

        /* determine operator status */

  rm = 0; si = 1;
  if (SUPER_OP && !IS_ONE_PICKLINE) {rm = 1; si = 0;}
  pickline = op_pl;
  sprintf(buf[0], "%d", pickline);
  
  fix(stockout_input);
  sd_clear_screen();
  sd_screen_off();
  sd_text(stockout_input);
  sd_screen_on();

  for(i=0;i<NUM_PROMPTS;i++)
  for(n=0;n<BUF_SIZE;n++)
  buf[i][n] = 0;

  for(i = si; i < LAST_PROMPT; i++)
  sd_prompt(&fld[i],rm);                  /* display prompts                 */
  i = si;


        /* main loop to gather input */

  while(1)
  {
    t = sd_input(&fld[i],rm,&rm,buf[i],0);

    if(t == EXIT) leave();
    else if(t == UP_CURSOR && i > si) i--;
    
    else if(t == DOWN_CURSOR || t == TAB)
    {
      if(i == (LAST_PROMPT - 1)) i = 0;
      else i++;
    }
    else if (t == RETURN)
    {
                        /* validate proper pickline */

      if (si == 0)                        /* have pickline input             */
      {
        n = pl_lookup(buf[0], pickline);  /* numeric of pickline             */

        if (n <= 0)          
        {
          eh_post(ERR_CODE,buf[0]);       /* invalid pickline                */
          i=0;
          continue;
        }
        pickline = n;
        sprintf(buf[0], "%d", pickline);
        chng_pkln(buf[0]);
      }          
                        /* validate entered order number */

      if(!(*buf[1]) && !(*buf[2]))        /* nothing entered                 */
      {
        eh_post(ERR_REQ_ORDER, 0);
        i=1;
        continue;
      }
      if(*buf[2])                         /* if order number entered         */
      {
        order_num=atol(buf[2]);
        ret = oc_find(pickline, order_num);
        if(!ret)
        {
          eh_post(ERR_ORDER,buf[2]);
          i=2;
          continue;
        }
      }
      break;
    }
  }

        /* process print request */

  for(i=1;i<3;i++)                        /* fill non-entered with zero      */
  {
    if(*buf[i])
    ;
    else
    strcpy(buf[i],"0");
  }

  sd_prompt(&fld[4],rm);
  while(1)
  {
    t = sd_input(&fld[4],rm,&rm,buf[4],0);
    switch(sd_early(t, code_to_caps(*buf[4]))) /* F041897 */
    {
      case (0):
        leave();

      case (4):                           /* print == y                      */
        if(!(*buf[0]))                    /* no pickline entered             */
        {
          strcpy(buf[0], getenv("PICKLINE"));/* set current                  */
        }
        sd_wait();
        close_all();
        execlp("stockout_create", "stockout_create",
          buf[0], buf[1], buf[2], "", "print",  0);

        krash("main", "stockout_create load", 1);

      case (5):                           /* print == n                      */
        
        sd_wait();
        if(!(*buf[0]))                    /* no pickline entered             */
        {
          strcpy(buf[0], getenv("PICKLINE"));/* set current                  */
        }
        close_all();
        execlp("stockout_create", "stockout_create",
          buf[0], buf[1], buf[2], "", "report",  0);

        krash("main", "stockout_create load", 1);

      case (6):
        eh_post(ERR_YN,0);
        break;
      }
  }
}

/* functions for stockout_input */

open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  oc_open();
  getparms(0);
}

close_all()
{
  oc_close();
  co_close();
  ss_close();
  sd_close();
}

leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
}

/* end of anticipated_stockout_input.c */
