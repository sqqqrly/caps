/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Pending transction strutures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/16/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
#ifndef PXF_STRUCTS_H
#define PXF_STRUCT_H

static char pxf_structs_h[] = "%Z% %M% %I% (%G% - %U%)";

struct pPM_data
{
  char pxPM[5];
  char pxbay_num[2];
};

struct pauto_data
{
  char pxauto_SKU[15];
  char pxauto_PM[5];
  char pxauto_plidx[1];
  char pxauto_skudescr[25];
};

struct pxf_data
{
  char pxdts[12];
  char pxcode;
  char pxoprname[8];
  char dxflag;

  union ppmorpsku
  {
    char pxSKU[15];
    struct pPM_data pPM;
    struct pauto_data pAUTO;
  }pskuppm;

  long px_file_pos;                       /* file position of record         */
};
#endif

/* end of pxf_structs.h */
