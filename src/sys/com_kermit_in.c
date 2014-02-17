/* #define DEBUG    */
/* #define BYTESIZE */
/*-------------------------------------------------------------------------*
 *  Custom Code:    SERVER - Kermit without kermon.
 *                  NOBYE  - Do not signoff server.
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order and SKU Input from Kermit/kermon.
 *
 *  Note:           With SERVER option kermit placed in server mode 
 *                  manually or by script file.  With NOBYE option the
 *                  server is not closed.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/12/95   |  tjt  Rewrite of com_menu_spc.
 *  08/15/95   |  tjt  Checkin PC directory.
 *-------------------------------------------------------------------------*/
static char com_kermit_in_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <errno.h>
#include <termio.h>
#include <fcntl.h>
#include <signal.h>
#include "eh_nos.h"
#include "iodefs.h"
#include "ss.h"
#include "sd.h"
#include "file_names.h"
#include "com_kermit_in.t"

#define MAX      200                         /* max number of files          */
#define LEN      16                          /* file name length             */
#define TIMEOUT  25                        /* max file xfer is 30 minutes  */
#define TIMEDIR  25                         /* max dir in 2 minutes         */
long    timeout = 0;

extern leave();
extern catcher();
extern char *memchr();

long com_fd = 0;
char comdev[32];                             /* communication port           */

struct termio raw_tty, sane_tty;             /* port sttributes              */

short ONE = 1;
short FNO = 12;

struct fld_parms fld1 = {11, 45, 25, 1, &ONE, "Select Function", 'a'};
struct fld_parms fld2 = {12, 45, 25, 1, &FNO, "Enter File Name", 'a'};

char incode[2] = {0};
long pid, status;

char names[MAX + 1][LEN];                   /* PC File names                 */
long max = 0;                               /* number of PC files            */

main(argc, argv)
long argc;
char **argv;
{
  register unsigned char t;
  char cbuf[80];
  
  putenv("_=com_kermit_in");
  chdir(getenv("HOME"));

  setpgrp();

  ss_open();
  sd_open(leave);
  sd_tab(7, "7 FILE");
  
  fix(com_kermit_in);
  
  sd_screen_off();                         /* show com menu screen           */
  sd_clear_screen();
  sd_text(com_kermit_in);
  sd_screen_on();

  get_com_port();                          /* get com port name              */

#ifndef SERVER
  eh_post(LOCAL_MSG, "Connecting to PC KERMON");

  com_open();
  if (com_fd == 0) leave(1);                 /* open has failed              */

  sprintf(cbuf, "%cLOAD KERMIT ORDERS%c", 0x02, 0x03);
  write(com_fd, cbuf, strlen(cbuf));
  sleep(5);

  ioctl(com_fd, TCSETAW, &sane_tty);         /* set to sane mode             */
#endif
  eh_post(LOCAL_MSG, "Getting PC Directory");

  get_directory_info();                     /* ask for remote directory      */

  sd_prompt(&fld1, 0);                      /* prompt for action code        */

  while (1)
  {
    t = sd_input(&fld1, 0, 0, incode, 0);   /* get code                      */
    if (t == EXIT) break;
    if (*incode == '3') break;
    if (*incode != '1' && *incode != '2')
    {
      eh_post(ERR_CODE, incode);
      continue;
    }
    if (max > 0) get_file_name();
    else eh_post(LOCAL_MSG, "Have received no file names");
  }
  leave(0);
}
/*-------------------------------------------------------------------------*
 *  Get File Name  - Select File Name From List To Process
 *-------------------------------------------------------------------------*/
get_file_name()
{
  register unsigned char t;
  register long k, m;
  char nbuf[16], ubuf[16], lbuf[16];
  
#ifdef DEBUG
  fprintf(stderr, "get_file_name()\n");
#endif

  sd_prompt(&fld2, 0);                      /* prompt for file name          */
  m = 0;
  
  while (1)
  {
    eh_post(LOCAL_MSG, "Use F7 To Cycle File Names");

    memcpy(nbuf, names[m], LEN);            /* copy file name to buffer      */
    strip_space(nbuf, LEN);                  

    t = sd_input(&fld2, 0, 0, nbuf, 0);
    if (t == EXIT) leave(0);
    if (t == UP_CURSOR) return 0;

    if (t == F_KEY_7)                       /* cycle to next file name       */
    {
      m++; 
      if (m >= max) m = 0;
      
      k = (m > MAX - 40) ? (MAX - 40) : m;

      sd_cursor(0, 14, 2);
      sd_text_2(names[k], 639);

      continue;
    }
    if (t != RETURN) continue;

    for (k = 0; k < LEN; k++)               /* upper case & lower case names */
    {
      ubuf[k] = toupper(nbuf[k]);
      lbuf[k] = tolower(nbuf[k]);
    }
    space_fill(ubuf, LEN);                  /* table is space filled         */

    for (k = 0; k < max; k++)
    {
      if (memcmp(names[k], ubuf, 12) == 0)
      {
        strip_space(ubuf, LEN);
        if (get_file_data(ubuf, lbuf)) return 1;/* transfer the file         */

        if (*incode == '1') process_order(lbuf);
        else process_sku(lbuf);

        return 0;
      }
    }
    eh_post(LOCAL_MSG, "File Not On PC");
  }
}
/*-------------------------------------------------------------------------*
 *  Process Order File and Save to otext/orders.save
 *-------------------------------------------------------------------------*/
