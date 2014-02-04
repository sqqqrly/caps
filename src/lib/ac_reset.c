/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Performs hard reset of area controller port by toggling RTS.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  02/10/94   |  tjt  Original implementation.
 *  09/07/95   |  tjt  Modifed for PC.
 *-------------------------------------------------------------------------*/
static char ac_reset_c[] = "%Z% %M% %I% (%G% - %U%)";


#include <termio.h>

long ac_reset(port)
register long port;
{
  register long ret;
  char text[80];

#ifdef DIGI
  sprintf(text, 
    "test -x /usr/bin/ditty && ditty rts %s", ttyname(port));   /* RTS on    */
  system(text);
  
  sleep(1);

  sprintf(text, 
    "test -x /usr/bin/ditty && ditty -rts %s", ttyname(port));  /* RTS off   */
  system(text);
  sleep(3);
#else
  ac_soft_reset(port, 9999);
#endif

  ret = ioctl(port, TCFLSH, 0);           /* flush input queue               */
  if (ret < 0) return krash("tc_reset", "Flush Input", 1);
   
  ret = ioctl(port, TCFLSH, 1);           /* flush output queue              */
  if (ret < 0) return krash("tc_reset", "Flush Output", 1);
   
  sleep(1);

  return 0;
}

/* end of ac_reset.c */
