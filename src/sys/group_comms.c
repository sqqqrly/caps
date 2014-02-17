#define DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Group commands screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/10/93   | tjt Added to mfc.  Revised extensively.
 *  05/27/94   | tjt Add pending operations flag.
 *  06/02/94   | tjt Remove transaction output.
 *  10/21/94   | tjt Add change priority in hold queue.
 *  11/03/94   | tjt Add coordinated picklines.
 *  01/23/95   | tjt Add new IS_ONE_PICKLINE.
 *  06/03/95   | tjt Add pickline input by name.
 *  06/21/95   | tjt Add clear work queue in find_order.
 *  04/18/97   | tjt Add language.h and code_to_caps.
 *  02/26/03   | aha Added fix for after groups if not on all picklines.
 *  08/06/03   | aha Added fix if an order is uw in after group.
 *-------------------------------------------------------------------------*/
static char group_comms_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include "message_types.h"
#include "caps_messages.h"
#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "group_comms.t"
#include "eh_nos.h"
#include "co.h"
#include "of.h"
#include "xt.h"
#include "language.h"
#include "Bard.h"

extern leave();
extern catcher();
extern hold_group();
extern activate_group();
extern cancel_group();
extern pick_group();
extern change_priority();

struct pending_item ox;

#define NUM_PROMPTS     9
#define BUF_SIZE        9

short ONE    = 1;
short PL_LEN = 9;
short GP_LEN = 4;
short ON_LEN = 5;

char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */

struct fld_parms fld[] = {
  {13,57,28,1,&PL_LEN, "Enter Pickline",'a'},
  {14,57,28,1,&ONE,    "Enter Code",'a'},
  {15,57,28,1,&GP_LEN, "Enter Group Number",'a'},
  {16,57,28,1,&ONE,    "Enter Priority",'a'},
  {16,57,28,1,&ONE,    "Are you sure? (y/n)       ",'a'},
  {16,57,28,1,&ONE,    "After Group or Order (g/o)",'a'},
  {17,57,28,1,&GP_LEN, "Enter After Group",'a'},
  {17,57,28,1,&ON_LEN, "Enter After Order",'n'},
  {18,57,28,1,&ONE,    "Create Pending (y/n)", 'a'}
};
/*
 * Global Variables
 */
long pickline;
long has_pickline = 0;

long found;

long after_order;
long after_save_order;
long after_block;
char after_priority;
long after_queue;

long order_block;
long order_count;

long least_queue_count;

char err_group[8];

short after_tote_block, save_block;
short savend1 = 0,savend2 = 0;
unsigned char t;

#ifdef DEBUG
FILE *DF;
#endif

