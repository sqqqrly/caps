/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Close order order index.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/13/93   |  tjt  Added to mfc.
 *  07/23/93   |  tjt  Rewritten.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char oc_close_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "file_names.h"
#include "of.h"

oc_close()
{
  if (oc_id)                              /* check is now open ok            */
  {
    shmdt(oc);                            /* release system segment          */
    oc    = 0;
    oc_id = 0;
  }
  if (oc_fd > 0)                          /* check index file is open        */
  {
    close(oc_fd);                         /* close index file                */
    oc_fd = 0;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save Shared Segment To Disk
 *--------------------------------------------------------------------------*/
oc_save()
{
  if (oc_fd <= 0) return 0;

  lseek(oc_fd, 0, 0);
  if (write(oc_fd, oc, oc_size) != oc_size)
  {
    krash("oc_save", "write error",1 );
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Save And Close Shared Segment
 *-------------------------------------------------------------------------*/
oc_close_save()
{
  oc_save();
  return oc_close();
}

/* end of oc_close.c */
