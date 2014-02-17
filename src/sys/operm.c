/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Operations command screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/11/93   |  tjt Added to mfc.
 *  05/16/94   |  tjt Added check_db to restoreplace.
 *  05/20.94   |  tjt Removed 'V', set refresh interval (only dotinit).
 *  06/10/94   |  tjt Add proceed to redisplay.
 *  06/23/94   |  tjt Removed pickline from redisplay.
 *  08/27/94   |  tjt Restored pickline to redisplay.
 *  09/28/94   |  tjt Fix UP_CURSOR aborts pickline actions.
 *  10/04/94   |  tjt Append null on error messages.
 *  11/07/94   |  tjt Print/purge error log requires super operator.
 *  01/23/95   |  tjt Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt Add pickline input by name.
 *  06/30/95   |  tjt Add check sku table changed on restoreplace.
 *  09/01/95   |  tjt Fix move sku table change test.
 *  07/05/96   |  tjt Fix add zone range to redisplay when no pickline or zone.
 *  07/31/96   |  tjt Fix clear redisplay flag on error message.
 *  04/18/97   |  tjt Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char operm_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "global_types.h"
#include "file_names.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "operm.t"
#include "eh_nos.h"
#include "message_types.h"
#include "caps_messages.h"
#include "getparms.h"
#include "language.h"

long TIMEOUT    = 60;
long MP_TIMEOUT = 60;
long RP_TIMEOUT = 600;
long time_flag = 0;
long redisplay_flag = 0;

extern timer();
extern leave();
extern catcher();
extern TPickline get_pickline();
extern TOrder get_order();

#define ABORT_PL 100

#define NUM_PROMPTS	7
#define LPICKLINE    8


short ONE       = 1;
short PLEN      = LPICKLINE;
short THREE     = 3;
short FOUR      = 4;
short LRANGE    = 7;
short LORDER    = 5;


struct fld_parms fld[] ={
  {20,48,20,1, &ONE,      "Enter Code",'a'},
  {21,48,20,1, &PLEN,     "Enter Pickline",'a'},
  {22,48,20,1, &LRANGE,   "Enter Zone Range",'a'},
  {22,48,20,1, &LORDER,   "Enter After Order Number",'n'},
  {21,48,20,1, &ONE,      "Proceed? (y/n)",'a'},
  {21,48,20,1, &ONE,      "Print Errors? (y/n)", 'a'},
  {22,48,20,1, &ONE,      "Purge Errors? (y/n)", 'a'}
};

unsigned char list[] =

{ShutdownRequest, ClientMessageEvent,
  InitializeEvent, ConfigureEvent,
  MarkplaceEvent, RestoreplaceEvent,
  ShortPrintDisableEvent, ShortPrintEnableEvent,
  RestockPrintDisableEvent, RestockPrintDisableEvent,
  PicklineUnlockEvent, PicklineLockEvent,
  PicklineDisableEvent, PicklineEnableEvent,
  ZoneRedisplayEvent, PicklineRedisplayEvent};

