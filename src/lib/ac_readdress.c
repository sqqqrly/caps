/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Readdresses entire area controller port.
 *                Returns   0  if completed.
 *                         -1  if noisy line.
 *                         >0  position of bad ac.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/06/94   |  tjt  Original inplmentation.
 *  07/06/94   |  tjt  Add ac_soft_reset call.
 *  04/23/01   |  aha  Added Technical Pub. #78.2 fix
 *-------------------------------------------------------------------------*/
static char ac_readdress_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"
 
#include <stdio.h>
#include <errno.h>

ac_readdress(port, start)
register long port;
register long start;
{
  register long count;                    /* cc count on port                */
  register long ret, trys;
  char buf1[20], buf2[20];                /* message buffers                 */
   
  ac_soft_reset(port, 9999);              /* soft reset port                 */

  count   = 0;                            /* number of ac'c on port          */
   
  ac_write(port, "9999060000", 10);       /* reset ac's to address zero      */
   
  while (1)
  {
    ret = ac_readout(port, buf2, 5);      /* read responses                  */
                                          /* aha 043201 */
    if (ret == -2) break;                 /* timeout - no more responses     */
    if (ret <= 0) return -1;              /* noisy line                      */
    
    if (memcmp(buf2, "000006", 6) != 0) return count;

    count++;
  }
  ac_write(port, "999904", 6);            /* block all communications        */
  sleep(1);
  trys = 0;

  while (count > 0)
  {
    sprintf(buf1, "000006%04d", start);   /* readdress a module              */
    ac_write(port, buf1, 10);

    ret = ac_readout(port, buf2, 5);      /* read or timeout                 */
                                          /* aha 042301 */
    if (ret <= 0) return start;           /* have an error                   */
    
    sprintf(buf1, "%04d06", start);       /* must echo new address           */
    if (strncmp(buf2, buf1, 6) != 0)      /* bad response                    */
    {
      if (trys > 3) return start;         /* addressing failed here          */
      trys++;                             /* count and try again             */
      continue;
    }
    sprintf(buf1, "%04d05", start);  
    ac_write(port, buf1, 6);              /* unblock to next ac              */
    start++;                              /* next module address             */
    count--;                              /* reduce module count             */
    trys = 0;                             /* clear retry counter             */
    sleep(1);                             /* don't remove this               */
                                          /* or 06 packet is truncated ???   */
  }
  ac_soft_reset(port, 9999);              /* software reset again            */

  return 0;                               /* zero us succesful complettion   */
}

/* end of ac_readdress.c */
