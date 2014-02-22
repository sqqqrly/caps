/* #define DEBUG  */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Productivity menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/06/93   |  tjt  Added to mfc.
 *  04/27/95   |  tjt  Add productivity feature check.
 *  06/05/01   |  aha  Added menu item 'S' for section maintenance.
 *-------------------------------------------------------------------------*/
static char pnmm_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "caps_copyright.h"
#include <stdio.h>
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "pnmm.t"

extern leave();

static short ONE = 1;

struct fld_parms fld1 = {18, 44, 25, 1, &ONE, "Enter Code", 'a' };

main()
{
  short i;
  short rm;
  short ret;
  unsigned char t;
  char buf[2], progname[40], text[64], *parm;

  putenv("_=pnmm");
  chdir(getenv("HOME"));

  open_all();
  
  fix(pnmm);
  sd_screen_off();
  sd_clear_screen();
  sd_text(pnmm);
  sd_screen_on();
  
  sd_prompt(&fld1, 0);

  parm = 0;
  
  while(1)
  {
    *buf = 0;
    *progname = 0;
    
    t = sd_input(&fld1, 0, &rm, buf, 0);

    *buf = toupper(*buf);
    if(t == EXIT) leave();

    if(!LEGAL(*buf))
    {
      eh_post(ERR_CODE, buf);
      continue;
    }
    switch(buf[0])
    {

      case 'A' :
                        
        strcpy(progname, "picker_acctability");
        break;
         
      case 'B' :
                        
        if (sp->sp_productivity != 'y')
        {
          eh_post(LOCAL_MSG, "Requires Productivity Feature");
          break;
        }
        strcpy(progname, "zone_bal_input");
        break;
      
      case 'C' :
                        
        if (sp->sp_productivity != 'y')
        {
          eh_post(LOCAL_MSG, "Requires Productivity Feature");
          break;
        }
        strcpy(progname, "config_sel_menu");
        break;
                   
      case 'M' :
                        
        if (sp->sp_config_status != 'y')
        {
          eh_post(ERR_NO_CONFIG, 0);
          continue;
        }
        strcpy(progname, "manpower_input");
        break;
                        
      case 'P' :
                        
        if (sp->sp_productivity != 'y')
        {
          eh_post(LOCAL_MSG, "Requires Productivity Feature");
          break;
        }
        strcpy(progname, "productivity");
        break;

      case 'R' :

        strcpy(progname, "pick_rate_entry");
        break;

      case 'S' :

        if (sp->sp_productivity != 'y')
        {
          eh_post(LOCAL_MSG, "Requires Productivity Feature");
          break;
        }
        strcpy(progname, "sections_maint");
        break;
                        
      case 'W' :
                        
        strcpy(progname, "remaining_picks");
        break;

      case 'Z' :
                        
        if (sp->sp_productivity != 'y')
        {
          eh_post(LOCAL_MSG, "Requires Productivity Feature");
          break;
        }
        parm = buf;
        
        strcpy(progname, "zero_prod_counts");
        break;
      
    }
    if (*progname)
    {
      close_all();
      execlp(progname, progname, parm, 0);
      open_all();
      sprintf(text, "Program %s not found", progname);
      eh_post(CRASH_MSG, text);
    }
    else
    {
      eh_post(ERR_CODE, buf);
    }
  }
}
open_all()
{
  sd_open(leave);
  ss_open();
  getparms("LEGAL5");
}
close_all()
{
  ss_close();
  sd_close();
}
/*
 *transfer control back to calling program
 */
leave()
{
  close_all();
  execlp("mmenu","mmenu",0);
  krash("leave", "mmenu load",1 );
}
