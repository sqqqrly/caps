/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Check diskette is readable.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/17/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char is_dsk_ready_c[] = "%Z% %M% %I% (%G% - %U%)";

/*
 *  Check Diskette for Condition
 */
#include <stdio.h>
#include <fcntl.h>

long is_disk_fd;

is_dsk_ready(who)
unsigned char who;                    /* s=ibm/8, d=unos/8, l=dos, h=dos     */
{
  extern is_disk_error();
  long k;
  char buf[512];

  switch(who)
  {
    case 's':            /* Check for unos 8 Single sided single density disk*/

      is_disk_fd = open("/dev/rcfs0", O_RDWR);
      k = read(is_disk_fd, buf, 512);      /* try a read                     */
      break;

    case 'd':            /* Check for unos 8 Double sided Double density disk*/

      is_disk_fd = open("/dev/rcfd0", O_RDWR);
      k = read(is_disk_fd, buf, 512);     /* try a read                      */
      break;

    case 'l':          /* Check for unos 5 1/4 Double sided high density disk*/

      is_disk_fd = open("/dev/msdos", O_RDWR);
      k = read(is_disk_fd, buf, 512);      /* try a read                     */
      break;

    case 'h':              /* Check for unos 5 1/4 Quad Densiity 1.2 Meg disk*/

      is_disk_fd = open("/dev/msdoshd", O_RDWR);
      k = read(is_disk_fd, buf, 512);      /* try a read                     */
      break;
  }
  close(is_disk_fd);                       /* close either type disk         */

  if (k == 512) return 0;                  /* read was ok                    */
  return -1;                               /* failed for some reason ???     */
}

/* end of is_dsk_ready.c */
