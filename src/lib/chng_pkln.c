/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Change pickline in environment and on screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Revised.
 *  02/12/95   |  tjt  Add -1 returns.
 *-------------------------------------------------------------------------*/
static char chng_pkln_c[] = "%Z% %M% %I% (%G% - %U%)";

/*--------------------------------------------------------------------------*
 * change the pickline on the screen
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "ss.h"
#include "co.h"
#include "sd.h"
#include "message_types.h"

chng_pkln(n)
char *n;
{
  static unsigned char buf1[32];
  unsigned char buf[16], buf2[9];
  register long k;
  
  if (!sp) return -1;                     /* ss must be open                 */

  k = atol(n);                            /* pickline as a number            */
  if (k < 1) return -1;                   /* must be over one                */
  if (k > sp->sp_picklines) return -1;    /* too big                         */
  
  sprintf(buf1, "PICKLINE=%d", k);        /* setup for environment           */
  putenv(buf1);                           /* put in environment              */
  
  strcpy(buf2, "PICKLINE");               /* default name                    */

  if (co)                                 /* configuration must be open      */
  {
    if (pl[k - 1].pl_pl) 
    {
      strncpy(buf2, pl[k - 1].pl_name, 8);
      
      sprintf(buf, "%c%c%-8.8s %2d", 1, 67, buf2, k);

      message_put(sd_server, ScreenDisplayEvent, buf, 13);
    }
  }
  return 0;
}

/* end of chng_pkln.c */
