/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    INFORMIX Equates.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/06/95   |  tjt  Revised for INFORMIX.
 *  04/18/96   |  tjt  Revised for Bard & Informix compiles.
 *  08/03/99   |  Informix to Oracle conversion by Ravi Jagannathan
 *-------------------------------------------------------------------------*/
#ifndef ORACLE_DEFINES
#define ORACLE_DEFINES

static char informix_defines_h[] = "%Z% %M% %I% (%G% - %U%)";

#define  NOLOCK         0                /* database open modes             */
#define  AUTOLOCK       1
#define  MANLOCK        1
#define  LOCK           1
#define  READONLY       0

extern long boxes_fd;
extern long box_list_fd;
extern long location_fd;
extern long log_fd;
extern long lot_fd;
extern long operator_fd;
extern long order_fd;
extern long order_scan_fd;
extern long packing_list_fd;
extern long pending_fd;
extern long picker_fd;
extern long picker_order_fd;
extern long pick_fd;
extern long pmfile_fd;
extern long prodfile_fd;
extern long queue_fd;
extern long remarks_fd;
extern long restock_fd;
extern long ship_label_fd;
extern long short_fd;
extern long tote_label_fd;
extern long transaction_fd;

/*-------------------------------------------------------------------------*
 *  Bard Global File Equates
 *-------------------------------------------------------------------------*/
#define database_open()           database_open_o()
#define database_close()          database_close_o()
#define database_commit()         database_commit_o()
#define commit_work()             commit_work_o()
#define begin_work()              begin_work_o()

#define box_list_open(x)          box_list_open_o(x)
#define box_list_close()          box_list_close_o()
#define box_list_setkey(x)        box_list_setkey_o(x)
#define box_list_startkey(x)      box_list_startkey_o(x)
#define box_list_stopkey(x)       box_list_stopkey_o(x)
#define box_list_unlock()
#define box_list_read(x,y)        box_list_read_o(x, y)
#define box_list_next(x,y)        box_list_next_o(x, y)
#define box_list_write(x)         box_list_write_o(x)
#define box_list_update(x)        box_list_update_o(x)
#define box_list_replace(x)       box_list_update_o(x)
#define box_list_delete(x)        box_list_delete_o(x)

#define boxes_open(x)             boxes_open_o(x)
#define boxes_close()             boxes_close_o()
#define boxes_setkey(x)           boxes_setkey_o(x)
#define boxes_startkey(x)         boxes_startkey_o(x)
#define boxes_stopkey(x)          boxes_stopkey_o(x)
#define boxes_unlock()
#define boxes_read(x,y)           boxes_read_o(x, y)
#define boxes_next(x,y)           boxes_next_o(x, y)
#define boxes_write(x)            boxes_write_o(x)
#define boxes_update(x)           boxes_update_o(x)
#define boxes_replace(x)          boxes_update_o(x)
#define boxes_delete(x)           boxes_delete_o(x)

#define log_open(x)               log_open_o(x)
#define log_close()               log_close_o()
#define log_setkey(x)             log_setkey_o(x)
#define log_startkey(x)           log_startkey_o(x)
#define log_stopkey(x)            log_stopkey_o(x)
#define log_unlock()              
#define log_read(x,y)             log_read_o(x, y)
#define log_next(x,y)             log_next_o(x, y)
#define log_write(x)              log_write_o(x)
#define log_update(x)             log_update_o(x)
#define log_replace(x)            log_update_o(x)
#define log_delete(x)             log_delete_o(x)

#define lot_open(x)               lot_open_o(x)
#define lot_close()               lot_close_o()
#define lot_setkey(x)             lot_setkey_o(x)
#define lot_startkey(x)           lot_startkey_o(x)
#define lot_stopkey(x)            lot_stopkey_o(x)
#define lot_unlock()              
#define lot_read(x,y)             lot_read_o(x, y)
#define lot_next(x,y)             lot_next_o(x, y)
#define lot_write(x)              lot_write_o(x)
#define lot_update(x)             lot_update_o(x)
#define lot_replace(x)            lot_update_o(x)
#define lot_delete(x)              lot_delete_o(x)

