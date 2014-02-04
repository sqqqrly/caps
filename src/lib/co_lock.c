/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Lock configuration shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/13/94   |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char co_lock_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "co.h"

long co_lock()
{
  struct flock x;

  if (co_fd <= 0) krash("co_lock", "co not open", 1);
  
  x.l_type   = F_WRLCK;
  x.l_whence = 0;
  x.l_start  = 0;
  x.l_len    = 0;
  
  while (1)
  {
    if (fcntl(co_fd, F_SETLKW, &x) != -1) return 0;
    
    if (errno == EDEADLK) krash("co_lock", "deadlock", 1);
    
    if (errno != ENOLCK && errno != EINTR)  krash("co_lock", "failed", 1);
    
    sleep(1);                               /* wait and try again            */
  }
  return 0;
}

/* end of co_lock.c */
