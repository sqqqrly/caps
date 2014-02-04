/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Soft reset one or all modules on a port.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/06/94   |  tjt  Original inplmentation.
 *  04/23/01   |  aha  Added Technical Pub. #78.2 fix
 *-------------------------------------------------------------------------*/
static char ac_softreset_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"
 
ac_soft_reset(port, address)
register long port;
register long address;
{
  char buf[256];                          /* message buffers                 */
   
  sprintf(buf, "%04d01", address);        /* edit soft reset packet          */

  ac_write(port, buf, 6);                 /* soft reset port                 */
   
  while (ac_readout(port, buf, 5) >= 0) {;}  /* read responses               */
                                             /* aha 042301 */
  
  return 0;                               /* zero is succesful complettion   */
}

/* end of ac_softreset.c */
