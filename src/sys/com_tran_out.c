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
 *  10/10/94   |  tjt Added to mfc from custom.
 *  12/16/94   |  tjt Add decrement transaction count.
 *  01/26/95   |  tjt Add Close/open db on markplace/restoreplace.
 *  07/12/95   |  tjt Add UNIX set to raw mode.
 *  07/21/95   |  tjt Revise Bard calls.
 *  04/19/96   |  tjt Add sp_to_xmit count.
 *  08/23/96   |  tjt Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char com_tran_out_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <termio.h>
#include <signal.h>
#include "message_types.h"
#include "global_types.h"
#include "xt.h"
#include "ss.h"

#include "Bard.h"

#define DELAY 15
#define CR    0x0d
#define EOT   0x04

extern leave();

FILE *com_fd;

struct trans_item xt;

struct termio raw_tty, sane_tty;

unsigned char list[] = {ShutdownRequest};

char terminator[] = {0x0a, 0x04};

main()
{
  putenv("_=com_tran_out");
  chdir(getenv("HOME"));

  database_open();

  transaction_open(AUTOLOCK);
  transaction_setkey(1);

  ss_open();

  message_open();
  message_select(list, 1);
  message_signal(SIGUSR1, leave);
  signal(SIGTERM, leave);
  
  com_open();
  if (com_fd == 0)
  {
    krash("main", "open comm port", 1);
  }
  while (1)
  {
    begin_work();
    while (!transaction_next(&xt, AUTOLOCK))
    {
      procqueue();                        /* process a transaction           */

      transaction_delete();               /* new delete the transaction      */

      commit_work();
      begin_work();
      
      if (sp->sp_to_count > sp->sp_to_xmit)
      {
        sp->sp_to_xmit += 1;              /* reduce transaction count        */
      }
      if (sp->sp_oi_mode != 0x20) break;  /* order input has started         */
    }
    commit_work();
    sp->sp_to_xmit = sp->sp_to_count;     /* insure are equal                */
    
    transaction_setkey(1);                /* reset to top - on empty         */
    sleep(DELAY);                         /* sleep and try for more          */
  }
}
/*-------------------------------------------------------------------------*
 *  Process a transaction item
 *-------------------------------------------------------------------------*/
procqueue()
{
  format_xt();
  putc(CR, com_fd);
  putc(EOT, com_fd);
  fflush(com_fd);
  return;
}
/*-------------------------------------------------------------------------*
 *  Format a transaction record
 *-------------------------------------------------------------------------*/
format_xt()
{
  fwrite(&xt.xt_code, 1, 1, com_fd);      /* output code                     */

  if (rf->rf_con > 0)                     /* output customer no              */
  {
    fwrite(xt.xt_con, rf->rf_con, 1, com_fd);
  }
  fwrite(&xt.xt_on[OrderLength - rf->rf_on], rf->rf_on, 1, com_fd); 

  if (rf->rf_pl > 0)                      /* output pickline                 */
  {
    fwrite(&xt.xt_pl[2 - rf->rf_pl], rf->rf_pl, 1, com_fd);
  }
  if (xt.xt_code == 'C') return 0;        /* order complete                  */
  if (xt.xt_code == 'U') return 0;        /* order underway                  */
  if (xt.xt_code == 'X') return 0;        /* order canceled                  */
  if (xt.xt_code == 'R') return 0;        /* order repicked                  */
  
  if (rf->rf_sku > 0)                     /* output sku                      */
  {
    fwrite(xt.xt_sku_mod1, rf->rf_sku, 1, com_fd);
  }
  else                                    /* output pm                       */
  {
    fwrite(&xt.xt_sku_mod1[15 - rf->rf_mod], rf->rf_mod, 1, com_fd);
  }
  fwrite(&xt.xt_quan1[4 - rf->rf_quan], rf->rf_quan, 1, com_fd);
  fwrite(&xt.xt_quan2[4 - rf->rf_quan], rf->rf_quan, 1, com_fd);

  fwrite(terminator, 2, 1, com_fd);
  fflush(com_fd);

  return 0;
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

  ioctl(fileno(com_fd), TCFLSH, 0);        /* flush input                    */
  ioctl(fileno(com_fd), TCFLSH, 1);        /* flush output                   */

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
  ss_close();
  message_close();
  transaction_close();
  if (com_fd) 
  {
    ioctl(fileno(com_fd), TCSETAW, &sane_tty);
    fclose(com_fd);
  }
  database_close();
  exit(0);
}

/* end of com_tran_out.c */
