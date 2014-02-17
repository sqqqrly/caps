/*------------------------------------------------------------------------*
 *  Custom Code:    LOCATION - for Walgreens location database file.
/*------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Total function diagnostics menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/23/94   |  tjt  Added to mfc.
 *  07/06/94   |  tjt  Add 88 tests replaced by - and *0.
 *  07/06/94   |  tjt  Add caller name to execl to alc_diag.
 *  07/12/94   |  tjt  Add make busy to lockout other processes.
 *  08/22/94   |  tjt  Add self tests 11 & 12.
 *  09/15/94   |  tjt  Add location database.
 *  02/18/95   |  tjt  Add port tests.
 *  02/18/95   |  tjt  Add port disable.
 *  06/01/95   |  tjt  Add fast line tets
 *  04/18/97   |  tjt  Add language and code_to_caps.
 *-------------------------------------------------------------------------*/
static char diag_menu3_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include "global_types.h"
#include "iodefs.h"
#include "ss.h"
#include "co.h"
#include "sd.h"
#include "language.h"
#include "diag_menu3.t"
#include "eh_nos.h"

extern leave();
extern catcher();

short pid, status;

#define LPORT 6

static short ONE = 1;
static short PORT = LPORT;

struct fld_parms fld[] ={
  {20,46,20,1,&ONE, "Enter Code",'a'},
  {21,46,20,1,&PORT,"Enter Port",'a'},
  {22,46,20,1,&ONE, "Print (y/n)?", 'a'}, 
  
};
char incode[2];
char yn[2] = {0};
char port[LPORT + 1] = {0};
char parm[8] = {0};

short rm;
unsigned char t;

long port_use[PortMax];

extern unsigned char get_port_parm();
extern unsigned char get_print_parm();

main(argc, argv)
int  argc;
char **argv;
{
  register long k;
  
  putenv("_=diag_menu3");
  chdir(getenv("HOME"));

  sd_open(leave);
  ss_open();
  co_open();
  
  fix(diag_menu3);
  sd_screen_off();
  sd_clear_screen();
  sd_text(diag_menu3);
  sd_screen_on();
  
  while (1)
  {
    for (k = 0; k < coh->co_ports; k++)
    {
      if (port_use[k]) po[k].po_status = 'x';
      port_use[k] = 0;
    }
    memset(incode, 0, 2);
    memset(port, 0, LPORT + 1);
    strcpy(parm, "-no");

    sd_prompt(&fld[0], 0);
    sd_prompt(&fld[1], 0);
    t = sd_input(&fld[0], 0, 0, incode, 0);

    if (t == EXIT) leave();
    *incode = toupper(*incode);

    t = get_port_parm();
    if (t == UP_CURSOR) continue;
    
    if (*incode == 'L' || *incode == 'F')
    {
      t = get_print_parm();
      if (t == UP_CURSOR) continue;
    }
    switch (*incode)
    {
      case 'F':
      case 'L':
      case 'H': 
      case 'B': break;
      
      case 'S': 
#ifdef LOCATION
                if (check_location()) break;

                eh_post(ERR_CODE, incode);
                *incode = 0;
#endif
                break;
                
      case '1':
      case '2':
      case '3':
      case '4':
      case '5': break;

      default:  
      
                eh_post(ERR_CODE, incode);
                *incode = 0;
                break;
    }
    if (!(*incode)) continue;

    load_loop_test();
  }
}
/*--------------------------------------------------------------------------*
 *	load and go on the alc_diag  when the keyboard is hit
 *	terminate testing						 
 *--------------------------------------------------------------------------*/
load_loop_test()
{
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
    execlp("alc_diag", "alc_diag", incode, port, parm, "diag_menu3", 0);
    eh_post(CRASH_MSG, "Program alc_diag not found");
    exit(1);
  }
  t = getchar();

  kill(pid, SIGTERM);
  wait(&status);			                   /* wait for alc_diag	     */

  pid = 0;
  
  sd_open();
  sd_tty_open();
  sd_screen_off();

  fix(diag_menu3);
  sd_clear_screen();
  sd_text(diag_menu3);
  sd_screen_on();
    
  return;
}
/*--------------------------------------------------------------------------*
 *  Catcher SIGTERM
 *--------------------------------------------------------------------------*/
catcher()
{
  if (pid) kill(pid, SIGTERM);
  leave();
}
#ifdef LOCATION
/*--------------------------------------------------------------------------*
 *  Check SKU Support Or Location Database
 *--------------------------------------------------------------------------*/
check_location()
{
  char name[64];
  struct stat x;
  long age_pm_db, age_loc_asc, age_loc_db;
  
  age_pm_db = age_loc_asc = age_loc_db = 0;

  sprintf(name, "%s/location.asc", getenv("DBPATH"));
  if (stat(name, &x) >= 0) age_loc_asc = x.st_mtime;

  sprintf(name, "%s/pmfile.dat", getenv("DBPATH"));
  if (stat(name, &x) >= 0) age_pm_db = x.st_mtime;

  sprintf(name, "%s/location.dat", getenv("DBPATH"));
  if (stat(name, &x) >= 0) age_loc_db = x.st_mtime;

  if (sp->sp_sku_support == 'y')
  {
    if (age_pm_db > age_loc_asc) system("$DBPATH/location.udx </dev/null");
    sprintf(name, "%s/location.asc", getenv("DBPATH"));
    if (stat(name, &x) >= 0) age_loc_asc = x.st_mtime;
  }
  else if (!age_loc_asc)
  {
    eh_post(LOCAL_MSG, "*** No Location File");
    return 0;
  }
  if (age_loc_asc > age_loc_db)
  {
    system("$DBPATH/location.ldx >/dev/null; Bdex location");
  }
  return 1;
}
#endif
/*--------------------------------------------------------------------------*
 *  Get Port Parameter
 *--------------------------------------------------------------------------*/
unsigned char get_port_parm()
{
  register unsigned char t;
  register long k, flag;
  
  memset(port, 0, LPORT + 1);
    
  while (1)
  {
    t = sd_input(&fld[1], sd_prompt(&fld[1], 0), 0, port, 0);
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
        if (!(po[k].po_flags & IsTotalFunction)) continue;
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
        if (!(po[k].po_flags & IsTotalFunction)) continue;
        po[k].po_status = 'd';
        port_use[k] = 1;
      }
      strcpy(port, "All");
      return t;
    }
    k--;                                  /* port subscript                  */

    if (po[k].po_status != 'x')
    {
      eh_post(LOCAL_MSG, "Port is not available for diagnostics");
      continue;
    }
    sprintf(port, "%d", k);
    port_use[k] = 1;
    po[k].po_status = 'd';
    
    return t;
  }
}
/*--------------------------------------------------------------------------*
 *  Get Print Parameter
 *--------------------------------------------------------------------------*/
unsigned char get_print_parm()
{
  register unsigned char t;
  
  while (1)
  {
    memset(yn, 0, 2);
    
    sd_prompt(&fld[2], 0);
    t = sd_input(&fld[2], 0, 0, yn, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return t;

    *yn = code_to_caps(*yn);
    if (*yn == 'y' || *yn == 'n') break;
    eh_post(ERR_YN, 0);
  }
  if (*yn == 'y') strcpy(parm, "-print");    
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

/* end of diag_menu3.c */

