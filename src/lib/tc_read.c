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
 *                           -1 interrupted by signal.
 *                         < 21 if short packet.
 *                         = 21 if bad format packet (STX or checksum error)
 *                              or #'s in it.
 *                         > 21 if a long packet.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/24/91    |  tjt  Original implementation.
 *  7/30/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char tc_read_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <errno.h>

#define  STX   0x02
#define  ETX   0x03

long tc_read(port, buffer)
register long port;
register unsigned char *buffer;
{
  extern unsigned char tc_checksum();

  register long k, errors, ret;
  register unsigned char *p, *q;
  unsigned char work[21];
   
  p = work; q = buffer;                   /* set pointers                    */
   
/*-------------------------------------------------------------------------*
 *  read a packet
 *-------------------------------------------------------------------------*/
  while (1)
  {
    ret = read(port, p, 21);              /* primary read of good? packet    */

    if (ret < 0) 
    {
      if (errno == EINTR) return -1;     /* interrupted by signal           */
      return krash("tc_read", "Read Failed", 1);
	 }
	 break;
  }
  memcpy(buffer, p + 1, 18);              /* copy to buffer                  */

  if (ret < 21) return ret;               /* short packet return             */

/*-------------------------------------------------------------------------*
 *  Since ETX is the wakeup byte, no ETX means packet exceed 21 bytes!
 *  No data is returned, only a count over 21.
 *-------------------------------------------------------------------------*/
  if (*(p + 20) != ETX)                   /* is a long packet                */
  {
    errors = ret;                         /* bad byte count                  */

    while (1)                             /* read until ETX found            */
    {
      while (1)
      {
        ret = read(port, p, 21);          /* find another packet             */

        if (ret < 0)
        {
          if (errno == EINTR) return -1;  /* interrupted by signal           */
          return krash("tc_read", "Read Failed", 1);
        }
        break;
      }
      errors += ret;
      if (*(p + ret - 1) == ETX) return errors;
    }
  }
/*-------------------------------------------------------------------------*
 *  good packet if STX, checksum, and #'s checkout.
 *-------------------------------------------------------------------------*/

  if (*p != STX) return 21;               /* invalid packet                  */
  
  for (k = 0; k < 18; k++)                /* check for error flags           */
  {
    if (*++p == '#') return 21;           /* has error flag bytes            */
  }
  if (tc_checksum(work + 1) != *(work + 19)) return 21;

  return 0;                               /* this is a good packet !!!       */
}


/* end of tc_read.c */
