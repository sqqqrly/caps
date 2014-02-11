/*
 *  prodfile.h
 *
 *  Record Structure For prodfile
 */

typedef struct
{
   unsigned char  p_pfsku[15];
   unsigned char  p_descr[25];
   unsigned char  p_fgroup[5];
   unsigned char  p_um[3];
   short          p_ipqty;
   short          p_cpack;
   unsigned char  p_bsloc[6];
   unsigned char  p_absloc[6];
   unsigned char  p_altid[25];

} prodfile_item;

#define prodfile_size 89

/* end of prodfile.h */
