/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Initiates CAPS and Computer Shutdown.
 *                  Requires ADMIN authority.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 11/30/93    |  tjt  Original Implementation.
 * 01/23/95    |  tjt  Renamed from logout.
 * 04/23/95    |  tjt  Add order file rebuild option.
 * 07/08/99    |  aha  Modifying system shutdown to use /tcb/bin/asroot.
 * 11/29/99    |  aha  Corrected system call for shutdown command and for
 *             |       for Purge parameter, now call /u/mfc/etc/order_clear_
 *             |       script.
 *-------------------------------------------------------------------------*/
#ident "(C) Kingway Material Handling Company %M% %H%"
static char caps_logout_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "message_types.h"
#include "kernel_types.h"
#include "sd.h"
#include "ss.h"

extern void leave();
extern void tick(int);
extern close_all();

long TIMEOUT = 100;

long tick_count = 0;

FILE *tty0;

char what[16];

main(argc, argv)
long argc;
char **argv;
{
  long who, type;

  putenv("_=caps_logout");
  chdir(getenv("HOME"));

  if (strcmp(argv[1], "-test") == 0)
  {
    system("/tcb/bin/asroot shutdown -g0 -y");
    printf("Shutdown Has Failed\n\n");
  }
  if (argc < 3)
  {
    printf("\r\n  You Must Give The Secret Word To Shutdown !!!\r\n");
    exit(0);
  }
  if (strcmp(argv[1], "I_Really_Mean_It") != 0)
  {
    printf("\r\n  You Must Give The Secret Word To Shutdown !!!\r\n");
    exit(0);
  }
  strcpy(what, argv[2]);

  setpgrp();

  ss_open();
  sp->sp_in_process_status = 's';
  ss_close_save();

  signal(SIGUSR1, leave);

  message_open();
  message_put(KernelDestination, KernelShutdown, 0, 0);
  message_close();

  sleep(2);
  printf("\r\n  CAPS Shutdown Process Is Beginning ..\r\n");
  printf("\r\n  CAPS Shutdown Requested\r\n");
  /* tick(1);*/
  leave();

  while (1) {pause();}                   /* loop forever on signals          */
}
/*-------------------------------------------------------------------------*
 *  Caught SIGUSR1 Signal From Kernel
 *-------------------------------------------------------------------------*/
void leave()
{
  /* alarm(0); */
  printf("\r\n  CAPS Has Shutdown\r\n");
  close_all();
}
/*-------------------------------------------------------------------------*
 *  Fix Status Of CAPS And Die Now !!!
 *-------------------------------------------------------------------------*/
close_all()
{
  ss_open();
  sp->sp_in_process_status = 'x';
  sp->sp_running_status = 'x';
  ss_close_save();

  if (strcmp(what, "Purge") == 0)
  {
    printf("\r\nOrder Purge Is Starting\r\n\n");
    TIMEOUT = 500;
    tick_count = 0;
    tick(1);
    system("etci/order_clear_script >dat/log/rebuild.log 2>&1");
  }
  else if (strcmp(what, "Rebuild") == 0)
  {
    printf("\r\nOrder Purge Is Starting\r\n\n");
    TIMEOUT = 2000;
    tick_count = 0;
    tick(1);
    system("etci/order_rebuild_script >dat/log/rebuild.log 2>&1");
  }
  printf("\r\n  System Shutdown Is Starting\r\n");

  system("/tcb/bin/asroot shutdown -g0 -y");     /* aha 070899 */
}
/*-------------------------------------------------------------------------*
 *  This Is A Ticking Time Bomb Which Detonates Once Started !!!
 *--------------------------------------------------------------------------*/
void tick(p)
int p;
{
  tick_count += 1;

  printf(".");
  if (!(tick_count % 60)) printf("\r\n");
  fflush(stdout);

  if (tick_count > TIMEOUT)
  {
    printf("\r\n  Timeout Waiting For CAPS\r\n");
    close_all();
  }
  signal(SIGALRM, tick) (1);
  alarm(1);
}

/* end of caps_logout.c */
