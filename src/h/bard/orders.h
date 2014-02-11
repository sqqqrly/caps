/*
 *  orders.h
 *
 *  Record Structure For orders
 */

typedef struct
{
   short          o_of_pl;
   long           o_of_on;
   short          o_of_no_picks;
   short          o_of_no_units;
   long           o_of_datetime;
   short          o_of_elapsed;
   long           o_of_picker;
   unsigned char  o_of_pri;
   unsigned char  o_of_status;
   unsigned char  o_of_repick;
   unsigned char  o_of_grp[6];
   unsigned char  o_of_con[15];

} orders_item;

#define orders_size 48

/* end of orders.h */
