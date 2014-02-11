/*------------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    System shared segment definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Revised.
 *  04/12/94   |  tjt  Modified for UNIX.
 *  05/12/94   |  tjt  Add check bays on one controller flag + spares.
 *  05/28/94   |  tjt  Add pending operations flag.
 *  06/30/94   |  tjt  Add immediate option to autocasing.
 *  07/16/94   |  tjt  Add option 'u' on rf_ignore_pick_text.
 *  08/22/94   |  tjt  Fix remove backorder parms.
 *  08/22/94   |  tjt  Add sp_um_in_pick_text parm.
 *  08/19/94   |  tjt  Add sp_suffix to 6 bytes for SCO PC alignment.
 *  04/22/95   |  tjt  Add sp_order_input_purge options.
 *  06/04/95   |  tjt  Add sp_port_by_name and sp_pl_by_name.
 *  06/29/95   |  tjt  Add sp_use_con option
 *  07/02/95   |  tjt  Add sp_skip_sku hold (k) option.
 *  07/02/96   |  tjt  Add rf_hold inhibited module option (i).
 *  04/16/96   |  tjt  Add rf_stkloc.
 *  04/16/96   |  tjt  Add stock location on order input.
 *  04/17/96   |  tjt  Add multibin lights (matrix or carousel).
 *  04/19/96   |  tjt  Add print and queued counts for notices.
 *  05/21/98   |  tjt  Add box_feature 's' - scanned open and close.
 *-------------------------------------------------------------------------*/
#ifndef SS_H
#define SS_H

static char ss_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "global_types.h"

/*-------------------------------------------------------------------------*
 *  System Parameter Structure
 *-------------------------------------------------------------------------*/

struct sp_item                            /* system-wide paramters           */
{
  char  sp_name[16];                      /* system name                     */
  char  sp_company[40];                   /* company name                    */
  
  long  sp_ports;                         /* maximum number of ports         */
  long  sp_picklines;                     /* maximum number of picklines     */
  long  sp_segments;                      /* maximum number of segments      */
  long  sp_zones;                         /* maximum number of zones         */
  long  sp_bays;                          /* maximum number of bays          */
  long  sp_products;                      /* maximum number of products      */
  long  sp_modules;                       /* maximum number of modules       */
  long  sp_lights;                        /* maximum number of lights        */
  long  sp_orders;                        /* maximum number of orders        */
  long  sp_picks;                         /* maximum number of picks         */
  
  char  sp_full_function;                 /* full function hardware (yns)    */
  char  sp_basic_function;                /* basic function hardware (yns)   */
  char  sp_total_function;                /* total function hardware (yns)   */
  char  sp_multibin_lights;               /* matrix or carousel (yn)         */
 
  char  sp_box_feature;                   /* box feature (yns) - scanned	  */
  char  sp_box_full;                      /* box full pick indicators (aynx) */
  char  sp_autocasing;                    /* autocasing feature (yn)         */
  char  sp_jump_zone;                     /* jump zone feature (yn)          */

  char  sp_steering;                      /* steering feature (yn)           */
  char  sp_late_entry;                    /* late entry feature (yn)         */
  char  sp_early_exit;                    /* early exit feature (yn)         */
  char  sp_master_bay_lamps;              /* master bay lamp feature (yn)    */

  char  sp_use_stkloc;                    /* order input has stkloc input    */
  char  sp_pickline_zero;                 /* pickline zero (y/n/z)           */
  char  sp_check_controllers;             /* check bay on one controller     */
  char  sp_pending_ops;                   /* pending operations allowed      */

  char  sp_unassigned_pm;                 /* pm record w/o sku assign (yn)   */
  char  sp_um_in_pick_text;               /* first byte of pick text in um   */
  char  sp_blink_over;                    /* blink threshold  (0..255)       */
  char  sp_lot_control;                   /* lot control feature (yn)        */

  char  sp_sku_support;                   /* product file feature (yn)       */
  char  sp_mirroring;                     /* product mirroring (yn)          */
  char  sp_productivity;                  /* pickline/zone productivity (yn) */
  char  sp_labels;                        /* label printing supported (yn)   */

