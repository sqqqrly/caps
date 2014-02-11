/*
 *  pending.h
 *
 *  Record Structure For pending
 */

typedef struct
{
   short          p_pnd_pl;
   long           p_pnd_on;
   unsigned char  p_pnd_group[6];
   unsigned char  p_pnd_con[15];
   short          p_pnd_flags;

} pending_item;

#define pending_size 32

/* end of pending.h */
