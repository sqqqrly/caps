/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Unlock system shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/12/94   |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char pr_unlock_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <sys/fcntl.h>
#include "pr.h"

long pr_unlock()
{
  struct flock x;
  
  if (pr_fd <= 0) krash("pr_unlock", "pr not open", 1);
  
  x.l_type   = F_UNLCK;
  x.l_whence = 0;
  x.l_start  = 0;
  x.l_len    = 0;
 
  if (fcntl(pr_fd, F_SETLK, &x) == -1)
  {
    krash("pr_unlock", "failed", 1);
  }
  return 0;                               /* finished ok                     */
}

/* end of pr_unlock.c */
