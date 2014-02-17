/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order commands screen.
 *
 *  Note: Won't sync coordinated orders when sp_use_con == 'y'.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/09/93   |  tjt  Added to mfc.
 *  05/27/94   |  tjt  Canceled to complete queue.
 *  05/27/94   |  tjt  Add pending operations flag.
 *  06/02/94   |  tjt  Remove transactions.
 *  07/11/94   |  tjt  Allow cancelled orders to be repicked.
 *  10/25/94   |  tjt  Fix pickline zero operation.
 *  10/25/94   |  tjt  Coordinated picklines.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *  10/08/01   |  aha  Modified for Eckerd's Tote Integrity.
 *  06/11/02   |  aha  Modified to queue cancelled order for packing list.
 *-------------------------------------------------------------------------*/
static char order_comms_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/****************************************************************************/
/*                                                                          */
/*                           Order Commands Screen                          */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global_types.h"
#include "message_types.h"
#include "caps_messages.h"
#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "order_comms.t"
#include "eh_nos.h"
#include "co.h"
#include "of.h"
#include "language.h"

#include "box.h"
#include "Bard.h"
#include "boxes.h"

extern encode_status();
extern leave();
extern catcher();

#define NUM_PROMPTS     7
#define BUF_SIZE        (CustomerNoLength + 2)

short ONE = 1;
short LPL = 8;
short LON = 5;

struct fld_parms fld[] = {
  {14,57,28,1,&LPL, "Enter Pickline",'a'},
  {15,57,28,1,&ONE, "Enter Code",'a'},
  {16,57,28,1,&LON, "Enter Order Number",'a'},
  {17,57,28,1,&LON, "Enter After Order",'a'},
  {17,57,28,1,&ONE, "Enter Priority",'a'},
  {17,57,28,1,&ONE, "Are You Sure? (y/n)",'a'},
  {18,57,28,1,&ONE, "Create Pending (y/n)", 'a'}
};
/*
 * Global Variables
 */
short rm, pickline, pfirst, plast;

char err_order[16];

long after_order;
long after_order_block;
char after_status;
char after_priority;
long after_queue;

long order;
long order_block;
char order_status;
char order_priority;
long order_queue;

struct pending_item ox;                   /* pending order operations        */
long pd;
long db;                                  /* box database                    */

unsigned char t;
char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */
char yn0[2];
short i,j,k,n,ret,found;

unsigned char list[] = {ClientMessageEvent};

