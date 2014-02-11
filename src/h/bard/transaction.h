/*
 *  transaction.h
 *
 *  Record Structure For transaction
 */

typedef struct
{
   long           t_xt_ref;
   long           t_xt_time;
   unsigned char  t_xt_group[6];
   unsigned char  t_xt_con[15];
   unsigned char  t_xt_on[7];
   unsigned char  t_xt_pl[2];
   unsigned char  t_xt_code;
   unsigned char  t_xt_sku_mod1[15];
   unsigned char  t_xt_stkloc[6];
   unsigned char  t_xt_quan1[4];
   unsigned char  t_xt_quan2[4];
   unsigned char  t_xt_zone[3];
   unsigned char  t_xt_lot[15];

} transaction_item;

#define transaction_size 86

/* end of transaction.h */
