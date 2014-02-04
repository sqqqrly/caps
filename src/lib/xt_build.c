/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Build initial part of a transaction.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  12/15/94   |  tjt  Add lot number.
 *  07/25/95   |  tjt  Stkloc is remaining picks on 'S' or 'P' transactions.
 *  09/21/95   |  tjt  Stkloc is box number on 'S' or 'P' transactions.
 *  10/30/95   |  tjt  Add orphan 'O' transaction.
 *-------------------------------------------------------------------------*/
static char xt_build_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "xt.h"
#include "Bard.h"
#include "message.h"
#include "message_types.h"

long xt_build(con,grp,on,pl,code,sku,mod,loc,ordered,picked,zno,lot)

char *con;                                /* customer order number           */
char *grp;                                /* group code                      */
long on;                                  /* order number                    */
long pl;                                  /* pickline                        */
char code;                                /* transaction code                */
char *sku;                                /* sku if sku system               */
long mod;                                 /* module number                   */
char *loc;                                /* stkloc if restock               */
long ordered, picked;                     /* quantities short/picked/restock */
long zno;                                 /* zone number                     */
char *lot;                                /* lot number                      */
{
  struct trans_item xt;
  char work[32];

  if (ss_fd <= 0) krash("xt_build", "ss not open", 1);
  
  memset(&xt, 0x20, sizeof(struct trans_item));
  
  xt.xt_time = time(0);
  
  if (grp)
  {
    sprintf(work, "%-*.*s", GroupLength, GroupLength, grp);
    memcpy(xt.xt_group, work, GroupLength);
  }
  if (con)
  {
    sprintf(work, "%-*.*s", CustomerNoLength, CustomerNoLength, con);
    memcpy(xt.xt_con, work, CustomerNoLength);
  }
  sprintf(work, "%08d", on);
  memcpy(xt.xt_on, work + 8 - OrderLength, OrderLength);
  
  sprintf(work, "%08d", pl);
  memcpy(xt.xt_pl, work + 8 - PicklineLength, PicklineLength);
  
  xt.xt_code = code;

  if (code == 'A' || code == 'S' || code == 'K' || code == 'P' || code == 'L')
  {
    if (rf->rf_sku) memcpy(xt.xt_sku_mod1, sku, rf->rf_sku);
    else 
    {
      sprintf(work, "%08d", mod);
      memcpy(xt.xt_sku_mod1 + SkuLength - 5, work + 3, 5);
    }
  }
  else if (code == 'O')
  {
    memcpy(xt.xt_sku_mod1, sku, rf->rf_sku);
    sprintf(work, "%08d", ordered);
    memcpy(xt.xt_quan1, work + 8 - QuantityLength, QuantityLength);
    memset(xt.xt_quan2, '0', QuantityLength);
  }
  if (loc) 
  {
    sprintf(work, "%-6.6s", loc);
    memcpy(xt.xt_stkloc, work, 6);
  }
  if (code == 'S' || code == 'P' || code == 'L' || code == 'K')
  {
    sprintf(work, "%08d", ordered);
    memcpy(xt.xt_quan1, work + 8 - QuantityLength, QuantityLength);

    sprintf(work, "%08d", picked);
    memcpy(xt.xt_quan2, work + 8 - QuantityLength, QuantityLength);

    sprintf(work, "%08d", zno);
    memcpy(xt.xt_zone, work + 8 - ZoneLength, ZoneLength);
  }
  if (code == 'U' || code == 'C')
  {
    sprintf(work, "%08d", zno);
    memcpy(xt.xt_zone, work + 8 - ZoneLength, ZoneLength);
  }
  if (lot && (code == 'P' || code == 'S' || code == 'L'|| code == 'B'))
  {
    sprintf(work, "%-*.*s", LotLength, LotLength, lot);
    memcpy(xt.xt_lot, work, LotLength);
  }
  sp->sp_to_count += 1;                    /* count transactions             */
  xt.xt_ref = sp->sp_to_count;             /* reference key for file         */
  
  if (sp->sp_to_flag == 'y' || sp->sp_to_flag == 'b')
  {
    transaction_write(&xt);                /* write transaction              */
  }
  if (sp->sp_to_flag == 'q' || sp->sp_to_flag == 'b')
  {
    if (!msgtask) krash("xt_build", "ipc not open", 1);

    message_put(0, TransactionEvent, &xt, sizeof(struct trans_item));
  }
  return 0;
}
   
/* end of xt_build.c */