main()
{
  putenv("_=order_comms");                /* program name                    */
  chdir(getenv("HOME"));
  
  open_all();
  
   /* set lengths into field structures */

  LON = rf->rf_on;                        /* order number length             */

  if (sp->sp_use_con == 'y' || sp->sp_use_con == 'b')
  {
    if (rf->rf_con >= LON) LON = rf->rf_con + 1;
  }
  fix(order_comms);
  sd_screen_off();
  sd_clear_screen();
  sd_text(order_comms);
  sd_screen_on();
     
  rm = 1;
  if (IS_ONE_PICKLINE) pickline = op_pl;
  else if (sp->sp_global_order_cmds != 'c')    /*  F102594                   */
  {
    pickline = op_pl;
    if (SUPER_OP) rm = 0;
  }
  
  while(1)
  {
    int go_back = 0;

    memset(buf, 0, NUM_PROMPTS * BUF_SIZE);/* clear to nulls                 */

    sd_cursor(0, fld[3].irow, 1);
    sd_clear_line();
    sd_cursor(0, fld[6].irow, 1);
    sd_clear_line();

    for (j = rm; j < 3; j++) sd_prompt(&fld[j], 0);
 
    while(1)
    {
      i = rm;
      
      while(1)
      {
        t = sd_input(&fld[i],0,0,buf[i],0);
        if(t == EXIT) leave();

        if(t == RETURN)
        {
           strip_space(buf[2], LON);
           if (*buf[2]) break;
	        eh_post(ERR_ORDER, "");
	        continue;
	     }
        if(t == UP_CURSOR) {if (i > rm) i--;}
        else if (i < 2) i++;
        else i = rm;
      }
      if (!rm && *buf[0])                 /* have a pickline entry           */
      {
        pfirst = plast = pl_lookup(buf[0], 0);       
                   
        if (!pfirst)
        {
          if (sp->sp_global_order_cmds == 'n')
          {
            eh_post(ERR_PL, buf[0]);
            continue;
          }
          pfirst = 1;
          plast  = sp->sp_picklines;
        }
        else if (pickline < 0)
        {
          eh_post(ERR_PL, buf[0]);
          continue;
        }
        sprintf(buf[0], "%d", pfirst);
        chng_pkln(buf[0]);              
      }
      else if (sp->sp_global_order_cmds == 'c')
      {
        pfirst = 1;
        plast  = sp->sp_picklines;
      }
      else pfirst = plast = op_pl;
 
      *buf[1] = tolower(*buf[1]);

      if(*buf[1] != 'a' && *buf[1] != 'c' && *buf[1] != 'h' &&
      *buf[1] != 'p' && *buf[1] != 'r' && *buf[1] != 'x')
      {
        eh_post(ERR_CODE,buf[1]);
        continue;
      }
      if (sp->sp_global_order_cmds == 'c' && *buf[1] == 'x')
      {
        eh_post(ERR_CODE,buf[1]);
        continue;
      }
      break;
    }

/*
 * Process the Requested Action
 */
    if(*buf[1] == 'a' || *buf[1] == 'p' || *buf[1] == 'r')
    {
      while(1)
      {
        t = sd_input(&fld[3],sd_prompt(&fld[3],0),0,buf[3],0);
        if(t == EXIT) leave();
        if(t == RETURN) break;
        else
        {
          go_back = 1;
          break;
        }
      }
      if(go_back) continue;
    }
    else if(*buf[1] == 'c')               /* cancel request                  */
    {
      sd_prompt(&fld[5], 0);
      memset(yn0, 0, 2);
      
      while(1)
      {
        t = sd_input(&fld[5],0,0,yn0,0);
        if(t == EXIT) leave();
        *buf[5] = code_to_caps(*yn0);
        if(*buf[5] == 'y') break;
        else if(*buf[5] == 'n')
        {
          go_back = 1;
          break;
        }
        else
        {
          eh_post(ERR_YN,0);
          continue;
        }
      }
      if (go_back) continue;
    }
    else if(*buf[1] == 'x')               /* change priority request         */
    {
      while(1)
      {
        t = sd_input(&fld[4],sd_prompt(&fld[4],0),0,buf[4],0);
        if(t == EXIT) leave();
        if(t == UP_CURSOR) 
        {
          go_back = 1;
          break;
        }
        if(*buf[4] != 'h' && *buf[4] != 'm' && *buf[4] != 'l')
        {
          eh_post(ERR_OF_PRI,0);
          continue;
        }
        else break;
      }
      if (go_back) continue;
    }
    oc_lock();

    ret = check_valid_action();
    if (ret == 1)
    {
      eh_post(LOCAL_MSG, "Action Invalid On Order");
      oc_unlock();
      continue;
    }
    else if (ret == 2)
    {
      eh_post(ERR_ORDER, buf[2]);
      oc_unlock();
      continue;
    }
    else if (ret == 3)
    {
      eh_post(LOCAL_MSG, "Order Partly Missing");
      oc_unlock();
      continue;
    }
    strip_space(buf[3], LON);
    if (*buf[3]) 
    {
      if (check_after_order())
      {
        eh_post(ERR_ORDER, buf[3]);
        oc_unlock();
        continue;
      }
    }
    if (sp->sp_global_order_cmds == 'c')
    {
      if (*buf[1] == 'a' || *buf[1] == 'p' || *buf[1] == 'r' || *buf[1] == 'x')
      {
        if (!*buf[3]) sync_queue();
      }
    }
    for (pickline = pfirst; pickline <= plast; pickline++)
    {
      if (!pl[pickline - 1].pl_pl) continue;

      if (get_after_order()) break;
      begin_work();
      do_action();            
      commit_work();                  /* unlock order if any locked       */
    }
    oc_unlock();
  }
}                                        
/*-------------------------------------------------------------------------*
 *  Open Everything
 *-------------------------------------------------------------------------*/
