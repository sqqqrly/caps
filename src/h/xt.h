/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Transaction file definition.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *  05/03/94   |  tjt  Zone Added.
 *  05/04/94   |  tjt  6 digit order number.
 *  06/15/94   |  tjt  Order queued.
 *  12/15/94   |  tjt  Add lot number.
 *  12/15/94   |  tjt  Split SKU and module field.
 *  05/23/95   |  tjt  Add global types.
 *-------------------------------------------------------------------------*/
#ifndef XT_H
#define XT_H

static char xt_h[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "global_types.h"

/*  
 *  Definition of Transaction File
 *
 *    A = Manual Entry            
 *    B = Box Close/Open Event
 *    C = Order Complete
 *    K = Restock SKU
 *    L = Lot Change
 *    O = Orphan Pick
 *    P = Pick Event 
 *    Q = Order Queued In Order Input
 *    R = Order Repicked
 *    S = Picked Short
 *    U = Order Underway
 *    W = Wave Done (No More Orders)
 *    X = Order Canceled
 */

struct trans_item
{
  long xt_ref;                            /* reference number                */
  long xt_time;                           /* transaction time                */
  char xt_group[GroupLength];             /* group code                      */
  char xt_con[CustomerNoLength];          /* customer order number           */
  char xt_on[OrderLength];                /* order number                    */
  char xt_pl[PicklineLength];             /* pickline number                 */
  char xt_code;                           /* transaction code                */
  char xt_sku_mod1[SkuLength];            /* SKU/module number               */
  char xt_stkloc[StklocLength];           /* CAPS stock location             */
  char xt_quan1[QuantityLength];          /* quantity ordered                */
  char xt_quan2[QuantityLength];          /* quantity shipped                */
  char xt_zone[ZoneLength];               /* zone number                     */
  char xt_lot[LotLength];                 /* lot number                      */
  
};

#define xt_box xt_stkloc                  /* same position in record         */

#endif

/* end of xt.h  */
