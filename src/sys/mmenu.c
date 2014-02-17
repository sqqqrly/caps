/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Main Menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/20/93   |  tjt  Revised for mfc.
 *  12/16/94   |  tjt  Add lot control.
 *  01/23/95   |  tjt  New IS_ONE_PICKLINE.
 *-------------------------------------------------------------------------*/
static char mmenu_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "mmenu.t"
#include "eh_nos.h"
#include "getparms.h"

extern leave();

static short ONE = 1;

struct fld_parms fld1 = {21, 40, 25, 1, &ONE, "Enter Code", 'a'};

main(argc, argv)
long argc;
char **argv;
{
  short rm;
  short ret;
  register unsigned char t, *p, **q;
  char buf[2];

  putenv("_=mmenu");                      /* this program                    */
  chdir(getenv("HOME"));
  
#ifdef DEBUG
  fprintf(stderr, "mmenu: pid=%d  pgrp=%d\n", getpid(), getpgrp());
#endif
  
  sd_open(leave);                         /* open tty server                 */
  ss_open();
  
  getparms("LEGAL1");                     /* get environment                 */

  fix(mmenu);                             /* specialize mm screen            */

  sd_screen_off();
  sd_clear_screen();                      /* clear entire screen             */
  sd_text(mmenu);                         /* show screen                     */
  sd_screen_on();

  if (sp->sp_running_status == 'y')
  {
    eh_post(LOCAL_MSG, "CAPS Is Running");/* show CAPS is running            */
  }
  else if (sp->sp_config_status == 'y')
  {
    eh_post(LOCAL_MSG, "CAPS Has Stopped");
    
/*  eh_post(LOCAL_MSG, "CAPS Has Left The Building");  */
  }
  else eh_post(ERR_NO_CONFIG, 0);         /* show not configured             */

  sd_prompt(&fld1, 0);

  while (1)
  {
    memset(buf, 0, 2);                    /* clear input field               */
    
    t = sd_input(&fld1, 0, &rm, buf, 0);
    if (t == UP_CURSOR) continue;         /* repeat prompt                   */
    if (t == EXIT) continue;
    
    *buf = toupper(*buf);                 /* to upper case                   */

    if(!LEGAL(*buf))
    {
      eh_post(ERR_CODE, buf);
      continue;
    }

    switch(*buf)
    {
      case 'L': leave();                  /* logoff                          */

      case 'O':                           /* operations commands             */

        loadprog("operm");
        break;
        
      case 'S':                           /* system commands                 */
         
        if(SUPER_OP) loadprog("syscomm");
        else eh_post(ERR_CODE, buf);
        break;
           
      case 'C':                           /* configuration                   */

        loadprog("confm");
        break;

      case 'E':                           /* order entry                     */

        if (sp->sp_config_status == 'y') loadprog("order_entry");
        else eh_post(ERR_NO_CONFIG, 0);
        break;

      case 'F':                           /* product file maintenance        */

        if(sp->sp_sku_support == 'y') loadprog("pfead");
        else eh_post(ERR_CODE, buf);
        break;

      case 'G':                           /* packing list and label printers */
      
        if (sp->sp_labels == 'y') loadprog("sp_menu");
        else eh_post(ERR_CODE, buf);
        break;

      case 'N':
      
        if (sp->sp_lot_control == 'y') loadprog("lot_control");
        else eh_post(ERR_CODE, buf);
        break;

      case 'P':                           /* productivity                    */

        if (sp->sp_productivity == 'y') loadprog("pnmm");
        else eh_post(ERR_CODE, buf);
        break;

      default:
      
        eh_post(ERR_CODE, buf);
        break;
    }
  }
}
loadprog(pname)
char *pname;
{
  char message[80];

  ss_close();
  sd_close();
  execlp(pname, pname, 0);                /* try to load a program           */
  sd_open(leave);
  ss_open();
  sprintf(message, "Program %s Not Found", pname);
  eh_post(CRASH_MSG, message);
}
/*-------------------------------------------------------------------------*
 * Leave
 *-------------------------------------------------------------------------*/
leave()
{
  sd_close();
  ss_close();
  execlp("op_logoff", "op_logoff", 0);
  krash("mmenu", "op_logoff load", 1);
}

/* end of mmenu.c */