open_all()
{
  database_open();
  sd_open(catcher);
  ss_open();
  co_open();
  oc_open();
  od_open();

  getparms(0);
  
  pending_open(AUTOLOCK);
  pending_setkey(1);

  if (sp->sp_box_feature == 's')
  {
    boxes_open(AUTOLOCK);
    boxes_setkey(2);
  }

  if (sp->sp_pl_mode == 'c')
     {
       packing_list_open(AUTOLOCK);
     }
}
/*-------------------------------------------------------------------------*
 *  Close All Files
 *-------------------------------------------------------------------------*/
close_all()
{
  pending_close();
  if (sp->sp_box_feature == 's') boxes_close();

  if (sp->sp_pl_mode == 'c')
     {
       packing_list_close();
     }

  oc_close();
  od_close();
  ss_close();
  co_close();
  sd_close();
  database_close();
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  close_all();
  execlp("operm", "operm", 0);
  krash("leave", "operm load", 1);
}
/*-------------------------------------------------------------------------*
 *  Message Catcher
 *--------------------------------------------------------------------------*/
catcher(who, type, buf, len)
long who, type, len;
char *buf;
{
  buf[len] = 0;
  
  if (type == ClientMessageEvent)
  {
    eh_post(*buf, buf + 1);
    return 0;
  }
  leave();
}
/*-------------------------------------------------------------------------*
 *  Process Action
 *-------------------------------------------------------------------------*/
do_action()
{
#ifdef DEBUG
  fprintf(stderr, "do_action(): pickline=%d order=%s\n", pickline, buf[2]);
#endif

  found = 0;                              /* no pending found, yet           */

/*
 * check the order number to be existing in the specified pickline
 */

  order = check_on(pickline, buf[2]);

#ifdef DEBUG
  fprintf(stderr, "do_action():  order =%d\n", order);
#endif

  order_block = oc_find(pickline, order); /* find the order block            */

#ifdef DEBUG
  fprintf(stderr, "do_action():  order_block =%d\n", order_block);
#endif

  sprintf(err_order, "%d-%s", pickline, buf[2]);

  if (order_block > 0)
  {
    od_get(order_block);
    order_status      = of_rec->of_status;
    order_priority    = of_rec->of_pri;
    order_queue       = encode_status(order_status, order_priority);
    if (order_queue < 0)
    {
      eh_post(ERR_ORDER, err_order);
      return;
    }
  }
  else if (sp->sp_pending_ops == 'y')
  {
     ox.pnd_pl = pickline;

     if (sp->sp_use_con == 'y')
     {
       memcpy(ox.pnd_con, buf[2], CustomerNoLength);
       space_fill(ox.pnd_con, CustomerNoLength);
       pending_setkey(3);
     }
     else
     {
       ox.pnd_on = order;
       pending_setkey(1);
     }
     if (pending_read(&ox, LOCK)) found = 0;
     else found = 1;
  }
/*
 * Process the Requested Action
 */
  switch (*buf[1])
  {
    case 'a': return activate_order();
                
    case 'h': return hold_order();
                
    case 'c': return cancel_order();

    case 'p': return pick_order();

    case 'r': return repick_order();
        
    case 'x': return change_priority();

    default:  eh_post(ERR_CODE, buf[1]);
              return;
  }
}
/*-------------------------------------------------------------------------*
 *  Check Valid Order Number
 *-------------------------------------------------------------------------*/
