/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Download one ac with firmware.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/07/94   |  tjt  Original implementation.
 *  07/06/94   |  tjt  Add ac_soft_reset.
 *-------------------------------------------------------------------------*/
static char ac_download_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

long ac_download(port, address, name)
register long port;
register long address;
register char *name;
{
  register long len, count;
  FILE *fd;                               /* file descriptor                 */
  char buf[256];                          /* firmware buffer                 */
  char buf1[270];
  
  fd = fopen(name, "r");
  if (fd == 0) return krash("ac_download - open", name, 1);	
   
  sprintf(buf1, "%04d07004800000000000", address);

  ac_write(port, buf, 21);                /* clobber low memory              */
  sleep(1);
  ac_reset(port);                         /* hard reset the port             */

  count = 0;

  while(fgets(buf, 256, fd))
  {
    len = strlen(buf) - 1;                /* data length                     */
    
    sprintf(buf1, "%04d07%*.*s", address, len, len, buf);

    ac_write(port, buf1, len + 6);        /* send firmware packet            */
    
    count++;                              /* count packets sent              */
    if (!(count %10)) sleep(1);           /* wait 1 second                   */
  }
  fclose(fd);

  ac_soft_reset(port, address);           /* software reset                  */

  return 0;
}

/* end of ac_download.c */
