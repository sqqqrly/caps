#define DEBUG
#define TANDY     
/* #define SHORT_OI  */
#define JCP
#define JCPNBOX
/*-------------------------------------------------------------------------*
 *  Custom:         SHORT_DF - Shorts if below zero picks.
 *                  SONOMA   - Send pick text to short ticket.
 *                  SHORT_OI - set oi_flag if shorts.
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Update database with pick information.
 *                  Supports KMart post-assignment of boxes.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/23/93   |  tjt  Rewritten;
 *  06/02/94   |  tjt  Transaction output added to sync picks & complete.
 *  06/24/94   |  tjt  Add order split request (walgreens).
 *  07/11/94   |  tjt  Add order cancel event remove remaining picks.
 *  08/04/94   |  tjt  Add complete transaction to OrderSplitRequest.
 *  09/08/94   |  tjt  Add picks to go to pickline item.
 *  11/14/94   |  tjt  Fix wave done transaction.
 *  11/15/94   |  tjt  Fix open/close boxes at mark/restoreplace.
 *  12/22/94   |  tjt  Add lot control.
 *  02/02/95   |  tjt  Remove UNOS queues.
 *  02/15/95   |  tjt  Add split flag to short notices.
 *  04/27/95   |  tjt  Add productivity counts of picks.
 *  05/05/95   |  tjt  Add bulletproof db open tests.
 *  07/01/95   |  tjt  Add remaining picks always.
 *  12/22/95   |  tjt  Add no restock notice of rqty <= 0;
 *  07/10/96   |  tjt  Add clear header flag on box full.
 *  12/02/96   |  tjt  Add units and lines to location.
 *  12/18/96   |  tjt  Add alloc to pmfile.
 *  05/26/98   |  tjt  Add box number to pick record using TPickBoxMessage.
 *  05/28/98   |  tjt  Add box number P/S transaction as lot number.
 *  06/07/98   |  tjt  Add box update for TPickBoxMessage.
 *  06/05/01   |  aha  Added section productivity tracking.
 *  10/08/01   |  aha  Modified for Eckerd's Tote Integrity.
 *  02/27/03   |  aha  Added Eckerd transactions to control by sp_text
 *-------------------------------------------------------------------------*/
static char pick_update_db_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>
#include "file_names.h"
#include "message_types.h"
#include "caps_messages.h"
#include "eh_nos.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "pr.h"
#include "xt.h"
#include "box.h"
#include "zone_status.h"
#include "order_cust_c.h"
#include "date_mani_c.h"

#include "Bard.h"
#include "bard/pmfile.h"
#include "boxes.h"
#include "bard/short_notice.h"
#include "bard/restock_notice.h"
#include "bard/remarks.h"
#include "bard/lot.h"

extern leave();

FILE *rfd;                                 /* restock numbers                */
long r_number;                             /* restock number                 */

typedef struct
{
    char lotno[15];

} lotno_item;
lotno_item *lots = 0;                       /* lot numbers                   */

long have_header = 0;                      /* has read order header record   */

unsigned char list[] = {ShutdownEvent, MarkplaceEvent, RestoreplaceEvent,
    ConfigureEvent, ModulePickEvent, BoxCloseEvent,
    OrderUnderwayEvent, OrderCompleteEvent, OrderCancelEvent, OrderRepickEvent,
    ModuleEnableEvent, ModuleInhibitEvent, OrderSplitRequest, 
    LotChangeRequest, ModulePickBoxEvent, 0};

long who, type, len;
TCapsMessageItem buf;

#ifdef DEBUG
FILE *DF;
#define DF_SIZE 4000000
#endif

main()
{
    putenv("_=pick_update_db");
    chdir(getenv("HOME"));

    setpgrp();

    signal_catcher(0);                        /* catch various signals         */

    message_open();
    message_select(list, sizeof(list));

#ifdef DEBUG
    DF = fopen("debug/pdb_bug", "w");

    fprintf(DF, "pick_update_db started:  \n" );
    fflush(DF);
#endif

    database_open();

    ss_open();
    co_open();
    oc_open();
    od_open();
    if (sp->sp_productivity == 'y')
    {
        pr_open();
    }

    if (sp->sp_short_notice == 'y')   short_open(AUTOLOCK);
    if (sp->sp_restock_notice == 'y') restock_open(AUTOLOCK);

    if (sp->sp_box_feature != 'n') boxes_open(AUTOLOCK);
    if (sp->sp_box_feature == 's') box_list_open(AUTOLOCK);

    rfd = fopen(restock_no_name, "r+");
    if (rfd == 0) krash("main", "open restock numbers", 1);
    fread(&r_number, 4, 1, rfd);

    if (sp->sp_running_status == 'y')
    {
        if (sp->sp_to_flag != 'n') xt_open();
        open_pm_file();
        open_lot_file();
    }
    while (1)
    {
#ifdef DEBUG
        fflush(DF);
        if (ftell(DF) > DF_SIZE)
        {
            fclose(DF);
            system("mv debug/pdb_bug debug/pdb_save 1>/dev/null 2>&1");
            DF = fopen("debug/pdb_bug", "w");
        }
        fprintf(DF, ".. waiting for message\n");
        fflush(DF);
#endif

        message_get(&who, &type, &buf, &len);

#ifdef DEBUG
        fprintf(DF, "who=%d type=%d %s len=%d\n", 
                who, type & 0x7f, type > 0x7f? "Event" : "Request", len);
        if (len) Bdump(&buf, len);
#endif

        begin_work();

        switch (type)
        {
            case ShutdownEvent: 

                commit_work();
                leave(0);

            case MarkplaceEvent:

                if (sp->sp_to_flag != 'n') xt_close();

                if (pmfile_fd)  pmfile_close();
                if (boxes_fd)   boxes_close();
                if (lot_fd)     lot_close();

                if (lots)       free(lots);

                lots      = 0;
                break;

            case ConfigureEvent:
            case RestoreplaceEvent:

                if (sp->sp_to_flag != 'n') xt_open();
                if (sp->sp_box_feature != 'n') boxes_open(AUTOLOCK);
                open_pm_file();
                open_lot_file();
                break;

            case ModulePickEvent:

                have_header = 0;
                module_pick_event(&buf);
                break;

            case ModulePickBoxEvent:

                have_header = 0;
                module_pick_box_event(&buf);
                break;

            case BoxCloseEvent:

                have_header = 0;                 /* F071096 - clear get_header flag  */
                box_close_event(&buf);

                if (sp->sp_box_feature != 'y') break;
                assign_box_number(&buf);
                break;

            case OrderUnderwayEvent:

                order_underway(&buf);
                break;

            case OrderCompleteEvent:

                order_complete(&buf);
                break;

            case OrderCancelEvent:

                order_cancel(&buf);
                break;

            case OrderRepickEvent:

                order_repick(&buf);
                break;

            case ModuleEnableEvent:

                if (sp->sp_sku_support != 'y') break;
                enable_inhibit_module(&buf, 'n');
                break;

            case ModuleInhibitEvent:

                if (sp->sp_sku_support != 'y') break;
                enable_inhibit_module(&buf, 'y');
                break;

            case OrderSplitRequest:

                oc_lock();
                if (!order_split(&buf))
                {
                    message_put(0, &buf, OrderSplitEvent, sizeof(TOrderSplitMessage));
                }
                oc_unlock();
                break;

            case LotChangeRequest:

                if (sp->sp_lot_control == 'y') lot_change(&buf);
                break;

        }                                      /* switch                         */
        commit_work();
    }                                        /* while loop                     */
}
/*-------------------------------------------------------------------------*
 *  Order Underway Event
 *-------------------------------------------------------------------------*/
