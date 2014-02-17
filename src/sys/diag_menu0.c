/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Basic function diagnostics menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/07/93   |  tjt  Added to mfc.
 *  07/01/94   |  tjt  Added position of SKU in test S.
 *  07/12/94   |  tjt  Added set busy to lockout other processes.
 *  02/18/95   |  tjt  Add port prompt.
 *  02/18/95   |  tjt  Add port disable.
 *  06/01/95   |  tjt  Add fast line test.
 *  06/03/95   |  tjt  Add port input by name.
 *  04/18/97   |  tjt  Add langguage.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char diag_men0_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "iodefs.h"
#include "global_types.h"
#include "co.h"
#include "ss.h"
#include "sd.h"
#include "diag_menu0.t"
#include "eh_nos.h"
#include "language.h"

extern leave();
extern catcher();

short pid, status;

#define LPORT 6

static short ONE = 1;
static short TWO = 2;
static short PORT = LPORT;

struct fld_parms fld[] ={
  {21, 48, 28, 1, &ONE,  "Enter Code",'a'},
  {23, 48, 28, 1, &ONE,  "Print (y/n)?", 'a'}, 
  {23, 48, 28, 1, &TWO,  "Enter Position", 'n'},
  {22, 48, 28, 1, &PORT, "Enter Port", 'a'},
};
char incode[2];
char yn[2] = {0};
char parm[8] = {0};
char port[LPORT + 1] = {0};
char device[16];

short rm;
unsigned char t;

long port_use[PortMax];

extern unsigned char get_port_parm();
extern unsigned char get_print_parm();
extern unsigned char get_sku_parm();

main(argc, argv)
int  argc;
char **argv;
{
  register long k;
  
  putenv("_=diag_menu0");
  chdir(getenv("HOME"));

  sd_open(leave);
  ss_open();
  co_open();
  
  fix(diag_menu0);
  sd_screen_off();
  sd_clear_screen();
  sd_text(diag_menu0);
  sd_screen_on();
  
  while (1)
  {
    for (k = 0; k < coh->co_ports; k++)
    {
      if (port_use[k]) po[k].po_status = 'x';
      port_use[k] = 0;
    }
    sd_cursor(0, 22, 1);
    sd_clear_line();
    sd_cursor(0, 23, 1);
    sd_clear_line();

    memset(incode, 0, 2);
    
    t = sd_input(&fld[0], sd_prompt(&fld[0], 0), &rm, incode, 0);

    if (t == EXIT) leave();
    *incode = toupper(*incode);

    t = get_port_parm();
    if (t == UP_CURSOR) continue;  

    if (*incode == 'L' || *incode == 'F')
    {
      t = get_print_parm();
      if (t == UP_CURSOR) continue;
    }
    if (*incode == 'S')
    {
      t = get_sku_parm();
      if (t == UP_CURSOR) continue;  
    }
    switch (*incode)
    {
      case 'T':  
      case 'F':
      case 'L': 
      case 'S':
      case 'H':
      case 'C':
      case 'M':
      case 'B':
      case 'Z':
      case '0':
      case '1':
      case '2':
      case '3':
      case '5': break;

      default:  
      
               eh_post(ERR_CODE, incode);
               *incode = 0;
               break;
    }
    if (!(*incode))     continue;
    if (t == UP_CURSOR) continue;

    if (*incode == 'T') load_util_test();
    else load_loop_test();
  }
}
/*--------------------------------------------------------------------------*
 *	load and go on the tlc_diag  when the keyboard is hit
 *	terminate testing						 
 *--------------------------------------------------------------------------*/
load_loop_test()
{
  long k;

  sd_clear_screen();
  sd_text("* * *   Hit Return To End Test   * * *");
  sd_tty_close();
  sd_close();
  sleep(1);

  signal(SIGTERM, catcher);
  
  if ((pid = fork()) == 0)		             /* child process		     */
  {
    ss_close();
    co_close();
    execlp("tlc_diag", "tlc_diag", incode, port, parm, 0);
    eh_post(CRASH_MSG, "Program tlc_diag not found");
    exit(1);
  }
  t = getchar();

  kill(pid, SIGTERM);
  wait(&status);			                   /* wait for tlc_diag	     */

  pid = 0;
  
  sd_open();
  sd_tty_open();
  sd_screen_off();

  fix(diag_menu0);
  sd_clear_screen();
  sd_text(diag_menu0);
  sd_screen_on();
    
  return;
}
/*--------------------------------------------------------------------------*
 *  Catch SIGTERM
 *--------------------------------------------------------------------------*/
