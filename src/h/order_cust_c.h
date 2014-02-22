/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Eckerd APS Database Table Structures
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/01/2001  | aha  Modified for Eckerd's Tote Integrity.
 * 03/17/2003  | aha  Added struct eckerd_trans_item.
 *-------------------------------------------------------------------------*/
#ifndef __ORDERCUSTCH__
#define __ORDERCUSTCH__
static char order_cust_c_h[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"
#include "date_mani_c.h"

#define GRP_SIZE           4
#define DC_CODE_SIZE       4
#define ORD_CONSTANT_SIZE  5
#define STORE_SIZE         6

#define TYPE_SIZE          2
#define SKU_SIZE           7
#define DESCR_SIZE        31
#define LOC_SIZE           8
#define WORK_CODE_SIZE    12

#define ORDER_LOADED       0
#define ORDER_RELEASED     1
#define ORDER_COMPLETE     2

#define XMIT_WAITING       0
#define XMIT_COMPLETE      1

#define TNX_EXIT           3


/*
 *  Data structure for order_cust database table
*/

typedef struct order_cust_struct
{
	long line_no;
	long caps_order_no;
        long pickline_no;
	char store_no[STORE_SIZE];
        char dc_code[DC_CODE_SIZE];
        char ord_constant[ORD_CONSTANT_SIZE];
	char group_code[GRP_SIZE];
	char planned_pick_date[11];
        long start_box;
        long end_box;
        short order_status;
        short xmit_status;
	long cust_order_nbr;
} ORDER_CUST_STRUCT;


/*
 * Data structure for order_cust_item database table
*/

typedef struct order_cust_item_struct
{
	long cust_order_nbr;
	char sku_no[SKU_SIZE];
	char pick_location[LOC_SIZE];
        char descr[DESCR_SIZE];
	long ordered_qty;
	long ratio;
        long assign_tote_id;
        char work_code[WORK_CODE_SIZE];
	char merch_type[TYPE_SIZE];
        long picked_qty;
        long actual_tote_id;
        long picker_id;
        char pick_datetime[DATETIME_SIZE];
	long cust_item_nbr;
} ORDER_CUST_ITEMS_STRUCT;


/*
 * Data structure for transaction updates
*/

typedef struct eckerd_trans_struct
{
        char          xt_code;
        long          xt_order;
        short         xt_pickline;
        short         xt_picked;
        long          xt_box;
        long          xt_picker;
        unsigned long xt_time;
        char          xt_sku[SKU_SIZE];
} Teckerd_trans_item;
#endif
/* end of order_cust_c.h */