order_underway(buf)
    register TOrderEventMessage *buf;
{
    struct zone_item *z;

    if (sp->sp_to_flag != 'n' && sp->sp_to_underway == 'y')
    {
        xt_build(buf->m_con, buf->m_grp, buf->m_order, buf->m_pickline,
                'U', 0,0,0,0,0, buf->m_zone, 0);
    }
#ifdef JCP11
    z = &zone[buf->m_zone - 1];
    if (buf->m_con[0] == 'Z' )
    {
        z->zt_flags |= ZoneAudit;
    }
#endif

#ifndef JCP
    if (sp->sp_box_feature != 's') return 0;
#endif
    if (rf->rf_box_len <= 0)       return 0;
    make_empty_boxes(buf);

    return 0;
}

/*-------------------------------------------------------------------------*
 *  Order Complete
 *-------------------------------------------------------------------------*/
order_complete(buf)
    register TOrderEventMessage *buf;
{
    Teckerd_trans_item xt_buf;

#ifdef DEBUG
    fprintf(DF,"order_complete() \n");
    fflush(DF);
#endif

    /* Update Order Complete Status for Eckerd Tote Integrity */
    if (sp->sp_to_flag != 'n' && sp->sp_to_complete == 'y')
    {
        memset(&xt_buf, 0x0, sizeof(Teckerd_trans_item));

        xt_buf.xt_code     = 'C';
        xt_buf.xt_order    = buf->m_order;
        xt_buf.xt_pickline = buf->m_pickline;

#ifdef DEBUG
        fprintf(DF, "xt_buf: code = %c, order = %d, pickline = %d\n",
                xt_buf.xt_code, xt_buf.xt_order, xt_buf.xt_pickline);
        fflush(DF);
#endif

        message_put(0, TransactionEvent, &xt_buf, sizeof(Teckerd_trans_item));
        sp->sp_to_count += 1;

        check_wave_done(buf);
    }  /* end of eckerd transaction update */

    if (sp->sp_box_feature != 's') return 0;
    if (rf->rf_box_len <= 0)       return 0;

    if (sp->sp_delete_empty_boxes == 's') delete_empty_boxes(buf);
    if (sp->sp_box_mode == 'c')           queue_box_list(buf);

    return 0;
}

/*-------------------------------------------------------------------------*
 *  Order Repick Event
 *-------------------------------------------------------------------------*/
order_repick(buf)
    register TOrderEventMessage *buf;
{
    if (sp->sp_to_repick == 'y' && of_rec->of_repick == 'y')
    {
        xt_build(buf->m_con, buf->m_grp, buf->m_order, buf->m_pickline,
                'R', 0,0,0,0,0, buf->m_zone, 0);

        check_wave_done(buf);
    }
    return 0;
}

/*-------------------------------------------------------------------------*
 *  Order Cancel Event
 *-------------------------------------------------------------------------*/
