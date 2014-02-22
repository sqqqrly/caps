/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Picker Error Database Support.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   06/05/01  |  aha  Added to mfc.
 *-------------------------------------------------------------------------*/
static char picker_error_maint_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/************************************************************************/
/*                                                                      */
/*       picker error maintenance entry and display menu  screen 7.0A   */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "pie_menu.t"

extern leave();

static short ONE = 1;

struct fld_parms fld1 = {16, 45, 28, 1, &ONE, "Enter Code",'a'};

main()
{
  short rm;
  short ret;
  unsigned char t;
  char buf[4];
  
  putenv("_=picker_error_maint");
  chdir(getenv("HOME"));
  
  open_all();
  
  fix(pie_menu);
  sd_screen_off();
  sd_clear_screen();
  sd_text(pie_menu);
  sd_screen_on();
  
  while(1)
  {
    memset(buf, 0, sizeof(buf));

    t = sd_input(&fld1,(sd_prompt(&fld1,0)),&rm,buf,0);

    if(t ==  EXIT) leave();
    if(t != RETURN) continue;                  /* invalid keys               */

    *buf = tolower(*buf);
 
    switch(*buf)
    {
      case 'a':  
        
        loadprog("acdlm_pie_screen", "add");
        break;
        
      case 'c':
        
        loadprog("acdlm_pie_screen", "change");
        break;
        
      case 'd':
        
        loadprog("acdlm_pie_screen", "delete");
        break;

      case 'l':
        
        loadprog("acdlm_pie_screen", "one");
        break;

      case 'm':
        
        loadprog("acdlm_pie_screen", "all");
        break;

      default:
    
        eh_post(ERR_CODE, buf);
        break;
    }                      /* end switch                     */
  } 
}
/*-------------------------------------------------------------------------*
 *  Load Program And Exit
 *-------------------------------------------------------------------------*/
loadprog(prog, parm)
register char *prog, *parm;
{
  char text[80];

  close_all();
  execlp(prog, prog, parm, 0);
  open_all();    
  sprintf(text, "Program %s not found", prog);
  eh_post(CRASH_MSG, text);
}

open_all()
{
  sd_open(leave);
  ss_open();
}

close_all()
{
  ss_close();
  sd_close();
}

/*
 * transfer control back to menu
 */
leave()
{
  close_all();
  execlp("picker_acctability", "picker_acctability", 0);
  krash("leave", "picker_acctability load", 0);
}

/* end of picker_error_maint.c */
