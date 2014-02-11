/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Initialization parameter tables.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/21/93   |  tjt  Original implementation.
 *  05/24/94   |  tjt  Fix transaction output to (bnqy).
 *  07/16/94   |  tjt  Add option 'u' on rf_ignore_pick_text.
 *  08/22/94   |  tjt  Fix backorder removed.
 *  08/22/94   |  tjt  Add unit of measure as sp_um_in_pick_text.
 *  09/07/94   |  tjt  Add options u (units) to remaining picks.
 *  06/04/95   |  tjt  Add port and pickline by name.
 *  06/05/95   |  tjt  Add ss refresh parameter.
 *  06/27/95   |  tjt  Add global types.
 *  04/16/96   |  tjt  Add stock location.
 *  05/21/98	|  tjt  Add scanned box open and box close.
 *-------------------------------------------------------------------------*/
#ifndef SS_INIT_H
#define SS_INIT_H

static char ss_init_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "global_types.h"

/*-------------------------------------------------------------------------*
 *
 *  Field Kinds Are:
 *
 *  c = character   (length = 1)
 *  s = string      (length = ?)
 *  b = byte        (length = 1)
 *  w = word        (length = 4)
 *  h = short       (length = 2)
 *  f = space       (length = 1) always set to space;
 *  z = zero        (length = ?) always set to zero.
 *-------------------------------------------------------------------------*/
 
typedef struct init_item
{
  char  init_type;                        /* kind of field                   */
  char  init_refresh;                     /* overlay on mark/restoreplace    */
  short init_length;                      /* length of field                 */
  long  init_min;                         /* minimum value                   */
  long  init_max;                         /* maximum value                   */
  char *init_desc;                        /* pointer to description          */

} Tinit_item;


