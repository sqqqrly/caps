/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Kernel message get for a task.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/14/94   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char message_gnw_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "kernel_types.h"

extern   long msgid;                      /* task identification             */

/*-------------------------------------------------------------------------*
 *  Returns task message parts (sender, type, length, and text).  Sender is
 *  the number of the sending task.  Each message is acknowledged since a
 *  new message is not sent until acknowledged.
 *  returns 0 if ok; returns 1 if no message; returns -1 if error.
 *-------------------------------------------------------------------------*/

message_gnw(who, type, text, length)
register long *who;
register long *type;
register char *text;
register long *length;
{
  TMessageItem buf;
  register long ret;

  ret = Message_GNW(&buf);                /* get a message                   */

  if (ret <  0) return -1;                /* was wrong length and ignored    */
  if (ret == 1) return 1;                 /* no message available            */
   
  *type    = buf.m_type;                  /* request/event type              */

  if (who)    *who    = buf.m_sender;     /* sender of request/event         */
  if (length) *length = buf.m_length;     /* length of text                  */

  if (buf.m_length > 0 && text)
  {
    memcpy(text, &buf.m_message, buf.m_length);
  }
  return message_put(KernelDestination, KernelAck, 0, 0);
}

/* end of message_gnw.c */
