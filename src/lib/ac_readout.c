/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Reads one packet from basic function port.
 *               Returns:     n if a good packet ( n is data length ).
 *                           -2 read timeout
 *                            0 bad packet
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   3/6/94    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char ac_readout_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>

extern ac_read_timeout();
long ac_read_timeout_flag;


long ac_readout(port, buffer, timeout)
register long port;
register unsigned char *buffer;
register long timeout;
{
  register long ret;

  ac_read_timeout_flag = 0;               /* clear flag                      */
  
  signal(SIGALRM, ac_read_timeout);       /* set read timeout catcher        */
  alarm(timeout);                         /* save time out                   */

  ret = ac_read(port, buffer);            /* read until data or timeout      */

  alarm(0);                               /* clear timeout alarm             */
  signal(SIGALRM, SIG_IGN);               /* ignore timeouts                 */
  
  if (ac_read_timeout_flag) return -2;    /* timeout was caught              */
  
  return ret;                             /* not a timeout - normal return   */
}
/*-------------------------------------------------------------------------*
 *  TC Read Timeout Catcher
 *-------------------------------------------------------------------------*/
ac_read_timeout()
{
  ac_read_timeout_flag = 1;
}

 
/* end of ac_readout.c */
