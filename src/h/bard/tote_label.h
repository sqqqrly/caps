/*
 *  tote_label.h
 *
 *  Record Structure For tote_label
 */

typedef struct
{
   long           t_tl_ref;
   long           t_tl_time;
   short          t_tl_copies;
   short          t_tl_pl;
   long           t_tl_on;
   short          t_tl_zone;

} tote_label_item;

#define tote_label_size 18

/* end of tote_label.h */
