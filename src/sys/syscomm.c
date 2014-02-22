#define NEW_RP
/* #define OLD_RP */
/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Custom Code:	  HANES  - on restoreplace check client and engine.
 *                  NEW_RP - revised restoreplace.
 *                  OLD_RP - orginal restoreplace.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    System commands screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/10/93   |  tjt  Added to mfc.
 *  05/16/94   |  tjt  Added check_db on restoreplace.
 *  07/12/94   |  tjt  Added check busy in mark/restoreplace.
 *  12/31/94   |  tjt  Add setpgrp/sleep to logout for sco.
 *  01/23/95   |  tjt  Rename logout and logoff.
 *  02/17/95   |  tjt  Add port enable and disable.
 *  04/23/95   |  tjt  Add order file rebuild option.
 *  06/03/95   |  tjt  Add port input by name
 *  06/04/95   |  tjt  Add sp_port_by_name display.
 *  06/30/95   |  tjt  Add co_st_changed flag on changed sku table.
 *  07/01/95   |  tjt  Add symbolic queue names.
 *  07/14/96   |  tjt  Add 'are' to 'There orders in the system'.
 *  06/11/98   |  tjt  Fix check_db for Bard only.
 *  07/07/98   |  tjt  Add IBM screens.
 *  07/16/98	|  tjt  Add alloc_init to update allocations.
 *  07/21/98   |  tjt  Add options for ibm.
 *  08/01/98   |  tjt  Fix message wait at Hanes on restoreplace.
 *-------------------------------------------------------------------------*/
static char syscomm_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "message_types.h"
#include "caps_messages.h"
#include "kernel.h"
#include "kernel_types.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "syscomm.t"
#include "eh_nos.h"
#include "getparms.h"
#include "zone_status.h"

long MP_TIMEOUT = 60;
long RP_TIMEOUT = 600;

char rp_status = 0;
extern leave();

#define LPORT 6

short ONE = 1;
short TWO = 2;
short SIX = 6;
short PORT = 6;

struct fld_parms fld1 = {19,48,25,1,&ONE,"Enter Code",'a'};
struct fld_parms fld2 = {20,48,25,1,&ONE,"Proceed ? (y/n)", 'a'};
struct fld_parms fld3 = {20,48,25,1,&SIX,"Adjust (+/- Sec)", 'a'};
struct fld_parms fld4 = {20,48,25,1,&PORT,"Enter Port", 'a'};

char pname[40], parm[20], text[80];

long uw_orders, q_orders;

unsigned char list[] = {ClientMessageEvent, MarkplaceEvent, RestoreplaceEvent,
  PortDisableEvent, PortEnableEvent};

main()
{
  register long k;
  register unsigned char t;
  char buf[2];
  
#ifdef DEBUG
  fprintf(stderr, "syscomm starting ...\n");
  fflush(stderr);
#endif
  
  putenv("_=syscomm");                    /* program name                    */
  chdir(getenv("HOME"));
  
  open_all();
  
  RP_TIMEOUT = sp->sp_rp_timeout;
  MP_TIMEOUT = sp->sp_mp_timeout;

#ifdef DEBUG
  fprintf(stderr, "setup screen()\n");
  fflush(stderr);
#endif
  
  fix(syscomm);
  sd_screen_off();
  sd_clear_screen();
  sd_text(syscomm);
  sd_screen_on();
  
  count_orders();                         /* count uw and queued orders      */

  while (1)
  {
    sd_cursor(0, 19, 1);						/* F070798 */
    sd_clear_line();
    sd_cursor(0, 20, 1);
    sd_clear_line();

    memset(buf, 0, 2);

    sd_prompt(&fld1, 0);
    t = sd_input(&fld1, 0, 0, buf, 0);
    if (t ==  EXIT) leave();

    sd_cursor(0, 20, 1);
    sd_clear_line();

    if (t == UP_CURSOR) continue;
    
    *buf = toupper(*buf);

    if(!LEGAL(*buf))
    {
      eh_post(ERR_CODE, buf);
      continue;
    }
    sd_cursor(0, 20, 1);
    sd_clear_rest();

    *pname = 0;
    *parm  = 0;
    
    switch(*buf)
    {
      case 'A': stop_feed(); break;

      case 'B': strcpy(pname, "backupmm"); break;

      case 'C': strcpy(pname, "change_op_info"); break;

      case 'D': strcpy(pname, "diagnostics"); break;

      case 'F': strcpy(pname, "record_format_srn"); break;

      case 'G': strcpy(pname, "transac_format_srn"); break;
      
      case 'I': initialize(); break;

      case 'M': markplace(); break;

      case 'N': restoreplace(); break;

      case 'P': strcpy(pname, "printer_control");
                break;

      case 'Q': reconfigure(); break;
     
      case 'R': recovery(); break;

      case 'S': shutdown_request(); break;

      case 'T': adjust_time(); break;

      case 'U': disable_port();
                break;
      
      case 'V': enable_port();
                break;
      
      case 'Z': forced_recovery(); break;
#ifdef IBM
      case 'E': strcpy(pname, "ibm_production"); break;		/* F070798 */
      
      case 'H': strcpy(pname, "ibm_maint"); break;				/* F070798 */
      
      case 'O': strcpy(pname, "ibm_release"); break;			/* F070798 */
#endif
    }                                
    if (*pname)
    {
      close_all();
      sd_close();
      execlp(pname, pname, parm, 0);
      open_all();
      sprintf(text, "Program %s Not Found", pname);
      eh_post(CRASH_MSG, text);
    }
  }                                       /* end while                       */
}
/*-------------------------------------------------------------------------*
 *  Stop Order Feed
 *-------------------------------------------------------------------------*/