  char  sp_inventory;                     /* accumulate inventory (yn)       */
  char  sp_restock_notice;                /* write restock notices (yn)      */
  char  sp_short_notice;                  /* write short notices  (yn)       */
  char  sp_global_order_cmds;             /* zero order cmds (cyn)           */

  char  sp_global_group_cmds;             /* zero group cmds (cyn)           */
  char  sp_order_input_anytime;           /* order input without config (yn) */
  char  sp_commo_orders_in;               /* communication order input (ynk) */
  char  sp_commo_trans_out;               /* communication trans output (ynk)*/

  char  sp_prodfile_inout;                /* communication product file (ynk)*/
  char  sp_pa_count;                      /* number of picker orders         */
  char  sp_delete_empty_boxes;            /* delete unused boxes (yn)        */
  char  sp_zone_status_events;            /* signal each zone event          */

  char  sp_late_entry_one_button;         /* one button late entry (yn)      */
  char  sp_no_pick_one_button;            /* one button no pick (yn)         */
  char  sp_early_exit_one_button;         /* one button no pick ee (yn)      */
  char  sp_pickline_view;                 /* save module display info (yn)   */

  char  sp_remaining_picks;               /* track picks in co segment (yn)  */
  char  sp_init_timeout;                  /* port timeout seconds            */
  char  sp_short_ticks;                   /* short counting ticks            */
  char  sp_mp_timeout;                    /* markplace time out              */
  
  short sp_rp_timeout;                    /* restoreplace/initialize timeout */
  char  sp_order_input_purge;             /* automatic purge (mnxcwy)        */
  char  sp_use_con;                       /* use customer order no (biny)    */

  char  sp_in_process_status;             /* current system action (xicdmns) */
  char  sp_init_status;                   /* hardware initialized (yn)       */
  char  sp_config_status;                 /* configuration status (yn)       */
  char  sp_running_status;                /* picking in operation (yn)       */
  
  char  sp_port_by_name;                  /* show port names (yn)            */
  char  sp_pl_by_name;                    /* show pickline names (yn)        */
  char  sp_order_selection;               /* order selection screen (y/n)    */
  char  sp_oi_mode;                       /* order input mode (cd )          */

  char  sp_to_mode;                       /* transaction output mode (cd )   */
  char  sp_sp_flag;                       /* short printing flag (yn)        */
  char  sp_rp_flag;                       /* restock printing flag (yn)      */
  char  sp_box_mode;                      /* pack list by box mode  (bcdmxz) */

  char  sp_tl_mode;                       /* tote label mode (abcemnxz)      */
  char  sp_tl_ahead;                      /* tote labels ahead count         */
  char  sp_sl_mode;                       /* ship label mode (bcemnxz)       */
  char  sp_pl_mode;                       /* packing list mode (bcdmxz)      */

  long  sp_sh_printed;                    /* short notices printed           */
  long  sp_sh_count;                      /* short notices enqueued          */
  long  sp_rs_printed;                    /* restock notices printed         */
  long  sp_rs_count;                      /* restock notices enqueued        */
  long  sp_tl_printed;                    /* tote labels printed             */
  long  sp_tl_count;                      /* tote labels enqueued            */
  long  sp_sl_printed;                    /* ship labels printed             */
  long  sp_sl_count;                      /* ship labels enqueued            */
  long  sp_pl_printed;                    /* packing lists printed           */
  long  sp_pl_count;                      /* packing lists enqueued          */
  long  sp_bpl_printed;                   /* box packing lists printed       */
  long  sp_bpl_count;                     /* box packing lists enqueued      */
  
  long  sp_log_count;                     /* product file maintenance actions*/
  long  sp_queue_count;                   /* network buffer count            */
  
  long  sp_tl_print;                      /* last tote label order printed   */
  long  sp_tl_order;                      /* last tote label order enqueued  */
  long  sp_sl_print;                      /* last ship label order printed   */
  long  sp_sl_order;                      /* last ship label order enqueued  */
  long  sp_pl_print;                      /* last packing list order printed */
  long  sp_pl_order;                      /* last packing list order enqueued*/

