/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Carousel dummy diagnostics menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/7/93    |  tjt Added to mfc.
 *-------------------------------------------------------------------------*/
static char diag_menu2_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "ss.h"
#include "co.h"
#include "sd.h"
#include "diag_menu2.t"

extern leave();

unsigned char t, buf[2];
short ONE = 1;

struct fld_parms fld = {1, 1, 1, 0, &ONE, 0, 'a'};

main()
{
  putenv("_=diag_menu2");
  chdir(getenv("HOME"));
  
  sd_open(leave);
  ss_open();
  co_open();
  
  fix(diag_menu2);
  sd_screen_off();
  sd_clear_screen();
  sd_text(diag_menu2);
  sd_screen_on();
      
  sd_cursor(0, 20, 25);
  sd_echo_flag = 0;
  
  t = sd_input(&fld, 0, 0, buf, 0);
      
  leave();
}
leave()
{
  sd_close();
  ss_close();
  co_close();
  execlp("diagnostics", "diagnostics", 0);
  krash("leave", "diagnostics load", 1);
}

/* end of diag_menu2.c */
