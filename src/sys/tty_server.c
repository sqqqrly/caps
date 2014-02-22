/* #define DEBUG2 */
#define APU
/*-------------------------------------------------------------------------*
 *  Custom Version: APU  - call telent for Hanes
 *                  AVON - calls view for Avon
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Message driven tty (keyboard & screen) server.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/08/93   |  tjt  Original implementation.
 *  06/12/94   |  tjt  Log on to APU.
 *  01/06/95   |  tjt  Only open once.
 *  01/23/95   |  tjt  Add ^R for resend.
 *  01/23/95   |  tjt  Reset keytabs to default on close.
 *  06/17/95   |  tjt  Detect wyse50 or not for delays.
 *  03/12/96   |  tjt  Catch signals to exit.
 *  04/25/97   |  tjt  Allow 0x80 - 0xaf bytes.
 *-------------------------------------------------------------------------*/
static char tty_driver_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include <termio.h>
#include <errno.h>
#include "file_names.h"
#include "tty_data.h"
#include "eh.h"
#include "eh_nos.h"
#include "message.h"
#include "message_types.h"
#include "kernel_types.h"

FILE *eh_mess_fd;                         /* error message text file         */
FILE *eh_err_fd;                          /* error log file                  */
long error_position = -1;                 /* position in error log file      */

FILE *rd = 0;                             /* record keystokes                */
FILE *pd = 0;                             /* replay keystrokes               */

struct termio raw_tty, sane_tty;          /* tty termio structures           */

extern screen_server();                   /* screen events processor         */
extern leave();                           /* exit routine                    */
extern catcher();
extern char *getenv();

char tty_name[16];                        /* name of this terminal           */
char printer[32];                         /* printer change                  */
char operator[32];                        /* operator change                 */
char lpr[40];                             /* print procedure                 */

/*--------------------------------------------------------------------------*
 *    Message                    Message Contents
 * ------------------     ---------------------------------
 * ClientMessageEvent     error number + text parm
 * SystemMessageEvent     message up to 80 bytes
 * TTYServerOpenRequest   tty name (usually from TTY=)
 * TTYServerCloseEvent    none
 * TTYServerWhoRequest    none
 * TTYServerWhoEvent      tty name (from TTY=)
 * ScreenClearEvent       row + col + clear (0=all 1=eol 2=eos 3=off 4=on)
 * ScreenDisplayEvent     row + col + text
 * ChangePrinterEvent     printer name (used to set PRINTER=)
 * ChangeOperatorEvent    operator name (used to set OPERATOR=)
 * InputFieldRequest      row + col + type (a,n,b) + echo + initial text
 * InputFieldEvent        input text + terminal symbol
 * KeystrokeRequest       row + col
 * KeystrokeEvent         encoded byte
 *-------------------------------------------------------------------------*/

unsigned char list[] = {

  ClientMessageEvent,
  SystemMessageEvent,
  TTYServerOpenRequest,
  TTYServerCloseEvent,
  TTYServerWhoRequest,
  ScreenClearEvent,
  ScreenDisplayEvent,
  ChangePrinterEvent,
  ChangeOperatorEvent,
  KeyTabRequest,
  InputFieldRequest,
  KeystrokeRequest,
  KeystrokeEvent};
        
long serving = 0;                         /* last logon task                 */

long WIDE = 80;                           /* default screen width            */
long DELAY = 50;                          /* nulls on attributes             */

unsigned char crt[24][132];               /* image of screen                 */

long cursor = 0;                          /* cursor on/off                   */
long focus  = 0;                          /* focus on/off                    */
long error  = 0;                          /* error line display              */
long fkeys  = 0;                          /* enable/disable function keys    */
long f_row  = 0;                          /* focus row                       */
long f_col  = 0;                          /* focus col                       */
long i_row  = 0;                          /* input row                       */
long i_col  = 0;                          /* input col                       */
long i_len  = 0;                          /* input length                    */
long i_echo = 0;                          /* echo data                       */
unsigned char i_type;                     /* n=numeric,a=alpha,b=byte,x=raw  */
unsigned char i_text[80];                 /* input field value               */

unsigned char last_line[81];              /* last message line display       */

#if defined(APU) || defined(AVON)
long on_apu = 0;
Putchar(x) register char x; {if (!on_apu) putchar(x);}
Fputs(x, y) register char *x; register long y;{if (!on_apu) fputs(x,y);}
#else
#define Putchar(x)    putchar(x)
#define Fputs(x, y)   fputs((x), (y))
#endif

