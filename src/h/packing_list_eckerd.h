/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Functions and definitions for use with Eckerd's
 *                  tote_packing_list.c without Tote Integrity. 
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/15/2001  | aha  Created file from packing_list_orlando.h
 *-------------------------------------------------------------------------*/
#ifndef __PACKING_LIST_ECKERDH__
#define __PACKING_LIST_ECKERDH__
static char packing_list_eckerd_h[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/*-------------------------------------------------------------------------*
 * Eckerds - without Tote Integrity
 *--------------------------------------------------------------------------*/

#define LENGTH        66                  /* length of page in lines         */
#define WIDTH         79                  /* width of page in columns        */
#define PICK_COL      1                   /* start column for picks          */
#define PICK_ROW      9                   /* line for first pick on page     */
#define TOTALS_ROW    PICK_ROW + line + 1 /* line for totals row             */
long    PICK_LINES =  1;                  /* lines for EACH pick             */
long    REPEAT =      50;                 /* number of picks on each page    */

long    total_ordered;
long    total_picked;
long    total_count;

before_request()
{
  total_ordered = 0;
  total_picked  = 0;
  total_count   = 0;
}
before_first_page()
{
  set_size(1);
}
before_each_page()
{
  aplace("Eckerd Drug", 11, 1, 1);
  aplace("Orlando Distribution Center", 27, 2, 1);
  aplace("Development & Test - Store Manifest", 35, 3, 1);
  
/*  if (of_rec->of_pl == 3) aplace(" - Refrigerated Items", 21, 3, 31); */

  aplace("Store #",  7, 1, 62);
  aplace(of_rec->of_con, 4, 1, 69);
  aplace("Order #",  7, 2, 62);
  nplace(of_rec->of_on, "999999", 2, 69);
  nplace(of_rec->of_pl, "99", 2, 75);
  aplace(udate, 8, 3, 62);
  aplace("Page", 4, 4, 62);
  nplace(page, "Z9", 4, 67);
  aplace("of", 2, 4, 70);
  nplace(last_page, "Z9", 4, 73);
  
  aplace("List of Items Shipped", 21, 5, 1);
  aplace("(* Short Items)", 15, 5, 26);
  
  aplace("Eckerd Item Number", 18, 7, 1);
  aplace("Description", 11, 7, 21);
  aplace("Qty Ordered", 11, 7, 52); 
  aplace("Qty Shipped", 11, 7, 65);
  
  aplace("------------------", 18, 8, 1);
  aplace("------------------------------", 30, 8, 21);
  aplace("-----------", 11, 8, 52);
  aplace("-----------", 11, 8, 65);

}
each_pick()
{

/* picks are relative to line 1 using PICK_ROW as base*/

  aplace(op_rec->pi_sku, 6, 1, 4);
  aplace(sku_rec.p_descr, 30, 1, 21); 
  nplace(op_rec->pi_ordered, "ZZZ9", 1, 59);
  nplace(op_rec->pi_picked, "ZZZ9", 1, 72);
  
  if (op_rec->pi_ordered > op_rec->pi_picked) aplace("*", 1, 1, 77);

  total_ordered += op_rec->pi_ordered;
  total_picked  += op_rec->pi_picked;
  total_count   += 1;
}
after_each_page()
{
  if(page < last_page)   aplace("CONTINUED .......", 17, TOTALS_ROW, 1);
}
after_last_page()
{
  aplace("Totals:", 7, TOTALS_ROW, 1);
  aplace("-SKU", 4, TOTALS_ROW, 25);
  nplace(total_count,   "ZZZ9", TOTALS_ROW, 21);

  nplace(total_ordered, "ZZZZZ9", TOTALS_ROW, 57);
  nplace(total_picked,  "ZZZZZ9", TOTALS_ROW, 70);
}
before_first_short_page()
{
  set_size(1);
}
before_each_short_page()
{
  aplace("Eckerd Drug", 11, 1, 1);
  aplace("Orlando Distribution Center", 27, 2, 1);
  aplace("Development & Test - Store Manifest", 35, 3, 1);
  
 if (of_rec->of_pl == 3) aplace(" - Refrigerated Items", 21, 3, 31);

  aplace("Store #",  7, 1, 62);
  aplace(of_rec->of_con, 4, 1, 69);
  aplace("Order #",  7, 2, 62);
  nplace(of_rec->of_on, "999999", 2, 69);
  nplace(of_rec->of_pl, "99", 2, 75);
  aplace(udate, 8, 3, 62);
  aplace("Page", 4, 4, 62);
  nplace(page, "Z9", 4, 67);
  aplace("of", 2, 4, 70);
  nplace(last_page, "Z9", 4, 73);
  
  aplace("List of Items Ordered but Short", 31, 5, 1);
  
  aplace("Eckerd Item Number", 18, 7, 1);
  aplace("Description", 11, 7, 21);
  aplace("Qty Ordered", 11, 7, 52); 
  aplace("Qty Shipped", 11, 7, 65);
  
  aplace("------------------", 18, 8, 1);
  aplace("------------------------------", 30, 8, 21);
  aplace("-----------", 11, 8, 52);
  aplace("-----------", 11, 8, 65);

}
each_short_pick()
{

 /*picks are relative to line 1 using PICK_ROW as base */

  aplace(op_rec->pi_sku, 6, 1, 4);
  aplace(sku_rec.p_descr, 30, 1, 21); 
  nplace(op_rec->pi_ordered, "ZZZ9", 1, 59);
  nplace(op_rec->pi_picked, "ZZZ9", 1, 72);

}
after_each_short_page()
{
  if(page < last_short_page)   
    aplace("CONTINUED .......", 17, TOTALS_ROW, 1);
}
after_last_short_page()
{
  aplace("End of Shorts", 13, TOTALS_ROW, 1);
}


#endif
/* end of packing_list_eckerd.h   */
