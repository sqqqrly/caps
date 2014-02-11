/*-------------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Order, picks, remarks, and pending file build.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/07/95   |  tjt  Rewritten for INFORMIX
 *  01/11/97   |  tjt  Store date time as last purge time.
 *  07/29/99   |  ravi Changed from informix dbaccess to sqlplus for oracle
 *-------------------------------------------------------------------------*/
static char of_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"
#include "global_types.h"
#include "ss.h"
#include "of.h"

extern char *getenv();

FILE *fd;

main(argc, argv)
long argc;
char **argv;
{
  register char *p;
  register long k, h, totsize;
  
  printf("Initialize Order Index Shared Segment\n");
  printf("Initialize Order, Pick, Remarks, and Pending Database Files\n\n");

  putenv("_=of_init");                    /* name to environ                 */
  p = (char *)getenv("HOME");             /* insure in home directory        */
  if (p) chdir(p);

  ss_open();                              /* get needed parameters           */

#ifdef INFORMIX
  system ("dbaccess -e $DATABASE $HOME/src/h/informix/orders.sql 1>dat/log/of_init.log 2>&1");
  system ("dbaccess -e $DATABASE  $HOME/src/h/informix/picks.sql 1>dat/log/of_init.log 2>&1");
  system ("dbaccess -e $DATABASE  $HOME/src/h/informix/remarks.sql 1>dat/log/of_init.log 2>&1");
  system ("dbaccess -e $DATABASE  $HOME/src/h/informix/pending.sql 1>dat/log/of_init.log 2>&1");
#endif

#ifdef ORACLE
  system ("sqlplus -s / @$HOME/src/h/oracle/orders.sql 1>dat/log/of_init.log 2>&1");
  system ("sqlplus -s / @$HOME/src/h/oracle/picks.sql 1>dat/log/of_init.log 2>&1");
  system ("sqlplus -s / @$HOME/src/h/oracle/remarks.sql 1>dat/log/of_init.log 2>&1");
  system ("sqlplus -s / @$HOME/src/h/oracle/pending.sql 1>dat/log/of_init.log 2>&1");
#endif
  totsize = sizeof(struct oc_rec) + sp->sp_orders * sizeof(struct oi_item);

  fd = fopen(oc_name, "w");
  if (fd == 0)
  {
    printf("Open Failed on %s\n", oc_name);
    exit(1);
  }
  oc = (struct oc_rec *)malloc(totsize);  /* allocate segment                */

  memset(oc, 0, totsize);                 /* clear entirely to zero          */

  oc->of_last_purge = time(0);
  oc->of_size       = sp->sp_orders;      /* number of orders                */
  oc->of_max_picks  = sp->sp_picks;       /* maximum picks per order         */

  printf("Order Index Allows %5d Orders\n", sp->sp_orders);
  printf("Picks       Allows %5d Picks Per Order\n", sp->sp_picks);
  if (rf->rf_rmks)
  {
    printf("Remarks     Allows %5d Bytes\n", rf->rf_rmks);
    if (rf->rf_ignore_rmks == 'y')
      printf("            But Remarks Are Ignored\n");
  }
  if (rf->rf_pick_text)
  {
    printf("Pick Text   Allows %5d Bytes\n", rf->rf_pick_text);
    if (rf->rf_ignore_pick_text == 'y')
      printf("            But Pick Text Is Ignored\n");
  }
  if (rf->rf_box_len)            printf("Box Numbers Allowed\n");
  if (!rf->rf_rmks)              printf("No Order Remarks Text\n");
  if (!rf->rf_pick_text)         printf("No Pick Text\n");
  if (!rf->rf_box_len)           printf("No Box Numbers\n");
  if (sp->sp_lot_control != 'y') printf("No Lot Numbers\n");
  printf("\n");
  
  oc->of_rec_size = sizeof(struct of_header);
  oc->op_rec_size = sizeof(struct of_pick_item);
  oc->or_rec_size = rf->rf_rmks;

  k = fwrite(oc, 1, totsize, fd);
  if (k != totsize)
  {
    printf("*** write error on %s\n", oc_name);
    return 1;
  }
  fclose(fd);
  ss_close();

  oc_remove();

  printf("\nAll Done\n\n");

  return 0;
}

/* end of of_init.c */
