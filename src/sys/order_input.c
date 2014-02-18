/* #define DEBUG */
#define DUPBOX_FIX
/*-------------------------------------------------------------------------*
 *  Custom Versions:  CANTON    - Load number of customer order number.
 *                    WALGREENS - Automatic order start.
 *                    CALVIN    - five digit order number - ignored.
 *                    BACKORDER - changed to custom code
 *                    HANES     - save reports
 *                    AUTOMATIC - Automatic order feed.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order input from ASCII text using either stdin or a file.
 *                  Returns: 0 = OK; 1 = rejects; 2 = serious error.
 *
 *  order_input [-o] [-g] [-s] [-v]  [file] [<file]
 *
 *              -o  prints each input order
 *              -g  prints first order in each group
 *              -s  is silent
 *              -v  validates input only
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/14/93   |  tjt Rewritten for mfc.
 *  03/31/94   |  tjt Added priority 'k' to hold with priority = m.
 *  03/31/94   |  tjt Added rf_hold to hold all and enable 'k' hold.
 *  05/10/94   |  tjt Fix do not add to of_no_units for bypassed sku/mod.
 *  05/10/94   |  tjt Fix reject orders with no picks.
 *  05/25/94   |  tjt Add ignore remarks flag.
 *  05/25/94   |  tjt Add validate option.
 *  05/25/94   |  tjt Add improved error messages.
 *  05/25/94   |  tjt Add validate option.
 *  06/15/94   |  tjt Add pause during markplace/restoreplace.
 *  06/15/94   |  tjt Add order queued transaction.
 *  07/11/94   |  tjt Fix bug in sync() should bypass terminator.
 *  07/11/94   |  tjt Add new orders may overwrite a cancelled order.
 *  07/14/94   |  tjt Fix printlog error when should be silent.
 *  07/15/94   |  tjt Add wrong pickline for sku message.
 *  08/18/94   |  tjt Add always print errors.
 *  08/22/94   |  tjt Fix backorder changes to custom code.
 *  08/22/94   |  tjt Add unit of measure added to order input.
 *  09/07/94   |  tjt Add picks to go to pickline item.
 *  09/07/94   |  tjt Add unit of measure in pick text allowed with dup mod.
 *  10/18/94   |  tjt Add message on premature eof.
 *  10/27/94   |  tjt Add pickline zero.
 *  11/07/94   |  tjt Add save report for possible reprint.
 *  12/14/94   |  tjt Add check for remarks init failure/missing.
 *  12/23/94   |  tjt Add null lot to transactions.
 *  06/04/95   |  tjt Fix unlink report file when no print.
 *  06/29/95   |  tjt Add generate order numbers.
 *  07/01/95   |  tjt Add always accumulate remaining picks.
 *  07/02/95   |  tjt Add hold orders with inhibit picks.
 *  07/02/95   |  tjt Add hold orders with bad sku. (not pickline zero).
 *  07/02/95   |  tjt Add flag orders with inhibited picks.
 *  04/04/96   |  tjt Add multiple picks for mirrored SKU's.
 *  04/17/96   |  tjt Revise to_go.
 *  04/20/96   |  tjt Add stkloc input.
 *  04/30/96   |  tjt Add orphan SKU option with skip_sku options.
 *  04/30/96   |  tjt Allow priority 'k' except when rf_hold in 'n'.
 *  07/05/96   |  tjt Add commit_work calls in dup order messages.
 *  07/31/96   |  tjt Fix change while to if (wipeout).
 *  11/19/96   |  tjt Add automatic order feed option.
 *  12/02/96   |  tjt Add lines and units to location.
 *  10/08/01   |  aha Modified for Eckerd's Tote Integrity to get start and
 *             |      end box numbers from remarks text.
 *  08/30/02   |  aha Modified to check for excessive box number range and
 *             |      duplicate box numbers in boxes table.
 *  04/09/03   |  aha Changed MAX_BOX_RANGE from 100 t0 200.
 *-------------------------------------------------------------------------*/
static char order_input_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/*-------------------------------------------------------------------------*
 *  order_input.c
 *
 *  Order Field Sequence
 *  ---------------------------------------------
 *  [Customer Order Number]       default = space
 *   Order Number                 required
 *  [Priority]                    default = h
 *  [Group]                       default = space
 *  [Pickline]                    default = 1
 *  [Remarks]                     optional
 *   SKU/Module                   required
 *   Quantity                     required
 *  [Pick Text]                   optional
 *
 *   Any field may be variable length terminated by  rf_ft.
 *   Order Number, SKU, Module, and Quantity may be preceded by leading spaces
 *   All records must begin with rf_rp (record preface).
 *   End of file (rf_eof) and record terminator (rf_rt) are optional.
 *
 *   Legal Record Format are as follows:
 *
 *   # record # record # #              (only preface defined)
 *   # record * # record * # *          (only preface and terminator)
 *   # record # record $                (only preface and eof)
 *   # record * # record * $            (all symbols defined)
 *-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ctype.h>
#include "global_types.h"
#include "of.h"
#include "ss.h"
#include "xt.h"
#include "co.h"
#include "st.h"
#include "zone_status.h"
#include "message_types.h"
#include "caps_messages.h"

#include "Bard.h"
#include "box.h"
#include "boxes.h"

extern char gnc();
extern leave();
int sync0();

FILE *od = 0;                             /* log output file                 */
char oname[16] = {0};                     /* log print file name             */

long box_fd = 0;                          /* boxes database                  */
boxes_item bo;                            /* box data item                   */

long verbose = 1;                         /* print error messages            */
long valid   = 0;                         /* dump details                    */
long oflag   = 0;                         /* print orders                    */
long gflag   = 0;                         /* print groups                    */
char last_grp[GroupLength];

FILE *fd;                                 /* input file or stdin             */
char fd_name[64] = {0};                   /* input file name                 */

#ifdef DEBUG
FILE *bug;
char bug_name[40];
#endif

#ifdef DUPBOX_FIX
#define MAX_BOX_RANGE   999
#endif

#define BUF 512                           /* big block input size            */

char buffer[BUF];                         /* input buffer                    */
char *next = buffer;                      /* next input byte                 */
long max = 0;                             /* bytes remaining in buffer       */

char ft  = 0;                             /* field terminator                */

long bad_count = 0;                       /* rejected orders                 */
long good_count = 0;                      /* accepted orders                 */

TZone entry_zone[PicklineMax];            /* entry and exit zones            */
TZone exit_zone[PicklineMax];

struct pending_item pnd;                  /* pending database                */
long pd = 0;                              /* pending file descriptor         */
struct of_pick_item *pi;                  /* pick record                     */
long m;                                   /* number of picks                 */
long pfirst, plast;                       /* pickline range                  */

