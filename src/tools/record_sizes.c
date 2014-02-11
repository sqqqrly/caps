/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Sizes of all CAPS Record Structures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/19/96   |  tjt  Original implementation.
 *  10/08/01   |  aha  Modified for Eckerd's Tote Integrity.
 *--------------------------------------------------------------------------*/
static char record_sizes_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include "bard/box_packing_list.h"
#include "boxes.h"
#include "bard/location.h"
#include "bard/maint_log.h"
#include "bard/lot.h"
#include "bard/operator.h"
#include "bard/orders.h"
#include "bard/packing_list.h"
#include "bard/pending.h"
#include "bard/picker.h"
#include "bard/picker_order.h"
#include "bard/picks.h"
#include "bard/pmfile.h"
#include "bard/prodfile.h"
#include "bard/queue.h"
#include "bard/remarks.h"
#include "bard/restock_notice.h"
#include "bard/ship_label.h"
#include "bard/short_notice.h"
#include "bard/tote_label.h"
#include "bard/transaction.h"

main()
{
  printf("CAPS Record Sizes\n\n");

  printf("Database File       Size\n");
  printf("----------------   -------\n");
  printf("box_packing_list   %5d\n", sizeof(box_packing_list_item));
  printf("boxes              %5d\n", sizeof(boxes_item));
  printf("location           %5d\n", sizeof(location_item));
  printf("lot                %5d\n", sizeof(lot_item));
  printf("maint_log          %5d\n", sizeof(maint_log_item));
  printf("operator           %5d\n", sizeof(operator_item));
  printf("orders             %5d\n", sizeof(orders_item));
  printf("packing_list       %5d\n", sizeof(packing_list_item));
  printf("pending            %5d\n", sizeof(pending_item));
  printf("picker             %5d\n", sizeof(picker_item));
  printf("picker_order       %5d\n", sizeof(picker_order_item));
  printf("picks              %5d\n", sizeof(picks_item));
  printf("pmfile             %5d\n", sizeof(pmfile_item));
  printf("prodfile           %5d\n", sizeof(prodfile_item));
  printf("queue              %5d\n", sizeof(queue_item));
  printf("remarks            %5d\n", sizeof(remarks_item));
  printf("restock_notice     %5d\n", sizeof(restock_notice_item));
  printf("ship_label         %5d\n", sizeof(ship_label_item));
  printf("short_notice       %5d\n", sizeof(short_notice_item));
  printf("tote_label         %5d\n", sizeof(tote_label_item));
  printf("transaction        %5d\n", sizeof(transaction_item));
  printf("\n");

}

/* end of record_sizes.c */

