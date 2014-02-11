/*
 *  picks.h
 *
 *  Record Structure For picks
 */

typedef struct
{
   short          p_pi_pl;
   long           p_pi_on;
   short          p_pi_module;
   short          p_pi_zone;
   short          p_pi_ordered;
   short          p_pi_picked;
   short          p_pi_flags;
   long           p_pi_datetime;
   long           p_pi_box;
   unsigned char  p_pi_sku[15];
   unsigned char  p_pi_ptext[32];
   unsigned char  p_pi_lot[15];

} picks_item;

#define picks_size 90

/* end of picks.h */
