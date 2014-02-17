/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Full function diagnostics menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/07/93   |  tjt  Added to mfc.
 *  06/04/94   |  tjt  Add old loop_test.
 *  07/12/94   |  tjt  Add set busy to lockout other processes.
 *  02/18/95   |  tjt  Add port disable.
 *  06/03/95   |  tjt  Add port input by name.
 *  09/21/95   |  tjt  Fix change ports always.
 *-------------------------------------------------------------------------*/
static char diag_menu1_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "diag_menu1.t"
#include "loop_diags.t"
#include "eh_nos.h"

extern leave();
void catcher(int signum); // Signal handler

short ONE = 1;
short LPL = 6;

struct fld_parms fld[] ={
  {20, 40, 20, 1, &ONE,   "Enter Code", 'a'},
  {21, 40, 20, 1, &LPL,   "Enter Port", 'a'},
  {22, 40, 20, 1, &ONE,   "Enter Speed (F/N)", 'a'},
  {23, 40, 20, 1, &ONE,   "Print? (Y/N)", 'a'},
};

short rm;
short ret;
short pid, status;
unsigned char t;
short i,j;
char speed[3] = {'F', 0, 0};

char buf[4][7] = {{0}, {"ALL"}, {"F"}, {"N"}};

char pparm[4] = {"A"};
char port_use[8];

extern unsigned char get_parms();
extern unsigned char get_print_parm();
extern unsigned char get_port_parm();
extern unsigned char change_speed();

long old = 0;

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  
  putenv("_=diag_menu1");
  chdir(getenv("HOME"));
  
  setpgrp();

  if (argc > 1)
  {
    if (*argv[1] == 'o') old = 1;
  }
  sd_open(leave);
  ss_open();
  co_open();
  
  fix(diag_menu1);
  fix(loop_diags);
  sd_screen_off();
  sd_clear_screen();
  if (old) sd_text(loop_diags);
  else sd_text(diag_menu1);
  sd_screen_on();   

  while(1)
  {
    *buf[0] = 0;                          /* clear code                      */
    *buf[3] = 'N';                        /* default print = no              */

    for (k = 0; k < 8; k++)
    {
      if (port_use[k]) po[k].po_status = 'x';
      port_use[k] = 0;
    }
    sd_prompt(&fld[0], 0);

    if (!old)
    {
      sd_prompt(&fld[1], 0);
      sd_cursor(0, fld[1].irow, fld[1].icol);
      sd_text(buf[1]);
    }
    sd_prompt(&fld[2], 0);
    sd_cursor(0, fld[2].irow, fld[2].icol);
    sd_text(buf[2]);

    sd_prompt(&fld[3], 0); 
    sd_cursor(0, fld[3].irow, fld[3].icol); 
    sd_text(buf[3]);

    t = sd_input(&fld[0], 0, &rm, buf[0], 0);

    if (t == EXIT) leave();
    
    *buf[0] = toupper(*buf[0]);
     
    switch (*buf[0])
    {
      case 'L':
      case 'R':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':  t = get_parms();
                 if (t == UP_CURSOR) break;
                 if (t == EXIT) leave();
                 load_plc_diag();
                 break;
                        
      default:       
      
                 eh_post(ERR_CODE, buf[0]);
                 break;
    }
  }
}

/*--------------------------------------------------------------------------*
 *      load and go on the plc_diag  when the keyboard is hit
 *      this parent sends a signal (SIGINT) to child.
 *--------------------------------------------------------------------------*/

load_plc_diag()
{
  register long k;
  
  sd_clear_screen();
  sd_text("* * *   Hit Return To Terminate Test   * * *");
  sd_tty_close();
  sd_close();
  sleep(1);
  
  signal(SIGTERM, catcher);
  
  if (old)
  {
    if ((pid = fork()) == 0)                /* child process                 */
    {
      ss_close();
      co_close();

      execlp("loop_test", "loop_test", buf[0], speed, buf[3], 0);
      eh_post(CRASH_MSG, "Program loop_test not found");
      exit(1);
    }
  }
  else
  {
    if ((pid = fork()) == 0)                /* child process                 */
    {
      ss_close();
      co_close();

      execlp("plc_diag", "plc_diag", buf[0], pparm, speed, buf[3], 0);
      eh_post(CRASH_MSG, "Program plc_diag not found");
      exit(1);
    }
  }
  t = getchar();

  kill(pid, SIGTERM);
  wait(&status);

  pid = 0;
  
  sd_open();
  sd_tty_open();
  sd_screen_off();
  
  fix(diag_menu1);
  fix (loop_diags);
  sd_clear_screen();
  if (old) sd_text(loop_diags);
  else sd_text(diag_menu1);
  sd_screen_on();

  return;
}
/*-------------------------------------------------------------------------*
 *  Catch SIGTERM
 *-------------------------------------------------------------------------*/
