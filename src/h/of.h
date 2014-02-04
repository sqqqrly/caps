#define MIOM
/*-------------------------------------------------------------------------*
 *  Custom Code:    GTE - added oi_box to oi_tab (no #ifdef usused).
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Definition of order file structures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Added to mfc.
 *  07/22/93   |  tjt  Rewritten for Bard.
 *  04/13/94   |  tjt  Modified for UNIX.
 *  08/22/94   |  tjt  Removed BACKORDER flag bit.
 *  08/22/94   |  tjt  Add unit of measure.
 *  06/30/95   |  tjt  Revise order index entry. 
 *  10/06/95   |  tjt  Revised for INFORMIX.
 *  01/10/97   |  tjt  Add oi_le for late entry zone.
 *  06/05/98   |  tjt  Add oi_box for scanned box number.
 *-------------------------------------------------------------------------*/
#ifndef OF_H
#define OF_H

static char of_h[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "global_types.h"
#include "Bard.h"

#define od_get(x)   od_fetch(x, LOCK)     /* get order header with lock      */
#define od_read(x)  od_fetch(x, NOLOCK);  /* get order header without lock   */

#define PENDING_CANCEL  1                 /* ox_flags for pending cancel     */
#define PENDING_HOLD    2                 /* ox_flags for pending hold       */
#define PENDING_USED    4                 /* ox_flags pending group used     */

#define NO_PICK         1                 /* pi_flags no pick module         */
#define PICKED          2                 /* pi_flags has been picked        */
#define SPLIT_PICK      4                 /* pi_flags split over boxes       */
#define VALIDATED      16                 /* pi_flags config check at input  */
#define RESTOCK        32                 /* pi_flags restock notice issued  */
#define OBSOLETE       64                 /* pi_flags module has been deleted*/
#define MIRROR        128                 /* pi_flags dup mirrored pick      */
#define REPICK        256                 /* pi_flags repick of item         */
#define PARTIAL       512                 /* pi_flags partial split pick     */

#define NO_BOX          0                 /* pi_box_number - not picked      */
#define SHORT_BOX       BoxMax            /* pi_box_number - special box     */
#define DUMMY_BOX       BoxMax + 1        /* pi_box_number - picked - no box */

#define TOTE_LABEL      1                 /* oi_flags - tote label queued    */
#define SHIP_LABEL      2                 /* oi_flags - ship label queued    */
#define PACK_LIST       4                 /* oi_flags - packing list queued  */
#define NEED_BOX        8                 /* oi_flags - unboxed picks        */
#define XMITTED        16                 /* oi_flags - has been transmitted */
#define HOLD_TOTE      32                 /* oi_flags - hold for release scan*/
#define MOVED          64                 /* oi_flags - picks have moved     */
#define INHIBITED     128                 /* oi_flags - has inhibited picks  */
#define ORPHANS       256                 /* oi_flags - has orphans          */
#define SHORTS        512                 /* oi_flags - has shorts           */
#define NEED_CONFIG  1024                 /* oi_flags - acd_pm_screen action */
#ifdef MIOM
#define NEED_BOX_SCAN   2048             /* oi_flags - Order need scan      */
#define DISABLE_ZONE    4096             /* oi_flags - Order need a redisplay */
#define ONLY_PICK       8192             /* oi_flags - Order need a redisplay */
#define NEED_NEXT       16384            /* oi_flags - Order need a next */
#endif

#define OC_FIRST        0                 /* enqueue first in queue          */
#define OC_LAST        -1                 /* enqueue last in queue           */
#define OC_COMPLETE     0                 /* complete queue                  */
#define OC_UW           1                 /* underway queue                  */
#define OC_HIGH         2                 /* high queue                      */
#define OC_MED          3                 /* medium queue                    */
#define OC_LOW          4                 /* low queue                       */
#define OC_HOLD         5                 /* held queue                      */
#define OC_WORK         6                 /* working queue                   */

/*-------------------------------------------------------------------------*
 *  Order File Header Record    (40 Bytes)
 *-------------------------------------------------------------------------*/

struct of_header
{
  short  of_pl;                           /* pickline                        */
  long   of_on;                           /* order number                    */
  short  of_no_picks;                     /* number of lines in the order    */
  short  of_no_units;                     /* number of units in the order    */
  long   of_datetime;                     /* date/Time of Status             */
  short  of_elapsed;                      /* elapsed picking seconds         */
  long   of_picker;                       /* picker id                       */

  char   of_pri;                          /* priority                        */
  char   of_status;                       /* order status code (h,q,c,u,x)   */
  char   of_repick;                       /* repick flag (y/n);              */
  char   of_grp[GroupLength];             /* group number                    */
  char   of_con[CustomerNoLength];        /* customer order number           */
};

/*--------------------------------------------------------------------------*
 *  Pick File Record   (46 Bytes + pick text)
 *--------------------------------------------------------------------------*/

struct of_pick_info
{
  TModule        of_mod;                  /* pick module                     */
  TQuantity      of_quan;                 /* quantity to pick                */
  TQuantity      of_short;                /* quantity short                  */
  unsigned short of_mod_flags;            /* various flags                   */
};

struct of_pick_item
{
  long           pi_reference;            /* unique pick key                 */
  short          pi_pl;                   /* pickline                        */
  long           pi_on;                   /* order number                    */
  TModule        pi_mod;                  /* pick module                     */
  TZone          pi_zone;                 /* zone of pick                    */
  TQuantity      pi_ordered;              /* quantity to pick                */
  TQuantity      pi_picked;               /* quantity short                  */
  unsigned short pi_flags;                /* pick/no pick/split pick         */
  
  long           pi_datetime;             /* time picked completed           */

  long           pi_box_number;           /* box number of this pick         */
  char           pi_sku[SkuLength];       /* sku number for this pick        */
  char           pi_pick_text[32];        /* customer pick text              */
  char           pi_lot[LotLength];       /* lot number for this pick        */
};

/*-------------------------------------------------------------------------*
 *  Remarks File Record   (4 Bytes + text)
 *-------------------------------------------------------------------------*/

struct of_rmks_item
{
  short         rmks_pl;                  /* pickline                        */
  long          rmks_on;                  /* order number                    */
  
  TRemarks      rmks_text;                /* remarks text                    */
};

/*-------------------------------------------------------------------------*
 *  Pending Cancel/Hold File 
 *-------------------------------------------------------------------------*/

struct pending_item
{
  short  pnd_pl;                          /* order pickline                  */
  long   pnd_on;                          /* order number (zero if group)    */
  char   pnd_group[GroupLength];          /* group number (null if order)    */
  char   pnd_con[CustomerNoLength];       /* customer order number         */
  short  pnd_flags;                       /* pending flags                   */
};

/*-------------------------------------------------------------------------*
 *  Order Pointer Entry Item
 *-------------------------------------------------------------------------*/

struct oc_entry
{
  long oc_first;                          /* first in queue                  */
  long oc_last;                           /* last in queue                   */
  long oc_count;                          /* number in queue                 */
};

struct oc_item
{
  struct oc_entry oc_queue[7];            /* seven queues per picikline      */
};

#define oc_comp oc_queue[OC_COMPLETE]     /* completed queue                 */
#define oc_uw   oc_queue[OC_UW]           /* underway queue                  */
#define oc_high oc_queue[OC_HIGH]         /* high priority queue             */
#define oc_med  oc_queue[OC_MED]          /* medium priority queue           */
#define oc_low  oc_queue[OC_LOW]          /* low priority queue              */
#define oc_hold oc_queue[OC_HOLD]         /* hold queue                      */
#define oc_work oc_queue[OC_WORK]         /* working queue                   */

/*-------------------------------------------------------------------------*
 *  Order File Index Item
 *-------------------------------------------------------------------------*/

struct oi_item
{
  unsigned       oi_pl:8;                 /* pickline                        */
  unsigned       oi_on:24;                /* order number                    */
  unsigned       oi_blink:20;             /* previous order in queue         */
  unsigned       oi_entry_zone:12;        /* least zone number               */
  unsigned       oi_flink:20;             /* next order in queue             */
  unsigned       oi_exit_zone:12;         /* exit zone number                */
  unsigned       oi_queue:4;              /* queue member (0..6)(cuhmlxw)    */
  unsigned       oi_le:12;                /* late entry zone, if any         */
  unsigned       oi_flags:16;             /* label and box flags             */
  unsigned char  oi_grp[GroupLength];     /* group code                      */
  unsigned char  oi_con[ConLength];       /* customer order number           */
  unsigned char  oi_box[BoxNoLength];	  /* scanned box number	             */

};

/*-------------------------------------------------------------------------*
 *  Order Control Shared Segment
 *-------------------------------------------------------------------------*/

struct oc_rec
{
  long           of_last_purge;           /* last order file purge time      */
  long           of_size;                 /* order index entries             */
  long           of_last_on;              /* assigned CAPS order numbers     */
  short          of_max_picks;            /* maximum picks per order         */
  short          of_rec_size;             /* order file record size          */
  short          op_rec_size;             /* pick file record size           */
  short          or_rec_size;             /* remarks record size             */

  struct oc_item oc_tab[PicklineMax + SegmentMax];/* order control table     */
  struct oi_item oi_tab[1];                       /* order index table       */
};

/*-------------------------------------------------------------------------*
 *  Order File Data Areas
 *-------------------------------------------------------------------------*/
 
extern struct of_header    *of_rec;       /* order header database record    */
extern struct of_pick_item *op_rec;       /* picks database record           */
extern struct of_rmks_item *or_rec;       /* remarks database record         */

extern struct oc_rec       *oc;           /* order control segment           */

extern long oc_fd;                        /* order index fd                  */
extern long oc_id;                        /* order index id                  */
extern long oc_size;                      /* order index file size           */

#endif

/* end of of.h */
