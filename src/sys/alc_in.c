#define DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Total function input for one port.
 *
 *  Called:				alc_in [port] [device]
 *
 *                   port 		- 0 .. table entry number.
 *                   device	- machine device name
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/12/94   |  tjt  Original implementation.
 *  12/31/94   |  tjt  Add open port only once.
 *  02/18/95   |  tjt  Add port disable.
 *  05/31/95   |  tjt  Add print errors on pickline restore.
 *  06/15/95   |  tjt  Add krash message on alc_init + rp_init failure.
 *  05/18/98   |  tjt  Add IO module as scanner input + ACIOPacketEvent.
 *  05/26/98   |  tjt  Fix copy of last hw map.
 *  05/21/01   |  aha  Accommodate scanner text longer than 16 bytes, now 24.
 *-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*
 *                  <--len = 9 + scan -->   
 *  Scanner Read     xxxx 10 xxx ssss ss cc\n - read from port
 *  Buffer Contents              ------- ----
 *                                  |     |
 *                                  |     +---> check and line feed
 *                                  +---> scan - actual length - no padding
 *
 *  Scanner Message  xxxx 10 xxx ssss ss00 0000 0000 p - sent to CAPS kernel
 *  Contents         ---- -- --- ------------------- -
 *                    |   |   |           |          |
 *                    |   |   |           |          +---> port number 0 ..
 *                    |   |   |           + ---> scan, 24 bytes, null padded.
 *                    |   |   +---> module number 000 ..
 *                    |   +---> switch action message
 *                    +---> command module 0001 ..
 *-------------------------------------------------------------------------*/
static char alc_in_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <signal.h>
#include "file_names.h"
#include "message_types.h"
#include "caps_messages.h"
#include "alc_packet.h"
#include "eh_nos.h"
#include "ss.h"
#include "co.h"

extern leave();
extern catcher();
extern timeout();

unsigned char prog[16];                   /* name of this program            */
TPort number;                             /* number of this port             */
unsigned char device[16];                 /* device name                     */

long port;                                /* port file id                    */

unsigned char list[] = {ShutdownRequest, PortInitializeRequest,
PortMarkplaceRequest, InitializeEvent, ConfigureEvent,
RestoreplaceEvent, PortDisableRequest, PortEnableRequest};

main(argc, argv)
long argc;
char **argv;
{
  register long ret;                      /* packet read return              */
  register long len;                      /* packet length                   */
  TPacketItem x;                          /* packet structures               */
  char debug_msg[35];

  debug_msg[34] = '\0';
  
  chdir(getenv("HOME"));                  /* insure in HOME directory        */
  
  if (argc < 3)
  {
    krash("main", "Missing Arguments", 1);
  }
  number = atol(argv[1]);                 /* port number                     */
  strcpy(device, argv[2]);                /* port device                     */
   
  sprintf(prog, "_=alc_in%d", number);
  putenv(prog);                           /* store program name              */

  setpgrp();

  signal_catcher(0);                      /* catch various signals           */
  
  message_open();
  message_select(list, sizeof(list));
  message_signal(SIGUSR1, catcher);
   
#ifdef DEBUG
  fprintf(stderr, "%s started:  msgtask=%d\n", prog, msgtask);
#endif

  ss_open();
  co_open();
  
  if (sp->sp_total_function == 'n') leave(0);

  if (number < 0 || number >= coh->co_ports)
  {
    krash("main", "invalid port number", 1);
  }
  if (sp->sp_total_function != 's' )
  {
    port = ac_open(device);               /* only openned once !!!           */
    if (port < 0)
    {
      krash("main", "Port Open Failed", 0);
      leave(1);
    }
  }
  strcpy(po[number].po_name, device);     /* name of port                    */
  po[number].po_status   = 'x';           /* status is closed                */
  po[number].po_disabled = 'n';           /* initially is enabled            */
  po[number].po_id     = msgtask;         /* alc replaces with self on init  */
  po[number].po_id_in  = msgtask;         /* this task                       */
  po[number].po_flags  = IsTotalFunction; /* what we are                     */
  
  while (1)
  {
    while (po[number].po_status != 'y' || sp->sp_total_function == 's')
    {
      pause();                            /* awake on any signal             */
    }
    len = ret = ac_read(port, &x);        /* get ac message                  */
    
#ifdef DEBUG
  if (len > 0) fprintf(stderr, "ac_read: len=%d [%*.*s]\n",len,len,len,&x);
  else fprintf(stderr, "ac_read: ret=%d\n", ret);
#endif

    if (ret < 0) continue;                /* interrupted by signal           */

    if (!ret) len = 9;                    /* length of a bad packet          */
    if (len > MessageText) len = MessageText;
    
    if (len == 10 && memcmp(x.SwitchPacket.packet_command, "10", 2) == 0)
    {
      x.SwitchPacket.packet_port = number;/* append port number              */
      message_put(0, ACSwitchPacketEvent, &x, sizeof(TSwitchMessage));
      continue;
    }
    if (len > 10 && memcmp(x.IOPacket.packet_command, "10", 2) == 0)/*F051898*/
    {
      x.IOPacket.packet_port = number;  /* append port number  */
      memset(x.IOPacket.packet_text + len - 9, 0, 33 - len);
#ifdef DEBUG
      memcpy(debug_msg, x.IOPacket.packet_text, 24);
      debug_msg[24] = '\0';
      fprintf(stderr, "x.IOPacket.packet_text = %s\n", debug_msg); 
      fprintf(stderr, "x.IOPacket.packet_port = %d, or %c\n",
              x.IOPacket.packet_port, x.IOPacket.packet_port); 
#endif
      message_put(0, ACIOPacketEvent, &x, sizeof(TIOMessage));
#ifdef DEBUG
      fprintf(stderr, "message type ACIOPacketEvent sent.\n");
      fprintf(stderr, "size of TIOMessage = %d\n", sizeof(TIOMessage));
      fprintf(stderr, "size of x.IOPacket = %d\n", sizeof(x.IOPacket));
#endif
      continue;
    }
    if (len == 9 && memcmp(x.NullPacket.packet_command, "00", 2) == 0)
    {
      if (x.NullPacket.packet_error == 'X') continue;
      if (x.NullPacket.packet_error == 'Y') continue;
    }
    message_put(0, ACErrorPacketEvent, &x, len);
  }
  leave(1);
}
/*-------------------------------------------------------------------------*
 *  Message Catcher
 *-------------------------------------------------------------------------*/
