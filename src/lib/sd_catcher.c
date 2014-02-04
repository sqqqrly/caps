/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Capture data from screen events.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/17/93   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*
 * Processes asynchronous messages while waiting for terminal input.
 *-------------------------------------------------------------------------*/
static char sd_catcher_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"
#include "message_types.h"

/*-------------------------------------------------------------------------*
 *  Screen Signal Catcher
 *-------------------------------------------------------------------------*/
sd_catcher(who, type, buf, len)
register long who;
register long type;
register unsigned char *buf;
register long len;
{
  if (type == InputFieldEvent)            /* got an input field              */
  {
    if (len > 0) memcpy(sd_buf, buf, len);/* save buffer                     */
  }
  else if (type == KeystrokeEvent)        /* got a keystroke                 */
  {
    sd_buf[0] = buf[0];                   /* save keystroke                  */
  }
  else if (type == TTYServerOpenEvent)    /* got a server open respsonse     */
  {
    sd_server = who;                      /* save server message id          */
  }
  else if (sd_func)                       /* process for other messages      */
  {
    (*sd_func)(who, type, buf, len);      /* process a real message          */
  }
  return 0;
}

/* end of sd_catcher.c */
