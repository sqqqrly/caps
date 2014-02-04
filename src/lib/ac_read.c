/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Reads one packet from area controller port.
 *               Returns:     n if a good packet (n is data length);
 *                            0 bad packet.
 *                           -1 interrupted by signal.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   2/10/94   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char ac_read_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <errno.h>

long ac_read(port, buffer)
register long port;
register unsigned char *buffer;
{
  register long len;
  unsigned char sum[2];
  
/*-------------------------------------------------------------------------*
 *  read a packet
 *-------------------------------------------------------------------------*/
  while (1)
  {
    len = read(port, buffer, 256);        /* primary read of good? packet    */

    if (len < 0) 
    {
      if (errno == EINTR) return -1;
      return krash("ac_read", "Read Failed", 1);
	 }
	 break;
  }
  if (buffer[len - 1] != 0x0a)            /* no new line found               */
  {
    while (1)                             /* read until NL found             */
    {
      while (1)
      {
        len = read(port, buffer, 256);    /* find another packet             */

        if (len < 0)
        {
          if (errno == EINTR) return -1;
          return krash("ac_read", "Read Failed", 1);
        }
        break;
      }

      if (buffer[len - 1] == 0x0a) 
      {
        buffer[len - 1] = 0; return 0;
      }
    }
  }
/*-------------------------------------------------------------------------*
 *  good packet with NL - now check sum.
 *-------------------------------------------------------------------------*/
  if (len < 3) return 0;
  len -= 3;
  memcpy(sum, buffer + len, 2);           /* save input checksum             */

  ac_checksum(buffer, len);               /* calculate a new checksum        */
  
  if (memcmp(sum, buffer + len, 2) != 0) 
  {
    memcpy(buffer + len, sum, 2);
    buffer[len + 2] = 0; return 0;
  }
  return len;                             /* this is a good packet !!!       */
}

/* end of ac_read.c */