order_cancel(buf)
    TOrderEventMessage *buf;
{
    register struct pl_item *p;
    register struct hw_item *h;
    register struct pw_item *i;

    if (sp->sp_to_flag != 'n' && sp->sp_to_cancel == 'y')
    {
        xt_build(buf->m_con, buf->m_grp, buf->m_order, buf->m_pickline,
                'X', 0,0,0,0,0, buf->m_zone,0);

        check_wave_done(buf);
    }
    p = &pl[buf->m_pickline - 1];

    pick_setkey(1);

    op_rec->pi_pl  = buf->m_pickline;
    op_rec->pi_on  = buf->m_order;
    op_rec->pi_mod = 1;

    pick_startkey(op_rec);

    op_rec->pi_mod = coh->co_prod_cnt;
    pick_stopkey(op_rec);

    while (!pick_next(op_rec, NOLOCK))
    {
        if (op_rec->pi_flags & (PICKED | NO_PICK | MIRROR)) continue;

        i = &pw[op_rec->pi_mod - 1];
        h = &hw[i->pw_ptr - 1];

        i->pw_units_to_go -= op_rec->pi_ordered;
        i->pw_lines_to_go -= 1;

        p->pl_lines_to_go -= 1;
        p->pl_units_to_go -= op_rec->pi_ordered;
    }

    if (sp->sp_box_feature != 's') return 0;
    if (rf->rf_box_len <= 0)       return 0;
    delete_all_boxes(buf);

    return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Wave Done
 *-------------------------------------------------------------------------*/
check_wave_done(buf)
    register TOrderEventMessage *buf;
{
    register struct oc_item *o;

    if (sp->sp_to_flag == 'n')        return 0;
    if (sp->sp_to_orders_done != 'y') return 0;

    o = &oc->oc_tab[buf->m_pickline - 1];

    if (o->oc_high.oc_count) return 0;
    if (o->oc_med.oc_count)  return 0;
    if (o->oc_low.oc_count)  return 0;
    if (o->oc_uw.oc_count)   return 0;

    xt_build(" ", buf->m_grp, 0, buf->m_pickline, 'W', 0,0,0,0,0,0,0);

    return 0;
}
/*-------------------------------------------------------------------------*
 *  Open Pick Module File
 *-------------------------------------------------------------------------*/
open_pm_file()
{
    pmfile_item pm;

    pmfile_open(AUTOLOCK);
    pmfile_setkey(1);

    return 0;
}
/*-------------------------------------------------------------------------*
 *  Open Lot Number  File
 *-------------------------------------------------------------------------*/
open_lot_file()
{
    register struct st_item *s;
    lot_item x;

    if (sp->sp_lot_control != 'y') return 0;

    lot_open(AUTOLOCK);
    lot_setkey(1);

    lots = (lotno_item *)malloc(15 * coh->co_products);
    memset(lots, 0, 15 * coh->co_products);

    while (!lot_next(&x, NOLOCK))
    {
        s = sku_lookup(x.l_lot_pl, x.l_lot_sku);
        if (!s) continue;

        if (s->st_mod < 1 || s->st_mod > coh->co_products) continue;

        if (*lots[s->st_mod - 1].lotno) continue;
        strip_space(x.l_lot_number, 15);

        memcpy(lots[s->st_mod - 1].lotno, x.l_lot_number, 15);
    }
    return 0;
}

/*-------------------------------------------------------------------------*
 *  Update Pick Module Inventory - Restock Notices
 *-------------------------------------------------------------------------*/
update_pm_file(buf, lines)
    register TPickMessage *buf;
    register long lines;
{
    register struct hw_item  *h;
    register struct pw_item  *i;
    register struct bay_item *b;
    register long qty, cases;

    pmfile_item pm;
    restock_notice_item rs;

    if (buf->m_picked <= 0) return 0;

    pm.p_pmodno = buf->m_module;
    if (pm.p_pmodno < 1 || pm.p_pmodno > coh->co_products) return 0;

    i = &pw[buf->m_module - 1];
    h = &hw[i->pw_ptr - 1];
    b = &bay[h->hw_bay - 1];

    pm.p_pmodno = buf->m_module;
    if (pmfile_read(&pm, LOCK)) return 0;

#ifdef SHORT_DF

    if (b->bay_flags & IsDummy)
    {
        if (pm.p_qty <= 0) 
        {    
            buf->m_picked = 0;
            return 0;
        }
        else (buf->m_picked > pm.p_qty) buf->m_picked = pm.p_qty;
    }
#endif

    pm.p_qty     -= buf->m_picked;
    pm.p_alloc   -= buf->m_picked;
    pm.p_cuunits += buf->m_picked;
    pm.p_cmunits += buf->m_picked;
    pm.p_culines += lines;
    pm.p_cmlines += lines;

    if (pm.p_rsflag == 'n' && pm.p_qty < pm.p_restock)
    {
        if (sp->sp_restock_notice == 'y' && pm.p_rqty > 0)
        {
            memset(&rs, 0, sizeof(restock_notice_item));

            rs.r_rs_ref      = ++sp->sp_rs_count;
            rs.r_rs_time     = time(0);
            rs.r_rs_number   = r_number;
            rs.r_rs_pl       = buf->m_pickline;
            rs.r_rs_mod      = buf->m_module;
            rs.r_rs_quantity = pm.p_restock - pm.p_qty;
            restock_write(&rs);

            pm.p_rsflag = 'y';

            r_number++;
            if (r_number > 99999) r_number = 1;
            fseek(rfd, 0, 0);
            fwrite(&r_number, 4, 1, rfd);
        }
        if (sp->sp_to_flag != 'n' && sp->sp_to_restock == 'y')
        {
            qty = pm.p_lcap - pm.p_qty;
            if (i->pw_case > 0) cases = qty / i->pw_case;
            else cases = 0;

            xt_build(" ", " ", 0, buf->m_pickline, 'K',
                    pm.p_pmsku, pm.p_pmodno, pm.p_stkloc, qty, cases, 0,0);
        }
        pmfile_replace(&pm);
        return 1;                              /* restock notice                 */
    }                                        /* end restock notice             */
    pmfile_replace(&pm);
    return 0;                                /* no restock notice              */
}
/*-------------------------------------------------------------------------*
 *  Module Enable & Inhibit
 *-------------------------------------------------------------------------*/
enable_inhibit_module(buf, yn)
    register TModuleMessage *buf;
    register char yn;
{
    pmfile_item pm;

    if (pmfile_fd <= 0) return 0;

    pmfile_setkey(1);

    pm.p_pmodno = buf->m_module;

    if (pmfile_read(&pm, LOCK)) return 0;
    pm.p_piflag = yn;
    pmfile_replace(&pm);

#ifdef DEBUG
    Bdump(&pm, sizeof(pmfile_item));
#endif
    return 0;
}
/*-------------------------------------------------------------------------*
 *  Box Close Event - Transaction Only Here  - F060798 box to lot in xt
 *-------------------------------------------------------------------------*/
box_close_event(buf)
    register TBoxOrderMessage *buf;
{
    char boxno[16];
    boxes_item box;

#ifdef DEBUG
    fprintf(DF, "box_close_event: pl=%d on=%d box=%d\n",
            buf->m_pickline, buf->m_order, buf->m_box);
#endif

    if (sp->sp_box_feature == 's')          /* F060798 - close open box */
    {
        boxes_setkey(2);

        box.b_box_pl     = buf->m_pickline;
        box.b_box_on     = buf->m_order;
        box.b_box_number = buf->m_box;

        if (!boxes_read(&box, LOCK))
        {
            box.b_box_status[0] = BOX_CLOSED;
            box.b_box_status[1] = '\0';
#ifdef DEBUG
            fprintf(DF, "box_close: pl=%d, on=%d, box=%d, status=%s\n",
                    box.b_box_pl, box.b_box_on, box.b_box_number, box.b_box_status);
            fprintf(DF, "    lines=%d, units=%d\n",
                    box.b_box_lines, box.b_box_units);
            fflush(DF);
#endif
            boxes_update1(&box);
            commit_work();
            begin_work();
        }
    }
    if (sp->sp_labels == 'y' && sp->sp_box_mode == 'd')
    {
        queue_box_label(buf->m_pickline, buf->m_order, buf->m_box);
    }
    if (sp->sp_to_box_close != 'y') return 0;

    if (get_header(buf->m_pickline, buf->m_order, NOLOCK))
    {
        sprintf(boxno, "%0*d", BoxNoLength  , buf->m_box);
#ifdef TANDY
#ifndef JCP
        if (buf->m_last) 
        {
            xt_build(of_rec->of_con, "LAST", of_rec->of_on, of_rec->of_pl,
                    'B', 0, 0, boxno+BoxNoLength-6, 0, 0, 0, boxno);  
        }
        else
            xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on, of_rec->of_pl,
                    'B', 0, 0, boxno+BoxNoLength-6, 0, 0, 0, boxno);  
#endif
#else
        xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on, of_rec->of_pl,
                'B', 0, 0, boxno+BoxNoLength-6, 0, 0, 0, boxno);  
#endif

#ifdef JCPNBOX
        if (buf->m_last == 0)  // if it is not the last box means Box Full box
        {
            xt_build(of_rec->of_con, 0, of_rec->of_on, of_rec->of_pl,
                    'B', 0, 0, 0, 0, 0, of_rec->of_pl, 0);  
        }
#endif
    }
    return 0;
}
/*-------------------------------------------------------------------------*
 *  A Pick Has Been Completed
 *-------------------------------------------------------------------------*/
