#define DEBUG
#define DETAIL
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Dummy controller for one port.
 *
 *  Execution:      dlc  [port]  [modules]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/28/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char dlc_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"
#include "message_types.h"
#include "caps_messages.h"
#include "ss.h"
#include "co.h"
#include "zone_status.h"

FILE *fd;
char fd_name[40];                         /* hw_map count file               */

unsigned char prog[16];                   /* name of this program            */
TPort number;                             /* number of this port             */
long modules;                             /* number of modules on this port  */

long who;                                 /* message sender                  */
long type;                                /* message type                    */
TCapsMessageItem buf;                     /* message buffer                  */
long len;                                 /* message length                  */

unsigned char list[] = {ShutdownRequest, PortInitializeRequest,
  PortMarkplaceRequest, InitializeEvent, ConfigureEvent,
  RestoreplaceEvent, PortDisableRequest, PortEnableRequest,
  ModulePickRequest, ZoneStartRequest};

main(argc, argv)
long argc;
char **argv;
{
  if (argc < 3)
  {
    krash("main", "Missing Arguments", 1);
  }
  number =  atol(argv[1]);                /* port number                     */
  modules = atol(argv[2]);                /* module count                    */
   
  sprintf(prog, "_=dlc%d", number);
  putenv(prog);                           /* store program name              */
  chdir(getenv("HOME"));                  /* insure in HOME directory        */

  setpgrp();

  message_open();
  message_select(list, sizeof(list));
   
#ifdef DEBUG
  fprintf(stderr, "%s started:  msgtask=%d\n", prog, msgtask);
#endif

  ss_open();
  co_open();
  
  if (number < 0 || number >= coh->co_ports)
  {
    krash("main", "invalid port number", 1);
  }
  
  sprintf(po[number].po_name, "dummy%d", number); /* name of port            */
  po[number].po_status = 'x';             /* status is closed                */
  po[number].po_id     = msgtask;         /* this task                       */
  po[number].po_id_in  = msgtask;         /* this task                       */
  po[number].po_flags  = IsDummy;         /* what we are                     */
  po[number].po_disabled = 'n';           /* set to enabled                  */
  
  while (1)
  {
    message_get(&who, &type, &buf, &len);

#ifdef DEBUG
  fprintf(stderr, "got message: who=%d type=%d len=%d\n", who, type, len);
  if (len > 0) Bdumpf(&buf, len, stderr);
#endif

    switch(type)
    {
      case ShutdownRequest: 
      case ShutdownEvent:   leave(0);
     
      case PortInitializeRequest:
     
        sprintf(fd_name, "%s.%s", hw_name, po[number].po_name);
     
        fd = fopen(fd_name, "w");
        if (fd == 0) krash("main", "open map", 1);

        fwrite(&modules, 4, 1, fd);
        fclose(fd);

        po[number].po_status = 'i';

        message_put(coh->co_id, PortInitializeEvent, 0, 0);
        break;
              
      case InitializeEvent:
    
        if (po[number].po_status == 'i') po[number].po_status = 'n';
        break;
              
      case ConfigureEvent:
      case RestoreplaceEvent:
     
        if (po[number].po_status == 'n') po[number].po_status = 'y';
        break;
      
      case PortMarkplaceRequest:
    
        po[number].po_status = 'x';
        message_put(coh->co_id, PortMarkplaceEvent, 0, 0);
        break;
    
      case PortDisableRequest:
    
        po[number].po_status = 'x';
        message_put(0, PortDisableEvent, &number, sizeof(TPortMessage));
        break;
      
      case PortEnableRequest:
    
        po[number].po_status = 'y';
        message_put(0, PortEnableEvent, &number, sizeof(TPortMessage));
        break;
      
      case ModulePickRequest:
      
       pick_request(&buf);
       break;
       
      case ZoneStartRequest:
      
        message_put(0, ZoneStartEvent, &buf, sizeof(TZoneMessage));
        zone_start(buf.ZoneMessage.m_zone);
        break;
        
      default: break;
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
  register struct hw_item  *h;
  register struct pw_item  *i;
  register struct bay_item *b;
  register long k;
  
#ifdef DEBUG
  fprintf(stderr,"pick_request() mod=%d quan=%d ref=%d\n",
    x->m_module, x->m_ordered, x->m_reference);
#endif
  
  i = &pw[x->m_module - 1];
  k = i->pw_ptr;                            /* module table subscript        */
  h = &hw[k - 1];
  b = &bay[h->hw_bay - 1];

  if (!(b->bay_flags & IsDummy)) return 0;

  i->pw_reference = x->m_reference;
  i->pw_ordered   = x->m_ordered;
  i->pw_picked    = x->m_ordered;
  i->pw_flags    |= (BinHasPick | BinPicked);

  h->hw_flags |= ModuleHasPick;
  
  if (i->pw_flags & PicksInhibited) i->pw_picked = 0;

#ifdef DETAIL
  Bdumpf(i, sizeof(struct pw_item), stderr);
  Bdumpf(h, sizeof(struct hw_item), stderr);
  fflush(stderr);
#endif
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Zone Start Request
 *-------------------------------------------------------------------------*/
zone_start(zn)
register long zn;
{
  register struct zone_item *z;
  
#ifdef DEBUG
  fprintf(stderr, "zone_start(%d)\n", zn);
#endif

  z = &zone[zn - 1];

  z->zt_flags &= ~ZoneInactive;
  
  switch (z->zt_status)
  {
    case ZS_COMPLETE:
    case ZS_AHEAD:
    case ZS_LOCKED:
    case ZS_WAITING:
    case ZS_INACTIVE: break;
    
    case ZS_LATE:
    case ZS_EARLY:
    case ZS_UNDERWAY:

        z->zt_flags |= ZoneInactive;

        zone_clear(buf.ZoneMessage.m_zone, 1);
        message_put(0, ZoneCompleteEvent, &buf, sizeof(TZoneMessage));
        message_put(0, ZoneNextEvent, &buf, sizeof(TZoneMessage));
        break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Clear Zone And Return Picks
 *-------------------------------------------------------------------------*/
zone_clear(zn, flag)
register long zn;                         /* zone number   
register long flag;                       /* 0 = clear, 1 = picks          */
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long j, k, m;
  TPickMessage x;
  
#ifdef DEBUG
  fprintf(stderr, "zone_clear(): zone=%d flag=%d\n", zn, flag);
#endif

  z = &zone[zn - 1];                      /* point to zone record            */

  if (flag == 1)                         /* setup return of picks            */
  {
    x.m_pickline = z->zt_pl;
    x.m_order    = z->zt_on;
  }
  z->zt_lines  = 0;
  z->zt_flags |= ZoneInactive;

  k = z->zt_first_bay;
  
  while (k > 0)
  {
    b = &bay[k - 1];
    k = b->bay_next;
    j = b->bay_port - 1;

    b->bay_picks = 0;

    for (m = b->bay_prod_first; m && m <= b->bay_prod_last; m++)
    {
      i = &pw[m - 1];
      h = &hw[i->pw_ptr - 1];

#ifdef DETAIL
  fprintf(stderr, "product=%d module=%d pw_flags=%x hw_flags=%x\n", 
    m, h->hw_mod, i->pw_flags, h->hw_flags);
  fflush(stderr);
#endif
      
      if (flag && (i->pw_flags & BinPicked))
      {
        x.m_module    = m;
        x.m_picked    = i->pw_picked;
        x.m_reference = i->pw_reference;
#ifdef DETAIL
  fprintf(stderr, "pick message\n");
  Bdumpf(&x, sizeof(TPickMessage), stderr);
  fflush(stderr);
#endif
        message_put(0, ModulePickEvent, &x, sizeof(TPickMessage));
      }
      i->pw_ordered = i->pw_picked  = 0;
      i->pw_flags  &= PicksInhibited;
      h->hw_flags  &= SwitchesDisabled;
    }
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave(flag)
register long flag;
{
  message_close();
  ss_close();
  co_close();
  
  exit(flag);
}
 
/* end of dlc.c */