Tinit_item ss_data[] = {

  {'s', 1, 16, 0, 0, "System Name - 16 bytes + null"},
  {'s', 1, 40, 0, 0, "Company Name - 40 bytes + null"},
  
  {'w', 0, 4,  1,    32, "Maximum Number of Ports      (1 .. 32)"},
  {'w', 0, 4,  1,    32, "Maximum Number of Picklines  (1 .. 32)"},
  {'w', 0, 4,  1,    64, "Maximum Number of Segments   (1 .. 64)"},
  {'w', 0, 4,  1,   999, "Maximum Number of Zones      (1 .. 999)"},
  {'w', 0, 4,  1,  2000, "Maximum Number of Bays       (1 .. 2000)"},
  {'w', 0, 4,  1, 30000, "Maximum Number of Products   (1 .. 30000)"},
  {'w', 0, 4,  1, 30000, "Maximum Number of Modules    (1 .. 30000)"},
  {'w', 0, 4,  1, 30000, "Maximum Number of Lights     (1 .. 30000)"},
  {'w', 0, 4,  1,100000, "Maximum Number of Orders     (1 .. 100000)"},
  {'w', 0, 4,  1, 32767, "Maximum Picks Per Order      (1 .. 32767)"},
  
  {'c', 0, 1,  0, 0, "Full Function                (nsy)"},
  {'c', 0, 1,  0, 0, "Basic Function               (nsy)"},
  {'c', 0, 1,  0, 0, "Total Function               (nys)"},
  {'c', 0, 1,  0, 0, "Multibin Matrix or Carousel  (ny)"},

  {'c', 1, 1,  0, 0, "Box Labels Feature           (nys)"},  /* scanned */
  {'c', 1, 1,  0, 0, "Box Full Enable              (anyx)"}, /* a = adjacent */
  {'c', 1, 1,  0, 0, "Autocasing                   (ny)"}, 

  {'c', 1, 1,  0, 0, "Jump Zone                    (ny)"},

  {'c', 1, 1,  0, 0, "Steering                     (ny)"},
  {'c', 1, 1,  0, 0, "Late Entry                   (ny)"},
  {'c', 1, 1,  0, 0, "Early Exit                   (ny)"},
  {'c', 1, 1,  0, 0, "Master Bay Lamps             (ny)"},

  {'c', 1, 1,  0, 0, "Stkloc In Order Input        (ny)"},
  {'c', 1, 1,  0, 0, "Pickline Zero Input          (nyz)"},
  {'c', 1, 1,  0, 0, "Check Controllers            (ny)"},
  {'c', 1, 1,  0, 0, "Pending Operations           (ny)"},
  
  {'c', 1, 1,  0, 0, "PM Records Without SKU       (ny)"},
  {'c', 1, 1,  0, 0, "UM In Pick Text              (any)"},
  {'b', 1, 1,  0, 255, "Blink Over                 (0 .. 255)"},
  {'c', 1, 1,  0, 0, "Lot Control                  (ny)"},

  {'c', 0, 1,  0, 0, "SKU Support                  (ny)"},
  {'c', 1, 1,  0, 0, "Product Mirroring            (ny)"},
  {'c', 1, 1,  0, 0, "Productivity                 (ny)"},
  {'c', 1, 1,  0, 0, "Label Printing               (ny)"},

  {'c', 1, 1,  0, 0, "Inventory Accum              (ny)"},
  {'c', 1, 1,  0, 0, "Process Restock Notices      (ny)"},
  {'c', 1, 1,  0, 0, "Process Short Notices        (ny)"},
  {'c', 1, 1,  0, 0, "Pickline Zero Order Cmds     (nyc)"},

  {'c', 1, 1,  0, 0, "Pickline Zero Group Cmds     (nyc)"},
  {'c', 1, 1,  0, 0, "Order Input w/o Config       (ny)"},
  {'c', 1, 1,  0, 0, "Commo Orders Input           (nky)"},
  {'c', 1, 1,  0, 0, "Commo Trans Output           (nky)"},

  {'c', 1, 1,  0, 0, "Commo Prodfile               (nky)"},
  {'b', 1, 1,  0,50, "Picker Order Count           (0 .. 50)"},
  {'c', 1, 1,  0, 0, "Delete Empty Boxes           (ny)"},
  {'c', 1, 1,  0, 0, "Zone Status Events           (ny)"},

  {'c', 1, 1,  0, 0, "Late Entry One Button Push   (ny)"},
  {'c', 1, 1,  0, 0, "No Pick One Button Push      (ny)"},
  {'c', 1, 1,  0, 0, "No Pick EE One Button Push   (ny)"},
  {'c', 1, 1,  0, 0, "Pickline Module View         (ny)"},

  {'c', 1, 1,  0, 0, "Remaining Picks              (ny)"},
  {'b', 1, 1,  1,255,"Port Init Timeout            (1 .. 255)"},
  {'b', 1, 1,  1, 10, "Short Counting Ticks         (1 .. 32)"},
  {'b', 1, 1,  30, 255, "Markplace Timeout         (30 .. 255)"},

  {'h', 1, 2, 60, 1000, "Restoreplace Timeout     (60 .. 1000)"},
  {'c', 1, 1,  0, 0, "Order Input Purge            (mnxcwy)"},
  {'c', 0, 1,  0, 0, "Use Customer Order Number    (biny)"},

  {'c', 0, 1,  0, 0, "System In Process Status     (xcimnr)"},
  {'c', 0, 1,  0, 0, "Initialize Status            (ny)"},
  {'c', 0, 1,  0, 0, "Configuration Status         (ny)"},
  {'c', 0, 1,  0, 0, "Running Status               (ny)"},
  
  {'c', 1, 1,  0, 0, "Port By Name                 (ny)"},
  {'c', 1, 1,  0, 0, "Pickline By Name             (ny)"},
  {'c', 0, 1,  0, 0, "Order Selection              (ny)"},
  {'f', 0, 1,  0, 0, "Order Input Mode             (cd)"},

  {'f', 0, 1,  0, 0, "Transaction Output Mode      (cd)"},
  {'c', 0, 1,  0, 0, "Short Printing Enabled       (ny)"},
  {'c', 0, 1,  0, 0, "Restock Printing Enabled     (ny)"},
  {'c', 0, 1,  0, 0, "Box Packing List Mode        (xbcdmz)"},

  {'c', 0, 1,  0, 0, "Tote Label Printing Mode     (xabcemnz)"},
  {'b', 0, 1,  0,255,"Tote Label Ahead Count       (0 .. 255)"},
  {'c', 0, 1,  0, 0, "Shipping Label Printing Mode (xbcemnz)"},
  {'c', 0, 1,  0, 0, "Packing List Printing Mode   (xbcdmz)"},
   
  {'z', 0, 4,  0, 0, "Short Notices Printed"},
  {'z', 0, 4,  0, 0, "Short Notices Enqueued"},
  {'z', 0, 4,  0, 0, "Restock Notices Printed"},
  {'z', 0, 4,  0, 0, "Restock Notices Enqueued"},
  {'z', 0, 4,  0, 0, "Tote Labels Printed"},
  {'z', 0, 4,  0, 0, "Tote Labels Enqueued"},
  {'z', 0, 4,  0, 0, "Ship Labels Printed"},
  {'z', 0, 4,  0, 0, "Ship Labels Enqueued"},
  {'z', 0, 4,  0, 0, "Packing Lists Printed"},
  {'z', 0, 4,  0, 0, "Packing Lists Enqueued"},
  {'z', 0, 4,  0, 0, "Box Packing Lists Printed"},
  {'z', 0, 4,  0, 0, "Box Packing Lists Enqueued"},
  
  {'z', 0, 4,  0, 0, "Product File Log Count"},
  {'z', 0, 4,  0, 0, "Network Buffer Queue Count"},
  
  {'z', 0, 4,  0, 0, "Tote Label Order Last Printed"},
  {'z', 0, 4,  0, 0, "Tote Label Order Last Enqueued"},
  {'z', 0, 4,  0, 0, "Shipping Label Order Last Printed"},
  {'z', 0, 4,  0, 0, "Shipping Label Order Last Enqueued"},
  {'z', 0, 4,  0, 0, "Packing List Order Last Printed"},
  {'z', 0, 4,  0, 0, "Packing List Order Last Enqueued"},

  {'w', 1, 4, 60, 99999999, "Save Segments Interval Seconds"},
  {'w', 1, 4,  0, 99999999, "Order Purge Lag Window Seconds"},

  {'z', 0, 4,  0, 0, "Transaction File Count"},
  {'z', 0, 4,  0, 0, "Transaction File Xmit"},

  {'c', 1, 1,  0, 0, "Diskette Format (hlsd)"},

  {'c', 0, 1,  0, 0, "Transaction Output Flag    (nbqy)"},
  {'c', 0, 1,  0, 0, "Spare                      (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - complete     (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - cancel       (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - underway     (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - repick       (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - manual input (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - short        (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - restock      (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - wave         (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - box close    (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - pick event   (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - order queued (ny)"},
  {'c', 0, 1,  0, 0, "Transaction - lot split    (ny)"},
  {'c', 0, 1,  0, 0, "Spare                      (ny)"},
  
  {0, 0, 0, 0, 0, 0}};


