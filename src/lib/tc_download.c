/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Download one tc with firmware.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/24/91    |  tjt  Original implementation.
 *  7/30/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char tc_download_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

long tc_download(port, address, name)
register long port;
register long address;
register char *name;
{
  FILE *fd;                               /* file descriptor                 */
  char buf[20];                           /* firmware buffer                 */
  long count;
  
  fd = fopen(name, "r");
  if (fd == 0) return krash("tc_download - open", name, 1);	
   
  sprintf(buf, "%04d00121000000000", address);
  tc_write(port, buf);
  sleep(1);
  tc_reset(port);
  sleep(1);

  sprintf(buf, "%04d00120000000000", address);
  count = 0;
  
  while(fread(&buf[8], 1, 11, fd) == 11)
  {
    if (buf[18] != '\n')
    {
      return krash("tc_download - read", name, 1);
    }
    tc_write(port, buf);                  /* send firmware packet            */
    count++;
    if (!(count % 1)) sleep(1);           /* give packets time               */
  }
  fclose(fd);

  sprintf(buf, "%04d00010000000000", address);
  tc_write(port, buf);
  sleep(1);

  return 0;
}

/* end of tc_download.c */
