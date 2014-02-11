/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Save all shared segments.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/5/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char save_segs_c[] = "%Z% %M% %I% (%G% - %U%)";

/* 
 *  Insures all segments are written to disk
 */
#include <stdio.h>
#include <fcntl.h>
#include "ss.h"
#include "co.h"
#include "of.h"
#include "pr.h"

extern long ss_size, pr_size, co_size, oc_size;

main()
{
  putenv("_=save_segs");                    /* program name                  */
  chdir(getenv("HOME"));
  
  printf("Save SS, CO, PR, and OC Shared Segments\n\n");

  open_all();
  close_all();

  printf("All Done\n\n");
}
/*-------------------------------------------------------------------------*
 *  Open System Segments
 *-------------------------------------------------------------------------*/
open_all()
{
  ss_open();
  oc_open();
  co_open();
  if (sp->sp_productivity == 'y') pr_open();

  return;
}
/*-------------------------------------------------------------------------*
 *  Close All Segments
 *-------------------------------------------------------------------------*/
close_all()
{
  if (sp->sp_productivity == 'y') 
  {
    printf("Save PR - %6d Bytes\n", pr_size);
    if (!find_lock(pr_fd, "pr")) pr_close_save();
  }
  printf("Save SS - %6d Bytes\n", ss_size);
  if (!find_lock(ss_fd, "ss")) ss_close_save();
  printf("Save OC - %6d Bytes\n", oc_size);
  if (!find_lock(oc_fd, "oc")) oc_close_save();
  printf("Save CO - %6d Bytes\n", co_size);
  if (!find_lock(co_fd, "co")) co_close_save();
  return;
}
/*------------------------------------------------------------------------*
 *  Check Any File Locks
 *------------------------------------------------------------------------*/
find_lock(n , p)
{
  struct flock f;

  f.l_whence = f.l_start = f.l_len = 0;
  f.l_type = F_WRLCK;
  
  fcntl(n, F_GETLK, &f);
  if (f.l_type == F_UNLCK) return 0;
  
  printf("*** PID: %d IS LOCKING %s\n", f.l_pid, p);
  return 1;
}
 
/* end of save_segs.c */
 
