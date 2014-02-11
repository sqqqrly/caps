/*
 *  location.h
 *
 *  Record Structure For location
 */

typedef struct
{
   unsigned char  l_lstkloc[6];
   unsigned char  l_lsku[15];
   long           l_lmod;

} location_item;

#define location_size 28

/* end of location.h */
