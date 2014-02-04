/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open screen driver links.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/07/93   |  tjt  Rewrite of screen driver.
 *  02/12/96   |  tjt  Catch signals to close screen driver.
 *-------------------------------------------------------------------------*/
static char sd_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <signal.h>
#include "sd.h"
#include "message_types.h"

extern sd_catcher();
extern sd_hangup();

/*-------------------------------------------------------------------------*
 *  The routine 'func' processes messages other than screen messages.
 *  sd_open must be followed by message_select(.. ) to add other types.
 *-------------------------------------------------------------------------*/

sd_open(func)
register long (*func)();
{
  register char *p;
  long ret;
   
  static unsigned char list[] =
    {TTYServerOpenEvent, InputFieldEvent, KeystrokeEvent};

  p = (char *)getenv("TTY");              /* get tty device name             */
  if (!p)
  {
    return krash("sd_open", "TTY not found", 1);
  }
  sd_func = func;                         /* message processing routines     */

  if (message_open() < 0)                 /* open message links              */
  {
     return krash("sd_open", "message open failed", 1);
  }
  message_select(list, 3);                /* append to message list          */
  message_signal(SIGUSR1, sd_catcher);    /* arm message catcher             */
  
  signal(SIGHUP,  sd_hangup);             /* catch exit signals              */
  signal(SIGQUIT, sd_hangup);
  signal(SIGTERM, sd_hangup);
  
  ret = message_wait(0, TTYServerOpenRequest, p, strlen(p),
    TTYServerOpenEvent, 100);

  if (ret < 0) return  krash("sd_open", "timeout", 1);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Hang Up Of Task
 *-------------------------------------------------------------------------*/
sd_hangup()
{
  signal(SIGHUP,  SIG_IGN);                /* ignore further signals         */
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGTRAP, SIG_IGN);
  
  sd_close();
  exit(99);
}

/* end of sd_open.c */
