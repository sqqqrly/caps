/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Restock report input screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/01/93   |  tjt  Added to mfc.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char restock_rpt_input_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      restock_rpt_input.c          screen 7.2                         */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "language.h"
#include "restock_rpt_input.t"

#define NUM_PROMPTS     4
#define BUF_SIZE        9

short ONE = 1;
short LPL = 8;

static struct fld_parms fld[] = {

  {8,20,1,1,&ONE,"Enter Code",'a'},
  {13,20,1,1,&ONE,"Enter Code",'a'},
  {14,20,1,1,&LPL,"Enter Pickline",'a'},
  {16,20,1,1,&ONE,"Print? (y/n)",'a'}

};

main()
{

  extern leave();

  short rm,i,n,pickline;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];

  putenv("_=restock_rpt_input");
  chdir(getenv("HOME"));

  open_all();

  fix(restock_rpt_input);
  sd_screen_off();
  sd_clear_screen();
  sd_text(restock_rpt_input);
  sd_screen_on();

  for(i=0;i<NUM_PROMPTS;i++)
  for(n=0;n<BUF_SIZE;n++)
  buf[i][n] = 0;

  sd_prompt(&fld[0],0);
  sd_prompt(&fld[1],0);                  

  if((SUPER_OP) && (!(IS_ONE_PICKLINE)))  
  {
    sd_prompt(&fld[2], 0);                 /* pickline prompt                */
  }
  sd_prompt(&fld[3],0);                    /* print prompt                   */
   
   /* main loop to gather input */

  while (1)
  {
    t = sd_input(&fld[0],0, 0,buf[0],0);

    if(t == EXIT) leave();
    if (t == UP_CURSOR) continue;

    *buf[0] = tolower(*buf[0]);                    /* lower case           */

    if(*buf[0] != 'a' && *buf[0] != 'u')
    {
      eh_post(ERR_CODE,buf[0]);
      continue;
    }
    while (1)
    {
      t = sd_input(&fld[1],0, 0,buf[1],0);

      if(t == EXIT) leave();
      if (t == UP_CURSOR) break;
    
      *buf[1] = tolower(*buf[1]);                    /* lower case       */
      if(*buf[1] != 'b' && *buf[1] != 's' && *buf[1] != 'p')
      {
        eh_post(ERR_CODE,buf[1]);
        continue;
      }
      while (1)
      {
        if (IS_ONE_PICKLINE || !SUPER_OP)  
        {
          pickline = op_pl;
          sprintf(buf[2], "%d", pickline);
        }
        else
        {
          t = sd_input(&fld[2],0, 0,buf[2],0);

          if(t == EXIT) leave();
          if (t == UP_CURSOR) break;

          pickline = pl_lookup(buf[2], 0);

          if (pickline < 0)
          {
            eh_post(ERR_PL, buf[2]);
            continue;
          }
          sprintf(buf[2], "%d", pickline);

          if (pickline > 0) chng_pkln(buf[2]);
        }
        while (1)
        {
          t = sd_input(&fld[3], 0, 0,buf[3],0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;

          switch(sd_early(t, code_to_caps(*buf[3])))  /* F041897 */
          {
            case (0): leave();

            case (4):                            /* fork to report program   */
            case (5):

              sd_wait();
              close_all();
              *buf[3] = code_to_caps(*buf[3]);  /* F041897                   */
              execlp("restock_rpt_create","restock_rpt_create",
                buf[0], buf[1], "", buf[2], buf[3], 0);
              krash("main", "restock_rpt_create load", 1);

            case (6):                             /* meaningless input       */

              eh_post(ERR_YN,0);
              break;
          }
        }                                  /* fld[3]                         */
      }                                    /* fld[2]                         */
    }                                      /* fld[1]                         */
  }                                        /* fld[0]                         */
}
leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
}


/*
 * open all files
 */
open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  getparms(0);
}

/*
 * close all files and transfer control backto calling program
 */
/*
 * close all files
 */
close_all()
{
  ss_close();
  co_close();
  sd_close();
}

/* end of restock_rpt_input.c */



