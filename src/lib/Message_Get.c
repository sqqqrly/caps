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
 *             |
 *-------------------------------------------------------------------------*
 *  Returns task message parts (sender, type, length, and text).  Sender is
 *  the number of the sending task.  Each message is acknowledged since a
 *  new message is not sent until acknowledged.
 *-------------------------------------------------------------------------*/
static char message_get_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "kernel_types.h"

message_get(who, type, text, length)
register long *who;
register long *type;
register char *text;
register long *length;
{
  TMessageItem buf;
  register long ret;
   
  ret = Message_Get(&buf);                /* get a message                   */

  if (ret < 0) return -1;                 /* was wrong length and ignored    */

  *type = buf.m_type;                     /* request/event type              */

  if (who)    *who    = buf.m_sender;     /* sender of request/event         */
  if (length) *length = buf.m_length;     /* length of text                  */

  if (text)                               /* has text buffer                 */
  {
    if (buf.m_length > 0) memcpy(text, &buf.m_message, buf.m_length);
    text[buf.m_length] = 0;
  }
  return message_put(KernelDestination, KernelAck, 0, 0);
}

/* end of message_get.c */