stop_feed()
{
  static TPickline pickline = 0;

  if (sp->sp_running_status != 'y')
  {
    eh_post(LOCAL_MSG, "System is not running");
    return;
  }
  if (sp->sp_config_status != 'y')
  {
    eh_post(ERR_NO_CONFIG, 0);
    return;
  }
  if (!get_answer(0)) return 0;
  
  message_put(0, PicklineStopRequest, &pickline, sizeof(TPickline));
  return;
}

/*-------------------------------------------------------------------------*
 *  Initialize
 *-------------------------------------------------------------------------*/
initialize()
{
  register long k;

  if (sp->sp_running_status == 'y')
  {
    eh_post(ERR_IS_CONFIG, 0);
    return 0;
  }
  if (q_orders || uw_orders)
  {
    strcpy(text, "*** There are orders in the system.  Use reconfigure. ***");
    sd_cursor(0, 22, 40 - strlen(text) / 2);
    sd_text(text);
    return 0;
  }
  for (k = 0; k < coh->co_ports; k++)
  {
    if (po[k].po_status == 'd')
    {
      eh_post(LOCAL_MSG, "Diagnostics are running");
      return;
    }
  }
  if (!get_answer(0)) return;

  strcpy(pname, "initialization");
  strcpy(parm,  "initialize");
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Order Purge & Reboot
 *-------------------------------------------------------------------------*/
order_purge()
{
  register long k;
  char what[8];
  
  if (sp->sp_running_status == 'y')
  {
    eh_post(LOCAL_MSG, "System not in markplace");
    return;
  }
  if (sp->sp_in_process_status != 'x')
  {
    eh_post(LOCAL_MSG, "System is busy");
    return;
  }
  strcpy(what, "Purge");

  for (k = 0; k < PicklineMax; k++)
  {
    if (oc->oc_tab[k].oc_queue[OC_HOLD].oc_count > 0 || 
        oc->oc_tab[k].oc_queue[OC_UW].oc_count   > 0 || 
        oc->oc_tab[k].oc_queue[OC_HIGH].oc_count > 0 || 
        oc->oc_tab[k].oc_queue[OC_MED].oc_count  > 0 || 
        oc->oc_tab[k].oc_queue[OC_LOW].oc_count  > 0)
    {
      strcpy(what, "Rebuild");
      break;
    }
  }
  if (!get_answer(0)) return;

  sd_clear_screen();
  sleep(2);
  sd_close();
  close_all();

  if (fork() == 0)
  {
    setpgrp();
    execlp("caps_logout", "caps_logout", "I_Really_Mean_It", what, 0);
    krash("order_purge", "caps_logout load", 1);
  }
  sleep(1);
  execlp("op_logoff", "op_logoff", 0);
  krash("order_purge", "op_logoff load", 1);
}
 
/*-------------------------------------------------------------------------*
 *  Markplace
 *-------------------------------------------------------------------------*/
markplace()
{
  if (sp->sp_running_status != 'y')
  {
    eh_post(LOCAL_MSG, "System is not running");
    return;
  }
  if (sp->sp_in_process_status != 'x')
  {
    eh_post(LOCAL_MSG, "System is busy");
    return;
  }
  if (!get_answer(0)) return;

  sd_wait();
  if (message_wait(0, MarkplaceRequest, 0, 0, -1, MP_TIMEOUT) < 0)
  {
    eh_post(LOCAL_MSG, "*** CAPS Is Not Responding");
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Restoreplace - F080798
 *-------------------------------------------------------------------------*/
restoreplace()
{
  register long k;
  long pid, status, timeout;
  char pstats[6], newstat, rpstatus;
  
  if (sp->sp_running_status == 'y')
  {
    eh_post(LOCAL_MSG, "System is already running");
    return;
  }
  if (sp->sp_in_process_status != 'x')
  {
    eh_post(LOCAL_MSG, "System is busy");
    return;
  }
  if (sp->sp_config_status != 'y')
  {
    eh_post(ERR_NO_CONFIG, 0);
    return;
  }
#ifdef HANES
  if (system("find_progs caps_client caps_server hanes_engine"));
  {
    eh_post(LOCAL_MSG, "Check Network/HanesEngine");
  }
#endif
  
  for (k = 0; k < coh->co_ports; k++)
  {
    if (po[k].po_disabled == 'y') continue;  /* diags ok if disabled         */
    if (po[k].po_status == 'd')
    {
      eh_post(LOCAL_MSG, "Diagnostics are running");
      return;
    }
  }
  if (!get_answer(0)) return;

#ifdef IBM
  system("etc/check_db 1>>dat/log/errlog 2>&1");
  system("alloc_init   1>>dat/log/errlog 2>&1");			/* ibm - F071698	*/
#endif

  sd_wait();

#ifdef BARD
  system("etc/check_db 1>dat/log/restore.log 2>&1"); /* F061198 */
#endif
  
#ifdef NEW_RP
  memset(pstats, 0, 6);
  newstat = 0;
  rpstatus = 0;									/* local status						  */
  rp_status = 0;                       	/* global event arrived				  */
    
  timeout = RP_TIMEOUT - 5;

  message_put(0, RestoreplaceRequest, 0, 0);
  sleep(5);
  
  while (timeout > 0)
  {
#ifdef DEBUG
    fprintf(stderr, "timeout=%3d rp_status=%d\n", timeout, rp_status);
#endif
    
    for (k = 0; k < coh->co_ports && k < 6; k++)
    { 
      if (po[k].po_id <= 0) break;
      
#ifdef DEBUG
       fprintf(stderr, "po[%d].po_status = %c\n", k, po[k].po_status);
#endif
      
      if (po[k].po_disabled == 'y') 
      {
        newstat = 'd';
        sprintf(text, "Port %s Disabled", po[k].po_name);
      }
      else
      {
        if (po[k].po_status == 'n') 
        {
          newstat = 'n';
          sprintf(text, "Port %s Initialized           ", po[k].po_name);
        }
        if (po[k].po_status == 'i')
        {
          newstat = 'i';
          sprintf(text, "Port %s Being Initialized     ", po[k].po_name);
        }
        if (po[k].po_status == 'y') 
        {
          newstat = 'y';
          sprintf(text, "Port %s OK                    ", po[k].po_name);
        }
        if (po[k].po_status == 'x') 
        {
          newstat = 'x';
          sprintf(text, "Port %s Being Initialized     ", po[k].po_name);
        }
      }
      if (pstats[k] != newstat)
      {
#ifdef DEBUG
         fprintf(stderr, "k=%d newstat=%c pstats=%d\n", k, newstat, pstats[k]);
#endif
        
        sd_cursor(0, 21 + (k/2), (k%2)*40 + 3);
        sd_text(text);
        pstats[k] = newstat;
      }
    }
    if (rp_status) break;
    
    if (sp->sp_running_status == 'y' && !rp_status) 
    {  
      eh_post(LOCAL_MSG, "CAPS Is Running OK");
    }
    if (sp->sp_in_process_status == 'n'&& rpstatus != 'n') 
    {
      eh_post(LOCAL_MSG, "CAPS Checking Hardware");
    }
    if (sp->sp_in_process_status == 'x' && rpstatus == 'n') 
    {
      eh_post(LOCAL_MSG, "CAPS Is Checking Tables");
    }
#ifdef DEBUG
    fprintf(stderr, "in_process=%c rpstatus=%c running_status=%c\n",
	    sp->sp_in_process_status, rpstatus, sp->sp_running_status);
    fflush(stderr);
#endif
    rpstatus = sp->sp_in_process_status;
    timeout -= 1;
    sleep(1);
  }
  for (k = 0; k < 3; k++)
  {
    sd_cursor(0, 21 + k, 1);
    sd_clear_line();
  }
  if (sp->sp_running_status != 'y')
  {
    eh_post(LOCAL_MSG, "Restore Has Failed - Do Diags or Z");
    return;
  }
#endif

#ifdef OLD_RP
  if (message_wait(0, RestoreplaceRequest, 0, 0, -1, RP_TIMEOUT) < 0)
  {
    eh_post(LOCAL_MSG, "*** CAPS Is Not Responding");
    return;
  }
#endif
  
#ifndef IBM
  if (coh->co_st_changed == 1)
  {
    sd_cursor(0, 22, 25);
    sd_text("* * *   Updating Picks   * * *");

    if (fork() == 0)
    {
      close_all();
      execlp("reconfigure_orders", "reconfigure_orders", 0);
      krash("restoreplace", "load reconfigure_orders", 1);
    }
    pid = wait(&status);
    if (pid < 0) krash("restoreplace", "reconfigure_orders failed", 1);
    else if (status) eh_post(LOCAL_MSG, "Orphan Picks - See Printer");

    sd_cursor(0, 22, 1);
    sd_clear_line();
  }
#endif
  return;
}
/*-------------------------------------------------------------------------*
 *  Reconfigure
 *-------------------------------------------------------------------------*/
reconfigure()
{
  register struct zone_item *z;
  register long k;

  if (sp->sp_running_status == 'y')
  {
    strcpy(text,
      "*** Need to (1) stop order feed (2) pick clean (3) markplace ***");
    sd_cursor(0, 21, 40 - strlen(text) / 2);
    sd_text(text);
    eh_post(ERR_IS_CONFIG, 0);
    return 0;
  }
  if (!uw_orders && !q_orders)
  {
    strcpy(text, "*** There are no orders in the system. Will configure. ***");
    sd_cursor(0, 21, 40 - strlen(text) / 2);
    sd_text(text);
    if (get_answer(0))
    {
      strcpy(pname, "initialization");
      strcpy(parm,  "initialize");
    }
    return 0;
  }
  if (get_answer(0))
  {
    strcpy(pname, "initialization");
    strcpy(parm,  "reconfigure");
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Recovery()
 *-------------------------------------------------------------------------*/
recovery()
{
  register struct zone_item *z;
  register long k;

  if (sp->sp_config_status != 'y')
  {
    eh_post(ERR_NO_CONFIG, 0);
    return;
  }
  if (sp->sp_running_status == 'y')
  {
    strcpy(text,
      "*** Need to markplace before recovery, but, before that, ***");
    sd_cursor(0, 21, 40 - strlen(text) / 2);
    sd_text(text);
    strcpy(text,
      "*** (1) stop order feed and (2) pick clean, if you can. ***");
    sd_cursor(0, 22, 40 - strlen(text) / 2);
    sd_text(text);
    eh_post(ERR_IS_CONFIG, 0);
    return 0;
  }
  if (!uw_orders && !q_orders)
  {
    strcpy(text, "*** There are no orders in the system. Will configure. ***");
    sd_cursor(0, 21, 40 - strlen(text) / 2);
    sd_text(text);
    if (get_answer(0))
    {
      strcpy(pname, "initialization");
      strcpy(parm,  "initialize");
    }
    return 0;
  }
  else if (uw_orders)
  {
    strcpy(text, 
      "*** The underway orders in the system will be recovered. ***");
    sd_cursor(0, 21, 40 - strlen(text) / 2);
    sd_text(text);
  }
  if (get_answer(0))
  {
    strcpy(pname, "initialization");
    strcpy(parm,  "recover");
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Forced Recovery()
 *-------------------------------------------------------------------------*/
forced_recovery()
{
  register struct zone_item *z;
  register long k;

  strcpy(text, "* * * W A R N I N G * * *");
  sd_cursor(0, 21, 40 - strlen(text) / 2);
  sd_text(text);
  strcpy(text, "*** Only do this after rebooting the system ***");
  sd_cursor(0, 22, 40 - strlen(text) / 2);
  sd_text(text);

  if (get_answer(0))
  {
    sp->sp_in_process_status = 'x';
    sp->sp_init_status    = 'n';
    sp->sp_config_status  = 'n';
    sp->sp_running_status = 'n';

    strcpy(pname, "initialization");
    strcpy(parm,  "recover");
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Adjust Time
 *-------------------------------------------------------------------------*/
adjust_time()
{
  unsigned char t, buf[8], yn[2], date1[64], date2[64];
  long now, adjust, then, pid, status;
  
  if (sp->sp_running_status == 'y')
  {
    eh_post(ERR_IS_CONFIG, 0);
    return 0;
  }
  sd_prompt(&fld3, 0);

  time(&now);
  sprintf(date1, "Current  Time: %24.24s", ctime(&now));
  sd_cursor(0, 19, 25); sd_text(date1);

  memset(buf, 0, 8);
  
  while (1)
  {
    t = sd_input(&fld3, 0, 0, buf, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) break;
  
    sscanf(buf, "%d", &adjust);

    time(&now);
    then = now + adjust;
    
    sprintf(date1, "Current  Time: %24.24s", ctime(&now));
    sprintf(date2, "Adjusted Time: %24.24s", ctime(&then));
  
    sd_cursor(0, 19, 25); sd_text(date1);
    sd_cursor(0, 20, 25); sd_text(date2);

    if (get_answer(1)) 
    {
      if (fork() == 0)
      {
        execlp("adjust_time", "adjust_time", buf, 0);
        krash("adjust_time", "load adjust_time", 1);
      }
      pid = wait(&status);
      if (pid < 0 || status) krash("adjust_time", "adjust_time failed", 1);
      break;
    }
  }
  sd_cursor(0, 16, 1);
  sd_clear_rest();
  return;
}
/*-------------------------------------------------------------------------*
 *  Shutdown Request
 *-------------------------------------------------------------------------*/
shutdown_request()
{
  if (sp->sp_running_status == 'y')
  {
    eh_post(ERR_IS_CONFIG, 0);
    return;
  }
  if (sp->sp_in_process_status == 's')
  {
    eh_post(LOCAL_MSG, "Shutdown Already In Process");
  }
  if (get_answer(0)) 
  {
    eh_post(ERR_SD_DONE, 0);
    sd_clear_screen();
    sleep(2);
    sd_close();
    close_all();
    if (fork() == 0)
    {
      setpgrp();
      execlp("caps_logout", "caps_logout", "I_Really_Mean_It", "Shutdown", 0);
      krash("shutdown_request", "caps_logout load", 1);
    }
    sleep(1);
    execlp("op_logoff", "op_logoff", 0);
    krash("shutdown_request", "op_logoff load", 1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Enable Port
 *-------------------------------------------------------------------------*/
enable_port()
{
  long count, disabled;
  TPort k;
  unsigned char t;
  char buf[LPORT + 1], yn[2];
  
  show_ports(&count, &disabled);

  if (!disabled)
  {
    eh_post(LOCAL_MSG, "No ports are disabled");
    return;
  }
  memset(buf, 0, LPORT + 1);

#ifdef IBM
  if (sp->sp_running_status == 'y')
  {
    eh_post(LOCAL_MSG, "Markplace To Enable");
    return;
  }
#endif
  while (1)
  {
    sd_prompt(&fld4, 0);
    t = sd_input(&fld4, 0, 0, buf, 0);
    if (t ==  EXIT) leave();
    if (t == UP_CURSOR) return;

    k = port_lookup(buf);
    
    if (k < 0 || k > count)
    {
      eh_post(LOCAL_MSG, "Invalid port number");
      continue;
    }
    if (!k)                                /* all ports                      */
    {
      if (sp->sp_running_status == 'y')
      {
        eh_post(LOCAL_MSG, "Markplace To Do All");
        continue;
      }
      for (k = 0; k < coh->co_ports; k++)
      {
        po[k].po_disabled = 'n';
      }
      show_ports(&count, &disabled);
      return;
    }
    k--;                                  /* port subscript                  */
    
    if (po[k].po_disabled == 'n')
    {
      eh_post(LOCAL_MSG, "Port is not disabled");
      continue;
    }
    if (sp->sp_running_status == 'y')
    {
      if (!get_answer(1)) return;

      sd_wait();
#ifdef DEBUG
  fprintf(stderr, "Enable Request to=%d port=%d\n", po[k].po_id, k);
  fflush(stderr);
#endif
      if (message_wait(po[k].po_id, PortEnableRequest, &k,
          sizeof(TPortMessage), -1, RP_TIMEOUT) < 0)
      {
        eh_post(LOCAL_MSG, "*** CAPS Is Not Responding");
      }
    }
    else po[k].po_disabled = 'n';
  
    show_ports(&count, &disabled);
    return;
  }
}
/*-------------------------------------------------------------------------*
 *  Disable Port
 *-------------------------------------------------------------------------*/
disable_port()
{
  long count, disabled;
  TPort k;
  unsigned char t;
  char buf[16], yn[2];
  
  show_ports(&count, &disabled);

  if (disabled >= count)
  {
    eh_post(LOCAL_MSG, "All ports are disabled");
    return;
  }
  memset(buf, 0, sizeof(buf));

#ifdef IBM
  if (sp->sp_running_status == 'y')
  {
    eh_post(LOCAL_MSG, "Markplace To Disable");
    return;
  }
#endif
  while (1)
  {
    sd_prompt(&fld4, 0);
    t = sd_input(&fld4, 0, 0, buf, 0);
    if (t ==  EXIT) leave();
    if (t == UP_CURSOR) return;

    k = port_lookup(buf);
    
    if (k < 0 || k > count)
    {
      eh_post(LOCAL_MSG, "Invalid port number");
      continue;
    }
    if (!k)                                /* all ports                      */
    {
      if (sp->sp_running_status == 'y')
      {
        eh_post(LOCAL_MSG, "Markplace To Do All");
        continue;
      }
      for (k = 0; k < coh->co_ports; k++)
      {
        po[k].po_disabled = 'y';
        po[k].po_status   = 'x';
      }
      show_ports(&count, &disabled);
      return;
    }
    k--;                                   /* port subscript                 */

    if (po[k].po_disabled == 'y')
    {
      eh_post(LOCAL_MSG, "Port is already disabled");
      continue;
    }
    if (sp->sp_running_status == 'y')
    {
      if (!get_answer(1)) return;
      sd_wait();

#ifdef DEBUG
  fprintf(stderr, "Disable Request to=%d port=%d\n", po[k].po_id, k);
  fflush(stderr);
#endif
      if (message_wait(po[k].po_id, PortDisableRequest, &k,
          sizeof(TPortMessage), -1, MP_TIMEOUT) < 0)
      {
        eh_post(LOCAL_MSG, "*** CAPS Is Not Responding");
      }
    }
    else 
    {
      po[k].po_disabled = 'y';
      po[k].po_status   = 'x';
    }
    show_ports(&count, &disabled);
    return;
  }
}
/*-------------------------------------------------------------------------*
 *  Show Port Status
 *-------------------------------------------------------------------------*/
show_ports(count, disabled)
long *count, *disabled;
{
  char text[80], work[32];
  register long k;
  register struct port_item *p;
  
  sd_cursor(0, 21, 1);
  sd_clear_line();
  sd_cursor(0, 20, 1);
  sd_clear_line();
  sd_cursor(0, 21, 20);
  
  *count = *disabled = 0;
  strcpy(text, "Disabled port(s) are ");
  
  for (k = 0, p = po; k < coh->co_ports; k++, p++)
  {
    if (!(p->po_id)) continue;
    *count += 1;                           /* count useful ports             */
    if (p->po_disabled == 'y')
    {
      if (sp->sp_port_by_name) sprintf(work, "%s", basename(p->po_name));
      else sprintf(work, "%d", k);

      if (strlen(text) + strlen(work) >= 80)
      {
        sd_text(text);
        *text = 0;
        sd_cursor(1, 1, sd_width - sd_col);/* to next line                   */
      }
      else if (*disabled) strcat(text, ", ");

      strcat(text, work);
      *disabled += 1;                      /* count disabled ports           */
    }
  }
  if (!(*disabled)) sprintf(text, "None of %d ports are disabled.", *count);

  if (strlen(text) < 60) sd_cursor(1, 1, 25);
  else						 sd_cursor(1, 1, 40 - strlen(text)/2);
  sd_text(text);
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Get Yes/No Answer
 *-------------------------------------------------------------------------*/
get_answer(rm)
register long rm;
{
  register unsigned char t;
  char buf[2];

  sd_prompt(&fld2, rm);
  while (1)
  {
    memset(buf, 0, 2);
    t = sd_input(&fld2, rm, 0, buf, 0);
    if (t == UP_CURSOR) break;
    if (t == EXIT) leave();
    *buf = tolower(*buf);

    if (*buf == 'y') return 1;
    if (*buf == 'n') return 0;

    eh_post(ERR_YN,0);
  }
  return 0;
}
/*--------------------------------------------------------------------------*
 *  Count Orders In System
 *--------------------------------------------------------------------------*/
count_orders()
{
  register long k;

  q_orders  = 0;
  uw_orders = 0;

  for (k = 0; k < PicklineMax; k++)
  {
    uw_orders += oc->oc_tab[k].oc_uw.oc_count;
    q_orders  += oc->oc_tab[k].oc_high.oc_count;
    q_orders  += oc->oc_tab[k].oc_med.oc_count;
    q_orders  += oc->oc_tab[k].oc_low.oc_count;
    q_orders  += oc->oc_tab[k].oc_hold.oc_count;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Catch Messages
 *-------------------------------------------------------------------------*/
catcher(who, type, buf, len)
register long who, type, len;
register TCapsMessageItem *buf;
{
  register long k;
  
  switch (type)
  {
    case ShutdownRequest:  leave();

    case ClientMessageEvent:
  
      buf->ErrorMessage.m_text[len - 1] = 0;
      eh_post(buf->ErrorMessage.m_error, buf->ErrorMessage.m_text);
      break;
      
    case MarkplaceEvent:
    
      eh_post(ERR_CONFIRM, "Markplace");
      break;
      
    case RestoreplaceEvent:
    
#ifdef DEBUG
  fprintf(stderr, "Restoreplace Event\n");
  fflush(stderr);
#endif
    
      rp_status = 1;									/* has completed OK				  */
      eh_post(ERR_CONFIRM, "Restoreplace");
      break;
      
    case PortDisableEvent:
    
      k = buf->PortMessage.m_port;
      po[k].po_disabled = 'y';
      eh_post(ERR_CONFIRM, "Port Disable");

#ifdef DEBUG
  fprintf(stderr, "Disable Event port=%d\n", k);
  fflush(stderr);
#endif
      break;
    
    case PortEnableEvent:

      k = buf->PortMessage.m_port;
      po[k].po_disabled = 'n';
      eh_post(ERR_CONFIRM, "Port Enable");
#ifdef DEBUG
  fprintf(stderr, "Enable Event port=%d\n", k);
  fflush(stderr);
#endif
      break;
  }
  return;
}
/*--------------------------------------------------------------------------*
 * transfer control back to calling program
 *--------------------------------------------------------------------------*/
open_all()
{
#ifdef DEBUG
  fprintf(stderr, "open_all()\n");
  fflush(stderr);
#endif
  
  sd_open(catcher);
  message_select(list, sizeof(list));
  ss_open();
  co_open();
  oc_open();
  getparms("LEGAL3");
}
close_all()
{
  ss_close_save();
  co_close();
  oc_close();
}
leave()
{
  sd_close();
  close_all();
  execlp("mmenu", "mmenu", 0);
  krash("syscomm", "load mmenu", 1);
}

/* end of syscomm.c */

