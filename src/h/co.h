#define SPLITPICK
#define TCBL
#define BOXFULL
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Configuration shared segment definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/26/93   |  tjt  Rewritten for ports, picklines, zones, bays, etc.
 *  04/04/94   |  tjt  Added ac and tc view.
 *  04/13/94   |  tjt  Modified for UNIX.
 *  05/12/94   |  tjt  Add IsOffline flag to disable zones and/or bays.
 *  09/02/94   |  tjt  Add BoxOperation Flag.
 *  11/14/94   |  tjt  Add pickline segment structure.
 *  03/17/95   |  tjt  Remove tc and ac pickline displays.
 *  03/17/95   |  tjt  Add ZC2, PM2 and PM4.
 *  10/21/95   |  tjt  Add picker for zone.
 *  04/15/96   |  tjt  Add matrix and carousels.
 *  04/26/96   |  tjt  Add put bays.
 *  06/07/97   |  tjt  Add PM6 for aisle controller.
 *  05/18/98   |  tjt  Add IO for RS232 scanner module.
 *  05/24/98	|  tjt  Add HasIOModule flag for bays and zones.
 *  05/24/98   |  tjt  Add hw_states for 8 scanner usage flags.
 *-------------------------------------------------------------------------*/
#ifndef CO_H
#define CO_H

static char co_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "global_types.h"
#include "st.h"

/*-------------------------------------------------------------------------*
 * Values for hw_type - all hardware (full, basic and total function)
 *-------------------------------------------------------------------------*/
 
#define BL  1           /* full & total function -  bay lamp		  */
#define ZC  2		/* full function - 5 byte zone controller	  */ 
#define PM  3		/* full function - 2 byte pick module	  	  */
#define PI  4		/* basic & total function pick indicator	  */
#define ZC2 5		/* total function - 10 byte zone controller	  */ 
#define PM2 6		/* total function - 2 byte pick module		  */
#define PM4 7		/* total function - 4 byte pick module		  */
#define PM6 8		/* total function - 6 byte large pick module	  */
#define BF  9		/* full & total function - box full module	  */
#define IO  10          /* total function - serial input/output module */

#ifdef TCBL
#define TB	11   /* total function - Tri Color Bay Lamp */
#endif
/*-------------------------------------------------------------------------*
 *  Values for hw_state when hw_type == IO (Total Function Scanner)
 *-------------------------------------------------------------------------*/

#define SCAN_PL_INDUCT		0x01	/* Pickline order induction	  */
#define SCAN_ZONE_INDUCT	0x02	/* Zone Induction		  */
#define SCAN_BOX		0x04	/* Box Open Scan (Closes others)  */
#define SCAN_LOT		0x08    /* Lot number for last pick	  */
#define SCAN_SERIAL		0x10	/* Serial number of last pick	  */
#define SCAN_CUSTOM1		0x20	/* Undefined			  */
#define SCAN_CUSTOM2		0x40	/* Undefined			  */
#define SCAN_CUSTOM3		0x80	/* Undefined 			  */

/*--------------------------------------------------------------------------*
 * Values for hw_flags, bay_flags, zone_flags, and port_flags
 *--------------------------------------------------------------------------*/


#define  BinUnassigned        0x01        /* product flags                   */
#define  PicksInhibited       0x02
#define  BinHasPick           0x04
#define  BinPicked            0x08
#define  BinSpare1            0x10
#define  BinSpare2            0x20
#define  BinSpare3            0x40
#define  BinSpare4            0x80
#define  MultiModule          0x0100      /* bay only flag                   */
#define  IsPI                 0x40000000  /* bay and zone flag               */


#define  SwitchesDisabled     0x01        /* module flags                    */
#define  ModuleRedisplay      0x02
#define  ModuleHasPick        0x04
#define  ModulePicked         0x08
#define  ModuleSpare1         0x80
#define  ModuleSpare2         0x10
#define  ModuleShortCount     0x20
#define  ModuleFlag           0x40
#define  ModuleBlink          0x80

#define  IsMasterBayLamp      0x02        /* bay flags                       */
#define  HasBayLamp           0x04        
#define  HasZoneController    0x08
#define  HasModules           0x10
#define  HasBayBarrier        0x20
#define  VertLights           0x40        /* matrix or light tree            */
#define  HortLights           0x80        /* carousel of light bar           */
#define  Multibin             0xc0        /* either is multibin lights       */

