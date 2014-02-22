/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Pickline View/Simulator Selection
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *             |
 *-------------------------------------------------------------------------*/
static char view_menu_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "ss.h"
#include "sd.h"
#include "eh_nos.h"
#include "view_menu.t"

extern leave();

long count;                           
char load_prog[24];
  
main()
{
  putenv("_=view_menu");
  
  chdir(getenv("HOME"));
  
  ss_open();
  
  if (sp->sp_full_function != 'n')
  {
    strcpy(load_prog, "ff_view");
    count++;
  }
  if (sp->sp_basic_function != 'n')
  {
    strcpy(load_prog, "bf_view");
    count++;
  }
  if (sp->sp_total_function != 'n')
  {
    strcpy(load_prog, "ac_view");
    count++;
  }
  if (count != 1) get_parms();
  
  ss_close();

  execlp(load_prog, load_prog, 0);
  krash("main", load_prog, 1);
}
/*-------------------------------------------------------------------------*
 *  Get View Selection
 *-------------------------------------------------------------------------*/
get_parms()
{
  static short ONE = 1;
  static struct fld_parms fld = {13,49,29,1,&ONE,"Enter Selection", 'a'};
  char incode[2];
  unsigned char t;
  
  sd_open(leave);
  
  fix(view_menu);
  sd_screen_off();
  sd_clear_screen();
  sd_text(view_menu);
  sd_screen_on();
  
  sd_prompt(&fld, 0);
  memset(incode, 0, sizeof(incode));
  
  while (1)
  {
    t = sd_input(&fld, 0, 0, incode, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) continue;
    
    *incode = tolower(*incode);
    
    if (*incode == 'a')      strcpy(load_prog, "ac_view");
    else if (*incode == 'b') strcpy(load_prog, "bf_view");
    else if (*incode == 'f') strcpy(load_prog, "ff_view");
    else
    {
      eh_post(ERR_CODE, incode);
      continue;
    }
    break;
  }
  sd_close();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  sd_close();
  execlp("operm", "operm", 0);
  krash("leave", "load operm", 1);
}

/* end of view_menu.c */
