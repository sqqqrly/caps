/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Picker Accountability menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/11/93   |  tjt  Added to mfc
 *  04/04/97   |  tjt  Added manual login and logout.
 *  06/05/01   |  aha  Added manual zone assignment without logging in and
 *             |       menu item E (= Picker Quality Control).
 *  07/25/01   |  aha  Removed manual zone assignment without logging in.
 *  11/01/01   |  aha  Added menu item "R = Report Zone Login Status"
 *-------------------------------------------------------------------------*/
static char picker_acountability_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "picker_acctability.t"
#include "message_types.h"
#include "caps_messages.h"
#include "global_types.h"

#include "Bard.h"
#include "bard/picker.h"
#define  p_cur_pl p_underway_orders


extern leave();

static short ONE   = 1;
static short NINE  = 9;
static short THREE = 3;

struct fld_parms fld1 = {17,44,25,1,&ONE,"Enter Code",'a'};
struct fld_parms fld2 = {18,44,25,3,&NINE, "Enter ID", 'n'};
struct fld_parms fld3 = {19,44,25,3,&THREE, "Enter Zone", 'n'};
char buf[2];

main()
{
  short i;
  short rm;
  unsigned char t;

  open_all();

  fix(picker_acctability);
  sd_screen_off();
  sd_clear_screen();
  sd_text(picker_acctability);
  sd_screen_on();
  
  while(1)
  {
    for (i=0;i<2;i++) buf[i] = 0;
    t = sd_input(&fld1,sd_prompt(&fld1,0),&rm,buf,0);

    *buf = tolower(*buf);
    if(t == EXIT) leave();

    switch(buf[0])
    {
      case 'd' :
        if ( sp->sp_running_status == 'y')
        {
          eh_post(ERR_IS_CONFIG, "");
          break;
        }
      /* otherwise, fall through to next case */
      case 'a' :
      case 'c' :  
       
        loadprog("picker_information");
        break;

      case 'e' :

        loadprog("picker_error_maint");
        break;
        
      case 'l' :
      case 'p' : 
      
        loadprog("picker_prod_rpts");
        break;
        
      case 'z' :

        loadprog("picker_zero_counts");
        break;
        
      case 'i': 
      
        process_login();
        break;       
      
      case 'o':
      
        process_logout();
        break;

      case 'r':

        loadprog("picker_login_rpt");
        break;
      
      default :
        
        eh_post(ERR_CODE, buf);
    }
  }
}
/*--------------------------------------------------------------------------*
 *  Process Log In
 *--------------------------------------------------------------------------*/