#define  FirstZone            0x0100      /* zone flags                      */
#define  LateEntry            0x0200
#define  Steering             0x0400
#define  JumpZone             0x0800
#define  EarlyExit            0x1000
#define  BoxOperation         0x2000
#define  ZoneInactive         0x4000
#define  IsOffline            0x8000

#define  EarlyExitModeNext    0x010000    /* pickline flags                  */
#define  EarlyExitModeLast    0x020000
#define  OrdersLocked         0x040000
#define  PendingOrderLock     0x080000
#define  StopOrderFeed        0x100000
#define  HasBadgeReader       0x200000
#define  IsMirror             0x400000
#define  IsSegmented          0x800000

#define  IsBasicFunction      0x01000000  /* port flags                      */
#define  IsFullFunction       0x02000000
#define  IsTotalFunction      0x04000000
#define  IsCarousel           0x08000000  /* bay and zone flag               */
#define  IsPut                0x10000000  /* bay and zone flag               */
#define  IsDummy              0x20000000 
#define  HasIOModule          0x40000000	/* F052498								  */
#define  HasBoxFull           0x80000000  /* Cosmair FF Box Full Lights      */
#ifdef TCBL
#define  HasBadgeReader	      0
#define  HasTCBayLamp	      0x200000
#define  IsDummy              0
#define  DemandFeed           0x20000000
#endif
#ifdef BOXFULL
#define  BoxFull         0x01  /* Box full sets this flag      */
#define  ScanBoxClose    0x02 /* Box full sets this flag      */
#endif


/*-------------------------------------------------------------------------*
 *  Configuration Header
 *-------------------------------------------------------------------------*/
 
struct co_header
{
  char          co_config_name[31];       /* configuration name              */
  unsigned char co_id;                    /* message id of ofc               */
  long          co_datetime;              /* date configuration built        */
  long          co_st_changed;            /* sku table modified              */
  
  long          co_ports;                 /* allocated ports                 */
  long          co_picklines;             /* allocated picklines             */
  long          co_segments;              /* allocated segments              */
  long          co_zones;                 /* allocated zones                 */
  long          co_bays;                  /* allocated bays                  */
  long          co_products;              /* allocated products              */
  long          co_modules;               /* allocated modules               */
  long          co_lights;                /* allocated lights                */
  
  long          co_pl_config;             /* picklines in configuration      */
  
  long          co_port_cnt;              /* actual hardware ports           */
  long          co_light_cnt;             /* actual total lights             */
  long          co_pl_cnt;                /* config picklines                */
  long          co_seg_cnt;               /* config segments                 */
  long          co_zone_cnt;              /* config zones                    */
  long          co_bay_cnt;               /* config bays                     */
  long          co_prod_cnt;              /* config products                 */
  long          co_mod_cnt;               /* config modules                  */
  long          co_hw_cnt;                /* config last light used          */
  long          co_st_cnt;                /* items in sku table              */
  
  long          co_po_offset;             /* offset to port table            */
  long          co_pl_offset;             /* offset to pickline table        */
  long          co_seg_offset;            /* offset to segment table         */
  long          co_zone_offset;           /* offset to zone table            */
  long          co_bay_offset;            /* offset to bay table             */
  long          co_hw_offset;             /* offset to hardware table        */
  long          co_pw_offset;             /* offset to pick table            */
  long          co_mh_offset;             /* offset to module table          */
  long          co_st_offset;             /* offset to sku table             */
  long          co_bl_view_offset;        /* offset to bl view table         */
  long          co_zc_view_offset;        /* offset to zc view table         */
  long          co_pm_view_offset;        /* offset to pm view table         */
  
};

/*-------------------------------------------------------------------------*
 *  Port Entry
 *-------------------------------------------------------------------------*/

struct port_item
{
  unsigned char  po_id;                   /* message address of port output  */
  unsigned char  po_id_in;                /* message address of port input   */
  char           po_status;               /* status of port (xidyn)          */
  														/* x=busy i=initialize running 	  */
  														/* d=diags running n=initialized	  */
  														/* y=configured or restored		  */
  char           po_disabled;             /* port disabled flag (yn)         */
  char           po_name[16];             /* port device name                */
  unsigned long  po_flags;                /* various flags                   */
  short          po_products;             /* products on port                */
  short          po_lights;               /* pick lights on port             */
#ifdef TCBL 
  short          po_count[12];             // added for IO Module
#else
  short          po_count[10];            /* lights on port by type F052698  */
#endif
  short          po_controllers;          /* number of controllers           */
};

/*-------------------------------------------------------------------------*
 *  Pickline Entry
 *-------------------------------------------------------------------------*/

