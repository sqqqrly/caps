/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Lock productivity shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/13/94   |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char pr_lock_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include "pr.h"

long pr_lock()
{

  struct flock x;

  if (pr_fd <= 0) krash("pr_lock", "pr not open", 1);
  
  x.l_type   = F_WRLCK;
  x.l_whence = 0;
  x.l_start  = 0;
  x.l_len    = 0;
  
  while (1)
  {
    if (fcntl(pr_fd, F_SETLKW, &x) != -1) return 0;
    
    if (errno == EDEADLK) krash("pr_lock", "deadlock", 1);
    
    if (errno != ENOLCK && errno != EINTR)  krash("pr_lock", "failed", 1);
    
    sleep(1);                               /* wait and try again            */
  }
  return 0;
}

/* end of pr_lock.c */

