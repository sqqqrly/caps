/*
 *  ship_label.h
 *
 *  Record Structure For ship_label
 */

typedef struct
{
   long           s_sl_ref;
   long           s_sl_time;
   short          s_sl_copies;
   short          s_sl_pl;
   long           s_sl_on;
   short          s_sl_zone;

} ship_label_item;

#define ship_label_size 18

/* end of ship_label.h */
