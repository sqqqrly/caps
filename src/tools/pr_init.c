/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear productivity table.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/24/93    |  tjt  Rewritten.
 *-------------------------------------------------------------------------*/
static char pr_init_c[] = "%Z% %M% %I% (%G% - %U%)";

/*  pr_init.c
 *
 *  builds a productivity segment
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_names.h"
#include "pr.h"
#include "ss.h"

FILE *fd;

struct pr_record    *pr;
struct pr_pl_item   *pp;
struct pr_zone_item *pz;
main()
{
  register long k;
  register long size, hsize, psize, zsize, now;

  printf("Clear Productivity Table\n");
   
  putenv("_=pr_init");                    /* name to environ                 */
  chdir(getenv("HOME"));                  /* to home directory               */

  ss_open();
  
  hsize = sizeof(struct pr_record);
  psize = sizeof(struct pr_pl_item);
  zsize = sizeof(struct pr_zone_item);

  size  = hsize + psize * sp->sp_picklines + zsize * sp->sp_zones;
  
  printf("Picklines:   %5d\n", sp->sp_picklines);
  printf("Zones:       %5d\n", sp->sp_zones);
  printf("Total Bytes: %5d\n\n", size);

  pr = (struct pr_record *)malloc(hsize);
  pp = (struct pr_pl_item *)malloc(psize);
  pz = (struct pr_zone_item *)malloc(zsize);
  
  if (!pr || !pp || !pz)
  {
    printf("*** Unable to allocate space\n\n");
    exit(1);
  }
  memset(pr, 0, hsize);
  memset(pp, 0, psize);
  memset(pz, 0, zsize);

  pr->pr_picklines = sp->sp_picklines;
  pr->pr_zones     = sp->sp_zones;

  now = time(0);

  pp->pr_pl_cur_start = now;
  pp->pr_pl_cum_start = now;
  pp->pr_pl_current   = now;
  
  fd = fopen(pr_name, "w");
  if (fd == 0)
  {
    printf("** pr_init failed on open %s\n", pr_name);
    exit(1);
  }
  if (fwrite(pr, hsize, 1, fd) != 1)
  {
    printf("*** pr_init write error on %s\n\n", pr_name);
	 exit(1);
  }
  for (k = 0; k < sp->sp_picklines; k++)
  {
    if (fwrite(pp, psize, 1, fd) != 1)
    {
      printf("*** pr_init write error on %s\n\n", pr_name);
	   exit(1);
	 }  
  }
  for (k = 0; k < sp->sp_zones; k++)
  {
    if (fwrite(pz, zsize, 1, fd) != 1)
    {
      printf("*** pr_init write error on %s\n\n", pr_name);
	   exit(1);
    }
  }
  ss_close();
  fclose(fd);

  pr_remove();

  printf("All Done\n\n");
  return 0;
}

/* end of pr_init.c */
