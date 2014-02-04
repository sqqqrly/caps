/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open productivity shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
static char pr_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "file_names.h"
#include "pr_open.h"

long pr_open()
{
  register long hsize, psize;
  
  pr_fd = open(pr_name, O_RDWR);            /* Open Segment                  */

  if (pr_fd == 0) return krash("pr_open", "failed on pr", 1);
  
  pr_size = lseek(pr_fd, 0, 2);             /* get current size              */

  pr_id = shmget(pr_key, pr_size, 0666);
  
  if (pr_id < 0)
  {
    pr_id = shmget(pr_key, pr_size, 0666 | IPC_CREAT | IPC_EXCL);
    
    if (pr_id < 0) krash("pr_open", "failed to share", 1);
    
    pr = (struct pr_record *)shmat(pr_id, 0, 0);
    if (!pr) krash("pr_open", "failed to attach", 1);
    
    lseek(pr_fd, 0, 0);
    if (read(pr_fd, pr, pr_size) != pr_size) krash("pr_open", "read error", 1);
  }
  else
  {
    pr = shmat(pr_id, 0, 0);
    if (!pr) krash("pr_open", "failed to attach", 1);
  }
  hsize = sizeof(struct pr_record);
  psize = pr->pr_picklines * sizeof(struct pr_pl_item);

  pp = (struct pr_pl_item *)((char *)pr + hsize);/* point to pickline table  */
  pz = (struct pr_zone_item *)((char *)pp + psize);/* point to zone table    */

  return 0;
}
pr_remove()                                /* remove shared segment          */
{
  pr_id = shmget(pr_key, 0, 0666);
  if (pr_id >= 0) shmctl(pr_id, IPC_RMID, 0);
  return 0;
}

/* end of pr_open.c */
