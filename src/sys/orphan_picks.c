/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Find orphan picks - unassigned pick sku's
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/01/95   |  tjt  Original inplementation.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  12/02/96   |  tjt  Add units and lines to location.
 *-------------------------------------------------------------------------*/
static char orphan_picks_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ss.h"
#include "co.h"
#include "of.h"
#include "st.h"
#include "Bard.h"

FILE *fd;
char fd_name[16], ed_name[16];

main()
{
  register struct pw_item *i;
  register struct st_item *s;
  register long k, errors;
  long pid, status;
  char command[80];
  
  putenv("_=orphan_picks");
  chdir(getenv("HOME"));
  
  database_open();

  ss_open();
  co_open();
  oc_open();
  od_open();
  
  if (sp->sp_sku_support == 'n') leave(0);
  
  errors = 0;
  tmp_name(fd_name);
  
  fd = fopen(fd_name, "w");
  if (fd == 0) krash("main", "open temp", 1);
  
  if (sp->sp_sku_support == 'y')           /* only when sku support          */
  {
    for (k = 0, i = pw; k < coh->co_prod_cnt; k++, i++)
    {
      if (i->pw_lines_to_go <= 0) continue;  /* has no picks                 */
  
      s = mod_lookup(k + 1);               /* find in sku table              */
      if (s) continue;                     /* module has a sku               */
    
      fprintf(fd, "No SKU Assigned To Module %d Has %d Picks\n", 
        k + 1, i->pw_lines_to_go);

      errors++;
    }
  }
#ifdef DEBUG
  fprintf(stderr, "errors=%d\n", errors);
#endif

  fprintf(fd, "\n\n");
  pick_setkey(1);
  
  begin_work();
  while (!pick_next(op_rec, NOLOCK))
  {
#ifdef DEBUG
  fprintf(stderr, "pl=%d  on=%d  mod=%d  flag=%x\n",
    op_rec->pi_pl, op_rec->pi_on, op_rec->pi_mod, op_rec->pi_flags);
#endif

    commit_work();
    begin_work();
    
    if (op_rec->pi_flags & VALIDATED) continue;
  
    fprintf(fd, 
      "Pickline: %2d  Order: %7.*d  Mod: %5d  SKU: %-15.15s  Quan: %3d\n",

      op_rec->pi_pl, rf->rf_on, op_rec->pi_on, op_rec->pi_mod,
      op_rec->pi_sku, op_rec->pi_ordered);

    errors++;
  }
  commit_work();
  fclose(fd);

  if (errors)
  {
    tmp_name(ed_name);

    if (fork() == 0)
    {
      execlp("prft", "prft", fd_name, ed_name, "sys/report/orphan_picks.h", 0);
      krash("main", "load prft", 1);
    }
    pid = wait(&status);
    if (!pid || status) krash("main", "prft failed", 1);
   
    sprintf(command, "%s %s", getenv("LPR"), fd_name);
  }
  else unlink(fd_name);
  leave(0);
}
/*-----------------------------------------------------------------------*
 *  Graceful Exit
 *-----------------------------------------------------------------------*/
leave(x)
long x;
{
  ss_close();
  co_close();
  oc_close();
  od_close();
  database_close();
  
  exit(x);
}

/* end of orphan_picks.c */
  
  
  
  
