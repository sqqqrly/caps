#define DEBUG
/* #define PICKS  */ 
#define GTE
#define JCP
/* #define TIMER */
/* #define EXTRA_NEXT */

#define PICK_BY_ZONE
#define NEWREMARKS
#define NEWBOXFULL /*	change to remove the boxes table rvj 092600	*/
#define STAPLES

/*-------------------------------------------------------------------------*
 *  Custom:         EXTRA_NEXT   - Log extra next pushs.
 *                  DAYTIMER     - Pickline 2 no early exit if same order in 3.
 *                                 Also, early exit in last zone of pickline.
 *                  DELL         - Special multiple boxes per zone.
 *                  DELL_BOXES   - Number of boxes in a zone.
 *                  INORDER      - branches merge is queued order.
 *                  PICK_BY_ZONE - Picker accountability by zone.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Message driver order flow control.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/17/93   | tjt  Rewritten.
 *  04/04/94   | tjt  Added ZS_INACTIVE.
 *  05/06/94   | tjt  Fix Restoreplace event for ff and alc.
 *  05/06/94   | tjt  Add Zone status of ZS_OFFLINE.
 *  05/10/95   | tjt  Fix Jump zone ahead and not finished to ahead.
 *  05/12/94   | tjt  Add IsOffline flag.
 *  05/26/94   | tjt  Fix Steering over ahead zone on subject order.
 *  05/27/94   | tjt  Cancelled orders to complete queue.
 *  05/31/94   | tjt  Self-loop test.
 *  06/01/94   | tjt  System recovery zones go to waiting and not locked.
 *  06/02/94   | tjt  Tranactions moved to pick_update_db to avoid race.
 *  07/29/94   | tjt  Automatic feed into waiting zones.
 *  08/11/94   | tjt  Enable on one pick module.
 *  10/04/94   | tjt  Save all shared segments on markplace.
 *  10/20/94   | tjt  Fix productivity to accumulate when no uw.
 *  11/17/94   | tjt  Add segmented pickline.
 *  02/02/95   | tjt  Remove UNOS queues.
 *  02/23/95   | tjt  Port redisplay.
 *  02/28/95   | tjt  Fix bug in early exit next.
 *  03/02/95   | tjt  Fix ignore zone next when disabled.
 *  04/22/95   | tjt  Fix pickline productivity when orders queued.
 *  04/26/95   | tjt  Fix picker accountabilty for order groups.
 *  04/27/95   | tjt  Fix productivity revised.
 *  04/28/95   | tjt  Add dummy port type.
 *  05/02/95   | tjt  Add case pack to pick text for eby-brown.
 *  06/21/95   | tjt  Add krash message on init failures.
 *  12/09/95   | tjt  Add zone to picks.
 *  04/04/96   | tjt  Fix disqualified segments.
 *  06/28/96   | tjt  Fix message destination problem.
 *  07/08/96   | tjt  Add check sku support too for st_init.
 *  07/08/96   | tjt  Add early exit from branched picklines.
 *  07/26/96   | tjt  Add check early exit range.
 *  07/31/96   | tjt  Fix change to strcpy in redisplay error message.
 *  08/23/96   | tjt  Add begin and commit work to pick_next.
 *  01/08/97   | tjt  Add late entry and jump zone to segments.
 *  01/09/97   | tjt  Fix late entry method.
 *  01/09/97   | tjt  Add merge orders in queues sequence.
 *  03/14/97   | tjt  Add demand feed attribute.
 *  03/14/97   | tjt  Add ZoneNextRequest for order feed.
 *  06/15/01   | aha  Added productivity tracking by section for completes.
 *  10/08/01   | aha  Modified for Eckerd's Tote Integrity.
 *  02/27/03   | aha  Fixed for reconfiguration.
 *-------------------------------------------------------------------------*/
static char ofc_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "global_types.h"
#include "file_names.h"
#include "eh_nos.h"
#include "box.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "pr.h"
#include "message_types.h"
#include "caps_messages.h"
#include "zone_status.h"
#include "picker_acct.h"

#include "Bard.h"
#include "bard/picker.h"
#include "bard/picker_order.h"
#include "bard/remarks.h"
#include "boxes.h"


void leave(int val);                      /* graceful exit routine           */
void port_timeout(int signum);            /* signal handler. watchdog on port requests */
long cvrt();

#define Qtote 0
#define Qship 1
#define Qpack 2

long MP_TIMEOUT;                          /* timeout for port response       */
long RP_TIMEOUT;                          /* timeout for port response       */

long pa_order_lines;                      /* to pass lines to badge reader   */
long pa_order_units;                      /* to pass units to badge reader   */

long shutdown_flag = 0;                   /* got a shutdown request          */

long next_save_time = 0;                  /* next segment save               */

long who;                                 /* global message sender           */
long type;                                /* global message type             */
long len;                                 /* global message length           */
TCapsMessageItem buf;                     /* global message buffer           */

long event_start;                         /* init/mp/rp start time           */
long event_count;                         /* ports to respond                */
long redisplay_count;                     /* zones in redisplay              */
long redisplay_request;                   /* zone, pickline or restoreplace  */
long requestor;                           /* action requestor                */
TCapsMessageItem rbuf;                    /* redisplay message               */
long event_status;                        /* action status                   */

unsigned char list[] = {PortInitializeEvent, PortMarkplaceEvent,
  InitializeRequest, ConfigureRequest, RestoreplaceRequest, MarkplaceRequest,
  PicklineLockRequest, PicklineUnlockRequest, PicklineDisableRequest,
  PicklineEnableRequest, PicklineStopRequest, SystemRecoveryEvent,
  ModuleInhibitRequest, ModuleEnableRequest, ShutdownRequest, ZoneNextEvent, 
  ZoneCompleteEvent, OrderCancelRequest, ZoneStopEvent, BadgeScanEvent,
  PicklineRedisplayRequest, ZoneRedisplayRequest, ZoneClearEvent,
  ZoneOfflineRequest, ZoneOnlineRequest, PortRedisplayRequest,
  ZoneNextRequest};
   
#ifdef DELL
  char zone_boxes[50] = {0};
  long DELL_BOXES = 3;
  long DELL_LAST  = 1;
#endif

#ifdef DEBUG
long stime;
#endif

#ifdef DEBUG
FILE *DF;
#define DF_SIZE 4000000
#endif


main(argc, argv)
long argc;
char **argv;
{
	register	Arg;
	int	SignalHandler();

  putenv("_=ofc");                        /* name to environ                 */
  chdir(getenv("HOME"));                  /* to home directory               */
  
  setpgrp();
  
  signal_catcher(0);                      /* catch any signal                */

  open_all();

  message_open();                         /* message selection               */
  message_select(list, sizeof(list));     /* message selection               */
  coh->co_id = msgtask;                   /* store message id to config      */

#ifdef DELL
  if (argc > 1) DELL_BOXES = atol(argv[1]);
  if (argc > 2) DELL_LAST  = atol(argv[2]);
#endif

#ifdef DEBUG
  DF = fopen("debug/ofc_bug", "w");

  fprintf(DF, "ofc started:  msgtask=%d\n", msgtask);
  fflush(DF);
#endif

  while (1)
  {
#ifdef DEBUG
    fflush(DF);
    if (ftell(DF) > DF_SIZE)
    {
      fclose(DF);
      system("mv debug/ofc_bug debug/ofc_save 1>/dev/null 2>&1");
      DF = fopen("debug/ofc_bug", "w");
    }
#endif
    message_get(&who, &type, &buf, &len); /* wait for a message              */

#ifdef DEBUG
  {
    long now;
    
    stime = microclock();
    now = time(0);
  
    fprintf(DF, "got message: who=%d type=%d %s len=%d %24.24s\n", 
      who, type & 0x7f, type > 0x7f ? "Event" : "Request", len, ctime(&now));
    if (len > 0) Bdump(&buf, len);
  }  
#endif

    *((char *)&buf + len) = 0;            /* insure is a null                */

    switch (type)                         /* branch on message type          */
    {
      case ShutdownRequest: shutdown(); break;

      case ShutdownEvent: leave(0); break;

      case InitializeRequest:
      
        port_initialize_request();
        break;

      case PortInitializeEvent:
      
        port_initialize_event();
        break;
      
      case MarkplaceRequest:

        pickline_disable(0);
        port_markplace_request();
        break;
      
      case PortMarkplaceEvent:
      
        port_markplace_event();
        break;

      case RestoreplaceRequest:
      
        port_restoreplace_request();
        break;

      case SystemRecoveryEvent:
      
        system_recovery();
        break;

      case ConfigureRequest:
      
        configure_request(&buf, len);
        break;

      case OrderCancelRequest:
      
        order_cancel_request(buf.OrderMessage.m_pickline,
        buf.OrderMessage.m_order);
        break;
      
      case PicklineDisableRequest:
      
        pickline_disable(buf.PicklineMessage.m_pickline);
        message_put(0, PicklineDisableEvent, &buf, len);
        break;
      
      case PicklineEnableRequest:
      
        pickline_enable(buf.PicklineMessage.m_pickline);
        message_put(0, PicklineEnableEvent, &buf, len);
        break;

      case PicklineLockRequest:
      
        pickline_lock(buf.OrderMessage.m_pickline, buf.OrderMessage.m_order);
        message_put(who, PicklineLockEvent, &buf, len);
        break;
        
      case PicklineUnlockRequest:
      
        pickline_unlock(buf.PicklineMessage.m_pickline);
        message_put(who, PicklineUnlockEvent, &buf, len);
        break;

      case PortRedisplayRequest:
      
        if (redisplay_count)
        {
          buf.ErrorMessage.m_error = LOCAL_MSG;
          strcpy(buf.ErrorMessage.m_text, "Redisplay Is Running");
          message_put(who, ClientMessageEvent, &buf, sizeof(buf)); //strlen(&buf));
          break;
        }
        redisplay_request = PortRedisplayRequest;
        requestor = who;
        memcpy(&rbuf, &buf, sizeof(TPicklineMessage));
        port_redisplay_request(buf.PortMessage.m_port);
        break;
                  
      case PicklineRedisplayRequest:
      
        if (redisplay_count)
        {
          buf.ErrorMessage.m_error = LOCAL_MSG;
          strcpy(buf.ErrorMessage.m_text, "Redisplay Is Running");
          message_put(who, ClientMessageEvent, &buf, sizeof(buf)); //strlen(&buf));
          break;
        }
        redisplay_request = PicklineRedisplayRequest;
        requestor = who;
        memcpy(&rbuf, &buf, sizeof(TPicklineMessage));
        pickline_redisplay_request(buf.PicklineMessage.m_pickline);
        break;
            
      case PicklineStopRequest:
      
        pickline_stop_request(buf.PicklineMessage.m_pickline);
        message_put(who, PicklineStopEvent, &buf, sizeof(TPicklineMessage));
        break;
            
      case ZoneStopEvent:
      
        zone_cancel_event(buf.ZoneMessage.m_zone);
        break;

      case ZoneCompleteEvent:

        finish_zone(buf.ZoneMessage.m_zone, time(0));
        break;

      case ZoneNextRequest:
      
        zone_next_request(&buf);
        break;
        
      case ZoneNextEvent:
      
        zone_next_event(buf.ZoneMessage.m_zone, 0);
        break;
        
      case ZoneRedisplayRequest:
      
        if (redisplay_count)
        {
          buf.ErrorMessage.m_error = LOCAL_MSG;
          strcpy(buf.ErrorMessage.m_text, "Redisplay Is Running");
          message_put(who, ClientMessageEvent, &buf, sizeof(buf)); //strlen(&buf));
#ifdef FIX
          break;
#endif
        }
        redisplay_request = ZoneRedisplayRequest;
        requestor = who;
        memcpy(&rbuf, &buf, sizeof(TZoneMessage));
        zone_redisplay_request(buf.ZoneMessage.m_zone);
        break;
      
      case ZoneClearEvent:
      
        zone_clear_event(buf.ZoneMessage.m_zone);

        if (redisplay_count == 0)
        {
          if (redisplay_request == RestoreplaceRequest)
          {
            message_put(0, RestoreplaceEvent, 0, 0);
            if (sp->sp_pa_count) pa_open();
          }
          else if (redisplay_request == PicklineRedisplayRequest)
          {
            message_put(requestor, PicklineRedisplayEvent, &rbuf,
             sizeof(TPicklineMessage));
          }
          else
          {
            message_put(requestor, ZoneRedisplayEvent, &rbuf, 
              sizeof(TZoneMessage));
          }
        }
        break;
      
      case ZoneOfflineRequest:
      
        zone_offline(&buf);
        break;

      case ZoneOnlineRequest:
      
        zone_online(&buf);
        break;

      case ModuleInhibitRequest:
      
        module_pick_inhibit(&buf);
        break;

      case ModuleEnableRequest:
      
        module_pick_enable(&buf);
        break;
      
      case BadgeScanEvent:

        pa_request(&buf.BadgeMessage);
        break;
    }                                     /* end of switch                   */
  }                                       /* end of main while loop          */
}
/*-------------------------------------------------------------------------*
 *  Shutdown Request
 *-------------------------------------------------------------------------*/
