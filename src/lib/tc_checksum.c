/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Calculates basic function checksum.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/24/91    |  tjt  Original implementation.
 *  7/30/03    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char tc_checksum_c[] = "%Z% %M% %I% (%G% - %U%)";

 
unsigned char tc_checksum(buffer)
register unsigned char *buffer;
{
  register unsigned long k, sum;

  sum = 0x05;
   
  for (k = 0; k < 18; k++) sum += *buffer++;

  return ((sum & 0x7f) | 0x80);
}

/* end of tc_checksum.c */
