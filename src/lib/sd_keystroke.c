/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Screen driver keystoke event.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *    7/7/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
static char sd_event_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"
#include "message_types.h"

/*-------------------------------------------------------------------------*
 *  sd_keystroke(NOECHO)  echo = 0  -  no cursor + no echo byte
 *  sd_keystroke(CURSOR)  echo = 1  -  cursor    + no echo byte
 *  sd_keystroke(ECHO)    echo = 3  -  cursor    + echo byte
 *-------------------------------------------------------------------------*/

unsigned char sd_keystroke(echo)
register long echo;                    
{
  unsigned char mbuf[3];
   
  if (sd_server <= 0) return krash("sd_keystroke", "sd not open", 1);

  mbuf[0] = sd_row;
  mbuf[1] = sd_col;
  mbuf[2] = echo;
  
  message_wait(sd_server, KeystrokeRequest, mbuf, 3, KeystrokeEvent, SDWAIT);

  return sd_buf[0];                   /* keystroke                        */
}

/* end of sd_keystroke.c */