  long  sp_save_window;                   /* save segments interval          */
  long  sp_purge_window;                  /* order purge lag time            */
  
  long  sp_to_count;                      /* transaction file count          */
  long  sp_to_xmit;                       /* transaction send count          */

  char  sp_diskette;                      /* diskette format (sdlh)          */

  char  sp_to_flag;                       /* transaction output flag (bnqy)  */
  char  sp_to_orphan;                     /* orphan picks      (yn)          */
  char  sp_to_complete;                   /* picked orders     (yn)          */
  char  sp_to_cancel;                     /* cancelled orders  (yn)          */
  char  sp_to_underway;                   /* order underway    (yn)          */
  char  sp_to_repick;                     /* repicked orders   (yn)          */
  char  sp_to_manual;                     /* manual orders     (yn)          */
  char  sp_to_short;                      /* short             (yn)          */
  char  sp_to_restock;                    /* restock notice    (yn)          */
  char  sp_to_orders_done;                /* wave complete     (yn)          */
  char  sp_to_box_close;                  /* box close         (yn)          */
  char  sp_to_pick_event;                 /* pick event        (yn)          */
  char  sp_to_order_queued;               /* order input event (yn)          */
  char  sp_to_lot_split;                  /* lot split event   (yn)          */
  
  char  sp_spare1;
};

/*-------------------------------------------------------------------------*
 *  Record Format Parameters
 *-------------------------------------------------------------------------*/  

struct rf_item                            /* record format parameters        */
{
  char  rf_rp;                            /* record preface symbol           */
  char  rf_rt;                            /* record terminator symbol        */
  char  rf_ft;                            /* field terminator symbol         */
  char  rf_eof;                           /* end of file symbol              */
  short rf_con;                           /* length of customer order        */
  short rf_on;                            /* length of order number          */
  short rf_pl;                            /* length of pickline number       */
  short rf_grp;                           /* length of group                 */
  short rf_pri;                           /* length of priority              */
  short rf_sku;                           /* length of SKU                   */
  short rf_mod;                           /* length of module number         */
  short rf_quan;                          /* length of quantity              */
  short rf_rmks;                          /* length of remarks               */
  short rf_box_pos;                       /* box offset                      */
  short rf_box_len;                       /* length of box number            */
  short rf_box_count;                     /* number of box labels            */
  short rf_pick_text;                     /* length of pick text             */
  short rf_stkloc;                        /* stock location                  */
  short rf_spare1;                        /* unused parameter                */
  char  rf_dup_flag;                      /* dup picks (yns)                 */
  char  rf_skip_sku;                      /* skip bad SKU/Mod (hoyn)         */
  char  rf_zero_quantity;                 /* zero (yns)                      */
  char  rf_hold;                          /* hold (inky)                     */
  char  rf_ignore_rmks;                   /* ignore remarks (yn)             */
  char  rf_ignore_pick_text;              /* ignore pick text (yn)           */
  
};

/*-------------------------------------------------------------------------*
 *  S Y S T E M   S E G M E N T
 *-------------------------------------------------------------------------*/

struct ss_item                            /* system segment                  */
{
  struct sp_item    sp_tab;               /* system paramters                */
  struct rf_item    rf_tab;               /* record format                   */
};

/*-------------------------------------------------------------------------*
 *  System Segment Pointers (Setup by ss_open call)
 *-------------------------------------------------------------------------*/

extern struct ss_item    *ssi;            /* points to entire ss             */
extern struct sp_item    *sp;             /* points to system parameters     */
extern struct rf_item    *rf;             /* points to record format         */

/*-------------------------------------------------------------------------*
 *  Files and Events (in ss_open.h)
 *-------------------------------------------------------------------------*/
 
extern long ss_fd;                        /* system segment file fd          */
extern long ss_id;                        /* system segment shared id        */
extern long ss_size;                      /* system segment file size        */

#endif

/* end of ss.h */
