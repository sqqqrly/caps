/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Message put from a task.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/14/94   |  tjt  Original implementation.
 *             |
 *-------------------------------------------------------------------------*
 *  Contructs a proper task message to the kernel.
 *-------------------------------------------------------------------------*/
static char message_put_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "kernel_types.h"

message_put(who, type, text, length)
register long who;
register long type;
register char *text;
register long length;
{
  TMessageItem buf;

  if ((who < 0 || who > MessageTasks) && who != KernelDestination)
  {
    return krash("message_put", "Invalid Destination", 1);
  }
  if (length > MessageText)
  {
    return krash("message_put", "Message Too Long", 1);
  }
  else if (length < 0)
  {
    return krash("message_put", "Message Too Short", 1);
  }
  if (length > 0 && !text)
  {
    return krash("message_put", "Length But No Text", 1);
  }
  buf.m_sender      = msgtask;            /* this task is sender             */
  buf.m_destination = who;                /* destination or zero             */
  buf.m_type        = type;               /* message type                    */
  buf.m_length      = length;             /* length of text part             */
   
  if (length) memcpy(&buf.m_message, text, length);/* copy text part         */

  return Message_Put(&buf);               /* write message to queue          */
}

/* end of message_put.c */