#define operator_open(x)          operator_open_o(x)
#define operator_close()          operator_close_o()
#define operator_setkey(x)        operator_setkey_o(x)
#define operator_startkey(x)      operator_startkey_o(x)
#define operator_stopkey(x)       operator_stopkey_o(x)
#define operator_unlock()
#define operator_read(x,y)        operator_read_o(x, y)
#define operator_next(x,y)        operator_next_o(x, y)
#define operator_write(x)         operator_write_o(x)
#define operator_update(x)        operator_update_o(x)
#define operator_replace(x)       operator_update_o(x)
#define operator_delete(x)         operator_delete_o(x)

#define order_open(x)             order_open_o(x)
#define order_close()             order_close_o()
#define order_setkey(x)           order_setkey_o(x)
#define order_startkey(x)         order_startkey_o(x)
#define order_stopkey(x)          order_stopkey_o(x)
#define order_unlock() 
#define order_read(x,y)           order_read_o(x, y)
#define order_next(x,y)           order_next_o(x, y)
#define order_write(x)            order_write_o(x)
#define order_update(x)           order_update_o(x)
#define order_replace(x)          order_update_o(x)
#define order_delete(x)            order_delete_o(x)

#define order_scan_open(x)        order_scan_open_o(x)
#define order_scan_close()        order_scan_close_o()
#define order_scan_setkey(x)      order_scan_setkey_o(x)
#define order_scan_startkey(x)    order_scan_startkey_o(x)
#define order_scan_stopkey(x)     order_scan_stopkey_o(x)
#define order_scan_unlock() 
#define order_scan_read(x,y)      order_scan_read_o(x, y)
#define order_scan_next(x,y)      order_scan_next_o(x, y)
#define order_scan_write(x)       order_scan_write_o(x)
#define order_scan_update(x)      order_scan_update_o(x)
#define order_scan_replace(x)     order_scan_update_o(x)
#define order_scan_delete(x)      order_scan_delete_o(x)

#define packing_list_open(x)      packing_list_open_o(x)
#define packing_list_close()      packing_list_close_o()
#define packing_list_setkey(x)    packing_list_setkey_o(x)
#define packing_list_startkey(x)  packing_list_startkey_o(x)
#define packing_list_stopkey(x)   packing_list_stopkey_o(x)
#define packing_list_unlock()
#define packing_list_read(x,y)    packing_list_read_o(x, y)
#define packing_list_next(x,y)    packing_list_next_o(x, y)
#define packing_list_write(x)     packing_list_write_o(x)
#define packing_list_update(x)    packing_list_update_o(x)
#define packing_list_replace(x)   packing_list_update_o(x)
#define packing_list_delete(x)    packing_list_delete_o(x)

#define pending_open(x)           pending_open_o(x)
#define pending_close()           pending_close_o()
#define pending_setkey(x)         pending_setkey_o(x)
#define pending_startkey(x)       pending_startkey_o(x)
#define pending_stopkey(x)        pending_stopkey_o(x)
#define pending_unlock()
#define pending_read(x,y)         pending_read_o(x, y)
#define pending_next(x,y)         pending_next_o(x, y)
#define pending_write(x)          pending_write_o(x)
#define pending_update(x)         pending_update_o(x)
#define pending_replace(x)        pending_update_o(x)
#define pending_delete(x)          pending_delete_o(x)

#define picker_open(x)            picker_open_o(x)
#define picker_close()            picker_close_o()
#define picker_setkey(x)          picker_setkey_o(x)
#define picker_startkey(x)        picker_startkey_o(x)
#define picker_stopkey(x)         picker_stopkey_o(x)
#define picker_unlock()
#define picker_read(x,y)          picker_read_o(x, y)
#define picker_next(x,y)          picker_next_o(x, y)
#define picker_write(x)           picker_write_o(x)
#define picker_update(x)          picker_update_o(x)
#define picker_replace(x)         picker_update_o(x)
#define picker_delete(x)          picker_delete_o(x)

#define picker_order_open(x)      picker_order_open_o(x)
#define picker_order_close()      picker_order_close_o()
#define picker_order_setkey(x)    picker_order_setkey_o(x)
#define picker_order_startkey(x)  picker_order_startkey_o(x)
#define picker_order_stopkey(x)   picker_order_stopkey_o(x)
#define picker_order_unlock() 
#define picker_order_read(x,y)    picker_order_read_o(x, y)
#define picker_order_next(x,y)    picker_order_next_o(x, y)
#define picker_order_write(x)     picker_order_write_o(x)
#define picker_order_update(x)    picker_order_update_o(x)
#define picker_order_replace(x)   picker_order_update_o(x)
#define picker_order_delete(x)    picker_order_delete_o(x)

