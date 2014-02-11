#define GTE
/*-------------------------------------------------------------------------*
 *  Custom Code:          GTE - special fields and sizes  F060698
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    CAPS global type definitions
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/01/93    |  tjt  Original implementation.
 * 04/16/96    |  tjt  Add stock location defines.
 * 06/05/98    |  tjt  Add BoxNoLength for scanned box numbers.
 * 10/08/01    |  aha  Modified for Eckerd's Tote Integrity. Uses #define GTE.
 *-------------------------------------------------------------------------*/
#ifndef __GLOBAL_TYPESH__
#define __GLOBAL_TYPESH__

static char global_types_h[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"
#include "oracle_defines.h"

/*-------------------------------------------------------------------------*
 *  System Limits On Numeric Values ( 1 .. Max )
 
 *-------------------------------------------------------------------------*/

#define  PortMax              32
#define  PicklineMax          32
#define  SegmentMax           64
#define  ZoneMax              999
#define  BayMax               2000
#define  OrderMax             9999999
#define  ProductMax           30000
#define  ModuleMax            30000
#define  QuantityMax          9999
#define  BoxMax               99999999

/*-------------------------------------------------------------------------*
 *  Length of Character or Display Fields
 *-------------------------------------------------------------------------*/

#define  PicklineLength       2
#define  PicklineNameLength   8
#define  ConfigNameLength     30
#define  ZoneLength           3
#define  BayLength            4
#define  OrderLength          7
#define  ProductLength        5
#define  ModuleLength         5
#define  QuantityLength       4
#define  SkuLength            15
#define  StklocLength         6
#define  CustomerNoLength     15
#define  GroupLength          6

#ifdef GTE
#define  ConLength            15         /* set to max  */
#define  BoxNoLength          8          /* from tote scan */

#else
#define  ConLength            10
#define  BoxNoLength          4
#endif

#define  BoxLength            6
#define  PickTextLength       32
#define  RemarksLength        4000
#define  LotLength            15
#define  TranLength           128
#define  DateLength           8

/*-------------------------------------------------------------------------*
 *  Defined Data Types
 *-------------------------------------------------------------------------*/

typedef  unsigned short TPort;
typedef  short          TPickline;
typedef  unsigned short TSegment;
typedef  unsigned short TZone;
typedef  unsigned short TBay;
typedef  long           TOrder;
typedef  short          TProduct;
typedef  short          TModule;
typedef  long           TBoxNumber;
typedef  unsigned short TController;
typedef  short          TQuantity;
typedef  char           TSku[SkuLength];
typedef  char           TStkloc[StklocLength];
typedef  char           TGroup[GroupLength];
typedef  char           TCon[CustomerNoLength];
typedef  char           TRemarks[RemarksLength];
typedef  char           TPickText[PickTextLength];
typedef  char           TLot[LotLength];

#endif

/* end of global_types.h */

