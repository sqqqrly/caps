/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Lock product transaction file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/12/94   |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char ptf_lock_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

extern FILE *ptf_fd;

long ptf_lock()
{
  struct flock x;

  if (ptf_fd <= 0) krash("ptf_lock", "ptf not open", 1);
  
  x.l_type   = F_WRLCK;
  x.l_whence = 0;
  x.l_start  = 0;
  x.l_len    = 0;
  
  while (1)
  {
    if (fcntl(fileno(ptf_fd), F_SETLKW, &x) != -1) return 0;
    
    if (errno == EDEADLK) krash("ptf_lock", "deadlock", 1);
    
    if (errno != ENOLCK && errno != EINTR)  krash("ptf_lock", "failed", 1);
    
    sleep(1);                               /* wait and try again            */
  }
  return 0;
}

/* end of ptf_lock.c */
