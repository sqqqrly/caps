/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Screen driver field structure.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/10/93    |  tjt  Revised.
 *-------------------------------------------------------------------------*/
#ifndef SD_H
#define SD_H

static char sd_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"

extern unsigned char sd_input();          /* field input function            */
extern unsigned char sd_keystroke();      /* keytroke input function         */

#define sd_clear_screen()  sd_clear(0)    /* clear whole screen              */
#define sd_clear_line()    sd_clear(1)    /* clear to end of line            */
#define sd_clear_rest()    sd_clear(2)    /* clear to end of screen          */
#define sd_screen_off()	   sd_clear(3)    /* screen off                      */
#define sd_screen_on()     sd_clear(4)    /* screen on                       */
#define sd_burst_out()     sd_clear(5)    /* display screen middle to end    */
#define sd_reset_screen()  sd_clear(6)    /* clear stored image              */
#define sd_screen_80()     sd_clear(7)    /* screen to 80 columns            */
#define sd_screen_132()    sd_clear(8)    /* screen to 132 columns           */
#define sd_tty_open()      sd_clear(9)    /* tty to raw mode                 */
#define sd_tty_close()     sd_clear(10)   /* tty to cooked mode              */
#define sd_fkeys_disable() sd_clear(11)   /* disable function keys           */
#define sd_fkeys_enable()  sd_clear(12)   /* enable function keys            */

#undef  ECHO

#define NUMERIC         'n'
#define ALPHANUMERIC    'a'
#define BYTE            'b'
#define NOECHO          0
#define CURSOR          1
#define ECHO            3
#define SDWAIT          999999

extern long sd_server;                    /* tty_server id                   */
extern long sd_row;                       /* current row                     */
extern long sd_col;                       /* current col                     */
extern long sd_width;                     /* screen width - 80 or 132        */
extern long sd_echo_flag;                 /* echo input byte                 */
extern long (*sd_func)();                 /* message processing function     */

extern unsigned char sd_buf[81];         /* returned value buffer           */

/*-------------------------------------------------------------------------*
 *  Field definition for screen driver input
 *-------------------------------------------------------------------------*/
 
struct fld_parms
{
  char  irow;                             /* row where input field begins    */
  char  icol;                             /* col where input field begins    */
  char  pcol;                             /* col where prompt begins         */
  char  arrow;                            /* 0 = no '-->' sequence           */
                                          /* 1 = '-->' before icol           */
  short *length;                          /* pointer to length of input field*/
  char  *prompt;                          /* text of prompt                  */
  char  type;                             /* a=alpha, n=numeric, x=raw       */
};

typedef struct fld_parms Tfld_parms;      /* as a typedef                    */

#define op_row		(sd_row + 1)
#define op_col		(sd_col + 1)

#endif

/* end of sd.h */