long order_purge = 0;                     /* purge any completed             */
long purge_time = 0;                      /* purge window                    */

long now;                                 /* current time                    */

/*-------------------------------------------------------------------------*
 *  Order Input 
 *-------------------------------------------------------------------------*/
main(argc, argv)
long argc;
char **argv;
{
  putenv("_=order_input");                /* name to environment             */
  chdir(getenv("HOME"));                  /* to home directory               */

  open_all_files(argc, argv);             /* open all needed files           */

  ft = rf->rf_ft;                         /* save field terminator           */

  get_orders();                           /* process orders                  */
  
  if (bad_count) leave(1);                /* some rejects                    */
  
  leave(0);                               /* successful termination          */
}
/*-------------------------------------------------------------------------*
 *  Terminate Gracefully
 *-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*
 *  Abort Order Input on Serious Error
 *-------------------------------------------------------------------------*/
abort0(text)
char *text;
{
  if (text) fprintf (od, "\n*** %s ***\n", text);
  fprintf (od,"\n*** Order Input Aborted on Serious Error ***\n");
  fprintf (od, "\n\n");
  fprintf (od, "End of Report: %5d Orders Rejected\n", bad_count);
  fprintf (od, "               %5d Orders Accepted\n", good_count);
  fprintf (od, "               %5d Total Orders\n", bad_count + good_count);

  leave(2);
}
leave(x)
register long x;
{
  if (sp->sp_box_feature == 's') boxes_close();

  message_close();
  ss_close();
  oc_close_save();
  od_close();
  co_close_save();
  pending_close();

  database_close();

  if (od) fclose(od);
  if (*fd_name) fclose(fd);

  if (valid || verbose || bad_count) printlog(); /* F071494  F081894         */
  else unlink(oname);
  
  exit(x);
}

/*-------------------------------------------------------------------------*
 *  process orders
 *-------------------------------------------------------------------------*/
