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
 *  07/22/93   |  tjt  Rewritten.
 *  05/25/94   |  tjt  Add ignore remarks flag.
 *  05/25/94   |  tjt  Add ignore pick text flag.
 *  06/29/95   |  tjt  Add customer order number as key.
 *  12/09/95   |  tjt  Add zone key to picks.
 *  01/11/97   |  tjt  Store date time in last purge time.
 *-------------------------------------------------------------------------*/
static char of_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#ifdef BARD
#include <stdio.h>
#include "Bheader.h"
#include "file_names.h"
#include "global_types.h"
#include "ss.h"
#include "of.h"

extern char *getenv();

FILE *fd;

Bfield_item key1[2] = {                   /* pickline + order number         */
  {0, 2, SHORT},
  {4, 4, LONG}};
   
Bfield_item key2[3] = {                   /* pickline + order + module       */
  {0, 2, SHORT},
  {4, 4, LONG},
  {8, 2, SHORT}};

Bfield_item key3[2] = {                   /* pickline + group                */
  {0, 2, SHORT},
  {8, GroupLength, UNSIGNED + CHARACTER}};
   
Bfield_item key4[3] = {                   /* pickline + order + box number   */
  {0,  2, SHORT},
  {4,  4, LONG},
  {24, 4, LONG}};

Bfield_item key5[2] = {                   /* pickline + customer order no    */
  {0,  2, SHORT},
  {27 + GroupLength, CustomerNoLength, UNSIGNED + CHARACTER}};

Bfield_item key6[3] = {                   /* pickline + order + zone         */
  {0,  2, SHORT},
  {4,  4, LONG},
  {10, 2, SHORT}};

Bfield_item key7[1] = {                   /* module                          */
  { 8, 2, SHORT}};

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

  system("rm $DBPATH/orders.dat");        /* delete order file               */
  system("rm $DBPATH/orders.idx");

  system("rm $DBPATH/picks.dat");         /* delete pick file                */
  system("rm $DBPATH/picks.idx");
  
  system("rm $DBPATH/remarks.dat");       /* delete remarks file             */
  system("rm $DBPATH/remarks.idx");
  
  system("rm $DBPATH/pending.dat");       /* delete pending file             */
  system("rm $DBPATH/pending.idx");
  
  totsize = sizeof(struct oc_rec) + sp->sp_orders * sizeof(struct oi_item);

  fd = fopen(oc_name, "w");
  if (fd == 0)
  {
    printf("Open Failed on %s\n", oc_name);
    exit(1);
  }
  oc = (struct oc_rec *)malloc(totsize);  /* allocate segment                */

  memset(oc, 0, totsize);                 /* clear entirely to zero          */

  oc->of_size      = sp->sp_orders;       /* number of orders                */
  oc->of_max_picks = sp->sp_picks;        /* maximum picks per order         */

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
  oc->op_rec_size = sizeof(struct of_pick_item) - PickTextLength - LotLength;

  if (sp->sp_lot_control == 'y')
  {
    oc->op_rec_size = sizeof(struct of_pick_item);
  }
  else if (rf->rf_pick_text && rf->rf_ignore_pick_text != 'y')
  {
    oc->op_rec_size += rf->rf_pick_text;
  }
  if (oc->op_rec_size & 1) oc->op_rec_size += 1;

  oc->or_rec_size =  sizeof(struct of_rmks_item);
  oc->or_rec_size -= RemarksLength;
  if (rf->rf_ignore_rmks != 'y') oc->or_rec_size += rf->rf_rmks;

  printf("Order    File Created Empty - Record %2d Bytes\n", oc->of_rec_size);

  B_create("orders", oc->of_rec_size, oc->of_rec_size, 0, 1, 1);
  h = B_open("orders", AUTOLOCK);
  B_addindex(h, 1, 0, key1, 2);
  if (sp->sp_use_con != 'n') B_addindex(h, 2, COMPRESS, key5, 2);
  B_close(h);

  printf("Pick     File Created Empty - Record %2d Bytes\n", oc->op_rec_size);

  B_create("picks",  oc->op_rec_size, oc->op_rec_size, 0, 1, 1);
  h = B_open("picks", AUTOLOCK);
  B_addindex(h, 1, 0, key2, 3);
  if (sp->sp_box_feature != 'n') B_addindex(h, 2, 0, key4, 3);
  B_addindex(h, 3, 0, key6, 3);          /* pickline + order + zone          */
#ifdef SONOMA
  B_addindex(h, 4, 0, key7, 1);          /* module only                      */
#endif
  B_close(h);

  if (rf->rf_rmks)
  {
    printf("Remarks  File Created Empty - Record %2d Bytes\n",
      oc->or_rec_size);
  }
  B_create("remarks", oc->or_rec_size, oc->or_rec_size, 0, 1, 1);
  h = B_open("remarks", AUTOLOCK);
  B_addindex(h, 1, 0, key1, 2);
  B_close(h);

  printf("Pending  File Created Empty - Record %2d Bytes\n",
  sizeof(struct pending_item));

  B_create("pending", sizeof(struct pending_item),
  sizeof(struct pending_item), 0, 1, 1);
  h = B_open("pending", AUTOLOCK);
  B_addindex(h, 1, 0, key1, 2);
  B_addindex(h, 2, 0, key3, 2);
  B_close(h);
  
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
#else
#include <stdio.h>
main()
{
  fprintf(stderr, "Bard is not supported\n\n");
}
#endif

/* end of of_init.c */
