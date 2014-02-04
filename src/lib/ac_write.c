/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Calculate checksum and send message with NL added.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  2/10/94    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char ac_write_c[] = "%Z% %M% %I% (%G% - %U%)";
 
#include <stdio.h>
#include <errno.h>

long ac_write(port, buffer, len)
register long port;
register unsigned char *buffer;
register long len;
{
  register long ret;
  unsigned char work[256];       

  memcpy(work, buffer, len);               /* copy to add checksum           */

  len = ac_checksum(work, len);            /* add checksum to message        */
   
  while (1)
  {
    ret = write(port, work, len);          /* write to port                  */
  
    if (ret < 0)
    {
      if (errno == EINTR) continue;
      return krash("ac_write", "system error", 1);
    }
    break;
  }
  if (ret != len) return krash("ac_write", "write error", 1);

  return 0;
}

/* end of ac_write.c */
