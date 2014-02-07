/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Primative area controller test.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  2/10/94    |  tjt Original implementation.
 *-------------------------------------------------------------------------*/
static char ac_test_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <string.h>

#include "microtimer.h"

long port;
char device[40];
char buffer[256];
char command[256];

main(argc, argv)
long argc;
char **argv;
{
  register long ret, len;
  register char *p;
  
  putenv("_=ac_test");
  chdir(getenv("HOME"));
  
  strcpy(device, "/dev/tty11");

  port = ac_open(device);
  ac_reset(port);


  while (1)
  {
    printf("Enter Command --> ");
    gets(command);
    ac_write(port, command, strlen(command));
  
    while (1)
    {
      start_timer(0);
      ret = ac_readout(port, buffer, 1);
      stop_timer(0);
      printf("Read: ret=%d Time = %s\n", ret, show_elapsed(0));
      if (ret <= 0) break;
      printf("Packet: [%*.*s]\n", ret, ret, buffer);
      Bdump(buffer, ret);
    } 
  }
}
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Reads one packet from area controller port.
 *               Returns:     n if a good packet (n is data length);
 *                            0 bad packet
 *
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

    if (len > 0) break;

    if (errno == EINTR) return -1;
    return krash("ac_read", "Read Failed", 1);
  }
  if (buffer[len - 1] != 0x0a)            /* no new line found               */
  {
    while (1)                             /* read until NL found             */
    {
      while (1)
      {
        len = read(port, buffer, 256);    /* find another packet             */

        if (len > 0) break;

        if (errno == EINTR) return -1;
        return krash("ac_read", "Read Failed", 1);
      }
      if (buffer[len - 1] == 0x0a) return 0;
    }
  }
/*-------------------------------------------------------------------------*
 *  good packet with NL - now check sum.
 *-------------------------------------------------------------------------*/
  if (len < 3) return 0;
  len -= 3;
  memcpy(sum, buffer + len, 2);           /* save input checksum             */

  ac_checksum(buffer, len);               /* calculate a new checksum        */
  
  if (memcmp(sum, buffer + len, 2) != 0) return 0;

  return len;                             /* this is a good packet !!!       */
}

/* end of ac_read.c */

/* end of ac_test.c */
