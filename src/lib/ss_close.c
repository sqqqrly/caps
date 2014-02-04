/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    close system shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   04/12/94  |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char ss_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "file_names.h"
#include "ss.h"

long ss_close()
{
  if (ssi)                                /* check is now open ok            */
  {
    shmdt(ssi);                           /* release system segment          */
    ssi   = 0;
    sp    = 0;
    rf    = 0;                            /* mark released                   */
    ss_id = 0;
  }
  if (ss_fd > 0)                          /* check file is open ok           */
  {
    close(ss_fd);                         /* close file                      */
    ss_fd = 0;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save Shared Segment To Disk
 *--------------------------------------------------------------------------*/
ss_save()
{
  if (ss_fd <= 0) return 0;

  lseek(ss_fd, 0, 0);
  
  if (write(ss_fd, ssi, ss_size) != ss_size) 
  {
    krash("ss_save", "write error", 1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save And Close System Segment
 *-------------------------------------------------------------------------*/
ss_close_save()
{
  ss_save();
  return ss_close();
}

/* end of ss_close.c */
