/*-------------------------------------------------------------------------*
 *  Custom Code:    USEHOLD
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Create Anticipated Stockout Report.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/02/93    |  tjt  Added to mfc.
 * 07/22/95    |  tjt  Revise Bard calls.
 * 08/30/95    |  tjt  Use hold queue too.
 * 08/23/96    |  tjt  Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char stockout_create_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      stockout_create.c                                               */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "ss.h"
#include "of.h"
#include "co.h"
#include "stockout_report.t"
#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"

/* arguments to main:

        argv[1] is the entered pickline number.
        argv[2] is the # of orders to consider or 0.
        argv[3] is the until order number or 0.
        argv[4] unused
        argv[5] is the print/report screen flag, which tells this 
                program where to end up.
        argv[6] is the background/foreground flag, which tells this
                program how to exit in event of power failure.
*/

/* global variables:            */
FILE *fp;                                 /* to write report                 */
char report_file[16];                     /* holds temporary file name       */
char print_file[16];                      /* required to print report        */

unsigned char pickline;                   /* argv[1]                         */
long num_of_orders;                       /* argv[2]                         */
long last_order;                          /* argv[3]                         */

struct pm_rec {                           /* pick module record              */

  long pm_num;                            /* pick module number              */
  long qty_on_hand;                       /* quantity                        */
  long qty_required;                      /* begins at zero                  */
  long num_orders;                        /* orders where qty_required       */
};

struct pm_rec *pr;                        /* pointer to structures           */

long orders_proc=0;                       /* count of orders processed       */
long pid, status;                         /* wait parameters                 */

pmfile_item pm;
prodfile_item pf;

long cases_req_fill;                      /* cases to fill lane              */
long units_req_fill;                      /* units to fill lane              */
long total_cases_req;                     /* cases req to do orders          */
long total_units_req;                     /* units req to do orders          */
char rsflag_buf[4];                       /* restock pending flag            */
long case_pack;                           /* units per case                  */
long lane_cap;                            /* lane capacity                   */
long block;                               /* order index block               */

main(argc,argv)
long argc;
char **argv;
{
  register long k;                        /* general purpose                 */
  long ret;                               /* general purpose                 */

  putenv("_=stockout_create");
  chdir(getenv("HOME"));

  pickline      = atol(argv[1]);
  num_of_orders = atol(argv[2]);
  last_order    = atol(argv[3]);
  
  open_all();

        /********************************/
        /*      load structures         */
        /********************************/

  pr= (struct pm_rec *)malloc(sizeof(struct pm_rec)*coh->co_prod_cnt);
  memset(pr, 0, sizeof(struct pm_rec) * coh->co_prod_cnt);

  pmfile_setkey(0);                      /* phiysical order for speed       */

  while(!pmfile_next(&pm, NOLOCK))
  {
    if (pm.p_pmodno > coh->co_prod_cnt) continue;
    
    k = pm.p_pmodno - 1;
    
    pr[k].qty_on_hand = pm.p_qty;
  }

        /****************************************/
        /*      analyze order queues            */
        /****************************************/

           
  oc_lock();                              /* lock index                      */

  block=oc->oc_tab[pickline-1].oc_uw.oc_first;/* underway                    */
  ret = process_stockout(block,num_of_orders,last_order);

  if(ret)
  {
    block=oc->oc_tab[pickline-1].oc_high.oc_first;/* high                    */
    ret = process_stockout(block,num_of_orders,last_order);
  }
  if(ret)
  {
    block=oc->oc_tab[pickline-1].oc_med.oc_first;/* med                      */
    ret = process_stockout(block,num_of_orders,last_order);
  }
  if(ret)
  {
    block=oc->oc_tab[pickline-1].oc_low.oc_first;/* low                      */
    ret = process_stockout(block,num_of_orders,last_order);
  }
#ifdef USEHOLD
  if(ret)
  {
    block=oc->oc_tab[pickline-1].oc_hold.oc_first;/* hold                    */
    ret = process_stockout(block,num_of_orders,last_order);
  }
#endif
  oc_unlock();                            /* unlock index                    */

        /********************************/
        /*      prepare report          */
        /********************************/
  fp = fopen((char *)tmp_name(report_file),"w");

  pmfile_setkey(1);
  prodfile_setkey(1);
  
  begin_work();
  
  for (k = 0; k < coh->co_prod_cnt; k++)
  {
    if(!pr[k].num_orders) continue;       /* qty_required > qty_on_hand.     */
    
    pm.p_pmodno = k + 1;
    if (pmfile_read(&pm, NOLOCK)) continue;

    memcpy(pf.p_pfsku, pm.p_pmsku, 15);
    if (prodfile_read(&pf, NOLOCK)) continue;
      
    case_pack = pf.p_cpack;
    if (case_pack <= 0) case_pack = 1;
      
    lane_cap = pm.p_lcap;
    if (lane_cap  <= 0 || pr[k].qty_on_hand > lane_cap)
    lane_cap  = pr[k].qty_on_hand;

    if(pr[k].qty_on_hand <= 0)
    {
      total_units_req = pr[k].qty_required;
      units_req_fill = lane_cap;
    }
    else
    {
      total_units_req = pr[k].qty_required - pr[k].qty_on_hand;
      units_req_fill = lane_cap - pr[k].qty_on_hand;
    }
    if(case_pack > 0)
    {
      total_cases_req = total_units_req/case_pack;
      cases_req_fill=units_req_fill/case_pack;
    }
    else
    {
      total_cases_req = total_units_req;
      cases_req_fill = units_req_fill;
    }
        
                        /* set restock pending indicator */

    if(pm.p_rsflag =='y') strcpy(rsflag_buf,"***");
    else strcpy(rsflag_buf,"   ");

                        /* first line of report */

    fprintf(fp,
      "%15.15s    %3.3s     %5d        %5d       %5d  %6.6s %6.6s   %3.3s  \n",
      pm.p_pmsku, pf.p_um, pr[k].num_orders,
      cases_req_fill, total_cases_req, pf.p_bsloc,
      pm.p_stkloc, rsflag_buf);

                        /* second line of report */

    fprintf(fp,"%25.25s  %5d      (%5d)     (%5d)  (%6.6s) (%3d)       \n",
      pf.p_descr, pr[k].qty_on_hand,
      units_req_fill, total_units_req, pf.p_absloc,
      case_pack);

                        /* third line of report */
    fprintf(fp,"%80c\n",' ');
    
    commit_work();
    begin_work();
  }
  commit_work();
  fclose(fp);

        /****************************************/
        /*      determine how to exit           */
        /****************************************/

  close_all();
  if(streql(argv[5],"print"))             /* time to print report            */
  {
    if (fork() == 0)                      /* child process                   */
    {
      execlp("prft", "prft", report_file, tmp_name(print_file),
      "sys/report/stockout_print.h",0);

      krash("stockout_create", "prft load", 0);
      exit(1);
    }
    pid = wait(&status);
    if (pid < 0 || status) krash("main", "prft failed", 1);

    execlp("stockout_input", "stockout_input", 0);

    krash("main", "stockout_input load", 1);
  }
  execlp("database_report", "database_report",
    stockout_report, report_file, "81","13","9",
    "sys/report/stockout_print.h",
    "bin/stockout_input","0",0);

  krash("main", "database_report load", 1);
}                                         /* END OF MAIN                     */

