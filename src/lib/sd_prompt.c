/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Display Prompt Text.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *    7/7/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
static char sd_prompt_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "iodefs.h"
#include "sd.h"
#include "message_types.h"

sd_prompt(f, r)
register struct fld_parms *f;
register short r;
{
  unsigned char buf[82];
  register long i, len, tlen, col;

  if (sd_server <= 0) return krash("sd_prompt", "sd not open", 1);

  if (f->irow > 24 || f->irow < 1) return -1;/* check row parameter          */

  memset(buf, 0x20, 80);                  /* clear while prompt area         */

  if (f->prompt)                          /* there is prompt text            */
  {
    i    = f->icol - f->pcol;             /* offset to input area            */
    col  = f->pcol;                       /* leftmost column                 */
    tlen = i + *(f->length);              /* total length                    */
    
    len = strlen(f->prompt);              /* length of prompt text           */
    if (len > tlen) tlen = len;

    memcpy(buf + 2, f->prompt, len);      /* put prompt in message           */
    
    if (f->arrow)                         /* arrow after prompt              */
    {
      memcpy(buf + i - 1, "-->", 3);      /* arrow to buffer                 */
    }
  }
  else                                    /* no prompt text                  */
  {
    i = 0;                                /* offset to input                 */
    col = f->icol;                        /* leftmost column                 */
    tlen = *(f->length);                  /* total length                    */
  }
  if (sd_echo_flag)
  {
    memset(buf + i + 2, sd_echo_flag, *(f->length));
  }
  buf[0] = f->irow + r - 1;               /* actual row                      */
  buf[1] = col - 1;
  
  message_put(sd_server, ScreenDisplayEvent, buf, tlen + 2);

  return r;
}

/* end of sd_prompt.c */
