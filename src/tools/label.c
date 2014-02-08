/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Test short and restock printing.
 *
 *  Execution:      label [s|r] [pickline]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/03/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss.h"
#include "co.h"
#include "Bard.h"
#include "bard/pmfile.h"
#include "bard/short_notice.h"
#include "bard/restock_notice.h"

#define random(x)    (rand() % (x))
#define randomize(x) (rand_r(&x))

static char label_c[] = "%Z% %M% %I% (%G% - %U%)";
pmfile_item x;

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=label");
  chdir(getenv("HOME"));

  unsigned int rand_state = (time(0) % 4095); // Used to store rand()'s state.
  randomize(rand_state); // Weak pseudo random number generator state init.

  database_open();

  ss_open();
  co_open();
  
  pmfile_open(READONLY);
  pmfile_setkey(1);

  x.p_pmodno = random(coh->co_prod_cnt / 2) + 1;
  pmfile_startkey(&x);
  
  x.p_pmodno = coh->co_prod_cnt;
  pmfile_stopkey(&x);
  
  pmfile_next(&x, NOLOCK);

  if (sp->sp_short_notice == 'y')   
  {
    short_open(AUTOLOCK);
    if (argv[1][0] == 's') queue_short(atol(argv[2]));
  }
  if (sp->sp_restock_notice == 'y') 
  {
    restock_open(AUTOLOCK);
    if (argv[1][0] == 'r') queue_restock(atol(argv[2]));
  }
  leave(0);
}

/*-------------------------------------------------------------------------*
 *  Restock Notices
 *-------------------------------------------------------------------------*/
queue_restock(pickline)
long pickline;
{
  restock_notice_item rs;
  
  memset(&rs, 0, sizeof(restock_notice_item));

  rs.r_rs_time     = time(0);
  rs.r_rs_number   = random(99999) + 1;
  rs.r_rs_pl       = pickline;
  rs.r_rs_mod      = x.p_pmodno;
  rs.r_rs_quantity = random(100) + 1;
  restock_write(&rs);

  return 0;                                /* no restock notice              */
}
/*-------------------------------------------------------------------------*
 *  Enqueue Short 
 *-------------------------------------------------------------------------*/
queue_short(pickline)
long pickline;
{
  short_notice_item   sh;

  memset(&sh, 0, sizeof(short_notice_item));

  sh.s_sh_time      = time(0);
  sh.s_sh_on        = random(99999) + 1;
  sh.s_sh_pl        = pickline;
  sh.s_sh_mod       = x.p_pmodno;
  sh.s_sh_ordered   = random(100) + 1;
  sh.s_sh_picked    = random(sh.s_sh_ordered);
  sh.s_sh_remaining = random(9999);
  sh.s_sh_picker    = 0;

  memset(sh.s_sh_con, random(26) + 'A', CustomerNoLength);
  memset(sh.s_sh_grp, random(26) + 'A', GroupLength);
  
  short_write(&sh);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave(x)
register long x;
{
  pmfile_close();
  short_close();
  restock_close();

  ss_close();
  co_close();
  
  database_close();

  exit(0);
}

/* end of label.c */
