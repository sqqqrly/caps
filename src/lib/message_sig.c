/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Request signal from kernel with each message.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/17/93   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*
 *  Requests a signal for each input message or removes such signal.
 *-------------------------------------------------------------------------*/
static char message_signal_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <signal.h>
#include "message.h"
#include "kernel_types.h"

extern message_catcher();

/*-------------------------------------------------------------------------*
 *  If value is non-zero, the signal is sent after a message is sent.
 *  Value is the code for the signal desired; zero removes signaling.
 *-------------------------------------------------------------------------*/

message_signal(value, func)
register long value;                      /* signal desired                  */
register long (*func)();                  /* message function                */
{
  msgsig  = value;                        /* to addressable field            */
  msgfunc = func;                         /* save function pointer           */

  if (value) signal(value, message_catcher);
  else signal(value, SIG_IGN);
   
  return message_put(KernelDestination, KernelSignal, &msgsig, 4);
}
/*-------------------------------------------------------------------------*
 *  Signal Catcher
 *-------------------------------------------------------------------------*/
message_catcher()
{
  TMessageItem buf;                       /* message buffer                  */
   
  if (msgsig) signal(msgsig, message_catcher);/* arm catcher again           */

  if (Message_Get(&buf) < 0)              /* get the message                 */
  {
    krash("message_catcher", "Get Failed", 0);
    return -1;
  }
  if (msgtype < 0 || buf.m_type == msgtype) msgtype = 0;

  (*msgfunc)(buf.m_sender, buf.m_type, &buf.m_message, buf.m_length);

  message_put(KernelDestination, KernelAck, 0, 0);

  return 0;
}

/* end of message_sig.c */

