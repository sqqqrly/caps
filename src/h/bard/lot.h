/*
 *  lot.h
 *
 *  Record Structure For lot
 */

typedef struct
{
   long           l_lot_time;
   short          l_lot_pl;
   unsigned char  l_lot_sku[15];
   unsigned char  l_lot_number[15];

} lot_item;

#define lot_size 36

/* end of lot.h */
