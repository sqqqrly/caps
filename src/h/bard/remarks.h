/*
 *  remarks.h
 *
 *  Record Structure For remarks
 */

typedef struct
{
   short          r_rmks_pl;
   long           r_rmks_on;
   unsigned char  r_rmks_text[8];

} remarks_item;

#define remarks_size 16

/* end of remarks.h */
