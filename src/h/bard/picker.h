/*
 *  picker.h
 *
 *  Record Structure For picker
 */

typedef struct
{
   long           p_picker_id;
   unsigned char  p_last_name[16];
   unsigned char  p_first_name[16];
   unsigned char  p_middle_initial[2];
   short          p_zone;
   short          p_status;
   short          p_underway_orders;
   long           p_start_time;
   long           p_current_time;
   long           p_cur_order_count;
   long           p_cur_lines;
   long           p_cur_units;
   long           p_cur_time;
   long           p_cum_order_count;
   long           p_cum_lines;
   long           p_cum_units;
   long           p_cum_time;

} picker_item;

#define picker_size 84

/* end of picker.h */
