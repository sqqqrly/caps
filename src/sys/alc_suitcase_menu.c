/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Suitcase diagnostics and utility functions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/23/94   |  tjt  Added to mfc.
 *  12/13/94   |  tjt  Add tests 4 & 5.
 *-------------------------------------------------------------------------*/
static char alc_suitcase_menu_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "alc_suitcase_menu.t"
#include "eh_nos.h"

extern leave();

short ONE = 1;
short FOUR = 4;

struct fld_parms fld[] ={
  {18,46,28,1,&ONE,"Enter Code",'a'},
  {19,46,28,1,&FOUR,"Enter Address", 'n'},
};
char incode[2] = {'L', 0};
char address[8] = {0};

short rm;
unsigned char t;

main(argc, argv)
int  argc;
char **argv;
{
  putenv("_=alc_suitcase_menu");
  chdir(getenv("HOME"));
  
  ss_open();
  sd_open(leave);

  fix(alc_suitcase_menu);
  sd_screen_off();
  sd_clear_screen();
  sd_text(alc_suitcase_menu);
  sd_screen_on();
  
  while (1)
  {
    t = sd_input(&fld[0], sd_prompt(&fld[0], 0), &rm, incode, 0);

    if (t == EXIT) leave();

    *incode= toupper(*incode);

    switch (*incode)
    {
      case 'L':
      case 'A':
      case 'D':
      case 'H':
      case '1':
      case '2':
      case '3': 
      case '4':
      case '5': break;

      default:  eh_post(ERR_CODE, incode);
                incode[0] = 0;
                break;
    }
    if (!incode[0]) continue;

    if (incode[0] == 'A')
    {
      t = sd_input(&fld[1], sd_prompt(&fld[1], 0), &rm, address, 0);
      if (t == EXIT) leave();
    }
    load_loop_test();
  }
}
/*--------------------------------------------------------------------------*
 *      load and go on the alc_diag  when the keyboard is hit
 *      terminate testing       
 *--------------------------------------------------------------------------*/
load_loop_test()
{
  short pid, status;

  if ((pid = fork()) == 0)                /* child process                   */
  {
    ss_close();
    execlp("alc_suitcase", "alc_suitcase", incode, address, 0);
    krash("load_loop_test", "load alc_suitcase", 1);
  }
  sd_clear_screen();
  sd_text("* * *   Hit Return To End Test   * * *");

  t = sd_keystroke(NOECHO);
  
  kill(pid, SIGTERM);
  
  wait(&status);                         /* wait for tlc_diag               */

  fix(alc_suitcase_menu);
  sd_screen_off();
  sd_clear_screen();
  sd_text(alc_suitcase_menu);
  sd_screen_on();
  
  return;
}
/*--------------------------------------------------------------------------*/
/*     return to calling menu                                               */
/*--------------------------------------------------------------------------*/
leave()
{
  ss_close();
  sd_close();
  execlp("diagnostics", "diagnostics", 0);
  krash("leave", "diagnostics load", 1);
}

/* end of alc_suitcase_menu.c */

