/* #define DEBUG */
/* #define WATCHDOG */
/*-------------------------------------------------------------------------*
 *  Custom Code:    WATCHDOG - for use with kernel2
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Kill any caps programs.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/23/93   |  tjt  Message added.
 *-------------------------------------------------------------------------*/
static char stop_caps_c[] = "%Z% %M% %I% (%G% - %U%)";

/*  stop_caps.c
 *
 *  kills any task with a bin/ preface
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/stat.h>

#include "ss.h"
#include "message.h"
#include "kernel.h"
#include "kernel_types.h"
#include "message_types.h"

FILE *junk;                               /* ps -a file                      */
char line[256];                           /* one line                        */
short len;                                /* length of line                  */
char pid[6];                              /* pid of process                  */
char string[40];                          /* command string                  */
short found;                              /* true if bin/                    */
short count = 0;                          /* tasks killed                    */
long  x, y;
jmp_buf save;

main()
{
  extern done();
  extern timeout1(), timeout2(), ignore();
  register char *p, *q, *r;
  register long k, ret;
  struct stat sbuf;
  char tname[16], command[80], name[64];

  putenv("_=stop_caps");                  /* nmame to environ                */
  chdir(getenv("HOME"));                  /* to home directory               */

  setpgrp();                              /* so quit doesn't kill scripts    */

  signal_catcher(1);                      /* never stop for anything         */
  
  printf("Stopping CAPS Operations\n\n");
  
  //system("save_segs");

#ifdef WATCHOUT
  system("$ETC/stop_watchdog >>dat/log/watch.dog");
#endif

  signal(SIGALRM, timeout1);
  alarm(5);
  if (setjmp(save) != 0) goto failed;
    
  ret = -1;
  ret = message_open();
  alarm(0);
  
  if (!ret)
  {
    signal(SIGALRM, timeout2);
    alarm(20);
    
    if (setjmp(save) != 0) goto failed;
    
    ss_open();
    if (sp->sp_running_status == 'y')
    {
      message_put(0, MarkplaceRequest, 0, 0);
      sleep(3);
    }
    ss_close_save();
    
    printf("Sending Shutdown To Kernel\n");

    message_put(KernelDestination, KernelShutdown, 0, 0);
    message_close();
    signal(SIGUSR1, done);
  	 pause();
    alarm(0);
  }
failed:

  sleep(3);
  
#ifndef DEBUG
  printf("Killing Any Remaining Caps Tasks\n");

  tmp_name(tname);
  sprintf(command, "ps -ef >%s", tname);
  system(command);
  junk = fopen(tname, "r");
  if (junk == 0)
  {
    printf("*** stop_caps failed on open temp file\n\n");
    return 1;
  }
  fgets(line, 256, junk);                 /* skip heading line               */

  while (fgets(line, 256, junk))          /* read one line                   */
  {
    len = strlen(line) - 1;					/* length less linefeed            */
    if (len == 0) continue;               /* null line                       */
    line[len] = 0;                        /* null to linefeed                */
    q = line + 9;                         /* first byte of pid               */
    r = line + 53;                        /* program name                    */
    p = pid;                              /* first byte of pid               */
    while (*q == 0x20) q++;               /* skip leading spaces             */
    while (*q != 0x20) *p++ = *q++;       /* get pid                         */
    *p = 0;                               /* add null                        */
    found = 0;                            /* set to not bin/                 */

    while (*r && *r != 0x20) r++;         /* to end of name                  */
    *r-- = 0;                             /* null at end                     */
    
    while (*r != '/' && *r != 0x20) r--;  /* get only program name part      */
    r++;                                  /* program name                    */

    if (strcmp(r, "stop_caps") == 0) continue;    /* do not kill self        */
    if (strcmp(r, "plc_kit_scan") == 0) continue; /* do not kill scan plc    */
    
    sprintf(name, "%s/bin/%s", getenv("HOME"), r);

#ifdef DEBUG
    fprintf(stderr, "pid=[%s] name=[%s]\n", pid, name); 
#endif

    ret = stat(name, &sbuf);
    if (ret < 0) continue;                /* don't kill if not in mfc/bin    */
    
    printf("Killing: PID = %s  Program = %s\n", pid, name);
    if (kill(atoi(pid), 9) >= 0) count++;
  }
#endif

  if (count) printf("CAPS has been terminated\n\n");
  else printf("CAPS was not running\n\n");
  unlink(tname);
  printf("Stop CAPS - All Done\n\n");

  return 0;
}
done()
{
  printf("Kernel Has Signaled Shutdown Is Done\n");
  count = 1;
}
timeout1()
{
  printf("Kernel Is Not Operating?\n");
  longjmp(save, 1);
}
timeout2()
{
  printf("Timeout Waiting For Kernel To Shutdown\n");
  longjmp(save, 1);
}
ignore()
{
  printf("\n*** Ignore Error Messages Above - No Kernel Queue ***\n\n");
}

/* end of stop_caps.c */