catcher()
{
  if (pid) kill(pid, SIGTERM);
  leave();
}
/*--------------------------------------------------------------------------*
 *	 Go to tc_util
 *--------------------------------------------------------------------------*/
load_util_test()
{
  long pid, status;

  if ((pid = fork()) == 0)		             /* child process		     */
  {
    ss_close();
    co_close();
    execlp("tc_util", "tc_util", device, 0);
    eh_post(CRASH_MSG, "tc_util not found");
    exit(1);
  }
  wait(&status);			                   /* wait for tlc_diag	     */

  fix(diag_menu0);
  sd_screen_off();
  sd_clear_screen();
  sd_text(diag_menu0);
  sd_screen_on();
    
  return;
}
/*--------------------------------------------------------------------------*
 *  Get Port
 *--------------------------------------------------------------------------*/
unsigned char get_port_parm()
{
  register unsigned char t;
  register long k, flag;
  
  memset(port, 0, LPORT + 1);
    
  while (1)
  {
    t = sd_input(&fld[3], sd_prompt(&fld[3], 0), 0, port, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) break;

    k = port_lookup(port);

    if (k < 0 || k > coh->co_ports)
    {
      eh_post(ERR_CODE, port);
      continue;
    }
    if (!k)                                 /* all ports                     */
    {
      if (sp->sp_running_status == 'y')
      {
        eh_post(ERR_IS_CONFIG, 0);
        continue;
      }
      for (k = flag = 0; k < coh->co_ports; k++)
      {
        if (!(po[k].po_flags & IsBasicFunction)) continue;
        if (po[k].po_status == 'd') flag = 1;
      }
      if (flag)
      {
        eh_post(LOCAL_MSG, "Diagnostics are running");
        continue;
      }
      memset(port_use, 0, PortMax);
      
      for (k = flag = 0; k < coh->co_ports; k++)
      {
        if (!(po[k].po_flags & IsBasicFunction)) continue;
        po[k].po_status = 'd';
        port_use[k] = 1;
      }
      strcpy(port, "All");
      return t;
    }
    k--;                                   /* port subscript                 */
    
    if (po[k].po_status != 'x')
    {
      eh_post(LOCAL_MSG, "Port is not available for diagnostics");
      continue;
    }
    sprintf(port, "%d", k);
    port_use[k] = 1;
    po[k].po_status = 'd';
    
    strcpy(device, po[k].po_name);
    
    return t;
  }
}
/*--------------------------------------------------------------------------*
 *  Get Print Parm
 *--------------------------------------------------------------------------*/
unsigned char get_print_parm()
{
  register unsigned char t;
  char buf[2];
  
  memset(yn, 0, 2);
  memset(buf, 0, 2);
    
  while (1)
  {
    t = sd_input(&fld[1], sd_prompt(&fld[1], 0), 0, buf, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) break;

    *yn = code_to_caps(*buf);
    if (*yn == 'y' || *yn == 'n' || *yn == 'b') break;
    eh_post(ERR_CODE, buf);
  }
  if (*yn == 'y')      strcpy(parm, "-print");    
  else if (*yn == 'b') strcpy(parm, "-bays");
  else memset(parm, 0, 4);
  return t;
}
/*-------------------------------------------------------------------------*
 *  Get SKU Parm
 *-------------------------------------------------------------------------*/
unsigned char get_sku_parm()
{
  register unsigned char t;
  
  sd_cursor(0, 22, 47);
  sd_text("Shows only 6 digits of the SKU.");
  sd_cursor(0, 23, 47);
  sd_text("Select the starting position.");

  memset(parm, 0, 4);
    
  while (1)
  {
    t = sd_input(&fld[2], sd_prompt(&fld[2], 0), 0, parm, 0);
    if (t == EXIT) leave();
    break;
  }
  return t;
}
/*--------------------------------------------------------------------------*
 *     return to calling menu
 *--------------------------------------------------------------------------*/
leave()
{
  register long k;
  
  for (k = 0; k < coh->co_ports; k++)
  {
    if (port_use[k]) po[k].po_status = 'x';
  }
  sd_close();
  ss_close();
  co_close();
  execlp("diagnostics", "diagnostics", 0);
  krash("leave", "diagnostics load", 1);
}

/* end of diag_menu0.c */