#define pick_open(x)              pick_open_o(x)
#define pick_close()              pick_close_o()
#define pick_setkey(x)            pick_setkey_o(x)
#define pick_startkey(x)          pick_startkey_o(x)
#define pick_stopkey(x)           pick_stopkey_o(x)
#define pick_unlock()
#define pick_read(x,y)            pick_read_o(x, y)
#define pick_next(x,y)            pick_next_o(x, y)
#define pick_write(x)             pick_write_o(x)
#define pick_update(x)            pick_update_o(x)
#define pick_replace(x)           pick_update_o(x)
#define pick_delete(x)            pick_delete_o(x)
#define pick_direct(x,y,z)        pick_direct_o(x, y, z)

#define pmfile_open(x)            pmfile_open_o(x)
#define pmfile_close()            pmfile_close_o()
#define pmfile_setkey(x)          pmfile_setkey_o(x)
#define pmfile_startkey(x)        pmfile_startkey_o(x)
#define pmfile_stopkey(x)         pmfile_stopkey_o(x)
#define pmfile_unlock()
#define pmfile_read(x,y)          pmfile_read_o(x, y)
#define pmfile_next(x,y)          pmfile_next_o(x, y)
#define pmfile_write(x)           pmfile_write_o(x)
#define pmfile_update(x)          pmfile_update_o(x)
#define pmfile_replace(x)         pmfile_update_o(x)
#define pmfile_delete(x)          pmfile_delete_o(x)

#define prodfile_open(x)          prodfile_open_o(x)
#define prodfile_close()          prodfile_close_o()
#define prodfile_setkey(x)        prodfile_setkey_o(x)
#define prodfile_startkey(x)      prodfile_startkey_o(x)
#define prodfile_stopkey(x)       prodfile_stopkey_o(x)
#define prodfile_unlock() 
#define prodfile_read(x,y)        prodfile_read_o(x, y)
#define prodfile_next(x,y)        prodfile_next_o(x, y)
#define prodfile_write(x)         prodfile_write_o(x)
#define prodfile_update(x)        prodfile_update_o(x)
#define prodfile_replace(x)       prodfile_update_o(x)
#define prodfile_delete(x)        prodfile_delete_o(x)

#define queue_open(x)             queue_open_o(x)
#define queue_close()             queue_close_o()
#define queue_setkey(x)           queue_setkey_o(x)
#define queue_startkey(x)         queue_startkey_o(x)
#define queue_stopkey(x)          queue_stopkey_o(x)
#define queue_unlock()
#define queue_read(x,y)           queue_read_o(x, y)
#define queue_next(x,y)           queue_next_o(x, y)
#define queue_write(x)            queue_write_o(x)
#define queue_update(x)           queue_update_o(x)
#define queue_replace(x)          queue_update_o(x)
#define queue_delete(x)           queue_delete_o(x)

#define remarks_open(x)           remarks_open_o(x)
#define remarks_close()           remarks_close_o()
#define remarks_setkey(x)         remarks_setkey_o(x)
#define remarks_startkey(x)       remarks_startkey_o(x)
#define remarks_stopkey(x)        remarks_stopkey_o(x)
#define remarks_unlock()
#define remarks_read(x,y)         remarks_read_o(x, y)
#define remarks_next(x,y)         remarks_next_o(x, y)
#define remarks_write(x)          remarks_write_o(x)
#define remarks_update(x)         remarks_update_o(x)
#define remarks_replace(x)        reamrks_update_o(x)
#define remarks_delete(x)          remarks_delete_o(x)

#define restock_open(x)           restock_open_o(x)
#define restock_close()           restock_close_o()
#define restock_setkey(x)         restock_setkey_o(x)
#define restock_startkey(x)       restock_startkey_o(x)
#define restock_stopkey(x)        restock_stopkey_o(x)
#define restock_unlock()
#define restock_read(x,y)         restock_read_o(x, y)
#define restock_next(x,y)         restock_next_o(x, y)
#define restock_write(x)          restock_write_o(x)
#define restock_update(x)         restock_update_o(x)
#define restock_replace(x)        restock_update_o(x)
#define restock_delete(x)         restock_delete_o(x)

