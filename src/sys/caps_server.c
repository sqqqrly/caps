#define DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    CAPS Server Task Is Started By inetd.
 *
 *  Entry in etc/services   = tcp_server 1150/tcp
 *
 *  Entry in etc/inetd.conf = tcp_server stream tcp nowait root 
 *                                  HOME/bin/caps_server caps_server HOME
 *
 *                            where HOME is the appropriate directory
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  12/18/94   |  tjt  Original implementation.
 *  01/10/95   |  tjt  Name changed from client to server
 *-------------------------------------------------------------------------*/
static char caps_server_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "message.h"
#include "message_types.h"

char HOME[64] = {"/u/mfc"};

extern long catcher();
extern long connection_timer();
extern long ping_timer();
extern long leave();

FILE *fd;                                  /* error and log file             */
char fd_name[64];

char ACK = 0x06;
char SYN = 0x16;
char buf[64];

long ret;
long now;
long count;                               /* message count                   */

unsigned char list[] = {ShutdownRequest};

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=caps_server");
  
  strcpy(HOME, argv[1]);
  chdir(HOME);
  
  sprintf(fd_name, "dat/log/caps_server.log");
  fd = fopen(fd_name, "a+");
 
  now = time(0);
  fprintf(fd, "Server Started: %24.24s\n\n", ctime(&now));
  fflush(fd);
  
  signal(SIGHUP,  SIG_IGN );
  signal(SIGTRAP, catcher);
  signal(SIGINT,  catcher);
  signal(SIGTERM, catcher);
  signal(SIGQUIT, catcher);
  signal(SIGPIPE, catcher);

  message_open();                       /* Open the ipc for new kernel       */
  message_select(list, sizeof(list));
  message_signal(SIGUSR1, leave);
    
/*-------------------------------------------------------------------------*
 *  Handshake With Server
 *-------------------------------------------------------------------------*/
 
   handshake();
   now = time(0);
   fprintf(fd, "Handshake OK: %24.24s\n", ctime(&now));
   fflush(fd);
   
/*-------------------------------------------------------------------------*
 *  Process Messages
 *-------------------------------------------------------------------------*/
   
  while (1)
  {
    signal(SIGALRM, ping_timer);           /* expecting pings                */
    alarm(60);
    
    ret = read(0, buf, 64);                /* read transaction from APU      */
    alarm(0);                              /* cancel ping timer              */
    
    if (!ret)
    {
      fprintf(fd, "Got EOF\n");            /* EOF - server has stopped       */
      leave(1);
    }
    else if (ret < 0)                       /* an error has occurred         */
    {
      fprintf(fd, "read failed: errno=%d\n", errno);
      leave(2);
    }
    ret = write(1, &ACK, 1);                /* ACK back to server            */
    if (ret <= 0)
    {
      fprintf(fd, "write failed: errno=%d\n", errno);
      leave(3);
    }
    if (*buf != SYN)                        /* a real message - not a ping   */
    {
      message_put(0, APURequest, buf, 64);

#ifdef DEBUG
      count++;
      fprintf(fd, "%5d: [%64.64s]\n", count, buf);
      fflush(fd);
#endif  
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Handshake With Server
 *-------------------------------------------------------------------------*/
handshake()
{
  long trys;
  char byte;
  
  for (trys = 0; trys < 3; trys++)
  {
    ret = write(1, &SYN, 1);            /* send synchronize byte             */
    
    if (ret <= 0)
    {
      fprintf(fd, "handshake: write failed: errno=%d\n", errno);
      leave(4);
    }
    signal(SIGALRM, connection_timer);
    alarm(15);
    
    ret = read(0, &byte, 1);            /* read ACK response                 */
    alarm(0);
    if (ret <= 0)
    {
      if (errno == EINTR) leave(5);
      fprintf(fd, "handshake: read failed: errno=%d\n", errno);
      leave(6);
    }
    if (byte == ACK) return 0;
  }
  fprintf(fd, "handshake: failed after 3 attempts\n");
  leave(7);
}
/*-------------------------------------------------------------------------*
 *  Connection Timer
 *-------------------------------------------------------------------------*/
long connection_timer()
{
  now = time(0);
  fprintf(fd, "Timeout: %24.24s\n", ctime(&now));
  fflush(fd);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Ping Timer
 *-------------------------------------------------------------------------*/
long ping_timer()
{
  now = time(0);
  fprintf(fd, "Ping Timeout: %24.24s\n", ctime(&now));
  fflush(fd);
  leave(8);
}

/*-------------------------------------------------------------------------*
 *  Signal Catcher
 *-------------------------------------------------------------------------*/
long catcher( what )
long what;
{
  fprintf(fd, "Caught Signal %d\n", what);
  sleep(10);
  leave(9);
}

/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
long leave(x)
long x;
{
  now = time(0);

  message_close();
  
  fprintf(fd, "Server Stopped %d: %24.24s\n", x, ctime(&now));
  fflush(fd);
  fclose(fd);
  
  exit(x);
}

/* end of caps_server.c */