void catcher(int signum)
{
  if (pid) kill(pid, SIGTERM);
  leave();
}
/*-------------------------------------------------------------------------*
 *  Get Parameters
 *-------------------------------------------------------------------------*/
unsigned char get_parms()
{
  register unsigned char t;
   
  while (1)
  {
    t = get_port_parm();
       
    if (t == EXIT || t == UP_CURSOR || t == RETURN) return t;

    while (1)
    {
      t = change_speed();
         
      if (t == UP_CURSOR) break;
      if (t == EXIT || t == RETURN) return t;

      while (1)
      {
        t = get_print_parm();
        if (t == UP_CURSOR) break;
        return t;
      }
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Get Port Parameter
 *-------------------------------------------------------------------------*/
unsigned char get_port_parm()
{
  register long k, count, flag;
 
  if (old) return DOWN_CURSOR;

  while (1)
  {
    t = sd_input(&fld[1], 0, &rm, buf[1], 0);

    if (t == EXIT || t == UP_CURSOR) return t;
      
    memset(port_use, 0, 8);

    k = port_lookup(buf[1]);

    if (k < 0 || k > coh->co_ports)
    {
      eh_post(ERR_CODE, buf[1]);
      continue;
    }
    if (!k)                                /* all ports                      */
    {
      strcpy(pparm, "A");

      if (sp->sp_running_status == 'y')
      {
        eh_post(ERR_IS_CONFIG, 0);
        continue;
      }
      for (k = flag = 0; k < coh->co_ports; k++)
      {
        if (!(po[k].po_flags & IsFullFunction)) continue;
        if (po[k].po_status == 'd') flag = 1;
      }
      if (flag)
      {
        eh_post(LOCAL_MSG, "Diagnostics are running");
        continue;
      }
      for (k = 0; k < coh->co_ports; k++)
      {
        if (!(po[k].po_flags & IsFullFunction)) continue;
        port_use[k] = 1;
        po[k].po_status = 'd';
      }
      return t;
    }
    k--;                                  /* port subscript                  */
    
    if (po[k].po_status != 'x')
    {
      eh_post(LOCAL_MSG,"Port not available for diagnostics");
      continue;
    }
    sprintf(pparm, "%d", k);
    po[k].po_status = 'd';
    port_use[k] = 1;
    return t;
  }
}
/*-------------------------------------------------------------------------*
 *  Get Print Parameter
 *-------------------------------------------------------------------------*/
unsigned char get_print_parm()
{
  register unsigned char t;

  while (1)
  {
    t = sd_input(&fld[3], 0, &rm, buf[3], 0);

    if (t == EXIT || t == UP_CURSOR) return t;
      
    if (*buf[3] >= 'a' && *buf[3] <= 'z') *buf[3] -= 0x20;

    buf[3][1] = 0;
    sd_cursor(0, fld[3].irow, fld[3].icol);
    sd_text(buf[3]);

    switch (*buf[3])
    {
      case 'N':
      case 'Y':      return t;
                        
      default:       eh_post(ERR_CODE, buf[3]);
                     break;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Change Display Speed
 *-------------------------------------------------------------------------*/
unsigned char change_speed()
{
  register unsigned char t, *p;
  register long k;
   
  while (1)
  {
    t = sd_input(&fld[2], 0, &rm, buf[2], 0);

    if (t == EXIT || t == UP_CURSOR) return t;

    if (*buf[2] >= 'a' && *buf[2] <= 'z') *buf[2] -= 0x20;

    buf[2][1] = 0;
    sd_cursor(0, fld[2].irow, fld[2].icol);
    sd_text(buf[2]);

    strcpy(speed, buf[2]);

    if (old)
    {
      if (*buf[2] == 'F' || *buf[2] == 'N') return t;

      eh_post(ERR_CODE, buf[2]);
      continue;
    }
    switch (*buf[2])
    {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':      speed[0] = 'S';
                     speed[1] = *buf[2];
                     speed[2] = 0;
                     return t;

      case 'N':
      case 'F':
      case 'X':      return t;
                
      default:       eh_post(ERR_CODE, buf[2]);
                     break;
    }
  }
}
/*-------------------------------------------------------------------------*
 *     return to calling menu
 *-------------------------------------------------------------------------*/
leave()
{
  register long k;
  
  for (k = 0; k < 8; k++)
  {
    if (port_use[k]) po[k].po_status = 'x';
  }
  sd_close();
  ss_close();
  co_close();
  
  execlp("diagnostics", "diagnostics", 0);
  krash("leave", "diagnostics load", 1);
}

/* end of diag_menu1.c */
