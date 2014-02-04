/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Calculate checksum and send message with STX and ETX added.
 *               Requires approximately 1.2 ms to execute !!! 
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/24/93    |  tjt  Original implementation.
 *  7/30/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char tc_write_c[] = "%Z% %M% %I% (%G% - %U%)";
 
#include <stdio.h>
#include <errno.h>

#define  STX   0x02
#define  ETX   0x03

long tc_write(port, buffer)
register long port;
register unsigned char *buffer;
{
  extern unsigned char tc_checksum();

  unsigned char output[21];
  register long ret;
   
  output[0]   = STX;                      /* add STX and ETX to message      */
  output[20]  = ETX;

  memcpy(output + 1, buffer, 18);         /* copy message to output buffer   */
         
  output[19] = tc_checksum(buffer);       /* add checksum to message         */
   
  while (1)
  {
    ret = write(port, output, 21);
  
    if (ret < 0)
    {
      if (errno == EINTR) continue;
      return krash("tc_write", "system error", 1);
    }
    break;
  }
  if (ret != 21) return krash("tc_write", "write error", 1);

  return 0;
}

/* end of tc_write.c */
