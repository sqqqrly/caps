/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Display Text.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *    7/7/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
static char sd_text_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "iodefs.h"
#include "sd.h"
#include "message_types.h"

sd_text(s)
register unsigned char *s;
{
  return sd_text_2(s, strlen(s));
}

sd_text_2(s, n)
register unsigned char *s;
register long n;
{
  unsigned char buf[136];
  long len;
                                                    
  if (sd_server <= 0) return krash("sd_prompt", "sd not open", 1);

  buf[0] = sd_row;                        /* message row + col + text        */
  buf[1] = sd_col;
  len    = 2;

  for (; n > 0 && *s; n--, s++)
  {
    if (*s == LINE_FEED || sd_col >= sd_width)
    {
      if (len > 2) message_put(sd_server, ScreenDisplayEvent, buf, len);

      sd_col = 0;
      sd_row++;
      if (sd_row >= 23) sd_row = 0;

      buf[0] = sd_row;                    /* new message setup               */
      buf[1] = sd_col;
      len    = 2;

      if (*s == LINE_FEED) continue;     /* do not store a lf               */
    }
    buf[len++] = *s;                      /* store byte to buffer            */
    sd_col++;
  }
  if (len > 2) message_put(sd_server, ScreenDisplayEvent, buf, len);
  
  return 0;
}

/* end of sd_text.c */