check_on(pickline, buf)
register long pickline;
register char *buf;
{
#ifdef DEBUG
  fprintf(stderr, "check_on(%d, %s)\n", pickline, buf);
#endif

  if (*buf == '#') 
  {
    if (sp->sp_use_con == 'n') return check_con(pickline, buf + 1);
    else                       return atol(buf + 1);
  }
  if (sp->sp_use_con == 'n')   return atol(buf);
  else                         return check_con(pickline, buf);
}
/*------------------------------------------------------------------------*
 *  Check Customer Order Number
 *------------------------------------------------------------------------*/
check_con(pickline, con)
register long pickline;
register char *con;
{
#ifdef DEBUG
  fprintf(stderr, "check_con(%d, %s)\n", pickline, con);
#endif

  strip_space(con, CustomerNoLength);     /* remove any spaces               */

  if (!*con) return 0;                    /* nothing entered                 */

  order_setkey(2);                        /* pickline + con                  */
  
  of_rec->of_pl = pickline;
  memcpy(of_rec->of_con, con, CustomerNoLength);
  space_fill(of_rec->of_con, CustomerNoLength);  /* field is space filled    */
  
  if (!order_read(of_rec, NOLOCK)) return of_rec->of_on;

  return 0;
}

/*------------------------------------------------------------------------*
 *  Confirm Pending
 *------------------------------------------------------------------------*/
