/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Create results file for transfer from CAPS to WMS using
 *                  completed order data from APS database tables.
 *
 *  Uses:           dat/number/xmit_number
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/08/01   | aha Created program for Eckerd's Tote Integrity.
 *-------------------------------------------------------------------------*/
static char trans_out_aps[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <time.h>
#include "ss.h"
#include "order_cust_c.h"

#define TRUE 1
#define FALSE 0
#define CON_ARRAY_SIZE          3000
#define MAX_XMIT_NUMBER        99999
#define DUMMY_DCCODE           "ECK"
#define TRANSMIT_HEADER_TYPE   "20"    /* Transmit Header Record Descriptor  */
#define ORDER_HEADER_TYPE      "22"    /* Order Header Record Descriptor     */
#define ORDER_DETAIL_TYPE      "24"    /* Order Detail Record Descriptor     */
#define TRANSMIT_TRAILER_TYPE  "29"    /* Transmit Trailer Record Descriptor */

FILE *od, *fd;
char *od_name = "outgoing/results.dat";   /* CAPS transaction output file    */

long record_count     = 0L;               /* record count                    */
long order_cnt        = 0L;               /* count of orders                 */
long pick_cnt         = 0L;               /* count of picks                  */
long sum_ordered      = 0L;               /* sum of ordered quantities       */
long sum_picked       = 0L;               /* sum of CAPS pick quantities     */

long xmit_number      = 0L;               /* file transmit number            */
const long new_number = 1L;               /* for rollover of transmit number */

char szDBName[16]         = {"caps"};
static int iInTransaction = FALSE;
const char order_type     = 'E';
const char header_type    = 'I';
long lCustOrderNumber[CON_ARRAY_SIZE];

extern leave();

main(argc, argv)
long argc;
char **argv[];
{
  int iStatus = 0;

  putenv("_=trans_out_aps");
  chdir(getenv("HOME"));
  
  ss_open();
  
  if (iOpenDatabase(szDBName))
     {
       krash("trans_out_aps", "Cannot open connection to database", 0);
       leave(1);
     }

  fd = fopen("dat/number/xmit_number", "r");
  if (fd) 
     {
       fscanf(fd, "%05d", &xmit_number);
       fclose(fd);
     }
  else
     {
       krash("trans_out_aps", "Cannot open file:xmit_number for read", 0);
       leave(1);
     }

  fd = fopen("dat/number/xmit_number", "w");

  if (fd) 
     {
       /* Rollover xmit_number if at max value */
       if (xmit_number == MAX_XMIT_NUMBER)
          {
            fprintf(fd, "%05d\n", new_number);
            fclose(fd);
          }
       else
          {
            fprintf(fd, "%05d\n", xmit_number + 1L);
            fclose(fd);
          }
     }
  else
     {
       krash("trans_out_aps", "Cannot open file:xmit_number for write", 0);
       leave(1);
     }

  if (xmit_number == 0)
     {
       krash("trans_out_aps", "Xmit number read from file is 0", 0);
       leave(1);
     }

  memset(lCustOrderNumber, 0x0, CON_ARRAY_SIZE);

  iStatus = iProcessHeader();

  if (iStatus)
     {
       krash("trans_out_aps", "Cannot create transmit header record", 0);
       leave(iStatus);
     }

  iStatus = iProcessOrders();

  if (iStatus)
     {
       krash("trans_out_aps", "Cannot create transaction records", 0);
       leave(iStatus);
     }

  iStatus = iProcessTrailer();

  if (iStatus)
     {
       krash("trans_out_aps", "Cannot create transmit trailer record", 0);
       leave(iStatus);
     }

  /* Only update order transmit status if actually processed any orders */
  if (order_cnt != 0L)
     {
       iStatus = iProcessXmitStatus();

       if (!iStatus)
          {
            krash("trans_out_aps", "Cannot update xmit status", 0);
            leave(1);
          }
       else
          {
            leave(0);
          }
     }
  else
     {
       leave(0);
     }
} 


/*-------------------------------------------------------------------------*
 *  Write Header Record
 *-------------------------------------------------------------------------*/
int iProcessHeader()
{
  ORDER_CUST_STRUCT order_cust;
  char szWhere[64],
       command[128];
  long now = 0L;
  struct tm *t;
  
  record_count = 1L;
  
  now = time(0);
  t = localtime(&now);

  memset(&order_cust, 0x0, sizeof(order_cust));

  memset(szWhere, 0x0, 64);
  strcpy(szWhere, " WHERE 1 = 1 ");

  memset(command, 0x0, 128);

/*
  if (!iOrderCustCreateCursor(szWhere))
     {
       return(1);
     }

*/
  /* Get an order record from APS database table, order_cust */
  /* in order to have the DC Code.  Do not worry if cannot   */
  /* fetch any records.                                      */
  ordercust_read_o(&order_cust, 0);

  //iOrderCustCloseCursor();

  sprintf(command, "%s%s",
          "if [ -s outgoing/results.dat ]; ",
          "then cp -f outgoing/results.dat outgoing/results.dat,1; fi");
  system(command);

  od = fopen(od_name, "w");
  if (od == 0)
     {
       krash("trans_out_aps", "Cannot open output file", 0);
       leave(1);
     }

  /* If no orders in APS database table order_cust, need a */
  /* character string for the DC Code.                     */
  if (order_cust.dc_code[0] == '\0')
     {
       strcpy(order_cust.dc_code, DUMMY_DCCODE);
     }

  fprintf(od, "%2.2s%05d%c%3.3s%05d%04d%02d%02d%02d%02d%02d\n",
          TRANSMIT_HEADER_TYPE,
          record_count,
          header_type,
          order_cust.dc_code,
          xmit_number,
          t->tm_year + 1900,
          t->tm_mon + 1,
          t->tm_mday,
          t->tm_hour,
          t->tm_min,
          t->tm_sec);
  
  fflush(od);
  
  return 0;
}


/*-------------------------------------------------------------------------*
 *  Process Order Header
 *-------------------------------------------------------------------------*/
int iProcessOrders()
{
  ORDER_CUST_STRUCT order_cust;
  ORDER_CUST_ITEMS_STRUCT order_item;
  int iError = 0;
  char szWhere[64];

  memset(&order_cust, 0x0, sizeof(order_cust));
  memset(&order_item, 0x0, sizeof(order_item));

  memset(szWhere, 0x0, 64);
  strcpy(szWhere, " WHERE order_status = 2 AND xmit_status = 0 ");

/*
  if (!iOrderCustCreateCursor(szWhere))
     {
       return(1); 
     }
*/

  ordercust_setkey_o(2);
  //while (iOrderCustFetchNext(&order_cust))
  order_cust.order_status = 2;
  order_cust.xmit_status = 0;
  
  while (ordercust_next_o(&order_cust, 2))
        {
          record_count += 1L;
          order_cnt    += 1L;

          fprintf(od, "%2.2s%05d%4.4s%06d%02d%c%4.4s%2.2s%2.2s\n",
                  ORDER_HEADER_TYPE,
                  record_count,
                  order_cust.store_no,
                  order_cust.caps_order_no,
                  order_cust.pickline_no,
                  order_type,
                  &order_cust.planned_pick_date[0],    /* year - yyyy */
                  &order_cust.planned_pick_date[5],    /* month - mm  */
                  &order_cust.planned_pick_date[8]);   /* day - dd    */

          fflush(od);

          lCustOrderNumber[order_cnt - 1] = order_cust.cust_order_nbr;

/*
          if (!iOrderCustItemCreateCursor(order_cust.cust_order_nbr))
             {
               iError = TRUE;
             }
          while (!iError && iOrderCustItemFetchNext(&order_item))
*/

	  ordercustitem_setkey_o(1);
          while (ordercustitem_next_o(&order_item,1))
                {
                  record_count += 1L;
                  pick_cnt     += 1L;

		  if (order_item.picker_id == 0)
			continue;

                  fprintf(od,
                          "%2.2s%05d%7.7s%6.6s%05d%05d%05d%04d%04d%08d%11.11s%08d%4.4s%2.2s%2.2s%2.2s%2.2s%2.2s%c\n",
                          ORDER_DETAIL_TYPE,
                          record_count,
                          order_item.pick_location,
                          order_item.sku_no,
                          order_item.ordered_qty,
                          order_item.ordered_qty,
                          order_item.ratio,
                          order_item.ordered_qty,
                          order_item.picked_qty,
                          order_item.actual_tote_id,
                          order_item.work_code,
                          order_item.picker_id,
                          &order_item.pick_datetime[0],  /* year - yyyy */
                          &order_item.pick_datetime[5],  /* month - mm  */
                          &order_item.pick_datetime[8],  /* day - dd    */
                          &order_item.pick_datetime[11], /* hour - hh   */
                          &order_item.pick_datetime[14], /* minute - mm */
                          &order_item.pick_datetime[17], /* second - ss */
                          order_item.merch_type[0]);
  
                  fflush(od);

                  sum_ordered += order_item.ordered_qty;
                  sum_picked  += order_item.picked_qty;

                  memset(&order_item, 0x0, sizeof(order_item));
                }

          //iOrderItemCloseCursor();
	ordercustitem_close_o();

          memset(&order_cust, 0x0, sizeof(order_cust));
        }
 
  //iOrderCustCloseCursor();
  ordercust_close_o();

  return(iError); 
}
  

/*-------------------------------------------------------------------------*
 *  Process Trailer Record
 *-------------------------------------------------------------------------*/
int iProcessTrailer()
{
  record_count += 1L;
  
  fprintf(od, "%2.2s%05d%08d%08d%08d%08d%08d%08d\n",
          TRANSMIT_TRAILER_TYPE,
          record_count,
          order_cnt,
          pick_cnt,
          sum_ordered,
          sum_ordered,
          sum_ordered,
          sum_picked);

  fflush(od);
      
  return 0;
}


/*-------------------------------------------------------------------------*
 *  Update Order Records with Xmit Status
 *-------------------------------------------------------------------------*/
int iProcessXmitStatus()
{
  ORDER_CUST_STRUCT order_update;
  int iError = 0;
  long k     = 0L;

  for (k = 0L; k < order_cnt; k++)
      {
        memset(&order_update, 0x0, sizeof(order_update));

        order_update.cust_order_nbr = lCustOrderNumber[k];
        order_update.xmit_status    = XMIT_COMPLETE;

        iBeginTransaction();

        iInTransaction = TRUE;

        iError = iOrderUpdate1(&order_update);

        iCommitTransaction();

        iInTransaction = FALSE;

        if (!iError) break;

      }

  return(iError);
}


/*-------------------------------------------------------------------------*
 *  Program Exit
 *-------------------------------------------------------------------------*/
leave(x)
register long x;
{
  char command[120];
  
  ss_close();
  iCloseDatabase();

  if (od) fclose(od);
  
  exit(x);
}
/* End of trans_out_aps.c */