module_pick_event(buf)
    register TPickMessage *buf;
{
    register struct hw_item   *h;
    register struct pw_item   *i;
    register struct bay_item  *b;
    register struct zone_item *z;
    register struct pl_item   *p;
    register struct st_item   *s;
    register unsigned char *lot;
    register long unpicked, block;
    char remaining[8], sku[SkuLength];
    lot_item x;
    long picktime = 0L;

    if (pick_direct(op_rec, LOCK, buf->m_reference)) return 0;

    lot = 0;

    if (sp->sp_lot_control == 'y' && lots)
    {
        lot = lots[buf->m_module - 1].lotno;
    }
    i = &pw[op_rec->pi_mod - 1];
    h = &hw[i->pw_ptr - 1];
    b = &bay[h->hw_bay - 1];
    z = &zone[b->bay_zone - 1];
    p = &pl[z->zt_pl - 1];

#ifdef DEBUG
    fprintf(DF, "module_pick_event: zone=%d bay=%d mod=%d\n",
            z->zt_zone, b->bay_number, h->hw_mod);
#endif

    i->pw_units_to_go -= op_rec->pi_ordered;
    i->pw_lines_to_go -= 1;

    p->pl_units_to_go -= op_rec->pi_ordered;
    p->pl_lines_to_go -= 1;

    if (sp->sp_remaining_picks == 'u') 
    {
        sprintf(remaining, "%06d", i->pw_units_to_go);
    }
    else sprintf(remaining, "%06d", i->pw_lines_to_go);  

    if (sp->sp_inventory == 'y') 
    {
        if (update_pm_file(buf, 1)) op_rec->pi_flags |= RESTOCK;
    }
    op_rec->pi_flags     |= PICKED;         /* mark as picked                  */
    op_rec->pi_box_number = DUMMY_BOX;      /* assigned to a dummy box         */
    op_rec->pi_picked     = buf->m_picked;
    op_rec->pi_datetime   = time(0);        /* time recorded as picked         */

    if (sp->sp_lot_control == 'y' && lot)   /* add lot number to the record    */
    {
        memcpy(op_rec->pi_lot, lot, LotLength);
    }
    if (op_rec->pi_ordered > op_rec->pi_picked)
    {
#ifdef SHORT_OI
        block = oc_find(op_rec->pi_pl, op_rec->pi_on);
        if (block)
        {
            oc->oi_tab[block - 1].oi_flags |= SHORTS;
        }
#endif

        if (sp->sp_to_flag != 'n' && 
                (sp->sp_to_short == 'y' || sp->sp_to_pick_event == 'y'))
        {
            if (get_header(op_rec->pi_pl, op_rec->pi_on, NOLOCK))
            {
                if (sp->sp_use_stkloc == 'y')
                {
                    s = mod_lookup(op_rec->pi_mod);
                }
                else { s = 0; }

                char *sku;
                if (s)
                {
                    sku = s->st_stkloc;
                }
                else
                {
                    sku = op_rec->pi_sku;
                }

                xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on, of_rec->of_pl,
                        'S', sku, 
                        op_rec->pi_mod, remaining,
                        op_rec->pi_ordered, op_rec->pi_picked, z->zt_zone, lot);
            }
        }    
        if (sp->sp_short_notice == 'y') 
        {
            if (sp->sp_remaining_picks == 'u') queue_short(i->pw_units_to_go);
            else                               queue_short(i->pw_lines_to_go);
        }
    }     
    else if (sp->sp_to_flag != 'n' && sp->sp_to_pick_event == 'y')
    {
        if (get_header(op_rec->pi_pl, op_rec->pi_on, NOLOCK))
        {
            if (sp->sp_use_stkloc == 'y')
            {
                s = mod_lookup(op_rec->pi_mod);
            }
            else s = 0;

            char *sku;
            if (s)
            {
                sku = s->st_stkloc;
            }
            else
            {
                sku = op_rec->pi_sku;
            }

            xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on, of_rec->of_pl,
                    'P', sku, 
                    op_rec->pi_mod, remaining,
                    op_rec->pi_ordered, op_rec->pi_picked, z->zt_zone, lot);
        }
    }
    if (sp->sp_box_feature != 's')
    {
        pick_replace(op_rec);                  /* update without box info        */
        return 0;
    }
    if (sp->sp_box_feature == 's')
    {
        if (!buf->m_picked)
        {
            op_rec->pi_box_number = SHORT_BOX;
        }
        else if (op_rec->pi_ordered > buf->m_picked)
        {
            unpicked = op_rec->pi_ordered - buf->m_picked;

            op_rec->pi_flags  |= SPLIT_PICK;    /* picked into DUMMY box           */
            op_rec->pi_ordered = buf->m_picked;
            pick_update(op_rec);
            commit_work();
            begin_work();

            op_rec->pi_ordered    = unpicked;   /* unpicked into SHORT box         */
            op_rec->pi_picked     = 0;
            op_rec->pi_box_number = SHORT_BOX;
            pick_write(op_rec);
            commit_work();
            begin_work();
            return 0;
        }
    }
    pick_update(op_rec);
    commit_work();
    begin_work();
    return 0;
}


/*-------------------------------------------------------------------------*
 *  A Pick Has Been Completed To A Box.                  - F052698
 *  Add box number to transaction as lot number. - F052898
 *-------------------------------------------------------------------------*/
