/*
 *  box_packing_list.h
 *
 *  Record Structure For box_packing_list
 */

typedef struct
{
   long           b_bpl_ref;
   long           b_bpl_time;
   short          b_bpl_copies;
   short          b_bpl_pl;
   long           b_bpl_on;
   long           b_bpl_box;
   unsigned char  b_bpl_printer[8];

} box_packing_list_item;

#define box_packing_list_size 28

/* end of box_packing_list.h */