process_login()
{
  picker_item pkr;
  char id[10], zn[10], text[64];
  struct zone_item *z;
  long picker;
  short int n = 0;
  unsigned char t;
  TZone zone_num = 0;
  TZoneDisplayMessage p;
  
  if (sp->sp_config_status != 'y')
  {
    eh_post(ERR_NO_CONFIG);
    return 0;
  }
  memset(id, 0x0, 10);
  memset(zn, 0x0, 10);
  memset(&pkr, 0x0, sizeof(picker_item));
  memset(&p, 0x0, sizeof(TZoneDisplayMessage));

  sd_prompt(&fld2, 0);
  sd_prompt(&fld3, 0);

  while (1)
  {
    t = sd_input(&fld2, 0, 0, id, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) break;
    
    picker = atol(id);
    pkr.p_picker_id = picker;
    
    begin_work();
    
    if (picker_read(&pkr, NOLOCK))
    {
      sprintf(text, "Invalid Badge ID %s", id);
      eh_post(LOCAL_MSG, text);
      commit_work();
      continue;
    }
    commit_work();

    while (1)
    {
      t = sd_input(&fld3, 0, 0, zn, 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;

      n = atoi(zn);

      if (n <= 0 || n > coh->co_zone_cnt)
      {
        sprintf(text, "INVALID ZONE %s", zn);
        eh_post(LOCAL_MSG, text);
        continue;
      }

      z = &zone[n - 1];
      zone_num = z->zt_zone;
      p.m_zone = z->zt_zone;

      if (z->zt_picker)
      {
         sprintf(text, "%s In Zone %s", z->zt_picker_name, zn);
         eh_post(LOCAL_MSG, text);
         continue;
      }

     if (t == RETURN) break;
    }
   
    if (pkr.p_status == 1)
       {
          sprintf(text, "PICKER %d ALREADY LOGGED IN", pkr.p_picker_id);
          eh_post(LOCAL_MSG, text);
          continue;
       }

    if (t == RETURN)
    {
      z->zt_picker = picker;
      memcpy(z->zt_picker_name, pkr.p_last_name, 12);
      sprintf(text, "%9.9s LOGGED", pkr.p_last_name);
      memcpy(p.m_text, text, 16);
  
      pkr.p_picker_id  = picker;
      pkr.p_start_time = time(0);
      pkr.p_cur_pl     = z->zt_pl;
      pkr.p_zone       = n;
      pkr.p_status     = 1;

      begin_work();
      picker_update(&pkr);
      commit_work();

      message_put(0, ZoneOnlineRequest, &zone_num, sizeof(TZone));
      message_put(0, ZoneDisplayRequest, &p, sizeof(TZoneDisplayMessage));
      break;
    }
  }
  sd_cursor(0, fld2.irow, 1); sd_clear_line();
  sd_cursor(0, fld3.irow, 1); sd_clear_line();
  
  sprintf(text, "%s LOGGED", pkr.p_last_name);  
  eh_post(ERR_CONFIRM, text);
  
  return 0;
}
/*--------------------------------------------------------------------------*
 *  Process Log Out
 *--------------------------------------------------------------------------*/
process_logout()
{
  picker_item pkr;
  unsigned long int now = 0L;
  time_t * nowptr = 0;
  char id[10],
       text[64],
       datemsg[20];
  struct zone_item *z;
  long picker = 0L,
       elapsed = 0L;
  unsigned char t;
  TZone zone_num = 0;
  TZoneDisplayMessage p;
 
  if (sp->sp_config_status != 'y')
  {
    eh_post(ERR_NO_CONFIG);
    return 0;
  }

  memset(id, 0x0, 10);
  memset(&pkr, 0x0, sizeof(picker_item));
  memset(&datemsg, 0x0, 20); 
  memset(&p, 0x0, sizeof(TZoneDisplayMessage));

  sd_prompt(&fld2, 0);

  while (1)
  {
    t = sd_input(&fld2, 0, 0, id, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) break;
    
    picker = atol(id);
    pkr.p_picker_id = picker;
    
    begin_work();
    
    if (picker_read(&pkr, LOCK))
    {
      sprintf(text, "Invalid Badge ID %s", id);
      eh_post(LOCAL_MSG, text);
      commit_work();
      continue;
    }
    else
    {
      commit_work();
      begin_work();
      picker_query(&pkr, picker);
      commit_work();
    }

    if (pkr.p_status == 0)
       {
          sprintf(text, "PICKER %d ALREADY LOGGED OUT", pkr.p_picker_id);
          eh_post(LOCAL_MSG, text);
          continue;
       }

    z = &zone[pkr.p_zone - 1];
    zone_num = z->zt_zone;
    p.m_zone = z->zt_zone;

    if (z->zt_picker != pkr.p_picker_id)
       {
          sprintf(text, "%s IS IN ZONE %d", z->zt_picker_name, z->zt_zone);
          eh_post(LOCAL_MSG, text);
          continue;
       }

    if (t == RETURN)
       {
         z->zt_picker = 0;
         memset(z->zt_picker_name, 0, 12);
         sprintf(text, "%9.9s LOGOUT", pkr.p_last_name);
         memcpy(p.m_text, text, 16);

         elapsed = time(0) - pkr.p_start_time;
    
         pkr.p_cur_time  += elapsed;
         pkr.p_cum_time  += elapsed;
         pkr.p_start_time = 0;
         pkr.p_zone       = 0;
         pkr.p_status     = 0;

         begin_work();
         picker_update(&pkr);
         commit_work();

         now = time(0);
         nowptr = (time_t *)&now;
         strftime(datemsg, 20, "%Y-%m-%d %T", localtime(nowptr));

         message_put(0, ZoneOfflineRequest, &zone_num, sizeof(TZone));
         message_put(0, ZoneDisplayRequest, &p, sizeof(TZoneDisplayMessage));

         break;
       }
  }

  sd_cursor(0, fld2.irow, 1); sd_clear_line();

  sprintf(text, "%s LOGGED OUT", pkr.p_last_name);   
  eh_post(ERR_CONFIRM, text);
  
  return 0;
}
/*
 *  load a programs
 */
loadprog(p)
{
  char text[64];
  
  close_all();
  execlp(p, p, buf, 0);
  open_all();
  sprintf(text, "Program %s Not Found", p);
  eh_post(CRASH_MSG, text);
  return;
}
/*
 *transfer control back to calling program
 */
leave()
{
  close_all();
  execlp("pnmm","pnmm", 0);
  krash("leave", "pnmm load", 1);
}

open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  database_open();
  picker_open(AUTOLOCK);
  picker_setkey(1);
  return 0;
}
close_all()
{
  picker_close();
  database_close();
  co_close();
  ss_close();
  sd_close();
  return 0;
}
/* end of picker_acctability.c */
