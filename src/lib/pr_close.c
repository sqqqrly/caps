/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Close productivity shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modideid for UNIX.
 *-------------------------------------------------------------------------*/
static char pr_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "file_names.h"
#include "pr.h"

long pr_close()
{
  if (pr_id)                              /* check is now open ok            */
  {
    shmdt(pr);                            /* release system segment          */
    pr    = 0;
    pr_id = 0;
  }
  if (pr_fd > 0)                          /* check file is open ok           */
  {
    close(pr_fd);                         /* close file                      */
    pr_fd = 0;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save Shared Segment To Disk
 *--------------------------------------------------------------------------*/
pr_save()
{
  if (pr_fd <= 0) return 0;
  
  lseek(pr_fd, 0, 0);
  if (write(pr_fd, pr, pr_size) != pr_size)
  {
    krash("pr_save", "write error", 1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save And Close Shared Segment
 *-------------------------------------------------------------------------*/
pr_close_save()
{
  pr_save();
  return pr_close();
}

/* end of pr_close.c */
