/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Item movement analysis input screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/19/93   |  tjt  Added to mfc.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char item_move_input_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      item_move_input.c          screen 7.8                           */
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
#include "item_move_input.t"

#define NUM_PROMPTS     7
#define BUF_SIZE        32

short ONE = 1;
short TWO = 2;
short L30 = 30;
short LPL = 8;

/* row modifier is either 2 or 0 for prompts... */

static struct fld_parms fld[] = {

  {4,35,1,1,&LPL,"Enter Pickline",'a'},   /* line 6 only                     */
  {8,35,1,1,&ONE,"Enter Ranked By",'a'},  /* line 8 or 10                    */
  {9,35,1,1,&ONE,"Enter Sorted By",'a'},  /* line 9 or 11                    */
  {10,35,1,1,&ONE,"Use Current Period? (y/n)",'a'},/* line 10 or 12          */
  {11,35,1,1,&ONE,"Use Cumulative Period? (y/n)",'a'},/* line 11 or 13       */
  {12,35,1,1,&L30,"Enter Configuration Name",'a'},/* line 12 or 14           */
  {13,35,1,1,&ONE,"Print? (y/n)",'a'}     /* line 13 or 15                   */

};

main()
{
  extern leave();                    /* for power failure               */

  short rm,ret,i,si,sop,n;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];
  unsigned char pickline;                 /* current operator pickline       */
  unsigned char rdflg=0;                  /* set to 1 if current rates       */
                                        /* set to 2 if cumulative rates */
  short all_pl=0;                         /* set if entered pickline = 0     */
  char pbuf[2];                           /* arg for create program          */

  putenv("_=item_move_input");
  chdir(getenv("HOME"));
  
  open_all();

        /* determine operator status */

  rm = 0; si = 1;
  if(SUPER_OP)  {rm = 2; si = 0;}         /* si = 0 if super operator        */

  if(IS_ONE_PICKLINE) {si = 1; pickline = op_pl;}
  else
  pickline = op_pl;                       /* set current pickline            */

        /* initialize and show screen display */

  
  fix(item_move_input);
  sd_screen_off();
  sd_clear_screen();                      /* clear entire screen             */
  sd_text(item_move_input);               /* display screen                  */
  sd_screen_on();
  
  for(i=0;i<NUM_PROMPTS;i++)              /* clear all buffers...            */
  for(n=0;n<BUF_SIZE;n++)
  buf[i][n] = 0;

        /* display prompts down to "Use Current Rates?" */

  for(i=si;i<4;i++)
  sd_prompt(&fld[i],rm);                  /* display prompts                 */
  i = si;                                 /* initial input                   */

  sd_cursor(0,rm + 6,1);
  sd_text("C = Cartons                L = Lines                    U = Units");

        /* main loop to gather input */

  while(1)
  {
    t = sd_input(&fld[i],rm,&rm,buf[i],0);
    if(t == EXIT)
    leave();
    else if(t == UP_CURSOR && i > si)
    i--;
    else if(t == DOWN_CURSOR || t == TAB)
    {
      if(i < 3) i++;
      else      i=si;
    }
    else if (t == RETURN)
    {
      if(!(IS_ONE_PICKLINE))              /* F052787                         */
      {
        n = pl_lookup(buf[0], op_pl);    /* pickline number                  */
        if (!n) all_pl = 1;              /* ascii zero entered               */
        else if (n > 0)                  /* something entered                */
        {
          sprintf(buf[0], "%d", n);
          chng_pkln(buf[0]);
          pickline=n;
        }
        else
        {
          eh_post(ERR_PL, buf[0]);
          i = si;
          continue;
        }
      }                                   /* F052787                         */
      *buf[1] = tolower(*buf[1]);         /* lower case                      */
      if(*buf[1]!='l' && *buf[1]!='c' && *buf[1]!='u')
      {
        eh_post(ERR_CODE,buf[1]);
        i=1;
        continue;
      }
      *buf[2] = tolower(*buf[2]);         /* lower case                      */
      if(*buf[2]!='l' && *buf[2]!='c' && *buf[2]!='u')
      {
        eh_post(ERR_CODE,buf[2]);
        i=2;
        continue;
      }
      if(code_to_caps(*buf[3]) != 'y' && code_to_caps(*buf[3]) != 'n')
      {
        eh_post(ERR_YN,0);
        i=3;
        continue;
      }
      if(code_to_caps(*buf[3]) == 'y')    /* use current rates               */
      rdflg=1;                            /* set rate determined flag        */
      break;                              /* from input loop                 */
    }
    else                                  /* invalid keys                    */
    ;
  }

        /* loop to determine current or cumulative rates... */

  if(!rdflg)                              /* rate not determined             */
  {
    sd_prompt(&fld[4],rm);                /* ask about cumulative rates      */
    si=3;
    i=4;
  }
  while(1)
  {
    if(rdflg)
    break;                                /* get out if rate determined      */

    while(1)
    {
      t=sd_input(&fld[i],rm,&rm,buf[i],0);
      if(t==EXIT)
      leave();
      else if(t==UP_CURSOR && i>si && code_to_caps(*buf[4]) != 'y')
      i--;
      else if(t==DOWN_CURSOR || t==TAB)
      {
        if(i < 4 && code_to_caps(*buf[3]) != 'y') i++;
        else if (code_to_caps(*buf[4]) != 'y')    i = si;
      }
      else if(t==RETURN)
      {
        if (code_to_caps(*buf[3]) == 'y')
        {
          rdflg=1;
          break;
        }
        else if (code_to_caps(*buf[4]) == 'y')
        {
          rdflg=2;
          break;
        }
        else if (code_to_caps(*buf[3]) != 'n')
        {
          eh_post(ERR_YN,0);
          i=3;
          continue;
        }
        else if(code_to_caps(*buf[4]) != 'n')
        {
          eh_post(ERR_YN,0);
          i=4;
          continue;
        }
        else
        ;
      }
    }
  }
        /* process print request */

  sd_prompt(&fld[6],rm);
  while(1)
  {
    t = sd_input(&fld[6],rm,&rm,buf[6],0);
    
    switch(sd_early(t, code_to_caps(*buf[6])))
    {
      case (0):
        leave();

      case (4):
        
        sd_wait();
        if(!(*buf[0]))                    /* no pickline entered             */
          sprintf(buf[0], "%d", pickline);
        sprintf(pbuf, "%d", rdflg);       /* convert for execl               */
        close_all();

        *buf[6] = code_to_caps(*buf[6]);  /* F041897                         */
    
        execlp("item_move_create", "item_move_create",
          buf[0],buf[1],buf[2],pbuf,"",buf[6],0);

        krash("main", "item_move_create load", 1);

      case (5):
        
        sd_wait();
        if(!(*buf[0]))                    /* no pickline entered             */
          sprintf(buf[0], "%d", pickline);
        sprintf(pbuf, "%d", rdflg);
        close_all();

        *buf[6] = code_to_caps(*buf[6]);  /* F041897                         */
    
        execlp("item_move_create", "item_move_create",
          buf[0],buf[1],buf[2],pbuf,"",buf[6],0);

        krash("main", "item_move_create load", 1);

      case (6):
        eh_post(ERR_YN,0);
        break;
      }
  }
}
leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
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
  co_close();
  ss_close();
  sd_close();
}


/* end of item_move_input.c */
