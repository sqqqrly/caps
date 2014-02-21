/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Module Inventory Database Support.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   12/11/93  |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char pfead_inventory_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      product file inventory menu     screen 7.0D                     */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "pfead_inventory.t"

extern leave();

static short ONE = 1;

struct fld_parms fld1 = {12, 45, 28, 1, &ONE, "Enter Code",'a'};

main()
{
  short rm;
  short ret;
  unsigned char t;
  char buf[4];
  
  putenv("_=pfead_inventroy");
  chdir(getenv("HOME"));
  
  open_all();
  
  fix(pfead_inventory);
  sd_screen_off();
  sd_clear_screen();
  sd_text(pfead_inventory);
  sd_screen_on();
  
  while(1)
  {
    memset(buf, 0, sizeof(buf));

    t = sd_input(&fld1,(sd_prompt(&fld1,0)),&rm,buf,0);

    if(t ==  EXIT) leave();
    if(t != RETURN) continue;                  /* invalid keys               */

/*-------------------------------------- defaults to pl = 1 in rap_screen
    if (sp->sp_config_status != 'y') 
    {
      eh_post(ERR_NO_CONFIG, 0);      
      continue;
    }
 *-------------------------------------*/
    *buf = tolower(*buf);
 
    switch(*buf)
    {
      case 'a':
        
        loadprog("rap_screen", "adjustments");
        break;

      case 'b':
        
        loadprog("batch_receipts_srn", 0);
        break;

      case 'p':
        
        loadprog("rap_screen", "phys_inv");
        break;

      case 'r':

        loadprog("rap_screen", "receipts");
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

/* end of pfead_inventory.c */
