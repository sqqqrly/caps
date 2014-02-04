/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Opens area controller port and sets mode and state.
 *               Read in canonical mode with LF being the break 
 *               characters.  The checksum will never match these.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  02/10/94   |  tjt  Originial implementation.
 *  09/07/95   |  tjt  Revised for PC.
 *  04/23/01   |  aha  Added Technical Pub. #78.2 fix
 *-------------------------------------------------------------------------*/
static char ac_open_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"


#include <stdio.h>
#include <fcntl.h>
#include <termio.h>

long ac_open(device_name)
register char *device_name;
{
  static struct termio term = {
    {IGNBRK | IGNPAR | IGNCR},                                 /* c_iflag    */
    {0},                                                       /* c_oflag    */
    {B19200 | CS7 | PARENB | PARODD | CREAD | HUPCL | CLOCAL}, 
                                                               /* c_cflag    */
                                                               /* aha 042301 */
    {ICANON | NOFLSH},                                         /* c_lflag    */
    {0},                                                       /* c_line     */
    {0, 0, 0, 0, 0, 0x0a, 0, 0}};                              /* NL wakeup  */

  register long port, ret;                /* port file descriptor            */
  char command[80];


  port = open(device_name, O_RDWR);       /* open basic function port        */
  if (port < 0) return -1;                /* cannot open this port           */

  ret = ioctl(port, TCFLSH, 0);           /* flush input queue               */
  if (ret < 0) return krash("ac_open - Flush Input", device_name, 1);
   
  ret = ioctl(port, TCFLSH, 1);           /* flush output queue              */
  if (ret < 0) return krash("ac_open - Flush Output", device_name, 1);

  ret = ioctl(port, TCSETA, &term);       /* set port paramters              */
  if (ret < 0) return krash("tc_open - Set Termio", device_name, 1);

#ifdef DIGI
  sprintf(command, 
    "test -x /usr/bin/ditty && ditty -rts %s 1>/dev/null 2>&1", device_name);
  system(command);
  sleep(1);
#endif

  return port;
}

/* end of ac_open.c */
