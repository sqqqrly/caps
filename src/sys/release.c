/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Eckerd APS to CAPS order input file processor.
 *                  Creates CAPS formatted order input file from
 *                  designated orders stored in the APS database
 *                  tables. 
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/01/2001  | aha  Modified for Eckerd's Tote Integrity.
 *-------------------------------------------------------------------------*/
static char release[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss.h"
#include "order_cust_c.h"
#define FileName  "otext/download"

#define  OC_ARRAY_SIZE   3000

FILE *pd, *DownFilePtr;
int L_FileOpened = 0;
char pd_name[46];          /* print file                      */
char szDBName[40];	   /* Database Name                   */
char DownFileName[50];
char szWhereStmt[100];
long L_Id;
const char order_priority          = 'k';   /* always hold priority from APS */
const char order_record_preface    = '<';
const char order_record_terminator = '>';
const char order_end_of_file       = '$';

main(int argc, char *argv[])
{
	int	iOpt, iStatus ;
	char szParam[1];

	putenv("-=release");

	L_Id = getpid();
	signal_catcher(0);

	sprintf(pd_name, "%s/dat/log/release_log", getenv("HOME"));
	if((pd = fopen(pd_name, "w")) == 0)
	{
		fprintf(stderr,"Can not Open Log File\n"); exit(-1);
	}

	if(argc > 3 || argc < 2)
	{
		fprintf(stderr,"Syntax Error :\n");
		fprintf(stderr," release <DB Name> [<WhereClause>] \n");
		message("Syntax Error : release <DB Name> [<WhereClause>]", 
								-1);
	}

	strcpy(szWhereStmt, "");
	if(argc == 2)
		strcpy(szDBName, argv[1]);
	else
	{
		strcpy(szDBName, argv[1]);
		strcpy(szWhereStmt, argv[2]);
	}
	
	if(iOpenDatabase(szDBName))
		message("Could not open database" -1);

	iStatus = iReleaseOrders();

	leave(iStatus);
} 

/*-------------------------------------------------------------------------*
 *  Program Exit
 *-------------------------------------------------------------------------*/
leave(long x)
{
	char command[128];
  
	if (x) 
		fprintf(pd,"\n\n*** Release of Order not complete on Errors\n");
	
	if(L_FileOpened == 1)
		fclose(DownFilePtr);
	fclose(pd);

	iCloseDatabase();
	exit(x);
}

int iReleaseOrders()
{
   ORDER_CUST_STRUCT order_cust;
   ORDER_CUST_ITEMS_STRUCT order_item;

   long lOrders[OC_ARRAY_SIZE][2],
        lIndex     = 0L,
        array_size = 0L;
   int iError = 0;
   char szWhere[] = "WHERE order_cust.order_status = 0",
        szNewClause[200],
        szError[80];

   memset(lOrders, 0x0, 2*OC_ARRAY_SIZE); 

   sprintf(DownFileName, "%s/%s_%d" ,getenv("HOME"), FileName, L_Id);

fprintf(stderr, "DownFileName=%s\n", DownFileName);

   if ((DownFilePtr = fopen(DownFileName, "w")) == NULL)
      {
        message("Could not open file for downloading" -1);
      }

   L_FileOpened = 1;

   if (strcmp(szWhereStmt, "") != 0)
      {
        strcpy(szNewClause, szWhere);
        strcat(szNewClause, " AND ");
        strcat(szNewClause, szWhereStmt);
      }
   else
      {
        strcpy(szNewClause, szWhere);
      }

/*
  ordercust_setkey_o(2);
  order_cust.order_status = 2;
  order_cust.xmit_status = 0;
Ravi - change the where clause to xmit status value
*/

   if (!iOrderCustCreateCursor(szNewClause))
      {
        return(-1);
      }

   memset(&order_cust,0,sizeof(order_cust));
   while (iOrderCustFetchNext(&order_cust))
   //while (ordercust_next_o(&order_cust,2))
         {
fprintf(stderr,"Caps Order No = %d\n",order_cust.caps_order_no); 
fprintf(stderr,"pickline_no = %d\n",order_cust.pickline_no); 
fprintf(stderr,"start_box = %d\n",order_cust.start_box); 
fprintf(stderr,"end_box  = %d\n",order_cust.end_box); 
fprintf(stderr,"store_no = %s\n",order_cust.store_no); 
fprintf(stderr,"group_code = %s\n",order_cust.group_code); 
fprintf(stderr,"planned_pick_date = %s\n",order_cust.planned_pick_date); 
fprintf(stderr,"Cust Order Number = %d\n",order_cust.cust_order_nbr); 

           fprintf(DownFilePtr, 
                   "%c%5.5s%2.2s%2.2s%4.4s%06d%c%3.3s%02d%08d%08d\n",
                   order_record_preface,
                   order_cust.store_no,
                   &order_cust.planned_pick_date[5],  /* month - mm  */
                   &order_cust.planned_pick_date[8],  /* day - dd    */
                   &order_cust.planned_pick_date[0],  /* year - yyyy */
                   order_cust.caps_order_no,
                   order_priority,
                   order_cust.group_code,
                   order_cust.pickline_no,
                   order_cust.start_box,
                   order_cust.end_box);

           fflush(DownFilePtr);
			
fprintf(stderr, "Customer Order Number = %d\n", 
		order_cust.cust_order_nbr);
           if (!iOrderCustItemCreateCursor(order_cust.cust_order_nbr))
              {
                iError = 1 ;
              }

           while (!iError && iOrderCustItemFetchNext(&order_item))
/*
	ordercustitem_setkey_o(1);
	
           while (ordercustitem_next_o(&order_item,1))
*/
fprintf(stderr, "Sku =%s\n", order_item.sku_no);
                 {
                   fprintf(DownFilePtr, 
                           "%6.6s%04d%7.7s%05d%08d%11.11s%c\n",
                           order_item.sku_no,
                           order_item.ordered_qty,
                           order_item.pick_location,
                           order_item.ratio,
                           order_item.assign_tote_id,
                           order_item.work_code,
                           order_item.merch_type[0]);

                   fflush(DownFilePtr);
                 }

           iOrderItemCloseCursor();
           //ordercustitem_close_o();

           fprintf(DownFilePtr, "%c\n", order_record_terminator);

           if (!iError)
              {
                lOrders[lIndex][0] = order_cust.caps_order_no;
                lOrders[lIndex][1] = order_cust.pickline_no;
                lIndex++;
                array_size++;
              }

         }

   if (array_size > 0L)
      {
        fprintf(DownFilePtr, "%c", order_end_of_file);
      }
   else    /* file is empty */
      {
        if (L_FileOpened == 1)
           {
             fclose(DownFilePtr);
             L_FileOpened = 0;
             unlink(DownFileName);
           }
      }

   iOrderCustCloseCursor();
   //ordercust_close_o();

   if (array_size > 0L)
      {
        for (lIndex = 0L; lIndex < array_size; lIndex++)
            {
              memset(&order_cust, 0x0, sizeof(order_cust));
              order_cust.caps_order_no = lOrders[lIndex][0];
              order_cust.pickline_no   = lOrders[lIndex][1];
              order_cust.order_status  = ORDER_RELEASED;

              iOrderUpdate(&order_cust);
            }
      }

   return(iError);
}

/*-------------------------------------------------------------------------*
 *  Error Message Display
 *-------------------------------------------------------------------------*/
message(char *p, long flag)
{
	fprintf(pd, "%s\n", p); 

	if (flag) 
		leave(1);
	return 0;
}

/*-------------------------------------------------------------------------*
 *  Convert to Binary
 *-------------------------------------------------------------------------*/
cvrt(unsigned char *p, long n)
{
	long x;

	for (x = 0; n > 0; n--, p++)
		x = 10 * x + (*p - '0');
  	return x;
}

/* end of release.c */
