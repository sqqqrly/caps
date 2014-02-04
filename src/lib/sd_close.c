/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Close screen driver links.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *    7/7/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
static char sd_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"
#include "message.h"
#include "message_types.h"

sd_close()
{
  if (msgtask)
  {
    if (sd_server) 
    {
      if (sd_width != 80) sd_screen_80();
      message_put(sd_server, TTYServerCloseEvent, 0, 0);
    }
    message_close();
  }
  sd_server = 0;                          /* mark as closed                  */

  return 0;
}

/* end of sd_close.c */

