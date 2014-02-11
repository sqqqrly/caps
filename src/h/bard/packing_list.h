/*
 *  packing_list.h
 *
 *  Record Structure For packing_list
 */

typedef struct
{
   long           p_pl_ref;
   long           p_pl_time;
   short          p_pl_copies;
   short          p_pl_pl;
   long           p_pl_on;
   short          p_pl_zone;

} packing_list_item;

#define packing_list_size 18

/* end of packing_list.h */