get_orders()
{
  register char c;
  register long k, n, wipeout, mflag;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register struct oi_item   *o;
  register struct st_item   *s;
  long block         = 0L,
       box_num       = 0L,
       start_box_num = 0L,
       end_box_num   = 0L;
  struct of_header save;
  char text[80],
       order_start_box[BoxNoLength+1],
       order_end_box[BoxNoLength+1],
       remarks_text[(2 * BoxNoLength) + 1];
#ifdef DUPBOX_FIX
  boxes_item box_dup;
  short dup_box_flag = 0;
  long box_range     = 0L;
  char box_msg[64];
#endif

  wipeout = 0;                            /* no pending groups yet           */

  while(1)                                /* until eof                       */
  {
    skip_white();                         /* skip 0x00 thru 0x20             */

    memset(of_rec, 0, sizeof(struct of_header));
    memset(of_rec->of_grp, 0x20, GroupLength);
    memset(of_rec->of_con, 0x20, CustomerNoLength);

    of_rec->of_datetime = time(0);
    if (rf->rf_hold == 'y') of_rec->of_status = 'h'; /* F033194 add hold all */
    else of_rec->of_status   = 'q';
    of_rec->of_repick   = 'n';
    of_rec->of_pri      = 'm';
                                        /* default to medium priority   */
/*
 *  Check First Byte Must be Preface
 */
    c = gnc();                            /* get preface byte                */

    if (rf->rf_eof && c == rf->rf_eof) break;/* is eof instead               */

    if (c != rf->rf_rp)                   /* first must be preface byte      */
    {
      message("Preface Missing", "Reject");/* no preface byte                */
      sync0();                             /* recover to next record          */
      continue;                           /* loop again                      */
    }
    c = gnc();                            /* get next byte                   */

/*
 *  Check Null Record as EOF
 */
    if (!rf->rf_rt && !rf->rf_eof && c == rf->rf_rp) break;
    if (rf->rf_rt  && !rf->rf_eof && c == rf->rf_rt) break;

    unc();                                /* unget last byte                 */
/*
 * Process Each Input Field
 */
    if (valid) fprintf(od, "\n\n");       /* space between orders            */

    if (get_con() == -1) continue;
    if (get_on()  == -1) continue;
    if (get_pri() == -1) continue;
    if (get_grp() == -1) continue;
    if (get_pl()  == -1) continue;
    if (get_rmk() == -1) continue;
    if (get_pk()  == -1) continue;

#ifdef CANTON
    memcpy(of_rec->of_con, or_rec->rmks_text + 6, 4);
#endif

    if (rf->rf_rt)                        /* terminator required             */
    {
      c = gnc();
      if (c != rf->rf_rt)
      {
        fprintf(od, "Order Terminator Missing\n");
        unc();
      }
    }
/*
 *  Check Markplace/Restoreplace Has Started
 */
    while (sp->sp_in_process_status != 'x')      /* 0615094                  */
    {
      sleep(5);
    }
/*
 *  Check All Picklines For Duplicates
 */
    oc_lock();

    if (sp->sp_order_input_purge == 'm' && sp->sp_pickline_zero != 'n')
    {
      mflag = 0;

      for (of_rec->of_pl = pfirst; of_rec->of_pl <= plast; of_rec->of_pl += 1)
      {
        memcpy(&save, of_rec, sizeof(struct of_header));

        if (sp->sp_use_con == 'y') order_setkey(2);
        else order_setkey(1);

        if (!order_read(of_rec, NOLOCK))    /* read any duplicate order      */
        {
          if (of_rec->of_status != 'h' && 
              of_rec->of_status != 'c' &&
              of_rec->of_status != 'x')
          {
            message("Duplicate Order", "Reject");
            mflag = 1;
          }
        }
        memcpy(of_rec, &save, sizeof(struct of_header));
      }
      if (mflag) {oc_unlock(); continue;}
    }
/* 
 *  Check if Order is a Duplicate In Any Queue
 */
    for (of_rec->of_pl = pfirst; of_rec->of_pl <= plast; of_rec->of_pl += 1)
    {
      memcpy(&save, of_rec, sizeof(struct of_header));

      if (sp->sp_use_con == 'y') order_setkey(2);
      else order_setkey(1);

      begin_work();
      if (!order_read(of_rec, LOCK))        /* read any duplicate order      */
      {
        if ((block = oc_find(of_rec->of_pl, of_rec->of_on)))
        {
          if (sp->sp_order_input_purge == 'm')
          {
            if (of_rec->of_status != 'h' && 
                of_rec->of_status != 'c' &&
                of_rec->of_status != 'x')
            {
              oc_unlock();
              message("Duplicate Order", "Reject");
              commit_work();
              continue;
            }
            message("Replacing Order", "Accept");
          }
          else if (sp->sp_order_input_purge == 'n' ||
             (sp->sp_order_input_purge == 'x' && of_rec->of_status != 'x') ||
              oc->oi_tab[block - 1].oi_queue != OC_COMPLETE)
          {
            oc_unlock();
            message("Duplicate Order", "Reject");
            commit_work();
            continue;
          }
          oc_dequeue(block);
          oc_delete(block);
        }
        od_delete();
        
        if (sp->sp_box_feature == 's')
        {
          bo.b_box_pl     = of_rec->of_pl;
          bo.b_box_on     = of_rec->of_on;
          bo.b_box_number = 0;

          boxes_startkey(&bo);
        
          bo.b_box_number = DUMMY_BOX;
          boxes_stopkey(&bo);
          
          while (!boxes_next(&bo, LOCK)) boxes_delete();
        }
        memcpy(of_rec, &save, sizeof(struct of_header));
      }
      else if (order_purge)
      {
        block = oc->oc_tab[of_rec->of_pl - 1].oc_comp.oc_first;

        if (block)
        {
          memcpy(&save, of_rec, sizeof(struct of_header));

          od_get(block);                    
      
          if (of_rec->of_datetime <= purge_time)
          {
            oc_dequeue(block);
            oc_delete(block);
            od_delete();

            if (sp->sp_box_feature == 's')
            {
              bo.b_box_pl     = of_rec->of_pl;
              bo.b_box_on     = of_rec->of_on;
              bo.b_box_number = 0;

              boxes_startkey(&bo);
          
              bo.b_box_number = DUMMY_BOX;
              boxes_stopkey(&bo);
          
              while (!boxes_next(&bo, LOCK)) boxes_delete();
            }
          }
          else order_purge = 0;              /* no more orders to purge      */
        
          memcpy(of_rec, &save, sizeof(struct of_header));
        }
      }
      commit_work();
/*
 *  Check Is Pending Action
 */
      pnd.pnd_pl = of_rec->of_pl;

      if (sp->sp_use_con == 'y')
      {
        memcpy(pnd.pnd_con, of_rec->of_con, CustomerNoLength);
        pending_setkey(3);
      }
      else
      {
        pnd.pnd_on = of_rec->of_on;
        pending_setkey(1);
      }
      begin_work();
      if (!pending_read(&pnd, LOCK))
      {
        if (pnd.pnd_flags & PENDING_HOLD)
        {
          of_rec->of_status = 'h';
          message ("Order Placed on Hold", "Accepted");
          pending_delete();
        }
        else if (pnd.pnd_flags & PENDING_CANCEL)
        {
          oc_unlock();
          pending_delete();
          message ("Order Cancelled", "Accepted");
          if (sp->sp_to_flag != 'n' && sp->sp_to_cancel == 'y')
          {
            xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on,
              of_rec->of_pl, 'X', 0, 0, 0, 0, 0, 0);
          }
          commit_work();
          continue;
        }
        else 
        {
          oc_unlock();
          message("Bad Pending Flag", "Skip");
        }
      }
      else if (of_rec->of_grp[0] != 0x20);
      {
        pnd.pnd_pl = of_rec->of_pl;
        memcpy(pnd.pnd_group, of_rec->of_grp, GroupLength);
        pending_setkey(2);

        if (!pending_read(&pnd, LOCK))
        {
          if (!(pnd.pnd_flags & PENDING_USED))
          {
            pnd.pnd_flags |= PENDING_USED;
            pending_replace(&pnd);
            wipeout = 1;                  /* flag purge index file           */
          }
          if (pnd.pnd_flags & PENDING_HOLD)
          {
            of_rec->of_status = 'h';      /* mark order held                 */
            message ("Order Placed on Hold", "Accepted");
          }
          else if (pnd.pnd_flags & PENDING_CANCEL)
          {
            message ("Order Cancelled", "Accepted");

            if (sp->sp_to_flag != 'n' && sp->sp_to_cancel == 'y')
            {
              xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on,
                of_rec->of_pl, 'X', 0, 0, 0, 0, 0, 0);
            }
            commit_work();
            continue;
          }
          else 
          {
            oc_unlock();
            message("Bad Pending Flag", "Skip");
          }
        }
      }
      commit_work();
      
      begin_work();
      block = oc_write(of_rec->of_pl, of_rec->of_on);
  
#ifdef DEBUG
  fprintf(bug, "new order block=%d pl=%d on=%d\n", 
    block, of_rec->of_pl, of_rec->of_on);
#endif

      if (!block) abort0("Order Index Is Full");

      o = &oc->oi_tab[block - 1];

      memcpy(o->oi_grp, of_rec->of_grp, GroupLength);
      memcpy(o->oi_con, of_rec->of_con, ConLength);
      
      if (exit_zone[of_rec->of_pl - 1] > 0)
      {
        o->oi_entry_zone = entry_zone[of_rec->of_pl - 1];
        o->oi_exit_zone  = exit_zone[of_rec->of_pl - 1];
      }
      else
      {
        o->oi_entry_zone = pl[of_rec->of_pl - 1].pl_first_zone;
        o->oi_exit_zone  = pl[of_rec->of_pl - 1].pl_first_zone;
      }
      if (of_rec->of_status == 'h')
      {
        oc_enqueue(block, OC_LAST, OC_HOLD);
      }
      else
      {
        switch (of_rec->of_pri)
        {
          case 'h': oc_enqueue(block, OC_LAST, OC_HIGH); break;
       
          case 'm': oc_enqueue(block, OC_LAST, OC_MED); break;

          case 'l': oc_enqueue(block, OC_LAST, OC_LOW); break;
        }
      }
      if (sp->sp_to_flag != 'n' && sp->sp_to_order_queued == 'y')
      {
        xt_build(of_rec->of_con, of_rec->of_grp, of_rec->of_on,
          of_rec->of_pl, 'Q', 0, 0, 0, 0, 0, 0);
      }
      if (oflag) message("", "Queued");
      else if (gflag)
      {
        if (memcmp(of_rec->of_grp, last_grp, GroupLength) != 0)
        {
          message("First In Group", "Queued");
        }
      }
      memcpy(last_grp, of_rec->of_grp, GroupLength);

      if (rf->rf_rmks && rf->rf_ignore_rmks != 'y')  /* F052594              */
      {
        or_rec->rmks_pl = of_rec->of_pl;
        or_rec->rmks_on = of_rec->of_on;

        if (sp->sp_box_feature == 's')
           {
              memset(remarks_text, 0x0, (2 * BoxNoLength) + 1);
              memset(order_start_box, 0x0, BoxNoLength + 1);
              memset(order_end_box, 0x0, BoxNoLength + 1);

              strncpy(remarks_text, or_rec->rmks_text, (2 * BoxNoLength) + 1);
              remarks_text[2 * BoxNoLength] = '\0';
              strncpy(order_start_box, &remarks_text[0], BoxNoLength);
              order_start_box[BoxNoLength]  = '\0';
              strncpy(order_end_box, &remarks_text[BoxNoLength], BoxNoLength);
              order_end_box[BoxNoLength]    = '\0';

              start_box_num = atol(order_start_box);
              end_box_num   = atol(order_end_box);

#ifdef DUPBOX_FIX
              box_range = end_box_num - start_box_num + 1L;

              if (box_range > MAX_BOX_RANGE)
                 {
                   memset(box_msg, 0x0, 64);
                   sprintf(box_msg, 
                           "Box Range: %-d ",
                           box_range);
                   message(box_msg, "Reject");
                   oc_dequeue(block);
                   oc_delete(block);
                   continue;
                 }
#endif

              bo.b_box_pl        = or_rec->rmks_pl;
              bo.b_box_on        = or_rec->rmks_on;
              bo.b_box_status[0] = BOX_UNUSED;
              bo.b_box_status[1] = '\0';
              bo.b_box_last[0]   = '\0';
              bo.b_box_lines     = 0;
              bo.b_box_units     = 0;

#ifdef DUPBOX_FIX
              dup_box_flag = 0;
#endif
              for (box_num = start_box_num; box_num <= end_box_num; box_num++)
                  {
                     bo.b_box_number = box_num;
#ifdef DUPBOX_FIX
                     memset(&box_dup, 0x0, sizeof(boxes_item));
                     if (!boxes_query(&box_dup, box_num))
                        {
                          memset(box_msg, 0x0, 64);
                          sprintf(box_msg, 
                                  "Dupl. Box: %-9.9d",
                                  box_num);
                          message(box_msg, "Reject");
                          oc_dequeue(block);
                          oc_delete(block);
                          dup_box_flag = 1;
                          break;
                        }
#endif
                     begin_work();
                     boxes_write(&bo);
                     commit_work();
                  }

#ifdef DUPBOX_FIX
              if (dup_box_flag)
                 {
                   continue;
                 }
#endif
           }

        remarks_write(or_rec);
      }
      of_rec->of_no_picks = 0;
      of_rec->of_no_units = 0;

      for (k = 0; k < m; k++)
      {
        if (pi[k].pi_pl != of_rec->of_pl) continue;

        i = &pw[pi[k].pi_mod - 1];
        h = &hw[i->pw_ptr - 1];

        if (pi[k].pi_flags & PicksInhibited) o->oi_flags |= INHIBITED;

        if (pi[k].pi_flags & VALIDATED)
        {
          pl[of_rec->of_pl - 1].pl_lines_to_go += 1;
          pl[of_rec->of_pl - 1].pl_units_to_go += pi[k].pi_ordered;

          i->pw_lines_to_go += 1;
          i->pw_units_to_go += pi[k].pi_ordered;
        }
        else o->oi_flags |= ORPHANS;
        
        of_rec->of_no_picks += 1;
        of_rec->of_no_units += pi[k].pi_ordered;
       
        pick_write(&pi[k]);
        
        if ((pi[k].pi_flags & VALIDATED) &&      
             sp->sp_sku_support == 'y' && sp->sp_mirroring == 'y')
        {
          s = sku_lookup(pi[k].pi_pl, pi[k].pi_sku);  /* 04/04/96            */
        
          pi[k].pi_flags |= MIRROR;        /* dup mirrored pick              */
          
          while (s->st_mirror > 0)         /* duplicate picks for mirroring  */
          {
            s++;
            pi[k].pi_mod = s->st_mod;
            
            if (s->st_mod <= coh->co_prod_cnt)
            {
              i = &pw[s->st_mod - 1];
              h = &hw[i->pw_ptr - 1];
              if (h->hw_bay)
              {
                b = &bay[h->hw_bay - 1];
                if (b->bay_zone)
                {
                  z = &zone[b->bay_zone - 1];

                  pi[k].pi_zone = z->zt_zone;
            
                  if (z->zt_zone < o->oi_entry_zone) 
                    o->oi_entry_zone = z->zt_zone;

                  if (z->zt_zone > o->oi_exit_zone)  
                    o->oi_exit_zone  = z->zt_zone;
                                    
                  pick_write(&pi[k]);
                }
              }
            }
          }
        }
      }
      if (sp->sp_pickline_zero == 'z' && of_rec->of_no_picks < 1)
      {
        oc_dequeue(block);                 /* null order removed             */
        oc_delete(block);
        continue;
      }
      if (order_write(of_rec))
      {
        bad_count++;
        abort0("Attempt To Duplicate Order In Database");
      }
      commit_work();
      
      good_count++;

#ifdef AUTOMATIC
      if (sp->sp_running_status != 'y') continue;   /* check is running      */
      
      if (o->oi_queue == OC_HOLD) continue;  /* order was held               */

      if (pl[o->oi_pl - 1].pl_flags &
         (OrdersLocked | StopOrderFeed | SwitchesDisabled)) continue;
      
      for (z = &zone[o->oi_entry_zone - 1]; z >= zone; z--)
      {
    	  if (!(z->zt_flags & (FirstZone | JumpZone))) continue;
        
        switch (z->zt_status)
        {
          case ZS_COMPLETE:                /* can start an order             */
          case ZS_WAITING:
    	              message_put(0, ZoneNextEvent, &z->zt_zone, sizeof(TZone));
    	              break;
    	              
    	    default:  break;                /* cannot start order              */ 
    	  }
        break;                            /* exit on first zone found        */
    	}
#endif

#ifdef WALGREENS
      now = time(0);
      
      sprintf(text, "echo %02d %06d %24.24s >>%s/dat/log/order_input.log",
        getenv("HOME"), o->oi_pl, o->oi_on, ctime(&now));
      system(text);

      if (o->oi_queue == OC_HOLD) continue;

      if (o->oi_exit_zone <= 0) continue;
    
      for (z = &zone[o->oi_entry_zone - 1]; z >= zone; z--)
      {
    	  if (!(z->zt_flags & (FirstZone | JumpZone))) continue;
        
        sprintf(text, "Zone %d", z->zt_zone);

        switch (z->zt_status)
        {
          case ZS_AHEAD:
          case ZS_UNDERWAY:
          case ZS_EARLY:
          case ZS_LATE: 
          case ZS_LOCKED:     
          case ZS_INACTIVE: 
          case ZS_OFFLINE:  
                    message(text, "Busy");
                    break;

          case ZS_COMPLETE:
          case ZS_WAITING:
                    message(text, "Started");
    	              message_put(0, ZoneNextEvent, &z->zt_zone, sizeof(TZone));
    	              break;
    	  }
        break;                            /* exit on first zone found        */
    	}
#endif
    }                                     /* end of output loop              */
    oc_unlock();
  }                                       /* end MAIN LOOP                   */