process_order(name)
register char *name;
{
  char command[80];
  
  eh_post(LOCAL_MSG, "Processing Orders");

  sprintf(command, "bin/order_input <otext/%s", name);
  system(command);

  sprintf(command, "mv otext/%s otext/orders.save 1>/dev/null 2>&1", name);
  system(command);
  
  eh_post(ERR_CONFIRM, "Order Input");

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process SKU File - Must Be 92 Byte Add, Change, Delete Format !!!
 *-------------------------------------------------------------------------*/
process_sku(name)
register char *name;
{
  char command[128];
  
  sprintf(command, "mv %s com/sku.save; cat com/sku.save %s >%s", 
   sku_batch_name, name, sku_batch_name);
  system(command);
  
  unlink("com/sku.save");

  eh_post(ERR_CONFIRM, "SKU File Transfer");
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get File From PC To /mfc/com Directory.
 *
 *  #
 *  # Get File From PC
 *  #
 *  rm /usr/spool/uucp/LCK*
 *  cd com
 *  echo "set line /dev/tty?
 *  set baud 9600
 *  get ? 
 *  quit" | kermit
 *
 *-------------------------------------------------------------------------*/
get_file_data(uname, lname)
register char *uname, *lname;
{
  char work[64], mess[64];
  FILE *fd;
  
#ifdef DEBUG
  fprintf(stderr, "get_file_data(%s, %s)\n", uname, lname);
#endif

  unlink("com/kermit_get");
  sprintf(work, "otext/%s", lname);        /* remove file by this name       */
  unlink(work);
  
  fd = fopen("com/kermit_get", "w");
  if (fd == 0) krash("get_file", "open com/kermit_get", 1);
  
  fprintf(fd, "#\n# Get File From PC\n#\n");
  fprintf(fd, "rm /usr/spool/uucp/LCK*\n");
  fprintf(fd, "cd otext\n");
  fprintf(fd, "echo \"set line %s\nset baud 9600\n", comdev);
#ifdef BYTESIZE
  fprintf(fd, "echo set terminal bytesize 8\n", comdev);
#endif
  fprintf(fd, "get %s\nquit\" | kermit\n", uname);

  fclose(fd);

  eh_post(LOCAL_MSG, "Transfering File From PC");
  
  if (fork() == 0)
  {
    setpgrp();

    signal(SIGALRM, catcher);
    alarm(TIMEOUT);   
  
    if ((pid = fork()) == 0)
    {
      system ("chmod 777 com/kermit_get 1>com/kermit_get_err 2>&1");
      sleep(1);
      system ("com/kermit_get 1>>com/kermit_get_err 2>&1");
      exit(0);
    }
    pid = wait(&status);
  
    alarm(0);

    if (pid < 0 || status > 0)
    {
      eh_post(LOCAL_MSG, "PC Kermit Is Not Responding");
      if (pid == -1 && errno == EINTR)
      {
        kill ( 0, SIGKILL );
        pid = wait(&status);
      }
    }
    exit(0);
  }
  pid = wait(&status);

  if (pid < 0 || status) 
  {
    timeout = 1;
    return 1;
  }
  fd = fopen(work, "r");
  if (fd == 0)
  {
    sprintf(mess, "File %s not transfered", lname);
    eh_post(LOCAL_MSG, mess);
    return 1;
  }
  fclose(fd);
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
  
  unlink("com/kermit_dir");                /* must delete old file for chmod */
  unlink("com/kermit_dir_file");

  fd = fopen("com/kermit_dir", "w");
  if (fd == 0) krash("get_directory_info", "open com/kermit_dir", 1);
  
  fprintf(fd, "#\n# Fetch Dir From PC\n#\n");
  fprintf(fd, "rm /usr/spool/uucp/LCK* 1>/dev/null 2>&1\n");
  fprintf(fd, "cd com\n");
  fprintf(fd, "echo \"set line %s\nset baud 9600\n", comdev);
#ifdef BYTESIZE
  fprintf(fd, "echo set terminal bytesize 8\n", comdev);
#endif
  fprintf(fd, "remote dir\nquit\" | kermit 1>kermit_dir_file 2>/dev/null\n");
  
  fclose(fd);

  if (fork() == 0)
  {
    setpgrp();

    signal(SIGALRM, catcher);
    alarm(TIMEDIR);
  
    if ((pid = fork()) == 0)
    {
      system ("chmod 777 com/kermit_dir 1>com/kermit_dir_err 2>&1");
      sleep(1);
      system ("com/kermit_dir 1>>com/kermit_dir_err 2>&1");    /* get trash? */
      unlink("com/kermit_dir_file");
      system ("com/kermit_dir 1>>com/kermit_dir_err 2>&1");    /* get again  */
      exit(0);
    }
    pid = wait(&status);
  
    alarm(0);

#ifdef DEBUG
  fprintf(stderr, "inner pid=%d status=%d errno=%d\n", pid, status, errno);
#endif

    if (pid < 0 || status > 0)
    {
      eh_post(LOCAL_MSG, "PC Kermit Is Not Responding");
      if (pid == -1 && errno == EINTR)
      {
        kill ( 0, SIGKILL );
        pid = wait(&status);
      }
    }
    exit(0);
  }
  pid = wait(&status);

#ifdef DEBUG
  fprintf(stderr, "pid=%d status=%d errno=%d\n", pid, status, errno);
#endif

  if (pid < 0 || status) 
  {
    timeout = 1;
    return 1;                              /* failed to get dir              */
  }
  get_file_names();

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get File Names - Table is space filled !!!
 *-------------------------------------------------------------------------*/
get_file_names()
{
  char nbuf[128], *p;
  long k;
  FILE *fd;
  
#ifdef DEBUG
  fprintf(stderr, "get_file_names()\n");
#endif

  memset(names, 0x20, MAX * LEN);
  
  fd = fopen("com/kermit_dir_file", "r");
  if (fd == 0) 
  {
    eh_post(LOCAL_MSG, "No directory file received");
    return 1;
  }
  while (fgets(nbuf, 127, fd))
  {
    k = strlen(nbuf);
    if (k < 35) continue;
    
    if (nbuf[0] == '$')  continue;
    if (nbuf[0] == 0x20) continue;           /* F081595                      */
    if (nbuf[0] == '.')  continue;
    if (nbuf[0] == '\r')  continue;
    if (nbuf[0] == '\n')  continue;
    
    p = memchr(nbuf, '-', k);
    if (!p) continue;
    p++;

    k = strlen(p);
    p = memchr(p, '-', k);
    if (!p) continue;
    p++;

    k = strlen(p);
    p = memchr(p, ':', k);
    if (!p) continue;
    
    nbuf[8] = nbuf[12] = 0;                  /* F081595                      */
    strip_space(nbuf, 9);
    strip_space(nbuf + 9, 4);
    strcpy(names[max], nbuf);
    if (*(nbuf + 9))  
    {
      strcat(names[max], ".");
      strcat(names[max], nbuf + 9);
    }
    space_fill(names[max], LEN);
    max++;
    if (max >= MAX) break;
  }
  fclose(fd);

  memset(names[MAX], 0, LEN);
    
  sd_cursor(0, 14, 2);                     /* display first page of table    */
  sd_text_2(names[0], 639);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Goodbye To PC
 *-------------------------------------------------------------------------*/
say_goodbye()
{
  char command[80];
  FILE *fd;

  eh_post(LOCAL_MSG, "Stopping PC Kermit");

  if (timeout) return 0;

  unlink("com/kermit_bye");

  fd = fopen("com/kermit_bye", "w");
  if (fd == 0) krash("get_directory_info", "open com/kermit_bye", 1);
  
  fprintf(fd, "#\n# Stop Server On PC\n#\n");
  fprintf(fd, "rm /usr/spool/uucp/LCK*\n");
  fprintf(fd, "echo \"set line %s\nset baud 9600\n", comdev);
#ifdef BYTESIZE
  fprintf(fd, "echo set terminal bytesize 8\n", comdev);
#endif
  fprintf(fd, "bye\nquit\" | kermit\n");
  
  fclose(fd);

  if (fork() == 0)
  {
    setpgrp();

    signal(SIGALRM, catcher);
    alarm(15);
  
    if ((pid = fork()) == 0)
    {
      system ("chmod 777 com/kermit_bye 1>com/kermit_bye_err 2>&1");
      sleep(1);
      system ("com/kermit_bye 1>>com/kermit_bye_err 2>&1");
      exit(0);
    }
    pid = wait(&status);
    
    alarm(0);
    
    if (pid < 0 || status > 0)
    {
      eh_post(LOCAL_MSG, "PC Kermit Is Not Responding");
      if (pid == -1 && errno == EINTR)
      {
        kill ( 0, SIGKILL );
        pid = wait(&status);
      }
    }
    exit(0);
  }
  pid = wait(&status);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Alarm Catcher - Kills Last Active Task
 *-------------------------------------------------------------------------*/
catcher()
{
  char text[80];

#ifdef DEBUG
  fprintf(stderr, "Got Timeout  pid=%d\n", pid);
  fflush(stderr);
#endif

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Com Port Name
 *-------------------------------------------------------------------------*/
get_com_port()
{
  FILE *ttys;
  char line[128];
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
  say_goodbye();
#endif

  sd_tab(7, "7  ");
 
  sp->sp_oi_mode = 0x20;

  ss_close();
  sd_close();

  if (com_fd) 
  {
    ioctl(com_fd, TCSETAW, &sane_tty);      /* set to sane mode           */
    close(com_fd);
  }
  execlp("bin/operm", "bin/operm", 0);
  krash("leave", "load operm", 1);
}

/* end of com_kermit_in.c */
