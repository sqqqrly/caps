/*----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Enable/Disable Restock Ticket Printing.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/3/93    |  tjt  added to mfc.
 *-------------------------------------------------------------------------*/
static char e_d_res_tick_prnt_c[] = "%Z% %M% %I% (%G% - %U%)";
/************************************************************************/
/*                                                                      */
/*      e_d_res_tick_prnt.c             screen 7.24                     */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "e_d_res_tick_prnt.t"

#define NUM_PROMPTS     1
#define BUF_SIZE        2

static short ONE = 1;

struct fld_parms fld ={6,30,1,1,&ONE,"Enable/Disable? (E/D)",'a'};

main()
{
  extern leave();
  short rm,n;
  unsigned char t;
  char buf[BUF_SIZE];

  putenv("_=e_d_res_tick_prnt");
  chdir(getenv("HOME"));
  
  open_all();

  fix(e_d_res_tick_prnt);
  sd_screen_off();
  sd_clear_screen();                      /* clear entire screen             */
  sd_text(e_d_res_tick_prnt);
  sd_screen_on();        
  
        /* clear buffer */

  for(n=0;n<BUF_SIZE;n++)
  buf[n] = 0;

  sd_cursor(0, 8, 1);
  if (sp->sp_rp_flag == 'y')
  {
    sd_text("Restock printing flag is currently enabled");
  }
  else
  {
    sd_text("Restock printing flag is currently disabled");
  }

  sd_prompt(&fld,0);

  while(1)                                /* loop till proper input          */
  {
    t=sd_input(&fld,0,&rm,buf,0);
    if(t==EXIT) leave();
    else if(t==RETURN)
    {
                        /* check input */

      *buf = tolower(*buf);
      if(*buf!='e' && *buf!= 'd')
      {
        eh_post(ERR_CODE,buf);
        continue;
      }
                        /* make change */

      if(*buf=='e')                       /* enable                          */
      {  
        sp->sp_rp_flag ='y';
        eh_post(ERR_CONFIRM,"Restk print enabled");
      }
      else                                /* disable                         */
      {
        sp->sp_rp_flag ='n';
        eh_post(ERR_CONFIRM,"Restk print disabled");
      }
      break;
    }
  }
  leave();
}

leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
}
open_all()
{
  ss_open();
  sd_open(leave);
}

close_all()
{
  sd_close();
  ss_close();
}


/* end of e_d_res_tick_prnt.c */
