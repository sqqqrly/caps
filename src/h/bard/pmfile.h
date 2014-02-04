/*
 *  pmfile.h
 *
 *  Record Structure For pmfile
 */

typedef struct
{
   long           p_pmodno;
   unsigned char  p_pmsku[15];
   long           p_qty;
   long           p_alloc;
   long           p_restock;
   long           p_rqty;
   long           p_lcap;
   unsigned char  p_stkloc[6];
   unsigned char  p_display[4];
   short          p_plidx;
   unsigned char  p_piflag;
   long           p_cuunits;
   long           p_cmunits;
   long           p_culines;
   long           p_cmlines;
   long           p_curecpt;
   long           p_cmrecpt;
   unsigned char  p_rsflag;
   unsigned char  p_acflag;

} pmfile_item;

#define pmfile_size 78

/* end of pmfile.h */