module_pick_box_event(buf)
    register TPickBoxMessage *buf;
{
    register struct hw_item   *h;
    register struct pw_item   *i;
    register struct bay_item  *b;
    register struct zone_item *z;
    register struct pl_item   *p;
    register struct st_item   *s;
    register unsigned char *lot;
    register long unpicked, block;
    char remaining[8], sku[SkuLength];
    char box_number[16];                      /* F052898 */
    lot_item x;
    boxes_item box;
    long picktime = 0L;
    Teckerd_trans_item xt_buf;
    long l_picker_id = 0L;

#ifdef DEBUG
    fprintf(DF, "module_pick_box_event(): reference = %d\n", buf->m_reference);
    fflush(DF);
#endif

    if (pick_direct(op_rec, LOCK, buf->m_reference)) return 0;

    lot = 0;

    if (sp->sp_lot_control == 'y' && lots)
    {
        lot = lots[buf->m_module - 1].lotno;
    }
    else                                                                                          /* F052898 - use box number               */
    {
        sprintf(box_number, "%0*d", BoxNoLength, buf->m_box);
        lot = box_number;
    }
    i = &pw[op_rec->pi_mod - 1];
    h = &hw[i->pw_ptr - 1];
    b = &bay[h->hw_bay - 1];
    z = &zone[b->bay_zone - 1];
    p = &pl[z->zt_pl - 1];

#ifdef DEBUG
    fprintf(DF, "module_pick_box_event: zone=%d bay=%d mod=%d\n",
            z->zt_zone, b->bay_number, h->hw_mod, buf->m_box 
           );
    fprintf(DF, "   pl=%d on=%d box=%d [%s] picked=%d picker ID=\n", 
            buf->m_pickline, buf->m_order,
            buf->m_box, box_number, buf->m_picked,
            buf->m_picker_id);
    fflush(DF);
#endif

    if (z->zt_picker == 0)
        l_picker_id = buf->m_picker_id;
    else
        l_picker_id = z->zt_picker;

    i->pw_units_to_go -= op_rec->pi_ordered;
    i->pw_lines_to_go -= 1;

    p->pl_units_to_go -= op_rec->pi_ordered;
    p->pl_lines_to_go -= 1;

    if (sp->sp_remaining_picks == 'u') 
    {
        sprintf(remaining, "%06d", i->pw_units_to_go);
    }
    else sprintf(remaining, "%06d", i->pw_lines_to_go);  

    if (buf->m_box != DUMMY_BOX)            /* a real box pick         */
    {
        boxes_setkey(2);

        box.b_box_pl     = buf->m_pickline;
        box.b_box_on     = buf->m_order;
        box.b_box_number = buf->m_box;

        if (!boxes_read(&box, LOCK))       /* F060798 - update box usage  */
        {
            box.b_box_lines  += 1;
            box.b_box_units  += buf->m_picked;
            begin_work();
            boxes_update1(&box);
            commit_work();
        }

        /* Update Pick info for Eckerd Tote Integrity */
        if (sp->sp_to_flag != 'n' && sp->sp_to_pick_event == 'y')
        {
            memset(&xt_buf, 0x0, sizeof(Teckerd_trans_item));

            strncpy(xt_buf.xt_sku, op_rec->pi_sku, SKU_SIZE - 1);
            xt_buf.xt_sku[SKU_SIZE - 1] = '\0';
            xt_buf.xt_code              = 'P';
            xt_buf.xt_order             = buf->m_order;
            xt_buf.xt_pickline          = buf->m_pickline;
            xt_buf.xt_picked            = buf->m_picked;
            xt_buf.xt_box               = buf->m_box;
            xt_buf.xt_picker            = l_picker_id;
            xt_buf.xt_time              = time(0);

#ifdef DEBUG
            fprintf(DF, "xt_buf: code = %c, order = %d, pickline = %d\n",
                    xt_buf.xt_code, xt_buf.xt_order, xt_buf.xt_pickline);
            fprintf(DF, "        picked = %d, box = %d, picker = %d\n",
                    xt_buf.xt_picked, xt_buf.xt_box, xt_buf.xt_picker);
            fprintf(DF, "        time = %d, sku = [%s]\n",
                    xt_buf.xt_time, xt_buf.xt_sku);
            fflush(DF);
#endif

            message_put(0, TransactionEvent, &xt_buf, sizeof(Teckerd_trans_item));

            sp->sp_to_count += 1;
        }  /* end of eckerd transaction update */
    }  /* end of real box update */

    if (sp->sp_inventory == 'y') 
    {
        if (update_pm_file(buf, 1)) op_rec->pi_flags |= RESTOCK;
    }
    op_rec->pi_flags     |= PICKED;         /* mark as picked                  */
    op_rec->pi_box_number = buf->m_box;     /* assigned to a real box          */
    op_rec->pi_picked     = buf->m_picked;
    op_rec->pi_datetime   = time(0);        /* time recorded as picked         */
#ifdef DEBUG
    fprintf(DF, "op_rec: box number = %d, picked = %d, datetime = %d\n",
            op_rec->pi_box_number, op_rec->pi_picked, op_rec->pi_datetime);
    fprintf(DF, "        reference = %d\n", op_rec->pi_reference);
    fflush(DF);
#endif

    if (sp->sp_lot_control == 'y' && lot)   /* add lot number to the record    */
    {
        memcpy(op_rec->pi_lot, lot, LotLength);
    }

    if (op_rec->pi_ordered > op_rec->pi_picked)
    {
#ifdef SHORT_OI
        block = oc_find(op_rec->pi_pl, op_rec->pi_on);
        if (block)
        {
            oc->oi_tab[block - 1].oi_flags |= SHORTS;
        }
#endif

        if (sp->sp_short_notice == 'y') 
        {
            if (sp->sp_remaining_picks == 'u') queue_short(i->pw_units_to_go);
            else                               queue_short(i->pw_lines_to_go);
        }
#ifdef DEBUG
        fprintf(DF, "Finished queueing shorts.\n");
        fflush(DF);
#endif
    }
    begin_work();
    pick_update(op_rec);
    commit_work();
    begin_work();
#ifdef DEBUG
    fprintf(DF, "Finished pick_update2(op_rec).\n");
    fflush(DF);
#endif

    return 0;
}  /* end of module_pick_box_event() */


/*-------------------------------------------------------------------------*
 *  Assign DUMMY box picks to a real box.
 *-------------------------------------------------------------------------*/
