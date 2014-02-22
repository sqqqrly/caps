/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Product Database Support.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   12/11/93  |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char pfead_prodfile_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      product file entry and display menu     screen 7.0B             */
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
#include "pfead_prodfile.t"

extern leave();

static short ONE = 1;

struct fld_parms fld1 = {11, 45, 28, 1, &ONE, "Enter Code",'a'};

main()
{
  short rm;
  short ret;
  unsigned char t;
  char buf[4];
  
  putenv("_=pfead_prodfile");
  chdir(getenv("HOME"));
  
  open_all();
  
  fix(pfead_prodfile);
  sd_screen_off();
  sd_clear_screen();
  sd_text(pfead_prodfile);
  sd_screen_on();
  
  while(1)
  {
    memset(buf, 0, sizeof(buf));

    t = sd_input(&fld1,(sd_prompt(&fld1,0)),&rm,buf,0);

    if(t ==  EXIT) leave();
    if(t != RETURN) continue;                  /* invalid keys               */

    *buf = tolower(*buf);
 
    if (!LEGAL(toupper(*buf)))
    {
      eh_post(ERR_CODE, buf);
      continue;
    }
    switch(*buf)
    {
      case 'a':
        
        loadprog("acd_sku_screen", "add");
		  break;
		  
      case 'c':
        
        loadprog("acd_sku_screen", "change");
        break;
        
      case 'd':
        
        loadprog("acd_sku_screen", "delete");
        break;
        
      default:
    
        eh_post(ERR_CODE, buf);
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
 * transfer control back to menu
 */
leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 0);
}

/* end of pfead_prodfile.c */
