/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *	 Description:    Open system shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/12/94   |  tjt  Rewritten for UNIX.
 *-------------------------------------------------------------------------*/
static char ss_open_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "file_names.h"
#include "ss_open.h"

long ss_open()
{
  ss_fd = open(ss_name, O_RDWR);           /* system file                    */

  if (ss_fd < 0) return krash("ss_open", "failed to open ss", 1);

  ss_size = lseek(ss_fd, 0, 2);            /* get size of file               */
  
  ss_id = shmget(ss_key, ss_size, 0666);   /* get shared segment id          */
  
  if (ss_id < 0)                           /* check exists                   */
  {
    ss_id = shmget(ss_key, ss_size, 0666 | IPC_CREAT | IPC_EXCL);

    if (ss_id < 0) krash("ss_open", "failed to share", 1);

    ssi = (struct ss_item *)shmat(ss_id, 0, 0);          
    if (!ssi) return krash("ss_open", "failed to attach", 1);

    lseek(ss_fd, 0, 0);
    if (read(ss_fd, ssi, ss_size) != ss_size) 
    {
      krash("ss_open", "read error", 1);
    }
  }
  else
  {
    ssi = shmat(ss_id, 0, 0);
    if (!ssi) return krash("ss_open", "failed to attach", 1);
  }
  sp = &ssi->sp_tab;                       /* point to system parameters     */
  rf = &ssi->rf_tab;                       /* point to record format         */

  return 0;
}
long ss_remove()                           /* remove shared segment          */
{
  ss_id = shmget(ss_key, 0, 0666);         
  if (ss_id >= 0) shmctl(ss_id, IPC_RMID, 0);
  return 0;
}

/* end of ss_open.c */
