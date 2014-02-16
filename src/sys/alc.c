#define NOBLINK
#define REFRESH
#define DEBUG
#define DETAIL
#define TANDY 
/* #define TCBL_ALC */ 
/* #define RESPONSE */
/* #define RED       */
/* #define YELLOW      */
#define MIOM
/* #define BOXFULL */
#define STAPLES
 
/*-------------------------------------------------------------------------*
 *  Custom Code:    RED      - Red color for picks over sp_blink_over.
 *                  NOBLINK  - Blink for picks over sp_blink_over.
 *                  RESPONSE - Zone event response time. (requires NOBLINK).
 *                  REFRESH  - Redisplay on button push full function zc.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Total function driver.
 *
 *  Execution:      alc [text_name]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/11/93   |  tjt  Original implementation.
 *  05/12/94   |  tjt  Add IsOffline flag.
 *  05/13/94   |  tjt  Fix order numbers 1..6 right justified in zc.
 *  06/14/94   |  tjt  Add bays with no modules.
 *  06/24/94   |  tjt  Fix Controller Displays
 *  12/31/94   |  tjt  Add open ports only once.
 *  02/24/95   |  tjt  Add port disable.
 *  02/28/95   |  tjt  Change text load.
 *  03/16/95   |  tjt  Remove cluster picking.
 *  03/17/95   |  tjt  Add ZC2, PM2, and PM4.
 *  03/23/95   |  tjt  Add enable/disable diagnostics.
 *  05/18/95   |  tjt  Add autocasing in pick text.
 *  05/24/95   |  tjt  Add multiple modules in pick text.
 *  05/25/95   |  tjt  Add blink option.
 *  06/21/95   |  tjt  Fix blink option.
 *  06/21/95   |  tjt  Fix multiple modules.
 *  06/28/95   |  tjt  Add disable port.
 *  07/18/95   |  tjt  Add ignore zero pick quantity.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  12/10/95   |  tjt  Response time debug.
 *  01/19/96   |  tjt  Use oi_con for customer number.
 *  04/16/96   |  tjt  Revise for hw & mh changes
 *  05/26/96   |  tjt  Add box full module.
 *  06/28/96   |  tjt  Fix bug in zone_clearp of module flags.
 *  06/28/96   |  tjt  Fix done message in zone controller.
 *  01/15/96   |  sg   Add full function.
 *  01/07/97   |  tjt  Add PM6.
 *  05/26/98   |  tjt  Add IO module.
 *  06/05/98   |  tjt  Add oi_box field and zone display.
 *  06/06/98   |  tjt  Add ZC displays for GTE.
 *  06/07/98   |  tjt  Add open boxes for order.
 *  05/20/01   |  aha  Modified for Eckerd's picker scan login with sections. 
 *  09/18/01   |  aha  Modified for Eckerd's tote integrity.
 *  03/29/02   |  aha  Modified for Eckerd's tote integrity addendum changes.
 *  02/26/03   |  aha  Added fix for MBLs in zone_clear().
 *  03/17/03   |  aha  Added fixes for Eckerd's "Speed Issue" problem.
 *-------------------------------------------------------------------------*/
static char alc_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "alc_text.h"
#include "global_types.h"
#include "message_types.h"
#include "caps_messages.h"
#include "eh_nos.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "zone_status.h"
#include "box.h"

#include "Bard.h"
#include "boxes.h"
#include "bard/picker.h"
#include "sections.h"
#include "date_mani_c.h"


#define MAX_MSG_SIZE   253                /* maximum message size for       */
                                          /* ac_write()                     */
#define MAX_CMD_LGTH   240                /* maximum length of command for  */
                                          /* message sent to ac_write()     */
void leave(flag);

long microclock();

long short_count = 0;                     /* number of shorts active         */
long SHORT_INTERVAL = 3;                  /* 300 ms per short count          */
long short_catcher();                     /* short processor                 */

long STIME = 3;                           /* simulated zone event time       */
long sim_flag = 0;                        /* allow sim only at good times    */
long superpicker = 0;                     /* superpicker running flag        */
extern simulator();

long port[PortMax];                       /* open area controller ports      */
long ports_open = 0;                      /* running or not                  */

short first_ac_on_port[PortMax+1];        /* offset to first hw_on_ac table  */
short *first_hw_on_ac;                    /* offset in hw table              */

unsigned char list[] = {ShutdownRequest, PortMarkplaceRequest,
  InitializeEvent, MarkplaceEvent, RestoreplaceEvent, ConfigureEvent,
  PicklineDisableRequest, PicklineEnableRequest, ACSwitchPacketEvent,
  ZoneStartRequest, ModulePickRequest, ZoneClearRequest, ZoneStopRequest,
  ZoneOfflineRequest, ZoneOnlineRequest, ZoneDisplayRequest,
  ACErrorPacketEvent, StartSuperpicker, StopSuperpicker,
  PortDisableRequest, PortDisableEvent, PortEnableRequest, PortEnableEvent,
#ifdef TCBL_ALC		
  ACIOPacketEvent,TcblWriteEvent};
#else
  ACIOPacketEvent};
#endif
  
long who, type, len;                      /* global message values           */
TCapsMessageItem buf;                     /* global message buffer           */

/*-------------------------------------------------------------------------*
 *   Default Total Function Messages 
 *-------------------------------------------------------------------------*/
alc_data_item am = {

  {"  AHEAD         "},                   /* Start Ahead Zone                */
  {" COMPLETE       "},                   /* Start Complete Zone             */
  {"  LOCKED        "},                   /* Start Locked Zone               */
  {"EXIT  %0.4g%1.6o"},                   /* Start Early Exit - No Picks     */
  {"PICK  %0.4g%1.6o"},                   /* Start Early Exit - Picks        */
  {"LATE ENTRY%1.6o"},                    /* Start Late Entry                */
  {"PASS  %0.4g%1.6o"},                   /* Start Underway - No Picks       */
  {"PICK  %0.4g%1.6o"},                   /* Start Underway - Picks          */
#ifdef TANDY
  {"SCAN LABEL      "},                   /* Start Waiting                   */
#else
  {"PRESS NEXT      "},                   /* Start Waiting                   */
#endif
  {"PASS  %0.4g%1.6o"},                   /* Finish - Pass Tote              */
  {"EXIT  %0.4g%1.6o"},                   /* Finish - Exit Tote              */
  {"HOLD ORDER%1.6o"},                    /* Finish - Hold Tote              */
  {"LOGIN %0.4z      "},                  /* Picker Login                    */

};

#ifdef DEBUG
FILE *DF;
#define DF_SIZE 4000000
#endif

typedef struct
{
  long (*down_func)();                    /* switch down function            */
  long down_next;                         /* switch down next state          */
  long (*up_func)();                      /* switch up function              */
  long up_next;                           /* switch up next state            */

} state_item;

state_item state_table[] = {0,0,0,0};

#ifdef RESPONSE
long rstart[200] = {0};
#endif

#define TOTE_BARCODE_LEN      8
#define BADGE_BARCODE_LEN     9
#define ZONE_BARCODE_LEN      2
#define NUMBER_OF_SCANNERS   BayMax
#define MAX_ZONES            99
picker_scan_item pscan[NUMBER_OF_SCANNERS];

#define p_cur_pl  p_underway_orders

typedef struct
{
   long           picker_id;
   short          zone;
} picker_status;

/*
picker_status pstatus[NUMBER_OF_SCANNERS];
*/


main(argc, argv)
long argc;
char **argv;
{
  register long k;
  
  putenv("_=alc");                        /* store program name              */
  chdir(getenv("HOME"));                  /* insure in HOME directory        */
      
  setpgrp();

  signal_catcher(0);                      /* catch various signals           */
  
  message_open();                         /* link to kernel                  */
  message_select(list, sizeof(list));     /* messages desired                */
   
#ifdef DEBUG
  DF = fopen("debug/alc_bug", "w");

  fprintf(DF, "alc started:  msgtask=%d\n", msgtask);
  fflush(DF);
#endif
  
  ss_open();
  co_open();
  oc_open();

  database_open();
  picker_open(AUTOLOCK);
  picker_setkey(1);
  //sections_open(AUTOLOCK);
  //sec_prd_open(AUTOLOCK);
  if (sp->sp_box_feature == 's')                /* F060798 */
  {
    boxes_open(AUTOLOCK);
  }
  memset(&pscan, 0x0, sizeof(picker_scan_item));

/*
  memset(&pstatus, 0x0, sizeof(picker_scan_item));
*/

#ifdef TANDY
	od_open();
#endif
  
  if (argc > 1) load_text(argv[1]);
  
  if (sp->sp_short_ticks > 0) SHORT_INTERVAL = sp->sp_short_ticks;

  if (sp->sp_total_function == 'n') leave(0);

  while (1)
  {
#ifdef DEBUG
    fflush(DF);
    if (ftell(DF) > DF_SIZE)
    {
      fclose(DF);
      system("mv debug/alc_bug debug/alc_save 1>/dev/null 2>&1");
      DF = fopen("debug/alc_bug", "w");
    }
#endif

    sim_flag = 0;
    message_get(&who, &type, &buf, &len); /* get a message                   */
    sim_flag = 1;
    
#ifdef DEBUG
    fprintf(DF, "packet:  Who=%d Type=%d %s Len=%d\n",
    who, type & 0x7f, type & 0x80 ? "Event" : "Request", len);
    if (len > 0) Bdumpf(&buf, len, DF);
    fflush(DF);
#endif
    
    switch(type)
    {
      case ShutdownEvent:  leave(0);      /* quit processing                 */

      case InitializeEvent:               /* open all ports                  */

	if (!ports_open) open_ports();    /* open all ports   F123194        */
	break;
	
      case ConfigureEvent:
      
	config_ports();                   /* relate ports to bays            */
	broadcast_message("CONFIGURED      ", "__C__");
#ifndef TANDY
	pickline_disable(0);
#endif
/*        disable_diagnostics();   */
	enable_diagnostics();
	break;

      case RestoreplaceEvent:
      
	config_ports();                   /* relate ports to bays            */
	pickline_disable(0);
/*        disable_diagnostics();   */
	enable_diagnostics();
	if (sp->sp_pickline_view == 'y') pickline_restoreplace();
	break;
	 
      case PortEnableRequest:
      
	k = buf.PortMessage.m_port;
	message_pass(who, po[k].po_id_in, type, &buf, len);
	break;
	
      case PortEnableEvent:
      
	k = buf.PortMessage.m_port;
	po[k].po_disabled = 'n';
	message_put(coh->co_id, PortRedisplayRequest, &buf, len);
	break;
	
      case ShutdownRequest:
      case PortMarkplaceRequest:
      
#ifdef SIMULATOR
	superpicker = 0;
	alarm(0);
#endif
	pickline_disable(0);
	short_count = 0;                  /* stop shorts and blink           */
	return_picks();                   /* send all completed picks        */
	break;
      
      case MarkplaceEvent:
      
	broadcast_message("MARKPLACED      ", "__P__");
	enable_diagnostics();
	break;
      
      case PortDisableRequest:
      
	k = buf.PortMessage.m_port;
	port_disable(k + 1);
	message_put(po[k].po_id_in, type, &buf, len);
	break;
	
      case PortDisableEvent:
	
	k = buf.PortMessage.m_port;
	port_message(k, "MARKPLACED      ", "__P__");
	po[k].po_disabled = 'y';
	break;
      
      case PicklineDisableRequest:
      
	pickline_disable(buf.PicklineMessage.m_pickline);
	break;
     
      case PicklineEnableRequest:
      
	pickline_enable(buf.PicklineMessage.m_pickline);
	break;
      
      case ACSwitchPacketEvent:
    
	if (sp->sp_running_status == 'y') process_packet(&buf);
	break;
	
      case ACIOPacketEvent:
      
	if (sp->sp_running_status == 'y') process_io_packet(&buf);
	break;
    
      case ACErrorPacketEvent:
	
	process_error_packet();
	break;

      case ModulePickRequest:
      
	pick_request(&buf);
	break;
      
      case ZoneStartRequest:
      
	start_request(buf.ZoneMessage.m_zone);
	message_put(0, ZoneStartEvent, &buf, sizeof(TZone));
	break;
	
      case ZoneClearRequest:
      
	zone_clear(buf.ZoneMessage.m_zone, 1);
	message_put(coh->co_id, ZoneClearEvent, &buf, sizeof(TZone));
	break;
    
      case ZoneStopRequest:
      
	zone_clear(buf.ZoneMessage.m_zone, 1);
	message_put(coh->co_id, ZoneStopEvent, &buf, sizeof(TZone));
	break;

      case ZoneDisplayRequest:
      
	zone_message_request(&buf);
	break;

      case ZoneOfflineRequest:
      
	zone_offline(buf.ZoneMessage.m_zone);
	break;
	
      case ZoneOnlineRequest:
      
	zone_online(buf.ZoneMessage.m_zone);
	break;

#ifdef TCBL_ALC
      case TcblWriteEvent:
	tcbl_write_event(&buf);
	break;
#endif

#ifdef SIMULATOR
      case StartSuperpicker:
      
	if (sp->sp_total_function == 's' && !superpicker)
	{
	  superpicker = 1;
	  signal(SIGALRM, simulator);
	  alarm(STIME);
	}
	break;
	
      case StopSuperpicker:
      
	superpicker = 0;
	alarm(0);
	break;
#endif

	default: break;
      }                                   /* end of swtich                   */
#ifdef DEBUG
  fprintf(DF, "packet done - waiting\n");
  fflush(DF);
#endif
  
  }                                       /* end of while loop               */
}
/*-------------------------------------------------------------------------*
 *  Zone Clear - Flush And Blank Zone
 *-------------------------------------------------------------------------*/
zone_clear(zn, flag)
register long zn;                         /* zone number                     */
register long flag;                       /* 1 = clear, 2 = return picks     */
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct bay_item  *bn;
  register struct zone_item *z;
  register long j, k, m, jn;
  register long count = 0L;
  TPickMessage x;
  TPickBoxMessage y;
  unsigned char text[MAX_MSG_SIZE];
  long ntime = 0L;
  register long hw_cnt    = 0L,
                first_mod = 0L,
                m_first   = 0L,
                cmd_lgth  = 0L,
                m_mbl     = 0L,
                zc_mod    = 0L,
                bf_mod    = 0L,
                bl_mod    = 0L;
#ifdef TANDY
  register struct hw_item   *hz;
  TZone redisplay_zone;
  long clear_flag = 0;
#endif
  
  z = &zone[zn - 1];                      /* point to zone record            */

#ifdef DEBUG
  fprintf(DF, "zone_clear(): Zone=%d z->zt_last_light=%d flag=%d\n", zn,flag,
			z->zt_last_light);
  fflush(DF);
  ntime = time(0);
  fprintf(DF, "              Time=[%24.24s]\n", ctime(&ntime));
  fflush(DF);
#endif
	

  if (flag & 2)                           /* setup return of picks           */
  {
    x.m_pickline = z->zt_pl;
    x.m_order    = z->zt_on;
  }
  if (flag & 1)
  {
    z->zt_lines  = 0;
    z->zt_flags |= ZoneInactive;
  }
  k = z->zt_first_bay;
  
  while (k > 0)                           /* over all bays in the zone       */
  {
    m_mbl = -1L;
    b = &bay[k - 1];
    k = b->bay_next;
    j = b->bay_port - 1;

    if (flag & 1) b->bay_picks = 0;

    if (b->bay_zc && (flag & 1))
    {
      h = &hw[b->bay_zc - 1];
      h->hw_state = 0;

      zc_mod = h->hw_mod_address;

      if (sp->sp_pickline_view == 'y')
      {
	memset(zcv[h->hw_mod - 1].hw_display, 0x20, 16);
      }
    }
    if (b->bay_bl && (flag & 1))          /* clear bay lamp                  */
    {
      h = &hw[b->bay_bl - 1];
      h->hw_state = 0;
      bl_mod = h->hw_mod_address;

      if (sp->sp_pickline_view == 'y')
      {
	blv[h->hw_mod - 1].hw_display[0] = '0';
      }
    }
    count = 0L;
    while (b->bay_mbl && (flag & 1))      /* clear master bay lamp           */
    {
      bn = &bay[b->bay_mbl - 1];
      jn = bn->bay_port - 1;
      bn->bay_picks = 0;
     
      h = &hw[bn->bay_bl - 1];
      h->hw_state = 0;

      m_mbl = bn->bay_bl - 1;

      sprintf(text, "%04d10%03dB", h->hw_controller, h->hw_mod_address);
      Ac_Write(jn, text, 10, 0);

      if (sp->sp_pickline_view == 'y')
      {
	blv[h->hw_mod - 1].hw_display[0] = '0';
      }

      count++;
      if (count > 3)
         {
           break;
         }
    }
    if (b->bay_bf && sp->sp_box_full == 'y')
       {
          h = &hw[b->bay_bf - 1];

          bf_mod = h->hw_mod_address;
       }
    for (m = b->bay_prod_first; m && m <= b->bay_prod_last; m++)
    {
      i = &pw[m - 1];
	
      if (flag & 4)                       /* ExM common extinguish           */
      {
	if ((i->pw_flags & BinHasPick) && !(i->pw_flags & BinPicked))
	{
	  i->pw_case_picked = i->pw_case_ordered;
	  i->pw_pack_picked = i->pw_pack_ordered;
	  i->pw_picked      = i->pw_ordered;
	  
	  i->pw_flags |= BinPicked;
	  h->hw_flags |= ModulePicked;
	}
      }
      if ((i->pw_flags & BinPicked) && (flag & 2))
         {
           x.m_module    = m;
           x.m_picked    = i->pw_picked +
                           i->pw_case_picked * i->pw_case + 
                           i->pw_pack_picked * i->pw_pack;

           x.m_reference = i->pw_reference;

           if (sp->sp_box_feature == 's')     /* F052698 - pick with box */
              {
                y.m_pickline  = z->zt_pl;
                y.m_order     = z->zt_on;
                y.m_module    = x.m_module;
                y.m_picked    = x.m_picked;
                y.m_reference = x.m_reference;
                y.m_box       = 0;
			 
                if (!(oc->oi_tab[z->zt_order - 1].oi_flags & NEED_BOX))
                   {
                     memset(text, 0x0, BoxNoLength+1);
                     memcpy(text,
                            oc->oi_tab[z->zt_order - 1].oi_box,
                            BoxNoLength);
                     text[BoxNoLength] = '\0';
                     y.m_box = atol(text);
                   }
                message_put(0, ModulePickBoxEvent, &y, sizeof(TPickBoxMessage));
              }
           else
              {
                message_put(0, ModulePickEvent, &x, sizeof(TPickMessage));
              }
#ifdef TANDY
           if (i->pw_flags & BinPicked)
              {
                i->pw_ordered      = i->pw_picked      = 0;
                i->pw_case_ordered = i->pw_case_picked = 0;
                i->pw_pack_ordered = i->pw_pack_picked = 0;
                i->pw_flags       &= PicksInhibited;
              }
#else
           i->pw_ordered      = i->pw_picked      = 0;
           i->pw_case_ordered = i->pw_case_picked = 0;
           i->pw_pack_ordered = i->pw_pack_picked = 0;
           i->pw_flags       &= PicksInhibited;
#endif
         }
      else if (flag & 1)
         {
           i->pw_ordered      = i->pw_picked      = 0;
           i->pw_case_ordered = i->pw_case_picked = 0;
           i->pw_pack_ordered = i->pw_pack_picked = 0;
           i->pw_flags       &= PicksInhibited;
         }
    }

    for (m = b->bay_mod_first; m && m <= b->bay_mod_last; m++)
    {
      h = &hw[mh[m - 1].mh_ptr - 1];
      
      if (flag & 1)                       /* clear all modules               */
      {
	if (sp->sp_pickline_view == 'y')
	{
	  memset(pmv[h->hw_mod - 1].hw_display, 0x20, 6);
	}
	h->hw_state   = 0;
	h->hw_save    = 0;
	h->hw_current = 0;
	h->hw_flags  &= SwitchesDisabled;
      }
      else if (h->hw_flags & ModulePicked)
      {
#ifdef TANDY
        if (h->hw_mod == z->zt_last_light && clear_flag)
           {
              h->hw_flags &= ~ModulePicked;
              h->hw_state   = 2;
              if (z->zt_lines)
                 {
                    b->bay_picks += 1;
                    z->zt_lines  += 1;
                 }
              else
                 {
                    b->bay_picks = 1;
                    z->zt_lines  = 1;
                 }
              hz = &hw[b->bay_zc - 1];
              hz->hw_state = 3;
              if (h->hw_type == PM2)
                 {
                    process_pm2_switch(z, b, h, 2);
                 }
              else
                 {
                    process_pm4_switch(z, b, h, 2);
                 }
           }
        else
           {
#endif
              if (sp->sp_pickline_view == 'y')
                 {
                   memset(pmv[h->hw_mod - 1].hw_display, 0x20, 6);
                 }

              h->hw_state   = 0;
              h->hw_save    = 0;
              h->hw_current = 0;
              h->hw_flags  &= SwitchesDisabled;
#ifdef TANDY
           }
#endif
      }
    }  /* end of for loop to set each pick module's parameters */

    if (flag & 1)
       {
         /* Find first hw_mod_address in bay */
         m = mh[b->bay_mod_first - 1].mh_ptr - 1;

         first_mod = hw[m].hw_mod_address;
     
         if (b->bay_zc && zc_mod < first_mod)
            {
              first_mod = zc_mod;
              m         = b->bay_zc - 1;
            }

         if (b->bay_bl && bl_mod < first_mod)
            {
              first_mod = bl_mod;
              m         = b->bay_bl - 1;
            }

         if (b->bay_bf && bf_mod < first_mod)
            {
              first_mod = bf_mod;
              m         = b->bay_bf - 1;
            }

         hw_cnt = 0L;

         m_first = m;

         while (hw[m].hw_bay == b->bay_number)
               {
                 hw_cnt++;
                 m++;

                 if (m_mbl >= 0L                                         &&
                     hw[m_mbl].hw_mod_address == hw[m].hw_mod_address    &&
                     hw[m_mbl].hw_controller  == hw[m - 1].hw_controller   )
                    {  /* found a Master Bay Lamp in the module range */
                      hw_cnt++;
                      m++;
                    }
               }  /* end of while loop to get device count in bay */

         m = 0L;
         while (hw_cnt > 0L)
               {
                 m = m_first;

                 if (hw_cnt > MAX_CMD_LGTH)
                    {
                      cmd_lgth = MAX_CMD_LGTH;
                    }
                 else
                    {
                      cmd_lgth = hw_cnt; 
                    }

                 /* Setup beginning part of message with first */
                 /* hw_mod_address in the bay to start at      */
                 memset(text, 0x0, MAX_MSG_SIZE);
                 sprintf(text, "%04d10%03d",
                         hw[m].hw_controller,
                         hw[m].hw_mod_address);

                 memset(&text[9], 'B', cmd_lgth);
                 Ac_Write(j, text, cmd_lgth + 9, 0);

                 hw_cnt  -= cmd_lgth;
                 m_first += cmd_lgth;
               }  /* end of while loop to write enable messages */
       }
  }  /* end of while loop for each bay */
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Return All Picks On Markplace or Shutdown
 *-------------------------------------------------------------------------*/