assign_box_number(buf)
    register TBoxOrderMessage *buf;
{
    boxes_item box;

    boxes_setkey(2);

    box.b_box_pl     = buf->m_pickline;
    box.b_box_on     = buf->m_order;
    box.b_box_number = buf->m_box;

#ifdef DEBUG
    fprintf(DF, "assign_box_number pl=%d on=%d box=%d\n",
            box.b_box_pl, box.b_box_on, box.b_box_number);
#endif

    if (boxes_read(&box, LOCK)) return 0;

#ifdef DEBUG
    Bdump(&box, sizeof(boxes_item));
#endif

    if (box.b_box_status[0] != BOX_OPEN)
    {
        return 0;
    }
    pick_setkey(2);

    op_rec->pi_pl         = buf->m_pickline;
    op_rec->pi_on         = buf->m_order;
    op_rec->pi_box_number = DUMMY_BOX;
    pick_startkey(op_rec);

    while (!pick_next(op_rec, LOCK))
    {
        box.b_box_lines += 1;
        box.b_box_units += op_rec->pi_picked;
        op_rec->pi_box_number = buf->m_box;
        pick_update(op_rec);
    }
    if (box.b_box_lines > 0)
    {
        box.b_box_status[0] = BOX_CLOSED;
        box.b_box_status[1] = '\0';
        boxes_update1(&box);
        commit_work();
        begin_work();

        if (sp->sp_labels == 'y' && sp->sp_box_mode == 'd')
        {
            queue_box_label(of_rec->of_pl, of_rec->of_on, buf->m_box);
        }
        return 0;
    }
    box.b_box_status[0] = BOX_UNUSED;
    box.b_box_status[1] = '\0';
    boxes_update1(&box);
    commit_work();
    begin_work();
    return 0;
}
make_empty_boxes(buf)
    register TOrderEventMessage *buf;
{
    register long k;
    register char *p;
    boxes_item box;
#ifdef JCP
    long block;
    register struct oi_item *o;
    char boxno[16];
#endif

    or_rec->rmks_pl = buf->m_pickline;
    or_rec->rmks_on = buf->m_order;

    if (remarks_read(or_rec, NOLOCK)) return 0;
#ifdef DEBUG
    fprintf(DF,"rvj - make_empty %8.8s\n",or_rec->rmks_text);
    fflush(DF);
#endif
#ifdef JCP
    block = oc_find(buf->m_pickline, buf->m_order);
    o = &oc->oi_tab[block - 1];
#endif

    p = or_rec->rmks_text + rf->rf_box_pos;
#ifdef DEBUG
    fprintf(DF,"rvj - make_emp rmks - %s\n",p);
    fflush(DF);
#endif

    for (k = 0; k < rf->rf_box_count; k++, p += rf->rf_box_len)
    {
        memset(&box, 0, sizeof(boxes_item));

        box.b_box_number = cvrt(p+1, rf->rf_box_len);
        if (!box.b_box_number) continue;

        box.b_box_pl        = buf->m_pickline;
        box.b_box_on        = buf->m_order;
        box.b_box_status[0] = BOX_UNUSED;
        box.b_box_status[1] = '\0';
        box.b_box_last[0]   = 0x20;
        box.b_box_last[1]   = '\0';

#ifdef DEBUG 
        fprintf(DF,"rvj - make_empty box_number %d\n",box.b_box_number);
        fflush(DF);
#endif

        boxes_write(&box);
        commit_work();
        begin_work();
    }

    memset(oc->oi_tab[block - 1].oi_box, 0x20, BoxNoLength);
    sprintf(boxno, "%0*d", BoxNoLength, box.b_box_number); 
#ifdef DEBUG
    fprintf(DF,"rvj - make_empty boxno %16.16s\n",boxno);
    fflush(DF);
#endif
    memcpy(o->oi_box, boxno, BoxNoLength);  

    return 0;
}
/*-------------------------------------------------------------------------*
 *  Delete Empty Boxes Of A Completed Order
 *-------------------------------------------------------------------------*/
delete_empty_boxes(buf)
    register TOrderEventMessage *buf;
{
    boxes_item box;

    boxes_setkey(1);

    box.b_box_pl        = buf->m_pickline;
    box.b_box_on        = buf->m_order;
    box.b_box_status[0] = BOX_UNUSED;
    box.b_box_status[1] = '\0';

    boxes_startkey(&box);

    while (!boxes_next(&box, LOCK)) boxes_delete();

    return 0;
}
/*-------------------------------------------------------------------------*
 *  Delete All Boxes Of A Canceled Order
 *-------------------------------------------------------------------------*/
delete_all_boxes(buf)
    register TOrderEventMessage *buf;
{
    boxes_item box;

    boxes_setkey(2);

    box.b_box_pl     = buf->m_pickline;
    box.b_box_on     = buf->m_order;
    box.b_box_number = 0;
    boxes_startkey(&box);

    box.b_box_number = DUMMY_BOX;
    boxes_stopkey(&box);

    while (!boxes_next(&box, LOCK)) boxes_delete();

    return 0;
}
/*-------------------------------------------------------------------------*
 *  Enqueue Short 
 *-------------------------------------------------------------------------*/
