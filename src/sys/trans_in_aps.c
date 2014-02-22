/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Eckerd APS Input File (Orders) Transaction processor.
 *                  Reads formatted input file and places data in the aps
 *                  database tables. 
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/01/2001  | aha  Modified for Eckerd's Tote Integrity.
 *-------------------------------------------------------------------------*/
static char trans_in_aps[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss.h"
#include "eckerd_c.h"
#include "order_cust_c.h"

FILE *id, *pd;
char id_name[40];               /* input eckerd order file         */
char pd_name[46];               /* log file                        */
char szDBName[40];		/* Database Name                   */
char szDistCent[DC_CODE_SIZE];  /* Distribution Center Code        */

long record_count  = 0;         /* record count                    */
long order_count   = 0;         /* count of orders                 */
long pick_count    = 0;         /* count of picks                  */
long sum_ordered   = 0;         /* sum of ordered quantities       */
long sum_allocated = 0;         /* sum of allocated quantities     */
long sum_pick      = 0;         /* sum of CAPS pick quantities     */
long lLastOrderNbr = 0;


long L_Id;
int L_TopFlag = 0;

unsigned short int k = 0;

OutBoundFile buf;

#define TRUE 1
#define FALSE 0

static int iInTransaction = FALSE;
main(int argc, char *argv[])
{
	int	iOpt;
	int 	L_ReturnStatus = 0;
	char szParam[1];
	char Command[50];

  	putenv("_=trans_in_aps");
	chdir(getenv("HOME"));

	setpgrp();

	L_Id = getpid();

	sprintf(pd_name, "%s/dat/log/trans_in_aps_log", getenv("HOME"));

	printf("Name = %s.\n", pd_name);

	if((pd = fopen(pd_name, "w")) == 0)
		fprintf(stderr,"Can not Open tmp File\n");

	if(argc > 4 || argc < 4)
	{
		fprintf(stderr,"Syntax Error :\n");
		fprintf(stderr," trans_in_aps <DB Name> <FileName> <A/M>\n");
		exit(1);
	}

	strcpy(szDBName, argv[1]);
	strcpy(id_name, argv[2]);
	strcpy(szParam, argv[3]);
	szParam[1] = 0;

	if(szParam[0] != 'A' && szParam[0] != 'M')
	{
		fprintf(stderr," Please Enter from [A/M] only\n");
		exit(1);
	}

	if((id = fopen(id_name, "r")) == 0)
		message("Can't Open Input File", 1);

	if(iOpenDatabase(szDBName))
		message("Could not open database" -1);

        memset(szDistCent, 0, DC_CODE_SIZE);

	load_file();

	iCloseDatabase();

	if(szParam[0] == 'A')
	{
		strcpy(Command, "merge ");
		strcat(Command, szDBName);
		L_ReturnStatus = system(Command);
		
		if(L_ReturnStatus == 0)
		{
			strcpy(Command, "release ");
			strcat(Command, szDBName);
			L_ReturnStatus = system(Command);
		}
		else
			leave(L_ReturnStatus);
	}
	else
		leave(0);
} 
/*-------------------------------------------------------------------------*
 *  Load File To APS Database
 *-------------------------------------------------------------------------*/
int load_file()
{
  	char *ret;
        char text[80],
             tcode[3];

        memset(text, 0, 80);
        memset(tcode, 0, 3);

	while (1) 
	{
		ret = fgets((char *)&buf, sizeof(buf), id);  

                if (!ret)
                   {
                     message("Read Error or Unexpected EOF", 1);
                   }

                if (memcmp(buf.XmitHeader.tnx_code, "10", 2) == 0)
                   {
                     process_header();
                   }
		else if (memcmp(buf.OrderHeader.tnx_code, "12", 2) == 0) 
                   {
                     process_order();
                   }
		else if (memcmp(buf.PickItem.tnx_code, "14", 2) == 0) 
                   {
                     process_pick();
                   }
                else if (memcmp(buf.XmitTrailer.tnx_code, "19", 2) == 0)
                   {
                     process_trailer();
                     break;
                   }
                else
                   {
                     strncpy(tcode, (char*)&buf, 2);
                     tcode[2] = 0; 
                     sprintf(text, "Incorrect Transaction Code - %s", tcode);
                     message("Incorrect Transaction Code", 1);
                     break;
                   }
	}
	return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Header Record
 *-------------------------------------------------------------------------*/
process_header()
{
	ob_xmit_header *p;
	long now;
	char New[9];
  
	p = &buf.XmitHeader; 
  
	strncpy(New, (char *)&buf, 8);
	New[8]=0;

        if (strcmp(New, "1000001O") != 0)
           {
             message("Xmit Header Invalid", 1);
           }

	now = time(0);

	fprintf(pd, "Order Input At %24.24s\n\n", ctime(&now));
	fprintf(pd, "%-16s %s\n",        "File:  ", id_name);
	fprintf(pd, "%-16s %3.3s\n",     "Center:", p->dist_center);
	fprintf(pd, "%-16s %5.5s\n",     "Number:", p->xmit_number);
	fprintf(pd, "%-16s %4.4s-", "Date:",    &p->datetime[0]);
	fprintf(pd, "%2.2s-",&p->datetime[4]);
	fprintf(pd, "%2.2s ",&p->datetime[6]);
	fprintf(pd, " %2.2s:",&p->datetime[8]);
	fprintf(pd, "%2.2s:",&p->datetime[10]);
	fprintf(pd, "%2.2s\n\n",&p->datetime[12]);

        /* Save Distribution Center Code from Xmit Header */
        strncpy(szDistCent, p->dist_center, DC_CODE_SIZE - 1);
        szDistCent[DC_CODE_SIZE - 1] = 0;
  
	record_count = 1;
  
	return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Order Header
 *-------------------------------------------------------------------------*/
process_order()
{
	ob_order_header *h;
	char pri, text[80];
  
	h = &buf.OrderHeader;

        if (!record_count)
           {
             message("Xmit Header Missing", 1);
           }
        else
           {
             record_count += 1;
           }

	if (cvrt(h->line_number, sizeof(h->line_number)) != record_count)
	{
		sprintf(text, "Record Count Error - Expecting Record %d", 
							record_count);
		message(text, 1);
	}

	OrderCustProcess(h); 
	order_count += 1;
	return 0;
}

int OrderCustProcess(char *h)
{
	int iReturnStatus = FALSE;
	int iDeleteOk = TRUE;
	long cust_order_nbr = 0;
	char text[85];

	ORDER_CUST_STRUCT pOrder;

        memset(&pOrder, 0x0, sizeof(pOrder));

	lLastOrderNbr = 0;
	iOrderLoad(h, &pOrder);

	iBeginTransaction();

	iInTransaction = TRUE;

/*
fprintf(stderr, "lCapsOrderNo=%d\n", pOrder.caps_order_no);
fprintf(stderr, "lPickLineNo=%d\n", pOrder.pickline_no);
*/

	if(iOrderFind(&pOrder)) // && iOrderGet(&pOrder))
        {
		// Ravi iDeleteOk = iOrderCustDelete(&pOrder);
		iOrderCustDelete(&pOrder);
                fprintf(pd, "Deleted on = %d, pl = %d\n",
                        pOrder.caps_order_no, pOrder.pickline_no);
                fprintf(stderr, "Deleted on = %d, pl = %d\n",
                        pOrder.caps_order_no, pOrder.pickline_no);
                fflush(pd);

        }

        if (iDeleteOk)
           {
             iCommitTransaction();
             iBeginTransaction();
             iOrderLoad(h, &pOrder);
             iReturnStatus = iOrderInsert(&pOrder); 
	     //fprintf(stderr, "iReturnStatus = %d\n", iReturnStatus);
           }
        else
           {
             iRollbackTransaction();
             sprintf(text, 
                     "%s - Order No %d, Pickline = %d\n", 
                     "Delete Record Error",
                     pOrder.caps_order_no,
                     pOrder.pickline_no);
             message(text, 1);
           }

        for (k = 0; k < TNX_EXIT; k++)
            {
              if (iReturnStatus)
                 {
                   iCommitTransaction();
                   break;
                 }
              else
                 {
                   iRollbackTransaction();

                   if (k == (TNX_EXIT - 1))
                      {
                        sprintf(text,
                                "%s - Order No %d, Pickline = %d\n",
                                "Insert Record Error",
                                pOrder.caps_order_no,
                                pOrder.pickline_no);
                        message(text, 1);
                      }

                   iBeginTransaction();
                   iReturnStatus = iOrderInsert(&pOrder);
                 }
            }

	iInTransaction = FALSE;

        if (iOrderFind(&pOrder)) 
           {
             //iOrderGet(&pOrder);

             lLastOrderNbr = pOrder.cust_order_nbr;
           }
	else
		fprintf(stderr, "IOrderFind ELSE\n");

   	return(iReturnStatus);
}

int iOrderLoad(char *buf, ORDER_CUST_STRUCT *pOrder)
{
	ob_order_header *pDownload;
	char	szBuffer[85];

	pDownload = (ob_order_header *)buf;
/*
fprintf(stderr, "lLineNo=%s\n", pDownload->line_number);
fprintf(stderr, "lCapsOrderNo=%s\n", pDownload->caps_order);
fprintf(stderr, "lPickLineNo=%s\n", pDownload->pickline);
fprintf(stderr, "pszStoreNo=%s\n", pDownload->store_id);
fprintf(stderr, "pszOrderConstant=%s\n", pDownload->order_constant);
fprintf(stderr, "pszGroupCode=%s\n", pDownload->group);
fprintf(stderr, "planned date=%s\n", pDownload->planned_pick_date);
fprintf(stderr, "lStartBox=%s\n", pDownload->start_box_number);
fprintf(stderr, "lEndBox=%s\n", pDownload->end_box_number);
*/


fprintf(stderr, "planned date=%s\n", pDownload->planned_pick_date);
        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->line_number, 
               sizeof(pDownload->line_number));
        StrClip(szBuffer);
        pOrder->line_no = atol(szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->caps_order, 
               sizeof(pDownload->caps_order));
        StrClip(szBuffer);
        pOrder->caps_order_no = atol(szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->pickline, 
               sizeof(pDownload->pickline));
        StrClip(szBuffer);
        pOrder->pickline_no = atol(szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->start_box_number,
               sizeof(pDownload->start_box_number));
        StrClip(szBuffer);
        pOrder->start_box = atol(szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->end_box_number,
               sizeof(pDownload->end_box_number));
        StrClip(szBuffer);
        pOrder->end_box = atol(szBuffer);

        pOrder->order_status   = ORDER_LOADED;
        pOrder->xmit_status    = XMIT_WAITING;
        //pOrder->cust_order_nbr = 0L;
        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->store_id, 
               sizeof(pDownload->store_id) );
        StrClip(szBuffer);

        strcpy(pOrder->store_no, szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, "***", 3);
        strcpy(pOrder->dc_code, szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->order_constant,
               sizeof(pDownload->order_constant));
        strcpy(pOrder->ord_constant, szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->group, 
               sizeof(pDownload->group));
        StrClip(szBuffer);
        strcpy(pOrder->group_code, szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->planned_pick_date, 
               sizeof(pDownload->planned_pick_date));
        StrClip(szBuffer);
        sprintf(pOrder->planned_pick_date, "%4.4s%2.2s%2.2s", 
                                           &szBuffer[0],
                                           &szBuffer[4],
                                           &szBuffer[6]);


fprintf(stderr, "planned date=%s\n", pOrder->planned_pick_date);
/*
fprintf(stderr, "lLineNo=%d\n", pOrder->line_no);
fprintf(stderr, "lCapsOrderNo=%d\n", pOrder->caps_order_no);
fprintf(stderr, "lPickLineNo=%d\n", pOrder->pickline_no);
fprintf(stderr, "lStartBox=%d\n", pOrder->start_box);
fprintf(stderr, "lEndBox=%d\n", pOrder->end_box);
fprintf(stderr, "order_status=%d\n", pOrder->order_status);
fprintf(stderr, "xmit_status=%d\n", pOrder->xmit_status);
fprintf(stderr, "pszStoreNo=%s\n", pOrder->store_no);
fprintf(stderr, "pszOrderConstant=%s\n", pOrder->ord_constant);
fprintf(stderr, "pszDcCode=%s\n", pOrder->dc_code);
fprintf(stderr, "pszGroupCode=%s\n", pOrder->group_code);
fprintf(stderr, "planned date=%s\n", pOrder->planned_pick_date);
*/

        return (TRUE);
}

/*-------------------------------------------------------------------------*
 *  Process Pick
 *-------------------------------------------------------------------------*/
process_pick()
{
	ob_pick_item *p;
	char text[80];
  
	p = &buf.PickItem;
  
        if (!record_count)
           {
             message("Xmit Header Missing", 1);
           }
        else
           {
             record_count += 1;
           }
  
        if (cvrt(p->line_number, sizeof(p->line_number)) != record_count)
           {
             sprintf(text,
                     "Record Count Error - Expecting Record %d", 
                     record_count);
             message(text, 1);
           }

        OrderCustItemProcess(p);

        pick_count    += 1;
        sum_ordered   += cvrt(p->ordered_quantity, 5);
        sum_allocated += cvrt(p->allocated_quantity, 5);
        sum_pick      += cvrt(p->pick_quantity, 4);

	return 0;
}

int OrderCustItemProcess(char *p)
{
        int iReturnStatus = FALSE;
        char text[80];

        ORDER_CUST_ITEMS_STRUCT pOrderItem;

        iOrderItemLoad(p, &pOrderItem);

fprintf(stderr, "sku_no=%s\n",pOrderItem.sku_no);
fprintf(stderr, "pick_location=%s\n",pOrderItem.pick_location);
fprintf(stderr, "descr=%s\n",pOrderItem.descr);
fprintf(stderr, "work_code=%s\n",pOrderItem.work_code);
fprintf(stderr, "merch_type=%s\n",pOrderItem.merch_type);
fprintf(stderr, "pick_datetime=%s\n",pOrderItem.pick_datetime);

fprintf(stderr, "ordered_qty=%d\n",pOrderItem.ordered_qty);
fprintf(stderr, "ratio=%d\n",pOrderItem.ratio);
fprintf(stderr, "assign_tote_id=%d\n",pOrderItem.assign_tote_id);
fprintf(stderr, "picker_id=%d\n",pOrderItem.picker_id);


        iBeginTransaction();

        iInTransaction = TRUE;

        iReturnStatus = iOrderItemInsert(&pOrderItem);  

        for (k = 0; k < TNX_EXIT; k++)
            {
              if (iReturnStatus)
                 {
                   iCommitTransaction();
                   break;
                 }
              else
                 {
                   iRollbackTransaction();

                   if (k == (TNX_EXIT - 1))
                      {
                        sprintf(text,
                                "%s - Sku No %s, WMS Assignment = %s\n",
                                "Insert Record Error",
                                pOrderItem.sku_no,
                                pOrderItem.work_code);
                        message(text, 1);
                      }

                   iBeginTransaction();
                   iReturnStatus = iOrderItemInsert(&pOrderItem);
                 }
            }

        iInTransaction = FALSE;

// Ravi
	exit(0);
        return(iReturnStatus);
}

int iOrderItemLoad(char *buf, ORDER_CUST_ITEMS_STRUCT *pOrderItem)
{
        ob_pick_item *pDownload;
        char szBuffer[85];

        pDownload = (ob_pick_item *)buf;

        pOrderItem->cust_order_nbr = lLastOrderNbr;

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->pick_slot, 
               sizeof(pDownload->pick_slot));
        StrClip(szBuffer);
        strcpy(pOrderItem->pick_location, szBuffer);

        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->pick_sku, 
               sizeof(pDownload->pick_sku));
        StrClip(szBuffer);
        strcpy(pOrderItem->sku_no, szBuffer);



        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->pick_desc,
               sizeof(pDownload->pick_desc));
        StrClip(szBuffer);
        strcpy(pOrderItem->descr, szBuffer);



        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->pick_quantity, 
               sizeof(pDownload->pick_quantity));
        StrClip(szBuffer);
        pOrderItem->ordered_qty = atol(szBuffer);



        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->allocated_ratio, 
               sizeof(pDownload->allocated_ratio));
        StrClip(szBuffer);
        pOrderItem->ratio = atol(szBuffer);



        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->tote_id,
               sizeof(pDownload->tote_id));
        StrClip(szBuffer);
        pOrderItem->assign_tote_id = atol(szBuffer);



        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->work_code,
               sizeof(pDownload->work_code));
        StrClip(szBuffer);
        strcpy(pOrderItem->work_code, szBuffer);
