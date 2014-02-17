/*-------------------------------------------------------------------------*
 *  box_packing_list.h
 *
 *  For Kmart.
 *
 *  02/15/95   tjt  Add split pick indication
 *-------------------------------------------------------------------------*/

#define  LENGTH      55                   /* length of page in lines         */
#define  WIDTH       80                   /* width of page in columns        */
#define  PICK_COL     1                   /* start column for picks          */
#define  PICK_ROW    14                   /* line for first pick on page     */

long  PICK_LINES = 1;                     /* lines for EACH pick             */
long  REPEAT     = 40;                    /* number of picks on each page    */

char  HOF = 0x0c;
long    flag;

before_request()
{
  picker.p_picker_id = of_rec->of_picker;
   
  if (B_read(pkr_fd, &picker, NOLOCK))
  {
    picker.p_picker_id = 0;
    strcpy(picker.p_last_name, " ");
  }
  flag = 0;
}
before_first_page()
{
  set_size(1);
}
before_each_page()
{
  if (flag)
  {
    aplace(&HOF, 1, 1, 1);                /* force form feed too !!!         */
  }
  flag = 1;
  set_size(1);

  aplace("K   M a r t   -   P a c k i n g   L i s t", 41, 3, 20);
  aplace(sp->sp_company,strlen(sp->sp_company),4,20);
  aplace("PICKLINE:", 9, 7, 1);
  aplace("ORDER:", 6, 7, 15);
  aplace("BOX:", 4, 7, 61);
  nplace(of->of_hdr.of_pl, "99", 7, 11);
  nplace(of->of_hdr.of_on, "99999", 7, 22);
  nplace(qi.paper_box_number, "999999", 7, 66);
   
  aplace("PICKER: ", 8, 8, 1);
  nplace(picker.p_picker_id, "99999", 8, 11);
  picker.p_last_name[15] = 0;
  aplace(picker.p_last_name, strlen(picker.p_last_name), 8, 22);

  aplace("Load No:", 8, 10, 1);
  aplace("Schedule No:", 12, 10, 27);
  aplace("Batch No:", 9, 10, 58);
  aplace(rmks->r_load_no, 4, 10, 11);
  aplace(rmks->r_schedule_no, 6, 10, 40);
  aplace(rmks->r_batch_no, 3, 10, 68);   /* used to be 4, 10, 68 11/02/92 CWS*/
   
  aplace(
  "ModSlot      SKU              Description         Pkg   Ordered  Picked",
  71, 12, 1);
   
  aplace(
  "------- ------------- ------------------------- ------- ------- -------",
  71, 13, 1);

}
each_pick()
{

/* picks are relative to line 1 using PICK_ROW as base*/

  aplace(pkm_rec.p_stkloc, 6,  1, 1);
  aplace(sku_rec.p_pfsku, 13, 1, 9);
  aplace(sku_rec.p_descr, 25, 1, 23);
  nplace(sku_rec.p_ipqty, "ZZZ,ZZ9", 1, 49);
  nplace(op_rec->pi_ordered, "ZZZ,ZZ9", 1, 57);
  nplace(op_rec->pi_picked,  "ZZZ,ZZ9", 1, 65);
  if (pkm_rec.p_piflag == 'y') aplace("*", 1, 1, 73);
  if (op_rec->pi_flags & SPLIT_PICK) aplace("SPLIT", 5, 1, 73);   /* F021595 */
}
after_each_page()
{
  if(page < last_page)   aplace("CONTINUED ...", 13, 1, 1);
}
after_last_page()
{
  return;
}
/* end of packing_list.h   */
