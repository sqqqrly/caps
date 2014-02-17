/* #define DEBUG  */
/* #define DEBUG2 */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    TCP/IP  Client/Sender 
 *                  Wakes on sp_to_count increase or SIGUSR2.
 *
 *  Usage:          caps_client  [host]   [port]
 *                  apu_client   [host]   [port]
 *
 *            i.e.  caps_client  hanes 1150  &
 *                  apu_client   caps  1150  &
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  12/18/94   |  tjt Original implementation.
 *  01/10/95   |  tjt Name changed from server to client.
 *  01/11/95   |  tjt Revised to start by hanes engine.
 *  07/21/95   |  tjt Revise Bard calls.
 *  04/19/96   |  tjt Add sp_to_xmit count.
 *-------------------------------------------------------------------------*/
static char caps_client_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>

#include "ss.h"
#include "global_types.h"

#define TIMEOUT 30
#define INTERVAL 0                       /* time between messages            */

#include "Bard.h"
#include "bard/queue.h"

FILE *fd;
char *fd_name = "dat/log/caps_client.log";

// Signal handlers
void interrupt_handler(int signum);
void connect_timer(int signum);
void wakeup(int signum);

struct hostent *h, *gethostbyname();
struct sockaddr_in sock_fd;

int    s;                                 /* socket handle                   */
int    ret;

char ACK = 0x06;
char NAK = 0x15;
char SYN = 0x16;

char buf[128];
long now;
long trys;

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=caps_client");
  chdir("/mfc");

  database_open();

  fd = fopen(fd_name, "a+");

  now = time(0);
  
  fprintf(fd, "Client %d Started. %24.24s\n", getpid(), ctime(&now));
  fprintf(fd, "caps_client %s %s\n\n", argv[1], argv[2]);
  
  if (argc < 3)
  {
    fprintf(fd, "Arguments are [host] and [port]\n");
    fprintf(fd, "*** Client Has Terminated ***\n\n");
    leave(1);
  }
  fflush(fd);
  
  signal( SIGHUP,  SIG_IGN );             /* no hangup                       */
  signal( SIGUSR2, wakeup );              /* wake from sleep on signal       */
    
  signal( SIGINT,  interrupt_handler );   /* catch these and report          */
  signal( SIGTERM, interrupt_handler );
  signal( SIGQUIT, interrupt_handler );
  signal( SIGALRM, interrupt_handler );
  signal( SIGPIPE, interrupt_handler );
  
/*-------------------------------------------------------------------------*
 *  Body of Client
 *-------------------------------------------------------------------------*/
  trys = 0;                               /* clear connect attempts          */
  
  while (trys < 4)                        /* approximately 2 minutes         */
  {
    trys++;                               /* count attempts                  */
    
    if (!open_port(argv[1], argv[2]))     /* open socket to server           */
    {
      if (!handshake())                   /* handshake with client           */
      {
        now = time(0);
        fprintf(fd, "Handshake OK: %24.24s\n", ctime(&now));
        fflush(fd);
        
        queue_open(AUTOLOCK);             /* open queue database             */
        ss_open();

        send_transactions();              /* send data until error           */

        leave(2);                         /* die on lost connection          */
      }
    }
    if (s) close(s);                      /* close the socket                */
    s = 0;                                /* mark socket closed              */
    sleep(15);                            /* avoid fast respawn              */
  }
  leave(3);                               /* die after trys                  */
}
/*-------------------------------------------------------------------------*
 *  Send Tranactions Until Stopped
 *-------------------------------------------------------------------------*/
