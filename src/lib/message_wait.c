/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Put and Wait for message signal or signal flag.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/19/93   |  tjt  Original implementation.
 *             |
 *-------------------------------------------------------------------------*
 *  Message signal interrupts sleep and set msgflag. 
 *-------------------------------------------------------------------------*/
static char message_wait_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"

message_wait(who, type, text, length, what, interval)
register long who;
register long type;
register char *text;
register long length;
register long what;
register long interval;
{
  register long ret, when;

  msgtype = what;                         /* wait for type (-1 == anything)  */

  when = time(0) + interval;              /* timeout interval                */

  ret = message_put(who, type, text, length);

  if (ret < 0) return ret;                /* an error occurred               */

  while (interval >= 0 && msgtype)        /* until timeout or signal         */
  {
    sleep(1);
    interval = when - time(0);            /* time remaining                  */
  }
  return interval;                        /* less than zero is timeout       */
}

/* end of message_wait.c */