return_picks()
{
  register long k;
  register struct zone_item *z;

  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (!(z->zt_flags & IsTotalFunction)) continue;
    zone_clear(z->zt_zone, 3);
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Start A Zone
 *-------------------------------------------------------------------------*/
start_request(zn)
register long zn;
{
  TZoneStatusMessage y;
  register struct zone_item *z;
  long ntime = 0L;
  
  z = &zone[zn - 1];                      /* point to zone record            */
  
#ifdef DEBUG
  fprintf(DF, "start_request(): Zone=%d Status=%c Lines=%d\n",
  z->zt_zone, z->zt_status, z->zt_lines);
  fflush(DF);
  ntime = time(0);
  fprintf(DF, "                 Time=[%24.24s]\n", ctime(&ntime));
  fflush(DF);
#endif
  
  if (!(z->zt_flags & IsTotalFunction)) return 0;

  if (show_zone_display(z) < 0) return 0; /* ignore bad status               */

  z->zt_flags &= ~ZoneInactive;
  
#ifdef RESPONSE
  if (z->zt_status == ZS_UNDERWAY && rstart[z->zt_zone] > 0)
  {
    long now;
    
    now = time(0);
    
    fprintf(stderr, "%5.5s Zone:%3d Lines:%2d Time=%d\n",
    ctime(&now) + 11, z->zt_zone, z->zt_lines,
    microclock() - rstart[z->zt_zone]);
  }
  rstart[z->zt_zone] = 0;
#endif

  if (sp->sp_zone_status_events == 'y')
  {
    y.m_pickline = z->zt_pl;
    y.m_zone     = z->zt_zone;
    y.m_order    = z->zt_on;
    y.m_status   = z->zt_status;
    message_put(0, ZoneStatusEvent, &y, sizeof(TZoneStatusMessage));
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Show Zone Display - text is tf - fftext is ff - zc displays.
 *-------------------------------------------------------------------------*/
show_zone_display(z)
register struct zone_item *z;
{
  register long state;
  char text[20], fftext[6], order[8];
  unsigned short int index = 0;
  register struct oc_item *oz;
  register long block      = 0L,
                k          = 0L,
                next_order = 0L;
  long ntime = 0L;
#ifdef TANDY
  long found;
#endif
  
#ifdef DEBUG
  fprintf(DF, "show_zone_display(): Zone=%d Status=%c Lines=%d\n",
    z->zt_zone, z->zt_status, z->zt_lines);
  fflush(DF);
  ntime = time(0);
  fprintf(DF, "                     Time = [%24.24s]\n", ctime(&ntime));
  fflush(DF);
#endif

  
#ifdef BOXFULL
/* Not needed for Eckerd's - aha 100801 */
/*
  if (!(z->zt_flags & DemandFeed) && (z->zt_flags & ScanBoxClose))
     {
       zone_message(z,"SCAN LABEL      ",0,-1);
       return;
     }
*/
#endif

  state = 2;
  
  sprintf(order, "%6.*d", rf->rf_on, z->zt_on % 1000000);

  oz = &oc->oc_tab[z->zt_pl - 1];

  /* Get next order number for "SCAN LABEL" message */
  for (k = OC_HIGH; k <= OC_LOW; k++)
      {
        block = oz->oc_queue[k].oc_first;
        if (block)
           {
             break;
           }
      }

  next_order = oc->oi_tab[block - 1].oi_on;
#ifdef DEBUG
  fprintf(DF, "  Next order is %d from queue %d on pickline %d\n",
          next_order, k, z->zt_pl);
  fflush(DF);
#endif

  switch (z->zt_status)
  {
    case ZS_AHEAD:
    
      strcpy(fftext, "AAAAA");
      if (sp->sp_productivity == 'y')
         {
            if (!z->zt_picker)
               {
                  zone_display(text, am.alc_login, z);
                  break;
               }
            else
               {
                  zone_display(text, am.alc_ahead, z);
                  break;
               }
         }
      else
         {
            zone_display(text, am.alc_ahead, z);
            break;
         }

      break;
	    
    case ZS_LATE:
    
      strcpy(fftext, "EEEEE");

      if (sp->sp_productivity == 'y')
         {
            if (!z->zt_picker)
               {
                  zone_display(text, am.alc_login, z);
                  break;
               }
            else
               {
                  zone_display(text, am.alc_late_entry, z);
                  if (sp->sp_late_entry_one_button != 'y') state = 0;
                  break;
               }
         }
      else
         { 
            zone_display(text, am.alc_late_entry, z);
            if (sp->sp_late_entry_one_button != 'y') state = 0;
            break;
         }

      break;
      
    case ZS_COMPLETE:

      strcpy(fftext, "CCCCC");
      if (sp->sp_productivity == 'y')
         {
            if (!z->zt_picker)
               {
                  zone_display(text, am.alc_login, z);
                  break;
               }
            else
               {
                  zone_display(text, am.alc_complete, z);
                  break;
               }
         }
      else
         { 
            zone_display(text, am.alc_complete, z);
            break;
         }

      break;
 
    case ZS_LOCKED:
      
      strcpy(fftext, "LLLLL");
      zone_display(text, am.alc_locked, z);
      break;
    
    case ZS_WAITING:
    
      strcpy(fftext, "PPPPP");
      if (sp->sp_productivity == 'y')
         {
            if (!z->zt_picker)
               {
                  zone_display(text, am.alc_login, z);
                  break;
               }
            else
               {
                  if (oz->oc_queue[OC_HIGH].oc_first == 0 &&
                      oz->oc_queue[OC_MED].oc_first  == 0 &&
                      oz->oc_queue[OC_LOW].oc_first  == 0 &&
                      oz->oc_queue[OC_HOLD].oc_first == 0   )
                     {
                       zone_display(text, am.alc_complete, z);
                     }
                  else
                     {
                       memset(text, 0x0, 20);
                       if (next_order)
                          {
                            sprintf(text, "SCAN LABEL%06.6d", next_order);
                          }
                       else
                          {
                            zone_display(text, am.alc_complete, z);
                          } 
                     }
                  break;
               }
         }
      else
         {
            if (oz->oc_queue[OC_HIGH].oc_first == 0 &&
                oz->oc_queue[OC_MED].oc_first  == 0 &&
                oz->oc_queue[OC_LOW].oc_first  == 0 &&
                oz->oc_queue[OC_HOLD].oc_first == 0   )
               {
                 zone_display(text, am.alc_complete, z);
               }
            else
               {
                 memset(text, 0x0, 20);
                 if (next_order)
                    {
                      sprintf(text, "SCAN LABEL%06.6d", next_order);
                    }
                 else
                    {
                      zone_display(text, am.alc_complete, z);
                    } 
               }
            break;
         }

      break;

    case ZS_EARLY:
    
      if (z->zt_lines > 0)
      {
        strcpy(fftext, order + 1);
        if (sp->sp_productivity == 'y')
           {
             if (!z->zt_picker)
                {
                   zone_display(text, am.alc_login, z);
                   break;
                }
             else
                {
                   zone_display(text, am.alc_ee_picks, z);
                   break;
                }
           }
        else
           { 
             zone_display(text, am.alc_ee_picks, z);
             break;
           }
      }
      else
      {
	strcpy(fftext, order + 1);
	fftext[0] = 'E';

        if (sp->sp_productivity == 'y')
           {
             if (!z->zt_picker)
                {
                   zone_display(text, am.alc_login, z);
                   break;
                }
             else
                {
                   zone_display(text, am.alc_ee_no_picks, z);
                   if (sp->sp_early_exit_one_button != 'y') state = 0;
                   break;
                }
           }
        else
           { 
             zone_display(text, am.alc_ee_no_picks, z);
             if (sp->sp_early_exit_one_button != 'y') state = 0;
             break;
           }
      }
      break;
    
    case ZS_UNDERWAY:
    
      if (z->zt_lines > 0)
      {
        strcpy(fftext, order + 1);
        if (z->zt_flags & FirstZone)
           {
             if (sp->sp_productivity == 'y')
                {
                   if (!z->zt_picker)
                      {
                         zone_display(text, am.alc_login, z);
                         break;
                      }
                   else
                      {
                         zone_display(text, am.alc_uw_picks, z);
                         break;
                      }
                }
             else
                { 
                   zone_display(text, am.alc_uw_picks, z);
                   break;
                }
           }
        else if (sp->sp_productivity == 'y')
           {
             if (!z->zt_picker)
                {
                   zone_display(text, am.alc_login, z);
                   break;
                }
             else
                {
                   zone_display(text, am.alc_uw_picks, z);
                   break;
                }
           }
        else
           {
             zone_display(text, am.alc_uw_picks, z);
             break;
           }
      }
      else
      {
	strcpy(fftext, order + 1);
	fftext[0] = '-';
       
        if (sp->sp_productivity == 'y')
           {
              if (!z->zt_picker)
                 {
                    zone_display(text, am.alc_login, z);
                    break;
                 }
              else
                 {
                    zone_display(text, am.alc_uw_no_picks, z);
                    if (sp->sp_no_pick_one_button != 'y') state = 0;
                    break;
                 }
           }
        else
           { 
              zone_display(text, am.alc_uw_no_picks, z);
              if (sp->sp_no_pick_one_button != 'y') state = 0;
              break;
           }
      }
      break;
      
    case ZS_OFFLINE:
    
      strcpy(fftext, "OFF  ");
      zone_display(text, am.alc_login, z);
      state = 0;
      break;
 

    default:
    
      return -1;                          /* invalid zone status             */
  }
#ifdef DEBUG
  fprintf(DF, "In show_zone_display(), state = %d, text = %s\n",
          state, text);
  fflush(DF);
#endif
  zone_message(z, text, fftext, state); /* F052698 - add dd + tf text       */
  return 0;
}


/*-------------------------------------------------------------------------*
 * Zone Message Request
 *-------------------------------------------------------------------------*/
zone_message_request(x)
register TZoneDisplayMessage *x;
{
  register struct zone_item *z;
  char fftext[6];
  
#ifdef DEBUG
  fprintf(DF, "zone_message_request()zone=%d\n",x->m_zone);
  fflush(DF);
#endif
 
  if (x->m_zone < 1 || x->m_zone > coh->co_zone_cnt) 
  {
#ifdef DEBUG
  fprintf(DF, "zone_message_request()zone=%d not in limit \n",x->m_zone);
  fflush(DF);
#endif
	return 0;
  } 
  z = &zone[x->m_zone - 1];
  strncpy(fftext, x->m_text, 5);                                /* F052698 - add ff text                          */
  fftext[5] = 0;
  
  zone_message(z, x->m_text, fftext, 5);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Message Sender To Zone Controller - !fftext is no display!
 *-------------------------------------------------------------------------*/
zone_message(z, p, fftext, state)
register struct zone_item *z;
register char *p, *fftext;
register long state;
{
  register struct bay_item *b;
  register struct hw_item *h;
  register long j, k;
  char text[32];
  
  k = z->zt_first_bay;

#ifdef DEBUG
  fprintf(DF, "zone_message(): Zone=%d [%s]\n", z->zt_zone, p);
  fflush(DF);
#endif

  while (k > 0)
  {
    b = &bay[k - 1];
    k = b->bay_next;
    if (!b->bay_zc) continue;

    h = &hw[b->bay_zc - 1];
    j = b->bay_port - 1;

    if (state >= 0)
    {
      h->hw_save = h->hw_state;
      h->hw_state = state;
    }
    if (h->hw_type == ZC2)                                              /* F011596                                                                */
    {
      sprintf(text, "%04d09%03d%16.16s", 
	      h->hw_controller, h->hw_mod_address, p);
      Ac_Write(j, text, 25, 0);
#ifdef DEBUG
      fprintf(DF, "zone_message(): Zone=%d, text = %s, state = %d\n",
              z->zt_zone, p, h->hw_state);
      fflush(DF);
#endif

      if (sp->sp_pickline_view == 'y')
      {
	memcpy(zcv[h->hw_mod - 1].hw_display, text + 9, 16);
      }
    }
    else if (h->hw_type == ZC && fftext)        /* full function ZC                                       */
    {
      sprintf(text, "%04d09%03d%5.5s", 
	      h->hw_controller, h->hw_mod_address, fftext);
      Ac_Write(j, text, 14, 0);

      if (sp->sp_pickline_view == 'y')
      {
	memcpy(zcv[h->hw_mod - 1].hw_display, text + 9, 5);
      }
    }
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Pick Request
 *-------------------------------------------------------------------------*/
pick_request(x)
register TPickRequestMessage *x;
{
  register struct hw_item   *h, *g;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long j, k, quan;
  char text[20];
  long ntime = 0L;
  
#ifdef DEBUG
  fprintf(DF, "pick_request()\n");
  fflush(DF);
  ntime = time(0);
  fprintf(DF, "              Time = [%24.24s]\n", ctime(&ntime));
  fflush(DF);
#endif

  i = &pw[x->m_module - 1];
  k = i->pw_ptr;
  h = &hw[k - 1];
  b = &bay[h->hw_bay - 1];
  z = &zone[b->bay_zone - 1];
  
  if (!(b->bay_flags & IsTotalFunction)) return 0;
  
  j = b->bay_port - 1;
  
  if (i->pw_flags & PicksInhibited || x->m_ordered < 1)
  {
    z->zt_lines--;
    x->m_ordered = 0;
    message_put(0, ModulePickEvent, x, sizeof(TPickMessage));
    return 0;
  }
  b->bay_picks   += 1;

  h->hw_flags    |= ModuleHasPick;

  i->pw_reference = x->m_reference;
  i->pw_flags    |= BinHasPick;

  quan = x->m_ordered;
    
  if (sp->sp_um_in_pick_text != 'n')
  {
    i->pw_case = i->pw_pack = 0;          /* clear case and pack factors     */

    switch (x->m_um)
    {
      case 'C': i->pw_case = x->m_case_pack; break;
      case 'P': i->pw_pack = x->m_case_pack; break;
      default:  i->pw_case = 1000; i->pw_pack = 100; break;
    }
  }
  if (h->hw_type == PM2 || i->pw_case != 1000 || i->pw_pack != 100 ||
    (b->bay_flags & Multibin))
  {
    if (i->pw_case > 0 && quan >= i->pw_case)
    {
      i->pw_case_ordered = quan / i->pw_case;
      quan -= (i->pw_case * i->pw_case_ordered);
    }
    if (i->pw_pack > 0 && quan >= i->pw_pack)
    {
      i->pw_pack_ordered = quan / i->pw_pack;
      quan -= (i->pw_pack * i->pw_pack_ordered);
    }
  }
  i->pw_ordered = quan;
    
  if (b->bay_picks == 1) check_bl_on(b);

#ifdef DEBUG
  fprintf(DF, "Bay=%d Picks=%d Mod=%d Quan=%d (%d,%d,%d)\n",
    b->bay_number, b->bay_picks, x->m_module, x->m_ordered,
    i->pw_case_ordered, i->pw_pack_ordered, i->pw_ordered);
  fflush(DF);
#endif

  for (k = 1; k <= x->m_count; k++)     /* multiple display modules        */
  {
    if (k + h->hw_mod > coh->co_mod_cnt) break;/* no more lights           */

    g = &hw[mh[k + h->hw_mod - 1].mh_ptr - 1];

    g->hw_flags |= ModuleHasPick;
    g->hw_save   = k;                   /* offset to base module           */
  }
  if (!(b->bay_flags & Multibin))       /* not matrix or carousel          */
  {
    h->hw_state   = 0;
    h->hw_current = x->m_module;
    if (h->hw_type == PM2) process_pm2_switch(z, b, h, 2);
    else process_pm4_switch(z, b, h, 2);
    return 0;
  }
  i->pw_case_picked = i->pw_case_ordered;
  i->pw_pack_picked = i->pw_pack_ordered;
  i->pw_picked      = i->pw_ordered;
  
  if (!h->hw_current || x->m_module < h->hw_current)/* new least product   */
  {
    h->hw_state   = 0;
    h->hw_current = x->m_module;
    if (h->hw_type == PM6) process_pm6_matrix(z, b, h, 2);
    else
    {
      if (b->bay_flags & VertLights) process_pm4_matrix(z, b, h, 2);
      if (b->bay_flags & HortLights) process_pm4_bar(z, b, h, 2);
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Error Packet
 *-------------------------------------------------------------------------*/
process_error_packet()
{
  char text[512];
  
  memcpy(text, &buf, len);
  text[len] = 0;

#ifdef DEBUG
  fprintf(DF, "process_error_packet(): [%s]\n", text);
  fflush(DF);
#else
  krash("process_error_packet", text, 0);
#endif
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Input Switch Packet 
 *-------------------------------------------------------------------------*/
process_packet(buf)
register unsigned char *buf;
{
  register struct bay_item *b;
  register struct zone_item *z;
  register struct hw_item *h;
  register long k, m, n;
  long ac, mod, what;
  char text[32];
  register struct oi_item *o;

  
  sscanf(buf,  "%04d", &ac);              /* controller address              */
  sscanf(buf + 6,  "%03d", &mod);         /* module address                  */
  what = buf[9] - '0';                    /* switch action                   */
  k    = buf[10];                         /* port number                     */

#ifdef DEBUG
  fprintf(DF, "process_packet(): AC=%d Mod=%d Swt=%d Port=%d\n",
    ac, mod, what, k);
  fflush(DF);
#endif

  if (k < 0 || k >= coh->co_port_cnt)   
  {
    return process_error_packet();
  }
  if (ac < 0 || ac > (first_ac_on_port[k + 1]  - first_ac_on_port[k]))
  {  
    return process_error_packet();
  }
  m = first_ac_on_port[k];                /* offset to ac list               */
  n = first_hw_on_ac[m + ac - 1] + mod;   /* offset in hw for module         */

  if (n < 0 || n >= coh->co_light_cnt) 
  {
    return process_error_packet();
  }
  h = &hw[n];

  if (!h->hw_bay) return 0;               /* module not configured           */
  b = &bay[h->hw_bay - 1];                /* point to bay                    */
  if (!b->bay_zone) return 0;             /* no zone                         */
  z = &zone[b->bay_zone - 1];             /* point to zone                   */

#ifdef TANDY
    if (h->hw_type == PM2 || h->hw_type == PM4 || h->hw_type == PM6)
    {
	z->zt_last_light = h->hw_mod;
    }
    else
    {
/* commented this to retain the last light before box full*/
	//z->zt_last_light = 0;  
    }
#endif

  if (sp->sp_box_feature == 's')    /* F052698 - check has a box yet  */
  {
    if (z->zt_status == ZS_UNDERWAY || z->zt_status == ZS_EARLY )        
    {
      if (oc->oi_tab[z->zt_order - 1].oi_flags & NEED_BOX) 
      {
	if (what == 0) return 0;    /* ignore button up   */
	if (!(po[b->bay_port - 1].po_flags & IsFullFunction)) return 0;
	
	if (h->hw_type == ZC)           process_zc_refresh(z, b, h);
	else if (h->hw_type == PM2)     process_pm_refresh(z, b, h);

	return 0;
      }
    }
  }
  if (!(b->bay_flags & Multibin))         /* hw_save used for long switch    */
  {
    if (h->hw_type == PM2 || h->hw_type == PM4 || h->hw_type == PM6)
    {
      if (h->hw_save > 0)                 /* multiple display modules        */
      {
	h = &hw[mh[h->hw_mod - h->hw_save - 1].mh_ptr - 1];
      }
    }
  }
#ifdef DEBUG
  fprintf(DF, "zone address = 0x%x\n", z);
  Bdumpf(DF, z, sizeof(struct zone_item), DF);
  
  fprintf(DF, "Zone=%d Status=%c Lines=%d ",
  z->zt_zone, z->zt_status, z->zt_lines);

  fprintf(DF, " Bay=%d Picks=%d Module=%d State=%d",
  b->bay_number, b->bay_picks, h->hw_mod, h->hw_state);

  if (z->zt_flags & ZoneInactive) fprintf(DF, " ZoneInactive");
  fprintf(DF, "\n");
  fflush(DF);
#endif

  if (z->zt_flags & IsOffline)    return 0;             /* ignore when offline       */   
  if (z->zt_flags & ZoneInactive) return 0;             /* ignore when inactive      */   
  if (h->hw_flags & SwitchesDisabled) return 0; /* should be disabled        */
  
  h->hw_switch = what;

  o = &oc->oi_tab[z->zt_order - 1];

  switch (h->hw_type)
  {
    case ZC:
    case ZC2: process_zc_switch(z, b, h, what);
	      break;
    
    case PM2: 
	      if (b->bay_flags & VertLights)
		process_pm4_matrix(z, b, h, what);
	      else if (b->bay_flags & HortLights)
		process_pm4_bar(z, b, h, what);
	      else
		process_pm2_switch(z, b, h, what);
	      
	      //if (z->zt_lines <= 0) process_zc_done(z);
	      if (z->zt_lines <= 0 && z->zt_on != 0) process_zc_done(z);

	      break;
    
    case PM4: 
	      if (b->bay_flags & VertLights)/* F062896                       */
		process_pm4_matrix(z, b, h, what);
	      else if (b->bay_flags & HortLights)
		process_pm4_bar(z, b, h, what);
	      else
		process_pm4_switch(z, b, h, what);

	      if (z->zt_lines <= 0 && z->zt_on != 0) process_zc_done(z);
	      break;
	      
    case PM6: if (b->bay_flags & VertLights)
	      process_pm6_matrix(z, b, h, what);

	      if (z->zt_lines <= 0 && z->zt_on != 0) process_zc_done(z);
	      break;
	      
    case BF:  process_box_full(z, b, h, what);
	      break;

    default:  break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Scanner Packet 
 *-------------------------------------------------------------------------*/
process_io_packet(buf)
register TIOMessage *buf;
{
  register struct zone_item *z;
  register struct bay_item *b;
  register struct hw_item *h;
  register long k, m, n;
  long ac, mod;
  
  sscanf(buf->m_ac,  "%04d", &ac);        /* controller address              */
  sscanf(buf->m_mod,  "%03d", &mod);      /* module address                  */

  k = buf->m_port;                           /* port number                     */

#ifdef DEBUG
  fprintf(DF, "process_io_packet(): AC=%d Mod=%d Port=%d Scan=[%s]\n",
	  ac, mod, buf->m_port, buf->m_text);
  fflush(DF);
#endif

  if (k < 0 || k >= coh->co_port_cnt)   
  {
    return process_error_packet();                      /* bad port                                                       */
  }
  if (ac < 0 || ac > (first_ac_on_port[k + 1]  - first_ac_on_port[k]))
  {
    return process_error_packet();                      /* bad command module address             */
  }
  m = first_ac_on_port[k];                /* offset to ac list               */
  n = first_hw_on_ac[m + ac - 1] + mod;   /* offset in hw for module         */

  if (n < 0 || n >= coh->co_light_cnt) 
  {
    return process_error_packet();                      /* invalid module hwix                            */
  }
  h = &hw[n];
#ifdef DEBUG
  fprintf(DF, "process_io_packet n=%d ,m=%d, ac=%d mod = %d)\n", 
	  n, m, ac, h->hw_mod);
  fflush(DF);
#endif
  if (!h->hw_bay) return 0;               /* module not configured           */
  b = &bay[h->hw_bay - 1];                /* point to bay                    */
#ifdef DEBUG
  fprintf(DF, "process_io_packet(bay =%d, )\n", 
	   b->bay_number);
  fflush(DF);
#endif
  if (!b->bay_zone) return 0;             /* no zone                         */
  z = &zone[b->bay_zone - 1];             /* point to zone                   */
#ifdef DEBUG
  fprintf(DF, "process_io_packet(%d, %d, %d , %s)\n", 
	  z->zt_zone, b->bay_number, h->hw_mod, buf->m_text);
  fflush(DF);
#endif
  
  if (z->zt_flags & IsOffline)    return 0;             /* ignore when offline            */
  if (z->zt_flags & ZoneInactive) return 0;             /* ignore when inactive           */
  if (h->hw_flags & SwitchesDisabled) return 0; /* should be enabled                      */

/*---------------------------------------------------------------------------*
 *  Add code here to process special scans
 *---------------------------------------------------------------------------*/

  if (h->hw_state & SCAN_PL_INDUCT)     process_pl_scan(z, b, h, buf->m_text);
  if (h->hw_state & SCAN_ZONE_INDUCT)   {;}
  /* aha - Use SCAN_BOX for Eckerd's picker login and tote scans */
  if (h->hw_state & SCAN_BOX)           process_zn_scan(z, b, h, buf->m_text);
  if (h->hw_state & SCAN_LOT)                           {;}
  if (h->hw_state & SCAN_SERIAL)                        {;}
  if (h->hw_state & 0x20)                                       {;}
  if (h->hw_state & 0x40)                                       {;}
  if (h->hw_state & 0x80)                                       {;}
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process A Pickline Order Induction Scan
 *-------------------------------------------------------------------------*/
process_pl_scan(z, b, h, p)
register struct zone_item *z;
register struct bay_item *b;
register struct hw_item *h;
unsigned char *p;
{
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process A Zone Complete Event - Nothing to Full Function
 *-------------------------------------------------------------------------*/
process_zc_done(z)
register struct zone_item *z;
{
  register long ret;
  char text[32], fftext[6];
#ifdef TANDY
  long found;
  long block;
  register struct oi_item *o;
#endif

#ifdef DEBUG
  fprintf(DF, "process_zc_done(): Zone=%d\n", z->zt_zone);
  fflush(DF);
#endif

  if (z->zt_lines > 0) return 0;


  switch (z->zt_status)
  {
    case ZS_EARLY:
		       strcpy(fftext, "CCC  "); /* F052698 */
		       zone_display(text, am.alc_exit_tote, z);
		       zone_message(z, text, fftext, -1);
		       break;

    case ZS_UNDERWAY:  ret = pass_tote(z);

		       if (ret == 1)
		       {
			 strcpy(fftext, "CCC  ");
			 zone_display(text, am.alc_exit_tote, z);
		       }
		       else if (ret == 2)
		       {
			 strcpy(fftext,  "PASS ");
			 zone_display(text, am.alc_pass_tote, z);
		       }
		       else if (ret == 3)
		       {
			 strcpy(fftext, "HHH  ");
			 zone_display(text, am.alc_hold_tote, z);
		       }
		       zone_message(z, text, 0, -1); /* 0 is no display  */
						     /* use fftext for ff */
		       break;
		       
    default:           break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process A Zone Controller Switch Action
 *-------------------------------------------------------------------------*
 *
 *  2-Button Start                1-Button Start
 *
 *  [State 0] ---> [State 1] ---> [State 2] ---> [State 3] ---> [State 4]
 *            EXT            up             NEXT           up
 *                                                         zone_clear()   
 *                                                         Complete Event
 *                                                         Next Event
 *  Special Message: [State 5] ---> [State 2]
 *                             Ext
 *                             Redisplay
 *-------------------------------------------------------------------------*/
process_zc_switch(z, b, h, what)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
register long what;
{
  register long ret;
  TBoxOrderMessage buf;
  char text[32];
#ifdef MIOM
  register struct oi_item *o;
  long block;
#endif

#ifdef DEBUG
  fprintf(DF, "process_zc_switch(): Zone=%d Bay=%d Mod=%d State=%d Swt=%d\n",
    z->zt_zone, b->bay_number, h->hw_mod, h->hw_state, what);
  fflush(DF);
#endif

  if (h->hw_state == 5 )
		return show_zone_display(z);

  o = &oc->oi_tab[z->zt_order - 1];
//	order structure is used to check the DISABLE FLAG

  switch (z->zt_status)
  {
    case ZS_LATE:     if (what == 1 && h->hw_state == 0)      h->hw_state = 1;
		      else if (what == 0 && h->hw_state == 1) h->hw_state = 2;
		      else if (what == 2 && h->hw_state == 2) h->hw_state = 3;
		      else if (what == 0 && h->hw_state == 3)
		      {
			zone_clear(z->zt_zone, 1);
#ifdef RESPONSE
			rstart[z->zt_zone] = microclock();
#endif
			message_put(coh->co_id, ZoneNextEvent,
				    &z->zt_zone, sizeof(TZone));
		      }
		      else if (what == 0 && h->hw_state != 3)
		      {  
			process_zc_refresh(z, b, h);
		      }
		      break;

    case ZS_AHEAD:    if (what == 0)
                         {
                           if (sp->sp_productivity == 'y')
                              {
                                 message_put(0, ZoneRedisplayRequest,
                                             &z->zt_zone, sizeof(TZone));
                              }
                           process_zc_refresh(z, b, h);
                         }
                      break;              /* switches are ignored            */

    case ZS_OFFLINE:  if (what == 0)process_zc_refresh(z, b, h);
		      break;

    case ZS_COMPLETE: if (what == 2 && h->hw_state == 2) h->hw_state = 3;
		      else if (what == 0 && h->hw_state == 3)
		      {
			zone_clear(z->zt_zone, 1);
			
#ifdef RESPONSE
			rstart[z->zt_zone] = microclock();
#endif
			message_put(coh->co_id, ZoneNextEvent,
				    &z->zt_zone, sizeof(TZone));
		      }
		      else if (what == 0 && h->hw_state != 3)
		      {  
			process_zc_refresh(z, b, h);
		      }
		      break;

    case ZS_LOCKED:   if (what == 2 && h->hw_state == 2) h->hw_state = 3;
		      else if (what == 0 && h->hw_state == 3)
		      {
			zone_clear(z->zt_zone, 1);
#ifdef RESPONSE
			rstart[z->zt_zone] = microclock();
#endif
			message_put(coh->co_id, ZoneNextEvent,
				    &z->zt_zone, sizeof(TZone));
		      }
		      else if (what == 0 && h->hw_state != 3)
		      {  
			process_zc_refresh(z, b, h);
		      }
		      break;

    case ZS_WAITING:  if (what == 2) h->hw_state = 3;
		      else if (what == 0 && h->hw_state == 3)
		      { 
			zone_clear(z->zt_zone, 1);
#ifdef RESPONSE
			rstart[z->zt_zone] = microclock();
#endif
			message_put(coh->co_id, ZoneNextEvent,
				    &z->zt_zone, sizeof(TZone));
		      }
		      else if (what == 0 && h->hw_state != 3)
		      {  
			process_zc_refresh(z, b, h);
		      }
		      break;
		       
    case ZS_EARLY:   if (sp->sp_box_feature == 'n')
                        {
#ifdef DEBUG
                          fprintf(DF, "In process_zc_switch()-ZS_UNDERWAY:\n");
                          fprintf(DF, "Zone=%d State=%d Swt=%d Lines = %d\n",
                                  z->zt_zone, h->hw_state, what, z->zt_lines);
                          fflush(DF);
#endif
                          if (what == 0 && h->hw_state == 3)
                             {
                               /* do nothing here */;
                             }
                          else if (z->zt_lines == 0 && z->zt_feeding == 0)
                             {
                               /* do nothing here */;
                             }
                          else
                             {
                               show_zone_display(z);
                             }
                        }
                      else if (sp->sp_box_feature == 's' && what == 1)
		      {
#ifdef TANDY
				if ( z->zt_flags & BoxFull)
				{
					z->zt_flags &= ~BoxFull ;
					zone_enable(z);
				}
				if (z->zt_flags & ScanBoxClose)
                                   {
                                     z->zt_flags &= ~ScanBoxClose;
                                   }
				if (o->oi_flags & DISABLE_ZONE)
                                   {
                                     zone_enable(z);
                                   }
				show_zone_display(z);
		       }
			else if (sp->sp_box_feature == 's' && what == 2	
				&& z->zt_flags & BoxFull )
		       {
				z->zt_flags &= ~BoxFull ;
				z->zt_flags |= ScanBoxClose ;
    				zone_clear(z->zt_zone, 2);                          					/* dump picks so far  */
				zone_disable(z,1);
				/* sprintf(text,"SCAN LABEL%06.6d",z->zt_on); */
                                sprintf(text, "SCAN LABEL      ");
			        zone_message(z,text,0,0);
				break;
			}
			else if (sp->sp_box_feature == 's' && what == 2	
				&& o->oi_flags & DISABLE_ZONE )
		        {
                		zone_clear(z->zt_zone,2);
                		zone_clear(z->zt_zone,1);
                		message_put(coh->co_id, ZoneCompleteEvent,
                           		 &z->zt_zone, sizeof(TZone));
                		message_put(coh->co_id, ZoneNextEvent,
                           		 &z->zt_zone, sizeof(TZone));
                		return(0);
			}
#else
				show_zone_display(z);
#endif
		      if (z->zt_lines)
		      {
			process_zc_refresh(z, b, h);
			break;
		      }
		      if (what == 1 && h->hw_state == 0)      h->hw_state = 1;
		      else if (what == 0 && h->hw_state == 1) h->hw_state = 2;
		      else if (what == 2 && h->hw_state == 2) h->hw_state = 3;
		      else if (what == 0 && h->hw_state == 3)
		      {
			zone_clear(z->zt_zone, 3);
#ifdef RESPONSE
			rstart[z->zt_zone] = microclock();
#endif
#ifdef TCBL_REMOVE
			message_put(0, ZoneCompleteEvent,
				     &z->zt_zone, sizeof(TZone));
#else
			message_put(coh->co_id, ZoneCompleteEvent,
				     &z->zt_zone, sizeof(TZone));
#endif
			message_put(coh->co_id, ZoneNextEvent,
				    &z->zt_zone, sizeof(TZone));
		      }
		      else if (what == 0 && h->hw_state != 3)
		      {  
			process_zc_refresh(z, b, h);
		      }
		      break;
		      
    case ZS_UNDERWAY:if (sp->sp_box_feature == 'n')
                        {
#ifdef DEBUG
                          fprintf(DF, "In process_zc_switch()-ZS_UNDERWAY:\n");
                          fprintf(DF, "Zone=%d State=%d Swt=%d Lines = %d\n",
                                  z->zt_zone, h->hw_state, what, z->zt_lines);
                          fflush(DF);
#endif
                          if (what == 0 && h->hw_state == 3)
                             {
                               /* do nothing here */;
                             }
                          else if (z->zt_lines == 0 && z->zt_feeding == 0)
                             {
                               /* do nothing here */;
                             }
                          else
                             {
                               show_zone_display(z);
                             }
                        }
                     else if (sp->sp_box_feature == 's' && what == 1)
			{
#ifdef TANDY
				if ( z->zt_flags & BoxFull)
				{
					z->zt_flags &= ~BoxFull ;
					zone_enable(z);
				}
				if (z->zt_flags & ScanBoxClose)
                                   {
                                     z->zt_flags &= ~ScanBoxClose;
                                   }
				if (o->oi_flags & DISABLE_ZONE)
                                   {
                                     zone_enable(z);
                                   }
				show_zone_display(z);
			 }
			else if (sp->sp_box_feature == 's' && what == 2	
				&& z->zt_flags & BoxFull )
			{
				z->zt_flags &= ~BoxFull ;
				z->zt_flags |= ScanBoxClose ;
    				zone_clear(z->zt_zone, 2);                          					/* dump picks so far  */
    				zone_disable(z,1);
                                sprintf(text, "SCAN LABEL      ");
			        zone_message(z,text,0,0);
				break;
			}
			else if (sp->sp_box_feature == 's' && what == 2	
				&& o->oi_flags & DISABLE_ZONE )
		        {
				if (z->zt_flags & SwitchesDisabled )
					return 0;
				// It does not allow the next switch to complete
				// the order if the zone is locked after a scan
                		zone_clear(z->zt_zone,2);
                		zone_clear(z->zt_zone,1);
                		/* message_put(0, ZoneCompleteEvent,
                            		 &z->zt_zone, sizeof(TZone)); */
                		message_put(coh->co_id, ZoneCompleteEvent,
                           		 &z->zt_zone, sizeof(TZone));
                		message_put(coh->co_id, ZoneNextEvent,
                           		 &z->zt_zone, sizeof(TZone));
                		return(0);
			}
#else
				show_zone_display(z);
#endif
		      if (z->zt_lines)
		      {
			if (what == 0) process_zc_refresh(z, b, h);
			break;
		      }
		
		      if (what == 1 && h->hw_state == 0)      h->hw_state = 1;
		      else if (what == 0 && h->hw_state == 1) h->hw_state = 2;
		      else if (what == 2 && h->hw_state == 2) h->hw_state = 3;
		      else if (what == 0 && h->hw_state == 3)
		      {
			zone_clear(z->zt_zone, 3);
#ifdef RESPONSE
			rstart[z->zt_zone] = microclock();
#endif
#ifdef TCBL_REMOVE
			message_put(0, ZoneCompleteEvent,
				     &z->zt_zone, sizeof(TZone));
#else
			message_put(coh->co_id, ZoneCompleteEvent,
				     &z->zt_zone, sizeof(TZone));
#endif
			message_put(coh->co_id, ZoneNextEvent,
				    &z->zt_zone, sizeof(TZone));
		      }
		      else if (what == 0 && h->hw_state != 3)
		      {  
			process_zc_refresh(z, b, h);
		      }
		      break;

    default:          process_zc_refresh(z, b, h);
		      break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Refresh Full Function - F052698 refreshes full function zc's
 *-------------------------------------------------------------------------*/
process_zc_refresh(z, b, h)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
{
  char text[32];

#ifdef DEBUG
  fprintf(DF, "process_zc_refresh()\n");
  fflush(DF);
#endif

#ifdef REFRESH
  if (h->hw_type != ZC) return 0;                               /* not full function                              */
  
  if (sp->sp_pickline_view != 'y') return 0;    /* need last display !            */

  sprintf(text, "%04d09%03d%5.5s",
    h->hw_controller, h->hw_mod_address, zcv[h->hw_mod - 1].hw_display);
    
  Ac_Write(b->bay_port - 1, text, 14, 0);
#endif
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Refresh Full Function - F052698 refreshes full function pm's
 *-------------------------------------------------------------------------*/
process_pm_refresh(z, b, h)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
{
  char text[32];

#ifdef DEBUG
  fprintf(DF, "process_pm_refresh()\n");
  fflush(DF);
#endif

#ifdef REFRESH
  if (h->hw_type != PM2) return 0;      
  if (sp->sp_pickline_view != 'y') return 0;    /* need last display !            */
  if (b->bay_zc == 0) return 0;
  
  if (hw[b->bay_zc - 1].hw_type != ZC) return 0;/* need full function zc         */
  
  sprintf(text, "%04d09%03d%c%c%c%c",
    h->hw_controller, h->hw_mod_address,
    pmv[h->hw_mod - 1].hw_display[0] & 0x7f,
    pmv[h->hw_mod - 1].hw_display[1],
    pmv[h->hw_mod - 1].hw_display[2],
    pmv[h->hw_mod - 1].hw_display[3]);
    
  Ac_Write(b->bay_port - 1, text, 13, 0);
#endif
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Pass Tote Message
 *
 *  Returns:  1 = EXIT; 2 = PASS; 3 = HOLD.
 *-------------------------------------------------------------------------*/
pass_tote(z)
register struct zone_item *z;
{
  register struct oi_item *o;

  if (!z->zt_feeding) return 1;           /* exit tote message here          */
  
  o = &oc->oi_tab[z->zt_order - 1];       /* order in current zone           */

  if (o->oi_queue == OC_HOLD) return 3;   /* order is now on hold            */

  return 2;                               /* pass tote to next zone          */
}
/*-------------------------------------------------------------------------*
 *  Process Pick Module Switches - Down (what==1) UP (what==0)
 *-------------------------------------------------------------------------*
 *        up             up             up             up
 *     +------+       +------+       +------+       +------+
 *     V      |       V      |       V      |       V      |
 * [State 0] -+-> [State 1] -+-> [State 2] -+-> [State 3] -+-> (4) 
 *     |     down     |     down           down           down
 *     |  show cases  |  show packs     show units     mark picked
 *     |  show thou   |  show hund                    show __
 *  show CC        show PP
 *     V              V
 *  [State 8]      [State 9]
 *    down           down
 *  show cases     show packs
 *     |              |
 *     V              V
 *    (1)            (2)
 *
 *        up             up             up             up
 *     +------+       +------+       +------+       +------+
 *     V      |       V      |       V      |       V      |
 * [State 4] -+-> [State 5] -+-> [State 6] -+-> [State 7] -+->
 *           down           down          down           down
 *          unpick       start shorts   start shorts    mark picked
 *          bl on         show packs     show units       show ____
 *         show cases        |              |
 *         show CC       [State 12]         |
 *            |           show PP           +--> [State 14] --> (7)
 *            |              |                              up
 *         [State 10]        |                           show units
 *         show cases        +-> [State 13] --> (6)
 *            |                             up      
 *            |                          show packs
 *            +---> [State 11] --> (5)
 *                            up          
 *                          show cases  
 *-------------------------------------------------------------------------*/
process_pm2_switch(z, b, h, what)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
register long what;
{
  register struct pw_item   *i;
  register long j;
  char text[20];
  
#ifdef DEBUG
  fprintf(DF, "process_pm2_switch(): Zone=%d Bay=%d Mod=%d State=%d Swt=%d\n",
  z->zt_zone, b->bay_number, h->hw_mod, h->hw_state, what);
  fflush(DF);
#endif
  
  j = b->bay_port - 1;
  
  i = &pw[h->hw_current - 1];             /* point to pick                   */
  
  if (!(i->pw_flags & BinHasPick)) return 0;
  
  switch (h->hw_state)
  {
    case 0: if (!what) return 0;          /* looping on cases                */
	    
	    h->hw_state = 1;

	    if (i->pw_case_ordered > 0)
	    {
	      i->pw_case_picked = i->pw_case_ordered;

	      if (i->pw_case == 1000)
	      {   
		sprintf(text, "%04d09%03d%3dT",
		      h->hw_controller, h->hw_mod_address, i->pw_case_ordered);
	      }
	      else if (i->pw_case_ordered < 10)
	      {
		sprintf(text, "%04d09%03d%3dC",
		      h->hw_controller, h->hw_mod_address, i->pw_case_ordered);
	      }
	      else
	      {
		sprintf(text, "%04d09%03d  CC",
			h->hw_controller, h->hw_mod_address);
		h->hw_state = 8;
	      }
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      return 0;
	    }                             /* fall through - no cases/thou    */

    case 1: if (!what) return 0;          /* looping on inner packs          */
		
	    h->hw_state = 2;

	    if (i->pw_pack_ordered > 0)
	    {
	      i->pw_pack_picked = i->pw_pack_ordered;

	      if (i->pw_pack == 100)
	      {
		sprintf(text, "%04d09%03d%3dH",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_ordered);
	      }
	      else if (i->pw_pack_ordered < 10)
	      {
		sprintf(text, "%04d09%03d%3dP",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_ordered);
	      }
	      else
	      {
		sprintf(text, "%04d09%03d  PP",
		       h->hw_controller, h->hw_mod_address);
		h->hw_state = 9;
	      }
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      return 0;
	    }                             /* fall through - no cases/thou    */
	    
    case 2: if (!what) return 0;          /* switch up on pick               */
  
	    h->hw_state = 3;

	    if (!i->pw_case_ordered && !i->pw_pack_ordered)
	    {  
	      if (sp->sp_blink_over && i->pw_ordered > sp->sp_blink_over)
	      {
#ifdef RED
		sprintf(text, "%04d10%03dR",    /* show red light            */
			       h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 10, h);
#endif
#ifdef YELLOW
		sprintf(text, "%04d10%03dY",    /* show red light            */
			       h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 10, h);
#endif
#ifndef NOBLINK
		h->hw_flags |= ModuleBlink;
		short_count += 1;

		if (short_count == 1)
		{
		  caps_interval(SHORT_INTERVAL, short_catcher);
		}
#endif
	      }
	    }
	    if (i->pw_ordered > 0)
	    {
	      i->pw_picked = i->pw_ordered;
	      
	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_ordered);
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      return 0;
	    }                             /* fall through - no units         */
	    
    case 3: if (!what) return 0;          /* units picks are done            */
	    
#ifndef NOBLINK
	    if (h->hw_flags & ModuleBlink)
	    {
	      h->hw_flags &= ~ModuleBlink;
	      short_count--;
	    }
#endif
	    z->zt_lines  -= 1;            /* mark now picked                 */
	    b->bay_picks -= 1;            /* mark now picked                 */

	    i->pw_flags |= BinPicked;
	    h->hw_flags |= ModulePicked;
 
	    if (b->bay_picks <= 0) check_bl_off(b);

	    sprintf(text, "%04d09%03d____",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 13, h);

	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
	    }
	    sprintf(text, "%04d10%03dF",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 10, h);
#ifdef RED
	    if (!i->pw_case_ordered && !i->pw_pack_ordered)
	    {
	      if (sp->sp_blink_over && i->pw_ordered > sp->sp_blink_over)
	      {
		//sprintf(text, "%04d10%03dG",  
		sprintf(text, "%04d10%03dR",  
		      h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 10, h);
	      }
	    }
#endif
#ifdef YELLOW
	    if (!i->pw_case_ordered && !i->pw_pack_ordered)
	    {
	      if (sp->sp_blink_over && i->pw_ordered > sp->sp_blink_over)
	      {
		//sprintf(text, "%04d10%03dG",  
		sprintf(text, "%04d10%03dY",  
		      h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 10, h);
	      }
	    }
#endif
	    h->hw_state = 4;
	    return 0;
	    
    case 4: if (!what) return 0;          /* switch down after pick          */

	    h->hw_state   = 5;

	    z->zt_lines  += 1;
	    b->bay_picks += 1;

	    if (b->bay_picks == 1) check_bl_on(b);

	    i->pw_flags  &= ~BinPicked;
	    h->hw_flags  &= ~ModulePicked;

	    if (i->pw_case_ordered > 0)
	    {
	      if (i->pw_case_picked >= 10 && i->pw_case != 1000)
	      {
		sprintf(text, "%04d09%03d  CC",
			h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 13, h);
	    
		if (sp->sp_pickline_view == 'y')
		{
		  memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
			 pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
		}
		h->hw_state  = 10;
		return 0;
	      }
	      h->hw_state   = 11;
	      short_count  += 1;

	      h->hw_flags  |= ModuleShortCount;

	      if (short_count == 1)
	      {
		caps_interval(2 * SHORT_INTERVAL, short_catcher);
	      }
	      return 0;
	    }

    case 5: if (!what) return 0;          /* switch down after pick                       */ 
    
	    h->hw_state = 6;

	    if (i->pw_pack_ordered > 0)
	    {
	      if (i->pw_pack_picked >= 10 && i->pw_pack != 100)
	      {
		sprintf(text, "%04d09%03d  PP",
			h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 13, h);
	    
		if (sp->sp_pickline_view == 'y')
		{
		  memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
			 pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
		}
		h->hw_state  = 12;
		return 0;
	      }
	      h->hw_state   = 13;
	      short_count  += 1;

	      h->hw_flags  |= ModuleShortCount;

	      if (short_count == 1)
	      {
		caps_interval(2 * SHORT_INTERVAL, short_catcher);
	      }
	      return 0;
	    }

    case 6: if (!what) return 0;          /* switch down on pick             */
     
	    h->hw_state = 7;
	    
	    if (i->pw_ordered > 0)
	    {
	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_picked);
	      Ac_Write(j, text, 13, h);
	    
	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state   = 14;
	      short_count  += 1;

	      h->hw_flags  |= ModuleShortCount;

	      if (short_count == 1)
	      {
		caps_interval(2 * SHORT_INTERVAL, short_catcher);
	      }
	      return 0;
	    }

    case 7: if (!what) return 0;          /* short confirmed                 */
	    
	    z->zt_lines  -= 1;            /* mark now picked                 */
	    b->bay_picks -= 1;            /* mark now picked                 */

	    i->pw_flags |= BinPicked;
	    h->hw_flags |= ModulePicked;

	    if (b->bay_picks <= 0) check_bl_off(b);
 
	    sprintf(text, "%04d09%03d____",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
	    }
	    sprintf(text, "%04d10%03dF",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 10, h);
	    
	    h->hw_state = 4;
	    return 0;

    case 8: if (!what) return 0;          /* switch after CC                 */

	    if (i->pw_case_ordered > 99)
	    {
	      sprintf(text, "%04d09%03d****",
		      h->hw_controller, h->hw_mod_address);
	    }
	    else
	    {
	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_case_ordered);
	    }
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_flags  |= ModuleShortCount;

	    h->hw_state   = 1;
	    return 0;

    case 9: if (!what) return 0;          /* switch after PP                 */

	    if (i->pw_pack_ordered > 99)
	    {
	      sprintf(text, "%04d09%03d****",
		      h->hw_controller, h->hw_mod_address);
	    }
	    else
	    {
	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_ordered);
	    }
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {  
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_flags  |= ModuleShortCount;

	    h->hw_state   = 12;
	    return 0;
	    
    case 10: if (!what) return 0;         /* switch after CC                 */

	    if (i->pw_case_picked < 10)
	    {
	      sprintf(text, "%04d09%03d%3dC",
		      h->hw_controller, h->hw_mod_address, i->pw_case_picked);
	    }
	    else
	    {
	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_case_picked);
	    }
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_state   = 11;
	    short_count  += 1;

	    h->hw_flags  |= ModuleShortCount;
 
	    if (short_count == 1)
	    {
	      caps_interval(2 * SHORT_INTERVAL, short_catcher);
	    }
	    return 0;

    case 12: if (!what) return 0;         /* switch after PP                 */

	    if (i->pw_pack_picked < 10)
	    {
	      sprintf(text, "%04d09%03d%3dP",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_picked);
	    }
	    else
	    {
	       sprintf(text, "%04d09%03d%4d",
		       h->hw_controller, h->hw_mod_address, i->pw_pack_picked);
	    }
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_state   = 13;
	    short_count  += 1;

	    h->hw_flags  |= ModuleShortCount;

	    if (short_count == 1)
	    {
	      caps_interval(2 * SHORT_INTERVAL, short_catcher);
	    }
	    return 0;

    case 11: if (what) return 0;          /* switch up on short              */
	    
	    h->hw_flags &= ~ModuleShortCount;
	    short_count -= 1;

	    if (i->pw_case == 1000)
	    {
	      sprintf(text, "%04d09%03d%3dT",
		      h->hw_controller, h->hw_mod_address, i->pw_case_picked);
	    }
	    else if (i->pw_case_picked < 10)
	    {
	      sprintf(text, "%04d09%03d%3dC",
		      h->hw_controller, h->hw_mod_address, i->pw_case_picked);
	    }
	    else
	    {
	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_case_picked);
	    }
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_state = 5;
	    return 0;
		 
    case 13: if (what) return 0;          /* switch up on short              */
	    
	    h->hw_flags &= ~ModuleShortCount;
	    short_count -= 1;

	    if (i->pw_pack == 100)
	    {
	      sprintf(text, "%04d09%03d%3dH",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_picked);
	    }
	    else if (i->pw_pack_picked < 10)
	    {
	      sprintf(text, "%04d09%03d%3dP",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_picked);
	    }
	    else
	    {
	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_picked);
	    }
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_state = 6;
	    return 0;
	    
    case 14: if (what) return 0;          /* switch up on short              */
	    
	    h->hw_flags &= ~ModuleShortCount;
	    short_count -= 1;

	    sprintf(text, "%04d09%03d%4d",
		    h->hw_controller, h->hw_mod_address, i->pw_picked);
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_state = 7;
	    return 0;
    
    default: return 0;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Pick Module Switches - Down (what==2) Up (what==0)
 *-------------------------------------------------------------------------*
 *        up             up             up             up
 *     +------+       +------+       +------+       +------+
 *     V      |       V      |       V      |       V      |
 * [State 0] -+-> [State 1] -+-> [State 2] -+-> [State 3] -+-> (4) 
 *           down           down           down           down
 *        show cases     show packs     show units     mark picked
 *                                                     bl off
 *                                                     show ____
 *
 *        up             up             up             up
 *     +------+       +------+       +------+       +------+
 *     V      |       V      |       V      |       V      |
 * [State 4] -+-> [State 5] -+-> [State 6] -+-> [State 7] -+->
 *           down           down          down           down
 *          unpick       start shorts   start shorts    mark picked
 *          bl on         show packs     show units       show ____
 *         show cases        |              |
 *            |              |              |
 *            |              |              +--> [State 10] --> (7)
 *            |              |                              up
 *            |              |                           show units
 *            |              +-> [State 9] --> (6)
 *            |                            up      
 *            |                         show packs
 *            +---> [State 8] --> (5)
 *                            up          
 *                          show cases  
 *-------------------------------------------------------------------------*/
process_pm4_switch(z, b, h, what)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
register long what;
{
  register struct pw_item *i;
  register long j;
  char text[20], symbol;
  
#ifdef DEBUG
  fprintf(DF, "process_pm4_switch(): Zone=%d Bay=%d Mod=%d State=%d Swt=%d\n",
  z->zt_zone, b->bay_number, h->hw_mod, h->hw_state, what);
  fflush(DF);
#endif

  j = b->bay_port - 1;

  i = &pw[h->hw_current - 1];
  
  if (!(i->pw_flags & BinHasPick)) return 0;

  switch (h->hw_state)
  {
    case 0: if (!what) return 0;          /* looping on cases                */
	    
	    h->hw_state = 1;              /* next state always               */

	    if (i->pw_case_ordered > 0)
	    {
	      i->pw_case_picked = i->pw_case_ordered;

	      sprintf(text, "%04d09%03d%3dC",
	      h->hw_controller, h->hw_mod_address, i->pw_case_ordered);
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      return 0;
	    }

    case 1:  if (!what) return 0;         /* looping on inner packs          */
		
	    h->hw_state = 2;              /* next state always               */

	    if (i->pw_pack_ordered > 0)
	    {
	      i->pw_pack_picked = i->pw_pack_ordered;

	      sprintf(text, "%04d09%03d%3dP",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_ordered);
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      return 0;
	    }
	    
    case 2: if (!what) return 0;          /* loop on units                   */
	      
	    h->hw_state = 3;              /* next state always               */
	    
	    if (!i->pw_case_ordered && !i->pw_pack_ordered)
	    {
	      if (sp->sp_blink_over && i->pw_ordered > sp->sp_blink_over)
	      {
#ifdef RED
		 sprintf(text, "%04d10%03dR", /* show red color             */
			 h->hw_controller, h->hw_mod_address);
		 Ac_Write(j, text, 10, h);
#endif
#ifdef YELLOW
		sprintf(text, "%04d10%03dY",    /* show red light            */
			       h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 10, h);
#endif

#ifndef NOBLINK
		h->hw_flags |= ModuleBlink;
		short_count += 1;

		if (short_count == 1)
		{
		  caps_interval(SHORT_INTERVAL, short_catcher);
		}
#endif
	      }
	    }
	    if (i->pw_ordered > 0)
	    {
	      i->pw_picked = i->pw_ordered;

	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_ordered);
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      return 0;
	    }
	    
    case 3: if (!what) return 0;          /* units picks are done            */
	    
#ifndef NOBLINK
	    if (h->hw_flags & ModuleBlink)
	    {
	      h->hw_flags &= ~ModuleBlink;
	      short_count--;
	    }
#endif
	    z->zt_lines  -= 1;            /* mark now picked                 */
	    b->bay_picks -= 1;            /* mark now picked                 */

	    i->pw_flags |= BinPicked;
	    h->hw_flags |= ModulePicked;
 
	    if (b->bay_picks <= 0) check_bl_off(b);

	    sprintf(text, "%04d09%03d____",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 13, h);

	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
	    }
	    sprintf(text, "%04d10%03dF",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 10, h);
#ifdef YELLOW
	    if (!i->pw_case_ordered && !i->pw_pack_ordered)
	    {
	      if (sp->sp_blink_over && i->pw_ordered > sp->sp_blink_over)
	      {
		//sprintf(text, "%04d10%03dG",  
		sprintf(text, "%04d10%03dY",  
		      h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 10, h);
	      }
	    }
#endif
#ifdef RED
	    if (!i->pw_case_ordered && !i->pw_pack_ordered)
	    {
	      if (sp->sp_blink_over && i->pw_ordered > sp->sp_blink_over)
	      {
		//sprintf(text, "%04d10%03dG", 
		sprintf(text, "%04d10%03dR", 
			h->hw_controller, h->hw_mod_address);
		Ac_Write(j, text, 10, h);
	      }
	    }
#endif
	    h->hw_state = 4;
	    return 0;
	    
    case 4: if (!what) return 0;          /* switch down after pick          */

	    z->zt_lines  += 1;
	    b->bay_picks += 1;

	    if (b->bay_picks == 1) check_bl_on(b);

	    i->pw_flags &= ~BinPicked;
	    h->hw_flags &= ~ModulePicked;

	    if (i->pw_case_ordered > 0)
	    {
	      sprintf(text, "%04d09%03d%3dC",
		      h->hw_controller, h->hw_mod_address, i->pw_case_picked);
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state   = 8;
	      short_count  += 1;

	      h->hw_flags |= ModuleShortCount;

	      if (short_count == 1)
	      {
		caps_interval(2 * SHORT_INTERVAL, short_catcher);
	      }
	      return 0;
	    }
	    h->hw_state = 5;
	    
    case 5: if (!what) return 0;          /* switch down after pick          */

	    if (i->pw_pack_ordered > 0)
	    {
	      sprintf(text, "%04d09%03d%3dP",
		      h->hw_controller, h->hw_mod_address, i->pw_pack_picked);
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state   = 9;
	      short_count  += 1;

	      h->hw_flags  |= ModuleShortCount;

	      if (short_count == 1)
	      {
		caps_interval(2 * SHORT_INTERVAL, short_catcher);
	      }
	      return 0;
	    }
	    h->hw_state = 6;
	    
    case 6: if (!what) return 0;          /* switch down after pick          */

	    if (i->pw_ordered > 0)
	    {
	      sprintf(text, "%04d09%03d%4d",
		      h->hw_controller, h->hw_mod_address, i->pw_picked);
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state   = 10;
	      short_count  += 1;

	      h->hw_flags |= ModuleShortCount;

	      if (short_count == 1)
	      {
		caps_interval(2 * SHORT_INTERVAL, short_catcher);
	      }
	      return 0;
	    }
	    h->hw_state = 7;
	    
    case 7: if (!what) return 0;          /* short confirmed                 */
	    
	    z->zt_lines  -= 1;            /* mark now picked                 */
	    b->bay_picks -= 1;            /* mark now picked                 */

	    i->pw_flags |= BinPicked;
	    h->hw_flags |= ModulePicked;

	    if (b->bay_picks <= 0) check_bl_off(b);

	    sprintf(text, "%04d09%03d____",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
	    }
	    sprintf(text, "%04d10%03dF",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 10, h);
	    
	    h->hw_state = 4;
	    return 0;

    case 8: if (what) return 0;           /* switch up on short              */
	    
	    h->hw_flags &= ~ModuleShortCount;
	    short_count -= 1;

	    sprintf(text, "%04d09%03d%3dC",
		    h->hw_controller, h->hw_mod_address, i->pw_case_picked);
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_state = 5;

	    return 0;
		 
    case 9: if (what) return 0;           /* switch up on short              */
	    
	    h->hw_flags &= ~ModuleShortCount;
	    short_count -= 1;

	    sprintf(text, "%04d09%03d%3dP",
		    h->hw_controller, h->hw_mod_address, i->pw_pack_picked);
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_state = 6;

	    return 0;
	    
    case 10: if (what) return 0;          /* switch up on short              */
	    
	    h->hw_flags &= ~ModuleShortCount;
	    short_count -= 1;

	    sprintf(text, "%04d09%03d%4d",
		    h->hw_controller, h->hw_mod_address, i->pw_picked);
	    Ac_Write(j, text, 13, h);
	    
	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	    }
	    h->hw_state = 7;
	    return 0;
    
    default: return 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Process Pick Module Switches For Matrix Picking
 *-------------------------------------------------------------------------*
 *         up             up
 *     +------+        +-------+
 *     V      |        V       |
 * [State 0] -+-> [State 21] --+--> [State 22] ---> [State 23] ---> restart.
 *           down            down              up  
 *        show units     start shorting   if any change in picked,
 *                                          show units and go to 21.
 *                                        if any more picks, show next pick.
 *                                        otherwise, show ____ and bl off.
 *-------------------------------------------------------------------------*/
process_pm4_matrix(z, b, h, what)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
register long what;
{
  register struct pw_item   *i;
  register long j, k;
  char text[20];
  
#ifdef DEBUG
  fprintf(DF,
  "process_pm4_matrix(): Zone=%d Bay=%d Mod=%d State=%d Swt=%d pw=%d\n",
  z->zt_zone, b->bay_number, h->hw_mod, h->hw_state, what, h->hw_current);
  fflush(DF);
#endif
  
  j = b->bay_port - 1;
  
  i = &pw[h->hw_current - 1];             /* point to pick                   */
  
  switch (h->hw_state)
  {
    case 0: if (!what) return 0;          /* pick case or thousands          */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (i->pw_case_ordered > 0)
	    {
	      if (i->pw_case == 1000)
	      {
		sprintf(text, "%04d09%03d%2.2s%1dT",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_case_picked);
	      }
	      else
	      {
		sprintf(text, "%04d09%03d%2.2s%1dC",
			h->hw_controller, h->hw_mod_address,
		i->pw_display, i->pw_case_picked);
	      }
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 24;
	      return 0;
	    }

    case 21:if (!what) return 0;          /* pick packs or hundreds          */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (i->pw_pack_ordered > 0)
	    {
	      if (i->pw_pack == 100)
	      {
		sprintf(text, "%04d09%03d%2.2s%1dH",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_pack_picked);
	      }
	      else
	      {
		sprintf(text, "%04d09%03d%2.2s%1dP",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_pack_picked);
	      }
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 26;
	      return 0;
	    }
	    
    case 22:if (!what) return 0;          /* pick units                      */

	    if (!(i->pw_flags & BinHasPick)) return 0;

#ifdef RED
	    if (!i->pw_case_ordered && !i->pw_pack_ordered)
	    {
	      if (sp->sp_blink_over && i->pw_ordered > sp->sp_blink_over)
	      {
		sprintf(text, "%04d10%03dR",    /* show red light            */
			h->hw_controller, h->hw_mod_address);

		Ac_Write(j, text, 10, 0);
	      }
	    }
#endif
	    if (i->pw_ordered > 0)
	    {
	      sprintf(text, "%04d09%03d%2.2s%2d",
		      h->hw_controller, h->hw_mod_address,
		      i->pw_display, i->pw_picked);

	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 28;
	      return 0;
	    }
    case 30:if (!what) return 0;          /* units picks are done            */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    z->zt_lines  -= 1;            /* mark now picked                 */
	    b->bay_picks -= 1;            /* mark now picked                 */

	    i->pw_flags |= BinPicked;

	    if (b->bay_picks <= 0) check_bl_off(b);

	    h->hw_current++;
	    i++;
	    
	    while (h->hw_current < h->hw_first + b->bay_width)
	    {
	      if (i->pw_flags & BinHasPick)
	      {
		h->hw_state = 0;
		process_pm4_matrix(z, b, h, 2);
		return 0;
	      }
	      h->hw_current += 1;
	      i++;
	    }
	    h->hw_flags |= ModulePicked;

	    sprintf(text, "%04d09%03d____",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 13, h);

	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
	    }
	    sprintf(text, "%04d10%03dF",
	    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 10, h);

	    h->hw_state   = 31;
	    h->hw_current = h->hw_first;
	    return 0;
	    
    case 31:if (!what) return 0;          /* recycle all picks               */

	    if (h->hw_current != h->hw_first) return 0;
	    
	    while (h->hw_current < h->hw_first + b->bay_width)
	    {
	      if (i->pw_flags & BinHasPick)
	      {
		i->pw_flags &= ~BinPicked;

		z->zt_lines  += 1;
		b->bay_picks += 1;

		if (b->bay_picks == 1) check_bl_on(b);
	      }
	      h->hw_current++;
	      i++;
	    }
	    h->hw_current = h->hw_first;
	    h->hw_flags  &= ~ModulePicked;
    
	    i = &pw[h->hw_current - 1];
	    
	    while (h->hw_current < h->hw_first + b->bay_width)
	    {
	      if (i->pw_flags & BinHasPick) break;
	      h->hw_current++;
	      i++;
	    }
	    h->hw_state = 0;
	    process_pm4_matrix(z, b, h, 2);
	    return 0;

    case 24:if (!what) return 0;          /* case picks started              */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	   
	    if (short_count == 1)
	    {
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 25;
	    return 0;

    case 25:if (what) return 0;           /* case picks done                 */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (h->hw_flags & ModuleFlag) /* was a short tick                */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 0;
	      process_pm4_matrix(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 21;
	    process_pm4_matrix(z, b, h, 2);
	    return 0;

    case 26:if (!what) return 0;          /* pack picks are started          */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	    
	    if (short_count == 1)
	    {
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 27;
	    return 0;

    case 27:if (what) return 0;           /* pack picks are done             */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    short_count -= 1;
	    h->hw_flags &= ~ModuleShortCount;
	    h->hw_save = 0;
	     
	    if (h->hw_flags & ModuleFlag) /* was a short tick                */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 21;
	      process_pm4_matrix(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 22;
	    process_pm4_matrix(z, b, h, 2);
	    return 0;
	     
    case 28:if (!what) return 0;          /* unit picks started              */
    
	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	    
	    if (short_count == 1)
	    {
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 29;
	    return 0;

    case 29:if (what) return 0;           /* unit picks are done             */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    short_count -= 1;
	    h->hw_flags &= ~ModuleShortCount;
	    h->hw_save = 0;
	    
	    if (h->hw_flags & ModuleFlag) /* was a short tick                */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 22;
	      process_pm4_matrix(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 30;
	    process_pm4_matrix(z, b, h, 2);
	    return 0;
	     
    default: return 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Process Large Pick Module
 *-------------------------------------------------------------------------*/

process_pm6_matrix(z, b, h, what)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
register long what;
{
  register struct pw_item   *i;
  register long j, k;
  char text[20];
  
#ifdef DEBUG
  fprintf(DF,
  "process_pm6_matrix(): Zone=%d Bay=%d Mod=%d State=%d Swt=%d pw=%d\n",
  z->zt_zone, b->bay_number, h->hw_mod, h->hw_state, what, h->hw_current);
  fflush(DF);
#endif
  
  j = b->bay_port - 1;
  
  i = &pw[h->hw_current - 1];             /* point to pick                   */
  
  switch (h->hw_state)
  {
    case 0: if (!what) return 0;          /* pick case or thousands          */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (i->pw_case_ordered > 0)
	    {
	      if (i->pw_case == 1000)
	      {
		sprintf(text, "%04d09%03d%4.4s%1dT",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_case_picked);
	      }
	      else
	      {
		sprintf(text, "%04d09%03d%4.4s%1dC",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_case_picked);
	      }
	      Ac_Write(j, text, 15, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 6);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 24;
	      return 0;
	    }

    case 21:if (!what) return 0;          /* pick packs or hundreds          */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (i->pw_pack_ordered > 0)
	    {
	      if (i->pw_pack == 100)
	      {
		sprintf(text, "%04d09%03d%4.4s%1dH",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_pack_picked);
	      }
	      else
	      {
		sprintf(text, "%04d09%03d%4.4s%1dP",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_pack_picked);
	      }
	      Ac_Write(j, text, 15, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 6);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 26;
	      return 0;
	    }
	    
    case 22:if (!what) return 0;          /* pick units                      */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (i->pw_ordered > 0)
	    {
	      sprintf(text, "%04d09%03d%4.4s%2d",
		      h->hw_controller, h->hw_mod_address,
		      i->pw_display, i->pw_picked);

	      Ac_Write(j, text, 15, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 6);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 28;
	      return 0;
	    }
    case 30:if (!what) return 0;          /* units picks are done            */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    z->zt_lines  -= 1;            /* mark now picked                 */
	    b->bay_picks -= 1;            /* mark now picked                 */

	    i->pw_flags |= BinPicked;

	    if (b->bay_picks <= 0) check_bl_off(b);

	    h->hw_current++;
	    i++;
	    
	    while (h->hw_current < h->hw_first + b->bay_width)
	    {
	      if (i->pw_flags & BinHasPick)
	      {
		 h->hw_state = 0;
		 process_pm6_matrix(z, b, h, 2);
		 return 0;
	      }
	      h->hw_current += 1;
	      i++;
	    }
	    h->hw_flags |= ModulePicked;

	    sprintf(text, "%04d09%03d______",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 15, h);

	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 6);
	    }
	    sprintf(text, "%04d10%03dF",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 10, h);

	    h->hw_state   = 31;
	    h->hw_current = h->hw_first;
	    return 0;
	    
    case 31:if (!what) return 0;          /* recycle all picks               */

	    if (h->hw_current != h->hw_first) return 0;
	    
	    while (h->hw_current < h->hw_first + b->bay_width)
	    {
	      if (i->pw_flags & BinHasPick)
	      {
		i->pw_flags &= ~BinPicked;

		z->zt_lines  += 1;
		b->bay_picks += 1;

		if (b->bay_picks == 1) check_bl_on(b);
	      }
	      h->hw_current++;
	      i++;
	    }
	    h->hw_current = h->hw_first;
	    h->hw_flags  &= ~ModulePicked;

	    i = &pw[h->hw_current - 1];
	    
	    while (h->hw_current < h->hw_first + b->bay_width)
	    {
	      if (i->pw_flags & BinHasPick) break;
	      h->hw_current++;
	      i++;
	    }
	    h->hw_state = 0;
	    process_pm6_matrix(z, b, h, 2);
	    return 0;

    case 24:if (!what) return 0;          /* case picks started              */

	    if (!(i->pw_flags & BinHasPick)) return 0;
 
	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	   
	    if (short_count == 1)
	    {
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 25;
	    return 0;

    case 25:if (what) return 0;           /* case picks done                 */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (h->hw_flags & ModuleFlag) /* was a short tick                    */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 0;
	      process_pm6_matrix(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 21;
	    process_pm6_matrix(z, b, h, 2);
	    return 0;

    case 26:if (!what) return 0;          /* pack picks are started          */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	    
	    if (short_count == 1)
	    {
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 27;
	    return 0;

    case 27:if (what) return 0;           /* pack picks are done             */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    short_count -= 1;
	    h->hw_flags &= ~ModuleShortCount;
	    h->hw_save = 0;
	    
	    if (h->hw_flags & ModuleFlag) /* was a short tick                */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 21;
	      process_pm6_matrix(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 22;
	    process_pm6_matrix(z, b, h, 2);
	    return 0;
	     
    case 28:if (!what) return 0;          /* unit picks started              */
    
	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	    
	    if (short_count == 1)
	    {
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 29;
	    return 0;

    case 29:if (what) return 0;           /* unit picks are done             */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    short_count -= 1;
	    h->hw_flags &= ~ModuleShortCount;
	    h->hw_save = 0;
	    
	    if (h->hw_flags & ModuleFlag) /* was a short tick                */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 22;
	      process_pm6_matrix(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 30;
	    process_pm6_matrix(z, b, h, 2);
	    return 0;
	     
    default: return 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Process Light Bar
 *--------------------------------------------------------------------------*/
process_pm4_bar(z, b, h, what)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
register long what;
{
  register struct pw_item   *i;
  register long j;
  char text[20];
  
#ifdef DEBUG
  fprintf(DF,
  "process_pm4_bar(): Zone=%d Bay=%d Mod=%d State=%d Swt=%d pw=%d\n",
  z->zt_zone, b->bay_number, h->hw_mod, h->hw_state, what, h->hw_current);
  fflush(DF);
#endif

  j = b->bay_port - 1;
  
  i = &pw[h->hw_current - 1];             /* point to pick                   */
  
  switch (h->hw_state)
  {
    case 0: if (!what) return 0;          /* pick case or thousands          */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (i->pw_case_ordered > 0)
	    {
	      if (i->pw_case == 1000)
	      {
		sprintf(text, "%04d09%03d%2.2s%1dT",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_case_picked);
	      }
	      else
	      {
		sprintf(text, "%04d09%03d%2.2s%1dC",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_case_picked);
	      }
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 24;
	      return 0;
	    }

    case 21:if (!what) return 0;          /* pick packs or hundreds          */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (i->pw_pack_ordered > 0)
	    {
	      if (i->pw_pack == 100)
	      {
		sprintf(text, "%04d09%03d%2.2s%1dH",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_pack_picked);
	      }
	      else
	      {
		sprintf(text, "%04d09%03d%2.2s%1dP",
			h->hw_controller, h->hw_mod_address,
			i->pw_display, i->pw_pack_picked);
	      }
	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 26;
	      return 0;
	    }
	    
    case 22:if (!what) return 0;          /* pick units                      */

	    if (!(i->pw_flags & BinHasPick)) return 0;

#ifdef RED
	    if (!i->pw_case_ordered && !i->pw_pack_ordered)
	    {
	      if (sp->sp_blink_over && i->pw_ordered > sp->sp_blink_over)
	      {
		sprintf(text, "%04d10%03dR", /* show red light              */
			h->hw_controller, h->hw_mod_address);

		Ac_Write(j, text, 10, 0);
	      }
	    }
#endif
	    if (i->pw_ordered > 0)
	    {
	      sprintf(text, "%04d09%03d%2.2s%2d",
		      h->hw_controller, h->hw_mod_address,
		      i->pw_display, i->pw_picked);

	      Ac_Write(j, text, 13, h);

	      if (sp->sp_pickline_view == 'y')
	      {
		memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
		       pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
	      }
	      h->hw_state = 28;
	      return 0;
	    }
    case 30:if (!what) return 0;          /* units picks are done            */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    z->zt_lines  -= 1;            /* mark now picked                 */
	    b->bay_picks -= 1;            /* mark now picked                 */

	    i->pw_flags |= BinPicked;
	    h->hw_flags |= ModulePicked;

	    if (b->bay_picks <= 0) check_bl_off(b);

	    if (b->bay_picks <= 0) check_bl_off(b);

	    h->hw_current += b->bay_width;
	    
	    while (h->hw_current <= b->bay_prod_last)
	    {
	      i = &pw[h->hw_current - 1];
	      
	      if (i->pw_flags & BinHasPick)
	      {
		 h->hw_state = 0;
		 process_pm4_bar(z, b, h, 2);
		 return 0;
	      }
	      h->hw_current += b->bay_width;
	    }
	    h->hw_flags |= ModulePicked;
	    
	    sprintf(text, "%04d09%03d____",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 13, h);

	    if (sp->sp_pickline_view == 'y')
	    {
	      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
	    }
	    sprintf(text, "%04d10%03dF",
		    h->hw_controller, h->hw_mod_address);
	    Ac_Write(j, text, 10, h);
	    h->hw_state = 31;
	    h->hw_current = h->hw_first;
	    return 0;
	    
    case 31:if (!what) return 0;          /* recycle all picks               */

	    if (h->hw_current != h->hw_first) return 0;
	    
	    while (h->hw_current <= b->bay_prod_last)
	    {
	      i = &pw[h->hw_current - 1];
	      
	      if (i->pw_flags & BinHasPick)
	      {
		i->pw_flags &= ~BinPicked;

		z->zt_lines  += 1;
		b->bay_picks += 1;

		if (b->bay_picks == 1) check_bl_on(b);
	      }
	      h->hw_current += b->bay_width;
	    }
	    h->hw_current = h->hw_first;
	    h->hw_flags  &= ~ModulePicked;

	    while (h->hw_current <= b->bay_prod_last)
	    {
	      i = &pw[h->hw_current - 1];
	      if (i->pw_flags & BinHasPick) break;
	      h->hw_current += b->bay_width;
	    }
	    h->hw_state = 0;
	    process_pm4_bar(z, b, h, 2);
	    return 0;

    case 24:if (!what) return 0;          /* case picks started              */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	   
	    if (short_count == 1)
	    {
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 25;
	    return 0;

    case 25:if (what) return 0;           /* case picks done                 */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    if (h->hw_flags & ModuleFlag) /* was a short tick                */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 0;
	      process_pm4_bar(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 21;
	    process_pm4_bar(z, b, h, 2);
	    return 0;

    case 26:if (!what) return 0;          /* pack picks are started          */
 
	    if (!(i->pw_flags & BinHasPick)) return 0;

	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	    
	    if (short_count == 1)
	    {
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 27;
	    return 0;

    case 27:if (what) return 0;           /* pack picks are done             */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    short_count -= 1;
	    h->hw_flags &= ~ModuleShortCount;
	    h->hw_save = 0;
	    
	    if (h->hw_flags & ModuleFlag) /* was a short tick                */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 21;
	      process_pm4_bar(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 22;
	    process_pm4_bar(z, b, h, 2);
	    return 0;
	     
    case 28:if (!what) return 0;          /* unit picks started              */
    
	    h->hw_flags |= ModuleShortCount;
	    short_count += 1;
	    h->hw_save   = 0;
	    
	    if (short_count == 1)
	    { 
	      caps_interval(SHORT_INTERVAL, short_catcher);
	    }
	    h->hw_state = 29;
	    return 0;

    case 29:if (what) return 0;           /* unit picks are done             */

	    if (!(i->pw_flags & BinHasPick)) return 0;

	    short_count -= 1;
	    h->hw_flags &= ~ModuleShortCount;
	    h->hw_save = 0;
	    
	    if (h->hw_flags & ModuleFlag) /* was a short tick                */
	    {
	      h->hw_flags &= ~ModuleFlag;
	      h->hw_state  = 22;
	      process_pm4_bar(z, b, h, 2);
	      return 0;
	    }
	    h->hw_state = 30;
	    process_pm4_bar(z, b, h, 2);
	    return 0;

    default: return 0;
  }
}
/*------------------------------------------------------------------------*
 *  Process Box Full
 *------------------------------------------------------------------------*/
process_box_full(z, b, h, what)
register struct zone_item *z;
register struct bay_item  *b;
register struct hw_item   *h;
register long what;
{
  register struct bay_item *bn;
  register struct hw_item  *hn;
  register long k = 0L;
  TBoxOrderMessage buf;
  char text[80];
  char box_str[BoxNoLength+1];
  long box_num = 0L;
  register struct oi_item *o;

#ifdef DEBUG
  fprintf(DF, "process_box_full(): Zone=%d Bay=%d Mod=%d State=%d Swt=%d\n",
          z->zt_zone, b->bay_number, h->hw_mod, h->hw_state, what);
  fflush(DF);
#endif
  
  if (what) return 0;                     /* ignore switch down              */
  

  if (z->zt_status != ZS_UNDERWAY && z->zt_status != ZS_EARLY) return 0;

  /* Do not allow box full to happen if "EXIT TOTE" on ZC */
  k = z->zt_first_bay;

  while (k > 0L)
        {
          bn = &bay[k - 1L];
          k  = bn->bay_next;

          if (!bn->bay_zc)
             {
               continue;
             }

          hn = &hw[bn->bay_zc - 1];
          break;
        }  /* end of while loop to find a zone controller */

  if (hn->hw_type == ZC2)
     {
       if (sp->sp_pickline_view == 'y')
          {
             if (zcv[hn->hw_mod - 1].hw_display[0] == 'E')
                {
#ifdef DEBUG
                   fprintf(DF, "process_box_full:  hw_display[0] = %c\n",
                           zcv[hn->hw_mod - 1].hw_display[0]);
                   fflush(DF);
#endif
                   return 0;
                }
          }
     }

#ifdef TANDY
  o = &oc->oi_tab[z->zt_order - 1];      /* point to order queue      */
  if (!(z->zt_flags & DemandFeed))
  {
	if (o->oi_flags & NEED_BOX_SCAN) 
	{
  		sprintf(text, "NEED  BOX  SCAN");   
  		zone_message(z, text, 0, 0);
		return(0);
	}
   }
	
  z->zt_flags |= BoxFull;
  memset(box_str, 0x0, BoxNoLength+1);
  memcpy(box_str, o->oi_box, BoxNoLength);
  box_str[BoxNoLength] = '\0';
  box_num = atol(box_str); 
  sprintf(text, "CLOSE BOX %-6.6s", &box_str[2]);
  zone_message(z, text, 0, 0);
#endif
  
  buf.m_pickline = z->zt_pl;
  buf.m_order    = z->zt_on;
  buf.m_box      = box_num;
  
  zone_clear(z->zt_zone, 2);              /* return picks so far             */
  
  message_put(0, BoxCloseEvent, &buf, sizeof(TBoxOrderMessage));
  return 0;
}
/*------------------------------------------------------------------------*
 *  Short Counting Routine
 *------------------------------------------------------------------------*/
long short_catcher()
{
  register struct hw_item  *h;
  register struct pw_item  *i;
  register struct bay_item *b;
  register long k, size;
  static long blink = 0;
  char text[20];
  
  if (short_count <= 0) return 0;

  if (sim_flag > 0)
  {
    caps_interval(1, short_catcher);      /* wait 100ms before shorting      */
    return 0;
  }
  if (blink > 0) blink--;
  else blink = 4;
  
  for (k = 0, h = hw; k < coh->co_light_cnt; k++, h++)
  {
    if (h->hw_type == PM6) size = 4;
    else                   size = 2;
    
    b = &bay[h->hw_bay - 1];

    i = &pw[h->hw_current - 1];

#ifndef NOBLINK
    if (h->hw_flags & ModuleBlink)
    {
      if (blink == 2)
      {
	sprintf(text, "%04d09%03d%4d",
		 h->hw_controller, h->hw_mod_address, i->pw_picked);
	Ac_Write(b->bay_port - 1, text, 13, h);
      }
      else if (blink == 4)
      {
	sprintf(text, "%04d09%03d    ",
		h->hw_controller, h->hw_mod_address);
	Ac_Write(b->bay_port - 1, text, 13, h);
      }
      continue;
    }
#endif
    
    if (!(h->hw_flags & ModuleShortCount)) continue;

#ifdef DEBUG
  fprintf(DF, "shorts: count=%d mod=%d pw=%d state=%d save=%d\n",
    short_count, h->hw_mod, h->hw_current, h->hw_state, h->hw_save);
  fflush(DF);
#endif

    switch (h->hw_state)
    {
      case 14:
      case 10: i->pw_picked += 1;
	       if (i->pw_picked > i->pw_ordered) i->pw_picked = 0;
    
	       sprintf(text, "%04d09%03d%4d",
		       h->hw_controller, h->hw_mod_address, i->pw_picked);
	       break;

      case 8:  i->pw_case_picked += 1;
	       if (i->pw_case_picked > i->pw_case_ordered)
	       {
		 i->pw_case_picked = 0;
	       }
	       sprintf(text, "%04d09%03d%3dC",
		       h->hw_controller, h->hw_mod_address, i->pw_case_picked);
	       break;

      case 9:  i->pw_pack_picked += 1;
	       if (i->pw_pack_picked > i->pw_pack_ordered)
	       {
		 i->pw_pack_picked = 0;
	       }
	       sprintf(text, "%04d09%03d%3dP",
		       h->hw_controller, h->hw_mod_address, i->pw_pack_picked);
	       break;
	       
      case 11: i->pw_case_picked += 1;
	       if (i->pw_case_picked > i->pw_case_ordered)
	       {
		 i->pw_case_picked = 0;
	       }
	       if (i->pw_case == 1000)
	       {
		 sprintf(text, "%04d09%03d%3dT",
			 h->hw_controller, h->hw_mod_address, 
			 i->pw_case_picked);
	       }
	       else if (i->pw_case_picked < 10)
	       {
		 sprintf(text, "%04d09%03d%3dC",
			 h->hw_controller, h->hw_mod_address, 
			 i->pw_case_picked);
	       }
	       else
	       {
		 sprintf(text, "%04d09%03d%4d",
			 h->hw_controller, h->hw_mod_address, 
			 i->pw_case_picked);
	       }
	       break;

      case 13: i->pw_pack_picked += 1;
	       if (i->pw_pack_picked > i->pw_pack_ordered)
	       {
		 i->pw_pack_picked = 0;
	       }
	       if (i->pw_pack == 100)
	       {
		 sprintf(text, "%04d09%03d%3dH",
			 h->hw_controller, h->hw_mod_address, 
			 i->pw_pack_picked);
	       }
	       else if (i->pw_pack_picked < 10)
	       {
		 sprintf(text, "%04d09%03d%3dP",
			 h->hw_controller, h->hw_mod_address,
			 i->pw_pack_picked);
	       }
	       else
	       {
		 sprintf(text, "%04d09%03d%4d",
			 h->hw_controller, h->hw_mod_address,
			 i->pw_pack_picked);
	       }
	       break;
	    
      case 25: if (!(h->hw_flags & ModuleFlag))
	       {
		 if (h->hw_save < 2)
		 {
		   h->hw_save += 1;
		   continue;
		 }
		 h->hw_flags |= ModuleFlag;
		 h->hw_save   = 0;
	       }
	       i->pw_case_picked += 1;
	       
	       if (i->pw_case_picked > i->pw_case_ordered)
	       {
		 i->pw_case_picked = 0;
	       }
	       if (i->pw_case == 1000)
	       {
		 sprintf(text, "%04d09%03d%*.*s%1dT",
			 h->hw_controller, h->hw_mod_address,
			 size, size, i->pw_display, i->pw_case_picked);
	       }
	       else
	       {
		 sprintf(text, "%04d09%03d%*.*s%1dC",
			 h->hw_controller, h->hw_mod_address,
			 size, size, i->pw_display, i->pw_case_picked);
	       }
	       break;
	       
      case 27: if (!(h->hw_flags & ModuleFlag))
	       {
		 if (h->hw_save < 2)
		 {
		   h->hw_save += 1;
		    continue;
		 }
		 h->hw_flags |= ModuleFlag;
		 h->hw_save   = 0;
	       }
	       i->pw_pack_picked += 1;
	       if (i->pw_pack_picked > i->pw_pack_ordered)
	       {
		 i->pw_pack_picked = 0;
	       }
	       if (i->pw_pack == 100)
	       {
		 sprintf(text, "%04d09%03d%*.*s%1dH",
			 h->hw_controller, h->hw_mod_address,
			 size, size, i->pw_display, i->pw_pack_picked);
	       }
	       else
	       {
		 sprintf(text, "%04d09%03d%*.*s%1dP",
			h->hw_controller, h->hw_mod_address,
			size, size, i->pw_display, i->pw_pack_picked);
	       }
	       break;
      case 21:
      case 29: if (!(h->hw_flags & ModuleFlag))
	       {  
		 if (h->hw_save < 2)
		 {
		   h->hw_save += 1;
		   continue;
		 }
		 h->hw_flags |= ModuleFlag;
		 h->hw_save   = 0;
	       }
	       i->pw_picked += 1;
	       if (i->pw_picked > i->pw_ordered) i->pw_picked = 0;
	       
	       sprintf(text, "%04d09%03d%*.*s%2d",
		       h->hw_controller, h->hw_mod_address,
		       size, size, i->pw_display, i->pw_picked);

	       break;
    }
    Ac_Write(b->bay_port - 1, text, strlen(text), h);

    if (sp->sp_pickline_view == 'y')
    {
      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, size + 2);
	     pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
    }
  }
  if (short_count > 0)
  {
    caps_interval(SHORT_INTERVAL, short_catcher);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Bay Lamp On Status
 *-------------------------------------------------------------------------*/
check_bl_on(b)
register struct bay_item *b;
{
  register struct hw_item *h;
  char text[20];
  
  if (b->bay_picks != 1) return 0;
  if (b->bay_bl)
  {
    h = &hw[b->bay_bl - 1];

    sprintf(text, "%04d09%03d01", h->hw_controller, h->hw_mod_address);
    Ac_Write(b->bay_port - 1, text, 11, 0);

    if (sp->sp_pickline_view == 'y')
    {
      blv[h->hw_mod - 1].hw_display[0] = '1';
    }
  }
  while (b->bay_mbl)
  {
    b = &bay[b->bay_mbl - 1];
    b->bay_picks += 1;
    if (b->bay_picks > 1) return 0;
    if (!b->bay_bl) return 0;
    h = &hw[b->bay_bl - 1];

    sprintf(text, "%04d09%03d01", h->hw_controller, h->hw_mod_address);
    Ac_Write(b->bay_port - 1, text, 11, 0);

    if (sp->sp_pickline_view == 'y')
    {
      blv[h->hw_mod - 1].hw_display[0] = '1';
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Bay Lamp Off Status
 *-------------------------------------------------------------------------*/
check_bl_off(b)
register struct bay_item *b;
{
  register struct hw_item *h;
  char text[20];
  
  if (b->bay_bl)
  {
    h = &hw[b->bay_bl - 1];

    sprintf(text, "%04d10%03dB", h->hw_controller, h->hw_mod_address);
    Ac_Write(b->bay_port - 1, text, 10, 0);

    if (sp->sp_pickline_view == 'y')
    {
      blv[h->hw_mod - 1].hw_display[0] = '0';
    }
  }
  while (b->bay_mbl)
  {
    b = &bay[b->bay_mbl - 1];
    if (b->bay_picks > 0) b->bay_picks -= 1;
    if (b->bay_picks) return 0;
    if (!b->bay_bl) return 0;
    h = &hw[b->bay_bl - 1];

    sprintf(text, "%04d10%03dB", h->hw_controller, h->hw_mod_address);
    Ac_Write(b->bay_port - 1, text, 10, 0);

    if (sp->sp_pickline_view == 'y')
    {
      blv[h->hw_mod - 1].hw_display[0] = '0';
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 * Pickline Enable              - F052698 - add enable scanners
 *-------------------------------------------------------------------------*/
pickline_enable(n)
register long n;
{
  register struct zone_item *z;
  register struct bay_item *b;
  register struct hw_item *h;
  register long k;
  char text[32];
  
  if (!ports_open) return 0;
  
#ifdef DEBUG
  fprintf(DF, "pickline_enable(%d)\n", n);
  fflush(DF);
#endif

  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (n && n != z->zt_pl) continue;
    if (z->zt_flags & IsOffline) continue;
    zone_enable(z);
  }
  for (k = 0, h = hw; k < coh->co_light_cnt; k++, h++)
  {
#ifdef TCBL_ALC
// inititialize the state to 0 in order to redisplay the color of tcbl
// from tbcl-d.c

    if (h->hw_type == TB) 
	h->hw_state = 0;                     
#endif
    if (h->hw_type != IO) continue;                     
    if (!(h->hw_bay)) continue;
    b = &bay[h->hw_bay - 1];
    if (!(b->bay_zone)) continue;
    z = &zone[b->bay_zone - 1];
    if (n && n != z->zt_pl) continue;           /* scanner in pickline                           */
    
    h->hw_flags &= ~SwitchesDisabled;

    sprintf(text, "%04d10%03dE", h->hw_controller, h->hw_mod_address);
    Ac_Write(b->bay_port - 1, text, 10, 0);

  }
  return 0;
}


/*-------------------------------------------------------------------------*
 * Zone Enable
 *-------------------------------------------------------------------------*/
zone_enable(z)
register struct zone_item *z;
{
  register struct bay_item *b;
  register struct bay_item *bn;
  register struct hw_item *h;
  register long j, k, m;
  char text[MAX_MSG_SIZE];
  register long hw_cnt    = 0L,
                first_mod = 0L,
                m_first   = 0L,
                m_mbl     = 0L,
                cmd_lgth  = 0L,
                zc_mod    = 0L,
                bf_mod    = 0L,
                bl_mod    = 0L;

#ifdef DEBUG
  fprintf(DF, "zone_enable(): Zone=%d\n", z->zt_zone);
  fflush(DF);
#endif

  if (!(z->zt_flags & IsTotalFunction)) return 0;

  z->zt_flags  &= ~SwitchesDisabled;
  k = z->zt_first_bay;
  
  while (k > 0)
  {
    m_mbl = -1L;
    b = &bay[k - 1];
    k = b->bay_next;
    
    if (!(b->bay_flags & IsTotalFunction)) continue;
    b->bay_flags &= ~SwitchesDisabled;

    j = b->bay_port - 1;
      
    if (b->bay_zc)                        /* enable zone controller          */
    {
      h = &hw[b->bay_zc - 1];

      h->hw_flags &= ~SwitchesDisabled;

      zc_mod = h->hw_mod_address;
    }

    if (sp->sp_productivity == 'y')
       {
         if (!z->zt_picker)
            {
              if (b->bay_zc)
                 {
                   sprintf(text, "%04d10%03dE",
                                 h->hw_controller,
                                 zc_mod);
                   Ac_Write(j, text, 10, 0);
                 }

              continue;
            }
       }

    if (b->bay_bl)
       {
         h = &hw[b->bay_bl - 1];

         h->hw_flags &= ~SwitchesDisabled;

         bl_mod = h->hw_mod_address;
       }

    if (b->bay_mbl)
       {
         bn = &bay[b->bay_mbl - 1];

         m_mbl = bn->bay_bl - 1;
       }

    if (b->bay_bf && sp->sp_box_full == 'y')/* enable box full module        */
       {
         h = &hw[b->bay_bf - 1];

         h->hw_flags &= ~SwitchesDisabled;

         bf_mod = h->hw_mod_address;
       }
    for (m = b->bay_mod_first; m && m <= b->bay_mod_last; m++)
        {
          h = &hw[mh[m - 1].mh_ptr - 1];
    
          h->hw_flags &= ~SwitchesDisabled;

        }  /* end of for loop to set pick modules' hw_flags */

    /* Find first hw_mod_address in bay */
    m = mh[b->bay_mod_first - 1].mh_ptr - 1;

    first_mod = hw[m].hw_mod_address;

    if (b->bay_zc && zc_mod < first_mod)
       {
         first_mod = zc_mod;
         m         = b->bay_zc - 1;
       }

    if (b->bay_bl && bl_mod < first_mod)
       {
         first_mod = bl_mod;
         m         = b->bay_bl - 1;
       }

    if (b->bay_bf && bf_mod < first_mod)
       {
         first_mod = bf_mod;
         m         = b->bay_bf - 1;
       }

    hw_cnt = 0L;

    m_first = m;

    while (hw[m].hw_bay == b->bay_number)
          {
            hw_cnt++;
            m++;

            if (m_mbl >= 0L                                         &&
                hw[m_mbl].hw_mod_address == hw[m].hw_mod_address    &&
                hw[m_mbl].hw_controller  == hw[m - 1].hw_controller   )
               {  /* found a Master Bay Lamp in the module range */
                 hw_cnt++;
                 m++;
               }
          }  /* end of while loop to get device count in bay */

    m = 0L;
    while (hw_cnt > 0L)
          {
            m = m_first;

            if (hw_cnt > MAX_CMD_LGTH)
               {
                 cmd_lgth = MAX_CMD_LGTH;
               }
            else
               {
                 cmd_lgth = hw_cnt; 
               }

            /* Setup beginning part of message with first */
            /* hw_mod_address in the bay to start at      */
            memset(text, 0x0, MAX_MSG_SIZE);
            sprintf(text, "%04d10%03d",
                    hw[m].hw_controller,
                    hw[m].hw_mod_address);

            memset(&text[9], 'E', cmd_lgth);
            Ac_Write(j, text, cmd_lgth + 9, 0);

            hw_cnt  -= cmd_lgth;
            m_first += cmd_lgth;
          }  /* end of while loop to write enable messages */
  }  /* end of while loop for each bay */

  return 0;
}


/*-------------------------------------------------------------------------*
 * Pickline Disable                          - F052698 add disable scanners
 *-------------------------------------------------------------------------*/
pickline_disable(n)
register long n;
{
  register struct zone_item *z;
  register struct bay_item *b;
  register struct hw_item *h;
  register long k;
  char text[32];

  if (!ports_open) return 0;
  
#ifdef DEBUG
  fprintf(DF, "pickline_disable(%d):\n", n);
  fflush(DF);
#endif

  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (n && n != z->zt_pl) continue;
    if (z->zt_flags & IsOffline) continue;
    zone_disable(z,0);
  }
  for (k = 0, h = hw; k < coh->co_light_cnt; k++, h++)  /* F052698      */
  {
    if (h->hw_type != IO) continue;                     /* is a scanner */
    if (!(h->hw_bay)) continue;
    b = &bay[h->hw_bay - 1];
    if (!(b->bay_zone)) continue;
    z = &zone[b->bay_zone - 1];
    if (n && n != z->zt_pl) continue;           /* scanner in pickline  */
    
    h->hw_flags |= SwitchesDisabled;

    sprintf(text, "%04d10%03dD", h->hw_controller, h->hw_mod_address);
    Ac_Write(b->bay_port - 1, text, 10, 0);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 * Port Disable                 - F052698 disable scanners
 *-------------------------------------------------------------------------*/
port_disable(n)
register long n;
{
  register struct zone_item *z;
  register struct bay_item  *b;
  register struct hw_item *h;
  register long k;
  char text[32];

  if (!ports_open) return 0;
  
#ifdef DEBUG
  fprintf(DF, "port_disable(%d):\n", n);
  fflush(DF);
#endif

  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (z->zt_flags & IsOffline) continue;
    b = &bay[z->zt_first_bay - 1];
    if (b->bay_port != n) continue;
    zone_clear(z->zt_zone,1);
  }

  for (k = 0, h = hw; k < coh->co_light_cnt; k++, h++)  /* F052698    */
  {
    if (h->hw_type != IO) continue;                     /* is a scanner    */
    if (!(h->hw_bay)) continue;
    b = &bay[h->hw_bay - 1];
    if (b->bay_port != n) continue;                     /* scanner on port  */
    
    h->hw_flags |= SwitchesDisabled;

    sprintf(text, "%04d10%03dD", h->hw_controller, h->hw_mod_address);
    Ac_Write(b->bay_port - 1, text, 10, 0);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 * Zone Disable
 *-------------------------------------------------------------------------*/
zone_disable(z,flag)
register struct zone_item *z;
short flag;
{
  register struct bay_item *b;
  register struct bay_item *bn;
  register struct hw_item *h;
  register long j, k, m;
  char text[MAX_MSG_SIZE];
  register long hw_cnt    = 0L,
                first_mod = 0L,
                m_first   = 0L,
                m_mbl     = 0L,
                cmd_lgth  = 0L,
                io_m      = 0L,
                zc_mod    = 0L,
                bl_mod    = 0L,
                bf_mod    = 0L;

#ifdef DEBUG
  fprintf(DF, "zone_disable(%d)\n", z->zt_zone);
  fflush(DF);
#endif
  if (!(z->zt_flags & IsTotalFunction)) return 0;
  
  z->zt_flags |= SwitchesDisabled;
  k = z->zt_first_bay;

  while (k > 0)
  {
    m_mbl = -1L;
    b = &bay[k - 1];
    k = b->bay_next;

    if (!(b->bay_flags & IsTotalFunction)) continue;
    b->bay_flags |= SwitchesDisabled;

    j = b->bay_port - 1;
      
    if (b->bay_zc)             /* disable zone controller         */
    {
      h = &hw[b->bay_zc - 1];

      zc_mod = h->hw_mod_address;
    }
    if (b->bay_bl)
       {
         h = &hw[b->bay_bl - 1];

         bl_mod = h->hw_mod_address;
       }

    if (b->bay_mbl)
       {
         bn = &bay[b->bay_mbl - 1];

         m_mbl = bn->bay_bl - 1;
       }

    if (b->bay_bf)                        /* disable box full module         */
    {
      h = &hw[b->bay_bf - 1];

      h->hw_flags |= SwitchesDisabled;

      bf_mod = h->hw_mod_address;
    }
    for (m = b->bay_mod_first; m && m <= b->bay_mod_last; m++)
    {
      h = &hw[mh[m - 1].mh_ptr - 1];
    
      h->hw_flags |= SwitchesDisabled;
    }  /* end of for loop to set pick modules' hw_flags */

    /* Find first hw_mod_address in bay */
    m = mh[b->bay_mod_first - 1].mh_ptr - 1;

    first_mod = hw[m].hw_mod_address;

    if (b->bay_zc && zc_mod < first_mod)
       {
         first_mod = zc_mod;
         m         = b->bay_zc - 1;
       }

    if (b->bay_bl && bl_mod < first_mod)
       {
         first_mod = bl_mod;
         m         = b->bay_bl - 1;
       }

    if (b->bay_bf && bf_mod < first_mod)
       {
         first_mod = bf_mod;
         m         = b->bay_bf - 1;
       }

    hw_cnt = 0L;
    io_m   = -1L;

    m_first = m;

    while (hw[m].hw_bay == b->bay_number)
          {
            if (hw[m].hw_type == IO)
               {
                 io_m = m;
               }
            hw_cnt++;
            m++;

            if (m_mbl >= 0L                                         &&
                hw[m_mbl].hw_mod_address == hw[m].hw_mod_address    &&
                hw[m_mbl].hw_controller  == hw[m - 1].hw_controller   )
               {  /* found a Master Bay Lamp in the module range */
                 hw_cnt++;
                 m++;
               }
          }  /* end of while loop to get device count in bay */

    m = 0L;
    while (hw_cnt > 0L)
          {
            m = m_first;

            if (hw_cnt > MAX_CMD_LGTH)
               {
                 cmd_lgth = MAX_CMD_LGTH;
               }
            else
               {
                 cmd_lgth = hw_cnt; 
               }

            /* Setup beginning part of message with first */
            /* hw_mod_address in the bay to start at      */
            memset(text, 0x0, MAX_MSG_SIZE);
            sprintf(text, "%04d10%03d",
                    hw[m].hw_controller,
                    hw[m].hw_mod_address);

            memset(&text[9], 'D', cmd_lgth);
            Ac_Write(j, text, cmd_lgth + 9, 0);

            hw_cnt  -= cmd_lgth;
            m_first += cmd_lgth;
          }  /* end of while loop to write enable messages */

    if (!flag)
       {
         if (b->bay_zc)
            {
               h = &hw[b->bay_zc - 1];
               h->hw_flags |= SwitchesDisabled;
            }
       }
    else
       {
         if (b->bay_zc)
            {
               h = &hw[b->bay_zc - 1];
               memset(text, 0x0, MAX_MSG_SIZE);
               sprintf(text, "%04d10%03dE",
                             h->hw_controller,
                             h->hw_mod_address);
               Ac_Write(j, text, 10, 0);
            }
       }

    if (io_m > -1L)
       {
          memset(text, 0x0, MAX_MSG_SIZE);
          sprintf(text, "%04d10%03dE",
                        hw[io_m].hw_controller,
                        hw[io_m].hw_mod_address);
          Ac_Write(j, text, 10, 0);
       }

  }  /* end of while loop for each bay in the zone */
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Offline
 *-------------------------------------------------------------------------*/
zone_offline(zn)
register long zn;
{
  register struct zone_item *z;
  
  if (zn < 1 || zn > coh->co_zone_cnt) return 0;
  
  z = &zone[zn - 1];
  if (sp->sp_productivity == 'y')
     {
       if (!z->zt_picker)
          {
            zone_disable(z,1);
          }
       else
          {
            zone_disable(z,0);
          }
     }
  else
     {
       zone_disable(z,0);
     }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Online
 *-------------------------------------------------------------------------*/
zone_online(zn)
register long zn;
{
  register struct zone_item *z;
  
  if (zn < 1 || zn > coh->co_zone_cnt) return 0;
  
  z = &zone[zn - 1];
  zone_enable(z);
  return 0;
}
/*-------------------------------------------------------------------------*
 * Pickline Restoreplace                - F052698 add full function
 *-------------------------------------------------------------------------*/
pickline_restoreplace()
{
  register struct zone_item *z;
  register struct bay_item *b;
  register struct hw_item *h;
  register long j, k, m;
  char text[32];

  if (!ports_open) return 0;
  if (!zcv) return 0;                     /* pickline view is required       */

#ifdef DEBUG
  fprintf(DF, "pickline_restoreplace()\n");
  fflush(DF);
#endif

  for (k = 1, b = bay; k <= coh->co_bay_cnt; k++, b++)
  {
    if (!(b->bay_flags & IsTotalFunction)) continue;
    if (!b->bay_zone) continue;

    j = b->bay_port - 1;
      
    if (b->bay_bl)
    {
      h = &hw[b->bay_bl - 1];

      sprintf(text, "%04d09%03d0%c", h->hw_controller, h->hw_mod_address,
	blv[h->hw_mod - 1].hw_display[0]);
      Ac_Write(j, text, 11, 0);
    }
    if (b->bay_zc)
    {
      h = &hw[b->bay_zc - 1];

      if (h->hw_type == ZC2)
      {
	sprintf(text, "%04d09%03d%16.16s", 
	  h->hw_controller, h->hw_mod_address,
	  zcv[h->hw_mod - 1].hw_display);
	Ac_Write(j, text, 25, 0);
      }
      else if (h->hw_type == ZC)
      {  
	sprintf(text, "%04d09%03d%5.5s", 
	  h->hw_controller, h->hw_mod_address,
	  zcv[h->hw_mod - 1].hw_display);
	Ac_Write(j, text, 14, 0);
      }
    }
    for (m = b->bay_mod_first; m && m <= b->bay_mod_last; m++)
    {
      h = &hw[mh[m - 1].mh_ptr - 1];

      sprintf(text, "%04d09%03d%c%c%c%c",
	h->hw_controller, h->hw_mod_address,
	pmv[h->hw_mod - 1].hw_display[0] & 0x7f,
	pmv[h->hw_mod - 1].hw_display[1],
	pmv[h->hw_mod - 1].hw_display[2],
	pmv[h->hw_mod - 1].hw_display[3]);
      
      Ac_Write(j, text, 13, 0);

      if (pmv[h->hw_mod - 1].hw_display[0] < 0x7f)
      {
	sprintf(text, "%04d10%03dF", h->hw_controller, h->hw_mod_address);
	Ac_Write(j, text, 10, 0);
      }
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Send Message To All Ports
 *-------------------------------------------------------------------------*/
broadcast_message(text, fftext)
register char *text, *fftext;
{
  register long k;
  
  if (!ports_open) return 0;
  
#ifdef DEBUG
  fprintf(DF, "broadcast_message() text=[%s] fftest=[%d]\n", text, fftext);
  fflush(DF);
#endif

  for (k = 0; k < coh->co_port_cnt; k++)
  {
    if (!(po[k].po_flags & IsTotalFunction)) continue;
    port_message(k, text, fftext);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Enable Diagnostics
 *-------------------------------------------------------------------------*/
enable_diagnostics()
{
  register long k;
  
  for (k = 0; k < coh->co_port_cnt; k++)
  {
    if (!po[k].po_id) continue;
    if (!(po[k].po_flags & IsTotalFunction)) continue;
    if (po[k].po_disabled == 'y') continue;
    Ac_Write(k, "999915", 6, 0);
  }
}
/*-------------------------------------------------------------------------*
 *  Disable Diagnostics
 *-------------------------------------------------------------------------*/
disable_diagnostics()
{
  register long k;
  
  for (k = 0; k < coh->co_port_cnt; k++)
  {
    if (!po[k].po_id) continue;
    if (po[k].po_disabled == 'y') continue;
    Ac_Write(k, "999916", 6, 0);
  }
}
/*-------------------------------------------------------------------------*
 *  Send Message To Zone Controllers On One Port
 *-------------------------------------------------------------------------*/
port_message(n, text, fftext)
register long n;
register char *text, *fftext;
{
  register struct bay_item *b;
  register struct hw_item *h;
  register long k;
  char p[32];
  
  if (!ports_open) return 0;
  
  for (k = 0, b = bay; k < coh->co_bay_cnt; k++, b++)
  {
    if (b->bay_port != n + 1) continue;
    if (!b->bay_zc)           continue;
    
    h = &hw[b->bay_zc - 1];
    
    if (h->hw_type == ZC)
    {
      sprintf(p, "%04d09%03d%5.5s", 
	    h->hw_controller, h->hw_mod_address, fftext);

      Ac_Write(b->bay_port - 1, p, 14, 0);
    
      if (sp->sp_pickline_view == 'y')
      {
	memcpy(zcv[h->hw_mod - 1].hw_display, p + 9, 5);
      }
    }
    else if (h->hw_type == ZC2)
    {
      sprintf(p, "%04d09%03d%16.16s", 
	    h->hw_controller, h->hw_mod_address, text);

      Ac_Write(b->bay_port - 1, p, 25, 0);
    
      if (sp->sp_pickline_view == 'y')
      {
	memcpy(zcv[h->hw_mod - 1].hw_display, p + 9, 16);
      }
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open All Ports - At Initialization
 *-------------------------------------------------------------------------*/
open_ports()
{
  register long k;
  
  for (k = 0; k < coh->co_ports; k++)     /* open any ac port                */
  {
    if (po[k].po_flags & IsTotalFunction)
    {
      po[k].po_id = msgtask;              /* destination for messages        */
      
      if (sp->sp_total_function == 's')
      {
	strcpy(po[k].po_name, "/dev/null");
	port[k] = open("/dev/null", O_WRONLY);
      }
      else port[k] = ac_open(po[k].po_name);
     
      if (port[k] > 0) ports_open = 1;
      else port[k] = 0;
      
#ifdef DETAIL
      fprintf(DF, "open_ports() k=%d [%s] ports_open = %d\n",
      k, po[k].po_name, ports_open);
#endif
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Load Text File
 *-------------------------------------------------------------------------*/
load_text(text_name)
char *text_name;
{
  register long count, length;
  char buf[128], *p, *q;
  FILE *fd;
  
  fd = fopen(text_name, "r");
  if (fd == 0) return 0;

  memset(&am, 0, sizeof(am));
   
  q = (char *)&am;
  count = 0;
  
  while (count < 17)
  {
    if (!fgets(buf, 128, fd)) break;
    if (*buf == '#') continue;
    length = strlen(buf) - 1;
    buf[length] = 0;
     
    p = (char *)memchr(buf + 1, '"', length);
    if (p) *p = 0;
    strncpy(q, buf+1, 24);
    count++;
    q += 24;
  }
  fclose(fd);

#ifdef DEBUG
  Bdumpf(&am, sizeof(am), DF);
  fflush(DF);
#endif
   
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Configure Ports - Needs Configuration Information
 *-------------------------------------------------------------------------*/
config_ports()
{
  register struct bay_item *b;
  register struct hw_item *h;
  register long count, j, k, base;

#ifdef DEBUG
  fprintf(DF, "config_ports\n");
  fflush(DF);
#endif

  if (first_hw_on_ac) free(first_hw_on_ac);

  for (count = k = 0; k < coh->co_ports; k++)
  {
    first_ac_on_port[k] = count;
    if (po[k].po_flags & IsTotalFunction)
    {
      count += po[k].po_controllers;
    }
  }
  first_ac_on_port[k] = count;            /* total count after last port     */
  
  first_hw_on_ac = (short *)malloc(2 * count);
  memset(first_hw_on_ac, 0, 2 * count);
  
  for (k = 0, h = hw; k < coh->co_light_cnt; k++, h++)
  {
    if (!h->hw_bay) continue;
    if (h->hw_mod_address) continue;
    
    b = &bay[h->hw_bay - 1];
    if (!(b->bay_flags & IsTotalFunction)) continue;
    j = b->bay_port;
    if (j < 1 || j > coh->co_port_cnt) continue;

    base = first_ac_on_port[j - 1] + h->hw_controller - 1;
    first_hw_on_ac[base] = k;
  }
#ifdef DEBUG
  fprintf(DF, "First AC on Port\n");
  Bdumpf(first_ac_on_port, sizeof(first_ac_on_port), DF);
  fprintf(DF, "First HW on AC: count=%d\n", count);
  Bdumpf(first_hw_on_ac, 2 * count, DF);
  fflush(DF);
#endif
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Display Message
 *
 *  %m.n o  m = offset  n = length for a 7 digit order number.
 *  %m.n g  m = offset  n = length for a 4 character group.
 *  %m.n z  m = offset  n = length for a 7 digit zone number Zxxx.
 *  %m.n b  m = offset  n = length for a 8 character box number.
 *  %m.n c  m = offset  n = length for a 15 character customer order number.
 *-------------------------------------------------------------------------*/
zone_display(dest, format, z)
register char *dest, *format;
register struct zone_item *z;
{
  register long count, m, n;
  char work[16], *p;
  register struct oi_item *q;
  
#ifdef DEBUG
  fprintf(DF, "zone_display() zone=%d  format=[%s]\n", z->zt_zone, format);
  fflush(DF);
#endif
  
  count = strlen(format);
  
  while (count > 0)
  {
    if (*format != '%') {*dest++ = *format++; count--; continue;}
    format++; count--;
    
    m = *format - '0';  format++; count--;
    if (*format != '.')
    {
      m = 10 * m + (*format - '0');
      format++; count--;
    }
    format++; count--;
    
    n = *format - '0'; format++; count--;
    if (*format >= '0' && *format <= '9')
    {
      n = 10 * n + (*format - '0');
      format++; count--;
    }
    switch (*format)
    {
      case 'o': sprintf(work, "%7.*d", rf->rf_on, z->zt_on);
		if (n < 1) break;
		if (m + n > OrderLength) break;
		memcpy(dest, work + m, n);
		dest += n;
		break;
		
      case 'g': if (z->zt_order < 1 || z->zt_order > oc->of_size) break;
		if (n < 1) break;
		if (m + n > GroupLength) break;
		p = oc->oi_tab[z->zt_order - 1].oi_grp + m;
		for (; n > 0 && m < rf->rf_grp; n--, m++)
		{
		  *dest++ = toupper(*p++);
		}
		for (; n > 0; n--) *dest++ = 0x20;
		break;

      case 'b': if (z->zt_order < 1 || z->zt_order > oc->of_size) break;
		if (n < 1) break;
		if (m + n > BoxNoLength) break;

		q = &oc->oi_tab[z->zt_order - 1];
#ifdef DEBUG
                fprintf(DF, "rvj zone_display box = %8.8s m=%d n=%d\n",
                        q->oi_box,m,n);
                fflush(DF);
#endif
		p = q->oi_box + m;
		
		for (; n > 0 && m < BoxNoLength; n--, m++)
		{
		  *dest++ = toupper(*p++);
		}
		for (; n > 0; n--) *dest++ = 0x20;
		break;
		
      case 'c': if (z->zt_order < 1 || z->zt_order > oc->of_size) break;
		if (n < 1) break;
		if (m + n > ConLength) break;

		q = &oc->oi_tab[z->zt_order - 1];
		p = q->oi_con + m;
		
		for (; n > 0 && m < ConLength; n--, m++)
		{
		  *dest++ = toupper(*p++);
		}
		for (; n > 0; n--) *dest++ = 0x20;
		break;
		
      case 'z': 
		if (n < 1) break;
		if (m + n > 4) break;
                sprintf(work, "Z%-*.*d", n, n, z->zt_zone);
		memcpy(dest, work, strlen(work));
		dest += n+1;
		break;
      
      default:  break;
    }
    format++; count--;
  }
  *dest = 0;                              /* append a null to string         */
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Write To Total Function Port
 *-------------------------------------------------------------------------*/
Ac_Write(n, text, len, h)
register long n;
register char *text;
register long len;
register struct hw_item *h;
{
  register struct bay_item *b;
  register long k, ret;
  unsigned char work[16];
  
  if (po[n].po_disabled == 'y') return 0;
  if (!ports_open) return 0;

#ifdef DETAIL
  fprintf(DF, "Sending: Port=%d [%*.*s]\n", n, len, len, text);
  fflush(DF);
#endif
  
  ret = ac_write(port[n], text, len);
  
  if (!h) return ret;
  if (h->hw_type != PM2 && h->hw_type != PM4 && h->hw_type != PM6) return ret;
  b = &bay[h->hw_bay - 1];
  if (b->bay_flags & Multibin) return ret;
  
  for (k = h->hw_mod + 1; k < coh->co_mod_cnt; k++)
  {
    h = &hw[mh[k - 1].mh_ptr - 1];
    if (!h->hw_save) return ret;
  
    sprintf(work, "%04d%2.2s%03d%*.*s", h->hw_controller, text + 4,
    h->hw_mod_address, len - 9, len - 9, text + 9);

    if (sp->sp_pickline_view == 'y' && memcmp(text + 4, "09", 2) == 0)
    {
      memcpy(pmv[h->hw_mod - 1].hw_display, text + 9, 4);
      pmv[h->hw_mod - 1].hw_display[0] |= 0x80;
    }
    ret = ac_write(port[n], work, len);
  
#ifdef DETAIL
    fprintf(DF, "Sync:    Port=%d [%*.*s]\n", n, len, len, work);
    fflush(DF);
#endif
  }
  return ret;
}
/*-------------------------------------------------------------------------*
 *  Close All Ports
 *-------------------------------------------------------------------------*/
close_ports()
{
  register long k;
  
  ports_open = 0;
  
  for (k = 0; k < coh->co_ports; k++)
  {
    if (!port[k]) continue;
    if (!(po[k].po_flags & IsTotalFunction)) continue;
    if (sp->sp_total_function == 's') {alarm(0); close(port[k]);}
    else tc_close(port[k]);
    port[k] = 0;
  }
}
#ifdef SIMULATOR
/*-------------------------------------------------------------------------*
 *  Simulator
 *-------------------------------------------------------------------------*/
simulator()
{
  register struct pl_item   *p;
  register struct bay_item  *b;
  register struct zone_item *z;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register long k, m, n;

#ifdef DEBUG
  fprintf(DF, "simulator entry\n");
  fflush(DF);
#endif

  if (!superpicker) return 0;

  if (!config_ports || sim_flag)
  {
    signal(SIGALRM, simulator);
    alarm(STIME);
    return 0;
  }
  for (k = coh->co_zone_cnt; k > 0; k--)
  {
    z = &zone[k - 1];
    if (!z->zt_zone) continue;
    if (!z->zt_pl) continue;
    if (!(z->zt_flags & IsTotalFunction)) continue;
    if (z->zt_flags & ZoneInactive) continue;
    
    p = &pl[z->zt_pl - 1];
    if (p->pl_flags & SwitchesDisabled) continue;
    
    switch (z->zt_status)
    {
      case ZS_AHEAD:    break;

      case ZS_UNDERWAY:
      case ZS_EARLY:

	n = z->zt_first_bay;
	
	while (n)
	{
	  b = &bay[n - 1];

	  for (m = b->bay_mod_first; m <= b->bay_mod_last; m++)
	  {
	    h = &hw[mh[m - 1].mh_ptr - 1];
	    i = &pw[h->hw_first - 1];

	    if (!(h->hw_flags & ModuleHasPick)) continue;
	    if (i->pw_flags & BinPicked) continue;
	    i->pw_flags |= BinPicked;
	    i->pw_picked = i->pw_ordered;
	    z->zt_lines--;
	    b->bay_picks--;
	  }
	  n = b->bay_next;
	}
	if (z->zt_lines > 0) break;

	zone_clear(k, 3);
#ifdef TCBL_REMOVE
			message_put(0, ZoneCompleteEvent,
				     &z->zt_zone, sizeof(TZone));
#else
			message_put(coh->co_id, ZoneCompleteEvent,
				     &z->zt_zone, sizeof(TZone));
#endif
	message_put(coh->co_id, ZoneNextEvent, &z->zt_zone, sizeof(TZone));
	break;
	    
      case ZS_LOCKED:
      case ZS_COMPLETE:
      case ZS_WAITING:
      case ZS_LATE:
	
	zone_clear(k, 1);
	message_put(coh->co_id, ZoneNextEvent, &z->zt_zone, sizeof(TZone));
	break;
      }
  }
  signal(SIGALRM, simulator);
  alarm(STIME);
  return 0;
}
#endif
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
void leave(flag) // flag is a signum
register long flag;
{
  close_ports();
  message_close();
#ifdef TANDY
  od_close();
#endif

  //sections_close();
  //sec_prd_close(); 
  picker_close();
  xt_close();
  
  if (sp->sp_box_feature == 's')                        /* F060798 */
  {
    boxes_close();      
  }

  database_close();
  ss_close();
  co_close();
  oc_close();
  exit(0);
}


#ifdef TANDY
break_quantity(i, quan)
register struct pw_item *i;
register long quan;
{
#ifdef TANDY1
  i->pw_ordered = i->pw_case_ordered = i->pw_pack_ordered = 0;
  i->pw_picked  = i->pw_case_picked  = i->pw_pack_picked  = 0;

  if (i->pw_case > 0 && quan >= i->pw_case)
  {
    i->pw_case_ordered = quan / i->pw_case;
    quan -= (i->pw_case_ordered * i->pw_case);
  }
  if (i->pw_pack > 0 && quan >= i->pw_pack)
  {
    i->pw_pack_ordered = quan / i->pw_pack;
    quan -= (i->pw_pack_ordered * i->pw_pack);
  }
#endif
  i->pw_ordered = quan;
  return 0;
}
#endif

#ifdef TCBL_ALC
tcbl_write_event(x)
register TTcblMessage *x;
{
	
  register long k, ret;
  
  if (po[x->m_port].po_disabled == 'y') return 0;
  if (!ports_open) return 0;

  ret = ac_write(port[x->m_port], x->m_text, x->m_length);
  
  return ret;
}
#endif

process_zn_scan(z, b, h, p)
register struct zone_item *z;
register struct bay_item *b;
register struct hw_item *h;
unsigned char *p;
{
#ifdef DEBUG
  fprintf(DF, "process_zn_scan(%d, %d, %d, %s)\n", 
	  z->zt_zone, b->bay_number, h->hw_mod, p);
  fflush(DF);
#endif

  if (strlen(p) > ZONE_BARCODE_LEN && 
      strlen(p) == BADGE_BARCODE_LEN)
     {
        if (sp->sp_productivity == 'y')
           {
             process_picker(z, b, h, p);
           }

        return 0;
     }
  else if (strlen(p) <= ZONE_BARCODE_LEN)
     {
       if (sp->sp_productivity == 'y')
          {
            process_zone_scan(z, b, h, p);
          }

       return 0;
     }
  else if (strlen(p) == TOTE_BARCODE_LEN)
     {
       if (sp->sp_box_feature == 's')
          {
            process_tote_scan(z, b, h, p);
          }

       return 0;
     }
  else
     {
       return 0;
     }
} /* end of process_zn_scan */


process_tote_scan(z, b, h, p)
register struct zone_item *z;
register struct bay_item *b;
register struct hw_item *h;
unsigned char *p;
{
  register long int block        = 0L,
                    block1       = 0L,
                    k            = 0L,
                    next_order   = 0L,
                    scanned_tote = 0L;
  register short int start_flag  = 0;
  unsigned short int i  = 0;
  register struct oi_item *o;
  register struct oc_item *oy;
  register struct pl_item *ptr_pkln;
  register struct zone_item *zn;
  register struct hw_item *hz;
  TZoneOrderMessage x;
  char text[32];
  char box_num[9];
  char tote_barcode[TOTE_BARCODE_LEN+1];
  boxes_item box,
             cbox,
             newbox;
  unsigned short cur_bay  = 0;

#ifdef DEBUG
  fprintf(DF, "process_tote_scan(%d, %d, %d, %s)\n", 
	  z->zt_zone, b->bay_number, h->hw_mod, p);
  fflush(DF);
#endif

  memset(&box, 0x0, sizeof(boxes_item));
  memset(&newbox, 0x0, sizeof(boxes_item));
  memset(&cbox, 0x0, sizeof(boxes_item));

  tote_barcode[TOTE_BARCODE_LEN] = '\0';

  sprintf(tote_barcode, "%-*.*s", TOTE_BARCODE_LEN+1,
          TOTE_BARCODE_LEN+1, p);
  tote_barcode[TOTE_BARCODE_LEN] = '\0';
  scanned_tote = atol(tote_barcode);
#ifdef DEBUG
  fprintf(DF, "tote_barcode = %s, scanned_tote = %d\n", 
	  tote_barcode, scanned_tote);
  fflush(DF);
#endif

  /* Check to ensure tote was scanned only if ZC shows "SCAN LABEL" msg */
  cur_bay = z->zt_first_bay;
  hz = 0;

  while (cur_bay > 0)
        {
           if (bay[cur_bay - 1].bay_zc)
              {
                hz = &hw[bay[cur_bay - 1].bay_zc - 1];
                break;
              }

           cur_bay = bay[cur_bay - 1].bay_next;
        }  /* end of while loop to get a zone's ZC */

  if (sp->sp_pickline_view == 'y' && hz > 0)
     {
       if (zcv[hz->hw_mod - 1].hw_display[0] != 'S')
          {
             /* ZC not displaying "SCAN LABEL ..." msg */
             /* so ignore barcode scan                 */
#ifdef DEBUG
             fprintf(DF, "Zone %d Bay %d not displaying SCAN LABEL\n",
                     z->zt_zone, cur_bay);
             fprintf(DF, "hw_display[0] = %c\n",
                     zcv[hz->hw_mod - 1].hw_display[0]);
             fflush(DF);
#endif
             return 0;
          }
     }

  /* Check Database for validity of scanned tote number */
  begin_work();
  if (boxes_query(&box, scanned_tote))
     {
       sprintf(text, "BAD BOX %08d", scanned_tote);
#ifdef DEBUG
       fprintf(DF, "BAD BOX %08d\n", scanned_tote);
       fflush(DF);
#endif
       zone_message(z, text, 0, -1);
       return 0;
     }
  commit_work();
#ifdef DEBUG
  fprintf(DF, "Box: pl=%d, on=%d, box=%d, status=%s\n",
          box.b_box_pl, box.b_box_on, box.b_box_number, box.b_box_status);
  fflush(DF);
#endif

  /* Ensure order has no open boxes */
  cbox.b_box_pl        = box.b_box_pl;
  cbox.b_box_on        = box.b_box_on;
  cbox.b_box_status[0] = BOX_OPEN;
  cbox.b_box_status[1] = '\0';

  if (!boxes_query1(&cbox))
     {
        if (cbox.b_box_number == box.b_box_number)
           {
#ifdef DEBUG
             fprintf(DF, "  BOX STILL OPEN %d\n", cbox.b_box_number);
             fflush(DF);
#endif
             return 0;
           }
        else
           {
             memset(box_num, 0x0, 9);
             memset(text, 0x0, 32);
             sprintf(box_num, "%08d", cbox.b_box_number);
             sprintf(text, "BOX OPEN  %s", &box_num[2]);
#ifdef DEBUG
             fprintf(DF, "  BOX STILL OPEN [%s]\n", box_num);
             fflush(DF);
#endif
             zone_message(z, text, 0, -1);
             return 0;
           }
     }

  /* Check orders data structure for validity of order number */
  block = oc_find(box.b_box_pl, box.b_box_on);
#ifdef DEBUG
  fprintf(DF, "block = %ld, box's pickline = %d, box's order number = %ld\n",
          block, box.b_box_pl, box.b_box_on);
  fflush(DF);
#endif

  if (block <= 0L)
     {
       sprintf(text, "NOT IN CAPS%06d", box.b_box_on);
       zone_message(z, text, 0, -1);
       return 0;
     } 

  /* Get pointer to the scanned tote's order data structure */
  o = &oc->oi_tab[block - 1];

  /* Check if order is complete */
  if (o->oi_queue == OC_COMPLETE)
     {
       sprintf(text, "ORDER DONE%06d", box.b_box_on);
#ifdef DEBUG
       fprintf(DF, "ORDER DONE%06d\n", box.b_box_on);
       fflush(DF);
#endif
       zone_message(z, text, 0, -1);
       return 0;
     }

  /* Make sure scanned tote goes with next FIFO order for new orders */
  if (o->oi_queue == OC_HIGH ||
      o->oi_queue == OC_MED  ||
      o->oi_queue == OC_LOW  ||
      o->oi_queue == OC_HOLD   )
     {
       oy = &oc->oc_tab[z->zt_pl - 1];

       block1 = 0L;
       for (k = OC_HIGH; k <= OC_LOW; k++)
           {
             block1 = oy->oc_queue[k].oc_first;
             if (block1)
                {
                  break;
                }
           }

       if (block1 > 0L)
          {
            next_order = oc->oi_tab[block1 - 1].oi_on;
          }

#ifdef DEBUG
       fprintf(DF, "  Next order is %d from queue %d on pickline %d\n",
               next_order, k, z->zt_pl);
       fflush(DF);
#endif

       if (next_order == 0L)
          {
            sprintf(text, "NO ACTIVE ORDERS");
            zone_message(z, text, 0, -1);
            return 0;
          }

       if (next_order != box.b_box_on)
          {
            sprintf(text, "WRONG STOR%06.6d", box.b_box_on);
            zone_message(z, text, 0, -1);
            return 0;
          }
     }

  /* Find zone where order for scanned tote should be        */
  /* and check to be sure someone is logged in to that zone. */ 
  if (o->oi_queue == OC_UW)
     {
       for (i = 0; i < coh->co_zone_cnt; i++)
           {
             zn = &zone[i];
             if (box.b_box_pl        == zn->zt_pl  &&
                 box.b_box_on        == zn->zt_on  &&
                 box.b_box_status[0] != BOX_CLOSED   )
                {
#ifdef DEBUG
                  fprintf(DF, "In process_tote_scan(), zone = %d\n",
                          zn->zt_zone);
                  fflush(DF);
#endif
                  if (sp->sp_productivity == 'y')
                     {
                       if (!zn->zt_picker)
                          {
                            sprintf(text, "LOGIN Z%-3.3d      ", zn->zt_zone);
#ifdef DEBUG
                            fprintf(DF, "OC_UW Q LOGIN Z%-3.3d\n", zn->zt_zone);
                            fflush(DF);
#endif
                            zone_message(z, text, 0, -1);  /* 0 is no display */
                            zone_message(zn, text, 0, -1); /* 0 is no display */
                            return 0;
                          }
                       else
                          {
                            break;
                          }
                     }
                  else
                     {
                       break;
                     }
                }
           }
     }
  else if (o->oi_queue == OC_HIGH ||
           o->oi_queue == OC_MED  ||
           o->oi_queue == OC_LOW    )
     {
       for (i = 0; i < coh->co_zone_cnt; i++)
           {
             zn = &zone[i];
             if (box.b_box_pl        == zn->zt_pl   &&
                 o->oi_entry_zone    == zn->zt_zone &&
                 box.b_box_status[0] != BOX_CLOSED    )
                {
#ifdef DEBUG
                  fprintf(DF, "In process_tote_scan(), zone = %d\n",
                          zn->zt_zone);
                  fprintf(DF, "HML Queues \n");
                  fflush(DF);
#endif
                  start_flag = 1;
                  break;
                }
           }
     }

  /* Check if pickline has orders locked or disabled */
  ptr_pkln = &pl[zn->zt_pl - 1];

  if (ptr_pkln->pl_flags & StopOrderFeed   ||
      ptr_pkln->pl_flags & OrdersLocked    ||
      ptr_pkln->pl_flags & SwitchesDisabled  )
     {
#ifdef DEBUG
       fprintf(DF, "Pickline %d disabled/orders locked\n", zn->zt_pl);
       fflush(DF);
#endif
       return 0;
     }

  if (start_flag)
     {
       zn = &zone[ptr_pkln->pl_first_zone - 1];
#ifdef DEBUG
       fprintf(DF, "start_flag = %d, enabling zone %d\n",
               start_flag, ptr_pkln->pl_first_zone);
       fflush(DF);
#endif
     }

  if (zn->zt_lines)
     {
       if (zn->zt_on != box.b_box_on)
          {
            sprintf(text, "WRONG STOR%06.6d", box.b_box_on);
            zone_message(z, text, 0, -1);
            return 0;
          }
     }

  /* Make sure cannot reopen a box */
  if (box.b_box_status[0] == BOX_CLOSED)
     {
        sprintf(text, "BOX IS    CLOSED");
#ifdef DEBUG
        fprintf(DF, "BOX IS CLOSED, Box Number = %08d\n", box.b_box_number);
        fflush(DF);
#endif
        zone_message(z, text, 0, -1);  /* 0 is no display */
        return 0;
     }

  /* Save open box status of scanned tote to database */ 
  newbox.b_box_pl        = box.b_box_pl;                             
  newbox.b_box_on        = box.b_box_on;
  newbox.b_box_number    = box.b_box_number;
  newbox.b_box_last[0]   = 0x20;
  newbox.b_box_last[1]   = '\0';
  newbox.b_box_status[0] = BOX_OPEN;
  newbox.b_box_status[1] = '\0';
  newbox.b_box_lines     = 0;
  newbox.b_box_units     = 0;
 
  begin_work(); 
  boxes_update1(&newbox);
  commit_work();

  memcpy(o->oi_box, tote_barcode, BoxNoLength);        /* save box number */
  o->oi_flags &= ~NEED_BOX;                                    

  zone_enable(zn); 
  if (zn->zt_status == ZS_UNDERWAY)
     {
       show_zone_display(zn);
     }

  x.m_pickline = box.b_box_pl;
  x.m_order    = box.b_box_on;
  x.m_zone     = zn->zt_zone;

  message_put(0, ZoneNextRequest, &x, sizeof(TZoneOrderMessage));

  return 0;

} /* end of process_tote_scan */


process_picker(z, b, h, p)
register struct zone_item *z;
register struct bay_item *b;
register struct hw_item *h;
unsigned char *p;
{
    picker_item pkr;
    char badge_barcode[BADGE_BARCODE_LEN+1];
    long picker = 0L;
    short int index = 0;
    short int i = 0;
    char text[32];

    memset(&pkr, 0x0, sizeof(picker_item));

#ifdef DEBUG
  fprintf(DF, "process_picker(%d, %d, %d, [%s])\n", 
	  z->zt_zone, b->bay_number, h->hw_mod, p);
  fflush(DF);
#endif

    badge_barcode[BADGE_BARCODE_LEN] = '\0';

    sprintf(badge_barcode, "%-*.*s", BADGE_BARCODE_LEN+1,
            BADGE_BARCODE_LEN+1, p);
    badge_barcode[BADGE_BARCODE_LEN] = '\0';
    picker = atol(badge_barcode);
#ifdef DEBUG
  fprintf(DF, "badge_barcode = %s, picker = %ld\n", 
	  badge_barcode, picker);
  fflush(DF);
#endif
    pkr.p_picker_id = picker;

/*
07-14-08
    for (i = 0; i < NUMBER_OF_SCANNERS; i++)
    {
	if (pstatus[i].picker_id == pkr.p_picker_id)
	{
		fprintf(DF, "in the loop picker = %d\n", 
			pstatus[i].picker_id);
		pkr.p_zone = pstatus[i].zone;
		pkr.p_status = 1;
	}
    }
*/

#ifdef DEBUG
    fprintf(DF, "In process_picker() No Database:\n");
    fprintf(DF, "picker = %ld, zone assigned = %d, status = %d\n",
            pkr.p_picker_id, pkr.p_zone, pkr.p_status);
    fprintf(DF, "zone status = %c\n", z->zt_status); 
    fflush(DF);
#endif

    begin_work();
    if (picker_read(&pkr, LOCK))
    {
        sprintf(text, "Invalid %*s", BADGE_BARCODE_LEN, badge_barcode);
	zone_message(z, text, 0, -1); 
	commit_work();
       	return 0;
    }
    else
    {
        commit_work();
        begin_work();
        picker_query(&pkr, picker);
        commit_work();
    }


#ifdef DEBUG
    fprintf(DF, "In process_picker():\n");
    fprintf(DF, "picker = %ld, zone assigned = %d, status = %d\n",
            pkr.p_picker_id, pkr.p_zone, pkr.p_status);
    fprintf(DF, "zone status = %c\n", z->zt_status); 
    fflush(DF);
#endif

    for (index = 0; index < NUMBER_OF_SCANNERS; index++)
        {
          if (pkr.p_status == 1) break;

          if (pscan[index].ps_scan_zone == z->zt_zone)
             {
               sprintf(text, "WHAT ZONE?      ");
               zone_message(z, text, 0, -1); /* 0 is no display           */
               return 0;
             }
        }

    for (index = 0; index < NUMBER_OF_SCANNERS; index++)
        {
          if (pkr.p_status == 1) break;

          if (pscan[index].ps_picker_id == 0L &&
              pscan[index].ps_scan_zone == 0)
             {
               pscan[index].ps_picker_id = pkr.p_picker_id;
               pscan[index].ps_scan_zone = z->zt_zone;
               break; 
             }
        }

    if (pkr.p_status == 0)
       {
         sprintf(text, "WHAT ZONE?      ");
	 zone_message(z, text, 0, -1); /* 0 is no display           */
       }
    else
       {
          process_picker_logout(z, &pkr);
       }

    return 0;
}   /* end of process_picker() */


process_zone_scan(z, b, h, p)
register struct zone_item *z;
register struct bay_item *b;
register struct hw_item *h;
unsigned char *p;
{
    picker_item pkr, *pkrptr;
    char zone_barcode[ZONE_BARCODE_LEN+1];
    short int scanned_zone = 0;
    unsigned short int index = 0;
    long picker = 0L;
    char text[32];
    char picker_name[12];

    memset(&pkr, 0x0, sizeof(picker_item));

#ifdef DEBUG
  fprintf(DF, "process_zone_scan(%d, %d, %d, [%s])\n", 
	  z->zt_zone, b->bay_number, h->hw_mod, p);
  fflush(DF);
#endif

    zone_barcode[ZONE_BARCODE_LEN] = '\0';

    sprintf(zone_barcode, "%-*.*s", ZONE_BARCODE_LEN+1,
            ZONE_BARCODE_LEN+1, p);
    zone_barcode[ZONE_BARCODE_LEN] = '\0';
    scanned_zone = atoi(zone_barcode);
#ifdef DEBUG
  fprintf(DF, "zone_barcode = %s, scanned_zone = %d\n", 
	  zone_barcode, scanned_zone);
  fflush(DF);
#endif

    if (scanned_zone <= 0 ||
        scanned_zone > coh->co_zone_cnt ||
        scanned_zone > MAX_ZONES)
       {
          sprintf(text, "INVALID ZONE %-3.3d", scanned_zone);
	  zone_message(z, text, 0, -1); /* 0 is no display           */
          return 0;
       }

    for (index = 0; index < NUMBER_OF_SCANNERS; index++)
        {
          if (pscan[index].ps_scan_zone == z->zt_zone)
             {
               pkr.p_picker_id = pscan[index].ps_picker_id;
               pscan[index].ps_picker_id = 0L;
               pscan[index].ps_scan_zone = 0;
               break; 
             }
        }

    if (index == NUMBER_OF_SCANNERS &&
        pkr.p_zone == 0 &&
        pkr.p_picker_id == 0L)
       {
         sprintf(text, "SCAN BADGE FIRST");
         zone_message(z, text, 0, -1);      /* 0 is no display */
         return 0;
       }

#ifdef DEBUG
    fprintf(DF, "In process_zone_scan():\n");
    fprintf(DF, "picker = %ld, zone assigned = %d, status = %d\n",
            pkr.p_picker_id, scanned_zone, pkr.p_status);
    fprintf(DF, "zone status = %c\n", z->zt_status); 
    fflush(DF);
#endif

    picker = pkr.p_picker_id;

    begin_work();
    picker_query(&pkr, picker);
    commit_work();

    pkr.p_zone = scanned_zone;
 
    memset(picker_name, 0 , sizeof(picker_name));
    memcpy(picker_name, pkr.p_last_name, 12);
    picker_name[11] = '\0';
    process_picker_login(z, picker, picker_name);

    return 0;
}   /* end of process_zone_scan() */


process_picker_login(z, pickerid, pickername)
struct zone_item *z;
long pickerid;
char *pickername;
{
    struct zone_item *zn;
    picker_item pckr;
    sections_item sect;
    section_prod_item secprd;
    char text[32];
    //char pickername[32];

#ifdef DEBUG
  fprintf(DF, "process_picker_login(%d, %d)\n", 
	  z->zt_zone, pickerid);
  fflush(DF);
#endif

    memset(&pckr, 0x0, sizeof(picker_item));
    memset(&sect, 0x0, sizeof(sections_item));
    memset(&secprd, 0x0, sizeof(section_prod_item));

    zn = &zone[z->zt_zone - 1];

    if (zn->zt_picker)
       {
          sprintf(text, "%9.9s IN %-3.3d", zn->zt_picker_name, zn->zt_zone);
          zone_message(z, text, 0, -1); /* 0 is no display           */
          return 0;
       }

  //  sprintf(text, "%09d LOGGED",p->p_picker_id);
    sprintf(text, "%9.9s LOGGED",pickername);
    zone_message(z, text, 0, -1); /* 0 is no display           */
    zone_message(zn, text, 0, -1); /* 0 is no display */

/*
    memset(pickername, 0, sizeof(pickername));
    sprintf(pickername, "%09d", p->p_picker_id);
*/

    zn->zt_picker = pickerid;
   // memcpy(zn->zt_picker_name, pickername, 12);
    memcpy(zn->zt_picker_name, pickername, 12);
    zn->zt_picker_name[11] = '\0';
  
#ifdef DEBUG
	fprintf(DF, "zt = [%s] pkr = [%s]\n",
		zn->zt_picker_name, pickername);
	fflush(DF);
#endif

    //memcpy(&pckr, p, sizeof(picker_item));
    memset(&pckr, 0, sizeof(picker_item));

    pckr.p_start_time = time(0);
    pckr.p_cur_pl     = zn->zt_pl;
    pckr.p_zone       = zn->zt_zone;
    pckr.p_status     = 1;
    pckr.p_picker_id  = pickerid;
    
    begin_work(); 
    picker_update(&pckr);
    commit_work();

/*
    pstatus[zn->zt_zone].picker_id = zn->zt_picker;
    pstatus[zn->zt_zone].zone = zn->zt_zone;
//--New

    begin_work();
    while(!sections_next_z(&sect, pckr.p_zone))
         {
#ifdef DEBUG
            fprintf(DF, "Zone = %d, Section = %s\n",
                    pckr.p_zone, sect.s_caps_section);
            fflush(DF);
#endif
            if (strcmp(secprd.scp_caps_sections, sect.s_caps_section) == 0)
               {
                  continue;
               }
            else
               {
                  strcpy(secprd.scp_caps_sections, sect.s_caps_section);
                  secprd.scp_picker_id = pckr.p_picker_id;
                  secprd.scp_zone = pckr.p_zone;
                  secprd.scp_cum_order_count = 0L;
                  secprd.scp_cum_lines = 0L;
                  secprd.scp_cum_units = 0L;
                  secprd.scp_cum_time = 0L;
                  secprd.scp_start_time = time(0);
                  secprd.scp_login_time = time(0);
                  secprd.scp_logout_time = 0L;
                  secprd.scp_status = 1;
                  secprd.scp_record_date[0] = '\0';
                  //sec_prd_write(&secprd);
               }
         }
    commit_work();
*/
    zone_enable(zn);

    return 0;
}  /* end of process_picker_login() */


process_picker_logout(z, p)
register struct zone_item *z;
register picker_item *p;
{
    struct zone_item *zn;
    picker_item pckr;
    sections_item secto;
    section_prod_item secprdo;
    char text[32];
    long elapsed = 0;
    unsigned long int now = 0L;
    time_t * nowptr = 0;
    char datemsg[DATETIME_SIZE];

    memset(datemsg, 0x0, DATETIME_SIZE);

#ifdef DEBUG
  fprintf(DF, "process_picker_logout(%d, %ld)\n", 
	  z->zt_zone, p->p_picker_id);
  fflush(DF);
#endif

    memset(&pckr, 0x0, sizeof(picker_item));
    memset(&secto, 0x0, sizeof(sections_item));
    memset(&secprdo, 0x0, sizeof(section_prod_item));

    zn = &zone[p->p_zone - 1];
#ifdef DEBUG
    fprintf(DF, "process_picker_logout(): zone = %d, picker = %d\n",
            p->p_zone, zn->zt_picker);
    fflush(DF);
#endif

    if (zn->zt_picker != p->p_picker_id)
       {
         sprintf(text, "%9.9s IN %d", zn->zt_picker_name, zn->zt_zone);
         zone_message(z, text, 0, -1); /* 0 is no display           */
         return 0;
       }

    sprintf(text, "%09d LOGOUT",p->p_picker_id);
    //sprintf(text, "%9.9s LOGOUT",p->p_last_name);
    zone_message(z, text, 0, -1); /* 0 is no display           */
    zone_message(zn, text, 0, -1); /* 0 is no display) */

    zone_picker_logout(zn->zt_zone, 2,zn->zt_picker);

#ifdef DEBUG
	fprintf(DF, "zt = [%s] pkr = [%s]\n",
		zn->zt_picker_name, p->p_last_name);
	fflush(DF);
#endif

    memcpy(&pckr, p, sizeof(picker_item));

    zn->zt_picker = 0;
    memset(zn->zt_picker_name, 0, 12);
    zn->zt_picker_name[11]= '\0';
  
    elapsed = time(0) - pckr.p_start_time;
    
    pckr.p_cur_time  += elapsed;
    pckr.p_cum_time  += elapsed;
    pckr.p_start_time = 0;
    pckr.p_zone       = 0;
    pckr.p_status     = 0;
   
    begin_work(); 
    picker_update(&pckr);
    commit_work();

/*
    pstatus[zn->zt_zone].picker_id = 0;
    pstatus[zn->zt_zone].zone = 0;
*/

    now = time(0);
    nowptr = (time_t *)&now;
    strftime(datemsg,
             DATETIME_SIZE,
             "%Y-%m-%d %T",
             localtime(nowptr));
#ifdef DEBUG
    fprintf(DF, "process_picker_logout():  datemsg = %s\n",
            datemsg);
    fflush(DF);
#endif

/*
    begin_work();
    while(!sections_next_z(&secto, zn->zt_zone))
         {
           if (strcmp(secprdo.scp_caps_sections, secto.s_caps_section) == 0)
              {
                 continue;
              }
           else
              {
                 strcpy(secprdo.scp_caps_sections, secto.s_caps_section);
                 secprdo.scp_picker_id = pckr.p_picker_id;
                 secprdo.scp_logout_time = time(0);
                 secprdo.scp_status = 0;
                 strcpy(secprdo.scp_record_date, datemsg);
#ifdef DEBUG
         fprintf(DF, "process_picker_logout(): section = %s\n",
                 secprdo.scp_caps_sections);
         fprintf(DF, "  picker = %ld, stoptime = %d, status = %d, date = %s\n",
                 secprdo.scp_picker_id, secprdo.scp_logout_time,
                 secprdo.scp_status,
                 secprdo.scp_record_date);
         fflush(DF);
#endif
                 //sec_prd_update3(&secprdo);
              }
         }
    commit_work();
*/
    zone_disable(zn, 1);

    return 0;
}  /* end of process_picker_logout() */



zone_picker_logout(zn, flag,picker_id)
register long zn;                         /* zone number                     */
register long flag;                       /* 1 = clear, 2 = return picks     */
register long picker_id;
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct bay_item  *bn;
  register struct zone_item *z;
  register long j, k, m, jn;
  register long count = 0L;
  TPickMessage x;
  TPickBoxMessage y;
  unsigned char text[MAX_MSG_SIZE];
  long ntime = 0L;
  register long hw_cnt    = 0L,
                first_mod = 0L,
                m_first   = 0L,
                cmd_lgth  = 0L,
                m_mbl     = 0L,
                zc_mod    = 0L,
                bf_mod    = 0L,
                bl_mod    = 0L;
#ifdef TANDY
  register struct hw_item   *hz;
  TZone redisplay_zone;
  long clear_flag = 0;
#endif
  
  z = &zone[zn - 1];                      /* point to zone record            */

#ifdef DEBUG
  fprintf(DF, 
   "zone_picker_logout(): Zone=%d z->zt_last_light=%d flag=%d picker=%d\n", 
		zn, z->zt_last_light,flag, picker_id);
  fflush(DF);
  ntime = time(0);
  fprintf(DF, "              Time=[%24.24s]\n", ctime(&ntime));
  fflush(DF);
#endif
	

  if (flag & 2)                           /* setup return of picks           */
  {
    x.m_pickline = z->zt_pl;
    x.m_order    = z->zt_on;
  }
  if (flag & 1)
  {
    z->zt_lines  = 0;
    z->zt_flags |= ZoneInactive;
  }
  k = z->zt_first_bay;
  
  while (k > 0)                           /* over all bays in the zone       */
  {
    m_mbl = -1L;
    b = &bay[k - 1];
    k = b->bay_next;
    j = b->bay_port - 1;

    for (m = b->bay_prod_first; m && m <= b->bay_prod_last; m++)
    {
      i = &pw[m - 1];
	
      if ((i->pw_flags & BinPicked) && (flag & 2))
         {
           x.m_module    = m;
           x.m_picked    = i->pw_picked +
                           i->pw_case_picked * i->pw_case + 
                           i->pw_pack_picked * i->pw_pack;

           x.m_reference = i->pw_reference;

           if (sp->sp_box_feature == 's')     /* F052698 - pick with box */
              {
                y.m_pickline  = z->zt_pl;
                y.m_order     = z->zt_on;
                y.m_module    = x.m_module;
                y.m_picked    = x.m_picked;
                y.m_reference = x.m_reference;
                y.m_box       = 0;
                y.m_picker_id = picker_id;
			 
                if (!(oc->oi_tab[z->zt_order - 1].oi_flags & NEED_BOX))
                   {
                     memset(text, 0x0, BoxNoLength+1);
                     memcpy(text,
                            oc->oi_tab[z->zt_order - 1].oi_box,
                            BoxNoLength);
                     text[BoxNoLength] = '\0';
                     y.m_box = atol(text);
                   }
		#ifdef DEBUG
			fprintf(DF, "Before ModulePickBoxEvent\n");
			fflush(DF);
		#endif
                message_put(0, ModulePickBoxEvent, &y, sizeof(TPickBoxMessage));
              }
           else
              {
		#ifdef DEBUG
			fprintf(DF, "Before ModulePickBoxEvent\n");
			fflush(DF);
		#endif
                message_put(0, ModulePickEvent, &x, sizeof(TPickMessage));
              }
#ifdef TANDY
           if (i->pw_flags & BinPicked)
              {
                i->pw_ordered      = i->pw_picked      = 0;
                i->pw_case_ordered = i->pw_case_picked = 0;
                i->pw_pack_ordered = i->pw_pack_picked = 0;
                i->pw_flags       &= PicksInhibited;
              }
#else
           i->pw_ordered      = i->pw_picked      = 0;
           i->pw_case_ordered = i->pw_case_picked = 0;
           i->pw_pack_ordered = i->pw_pack_picked = 0;
           i->pw_flags       &= PicksInhibited;
#endif
         }
      else if (flag & 1)
         {
           i->pw_ordered      = i->pw_picked      = 0;
           i->pw_case_ordered = i->pw_case_picked = 0;
           i->pw_pack_ordered = i->pw_pack_picked = 0;
           i->pw_flags       &= PicksInhibited;
         }
    }

    for (m = b->bay_mod_first; m && m <= b->bay_mod_last; m++)
    {
      h = &hw[mh[m - 1].mh_ptr - 1];
      
      if (flag & 1)                       /* clear all modules               */
      {
	if (sp->sp_pickline_view == 'y')
	{
	  memset(pmv[h->hw_mod - 1].hw_display, 0x20, 6);
	}
	h->hw_state   = 0;
	h->hw_save    = 0;
	h->hw_current = 0;
	h->hw_flags  &= SwitchesDisabled;
      }
      else if (h->hw_flags & ModulePicked)
      {
#ifdef DEBUG
	fprintf(DF, "After ModulePicked Loop \n");
	fflush(DF);
#endif
#ifdef TANDY_BANG_BANG
        if (h->hw_mod == z->zt_last_light && clear_flag)
           {
              h->hw_flags &= ~ModulePicked;
              h->hw_state   = 2;
              if (z->zt_lines)
                 {
                    b->bay_picks += 1;
                    z->zt_lines  += 1;
                 }
              else
                 {
                    b->bay_picks = 1;
                    z->zt_lines  = 1;
                 }
              hz = &hw[b->bay_zc - 1];
              hz->hw_state = 3;
              if (h->hw_type == PM2)
                 {
                    process_pm2_switch(z, b, h, 2);
                 }
              else
                 {
                    process_pm4_switch(z, b, h, 2);
                 }
           }
        else
           {
#endif
              if (sp->sp_pickline_view == 'y')
                 {
                   memset(pmv[h->hw_mod - 1].hw_display, 0x20, 6);
                 }

              h->hw_state   = 0;
              h->hw_save    = 0;
              h->hw_current = 0;
              h->hw_flags  &= SwitchesDisabled;
#ifdef TANDY_BANG_BANG
           }
#endif
      	sprintf(text, "%04d10%03dB", h->hw_controller, h->hw_mod_address);
      	Ac_Write(j, text, 10, 0);
      }
    }  /* end of for loop to set each pick module's parameters */

  }  /* end of while loop for each bay */
  return 0;
}