/*
 *  Purge Used Groups from Index
 */
  if (wipeout)                            /* for entire index  F073196       */
  {
    pending_setkey(0);                    /* physical order                  */
     
    begin_work();
    while (!pending_next(&pnd, LOCK))     /* scan whole file for deletes     */
    {
      if (pnd.pnd_flags & PENDING_USED) pending_delete();
    }
    commit_work();
  }
/*
 *  End of File
 */
  fprintf (od, "\n\n");
  fprintf (od, "End of Report: %5d Orders Rejected\n", bad_count);
  fprintf (od, "               %5d Orders Accepted\n", good_count);
  fprintf (od, "               %5d Total  Orders\n", bad_count + good_count);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Release Input Log to Print Queue
 *-------------------------------------------------------------------------*/
printlog()
{
  static char pname[16];
  char command[80];
  long pid, status;

  if (fork() == 0)
  {
#ifdef HANES
    sprintf(command, "cp %s tmp/batch.%.*s", oname, GroupLength, last_grp);
    system(command);
#endif
    
    tmp_name(pname);
    execlp("prft", "prft", oname, pname, "sys/report/order_input_report.h", 0);
    krash("printlog", "load prft", 1);
  }
  pid = wait(&status);
  if (pid < 0 || status) krash("printlog", "prft failed", 1);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Message to Log
 *-------------------------------------------------------------------------*/
message(string, condition)
char *string, *condition;
{
  char text[128];

  if (memcmp(condition, "Reject", 6) == 0) bad_count++;

  sprintf(text, "%-7.*d %15.15s   %2d   %-6.*s   %c   %-20s %s\n",
    rf->rf_on, of_rec->of_on, of_rec->of_con, of_rec->of_pl, 
    GroupLength, of_rec->of_grp, of_rec->of_pri, string, condition);
  
  fprintf(od, "%s", text);

#ifdef DEBUG
  fprintf(bug, "%s", text);
#endif

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Customer Order Number
 *-------------------------------------------------------------------------*/
get_con()
{
#ifdef CALVIN
  char work[8];
#endif

#ifdef DEBUG
  fprintf(bug, "\nget_con() ");
#endif
  if (rf->rf_con < 1) return 0;           /* no customer order number        */
  
  get_field(of_rec->of_con, rf->rf_con);
  
  if (valid) valid_dump(1, of_rec->of_con, rf->rf_con);
  
#ifdef CALVIN
  memcpy(work, of_rec->of_con, 7);
  work[7] = 0;
  
  of_rec->of_on = atol(work);
#endif

  return 0;
}
/*-------------------------------------------------------------------------*
 * Get Order Number
 *-------------------------------------------------------------------------*/
get_on()
{
  register long x;
  char on_text[8], text[24];
  
#ifdef DEBUG
  fprintf(bug, "\nget_on() ");
#endif
  skip_white();                           /* bypass whitespace               */

#ifdef CALVIN
  x = get_number(on_text[8], 5);
#else

  if (sp->sp_use_con == 'y')              /* generate order number           */
  {
    oc->of_last_on += 1;
    if (oc->of_last_on > OrderMax) oc->of_last_on = 1;
    of_rec->of_on = oc->of_last_on;
    return 0;
  }
  x = get_number(on_text, rf->rf_on);
  if (valid) valid_dump(2, on_text, rf->rf_on);

  if (x < 1)
  {
    sprintf(text, "Order %s Invalid", on_text);
    message (text, "Reject");
    sync0();
    return -1;
  }
  of_rec->of_on = x;
#endif

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Pickline
 *-------------------------------------------------------------------------*/
get_pl()
{
  register long x;
  char pl_text[4], text[24];
  
#ifdef DEBUG
  fprintf(bug, "\nget_pl() ");
#endif
  if (rf->rf_pl < 1)
  {
    if (sp->sp_pickline_zero != 'n')      /* F102794 assign pickline        */
    {
      of_rec->of_pl = 0;
      pfirst = 1; 
      plast  = coh->co_pl_cnt;
      return 0;
    }
    of_rec->of_pl = pfirst = plast = 1;   /* default                         */
    return 0;
  }
  skip_white();                           /* bypass whitespace               */

  x = get_number(pl_text, rf->rf_pl);

  if (valid) valid_dump(5, pl_text, rf->rf_pl);

  of_rec->of_pl = pfirst = plast = x;     /* store pickline  F102894         */

  if (x < 1)
  {
    if (sp->sp_pickline_zero != 'n')      /* F102794 parallel when zero     */
    {
      of_rec->of_pl = 0;
      pfirst = 1; 
      plast  = coh->co_pl_cnt;
      return 0;
    }
    sprintf(text, "Pickline %s Invalid", pl_text);
    message (text, "Reject");
    sync0();
    return -1;
  }
  if (!co) return 0;                      /* no configuration                */
  
  if (x <= coh->co_pl_cnt)                /* within range                    */
  {
    if (pl[x - 1].pl_pl) return 0;        /* pickline is in configuration    */
  }
  message("Pickline Is Invalid", "Reject");
  sync0();
  return -1;
}
/*-------------------------------------------------------------------------*
 *  Get Group
 *-------------------------------------------------------------------------*/
get_grp()
{
  register long k;
  
  if (rf->rf_grp < 1) return 0;

#ifdef DEBUG
  fprintf(bug, "\nget_grp() ");
#endif

  get_field(of_rec->of_grp, rf->rf_grp);

  if (valid) valid_dump(4, of_rec->of_grp, rf->rf_grp);
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Priority
 *-------------------------------------------------------------------------*/
get_pri()
{
  char c;

  if (rf->rf_pri != 1) return 0;

  get_field(&of_rec->of_pri, rf->rf_pri);
  
  if (valid) valid_dump(3, &of_rec->of_pri, 1);

  if (isupper(of_rec->of_pri)) of_rec->of_pri += 0x20;

  if (of_rec->of_pri == 'k')   
  {
    of_rec->of_pri = 'm';                  /* priority k is hold  F0303194   */
    if (rf->rf_hold != 'n') of_rec->of_status = 'h';
    return 0;
  }
  if (of_rec->of_pri != 'h' && of_rec->of_pri != 'm' && of_rec->of_pri != 'l')
  {
    message ("Priority Not H,M,L", "Reject");
    sync0();
    return -1;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Remarks  
 *-------------------------------------------------------------------------*/
get_rmk()
{
  register char *p;
  char r_text[1024];

  if (rf->rf_rmks < 1) return 0;
#ifdef DEBUG
  fprintf(bug, "\nget_rmks() ");
#endif

  if (rf->rf_ignore_rmks == 'y') p = r_text;
  else 
  {
    if (!or_rec) krash("get_rmk", "remarks init error", 1);  /* F121494 */
    p = or_rec->rmks_text;
  }
  get_field(p, rf->rf_rmks);
  
  if (valid) valid_dump(6, p, rf->rf_rmks);
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get All Picks For An Order
 *-------------------------------------------------------------------------*/
get_pk()
{
  register char c;
  register long k, ret, skip;

  m = 0;                                  /* clear pick count                */

  for (k = 0; k < PicklineMax; k++)
  {
    entry_zone[k] = sp->sp_zones;
    exit_zone[k]  = 0;
  }
#ifdef DEBUG
  fprintf(bug, "\nget_pk() ");
#endif

  while (1)
  {
    skip_white();                         /* bypass any white space          */

    c = gnc();
    
    if (c == rf->rf_rp  && !rf->rf_rt) break;/* new order                    */
    if (c == rf->rf_eof && !rf->rf_rt) break;/* end of file                  */
    if (c == rf->rf_rt  &&  rf->rf_rt) break;/* order terminator             */

    unc();                                /* replace first byte              */
    skip = 0;                             /* skip flag                       */

    memset(&pi[m], 0, sizeof(struct of_pick_item));
    pi[m].pi_pl = of_rec->of_pl;
    pi[m].pi_on = of_rec->of_on;          /* setup for a new item            */
    
    if (sp->sp_use_stkloc == 'y' && rf->rf_stkloc > 0) ret = get_stkloc();
    else if (rf->rf_sku)          ret = get_sku();
    else                          ret = get_pm();

    if (ret == -1) return -1;             /* an error has occured            */
    if (ret == -2) skip = 1;

    skip_white();
    ret = get_quan();
    if (ret == -1) return -1;             /* an error has occured            */
    if (ret == -2) skip = 1;

    if (rf->rf_pick_text) get_pt();       /* get pick text                   */

    if (!skip)                            /* check for dups picks            */
    {
      for (k = 0; k < m; k++)
      {
        if (rf->rf_sku)
        {
           if (memcmp(pi[k].pi_sku, pi[m].pi_sku, rf->rf_sku) != 0) continue;
        }
        else if (pi[k].pi_mod != pi[m].pi_mod) continue;

        if (rf->rf_dup_flag == 'n')
        {
          message("Duplicate Pick", "Reject");
          sync0();
          return -1;
        }
        if (rf->rf_dup_flag == 's')
        {
          message("Duplicate Pick", "Skip");
          skip = 1;
          break;
        }
        pi[k].pi_ordered += pi[m].pi_ordered;
        skip = 2;
        break;
      }                                   /*  end duplicates found           */
    }
    if (skip == 1)                        /* skip bad sku/mod                */
    {
      fprintf(od, "Bypassing: SKU %15.15s Module %4d Quan: %3d\n",
        pi[m].pi_sku, pi[m].pi_mod, pi[m].pi_ordered);
      continue;
    }                                     /* end of dup check                */
#ifdef DEBUG
  fprintf(bug, "\nPick: pl=%d on=%d mod=%d quan=%d sku=%*.*s",
  pi[m].pi_pl, pi[m].pi_on, pi[m].pi_mod, pi[m].pi_ordered, 
  rf->rf_sku, rf->rf_sku, pi[m].pi_sku);
#endif

    if (skip == 2) continue;             /* pick added to other pick         */
    m++;                                 /* step to next pick                */

    if (m > oc->of_max_picks)
    {
      message("Too Many Picks", "Reject");
      sync0();
      return -1;
    }
  }
  unc();                                  /* replace terminator              */

  if (m <= 0)                             /* F051094 - reject no picks       */
  {
    message("No Picks", "Reject");
    sync0();
    return -1;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get SKU
 *-------------------------------------------------------------------------*/
get_sku()
{
  register struct st_item   *s;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k;
  char text[64];
   
#ifdef DEBUG
  fprintf(bug, "\nget_sku(%d) ", m);
#endif
  get_field(pi[m].pi_sku, rf->rf_sku);
  
  if (valid) valid_dump(7, pi[m].pi_sku, rf->rf_sku);

  if (!co) return 0;

  for (k = pfirst; k <= plast; k++)
  {
    s = (struct st_item *)sku_lookup(k, pi[m].pi_sku);
    if (s) break;
  }
  if (s)
  {
    pi[m].pi_mod = s->st_mod;

    if (s->st_mod <= coh->co_prod_cnt)
    {
      i = &pw[s->st_mod - 1];
      h = &hw[i->pw_ptr - 1];
      if (h->hw_bay)
      {
        b = &bay[h->hw_bay - 1];
        if (b->bay_zone)
        {
          z = &zone[b->bay_zone - 1];

          if (k == z->zt_pl)
          {
            pi[m].pi_pl     = k;
            pi[m].pi_flags |= VALIDATED;
            pi[m].pi_zone   = z->zt_zone;
            
            if (z->zt_zone < entry_zone[k-1]) entry_zone[k-1] = z->zt_zone;
            if (z->zt_zone > exit_zone[k-1])  exit_zone[k-1]  = z->zt_zone;

            if (i->pw_flags & PicksInhibited && rf->rf_hold == 'i')
            {
              of_rec->of_status = 'h';
            }
            return 0;
          }
        }
      }
    }
  }
  for (k = 1; k <= coh->co_pl_cnt; k++)
  {
    if (sku_lookup(k, pi[m].pi_sku)) break;
  }
  if (k < coh->co_pl_cnt)
  {
    sprintf(text,"Wrong PL %*.*s ", rf->rf_sku, rf->rf_sku, pi[m].pi_sku);
  }
  else
  {
    sprintf(text,"Missing SKU %*.*s", rf->rf_sku, rf->rf_sku, pi[m].pi_sku);
  }
  if (rf->rf_skip_sku == 'y')
  {
    message(text, "Skip");
    return -2;
  }
  else if (rf->rf_skip_sku == 'o')
  {
    pi[m].pi_pl  = pfirst;
    pi[m].pi_mod = 0;
    
    message(text, "Orphan");
    return 0;
  }
  else if (rf->rf_skip_sku == 'h')
  {
    pi[m].pi_pl  = pfirst;
    pi[m].pi_mod = 0;
    
    of_rec->of_status = 'h';
    
    message(text, "Hold");
    return 0;
  }
  message(text, "Reject");                /* reject bad sku                  */
  sync0();
  return -1;
}
/*-------------------------------------------------------------------------*
 *  Get Stock Location
 *-------------------------------------------------------------------------*/
get_stkloc()
{
  register struct st_item   *s;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k;
  char text[64], work[StklocLength];
   
#ifdef DEBUG
  fprintf(bug, "\nget_stkloc(%d) ", m);
#endif
  get_field(work, rf->rf_stkloc);
  
  if (valid) valid_dump(11, work, rf->rf_stkloc);

  if (!co) return 0;

  s = (struct st_item *)stkloc_lookup(work);

  if (s)
  {
    pi[m].pi_mod = s->st_mod;
    memcpy(pi[m].pi_sku, s->st_sku, SkuLength);
    
    if (s->st_mod <= coh->co_prod_cnt)
    {
      i = &pw[s->st_mod - 1];
      h = &hw[i->pw_ptr - 1];
      if (h->hw_bay)
      {
        b = &bay[h->hw_bay - 1];
        if (b->bay_zone)
        {
          z = &zone[b->bay_zone - 1];

          if (s->st_pl == z->zt_pl)
          {
            pi[m].pi_pl     = k = s->st_pl;
            pi[m].pi_flags |= VALIDATED;
            pi[m].pi_zone   = z->zt_zone;
            
            if (z->zt_zone < entry_zone[k-1]) entry_zone[k-1] = z->zt_zone;
            if (z->zt_zone > exit_zone[k-1])  exit_zone[k-1]  = z->zt_zone;

            if (i->pw_flags & PicksInhibited && rf->rf_hold == 'i')
            {
              of_rec->of_status = 'h';
            }
            return 0;
          }
        }
      }
    }
  }
  sprintf(text,"Missing Location %*.*s", rf->rf_stkloc, rf->rf_stkloc, work);

  if (rf->rf_skip_sku == 'y')
  {
    message(text, "Skip");
    return -2;
  }
  message(text, "Reject");                /* reject bad stkloc               */
  sync0();
  return -1;
}
/*-------------------------------------------------------------------------*
 *  Get Module - Return 0 is OK; -1 is error; -2 is skip.
 *-------------------------------------------------------------------------*/
get_pm()
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k, x;
  char mod_text[8], text[64];
  
#ifdef DEBUG
  fprintf(bug, "\nget_pm() ");
#endif

  x = get_number(mod_text, rf->rf_mod);

  if (valid) valid_dump(8, mod_text, rf->rf_mod);

  if (x < 1)
  {
    sprintf (text, "Module %s Invalid", mod_text);
    message (text, "Reject");
    sync0();
    return -1;
  }
  pi[m].pi_mod = x;

  sprintf(pi[m].pi_sku, "%05d", x);

  if (!co) return 0;

  if (x <= coh->co_prod_cnt)
  {
    i = &pw[x - 1];
    h = &hw[i->pw_ptr - 1];
    if (h->hw_bay)
    {
      b = &bay[h->hw_bay - 1];
      if (b->bay_zone)
      {
        z = &zone[b->bay_zone - 1];
        k = z->zt_pl;

        if (k == of_rec->of_pl || !of_rec->of_pl)
        {
          pi[m].pi_pl     = k;
          pi[m].pi_flags |= VALIDATED;
          pi[m].pi_zone   = z->zt_zone;
      
          if (z->zt_zone < entry_zone[k-1]) entry_zone[k-1] = z->zt_zone;
          if (z->zt_zone > exit_zone[k-1])  exit_zone[k-1]  = z->zt_zone;

          if (i->pw_flags & PicksInhibited && rf->rf_hold == 'i')
          {
            of_rec->of_status = 'h';
          }
          return 0;
        }
      }
    }
  }
  sprintf(text,"Module %d Missing", x);

  if (rf->rf_skip_sku == 'y')
  {
    message(text, "Skip");
    return -2;
  }
  message(text, "Reject");                /* reject bad sku                  */
  sync0();
  return -1;
}
/*-------------------------------------------------------------------------*
 *  Get Quantity - Return  0 is OK; -1 is error; -2 is bypass.
 *-------------------------------------------------------------------------*/
get_quan()
{
  char q_text[8], text[80];
  register long x;

#ifdef DEBUG
  fprintf(bug, "\nget_quan() ");
#endif

  x = get_number(q_text, rf->rf_quan);
  
  if (valid) valid_dump(9, q_text, rf->rf_quan);

  if (x < 0)
  {
	 sprintf (text, "Quantity %s Invalid", q_text);
    message (text, "Reject");
    sync0();
    return -1;
  }
  pi[m].pi_ordered = x;

  if (x < 1)
  {
    if (rf->rf_zero_quantity == 'y') return 0;

    if (rf->rf_sku) sprintf(text, "SKU %*.*s",
      rf->rf_sku, rf->rf_sku, pi[m].pi_sku);

    else sprintf(text, "Module %d", pi[m].pi_mod);
    
    if (rf->rf_zero_quantity == 'n')
    {
      message(text, "Reject 0 Pick");
      sync0();
      return -1;
    }
    message (text, "Skip 0 Pick");
    return -2;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Pick Text
 *-------------------------------------------------------------------------*/
get_pt()
{
  register char *p;
  register long k;
  
  if (rf->rf_pick_text < 1) return 0;

#ifdef DEBUG
  fprintf(bug, "\nget_pt() ");
#endif
  get_field(pi[m].pi_pick_text, rf->rf_pick_text);

  if (valid) valid_dump(10, pi[m].pi_pick_text, rf->rf_pick_text);
/*
 *  Backorder - any non-space in backorder flag, changes to no-pick.
 */
#ifdef BACKORDER
  p = pi[m].pi_pick_text + BACKORDER;

  if (*p == 0x20) return 0;
  pi[m].pi_flags |= NO_PICK;
#endif

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Skip Rest of an Order
 *-------------------------------------------------------------------------*/
int sync0()
{
  register long k;
  register char c;

  k = 0;
  
  if (valid) fprintf(od, "\nBypassing: ");

  while (1)
  {
    c = gnc();
    
    if (c == rf->rf_rp) break;                 /* found record preface       */
    
    if (c == rf->rf_eof && rf->rf_eof) break;  /* found end of file          */

    if (c == rf->rf_rt && rf->rf_rt) return 0; /* found record terminator    */
                                               /* F071194 was break before   */
    if (valid)
    {
      if (k >= 80) {fprintf(od, "\n           "); k = 0;}
      fprintf(od, "%c", c);
      k++;
    }
  }
  unc();                                  /* unget last byte                 */
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get An Input Field
 *-------------------------------------------------------------------------*/
get_field(p, n)
register char *p;
register long n;
{
  register char c;

  c = gnc();                              /* get first byte                  */

  while (n > 0 && c != ft)                /* fill field until ft (if any)    */
  {
    *p++ = c; n--;                        /* store a good byte               */
    c = gnc();                            /* look ahead one byte             */
  }
  if (n > 0) memset(p, 0x20, n);          /* clear rest to spaces            */

  if (c != ft) unc();                     /* unput good byte                 */
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get An Numeric Input Field
 *-------------------------------------------------------------------------*/
get_number(p, n)
register char *p;
register long n;
{
  register char c;
  register long flag, x;

  x = flag = 0;                           /* clear work area                 */
  c = gnc();                              /* get first byte                  */
  *p++ = c;
  
  while (n > 0 && c != ft)                /* fill field until ft (if any)    */
  {
    if (c < '0' || c > '9') flag = 1;     /* flag error                      */
    else x = 10 * x + (c - '0');          /* convert to integer              */
    n--;                                  /* reduce count                    */
    c = gnc();                            /* look ahead one byte             */
    *p++ = c;
  }
  if (c != ft) unc();                     /* unput good byte                 */
  *--p = 0;
  
  if (flag) return -1;                    /* not a good numeric              */
  return x;                               /* return a value                  */
}

/*-------------------------------------------------------------------------*
 *  Get Next Input Character
 *-------------------------------------------------------------------------*/
char gnc()
{
  static long eof = 0;                    /* eof == 1 is last buffer         */

  while (max < 1)                         /* buffer is empty                 */
  {
    if (eof) 
    {
      message("Premature EOF", "Rejected"); /* F101894                       */
      abort0("Premature End Of File");
    }
    max  = fread(buffer, 1, BUF, fd);
    if (max < BUF) eof = 1;               /* flag last buffer short or empty */
    next = buffer;                        /* reset pointer                   */
  }
  max--;                                  /* reduce bytes in buffer count    */

#ifdef DEBUG
  if (*next <= 0x20) fprintf(bug, " 0x%02x", *next);
  else fprintf(bug, " %c", *next);
#endif

  return *next++;                         /* return a byte                   */
}
/*-------------------------------------------------------------------------*
 *  Unget Last Byte
 *-------------------------------------------------------------------------*/
unc()
{
  if (next > buffer) {next--; max++;}     /* backup one in buffer            */
#ifdef DEBUG
  if (*next <= 0x20) fprintf(bug, " unc()=0x%02x", *next);
  else fprintf(bug, " unc()=%c", *next);
#endif

  return 0;
}

/*-------------------------------------------------------------------------*
 *  Check Is Whitespace 
 *-------------------------------------------------------------------------*/
skip_white()
{
  register char c;
#ifdef DEBUG
  fprintf(bug, "\nskip_white() ");
#endif
  do c = gnc(); while (c <= 0x20);        /* bypass control and space        */
  unc();                                  /* unget last byte                 */
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Dump Information
 *-------------------------------------------------------------------------*/
valid_dump(who, p, n)
register long who;
register char *p;
register long n;
{
  char what[1024];
  register long k;

  if (n > 0) strncpy(what, p, n);
  what[n] = 0;

  switch (who)
  {
    case 1:  fprintf(od, "Cust No: %s  ", what); break;
    case 2:  fprintf(od, "Order: %s  ", what); break;
    case 3:  fprintf(od, "Priority: %s  ", what); break;
    case 4:  fprintf(od, "Group: %s  ", what); break;
    case 5:  fprintf(od, "Pickline: %s  ", what); break;
    case 6:  fprintf(od, "\nRemarks: %-64.64s", what);

             for (k = 64; k < n; k += 64);
             {
               fprintf(od, "\n         %-64.64s", what + k);
             }
             break;
    
    case 7:  fprintf(od, "\nSKU: %s  ", what); break;
    case 8:  fprintf(od, "\nModule: %s  ", what); break;
    case 9:  fprintf(od, "Quan: %s  ", what); break;
    case 10: fprintf(od, "Pick Text: %s  ", what); break;
    case 11: fprintf(od, "Stkloc: %s  ", what); break;
    default: break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
open_all_files(argc, argv)
long argc;
char **argv;
{
  register long k;
  char text[80];

  fd = stdin;                             /* use standard in by default      */

  for (k = 1; k < argc; k++)
  {
    if (strcmp(argv[k], "-s") == 0)      verbose = 0;
    else if (strcmp(argv[k], "-v") == 0) valid   = 1;
    else if (strcmp(argv[k], "-o") == 0) oflag   = 1;
    else if (strcmp(argv[k], "-g") == 0) gflag   = 1;
    else if (!fd_name[0])
    {
      strcpy(fd_name, argv[k]);           /* a real file name                */
      fd = fopen(fd_name, "r");
      if (fd == 0) 
      {
        sprintf(text, "Cannot Open Input File %s", fd_name);
        abort0(text);
      }
    }
  }
  tmp_name(oname);                        /* get error/log file name         */
  od = fopen(oname, "w");
  if (od == 0)
  {
    krash("order_input", "tmp file", 1);
    exit(1);
  }
  database_open();
  ss_open();                              /* open system segment             */
  oc_open();                              /* order index segment             */
  od_open();                              /* order data files                */
  co_open();                              /* config and hardware             */
  message_open();

  pending_open(AUTOLOCK);                 /* pending database                */
  
  if (sp->sp_to_flag != 'n') xt_open();   /* transaction output              */

  if (sp->sp_order_input_anytime != 'y')
  {
    if (sp->sp_config_status != 'y')
    {
      abort0("System Must Be Configured Or In Markplace");
    }
  }
  pi = (struct of_pick_item *)malloc
  	(oc->of_max_picks * sizeof(struct of_pick_item));
  	
  if (!pi) abort0("Cannot Allocate Space For Picks");
  
  if (sp->sp_order_input_purge == 'w' || sp->sp_order_input_purge == 'm')
  {
    order_purge = 1;
    purge_time  = time(0) - sp->sp_purge_window;
  }
  if (sp->sp_order_input_purge == 'y')
  {
    order_purge = 1;
    purge_time  = time(0);
  }
  if (sp->sp_box_feature == 's') boxes_open(AUTOLOCK);

#ifdef DEBUG
  sprintf(bug_name, "debug/order_input.%d", getpid());
  bug = fopen(bug_name, "w");
#endif

  return 0;
}

/* end of order_input.c */
