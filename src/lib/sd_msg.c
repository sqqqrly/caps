/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Display local text on bottom line of screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *    7/7/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
static char sd_msg_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "iodefs.h"
#include "sd.h"
#include "message_types.h"

sd_msg(s)
register unsigned char *s;
{
  if (sd_server <= 0) return krash("sd_msg", "sd not open", 1);

  message_put(sd_server, SystemMessageEvent, s, strlen(s));
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Special WAIT Message
 *-------------------------------------------------------------------------*/
 
sd_wait()
{
  static unsigned char wait_message[] = 
  "                      *  *  *  *  W  A  I  T  *  *  *  * ";

  register long len;
  
  if (sd_server <= 0) return krash("sd_open", "sd not open", 1);
    
  sd_cursor(0, 24, 1);
  sd_clear(1);

  len = strlen(wait_message);

  wait_message[0]       = BLINK; 
  wait_message[len - 1] = NORMAL;

  message_put(sd_server, SystemMessageEvent, wait_message, len);

  return 0;
}


/* end of sd_msg.c */
