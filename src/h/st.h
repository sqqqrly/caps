/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Definition of SKU table data structures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/16/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
#ifndef ST_H
#define ST_H

static char st_h[] = "%Z% %M% %I% (%G% - %U%)";

/*------------------------------------------------------------------------*
 *  SKU Table in Product Order (order by pm_mod in pmfile).
 *  One item for each product in the pmfile.
 *------------------------------------------------------------------------*/

struct st_item
{
  unsigned char st_pl;                    /* pickline of product             */
  unsigned char st_sku[SkuLength];        /* product sku                     */
  unsigned char st_stkloc[StklocLength];  /* stock location                  */
  unsigned char st_mirror;                /* mirror count                    */
  TModule       st_mod;                   /* module number                   */

};

extern struct st_item *sku_lookup();
extern struct st_item *mod_lookup();
extern struct st_item *stkloc_lookup();

#endif

/* end of st.h */
