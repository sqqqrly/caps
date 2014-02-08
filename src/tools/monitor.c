#define DELL
/*----------------------------------------------------------------------
 *  Custom Code:    DELL - picker and picker name in zone table.
 *----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Display Values From Shared Segments
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/12/94   |  tjt  Add Offline flag.
 *  11/14/94   |  tjt  Add pickline segments.
 *  12/23/94   |  tjt  Add lot transaction.
 *  03/17/95   |  tjt  Add ZC2, PM2 and PM4 modules.
 *  03/28/95   |  tjt  Add show SKU with pick modules.
 *  06/21/95   |  tjt  Add ZC2 to show_zc.
 *  06/21/95   |  tjt  Add PM2 and PM4 to show_mod.
 *  06/21/95   |  tjt  Fix order number check.
 *  07/01/95   |  tjt  Add sku table changed.
 *  09/15/95   |  tjt  Add HasBoxFull.
 *  09/15/95   |  tjt  Add BF (box full) module.
 *  09/23/95   |  tjt  Add box full transaction.
 *  04/16/96   |  tjt  Remove cluster zones.
 *  04/16/96   |  tjt  Add rf_stkloc and sp_use_stkloc.
 *  04/16/96   |  tjt  Add product lookup.
 *  05/20/96   |  tjt  Add box full module.
 *  05/26/98   |  tjt  Add IO Module.
 *-------------------------------------------------------------------------*/
static char monitor_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "Bard.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "pr.h"
#include "st.h"

struct pending_item pnd;

char code[8];
char parm[8];
char order[8];
long c;
long p;
long block;
long o;
long k;
long j;
char junk[30];

char xlate[] = {'-', '+', 'U', 'D', 'C', 'A', 'a', 'e', ' ', 'E', 'L', 'N'};
char sku[20];

extern show_sp(), show_rf(), show_coh(), show_po(), show_pl();
extern show_zone(), show_bay(), show_hw(), show_bl(), show_zc();
extern show_mod(), show_sku(), show_order(), show_block(), show_group();
extern show_oc_header(), show_oc_tab(), show_pp(), show_pz();
extern show_size(), show_bf(), show_io();

#define MAX 22

void (*where[MAX])() = {show_sp, show_rf, show_coh, show_po, show_pl,
show_zone, show_bay, show_hw, show_bl, show_zc, show_mod, show_sku,
show_order, show_block, show_group, show_oc_header, show_oc_tab,
show_pp, show_pz, show_size, show_bf, show_io};

/*-------------------------------------------------------------------------*
 *   M A I N    L O O P
 *-------------------------------------------------------------------------*/
main()
{
  putenv("_=monitor");                     /* incure name in environ       */
  chdir(getenv("HOME"));                   /* to home directory     */

  database_open();
  
  ss_open();
  co_open();
  oc_open();
  od_open(); 
  pr_open();

  pending_open(READONLY);

  while (1)
  {
    printf(" 1 - System Parameters\n");
    printf(" 2 - Record Format\n");
    printf(" 3 - Configuration Header\n");
    printf(" 4 - Port\n");
    printf(" 5 - Pickline\n");
    printf(" 6 - Zone\n");
    printf(" 7 - Bay\n");
    printf(" 8 - HWIX\n");                  
    printf(" 9 - BL    by Module Number\n");          
    printf("10 - ZC    by Module Number\n");
    printf("11 - PM/PI by Module or Product Number\n");
    printf("21 - BF    by Module Number\n");
    printf("22 - IO    by Module Number\n");
    printf("12 - SKU Lookup\n");
    printf("13 - Order/Pending By Pickline and Order\n");
    printf("14 - Order By Block\n");
    printf("15 - Pending Group\n");
    printf("16 - Order Header\n");
    printf("17 - Order Pickline Control\n");
    printf("18 - Pickline Productivity\n");
    printf("19 - Zone Productivity\n");
    printf("20 - Segment Sizes\n");

    printf("\nEnter Code ---> ");
    gets(code);

    c = cvrt(code) - 1;
  
    if (c < 0 || c >= MAX) break;

    (*where[c])();
  }
  pending_close();

  ss_close();
  co_close();
  oc_close();                        
  od_close();
  pr_close();

  database_close();
  
  return 0;
}                                         /* end of main                     */
/*-------------------------------------------------------------------------*
 *  Convert to Integer
 *-------------------------------------------------------------------------*/
cvrt(p)
char *p;
{
  register long x;
  
  x = 0;

  for(; *p; p++) x = 10 * x + *p - '0';

  return x;
}
/*-------------------------------------------------------------------------*
 *  More Question
 *-------------------------------------------------------------------------*/
