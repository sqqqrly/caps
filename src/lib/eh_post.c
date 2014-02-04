/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Post error message.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/7/93     |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char eh_post_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"
#include "message_types.h"
#include "eh.h"

/*-------------------------------------------------------------------------*
 *  Post an error message
 *-------------------------------------------------------------------------*/

eh_post(n, p)
register long n;                          /* error number                    */
register char *p;                         /* error text                      */
{
  register long k;
  unsigned char buf[42];

  if (n < 1) return 0;                    /* zero is not an error            */

  if (n >= EH_MAX) return -1;             /* bad error number                */

  if (p)                                  /* has some text                   */
  {
    k = strlen(p);                        /* length of text                  */
    if (k > 40) k = 40;                   /* cannot exceed 40 bytes          */
  }
  else k = 0;                             /* has no text                     */

  buf[0] = n;                             /* error number to message         */
  if (k > 0) memcpy(buf + 1, p, k);       /* error insertion text            */
  
  message_put(sd_server, ClientMessageEvent, buf, k + 1);

  return 0;
}

/* end of eh_post.c */
