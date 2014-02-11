/*
 *  restock_notice.h
 *
 *  Record Structure For restock_notice
 */

typedef struct
{
   long           r_rs_ref;
   long           r_rs_time;
   short          r_rs_pl;
   short          r_rs_mod;
   long           r_rs_number;
   short          r_rs_quantity;

} restock_notice_item;

#define restock_notice_size 18

/* end of restock_notice.h */
