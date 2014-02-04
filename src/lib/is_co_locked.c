/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Test if configuration shared segment is locked.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/13/94   | tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char is_co_locked_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>
#include "co.h"
#include "eh_nos.h"

long is_co_locked()
{
  struct flock x;

  if (co_fd <= 0)
  {
    return krash("is_co_locked", "co not open", 1);
  }
  x.l_type   = F_WRLCK;
  x.l_whence = 0;
  x.l_start  = 0;
  x.l_len    = 0;
  
  if (fcntl(co_fd, F_GETLK, &x) == -1) krash("is_co_locked", "get lock", 1);

  if (x.l_type == F_UNLCK) return 0;

  eh_post(ERR_HW_LOCK, 0);
  return 1;                               /* co is locked                    */
}

/* end of is_co_locked */
