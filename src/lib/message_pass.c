/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Message pass from a task with sender specified.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  02/24/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*
 *  Contructs a proper task message to the kernel.
 *-------------------------------------------------------------------------*/
static char message_pass_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "kernel_types.h"

message_pass(sender, dest, type, text, length)
register long sender;
register long dest;
register long type;
register char *text;
register long length;
{
  TMessageItem buf;

  if ((dest < 0 || dest > MessageTasks) && dest != KernelDestination)
  {
    return krash("message_pass", "Invalid Destination", 1);
  }
  if (length > MessageText)
  {
    return krash("message_pass", "Message Too Long", 1);
  }
  else if (length < 0)
  {
    return krash("message_pass", "Message Too Short", 1);
  }
  if (length > 0 && !text)
  {
    return krash("message_pass", "Length But No Text", 1);
  }
  buf.m_sender      = sender;             /* specified sender                */
  buf.m_destination = dest;               /* destination or zero             */
  buf.m_type        = type;               /* message type                    */
  buf.m_length      = length;             /* length of text part             */
   
  if (length) memcpy(&buf.m_message, text, length);/* copy text part         */

  return Message_Put(&buf);               /* write message to queue          */
}

/* end of message_pass.c */