send_transactions()
{
  register long k, number;
  queue_item q;

#ifdef DEBUG
  fprintf(fd, "send_transactions()\n");
#endif

  while (1)                               /* send forever                    */
  {
    queue_setkey(1);                      /* reset to top of database        */
    number = 0;                           /* burst counter                   */
  
    while (!queue_next(&q, LOCK))         /* read oldest queued message      */
    {
#ifdef DEBUG
  fprintf(fd, "Sending: [%64.64s]\n", q.q_queue_text);
  fflush(fd);
#endif      
      ret = write(s, q.q_queue_text, sizeof(q.q_queue_text));
      if (ret <= 0)
      {
        fprintf(fd, "write() failed: errno=%d\n", errno);
        fflush(fd);
        return 1;
      }
      signal(SIGALRM, connect_timer);      /* allow for response  */
      alarm(TIMEOUT);
  
      ret = read(s, buf, 1);
      alarm(0);

      if (ret <= 0)                        /* read fails on lost server !!!  */
      {
        if (errno == EINTR) 
        {
          fprintf(fd, "transaction %d ACK timeout\n", number + 1);
          fflush(fd);
          return 1;
        }
        fprintf(fd, "read() failed: errno=%d\n", errno);
        fflush(fd);
        return 1;
      }
      if (*buf != ACK)                     /* not an ACK - assume the worst  */
      {
        fprintf(fd, "last write not ACKed: Got 0x%02x\n", *buf); 
        return 1;
      }
      queue_delete();                      /* delete this message now        */
      sp->sp_to_xmit += 1;                 /* now increment xmit count       */
    
      number++;                            /* count transactions             */
 
      if (INTERVAL > 0) sleep(INTERVAL);   /* avoid fast transmission        */
    }
#ifdef DEBUG
  fprintf(fd, "queue db is empty\n");
#endif

    sp->sp_to_xmit = sp->sp_to_count;      /* clear to zero anyway ???       */
    
    for (k = 0; k < 15; k++)               /* loop for awhile                */
    {
      sleep(1);                            /* sleep a little                 */
      if (sp->sp_to_count > sp->sp_to_xmit) break;   /* check any new items  */
    }
    if (sp->sp_to_count > sp->sp_to_xmit) continue;  /* go send transaction  */
    
    memset(q.q_queue_text, 0x20, sizeof(q.q_queue_text));
    q.q_queue_text[0] = SYN;

#ifdef DEBUG2
    fprintf(fd, "ping\n");
    fflush(fd);
#endif

    ret = write(s, q.q_queue_text, sizeof(q.q_queue_text));
    if (ret <= 0)
    {
      fprintf(fd, "write() failed: errno=%d\n", errno);
      fflush(fd);
      return 1;
    }
    signal(SIGALRM, connect_timer);        /* allow for response  */
    alarm(TIMEOUT);
  
    ret = read(s, buf, 1);
    alarm(0);

    if (ret <= 0)                          /* read fails on lost client !!!  */
    {
      if (errno == EINTR)
      {
        fprintf(fd, "SYN ACK timeout\n");
        fflush(fd);
        return 1;
      }
      fprintf(fd, "read() failed: errno=%d\n", errno);
      fflush(fd);
      return 1;
    }
    if (*buf != ACK)                       /* not an ACK - assume the worst  */
    {
      fprintf(fd, "last SYN write not ACKed: Got 0x%02x\n", *buf); 
      return 1;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Initial Handshake With Client
 *-------------------------------------------------------------------------*/
handshake()
{
  long trys;

  trys = 0; 

  while (1)
  {
    signal(SIGALRM, connect_timer);        /* allow for response  */
    alarm(TIMEOUT);
  
    *buf = 0;									    /* clear buffer                   */

    ret = read(s, buf, 1);                 /* read an SYN                    */
    alarm(0);
    
    if (ret <= 0)
    {
      if (errno == EINTR)
      {
        fprintf(fd, "SYN handshake read timeout\n");
        fflush(fd);
        return 1;
      }
      fprintf(fd, "handshake: read() failed: errno=%d\n", errno);
      fflush(fd);
      return 1;
    }
    if (*buf == SYN) break;                /* got what we expected           */
  
    ret = write(s, &NAK, 1);               /* write an NAK                   */
    if (ret <= 0)
    {
      fprintf(fd, "handshake: write() failed: errno=%d\n", errno);
      fflush(fd);
      return 1;
    }
    trys++;                                /* increment attempt count        */
    if (trys > 3)
    {
      fprintf(fd, "handsake: failed after 3 attempts\n");
      fflush(fd);;
      return 1;
    }
  }
#ifdef DEBUG
  fprintf(fd, "Got SYN=0x%02x\n", *buf);
  fflush(fd);
#endif

  ret = write(s, &ACK, 1);                 /* write an ACK                   */
  if (ret <= 0)
  {
    fprintf(fd, "handshake: write() failed: errno=%d\n", errno);
    fflush(fd);
    return 1;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open Connection To Host - Fails If No Connection In TIMEOUT Seconds
 *-------------------------------------------------------------------------*/
open_port(host, port)
char *host, *port;
{

/*-------------------------------------------------------------------------*
 *  Open Socket - Using TCP Protocol
 *-------------------------------------------------------------------------*/
  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s < 0)
  {
    fprintf(fd, "socket() failed: errno=%d\n", errno);
    fflush(fd);
    return 1;
  }
#ifdef DEBUG
  fprintf(fd, "socket()\n");
  fprintf(fd, "  s  [%d]\n", s);
  fprintf(fd, "\n");
#endif
/*-------------------------------------------------------------------------*
 *  Get Host By Name 
 *-------------------------------------------------------------------------*/

  h = gethostbyname(host);
  if (!h)
  {
    fprintf(fd, "gethostbyname() failed: errno=%d\n", errno);
    fflush(fd);
    return 1;
  }
#ifdef DEBUG
  fprintf(fd, "gethostbyname()\n");
  fprintf(fd, "  h_name     [%s]\n",   h->h_name);
  fprintf(fd, "  h_addrtype [%d]\n",   h->h_addrtype);
  fprintf(fd, "  h_length   [%d]\n",   h->h_length);
  fprintf(fd, "  h_addr     [%08x]\n", *(long *)h->h_addr);
  fprintf(fd, "\n");
#endif
  
/*-------------------------------------------------------------------------*
 *  Connect to Host Computer + Host Port
 *-------------------------------------------------------------------------*/
  memset(&sock_fd, 0, sizeof(sock_fd));
  sock_fd.sin_family = AF_INET;
  sock_fd.sin_port   = htons(atoi(port));
  memcpy(&sock_fd.sin_addr, h->h_addr, h->h_length);
  
  fprintf(fd, "Connecting To %d.%d.%d.%d Port %d\n", 
    (sock_fd.sin_addr.s_addr >> 24) & 0xff, (sock_fd.sin_addr.s_addr >> 16) & 0xff,
    (sock_fd.sin_addr.s_addr >> 8) & 0xff,   sock_fd.sin_addr.s_addr & 0xff,
    ntohs(sock_fd.sin_port));

  while (1)
  {
    signal(SIGALRM, connect_timer);
    alarm(TIMEOUT);
  
    ret = connect(s, &sock_fd, sizeof(sock_fd));

    alarm(0);
    if (!ret) break;                     /* we have a connection             */
    if (errno == EINTR)                  /* interrupt by alarm               */
    {
      fprintf(fd, "connect timeout\n");
      fflush(fd);

      close(s);                          /* close the socket                 */
      s = 0;                             /* mark socket as closed            */
      return 1;
    }
    if (ret < 0)                         /* an error return                  */
    {
      fprintf(fd, "connect() failed: errno=%d\n", errno);
      fflush(fd);
      return 1;
    }
  }
#ifdef DEBUG
  fprintf(fd, "connect()\n");
  fprintf(fd, "  sin_family [%d]\n", sock_fd.sin_family);
  fprintf(fd, "  sin_port   [%d]\n", sock_fd.sin_port);
  fprintf(fd, "  sin_addr   [%08x]\n", sock_fd.sin_addr.s_addr);
  fprintf(fd, "\n");
#endif
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Connection Timeout
 *-------------------------------------------------------------------------*/
void connect_timer(int signum)
{
  now = time(0);

  fprintf(fd, "Timeout: %24.24s\n", ctime(&now));
  fflush(fd);
  
  // signal handler must be void
  //return 0;
}

/*-------------------------------------------------------------------------*
 *  Wakeup On New Data - Signal From Any Engine
 *-------------------------------------------------------------------------*/
void wakeup(int signum)
{
  signal(SIGUSR2, wakeup);                /* ignore any queue signals       */

#ifdef DEBUG
  fprintf(fd, "Got Data Wakeup\n");
  fflush(fd);
#endif

  // signal handler must be void
  //return 0;
}

/*-------------------------------------------------------------------------*
 *  Signal Handler
 *-------------------------------------------------------------------------*/
void interrupt_handler(int signum)
{
  switch (signum)
  {
    case SIGHUP:   fprintf(fd, "Got signal SIGHUP\n");     break;
    case SIGINT:   fprintf(fd, "Got signal SIGINT\n");     break;
    case SIGTERM:  fprintf(fd, "Got signal SIGTERM\n");    break;
    case SIGQUIT:  fprintf(fd, "Got signal SIGQUIT\n");    break;
    case SIGPIPE:  fprintf(fd, "Got signal SIGPIPE\n");    break;
    default:       fprintf(fd, "Got signal %d\n", signum); break;
  }
  leave(4);
}
/*-------------------------------------------------------------------------*
 *  Graceful and Ungraceful Exit
 *-------------------------------------------------------------------------*/
leave(x)
long x;
{
  ss_close();
  
  if (s) close(s);

  queue_close();
  database_close();

  now = time(0);
  fprintf(fd, "Client Stopped %d: %24.24s\n", x, ctime(&now));
  fflush(fd);
  
  exit(x);
}

/* end of caps_client.c */
