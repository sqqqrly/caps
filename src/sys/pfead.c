/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Product and Module Database Support.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   10/06/93  |  tjt  Added to mfc.
 *   08/03/95  |  tjt  Allow queries when running.
 *-------------------------------------------------------------------------*/
static char pfead_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      product file entry and display menu     screen 7.0              */
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
#include "pfead.t"

extern leave();

static short ONE = 1;

struct fld_parms fld1 = {12,45,28,1,&ONE,"Enter Code",'a'};

main()
{
  short rm;
  short ret;
  unsigned char t;
  char buf[4];
  
  putenv("_=pfead");
  chdir(getenv("HOME"));
  
  open_all();
  
  fix(pfead);
  sd_screen_off();
  sd_clear_screen();
  sd_text(pfead);
  sd_screen_on();
  
  while(1)
  {
    memset(buf, 0, sizeof(buf));

    t = sd_input(&fld1,(sd_prompt(&fld1,0)),&rm,buf,0);

    if(t ==  EXIT) leave();
    if(t != RETURN) continue;                  /* invalid keys               */

    if (!LEGAL(toupper(*buf)))
    {
      eh_post(ERR_CODE, buf);
      continue;
    }
    *buf = tolower(*buf);
 
    switch(*buf)
    {
      case 'i':
        loadprog("pfead_inventory", 0);
        break;
        
      case 'm':
        
        loadprog("pfead_pmfile", 0);
        break;
        
      case 'p':
        
        loadprog("pfead_prodfile", 0);
		  break;
		  
      case 'r':  
        
        loadprog("pfead_reports", 0);
        break;
        
      default:
    
        eh_post(ERR_CODE,buf);
        break;
    }                                      /* end switch                     */
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
  getparms("LEGAL7");
}
close_all()
{
  ss_close();
  sd_close();
}
/*
 * transfer control back to main menu
 */
leave()
{
  close_all();
  execlp("mmenu", "mmenu", 0);
  krash("leave", "mmemu load", 0);
}

/* end of pfead.c */
