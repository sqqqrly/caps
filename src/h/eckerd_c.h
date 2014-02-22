/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Eckerd APS Input File Data Structures
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/01/2001  | aha  Modified for Eckerd's Tote Integrity.
 *-------------------------------------------------------------------------*/
#ifndef __ECKERDCH__
#define __ECKERDCH__
static char eckerd_c_h[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/*
 *  Outbound Xmit Header - 10
 */
typedef struct
{
  unsigned char tnx_code[2];           /* transaction code    '10'          */
  unsigned char line_number[5];        /* line sequence number '00001'      */
  unsigned char direction;             /* outbound direction  'O'           */ 
  unsigned char dist_center[3];        /* distribution center 'DAL' 'ATL'.. */
  unsigned char xmit_number[5];        /* transmission number               */
  unsigned char datetime[14];          /* time stamp CCYYMMDDHHMMSS         */

} ob_xmit_header;                      /* size - 30 bytes                   */  

/*
 *  Outbound Order Header - 12
 */
typedef struct
{
  unsigned char tnx_code[2];           /* transaction code  '12'            */
  unsigned char line_number[5];        /* line sequence number              */
  unsigned char caps_order[7];         /* CAPS order number                 */
  unsigned char pickline[2];           /* CAPS pickline                     */
  unsigned char store_id[5];           /* store number                      */
  unsigned char order_constant[4];     /* order constant - always 'ELHR'    */
  unsigned char group[3];              /* CAPS group                        */
  unsigned char planned_pick_date[8];  /* CCYYMMDD                          */
  unsigned char start_box_number[8];   /* order's starting box number       */
  unsigned char end_box_number[8];     /* order's ending box number         */

} ob_order_header;                     /* size - 50 bytes                   */

/*
 *  Outbound Order Detail (Pick Item) - 14
 */
typedef struct
{
  unsigned char tnx_code[2];           /* transaction code '14'             */
  unsigned char line_number[5];        /* line sequence number              */
  unsigned char pick_slot[7];          /* shipping location/route           */
  unsigned char pick_sku[6];           /* product identification            */
  unsigned char pick_desc[30];         /* product description               */
  unsigned char ordered_quantity[5];   /* 5 digit order amount              */
  unsigned char allocated_quantity[5]; /* 5 digit quantity to pick amount   */
  unsigned char allocated_ratio[5];    /* allocation ratio                  */
  unsigned char pick_quantity[4];      /* CAPS order quantity               */
  unsigned char tote_id[8];            /* Assigned box number for pick      */
  unsigned char work_code[11];         /* WMS Assignment number             */
  unsigned char merch_type[1];         /* merchandise type 'E'              */

} ob_pick_item;                        /* size - 89 bytes                   */

/*
 *  Outbound Xmit Trailer - 19 
 */
typedef struct
{
  unsigned char tnx_code[2];           /* transaction code '19'             */
  unsigned char line_number[5];        /* line sequence number              */
  unsigned char order_count[8];        /* count of order headers            */
  unsigned char pick_count[8];         /* count of order details (pick item)*/
  unsigned char sum_ordered[8];        /* sum of ordered quantities         */
  unsigned char sum_allocated[8];      /* sum of allocated quantities       */
  unsigned char sum_allocated_two[8];  /* sum of allocated-2 quantities     */
  unsigned char sum_pick[8];           /* sum of pick quantities            */

} ob_xmit_trailer;                     /* size - 55 bytes                   */

/*
 *  Common Record Area
 */
 
typedef union
{
  ob_xmit_header   XmitHeader;
  ob_order_header  OrderHeader;
  ob_pick_item     PickItem;
  ob_xmit_trailer  XmitTrailer;
  unsigned char    MaxRecord[128];

} OutBoundFile;

#endif
/* end of eckerd_c.h */