#define ship_label_open(x)        ship_label_open_o(x)
#define ship_label_close()        ship_label_close_o()
#define ship_label_setkey(x)      ship_label_setkey_o(x)
#define ship_label_startkey(x)    ship_label_startkey_o(x)
#define ship_label_stopkey(x)     ship_label_stopkey_o(x)
#define ship_label_unlock()
#define ship_label_read(x,y)      ship_label_read_o(x, y)
#define ship_label_next(x,y)      ship_label_next_o(x, y)
#define ship_label_write(x)       ship_label_write_o(x)
#define ship_label_update(x)      ship_label_update_o(x)
#define ship_label_replace(x)     ship_label_update_o(x)
#define ship_label_delete(x)       ship_label_delete_o(x)

#define short_open(x)             short_open_o(x)
#define short_close()             short_close_o()
#define short_setkey(x)           short_setkey_o(x)
#define short_startkey(x)         short_startkey_o(x)
#define short_stopkey(x)          short_stopkey_o(x)
#define short_unlock()
#define short_read(x,y)           short_read_o(x, y)
#define short_next(x,y)           short_next_o(x, y)
#define short_write(x)            short_write_o(x)
#define short_update(x)           short_update_o(x)
#define short_replace(x)          short_update_o(x)
#define short_delete(x)           short_delete_o(x)

#define tote_label_open(x)        tote_label_open_o(x)
#define tote_label_close()        tote_label_close_o()
#define tote_label_setkey(x)      tote_label_setkey_o(x)
#define tote_label_startkey(x)    tote_label_startkey_o(x)
#define tote_label_stopkey(x)     tote_label_stopkey_o(x)
#define tote_label_unlock()
#define tote_label_read(x,y)      tote_label_read_o(x, y)
#define tote_label_next(x,y)      tote_label_next_o(x, y)
#define tote_label_write(x)       tote_label_write_o(x)
#define tote_label_update(x)      tote_label_update_o(x)
#define tote_label_replace(x)     tote_label_update_o(x)
#define tote_label_delete(x)      tote_label_delete_o(x)

#define transaction_open(x)       transaction_open_o(x)
#define transaction_close()       transaction_close_o()
#define transaction_setkey(x)     transaction_setkey_o(x)
#define transaction_startkey(x)   transaction_startkey_o(x)
#define transaction_stopkey(x)    transaction_stopkey_o(x)
#define transaction_unlock()
#define transaction_read(x,y)     transaction_read_o(x, y)
#define transaction_next(x,y)     transaction_next_o(x, y)
#define transaction_write(x)      transaction_write_o(x)
#define transaction_update(x)     transaction_update_o(x)
#define transaction_replace(x)    transaction_update_o(x)
#define transaction_delete(x)     transaction_delete_o(x)

#define retransmit_open(x)       retransmit_open_o(x)
#define retransmit_close()       retransmit_close_o()
#define retransmit_setkey(x)     retransmit_setkey_o(x)
#define retransmit_startkey(x)   retransmit_startkey_o(x)
#define retransmit_stopkey(x)    retransmit_stopkey_o(x)
#define retransmit_unlock()
#define retransmit_read(x,y)     retransmit_read_o(x, y)
#define retransmit_next(x,y)     retransmit_next_o(x, y)
#define retransmit_write(x)      retransmit_write_o(x)
#define retransmit_update(x)     retransmit_update_o(x)
#define retransmit_replace(x)    retransmit_update_o(x)
#define retransmit_delete(x)     retransmit_delete_o(x)

#define group_open(x)       group_open_o(x)
#define group_close()       group_close_o()
#define group_setkey(x)     group_setkey_o(x)
#define group_startkey(x)   group_startkey_o(x)
#define group_stopkey(x)    group_stopkey_o(x)
#define group_unlock()
#define group_read(x,y)     group_read_o(x, y)
#define group_next(x,y)     group_next_o(x, y)
#define group_write(x)      group_write_o(x)
#define group_update(x)     group_update_o(x)
#define group_delete(x)     group_delete_o(x)
#endif

/* end of oracle_defines.h */
