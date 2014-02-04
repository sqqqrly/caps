/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Lock the order file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/13/94   |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char oc_lock_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include "of.h"

oc_lock()
{
  struct flock x;

  if (oc_fd <= 0) krash("oc_lock", "oc not open", 1);
  
  x.l_type   = F_WRLCK;
  x.l_whence = 0;
  x.l_start  = 0;
  x.l_len    = 0;
  
  while (1)
  {
    if (fcntl(oc_fd, F_SETLKW, &x) != -1) return 0;
    
    if (errno == EDEADLK) krash("oc_lock", "deadlock", 1);
    
    if (errno != ENOLCK && errno != EINTR)  krash("oc_lock", "failed", 1);
    
    sleep(1);                               /* wait and try again            */
  }
  return 0;
}

/* end of oc_lock.c */
