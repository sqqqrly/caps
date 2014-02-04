/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Unlock pending product file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/12/94   |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char ppf_unlock_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>

extern FILE *ppf_fd;

long ppf_unlock()
{
  struct flock x;
  
  if (ppf_fd <= 0) krash("ppf_unlock", "ppf not open", 1);
  
  x.l_type   = F_UNLCK;
  x.l_whence = 0;
  x.l_start  = 0;
  x.l_len    = 0;
 
  if (fcntl(fileno(ppf_fd), F_SETLK, &x) == -1)
  {
    krash("ppf_unlock", "failed", 1);
  }
  return 0;                               /* finished ok                     */
}

/* end of ppf_unlock.c */
