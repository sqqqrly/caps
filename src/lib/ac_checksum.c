/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Calculates area controller checksum.
 *               Returns length of buffer with checksum and NL.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  2/10/94    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char ac_checksum_c[] = "%Z% %M% %I% (%G% - %U%)";

 
ac_checksum(p, len)
register unsigned char *p;
register long len;
{
  static unsigned char hex[16] = 
    {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

  register unsigned char sum;
  register long k;
  
  sum = 0;
  
  for (k = 0; k < len; k++) sum += *p++;  /* arithmetic sum of all bytes     */
  
  *p++ = hex[(sum >> 4) & 0x0f];          /* check sum                       */
  *p++ = hex[sum & 0x0f];
  *p++ = 0x0a;                            /* append a line feed              */
  *p   = 0; 
  
  return (len + 3);                       /* length of packet                */
}

/* end of ac_checksum.c */
