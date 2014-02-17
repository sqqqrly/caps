/*-------------------------------------------------------------------------*
 *  box_packing_list.h
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 *  noxell - box_packing_defines
 *-------------------------------------------------------------------------*/

#define LENGTH          60                /* length of page in lines         */
#define WIDTH           80                /* width of page in columns        */
#define PICK_COL        1                 /* start column for picks          */
#define PICK_ROW        19                /* line for first pick on page     */
#define OFFLINE_LINES   26                /* number of offline lines         */
#define OFFLINE_ROW     18                /* start line for offline picks    */
#define FREIGHT_ITEMS   12                /* number of freight items         */
#define FREIGHT_ROW     47                /* start line for freight data     */
#define TOTALS_ROW      59                /* line for totals row             */

long    PICK_LINES =    1;                /* lines for EACH pick             */
long    REPEAT =        37;               /* number of picks on each page    */

char    HOF = 0x0c;
char    work[12];
int     i, row;
long    flag;
long    pages;
short   last_box;

before_request()
{
  flag = 0;

  if(rmks->print_cus_descr == '0')
  {
    REPEAT = 37;
    PICK_LINES = 1;
  }
  else
  {
    REPEAT = 18;
    PICK_LINES = 2;
  }
}
before_first_page()
{
  set_size(1);
}
before_each_page()
{
  if(box.b_box_last == 0x20)              /* this is NOT the last box        */
  {
    pages = last_page;
    last_box = 0;
  }
  else                                    /* this IS the last box            */
  {
    pages = last_page + 1;
    last_box = 1;
  }

  if (flag)
  {
    aplace(&HOF, 1, 1, 1);                /* force form feed too !!!         */
  }
  flag = 1;
  aplace("BOX PACKING LIST", 16, 2, 26);
  aplace("NOXELL CORPORATION", 18, 2, 55);

   /* box-id info */
  aplace("BOX", 3, 7, 45);
  nplace(qi.paper_box_number, "999", 7, 49);

   /* page number info */
  sprintf(work, "%d of %d  ", page, pages);/* spaces in format required!     */
  aplace(work, 8, 7, 64);

  aplace(rmks->address1, 20,  7, 1);
  aplace(rmks->address2, 20,  8, 1);
  aplace(rmks->address3, 20,  9, 1);
  aplace(rmks->address4, 20, 10, 1);
  aplace(rmks->zipcode, 9, 10, 22);
  aplace(rmks->cust_number, 9, 8, 31);
  nplace(cvrt(rmks->full_case,5), "ZZZ Z9", 10, 55);

  aplace("R-", 2, 9, 63);
  aplace(&rmks->repack_cnt_sz1, 1, 9, 65);
  nplace(cvrt(rmks->repack_cnt_cnt1,3), "ZZ9", 10, 63);

  aplace("R-", 2, 9, 68);
  aplace(&rmks->repack_cnt_sz2, 1, 9, 70);
  nplace(cvrt(rmks->repack_cnt_cnt2, 3),"ZZ9", 10, 68);

  aplace(udate, 8, 13, 1);
  aplace(rmks->cust_po_number, 18, 13, 11);
  aplace(of_rec->of_con, 8, 13, 32);
  aplace(rmks->carrier_name, 20, 15, 1);
  aplace(rmks->tracer_number, 9, 15, 35);
  aplace(rmks->warehouse_desc, 20, 15, 47);
}
each_pick()
{
/* The row values specified below are relative to PICK_ROW                   */

  if ( pick->of_short )                   /* shorted                         */
  {
    aplace("* SHORT *", 9, 1, 1);
    nplace(pick->of_quan - pick->of_short, "ZZZZZ", 1, 12);
    aplace(sku_rec.p_pfsku, 5, 1, 18);    /* 1st five digits of sku          */
    aplace("*****", 5, 1, 28);            /* full case column                */
    aplace("*****", 5, 1, 34);            /* dozens column                   */
    aplace("**", 2, 1, 40);               /* units column                    */
    aplace("********", 8, 1, 43);         /* weight column                   */
    aplace(ptext->sku_class, 2, 1, 52);
    aplace(sku_rec.p_descr, 20, 1, 55);   /* 1st twenty digits of descr      */
    if(rmks->print_cus_descr == '1')
    {
      aplace(ptext->cust_sku_descr, 20, 2, 55);
    }
  }
  else
  {
    nplace(pick->of_quan, "ZZZZZ", 1, 12);
    aplace(sku_rec.p_pfsku, 5, 1, 18);    /* 1st five digits of sku          */
    aplace(ptext->sku_full_cases, 5, 1, 28);
    aplace(ptext->sku_doz, 5, 1, 34);
    aplace(ptext->sku_units, 2, 1, 40);
    aplace(ptext->sku_weight, 8, 1, 43);
    aplace(ptext->sku_class, 2, 1, 52);
    aplace(sku_rec.p_descr, 20, 1, 55);   /* 1st twenty digits of descr      */
    if(rmks->print_cus_descr == '1')
    {
      aplace(ptext->cust_sku_descr, 20, 2, 55);
    }
  }
}
after_each_page()
{
  set_base(57,12);
  if(page < last_page)   aplace("CONTINUED .......", 17, 1, 1);
  else if(last_box)
  aplace("FULL CASE, FREIGHT DATA, AND TOTALS ON NEXT PAGE...", 51, 1, 1);
}
after_last_page()
{
  if(!last_box) return;                   /* Only do this if its the last box*/

  print_all();                            /* print what we have              */
  set_base(1,1);                          /* force topcorner                 */
  page += 1;                              /* increment page                  */

  before_each_page();                     /* reprint header                  */

  if(rmks->is_ups == '0')                 /* off-line case picks             */
  {
    aplace("************ SEE BILL OF LADING FOR FULL CASE ITEMS *************",
      64, OFFLINE_ROW, 12);
  }
  else if(rmks->is_ups == '1' &&
    strncmp(&rmks->offline_case[0][5], "     ", 5) != 0)/* have data         */
  {
    aplace("******************* OFF-LINE FULL CASE PICKS *******************",
      64, OFFLINE_ROW, 12);

    for( i = 0, row = OFFLINE_ROW + 2; i < OFFLINE_LINES; i++, row++)
    {
      aplace(&rmks->offline_case[i][0],  5, row, 12);/* = shelf pack       */
      aplace(&rmks->offline_case[i][5],  5, row, 18);/* = 1st 5 of sku     */  
      aplace(&rmks->offline_case[i][11], 5, row, 28);/* = full cases       */  
      aplace(&rmks->offline_case[i][16], 5, row, 34);/* = dozens           */  
      aplace(&rmks->offline_case[i][21], 2, row, 40);/* = units            */  
      aplace(&rmks->offline_case[i][23], 8, row, 43);/* = weight           */  
      aplace(&rmks->offline_case[i][31], 2, row, 52);/* = class            */  
      aplace(&rmks->offline_case[i][33],20, row, 55);/* = descr            */  
    }                                       /* end for loop                */  
  }                                         /* end if                      */  

/* freight data */

  aplace("------------------------------------------------------------",
    60, FREIGHT_ROW, 12);

  aplace("       |  FR-RTE  |        ||       |  FR-RTE   |        ",
    56,FREIGHT_ROW + 1,14);

  aplace(" CASES |  CLASS   | WEIGHT || CASES |  CLASS    | WEIGHT ",
    56,FREIGHT_ROW + 2,14);
  
  aplace("============================================================",
    60, FREIGHT_ROW + 3, 12);

   /* left hand column of freight_data  - freight items 1 - 6 */

  for(i = 0, row = FREIGHT_ROW + 4; i < FREIGHT_ITEMS / 2; i++, row++)
  {
    if (strncmp(&rmks->freight_data[i][0], "     ", 5) == 0) break;

        /* break on no cases - ie. no more freight_data */

    aplace(&rmks->freight_data[i][0], 5, row, 15);/* Freight Cases           */
    aplace(&rmks->freight_data[i][5], 2, row, 24);/* Freight Class           */
    aplace("-", 1, row, 26);                      /* Literal                 */
    aplace(&rmks->freight_data[i][7], 5, row, 27);/* Freight Rate            */
    aplace(&rmks->freight_data[i][12],5, row, 34);/* Freight weight          */

  }
   /* right hand column of freight_data  - freight items 7 - 12 */

  for(i = 6, row = FREIGHT_ROW + 4; i < FREIGHT_ITEMS; i++, row++)
  {
    if (strncmp(&rmks->freight_data[i][0], "     ", 5) == 0) break;

        /* break on no cases - ie no more freight_data */

    aplace(&rmks->freight_data[i][0], 5, row, 44);/* Freight Cases           */
    aplace(&rmks->freight_data[i][5], 2, row, 53);/* Freight Class           */
    aplace("-", 1, row, 55);                /* Literal                       */
    aplace(&rmks->freight_data[i][7], 5, row, 56);/* Freight Rate            */
    aplace(&rmks->freight_data[i][12],5, row, 64);/* Freight weight          */
  }   
  aplace("(N.M.F.C. DESCRIPTIONS ARE ON THE BACK OF THE BILL OF LADING)",
    61, 58, 1);
  aplace("CASES:",6,TOTALS_ROW,9);
  aplace(rmks->cases, 6, TOTALS_ROW, 16);
  aplace("WEIGHT:",7,TOTALS_ROW,31);
  aplace(rmks->weight,7,TOTALS_ROW,39);
  aplace("CUBE:", 5, TOTALS_ROW, 51);
  aplace(rmks->cube, 8, TOTALS_ROW, 57);
}


/* end of box_packing_list.h   */