shutdown()
{
  if (shutdown_flag) return 0;            /* already got a shutdown          */
  shutdown_flag = 1;                      /* mark shutdown underway          */
  alarm(0);                               /* stop alarms                     */
  pickline_disable(0);
  save_segs();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Next Request For Order Feed Into A Zone
 *-------------------------------------------------------------------------*/
zone_next_request(x)
register TZoneOrderMessage *x;
{
  register struct pl_item   *p;
  register struct zone_item *z;
  register struct bay_item *b;
  register struct oi_item   *o;
  register long zn, pn, block;
  
#ifdef DEBUG
  fprintf(DF, "zone_next_request() pl=%d zn=%d on=%d\n",
    x->m_pickline, x->m_zone, x->m_order);
#endif

  if (sp->sp_running_status != 'y') return 0; /* ignore request              */
  
  zn = x->m_zone;                         /* zone number                     */
  if (zn < 1 || zn > coh->co_zone_cnt) return 0; /* out of range             */
  
  pn = x->m_pickline;
  
  z = &zone[x->m_zone - 1];               /* point to zone                   */
  if (z->zt_pl != pn) return 0;           /* picklines do not agree          */
  
  p = &pl[pn - 1];                        /* point to pickline               */
  
  if (p->pl_flags & (StopOrderFeed | OrdersLocked | SwitchesDisabled))
  {
    return 0;                             /* pickline not available          */
  }
  if (!(z->zt_flags & DemandFeed)) return 0; /* not allowed in zone          */
  
  switch (z->zt_status)
  {
    case ZS_UNDERWAY:           
    case ZS_EARLY:
    case ZS_AHEAD:
    case ZS_INACTIVE:
    case ZS_LATE:      return 0;          /* unexpected status               */
    
    case ZS_WAITING:        
    case ZS_COMPLETE:
    case ZS_LOCKED:
    case ZS_OFFLINE:   break;             /* attempt to start an order       */

    default:           return 0;          /* status is trash                 */
  }
  block = oc_find(x->m_pickline, x->m_order);
  
  if (block < 1) return 0;                /* order is invalid                */
  
  o = &oc->oi_tab[block - 1];             /* point to index entry            */
  
#ifdef STAPLES
  if (z->zt_flags & (JumpZone | FirstZone)) 
#else
  if (z->zt_flags & FirstZone)            /* order must be queued            */
#endif
  {
    if (o->oi_queue == OC_UW)       return 0; 
    if (o->oi_queue == OC_WORK)     return 0;
    if (o->oi_queue == OC_COMPLETE) return 0;
  
    oc_lock();
    oc_dequeue(block);
    oc_enqueue(block, OC_LAST, OC_UW);
    o->oi_entry_zone = z->zt_zone;
    oc_unlock();
    
    z->zt_on    = x->m_order;
    z->zt_order = block;
    start_order(z, block, 0);
  }
  else
  {
#ifndef STAPLES
  if (z->zt_flags & JumpZone)
  {
      if (find_next_order(z, z->zt_picker)) 
      {
        finish_zone(z->zt_zone, z->zt_time);
        message_put(0, ZoneNextEvent, &z->zt_zone, sizeof(TZone));
        return 0;
      }
  }
#endif
    if (o->oi_queue != OC_UW)   return 0;
    if (o->oi_entry_zone != zn) return 0;
    
    z->zt_on    = x->m_order;
    z->zt_order = block;
  }

  start_zone(z);
  
#ifdef PICK_BY_ZONE
  pa_by_zone(z);
#endif

  b = &bay[z->zt_first_bay - 1];          /* point to first bay              */
  
  message_put(po[b->bay_port - 1].po_id, ZoneStartRequest,
    &z->zt_zone, sizeof(TZone));
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Next Event From A Zone
 *-------------------------------------------------------------------------*/
zone_next_event(zn, picker)
register long zn;
register long picker;                     /* picker badge number or zero     */
{
  register struct bay_item  *b;
  register struct pl_item   *p;
  register struct zone_item *z;
  register struct oi_item   *o;
  register struct oc_item   *q;
  register unsigned char last_status;
  register long block = 0L,
                next  = 0L;
  register short foundone = 0;
  char text[80];
  long now;

   
  z = &zone[zn - 1];                      /* point to zone                   */
  if (!z->zt_pl) return 0;                /* orphan zone - no pickline       */
  p = &pl[z->zt_pl - 1];                  /* point to pickline               */
  b = &bay[z->zt_first_bay - 1];          /* point to first bay              */
  
#ifdef DEBUG
  fprintf(DF, "zone_next_event()Start Zone=%d time=%d\n", 
    z->zt_zone, microclock() - stime);

  fprintf(DF, "  Zone=%d Picker=%d Status=%c Lines=%d Flags=%x\n",
  z->zt_zone, picker, z->zt_status, z->zt_lines, z->zt_flags);
#endif

  if (z->zt_lines) 
  {
#ifdef EXTRA_NEXT
    sprintf(text, "Z%d Has Lines %d", zn, z->zt_lines);
    krash("zone_next_event", text, 0);
#endif
    return 0;
  }
  z->zt_time = time(0);                   /* save zone start time            */

  switch (z->zt_status)
  {
    case ZS_UNDERWAY:                     /* these are not expected          */
    case ZS_EARLY:

#ifdef EXTRA_NEXT
      sprintf(text, "Z%d Bad Status %c", zn, z->zt_status);
      krash("zone_next_event", text, 0);
#endif
      return 0;

    case ZS_AHEAD:                        /* ahead only from automatic next  */
    case ZS_WAITING:                      /* these are picker requests       */
    case ZS_INACTIVE:
    case ZS_COMPLETE:
    case ZS_LOCKED:
     
#ifdef DELL
      next = z->zt_feeding;
/*      if (!next) next = zn + 1;   */
      
      if (next)
      {
        if (zone[next - 1].zt_queued >= zone_boxes[next - 1])
        {
          z->zt_status = ZS_OFFLINE;      /* special state                   */
          break;
        }
      }
#endif
      if (z->zt_flags & DemandFeed)       /* only scanner feeds an order     */
         {
           foundone = 0;
           if (z->zt_flags & IsSegmented)     /* segmented pickline F111794  */
              {
                 q = &oc->oc_tab[PicklineMax + z->zt_segment - 1];
                 block = q->oc_uw.oc_first;     
              }
           else
              {
                 q = &oc->oc_tab[z->zt_pl - 1];
                 block = q->oc_uw.oc_first;
              }

           while (block)                  /* search underway queue           */
                 {
                   o = &oc->oi_tab[block - 1];
                   if (o->oi_entry_zone && o->oi_entry_zone == z->zt_zone)
                      {
                        foundone = 1;
                        break;
                      }
                   block = o->oi_flink;
                 }

        if (foundone == 0)
           {
             z->zt_status = ZS_WAITING;
             break;
           }
      }
      if (find_next_order(z, picker))     /* attempt an order feed           */
      {
        finish_zone(z->zt_zone, z->zt_time);
/*
        if (z->zt_flags & DemandFeed)       
        {
          z->zt_status = ZS_WAITING;
          break;
        }
*/
        message_put(0, ZoneNextEvent, &z->zt_zone, sizeof(TZone));
        return 0;
      }
      break;
     
    case ZS_LATE:
     
      start_zone(z);                      /* feed the current order          */

      break;
          
    case ZS_OFFLINE:
          
      break;                              /* only redisplay message          */
  }
  if (sp->sp_productivity == 'y')
  {
    if (z->zt_zone <= pr->pr_zones && z->zt_status == ZS_AHEAD)
    {
      pz[z->zt_zone - 1].pr_zone_cum_ah_cnt += 1;
      pz[z->zt_zone - 1].pr_zone_cur_ah_cnt += 1;
    }
  }
#ifdef PICK_BY_ZONE
  pa_by_zone(z);
#endif

#ifdef TIMER
  if (z->zt_status == ZS_UNDERWAY)
  {
    fprintf(DF, "Time=%d\n", microclock() - stime);
  }
#endif

  message_put(po[b->bay_port - 1].po_id, ZoneStartRequest,
    &z->zt_zone, sizeof(TZone));
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Automatic Next When Preceding Zone Is Finished
 *-------------------------------------------------------------------------*/
automatic_next(zn, limit, block)
register long zn, limit, block;
{
  register struct zone_item *z;
  char text[40];
  long now;
   
#ifdef DEBUG
  fprintf(DF, "automatic_next() zn=%d limit=%d block=%d\n", 
    zn, limit, block);
#endif
  
  now = time(0);

  if (zn < 1 || zn >coh->co_zone_cnt) 
  {
    sprintf(text, "Z%d Invalid", zn);
    krash("automatic_next", text, 0);
    return 0;
  }
  while (zn && zn != limit)
  {
    z = &zone[zn - 1];               

    if (z->zt_status == ZS_AHEAD && (!z->zt_order || z->zt_order == block))
    {
      if (sp->sp_productivity == 'y') 
      {
        pl_productivity(z->zt_pl, now);
        zone_productivity(z->zt_zone, now);
      }
      z->zt_order  = z->zt_on = 0;
      z->zt_status = ZS_INACTIVE;
      z->zt_flags |= ZoneInactive;
      message_put(0, ZoneNextEvent, &z->zt_zone, sizeof(TZone));
    }
    zn = z->zt_feeding;
  }
  if (!zn) return 0;
  
  z = &zone[limit - 1];
  if (!(z->zt_flags & IsDummy))
  if (z->zt_status == ZS_UNDERWAY ||
      z->zt_status == ZS_LATE     ||
      z->zt_status == ZS_EARLY    ||
      z->zt_status == ZS_INACTIVE ||
      z->zt_status == ZS_WAITING  ||
      z->zt_status == ZS_OFFLINE) return 0;
  
  if (z->zt_flags & IsOffline) return 0;  /* F051294 - no feed into offline  */

  if (sp->sp_productivity == 'y') 
  {
    pl_productivity(z->zt_pl, now);
    zone_productivity(z->zt_zone, now);
  }
  z->zt_status = ZS_INACTIVE;
  z->zt_flags |= ZoneInactive;
  message_put(0, ZoneNextEvent, &z->zt_zone, sizeof(TZone));

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Find Next Order To Feed Into A Zone
 *-------------------------------------------------------------------------*/
find_next_order(z, picker)
register struct zone_item *z;
register long picker;
{
  register struct zone_item *y;
  register struct pl_item *p;
  register struct oi_item *o, *q;
  register long k, block, block2, ret;
  
  p = &pl[z->zt_pl - 1];
  
#ifdef DEBUG
  fprintf(DF, "find_next_order() Zone=%d Picker=%d\n",
    z->zt_zone, picker);
#endif

  z->zt_order = z->zt_on = 0;
  
  if (p->pl_flags & StopOrderFeed)
  {
    z->zt_status = ZS_LOCKED;
    return 0;
  }
  oc_lock();

  if (z->zt_flags & IsSegmented)          /* segmented pickline F111794      */
  {
    block = oc->oc_tab[PicklineMax + z->zt_segment - 1].oc_uw.oc_first;     
  }
  else block = oc->oc_tab[z->zt_pl - 1].oc_uw.oc_first;

  while (block)                           /* search underway queue           */
  {
    o = &oc->oi_tab[block - 1];
    if (o->oi_entry_zone && o->oi_entry_zone <= z->zt_zone) break;
    block = o->oi_flink;
  }
#ifdef DEBUG
  fprintf(DF, "find_next_order(): Zone=%d, block = %d, Zone Status = %c\n",
          z->zt_zone, block, z->zt_status);
  fflush(DF);
#endif

  if (block == 0 &&
      (z->zt_status == ZS_WAITING || z->zt_status == ZS_INACTIVE))
     { 
       if (!(z->zt_flags & FirstZone) || !(z->zt_flags & DemandFeed))
          {
            find_order(z);
            /* z->zt_status = ZS_COMPLETE; */
            /* z->zt_status = ZS_AHEAD; */
            oc_unlock();
#ifdef DEBUG
            fprintf(DF, "find_next_order(): Zone=%d, Status = %c\n",
                    z->zt_zone, z->zt_status);
            fflush(DF);
#endif
            return 0;
          }
     }

  if (block)                              /* next order in underway found    */
  {
    z->zt_status = ZS_AHEAD;              /* default zone status             */
    z->zt_order  = block;                 /* potential ahead order           */
    z->zt_on     = o->oi_on;              /* potential order number          */
    
    if (o->oi_flags & HOLD_TOTE)          /* go ahead until order released   */
    {
      oc_unlock();
      return 0;                           /* start zone as AHEAD             */
    }
    if (o->oi_entry_zone == z->zt_zone)   /* F010997 - fix late entry        */
    {
      ret = 0;                            /* initially start underway zone   */
      
      if (p->pl_flags & LateEntry)        /* is a late entry pickline        */
      {
        if (z->zt_flags & IsSegmented)    /* find primary index block        */
        {
          block2 = oc_find(z->zt_pl, z->zt_on); 
          q = &oc->oi_tab[block2 - 1];         
        }
        else q = o;                       /* is not segmented                */
        
        if (q->oi_le > 0)                 /* waiting for late entry          */
        {
          if (z->zt_zone >= q->oi_le)     /* this is late entry point        */
          {
            q->oi_le = 0;                 /* remove late entry flag          */
            z->zt_status = ZS_LATE;       /* start late entry zone           */
          }
          else ret = 1;                   /* skip before late entry          */
        }
      }
      if (z->zt_status != ZS_LATE) start_zone(z);
      if (z->zt_flags & Steering)         /* is a steering zone              */
      {
        if (z->zt_lines <= 0) ret = 1;    /* skip no pick zone               */
      }
      //if (z->zt_status != ZS_LATE) start_zone(z);
	// this is the steering fix this line is copied above the
	// steering flag is checked for zone.	-- July 14, Bob,Carl,Tom, ravi.
      
      check_save_segs();
      oc_unlock();
      return ret;
    }
    if (z->zt_flags & JumpZone)           /* zone will go ahead              */
    {
      block = find_jump_order(z);
      if (block)
      {
        oc_dequeue(block);
        oc_enqueue(block, o->oi_blink, OC_UW);
        oc->oi_tab[block - 1].oi_entry_zone = z->zt_zone;
        start_order(z, block, picker);
        if (z->zt_status != ZS_LATE) start_zone(z);
      }
      else z->zt_status = ZS_AHEAD;       /* ahead - not finished F051094    */
    }
    check_save_segs();
    oc_unlock();
    return 0;
  }
  if (pl[z->zt_pl - 1].pl_flags & OrdersLocked)  /* lock is active           */
  {
    z->zt_status = ZS_LOCKED;
    check_save_segs();
    oc_unlock();
    return 0;
  }
  if ((z->zt_flags & IsSegmented) && 
     !(z->zt_flags & FirstZone)   &&
     !(z->zt_flags & LateEntry)   &&      /* F010897 */
     !(z->zt_flags & JumpZone))           /* F010897 */
  {
    find_segment_order(z);                /* check AHEAD or COMPLETE         */
    check_save_segs();
    oc_unlock();
    return 0;
  }
  if (p->pl_flags & JumpZone) block = find_jump_order(z);
  else                        block = find_order(z);
       
  if (block <= 0)                         /* start AHEAD or COMPLETE         */
  {
    check_save_segs();
    oc_unlock();
    return 0;
  }
  o = &oc->oi_tab[block - 1];             /* new order to start              */

  oc_dequeue(block);

  if ((o->oi_flags & INHIBITED) && rf->rf_hold == 'i')
  {
    begin_work();
    od_get(block);
    of_rec->of_status = 'h';
    of_rec->of_datetime = time(0);
    od_update(block);
    commit_work();
    oc_enqueue(block, OC_LAST, OC_HOLD);
    z->zt_status = ZS_WAITING;
    z->zt_on     = o->oi_on;
    check_save_segs();
    oc_unlock();
    return 0;
  }
  o->oi_le = 0;                           /* insure is cleared               */

  if ((p->pl_flags & LateEntry) && !(z->zt_zone & JumpZone))
  {
    for (k = o->oi_entry_zone; k >= z->zt_zone; k--)
    {
      if (zone[k - 1].zt_flags & LateEntry)
      {
        o->oi_le = k;                     /* this is late entry point        */
        break;
      }
    }
  }
  oc_enqueue(block, OC_LAST, OC_UW);
  o->oi_entry_zone = z->zt_zone;
  start_order(z, block, picker);
  
  if (z->zt_flags & IsSegmented)          /* F111794                         */
  {
    o->oi_entry_zone = p->pl_last_zone;
    explode_order(z->zt_pl, block, 0);  
    z->zt_order = oc->oc_tab[PicklineMax + z->zt_segment - 1].oc_uw.oc_last;
    oc->oi_tab[z->zt_order - 1].oi_entry_zone = z->zt_zone; /* F010897 */
  }
  if (z->zt_status != ZS_LATE) start_zone(z);  /* not a jump start           */
  
  check_save_segs();
  oc_unlock();
  
  if (o->oi_le > 0) return 1;
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Start An Order Underway
 *-------------------------------------------------------------------------*/
start_order(z, block, picker)
register struct zone_item *z;
register long block, picker;
{
  register struct pl_item *p;
  TOrderEventMessage x;
  
#ifdef DEBUG
  fprintf(DF, "start_order: zone=%d block=%d time=%d\n",
    z->zt_zone, block, microclock() - stime);
#endif
  
  p = &pl[z->zt_pl - 1];
  
  if (p->pl_flags & EarlyExitModeNext)    /* fix exit next zone              */
  {
    if (oc->oi_tab[block - 1].oi_exit_zone < p->pl_last_zone)
    {
      oc->oi_tab[block - 1].oi_exit_zone += 1;
    }
  }
  begin_work();
  od_get(block);
  
  of_rec->of_status   = 'u';
  of_rec->of_datetime = time(0);
  of_rec->of_picker   = picker;

  od_update(block);
  commit_work();
        
  x.m_pickline = of_rec->of_pl;
  x.m_order    = of_rec->of_on;
  x.m_zone     = z->zt_zone;
  memcpy(x.m_grp, of_rec->of_grp, GroupLength);
  memcpy(x.m_con, of_rec->of_con, CustomerNoLength);

  message_put(0, OrderUnderwayEvent, &x, sizeof(TOrderEventMessage));

  z->zt_order = block;
  z->zt_on    = of_rec->of_on;

  pa_order_lines = of_rec->of_no_picks;
  pa_order_units = of_rec->of_no_units;

  if (sp->sp_labels == 'y') check_entry_labels(block);

  if (p->pl_flags & PendingOrderLock)
  {
    if (z->zt_on == p->pl_order) p->pl_flags |= OrdersLocked;
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Start An Order In A Zone
 *-------------------------------------------------------------------------*/
start_zone(z)
register struct zone_item *z;
{
  register struct pl_item  *p;
  register struct bay_item *b, *c;
  register struct hw_item  *h;
  register struct pw_item  *i;
  register struct oi_item  *o;
  TPickRequestMessage x;
  register long k, pon, mirror;
  TZone prev;
  
#ifdef DEBUG
fprintf(DF, "start_zone() zone=%d order=%d block=%d red=%d time=%d,",
    z->zt_zone, z->zt_on, z->zt_order, redisplay_count, microclock() - stime);
#endif

  z->zt_lines  = 0;                       /* clear anyway                    */
  z->zt_units  = 0;
  z->zt_status = ZS_UNDERWAY;             /* set status anyway               */
  
#ifdef DELL
  if (z->zt_queued > 0 && redisplay_count <= 0) 
  {
    z->zt_queued -= 1;                   /* reduce queued into zone          */
                                         /* message for conveyor gate drop   */
    
    message_put(0, BoxNextEvent, &z->zt_zone, sizeof(TZone));
    
    prev = z->zt_source;
    
    if (prev > 0)                        /* unlock previous zone             */
    {
      if (zone[prev - 1].zt_status == ZS_OFFLINE)
      {
        zone[prev - 1].zt_status = ZS_WAITING;
        message_put(0, ZoneNextEvent, &prev, sizeof(TZone));
      }
    }
  }
#endif
  
  if (z->zt_flags & IsMirror) disqualify(z);
  else 
  {
    if (sp->sp_mirroring == 'y' && (z->zt_flags & IsSegmented)) disqualify(z);
  }
  p = &pl[z->zt_pl - 1];                  /* point to pickline               */
  o = &oc->oi_tab[z->zt_order - 1];       /* point to order index            */
  
  if (p->pl_flags & (EarlyExitModeLast | EarlyExitModeNext | EarlyExit))   
  {
#ifdef DAYTIMER
    if (z->zt_zone >= o->oi_exit_zone) 
    {
      if (o->oi_pl != 2)              z->zt_status = ZS_EARLY;
      else if (!oc_find(3, o->oi_on)) z->zt_status = ZS_EARLY;
    }
#else
    if (z->zt_flags & EarlyExit)     /* F022895 */
    {
      if (z->zt_zone >= o->oi_exit_zone) z->zt_status = ZS_EARLY;
    }
#endif
  }
  if (z->zt_zone < o->oi_entry_zone) return 0;/* no picks can be found       */
  if (z->zt_zone > o->oi_exit_zone)  return 0;
  
  if (z->zt_flags & IsMirror) mirror = 1;
  else mirror = 0;

  pick_setkey(3);                         /* pl + on + zone                  */

  op_rec->pi_pl   = z->zt_pl;
  op_rec->pi_on   = z->zt_on;
  op_rec->pi_zone = z->zt_zone;

  pick_startkey(op_rec);

  //begin_work();
  while (!pick_next(op_rec, NOLOCK))
  {
#ifdef PICKS
    fprintf(DF, "pl=%d order=%d mod=%d flag=%d quan=%d time=%d\n",
      op_rec->pi_pl, op_rec->pi_on, op_rec->pi_mod, 
      op_rec->pi_flags, op_rec->pi_ordered, microclock() - stime);
#endif

    if (op_rec->pi_flags & (NO_PICK | PICKED)) continue;
    if (op_rec->pi_mod < 1) continue;
    
    x.m_pickline  = op_rec->pi_pl;
    x.m_order     = op_rec->pi_on;
    x.m_module    = op_rec->pi_mod;
    x.m_ordered   = op_rec->pi_ordered;
   // x.m_reference = op_rec->pi_reference;

#ifdef PICKS
  fprintf(DF, "pl=%d on=%d mod=%d quan=%d ref=%d\n",
    x.m_pickline, x.m_order, x.m_module, x.m_ordered, x.m_reference);
#endif

    if (sp->sp_um_in_pick_text == 'y')       /* unit of measure only       */
    {
      x.m_um = op_rec->pi_pick_text[0];
      x.m_case_pack = 1;
      x.m_count     = 0;
    }
    else if (sp->sp_um_in_pick_text == 'a')   /* um, cases, and count        */
    {
      x.m_um        = op_rec->pi_pick_text[0];
      x.m_case_pack = cvrt(&op_rec->pi_pick_text[1], 3);
      x.m_count     = cvrt(&op_rec->pi_pick_text[4], 2);

      if (x.m_case_pack < 1) x.m_case_pack = 1;
    }
    else                                      /* default is units            */
    {
      x.m_um        = 'U';
      x.m_case_pack = 1;
      x.m_count     = 0;
    }
    z->zt_lines += 1;
    z->zt_units += op_rec->pi_ordered;
      
    i   = &pw[x.m_module - 1];
    h   = &hw[i->pw_ptr  - 1];
    c   = &bay[h->hw_bay - 1];
    pon = po[c->bay_port - 1].po_id;
      
//	fprintf(DF,"pon - %d  bay %d \n",pon,c->bay_number);
    message_put(pon, ModulePickRequest, &x, sizeof(TPickRequestMessage));
  }
  //commit_work();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Disqualify Mirrored Segments
 *-------------------------------------------------------------------------*/
disqualify(z)
register struct zone_item *z;
{
  register struct pl_item *p;
  register struct seg_item *s, *t;
  register long k, block;
  
  s = &sg[z->zt_segment - 1];              /* pointer to segment             */
  if (s->sg_first_zone != z->zt_zone) return 0;

  p = &pl[z->zt_pl - 1];
  
  t = &sg[p->pl_first_segment - 1];
  
  for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, t++)
  {
    if (t->sg_snode != s->sg_snode) continue;
    if (t->sg_segment == s->sg_segment) continue;
    
    block = oc_find(PicklineMax + k, z->zt_on);
    if (block < 1) continue;
    oc->oi_tab[block - 1].oi_entry_zone = 0;
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Finish A Zone - Underway and Early Exit Zone Only.
 *-------------------------------------------------------------------------*/
finish_zone(zn, now)
register long zn;
register long now;
{
  register struct zone_item *z;
  register struct oi_item *o;
  register struct seg_item *s;
  register long next, limit, order, block, flag;
  char text[80];
  TOrderEventMessage x;
#ifdef GTE
  TBoxOrderMessage y;       /* F060798 */
#endif
  long picktime = 0L;
	
  
  if (zn < 1 || zn >coh->co_zone_cnt)
  {
    sprintf(text, "Z%d Invalid", zn);
    krash("finish_zone", text, 0);
    return 0;
  }
  z = &zone[zn - 1];

#ifdef DEBUG
  fprintf(DF, "finish_zone: zone=%d order=%d block=%d\n",
  z->zt_zone, z->zt_on, z->zt_order);
#endif

  if (z->zt_order < 1)                    /* no order is active              */
  {
    sprintf(text, "Z%d Has No Order", zn);
    krash("finish_zone", text, 0);
    return 0;
  }
  if (z->zt_lines) 
  {
    sprintf(text, "Z%d Has %d Lines", zn, z->zt_lines);
    krash("finish_zone", text, 0);
    return 0;
  }
  o = &oc->oi_tab[z->zt_order - 1];       /* point to order index            */
  
  if (z->zt_zone != o->oi_entry_zone)
  {
    sprintf(text, "Z%d Order %05d Entry Z%d", zn, z->zt_on, o->oi_entry_zone);
    krash("finish_zone", text, 0);
  }
  z->zt_count++;                          /* count orders through zone       */


  if (sp->sp_productivity == 'y')         /* count order completed           */
  {
    if (z->zt_zone <= pr->pr_zones)
    {
      pl_productivity(z->zt_pl, now);
      zone_productivity(z->zt_zone, now);

      pz[z->zt_zone - 1].pr_zone_cum_orders += 1;
      pz[z->zt_zone - 1].pr_zone_cur_orders += 1;
    }
  }
  limit = next = o->oi_entry_zone = z->zt_feeding;
  
#ifdef DELL
  if (next) zone[next - 1].zt_queued += 1;
  else      zone[zn].zt_queued       += 1;
#endif

#ifdef DAYTIMER
  if (z->zt_zone >= o->oi_exit_zone) z->zt_status = ZS_EARLY;
#endif

  if (z->zt_flags & IsSegmented)                 /* F111794                  */
  {
    if (z->zt_status == ZS_EARLY)                /* end of a order           */
    {
      s = &sg[z->zt_segment - 1];                /* pointer to segment       */

      z->zt_order = oc_find(z->zt_pl, z->zt_on); 

      implode_order(z->zt_pl, s->sg_enode, z->zt_on);  /* delete order       */
    }                                            /* end !next                */
    else if (!next)                              /* end of a segment         */
    {
      s = &sg[z->zt_segment - 1];                /* pointer to segment       */

#ifdef INORDER

      if (check_segment_done(z->zt_pl, s->sg_enode, z->zt_on))
      {
        z->zt_order = oc_find(z->zt_pl, z->zt_on);

        if (s->sg_enode == 99)
        {
          implode_order(z->zt_pl, s->sg_enode, z->zt_on);  /* delete order   */
        }
        else
        {
          flag = 0;
          
          while (check_segment_first_done(z->zt_pl, s->sg_enode))
          {  
            block = oc->oc_tab[PicklineMax + z->zt_segment - 1].oc_uw.oc_first;
            z->zt_on = oc->oi_tab[block - 1].oi_on;
            z->zt_order = oc_find(z->zt_pl, z->zt_on);
            
            implode_order(z->zt_pl, s->sg_enode, z->zt_on);
            explode_order(z->zt_pl, z->zt_order, s->sg_enode);
            
            flag = 1;
          }                                      /* end while                */
          if (flag) automatic_segment_next(z);   /* start feeding zone       */

          z->zt_order  = z->zt_on = 0;              
          z->zt_status = ZS_INACTIVE;
          return 0;
        }                                    
      }
      else                                       /* order not done           */
      {
        z->zt_order  = z->zt_on = 0;            
        z->zt_status = ZS_INACTIVE;
        return 0;
      }

#else
      if (check_segment_done(z->zt_pl, s->sg_enode, z->zt_on))
      {
        z->zt_order = oc_find(z->zt_pl, z->zt_on);

        implode_order(z->zt_pl, s->sg_enode, z->zt_on);/* delete order       */
      
        if (s->sg_enode != 99)                   /* end of pickline          */
        {
          explode_order(z->zt_pl, z->zt_order, s->sg_enode);

          automatic_segment_next(z);             /* start feeding zones      */

          z->zt_order  = z->zt_on = 0;              
          z->zt_status = ZS_INACTIVE;
          return 0;
        }
      }
      else                                       /* order not done           */
      {
        z->zt_order  = z->zt_on = 0;            
        z->zt_status = ZS_INACTIVE;
        return 0;
      }
#endif
    }
  }                                              /* end IsSegmented          */
  if (z->zt_status == ZS_EARLY || !next)         /* this order is done       */
  {
    oc_lock();
    oc_dequeue(z->zt_order);
    oc_enqueue(z->zt_order, OC_LAST, OC_COMPLETE);
    begin_work();
    od_get(z->zt_order);
    of_rec->of_status   = 'c';
    of_rec->of_elapsed += (now - of_rec->of_datetime);
    of_rec->of_datetime = now;
    if (sp->sp_pa_count) pa_completion(PA_CURRENT | PA_PICKED);
    if (sp->sp_labels == 'y') check_exit_labels();
#ifdef GTE
    if (sp->sp_box_feature == 's')   /* scanned box number GTE    */
    {
      if (!(oc->oi_tab[z->zt_order - 1].oi_flags & NEED_BOX))   /* have a box */
      {
	y.m_pickline = of_rec->of_pl;           /* F060798                                                                */
	y.m_order        = of_rec->of_on;
	y.m_last         = 1;	/* this flag is set for last box in a order */
				// ravi
	memcpy(text, oc->oi_tab[z->zt_order - 1].oi_box, BoxNoLength);
	text[BoxNoLength] = 0;
	y.m_box = atol(text);
#ifdef DEBUG
        fprintf(DF, "last=%d box=%d\n",
                y.m_last, y.m_box);
        fflush(DF);
#endif
	message_put(0, BoxCloseEvent, &y, sizeof(TBoxOrderMessage));
      }
    }
#endif

    x.m_pickline = of_rec->of_pl;
    x.m_order    = of_rec->of_on;
    x.m_zone     = z->zt_zone;
    memcpy(x.m_grp, of_rec->of_grp, GroupLength);
    memcpy(x.m_con, of_rec->of_con, CustomerNoLength);

    if (of_rec->of_repick == 'y')
    {
      message_put(0, OrderRepickEvent, &x, sizeof(TOrderEventMessage));
    }
    else
    {
      message_put(0, OrderCompleteEvent, &x, sizeof(TOrderEventMessage));
    }
    od_update(z->zt_order);
    commit_work();
    check_save_segs();
    oc_unlock();
  
    pl[z->zt_pl - 1].pl_complete += 1;
  
    if (sp->sp_productivity == 'y')
    {
      if (z->zt_pl <= pr->pr_picklines)
      {
        pp[z->zt_pl - 1].pr_pl_cur_completed += 1;
        pp[z->zt_pl - 1].pr_pl_cum_completed += 1;
      }
    }
    limit = 0;                             /* until end of pickline          */
  }
  else                                     /* next is NEVER zero here        */
  {
    if (zone[next - 1].zt_flags & Steering) 
    {
      limit = automatic_steering(z, o);
    }
  }
  if (next)
  {
    automatic_next(next, limit, z->zt_order);
  }
  z->zt_order  = z->zt_on = 0;

  z->zt_status = ZS_INACTIVE;
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Find Limit of Steering - Last Zone of Pickline Is NEVER Steering
 *-------------------------------------------------------------------------*/
automatic_steering(z, o)
register struct zone_item *z;
register struct oi_item *o;
{
  register struct hw_item *h;
  register struct pw_item *i;
  register struct bay_item *b;
  register long next, next_pick, nextz, limit;
  
#ifdef DEBUG
  fprintf(DF, "automatic_steering() zone=%d order=%d block=%d time=%d\n", 
    z->zt_zone, z->zt_on, z->zt_order, microclock() - stime);
#endif

  next = z->zt_feeding;                  /* next zone after finished zone   */

  if (!next) return 0;                    /* no next zone - impossible ???   */
  z = &zone[next - 1];                    /* next zone to feed               */

  if (!(z->zt_flags & Steering)) return next;  /* no point in checking       */

  b = &bay[z->zt_first_bay - 1];

  pick_setkey(3);
  op_rec->pi_pl   = o->oi_pl;
  op_rec->pi_on   = o->oi_on;
  op_rec->pi_zone = next;
  
  pick_startkey(op_rec);

  op_rec->pi_zone = coh->co_zone_cnt;
  pick_stopkey(op_rec);
    
  //begin_work();
  if (pick_next(op_rec, NOLOCK))
  {
    //commit_work();
    return next;
  }
  //commit_work();
  
  i = &pw[op_rec->pi_mod - 1];
  h = &hw[i->pw_ptr - 1];
  next_pick = bay[h->hw_bay - 1].bay_zone;

  oc_lock();
  if (o->oi_blink) limit = oc->oi_tab[o->oi_blink - 1].oi_entry_zone;
  else             limit = pl[z->zt_pl - 1].pl_last_zone;

#ifdef DEBUG
  fprintf(DF, "next=%d next_pick=%d limit=%d\n", next, next_pick, limit);
#endif

  while (next && next < limit && next < next_pick)
  {
    z = &zone[next - 1];
    if (!(z->zt_flags & Steering)) break;
    next = z->zt_feeding;
  }
  o->oi_entry_zone = next;
  oc_unlock();

  return next;
}
/*-------------------------------------------------------------------------*
 *  Find An Order Which MAY Go Underway  (No Jump Or Late Entry)
 *
 *  returns block  -  an order may be fed.
 *  returns 0      -  no order may be fed.   
 *-------------------------------------------------------------------------*/
find_order(z)
register struct zone_item *z;
{
  register long block, k;
  
#ifdef DEBUG
  fprintf(DF, "find_order() Zone=%d time=%d\n", 
    z->zt_zone, microclock() - stime);
#endif

  for (k = OC_HIGH; k <= OC_LOW; k++)     /* over all input queues           */
  {
    block  = oc->oc_tab[z->zt_pl - 1].oc_queue[k].oc_first;
  
    if (block)
    {
      if (z->zt_flags & FirstZone)        /* first zone feed only            */
      {
        z->zt_status = ZS_UNDERWAY;
        return block;
      }
      z->zt_status = ZS_AHEAD;            /* order are in queue              */
      return 0;
    }
  }

  z->zt_status = ZS_COMPLETE;
  return 0;                               /* queues are empty                */
}
/*-------------------------------------------------------------------------*
 *  Find a Jump Order Which MAY Go Underway  
 *
 *  returns block  -  an order may be fed.
 *  returns 0      -  no order may be fed.   
 *-------------------------------------------------------------------------*/
find_jump_order(z)
register struct zone_item *z;
{
  register struct oi_item *o;
  register long block, next, k, count;
  
#ifdef DEBUG
  fprintf(DF, "find_jump_order() Zone=%d time=%d\n",
    z->zt_zone, microclock() - stime);
#endif
  
  count = 0;                              /* count available orders          */
  
  for (k = OC_HIGH; k <= OC_LOW; k++)     /* over all input queues           */
  {
    next = oc->oc_tab[z->zt_pl - 1].oc_queue[k].oc_first;
    
    while (next)
    {
      block = next;                       /* next in queue                   */
      o = &oc->oi_tab[block - 1];         /* point to index entry            */
      next = o->oi_flink;                 /* next after                      */
     
      if (o->oi_entry_zone > z->zt_end_section)   continue;
      if (o->oi_exit_zone  < z->zt_start_section) continue;
      
      count++;                            /* count orders which will fed zone*/
      
      if (o->oi_entry_zone < z->zt_start_section) continue;

      if (z->zt_flags & (JumpZone | FirstZone))  /* must be an entry zone    */
      {
        if (z->zt_flags & FirstZone)      /* first zone in pickline          */
        {
          z->zt_status = ZS_UNDERWAY;     /* first zone entry                */
          return block;
        }
        z->zt_status = ZS_LATE;           /* jump zone entry                 */
        return block;                     /* block to start                  */
      }
      else                                /* not a jump zone                 */
      {
        z->zt_status = ZS_AHEAD;          /* something will feed zone        */
        return 0;
      }
    }
  }
  if (count) z->zt_status = ZS_AHEAD;      /* some orders yet to feed         */
  else z->zt_status = ZS_COMPLETE;        /* nothing will feed               */
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check AHEAD or COMPLETE
 *-------------------------------------------------------------------------*/
find_segment_order(z)
register struct zone_item *z;
{
  register struct pl_item  *p;
  register struct seg_item *s;
  register long k, entry;
  
#ifdef DEBUG
  fprintf(DF, "find_segment_order() Zone=%d\n", z->zt_zone);
#endif

  p = &pl[z->zt_pl - 1];                   /* pointer to pickline            */
  
  z->zt_order = z->zt_on = 0;              /* clear initially                */
  entry = sg[z->zt_segment - 1].sg_snode;  /* entry node                     */

  s = &sg[p->pl_first_segment - 1];        /* pointer to segment             */

  for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, s++)
  {
    if (s->sg_enode != entry) continue;
    z->zt_order = oc->oc_tab[PicklineMax + k - 1].oc_uw.oc_first;
    if (!z->zt_order) continue;

    z->zt_on = oc->oi_tab[z->zt_order - 1].oi_on;
    z->zt_status = ZS_AHEAD;
    return 0;
  }
  if (z->zt_flags & IsMirror)
  {
    z->zt_status = ZS_COMPLETE;
    return 0;
  }
  for (k = OC_HIGH; k <= OC_LOW; k++)
  {
    z->zt_order = oc->oc_tab[z->zt_pl - 1].oc_queue[k].oc_first;
    if (!z->zt_order) continue;

    z->zt_on = oc->oi_tab[z->zt_order - 1].oi_on;
    z->zt_status = ZS_AHEAD;
    return 0;
  }
  z->zt_status = ZS_COMPLETE;
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Order Done In Segment 
 *
 *  1. Searches all segments of a pickline.
 *  2. Check for end node == node.
 *  3. Check specified order is done (i.e. entry_zone == 0).
 *  4. Return order number.
 *-------------------------------------------------------------------------*/
check_segment_done(pline, node, what)
register long pline, node, what;
{
  register struct pl_item  *p;
  register struct seg_item *s;
  register struct oi_item  *o;
  register long k, block, order;

#ifdef DEBUG
  fprintf(DF, "check_segment_done() PL=%d Node=%d Order=%d\n", 
    pline, node, what);
#endif

  p = &pl[pline - 1];                       /* pointer to pickline           */
  s = &sg[p->pl_first_segment - 1];         /* pointer to segment            */
  
  order = 0;                                /* initially not found           */
  
  for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, s++)
  {
    if (s->sg_enode != node) continue;      /* end is not subject node       */

    block = oc_find(PicklineMax + k, what); /* 04/04/96 find in queue        */

    if (!block) return 0;                   /* nothing in queue              */

    o = &oc->oi_tab[block - 1];             /* pointer to index entry        */

    if (o->oi_entry_zone) return 0;         /* order is not done             */

    order = o->oi_on;
    if (what && order != what) return 0;    /* specific order not found      */
  }
  return order;
}
/*-------------------------------------------------------------------------*
 *  Check First Order Done In Segment 
 *
 *  1. Searches all segments of a pickline.
 *  2. Check for end node == node.
 *  3. Check first order is done (i.e. entry_zone == 0).
 *  4. Return order number.
 *-------------------------------------------------------------------------*/
check_segment_first_done(pline, node)
register long pline, node;
{
  register struct pl_item  *p;
  register struct seg_item *s;
  register struct oi_item  *o;
  register long k, block;

#ifdef DEBUG
  fprintf(DF, "check_segment_first_done() PL=%d Node=%d\n", 
    pline, node);
#endif

  p = &pl[pline - 1];                       /* pointer to pickline           */
  s = &sg[p->pl_first_segment - 1];         /* pointer to segment            */
  
  for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, s++)
  {
    if (s->sg_enode != node) continue;      /* end is not subject node       */

    block = oc->oc_tab[PicklineMax + k - 1].oc_uw.oc_first;       

    if (!block) return 0;                   /* nothing in queue              */

    o = &oc->oi_tab[block - 1];             /* pointer to index entry        */

    if (o->oi_entry_zone) return 0;         /* order is not done             */
  }
  return 1;                                 /* first order is done           */
}


/*-------------------------------------------------------------------------*
 *  Explode Order In All Segment With Starting Node
 *-------------------------------------------------------------------------*/
explode_order(pline, block, node)
register long pline, block, node;
{
  register struct pl_item   *p;
  register struct zone_item *z;
  register struct seg_item  *s;
  register struct oi_item   *o, *q;
  register long j, k, offset;
  
#ifdef DEBUG
  fprintf(DF, "explode_order() PL=%d Block=%d Node=%d\n", 
    pline, block, node);
#endif

  p = &pl[pline - 1];                       /* pointer to pickline           */
  q = &oc->oi_tab[block - 1];               /* primary record                */
  s = &sg[p->pl_first_segment - 1];         /* pointer to segment            */

  for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, s++)
  {
    if (s->sg_snode != node) continue;      /* does not feed here            */

    block = oc_write(PicklineMax + k, q->oi_on);

    if (block < 1) krash("explode_order", "Order Index Full", 1);

    o = &oc->oi_tab[block - 1];
  
    memcpy(o->oi_grp, q->oi_grp, GroupLength);
    o->oi_entry_zone = s->sg_first_zone;
    o->oi_exit_zone  = s->sg_last_zone;
    o->oi_le         = 0;
    
    oc_enqueue(block, OC_LAST, OC_UW);
    
    if (!s->sg_below && s->sg_last_zone > q->oi_exit_zone &&      /* F070896 */
        q->oi_entry_zone >= s->sg_first_zone)                     /* F072696 */
    {
      offset = s->sg_last_zone - q->oi_exit_zone;  /* EE zone from end       */
    }
  }
/*
 *  Determine Early Exit Zone In Each Branch - F070896
 */
  if (p->pl_flags & (EarlyExitModeLast | EarlyExitModeNext | EarlyExit))
  {
    if (p->pl_flags & EarlyExitModeNext) offset -= 1;
    
    if (offset <= 0) return 0;             /* not actually early exit        */
    
    s = &sg[p->pl_first_segment - 1];      /* pointer to segment             */
    
    for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, s++)
    {
      if (s->sg_snode != node) continue;   /* does not feed here             */
      
      block = oc->oc_tab[PicklineMax + k - 1].oc_uw.oc_last;
      
      o = &oc->oi_tab[block - 1];
      
      o->oi_exit_zone -= offset;
      
      for (j = 0, z = &zone[o->oi_exit_zone - 1]; j < offset; j++, z++)
      {
        if (z->zt_flags & EarlyExit) break;  /* find an exit zone            */
        o->oi_exit_zone += 1;
      }
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Implode Order In Prior Segment Branches
 *-------------------------------------------------------------------------*/
implode_order(pline, node, what)
register long pline, node, what;
{
  register struct pl_item  *p;
  register struct seg_item *s;
  register long k, block;

#ifdef DEBUG
  fprintf(DF, "implode_order() PL=%d Node=%d Order=%d\n",
    pline, node, what);
#endif

  p = &pl[pline - 1];                       /* pointer to pickline           */
  s = &sg[p->pl_first_segment - 1];         /* pointer to segment            */
  
  for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, s++)
  {
    if (s->sg_enode != node) continue;      /* end is not subject node       */

    block = oc_find(PicklineMax + k, what); /* 04/04/96 find in queue        */

    if (!block) continue;                   /* nothing in queue              */

    oc_dequeue(block);
    oc_delete(block);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Automatic Next In Following Branches
 *-------------------------------------------------------------------------*/
automatic_segment_next(z)
register struct zone_item *z;
{
  register struct seg_item *s;
  register long n, now;
  
#ifdef DEBUG
  fprintf(DF, "automatic_segment_next() Zone=%d\n", z->zt_zone);
#endif

  n = z->zt_segment;
  n = sg[n - 1].sg_next;
  
  while (n)
  {
    s = &sg[n - 1];
    z = &zone[s->sg_first_zone - 1];               

    if (z->zt_status == ZS_AHEAD)
    {
      if (sp->sp_productivity == 'y') 
      {
        now = time(0);
        pl_productivity(z->zt_pl, now);
        zone_productivity(z->zt_zone, now);
      }
      z->zt_order = z->zt_on = 0;
      message_put(0, ZoneNextEvent, &z->zt_zone, sizeof(TZone));
    }
    n = sg[n - 1].sg_below;
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Initialize Request To Ports
 *-------------------------------------------------------------------------*/
port_initialize_request()
{
  register long k;
  long pid, status;
  char name[32];
  
#ifdef DEBUG
  fprintf(DF, "port_initialize_request()\n");
#endif

  if (sp->sp_in_process_status != 'x')
  {
    return message(LOCAL_MSG, "System Currently Busy");
  }
  if (sp->sp_running_status == 'y') return message(ERR_IS_CONFIG, 0);

  sp->sp_in_process_status = 'i';
  sp->sp_init_status = 'n';
  ss_save();
  
  for (k = event_count = 0; k < coh->co_ports; k++)
  {
    if (po[k].po_flags & IsFullFunction)       event_count++;
    else if (po[k].po_flags & IsBasicFunction) event_count++;
    else if (po[k].po_flags & IsTotalFunction) event_count++;
    else if (po[k].po_flags & IsDummy)         event_count++;
    else continue;
  
    sprintf(name, "%s.%s", hw_name, basename(po[k].po_name));
    unlink(name);
  }
  requestor = who;                        /* task asking for action          */
  event_status = 0;                       /* mark action successful          */
  event_start  = time(0);                 /* initialize start time           */
  
  if (event_count)
  {
    message_put(0, PortInitializeRequest, 0, 0);

    signal(SIGALRM, port_timeout);        /* watchdog timer                  */
    alarm(RP_TIMEOUT);                    /* timeout on initialize           */
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Markplace Request To Ports
 *-------------------------------------------------------------------------*/
port_markplace_request()
{
  register struct zone_item *z;
  register long k;
  long pid, status;
  char text[80];
  
#ifdef DEBUG
  fprintf(DF, "port_markplace_request()\n");
#endif

  if (sp->sp_in_process_status != 'x')
  {
    return message(LOCAL_MSG, "System Currently Busy");
  }
  if (sp->sp_running_status != 'y' || sp->sp_config_status != 'y')
  {
    return message(ERR_NO_CONFIG,0);
  }
  if (sp->sp_box_full == 'y')
  {
    for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
    {
      if (!(z->zt_flags & IsBasicFunction)) continue;
      if (z->zt_flags & BoxOperation)
      {
        sprintf(text, "Box Action In Process - Zone %d", z->zt_zone);
        return message(CRASH_MSG, text);
      }
    }
  }
  sp->sp_in_process_status = 'm';
  sp->sp_init_status = 'n';
  ss_save();
  
  for (k = event_count = 0; k < coh->co_ports; k++)
  {
    if (po[k].po_flags & IsFullFunction)       event_count++;
    else if (po[k].po_flags & IsBasicFunction) event_count++;
    else if (po[k].po_flags & IsTotalFunction) event_count++;
    else if (po[k].po_flags & IsDummy)         event_count++;
  }
  requestor = who;                        /* task asking for action          */
  event_status = 0;                       /* mark action successful          */
  event_start  = time(0);                 /* start time                      */
  
  if (event_count)
  {
    message_put(0, PortMarkplaceRequest, 0, 0);

    signal(SIGALRM, port_timeout);        /* watchdog timer                  */
    alarm(MP_TIMEOUT);                    /* timeout on initialize           */
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Restoreplace Request To Ports
 *-------------------------------------------------------------------------*/
port_restoreplace_request()
{
  register long k;
  long pid, status;
  char name[32];
  FILE *fd;
  
#ifdef DEBUG
  fprintf(DF, "port_restoreplace_request()\n");
#endif

  if (sp->sp_in_process_status != 'x')
  {
    return message(LOCAL_MSG, "System Currently Busy");
  }
  if (sp->sp_running_status == 'y') return message(ERR_IS_CONFIG, 0);
  if (sp->sp_config_status == 'n')  return message(ERR_NO_CONFIG, 0);

  sp->sp_in_process_status = 'n';
  sp->sp_init_status = 'n';
  ss_save();
  
  for (k = event_count = 0; k < coh->co_ports; k++)
  {
    if (po[k].po_flags & IsFullFunction)       event_count++;
    else if (po[k].po_flags & IsBasicFunction) event_count++;
    else if (po[k].po_flags & IsTotalFunction) event_count++;
    else if (po[k].po_flags & IsDummy)         event_count++;
    else continue;
  }
  for (k = 0; k < coh->co_ports; k++)
  {
    sprintf(name, "%s.%s", hw_name, basename(po[k].po_name));
    unlink(name);
  }
  requestor = who;                        /* task asking for action          */
  event_status = 0;                       /* mark action successful          */
  event_start  = time(0);                 /* start time                      */
  
  message_put(0, PortInitializeRequest, 0, 0);

  signal(SIGALRM, port_timeout);          /* watchdog timer                  */
  alarm(RP_TIMEOUT);                      /* timeout on initialize           */
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Timeout On Port Events
 *-------------------------------------------------------------------------*/
void port_timeout(int signum)
{
  if (event_count <= 0) return; // 0;         /* ignore anyway - no events       */
  who = requestor;                        /* send message to requestor       */

  sp->sp_in_process_status = 'x';         /* mark nothing running            */

  message(LOCAL_MSG, "CAPS Is Not Responding");

  event_count = 0;                        /* ignore any more events          */
  //return 0;
}
/*-------------------------------------------------------------------------*
 *  Initialize/Restoreplace Event From A Port
 *-------------------------------------------------------------------------*/
port_initialize_event()
{
  register struct zone_item *z;
  register long k;
  
#ifdef DEBUG
  fprintf(DF, "port_initialize_event() count=%d in_process=%c time=%d\n",
    event_count, sp->sp_in_process_status, time(0) - event_start);
#endif

  if (event_count <= 0) return 0;         /* ignore anything now             */

  event_count--;                          /* reduce event count              */
  
  if (event_count) return 0;
  alarm(0);
  
  for (k = event_status = 0; k < coh->co_ports; k++)
  {
#ifdef DEBUG
  fprintf(DF, "port=%d %s status=%c\n", k, po[k].po_name, po[k].po_status);
#endif

    if (po[k].po_status == 'x') event_status++;
  }
  who = requestor;

  if (sp->sp_in_process_status == 'i')    /* initialize                      */
  {
    sp->sp_in_process_status = 'x';

    if (event_status)   {mark_all_ports(); return message(ERR_INIT, 0);}
    if (call_hw_init()) {mark_all_ports(); return message(ERR_INIT, 0);}

    sp->sp_init_status = 'y';
    ss_save();
     
    message_put(0, InitializeEvent, 0, 0);
  }
  else if (sp->sp_in_process_status == 'n')/* restoreplace                   */
  {
    sp->sp_in_process_status = 'x';

    if (event_status) return message(ERR_RP, "See Printer");

    message_put(0, InitializeEvent, 0, 0);

    if (call_rp_init())
    {
      mark_all_ports(); return message(ERR_RP, "See Printer");
    }
    if (call_st_init())
    {
      mark_all_ports(); return message(ERR_RP, "See Printer");
    }
    sp->sp_init_status       = 'y';
    sp->sp_config_status     = 'y';
    sp->sp_running_status    = 'y';
    ss_save();

    redisplay_count = 0;                    /* insure is zero here           */
    redisplay_request = RestoreplaceRequest;/* redisplay for restoreplace    */
    
    for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
    {
      if (z->zt_flags & (IsBasicFunction | IsTotalFunction))
      {
        zone_redisplay_request(z->zt_zone);
      }
    }
    if (redisplay_count <= 0)               /* F050694 - restoreplace event  */
    {
      message_put(0, RestoreplaceEvent, 0, 0);
      if (sp->sp_pa_count) pa_open();
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Markplace Event From A Port
 *-------------------------------------------------------------------------*/
port_markplace_event()
{
  register long k;
  
  event_count--;
  if (event_count) return 0;
  alarm(0);

  sp->sp_init_status       = 'n';
  sp->sp_config_status     = 'y';
  sp->sp_running_status    = 'n';
  sp->sp_in_process_status = 'x';
  
  if (sp->sp_pa_count) pa_close();
  save_segs();
 
  message_put(0, MarkplaceEvent, 0, 0);

  return 0;
}

/*-------------------------------------------------------------------------*
 *  Complete Initialization
 *-------------------------------------------------------------------------*/
call_hw_init()
{
  long pid = 0, status = 0;
  char text[80];
#ifdef DEBUG
  fprintf(DF, "call_hw_init()\n");
#endif
	 
  if ((pid = fork()) == 0)
  {
    close_all();
    setpgrp();
    execlp("hw_init", "hw_init", "-m", 0);
    krash("call_hw_init", "load hw_init", 0);
    exit(1);
  }
  pid = wait(&status);
  if (pid <= 0 || status)
  {
    sprintf(text, "hw_init pid=%d status=0x%04x", pid, status);
    krash("call_hw_init", text, 0);
  }


#ifdef DEBUG
  fprintf(DF, "after call_hw_init()\n");
#endif

  return status;
}
/*-------------------------------------------------------------------------*
 *  Complete Restoreplace
 *-------------------------------------------------------------------------*/
call_rp_init()
{
  long pid, status;
  char text[80];
  
#ifdef DEBUG
  fprintf(DF, "call_rp_init()\n");
#endif

  if ((pid = fork()) == 0)
  {
    close_all();
    setpgrp();
    execlp("rp_init", "rp_init", "-m", 0);
    krash("call_rp_init", "load rp_init", 0);
    exit(1);
  }
  pid = wait(&status);
  
  if (pid <= 0 || status)
  {
    sprintf(text, "rp_init pid=%d status=0x%04x", pid, status);
    krash("call_rp_init", text, 0);
  }
  return status;
}
/*-------------------------------------------------------------------------*
 *  Complete SKU Initiailize
 *-------------------------------------------------------------------------*/
call_st_init()
{
  long pid, status;
  char text[80];
  
#ifdef DEBUG
  fprintf(DF, "call_st_init()\n");
#endif

  if (rf->rf_sku && sp->sp_sku_support == 'y')  /* F070896 */
  {
    if ((pid = fork()) == 0)
    {
      close_all();
      setpgrp();
      execlp("st_init", "st_init", "-m", 0);
      krash("call_st_init", "load st_init", 0);
      exit(1);
    }
    pid = wait(&status);

    if (pid <= 0 || status)
    {
      sprintf(text, "st_init pid=%d status=0x%04x", pid, status);
      krash("call_st_init", text, 0);
    }
    return status;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Configure Request
 *-------------------------------------------------------------------------*/
configure_request(buf, len)
char *buf;
long len;
{
  register struct zone_item *z;
  register struct pl_item *p;
  register long j, k, last_entry, end_line;
  char config_name[40], text[80];
  long pid, status;

  if (sp->sp_in_process_status != 'x')    /* something already running       */
  {
    return message(LOCAL_MSG, "System Currently Busy");
  }
  if (sp->sp_init_status == 'n')
  {
    return message(LOCAL_MSG, "Not Initialized");
  }
  if (sp->sp_running_status == 'y') return message(ERR_IS_CONFIG, 0);
  
  memcpy(config_name, buf, len);
  config_name[len] = 0;
  
  if ((pid = fork()) == 0)
  {
    close_all();
    setpgrp();
    execlp("configure", "configure", config_name, "-m", 0);
    krash("configure", "load configure", 0);
    exit(1);
  }
  pid = wait(&status);

  if (pid <= 0 || status)
  {
    sprintf(text, "configure %s pid=%d status=0x%04x", 
      config_name, pid, status);
    krash("configure_request", text, 0);
  }
  if (status)         {mark_all_ports(); return message(ERR_CONFIG, 0);}
  if (call_st_init()) {mark_all_ports(); return message(ERR_CONFIG, 0);}

  sp->sp_config_status     = 'y';
  sp->sp_running_status    = 'y';
  sp->sp_in_process_status = 'x';
  ss_save();

  message_put(0, ConfigureEvent, config_name, strlen(config_name));
  
  if (sp->sp_pa_count) pa_open();

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Mark All Ports Off
 *-------------------------------------------------------------------------*/
mark_all_ports()
{
  register long k;
  
  for (k = 0; k < coh->co_port_cnt; k++) po[k].po_status = 'x';
}
/*-------------------------------------------------------------------------*
 *  System Recovery Event
 *-------------------------------------------------------------------------*/
system_recovery()
{
  register struct bay_item *b;
  register struct zone_item *z;
  register long k, now;
  
#ifdef DEBUG
  fprintf(DF, "system_recovery\n");
#endif

  now = time(0);
  
  for (k = 0; k < coh->co_pl_cnt; k++) 
  {
#ifdef DELL
    count_queued(k + 1);
#endif
    pl[k].pl_flags &= ~StopOrderFeed;
  }
  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (!z->zt_pl) continue;
    z->zt_time = now;

    if (z->zt_flags & JumpZone) continue;

    if (z->zt_flags & FirstZone)          /* first zone in pickline          */
    {
      if (pl[z->zt_pl - 1].pl_flags & HasBadgeReader)
      {
        find_next_order(z, 0);            /* try to feed first zone          */
        b = &bay[z->zt_first_bay - 1];
        message_put(po[b->bay_port - 1].po_id, ZoneStartRequest,
          &z->zt_zone, sizeof(TZone));
        continue;
      }
    }
    zone_next_event(z->zt_zone, 0);     /* try to feed the zone            */

    if (z->zt_status == ZS_LOCKED)        /* F060194 set locked to waiting   */
    {
      z->zt_status = ZS_WAITING;      
      b = &bay[z->zt_first_bay - 1];      /* F062896 - bad destination fix   */
      message_put(po[b->bay_port - 1].po_id, ZoneStartRequest,
        &z->zt_zone, sizeof(TZone));
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Pickline Switch Disable
 *-------------------------------------------------------------------------*/
pickline_disable(n)
register long n;
{
  register long k;

  for (k = n > 0 ? n - 1 : 0; k < coh->co_pl_cnt; k++)
  {
    if (pl[k].pl_pl)
    {
      if (pl[k].pl_flags & SwitchesDisabled) continue;
      if (sp->sp_pa_count) pa_stop(k + 1);
      if (sp->sp_productivity == 'y') accumulate_productivity(k + 1);
      pl[k].pl_flags |= SwitchesDisabled;
    }
    if (n) break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Pickline Switch Enable
 *-------------------------------------------------------------------------*/
pickline_enable(n)
register long n;
{
  register long k;
  
  for (k = n > 0 ? n - 1 : 0; k < coh->co_pl_cnt; k++)
  {
    if (pl[k].pl_pl)
    {
      if (!(pl[k].pl_flags & (SwitchesDisabled | StopOrderFeed))) continue;
      if (sp->sp_pa_count) pa_start(k + 1);
      pl[k].pl_flags &= ~(SwitchesDisabled | StopOrderFeed);
      if (sp->sp_productivity == 'y') start_productivity(k + 1);
#ifdef DELL
      count_queued(k + 1);
#endif 
    }
    if (n) break;
  }
  return 0;
}
#ifdef DELL
/*-------------------------------------------------------------------------*
 *  Count Queued Orders
 *-------------------------------------------------------------------------*/
count_queued(pickline)
register long pickline;
{
  register struct oi_item *o;
  register struct zone_item *z;
  register long k, block, next;
  
  if (pickline < 0 || pickline > coh->co_pl_cnt) return 0;
  
  for (k = 1, z = zone; k <= coh->co_zone_cnt; k++, z++)
  {
    if (z->zt_pl != pickline) continue;
     
    z->zt_queued = 0;
    
    next = z->zt_feeding;
    if (next > 0)
    {
      zone_boxes[k - 1] = DELL_BOXES;
    }
    else 
    {
      zone_boxes[k - 1] = DELL_LAST;      /* last zone in pickline           */
      zone_boxes[k]     = 1;              /* inspection station              */
    }
  }
  block = oc->oc_tab[pickline - 1].oc_uw.oc_first;
  
  while (block > 0)
  {
    o = &oc->oi_tab[block - 1];
    block = o->oi_flink;
    
    z = &zone[o->oi_entry_zone - 1];
    
    if (z->zt_status == ZS_UNDERWAY || z->zt_status == ZS_EARLY ||
        z->zt_status == ZS_LATE)
    {
      if (z->zt_on == o->oi_on) continue;
    }
    z->zt_queued += 1;
  }
}
#endif
/*-------------------------------------------------------------------------*
 *  Pickline Lock Orders
 *-------------------------------------------------------------------------*/
pickline_lock(n, m)
register long n, m;
{
  register long k;
  
  for (k = n > 0 ? n - 1 : 0; k < coh->co_pl_cnt; k++)
  {
    pl[k].pl_order = m;
    pl[k].pl_flags &= ~(OrdersLocked | PendingOrderLock);
    
    if (m) pl[k].pl_flags |= PendingOrderLock;
    else   pl[k].pl_flags |= OrdersLocked;
    if (n) break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Pickline Unlock Orders
 *-------------------------------------------------------------------------*/
pickline_unlock(n)
register long n;
{
  register long k;
  
  for (k = n > 0 ? n - 1 : 0; k < coh->co_pl_cnt; k++)
  {
    pl[k].pl_order = 0;
    pl[k].pl_flags &= ~(OrdersLocked | PendingOrderLock);
    
    if (sp->sp_labels == 'y' && sp->sp_tl_mode == 'a')
    {
      queue_ahead(Qtote, k + 1, TOTE_LABEL, sp->sp_tl_ahead);
    }
    if (n) break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Pickline Stop Orders
 *-------------------------------------------------------------------------*/
pickline_stop_request(n)
register long n;
{
  register long k;
  
  for (k = n > 0 ? n - 1 : 0; k < coh->co_pl_cnt; k++)
  {
    if (!pl[k].pl_pl) continue;
    pl[k].pl_flags |= StopOrderFeed;
    if (n) break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Valid Pickline Argument
 *-------------------------------------------------------------------------*/
valid_pickline(n)
register long n;
{
  char text[8];
  
  if (!n) return 0;
  if (n >= 1 && n <= coh->co_pl_cnt)
  {
    if (pl[n - 1].pl_pl) return n;
  }
  sprintf(text, "%d", n);
  message(ERR_PL, text);
  return -1;
}
/*--------------------------------------------------------------------------*
 *  Module Pick Inhibit
 *--------------------------------------------------------------------------*/
module_pick_inhibit(buf)
register TModuleMessage *buf;
{
  register struct pw_item *i;
  register long k;
  
  k = buf->m_module;
  
  if (sp->sp_config_status != 'y') return message(ERR_NO_CONFIG, 0);

  if (k < 1 || k > coh->co_prod_cnt) return message(ERR_PM_INV, 0);
  
  i = &pw[k - 1];
  
  if (i->pw_flags & PicksInhibited)
  {
    return message(LOCAL_MSG, "Picks Already Inhibited");
  }
  i->pw_flags |= PicksInhibited;
  
  message_put(0, ModuleInhibitEvent, buf, sizeof(TModuleMessage));
    
  return 0;
}
/*--------------------------------------------------------------------------*
 *  Module Pick Enable
 *--------------------------------------------------------------------------*/
module_pick_enable(buf)
register TModuleMessage *buf;
{
  register struct pw_item *i;
  register long k;
  
  k = buf->m_module;

  if (sp->sp_config_status != 'y') return message(ERR_NO_CONFIG, 0);

  if (k < 1 || k > coh->co_prod_cnt) return message(ERR_PM_INV, 0);
  
  i = &pw[k - 1];
  
  if (!(i->pw_flags & PicksInhibited))
  {
    return message(LOCAL_MSG, "Picks Already Enabled");
  }
  i->pw_flags &= ~PicksInhibited;

  message_put(0, ModuleEnableEvent, buf, sizeof(TModuleMessage));
    
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Redisplay - sends ZoneClear and responds to event.
 *-------------------------------------------------------------------------*/
zone_redisplay_request(zn)
{
  register struct zone_item *z;
  register struct bay_item *b;
  register long j;
  
  if (zn < 1 || zn >coh->co_zone_cnt) return 0;

  z = &zone[zn - 1];                      /*  point to zone                  */
  b = &bay[z->zt_first_bay - 1];          /*  point to bay                   */
  j = b->bay_port - 1;                    /*  send to first port only        */
    
#ifdef DEBUG
  fprintf(DF, "zone_redisplay_request: port=%d zone=%d\n", j, z->zt_zone);
#endif

  z->zt_flags |= ZoneInactive;            /* any packets are now ignored     */
    
  if (redisplay_count < 0) redisplay_count = 0;
  redisplay_count++;

  message_put(po[j].po_id, ZoneClearRequest, &z->zt_zone, sizeof(TZone));
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Clear Event - occurs from request, above.
 *-------------------------------------------------------------------------*/
zone_clear_event(zn)
register long zn;
{
  register struct zone_item *z;
  register struct bay_item *b;
  register long j;
  
#ifdef DEBUG
  fprintf(DF, "zone_clear_event(%d)\n", zn);
#endif

  if (zn < 1 || zn >coh->co_zone_cnt) return 0;

  z = &zone[zn - 1];                      /*  point to zone                  */
  b = &bay[z->zt_first_bay - 1];          /*  point to bay                   */
  j = b->bay_port - 1;                    /*  zone is on one port            */
  
  z->zt_time = time(0);

  if (z->zt_status == ZS_UNDERWAY || z->zt_status == ZS_EARLY) 
  {
    start_zone(z);
  }
  message_put(po[j].po_id, ZoneStartRequest, &z->zt_zone, sizeof(TZone));

  redisplay_count--;

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Offline Request  F051294
 *-------------------------------------------------------------------------*/
zone_offline(x)
register TZoneDisplayMessage *x;
{
  register struct zone_item *z;
  register struct bay_item  *b;
  
  if (x->m_zone < 1 || x->m_zone > coh->co_zone_cnt) return 0;
  
  z = &zone[x->m_zone - 1];
  if (z->zt_flags & IsOffline) return 0;

  if (sp->sp_productivity == 'y')
     {
       if (!z->zt_picker)
          {
            return 0;
          }
       else
          {
            z->zt_flags |= IsOffline;
          }
     }
  else
     {
       z->zt_flags |= IsOffline;
     }
  
  if (z->zt_status != ZS_UNDERWAY &&
      z->zt_status != ZS_EARLY    &&
      z->zt_status != ZS_LATE)
  {
    z->zt_status = ZS_OFFLINE;

    b = &bay[z->zt_first_bay - 1];    

     message_put(po[b->bay_port - 1].po_id, ZoneStartRequest,
      &z->zt_zone, sizeof(TZone));
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Online Request  F051294
 *-------------------------------------------------------------------------*/
zone_online(x)
register TZoneDisplayMessage *x;
{
  register struct zone_item *z;
  register struct bay_item  *b;
  
  if (x->m_zone < 1 || x->m_zone > coh->co_zone_cnt) return 0;
  
  z = &zone[x->m_zone - 1];

  if (sp->sp_productivity == 'y')
     {
       if (z->zt_picker)
          {
            return 0;
          }
       else
          {
            if (!(z->zt_flags & IsOffline)) return 0;

            z->zt_flags &= ~IsOffline;
          }
     }
  else
     {
       if (!(z->zt_flags & IsOffline)) return 0;

       z->zt_flags &= ~IsOffline;
     }
  
  if (z->zt_status == ZS_OFFLINE) 
  {
    z->zt_status = ZS_WAITING;
    zone_next_event(z->zt_zone, 0);
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Port Redisplay 
 *-------------------------------------------------------------------------*/
port_redisplay_request(n)
register long n;
{
  register struct zone_item *z;
  register struct bay_item *b;
  register long k;

#ifdef DEBUG
  fprintf(DF, "port_redisplay_request() port=%d\n", n);
#endif

  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (!z->zt_first_bay) continue;
    b = &bay[z->zt_first_bay - 1];
    if (b->bay_port != n + 1) continue;
    zone_redisplay_request(z->zt_zone);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Pickline Redisplay 
 *-------------------------------------------------------------------------*/
pickline_redisplay_request(n)
register long n;
{
  register struct zone_item *z;
  register long k;
  
#ifdef DEBUG
  fprintf(DF, "pickline_redisplay_request() pickline=%d\n", n);
#endif

  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (!z->zt_pl) continue;
    if (n && z->zt_pl != n) continue;
    zone_redisplay_request(z->zt_zone);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Cancel UnderWay Order
 *-------------------------------------------------------------------------*/
order_cancel_request(pickline, order)
register long pickline, order;
{
  register struct pl_item *p;
  register struct seg_item *s;
  register struct zone_item *z;
  register struct bay_item *b;
  register struct oi_item *o;
  register long j, k, n, block, flag, found;
#ifdef DELL
  long prev;
#endif
  char text[16];
  TOrderEventMessage x;
  
#ifdef DEBUG
  fprintf(DF, "order_cancel_request() PL=%d Order=%d\n",
    pickline, order);
#endif 

  sprintf(text, "%d-%05d", pickline, order);
  
  if (sp->sp_running_status != 'y') return 0;  /* ignore                     */

  oc_lock();
  block = oc_find(pickline, order);

  if (!block)                             /* order not found                 */
  {
    oc_unlock(); return message(ERR_ORDER, text);
  }
  o = &oc->oi_tab[block - 1];
  if (o->oi_queue != OC_UW)               /* order not underway              */
  {
    oc_unlock(); return message(ERR_ORDER, text);
  }
  message(ERR_OF_CANCEL, text);           /* confirm cancel order            */
  
  p = &pl[o->oi_pl - 1];                  /* pointer to pickline             */
  
  if (p->pl_flags & IsSegmented)
  {
    found = 0;

    s = &sg[p->pl_first_segment - 1];
    
    for (k = p->pl_first_segment; k <= p->pl_last_segment; k++, s++)
    {
      n = oc_find(PicklineMax + k, order);
      if (n < 1) continue;

      flag = 0;

      z = &zone[s->sg_first_zone - 1];

      for (j = s->sg_first_zone; j <= s->sg_last_zone; j++, z++)
      {
        if (z->zt_on != order) continue;
        if (z->zt_status == ZS_UNDERWAY ||
            z->zt_status == ZS_EARLY    ||
            z->zt_status == ZS_LATE)
        {
          b = &bay[z->zt_first_bay - 1];
      
          message_put(po[b->bay_port - 1].po_id, ZoneStopRequest,
            &z->zt_zone, sizeof(TZone));

          found = flag = 1;
        }
      }
      if (!flag)
      {
        oc_dequeue(n);
        oc_delete(n);
      }
    }
    if (found) return 0;
  }
  else                                     /* not segmented                  */
  {
    for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
    {
      if (z->zt_on != order) continue;
      if (z->zt_status == ZS_UNDERWAY ||
          z->zt_status == ZS_EARLY    ||
          z->zt_status == ZS_LATE)
      {
        b = &bay[z->zt_first_bay - 1];
      
        message_put(po[b->bay_port - 1].po_id, ZoneStopRequest,
          &z->zt_zone, sizeof(TZone));
        return 0;
      }
    }
  }
  z = &zone[o->oi_entry_zone - 1];
  
#ifdef DELL
  if (z->zt_queued > 0) z->zt_queued -= 1;

  prev = z->zt_source;
    
  if (prev > 0)                        /* unlock previous zone             */
  {
    if (zone[prev - 1].zt_status == ZS_OFFLINE)
    {
      zone[prev - 1].zt_status = ZS_WAITING;
      message_put(0, ZoneNextEvent, &prev, sizeof(TZone));
    }
  }
#endif
  begin_work();
  od_get(block);
  of_rec->of_elapsed += time(0) - of_rec->of_datetime;
  if (sp->sp_pa_count) pa_completion(PA_CANCELED | PA_CURRENT);
  
  x.m_pickline = of_rec->of_pl;
  x.m_order    = of_rec->of_on;
  x.m_zone     = z->zt_zone;
  memcpy(x.m_grp, of_rec->of_grp, GroupLength);
  memcpy(x.m_con, of_rec->of_con, CustomerNoLength);

  of_rec->of_status = 'x';
  of_rec->of_datetime = time(0);
  od_update(block);
  oc_dequeue(block);
  oc_enqueue(block, OC_LAST, OC_COMPLETE);
  commit_work();
  check_save_segs();
  oc_unlock();

  message_put(0, OrderCancelEvent, &x, sizeof(TOrderEventMessage));

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Stop Event Caused By Order Cancel
 *-------------------------------------------------------------------------*/
zone_cancel_event(zn)
register long zn;
{
  register struct zone_item *z;
  TOrderEventMessage x;
  
  z = &zone[zn - 1];                      /* point to zone                   */

#ifdef DEBUG
  fprintf(DF, "zone_cancel_event() Zone=%d Order=%d Block=%d\n",
    z->zt_zone, z->zt_on, z->zt_order);
#endif
  oc_lock();

  if (z->zt_flags & IsSegmented)
  {
    oc_dequeue(z->zt_order);
    oc_delete(z->zt_order);
    z->zt_order = oc_find(z->zt_pl, z->zt_on);    /* find order block        */
  }
  if (z->zt_order > 0)
  {
    begin_work();
    od_get(z->zt_order);
    of_rec->of_elapsed += time(0) - of_rec->of_datetime;
    if (sp->sp_pa_count) pa_completion(PA_CANCELED | PA_CURRENT);

    x.m_pickline = of_rec->of_pl;
    x.m_order    = of_rec->of_on;
    x.m_zone     = z->zt_zone;
    memcpy(x.m_grp, of_rec->of_grp, GroupLength);
    memcpy(x.m_con, of_rec->of_con, CustomerNoLength);

    of_rec->of_status = 'x';
    of_rec->of_datetime = time(0);
    od_update(z->zt_order);
    oc_dequeue(z->zt_order);
    oc_enqueue(z->zt_order, OC_LAST, OC_COMPLETE);
    commit_work();
    message_put(0, OrderCancelEvent, &x, sizeof(TOrderEventMessage));
  }
  check_save_segs();
  oc_unlock();

  z->zt_on     = z->zt_order = 0;
  z->zt_status = ZS_INACTIVE;

  zone_next_event(zn, 0);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Start Pickline Productivity
 *-------------------------------------------------------------------------*/
start_productivity(n)
register long n;
{
  register struct zone_item *z;
  register long k, now;

#ifdef DEBUG
  fprintf(DF, "start_productivity(%d)\n", n);
#endif

  now = time(0);
  
  if (n < 1 || n > pr->pr_picklines || n > coh->co_pl_cnt) return 0;
  
  pp[n - 1].pr_pl_current = now;
  
  for (k = 0, z = zone; k < pr->pr_zones && k < coh->co_zone_cnt; k++, z++)
  {
    if (!z->zt_zone) continue;
    if (n && z->zt_pl != n) continue;
    z->zt_time = now;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Accumulate Pickline Productivity 
 *-------------------------------------------------------------------------*/
accumulate_productivity(n)
register long n;
{
  register struct zone_item *z;
  register long k, now;
  
  now = time(0);
  
  if (n < 1 || n > pr->pr_picklines || n > coh->co_pl_cnt) return 0;
  
  pl_productivity(n , now);
  
  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (z->zt_pl != n) continue;
    zone_productivity(z->zt_zone, now);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Pickline Productivity - Pickline And All Zones In Pickline
 *-------------------------------------------------------------------------*/
pl_productivity(n, now)
register long n, now;
{
  register struct zone_item *z;
  register struct pr_pl_item *p;
  register long elapsed;

  if (n < 1 || n > pr->pr_picklines || n > coh->co_pl_cnt) return 0;
  if (!pl[n - 1].pl_pl) return 0;

  p = &pp[n - 1];
  
  elapsed = now - p->pr_pl_current;

#ifdef PROD
  fprintf(DF, "pl_productivity(%d, %d) elapsed=%d\n", n, now, elapsed);
#endif
  
  p->pr_pl_current = now;

  if (oc->oc_tab[n - 1].oc_uw.oc_count <= 0) return 0;            /* F042295 */

  p->pr_pl_cum_elapsed += elapsed;
  p->pr_pl_cur_elapsed += elapsed;

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Productivity - At Each Zone Finished Event
 *-------------------------------------------------------------------------*/
zone_productivity(n, now)
register long n, now;
{
  register struct zone_item *z;
  register struct pr_zone_item *p;
  register long elapsed;
  
  if (n < 1 || n > pr->pr_zones || n > coh->co_zone_cnt) return 0;
  
  z = &zone[n - 1];
  p = &pz[n - 1];
  
  elapsed = now - z->zt_time;
  z->zt_time = now;
 
#ifdef PROD
  fprintf(DF, "zone_productivity(%d, %d) elapsed=%d\n", n, now, elapsed);
#endif

  if (oc->oc_tab[z->zt_pl - 1].oc_uw.oc_count <= 0) return 0;     /* F042295 */

  if (z->zt_status == ZS_UNDERWAY || z->zt_status == ZS_EARLY)
  {
    p->pr_zone_cum_active += elapsed;
    p->pr_zone_cur_active += elapsed;
  }
  else if (z->zt_status == ZS_AHEAD)
  {
    p->pr_zone_cum_ahead += elapsed;
    p->pr_zone_cur_ahead += elapsed;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
void leave(int val)
{
  message_close();
  close_all();
  database_close();
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Close All Files
 *-------------------------------------------------------------------------*/
close_all()
{
  ss_close_save();
  co_close_save();
  oc_close_save();
  if (sp->sp_productivity == 'y') pr_close_save();
  od_close();
  picker_close();
  picker_order_close();

  if (sp->sp_labels == 'y')
  {
    tote_label_close();
    ship_label_close();
    packing_list_close();
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Error Message
 *-------------------------------------------------------------------------*/
message(err, text)
register long err;
register char *text;
{
  register long k;
  unsigned char buffer[80];
  
  *buffer = err;                          /* error code                      */
  k = 0;
  
  if (text)                               /* check has any error text        */
  {
    k = strlen(text);                     /* length of text                  */
    memcpy(buffer + 1, text, k);          /* append to buffer                */
  }
  message_put(who, ClientMessageEvent, buffer, k + 1);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save Segments
 *-------------------------------------------------------------------------*/
save_segs()
{
  ss_save();
  oc_save();
  co_save();
  pr_save();
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check If Time To Save Segments For Emergency Recovery
 *-------------------------------------------------------------------------*/
check_save_segs()
{
  long now;
  
  if (sp->sp_save_window <= 0) return 0;

  now = time(0);
  if (now < next_save_time) return 0;
  next_save_time = now + sp->sp_save_window;

  ss_save();
  oc_save();
  pr_save();

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open All Useful Stuff
 *-------------------------------------------------------------------------*/
open_all()
{
  database_open();
  ss_open();                              /* open shared segments            */
  co_open();
  oc_open();
  od_open();
  if (sp->sp_productivity == 'y') pr_open();

  if (sp->sp_labels == 'y')
  {
    tote_label_open(AUTOLOCK);
    ship_label_open(AUTOLOCK);
    packing_list_open(AUTOLOCK);
  }
  MP_TIMEOUT = sp->sp_mp_timeout;
  RP_TIMEOUT = sp->sp_rp_timeout;

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Picker Accountability Open and Establish Correct Values
 *-------------------------------------------------------------------------*/
pa_open()
{
  long fd;
  register long k, block, now;
  picker_item picker;
  PA_FILESTRUCT pa;
  
#ifdef DEBUG
  fprintf(DF, "pa_open()\n");
#endif
  open_pa_file(fd);
  read_pa_file(fd, &pa);
  pa.last_start_time = time(0);
  write_pa_file(fd, &pa);
  close_pa_file(fd);

  now = time(0);

  picker_open(AUTOLOCK);
  picker_setkey(1);
  
  picker_order_open(AUTOLOCK);
  picker_order_setkey(1);

  begin_work();
  while (!picker_next(&picker, LOCK))      /* clear picker records           */
  {
    picker.p_underway_orders = 0;
    picker.p_current_time    = 0;
#ifdef DEBUG
    fprintf(DF, "picker_replace 1: picker = %d, status = %d\n",
            picker.p_picker_id, picker.p_status);
    fflush(DF);
#endif
    picker_replace(&picker);
    commit_work();
    begin_work();
  }
  commit_work();

#ifdef PICK_BY_ZONE
  return 0;                               /* pickers assigned to zones       */
#endif
  
  for (k = 0; k < coh->co_pl_cnt; k++)    /* over all picklines              */
  {
    block = oc->oc_tab[k].oc_uw.oc_first;
    
    while (block)                         /* over entire underway queue      */
    {
      begin_work();
      od_get(block);                      /* get the order record            */

      if (of_rec->of_picker)              /* check assiged to a picker       */
      {
        picker.p_picker_id = of_rec->of_picker;
         
        if (!picker_read(&picker, LOCK))
        {
          picker.p_underway_orders += 1;  /* count orders assigned           */
#ifdef DEBUG
    fprintf(DF, "picker_replace 2: picker = %d, status = %d\n",
            picker.p_picker_id, picker.p_status);
    fflush(DF);
#endif
          picker_replace(&picker);
        }
        else of_rec->of_picker = 0;       /* no picker anymore ???           */
      }
      of_rec->of_datetime = now;          /* set time to now                 */

      od_update(block);
      commit_work();
      block = oc->oi_tab[block - 1].oi_flink;
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Picker Accountability Close
 *-------------------------------------------------------------------------*/
pa_close()
{
  long fd;
  PA_FILESTRUCT pa;
  long now;
  
#ifdef DEBUG
  fprintf(DF, "pa_close()\n");
#endif

  pa_stop(0);

  open_pa_file(fd);
  read_pa_file(fd, &pa);

  now = time(0) - pa.last_start_time;
  
  pa.current_config_time    += now;
  pa.cumulative_config_time += now;

  write_pa_file(fd, &pa);
  close_pa_file(fd);

  picker_close();
  picker_order_close();
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Picker Accountability Pickline Stop - Accumulate Elapsed Time
 *-------------------------------------------------------------------------*/
pa_stop(n)
register long n;
{
  register long k, block, now, elapsed;

#ifdef DEBUG
  fprintf(DF, "pa_stop(%d)\n", n);
#endif

  now = time(0);

  for (k = n > 0 ? n - 1 : 0; k < coh->co_pl_cnt; k++)
  {
    if (pl[k].pl_flags & SwitchesDisabled) continue;

    block = oc->oc_tab[k].oc_uw.oc_first;
    
    while (block)                         /* over entire underway queue      */
    {
      begin_work();
      od_get(block);                      /* get the order record            */

      elapsed = now - of_rec->of_datetime;
      if (elapsed > 0) of_rec->of_elapsed += elapsed;

      of_rec->of_datetime = now;

      od_update(block);
      commit_work();
      block = oc->oi_tab[block - 1].oi_flink;
    }
    if (n) break;                         /* only one pickline               */
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Picker Accountability Pickline Start - Set Date Time 
 *-------------------------------------------------------------------------*/
pa_start(n)
register long n;
{
  register long k, block, now, elapsed;

#ifdef DEBUG
  fprintf(DF, "pa_start(%d)\n", n);
#endif

  now = time(0);

  for (k = n > 0 ? n - 1 : 0; k < coh->co_pl_cnt; k++)
  {
    if (!(pl[k].pl_flags & SwitchesDisabled)) continue;

    block = oc->oc_tab[k].oc_uw.oc_first;
    
    while (block)                         /* over entire underway queue      */
    {
      begin_work();
      od_get(block);                      /* get the order record            */

      of_rec->of_datetime = now;          /* set time to now                 */

      od_update(block);                   /* only update to current time     */
      commit_work();
      block = oc->oi_tab[block - 1].oi_flink;
    }
    if (n) break;                         /* only one pickline               */
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Picker Accountability Order Start Request From Badge Reader
 *-------------------------------------------------------------------------*/
pa_request(x)
register TBadgeMessage *x;
{
  register struct pl_item *p;
  register struct zone_item *z;
  register long n, rate;
  TErrorMessage y;
  picker_item picker;
  char stuff[16];
  
#ifdef DEBUG
  fprintf(DF, "pa_request(%d, %d)\n", x->m_pickline, x->m_badge);
#endif

  if (x->m_pickline < 1 || x->m_pickline > coh->co_pl_cnt) return 0;

  p = &pl[x->m_pickline - 1];
  n = p->pl_first_zone;
  z = &zone[n - 1];
  
  y.m_error = 0;
    
  if (sp->sp_running_status != 'y')
  {
    strcpy(y.m_text, "  CAPS Is Not     Running ..");
    message_put(who, BadgeErrorEvent, &y, strlen(y.m_text) + 1);
    return ;
  }
  if (p->pl_flags & SwitchesDisabled)
  {
    sprintf(y.m_text, "Pickline %3d    Is Inhibited ..", x->m_pickline);
    message_put(who, BadgeErrorEvent, &y, strlen(y.m_text) + 1);
    return ;
  }
  if (sp->sp_pa_count < 1)
  {
    strcpy(y.m_text, "Badge Reader Is Not Required ..");
    message_put(who, BadgeErrorEvent, &y, strlen(y.m_text) + 1);
    return ;
  }
  if (p->pl_flags & OrdersLocked)
  {
    strcpy(y.m_text, "Pickline Orders Are Locked ..");
    message_put(who, BadgeErrorEvent, &y, strlen(y.m_text) + 1);
    return 0;
  }
  if (z->zt_status == ZS_UNDERWAY || z->zt_status == ZS_EARLY)
  {
    strcpy(y.m_text, "Entry Zone Has  An Order ..");
    message_put(who, BadgeErrorEvent, &y, strlen(y.m_text) + 1);
    return 0;
  }
  picker.p_picker_id = x->m_badge;
  
  begin_work();
  if (picker_read(&picker, LOCK))
  {
    commit_work();
    sprintf(y.m_text, "Badge %6d    Not Valid ..", x->m_badge);
    message_put(who, BadgeErrorEvent, &y, strlen(y.m_text) + 1);
    return 0;
  }
  if (picker.p_underway_orders >= sp->sp_pa_count)
  {
    commit_work();   
    strcpy(y.m_text, "Proceed With    Existing Orders");
    message_put(who, BadgeErrorEvent, &y, strlen(y.m_text) + 1);
    return 0;
  }
  zone_next_event(n, picker.p_picker_id);  /* try to start an order          */
  
  if (z->zt_status == ZS_UNDERWAY)
  {
    if (picker.p_cur_time > 0)
    {
      rate = (3600 * picker.p_cur_units) / picker.p_cur_time;
    }
    else rate = 0;
    
    sprintf(stuff, "%d/%d", pa_order_lines, pa_order_units);
    
    sprintf(y.m_text, "%-10.10s#%05d%-10.10s%-6d",
    picker.p_last_name, z->zt_on, stuff, rate);
    y.m_text[32] = 0;
    message_put(who, BadgePickerEvent, &y, strlen(y.m_text) + 1);

    if (picker.p_underway_orders < 1) picker.p_start_time = time(0);
    picker.p_underway_orders += 1;
#ifdef DEBUG
    fprintf(DF, "picker_replace 3: picker = %d, status = %d\n",
            picker.p_picker_id, picker.p_status);
    fflush(DF);
#endif
    picker_replace(&picker);
    commit_work();
    return 0;
  }
  commit_work();
  strcpy(y.m_text, "No Orders Are    Available ..");
  message_put(who, BadgeErrorEvent, &y, strlen(y.m_text) + 1);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Picker Accountabiltiy Order Completion/Cancel
 *-------------------------------------------------------------------------*/
pa_completion(what)
register unsigned long what;
{
  picker_item picker;
  picker_order_item po;
  long now, elapsed;                      /* F042695                         */

#ifdef PICK_BY_ZONE
  return 0;
#endif

#ifdef DEBUG
  fprintf(DF, "pa_completion order=%05d\n", of_rec->of_on);
#endif
  if (!of_rec->of_picker) return 0;       /* no picker was assigned          */
  
  now = time(0);                          /* get time                F042696 */

  picker.p_picker_id = of_rec->of_picker;
 
  begin_work();
  if (picker_read(&picker, LOCK)) 
  {
    commit_work();
    return 0;
  }
  if (picker.p_underway_orders > 0) picker.p_underway_orders -= 1;
  
  picker.p_cur_order_count += 1;
  picker.p_cur_lines       += of_rec->of_no_picks;
  picker.p_cur_units       += of_rec->of_no_units;

  picker.p_cum_order_count += 1;
  picker.p_cum_lines       += of_rec->of_no_picks;
  picker.p_cum_units       += of_rec->of_no_units;

  if (picker.p_underway_orders <= 0)
  {
    elapsed = now - picker.p_start_time;   /* total elasped of group F042695 */

    picker.p_cur_time += elapsed;
    picker.p_cum_time += elapsed;
  }
#ifdef DEBUG
    fprintf(DF, "picker_replace 4: picker = %d, status = %d\n",
            picker.p_picker_id, picker.p_status);
    fflush(DF);
#endif
  picker_replace(&picker);

  po.p_order_number    = of_rec->of_on;
  po.p_picker_id       = picker.p_picker_id;
  po.p_pickline        = of_rec->of_pl;
  po.p_order_status    = what;
  po.p_start_time      = picker.p_start_time;
  po.p_completion_time = now;                       /* same time F042695     */
  po.p_picking_time    = of_rec->of_elapsed;
  po.p_lines_picked    = of_rec->of_no_picks;
  po.p_units_picked    = of_rec->of_no_units;
  picker_order_write(&po);
  commit_work();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Accumulate Picker Productivity By Zone
 *-------------------------------------------------------------------------*/
pa_by_zone(z)
register struct zone_item *z;
{
  picker_item picker;
  register long now, elapsed;

#ifdef DEBUG
  fprintf(DF, "pa_by_zone(%d) picker=%d time=%d\n",
    z->zt_zone, z->zt_picker, microclock() - stime);
#endif

  if (z->zt_picker <= 0) return 0;        /* no picker assigned              */

  now = time(0);                          /* get time                        */

  picker.p_picker_id = z->zt_picker;
  
  begin_work();
  if (picker_read(&picker, LOCK)) 
  {
    commit_work();
    return 0;
  }
  if (picker.p_start_time > 0)
  {
    elapsed = now - picker.p_start_time; 

    picker.p_cur_time += elapsed;
    picker.p_cum_time += elapsed;
  }
  if (z->zt_lines > 0)
  {
    picker.p_cur_order_count += 1;
    picker.p_cur_lines       += z->zt_lines;
    picker.p_cur_units       += z->zt_units;

    picker.p_cum_order_count += 1;
    picker.p_cum_lines       += z->zt_lines;
    picker.p_cum_units       += z->zt_units;

    picker.p_start_time = now;
  }
  else picker.p_start_time = 0;

  if (z->zt_picker)
     {
        picker.p_status = 1;
        picker.p_zone   = z->zt_zone;
     }
  
#ifdef DEBUG
    fprintf(DF, "picker_replace 5: picker = %d, status = %d\n",
            picker.p_picker_id, picker.p_status);
    fflush(DF);
#endif
  picker_update(&picker);
  commit_work();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Entry Labels
 *-------------------------------------------------------------------------*/
check_entry_labels(block)
register long block;
{
  if (sp->sp_tl_mode == 'e' || sp->sp_tl_mode == 'n' || sp->sp_tl_mode == 'a')
  {
    if (!(oc->oi_tab[block - 1].oi_flags & TOTE_LABEL))
    {
      queue_label(Qtote, of_rec->of_pl, of_rec->of_on);
    }
  }
  if (sp->sp_sl_mode == 'e' || sp->sp_sl_mode == 'n')
  {
    if (!(oc->oi_tab[block - 1].oi_flags & SHIP_LABEL))
    {
      queue_label(Qship, of_rec->of_pl, of_rec->of_on);
    }
  }
  if (sp->sp_pl_mode == 'e' || sp->sp_pl_mode == 'n')
  {
    if (!(oc->oi_tab[block - 1].oi_flags & PACK_LIST))
    {
      queue_label(Qpack, of_rec->of_pl, of_rec->of_on);
    }
  }
  if (sp->sp_tl_mode == 'a')
  {
    queue_ahead(Qtote, of_rec->of_pl, TOTE_LABEL, sp->sp_tl_ahead);
  }
  if (sp->sp_tl_mode == 'n')
  {
    queue_ahead(Qtote, of_rec->of_pl, TOTE_LABEL, 1);
  }
  if (sp->sp_sl_mode == 'n')
  {
    queue_ahead(Qship, of_rec->of_pl, SHIP_LABEL, 1);
  }
  if (sp->sp_pl_mode == 'n')
  {
    queue_ahead(Qpack, of_rec->of_pl, PACK_LIST, 1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Exit Labels
 *-------------------------------------------------------------------------*/
check_exit_labels()
{
  if (sp->sp_tl_mode == 'c')
  {
    queue_label(Qtote, of_rec->of_pl, of_rec->of_on);
  }
  if (sp->sp_sl_mode == 'c')
  {
    queue_label(Qship, of_rec->of_pl, of_rec->of_on);
  }
  if (sp->sp_pl_mode == 'c')
  {
    queue_label(Qpack, of_rec->of_pl, of_rec->of_on);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 * Queue Packing List
 *-------------------------------------------------------------------------*/
queue_label(Q, pl, on)
register long Q;
register long pl, on;
{
  paper_item p;
  
  p.paper_time   = time(0);
  p.paper_copies = 1;
  p.paper_pl     = pl;
  p.paper_order  = on;
  p.paper_zone   = 0;
  
  switch(Q)
  {
    case Qtote: sp->sp_tl_order = on;
                p.paper_ref = ++sp->sp_tl_count;
                tote_label_write(&p); 
                break;
                
    case Qship: sp->sp_sl_order = on;
                p.paper_ref = ++sp->sp_sl_count;
                ship_label_write(&p); 
                break;

    case Qpack: sp->sp_pl_order = on;
                p.paper_ref = ++sp->sp_pl_count;
                packing_list_write(&p); 
                break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Any Kind Of Labels Ahead / Next
 *-------------------------------------------------------------------------*/
queue_ahead(Q, pl, flag, goal)
register long Q;
register long pl;
register unsigned long flag;
register long goal;
{
  register struct oc_item *q;
  register struct oi_item *o;
  register long block, count;
  
  oc_lock();
  
  q = &oc->oc_tab[pl - 1];

  while (q->oc_queue[OC_HIGH].oc_count < goal)
  {
    if (q->oc_queue[OC_MED].oc_count)
    {
      block = q->oc_queue[OC_MED].oc_first;
      oc_dequeue(block);
      begin_work();
      od_get(block);
      of_rec->of_pri      = 'h';
      of_rec->of_datetime = time(0);
      od_update(block);
      oc_enqueue(block, OC_LAST, OC_HIGH);
      commit_work();
    }
    else if (q->oc_queue[OC_LOW].oc_count)
    {
      block = q->oc_queue[OC_LOW].oc_first;
      oc_dequeue(block);
      begin_work();
      od_get(block);
      of_rec->of_pri      = 'h';
      of_rec->of_datetime = time(0);
      od_update(block);
      oc_enqueue(block, OC_LAST, OC_HIGH);
      commit_work();
    }
    else break;
  }
  oc_unlock();
  
  block = q->oc_queue[OC_HIGH].oc_first;
  count = 0;
  
  while (block && count < goal)
  {
    o = &oc->oi_tab[block - 1];
    if (!(o->oi_flags & flag))
    {
      o->oi_flags |= flag;
      queue_label(Q, o->oi_pl, o->oi_on);
    }
    count++;
    block = o->oi_flink;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Convert To Binary
 *-------------------------------------------------------------------------*/
long cvrt(p, n)
register char *p;
register long n;
{
  register long x;

  for (x = 0; n > 0; n--, p++)
  {
    if (*p < '0' || *p > '9') continue;
    x = 10 * x + (*p - '0');
  }
  return x;
}

#ifdef NEWREMARKS
make_empty_boxes(on,pl)
long on,pl;
{
  register long k;
  register char *p;
  boxes_item box;
  long block;
  register struct oi_item *o;
  char boxno[16];
  char box_str[BoxNoLength+1];
  
  
#ifdef NEWBOXFULL
	return;
#endif
#ifdef DEBUG
  fprintf(DF,"rvj - make_empty %9.9s\n",or_rec->rmks_text);
  fflush(DF);
#endif
  block = oc_find(pl, on);
  o = &oc->oi_tab[block - 1];

  p = o->oi_box ; 
 
#ifdef DEBUG 
  fprintf(DF,"rvj - make_empty box_number %9.9s\n",o->oi_box);
  fflush(DF);
#endif
  memset(&box, 0, sizeof(boxes_item));
  memset(box_str, 0x0, BoxNoLength+1);

  memcpy(box_str, o->oi_box, BoxNoLength);
  box_str[BoxNoLength] = '\0';
  box.b_box_number = atol(box_str); 
 
  box.b_box_pl        = pl;
  box.b_box_on        = on;
  box.b_box_status[0] = BOX_UNUSED;
  box.b_box_status[1] = '\0';
  box.b_box_last[0]   = 0x20;
  box.b_box_last[1]   = '\0';
 
#ifdef DEBUG   
  fprintf(DF,"rvj - make_empty box_number %d\n",box.b_box_number);
  fflush(DF);
#endif
  boxes_write(&box);

#ifdef JCP
  memset(oc->oi_tab[block - 1].oi_box, 0x20, BoxNoLength);
  sprintf(boxno, "%0*d", BoxNoLength, box.b_box_number); 
#ifdef DEBUG
  fprintf(DF,"rvj - make_empty boxno %16.16s\n",boxno);
  fflush(DF);
#endif
  memcpy(o->oi_box, boxno, BoxNoLength);  
#endif
  return 0;
}
#else
make_empty_boxes(on,pl)
long on,pl;
{
  register long k;
  register char *p;
  boxes_item box;
  long block;
  register struct oi_item *o;
  char boxno[16];
  
#ifdef NEWBOXFULL
	return;
#endif
  or_rec->rmks_pl = pl;
  or_rec->rmks_on = on;
  
  if (remarks_read(or_rec, NOLOCK)) return 0;
#ifdef DEBUG  
  fprintf(DF,"rvj - make_empty %9.9s\n",or_rec->rmks_text);
  fflush(DF);
#endif
  block = oc_find(pl, on);
  o = &oc->oi_tab[block - 1];

  p = or_rec->rmks_text + rf->rf_box_pos;
  
  for (k = 0; k < rf->rf_box_count; k++, p += rf->rf_box_len)
  {
    memset(&box, 0, sizeof(boxes_item));

    box.b_box_number = cvrt(p + 1, rf->rf_box_len); // use the box number
					// after C/T in the scan code
    if (!box.b_box_number) continue;
    
    box.b_box_pl        = pl;
    box.b_box_on        = on;
    box.b_box_status[0] = BOX_UNUSED;
    box.b_box_status[1] = '\0';
    box.b_box_last[0]   = 0x20;
    box.b_box_last[1]   = '\0';
#ifdef DEBUG    
    fprintf(DF,"rvj - make_empty box_number %d\n",box.b_box_number);
    fflush(DF);
#endif
    boxes_write(&box);
  }
#ifdef JCP
  memset(oc->oi_tab[block - 1].oi_box, 0x20, BoxNoLength);
  sprintf(boxno, "%0*d", BoxNoLength, box.b_box_number); 
fprintf(DF,"rvj - make_empty boxno %16.16s\n",boxno);
  memcpy(o->oi_box, boxno, BoxNoLength);  
#endif
  return 0;
}
#endif
/* end of ofc.c */