main()
{
  register long i, j, rm;
  register unsigned char t;
  
  putenv("_=group_comms");
  chdir(getenv("HOME"));

  open_all();

  GP_LEN = rf->rf_grp;
  ON_LEN = rf->rf_on;

  sd_screen_off();
  sd_clear_screen();
  fix(group_comms);
  sd_text(group_comms);
  sd_screen_on();
  
  rm = 1;

  if (IS_ONE_PICKLINE) pickline = op_pl;
  else if (sp->sp_global_group_cmds != 'c')
  {
    pickline = op_pl;
    if (SUPER_OP) rm = 0;
  }
  while(1)
  {
    memset(buf, 0, BUF_SIZE * NUM_PROMPTS);

    sd_cursor(0, 14, 1); sd_clear_line();
    sd_cursor(0, 15, 1); sd_clear_line();
    sd_cursor(0, 16, 1); sd_clear_line();
    sd_cursor(0, 17, 1); sd_clear_line();
    sd_cursor(0, 18, 1); sd_clear_line();
    
    for (j = rm; j < 3; j++) sd_prompt(&fld[j], 0);

    i = rm;

    while(1)
    {
      t = sd_input(&fld[i], 0, 0,buf[i], 0);

      if(t == EXIT) leave();
      if (t == UP_CURSOR) 
      {
        if (i > rm) i--;
        continue;
      }
      if (t == RETURN)
      {
        if (*buf[2]) break;
        eh_post(GRP_NOT_FND, "");
        continue;
      }
      if (i < 2) i++;
    }
    space_fill(buf[2], GroupLength);       /* pad group with spaces          */

/*-------------------------------------------------------------------------*
 *  Check Valid Pickline - Default Pickline In op_pl Of Operator
 *-------------------------------------------------------------------------*/
    if (!rm)
    {
      pickline = pl_lookup(buf[0], op_pl);

      if (!pickline)
      {
        if (sp->sp_global_group_cmds == 'n')
        {
          eh_post(ERR_PL, "ALL");
          continue;
        }
      }
      else if (pickline < 0)
      {
        eh_post(ERR_PL,buf[0]);
        continue;
      }
      else 
      {
        sprintf(buf[0], "%d", pickline);
        chng_pkln(buf[0]);  
      }
    }
    
/*-------------------------------------------------------------------------*
 * Check Action Code
 *-------------------------------------------------------------------------*/
    *buf[1] = tolower(*buf[1]);

    if (*buf[1] != 'a' && *buf[1] != 'c' && *buf[1] != 'h' &&
        *buf[1] != 'p' && *buf[1] != 'x')
    {
      eh_post(ERR_CODE, buf[1]);
      continue;
    }
    if (sp->sp_global_group_cmds == 'y' && !pickline)
    {
      if (*buf[1] == 'p')     /* changed by shiv on Paul request  11/18/97 */
      {       
        eh_post(ERR_CODE, buf[1]);
        continue;
      }
    }
    else if (sp->sp_global_group_cmds == 'c' && !pickline)
    {
      if (*buf[1] == 'x' || *buf[1] == 'p')
      {       
        eh_post(ERR_CODE, buf[1]);
        continue;
      }
    }
/*-------------------------------------------------------------------------*
 *  Are You Sure? - cancel and pickline == 0
 *-------------------------------------------------------------------------*/  
    if (*buf[1] == 'c' || !pickline)
    {
      sd_prompt(&fld[4], 0);              /* are you sure message            */
      
      while(1)
      {
        t = sd_input(&fld[4], 0, 0, buf[4], 0);
        if(t == EXIT) leave();
        if (t == UP_CURSOR) break;
        if(code_to_caps(*buf[4]) == 'y') break;
        if(code_to_caps(*buf[4]) == 'n') break;
        eh_post(ERR_YN,0);
      }
      if (code_to_caps(*buf[4]) == 'n') continue; 
      if (t == UP_CURSOR) continue;
    }
/*-------------------------------------------------------------------------*
 *  Priority Change
 *-------------------------------------------------------------------------*/
    if (*buf[1] == 'x')
    {
      while(1)
      {
        t = sd_input(&fld[3], sd_prompt(&fld[3],0),0,buf[3],0);
        if (t == EXIT) leave();
        if (t == UP_CURSOR) break;
        if(*buf[3] == 'h' || *buf[3] == 'm' || *buf[3] == 'l') break;
        eh_post(ERR_OF_PRI,0);
      }
      if (t == UP_CURSOR) continue;
    }
/*-------------------------------------------------------------------------*
 * prompt for after group and find the last order in that group
 *-------------------------------------------------------------------------*/
    if (*buf[1] == 'a' || *buf[1] == 'p')
    {
      while(1)
      {
        memset(buf[5], 0, BUF_SIZE);
        memset(buf[6], 0, BUF_SIZE);
        memset(buf[7], 0, BUF_SIZE);
        
        while(1)
        {
          t = sd_input(&fld[5],sd_prompt(&fld[5],0),0,buf[5],0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;
    
          if (!*buf[5]) break;
    
          *buf[5] = tolower(*buf[5]);
  
          if (*buf[5] == 'g' || *buf[5] == 'o') break;
      
          eh_post(ERR_CODE, buf[5]);
        }
        if (!*buf[5]) break;
        if (t == UP_CURSOR) break;
        
        if (*buf[5] == 'g')                /* after group requested          */
        {
          t = sd_input(&fld[6],sd_prompt(&fld[6],0),0,buf[6],0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) continue;
      
          if(*buf[6] == 0) break;
          space_fill(buf[6], GroupLength);
        
          if (memcmp(buf[2], buf[6], GroupLength) == 0)
          {
            eh_post(ERR_AFTER_GRP, buf[2]);   /* not same group (after self) */
            continue;
          }
          break;
        }
        else
        {
          t = sd_input(&fld[7],sd_prompt(&fld[7],0),0,buf[7],0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) continue;
          if(*buf[7] == 0) break;
      
          after_order = atol(buf[7]);
          if (after_order <= 0)
          {
            eh_post(ERR_ORDER, buf[7]);
            continue;
          }
          break;
        }
      }                                  /* end of while(1)                  */
      if (t == UP_CURSOR) continue;
    }                                    /* end of get after group.order     */
/*-------------------------------------------------------------------------*
 *  Single Pickline Action
 *-------------------------------------------------------------------------*/
    has_pickline = pickline;

    if (pickline)                  
    {
      strcpy(err_group, buf[2]);
      
      oc_lock();
      
      if (check_after_order(0))
      {
        oc_unlock();
        continue;
      }
      find_pending();
      do_action();
      oc_unlock();
      continue;
    }
    oc_lock();

    if (*buf[1] == 'h' && sp->sp_global_group_cmds == 'c') sync_hold_start();

    if (*buf[1] == 'a' && sp->sp_global_group_cmds == 'c') sync_after_order();

    for (pickline = 1; pickline <= sp->sp_picklines; pickline++)
    {
      if (pl[pickline - 1].pl_pl != pickline) continue;

      sprintf(err_group, "%d-%s", pickline, buf[2]);
      
      if (check_after_order(1)) break;
      find_pending();
      do_action();
    }
    oc_unlock();
  }
}
/*-------------------------------------------------------------------------*
 *  Confirm Pending
 *-------------------------------------------------------------------------*/
confirm()
{
  char yn[2];
  
  sd_prompt(&fld[8], 0);  
  
  if (*buf[8] == 'y') return 1;
  if (*buf[8] == 'n') return 0;

  memset(yn, 0, 2);
  
  while(1)
  {
    t = sd_input(&fld[8], 0, 0, yn, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return 0;
    *buf[8] = code_to_caps(*yn);
    if (*buf[8] == 'y') return 1;
    if (*buf[8] == 'n') return 0;

    eh_post(ERR_YN,0);
  }
}

/*-------------------------------------------------------------------------*
 *  Find Any Pending Action
 *-------------------------------------------------------------------------*/
find_pending()
{
  found = 0;

  if (sp->sp_pending_ops != 'y') return found;

  ox.pnd_pl = pickline;
  memcpy(ox.pnd_group, buf[2], GroupLength);

  begin_work();
  if (!pending_read(&ox, LOCK)) found = 1;
  else commit_work();
  
  return found;
}
/*-------------------------------------------------------------------------*
 * Process the Requested Action
 *-------------------------------------------------------------------------*/
do_action()
{
  switch (*buf[1])
  {
    case 'a': activate_group_control();
              break;

    case 'h': hold_group_control();
              break;
              
    case 'c': cancel_group_control();
              break;
              
    case 'p': pick_group_control();
              break;
              
    case 'x': change_priority_control();
              break;
  }
}
open_all()
{
  static unsigned char list[] = {ClientMessageEvent, 0};

  database_open();
  sd_open(catcher);
  message_select(list, 1);
  ss_open();
  co_open();
  oc_open();
  od_open();

  getparms(0);
  pending_open(AUTOLOCK);
  pending_setkey(2);

  DF = fopen("debug/group_comms.log", "w");
  return 0;
}
close_all()
{
  if (pending_fd) pending_close();
  oc_close_save();
  od_close();
  co_close();
  ss_close();
  sd_close();
  database_close();

  if (DF) fclose(DF);
  return;
}
/*-------------------------------------------------------------------------*
 *  Message Catcher
 *-------------------------------------------------------------------------*/
catcher(who, type, buf, len)
register long who, type, len;
register char *buf;
{
  switch (type)
  {
    case ShutdownRequest: leave();
    
    case ClientMessageEvent:
    
      eh_post(buf, buf + 1);
      break;
      
    default: break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 * function to transfer back to calling program
 *-------------------------------------------------------------------------*/

leave()
{
  close_all();
  execlp("operm","operm",0);
  krash("leave", "operm load", 1);
}
/*-------------------------------------------------------------------------*
 *  hold_group function
 *-------------------------------------------------------------------------*/
hold_group_control()
{
  if (found)                             /* it's pending hold/cancel        */
  {
    commit_work();                       /* unlock pending                  */
    eh_post(ERR_GRP_AP, err_group);
    return;
  }
  find_order(hold_group);
  
  if (!order_count && sp->sp_pending_ops != 'y')
  {
    eh_post(GRP_NOT_FND, err_group);
    return;
  }
  if (!order_count)                     /* pending hold request             */
  {
    if (!confirm())
    {
      eh_post(ERR_GRP_ABO, err_group);
      return;
    }
    ox.pnd_pl    = pickline;
    ox.pnd_on    = 0;
    ox.pnd_flags = PENDING_HOLD;

    memcpy(ox.pnd_group, buf[2], GroupLength);

    pending_write(&ox);

    eh_post(ERR_GH, err_group);
    return;
  }
  eh_post(ERR_GRP_HOLD, err_group);
  return;  
}
hold_group()
{
  of_rec->of_status = 'h';
  of_rec->of_datetime = time(0);
  od_update(order_block);
  oc_enqueue(order_block, after_block, OC_HOLD);
  
  after_block = order_block;
}
/*-------------------------------------------------------------------------*
 *   activate group function
 *-------------------------------------------------------------------------*/
activate_group_control()
{
  if (found)                              /* group is pending hold/cancel    */
  {
    pending_delete();
    commit_work();
    eh_post(ERR_GD, err_group);
    return;
  }
  find_order(activate_group);
  
  if(!order_count)  eh_post(GRP_NOT_FND,err_group);
  else eh_post(ERR_GRP_GPA, err_group);
  return;
}
activate_group()
{
  of_rec->of_status   = 'q';
  of_rec->of_pri      = after_priority;
  of_rec->of_datetime = time(0);
  od_update(order_block);
  
  oc_enqueue(order_block, after_block, after_queue);
  
  after_block = order_block;
}

/*-------------------------------------------------------------------------*
 * cancel group function
 *-------------------------------------------------------------------------*/
cancel_group_control()
{
  TOrderMessage x;
  register long next, block;

  if (found)                              /* pending cancel request          */
  {
    if (PENDING_HOLD & ox.pnd_flags)
    {
      ox.pnd_flags = PENDING_CANCEL;
      pending_update(&ox);
      commit_work();
      return;
    }
    commit_work();
    eh_post(ERR_GRP_AP, err_group);
    return;
  }
  sd_wait();

  find_order(cancel_group);
  
  sd_msg(" ");

  block = oc->oc_tab[pickline - 1].oc_queue[OC_UW].oc_first;
    
  while (block)
  {
    next = oc->oi_tab[block - 1].oi_flink;

    if (memcmp(buf[2], oc->oi_tab[block - 1].oi_grp, GroupLength) == 0)
    {
      order_count++;

      x.m_pickline = oc->oi_tab[block - 1].oi_pl;
      x.m_order    = oc->oi_tab[block - 1].oi_on;

      if (sp->sp_running_status == 'y')
      {
        message_put(0, OrderCancelRequest, &x, sizeof(TOrderMessage));
      }
      else 
      {
        eh_post(LOCAL_MSG, "Restoreplace To Cancel UW");
        return;
      }
    }
    block = next;
  }
  if (!order_count && sp->sp_pending_ops != 'y')
  {
    eh_post(GRP_NOT_FND, err_group);
    return;
  }
  if (!order_count)                      /* pending cancel request         */
  {
    if (!confirm())
    {
      eh_post(ERR_GRP_ABO, err_group);
      return;
    }
    ox.pnd_pl    = pickline;
    ox.pnd_on    = 0;
    ox.pnd_flags = PENDING_CANCEL;

    memcpy(ox.pnd_group, buf[2], GroupLength);

    pending_write(&ox);
  
    eh_post(ERR_OC, err_group);
    return;
  }
  eh_post(ERR_GRP_CAN, err_group);
  return;
}
cancel_group()
{
  TOrderEventMessage x;

  x.m_pickline = of_rec->of_pl;
  x.m_order    = of_rec->of_on;
  x.m_zone     = 0;
  memcpy(x.m_grp, of_rec->of_grp, GroupLength);
  memcpy(x.m_con, of_rec->of_con, CustomerNoLength);
  
  of_rec->of_status = 'x';
  of_rec->of_datetime = time(0);
  od_update(order_block);

  oc_enqueue(order_block, OC_LAST, OC_COMPLETE);

  message_put(0, OrderCancelEvent, &x, sizeof(TOrderEventMessage));

  return;
}
/*-------------------------------------------------------------------------*
 * pick group function
 *-------------------------------------------------------------------------*/
pick_group_control()
{
  if (found)                               /* is pending - can't pick        */
  {
    commit_work();
    eh_post(ERR_GRP_AP, err_group);
    return;
  }
  find_order(pick_group);
  
  if (!order_count) eh_post(GRP_NOT_FND, err_group);
  else eh_post(ERR_GRP_GPA, err_group);
  return;
}
pick_group()
{
  of_rec->of_pri      = after_priority;
  of_rec->of_status   = 'q';
  of_rec->of_datetime = time(0);
  od_update(order_block);

  oc_enqueue(order_block, after_block, after_queue);
        
  after_block = order_block;
}
/*-------------------------------------------------------------------------*
 * change priority function
 *-------------------------------------------------------------------------*/
change_priority_control()
{
  if (found)                               /* is pending                     */
  {
    commit_work();
    eh_post(ERR_GRP_CNA, err_group);
    return;
  }
  after_priority = *buf[3];
  
  if (after_priority == 'h')      after_queue = OC_HIGH;
  else if (after_priority == 'm') after_queue = OC_MED;
  else                            after_queue = OC_LOW;

  find_order(change_priority);
  
  change_hold_priority();                  /* change in hold queue w/o move  */

  if(!order_count) eh_post(GRP_NOT_FND,err_group);
  else eh_post(ERR_GRP_PRI, err_group);
  return;
}
change_priority()
{
  of_rec->of_pri      = after_priority;
  of_rec->of_status   = 'q';
  of_rec->of_datetime = time(0);
  od_update(order_block);

  oc_enqueue(order_block, after_block, after_queue);
        
  after_block = order_block;
  return;
}
/*-------------------------------------------------------------------------*
 *  Change Hold Queue Priority
 *-------------------------------------------------------------------------*/
change_hold_priority()
{
  register long block, next;

  block = oc->oc_tab[pickline - 1].oc_queue[OC_HOLD].oc_first;
    
  while (block)
  {
    next = oc->oi_tab[block - 1].oi_flink;

    if (memcmp(buf[2], oc->oi_tab[block - 1].oi_grp, GroupLength) == 0)
    {
      begin_work();
      od_get(block);
      if (of_rec->of_pri != after_priority) 
      {
        of_rec->of_pri      = after_priority;
        of_rec->of_datetime = time(0);
        od_update(block);
        order_count++;
      }
      commit_work();
    }
    block = next;
  }
  return 0;
}
 
/*-------------------------------------------------------------------------*
 *  Search Queues For Groups
 *-------------------------------------------------------------------------*/
find_order(func)
long (*func)();
{
  register long k, count;
  register long next, block;
  
  order_count = 0;

  oc->oc_tab[pickline - 1].oc_queue[OC_WORK].oc_first = 0;
  oc->oc_tab[pickline - 1].oc_queue[OC_WORK].oc_last  = 0;
  oc->oc_tab[pickline - 1].oc_queue[OC_WORK].oc_count = 0;
  
  if (sp->sp_global_group_cmds == 'c' && *buf[1] == 'h')
  {
    count = oc->oc_tab[pickline - 1].oc_queue[OC_HIGH].oc_count;
    count -= least_queue_count;
  }
  else count = 0;

  if (*buf[1] != 'a')
  {
    for (k = OC_HIGH; k <= OC_LOW; k++)
    {
      block = oc->oc_tab[pickline - 1].oc_queue[k].oc_first;
    
      while (block)
      {
        next = oc->oi_tab[block - 1].oi_flink;

        if (count > 0) {count--; block = next; continue;}

        if (memcmp(buf[2], oc->oi_tab[block - 1].oi_grp, GroupLength) == 0)
        {
          order_count++;
          oc_dequeue(block);
          oc_enqueue(block, OC_LAST, OC_WORK);
        }
        block = next;
      }
    }
  }
  if (*buf[1] != 'h' && *buf[1] != 'x')
  {
    block = oc->oc_tab[pickline - 1].oc_queue[OC_HOLD].oc_first;
    
    while (block)
    {
      next = oc->oi_tab[block - 1].oi_flink;

      if (memcmp(buf[2], oc->oi_tab[block - 1].oi_grp, GroupLength) == 0)
      {
        order_count++;
        oc_dequeue(block);
        oc_enqueue(block, OC_LAST, OC_WORK);
      }
      block = next;
    }
  }
  if (!order_count) return;
          
  while (order_block = oc->oc_tab[pickline - 1].oc_queue[OC_WORK].oc_first)
  {
    oc_dequeue(order_block);
    begin_work();
    od_get(order_block);
    (*func)();
    commit_work();
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Check After Group/Order
 *-------------------------------------------------------------------------*/
int check_after_order(flag)
register short flag;
{
  register struct oc_item *q;
  register long j        = 0L;

  after_block = 0L;

  q = &oc->oc_tab[pickline - 1];

  if (*buf[5] == 'o')
  {
    if (flag == 1)   /* For all picklines */
       {
         for (j = 1L; j <= coh->co_pl_cnt; j++)
             {
               after_block = oc_find(j, after_order);

               if (after_block > 0L)
                  {
                    if (j == pickline)
                       {
                         break;
                       }
                    else
                       {
                         if (q->oc_queue[OC_HIGH].oc_count > 0L)
                            {
                              after_block = q->oc_queue[OC_HIGH].oc_last;
                            }
                         else if (q->oc_queue[OC_MED].oc_count > 0L)
                            {
                              after_block = q->oc_queue[OC_MED].oc_last;
                            }
                         else if (q->oc_queue[OC_LOW].oc_count > 0L)
                            {
                              after_block = q->oc_queue[OC_LOW].oc_last;
                            }
                         else
                            {
                              after_block = OC_LAST;
                            }

                         break;
                       }
                  }

             }  /* end of for loop to search all picklines */
       }
    else if (flag == 0)  /* For specified pickline on input */
       {
         after_block = oc_find(pickline, after_order);
       }

    if (!after_block)
    {
      eh_post(ERR_ORDER, buf[7]);
      return 1;
    }
  }
  else if(*buf[5] == 'g')
  {
    if (flag == 1)   /* For all picklines */
       {
         find_last_order(buf[6], 1);
       }
    else if (flag == 0)  /* For specified pickline on input */
       {
         find_last_order(buf[6], 0);
       }

    if (!after_block)
    {
      eh_post(GRP_NOT_FND, buf[6]);
      return 1;
    }
  }
  else 
  {
    if (sp->sp_global_group_cmds == 'c') after_block = OC_LAST;
    else after_block = OC_FIRST;

    after_order    = 0;
    after_queue    = OC_HIGH;
    after_priority = 'h';
    return 0;
  }
  if (sp->sp_tl_mode == 'a') enter_high_order();
  
  if (memcmp(buf[2], oc->oi_tab[after_block - 1].oi_grp, GroupLength) == 0)
  {
    eh_post(ERR_AFTER_GRP, buf[2]);
    return 1;
  }

  if (after_block == OC_LAST)
     {
       after_queue    = OC_HIGH;
       after_priority = 'h';
       return 0;
     }
  else
     {
       after_queue = oc->oi_tab[after_block - 1].oi_queue;
  
       if (after_queue == OC_HIGH)
          {
            after_priority = 'h';
            return 0;
          }

       if (after_queue == OC_MED)
          {
            after_priority = 'm';
            return 0;
          }

       if (after_queue == OC_LOW)
          {
            after_priority = 'l';
            return 0;
          }
     }

  eh_post(ERR_GRP_NQ, buf[6]);
  return 1;
}


/*-------------------------------------------------------------------------*
 *  Find Last Queued Order Of A Group
 *-------------------------------------------------------------------------*/
find_last_order(p, pl_flag)
register char *p;
register short pl_flag;
{
  register short foundit = 0;
  register long  a_blk   = 0L,
                 k       = 0L,
                 j       = 0L;

  after_block = 0L;

#ifdef DEBUG
  fprintf(DF, "find_last_order(): after group = [%s], pickline = %d\n",
          p, pickline);
  fprintf(DF, "                   pl_flag = %d\n", pl_flag);
  fflush(DF);
#endif

  if (pl_flag == 1)
     {
       for (j = 1L; j <= coh->co_pl_cnt; j++)
           {
#ifdef DEBUG
             fprintf(DF, "flo(): a_group = [%s], pickline = %d\n",
                     p, j);
             fflush(DF);
#endif
             foundit = 0;
             for (k = OC_LOW; k >= OC_HIGH; k--)
                 {
                   a_blk = oc->oc_tab[j - 1].oc_queue[k].oc_last;

                   while (a_blk)
                         {
                           if (memcmp(p,
                                      oc->oi_tab[a_blk - 1].oi_grp,
                                      GroupLength)                == 0)
                              {
                                if (j == pickline)
                                   {
                                     after_block = a_blk;
#ifdef DEBUG
                                     fprintf(DF, "flo: Done: a_group = [%s]\n",
                                             p);
                                     fprintf(DF, "     after_block = %d\n",
                                             after_block);
                                     fprintf(DF, "     pickline = %d\n",
                                             pickline);
                                     fflush(DF);
#endif
                                     return;
                                   }
                                else
                                   {
                                     foundit = 1;
                                     break;
                                   }
                              }

                           a_blk = oc->oi_tab[a_blk - 1].oi_blink;
                         }  /* end of while loop for each order active queue */

                   if (foundit == 1)
                      {
                        break;
                      }
                 }  /* end of for loop to search all active queues */

             if (foundit == 1)
                {
                  continue;
                }
           }  /* end of for loop to search all picklines */
#ifdef DEBUG
       fprintf(DF, "flo(): group [%s] not found.\n", p);
       fflush(DF);
#endif

       if (foundit == 0)
          {
            return;
          }
       else
          {
            if (oc->oc_tab[pickline - 1].oc_queue[k].oc_count > 0L)
               {
                 after_block = oc->oc_tab[pickline - 1].oc_queue[k].oc_last;
               }
            else
               {
                 after_block = OC_LAST;
               }

            return;
          }
     }

  for (k = OC_LOW; k >= OC_HIGH; k--)
  {
    after_block = oc->oc_tab[pickline - 1].oc_queue[k].oc_last;
    
    while (after_block)
    {
      if (memcmp(p, oc->oi_tab[after_block-1].oi_grp, GroupLength) == 0)
      {
        return;
      }
      after_block = oc->oi_tab[after_block - 1].oi_blink;
    }
  }
  return;
}  /* end of find_last_order() */


/*-------------------------------------------------------------------------*
 *  Synchronize After Order Across Picklines
 *-------------------------------------------------------------------------*/
sync_after_order()
{
  register long k, block;
  register struct oi_item *x;

  if (check_after_order(0)) return 1;     /* failed                           */

  if (!after_order) return 0;

  for (k = 2; k <= sp->sp_picklines; k++)
  {
    if (!(pl[k - 1].pl_pl)) continue;
  
    block = find_order(k, after_order);
    if (!block)
    {
      eh_post(ERR_ORDER, buf[7]);
      return 1;
    }
    x = &oc->oi_tab[block - 1];
    if (x->oi_queue != OC_HIGH) 
    {
      eh_post(ERR_ORDER, buf[7]);
      return 1;
    }
  }
  *buf[5] = 'o';
  sprintf(buf[7], "%d", after_order);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Synchronize Hold Start
 *-------------------------------------------------------------------------*/
sync_hold_start()
{
  register long k, count;

  least_queue_count = OrderMax;

  for (k = 1; k <= sp->sp_picklines; k++)
  {
    if (!(pl[k - 1].pl_pl)) continue;

    count = oc->oc_tab[k - 1].oc_queue[OC_HIGH].oc_count;
    
    if (count < least_queue_count) least_queue_count = count;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Synchronize After Order
 *-------------------------------------------------------------------------*/
/*
 *  Determines where to enqueue orders on the high priority queue if 
 *  tote label ahead mode.
 */
/*   rewritten tjt - 10/10/93
 *
 * Tote label ahead mode entails queuing orders to the high priority queue
 * to have their tote labels printed.  These orders cannot be disturbed by 
 * any order that wants to be queued to the high priority queue.
 * Therefore, whenever an order is to be queued to the beginning or between
 * orders on the high queue, enter_high_order is summoned to determine the
 * next available location to put the order.  It will search orders on the 
 * high queue and check sp->oi_tl_flag (1 if label was printed).  The order
 * will then be placed after the last order that was printed.
 */

enter_high_order()
{
  static unsigned long labels = (TOTE_LABEL | SHIP_LABEL | PACK_LIST);
  register long block;
  register struct oi_item *x;
  
  block = after_block;
  
  if (!block) return;                     /* should not happen here          */
    
  if (oc->oi_tab[block - 1].oi_queue != OC_HIGH) return;

  while (block)
  {
    x = &oc->oi_tab[block - 1];

    if (!(x->oi_flags & labels)) return;
    after_block = block;
    block = x->oi_flink;
  }
  return;
}

/* end of group_comms.c */
