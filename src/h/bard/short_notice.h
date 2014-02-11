/*
 *  short_notice.h
 *
 *  Record Structure For short_notice
 */

typedef struct
{
   long           s_sh_ref;
   long           s_sh_time;
   short          s_sh_pl;
   long           s_sh_on;
   short          s_sh_mod;
   short          s_sh_ordered;
   short          s_sh_picked;
   short          s_sh_remaining;
   long           s_sh_picker;
   unsigned char  s_sh_split;
   unsigned char  s_sh_con[15];
   unsigned char  s_sh_grp[6];

} short_notice_item;

#define short_notice_size 50

/* end of short_notice.h */
