/*-------------------------------------------------------------------------*
 *  Custom:         PRINTED   - check packing list was printed
 *                  MARKPLACE - disable switches and markplace.
 *                  DISABLE   - disable switches only.
 *                  INFORMIX  - update statistics
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Purge order and box files.
 *
 *  order_purge   [time] [-p=pickline] [-c=cycle] [-a=age]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/27/93    |  tjt  Added to mfc.
 * 05/16/94    |  tjt  Added check_db after purging.
 * 06/20/94    |  tjt  Fix open/close transaction on mark/restoreplace.
 * 06/22/94    |  tjt  Add catch signals and lockm pickline on death.
 * 08/18/94    |  tjt  Fix bug in default time.
 * 11/20/94    |  tjt  Add pickline for purge. 
 * 11/29/94    |  tjt  Add cyclic purge.
 * 11/30/94    |  tjt  Add shutdown catcher.
 * 03/29/95    |  tjt  Fix open/close bug.
 * 07/21/95    |  tjt  Revise Bard calls.
 * 03/11/96    |  tjt  Add markplace or disable.
 * 11/11/96    |  tjt  Do not crash when order record not found.
 * 10/08/01    |  aha  Modified for Eckerd's Tote Integrity.
 *-------------------------------------------------------------------------*/
static char order_purge_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/*
 *  Purges Completed Orders from Order File
 */
#include <stdio.h>
#include <signal.h>
#include "global_types.h"
#include "message_types.h"
#include "of.h"
#include "ss.h"
#include "co.h"
#include "box.h"

#include "Bard.h"
#include "boxes.h"

long stop_it();

boxes_item bo;

long shutdown;                           /* shutdown requested               */
long purgetime;                          /* purge time age                   */
long cycle;                              /* cyclic purge interval            */
long age;                                /* purge age from now               */
long pfirst, plast;                      /* pickline purge range             */

main(argc, argv)
long argc;
char **argv;
{
  register long k;

  putenv("_=order_purge");
  chdir(getenv("HOME"));

  purgetime = time(0);

  pfirst = 1;
  plast  = PicklineMax;
  
  for (k = 1; k < argc; k++)                 /* F112094                      */
  {
    if (memcmp(argv[k], "-p=", 3) == 0)
    {
      pfirst = plast = atol(argv[k] + 3); 
    }
    else if (memcmp(argv[k], "-c=", 3) == 0) /* F112994                      */
    {
      cycle = atol(argv[k] + 3);
    }
    else if (memcmp(argv[k], "-a", 3) == 0)  /* F112994                      */
    {
      age = atol(argv[k] + 3);
    }
    else purgetime = atol(argv[k]);          /* F081894                      */
  }
  message_open();
  message_signal(SIGUSR1, stop_it);

  while (cycle > 0)
  {
    sleep(60 * cycle);
    purgetime = time(0) - (60 * age);
    purge();
    if (shutdown) leave();
  }
  purge();
  leave();
}
/*--------------------------------------------------------------------------*
 *  Shutdown Message
 *--------------------------------------------------------------------------*/
long stop_it()
{
  shutdown = 1;                           /* shutdown requested              */
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  message_close();
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Purge To Current Purgetime
 *-------------------------------------------------------------------------*/
purge()
{
  static TPickline pickline = 0;
  register long k;
  register struct oc_entry *q;
  register long ret, block;

  open_all();

#ifdef DISABLE
  message_put(0, PicklineDisable, &pickline, sizeof(TPickline));
#endif

#ifdef MARKPLACE
  if (sp->sp_running_status == 'y')
  {
    message_put(0, PicklineDisable, &pickline, sizeof(TPickline));
    sleep(3);
    message_put(0, MarkplaceRequest, 0, 0);
    sleep(7);
  }
#endif

#ifdef INFORMIX
  update_statistics();
#endif

  order_setkey(1);

  for (k = pfirst; k <= plast; k++)
  {
    if (!pl[k - 1].pl_pl) continue;       /* pickline not configured         */
    
    if (shutdown) break;                  /* shutdown requested              */
    q = &oc->oc_tab[k - 1].oc_comp;       /* completed queue                 */

    while (q->oc_first)                   /* until pickline purged F032995   */
    {
      if (shutdown) break;                /* shutdown requested              */
      oc_lock();
      block = q->oc_first;                /* first completed                 */
      
      if (block < 1 || block > oc->of_size) break;  /* bad queue             */

      begin_work();
      
      of_rec->of_pl = oc->oi_tab[block - 1].oi_pl;
      of_rec->of_on = oc->oi_tab[block - 1].oi_on;
   
      ret = order_read(of_rec, LOCK);

      if (ret)                            /* no order record                 */
      {
        oc_dequeue(block);   
        oc_delete(block);
        oc_unlock();
        commit_work();
        continue;
      }
      if (of_rec->of_datetime > purgetime)
      {
        commit_work();
        break;
      }
      if (sp->sp_box_feature == 's')
      {
#ifdef PRINTED
        if (of_rec->of_status == 'c')
        {
          if (!check_printed(of_rec->of_pl, of_rec->of_on)) 
          {
            commit_work();
            continue;
          }
        }
#endif
        delete_boxes(of_rec->of_pl, of_rec->of_on);
      }
      oc_dequeue(block);   
      oc_delete(block);
      od_delete();
      oc_unlock();
      commit_work();
    }
  }
  oc->of_last_purge = time(0);            /* purge completed time            */
  
  close_all();

#ifdef INFORMIX
  update_statistics();
#endif
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Boxes Have Been Printed
 *-------------------------------------------------------------------------*/
check_printed(pickline, order_number)
register long pickline, order_number;
{
  int     ret;

  bo.b_box_pl     = pickline;
  bo.b_box_on     = order_number;
  bo.b_box_number = 0;
  boxes_startkey(&bo);
  
  bo.b_box_number = 999999;
  boxes_stopkey(&bo);
  
  while (!boxes_next(&bo, NOLOCK))
  {
    if(bo.b_box_status[0] != BOX_PRINTED) /* not printed                     */
    return 0;                             /* don't purge                     */
  }
  return 1;

}
/*-------------------------------------------------------------------------*
 *  Delete Entire Box Packing List Data
 *-------------------------------------------------------------------------*/
delete_boxes(pickline, order_number)
register long pickline, order_number;
{
  bo.b_box_pl     = pickline;
  bo.b_box_on     = order_number;
  bo.b_box_number = 0;
  
  boxes_startkey(&bo);
  
  bo.b_box_number = DUMMY_BOX;
  
  boxes_stopkey(&bo);
  
  while (!boxes_next(&bo, LOCK)) boxes_delete();
  
  return;
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
open_all()
{
  database_open();
  ss_open();
  ss_save();
  co_open();
  co_save();
  oc_open();
  oc_save();
  od_open();
  if (sp->sp_box_feature == 's')
  {
    boxes_open(AUTOLOCK);
    boxes_setkey(2);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Close All Files
 *-------------------------------------------------------------------------*/
close_all()
{
  if (sp->sp_box_feature == 's') boxes_close();
  od_close();
  oc_close_save();
  co_close_save();
  ss_close_save();
  database_close();
  return 0;
}

/* end of order_purge.c */
  
