/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Position cursor.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   7/15/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
static char sd_cursor_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"

sd_cursor(x, r, c)
register long x, r, c;
{
  if (x)                                  /* relative cursor move            */
  {
    sd_row += r;
    sd_col += c;
  }
  else                                    /* absolute cursor move            */
  {
    sd_row = r - 1;
    sd_col = c - 1;
  }
  if (sd_row < 0) sd_row = 0;             /* insure is positive              */
  if (sd_col < 0) sd_col = 0;
  
  sd_row = (sd_row % 24);                 /* keep inside screen              */
  sd_col = (sd_col % sd_width);
  
  return 0;
}

/* end of sd_cursor.c */