main()
{
  unsigned char t, incode[2];
  long pid, status;
  
  putenv("_=operm");                      /* program name                    */
  chdir(getenv("HOME"));

  open_all();
  
  fix(operm);                             /* setup screen image              */
  sd_screen_off(); 
  sd_clear_screen();
  sd_text(operm);                         /* show initial screen             */
  sd_screen_on();
  
  sd_cursor(0, 10, 12);
  if (sp->sp_sp_flag == 'y') sd_text("Disable Short Printing       ");
  else                       sd_text("Enable Short Printing        ");

  sd_cursor(0, 18, 12);
  if (sp->sp_running_status == 'y') sd_text("Mark Place   ");
  else                              sd_text("Restore Place");

  sd_cursor(0, 14, 48);
  if (sp->sp_order_selection == 'y') sd_text("V = Order Selection");

  if (sp->sp_zones > 99)     LRANGE = 3;
  else if (sp->sp_zones > 9) LRANGE = 2;
  else                       LRANGE = 1;
  LRANGE    = LRANGE + LRANGE + 1;
  LORDER    = rf->rf_on;                  /* length of order number          */
  
  sd_prompt(&fld[0], 0);                  /* code prompt                     */

  while(1)
  {
    memset(incode, 0, 2);

    sd_cursor(0, 21, 1);
    sd_clear_line();
    sd_cursor(0, 22, 1);
    sd_clear_line();
    sd_cursor(0, 23, 1);
    sd_clear_line();

    t = sd_input(&fld[0], 0, 0, incode, 0);
    if (t == UP_CURSOR) continue;         /* repeat prompt                   */
    if (t ==  EXIT)     leave();

    *incode = toupper(*incode);           /* to upper case                   */

    if(!LEGAL(*incode))                   /*display error                    */
    {
      eh_post(ERR_CODE, incode);
      continue;
    }
    if (!SUPER_OP)
    {
      if(*incode == 'B' || *incode == 'C' || *incode == 'E'
      || *incode == 'M' || *incode == 'X' || *incode == 'Y')
      {
        eh_post(LOCAL_MSG, "Action requires super operator");
        continue;
      }
    }
    if(*incode == 'G' && rf->rf_grp == 0)
    {
      eh_post(LOCAL_MSG, "Group code is not used");
      continue;
    }

/* avoid any order commands while order_input is running */

    if ( (*incode == 'G' || *incode == 'O'
    || *incode == 'Q' || *incode == 'D') && (sp->sp_oi_mode != 0x20))
    {
      eh_post(LOCAL_MSG, "Order input is running");
      continue;
    }
    switch(*incode)
    {
      case 'A':

        break;

      case 'B':

        load_prog("order_input_menu");
        break;

      case 'C':

        load_prog("transac_output");
        break;
          
      case 'D' :
      
        load_prog("order_stat");
        break;
          
      case 'E' :
        
        enable_short_printing();
        break;

      case 'F' :

        load_prog("display_shorts");
        break;

      case 'G' :

        load_prog("group_comms"); 
        break;

      case 'H' :

        if (!err_no_config()) load_prog("inhibit_enable_pm");
        break;

      case 'I' :
        
        if (!err_no_config()) pickline_action(PicklineDisableRequest);
        break;

      case 'J' :
        
        if (!err_no_config()) pickline_action(PicklineEnableRequest);
        break;

      case 'K' :

        if (sp->sp_to_flag == 'y' || sp->sp_to_flag == 'b') 
        {
          load_prog("transac_review");
        }
        else eh_post(LOCAL_MSG,"Cannot review transactions");
        break;

      case 'L' :

        if (!err_no_config()) lockout_orders();
        break;

      case 'M' :
        
        if (sp->sp_running_status == 'y') mark_place();
        else restore_place();
        break;
          
      case 'O' :
        
        load_prog("order_comms");
        break;
          
      case 'P' :
        
        if (!err_no_config()) pickline_change();
        break;

      case 'Q' :
        
        load_prog("order_display");
        break;
          
      case 'R' :

        if (!err_no_config()) redisplay();
        break;

      case 'S' :
        
#ifdef CANTON
        load_prog("sys_stat3");
#else
        if (sp->sp_use_con == 'n')              
        {
          if (sp->sp_remaining_picks == 'n') load_prog("sys_stat");
          else                               load_prog("sys_stat2");
        }
        else
        {
          if (sp->sp_remaining_picks == 'n') load_prog("sys_stat1");
          else                               load_prog("sys_stat4");
        }
        break;
#endif          

      case 'T' :
        
        if (sp->sp_pickline_view != 'y') eh_post(ERR_CODE, incode);
        else if (sp->sp_config_status != 'y') eh_post(ERR_NO_CONFIG, 0);
        else load_prog("view_menu");
        break;

      case 'U' :
        
        if (!err_no_config()) pickline_action(PicklineUnlockRequest);
        break;

      case 'V' :

        if (sp->sp_order_selection == 'n') eh_post(ERR_CODE, "v");
        else load_prog("select_comms"); 
        break;                         

      case 'W' :
   
        load_prog("message_view");
        break;
        
      case 'X' :
        
        if (!SUPER_OP)
        {
          eh_post(ERR_SUPER, 0);
          break;
        }
        print_errors();
        purge_errors();
        break;

      case 'Y' :
        
        if (!err_no_config()) 
        {
          if (are_you_sure(0)) load_prog("zero_counts");
        }
        break;

      case 'Z' :

        if (sp->sp_config_status == 'y') load_prog("zone_stat");
        eh_post(ERR_NO_CONFIG, 0);
        break;
        
      default:
        eh_post(ERR_CODE, incode);
        break;
      
    }
  }  
}
/*-------------------------------------------------------------------------*
 *  Load A Programs
 *-------------------------------------------------------------------------*/