fprintf(stderr, "buffer=%s   work code=%s len = %d\n", 
	szBuffer, pOrderItem->work_code, strlen(pOrderItem->work_code));


        memset(szBuffer, 0x0, sizeof(szBuffer));
        memcpy(szBuffer, pDownload->merch_type, 
               sizeof(pDownload->merch_type));
        StrClip(szBuffer);
        strcpy(pOrderItem->merch_type, szBuffer);



        pOrderItem->picked_qty       = 0L;
        pOrderItem->actual_tote_id   = 0L;
        pOrderItem->picker_id        = 0L;
        pOrderItem->pick_datetime[0] = '\0';
        //pOrderItem->cust_item_nbr    = 0L;

	return (TRUE);
}
/*-------------------------------------------------------------------------*
 *  Process Trailer Record
 *-------------------------------------------------------------------------*/
process_trailer()
{
	ob_xmit_trailer *t;
	long value;
	char text[80];
  
	t = &buf.XmitTrailer;

        if (!record_count)
           {
             message("Xmit Header Missing", 1);
           }
        else
           {
             record_count += 1;
           }
  
	fprintf(pd, "%-16s %-5d\n", "Order Count:",     order_count);
	fprintf(pd, "%-16s %-5d\n", "Item Count:",      pick_count);
	fprintf(pd, "%-16s %-5d\n", "Total Ordered:",   sum_ordered);
	fprintf(pd, "%-16s %-5d\n", "Total Allocated:", sum_allocated);
	fprintf(pd, "%-16s %-5d\n", "Total To Pick:",   sum_pick);
  
	fprintf(stderr, "%-16s %-5d\n", "Order Count:",     order_count);
	fprintf(stderr, "%-16s %-5d\n", "Item Count:",      pick_count);
	fprintf(stderr, "%-16s %-5d\n", "Total Ordered:",   sum_ordered);
	fprintf(stderr, "%-16s %-5d\n", "Total Allocated:", sum_allocated);
	fprintf(stderr, "%-16s %-5d\n", "Total To Pick:",   sum_pick);
        if (cvrt(t->line_number, sizeof(t->line_number)) != record_count)
           {
             sprintf(text, 
                     "Record Count Error - Expecting Record %d", 
                     record_count);
             message(text, 1);
           }

	value = cvrt(t->order_count, sizeof(t->order_count));

	if (value != order_count)
	{
		sprintf(text, "Bad Order Count - Expecting %d", value);
		message(text, 1);
	}
	value = cvrt(t->pick_count, sizeof(t->pick_count));

	if (value != pick_count)
	{
		sprintf(text, "Bad Pick Item Count - Expecting %d", value);
		message(text, 1);
	}
	value = cvrt(t->sum_ordered, sizeof(t->sum_ordered));

	if (value != sum_ordered)
	{
		sprintf(text, "Bad Ordered Hash - Expecting %d", value);
		message(text, 1);
	}
	value = cvrt(t->sum_allocated, sizeof(t->sum_allocated));

	if (value != sum_allocated)
	{
		sprintf(text, "Bad Allocated Hash Total - Expecting %d", value);
		message(text, 1);
	}
	value = cvrt(t->sum_pick, sizeof(t->sum_pick));

	if (value != sum_pick)
	{
		sprintf(text, "Bad Pick Hash Total - Expecting %d", value);
		message(text, 1);
	}
	return 0;
}
/*-------------------------------------------------------------------------*
 *  Error Message Display
 *-------------------------------------------------------------------------*/
message(char *p, long flag)
{
	fprintf(pd, "%s\n", p); 
	printf("%s\n", p);

	if (flag) 
	{
		L_TopFlag = 1;
		leave(1);
	}
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
/*-------------------------------------------------------------------------*
 *  Program Exit
 *-------------------------------------------------------------------------*/
leave(long x)
{
	char command[128];
  
	if (x) 
		fprintf(pd, "\n\n*** Input Rejected On Errors\n");
  
	fclose(id);
	fclose(pd);

	if(L_TopFlag == 1)
		iCloseDatabase();

	exit(x);
}

static Usage(void)
{
   static const char *const cTbl[] =
   {
     "Usage: trans_in_aps [Mandatory]",
     "",
     "Mandatory :",
     " <database> <filename> <[A/M]> "
   };
   size_t n;

   fprintf(stderr, "\n");
   for (n = 0; n < sizeof(cTbl)/sizeof(cTbl[0]); n++ )
       {
         fprintf(stderr, "%s\n", cTbl[n]);
       }

   exit(-1);
}

/* end of trans_in_aps.c */
