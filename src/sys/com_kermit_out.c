/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Custom Code:    SERVER - Kermit runs without kermon.
 *                  NOBYE  - Do not close server on exit.
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Transaction Output To Kermit/kermon.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/14/95   |  tjt  Rewrite of com_menu_spc.
 *-------------------------------------------------------------------------*/
static char com_kermit_out_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termio.h>
#include <fcntl.h>
#include <signal.h>

#include "eh_nos.h"
#include "iodefs.h"
#include "ss.h"
#include "sd.h"
#include "file_names.h"

#define MAX      40                          /* max number of files          */
#define TIMEOUT  900                         /* max file xfer is 15 minutes  */

extern leave();
void catcher(int signum);                    /* Signal handler */

long have_server = 0;                        /* got a response               */

char dname[64] = "dat/files";                /* directory to transfer        */
char fname[64] = "zt";                       /* file to transfer             */

long com_fd = 0;
char comdev[32];                             /* communication port           */

struct termio raw_tty, sane_tty;             /* port sttributes              */

short ONE = 1;
short FNO = 12;

long pid, status;

main(argc, argv)
long argc;
char **argv;
{
  register unsigned char t;
  char cbuf[80];
  
  putenv("_=com_kermit_out");
  chdir(getenv("HOME"));

#ifdef DEBUG
  fprintf(stderr, "com_kermit_out()\n");
  fflush(stderr);
#endif

  if (argc >= 3) 
  {
    strcpy(dname, argv[1]);
    strcpy(fname, argv[2]);
  }
  ss_open();
  sd_open();
  
  get_com_port();

#ifndef SERVER
  eh_post(LOCAL_MSG, "Connecting to PC KERMON");

  com_open();
  if (com_fd == 0) leave(1);                 /* open has failed              */

  sprintf(cbuf, "%cLOAD KERMIT XTRANS%c", 0x02, 0x03);
  write(com_fd, cbuf, strlen(cbuf));
  sleep(5);

  ioctl(com_fd, TCSETAW, &sane_tty);         /* set to sane mode             */
#endif

  get_directory_info();                     /* ask for remote directory      */

  if (have_server) send_file();

  leave(0);
}
/*-------------------------------------------------------------------------*
 *  Send File To PC.
 *
 *  #
 *  # Send File To PC
 *  #
 *  rm /usr/spool/uucp/LCK*
 *  cd dat/files
 *  echo "set line /dev/tty?
 *  set baud 9600
 *  send zt
 *  quit" | kermit
 *
 *-------------------------------------------------------------------------*/