/************************************************/
/*      functions for stockout_report.c         */
/************************************************/

/* process_stockout():

        this function processes the pick module data against the
        order queues. returns 1 if end of processing condition not
        reached, 0 if otherwise.

        this function performs the following steps:
        1) update pm_rec table for quantity required this period.
        2) if qty required exceeds qty on hand, write # of orders
           processed to pm_rec table.
*/

process_stockout(block,num_of_orders,last_order)
long block;
long num_of_orders;
long last_order;
{
  register struct oi_item *o;
  register long k;                        /* general purpose                 */
  short last_flag=0;                      /* if set, terminate               */

  while(block)                        /* more orders to process in this queue*/
  {
    o = &oc->oi_tab[block - 1];
    
    orders_proc++;
    
    op_rec->pi_pl  = o->oi_pl;
    op_rec->pi_on  = o->oi_on;
    op_rec->pi_mod = 1;
    
    pick_startkey(op_rec);
    
    op_rec->pi_mod = coh->co_products;
    pick_stopkey(op_rec);

    while (!pick_next(op_rec, NOLOCK))
    {
      if (op_rec->pi_flags & PICKED) continue;
      
      k = op_rec->pi_mod - 1;
      pr[k].pm_num = k + 1;
      
      pr[k].qty_required += op_rec->pi_ordered;
      if (pr[k].qty_required >= pr[k].qty_on_hand)
      {
        if (!pr[k].num_orders) pr[k].num_orders = orders_proc;
      }
    }
    if (o->oi_on == last_order)      return 0;
    if(orders_proc == num_of_orders) return 0;
    
    block = o->oi_flink;                  /* set to next                     */
  }
  return 1;                               /* need more orders                */
}

leave()
{
  close_all();
  krash("leave", "load mmenu", 1);
}


open_all()
{
  database_open();
  ss_open();
  oc_open();
  od_open();
  co_open();
  pmfile_open(READONLY);
  prodfile_open(READONLY);
}

close_all()
{
  pmfile_close();
  prodfile_close();
  co_close();
  oc_close();
  od_close();
  ss_close();
  database_close();
}

/* end of stockout_create.c */
