/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    History and CAPS version date for logon logo.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 03/31/94 | tjt Add (order_input) rf_hold == 'y' to hold all.
 * 03/31/94 | tjt Add (order_input) rf_hold == 'k' to hold priority = 'k'.
 * 03/31/94 | tjt Add (record_format_screen) rf_hold input.
 * 04/06/94 | tjt Add (sys_stat2) lines/units remaining.
 * 04/18/94 | tjt Fix (tlc) Box Full + Recall no longer advances box.
 * 04/18/94 | tjt Fix (tlc) Lock other bays in zone when Box Full.
 * 04/22/94 | tjt Fix (tlc) Modified for simulator of tc_write function.
 * 04/30/94 | tjt Fix (all) Remove UNOS code - EC, queues, time, files, etc.
 *
 * 05/01/94 | Version 2.00 - UNIX version
 *
 * 05/20/94 | tjt Add (caps_messages.h) Worst message size.
 * 05/12/94 | tjt Add (co.h) IsOffline flags to disable zones.
 * 05/12/94 | tjt Add (ss.h) Check bay on one controller + more spares.
 * 05/25/94 | tjt Add (ss.h) Ignore remarks and pick text flags.
 * 05/27/94 | tjt Add (ss.h) Pending operations flag.
 * 05/30/94 | tjt Add (ss.h) Old FF diagnostics flag.
 * 05/12/94 | tjt Add (ss_init.h) Check bay on one controller + more spares.
 * 05/24/94 | tjt Fix (ss_init.h) Transaction output (bnqy).
 * 05/25/94 | tjt Add (ss_init.h) Add ignore remarks and pick text flags.
 * 05/27/94 | tjt Add (ss_init.h) Add pending operations flag.
 * 05/30/94 | tjt Add (ss_init.h) Old FF diagnostics flag.
 * 05/03/94 | tjt Add (xt.h) Zone field.
 * 05/06/94 | tjt Fix (xt.h) Six digit order number.
 * 05/04/94 | tjt Fix (zone_status.h) ZS_INACTIVE, 'O', added for offline.
 *
 * 05/19/94 | tjt Add (getparms) Default operator values.
 * 05/24/94 | tjt Fix (xt_build) Transaction output (bnqy).
 *
 * 05/12/94 | tjt Add (alc) IsOffline flag.
 * 05/13/94 | tjt Fix (alc) Order number 1..6 digits right just in zc.
 * 05/10/94 | tjt Fix (configue) Allow early exit on first zone of pickline.
 * 05/12/94 | tjt Fix (configure) No data line at EOF on error message
 * 05/12/94 | tjt Fix (configure) Bay modules must be on same controller.
 * 05/25/94 | tjt Add (configure_cvrt) Usage error message.
 * 05/13/94 | tjt Fix (display_shorts) Order number 1..6 digits right just.
 * 05/25/94 | tjt Add (display_shorts) Ignore pick terxt flag.
 * 05/27/94 | tjt Fix (group_comms) Pending flag.
 * 05/16/94 | tjt Add (initialization) check_db before initialize.
 * 05/30/94 | tjt Add (loop_test) old ff diagnostics to mfc.
 * 05/12/94 | tjt Add (plc) IsOffline flag for zones.
 * 05/06/94 | tjt Fix (ofc) Restoreplace event for ff and alc.
 * 05/06/94 | tjt Add (ofc) Zone status of ZS_OFFLINE.
 * 05/10/95 | tjt Fix (ofc) Jump zone ahead and not finished show ahead.
 * 05/12/94 | tjt Add (ofc) IsOffline flag for zones.
 * 05/25/94 | tjt Add (monitor) Add ignore remarks flag.
 * 05/25/94 | tjt Add (of_init) Ignore remarks and pick text flags.
 * 05/12/94 | tjt Add (op_logon) caps_version.
 * 05/16/94 | tjt Add (operm) check_db on restoreplace.
 * 05/20/94 | tjt Fix (operm) Remove 'V', set refresh interval. 
 * 05/27/94 | tjt Add (order_comms) Pending flag.
 * 05/13/94 | tjt Fix (order_display) Order number 1..6 digits right just.
 * 05/24/94 | tjt Fix (order_entry) Transaction output (bnqy).
 * 05/10/94 | tjt Fix (order_input) No add to of_no_units on SKU bypass.
 * 05/10/94 | tjt Fix (order_input) Reject orders with no picks.
 * 05/25/94 | tjt Add (order_input) Ignore remarks flag.
 * 05/25/94 | tjt Add (order_input) Validate option -v.
 * 05/13/94 | tjt Fix (order_picks) Order number 1..6 digits left just.
 * 05/25/94 | tjt Add (order_picks) Ignore pick text flag.
 * 05/16/94 | tjt Add (order_purge) check_purge_db after purge.
 * 05/13/94 | tjt Fix (order_stat) Order number 1..6 digits left just.
 * 05/19/94 | tjt Fix (order_stat) Get status only when picks in order.
 * 05/12/94 | tjt Add (plc) IsOffline flag.
 * 05/14/94 | tjt Fix (picker_prod_reports) Order number 1..6 digits rt just
 * 05/13/94 | tjt Fix (sp_menu) Order number 1..6 digits right just.
 * 05/25/94 | tjt Add (sp_menu) Ignore remarks flag.
 * 05/24/94 | tjt Fix (ss_init) Bytes rt, ft, and eof space are null.
 * 05/16/94 | tjt Add (syscomm) check_db on restoreplace.
 * 05/13/94 | tjt Fix (sys_stat) Order number 1..6 digits right just.
 * 05/13/94 | tjt Fix (sys_stat2) Order number 1..6 digits right just.
 * 05/12/94 | tjt Add (tlc) IsOffline flag.
 * 05/12/94 | tjt Add (tlc) Case/inner pack quantities.
 * 05/04/94 | tjt Add (transac_copy) Six digit order number.
 * 05/04/94 | tjt Add (transac_review) Six digit order number.
 * 05/04/94 | tjt Add (transac_short_print) Six digit order number.
 * 05/03/94 | tjt Fix (walgreens_engine) Revised for ASCII messages.
 * 05/04/94 | tjt Add (walgreens_engine) Six digit order number.
 * 05/06/94 | tjt Add (walgreens_engine) Login and logout message.
 * 05/06/94 | tjt Add (walgreens_engine) Release order in cluster.
 * 05/12/94 | tjt Add (walgreens_engine) IsOffline flag.
 * 05/13/94 | tjt Fix (zone_stat) Order number 1..6 digits right just.
 * 05/19/94 | tjt Add (zone_stat) Login as a zone status.
 * 05/20/94 | tjt Add (zone_stat) Refresh only if picks occur.
 * 
 * 05/27/94 | Version 2.01 - installed at Hanes.
 *
 * 05/27/94 | tjt Add (order_comms)
 * 06/02/94 | tjt Fix (group_comms) Move cancel trans to pick_update_db.
 * 06/02/94 | tjt Fix (ofc) Move transactions to pick_update_db.
 * 06/02/94 | tjt Fix (order_comms) Move cancel trans to pick_update_db.
 * 06/01/94 | tjt Fix (order_input) Custom automatic order release.
 * 06/02/94 | tjt Fix (pick_update_db) Most transactions to pick_update_db.
 * 06/01/94 | tjt Fix (walgreens_engine) Remove login and logout.
 * 06/02/94 | tjt Fix (walgreens_engine) No order release in first zone.
 *
 * 06/03/94 | Version 2.02 - installed at walgreens - Houston.
 *
 * 06/23/94 | tjt Add (message_types.h) OrderChange message type.
 * 06/23/94 | tjt Add (caps_messages.h) OrderChange message.
 * 06/15/94 | tjt Add (ss.h) Order queued transaction.
 * 06/15/94 | tjt Add (ss_init.h) Order queued transaction.
 *
 * 06/20/94 | tjt Fix (Bcheck) Zero based not one based recovered pages.
 *
 * 06/14/94 | tjt Add (alc) Bays with no modules.
 * 06/14/94 | tjt Add (caps_client) Make a process group.
 * 06/08/94 | tjt Add (caps_server) Locks swtiches on death.
 * 06/14/94 | tjt Add (caps_server) Make a process group.
 * 06/22/94 | tjt Add (co_init) Severe share bug when config verification.
 * 06/23/94 | tjt Fix (loop_test) No line test before other tests.
 * 06/14/94 | tjt Add (ofc) Bays with no modules.
 * 06/15/94 | tjt Add (order_input) Pause during markplace/restoreplace.
 * 06/15/94 | tjt Add (order_input) Order queued transaction.
 * 06/15/94 | tjt Add (order_input) Options -o to display each order.
 * 06/15/94 | tjt Add (order_input) Options -g to display first in group.
 * 06/07/94 | tjt Fix (order_picks) Bug in retrieval of picks.
 * 06/07/94 | tjt Add (order_purge) Quick and super quick purges.
 * 06/20/94 | tjt Fix (order_purge) Open/close transactions on mark/restore.
 * 02/22/94 | tjt Add (order_purge) Lock picklines on death by signal.
 * 06/14/94 | tjt Add (pick_loc_anal_create) Bays with no modules.
 * 06/14/94 | tjt Add (plc) Bays with no modules.
 * 06/23/94 | tjt Fix (reconfigure_orders) UW orders left in sequence.
 * 06/14/94 | tjt Add (tlc) Bays with no modules.
 * 06/15/94 | tjt Add (transac_format_screen) Add order queued transaction.
 * 06/07/94 | tjt Fix (zone_stat) Uses pl_complete for count.
 * 06/08/94 | tjt Add (zero_counts) Clear zone picks too.
 * 06/08/94 | tjt Fix (zero_prod_counts) Do not clear error line.
 *
 * 06/23/94 | Version 2.03 - Modified at hanes. 
 *
 * 06/24/94 | tjt Fix (inhibit_enable_pm) Center last line mod of display.
 * 06/24/94 | tjt Add (pick_update_db) Order split for Walgreens.
 *
 * 06/24/94 | Version 2.04 - Modified at hanes + walgreen's split order.
 *
 * 07/07/94 | tjt Add (file_names.h) Port save map - hw_save_name.
 * 07/07/94 | tjt Add (message_types.h) InitErrorMessage.
 *
 * 07/06/94 | tjt Add (ac_softreset) New library function.
 * 07/06/94 | tjt Add (ac_download) Call to ac_soft_reset.
 * 07/06/94 | tjt Add (ac_readdress) Call to ac_soft_reset.
 *
 * 06/30/94 | tjt Fix (acd_pm_screen) Return to add sku on assign pm.
 * 06/30/94 | tjt Fix (acd_sku_screen) Display prompts on addl add sku.
 * 07/06/94 | tjt Add (alc_diag) 88 tests replaced by - and *0.
 * 07/06/94 | tjt Add (alc_diag) Call to ac_soft_reset.
 * 07/06/94 | tjt Add (alc_diag_test) 88 tests replaced by - and *0.
 * 07/06/94 | tjt Add (alc_diag_test) Call to ac_soft_reset.
 * 07/06/94 | tjt Add (alc_init) Call to ac_soft_reset.
 * 07/07/94 | tjt Add (alc_init) Firmware date of XX/XX/XX is no check.
 * 07/07/94 | tjt Add (alc_init) Error messages (-m option).
 * 07/07/94 | tjt Add (configure) Error messages (-m option).
 * 07/07/94 | tjt New (compare_hw_maps) Compare current and saved maps.
 * 06/29/94 | tjt Fix (create_output_file) Bugs in output.
 * 07/01/94 | tjt Add (diag_menu0) SKU position entry for test 'S'.
 * 07/06/94 | tjt Add (diag_menu3) 88 tests replaced by - and *0.
 * 07/06/94 | tjt Add (diag_menu3) Caller added to execl to alc_diag.
 * 07/07/94 | tjt Add (hw_init) Error messages (-m option).
 * 06/29/94 | tjt Fix (i_o_sku_data) For DOS diskettes.
 * 06/30/94 | tjt Fix (plc_diag) reduce Failure line test count to 3 errors.
 * 07/07/94 | tjt Add (plc_init) Error messages (-m option).
 * 07/01/94 | tjt Add (product_file_query) Query name added to report
 * 07/01/94 | tjt Add (pfq_report) Query name added to report
 * 07/07/94 | tjt Add (rp_init) Error messages (-m option).
 * 07/07/94 | tjt New (save_hw_maps) Program to copy port maps.
 * 06/29/94 | tjt Fix (sku_diskette_read) For DOS diskettes.
 * 06/29/94 | tjt Fix (sku_diskette_write) For DOS diskettes.
 * 06/30/94 | tjt Fix (tlc_diag) Correct bay numbering in config/bays_only.
 * 07/01/94 | tjt Add (tlc_diag) Hard reset before test 'S' to clear TC's.
 * 07/01/94 | tjt Add (tlc_diag) SKU position parm for test 'S'.
 * 07/07/94 | tjt Fix (tlc_diag) Set process group added to kill subtasks.
 * 07/07/94 | tjt Add (tlc_init) Firmware date of XX/XX/XX is no check.
 * 07/07/94 | tjt Add (tlc_init) Error messages (-m option).
 *
 * 07/08/95 | Version 2.05 - Modified at Walgreens.
 *
 * 07/10/94 | tjt Add (ss.h) Software flag sp_use_unos_print.
 * 07/10/94 | tjt Add (ss_init.h) Software flag sp_use_unos_print.
 * 07/11/94 | tjt Fix (input_editor) Page up/down corrected.
 * 07/11/94 | tjt Fix (order_input) Bug in sync() or error reject.
 * 07/10/94 | tjt Add (syscomm) Flag sp_use_unos_print for old queues.
 *
 * 07/11/94 | Version 2.06 - Installed at Kmart Brighton CO.
 *
 * 07/28/94 | tjt Add (alc) insure order number is six digits.
 * 07/15/94 | tjt Add (acd_pm_screen) Dup stkloc message shows other mod no.
 * 07/12/94 | tjt Add (diag_menu0) Set and clear busy (in_process) flag.
 * 07/12/94 | tjt Add (diag_menu1) Set and clear busy (in_process) flag.
 * 07/12/94 | tjt Add (diag_menu3) Set and clear busy (in_process) flag.
 * 07/13/94 | tjt Fix (display_shorts) bug in get_picks.
 * 07/28/94 | tjt Fix (display_shorts) order number 1..7 digits.
 * 07/28/94 | tjt Fix (hanes_engine) order_number 1..7 digits
 * 07/12/94 | tjt Add (initialization) Check busy and abort.
 * 07/29/94 | tjt Fix (ofc) Automatic feed into waiting zones.
 * 08/02/94 | tjt Add (op_init) Default printer as a parameter.
 * 07/13/94 | tjt Fix (op_logon) Password entry signal problem.
 * 07/11/94 | tjt Add (order_comms) Allow cancelled order to be repicked.
 * 07/28/94 | tjt Fix (order_display) order number 1..7 digits.
 * 07/11/94 | tjt Add (order_input) New order overlays a cancelled order.
 * 07/14/94 | tjt Fix (order_input) No printlog when silent.
 * 07/15/94 | tjt Add (order_input) Wrong PL message for missing sku.
 * 07/28/94 | tjt Fix (order_picks) order number 1..7 digits.
 * 07/28/94 | tjt Fix (order_stat) order number 1..7 digits.
 * 07/11/94 | tjt Add (pick_update_db) Remove remaining picks on cancel.
 * 07/12/94 | tjt Add (syscomm) Check busy before mark/restore place.
 * 07/28/94 | tjt Fix (sys_stat) order number 1..7 digits.
 * 07/28/94 | tjt Fix (sys_stat2) order number 1..7 digits.
 * 07/28/94 | tjt Fix (tlc) insure order number is five digits.
 * 07/28/94 | tjt Fix (transac_copy) order number 1..7 digits.
 * 07/28/94 | tjt Fix (transac_review) order number 1..7 digits.
 * 07/28/94 | tjt Fix (transac_short_print) order number 1..7 digits.
 * 07/28/94 | tjt Fix (walgreens_engine) order_number 1..7 digits
 * 07/28/94 | tjt Fix (zone_stat) order number 1..7 digits.
 *
 * 07/29/94 | Version 2.07 - Basic Function Case Picking + Eby-Brown.
 *
 * 08/03/94 | tjt Fix (configure) master bay lamp.
 * 08/04/94 | tjt Add (confm) Add bay and module number to print.
 * 08/04/94 | tjt Add (confm) Save all to config/xxx.current.
 * 08/03/94 | tjt Add (eby_engine) Engine for trans.
 * 08/04/94 | tjt Add (walgreens_engine) Add complete trans to split tote.
 * 
 * 08/04/94 | Version 2.08 - Eby-Brown installation.
 *
 * 08/08/94 | tjt Add (file_names.h) Directories dat/segs, files, & maps.
 *
 * 08/05/94 | tjt Fix (backupmm) Various bugs.
 * 08/05/94 | tjt Fix (box_packing_list) Allow only boxid w/o pl + order.
 * 08/09/94 | tjt Add (box_packing_list) putenv and chdir.
 * 08/06/94 | tjt Add (compare_hw_maps) Ignore simulated hw.
 * 08/09/94 | tjt Add (packing_list) putenv and chdir.
 * 08/06/94 | tjt Add (save_hw_maps) Ignore simulated hw.
 * 08/09/94 | tjt Add (shipping_label) putenv and chdir.
 * 08/09/94 | tjt Add (tote_label) putenv and chdir.
 * 08/09/94 | tjt Fix (transac_copy) Correct transaction file directory.
 * 08/09/94 | tjt Fix (transac_output) Correct transaction file directory.
 *
 * 08/10/94 | Version 2.09 - Hanes update.
 *
 * 08/11/94 | tjt Fix (confm) Correct zone_list ranges.
 * 08/11/94 | tjt Fix (ofc) Enable only one pick module.
 * 08/11/94 | tjt Add (plc_diag) Print of any test.
 *
 * 08/12/94 | Version 2.10 - Hanes update as installed.
 *
 * 08/24/94 | tjt Fix (alc) Add group to controller display.
 * 08/22/94 | tjt Add (alc_diag) Self tests 11 & 12.
 * 08/22/94 | tjt Add (alc_diag_test) Self tests 11 & 12.
 * 08/22/94 | tjt Add (diag_menu3) Self tests 11 & 12.
 * 08/18/94 | tjt Add (order_input) Prints errors even when silent.
 * 08/22/94 | tjt Fix (order_input) Backorder to custom.
 * 08/18/94 | tjt Fix (order_purge) Bug in default time.
 * 08/20/94 | tjt Add (plc_diag) Show HWIX with line test.
 * 08/18/94 | tjt Fix (tlc) Various bugs in conveyables, one button, cases.
 *
 * 08/26/94 | Version 2.11 - Installed at Canton.
 *
 * 08/27/94 | tjt Fix (notice_printer) Remove TTYSETSTATE of ttymodes.
 * 09/15/94 | tjt Add (alc_diags) Location database.
 * 08/29/94 | tjt Add (acd_pm_screen) Check max module value.
 * 09/15/94 | tjt Add (diag_menu3) Location database.
 * 09/08/94 | tjt Fix (display_shorts) Bugs in 'x' and 'y' options.
 * 09/08/94 | tjt Add (notice_printer) Use revised to_go value.
 * 09/28/94 | tjt Add (notice_printer) Use to_go in short queue message.
 * 09/07/94 | tjt Add (od_config) Picks to go to pickline item.
 * 09/07/94 | tjt Add (od_delete) Picks to go to pickline item.
 * 09/07/94 | tjt Add (od_repick) Picks to go to pickline item.
 * 10/04/94 | tjt Fix (ofc) Save all segments on markplace.
 * 09/08/94 | tjt Add (operm) Option 'u' (units) to remaining picks.
 * 09/28/94 | tjt Fix (operm) UP_CURSOR aborts pickline action.
 * 09/07/94 | tjt Add (order_input) Picks to go to pickline item.
 * 09/07/94 | tjt Add (order_input) Multiple picks when um in pick text.
 * 09/08/94 | tjt Add (pick_update_db) Revised remaining picks.
 * 09/28/94 | tjt Add (pick_update_db) Remaining picks to short queue item.
 * 09/28/94 | tjt Add (picker_prod_reports) Production time to now if no MP.
 * 09/10/94 | tjt Add (plc) Short picking case, packs, and units > 99.
 * 09/08/94 | tjt Add (sys_stat2) Remaining picks to pickline item.
 * 09/27/94 | tjt Fix (zero_couts) Improve speed for remainign picks.
 *
 * 10/05/94 | Version 2.12 - Installed at Canton.
 *
 * 10/10/94 | tjt Add (com_menu_in) Generic communications input.
 * 10/10/94 | tjt Add (comrecv) Generic order and sku comm input.
 * 10/10/94 | tjt Add (comsend) Generic transaction batch output.
 * 10/10/94 | tjt Add (com_trans_out) Generic transaction queue output.
 * 10/08/94 | tjt Add (configure) Allow master bay lamp w/o bay lamp.
 * 10/25/94 | tjt Fix (group_comms) Change priority without hold move.
 * 10/21/94 | tjt Fix (group_comms) Coordinated picklines.
 * 10/11/94 | tjt Fix (hanes_engine) Fix bug in enable module.
 * 10/11/94 | tjt Add (inhibit_enable_pm) Rewrite + enable all.
 * 10/20/94 | tjt Fix (ofc) Fix productivity accum when no uw.
 * 11/07/94 | tjt Add (operm) Super operator to print/purge error log.
 * 10/25/94 | tjt Fix (order_comms) Coordinated picklines.
 * 10/18/94 | tjt Add (order_input) Message on premature eof.
 * 10/25/94 | tjt Add (order_input) Pickline zero allowed.
 * 10/08/94 | tjt Add (plc) Allow master bay lamp w/o/ bay lamp.
 * 11/07/94 | tjt Add (productivity) Calculate average lines and units.
 * 10/27/94 | tjt Fix (reconfigure_orders) Leave UW orders in sequence.
 * 10/28/94 | tjt Add (sp_menu) Allow split pick unsing F8.
 * 11/01/94 | tjt Fix (sys_stat) Fix max order number.
 * 11/01/94 | tjt Fix (sys_stat2) Fix pending display and max order number.
 * 11/01/94 | tjt Fix (sys_stat3) Fix pending display and max order number.
 *
 * 11/07/94 | Version 2.13 - Distribution tape.
 *
 * 11/07/94 | tjt Add (order_input) Save report for possible reprint,
 * 11/14/94 | tjt Fix (pick_update_db) Wave done transaction.
 *
 * 11/14/94 | Version 2.14 - Distribution tape.
 *
 * 11/14/94 | tjt Add (configure) Pickline segments.
 * 11/14/94 | tjt Add (confm) Pickline segments.
 * 11/14/94 | tjt Add (co_init) Pickline segments.
 * 11/15/94 | tjt Fix (inhibit_enable_pm) Display bug.
 * 11/14/94 | tjt Add (monitor) Pickline segments.
 * 11/15/94 | tjt Add (order_input) Pickline segments.
 * 11/20/94 | tjt Add (order_purge) Purge by pickline option.
 * 11/19/94 | tjt Add (reconfigure_orders) Pickline segments.
 * 11/19/94 | tjt Add (zone_stat) Pickline segments.
 *
 * 11/21/94 | Version 3.00 - Proctor & Gamble Distribution.
 *
 * 11/24/94 | tjt Add (box_packing_list) Order and print count.
 * 11/24/94 | tjt Add (packing_list) Order and print count.
 * 11/24/94 | tjt Add (shipping_label) Order and print count.
 * 11/24/94 | tjt Add (tote_label) Order and print count.
 * 11/24/94 | tjt Add (zero_counts) Order and print count.
 *
 * 11/24/94 | Version 3.1 - SCCS Baseline System
 *
 * 12/13/94 | tjt Add (alc_suitcase) tests 4 and 5.
 * 12/13/94 | tjt Add (alc_suitcase_menu) tests 4 and 5.
 * 12/10/94 | tjt Add (order_display) Subtotal on group (G. Hawk).
 * 12/14/94 | tjt Add (order_input) check of_init failure.
 * 12/11/94 | tjt Add (prft) Recognize form feed.
 *
 * 12/12/94 | Version 3.2 - Report for Gertrude Hawk and fixes
 *
 * 12/31/94 | tjt Fix (alc) Open port only once.
 * 12/31/94 | tjt Fix (alc_in) Open port only once.
 * 01/23/95 | tjt Fix (caps_logout) Renamed from logout.
 * 12/16/94 | tjt Add (com_tran_out) Decrement transaction count.
 * 01/26/95 | tjt Fix (com_tran_out) Close/open transactions on mark/restore.
 * 01/23/95 | tjt Add (configure) Count picklines in co_pl_config.
 * 01/23/95 | tjt Add (display_shorts) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (group_comms) New IS_ONE_PICKLINE.
 * 12/15/94 | tjt Add (hw_init) Module base custom HANES.
 * 01/23/95 | tjt Add (item_move_input) New IS_ONE_PICKLINE.
 * 12/15/94 | tjt Add (configure) Module base custom HANES.
 * 12/22/94 | tjt Add (lot_control) New program.
 * 12/22/94 | tjt Add (mmenu) Lot control menu option.
 * 01/23/95 | tjt Fix (mmenu) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (operm) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Fix (op_logoff) Renamed from logoff.
 * 01/23/95 | tjt Add (order_comms) New IS_ONE_PICKLINE.
 * 12/23/94 | tjt Add (order_entry) Null Lot in transactions.
 * 12/23/94 | tjt Add (order_input) Null Lot in transactions.
 * 01/23/95 | tjt Add (order_picks) New IS_ONE_PICKLINE.
 * 01/24/95 | tjt Add (order_picks) Display lot number + pick text.
 * 01/23/95 | tjt Add (order_display) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (order_stat) New IS_ONE_PICKLINE.
 * 12/22/94 | tjt Add (pick_update_db) Lot control.
 * 12/15/94 | tjt Add (plc) Module base custom HANES.
 * 12/16/94 | tjt Fix (plc) Bug in over 100 pick setup.
 * 01/24/95 | tjt Fix (plc) Bug in UXSX4 should redisplay BLANK.
 * 01/23/95 | tjt Add (pff_inquiry_input) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (pick_loc_input) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (productivity) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (remaining_picks) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (restock_rpt_input) New IS_ONE_PICKLINE.
 * 12/31/94 | tjt Add (syscomm) Add setpgrp/sleep for logout.
 * 01/23/95 | tjt Fix (syscomm) Renamed logout and logoff.
 * 01/23/95 | tjt Add (stock_stat_input) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (stockout_input) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (test_menu) New IS_ONE_PICKLINE.
 * 12/23/94 | tjt Add (transac_format_srn) Lot split/end transaction.
 * 01/23/95 | tjt Add (transac_review) New IS_ONE_PICKLINE.
 * 12/28/94 | tjt Fix (tlc) Bug in pickline disable.
 * 12/31/94 | tjt Fix (tlc) Open port only once.
 * 12/31/94 | tjt Fix (tlc_in) Open port only once.
 * 12/28/94 | tjt Fix (tlc_init) Bug in box full initialization.
 * 01/23/95 | tjt Fix (tty_server) Add ^R for resend screen.
 * 01/23/95 | tjt Fix (tty_server) Reset wyse 50 tabs to default on close.
 * 01/23/95 | tjt Add (zero_counts) New IS_ONE_PICKLINE.
 * 01/23/95 | tjt Add (zero_move_counts) New IS_ONE_PICKLINE.
 * 01/27/95 | tjt Add (zero_prod_counts) Set current for appearance only.
 *
 * 01/27/95 | Version 4.1 - Lot Control and fixes.
 *
 * 01/30/95 | tjt Fix (acd_sku_screen) Bug in module display on delete.
 * 02/02/95 | tjt Fix (box_packing_list) Remove UNOS queues.
 * 01/27/95 | tjt Fix (co_init) Bug in view allocation.
 * 02/02/95 | tjt Fix (ofc) Remove UNOS queues.
 * 01/27/95 | tjt Add (operm) No pickline view without configured.
 * 02/02/95 | tjt Fix (packing_list) Remove UNOS queues.
 * 02/02/95 | tjt Fix (pick_update_db) Remove UNOS queues.
 * 02/02/95 | tjt Fix (shipping_label) Remove UNOS queues.
 * 02/02/95 | tjt Fix (sp_menu) Remove UNOS queues.
 * 02/02/95 | tjt Fix (tote_label) Remove UNOS queues.
 *
 * 02/14/95 | Version 4.2
 *
 * 03/16/95 | tjt Add (ac_view) Four digit pick module.
 * 05/07/95 | tjt Fix (ac_view) Cursor move to module.
 * 02/18/95 | tjt Add (alc) Port disable.
 * 03/17/95 | tjt Add (alc) ZC2, PM2, and PM4.
 * 03/23/95 | tjt Add (alc) Enable and disable diagnostics.
 * 05/24/95 | tjt Add (alc) Multiple modules in pick text. (Eby-Brown)
 * 05/25/95 | tjt Add (alc) Blink on quantity.
 * 02/18/95 | tjt Add (alc_diag) Port disable.
 * 03/23/95 | tjt Add (alc_diag) ZC2, PM2, and PM4.
 * 05/31/95 | tjt Add (alc_diag) Print errors option.
 * 06/01/95 | tjt Add (alc_diag) Fast line test.
 * 03/23/95 | tjt Add (alc_diag_test) ZC2, PM2, and PM4.
 * 05/31/95 | tjt Add (alc_diag) print errors option.
 * 02/18/95 | tjt Add (alc_in) Port disable.
 * 05/31/95 | tjt Add (alc_in) Print errors of pickline restore.
 * 03/16/95 | tjt Add (alc_init) Four digit pick module.
 * 05/31/95 | tjt Add (alc_init) Print option.
 * 03/17/95 | tjt Add (alc_init) ZC2 and PM2 modules.
 * 03/25/95 | tjt Fix (bf_sim) Revised.
 * 03/25/95 | tjt Fix (bf_view) Revised.
 * 05/07/95 | tjt Fix (bf_view) Cursor move to module.
 * 04/23/95 | tjt Add (caps_logout) order file rebuild option.
 * 03/17/95 | tjt Fix (co_init) Remove tc and ac displays.
 * 03/22/95 | tjt Fix (compare_hw_maps) ss_open missing.
 * 05/10/95 | tjt Fix (config_entry) List files display.
 * 03/16/95 | tjt Add (configure) Four digit pick module.
 * 03/17/95 | tjt Add (configure) ZC2 and PM2 modules.
 * 04/25/95 | tjt Add (configure) TC without range allowed.
 * 04/28/95 | tjt Add (configure) Dummy port type.
 * 05/18/95 | tjt Fix (configure) Revise basic function bay module check.
 * 06/17/95 | tjt Fix (configure) Add port count for validate.
 * 06/03/95 | tjt Add (confm) Pickline input by name.
 * 04/28/94 | tjt Add (create_output_file) Revised pm file output format.
 * 02/18/95 | tjt Add (diag_menu0) Port disable.
 * 06/01/95 | tjt Add (diag_menu0) Fast line test.
 * 02/18/95 | tjt Add (diag_menu1) Port disable.
 * 02/18/95 | tjt Add (diag_menu3) Port disable.
 * 06/01/95 | tjt Add (diag_menu3) Fast lien test.
 * 06/03/95 | tjt Add (display_shorts) Pickline input by name.
 * 04/28/95 | tjt Add (dlc) Dummy port lien control.
 * 05/07/95 | tjt Fix (ff_view) Cursor move to module.
 * 05/11/95 | tjt Add (group_comms) Add patterns in group names.
 * 06/03/95 | tjt Add (inhibit_enable_pm) Pickline input by name.
 * 03/16/95 | tjt Add (hw_init) Four digit pick module.
 * 03/17/95 | tjt Add (hw_init) ZC2 and PM2.
 * 04/28/95 | tjt Add (hw_init) Dummy port type.
 * 03/23/95 | tjt Add (initialization) Mark ports closed on failure.
 * 05/07/95 | tjt Add (initialization) Over 12 ports + print init report.
 * 06/03/95 | tjt Add (lot_control) Pickline input by name.
 * 06/03/95 | tjt Add (manpower_input) Pickline input by name.
 * 03/17/95 | tjt Add (monitor) new module types.
 * 03/28/95 | tjt Add (monitor) Show SKU with pick modules.
 * 06/03/95 | tjt Add (monitor) Pickline amd port input by name.
 * 02/28/95 | tjt Fix (ofc) Bug in early exit next.
 * 03/02/95 | tjt Fix (ofc) Ignore zone next when pickline disabled.
 * 04/26/95 | tjt Fix (ofc) Elapsed time for picker accountability order group.
 * 04/27/95 | tjt Fix (ofc) Productivity revised.
 * 04/28/95 | tjt Add (ofc) Dummy port type.
 * 02/18/95 | tjt Add (operm) Port disable.
 * 06/03/95 | tjt Add (operm) Pickline and port input by name.
 * 06/03/95 | tjt Add (order_comms) Pickline input by name.
 * 06/03/95 | tjt Add (order_display) Pickline input by name.
 * 04/22/95 | tjt Add (order_input) sp_order_input_purge options.
 * 06/03/95 | tjt Add (order_picks) Pickline input by name.
 * 02/29/95 | tjt Fix (order_purge) Bugs in open/close & block test.
 * 06/03/95 | tjt Add (order_stat) Pickline input by name.
 * 06/03/95 | tjt Add (pff_inquiry_input) Pickline input by name.
 * 06/03/95 | tjt Add (pick_loc_input) Pickline input by name.
 * 06/03/95 | tjt Add (pick_rate_input) Pickline input by name.
 * 04/27/95 | tjt Add (pick_update_db) Productivity lines and units.
 * 04/26/95 | tjt Fix (picker_prod_rpts) Elapsed time for order group.
 * 02/18/95 | tjt Add (plc) Port disable.
 * 03/10/95 | tjt Add (plc) Show master bay lamp on test 3.
 * 05/25/95 | tjt Add (plc) Blink on quantity.
 * 05/31/95 | tjt Add (plc) Print errors on pickline restore.
 * 06/17/95 | tjt Add (plc) Remove EE displays.
 * 05/31/95 | tjt Add (plc_init) Print errors option.
 * 04/27/95 | tjt Add (pnmm) Check productivity feature.
 * 02/24/95 | tjt Fix (productivity) Clear elapsed2 for proper calculation.
 * 04/27/95 | tjt Fix (productivity) Revised extensively.
 * 06/03/95 | tjt Add (productivity) Pickline input by name.
 * 05/26/95 | tjt Add (rap_screen) Access either SKU, PM, or stkloc.
 * 02/22/95 | tjt Fix (reconfigure_orders) remaining picks on recovery.
 * 06/03/95 | tjt Add (remaining_picks) Pickline input by name.
 * 02/24/95 | tjt Add (rp_init) Better basic function message.
 * 03/16/95 | tjt Add (rp_init) Four digit pick module.
 * 03/17/95 | tjt Add (rp_init) ZC2 and PM2.
 * 04/28/95 | tjt Add (rp_init) Dummy port type.
 * 02/24/95 | tjt Add (save_hw_maps) Port disable.
 * 02/21/95 | tjt Fix (sp_menu) Remove test of max order number.
 * 06/03/95 | tjt Add (sp_menu) Pickline input by name.
 * 02/26/95 | tjt Fix (ss_init) Initialize by parameter name.
 * 06/03/95 | tjt Add (stock_stat_input) Pickline input by name.
 * 06/03/95 | tjt Add (stockout_input) Pickline input by name.
 * 04/23/95 | tjt Add (syscomm) order file rebuild option.
 * 06/03/95 | tjt Add (sys_stat) Pickline input by name.
 * 06/03/95 | tjt Add (sys_stat2) Pickline input by name.
 * 06/03/95 | tjt Add (sys_stat3) Pickline input by name.
 * 02/18/95 | tjt Add (tlc) Port disable.
 * 02/27/95 | tjt Add (tlc) Displays to sys/tlc_text file.
 * 03/28/95 | tjt Add (tlc) Zone display messages.
 * 02/18/95 | tjt Add (tlc_diag) Port disable.
 * 05/31/95 | tjt Add (tlc_diag) Print errors option.
 * 06/01/95 | tjt Add (tlc_diag Fast line test.
 * 06/01/95 | tjt Add (tlc_diag) Print bays on 'b' hidden option.
 * 02/18/95 | tjt Add (tlc_in) Port disable.
 * 05/31/95 | tjt Add (tlc_in) Print errors on pickline restore.
 * 06/17/95 | tjt Add (tty_server) Detect wyse50 or not for delays.
 * 06/03/95 | tjt Add (trasnac_review) Pickline input by name.
 * 06/03/95 | tjt Add (zone_counts) Pickline input by name.
 * 06/03/95 | tjt Add (zone_move_counts) Pickline input by name.
 * 06/03/95 | tjt Add (zone_prod_counts) Pickline input by name.
 * 06/03/95 | tjt Add (zone_bal_input) Pickline input by name.
 * 05/07/95 | tjt Fix (zone_stat) Display forward/backward.
 *
 * 06/19/95 | Version 4.3 
 *
 * 06/21/95 | tjt Fix (alc) Blink option test.
 * 06/21/95 | tjt Fix (alc) Multiple modules.
 * 06/27/95 | tjt Fix (alc) Ignore start of inactive zone.
 * 06/21/95 | tjt Add (alc_in) Krash messsage on alc_init failure.
 * 06/23/95 | tjt Add (ff_view) Allow super picker message anytime.
 * 06/21/95 | tjt Fix (group_comms) Clear work queue in find_orders.
 * 06/21/95 | tjt Fix (monitor) ZC2 added to show_zc.
 * 06/21/95 | tjt Fix (monitor) PM2 and PM4 added to show_mod.
 * 06/21/95 | tjt Fix (monitor) Check order number.
 * 06/21/95 | tjt Fix (of_diags) Clear work queue when repairing.
 * 06/21/95 | tjt Add (ofc) Krash message on init failures.
 * 06/27/95 | tjt Fix (order_stat) Case on yn response.
 * 06/21/95 | tjt Add (plc) Krash messsage on plc_init failure.
 * 06/27/95 | tjt Fix (plc) Ignore start of inactive zone.
 * 06/27/95 | tjt Fix (sys_stat) Use GroupLength for pnd_group.
 * 06/27/95 | tjt Fix (sys_stat2) Use GroupLength for pnd_group.
 * 06/27/95 | tjt Fix (sys_stat3) Use GroupLength for pnd_group.
 * 06/27/95 | tjt Fix (tlc) Ignore start of inactive zone.
 * 06/21/95 | tjt Add (tlc_in) Krash messsage on tlc_init failure.
 * 06/20/95 | tjt Fix (zone_stat) Allocation of tab and end display.
 *
 * 06/27/95 | Version 4.4
 *
 * 07/23/95 | tjt Fix (ac_view) Modify Bard calls.
 * 07/23/95 | tjt Fix (acd_pm_screen) Modify Bard calls.
 * 07/23/95 | tjt Fix (ac_sku_screen) Modify Bard calls.
 * 06/28/95 | tjt Fix (alc) Disable port swtiches.
 * 07/18/95 | tjt Fix (alc) Ignore zero pick quantity.
 * 07/23/95 | tjt Fix (alc) Modify Bard calls.
 * 07/23/95 | tjt Fix (alc_diag) Modify Bard calls.
 * 06/26/95 | tjt Fix (ansi_server) Modify typeahead.
 * 07/23/95 | tjt Fix (batch_receipts) Modify Bard calls.
 * 07/23/95 | tjt Fix (bf_view) Modify Bard calls.
 * 07/23/95 | tjt Fix (box_packing_list) Modify Bard calls.
 * 07/23/95 | tjt Fix (caps_client) Modify Bard calls.
 * 07/23/95 | tjt Fix (change_op_info) Modify Bard calls.
 * 07/12/95 | tjt Add (comsend) UNIX set raw mode.
 * 07/14/95 | tjt Add (comsend) Optional file name parameter.
 * 07/14/95 | tjt Fix (com_menu_in) Stop commo.
 * 07/12/95 | tjt Add (com_tran_out) UNIX set raw mode.
 * 07/23/95 | tjt Fix (com_tran_out) Modify Bard calls.
 * 06/29/95 | tjt Add (create_orders) Without order numbers.
 * 07/23/95 | tjt Fix (create_output) Modify Bard calls.
 * 07/23/95 | tjt Fix (delete_pm_screen) Modify Bard calls.
 * 07/27/95 | tjt Fix (tlc_diag) Config display by port.
 * 07/23/95 | tjt Fix (display_shorts) Modify Bard calls.
 * 07/23/95 | tjt Fix (end_day_maint) Modify Bard calls.
 * 07/23/95 | tjt Fix (ff_view) Modify Bard calls.
 * 07/23/95 | tjt Fix (item_move_create) Modify Bard calls.
 * 07/23/95 | tjt Fix (lot_control) Modify Bard calls.
 * 07/05/95 | tjt Fix (manpower_input) Directory name of pick_rates.
 * 07/23/95 | tjt Fix (manpower_rqmt) Modify Bard calls.
 * 07/21/95 | tjt Add (monitor) Revise Bard calls.
 * 07/23/95 | tjt Fix (notice_printer) Modify Bard calls.
 * 07/25/95 | tjt Fix (notice_printer) Short format #3 for Williams-Sonoma.
 * 07/25/95 | tjt Fix (notice_printer) Sku support not required for shorts.
 * 07/21/95 | tjt Add (od_close) Revise Bard calls.
 * 06/30/95 | tjt Add (od_config) Lookup SKU for changed module.
 * 07/01/95 | tjt Add (od_config) Accumulate remaining picks.
 * 07/02/95 | tjt Add (od_config) Check module range.
 * 07/02/95 | tjt Add (od_config) Flag orders with orphans. 
 * 07/02/95 | tjt Add (od_config) Flag orders with inhibitted picks.. 
 * 07/02/95 | tjt Add (od_config) Flag orders with orphans. 
 * 07/21/95 | tjt Add (od_config) Revise Bard calls.
 * 07/21/95 | tjt Add (od_delete) Revise Bard calls.
 * 07/21/95 | tjt Add (od_get) Revise Bard calls.
 * 07/25/95 | tjt Add (od_move) Move order to another pickline.
 * 07/21/95 | tjt Add (od_open) Revise Bard calls.
 * 06/30/95 | tjt Add (od_repick) Lookup SKU for changed module.
 * 07/01/95 | tjt Add (od_repick) Accumulate remaining picks.
 * 07/02/95 | tjt Add (od_repick) Flag orders with orphans. 
 * 07/02/95 | tjt Add (od_repick) Flag orders with inhibitted picks.. 
 * 07/21/95 | tjt Add (od_repick) Revise Bard calls.
 * 07/21/95 | tjt Add (od_status) Revise Bard calls.
 * 07/21/95 | tjt Add (od_update) Revise Bard calls.
 * 07/23/95 | tjt Fix (occ) Modify Bard calls.
 * 07/21/95 | tjt Add (of_diags) Revise Bard calls.
 * 06/29/95 | tjt Add (of_init) Customer order number as a key.
 * 07/23/95 | tjt Fix (op_logon) Modify Bard calls.
 * 06/30/95 | tjt Add (operm) Apply sku changes on restoreplace.
 * 06/29/95 | tjt Add (order_comms) Customer order number query.
 * 06/30/95 | tjt Add (order_comms) Apply sku changes to repicks.
 * 07/23/95 | tjt Fix (order_comms) Modify Bard calls.
 * 06/29/95 | tjt Add (order_display) Customer order number query.
 * 07/25/95 | tjt Add (order_display) Increase display lines.
 * 07/23/95 | tjt Fix (order_display) Modify Bard calls.
 * 08/03/95 | tjt Add (order_entry) Allow priority 'k'.
 * 08/03/95 | tjt Add (order_entry) Allow pickline 0.
 * 06/29/95 | tjt Add (order_input) Generate order numbers.
 * 07/01/95 | tjt Add (order_input) Accumulate remaining picks always.
 * 07/02/95 | tjt Add (order_input) Hold orders with inhibited picks.
 * 07/02/95 | tjt Add (order_input) Hold orders with bad sku. (not pl zero).
 * 07/02/95 | tjt Add (order_input) Flag orders orphan picks.
 * 07/02/95 | tjt Add (order_input) Flag orders with inhibited picks.
 * 07/23/95 | tjt Fix (order_input) Modify Bard calls.
 * 07/13/95 | tjt Add (order_input_menu) Check device conflict.
 * 07/14/95 | tjt Add (order_input_menu) Check commo feature is 'n'.
 * 07/14/95 | tjt Add (order_input_menu) Kermit option.
 * 06/29/95 | tjt Add (order_picks) Customer order number query.
 * 07/23/95 | tjt Fix (order_picks) Modify Bard calls.
 * 06/29/95 | tjt Add (order_stat) Customer order number query.
 * 07/23/95 | tjt Fix (order_stat) Modify Bard calls.
 * 08/03/95 | tjt Add (pfead) Allow queries when running.
 * 07/23/95 | tjt Fix (pff_inquiry_create) Modify Bard calls.
 * 07/23/95 | tjt Fix (pff_inquiry_input) Modify Bard calls.
 * 07/23/95 | tjt Fix (pick_loc_create) Modify Bard calls.
 * 07/01/95 | tjt Add (pick_update_db) Accumlate remaining picks always.
 * 07/23/95 | tjt Fix (pick_update_db) Modify Bard calls.
 * 07/25/95 | tjt Fix (pick_update_db) Option for pick text for short tickets.
 * 07/25/95 | tjt Fix (pick_update_db) Use stkloc for remaining picks.
 * 07/23/95 | tjt Fix (picker_db_clear) Modify Bard calls.
 * 07/23/95 | tjt Fix (picker_information) Modify Bard calls.
 * 07/23/95 | tjt Fix (picker_prod_reports) Modify Bard calls.
 * 07/23/95 | tjt Fix (picker_zero_counts) Modify Bard calls.
 * 07/18/95 | tjt Fix (plc) Ignore zero pick quantity.
 * 07/23/95 | tjt Fix (plc) Modify Bard calls.
 * 08/03/95 | tjt Fix (prft) Ignore HOF at start of print file.
 * 07/21/95 | tjt Add (print_queue) Revise Bard calls.
 * 07/23/95 | tjt Fix (rap_screen) Modify Bard calls.
 * 06/30/95 | tjt Add (reconfigure_orders) Apply sku changes.
 * 07/01/95 | tjt Add (reconfigure_orders) Accumulate remining picks.
 * 07/01/95 | tjt Add (reconfigure_orders) Reconfigure hold queue.
 * 07/23/95 | tjt Add (reconfigure_orders) Modify Bard calls.
 * 07/23/95 | tjt Add (recover_orders) Modify Bard calls.
 * 07/23/95 | tjt Add (remaining_picks) Modify Bard calls.
 * 07/23/95 | tjt Add (restock_rpt_create) Modify Bard calls.
 * 07/23/95 | tjt Add (rev_xactions) Modify Bard calls.
 * 07/11/95 | tjt Add (sd_input) Krash on bad field length.
 * 06/30/95 | tjt Add (st_init) Check any sku table changes.
 * 07/23/95 | tjt Add (shipping_label) Modify Bard calls.
 * 07/23/95 | tjt Add (sp_menu) Modify Bard calls.
 * 07/23/95 | tjt Add (st_init) Modify Bard calls.
 * 07/23/95 | tjt Add (stockout_create) Modify Bard calls.
 * 06/30/95 | tjt Add (syscomm) Apply sku changes on restoreplace.
 * 06/28/95 | tjt Fix (sys_stat) Show disabled port/pickline.
 * 07/23/95 | tjt Add (sys_stat) Modify Bard calls.
 * 06/28/95 | tjt Fix (sys_stat2) Show disabled port/pickline.
 * 07/23/95 | tjt Add (sys_stat2) Modify Bard calls.
 * 06/28/95 | tjt Fix (sys_stat3) Show disabled port/pickline.
 * 07/23/95 | tjt Add (sys_stat3) Modify Bard calls.
 * 06/30/95 | tjt Add (sys_stat4) Show customer order number.
 * 07/23/95 | tjt Add (sys_stat4) Modify Bard calls.
 * 08/01/95 | tjt Add (test_menu) Check is full function pickline.
 * 08/01/95 | tjt Add (test_menu) Check is full function zone.
 * 06/28/95 | tjt Fix (tlc) Disable port swtiches.
 * 07/18/95 | tjt Fix (tlc) Ignore zero pick quantity.
 * 07/23/95 | tjt Add (tlc) Modify Bard calls.
 * 07/23/95 | tjt Add (tlc_diag) Modify Bard calls.
 * 07/23/95 | tjt Add (tote_label) Modify Bard calls.
 * 07/23/95 | tjt Add (transac_copy) Modify Bard calls.
 * 07/13/95 | tjt Fix (transac_output) Check device conflict.
 * 07/14/93 | tjt Add (transac_output) Kermit output via kermon.
 * 07/14/95 | tjt Add (transac_output) Check transaction file feature.
 * 07/14/95 | tjt Fix (transac_output) Retransmit message.
 * 07/23/95 | tjt Add (transac_output) Modify Bard calls.
 * 06/30/95 | tjt Add (transac_review) Customer order number query.
 * 07/23/95 | tjt Add (transac_review) Modify Bard calls.
 * 07/23/95 | tjt Add (transac_short_report) Modify Bard calls.
 * 07/04/95 | tjt Fix (tty_server) Remove type ahead.
 * 07/21/95 | tjt Add (xt_build) Revise Bard calls.
 * 07/21/95 | tjt Add (xt_close) Revise Bard calls.
 * 07/21/95 | tjt Add (xt_open) Revise Bard calls.
 * 07/23/95 | tjt Add (zero_counts) Modify Bard calls.
 * 07/23/95 | tjt Add (zero_mov_counts) Modify Bard calls.
 * 07/07/95 | tjt Fix (zone_bal_input) Cummulalative prompt buffer.
 * 07/05/95 | tjt Fix (zone_balancing) Configuration output.
 * 07/23/95 | tjt Add (zone_stat) Modify Bard calls.
 *
 * 08/28/95 | Version 4.5
 *
 * 09/15/95 | tjt Add (configure) Check box feature with BOXFULL.
 * 09/21/95 | tjt Add (configure) Check box feature with BOXFULL.
 * 08/29/95 | tjt Fix (daytimer_scan) Revise oc lock and unlock.
 * 09/21/95 | tjt Fix (diag_menu) Always check port number.
 * 09/24/95 | tjt Fix (item_move_create) Minor bug in fread.
 * 09/23/95 | tjt Add (monitor) Box close transaction.
 * 09/08/95 | tjt Fix (order_picks) Display canceled status.
 * 09/22/95 | tjt Fix (order_picks) Up arrow in get_parms gives bad block.
 * 09/08/95 | tjt Fix (order_stat) Display canceled status.
 * 08/31/95 | tjt Add (order_unload) Unload only changes option.
 * 09/21/95 | tjt Add (plc) Fast zone display on first pick.
 * 08/21/95 | tjt Add (plc) Faster blink. Also different on/off rates.
 * 08/31/95 | tjt Add (reconfigure_orders) Unload only changes.
 * 08/28/95 | tjt Fix (restock_rpt_create) Check for division by zero.
 * 09/24/95 | tjt Add (rp_init) Mask box full bit on PM.
 * 08/31/95 | tjt Fix (st_init) Scan orders for st_change condition.
 * 08/30/95 | tjt Fix (stockout_create) Add use of hold queue option.
 * 09/23/95 | tjt Add (transac_format_srn) Box close transaction.
 * 08/29/95 | tjt Fix (ws_scan) Revise oc lock and unlock.
 *
 * 09/25/95 | Version 4.6 - Cosmair Release
 *
 * 10/02/95 | tjt Fix (database_report) Bug in fseek.
 * 10/02/95 | tjt Add (ss_init) Revised for PC.
 * 10/02/95 | tjt Add (ss_dump) Revised for PC.
 * 10/02/95 | tjt Add (item_move_create) Use 'sort' for PC.
 * 10/02/95 | tjt Add (pick_loc_create) Use 'sort' for PC.
 * 10/02/95 | tjt Add (pff_inquiry_create) Use 'sort' for PC.
 * 09/29/95 | tjt Add (order_input) Match option of held orders.
 *
 * 10/16/95 | Version 4.7 - Converion to PC
 *
 * 03/12/96 | tjt Add (ansi_server) catch SIGHUP etc. to exit.
 * 03/12/96 | tjt Add (ansi_server2) catch SIGHUP etc. to exit.
 * 03/08/96 | tjt Add (order_picks) datetime of last action.
 * 03/11/96 | tjt Add (order_purge) markplace or disable option.
 * 03/08/96 | tjt Add (order_stat) datetime of last action.
 * 03/06/96 | tjt Add (picker_prod_rpts) suppress zero entries option.
 * 03/10/96 | tjt Fix (sp_menu) features parms checking.
 * 03/12/96 | tjt Add (sd_open) catch SIGHUP etc. to exit.
 * 03/12/96 | tjt Add (tty_server) catch SIGHUP etc. to exit.
 *
 * 10/12/96 | Version 4.8 - Dell Phase II Test.
 *
 * 04/16/96 | tjt Fix (acd_pm_screen) Revise hw_case and hw_pack.
 * 04/16/96 | tjt Fix (acd_sku_screen) Revise hw_case and hw_pack.
 * 04/16/96 | tjt Fix (alc) for Revise hw and pw structures.
 * 04/19/96 | tjt Add (bax_packing_list) Printed count.
 * 04/19/96 | tjt Add (caps_client) Add sp_to_xmit count.
 * 04/16/96 | tjt Fix (co_init) Allocate pw by products.
 * 04/19/96 | tjt Add (com_tran_out) Add sp_to_xmit count.
 * 04/16/96 | tjt Fix (configure) Remove cluster CZ specification.
 * 04/16/96 | tjt Fix (confm) Remove cluster zones.
 * 04/18/96 | tjt Fix (create_db) Merge Bard and Informix versions.
 * 04/16/96 | tjt Fix (create_output_file) Bug in flcose at line 98.
 * 04/16/96 | tjt Fix (dlc) Revise hw and pw.
 * 04/18/96 | tjt Fix (inhibit_enable_pm) Revise hw and pw.
 * 04/18/96 | tjt Fix (manpower_rqmts) Revise hw and pw.
 * 04/19/96 | tjt Add (message_view) Add to mfc. Clear log count on purge.
 * 04/18/96 | tjt Fix (monitor) Revise hw and pw.
 * 04/16/96 | tjt Fix (od_config) Revise hw and pw.
 * 04/16/96 | tjt Fix (od_delete) Revise hw and pw.
 * 04/16/96 | tjt Fix (od_move) Revise hw and pw.
 * 04/16/96 | tjt Fix (od_repick) Revise hw and pw.
 * 04/16/96 | tjt Fix (ofc) Remove cluter zone. Revise hw and pw.
 * 04/17/96 | tjt Fix (order_input) Revise to_to. Revise hw and pw.
 * 04/18/96 | tjt Fix (order_picks) Revise hw an pw.
 * 04/17/96 | tjt Fix (order_unload) Revise to_go.
 * 04/17/96 | tjt Fix (order_picks) Revise to_go.
 * 04/17/96 | tjt Fix (orphan_picks) Revise to_go.
 * 04/19/96 | tjt Fix (packing_list) Add print count.
 * 04/19/96 | tjt Fix (pick_update_db) Revise hw and pw. New label counts.
 * 04/17/96 | tjt Fix (reconfigure_orders) Revise to_go.
 * 04/19/96 | tjt Add (shipping_label) Add print count.
 * 04/19/96 | tjt Add (sp_menu) Add label counts.
 * 04/17/96 | tjt Fix (st_init) Revise case, pack and inhibit.
 * 04/19/96 | tjt Fix (sys_stat) Add sp_to_xmit. New shot count names.
 * 04/19/96 | tjt Fix (sys_stat1) Add sp_to_xmit. New shot count names.
 * 04/19/96 | tjt Fix (sys_stat2) Add sp_to_xmit. New shot count names.
 * 04/19/96 | tjt Fix (sys_stat3) Add sp_to_xmit. New shot count names.
 * 04/19/96 | tjt Fix (sys_stat4) Add sp_to_xmit. New shot count names.
 * 04/17/96 | tjt Fix (tlc) Revise hw and pw.
 * 04/19/96 | tjt Add (tote_label) Add print count.
 * 04/17/96 | tjt Fix (zone_balancing) Remove cluster zones.
 * 04/19/96 | tjt Fix (zero_counts) Clear all label counts.
 * 05/01/96 | tjt Extensive revisions to MFC code for matrix and light
 *                bar picking.  Put configurations also supported.
 *                This version supports both Informix and Bard!  Also,
 *                runs or either the PC or CRDS.
 *
 * 05/01/96 | Version 6.0 - Gibson Greetings - Multibin Picking
 *
 * 05/20/96 | tjt Add (ac_view) Add box full module.
 * 05/26/96 | tjt Add (alc) Add box full module.
 * 05/26/96 | tjt Add (alc_diag) Add box full module.
 * 05/26/96 | tjt Add (alc_diag_test) Make hwix 1 .. based.
 * 05/20/96 | tjt Add (alc_init) Add box full module.
 * 05/20/96 | tjt Add (compare_hw_maps) Add box full module.
 * 05/26/96 | tjt Add (configure) Add box full module for total function.
 * 05/26/96 | tjt Add (confm) Add box full module.
 * 05/20/96 | tjt Add (hw_init) Add box full module.
 * 05/20/96 | tjt Add (initialization) Add box full module.
 * 05/20/96 | tjt Add (monitor) Add box full module.
 * 05/26/96 | tjt Add (pick_update_db) Add box close transaction.
 * 05/20/96 | tjt Add (rp_init) Add box full module.
 * 05/25/96 | tjt Add (select_comm) Ignore group when zero length.
 *
 * 05/27/96 | Version 6.1 - Frito-Lay - Box Full
 *
 * 06/28/96 | tjt Add (ac_view) Recognize box full button.
 * 08/23/96 | tjt Add (ac_view) Add begin and commit work.
 * 06/28/96 | tjt Fix (alc) Buf in zone_clear of module flags.
 * 06/28/96 | tjt Fix (alc) Done message in zone contoller for matrix.
 * 08/23/96 | tjt Add (alc_diag) Add begin and commit work.
 * 07/05/96 | tjt Add (configure) Promote HasBoxFull flag to zone above bay.
 * 07/08/96 | tjt Add (configure) Allow early exit in branched picklines.
 * 08/23/96 | tjt Add (batch_recipts_srn) Add commit work.
 * 08/23/96 | tjt Add (bf_view) Add begin and commit work.
 * 08/23/96 | tjt Add (com_tran_out) Add begin and commit work.
 * 08/23/96 | tjt Add (create_output_file) Add begin and commit work.
 * 07/14/96 | tjt Fix (diagnostics) Hard drive space display.
 * 08/23/96 | tjt Add (display_shorts) Add begin and commit work.
 * 08/23/96 | tjt Add (item_move_create) Add begin and commit work.
 * 08/23/96 | tjt Add (lot_control) Add begin and commit work.
 * 07/14/96 | tjt Fix (manpower_input) Display of Hirs:Min.
 * 08/23/96 | tjt Add (manpower_rqmt) Add begin and commit work.
 * 08/23/96 | tjt Add (notice_printer) Add begin and commit work.
 * 07/07/96 | tjt Add (ofc) Check SKU support too for st_init.
 * 07/08/96 | tjt Add (ofc) Early exit from branched picklines.
 * 07/26/96 | tjt Add (ofc) Check early exit range.
 * 07/31/96 | tjt Fix (ofc) Use strcpy in rediplay error message.
 * 07/31/96 | tjt Fix (operm) Clear redisplay flag on error message.
 * 07/12/96 | tjt Fix (op_logon) Change system to execl on get password
 * 08/23/96 | tjt Add (op_logon) Add begin and commit work.
 * 07/31/96 | tjt Fix (order_display) Remove unprinted temp file.
 * 08/23/96 | tjt Add (order_display) Add begin and commit work.
 * 07/05/96 | tjt Fix (order_input) Add commit_work to various errors.
 * 07/31/96 | jib Fix (order_input) Change while of if(wipeout).
 * 07/26/96 | tjt Add (order_picks) Adjust remining picks on change quan.
 * 07/26/96 | tjt Fix (order_picks) Change qunatity no allowed when underway.
 * 08/23/96 | tjt Add (order_picks) Add begin and commit work.
 * 07/05/96 | tjt Fix (order_stat) Set order number length to rf_on value.
 * 08/23/96 | tjt Add (order_stat) Add begin and commit work.
 * 08/23/96 | tjt Add (order_unload) Add begin and commit work.
 * 08/23/96 | tjt Add (orphan_picks) Add begin and commit work.
 * 07/05/96 | tjt Fix (operm) Add zone range when no pickline or zone.
 * 08/23/96 | tjt Add (pff_inquiry_create) Add begin and commit work.
 * 08/23/96 | tjt Add (pff_inquiry_input) Add begin and commit work.
 * 08/23/96 | tjt Add (pick_loc_create) Add begin and commit work.
 * 07/10/96 | tjt Fix (pick_update_db) Clear have_header flag in box full.
 * 08/23/96 | tjt Add (picker_information) Add begin and commit work.
 * 08/23/96 | tjt Add (picker_prod_rpts) Add begin and commit work.
 * 07/10/96 | tjt Fix (pfq_report) Fix comment line.
 * 08/23/96 | tjt Add (recover_orders) Add begin and commit work.
 * 08/23/96 | tjt Add (remining picks) Add begin and commit work.
 * 07/10/96 | tjt Fix (space_fill) Replace strlen with loop when no null byte.
 * 08/23/96 | tjt Add (st_init) Add begin and commit work.
 * 08/23/96 | tjt Add (stockout_create) Add begin and commit work.
 * 07/14/96 | tjt Add (syscomm) Insert 'are' in 'There orders in the system'.
 * 08/23/96 | tjt Add (sys_stat) Add begin and commit work.
 * 08/23/96 | tjt Add (sys_stat1) Add begin and commit work.
 * 08/23/96 | tjt Add (sys_stat2) Add begin and commit work.
 * 08/23/96 | tjt Add (sys_stat3) Add begin and commit work.
 * 08/23/96 | tjt Add (sys_stat4) Add begin and commit work.
 * 08/23/96 | tjt Add (tlc) Add oi_con to zone controller display.
 * 08/23/96 | tjt Add (tlc_diag) Add begin and commit work.
 * 08/23/96 | tjt Add (transac_review) Add begin and commit work.
 * 08/23/96 | tjt Add (transac_short_rpt) Add begin and commit work.
 * 08/23/96 | tjt Add (zero_counts) Add begin and commit work.
 * 06/28/96 | tjt Add (zone_balancing) Allow inverted zone numbering.
 * 08/23/96 | tjt Add (zone_balancing) Add begin and commit work.
 * 08/23/96 | tjt Add (zone_stat) Add begin and commit work.
 *
 * 09/12/96 | Version 6.2 - September 12, 1996.
 *
 * 01/07/97 | tjt Add (ac_view) Add PM6.
 * 01/07/97 | tjt Add (acd_pm_sreen) Increase length of display field.
 * 12/18/97 | tjt Add (acd_pm_sreen) Allocation field added to pmfile.
 * 01/07/97 | tjt Add (alc) New 6 digit module PM6.
 * 01/07/97 | tjt Add (alc_diag) Add PM6.
 * 01/07/97 | tjt Add (alc_diag_test) Add PM6.
 * 01/07/97 | tjt Add (alc_init) Add PM6.
 * 01/07/97 | tjt Add (compare_hw_maps) Add PM6.
 * 01/07/97 | tjt Add (configure) Add PM6.
 * 01/07/97 | tjt Add (hw_init) Add PM6.
 * 01/17/97 | tjt Add (initialization) Add PM6
 * 01/11/97 | tjt Add (of_init) Store init time as purge date and time.
 * 01/11/97 | tjt Add (of_init_bard) Store init time as purge date and time.
 * 01/08/97 | tjt Add (ofc) Allow late entry and jump zone into segments.
 * 01/09/97 | tjt Fix (ofc) Correct late entry processing.
 * 01/09/97 | tjt Add (ofc) Optional merge of parallel orders in sequence.
 * 01/11/97 | tjt Add (order_purge) Save date and time purge completed.
 * 01/07/97 | tjt Add (rp_init) Add PM6.
 * 01/07/97 | tjt Add (st_init) Add 4 character display field.
 * 01/11/97 | tjt Add (sys_stat) Show last purge date and time.
 * 01/11/97 | tjt Add (sys_stat1) Show last purge date and time.
 * 01/11/97 | tjt Add (sys_stat2) Show last purge date and time.
 * 01/11/97 | tjt Add (sys_stat4) Show last purge date and time.
 *
 * 01/12/97   Dell Module 11 Version 6.3
 *
 * 02/02/97 | tjt Add (ac_view) Pick indicators.
 * 03/21/97 | tjt Add (ac_view) MultiModule.
 * 02/04/97 | tjt Add (acd_sku_screen) Cases and packs to 15,000.
 * 02/02/97 | tjt Add (alc) Pick inidicators.
 * 03/21/97 | tjt Add (alc_diag) MultiModule.
 * 02/05/97 | tjt Add (alc_init) Pick indicators.
 * 03/21/97 | tjt Add (alc_init) MultiModule.
 * 03/21/97 | tjt Add (compare_hw_maps) MultiModule.
 * 02/02/97 | tjt Add (configure) Pick indicators.
 * 02/14/97 | tjt Add (configure) Demand Feed Induction.
 * 03/21/97 | tjt Add (configure) MultiModule.
 * 03/21/97 | tjt Add (config_select) MultiModule.
 * 02/03/97 | tjt Add (confm) Pick indicators.
 * 02/14/97 | tjt Add (confm) Demand Feed Induction.
 * 03/21/97 | tjt Add (confm) MultiModule.
 * 02/02/97 | tjt Add (hw_init) Pick indicators.
 * 03/21/97 | tjt Add (hw_init) MultiModule.
 * 03/21/97 | tjt Add (initialization) MultiModule.
 * 02/02/97 | tjt Add (monitor) Pick indicators.
 * 02/14/97 | tjt Add (monitor) Demand Feed Induction.
 * 03/21/97 | tjt Add (monitor) MultiModule.
 * 02/14/97 | tjt Add (zone_stat) Demand Feed Induction.
 * 06/15/01 | aha Modified for Eckerd's Manpower Planning & Zone Balancing.
 *          |     Version 6.5.E
 * 10/01/01 | aha Modified for Eckerd's Tote Integrity. Version 6.6.E
 * 03/21/02 | aha Modified picker productivity reports per Eckerd.
 * 03/29/02 | aha Modified per Tote Integrity Addendum for FIFO order flow.
 * 06/12/02 | aha Modified for cancel orders.
 * 09/06/02 | aha Modified for duplicate box numbers.
 * 02/27/03 | aha Added various fixes for Eckerd.
 * 04/09/03 | aha Added fixes for "Speed Issue" problem at Eckerd.
 * 08/06/03 | aha Added fix for group command at Eckerd.
 *
 *-------------------------------------------------------------------------*/
#include "caps_copyright.h"
#ifndef CAPS_VERSION_H
#define CAPS_VERSION_H

static char caps_version_h[] = "%Z% %M% %I% (%G% - %U%)";

unsigned char caps_version[] = "August 6, 2003 - Version 6.6.6.E" ;

#endif

/* end of caps_version.h */
