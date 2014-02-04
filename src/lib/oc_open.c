/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Open order index.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  07/23/93   |  tjt  Rewritten.
 *  04/13/94   |  tjt  Modidied for UNIX.
 *-------------------------------------------------------------------------*/
static char oc_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "file_names.h"
#include "oc_open.h"

oc_open()
{
  oc_fd = open(oc_name, O_RDWR);

  if (oc_fd < 0) return krash("oc_open", "failed on oc", 1);

  oc_size = lseek(oc_fd, 0, 2);
  
  oc_id = shmget(oc_key, oc_size, 0666);
  
  if (oc_id < 0)
  {
    oc_id = shmget(oc_key, oc_size, 0666 | IPC_CREAT | IPC_EXCL);
    
    if (oc_id < 0) krash("oc_open", "failed to share", 1);
    
    oc = (struct oc_rec *)shmat(oc_id, 0, 0);
    if (!oc) krash("oc_open", "failed to attach", 1);
    
    lseek(oc_fd, 0, 0);
    if (read(oc_fd, oc, oc_size) != oc_size) krash("oc_open", "read error", 1);
  }
  else
  {
    oc = shmat(oc_id, 0, 0);
    if (!oc) krash("oc_open", "failed to attach", 1);
  }
  return 0;
}
oc_remove()                                 /* remove shared segment         */
{
  oc_id = shmget(oc_key, 0, 0666);
  if (oc_id >= 0) shmctl(oc_id, IPC_RMID, 0);
  return 0;
}
  
/*  end of oc_open.c  */
