/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Close configuration shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/16/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char co_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "file_names.h"
#include "co.h"

long co_close()
{
  if (co_id)                              /* check is now open ok            */
  {
    shmdt(co);                            /* release system segment          */
    co    = 0;
    co_id = 0;
  }
  if (co_fd > 0)                          /* check file is open ok           */
  {
    close(co_fd);                         /* close file                      */
    co_fd = 0;
  }
  return 0;
}
/*--------------------------------------------------------------------------*
 *  Save Shared Segment To Disk
 *--------------------------------------------------------------------------*/
co_save()
{
  if (co_fd <= 0) return 0;

  lseek(co_fd, 0, 0);
  
  if (write(co_fd, co, co_size) != co_size)
  {
    krash("co_save", "write error", 1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save And Close Shared Segment
 *-------------------------------------------------------------------------*/
co_close_save()
{
  co_save();
  return co_close();
}

/* end of co_close.c */
