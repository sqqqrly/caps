/*
 *  picker_order.h
 *
 *  Record Structure For picker_order
 */

typedef struct
{
   long           p_order_number;
   long           p_picker_id;
   short          p_pickline;
   short          p_order_status;
   long           p_start_time;
   long           p_completion_time;
   long           p_picking_time;
   long           p_lines_picked;
   long           p_units_picked;

} picker_order_item;

#define picker_order_size 32

/* end of picker_order.h */
