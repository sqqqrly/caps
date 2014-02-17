/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Transaction Communications Output.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/10/94   | tjt Added to mfc from custom.
 *  07/12/95   | tjt Add UNIX set to raw mode.
 *  07/14/95   | tjt Add optional file name parameter.
 *-------------------------------------------------------------------------*/
static char comsend_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <termio.h>
#include "file_names.h"

extern leave();

FILE *com_fd;
FILE *tmp_fd;

struct termio raw_tty, sane_tty;

#define EOT 0x04

main(argc, argv)
long argc;
char **argv;
{
  register char c;

  putenv("_=comsend");
  chdir(getenv("HOME"));

  com_open();
  if (com_fd == 0) exit(1);

  if (argc > 1) tmp_fd = fopen(argv[1], "r");
  else tmp_fd = fopen(zt_name, "r");
  if (tmp_fd == 0) return 2;

  while ( (c = getc(tmp_fd)) > 0) putc(c, com_fd);
  
  putc(0x04, com_fd);
  fflush(com_fd);
  
  ioctl(fileno(com_fd), TCSETAW, &sane_tty); /* set to sane mode             */

  fclose(tmp_fd);
  fclose(com_fd);
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Open Output Communication Port
 *-------------------------------------------------------------------------*/
com_open()
{
  FILE *ttys;
  char  line[80];
  char *com_ttydev;
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
  com_ttydev = strtok( 0, ":" );          /* 3rd field is device name        */

  if( (com_fd = fopen( com_ttydev, "w" )) == 0 ) return 0;

  setup_tty_structs();
  
  ioctl(fileno(com_fd), TCFLSH, 0);       /* flush input                     */
  ioctl(fileno(com_fd), TCFLSH, 1);       /* flush output                    */

  ioctl(fileno(com_fd), TCSETAW, &raw_tty);/* set to raw mode                */

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Setup Raw and Sane TTY
 *-------------------------------------------------------------------------*/
setup_tty_structs()
{
  ioctl(fileno(com_fd), TCGETA, &raw_tty);
  ioctl(fileno(com_fd), TCGETA, &sane_tty);

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
leave()
{
  if (com_fd) fclose(com_fd);
  if (tmp_fd) fclose(tmp_fd);
  exit(3);
}

/* end of comsend.c */
