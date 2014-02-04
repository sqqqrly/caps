/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Reads one packet from basic function port.
 *               Requires about 1.8 ms to read a packet. (28 times faster) !
 *               Returns:     0 if a good packet.
 *                           -1 interrupted.
 *                           -2 read timeout
 *                         < 21 if short packet.
 *                         = 21 if bad format packet (STX or checksum error)
 *                              or #'s in it.
 *                         > 21 if a long packet.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/30/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char tc_readout_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>

extern tc_read_timeout();
long tc_read_timeout_flag;


long tc_readout(port, buffer, timeout)
register long port;
register unsigned char *buffer;
register long timeout;
{
  register long ret;

  tc_read_timeout_flag = 0;               /* clear flag                      */
  
  signal(SIGALRM, tc_read_timeout);       /* set read timeout catcher        */
  alarm(timeout);                         /* save time out                   */

  ret = tc_read(port, buffer);            /* read until data or timeout      */

  alarm(0);                               /* clear timeout alarm             */
  signal(SIGALRM, SIG_IGN);               /* ignore timeouts                 */
  
  if (tc_read_timeout_flag) return -2;    /* timeout was caught              */
  
  return ret;                             /* not a timeout - normal return   */
}
/*-------------------------------------------------------------------------*
 *  TC Read Timeout Catcher
 *-------------------------------------------------------------------------*/
tc_read_timeout()
{
  tc_read_timeout_flag = 1;
}

 
/* end of tc_readout.c */