Tinit_item rf_data[] = {

  {'c', 0, 1, 0, 0,                "Record Preface Symbol"},
  {'c', 0, 1, 0, 0,                "Record Terminator Symbol"},
  {'c', 0, 1, 0, 0,                "Field Terminator Symbol"},
  {'c', 0, 1, 0, 0,                "End of File Symbol"},
  {'h', 0, 2, 0, CustomerNoLength, "Customer Number Length (0 .. 15)"},
  {'h', 0, 2, 3, OrderLength,      "Order Number Length    (1 .. 7)"},
  {'h', 0, 2, 0, PicklineLength,   "Pickline Number Length (0 .. 2)"},
  {'h', 0, 2, 0, GroupLength ,     "Group Length           (0 .. 4)"},
  {'h', 0, 2, 0, 1,                "Priority Length        (0 .. 1)"},
  {'h', 0, 2, 0, SkuLength,        "SKU Length             (0 .. 15)"},
  {'h', 0, 2, 0, ModuleLength,     "Module Length          (0 .. 5)"},
  {'h', 0, 2, 1, QuantityLength,   "Quantity Length        (1 .. ?)"},
  {'h', 0, 2, 0, RemarksLength,    "Remarks Length         (0 .. ?)"},
  {'h', 0, 2, 0, RemarksLength,    "Box Offset             (0 .. ?)"},
  {'h', 0, 2, 0, BoxLength,        "Box Number Length      (0 .. 6)"},
  {'h', 0, 2, 0, 99,               "Box Number Count       (0 .. 99)"},
  {'h', 0, 2, 0, PickTextLength,   "Pick Text Length       (0 .. ?)"},
  {'h', 0, 2, 0, StklocLength,     "Stock Location Length  (0 .. ?)"},
  {'h', 0, 2, 0, 0,                "Spare                  (0 .. ?)"},
  {'c', 0, 1, 0, 0,                "Duplicate Pick Allowed (nys)"},
  {'c', 0, 1, 0, 0,                "Skip Bad SKU           (hony)"},
  {'c', 0, 1, 0, 0,                "Zero Quantity          (nys)"},
  {'c', 0, 1, 0, 0,                "Hold Orders            (inky)"},
  {'c', 0, 1, 0, 0,                "Ignore Remarks         (ny)"},
  {'c', 0, 1, 0, 0,                "Ignore Pick Text       (ny)"},
  
  {0, 0, 0, 0, 0, 0}};
      

#endif

/* end of ss_init.h */