load_prog(name)
char *name;
{
  char message[40];

  close_all();
  execlp(name, name, 0);                  /* attempt program load            */
  open_all();
  sprintf(message, "Program %s not found", name);
  eh_post(CRASH_MSG, message);
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Message processor
 *-------------------------------------------------------------------------*/
catcher(who, type, buf, len)
register long who, type;
register TCapsMessageItem *buf;
register long len;
{
  char work[32];                          /* notice work area                */

#ifdef DEBUG
  fprintf(stderr, "catcher: who=%d type=%d len=%d\n", who, type, len);
  if (len) Bdump(buf, len);
#endif

  switch(type)
  {
    case ShutdownRequest:
    case ShutdownEvent:
    
      close_all();
      alarm(0);
      exit(0);
      
    case ClientMessageEvent:

      alarm(0);
      time_flag = 0;
      redisplay_flag = 0;
      buf->ErrorMessage.m_text[len - 1] = 0;
      eh_post(buf->ErrorMessage.m_error, buf->ErrorMessage.m_text);
      break;
      
    case ShortPrintDisableEvent:
    
      eh_post(ERR_CONFIRM, "Short print disable");
      break;

    case ShortPrintEnableEvent:
    
      eh_post(ERR_CONFIRM, "Short print enable");
      break;

    case RestockPrintDisableEvent:
    
      eh_post(ERR_CONFIRM, "Restock print disabled");
      break;

    case RestockPrintEnableEvent:
    
      eh_post(ERR_CONFIRM, "Restock print enabled");
      break;
     
    case MarkplaceEvent:
    
      sd_cursor(0, 18, 12);
      sd_text("Restore Place");
      eh_post(ERR_MP_DONE, 0);
      break;
      
    case RestoreplaceEvent:
    
      alarm(0);
      time_flag = 0;

      sd_cursor(0, 18, 12);
      sd_text("Mark Place   ");
      eh_post(ERR_RP_DONE, 0);

      break;
      
    case ConfigureEvent:
    
      eh_post(ERR_CONFIRM, "Configuration");
      break;
      
    case InitializeEvent:
    
      eh_post(ERR_CONFIRM, "Initialization");
      break;

    case PicklineUnlockEvent:
    
      eh_post(ERR_CONFIRM, "Unlock Orders");
      break;

    case PicklineLockEvent:
    
      eh_post(ERR_CONFIRM, "Lock Orders");
      break;

    case PicklineDisableEvent:
    
      eh_post(ERR_CONFIRM, "Pickline Disabled");
      break;
      
    case PicklineEnableEvent:
    
      eh_post(ERR_CONFIRM, "Pickline Enabled");
      break;

     case ZoneRedisplayEvent:
     
       redisplay_flag = 0;
       sprintf(work, "Redisplay Zone %d", buf->ZoneMessage.m_zone);

       eh_post(ERR_CONFIRM, work);
       break;
     
     case PicklineRedisplayEvent:

       redisplay_flag = 0;
       sprintf(work, "Redisplay Pickline %d", buf->PicklineMessage.m_pickline);
       eh_post(ERR_CONFIRM, "Redisplay");
       break;

     default: break;
  }
  return 0;
}

/*-------------------------------------------------------------------------*
/* Return to main menu
 *-------------------------------------------------------------------------*/
leave()
{
  close_all();
  execlp("mmenu", "mmenu", 0);
  krash("operm", "load mmenu", 1);
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
open_all()
{
  sd_open(catcher);                       /* open screen driver              */
  message_select(list, sizeof(list));     /* various messages                */
  ss_open();
  co_open();

  getparms("LEGAL2");                     /* get operator environment        */

  RP_TIMEOUT = sp->sp_rp_timeout;
  MP_TIMEOUT = sp->sp_mp_timeout;
}
/*-------------------------------------------------------------------------*
 *  close all open files          
 *-------------------------------------------------------------------------*/
close_all()
{
  ss_close();
  co_close();
  sd_close();
  return;
}
/*-------------------------------------------------------------------------*
 *  check system is configured  
 *-------------------------------------------------------------------------*/
err_no_config()
{
  if (sp->sp_running_status == 'y') return 0;/* is configured and running    */

  eh_post(LOCAL_MSG, "Picking system is not running");
  return 1;                               /* not running                     */
}
/*-------------------------------------------------------------------------*
 *  Get Pickline Value
 *-------------------------------------------------------------------------*/
TPickline get_pickline()
{
  unsigned char t, buf[LPICKLINE + 1];
  TPickline pickline;
  
  if (IS_ONE_PICKLINE) return op_pl;
  if (!SUPER_OP)       return op_pl;
  
  memset(buf, 0, LPICKLINE + 1);

  sd_prompt(&fld[1], 0);
    
  while (1)
  {
    t = sd_input(&fld[1], 0, 0, buf, 0);
    if(t == EXIT)      leave();
    if(t == UP_CURSOR) return ABORT_PL;

    pickline = pl_lookup(buf, 0);

    if (!pickline) return 0;

    if (pickline < 0)
    {
      eh_post(ERR_PL, buf);
      continue;
    }
    if (pickline != op_pl)
    {
      op_pl = pickline;
      sprintf(buf, "%d", pickline);
      chng_pkln(buf);
    }
    break;
  }
  return op_pl;
}
/*-------------------------------------------------------------------------*
 *  Get An Order Number
 *-------------------------------------------------------------------------*/
TOrder get_order()
{
  unsigned char t, buf[8];
  TOrder order;
  
  sd_prompt(&fld[3], 0);
    
  while (1)
  {
    memset(buf, 0, 8);

    t = sd_input(&fld[3], 0, 0, buf, 0);
    if(t == EXIT)      leave();
    if(t == UP_CURSOR) return -1;

    order = atol(buf);

    if (order > OrderMax)
    {
      eh_post(LOCAL_MSG, "Order number of too large");
      continue;
    }
    break;
  }
  return order;
}
/*-------------------------------------------------------------------------*
 *  Get Zone Range
 *-------------------------------------------------------------------------*/
get_zone_range(low, high)
register TZone *low, *high;
{
  unsigned char t, buf[16], *p;

  sd_prompt(&fld[2], 0);

  while (1)
  {
    memset(buf, 0, 16);

    *low  = 0;
    *high = 0;

    t = sd_input(&fld[2], 0, 0, buf, 0);
    if (t == EXIT)      leave();
    if (t == UP_CURSOR) return -1;

    p = buf;

    while (*p >= '0' && *p <= '9')        /* low part of range               */
    {
      *low = *low * 10 + (*p++ - '0');
    }
    if (*p == '-')                        /* high part of range              */
    {
      p++;
      while (*p >= '0' && *p <= '9')
      {
        *high = *high * 10 + (*p++ - '0');
      }
    }
    else *high = *low;

    if (*p)
    {
      eh_post(LOCAL_MSG, "Only numbers and dash in range");
      continue;
    }
    if (*low < 1 && *high > 0)
    {
      eh_post(LOCAL_MSG, "Low zone cannot be zero");
      continue;
    }
    if (*low > *high)
    {
      eh_post(LOCAL_MSG, "Range must be 'low-high'");
      continue;
    }
    if (*low > coh->co_zone_cnt)
    {
      eh_post(LOCAL_MSG, "Low part of range too big");
      continue;
    }
    if (*high > coh->co_zone_cnt)
    {
      eh_post(LOCAL_MSG, "High part of range too big");
      continue;
    }
    break;
  }
  return 0;
}
 
/*-------------------------------------------------------------------------*
 *  Enable/Disable Short Printers
 *-------------------------------------------------------------------------*/
enable_short_printing()
{
  if (sp->sp_sp_flag == 'y')
  {
    sp->sp_sp_flag = 'n';
    message_put(0, ShortPrintDisableEvent, 0, 0);
    sd_cursor(0, 10, 12);
    sd_text("Enable Short Printing ");
    eh_post(ERR_CONFIRM, "Disable Short Print");
  }
  else
  {
    sp->sp_sp_flag = 'y';
    message_put(0, ShortPrintEnableEvent, 0, 0);
    sd_cursor(0, 10, 12);
    sd_text("Disable Short Printing");
    eh_post(ERR_CONFIRM, "Enable Short Print ");
  }
  return;
}
/*-------------------------------------------------------------------------*
 * Inhibit/Enable/Unlock action by pickline
 *-------------------------------------------------------------------------*/
pickline_action(what)
long what;
{
  TPickline pickline;

  while (1)
  {
    pickline = get_pickline();
    if (pickline == ABORT_PL) 
    {
      eh_post(LOCAL_MSG, "*** Aborted");
      return 0;
    }
    break;
  }
  sd_wait();
  if (message_wait(0, what, &pickline, sizeof(TPickline), -1 , TIMEOUT) < 0)
  {
    eh_post(LOCAL_MSG, "*** CAPS Is Not Responding");
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Markplace request
 *-------------------------------------------------------------------------*/
mark_place()
{
  if (sp->sp_in_process_status != 'x')
  {
    eh_post(LOCAL_MSG, "System Is Busy");
    return;
  }
  if (sp->sp_running_status != 'y')
  {
    eh_post(LOCAL_MSG, "System is not running");
    return;
  }
  if (!are_you_sure(0)) return;

  sd_wait();
  if (message_wait(0, MarkplaceRequest, 0, 0, -1, MP_TIMEOUT) < 0)
  {
    eh_post(LOCAL_MSG, "*** CAPS Is Not Responding");
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Restoreplace request
 *-------------------------------------------------------------------------*/
restore_place()
{
  long pid, status;
  register long k;
  
  if (sp->sp_running_status == 'y')
  {
    eh_post(LOCAL_MSG, "System is already running");
    return;
  }
  if (sp->sp_in_process_status != 'x')
  {
    eh_post(LOCAL_MSG, "System Is Busy");
    return;
  }
  for (k = 0; k < coh->co_ports; k++)
  {
    if (po[k].po_disabled == 'y') continue;  /* diags are ok if disabled     */
    if (po[k].po_status == 'd')
    {
      eh_post(LOCAL_MSG, "Diagnostics are running");
      return;
    }
  }
  if (!are_you_sure(0)) return;

  sd_wait();
  time_flag = 1;

  system("etc/check_db 1>dat/log/restore.log 2>&1");

  message_put(0, RestoreplaceRequest, 0, 0);

  signal(SIGALRM, timer);
  alarm(RP_TIMEOUT);

  while (time_flag)  {pause();}

  if (coh->co_st_changed == 1)
  {
    sd_cursor(0, 22, 25);
    sd_text("* * *  Updating Picks   * * *");

    if (fork() == 0)
    {
      ss_close();
      co_close();
      execlp("reconfigure_orders", "reconfigure_orders", 0);
      krash("catcher", "load reconfigure_orders", 1);
    }
    pid = wait(&status);
    if (pid < 0) krash("catcher", "reconfigure_orders failed", 1);
    else if (status) eh_post(LOCAL_MSG, "Orphan Picks - See Printer", 1);
  }
  return 0;
}
timer()
{
  eh_post(LOCAL_MSG, "*** CAPS Is Not Responding");
  time_flag = 0;
}
/*-------------------------------------------------------------------------*
 *  Lockout orders by pickline
 *-------------------------------------------------------------------------*/
lockout_orders()
{
  TOrderMessage loo;
  
  while  (1)
  {
    loo.m_pickline = get_pickline();
    if (loo.m_pickline == ABORT_PL) 
    {
      eh_post(LOCAL_MSG, "*** Aborted");
      return;
    }
    if (loo.m_pickline)
    {
      loo.m_order = get_order();
      if (loo.m_order < 0) continue;
    }
    else loo.m_order = 0;
    
    sd_wait();
    if (message_wait(0, PicklineLockRequest, &loo, 
      sizeof(TOrderMessage), -1, TIMEOUT) < 0)
    {
      eh_post(LOCAL_MSG, "*** CAPS Is Not Responding");
    }
    return;
  }
}
/*-------------------------------------------------------------------------*
 *  Change default pickline
 *-------------------------------------------------------------------------*/

pickline_change()
{
  TPickline pickline;

  pickline = get_pickline();
  if (pickline == ABORT_PL) 
  {
    eh_post(LOCAL_MSG, "*** Aborted");
    return;
  }
  if (!pickline)
  {
    eh_post(ERR_PL, "0");
    return;
  }
  eh_post(LOCAL_MSG, "Pickline changed");
  return;
}

/*-------------------------------------------------------------------------*
 *  Redisplay pickline and/or zones in pickline
 *-------------------------------------------------------------------------*/
redisplay()
{
  register struct zone_item *z;
  TPickline pickline;
  TZone low_zone, high_zone;

  low_zone = high_zone = 0;

  if (IS_ONE_PICKLINE) pickline = op_pl;
  else if (SUPER_OP)   
  {
    pickline = get_pickline();
    if (pickline == ABORT_PL)
    {
      eh_post(LOCAL_MSG, "*** Aborted");
      return;
    }
  }
  else pickline = op_pl;

  if (get_zone_range(&low_zone, &high_zone) < 0) return 0;

  if (!are_you_sure(2)) return 0; 
  sd_wait();

  if (pickline && !low_zone)
  {
    redisplay_flag = 1;
    message_put(0, PicklineRedisplayRequest, &pickline, sizeof(TPickline));
    while (redisplay_flag) pause();
    return;
  }
  if (!low_zone && !pickline)              /* F070596 */
  {
    low_zone  = 1;
    high_zone = coh->co_zone_cnt;
  }
  for (z = &zone[low_zone - 1]; low_zone <= high_zone; low_zone++, z++)
  {
    if (pickline && z->zt_pl != pickline) continue;
    if (!z->zt_pl) continue;               /* F070596 */
    
    redisplay_flag = 1;
    message_put(0, ZoneRedisplayRequest, &low_zone, sizeof(TZone));
    while (redisplay_flag) pause();
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Are You Sure Message
 *-------------------------------------------------------------------------*/
are_you_sure(rm)
register long rm;
{
  unsigned char t, buf[2], yn[2];
   
  memset(buf, 0, 2);
  memset(yn, 0, 2);
  
  sd_prompt(&fld[4], rm);
  while(1)
  {
    t = sd_input(&fld[4], rm, 0, yn, 0);
    if (t == UP_CURSOR) return 0;
    if (t == EXIT)      leave();
      
    *buf = code_to_caps(*yn);             /* F041897 */
    if (*buf == 'y') return 1;
    if (*buf == 'n') return 0;
    eh_post(ERR_YN, 0);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Set refresh interval
 *-------------------------------------------------------------------------*/
set_refresh_interval()
{
  static char work[32];
  unsigned char t, buf[8];
   
  memset(buf, 0, 8);
    
  sd_prompt(&fld[5], 0);
  t = sd_input(&fld[5], 0, 0, buf, 0);
  if (t == UP_CURSOR) return;
  if (t == EXIT)      leave();
      
  if (atol(buf) < 1) strcpy(buf, "1");

  sprintf(work, "REFRESH=%s", buf);
  putenv(work);
  
  return;
}
/*-------------------------------------------------------------------------*
 *  Print Error Log
 *-------------------------------------------------------------------------*/

print_errors()
{
  char temp1[16], temp2[16];
  long pid, stat;
  char command[128];
  unsigned char t, buf[2], yn[2];
  
  memset(buf, 0, 2);
  memset(yn, 0, 2);

  sd_prompt(&fld[5], 0);
  while(1)
  {
    t = sd_input(&fld[5], 0, 0, yn, 0);
    if (t == UP_CURSOR) return 0;
    if (t == EXIT)      leave();
      
    *buf = code_to_caps(*yn);

    if (*buf == 'y') break;
    if (*buf == 'n') return 0;
    eh_post(ERR_YN, 0);
  }
  tmp_name(temp1);
  tmp_name(temp2);
  
  sprintf(command, "cp %s %s", eh_err_name, temp1);
  system(command);
  
  if (fork() == 0)
  {
    execlp("prft", "prft", temp1, temp2, "sys/report/report_report.h",
     "    Error Listing", 0);
    krash("print_errors", "load prft", 1);
  }
  pid = wait(&stat);
  
  eh_post(ERR_CONFIRM, "Print errors ");
  return;
}

/*-------------------------------------------------------------------------*
 *  Purge error file
 *-------------------------------------------------------------------------*/
 
purge_errors()
{
  FILE *fd;
  unsigned char t, buf[2], yn[2];
  
  memset(buf, 0, 2);
  memset(yn, 0, 2);

  sd_prompt(&fld[6], 0);
  while(1)
  {
    t = sd_input(&fld[6], 0, 0, yn, 0);
    if (t == UP_CURSOR) return 0;
    if (t == EXIT)      leave();
      
    *buf = code_to_caps(*yn);
    
    if (*buf == 'y') break;
    if (*buf == 'n') return 0;
    eh_post(ERR_YN, 0);
  }
  fd = fopen(eh_err_name, "w");
  if (fd) fclose(fd);

  if (!are_you_sure(2)) return 0;

  eh_post(ERR_CONFIRM, "Purge Errors ");
  return;
}

/* end of operm.c */