struct pl_item
{
  char           pl_name[9];              /* pickline name                   */
  TPickline      pl_pl;                   /* pickline number                 */
  TSegment       pl_first_segment;        /* first segment in pickline       */
  TSegment       pl_last_segment;         /* last  segment in pickline       */
  TZone          pl_first_zone;           /* first zone in pickline          */
  TZone          pl_last_zone;            /* last  zone in pickline          */
  unsigned long  pl_flags;                /* various status flags            */
  char           pl_sam[3];               /* switch action mode              */
  long           pl_complete;             /* order completed count           */
  TOrder         pl_order;                /* pending lock order number       */
  long           pl_lines_to_go;          /* lines yet to pick               */
  long           pl_units_to_go;          /* units yet to pick               */
#ifdef TCBL
  long		 pl_tb_low;		  /*  tri color bay lamp low value   */
  long		 pl_tb_high;		  /*  tri color bay lamp high value   */
#endif
};

/*-------------------------------------------------------------------------*
 *  Pickline Segment Table Entry
 *-------------------------------------------------------------------------*/
 
struct seg_item
{
  TSegment       sg_segment;              /* segment number                  */
  TPickline      sg_pl;                   /* pickline of segment             */
  TZone          sg_first_zone;           /* first zone of segment           */
  TZone          sg_last_zone;            /* last  zone of segment           */
  unsigned long  sg_flags;                /* various status flags            */
  TSegment       sg_below;                /* branch segment below            */
  TSegment       sg_next;                 /* next segment in pickline        */
  TSegment       sg_prev;                 /* prev segment in pickline        */
  unsigned char  sg_snode;                /* first node of segment           */
  unsigned char  sg_enode;                /* last  node of segment           */
};

/*-------------------------------------------------------------------------*
 *  Zone Table Entry
 *-------------------------------------------------------------------------*/

struct zone_item
{
  TZone          zt_zone;                 /* zone number                     */
  TPickline      zt_pl;                   /* pickline number                 */
  TSegment       zt_segment;              /* pickline segment number         */
  TBay           zt_first_bay;            /* first bay in zone               */
  TZone          zt_start_section;        /* preceding entry zone            */
  TZone          zt_end_section;          /* zone before next entry zone     */
  unsigned long  zt_flags;                /* various status flags            */
#ifdef BOXFULL
  unsigned short  zt_flags_1;             /* various status flags            */
#endif
  long           zt_picker;               /* picker in this zone             */
  char           zt_picker_name[12];      /* last name of picker in zone     */
  char           zt_status;               /* zone status  (F,W,I,A)          */
  char           zt_queued;               /* queued into zone                */
  short          zt_lines;                /* active lines in current order   */
  short          zt_units;                /* active units in current order   */
  long           zt_count;                /* orders completed by zone        */
  long           zt_time;                 /* clock of last productivity      */

  TZone          zt_source;               /* source  zone number             */
  TZone          zt_feeding;              /* feeding zone number             */
  
  long           zt_order;                /* current underway order block    */
  TOrder         zt_on;                   /* current underway order number   */
#ifdef SPLITPICK
  TModule       zt_last_light;           /* module number by each type      */
#endif
};

/*-------------------------------------------------------------------------*
 *  Bay Table Entry
 *-------------------------------------------------------------------------*/

struct bay_item
{
  TBay           bay_number;              /* bay number                      */
  TZone          bay_zone;                /* zone for this bay               */
  TBay           bay_next;                /* next bay in this zone           */
  TModule        bay_zc;                  /* hwix of zone controller         */
  TModule        bay_bl;                  /* hwix of bay lamp                */
  TModule        bay_bf;                  /* hwix of box full module         */
  TBay           bay_mbl;                 /* bay of master bay lamp          */
  TProduct       bay_prod_first;          /* first product in bay            */
  TProduct       bay_prod_last;           /* last product in bay             */
  TModule        bay_mod_first;           /* first module in the bay         */
  TModule        bay_mod_last;            /* last module in the bay          */
  TBoxNumber     bay_box_number;          /* current open box                */
  unsigned long  bay_flags;               /* various yes/no flags            */

  unsigned char  bay_picks;               /* active picks in bay             */
  unsigned char  bay_current_picks;       /* matrix or carousel picks        */
  unsigned char  bay_current_shelf;       /* carousel active shelf           */
  unsigned char  bay_shelves;             /* matrix shelves - carousel trays */
  unsigned char  bay_width;               /* shelf or tray width             */
  unsigned char  bay_port;                /* light port for this bay         */
#ifdef TCBL
  TModule        bay_tcbl;                /* hwix of tri colored bay lamp    */
#endif
};