catcher(who, type, buf, len)
register long who;
register long type;
register TCapsMessageItem *buf;
register long len;
{
  long pid=0, status=0;
  char command[80];
  static TErrorMessage err = {ERR_RP, "See Printer"};
  
#ifdef DEBUG
  fprintf(stderr, "got message: who=%d type=%d len=%d\n", who, type, len);
  if (len > 0) Bdump(buf, len);
#endif

  switch(type)
  {
    case ShutdownRequest:
    case ShutdownEvent:   leave(0);
     
    case PortInitializeRequest:

       if (po[number].po_disabled == 'y')
       {
         po[number].po_status = 'i';

         message_put(coh->co_id, PortInitializeEvent, 0, 0);
         break;
       }
       po[number].po_status = 'x';
      
       if (fork() == 0)
       {
         ss_close();
         co_close();
         execlp("alc_init", "alc_init", device, "-m", 0);
         exit(1);
       }
       pid = wait(&status);
      
       if (!status && pid > 0) po[number].po_status = 'i';
       else
       {
         sprintf(command, "alc_init %s pid=%d status=0x%04x status=%d",
           device, pid, status, status);
         krash("catcher", command, 0);
       }
       message_put(coh->co_id, PortInitializeEvent, 0, 0);
       break;
              
    case InitializeEvent:
    
      if (po[number].po_status == 'i') po[number].po_status = 'n';
      break;
              
    case ConfigureEvent:
    case RestoreplaceEvent:
     
      if (po[number].po_disabled  == 'y')
      {
        po[number].po_status = 'x';
        break;
      }
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
    
      if (fork() == 0)
      {
        ss_close();
        co_close();
        execlp("alc_init", "alc_init", device, "-p", 0);
        exit(1);
      }
      pid = wait(&status);
      
      if (status || pid <= 0) 
      {
        sprintf(command, "alc_init %s pid=%d status=0x%04x",
          device, pid, status);
        krash("catcher", command, 0);

        message_put(who, ClientMessageEvent, &err, strlen(err.m_text) + 1);
        break;
      }
      if (fork() == 0)
      {
        ss_close();
        co_close();
        execlp("rp_init", "rp_init", 0);
        exit(1);
      }
      pid = wait(&status);
      
      if (status || pid <= 0) 
      {
        sprintf(command, "rp_init %s pid=%d status=0x%04x",
          device, pid, status);
        krash("catcher", command, 0);

        message_put(who, ClientMessageEvent, &err, strlen(err.m_text) + 1);
        break;
      }
      po[number].po_status = 'y';
      message_put(0, PortEnableEvent, &number, sizeof(TPortMessage));
      break;
      
    default: break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave(flag)
register long flag;
{
  if (port > 0) ac_close(port);
  message_close();
  ss_close();
  co_close();
  
  exit(flag);
}
 
/* end of alc_in.c */
