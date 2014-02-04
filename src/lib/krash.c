/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  CAPS crash error function.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/07/93   |  tjt   Original implementation.
 *-------------------------------------------------------------------------*/
static char krash_c[] = "%Z% %M% %I% (%G% - %U%)";

/*
 *  krash.c
 *
 *  Common Error Routine For CAPS Functions - Not For User Errors
 *
 *    Called as krash(where, text, fatal)
 * 
 *    where    is function name.
 *    text     is error message.
 *    fatal    is suspend flag.
 *
 */
#include <errno.h>
#include <signal.h>

long krash(where, text, fatal)
register char *where;                     /* subroutine name                 */
register char *text;                      /* error message text              */
register long fatal;                      /* 1 = fatal error                 */
{
  char message[256];

  if (strlen(text) > 80) text[80] = 0;

  sprintf(message, "ERROR: (%s errno=%d) %s",  where, errno, text);
   
  errlog(message);                        /* log message to file             */
   
  if (!fatal) return -1;                  /* serious but not fatal error     */

  errlog("Signal Sent To Suspend Program");
   
  if (kill(0, SIGTRAP) < 0)               /* attempt suspend process group   */
  {
    errlog("Suspend Signal Failed");      /* failed - exit instead           */
    exit(1);
  }
  errlog("Suspend Signal Was Ignored");
   
  return -1;
}

/* end of krash.c */