#define bay_controller bay_zc             /* TC address in port              */
#define bay_box_pick   bay_bl             /* TC current pick HWIX            */
#define bay_state      bay_mbl            /* TC box dialog state             */

/*-------------------------------------------------------------------------*
 *  Light Module Number to HWIX.
 *-------------------------------------------------------------------------*/

struct mh_item
{
  short mh_ptr;                           /* subscript in hw_item table      */
};

/*-------------------------------------------------------------------------*
 *  Hardware Module Item  (16 bytes per item)
 *  One item for each light in order of port wiring.
 *-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*
 *  Hardware Module Item 
 *  One item for each light in order of port wiring.
 *
 *  Multiple IO Hardware Module Item 
 *    hw_type   is MIO
 *    hw_switch is 0x0A, where A is input  0x00 .. 0x0f 
 *    hw_state  is 0x0B, where B is output 0x00 .. 0x0F
 *-------------------------------------------------------------------------*/

struct hw_item
{
  TModule       hw_mod;                   /* module number by each type      */
  TBay          hw_bay;                   /* assigned bay or zero            */
  
  unsigned char hw_type;                  /* module type                     */
  unsigned char hw_switch;                /* current switch position         */
  unsigned char hw_state;                 /* module state                    */
  unsigned char hw_save;                  /* saved state for tests           */
  unsigned char hw_flags;                 /* module status flags             */

  unsigned char hw_controller;            /* controller of this module       */
  short         hw_mod_address;           /* address of module on controller */

  short         hw_first;                 /* first product for this module   */
  short         hw_current;               /* current pick information        */
};

/*-------------------------------------------------------------------------*
 *  Product (Pick) Item
 *-------------------------------------------------------------------------*/

struct pw_item
{
  short         pw_ptr;                   /* subscript in hw_item table      */
  short         pw_mod;                   /* module number for this product  */
  
  char          pw_display[4];            /* display code for this location  */
  unsigned char pw_flags;                 /* various status flags            */
  
  long          pw_lines_to_go;           /* lines yet to pick               */
  long          pw_units_to_go;           /* units yet to pick               */
  long          pw_reference;             /* magic number passed with pick   */

  short         pw_case;                  /* case pack size, zero, or ref    */
  short         pw_case_ordered;          /* cases/thousands ordered         */
  short         pw_case_picked;           /* cases/thousands picked          */

  short         pw_pack;                  /* inner pack size, zero, or ref   */
  short         pw_pack_ordered;          /* packs/hundreds ordered          */
  short         pw_pack_picked;           /* packs/hundreds picked           */
  
  short         pw_ordered;               /* quantity to pick                */
  short         pw_picked;                /* current pick quantity           */
};

/*-------------------------------------------------------------------------*
 *  Hardware Display For Simulation And View Of Pickline Hardware
 *-------------------------------------------------------------------------*/

struct hw_bl_view_item
{
  unsigned char hw_display[1];             /* bay lamp current display       */
};

struct hw_pm_view_item
{
  unsigned char hw_display[6];             /* pick module current display    */
};

struct hw_zc_view_item
{
  unsigned char hw_display[16];            /* zone controller curretn display*/
};

/*-------------------------------------------------------------------------*
 *  Configuration/Hardware Pointers
 *-------------------------------------------------------------------------*/
 
extern long co_fd;                        /* configuration file fd           */
extern long co_id;                        /* configuration segment id        */
extern long co_size;                      /* configuration file size         */

extern unsigned char     *co;             /* shared segment                  */
extern struct co_header  *coh;            /* pointer to config header        */
extern struct port_item  *po;             /* pointer to port table           */
extern struct pl_item    *pl;             /* pointer to pickline table       */
extern struct seg_item   *sg;             /* pointer to segment table        */
extern struct bay_item   *bay;            /* pointer to bay table            */
extern struct zone_item  *zone;           /* pointer to zone                 */
extern struct hw_item    *hw;             /* pointer to hardware table       */
extern struct pw_item    *pw;             /* pointer to pick table           */
extern struct mh_item    *mh;             /* pointer to module/hw table      */
extern struct st_item    *st;             /* pointer to sku table            */

extern struct hw_bl_view_item *blv;       /* pointer to bay lamp view        */
extern struct hw_zc_view_item *zcv;       /* pointer to zone controller view */
extern struct hw_pm_view_item *pmv;       /* pointer to pick module view     */

#endif

/* end of co.h */
