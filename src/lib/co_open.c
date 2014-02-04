/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open configuration shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/13/94   |   tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char co_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "file_names.h"
#include "co_open.h"

long co_open()
{
  long size;
  
  co_fd = open(co_name, O_RDWR);            /* Open Segment                  */

  if (co_fd < 0) return krash("co_open", "failed on co not open", 1);

  co_size = lseek(co_fd, 0, 2);             /* get current size              */

  co_id = shmget(co_key, co_size, 0666);
  
  if (co_id < 0)
  {
    co_id = shmget(co_key, co_size, 0666 | IPC_CREAT | IPC_EXCL);
    
    if (co_id < 0) krash("co_open", "failed to share", 1);

    co = (unsigned char *)shmat(co_id, 0, 0);
    if (!co) krash("co_open", "failed to attach", 1);
    
    lseek(co_fd, 0, 0);
    if (read(co_fd, co, co_size) != co_size) krash("co_open", "read error", 1);
  }
  else
  {
    co = shmat(co_id, 0, 0);
    if (!co) krash("co_open", "failed to attach", 1);
  }
  coh   = (struct co_header *)co;
 
  po    = (struct po_item *)   ((char *)co + coh->co_po_offset);
  pl    = (struct pl_item *)   ((char *)co + coh->co_pl_offset);
  sg    = (struct seg_item *)  ((char *)co + coh->co_seg_offset);
  zone  = (struct zone_item *) ((char *)co + coh->co_zone_offset);
  bay   = (struct bay_item *)  ((char *)co + coh->co_bay_offset);
  hw    = (struct hw_item *)   ((char *)co + coh->co_hw_offset);
  pw    = (struct pw_item *)   ((char *)co + coh->co_pw_offset);
  mh    = (struct mh_item *)   ((char *)co + coh->co_mh_offset);

  if (coh->co_st_offset)                   /* sku table                      */
  {
    st = (struct st_item *) ((char *)co + coh->co_st_offset);
  }
  else st = 0;
  
  if (coh->co_bl_view_offset)              /* bay lamp view                  */
  {
    blv = (struct hw_bl_view_item *) ((char *)co + coh->co_bl_view_offset);
  }
  else blv = 0;
  
  if (coh->co_zc_view_offset)              /* zone controller view           */
  {
    zcv = (struct hw_zc_view_item *) ((char *)co + coh->co_zc_view_offset);
  }
  else zcv = 0;
  
  if (coh->co_pm_view_offset)               /* pick module view              */
  {
    pmv = (struct hw_pm_view_item *)  ((char *)co + coh->co_pm_view_offset);
  }
  else pmv = 0;

  return 0;
}
long co_remove()                            /* remove shared segment         */
{
  co_id = shmget(co_key, 0, 0666);
  if (co_id >= 0) shmctl(co_id, IPC_RMID, 0);
  return 0;
}

/* end of co_open.c */