more()
{
  char ans[8];

  printf("More ?");
  fflush(stdout);
  gets(ans);
  *ans = tolower(*ans);
  if (*ans == 'n' || *ans == 'q') return 0;
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Show Configuration Flags
 *-------------------------------------------------------------------------*/
show_hw_flags(x)
register long x;
{
  if (x & SwitchesDisabled)  printf(" Disabled");
  if (x & ModuleRedisplay)   printf(" Redisplay");
  if (x & ModuleShortCount)  printf(" Shorting");
  if (x & ModuleFlag)        printf(" Flag");
  if (x & ModuleBlink)       printf(" Blink");
  if (x & ModuleHasPick)     printf(" HasPick");
  if (x & ModulePicked)      printf(" Picked");

  printf("\n");
}
show_pw_flags(x)
register long x;
{
  if (x & BinUnassigned)     printf(" Unassigned");
  if (x & PicksInhibited)    printf(" Inhibited");
  if (x & BinHasPick)        printf(" HasPick");
  if (x & BinPicked)         printf(" Picked");

  printf("\n");
}
show_flags(x)
register long x;
{
  if (x & IsBasicFunction)   printf(" BF");
  if (x & IsFullFunction)    printf(" FF");
  if (x & IsTotalFunction)   printf(" TF");
  if (x & IsDummy)           printf(" DF");
  if (x & HasBoxFull)        printf(" Box");
  if (x & ScanBoxClose)      printf(" ScanBoxClose");
  if (x & HasIOModule)	     printf(" IO");	
  
  if (x & SwitchesDisabled)  printf(" Disabled");
  if (x & IsMasterBayLamp)   printf(" IsMBL");
  if (x & HasBayLamp)        printf(" HasBL");
  if (x & HasZoneController) printf(" HasZC");
  if (x & HasModules)        printf(" HasPM");
  if (x & HasBayBarrier)     printf(" Barrier");
  if (x & VertLights)        printf(" Matrix");
  if (x & IsCarousel)        printf(" Carousel");
  if (x & IsPut)             printf(" Put");
  if (x & HortLights)        printf(" LightBar");
  
  if (x & FirstZone)         printf(" First");
  if (x & LateEntry)         printf(" LE");
  if (x & Steering)          printf(" ST");
  if (x & JumpZone)          printf(" JZ");
  if (x & EarlyExit)         printf(" EE");
  if (x & BoxOperation)      printf(" Box");
  if (x & DemandFeed)        printf(" SI");
  if (x & ZoneInactive)      printf(" Idle");
  if (x & IsOffline)         printf(" Offline");
  
  if (x & EarlyExitModeNext) printf(" EEMODE=NEXT");
  if (x & EarlyExitModeLast) printf(" EEMODE=LAST");
  if (x & OrdersLocked)      printf(" Locked");
  if (x & PendingOrderLock)  printf(" PendingLock");
  if (x & StopOrderFeed)     printf(" StopFeed");
  if (x & HasBadgeReader)    printf(" Badge");
  if (x & IsMirror)          printf(" Mirror");
  if (x & IsSegmented)       printf(" Segments");
  
  printf("\n");
}
/*-------------------------------------------------------------------------*
 *  Order data
 *-------------------------------------------------------------------------*/
order_data(block)
register long block;
{
  register long m;
  unsigned char status[ZoneMax];
  char *p;
  
  printf("Order Block = %-5d\n", block);
  printf("PL/Segment  = %-4d  ", oc->oi_tab[block - 1].oi_pl);
  printf("Order       = %7.*d ",  OrderLength, oc->oi_tab[block - 1].oi_on);
  printf("Group       = %*.*s\n",
    GroupLength, GroupLength, oc->oi_tab[block - 1].oi_grp);
  printf("Blink       = %-4d  ", oc->oi_tab[block - 1].oi_blink);
  printf("Flink       = %-4d    ", oc->oi_tab[block - 1].oi_flink);
  printf("Queue       = %d\n",   oc->oi_tab[block - 1].oi_queue);
  printf("Entry Zone  = %-4d  ", oc->oi_tab[block - 1].oi_entry_zone);
  printf("Exit Zone   = %-4d    ", oc->oi_tab[block - 1].oi_exit_zone);
  printf("Con         = [%*.*s]\n", ConLength, ConLength,
                                    oc->oi_tab[block - 1].oi_con);
  printf("Current Box = [%*.*s]\n", BoxNoLength, BoxNoLength,
                                    oc->oi_tab[block - 1].oi_box);
  
  printf("Flags       =");
  if (oc->oi_tab[block - 1].oi_flags & TOTE_LABEL) printf(" Tote");
  if (oc->oi_tab[block - 1].oi_flags & SHIP_LABEL) printf(" Ship");
  if (oc->oi_tab[block - 1].oi_flags & PACK_LIST)  printf(" Pack");
  if (oc->oi_tab[block - 1].oi_flags & NEED_BOX)   printf(" NeedBox");
  if (oc->oi_tab[block - 1].oi_flags & XMITTED)    printf(" Sent");
  if (oc->oi_tab[block - 1].oi_flags & HOLD_TOTE)  printf(" Hold");
  if (oc->oi_tab[block - 1].oi_flags & MOVED)      printf(" Move");
  if (oc->oi_tab[block - 1].oi_flags & INHIBITED)  printf(" Inhib");
  if (oc->oi_tab[block - 1].oi_flags & ORPHANS)    printf(" Orphans");
  if (oc->oi_tab[block - 1].oi_flags & SHORTS)     printf(" Shorts");
  if (oc->oi_tab[block - 1].oi_flags & NEED_CONFIG)printf(" ACD");
  
  printf("\n");
  
  if (oc->oi_tab[block - 1].oi_pl > PicklineMax)
  {
    gets(code);
    return;
  }
  od_read(block);
  
  printf("\nOrder Record\n");
  printf("Pickline    = %-4d  ",    of_rec->of_pl);
  printf("Order       = %7.*d ",    OrderLength, of_rec->of_on);
  printf("Picker      = %d\n",      of_rec->of_picker);
  printf("Picks       = %-4d  ",    of_rec->of_no_picks);
  printf("Units       = %-4d    ",  of_rec->of_no_units);
  printf("Elapsed     = %d\n",      of_rec->of_elapsed);
  printf("Priority    = %c     ",   of_rec->of_pri);
  printf("Status      = %c       ", of_rec->of_status);
  printf("Group       = %*.*s\n", 
    GroupLength, GroupLength, of_rec->of_grp);
  printf("Customer No = %15.15s\n", of_rec->of_con);
  printf("Time        = %s\n",    ctime(&of_rec->of_datetime));

  if (!more()) return;

  printf("\nUnderway Status\n");

  od_status(block, status);
  
  for (k = 0; k < coh->co_zone_cnt; k++)
  {
    if (!(k % 20))
    {
      printf("\nZone");
      for (m = k; m < k + 20; m++) printf(" %2d", (m + 1) % 100);
      printf("\n      ");
    }
    printf("%c  ", status[k]);
  }
  printf("\n");
  if (!more()) return;
  
  printf("\nPicks\n");

  pick_setkey(1);
  
  op_rec->pi_pl  = of_rec->of_pl;
  op_rec->pi_on  = of_rec->of_on;
  op_rec->pi_mod = 0;
  pick_startkey(op_rec);
  
  op_rec->pi_mod = 20000;
  pick_stopkey(op_rec);

  k = 0;

  while (!pick_next(op_rec, NOLOCK))
  {
    k++;

    if (op_rec->pi_datetime > 0) 
    {
      p = (char *)ctime(&op_rec->pi_datetime) + 4;
      *(p + 15) = 0;
    }
    else p = " ";
    
    printf("%-3d: mod=%-4d zn=%2d quan=%-2d pick=%-2d box=%d sku=%s %s",
      k, op_rec->pi_mod, op_rec->pi_zone, op_rec->pi_ordered, 
      op_rec->pi_picked, op_rec->pi_box_number, op_rec->pi_sku, p);
 
    if (op_rec->pi_flags & NO_PICK)    printf(" Inhibited");
    if (op_rec->pi_flags & SPLIT_PICK) printf(" Split");
    if (op_rec->pi_flags & PICKED)     printf(" Picked");
    if (op_rec->pi_flags & VALIDATED)  printf(" OK");
    if (op_rec->pi_flags & RESTOCK)    printf(" Restock");
    if (op_rec->pi_flags & OBSOLETE)   printf(" Obsolete");
    if (op_rec->pi_flags & MIRROR)     printf(" Mirror");
    printf("\n");

    if (!(k % 12)) 
    {
      if (!more()) return;
    }
  }
  printf("\n");
  if (rf->rf_rmks && rf->rf_ignore_rmks != 'y')
  {
    printf("Hit Return To See Remarks\n");
    if (!more()) return;
    
    or_rec->rmks_pl = of_rec->of_pl;
    or_rec->rmks_on = of_rec->of_on;
    
    if (!remarks_read(or_rec, NOLOCK))
    {
      Bdump(or_rec->rmks_text, rf->rf_rmks);
    }
  }
  return;
}
/*-------------------------------------------------------------------------*
 * print oc queue item
 *-------------------------------------------------------------------------*/
queue(a)
struct oc_entry *a;
{
  printf("%6d%6d%6d\n", a->oc_first, a->oc_last, a->oc_count);
  return;
}
/*-------------------------------------------------------------------------*
 * System Parameters
 *-------------------------------------------------------------------------*/
show_sp()
{
  printf("\n\nSystem Paramters\n");
  printf("ssi = %x, sp = %x\n", ssi, sp);
  printf("System      = %s\n", sp->sp_name);
  printf("Company     = %s\n", sp->sp_company);

  printf("\nAllocation Limits\n\n");
  printf("Max Ports       = %-4d  ", sp->sp_ports);
  printf("Max Picklines   = %-4d  ", sp->sp_picklines);
  printf("Max Segments    = %-4d\n", sp->sp_segments);
  printf("Max Zones       = %-4d  ", sp->sp_zones);
  printf("Max Bays        = %-4d  ", sp->sp_bays);
  printf("Max Products    = %-4d\n", sp->sp_products);
  printf("Max Modules     = %-4d  ", sp->sp_modules);
  printf("Max Lights      = %-4d  ", sp->sp_lights);
  printf("Max Orders      = %-4d\n", sp->sp_orders);
  printf("Max Picks       = %-4d\n", sp->sp_picks);

  if (!more()) return;
  
  printf("\nFeatures\n\n");
  printf("Full Function   = %c     ", sp->sp_full_function);
  printf("Basic Function  = %c     ", sp->sp_basic_function);
  printf("Total Function  = %c\n",    sp->sp_total_function);

  printf("Multibin Lights = %c     ", sp->sp_multibin_lights);
  printf("Boxes           = %c     ", sp->sp_box_feature);
  printf("Box Full        = %c\n",    sp->sp_box_full);

  printf("Autocasing      = %c     ", sp->sp_autocasing);
  printf("Jump Zone       = %c     ", sp->sp_jump_zone);
  printf("Steering        = %c\n",    sp->sp_steering);

  printf("Late Entry      = %c     ", sp->sp_late_entry);
  printf("Early Exit      = %c     ", sp->sp_early_exit);
  printf("Master Bay Lamp = %c\n",    sp->sp_master_bay_lamps);

  printf("Use Stkloc      = %c     ", sp->sp_use_stkloc);
  printf("Pickline Zero In= %c     ", sp->sp_pickline_zero);
  printf("Check Bay Breaks= %c\n",    sp->sp_check_controllers);

  printf("Pending Ops     = %c     ", sp->sp_pending_ops);        
  printf("Unassigned PM's = %c     ", sp->sp_unassigned_pm);
  printf("UM In Pick Text = %c\n",    sp->sp_um_in_pick_text);

  printf("Blink Over      = %-4d  ",  sp->sp_blink_over);
  printf("Lot Control     = %c     ", sp->sp_lot_control);
  printf("SKU Support     = %c\n",    sp->sp_sku_support);

  printf("Mirroring       = %c     ", sp->sp_mirroring);
  printf("Productivity    = %c     ", sp->sp_productivity);
  printf("Labels          = %c\n",    sp->sp_labels);

  printf("Inventory       = %c     ", sp->sp_inventory);
  printf("Restock Notices = %c     ", sp->sp_restock_notice);
  printf("Short Notices   = %c\n",    sp->sp_short_notice);

  printf("Zero Order Cmds = %c     ", sp->sp_global_order_cmds);
  printf("Zero Group Cmds = %c     ", sp->sp_global_group_cmds);
  printf("Orders Anytime  = %c\n",    sp->sp_order_input_anytime);

  printf("Comm Orders In  = %c     ", sp->sp_commo_orders_in);
  printf("Comm Trans Out  = %c     ", sp->sp_commo_trans_out);
  printf("Comm Prodfile   = %c\n",    sp->sp_prodfile_inout);

  printf("Picker Orders   = %-4d  ",  sp->sp_pa_count);
  printf("Delete Empty Box= %c     ", sp->sp_delete_empty_boxes);
  printf("Zone Events     = %c\n",    sp->sp_zone_status_events);

  printf("LE 1 Push       = %c     ", sp->sp_late_entry_one_button);
  printf("No Pick 1 Push  = %c     ", sp->sp_no_pick_one_button);
  printf("EE 1 Push       = %c\n",    sp->sp_early_exit_one_button);

  printf("Pickline View   = %c     ", sp->sp_pickline_view);
  printf("Remaining Picks = %c     ", sp->sp_remaining_picks);
  printf("Port Timeout    = %-5d\n",  sp->sp_init_timeout);

  printf("Short Ticks     = %-4d  ",  sp->sp_short_ticks);
  printf("MP Timeout      = %-5d ",   sp->sp_mp_timeout);
  printf("RP/Init Timeout = %-5d\n",  sp->sp_rp_timeout);
  
  printf("Order Purge     = %c     ", sp->sp_order_input_purge);
  printf("Port Names      = %c     ", sp->sp_port_by_name);
  printf("Pickline Names  = %c\n",    sp->sp_pl_by_name);
  
  printf("Use Customer No = %c\n",    sp->sp_use_con);

  if (!more()) return;

  printf("\nStatus Flags\n\n");
  printf("In Process      = %c     ", sp->sp_in_process_status);
  printf("Init Flag       = %c     ", sp->sp_init_status);
  printf("Config Flag     = %c\n",    sp->sp_config_status);

  printf("Running Flag    = %c     ", sp->sp_running_status);
  printf("Order Mode      = %c     ", sp->sp_oi_mode);
  printf("Trans Mode      = %c\n",    sp->sp_to_mode);

  printf("Short Printing  = %c     ", sp->sp_sp_flag);
  printf("Restock Print   = %c     ", sp->sp_rp_flag);
  printf("Box Print Mode  = %c\n",    sp->sp_box_mode);

  printf("Tote Print Mode = %c     ", sp->sp_tl_mode);
  printf("Tote Ahead      = %d     ", sp->sp_tl_ahead);
  printf("Ship Print Mode = %c\n",    sp->sp_sl_mode);

  printf("Pack Print Mode = %c     ", sp->sp_pl_mode);
  printf("Short Count     = %-5d ",   sp->sp_sh_count);
  printf("Restock Count   = %-5d\n",  sp->sp_rs_count);
  
  printf("Maint Log Count = %-5d ",   sp->sp_log_count);
  printf("Short Printed   = %-5d ",   sp->sp_sh_printed);
  printf("Restock Printed = %-5d\n",  sp->sp_rs_printed);
  
  printf("Tote Count      = %-5d ",   sp->sp_tl_count);
  printf("Ship Count      = %-5d ",   sp->sp_sl_count);
  printf("Packing Count   = %d\n",    sp->sp_pl_count);

  printf("Tote Printed    = %-5d ",   sp->sp_tl_printed);
  printf("Ship Printed    = %-5d ",   sp->sp_sl_printed);
  printf("Packing Printed = %d\n",    sp->sp_pl_printed);

  printf("Tote Last Order = %-5d ",   sp->sp_tl_order);
  printf("Ship Last Order = %-5d ",   sp->sp_sl_order);
  printf("Pack Last Order = %-5d\n",  sp->sp_pl_order);

  printf("Tote Last Print = %-5d ",   sp->sp_tl_order);
  printf("Ship Last Print = %-5d ",   sp->sp_sl_print);
  printf("Pack Last Print = %-5d\n\n",sp->sp_pl_print);

  printf("Save Interval   = %-5d ",  sp->sp_save_window);
  printf("Purge Window    = %-5d ",  sp->sp_purge_window);
  printf("Diskette Format = %c\n\n",   sp->sp_diskette);

  printf("Trans Flag      = %c     ", sp->sp_to_flag);
  printf("Trans Count     = %-5d ",   sp->sp_to_count);
  printf("Trans Xmit      = %d\n",    sp->sp_to_xmit);
  
  printf("To Box Close    = %c     ", sp->sp_to_box_close);
  printf("To Complete     = %c     ", sp->sp_to_complete);
  printf("To Cancel       = %c\n",    sp->sp_to_cancel);

  printf("To Underway     = %c     ", sp->sp_to_underway);
  printf("To Repick       = %c     ", sp->sp_to_repick);
  printf("To Manual       = %c\n",    sp->sp_to_manual);

  printf("To Shortover    = %c     ", sp->sp_to_short);
  printf("To Restock      = %c     ", sp->sp_to_restock);
  printf("To Wave         = %c\n",    sp->sp_to_orders_done);

  printf("To Picks        = %c     ", sp->sp_to_pick_event);
  printf("To Queued       = %c     ", sp->sp_to_order_queued);
  printf("To Lot End      = %c\n",    sp->sp_to_lot_split);
  
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Record Format Paramters
 *-------------------------------------------------------------------------*/
show_rf()
{
  printf("\n\nRecord Format\n");
  printf("ssi = %x, rf = %x\n", ssi, rf);
  printf("rp         = %c (0x%02x)\n", rf->rf_rp, rf->rf_rp);
  printf("rt         = %c (0x%02x)\n", rf->rf_rt, rf->rf_rt);
  printf("ft         = %c (0x%02x)\n", rf->rf_ft, rf->rf_ft);
  printf("eof        = %c (0x%02x)\n\n", rf->rf_eof, rf->rf_eof);
  printf("con        = %-4d  ", rf->rf_con);
  printf("order      = %-4d\n", rf->rf_on);
  printf("pickline   = %-4d  ", rf->rf_pl);
  printf("group      = %-4d\n", rf->rf_grp);
  printf("priority   = %-4d  ", rf->rf_pri);
  printf("sku        = %-4d\n", rf->rf_sku);
  printf("stkloc     = %-4d\n", rf->rf_stkloc);
  printf("mod        = %-4d  ", rf->rf_mod);
  printf("quan       = %-4d\n", rf->rf_quan);
  printf("remarks    = %-4d  ", rf->rf_rmks);
  printf("box pos    = %-4d\n", rf->rf_box_pos);
  printf("box len    = %-4d  ", rf->rf_box_len);
  printf("box count  = %-4d\n", rf->rf_box_count);
  printf("pick text  = %-4d  ", rf->rf_pick_text);
  printf("dups picks = %c     ", rf->rf_dup_flag);
  printf("skip sku   = %c\n",    rf->rf_skip_sku);
  printf("zero quan  = %c     ", rf->rf_zero_quantity);
  printf("hold all   = %c     ", rf->rf_hold);
  printf("ignore rmks= %c\n",    rf->rf_ignore_rmks);
  printf("ignore pt  = %c\n",    rf->rf_ignore_pick_text);
  printf("\n");
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 * Configuration Header
 *-------------------------------------------------------------------------*/
show_coh()
{
  printf("\n\nConfiguration Header\n");
  printf("co = %x, coh = %x\n", co, coh);
  printf("Name = %s  ", coh->co_config_name);
  printf("ID = %d\n",  coh->co_id);
  printf("Date = %s\n", ctime(&coh->co_datetime));
                                       
  printf("Max Ports       = %-4d  ", coh->co_ports);
  printf("Max Picklines   = %-4d  ", coh->co_picklines);
  printf("Max Segments    = %-4d\n", coh->co_segments);
  printf("Max Zones       = %-4d  ", coh->co_zones);
  printf("Max Bays        = %-4d  ", coh->co_bays);
  printf("Max Products    = %-4d\n", coh->co_products);
  printf("Max Modules     = %-4d  ", coh->co_modules);
  printf("Max Lights      = %-4d  ", coh->co_lights);
  printf("SKU Table       = %-4d\n", coh->co_st_changed);

  printf("\n");
  printf("Port Count      = %-4d  ", coh->co_port_cnt);
  printf("Light Count     = %-4d  ", coh->co_light_cnt);
  printf("Pickline Count  = %-4d\n", coh->co_pl_cnt);
  printf("Segment Count   = %-4d  ", coh->co_seg_cnt);
  printf("Zone Count      = %-4d  ", coh->co_zone_cnt);
  printf("Bay Count       = %-4d\n", coh->co_bay_cnt);
  printf("Product Count   = %-4d  ", coh->co_prod_cnt);
  printf("Module Count    = %-4d  ", coh->co_mod_cnt);
  printf("Last Light Used = %-4d\n", coh->co_hw_cnt);
  printf("SKU Count       = %-4d  ", coh->co_st_cnt);
  printf("Picklines       = %d\n",   coh->co_pl_config);
  printf("\n");
  printf("Port Offset     = %-5d ",  coh->co_po_offset);
  printf("Pickline Offset = %-5d ",  coh->co_pl_offset);
  printf("Segment Offset  = %-5d\n", coh->co_seg_offset);
  printf("Zone Offset     = %-5d ",  coh->co_zone_offset);
  printf("Bay Offset      = %-5d ",  coh->co_bay_offset);
  printf("HW Offset       = %-5d\n", coh->co_hw_offset);
  printf("PW Offset       = %-5d\n", coh->co_pw_offset);
  printf("MH Offset       = %-5d ",  coh->co_mh_offset);
  printf("ST Offset       = %-5d ",  coh->co_st_offset);
  printf("BL View Offset  = %-5d\n", coh->co_bl_view_offset);
  printf("ZC View Offset  = %-5d ",  coh->co_zc_view_offset);
  printf("PM View Offset  = %-5d\n", coh->co_pm_view_offset);
  
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Ports
 *-------------------------------------------------------------------------*/
show_po()
{
  printf("\n\nPort\n");
  printf("co = %x, po = %x\n", co, po);
  printf(
"No ID ID Sta Device Name     Prod Mods  BL  ZC  PM  PM46 IO/BF TC  Flag\n");
 printf(
"-- -- -- --- --------------- ---- ---- --- --- ---- ---- ---- --- -------\n");

  for (k = 0; k < coh->co_ports; k++)
  {
    printf("%2d %2d %2d %c %c %-15s%5d%5d%4d%4d%5d%5d%5d%4d ", k, 
     po[k].po_id,  po[k].po_id_in, po[k].po_status ? po[k].po_status : 0x20,
     po[k].po_disabled ? po[k].po_disabled : 0x20,
     po[k].po_name, po[k].po_products, po[k].po_lights,
     po[k].po_count[BL - 1], 
     po[k].po_count[ZC - 1] + po[k].po_count[ZC2 - 1],
     po[k].po_count[PM - 1] + po[k].po_count[PI - 1] + po[k].po_count[PM2 - 1],
     po[k].po_count[PM4 - 1] + po[k].po_count[PM6 - 1],
     po[k].po_count[BF - 1],
     po[k].po_controllers);
 
    show_flags(po[k].po_flags);
  }
  gets(code);
  return;
}
/*------------------------------------------------------------------------*
 *  Pickline
 *------------------------------------------------------------------------*/
show_pl()
{
  printf("Enter Pickline ---> ");
  gets(parm);
  p = cvrt(parm) - 1;

  printf("\n\nPickline %s\n", parm);

  if (p < 0 || p >= coh->co_picklines)
  {
    printf("Pickline Is Out Of Range\n");
    gets(code);
    return;
  }
  printf("co = %x, pl = %x, pl[%d] = %x\n", co, pl, p+1, &pl[p]);

  printf("Name           = %s\n",    pl[p].pl_name);
  printf("Pickline       = %-4d  ",  pl[p].pl_pl);
  printf("Complete Count = %-4d  ",  pl[p].pl_complete);
  printf("SAM            = %3.3s\n", pl[p].pl_sam);
  printf("Lock Order     = %05d ",   pl[p].pl_order);
  printf("First Segment  = %-4d  ",  pl[p].pl_first_segment);
  printf("Last  Segment  = %-4d\n",  pl[p].pl_last_segment);
  printf("First Zone     = %-4d  ",  pl[p].pl_first_zone);
  printf("Last  Zone     = %-4d\n",  pl[p].pl_last_zone);
  printf("Lines To Go    = %5d ",    pl[p].pl_lines_to_go);
  printf("Units To Go    = %d\n",    pl[p].pl_units_to_go);
  printf("Flags          =");        show_flags(pl[p].pl_flags);
  
  for (k = 0; k < coh->co_segments; k++)
  {
    if (sg[k].sg_pl == p + 1)
    {
      printf("\nsg[%d] = %x\n", k+1, &sg[k]);
      printf("Segment        = %-4d  ",  sg[k].sg_segment);
      printf("Pickline       = %-4d  ",  sg[k].sg_pl);
      printf("First Zone     = %-4d\n",  sg[k].sg_first_zone);
      printf("Last  Zone     = %-4d  ",  sg[k].sg_last_zone);
      printf("Segment Below  = %-4d  ",  sg[k].sg_below);
      printf("Segment Next   = %-4d\n",  sg[k].sg_next);
      printf("Segment Prev   = %-4d  ",  sg[k].sg_prev);
      printf("Starting Node  = %-4d  ",  sg[k].sg_snode);
      printf("Ending Node    = %-4d\n",  sg[k].sg_enode);
      printf("Flags          =");        show_flags(sg[k].sg_flags);
    }
  }
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Zone
 *-------------------------------------------------------------------------*/
show_zone()
{
  printf("Enter Zone ---> ");
  gets(parm);
  p = cvrt(parm) - 1;

  printf("\n\nZone %s\n", parm);

  if (p < 0 || p >= coh->co_zones)
  {
     printf("Zone Is Out Of Range\n");
     gets(code);
     return;
  }
  printf("co = %x, zone = %x, zone[%d] = %x\n",co, zone, p+1, &zone[p]);

  printf("Zone       = %-4d  ",  zone[p].zt_zone);
  printf("Pickline   = %-4d  ",  zone[p].zt_pl);
  printf("Segment    = %-4d\n",  zone[p].zt_segment);
  printf("Status     = %c     ", zone[p].zt_status);
  printf("First Bay  = %-4d  ",  zone[p].zt_first_bay);
  printf("Start Area = %-4d\n",  zone[p].zt_start_section);
  printf("End Area   = %-4d  ",  zone[p].zt_end_section);
  printf("Count      = %-5d ",   zone[p].zt_count);
  printf("Lines      = %-4d\n",  zone[p].zt_lines);
  printf("Order      = %-5d ",   zone[p].zt_on);
  printf("Block      = %-5d ",   zone[p].zt_order);
  printf("Queued     = %-4d\n",  zone[p].zt_queued);
  printf("Source     = %-4d  ",  zone[p].zt_source);
  printf("Feeding    = %-4d  ",  zone[p].zt_feeding);
#ifdef DELL
  printf("Picker     = %d\n",    zone[p].zt_picker);
  printf("Picker Name=%12.12s\n",zone[p].zt_picker_name);
#endif
  if (zone[p].zt_time) strcpy(junk, ctime(&zone[p].zt_time));
  else strcpy (junk, "no time available\n");
  printf("Time       = %s", junk);

  printf("Flags      ="); show_flags(zone[p].zt_flags);
  
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Bay
 *-------------------------------------------------------------------------*/
show_bay()
{
  printf("Enter Bay ---> ");
  gets(parm);
  p = cvrt(parm) - 1;

  printf("\n\nBay %s\n", parm);

  if (p < 0 || p >= coh->co_bays)
  {
     printf("Bay Is Out Of Range\n");
     gets(code);
     return;
  }
  printf("co = %x, bay = %x, bay[%d] = %x\n",co, bay, p+1, &bay[p]);

  printf("Bay           = %-4d  ", bay[p].bay_number);
  printf("Zone          = %-4d  ", bay[p].bay_zone);
  printf("Next Bay      = %-4d\n", bay[p].bay_next);
  if (bay[p].bay_flags & IsBasicFunction)
  {
    printf("TC Controller = %-4d  ", bay[p].bay_controller);
    printf("Box State     = %-4d  ", bay[p].bay_state);
    printf("Box Pick Mod  = %-4d\n", bay[p].bay_box_pick);
  }
  else
  {
    printf("ZC HWIX       = %-4d  ", bay[p].bay_zc);  
    printf("BL HWIX       = %-4d\n", bay[p].bay_bl);
    printf("BF HWIX       = %-4d  ", bay[p].bay_bf);
    printf("MBL Bay       = %-4d\n", bay[p].bay_mbl);
  }
  printf("First Product = %-4d  ", bay[p].bay_prod_first);
  printf("Last  Product = %-4d\n", bay[p].bay_prod_last);
  printf("First Module  = %-4d  ", bay[p].bay_mod_first);
  printf("Last  Module  = %-4d\n", bay[p].bay_mod_last);
  printf("Shelves       = %-4d  ", bay[p].bay_shelves);
  printf("Shelf Width   = %-4d  ", bay[p].bay_width);
  printf("Current Shelf = %-4d\n", bay[p].bay_current_shelf);
  printf("Total Picks   = %-4d  ", bay[p].bay_picks);
  printf("Current Picks = %-4d  ", bay[p].bay_current_picks);
  printf("Port          = %-4d\n", bay[p].bay_port);
  printf("Box Number    = %-6d\n", bay[p].bay_box_number);
  printf("Flags         ="); show_flags(bay[p].bay_flags);
  if ((bay[p].bay_flags & IsBasicFunction) && sp->sp_pickline_view)
  {
    printf("Display        = [%10.10s]\n", zcv[p].hw_display);
  }
  printf("\n");

  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Module By HWIX
 *-------------------------------------------------------------------------*/
show_hw()
{
  printf("Enter HWIX ---> ");
  gets(parm);
  p = cvrt(parm) - 1;

  printf("\n\nHWIX %s\n", parm);

  if (p < 0 || p >= coh->co_lights) 
  {
    printf("HWIX Is Out of Range\n");
    gets(code);
    return;
  }
  display_hwix(p);
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Display HW Entry
 *-------------------------------------------------------------------------*/
display_hwix(p)
register long p;
{
  register long k, len;
  register unsigned char *q;
  register struct st_item *s;
  
  printf("co = %x, hw = %x, hw[%d] = %x, mh_ptr = %d\n",  
  co ,hw , p+1, &hw[p], mh[hw[p].hw_mod - 1].mh_ptr);
  
  printf("Module       = %-5d ", hw[p].hw_mod);
  printf("Bay          = %-5d ", hw[p].hw_bay);
  printf("Type         = %d\n",  hw[p].hw_type);
  printf("State        = %-5d ", hw[p].hw_state);
  printf("Save         = %-5d ", hw[p].hw_save);
  printf("Switches     = %d\n",  hw[p].hw_switch);
  printf("Controller   = %-5d ", hw[p].hw_controller);
  printf("Mod Address  = %d\n",  hw[p].hw_mod_address);
  printf("First Prod   = %-5d ", hw[p].hw_first);
  printf("Current Prod = %d\n",  hw[p].hw_current);
  
  if (sp->sp_pickline_view == 'y')
  {
    if (hw[p].hw_type == BL)      
    {
      len = 1; q = blv[hw[p].hw_mod - 1].hw_display;
    }
    else if (hw[p].hw_type == ZC) 
    {
      len = 5;  q = zcv[hw[p].hw_mod - 1].hw_display;
    }
    else if (hw[p].hw_type == ZC2) 
    {
      len = 16; q = zcv[hw[p].hw_mod - 1].hw_display;
    }
    else if (hw[p].hw_type == PI)
    {
      len = 1; q = pmv[hw[p].hw_mod - 1].hw_display;
    }
    else if (hw[p].hw_type == PM)
    {
      len = 2; q = pmv[hw[p].hw_mod - 1].hw_display;
    }
    else if (hw[p].hw_type == PM2)
    {
      len = 4; q = pmv[hw[p].hw_mod - 1].hw_display;
    }
    else if (hw[p].hw_type == PM4)
    {
      len = 4; q = pmv[hw[p].hw_mod - 1].hw_display;
    }
    else len = 0;
    
    if (len > 0)
    {
      printf("Display     = ");
    
      if (hw[p].hw_controller) 
      {
        printf("[");
        for (k = 0; k < len; k++, q++) printf("%c", *q & 0x7f);
        printf("]");
      }
      else for (k = 0; k < len; k++, q++) printf(" 0x%02x", *q);
      printf("\n");
    }
  }
  printf("Flags        ="); show_hw_flags(hw[p].hw_flags);
  printf("\n");

  return;
}
/*-------------------------------------------------------------------------*
 *  Display MH Entry
 *-------------------------------------------------------------------------*/
display_pw(p)
register long p;
{
  register struct st_item *s;
  
  printf("co = %x, pw = %x, pw[%d] = %x\n",  co ,pw , p+1, &pw[p]);

  printf("HWIX         = %-5d ", pw[p].pw_ptr);
  printf("Module       = %d\n",  pw[p].pw_mod);
  
  printf("Lines To Go  = %-5d ", pw[p].pw_lines_to_go);
  printf("Units To Go  = %d\n",  pw[p].pw_units_to_go);
  printf("Case Ordered = %-5d ", pw[p].pw_case_ordered);
  printf("Case Picked  = %-5d ", pw[p].pw_case_picked);
  printf("Case         = %d\n",  pw[p].pw_case);
  printf("Pack Ordered = %-5d ", pw[p].pw_pack_ordered);
  printf("Pack Picked  = %-5d ", pw[p].pw_pack_picked);
  printf("Pack         = %d\n",  pw[p].pw_pack);
  printf("Ordered      = %-5d ", pw[p].pw_ordered);
  printf("Picked       = %-5d ", pw[p].pw_picked);
  printf("Reference    = %d\n",  pw[p].pw_reference);
  printf("Shelf Code   = [%2.2s]\n", pw[p].pw_display);
  
  printf("Flags        ="); show_pw_flags(pw[p].pw_flags);
  printf("\n");

  s = (struct st_item *)mod_lookup(p + 1);
 
  if (s)
  {
    printf("Pickline: %d  ", s->st_pl);
    printf("SKU: [%-.*s]  ",    rf->rf_sku, s->st_sku);
    printf("Stkloc: [%-.*s]  ", rf->rf_stkloc, s->st_stkloc);
    printf("Mirror: %d\n\n", s->st_mirror);
  }
  else printf("No SKU Assigned\n\n");
  display_hwix(pw[p].pw_ptr - 1);

  return;
}
/*-------------------------------------------------------------------------*
 *  Bay Lamp By Module Number
 *-------------------------------------------------------------------------*/
show_bl()
{
  printf("Enter BL Module ---> ");
  gets(parm);
  p = cvrt(parm);

  printf("Bay Lamp %s\n\n", parm);

  if (p < 1 || p > coh->co_lights)
  {
    printf("Module Is Out Of Range\n");
    gets(code);
    return;
  }
  for (k = 0; k < coh->co_lights; k++)
  {
    if (hw[k].hw_type != BL) continue;
    if (hw[k].hw_mod == p)   break;
  }
  if (k < coh->co_lights) display_hwix(k);
  else printf("Not Found\n");
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Zone Controller By Module Number
 *-------------------------------------------------------------------------*/
show_zc()
{
  printf("Enter ZC Module ---> ");
  gets(parm);
  p = cvrt(parm);

  printf("Zone Controller %s\n\n", parm);

  if (p < 1 || p > coh->co_lights)
  {
    printf("Module Is Out Of Range\n");
    gets(code);
    return;
  }
  for (k = 0; k < coh->co_lights; k++)
  {
    if (hw[k].hw_type != ZC && hw[k].hw_type != ZC2) continue; 
    if (hw[k].hw_mod == p)   break;
  }
  if (k < coh->co_lights) display_hwix(k);
  else printf("Not Found\n");
  gets(code);
}
/*-------------------------------------------------------------------------*
 *  IO By Module Number
 *-------------------------------------------------------------------------*/
show_io()
{
  printf("Enter IO Module ---> ");
  gets(parm);
  p = cvrt(parm);

  printf("IO Module %s\n\n", parm);

  if (p < 1 || p > coh->co_lights)
  {
    printf("Module Is Out Of Range\n");
    gets(code);
    return;
  }
  for (k = 0; k < coh->co_lights; k++)
  {
    if (hw[k].hw_type != IO) continue; 
    if (hw[k].hw_mod == p)   break;
  }
  if (k < coh->co_lights) display_hwix(k);
  else printf("Not Found\n");
  gets(code);
}
/*-------------------------------------------------------------------------*
 *  Box Full By Module Number
 *-------------------------------------------------------------------------*/
show_bf()
{
  printf("Enter BF Module ---> ");
  gets(parm);
  p = cvrt(parm);

  printf("Box Full %s\n\n", parm);

  if (p < 1 || p > coh->co_lights)
  {
    printf("Module Is Out Of Range\n");
    gets(code);
    return;
  }
  for (k = 0; k < coh->co_lights; k++)
  {
    if (hw[k].hw_type != BF) continue; 
    if (hw[k].hw_mod == p)   break;
  }
  if (k < coh->co_lights) display_hwix(k);
  else printf("Not Found\n");
  gets(code);
}
/*-------------------------------------------------------------------------*
 *  PM/PI By Module Number
 *-------------------------------------------------------------------------*/
show_mod()
{
  char type;
  
  if (coh->co_products > coh->co_modules || 
      coh->co_prod_cnt > coh->co_mod_cnt)
  {
    printf("Enter PM/PI Product/Module (Pxxx or Mxxx) ---> ");
    gets(parm);
  
    type = tolower(parm[0]);
    if (type == 'm' || type == 'p') p = cvrt(parm + 1);
    else
    {
      type = 'm';
      p = cvrt(parm);
    }
    printf("Module %c%d\n\n", type, p);
  }
  else 
  {
    printf("Enter PM/PI Module ---> ");
    gets(parm);
    p = cvrt(parm);
    printf("Module %d\n\n", p);
    type = 'p';
  }
  if (type == 'm')
  {  
    for (k = 0; k < coh->co_lights; k++)
    {
      if (hw[k].hw_type != PM  && 
          hw[k].hw_type != PM2 &&
          hw[k].hw_type != PM4) continue;
      if (hw[k].hw_mod == p) break;
    }
    if (k < coh->co_lights) display_hwix(k);
    else printf("Not Found\n");
    gets(code);
    return;
  }
  if (p >= 1 && p <= coh->co_prod_cnt) display_pw(p - 1);
  else printf("Not Found\n");
  
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Order By Pickline and Order
 *-------------------------------------------------------------------------*/
show_order()
{
  printf("Enter Pickline ---> ");
  gets(parm);

  p = cvrt(parm);

  if (p < 1 || p > PicklineMax + SegmentMax)
  {
    printf("Pickline Is Out Of Range\n");
    gets(code);
    return;
  }
  printf("Enter Order    ---> ");
  gets(order);
  o = cvrt(order);

  if (o < 1 || o > 9999999)
  {
    printf("Order Number Os Out Of Range\n");
    gets(code);
    return;
  }
  block = oc_find(p, o);

  if (block)
  {
    order_data(block);
    gets(code);
    return;
  }
  pending_setkey(1);
  
  pnd.pnd_pl = p;
  pnd.pnd_on = o;
  
  if (pending_read(&pnd, NOLOCK)) printf("\nNo Order Or Pending\n");
  else
  {
    printf("Pending Order %05d In Pickline %d\n\n", o, p);
    printf("Pickline  = %-4d\n",  pnd.pnd_pl);
    printf("Order     = %-5d\n",  pnd.pnd_on);
    printf("Group     = %4.4s\n", pnd.pnd_group);
    printf("Flags     =");
    if (pnd.pnd_flags & PENDING_CANCEL) printf(" Cancel");
    if (pnd.pnd_flags & PENDING_HOLD)   printf(" Hold");
    if (pnd.pnd_flags & PENDING_USED)   printf(" Used");
    printf("\n");
  }
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Pending Group
 *-------------------------------------------------------------------------*/
show_group()
{
  printf("Enter Pickline ---> ");
  gets(parm);
  p = cvrt(parm);

  if (p < 1 || p > coh->co_picklines)
  {
    printf("Pickline Is Out Of Range\n");
    gets(code);
    return;
  }
  printf("Enter Group   ---> ");
  gets(order);

  printf("Pending Group %s In Pickline %s\n\n", order, parm);

  pending_setkey(2);
  
  pnd.pnd_pl = p;
  memcpy(pnd.pnd_group, order, 4);
  
  if (pending_read(&pnd, NOLOCK)) printf("No Pending For This Group\n");
  else
  {
    printf("Pickline  = %-4d\n",  pnd.pnd_pl);
    printf("Order     = %-5d\n",  pnd.pnd_on);
    printf("Group     = %4.4s\n", pnd.pnd_group);
    printf("Flags     =");
    if (pnd.pnd_flags & PENDING_CANCEL) printf(" Cancel");
    if (pnd.pnd_flags & PENDING_HOLD)   printf(" Hold");
    if (pnd.pnd_flags & PENDING_USED)   printf(" Used");
    printf("\n");
  }
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Order By Block Number
 *-------------------------------------------------------------------------*/
show_block()
{
  printf("Enter Block ---> ");
  gets(order);
  block = cvrt(order);

  if (block < 1 ||block  > oc->of_size)
  {
    printf("Block Is Out Of Range\n");
    gets(code);
    return;
  }
  if (!oc->oi_tab[block - 1].oi_pl)
  {
    if (oc->oi_tab[block - 1].oi_flags) printf("Deleted Index Entry\n");
    else printf("Empty Index Entry\n");
    gets(code);
    return;
  }
  order_data(block);
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Order Pickline Control
 *-------------------------------------------------------------------------*/
show_oc_tab()
{
  printf("Enter Pickline  ---> ");
  gets(parm);
  p = cvrt(parm) - 1;

  printf("\n\nPickline %s Order Control\n", parm);

  if (p < 0 || p > PicklineMax + SegmentMax)
  {
    printf("Pickline Is Out Of Range\n");
    gets(code);
    return;
  }
  printf("oc = %x, oc_tab = %x, oc_tab[%d] = %x\n",
  oc, &oc->oc_tab[0], p+1, &oc->oc_tab[p]);
  printf("       first  last count\n");
  printf("comp  "); queue(&oc->oc_tab[p].oc_comp);
  printf("uw    "); queue(&oc->oc_tab[p].oc_uw);
  printf("high  "); queue(&oc->oc_tab[p].oc_high);
  printf("med   "); queue(&oc->oc_tab[p].oc_med);
  printf("low   "); queue(&oc->oc_tab[p].oc_low);
  printf("hold  "); queue(&oc->oc_tab[p].oc_hold);
  printf("work  "); queue(&oc->oc_tab[p].oc_work);
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Order File Header
 *-------------------------------------------------------------------------*/
show_oc_header()
{
  printf("\n\nOrder File Header\n");
  printf("oc = %x\n", oc);
  printf("Max Orders        = %d\n", oc->of_size);
  printf("Last Order Number = %d\n", oc->of_last_on);
  printf("Max Picks         = %d\n", oc->of_max_picks);
  printf("Order Rec Size    = %d\n", oc->of_rec_size);
  printf("Pick Rec Size     = %d\n", oc->op_rec_size);
  printf("Remarks Rec Size  = %d\n", oc->or_rec_size);
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Pickline Productivity
 *-------------------------------------------------------------------------*/
show_pp()
{
  printf("Enter Pickline ---> ");
  gets(parm);
  p = cvrt(parm) - 1;

  printf("Pickline %s Productivity\n", parm);

  if (p < 0 || p >= pr->pr_picklines)
  {
    printf("Pickline Is Out Of Range\n");
    gets(code);
    return;
  }
  printf("pr = %x pp = %x pp[%d] = %x\n", pr , pp, p+1, &pp[p]);

  printf("Cur Completed  = %d\n", pp[p].pr_pl_cur_completed);
  printf("Cum Completed  = %d\n", pp[p].pr_pl_cum_completed);
  if (pp[p].pr_pl_cum_start) strcpy(junk, ctime(&pp[p].pr_pl_cum_start));
  else strcpy(junk, "no time available\n");
  printf("Cum Start      = %s", junk);
  if (pp[p].pr_pl_cum_start) strcpy(junk, ctime(&pp[p].pr_pl_cur_start));
  else strcpy(junk, "no time available\n");
  printf("Cur Start      = %s", junk);
  printf("Cum Elapsed    = %d\n", pp[p].pr_pl_cum_elapsed);
  printf("Cur Elapsed    = %d\n", pp[p].pr_pl_cur_elapsed);
  if (pp[p].pr_pl_current) strcpy(junk, ctime(&pp[p].pr_pl_current));
  else strcpy(junk, "no time available\n");
  printf("Current Tine   = %s", junk);

  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Zone Productivity
 *-------------------------------------------------------------------------*/
show_pz()
{
  printf("Enter Zone ---> ");
  gets(parm);
  p = cvrt(parm) - 1;

  printf("Zone %s Productivity\n", parm);

  if (p < 0 || p >= pr->pr_zones)
  {
    printf("Zone Is Out Of Range\n");
    gets(code);
    return;
  }
  printf("pr = %x pz = %x pz[%d] = %x\n", pr, pz, p+1, &pz[p]);

  printf("    Orders  Lines  Units Aheads  Time  Active\n");
  printf("    ------ ------ ------ ------ ------ ------\n");

  printf("Cum%7d%7d%7d%7d%7d%7d\n",
  pz[p].pr_zone_cum_orders,
  pz[p].pr_zone_cum_lines,
  pz[p].pr_zone_cum_units,
  pz[p].pr_zone_cum_ah_cnt,
  pz[p].pr_zone_cum_ahead,
  pz[p].pr_zone_cum_active);

  printf("Cur%7d%7d%7d%7d%7d%7d\n",
  pz[p].pr_zone_cur_orders,
  pz[p].pr_zone_cur_lines,
  pz[p].pr_zone_cur_units,
  pz[p].pr_zone_cur_ah_cnt,
  pz[p].pr_zone_cur_ahead,
  pz[p].pr_zone_cur_active);
  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  SKU Lookup
 *-------------------------------------------------------------------------*/
show_sku()
{
  register long k;
  char what[4];
  struct st_item *s;

  printf("Lookup on 1=sku 2=mod 3=stkloc ---> ");
  gets(what);
  
  if (*what == '1')
  {
    printf("Enter Pickline ---> ");
    gets(parm);
    p = cvrt(parm);

    if (p <= 0 || p > coh->co_pl_cnt)
    {
      printf("Pickline Is Out Of Range\n");
      gets(code);
      return;
    }
  }
  if (*what == '1')      printf("Enter SKU      ---> ");
  else if (*what == '2') printf("Enter Module   ---> ");
  else                   printf("Enter Location ---> ");
  
  gets(sku);
  
  if (*what == '1')      s = (struct st_item *)sku_lookup(p, sku);
  else if (*what == '2') s = (struct st_item *)mod_lookup(atol(sku));
  else                   s = (struct st_item *)stkloc_lookup(sku);
  
  if (s)
  {
    printf("\nst = %x  st[%d] = %x\n\n", st, s - st, s);

    if (*what == '1') k = s->st_mirror;
    else              k = 0;
    
    for (; k >= 0; k--, s++)
    {
      printf("Pickline:  %d\n",      s->st_pl);
      printf("SKU:       [%-.*s]\n", rf->rf_sku, s->st_sku);
      printf("Stkloc:    [%-.*s]\n", rf->rf_stkloc, s->st_stkloc);
      printf("Module:    %d\n",      s->st_mod);
      printf("Mirror:    %d\n\n",    s->st_mirror);
    } 
  }
  else printf("Not Found\n");

  gets(code);
  return;
}
/*-------------------------------------------------------------------------*
 *  Segment Sizes
 *-------------------------------------------------------------------------*/
show_size()                         
{
  printf("\n\n    CAPS Shared Segment File Sizes \n\n");

  printf("        ss (%08x) %d bytes\n", sp, ss_size);
  printf("        co (%08x) %d bytes\n", co, co_size);
  printf("        oc (%08x) %d bytes\n", oc, oc_size);
  printf("        pr (%08x) %d bytes\n", pr, pr_size);

  gets(code);
  return;
}

/* end of monitor.c */
