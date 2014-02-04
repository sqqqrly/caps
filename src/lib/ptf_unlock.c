/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Unlock product transaction file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/12/94   |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char ptf_unlock_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>

extern FILE * ptf_fd;

long ptf_unlock()
{
  struct flock x;
  
  if (ptf_fd <= 0) krash("ptf_unlock", "ptf not open", 1);
  
  x.l_type   = F_UNLCK;
  x.l_whence = 0;
  x.l_start  = 0;
  x.l_len    = 0;
 
  if (fcntl(fileno(ptf_fd), F_SETLK, &x) == -1)
  {
    krash("ptf_unlock", "failed", 1);
  }
  return 0;                               /* finished ok                     */
}

/* end of ptf_unlock.c */