main(argc, argv)
long argc;
char **argv;
{
  register char *p;

  putenv("_=tty_server");                 /* name of this program            */
  chdir(getenv("HOME"));
  
  setup_tty_structs();                    /* setup raw and saved modes       */
  
  signal(SIGHUP,  catcher);               /* catch death of parent           */
  signal(SIGQUIT, catcher);               /* catch suspend of parent         */
  signal(SIGTRAP, catcher);               /* catch suspend of parent         */
  signal(SIGTERM, catcher);               /* catch terminate of parent       */
  
  p = getenv("TTY");                      /* get who we are                  */
  if (!p)                                 /* cannot function without TTY     */
  {
    krash("tty_server", "TTY Not Found", 0);
    leave(-1);
  }
  strncpy(tty_name, p, 15);               /* save name of tty device         */
 
  p = getenv("OPERATOR");                 /* get operator name               */
  if (p) sprintf(operator, "OPERATOR=%s", p);

  p = getenv("LPR");                      /* get printer procedure           */
  if (p) strncpy(lpr, p, 40);

  p = getenv("TERM");                     /* get terminal                    */
  if (p)
  {
    if (strcmp(p, "wyse50") == 0) DELAY = 0;   /* no delays needed           */
#ifdef DEBUG2
  fprintf(stderr, "TERM=%s DELAY=%d\n", p, DELAY);
#endif
  }

  Fputs(screen_off, stdout);              /* turn screen off                 */
  screen_clear(0, 0, 0);                  /* clear screen                    */

#ifdef APU
  memcpy(&keytabs[1][5], "  APU", 5);     /* override HELP key               */
#endif

#ifdef AVON
  memcpy(&keytabs[1][5], " VIEW", 5);     /* override HELP key               */
#endif

  error_open();                           /* open error files                */

  if (message_open() < 0) leave(-2);      /* open message queue              */
  message_select(list, sizeof(list));     /* select tty messages             */
  message_signal(SIGUSR1, screen_server); /* arm message catcher             */
        
  kill((short)getppid(), SIGUSR1);        /* tell parent we are ready        */

  p = getenv("RECORD");
  if (p) rd = fopen(p, "w");              /* record keystrokes               */
    
  p = getenv("REPLAY");                   /* replay keystrokes               */
  if (p) 
  {
    pd = fopen(p, "r");
    if (pd) fgetc(pd);
  }
  while (1)
  {
    pause();
    if (serving) keyboard_server();       /* process keyboard events         */
  }
  leave(-3);                              /* close and exit                  */
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Open or Reopen Terminal
 *-------------------------------------------------------------------------*/
tty_open()
{
  register long n;
  
  ioctl(fileno(stdin),  TCFLSH, 0);       /* flush input                     */
  ioctl(fileno(stdout), TCFLSH, 1);       /* flush output                    */
  
  ioctl(fileno(stdin), TCSETAW, &raw_tty);/* set to raw mode                 */

  Fputs(screen_off, stdout);              /* screen off                      */
  
  if (WIDE != 80)
  {
    Fputs(screen_80, stdout);             /* set to 80 columns               */
    fflush(stdout);
    WIDE = 80;
    if (DELAY) sleep(1);                  /* give time to complete           */
  }
  Fputs(screen_off, stdout);              /* screen off                      */
  Fputs(clear_screen, stdout);            /* clear screen                    */
  Fputs(cursor_off, stdout);              /* turn off cursor                 */
  Fputs(scroll_off, stdout);              /* turn off scrolling              */
  Fputs(revtabs_on, stdout);              /* reverse dim key tabs            */
  Fputs(host_line_off, stdout);           /* clear message line              */
  
  for (n = 0; n < 8; n++)
  {
    Fputs(keytabs[n], stdout);            /* key tab name                    */
    Fputs(keyvalue[n], stdout);           /* key tab sequence                */
  }
  fflush(stdout);
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Setup Raw and Sane TTY
 *-------------------------------------------------------------------------*/
setup_tty_structs()
{
  ioctl(fileno(stdin), TCGETA, &raw_tty);
  ioctl(fileno(stdin), TCGETA, &sane_tty);

  raw_tty.c_cc[VINTR]  = 0;
  raw_tty.c_cc[VQUIT]  = 0;
  raw_tty.c_cc[VERASE] = 0;
  raw_tty.c_cc[VKILL]  = 0;
  raw_tty.c_cc[VMIN]   = 1;
  raw_tty.c_cc[VTIME]  = 1;
  raw_tty.c_cc[VEOL2]  = 0;
/*
  raw_tty.c_cc[VSWTCH] = 0;
*/
  raw_tty.c_iflag      = IGNBRK+IGNPAR+ISTRIP+IXON+IXANY;
  raw_tty.c_oflag      = 0;
  raw_tty.c_lflag      = 0;
  raw_tty.c_cflag     &= ~HUPCL;

  return;
}

/*-------------------------------------------------------------------------*
 *  Close TTY
 *-------------------------------------------------------------------------*/
tty_close()
{
  register long n;

  for (n = 0; n < 8; n++)
  {
#ifndef AVON
    Fputs(reset_keyvalue[n], stdout);     /* default wyse 50 function keys   */
#endif
    Fputs(keyclear[n], stdout);           /* clear tab name                  */
  }
  if (WIDE != 80)
  {
    Fputs(screen_80, stdout);             /* set to 80 columns               */
    fflush(stdout);
    WIDE = 80;
    if (DELAY) sleep(1);
  }
  Fputs(cursor_on, stdout);               /* turn on cursor                  */
  Fputs(scroll_on, stdout);               /* turn on scrolling               */
  Fputs(revtabs_off, stdout);             /* normal key tabs                 */
  Fputs(host_line_off, stdout);           /* clear message line              */
  fflush(stdout);                         /* flush output                    */
  
  ioctl(fileno(stdin), TCSETAW, &sane_tty);/* reset to original mode         */

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Catch Signals
 *-------------------------------------------------------------------------*/
catcher()
{
  leave(1);
}
/*--------------------------------------------------------------------------*
 *  Shutdown Task Gracefully
 *--------------------------------------------------------------------------*/
leave(flag)
register long flag;                       /* flag = 0 is clear screen        */
{
  int parent;
  
  parent = getppid();                     /* get parent id                   */
  
  signal(SIGHUP,  SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGTRAP, SIG_IGN);
  
  message_close();                        /* close message link              */

  if (eh_err_fd)  fclose(eh_err_fd);
  if (eh_mess_fd) fclose(eh_mess_fd);
  if (pd)         fclose(pd);
  if (rd)         fclose(rd);

  tty_close();
  
#ifdef DEBUG2
  fprintf(stderr, "tty_server termination Signal=%d parent=%d\n\r", 
    flag, parent);
  system("stty 1>&2");
#endif

  if (flag && parent > 0)
  {
    kill(parent, SIGTERM);                /* kill parent                     */
    sleep(1);                             /* allow graceful termination      */
    kill(parent, SIGKILL);                /* kill anything left              */
  }
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Keyboard Server - Reads and encodes bytes - send message to self.
 *-------------------------------------------------------------------------*/
keyboard_server()
{
  register long ret;
  TMessageItem buf;
  unsigned char c, d;                     /* input byte                      */

  while (1)                               /* read keyboard forever           */
  {
    if (!serving) return 0;               /* loop until open again           */
    
    c = getchar();                        /* get one byte                    */

    if (c == 0xff && errno == EINTR) continue;   /* interrupted io           */

    if (c > LAST_CHAR) c = 0;             /* out of range - make null        */
    else if (c < 0x7f) c = sd_in_byte[c]; /* translate byte to code          */

    if (c == ESCAPE)                      /* check ecsape pair               */
    {
      c = getchar();
      if (c == '~')                       /* ESCAPE ~ is kill task           */
      {
        if (serving)
        {
          buf.m_sender      = serving;
          buf.m_destination = KernelDestination;
          buf.m_type        = KernelLogOut;
          buf.m_length      = 0;
          Message_Put(&buf);
          serving = 0;
        }
        leave(0);                         /* exit now                        */
      }
      if (c <= '0' && c >= '9')
      {
        c -= 0x30;
        d = getchar();
        if (d >= '0' && d <= '9') c = (10 * c) + (d - 0x30);
        d = getchar();
        if (d >= '0' && d <= '9') c = (10 * c) + (d - 0x30);
        if (c < 0x80 || c > LAST_CHAR) c = 0;
      }
      else
      {
        if (c > 0x7f) c = 0;              /* out of range - make null        */
        else c = sd_escape_byte[c];       /* translate byte to code          */
      }
    }
#ifdef APU
    if (c == F_KEY_2 && fkeys) 
    {
      Fputs(screen_on, stdout);
      tty_close();
      
      on_apu = 1;
      system("/u/mfc/bin/ApplMenu");
/*
      system("echo; echo Connecting to APU; echo; /usr/ucb/telent hanes");
*/
      on_apu = 0;

      tty_open();
      Fputs(screen_off, stdout);
      screen_refresh();
      Fputs(screen_on, stdout);
      continue;
    }
#endif

#ifdef AVON
    if (c == F_KEY_2 && fkeys) 
    {
      Fputs(screen_on, stdout);
      tty_close();
      
      on_apu = 1;
      system("view");
      on_apu = 0;
      
      tty_open();
      Fputs(screen_off, stdout);
      screen_refresh();
      Fputs(screen_on, stdout);
      continue;
    }
#endif
    if (!c)                               /* not recognized                  */
    {
      Putchar(BEEP);                      /* beep touble                     */
      fflush(stdout);
      continue;
    }
    message_put(msgtask, KeystrokeEvent, &c, 1);
  }
}
/*-------------------------------------------------------------------------*
 *  Screen display message processor
 *-------------------------------------------------------------------------*/
screen_server(who, type, b, len)
register long who;
register long type;
register unsigned char *b;
register long len;
{
  register long k;

  b[len] = 0;                             /* append null in buffer           */

#ifdef DEBUG
  fprintf(stderr, "screen_server() who=%d type=%d len=%d\n", who, type, len);
  if (len > 0) Bdump(b, len);
#endif

  switch (type)
  {
    case ShutdownRequest:                 /* system is stopping              */
 
		break;                              /* no action required              */
		
    case ShutdownEvent:                   /* terminate tty_server            */
    
      leave(0);                           /* terminate gracefully            */
      
    case TTYServerOpenRequest:            /* respond if TTY matches          */

      screen_open(who, b);
      break;

    case TTYServerCloseEvent:             /* remove any focus by who         */

      screen_close(who);
      break;

    case TTYServerWhoRequest:             /* tell TYY to who                 */

      message_put(who, TTYServerWhoEvent, tty_name, strlen(tty_name));
      break;
      
    case ScreenClearEvent:                /* clear screen, line, etc         */

      screen_clear(*b, *(b+1), *(b+2));
      break;

    case ChangePrinterEvent:              /* save new printer name           */
    
      sprintf(printer, "PRINTER=%s", b);
      putenv(printer);
      break;

    case ChangeOperatorEvent:             /* save new operator name          */
    
      sprintf(operator, "OPERATOR=%s", b);
      putenv(operator);
      break;

    case ScreenDisplayEvent:              /* display a field                 */

      screen_display(*b, *(b+1), b+2, len-2);
      break;

    case KeyTabRequest:
    
      change_keytab(b, len);
      break;

    case InputFieldRequest:               /* get a field                     */

      if (rd) fputc(1, rd);               /* record a terminator             */
      input_field(who, b, len);
      if (pd) get_keystrokes();
      break;

    case KeystrokeRequest:                /* get next single byte            */

      if (rd) fputc(1, rd);               /* record a terminator             */
      input_byte(who, b, len);
      if (pd) get_keystrokes();

      break;

    case KeystrokeEvent:                  /* process a keystroke             */

      if (rd) fputc(*b, rd);              /* record a keystroke              */

      if (*b >= F_KEY_1 && *b <= F_KEY_8)
      {
        fflush(stdout);
      }
      if (focus && i_type == 'b')         /* single byte input               */
      {
        if (i_echo) 
        {
          if (i_echo & 2)
          { 
            if (*b >= SPACE && *b < DELETE) Putchar(*b);
            if (*b > DELETE && *b < LAST_CHAR)
            {
              Putchar(*b);                /* F042597                         */
            }
          }
          cursor_out();
        }
        if (*b == PRINT && fkeys) screen_print();
        message_put(serving, KeystrokeEvent, b, 1);
        focus = 0;
        return 0;
      }
      if (*b == SEND)
      {
        tty_open();                       /* reopen entire screen            */
        screen_refresh();                 /* repaint entire screen           */
        Fputs(screen_on, stdout);
        fflush(stdout);
        break;
      }
      if (i_type != 'x')                  /* x is a special raw field        */
      {
        if (*b == LOG && fkeys)   {error_log(); break;}
        if (*b == PRINT && fkeys) {screen_print(); break;}
      }
      keystroke(*b);
      break;

    case ClientMessageEvent:              /* build an error message          */

      error_message(*b, b+1);
      break;

    case SystemMessageEvent:              /* display a system message        */

      system_message(b, len);
      break;
  }
#ifdef DEBUG
  fprintf(stderr, "exit screen_server()\n");
#endif

  return 0;
}
/*-------------------------------------------------------------------------*
 *  TTY Server Open Request
 *-------------------------------------------------------------------------*/
screen_open(who, name)
register long who;
register char *name;
{
  if (strcmp(tty_name, name) != 0) return 0;/* not for this server           */

  serving = who;

  message_put(who, TTYServerOpenEvent, 0, 0);/* tell user who we are         */

  return 0;
}
/*-------------------------------------------------------------------------*
 *  TTY Server Close request
 *-------------------------------------------------------------------------*/
screen_close(who)
{
  if (focus)                              /* current keyboard task           */
  {
    cursor_out();                         /* turn off cursor                 */
    focus = 0;                            /* remove focus                    */
  }
  serving = 0;
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Screen Clear
 *-------------------------------------------------------------------------*/
screen_clear(r, c, how)
register unsigned char r, c;              /* row and column                  */
register unsigned char how;               /* how to clear screen             */
{
  register long k, len;

  if (cursor) cursor_out();               /* turn off cursor                 */

  switch (how)                            /* all, line, or rest of screen    */
  {
    case 0:                               /* clear entire screen             */

      Fputs(clear_screen, stdout);        /* clear screen                    */
      fflush(stdout);
      memset(crt, 0x20, 24 * 132);        /* clear screen image              */
      for (k = 0; k < 24; k++)
      {
        position(k, WIDE - 1);
        Fputs(sd_attr[NORMAL - ULC], stdout);
      }
      position(0, 0);
      break;
                                                
    case 1:                               /* clear to end of line            */

      if (r > 23 || c >= 132) break; 
      position(r, c);
      memset(&crt[r][c], 0x20, 132 - c);
      Fputs(clear_line, stdout);
      fflush(stdout);
      break;
                
    case 2:                               /* clear to end of screen          */

      if (r > 23 || c >= 132) break;
      position(r, c);
      memset(&crt[r][c], 0x20, (24 - r) * 132 - c);
      Fputs(clear_forward, stdout);       /* clear forward to end of screen  */
      fflush(stdout);
      break;
      
    case 3:                               /* screen off                      */
 
      Fputs(screen_off, stdout);
      fflush(stdout);
      return 0;
    
    case 4:                               /* screen on                       */

      Fputs(screen_on, stdout);
      fflush(stdout);
      break;

    case 5:                               /* star burst                      */
    
      burst_out();
      break;

    case 6:                               
    
      burst_in();
      break;

	 case 7:                               /* 80 column screen                */  
	 
      Fputs(screen_off, stdout);
	   Fputs(screen_80, stdout);
      fflush(stdout);
      if (DELAY) sleep(1);
      WIDE = 80;
      Fputs(screen_off, stdout);
      Fputs(cursor_off, stdout);          /* turn off cursor                 */
      fflush(stdout);
      cursor = 0;
	   break;
	   
	 case 8:                               /* 132 column screen               */  

      Fputs(screen_off, stdout);
      Fputs(screen_132, stdout);
      fflush(stdout);
      if (DELAY) sleep(2);
      WIDE = 132;
      Fputs(screen_off, stdout);
      Fputs(cursor_off, stdout);          /* turn off cursor                 */
      fflush(stdout);
      cursor = 0;
      break;
     
    case 9:                               /* set tty in raw mode             */
    
      tty_open();
      fkeys = 1;                          /* function keys work              */
      break;
    
    case 10:                              /* set tty in cooked mode          */

      tty_close();
      break;

    case 11:                              /* disable function keys           */
    
      fkeys = 0;
      break;
    
    case 12:                              /* enable function keys            */

      fkeys = 1;
      break;

    default:
    
    break;
  }
  if (focus) focus_on();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Special Screen Display
 *-------------------------------------------------------------------------*/
burst_out()
{
  register long j, k;

  Fputs(clear_screen, stdout);
  Fputs(screen_on, stdout);
  fflush(stdout);
  
  for (j = 0; j < 40; j++)
  {
    for (k = 0; k < 24; k++)
    {
      if (crt[k][40 + j] != 0x20) 
      {
        position(k, 40 + j);
        screen_out(&crt[k][40 + j], 1);
      }
      if (crt[k][39 - j] != 0x20) 
      {
        position(k, 39 - j);
        screen_out(&crt[k][39 - j], 1);
      }
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Special Screen Display
 *-------------------------------------------------------------------------*/
burst_in()
{
  register long j, k;

  Fputs(clear_screen, stdout);
  Fputs(screen_on, stdout);
  fflush(stdout);
  
  for (j = 39; j >= 0; j--)
  {
    for (k = 0; k < 24; k++)
    {
      if (crt[k][40 + j] != 0x20) 
      {
        position(k, 40 + j);
        screen_out(&crt[k][40 + j], 1);
      }
      if (crt[k][39 - j] != 0x20) 
      {
        position(k, 39 - j);
        screen_out(&crt[k][39 - j], 1);
      }
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Screen display
 *-------------------------------------------------------------------------*/
screen_display(r, c, p, len)
register unsigned char r, c;              /* row and column                  */
register unsigned char *p;
register long len;
{
  register long k, skip;
  register unsigned char *a, *b;

  if (r > 23 || c >= WIDE) return 0;      /* ignore bad parms                */

  if (len + c > WIDE) len = WIDE - c;     /* limited to one line             
  if (len <= 0) return 0;                 /* ignore bad parms                */

  a = &crt[r][c];                         /* position in screen image        */
  
  if (memcmp(a, p, len) == 0) return 0;   /* entirely the same               */

  if (cursor) cursor_out();               /* turn off cursor                 */

  for (; len > 0; len--, c++, a++, p++)   /* check same as before            */
  {
    if (*a != *p) break;                  /* found left side                 */
  }
  b = a + len - 1;                        /* rightside of screen             */
  a = p + len - 1;                        /* rightside of data               */

  for (; len > 0; len--, a--, b--)
  {
    if (*a != *b) break;                  /* found right side                */
  }
  memcpy(&crt[r][c], p, len);             /* copy into image                 */

  position(r, c);                         /* position for output             */
  screen_out(p, len);                     /* display on screen               */

  if (focus) focus_on();                  /* reset for input if any          */
  fflush(stdout);
 
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Change Or Reset Key Tabs
 *-------------------------------------------------------------------------*/
change_keytab(b, len)
register char *b;
register long len;
{
  register long n;

  if (*b >= 1 && *b <= 8)                  /* set a key tab                  */
  {
    Putchar(0x1b);
    Putchar('z');
    Putchar(*b + 0x2f);
     
    if (len > 1) Fputs(b + 1, stdout);

    Putchar('\r');
  }
  else
  {
    for (n = 0; n < 8; n++)
    {
      Fputs(keytabs[n], stdout);          /* key tab name                  */
      Fputs(keyvalue[n], stdout);         /* key tab sequence              */
    }
  }
  fflush(stdout);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Screen refresh
 *-------------------------------------------------------------------------*/
screen_refresh()
{
  register long r;
        
  if (cursor) cursor_out();               /* turn cursor off                 */

  for (r = 0; r < 24; r++)                /* over all rows                   */
  {
    position(r, 0);                       /* next row                        */
    screen_out(crt[r], WIDE);             /* display one line                */
  }
  if (focus) focus_on();                  /* turn cursor on if input         */

  return 0;
}
/*--------------------------------------------------------------------------*
 *  Output To Screen
 *--------------------------------------------------------------------------*/
screen_out(p, len)
register unsigned char *p;
register long len;
{
  register long k;
  register unsigned char *q;

  for (; len > 0; len--, p++)
  {
    if (*p >= ULC && *p <= REVDIM)
    {
      q = sd_attr[*p - ULC];              /* point to attributes             */
      while (*q) Putchar(*q++);           /* output graphic/video            */
      fflush(stdout);

      if (*p >= NORMAL) for (k = 0; k < DELAY; k++) Putchar(0x00);
    }
    if (*p > DELETE && *p <= LAST_CHAR)   /* F042597                         */
    {
      Putchar(*p);
    }
    else if (*p < DELETE) Putchar(*p);
    
    else Putchar(0x20);
    fflush(stdout);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Print Entire Screen
 *-------------------------------------------------------------------------*/
screen_print()
{
  register long k, j;
  register unsigned char *p;
  FILE *fd;
  char tname[16], date[32], command[80];
  long now;
  
  fprintf(stdout, "%cF* * *   W A I T   * * *\r", 0x1b);
  fflush(stdout);
  
  tmp_name(tname);                        /* get a file name                 */

  fd = fopen(tname, "w");
  if (fd == 0)
  {
    return krash("screen_print", "Can't open temp", 0);
  }
  time(&now);
  strcpy(date, ctime(&now));              /* get current date time           */
  date[24] = 0;
  
  fprintf(fd, "\n\n%s\n\n", date);
  fprintf(fd, "TTY Terminal: %s   Operator: %s\n\n", tty_name, operator+9);
    
  fprintf(fd, "+");
  for (j = 0; j < WIDE; j++) fprintf(fd, "-");
  fprintf(fd, "+\n");

  for (k = 0; k < 24; k++)
  {
    p = crt[k];
    
    fprintf(fd, "+");
    for (j = 0; j < WIDE; j++, p++)
    {
      if (*p >= ULC && *p <= VERT) fprintf(fd, "*");
      else if (*p >= DELETE) fprintf(fd, " ");
      else fprintf(fd, "%c", *p);
    }
    fprintf(fd, "+\n");
  }
  fprintf(fd, "+");
  for (j = 0; j < WIDE; j++) fprintf(fd, "-");
  fprintf(fd, "+\n\n");

  fprintf(fd, "Last Message Line:\n%s\n", last_line);
  fclose(fd);

  sprintf(command, "%s %s", lpr, tname);
  system(command);

  fprintf(stdout, "%cF\r%c", 0x1b, BEEP);
  fflush(stdout);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Setup Field Input   (InputFieldRequest)
 *-------------------------------------------------------------------------*/
input_field(who, b, len)
register long who;
register unsigned char *b;
register long len;
{
  i_len = len - 4;                        /* length of data field            */

  if (focus || i_len < 1)                 /* conflict or error               */
  {
    message_put(who, InputFieldEvent, 0, 0);/* return nothing is conflict    */
    return 0;
  }
  focus   = 1;                            /* mark input is active            */
  cursor  = 0;                            /* mark cursor as off              */
  i_row   = f_row = *b++;                 /* field row                       */
  i_col   = f_col = *b++;                 /* field col                       */
  i_type  = *b++;                         /* field type                      */
  i_echo  = *b++;                         /* echo and fill flag              */
  
  memcpy(i_text, b, i_len + 1);           /* input value + null              */

  show_field();                           /* display if has data             */

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Setup Byte Input   (KeystrokeRequest)
 *-------------------------------------------------------------------------*/
input_byte(who, b, len)
register long who;
register unsigned char *b;
register long len;
{
  if (focus)                              /* conflicting focus               */
  {
    message_put(who, InputFieldEvent, 0, 0);/* return nothing is conflict    */
    return 0;
  }
  focus   = 1;                            /* mark input is active            */
  cursor  = 0;                            /* mark cursor off                 */
  i_row   = f_row = *b++;                 /* field row                       */
  i_col   = f_col = *b++;                 /* field col                       */
  i_type  = 'b';                          /* field type                      */
  i_echo  = *b;                           /* echo and fill flag              */
  
  if (i_echo) focus_on();                 /* turn on cursor                  */

  return 0;
}
/*--------------------------------------------------------------------------*
 *  Keystroke Event
 *--------------------------------------------------------------------------*/
keystroke(c)
register unsigned char c;
{
  register long i, n;

  if (!c || !serving || !focus)           /* something is wrong              */
  {
    Putchar(BEEP);                        /* beep touble                     */
    fflush(stdout);
    return 0;
  }
  if (error)                              /* something on last line          */
  {
    cursor_out();                         /* turn off cursor                 */
    error = 0;                            /* error line removed              */
    error_position = -1;                  /* reset log to most recent        */
    position(23, 0);                      /* last line on screen             */
    fwrite(crt[23], 80, 1, stdout);       /* show actual screen line         */
    focus_on();                           /* turn cursor on                  */
  }
  if (c == RETURN || c == TAB || c == DOWN_CURSOR ||
      c == UP_CURSOR || c >= F_KEY_1)
  {
    i_text[i_len] = c;                    /* return terminating symbol       */
    i_text[i_len + 1] = f_col;            /* where the cursor is now         */
    
    message_put(serving, InputFieldEvent, i_text, i_len + 2);
    cursor_out();
    focus = 0;
    return 0;
  }
  i = f_col - i_col;                      /* current length                  */

  if (c == LEFT_CURSOR)                   /* only move cursor left           */
  {
    if (i > 0)
    {
      f_col--;
      if (i_echo) Putchar(LEFT_CURSOR);   /* position one left               */
    }
    else Putchar(BEEP);
    fflush(stdout);
    return 0;
  }
  else if (c == RIGHT_CURSOR)             /* move right if any data          */
  {
    if (i < i_len - 1)                    /* can still move right            */
    {
      if (i_text[i])                      /* is data too - so can move       */
      {
        if (i_echo) Putchar(crt[f_row][f_col]);
        f_col++;
      }
      else Putchar(BEEP);                 /* am at end of data - no move     */
    }
    else Putchar(BEEP);                   /* am at end of field - no move    */
    fflush(stdout);
    return 0;
  }
  else if (c == DELETE)                   /* delete one byte under cursor    */
  {
    if (i == i_len - 1 && i_text[i])      /* delete last byte of field       */
    {
      i_text[i] = 0;                      /* delete leftmost byte            */
    }
    else                                  /* not last byte in field          */
    {
      if (i > 0)                          /* not leftmost byte               */
      {
        i--; f_col--;                     /* delete byte to left of cursor   */
      }
      if (*i_text)                        /* there is some data              */
      {
        for (; i < i_len; i++) i_text[i] = i_text[i + 1];
        i_text[i_len] = 0;
      }
      else Putchar(BEEP);                 /* no data in the field            */
    }
    show_field();
  }
  else                                    /* this is a real data byte        */
  {
    if (i_type == 'n' && (c < '0' || c > '9'))
    {
      error_message(ERR_NUMERIC, "");     /* must be a number                */
      return 0;
    }
    for (n = i_len - 2; n >= i; n--)      /* open for a new byte             */
    {
      i_text[n + 1] = i_text[n];
    }
    i_text[i] = c;                        /* insert new data                 */
    if (i < i_len - 1) f_col++;           /* move cursor if not at end       */
    show_field();
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Field To Screen With FILL Bytes
 *-------------------------------------------------------------------------*/
show_field()
{
  register long n;

  cursor_out();                           /* turn off cursor                 */

  if (i_echo)                             /* echo is also fill symbol        */
  {
    n = strlen(i_text);                   /* total length of data            */
        
    if (n > 0)                            /* copy data part                  */
    {
      memcpy(&crt[i_row][i_col], i_text, n);
    }
    if (n < i_len)                        /* fill rest                       */
    {
      memset(&crt[i_row][i_col + n], i_echo, i_len - n);
    }
    position(i_row, i_col);
    fwrite(&crt[i_row][i_col], i_len, 1, stdout);
    fflush(stdout);
  }
  focus_on();                             /* reset cursor and position       */

  return 0;
}
/*-------------------------------------------------------------------------*
 *  System Message - Display on last line
 *-------------------------------------------------------------------------*/
system_message(p, len)
register unsigned char *p;
register long len;
{
  if (len > 80) len = 80;                 /* only one line allowed           */

  if (len > 0)                            /* copy text part                  */
  {
    memcpy(last_line, p, len);
  }
  if (len < WIDE)                         /* clear rest of line              */
  {
    memset(last_line + len, 0x20, WIDE - len);
  }
  if (cursor) cursor_out();               /* turn curosr off                 */

  position(23, 0);                        /* last line of screen             */
  screen_out(p, len);                     /* output line to screen           */

  if (focus) focus_on();                  /* replace cursor if input         */
  fflush(stdout);
  error = 1;                              /* something on error line         */

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Postion Cursor
 *-------------------------------------------------------------------------*/
position(r, c)
register long r, c;                       /* row and column                  */
{
  register long k;
  char text[12];

  if (c >= 80)
  {
    sprintf(text, "%ca%dR%dC", 0x1b, r+1, c+1);
    Fputs(text, stdout);
    fflush(stdout);
    return 0;
  }
  Putchar(0x1b);                          /* cursor movement                 */
  Putchar('=');
  Putchar(0x20 + r);
  Putchar(0x20 + c);
  fflush(stdout);
        
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Focus On - Prepare for an input byte
 *-------------------------------------------------------------------------*/
focus_on()
{
  if (!focus) return 0;                   /* no input is active              */

  if (i_echo) position(f_row, f_col);     /* insure proper position          */
  else return 0;                          /* no echo - no cursor             */
  
  if (cursor) return 0;                   /* cursor is already on            */
  Fputs(cursor_on, stdout);               /* turn cursor on                  */
  fflush(stdout);
  cursor = 1;                             /* flag cursor in on               */

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Turn off cursor if on.
 *-------------------------------------------------------------------------*/
cursor_out()
{
  if (!cursor) return 0;                  /* cursor is already off           */
        
  Fputs(cursor_off, stdout);              /* turn cursor off                 */
  fflush(stdout);
  cursor = 0;                             /* flag cursor off                 */
        
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open Error Files
 *-------------------------------------------------------------------------*/
error_open()
{
  register char *p;
  char text[64];
  
  p = getenv("LANGUAGE");
  if (p)
  {
    if (*p) 
    {
      sprintf(text, "language/%s/text/eh_mess", p);
      eh_mess_fd = fopen(text, "r");
    }
  }
  if (eh_mess_fd == 0) eh_mess_fd = fopen(eh_mess_name, "r");
  
  if (eh_mess_fd == 0)
  {
    return krash("error_open", eh_mess_name, 1);
  }
  eh_err_fd = fopen(eh_err_name, "r+");
  if (eh_err_fd == 0)
  {
    eh_err_fd = fopen(eh_err_name, "w+");
    if (eh_err_fd == 0)
    {
      return krash("error_open", eh_err_name, 1 );
    }
  }
  return 0;                               /* all are open                    */
}
/*-------------------------------------------------------------------------*
 *  Display Error Log
 *-------------------------------------------------------------------------*/
error_log()
{
  char buffer[81];

  if (error_position < 0)                 /* off top of file                 */
  {
    fseek(eh_err_fd, 0, 2);               /* seek to end of file             */
    error_position = ftell(eh_err_fd);    /* bytes in the file               */
  }
  error_position -= 81;                   /* back up one record              */
  if (error_position < 0)                 /* off top of file                 */
  {
    cursor_out();
    error = 0;                            /* flag error line cleared         */
    position(23, 0);                      /* position on last line           */
    fwrite(crt[23], 80, 1, stdout);       /* display screen last line        */
    fflush(stdout);
    focus_on();
    return 0;
  }
  fseek(eh_err_fd, error_position, 0);    /* seek to message                 */

  if (fread(buffer, 81, 1, eh_err_fd) != 1) return 0;

  buffer[80] = 0;
  system_message(buffer);                 /* show 80 byte message            */
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Format An Error Message
 *-------------------------------------------------------------------------*/
error_message(n, p)
register long n;                          /* error number                    */
register char *p;                         /* error text                      */
{
  register char *q;
  register long k;
  eh_item  eh_buf;                        /* Error Message Buffer            */
  char     ebuf[81];                      /* Error File Buffer               */
  char     work[80];                      /* text work area                  */
  long     now;
  
  if (n < 1) return 0;                    /* zero is not an error            */

  if (n >= EH_MAX)
  {
    return krash("error_message", "Bad Error Number", 0);
  }
/*
 *  Read Error Message from eh_mess file
 */
  fseek(eh_mess_fd, n * sizeof(eh_item), 0);/* seek error message text       */
  
  if (fread(&eh_buf, sizeof(eh_item), 1, eh_mess_fd) != 1)
  {
    return krash("error_message - read", eh_mess_name);
  }
/*
 *  Build Error Message
 */
  memset(ebuf, 0x20, 80);                 /* clear message text              */
  ebuf[80] = 0;
  
  ebuf[0] = eh_buf.eh_nos[0];
  ebuf[1] = eh_buf.eh_nos[1];             /* move error number to output     */
  ebuf[2] = eh_buf.eh_nos[2];

/*
 *  move message text 
 */

  eh_buf.eh_lf = 0;                       /* null at end of text             */
  sprintf(work, eh_buf.eh_text, p);       /* build a message                 */
  k = strlen(work);
  if (k > 46) k = 46;
  memcpy(ebuf + 4, work, k);              /* message text                    */

/*
 *  put time in message
 */
  time(&now);
  strcpy(work, ctime(&now));              /* put date/time in output         */
  memcpy(ebuf + 51, work + 4, 20);        /* move to message                 */

/*
 *  put operator name to message
 */
  k = strlen(operator + 9);
  if (k > 8) k = 8;
  if (k > 0) memcpy(ebuf + 72, operator + 9, k);

/*
 *  Append To Error File
 */
  if (eh_buf.eh_log == 'x')
  {
    fseek(eh_err_fd, 0, 2);               /* to end of file                  */

    ebuf[80] = 0x0a;                      /* add a line feed                 */

    if (fwrite(ebuf, 81, 1, eh_err_fd) != 1)
    {
      return krash("error_message - write", eh_err_name, 1);
    }
    fflush(eh_err_fd);
  }
  if (eh_buf.eh_broadcast == 'x')
  {
    message_put(0, SystemMessageEvent, ebuf, 80);
  }
  else system_message(ebuf);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Keystroke Replay  [c == 0 is eof; c == 1 is end of field]
 *-------------------------------------------------------------------------*/
get_keystrokes()
{
  unsigned char c;
  
  while (1)
  {
    c = fgetc(pd);
    if (c == 1) break;
    if (c == 0xff)
    {
      fclose(pd);
      pd = 0;
      return 0;
    }
    message_put(msgtask, KeystrokeEvent, &c, 1);
  }
  sleep(2);
}

/* end of tty_server.c */