send_file()
{
  FILE *fd;
  
#ifdef DEBUG
  fprintf(stderr, "send_file()\n");
#endif

  unlink("com/kermit_send");

  fd = fopen("com/kermit_send", "w");
  if (fd == 0) krash("send_file", "open com/kermit_send", 1);
  
  fprintf(fd, "#\n# Send File To PC\n#\n");
  fprintf(fd, "rm /usr/spool/uucp/LCK*\n");
  fprintf(fd, "cd %s\n", dname);
  fprintf(fd, "echo \"set line %s\nset baud 9600\n", comdev);
  fprintf(fd, "send %s\nquit\" | kermit\n", fname);

  fclose(fd);

  eh_post(LOCAL_MSG, "Transfering File To PC");
  
  signal(SIGALRM, catcher);
  alarm(TIMEOUT);   
  
  if ((pid = fork()) == 0)
  {
    system ("chmod 777 com/kermit_send 1>com/kermit_send_err 2>&1");
    sleep(1);
    system ("com/kermit_send 1>>com/kermit_send_err 2>&1");
    exit(0);
  }
  pid = wait(&status);
  
  alarm(0);

  if (pid < 0 || status > 0)
  {
    eh_post(LOCAL_MSG, "PC Kermit Is Not Responding");
    return 0;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Fetch Directory From the PC
 *
 *  #
 *  # Fetch Dir From PC
 *  #
 *  rm /usr/spool/uucp/LCK* 1>/dev/null 2>&1
 *  cd com
 *  echo "set line /dev/tty?
 *  set baud 9600
 *  remote dir
 *  quit " | kermit 1>tmp?? 2>/dev/null
 *-------------------------------------------------------------------------*/
get_directory_info()
{
  char command[80];
  FILE *fd;

#ifdef DEBUG
  fprintf(stderr, "get_directory_info()\n");
#endif
  
  unlink("com/kermit_dir");
  unlink("com/kermit_dir_file");

  fd = fopen("com/kermit_dir", "w");
  if (fd == 0) krash("get_directory_info", "open com/kermit_dir", 1);
  
  fprintf(fd, "#\n# Fetch Dir From PC\n#\n");
  fprintf(fd, "rm /usr/spool/uucp/LCK* 1>/dev/null 2>&1\n");
  fprintf(fd, "cd com\n");
  fprintf(fd, "echo \"set line %s\nset baud 9600\n", comdev);
  fprintf(fd, "remote dir\nquit\" | kermit 1>kermit_dir_file 2>/dev/null\n");
  
  fclose(fd);

  signal(SIGALRM, catcher);
  alarm(30);
  
  if ((pid = fork()) == 0)
  {
    system ("chmod 777 com/kermit_dir 1>com/kermit_dir_err 2>&1");
    sleep(1);
    system("com/kermit_dir 1>>com/kermit_dir_err 2>&1");
    unlink("com/kermit_dir_file");
    system("com/kermit_dir 1>>com/kermit_dir_err 2>&1");
    exit(0);
  }
  pid = wait(&status);
  
  alarm(0);

  if (pid < 0 || status > 0)
  {
    eh_post(LOCAL_MSG, "PC Kermit Is Not Responding");
    return 1;
  }
  fd = fopen("com/kermit_dir_file", "r");
  if (fd == 0)
  {
    eh_post(LOCAL_MSG, "PC Kermit Is Not Responding");
    return 1;
  }
  fclose(fd);

  have_server = 1;

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Goodbye To PC
 *-------------------------------------------------------------------------*/
say_goodbye()
{
  char command[80];
  FILE *fd;

#ifdef DEBUG
  fprintf(stderr, "say_goodbye()\n");
  fflush(stderr);
#endif

  eh_post(LOCAL_MSG, "Stopping PC Kermit");

  unlink("com/kermit_bye");

  fd = fopen("com/kermit_bye", "w");
  if (fd == 0) krash("get_directory_info", "open com/kermit_bye", 1);
  
  fprintf(fd, "#\n# Stop Server On PC\n#\n");
  fprintf(fd, "rm /usr/spool/uucp/LCK*\n");
  fprintf(fd, "echo \"set line %s\nset baud 9600\n", comdev);
  fprintf(fd, "bye\nquit\" | kermit\n");
  
  fclose(fd);

  signal(SIGALRM, catcher);
  alarm(30);
  
  if ((pid = fork()) == 0)
  {
    system ("chmod 777 com/kermit_bye 1>com/kermit_bye_err 2>&1");
    sleep(1);
    system("com/kermit_bye 1>>com/kermit_bye_err 2>&1");
    exit(0);
  }
  pid = wait(&status);
  
  alarm(0);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Alarm Catcher - Kills Last Active Task
 *-------------------------------------------------------------------------*/
void catcher(int signum)
{
#ifdef DEBUG
  fprintf(stderr, "catcher(): got timeout\n");
#endif

  kill (SIGTERM, pid);
  //return 0;
}

/*-------------------------------------------------------------------------*
 *  Get Com Port Name
 *-------------------------------------------------------------------------*/
get_com_port()
{
  FILE *ttys;
  char  line[80];
  char *p;
  char *strtok();
  char *ret;
  
  if( (ttys = fopen( "/etc/ttys", "r" )) == 0 ) return 0;

  while( ret = fgets( line, sizeof(line), ttys) )
  {
    if( !strcmp( strtok( line, ":" ), "com" ) ) break;
  }
  fclose( ttys );
  if (!ret) return 0;
  
  strtok( 0, ":" );                       /* Skip past 2nd field             */
  p = strtok( 0, ":" );                   /* 3rd field is device name        */

  strcpy(comdev, p);                      /* save comm port name             */
}
/*-------------------------------------------------------------------------*
 *  Open Output Communication Port
 *-------------------------------------------------------------------------*/
com_open()
{
  if ( (com_fd = open( comdev, O_RDWR )) <= 0 ) 
  {
    krash("com_open", comdev, 1);
  }

#ifdef DEBUG
  fprintf(stderr, "Comm Port is %s\n", comdev);
#endif

  setup_tty_structs();
  
  ioctl(com_fd, TCFLSH, 0);               /* flush input                     */
  ioctl(com_fd, TCFLSH, 1);               /* flush output                    */

  ioctl(com_fd, TCSETAW, &raw_tty);       /* set to raw mode                 */

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Setup Raw and Sane TTY
 *-------------------------------------------------------------------------*/
setup_tty_structs()
{
  ioctl(com_fd, TCGETA, &raw_tty);
  ioctl(com_fd, TCGETA, &sane_tty);

  raw_tty.c_cc[VINTR]  = 0;
  raw_tty.c_cc[VQUIT]  = 0;
  raw_tty.c_cc[VERASE] = 0;
  raw_tty.c_cc[VKILL]  = 0;
  raw_tty.c_cc[VMIN]   = 1;
  raw_tty.c_cc[VTIME]  = 1;
  raw_tty.c_cc[VEOL2]  = 0;
  //raw_tty.c_cc[VSWTCH] = 0;
  raw_tty.c_iflag      = IGNBRK+IGNPAR+ISTRIP+IXON+IXANY;
  raw_tty.c_oflag      = 0;
  raw_tty.c_lflag      = 0;
  raw_tty.c_cflag     &= ~HUPCL;

  return;
}

/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave(x)
register long x;
{
#ifndef NOBYE
  if (have_server) say_goodbye();
#endif
  sp->sp_to_mode = 0x20;

  ss_close();
  sd_close();
  
  if (com_fd) 
  {
    ioctl(com_fd, TCSETAW, &sane_tty);      /* set to sane mode           */
    close(com_fd);
  }
  exit(x);
}

/* end of com_kermit_out.c */
