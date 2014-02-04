/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear Screen.
 *
 *                  sd_clear(n)  0 = screen,     1 = to end of line,
 *                               2 = to end of screen, 
 *                               3 = screen off, 4 = screen on,
 *                               5 = burst out,  6 = burst in,
 *                               7 = screen 80,  8 = screen 132
 *                               9 = open tty   10 = close tty
 *                              11 = disable    12 = enable function keys
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *    7/7/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
static char sd_text_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"
#include "message_types.h"

sd_clear(x)
register long x;
{
  unsigned char buf[3];

  if (x < 0 || x > 12) return -1;         /* invalid parm                    */

  if (sd_server <= 0) return krash("sd_clear", "sd not open", 1);
  
  if (!x) sd_row = sd_col = 0;            /* to top of screen on clear all   */
  else if (x == 7) sd_width = 80;         /* change to 80 columns            */
  else if (x == 8) sd_width = 132;        /* change to 132 columns           */
  
  buf[0] = sd_row;                        /* build a clear message           */
  buf[1] = sd_col;
  buf[2] = x;

  message_put(sd_server, ScreenClearEvent, buf, 3);

  return 0;
}

/* end of sd_clear.c */
