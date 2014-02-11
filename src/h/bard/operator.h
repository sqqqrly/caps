/*
 *  operator.h
 *
 *  Record Structure For operator
 */

typedef struct
{
   unsigned char  o_op_name[9];
   unsigned char  o_op_desc[33];
   unsigned char  o_op_printer[9];
   unsigned char  o_op_pickline[3];
   unsigned char  o_op_level[2];
   unsigned char  o_op_mm[32];
   unsigned char  o_op_ops[32];
   unsigned char  o_op_sys[32];
   unsigned char  o_op_config[32];
   unsigned char  o_op_prod[32];
   unsigned char  o_op_sku[32];
   unsigned char  o_op_label[32];

} operator_item;

#define operator_size 280

/* end of operator.h */