confirm()
{
  register unsigned char t;

  sd_prompt(&fld[6], 0);
      
  if (*buf[6] == 'y') return 1;            /* while zero pickline            */
  if (*buf[6] == 'n') return 0;

  memset(yn0, 0, 2);
  
  while(1)
  {
    t = sd_input(&fld[6],0,0,yn0,0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return 0;
    *buf[6] = code_to_caps(*yn0);
    if (*buf[6] == 'y') return 1;
    if (*buf[6] == 'n') return 0;

    eh_post(ERR_YN,0);
  }
}
/*-------------------------------------------------------------------------*
 *  hold_order function
 *-------------------------------------------------------------------------*/
hold_order()
{
  if(order_block)
  {
    if(order_status == 'q')               /* if still queued                 */
    {
      oc_dequeue(order_block);

      of_rec->of_status   = 'h';       /* put in hold queue               */
      of_rec->of_datetime = time(0);
      od_update(order_block);
      oc_enqueue(order_block, OC_LAST, OC_HOLD);
      eh_post(ERR_OF_HOLD, err_order);
    }
    else
    {
      eh_post(ERR_OF_QUE, err_order);
    }
  }
  else if (sp->sp_pending_ops != 'y')
  {
    eh_post(ERR_ORDER, err_order);
    return;
  }
  else if (!found)                         /* pending hold request           */
  {
    if (!confirm())
    {
      eh_post(ERR_OF_ACT, 0);
      return;
    }
    memset(&ox, 0, sizeof(struct pending_item));

    ox.pnd_pl    = pickline;
    ox.pnd_flags = PENDING_HOLD;

    if (sp->sp_use_con == 'y') 
    {
      memcpy(ox.pnd_con, buf[2], CustomerNoLength);
      space_fill(ox.pnd_con, CustomerNoLength);
    }
    else ox.pnd_on = order;

    pending_write(&ox);
    
    eh_post(ERR_OH, err_order);           /*display message                  */
  }
  else
  {
    eh_post(ERR_OF_PND, 0);               /* already pending                 */
  }
  return;
}
/*-------------------------------------------------------------------------*
 *   activate order function
 *-------------------------------------------------------------------------*/
activate_order()
{
  if (order_block)
  {
    if (order_status != 'h')               /* not in hold queue              */
    {
      eh_post(ERR_OF_ACT, 0);
      return;
    }
    if (!after_order)                      /* first if no after              */
    {
      after_status = 'q';
      after_priority = order_priority;
      after_queue    = encode_status(after_status, after_priority);
      if (after_queue < 0)
      {
        eh_post(ERR_ORDER, err_order);    /* bad priority ?                  */
        return;
      }
    }
    if (sp->sp_tl_mode == 'a') enter_high_order();

    oc_dequeue(order_block);

    of_rec->of_status   = after_status;
    of_rec->of_pri      = after_priority;
    of_rec->of_datetime = time(0);
    od_update(order_block);               /* update order file               */

    oc_enqueue(order_block, after_order_block, after_queue);
    eh_post(ERR_OA, buf[2]);              /* confirmation message            */
         
    return;
  }
  if (!found || sp->sp_pending_ops != 'y') /*order not found                 */
  {
    eh_post(ERR_ORDER, buf[2]);
    return;
  }
  pending_delete();
  
  eh_post(ERR_OD, err_order);             /* delete message                  */
  return;
}
/*------------------------------------------------------------------------*
 * cancel order function
 *------------------------------------------------------------------------*/
cancel_order()
{
  TOrderMessage x;
  TOrderEventMessage y;
  paper_item qi;

  if (sp->sp_pl_mode == 'c')
     {
       memset(&qi, 0x0, sizeof(paper_item));
       qi.paper_copies = 1;
       qi.paper_pl     = of_rec->of_pl;
       qi.paper_order  = of_rec->of_on;
       qi.paper_zone   = 0;
       qi.paper_time   = time(0);
     }
  
  if (order_block)
  {
    if (order_status == 'q' || order_status == 'h')
    {
      y.m_pickline = of_rec->of_pl;
      y.m_order    = of_rec->of_on;
      y.m_zone     = 0;
      memcpy(y.m_grp, of_rec->of_grp, GroupLength);
      memcpy(y.m_con, of_rec->of_con, CustomerNoLength);
      
      of_rec->of_status = 'x';
      of_rec->of_datetime = time(0);
      od_update(order_block);
      oc_dequeue(order_block);
      oc_enqueue(order_block, OC_LAST, OC_COMPLETE);
      message_put(0, OrderCancelEvent, &y, sizeof(TOrderEventMessage));
      if (sp->sp_pl_mode == 'c')
         {
           qi.paper_ref = ++sp->sp_pl_count;
           begin_work();
           packing_list_write(&qi);
           commit_work();
         }
      eh_post(ERR_OF_CANCEL, err_order);
    }
    else if (order_status == 'u' || order_status == 'e')
    {
      x.m_pickline = of_rec->of_pl;
      x.m_order    = of_rec->of_on;

      if (sp->sp_running_status == 'y')
      {
        message_put(0, OrderCancelRequest, &x, sizeof(TOrderMessage));
        if (sp->sp_pl_mode == 'c')
           {
             qi.paper_ref = ++sp->sp_pl_count;
             begin_work();
             packing_list_write(&qi);
             commit_work();
           }
      }
      else eh_post(LOCAL_MSG, "Restoreplace To Cancel UW");
    }
    else                                  /* cancel error                    */
    {
      eh_post(ERR_OF_COMP, err_order);
      return;
    }
    return;
  }
  if (sp->sp_pending_ops != 'y')
  {
    eh_post(ERR_ORDER, err_order);
    return;
  }
  if (!found)                             /* create pending cancel           */
  {
    if (!confirm())
    {
      eh_post(ERR_OF_ACT, 0);
      return;
    }
    ox.pnd_pl    = pickline;
    ox.pnd_flags = PENDING_CANCEL;

    if (sp->sp_use_con == 'y') 
    {
      memcpy(ox.pnd_con, buf[2], CustomerNoLength);
      space_fill(ox.pnd_con, CustomerNoLength);
    }
    else ox.pnd_on = order;

    pending_write(&ox);
    
    eh_post(ERR_OC, err_order);
    return;
  }
  if (ox.pnd_flags == PENDING_CANCEL)     /* must be pending hold            */
  {
    eh_post(ERR_OF_PND, 0);
    return;
  }
  ox.pnd_flags = PENDING_CANCEL;
  pending_replace(&ox);
  eh_post(ERR_OC, err_order);

  return;
}
/*-------------------------------------------------------------------------*
 * pick order function
 *-------------------------------------------------------------------------*/
pick_order()
{
  if (order_block <= 0)
  {
    eh_post(ERR_ORDER, err_order);
    return;
  }
  if (order_status != 'q' && order_status != 'h')
  {
    eh_post(ERR_NOT_QH, err_order);
    return;
  }
  if (!after_order)
  {
    after_status      = 'q';
    after_priority    = 'h';
    after_queue       = OC_HIGH;
  }
  oc_dequeue(order_block);

  of_rec->of_status   = after_status;
  of_rec->of_pri      = after_priority;
  of_rec->of_datetime = time(0);
  od_update(order_block);                 /* update order file               */

  if (sp->sp_tl_mode == 'a') enter_high_order();

  oc_enqueue(order_block, after_order_block, after_queue);
  eh_post(ERR_CONFIRM,"Pick Command");

  return;
}
/*-------------------------------------------------------------------------*
 * repick order function
 *-------------------------------------------------------------------------*/
repick_order()
{
  if (order_block <= 0)
  {
    eh_post(ERR_ORDER, err_order);
    return;
  }
  if (order_status != 'c' && order_status != 'x')
  {
    eh_post(ERR_OF_NCOMP, 0);
    return;
  }
  if (!after_order)
  {
    after_status      = 'q';
    after_priority    = 'h';
    after_queue       = OC_HIGH;
  }
  oc->oi_tab[order_block - 1].oi_flags = 0;

  oc_dequeue(order_block);

  of_rec->of_status   = after_status;
  of_rec->of_pri      = after_priority;
  of_rec->of_picker   = 0;
  of_rec->of_elapsed  = 0;
  of_rec->of_repick   = 'y';
  of_rec->of_datetime = time(0);
  od_update(order_block);                 /* update order file               */

  if (sp->sp_tl_mode == 'a') enter_high_order();

  oc_enqueue(order_block, after_order_block, after_queue);
  
  od_repick(order_block, 1);              /* fix picks                       */
  
  if (sp->sp_box_feature == 's')
  {
    delete_boxes(of_rec->of_pl, of_rec->of_on);
  }
  eh_post(ERR_CONFIRM, "Repick Command");

  return;
}
/*-------------------------------------------------------------------------*
 * change priority function
 *-------------------------------------------------------------------------*/
change_priority()
{
  if (order_block <= 0)
  {
    eh_post(ERR_ORDER, err_order);
    return;
  }
  if (order_status != 'q' && order_status != 'h')
  {
    eh_post(ERR_NOT_QH, err_order);
    return;
  }
  after_priority = *buf[4];
  after_status   = order_status;
  after_queue    = encode_status(after_status, after_priority);

  if (after_queue < 0)                    /* should never happen here        */
  {                                       /* to prevent surprises            */
    eh_post(ERR_CODE, buf[4]);            /* bad priority ?                  */
    return;
  }
  if (order_priority == after_priority)
  {
    eh_post(ERR_SAME_PRI, 0);
    return;
  }
  of_rec->of_pri = after_priority;
  od_update(order_block);

  if (order_status == 'q')
  {
    oc_dequeue(order_block);
    oc_enqueue(order_block, OC_LAST, after_queue);
  }
  eh_post(ERR_CONFIRM, "Change Priority");
   
  return;
}
/*-------------------------------------------------------------------------*
 *  Delete All Boxes Of An Order
 *-------------------------------------------------------------------------*/
delete_boxes(pickline, order)
register long pickline, order;
{
  boxes_item box;
  
  box.b_box_pl     = pickline;
  box.b_box_on     = order;
  box.b_box_number = 0;
  boxes_startkey(&box);
  
  box.b_box_number = DUMMY_BOX;
  boxes_stopkey(&box);
  
  while (!boxes_next(&box, LOCK)) boxes_delete();

  return 0;
}
/*------------------------------------------------------------------------*
 *  Get After Order 
 *------------------------------------------------------------------------*/
get_after_order()
{
  register struct oi_item *o;

#ifdef DEBUG
  fprintf(stderr, "get_after_order() buf[3]=%s\n", buf[3]);
#endif

  if (!*buf[3]) 
  {
    after_order = 0;
    if (sp->sp_global_order_cmds == 'c') after_order_block = OC_LAST;
    else after_order_block = OC_FIRST;
    return 0;
  }
  after_order = check_on(pickline, buf[3]);

#ifdef DEBUG
  fprintf(stderr, "get_after_order():  after_order =%d\n", after_order);
#endif

  after_order_block = oc_find(pickline, after_order);

#ifdef DEBUG
  fprintf(stderr, "get_after_order():  after_order_block =%d\n", 
    after_order_block);
#endif
      
  if (!after_order_block) 
  {
    if (sp->sp_global_order_cmds == 'c')
    {
      after_order = 0;
      after_order_block = OC_FIRST;
      return 0;
    }
    eh_post(ERR_ORDER, buf[3]);
    return 1;
  }
  o = &oc->oi_tab[after_order_block - 1];

  if (o->oi_queue != OC_HIGH && o->oi_queue != OC_MED && o->oi_queue != OC_LOW)
  {
    if (sp->sp_global_order_cmds == 'c')
    {
      after_order = 0;
      after_order_block = OC_FIRST;
      return 0;
    }
    eh_post(ERR_ORDER, buf[3]);
    return 1;
  }
  od_read(after_order_block);             /* get order with no lock          */

  after_status   = of_rec->of_status;
  after_priority = of_rec->of_pri;
  after_queue    = encode_status(after_status, after_priority);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Check Order Action Valid In All Picklines
 *
 *  return 0 - valid order and action.
 *         1 - action invalid or good order.
 *         2 - none found.
 *         3 - some found. (missing records ?).
 *
 *  1. Order must exist in all picklines.
 *  2. Must be in proper queue(s) for the action.
 *  3. Activate may be all pending.
 *  4. Cancel may go to pending.
 *  5. Hold may go to pending.
 *
 *-------------------------------------------------------------------------*/
check_valid_action()
{
  register long k, queue, count, valid, possible;

#ifdef DEBUG
  fprintf(stderr, "check_valid_action():\n");
#endif

  count = possible = valid = 0;
  
  for (k = pfirst; k <= plast; k++)
  {
    if (!(pl[k - 1].pl_pl)) continue;       /* pickline is not used          */
    possible++;                             /* eligible picklines            */
    
    order = check_on(k, buf[2]);            /* get caps order number         */
    order_block = oc_find(k, order);        /* order must exist in all lines */
    if (!order_block) continue;
    count++;                                /* orders found anywhere         */
    
    queue = oc->oi_tab[order_block - 1].oi_queue;
    
    if (*buf[1] == 'a')                    /* activate orders                */
    {
      if (queue != OC_HOLD) continue;      /* must be in hold queue/pending  */
    }
    else if (*buf[1] == 'p' || *buf[1] == 'x')  /* must be queued or hold    */
    {
      if (queue == OC_UW || queue == OC_COMPLETE) continue;
    }
    else if (*buf[1] == 'h')                    /* only queued or pending    */
    {
      if (queue != OC_LOW && queue != OC_MED && queue != OC_HIGH) continue;
    }
    else if (*buf[1] == 'r')                   /* only repick completed      */
    {
      if (queue != OC_COMPLETE) continue;
    }
    valid++;                                   /* count in good queue        */
  }
  if (!possible) return 2;                     /* no picklines valid         */
  if (!count)                                  /* no orders queued anywhere  */
  {
    if (sp->sp_pending_ops == 'y')             /* pending is allowed         */
    {
      if (*buf[1] == 'a' || *buf[1] == 'h' || *buf[1] == 'c') return 0;
    }
    return 2;                                  /* no such order              */
  }
  if (count == valid)    return 0;             /* all were found             */
  if (count == possible) return 1;             /* all found - wrong queue    */
  
  return 3;                                    /* partially found            */
}
/*-------------------------------------------------------------------------*
 *  Check Order and After Order.
 *
 *  1. After Order must be exist in all picklines. 
 *  2. After order must be QUEUED in all picklines.
 *-------------------------------------------------------------------------*/
check_after_order()
{
  register long k, count;
  register struct oi_item *x;

#ifdef DEBUG
  fprintf(stderr, "check_after_order(): order=%s\n", buf[3]);
#endif

  if (!*buf[3]) return 1;                   /* failed - no after order       */

  count = 0;
   
  for (k = pfirst; k <= plast; k++)
  {
    if (!(pl[k - 1].pl_pl)) continue;       /* ignore unused picklines       */

    after_order = check_on(k, buf[3]);

    after_order_block = oc_find(k, after_order); 
    if (!after_order_block) return 1;       /* after order must exist        */
    
    x = &oc->oi_tab[after_order_block - 1];

    if (x->oi_queue < OC_HIGH || x->oi_queue > OC_LOW) return 1;

    count++;                                /* count good orders             */
  }
  if (count) return 0;                      /* found something               */
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Synchronize Queue Position - When No After Is Given
 *
 *  Attempt to find an order to queue after.
 *-------------------------------------------------------------------------*/
sync_queue()
{
  register long k, j, least, count, block;
  
  for (k = pfirst, j = 0; k <= plast; k++)
  {
    if (!pl[k - 1].pl_pl) continue;
    count = oc->oc_tab[k - 1].oc_queue[OC_HIGH].oc_count;
    if (!j || count < least)
    {
      least = count;
      j = k;
    }
  }
  if (!j) return 0;
  
  block = oc->oc_tab[j - 1].oc_queue[OC_UW].oc_last;
  if (!block) return 0;
  
  sprintf(buf[3], "%d", oc->oi_tab[block - 1].oi_on);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Determines where to enqueue orders on the high priority queue if 
 *  tote label ahead mode.
 *
 * F073087 -
 *
 * Tote label ahead mode entails queuing orders to the high priority queue
 * to have their tote labels printed.  These orders cannot be disturbed by 
 * any order that wants to be queued to the high priority queue.
 * Therefore, whenever an order is to be queued to the beginning or between
 * orders on the high queue, enter_high_order is summoned to determine the
 * next available location to put the order.  It will search orders on the 
 * high queue and check sp->oi_tl_flag (1 if label was printed).  The order
 * will then be placed after the last order that was printed.
 *-------------------------------------------------------------------------*/

enter_high_order()
{
  static unsigned long labels = (TOTE_LABEL | SHIP_LABEL | PACK_LIST);
  register block;
  register struct oi_item *x;

  if (after_status   != 'q') return;      /* only queued                     */
  if (after_priority != 'h') return;      /* only high queue                 */

  block = after_order_block;              /* initially same                  */

  if (!block) block = oc->oc_tab[pickline - 1].oc_queue[after_queue].oc_first;

  while (block)                           /* end or empty queue              */
  {
    x = &oc->oi_tab[block - 1];           /* point to index                  */

    if (!(x->oi_flags & labels)) break;   /* end of tote list                */
    after_order_block = block;            /* save last                       */
    block = x->oi_flink;                  /* go to next                      */
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Encode Queue Pointer
 *-------------------------------------------------------------------------*/
encode_status(status, priority)
register char status, priority;
{
  switch (status)
  {
    case 'h': return OC_HOLD;

    case 'c':
    case 'x': return OC_COMPLETE;

    case 'u':
    case 'e': return OC_UW;

    case 'q': switch (priority)
    {
      case 'h': return OC_HIGH;

      case 'm': return OC_MED;

      case 'l': return OC_LOW;

      default:  return -1;
    }
    default:  return -1;
  }
}
 
 
/* end of order_comms.c */