queue_short(to_go)
    register long to_go;
{
    short_notice_item   sh;

    memset(&sh, 0, sizeof(short_notice_item));

    if (!get_header(op_rec->pi_pl, op_rec->pi_on, NOLOCK)) return 0;

    sh.s_sh_ref       = ++sp->sp_sh_count;
    sh.s_sh_time      = time(0);
    sh.s_sh_on        = op_rec->pi_on;
    sh.s_sh_pl        = op_rec->pi_pl;
    sh.s_sh_mod       = op_rec->pi_mod;
    sh.s_sh_ordered   = op_rec->pi_ordered;
    sh.s_sh_picked    = op_rec->pi_picked;
    sh.s_sh_remaining = to_go;
    sh.s_sh_picker    = of_rec->of_picker;

    if (op_rec->pi_flags & SPLIT_PICK) sh.s_sh_split = '*';  /* F021595        */

#ifdef SONOMA
    memcpy(sh.s_sh_con, op_rec->pi_pick_text, rf->rf_pick_text);
#else
    memcpy(sh.s_sh_con, of_rec->of_con, CustomerNoLength);
#endif

    memcpy(sh.s_sh_grp, of_rec->of_grp, GroupLength);

    if (short_fd > 0) short_write(&sh);

    return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Order Header For A Pick Event
 *-------------------------------------------------------------------------*/
get_header(pl, order, lock)
    register long pl, order, lock;
{
    if (have_header) return 1;

    of_rec->of_pl = pl;
    of_rec->of_on = order;

    if (order_read(of_rec, lock)) return 0;

    have_header = 1;

    return 1;
}
/*-------------------------------------------------------------------------*
 *  Queue Box Packing Lists For All Boxes
 *-------------------------------------------------------------------------*/
queue_box_list(buf)
    register TOrderEventMessage *buf;
{
    boxes_item box;
#ifdef DEBUG  
    fprintf(DF,"queue_box_list - rvj\n");
    fflush(DF);
#endif

    boxes_setkey(2);

    box.b_box_pl     = buf->m_pickline;
    box.b_box_on     = buf->m_order;
    box.b_box_number = 1;

    boxes_startkey(&box);

    box.b_box_number = SHORT_BOX - 1;
    boxes_stopkey(&box);

    while (!boxes_next(&box, LOCK))
    {
        if (box.b_box_lines <= 0) continue;

        if (box.b_box_status[0] == BOX_CLOSED)
        {
            box.b_box_status[0] = BOX_QUEUED;
            box.b_box_status[1] = '\0';
            boxes_update(&box);
            queue_box_label(box.b_box_pl, box.b_box_on, box.b_box_number);
        }
    }
    return 0;
}
/*-------------------------------------------------------------------------*
 *  Queue Box Packing List For A Specific Box
 *-------------------------------------------------------------------------*/
queue_box_label(pl, on, bno)
    register long pl, on, bno;
{
    box_paper_item p;

    p.paper_ref        = ++sp->sp_bpl_count;
    p.paper_time       = time(0);
    p.paper_copies     = 1;
    p.paper_pl         = pl;
    p.paper_order      = on;
    p.paper_box_number = bno;
    memset(p.paper_printer, 0, sizeof(p.paper_printer));

    if (box_list_fd > 0) box_list_write(&p);

    return 0;
}
/*-------------------------------------------------------------------------*
 *  Convert Numeric Part Of Box Numbers
 *-------------------------------------------------------------------------*/
cvrt(p, n)
    register char *p;
    register long n;
{
    register long x;

    x = 0;

    while (n > 0)
    {
        if (*p < '0' || *p > '9') return 0;
        x = 10 * x + (*p - '0');
        p++;
        n--;
    }
    return x;
}
/*-------------------------------------------------------------------------*
 *  Order Split Into Picked and Unpicked Portions
 *-------------------------------------------------------------------------*
 *  1. If the ordered is queued or hold - only change the order number.
 *  2. If complete - ignore request.
 *  3. If underway and active - clear zone and return picks.
 *  3. If underway 
 *     a. copy old order record to new and change order number.
 *     b. copy old remarks record to new and change order number.
 *     c. change underway order index entry to new number.
 *     d. create and add order index entry to complete queue for old number.
 *     e. change all unpicked items to new order number.
 *-------------------------------------------------------------------------*
 *  Return of picks is a problem - the simplest way is to SEIZE the picks
 *  from the hardware table and then redisplay the zone; otherwise, if ofc,
 *  is envolved, multiple coordinated messages are required to eventually
 *  complete the return of picks and then complete the split.
 *-------------------------------------------------------------------------*/
order_split(buf)
    register TOrderSplitMessage *buf;
{
    register long block;
    register struct zone_item *z;
    register struct oi_item *o;
    register long k, split;
    struct of_header    of_new;
    struct of_rmks_item or_new;

#ifdef DEBUG
    fprintf(DF, "order_split()\n");
#endif

    if (oc_find(buf->m_pickline, buf->m_neworder)) return 1;  /* new is dup    */

    block = oc_find(buf->m_pickline, buf->m_order);  /* find order in index    */
    if (!block) return 1;

    o = &oc->oi_tab[block - 1];              /* point to order index item      */

    if (o->oi_queue == OC_COMPLETE) return 0;  /* ignore complete orders       */

    have_header = 0;
    if (!get_header(o->oi_pl, o->oi_on, LOCK)) return 1;  /* get order record  */

    split = 0;                               /* only split if underway         */

    if (o->oi_queue == OC_UW)                /* order is underway              */
    {
        split = 1;                             /* order will split               */

        for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
        {
            if (z->zt_status == ZS_UNDERWAY ||
                    z->zt_status == ZS_LATE     ||
                    z->zt_status == ZS_EARLY)
            {
                if (z->zt_order == block && !(z->zt_flags & ZoneInactive))
                {
                    z->zt_flags |= ZoneInactive;     /* seize the zone !!!             */
                    return_picks(z);
                    break;
                }
            }
        }
        if (k >= coh->co_zone_cnt) z = 0;
    }
    else z = 0;

    memcpy(&of_new, of_rec, sizeof(struct of_header));
    of_new.of_on = buf->m_neworder;
    if (rf->rf_con) 
    {
        memset(of_new.of_con, 0x20, sizeof(of_new.of_con));
        memcpy(of_new.of_con, buf->m_con, sizeof(buf->m_con));
    }
    of_rec->of_datetime = of_new.of_datetime = time(0);
    of_rec->of_no_picks = of_new.of_no_picks = 0;
    of_rec->of_no_units = of_new.of_no_units = 0;

    pick_setkey(1);

    op_rec->pi_pl  = o->oi_pl;               /* setup to process all picks     */
    op_rec->pi_on  = o->oi_on;
    op_rec->pi_mod = 1;

    pick_startkey(op_rec);

    op_rec->pi_mod = coh->co_prod_cnt;
    pick_stopkey(op_rec);

    while (!pick_next(op_rec, LOCK))
    {
        if (op_rec->pi_flags & PICKED)        /* old pick - only count           */
        {
            of_rec->of_no_picks += 1;
            of_rec->of_no_units += op_rec->pi_ordered;
            continue;
        }
        of_new.of_no_picks += 1;
        of_new.of_no_units += op_rec->pi_ordered;

        op_rec->pi_on = of_new.of_on;          /* now spilt to a new order       */
        pick_update(op_rec);
    }
    if (!of_rec->of_no_picks) split = 0;     /* there are no old picks         */
    if (!of_new.of_no_picks)                 /* there are no picks to split    */
    {
        if (z) message_put(0, ZoneRedisplayRequest, &z->zt_zone, sizeof(TZone));
        return 0;
    }
    o->oi_on = buf->m_neworder;              /* change order index number      */

    if (split)                               /* need two orders                */
    {
        block = oc_write(of_rec->of_pl, of_rec->of_on);
        if (!block) krash("order_split", "order index is full", 1);
        memcpy(oc->oi_tab[block - 1].oi_grp, of_rec->of_grp, GroupLength);
        oc_enqueue(block, OC_LAST, OC_COMPLETE);
        of_rec->of_status = 'c';
        order_update(of_rec);
        order_write(&of_new);
    }
    else order_update(&of_new);              /* only change order number       */

    if (z)                                   /* now restart the zone           */
    {
        z->zt_on = buf->m_neworder;
        message_put(0, ZoneRedisplayRequest, &z->zt_zone, sizeof(TZone));
    }
    if (sp->sp_to_flag != 'n' && sp->sp_to_complete == 'y')
    {
        xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on, of_rec->of_pl,
                'C', 0,0,0,0,0, (z) ? z->zt_zone : 0,0);
    }
    return 0;
}
/*-------------------------------------------------------------------------*
 *  Return Picks From A Seized Zone
 *-------------------------------------------------------------------------*/
