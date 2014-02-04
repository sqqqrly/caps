/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Readdresses entire basic function port.
 *                Returns   0  if completed.
 *                         -1  if noisy line.
 *                         >0  position of bad tc.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/24/91    |  tjt  Original inplmentation.
 *  7/30/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char tc_readdress_c[] = "%Z% %M% %I% (%G% - %U%)";
 
#include <stdio.h>
#include <errno.h>

tc_readdress(port, start)
register long port;
register long start;
{
  register long count;                    /* tc count on port                */
  register long k, ret;
  char buf1[20], buf2[20];                /* message buffers                 */
   
  tc_write(port, "999900010000000000");   /* soft reset entire port          */
  sleep(1);
   
  count   = 0;                            /* number of tc'c on port          */
   
  tc_write(port, "999900110000000000");   /* reset tc's to address zero      */
   
  while (1)
  {
    ret = tc_readout(port, buf2, 2);      /* read responses                  */
    if (ret == -2) break;                 /* time out on readdress           */
    if (ret) return -1;                   /* noisy line                      */
    
    count++;
    
    if (strncmp(buf2, "0000", 4) != 0)
    {
      return count;                       /* couldn't do this one            */
    }
  }
  tc_write(port, "999900090000000000");   /* block all communications        */

  for (k = 1; k <= count; k++)            /* for all tc's on port            */
  {
    sprintf(buf1, "00000011000000%04d", start);
    tc_write(port, buf1);

    ret = tc_readout(port, buf2, 2);      /* read or timeout                 */
    if (ret) return k;                    /* have an error                   */
    
    if (strncmp(buf2, buf1 + 14, 4) != 0) return k;/* not readdressed        */

    sprintf(buf1, "%04d00100000000000", start++);
    tc_write(port, buf1);                 /* unblock to next tc              */
  }
  tc_write(port, "999900010000000000");   /* soft reset again                */
  sleep(1);
  
  return 0;
}

/* end of tc_readdress.c */
