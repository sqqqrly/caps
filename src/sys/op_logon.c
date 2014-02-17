/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Operator logon and logoff caps system.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/13/93   |  tjt  Original impletmentation
 *  05/12/94   |  tjt  Added caps_version.h
 *  07/13/94   |  tjt  Fix password entry.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  03/07/96   |  tjt  Added punt() when screen driver not functioning.
 *  08/23/96   |  tjt  Add begin and commit work. 
 *-------------------------------------------------------------------------*/
static char op_logon_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "iodefs.h"
#include "sd.h"
#include "caps_version.h"
#include "message_types.h"
#include "motd.t"

#include "Bard.h"
#include "bard/operator.h"

long leave();                             /* graceful exit routine           */
long punt();                              /* ungraceful exit routine         */

extern unsigned char *getenv();

operator_item op;                         /* database record                 */

char work[80];
char work1[40], work2[40], work3[40];     /* parm space                      */
char work4[40], work5[40], work6[40];
char work7[40], work8[40], work9[40];
char work10[40], work11[40];

long refresh = 1;

char exit_prog[40];

main(argc, argv, arge)
long argc;
char **argv;
char **arge;
{
  register unsigned char t, *p;
  register long n, ret;

#ifdef DEBUG    
  FILE *dd;
  dd = fopen("debug/op_logon.bug", "w");
#endif
  
  putenv("_=op_logon");                   /* name to environemnt             */
  chdir(getenv("HOME"));
  
#ifdef DEBUG
  fprintf(dd, "op_logon()\n");
  
  for (n = 0; n < 32; n++)
  {
    if (!arge[n]) break;
    fprintf(dd, "arge[%d] = [%s]\n", n, arge[n]);
  }
  fflush(dd);
#endif

  signal(SIGQUIT, punt);                 /* log off on any error            */
  signal(SIGHUP,  punt);
  signal(SIGINT,  punt);
  signal(SIGTRAP, punt);
  
#ifdef DEBUG
  fprintf(dd, "now open server\n");
  fflush(dd);
#endif

  sd_open(leave);                         /* open messages to kernel         */
  
#ifdef DEBUG
    fprintf(dd, "sd_server = %d\r\n", sd_server);
    fflush(dd);
    fclose(dd);
#endif
  
  sd_tty_open();
  
  signal(SIGQUIT, leave);                 /* log off on any error            */
  signal(SIGHUP,  leave);
  signal(SIGINT,  leave);
  signal(SIGTRAP, leave);
  
  p = getenv("START");
  if (p) strcpy(exit_prog, p);
  else strcpy(exit_prog, "mmenu");

/*-------------------------------------------------------------------------*
 *  Display the CAPS Logo File                                    
 *-------------------------------------------------------------------------*/

  n = strlen(caps_version);
  memcpy(&motd[1480 - n/2], caps_version, n);

  sd_text(motd);                
  sd_clear(5);                           /* expanding display               */
  
/*-------------------------------------------------------------------------*
 *  Get Operator Name
 *-------------------------------------------------------------------------*/

  p = getenv("LOGNAME");
  if (!p) 
  {
    sd_msg("                          * * * Cannot Find LOGNAME * * *");
    sleep(5);
    leave(0);
  }
  database_open();

  operator_open(READONLY);
  operator_setkey(1);

  memset(&op, 0, sizeof(operator_item));
  strcpy(op.o_op_name, p);
  
  begin_work();
  ret = operator_read(&op, NOLOCK);
  commit_work();
  operator_close();
  database_close();
  
  if (ret)
  {
    sd_msg("                      * * * Operator Not In Database * * *");
    sleep(5);
    leave(0);
  }

/*-------------------------------------------------------------------------*
 *  Store Parameters In Environment
 *-------------------------------------------------------------------------*/

  sprintf(work1, "OPERATOR=%-.8s", op.o_op_name);
  putenv(work1);
  message_put(sd_server, ChangeOperatorEvent, op.o_op_name, 
    strlen(op.o_op_name));
  sprintf(work2, "LEVEL=%c", op.o_op_level[0]);
  putenv(work2);
  sprintf(work3, "PRINTER=%-.8s", op.o_op_printer);
  message_put(sd_server, ChangePrinterEvent, op.o_op_printer, 
    strlen(op.o_op_printer));
  putenv(work3);
  sprintf(work4, "LEGAL1=%-.32s", op.o_op_mm);
  putenv(work4);
  sprintf(work5, "LEGAL2=%-.32s", op.o_op_ops);
  putenv(work5);
  sprintf(work6, "LEGAL3=%-.32s", op.o_op_sys);
  putenv(work6);
  sprintf(work7, "LEGAL4=%-.32s", op.o_op_config);
  putenv(work7);
  sprintf(work8, "LEGAL5=%-.32s", op.o_op_prod);
  putenv(work8);
  sprintf(work9, "LEGAL7=%-.32s", op.o_op_sku);
  putenv(work9);
  sprintf(work10, "LEGAL8=%-.32s", op.o_op_label);
  putenv(work10);

  p = getenv("REFRESH");
  if (p) 
  {
    refresh = atol(p);
    if (refresh < 1) refresh = 1;
  }
  sprintf(work11, "REFRESH=%d", refresh);
  putenv(work11);
  
/*-------------------------------------------------------------------------*
 *  Get Any Keystroke
 *-------------------------------------------------------------------------*/

  strip_space(op.o_op_desc, 32);
  strip_space(op.o_op_name, 8);
  
  sprintf(work, "%c     Hello %-.8s - %-.32s     %c", 
    REVDIM, op.o_op_name, op.o_op_desc, NORMAL);
  sd_cursor(0, 23, 40 - strlen(work) / 2);
  sd_text(work);
  
  sd_echo_flag = 0;
  t = sd_keystroke(0);                   /* anything - no cursor             */
  
  if (t == EXIT) leave(0);
  if (t == 'y' || t == 'Y') get_password();
  
  sd_close();

  execlp(exit_prog, exit_prog, 0);
  krash("op_logon", "Load mmenu", 0);
  exit(1);
}
/*-------------------------------------------------------------------------*
 *  Get New Operator Password  
 *-------------------------------------------------------------------------*/
get_password()
{
  sd_clear_screen();
  sd_tty_close();
  sd_close();
  sleep(2);
  
  printf("\n\nChange Your Password And Logon Again\n\n");

#ifdef INTEL
  system("/bin/passwd");                   /* get new password               */
#else  
  system("/bin/cpw");                      /* get new password               */
#endif

  printf("\r\n\n Goodbye ...\r\n\n");

  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Logoff or Error Exit From CAPS
 *-------------------------------------------------------------------------*/
long leave(flag)
register long flag;
{
  signal(SIGTRAP, punt);                  /* ungraceful exit if any error    */
  
  sd_clear_screen();                      /* clear screen                    */
  sd_screen_on();
  sd_cursor(0, 3, 1);
  printf("\r\n\n Goodbye ...\r\n\n");
  sleep(2);
  sd_tty_close();
  sd_close();
  sleep(1);                               /* wait for server to close        */
  
  kill(0, SIGTERM);                       /* kill process group              */
  
  exit(flag);                             /* server dies on hangup signal    */
}
/*-------------------------------------------------------------------------*
 *  Exit Program When Screen Driver Not Open
 *-------------------------------------------------------------------------*/
long punt()
{
  printf("\r\n\n Goodbye ...\r\n\n");
  kill(0, SIGTERM);
  exit(1);
}

/* end of op_logon.c */