return_picks(z)
    register struct zone_item *z;
{
    register struct bay_item *b;
    register struct pw_item  *i;
    register long j, k;
    TPickMessage x;

    k = z->zt_first_bay;

    while (k)
    {
        b = &bay[k - 1];
        k = b->bay_next;

        for (j = b->bay_prod_first; j <= b->bay_prod_last; j++)
        {
            i = &pw[j - 1];

            if (!(i->pw_flags & BinPicked)) continue;

            x.m_pickline  = z->zt_pl;
            x.m_order     = z->zt_on;
            x.m_module    = j;
            x.m_picked    = i->pw_picked;
            x.m_reference = i->pw_reference;

            module_pick_event(&x);               /* process as an event            */
        }
    }
    return 0;
}
/*-------------------------------------------------------------------------*
 *  Lot Change and Split Request
 *-------------------------------------------------------------------------*/
lot_change(x)
    register TLotMessage *x;
{
    TErrorMessage err;
    register struct hw_item   *h;
    register struct pw_item   *i;
    register struct bay_item  *b;
    register struct zone_item *z;
    register struct pl_item   *p;
    register struct st_item   *s;
    register char *lot;

    pmfile_item pm;
    long modno;
    lot_item y;
    char text[80];

#ifdef DEBUG
    fprintf(DF, "lot_control(): pl=%d sku=%-15.15s lot=%-15.15s quan=%d\n",
            x->m_pickline, x->m_key, x->m_lot, x->m_quantity);
#endif

    strip_space(x->m_lot, LotLength);
    sprintf(text, "%d:%-15.15s LOT:%-15.15s", x->m_pickline, x->m_key, x->m_lot);

    strcpy(err.m_text, text);               /* ack/nak confirmation message    */

    s = 0;                                  /* iniiially sku is not found      */

    if (x->m_keytype == 2)                  /* key is stock location           */
    {
        s = stkloc_lookup(x->m_key);
    }
    else if (x->m_keytype == 3)             /* key is module number            */
    {
        sscanf(x->m_key, "%d", &modno);
        s = mod_lookup(modno);
    }
    else                                    /* key is sku number               */
    {
        s = sku_lookup(x->m_pickline, x->m_key);
    }
    if (!s)
    {
        krash("lot_change", text, 0);

        err.m_error = ERR_SKU_INV;
        message_put(who, ClientMessageEvent, &err, strlen(err.m_text) + 2);

        return 0;
    }
    modno = s->st_mod;                      /* module of sku/stkloc            */

    y.l_lot_time = 0;
    y.l_lot_pl   = s->st_pl;
    memcpy(y.l_lot_sku, s->st_sku, SkuLength);
    lot_startkey(&y);

    y.l_lot_time = 0x7fffffff;
    lot_stopkey(&y);

    strip_space(x->m_lot, LotLength);

    while(!lot_next(&y, NOLOCK))            /* ignore any duplicates           */
    {
        strip_space(y.l_lot_number, LotLength);

        if (memcmp(y.l_lot_number, x->m_lot, LotLength) == 0) 
        {
            err.m_error = ERR_DUP_SCAN;         /* confirmation message            */
            message_put(who, ClientMessageEvent, &err, strlen(err.m_text) + 2);
            return 0;
        }
    }
    if (lots && sp->sp_to_flag != 'n' && sp->sp_to_lot_split == 'y')
    {
        have_header = 0;

        lot = lots[modno - 1].lotno;

        if (*lot)
        {
            i = &pw[modno - 1];
            h = &hw[i->pw_ptr - 1];
            b = &bay[h->hw_bay - 1];
            z = &zone[b->bay_zone - 1];

            if ((i->pw_flags & BinHasPick) && 
                    (z->zt_status == ZS_UNDERWAY || z->zt_status == ZS_EARLY))
            {
                get_header(z->zt_pl, z->zt_on, NOLOCK);
            }
            if (have_header)
            {
                xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on, 
                        of_rec->of_pl, 'L', s->st_sku, modno, 0, 0, x->m_quantity,
                        z->zt_zone, lot);
            }
            else
            {
                xt_build(" ", " ", 0, z->zt_pl,
                        'L', s->st_sku, modno, 0, 0, 0, z->zt_zone, lot);
            }
        }
    }
    y.l_lot_time = time(0);
    y.l_lot_pl   = s->st_pl;
    memcpy(y.l_lot_sku, s->st_sku, SkuLength);
    memcpy(y.l_lot_number, x->m_lot, LotLength);

    memcpy(lot, x->m_lot, 15);

    lot_write(&y);

    err.m_error = ERR_CONFIRM;              /* confirmation message            */
    message_put(who, ClientMessageEvent, &err, strlen(err.m_text) + 2);

    return 0;
}

/*-------------------------------------------------------------------------*
 *  Disable Picklines
 *-------------------------------------------------------------------------*/
disable_picklines()
{
    static TPickline zero = 0;

    if (sp->sp_running_status == 'y')
    {
        message_put(0, PicklineDisableRequest, &zero, sizeof(TPickline));
        krash("signal_catcher", "pickline disabled on signal", 0);
    }
    leave(1);
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave(x)
    register long x;
{
    if (sp->sp_to_flag != 'n') xt_close();
    if (sp->sp_box_feature != 'n') boxes_close();
    if (sp->sp_box_feature == 's') box_list_close();
    pmfile_close();
    short_close();
    restock_close();
    lot_close();

    if (lots) free(lots);
    fclose(rfd);

    if (sp->sp_productivity == 'y') pr_close_save();
    ss_close();
    co_close();
    oc_close();
    od_close();
    database_close();
    exit(x);
}
/* end of pick_update_db.c */
