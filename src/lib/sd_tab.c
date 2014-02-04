/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Display Key Tabs.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   10/1/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
static char sd_tab_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"
#include "message_types.h"

sd_tab(n, s)
register long n;
register unsigned char *s;
{
  register long len;
  unsigned char buf[9];
                                                    
  if (sd_server <= 0) return krash("sd_tab", "sd not open", 1);

  len = strlen(s);
  if (len > 8) len = 8;

  buf[0] = n;
  memcpy(buf + 1, s, len);

  message_put(sd_server, KeyTabRequest, buf, len + 1);
  
  return 0;
}

/* end of sd_tab.c */
