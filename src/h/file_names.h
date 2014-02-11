/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    CAPS File Names.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/23/93   |  tjt  Original implementation.
 *  07/07/94   |  tjt  Add hw_save_name.
 *-------------------------------------------------------------------------*/
#ifndef FILE_NAMES_H
#define FILE_NAMES_H

static char file_names_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Files in directory $HOME/sys are relatively static but may be
 *  tailored for a specific user.
 *
 *  Directory $HOME/sys/report contains report heading files.  
 *  Directory $HOME/sys/firmware contains basic function firmware.
 *-------------------------------------------------------------------------*/
 
#define com_suffix_name "sys/com_suffix"  /* communications prog name suffix */
#define dotinit_name    "sys/dotinit"     /* used by logon for environemnt   */
#define eh_mess_name    "sys/eh_mess"     /* error message text              */
#define fl_text_name    "sys/fl_text"     /* text of customer menu options   */
#define fl_table_name   "sys/fl_table"    /* compressed customer menu options*/
#define motd_name       "sys/motd"        /* used by op_logon for banner     */
#define rf_text_name    "sys/rf_text"     /* default record format parms     */
#define sp_text_name    "sys/sp_text"     /* default system parms for ss_init*/

#define firmdate_name   "sys/firmware/firmdate"/* current firmware date      */
#define firmware_name   "sys/firmware/tc00"/* current firmware file          */
#define suitcase_name   "dev/suitcase"    /* basic function suitcase         */

#define alc_firmdate_name "sys/alc/firmdate" /* current firmware date        */
#define alc_firmware_name "sys/alc/ac00"  /* current firmware file           */
#define alc_suitcase_name "dev/alc_suitcase"/* area controller suitcase      */

/*-------------------------------------------------------------------------*
 *  Files in directory $HOME/dat are live, current data.
 *
 *  Directory $HOME/dat/log            contains log and error files.
 *  Directory $HOME/dat/ec             contains event counts.
 *  Directory $HOME/dat/numbers        contains counters.
 *  Directory $HOME/dat/queues         contains unos queues.
 *  Directory $HOME/dat/pick_rate      contains binary pick rate files
 *  Directory $HOME/dat/pick_rate_text contains pick rate text files
 *  Directory $HOME/dat/query          contains user SQL query script
 *-------------------------------------------------------------------------*/
 
#define co_name         "dat/segs/co"     /* configuration shared segment    */
#define oc_name         "dat/segs/oc"     /* order index shared segment      */
#define pr_name         "dat/segs/pr"     /* productivity shared segment     */
#define ss_name         "dat/segs/ss"     /* system shared segment           */

#define brf_name        "dat/files/brf"   /* batch receipts trans file       */
#define imt_name        "dat/files/imt"   /* item movement file              */
#define ppf_name        "dat/files/ppf"   /* product pending trans file      */
#define ptf_name        "dat/files/ptf"   /* product transaction file        */
#define zt_name         "dat/files/zt"    /* transaction file xmit copy      */

#define hw_name         "dat/maps/hw_map" /* port hardware map               */
#define hw_save_name    "dat/maps/hw_save_map"

#define pf_data_name    "dat/files/pf_data"/* files for i_o_sku_data etc.    */
#define pm_data_name    "dat/files/pm_data"
#define sku_batch_name  "dat/files/sku_batch"

#define batch_no_name   "dat/number/batch_no"
#define restock_no_name "dat/number/restock_no"

#define eh_err_name "dat/log/caps_errlog" /* CAPS message log                */
#define errlog_name "dat/log/errlog"      /* crash and errlog messages       */

#define box_packing_list "dat/queues/box_packing_list.q"
#define packing_list     "dat/queues/packing_list.q"
#define shipping_label   "dat/queues/shipping_label.q"
#define tote_label       "dat/queues/tote_label.q"
 
#define pick_rate        "dat/pick_rate"  /* pick rate files                 */
#define pick_rate_text   "dat/pick_rate_text"/* pick rate text files         */

#define query_name       "dat/query"      /* SQL query files                 */

#endif

/* end of file_names.h */
