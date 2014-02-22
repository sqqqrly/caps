/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order selection commands screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/22/95   | tjt  Add  Orginal implementation.
 *  06/03/95   | tjt  Add pickline input by name.
 *  04/18/97   | tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char select_comms_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "message_types.h"
#include "caps_messages.h"
#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "select_comms.t"
#include "eh_nos.h"
#include "co.h"
#include "of.h"
#include "xt.h"
#include "Bard.h"
#include "language.h"

extern leave();
extern catcher();
extern activate_group();
extern cancel_group();
extern change_group();
extern find_group();
extern hold_group();
extern pick_group();
extern pickline_group();
extern change_priority();

FILE *fp;
char fp_name[16];
long savefp = 0;

#define NUM_PROMPTS     14
#define BUF_SIZE        20
#define WIDTH           75

short ONE     = 1;
short GP_LEN  = 4;
short PL_LEN  = 8;
short ON_LEN  = 5;
short LFLD    = 16;

char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */

#define code_buf    buf[0]
#define pl_buf      buf[1]
#define on_buf      buf[2]
#define grp_buf     buf[3]
#define con_buf     buf[4]
#define pri_buf     buf[5]
#define q_buf       buf[6]

#define new_pl_buf  buf[7]
#define new_grp_buf buf[8]
#define new_pri_buf buf[9]
#define aft_grp_buf buf[10]
#define aft_on_buf  buf[11]

#define count_buf   buf[12]
#define sure_buf    buf[13]

struct fld_parms fld[] = {
  {11,46,20,1,&ONE,    "Enter Code",'a'},
  {12,46,20,1,&LFLD,   "Enter Pickline",'a'},
  {13,46,20,1,&LFLD,   "Enter Order", 'a'},
  {14,46,20,1,&LFLD,   "Enter Group",'a'},
  {15,46,20,1,&LFLD,   "Enter Reference Number", 'a'},
  {16,46,20,1,&LFLD,   "Enter Priority                             (hml)",'a'},
  {17,46,20,1,&LFLD, "Enter Status                               (hqucx)",'a'},
  {18,46,20,1,&LFLD,   "Enter New Pickline", 'a'},
  {19,46,20,1,&GP_LEN, "Enter New Group", 'a'},
  {19,46,20,1,&ONE,    "Enter New Priority", 'a'},
  {19,46,20,1,&GP_LEN, "Enter After Group",'a'},
  {20,46,20,1,&ON_LEN, "Enter After Order",'n'},
  {21,46,20,1,&ON_LEN, "Enter Number of Orders", 'n'},
  {22,46,20,1,&ONE,    "Are You Sure? (y/n)", 'a'},

};
/*
 * Global Variables
 */
long pickline;
long new_pickline;
long has_pickline = 0;
char pl_wrk[4];

long after_order;
long after_save_order;
long after_block;
char after_priority;
long after_queue;

long order_block;
long found_count;
long count;
long max_count;
long orphans;

char order_plus[2] = {0};
char group_plus[2] = {0};
char con_plus[2]   = {0};
long plus_found = 0;
long minus_found = 0;

long least_queue_count;

short after_tote_block, save_block;
short savend1 = 0,savend2 = 0;
unsigned char t;

#ifndef PATTERN
#define grpcmp(x, y) (memcmp(x, y, GroupLength) == 0)
#endif

main()
{
  register long i, j, ret;
  register unsigned char t;
  char *p, work[32];
  long pid, stats;
  
  putenv("_=select_comms");
  chdir(getenv("HOME"));

  open_all();

  PL_LEN = rf->rf_pl;
  GP_LEN = rf->rf_grp;
  ON_LEN = rf->rf_on;

  sd_screen_off();
  sd_clear_screen();
  fix(select_comms);
  sd_text(select_comms);
  sd_screen_on();

  sd_tab(7, "7 CLEAR");

  while(1)
  {
    for (j = 11; j <= 23; j++)
    {
      sd_cursor(0, j, 1); sd_clear_line();
    }
    if (t == F_KEY_7) memset(buf, 0, BUF_SIZE * NUM_PROMPTS);

    for (j = 0; j < 7; j++) 
    {
      sd_prompt(&fld[j], 0);
      sd_cursor(0, fld[j].irow, fld[j].icol);
      sd_text(buf[j]);
    }
    i = 0;
    memset(order_plus, 0, 2);
    memset(group_plus, 0, 2);
    memset(con_plus, 0, 2);
    
    plus_found = 0;  minus_found = 1;
    
    while(1)                               /* gather new input               */
    {
      while (1)                            /* until RETURN is hit            */
      {
        t = sd_input(&fld[i], 0, 0,buf[i], 0);

        if (t == EXIT)           leave();
        else if (t == RETURN)    break;
        else if (t == F_KEY_7)   break;
        else if (t == UP_CURSOR) i = 6 - ((7 - i) % 7);
        else i = ((i + 1) % 7); 
      }
      if (t == F_KEY_7) break;
      
      *code_buf = tolower(*code_buf);      /* action to lowercase            */

/*-------------------------------------------------------------------------*
 *  Check For Plus Or Minus Sign
 *-------------------------------------------------------------------------*/
     j = strlen(on_buf) - 1;
     if (j >= 0)
     {
       if (on_buf[j] == '+' || on_buf[j] == '-')
       {
         *order_plus = on_buf[j];
         on_buf[j] = 0;
       }
     }
     j = strlen(grp_buf) - 1;
     if (j >= 0)
     {
       if (grp_buf[j] == '+' || grp_buf[j] == '-')
       {
         *group_plus = grp_buf[j];
         grp_buf[j] = 0;
       }
     }
     j = strlen(con_buf) - 1;
     if (j >= 0)
     {
       if (con_buf[j] == '+' || con_buf[j] == '-')
       {
         *con_plus = con_buf[j];
         con_buf[j] = 0;
       }
     }

/*-------------------------------------------------------------------------*
 *  Check Valid Pickline
 *-------------------------------------------------------------------------*/

      if (!*pl_buf) 
      {
        if ((SUPER_OP && sp->sp_global_group_cmds != 'n') || *code_buf == 'f')
        {
          strcpy(pl_buf, "*");
        }
        else sprintf(pl_buf, "%d", op_pl);
        
        sd_cursor(0, fld[1].irow, fld[1].icol);
        sd_text(pl_buf);
      }
      if (strcmp(pl_buf, "*") == 0 && *code_buf != 'f')
      {
        if (sp->sp_global_group_cmds == 'n')
        {
          eh_post(ERR_PL, pl_buf);
          
          i = 0;
          continue;
        }
      }
/*-------------------------------------------------------------------------*
 *  Check Action Code And Queue Value
 *-------------------------------------------------------------------------*/
      p = q_buf;
      while (*p) {*p = tolower(*p); p++;}

      if (*code_buf == 'a')               /* activate                        */
      {
        if (checkpat(q_buf, "h"))
        {
          strcpy(q_buf, "h");
          sd_cursor(0, fld[6].irow, fld[6].icol);
          sd_text(buf[6]);
        }
      }
      else if (*code_buf == 'c')          /* cancel                          */
      {
        if (checkpat(q_buf, "hqu"))
        {
          strcpy(q_buf, "[hqu]");
          sd_cursor(0, fld[6].irow, fld[6].icol);
          sd_text(buf[6]);
        } 
        if (!checkpat(q_buf, "u") && sp->sp_running_status != 'y')
        {
          eh_post(LOCAL_MSG, "Restoreplace To Cancel UW");
          return 1;
        }
      }
      else if (*code_buf == 'f')          /* find orders                     */
      {
        if (checkpat(q_buf, "hqucx") && strcmp(q_buf, "*"))
        {
          strcpy(q_buf, "*");
          sd_cursor(0, fld[6].irow, fld[6].icol);
          sd_text(buf[6]);
        }
      }
      else if (*code_buf == 'g')          /* group change                    */
      {
        if (GP_LEN <= 0)
        {
          eh_post(ERR_CODE, code_buf);
          i = 0;
          continue;
        }
        if (checkpat(q_buf, "hq"))
        {
          strcpy(q_buf, "[hq]");
          sd_cursor(0, fld[6].irow, fld[6].icol);
          sd_text(buf[6]);
        }
      }
      else if (*code_buf == 'h')          /* hold                            */
      {
        if (checkpat(q_buf, "q"))
        {
          strcpy(q_buf, "q");
          sd_cursor(0, fld[6].irow, fld[6].icol);
          sd_text(buf[6]);
        }
      }
      else if (*code_buf == 'p')          /* pick                            */
      {
        if (checkpat(q_buf, "q"))
        {
          strcpy(q_buf, "q");
          sd_cursor(0, fld[6].irow, fld[6].icol);
          sd_text(buf[6]);
        }
      }
      else if (*code_buf == 'x')          /* priority change                 */
      {
        if (checkpat(q_buf, "hq"))
        {
          strcpy(q_buf, "[hq]");
          sd_cursor(0, fld[6].irow, fld[6].icol);
          sd_text(buf[6]);
        }
      }
      else if (*code_buf == 'l')          /* pickline change                 */
      {
        if (checkpat(q_buf, "hq"))
        {
          strcpy(q_buf, "[hq]");
          sd_cursor(0, fld[6].irow, fld[6].icol);
          sd_text(buf[6]);
        }
      }
      else
      {
        eh_post(ERR_CODE, code_buf);
        i = 0;
        continue;
      }
      sd_cursor(0, fld[6].irow, fld[6].icol);
      sd_text(q_buf);

/*-------------------------------------------------------------------------*
 *  Check Priority Value
 *-------------------------------------------------------------------------*/
      p = pri_buf;
      while (*p) {*p = tolower(*p); p++;}

      if (!(*pri_buf))                    strcpy(pri_buf, "*");
      else if (strcmp(pri_buf, "?") == 0) strcpy(pri_buf, "*");
      else if (strcmp(pri_buf, "*") == 0) strcpy(pri_buf, "*");
      else if (checkpat(pri_buf, "hml"))
      {
        eh_post(ERR_CODE, pri_buf);
        i = 5;
        continue;
      }
/*-------------------------------------------------------------------------*
 *  Pickline Change
 *-------------------------------------------------------------------------*/
      if (*code_buf == 'l')
      {
        pickline = pl_lookup(pl_buf, -1);
        if (pickline < 1 || pickline > coh->co_pl_cnt)
        {
          eh_post(ERR_PL, pl_buf);
          i = 0;
          continue;
        }
        if (!pl[pickline - 1].pl_pl)
        {
          eh_post(ERR_PL, pl_buf);
          i = 0;
          continue;
        }
        memset(new_pl_buf, 0, BUF_SIZE);

        while(1)
        {
          t = sd_input(&fld[7], sd_prompt(&fld[7],0),0, new_pl_buf,0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;
          new_pickline = pl_lookup(new_pl_buf, 0);
          if (new_pickline < 1 || new_pickline > coh->co_pl_cnt)
          {
            eh_post(ERR_PL, new_pl_buf);
            continue;
          }
          if (!pl[new_pickline - 1].pl_pl || pickline == new_pickline)
          {
            eh_post(ERR_PL, new_pl_buf);
            continue;
          }
          break;
        }   
        if (t == UP_CURSOR) continue;
      }
/*-------------------------------------------------------------------------*
 *  Group Change
 *-------------------------------------------------------------------------*/
      if (*code_buf == 'g' && GP_LEN > 0)
      {
        memset(new_grp_buf, 0, BUF_SIZE);

        while(1)
        {
          t = sd_input(&fld[8], sd_prompt(&fld[8],0),0,new_grp_buf,0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;
          if (!(*new_grp_buf))
          {
            eh_post(ERR_GRP_REQ, 0);
            continue;
          }
          break;
        }
        if (t == UP_CURSOR) continue;
      }

/*-------------------------------------------------------------------------*
 *  Priority Change
 *-------------------------------------------------------------------------*/
      if (*code_buf == 'x')
      {
        while(1)
        {
          t = sd_input(&fld[9], sd_prompt(&fld[9],0),0,new_pri_buf,0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;
          if (*new_pri_buf == 'h' || 
              *new_pri_buf == 'm' || 
              *new_pri_buf == 'l') break;

          eh_post(ERR_OF_PRI,0);
        }
        if (t == UP_CURSOR) continue;
      }
/*-------------------------------------------------------------------------*
 * prompt for after group and find the last order in that group
 *-------------------------------------------------------------------------*/

      if (*code_buf == 'a' || *code_buf == 'p' || *code_buf == 'l')
      {
        memset(aft_grp_buf, 0, BUF_SIZE);
        memset(aft_on_buf,  0, BUF_SIZE);
        
        while(1)
        {
          if (GP_LEN > 0)
          {
            sd_prompt(&fld[10], 0);
            t = sd_input(&fld[10],0,0,aft_grp_buf,0);
            if (t == EXIT) leave();
            if (t == UP_CURSOR) break;
    
            if (*aft_grp_buf) 
            {
              strip_space(aft_grp_buf, rf->rf_grp);

              if (Blike(grp_buf, aft_grp_buf))
              {
                eh_post(ERR_AFTER_GRP, grp_buf);   /* not after self         */
                continue;
              }
              break;
            }
          }
          sd_prompt(&fld[11], 0);
          t = sd_input(&fld[11],0,0,aft_on_buf,0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) continue;
          break;
        }                                /* end of while(1)                  */
        if (t == UP_CURSOR) continue;
      }
/*-------------------------------------------------------------------------*
 *  Ask For Record Count
 *-------------------------------------------------------------------------*/
      sd_prompt(&fld[12], 0);
      
      while(1)
      {
        t = sd_input(&fld[12], 0, 0, count_buf, 0);
        if (t == EXIT) leave();
        if (t == UP_CURSOR) break;
        max_count = atol(count_buf);
        if (max_count < 1) max_count = 0x7fffffff;
        break;
      }
      if (t == UP_CURSOR) {i = 6; continue;}

/*-------------------------------------------------------------------------*
 *  Are You Sure? - all but find request
 *-------------------------------------------------------------------------*/  

      if (*code_buf != 'f')
      {
        sd_prompt(&fld[13], 0);           /* are you sure message            */
      
        while(1)
        {
          t = sd_input(&fld[13], 0, 0, sure_buf, 0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;
          *sure_buf = tolower(*sure_buf);
          if (code_to_caps(*sure_buf) == 'y') break;   /* F041897 */
          if (code_to_caps(*sure_buf) == 'n') break;
          eh_post(ERR_YN,0);
        }
        if (t == UP_CURSOR) {i = 6; continue;}
      }
      break;    
    }
    if (t == F_KEY_7) continue;
    if (code_to_caps(*sure_buf) == 'n') continue;   /* F041897 */

/*-------------------------------------------------------------------------*
 *  Pickline Action
 *-------------------------------------------------------------------------*/
    found_count = 0;                        /* total orders processed        */
    orphans = 0;                            /* clear orphans flag            */
    
    if (*code_buf != 'f') oc_lock();
    
    for (pickline = 1; pickline <= coh->co_pl_cnt; pickline++)
    {
      if (pl[pickline - 1].pl_pl != pickline) continue;

      sd_wait();

#ifdef DEBUG
  fprintf(stderr, "doing pickline = %d\n", pickline);
#endif

      if (!(*pl_buf))                    ret = do_action();
      else if (strcmp(pl_buf, "0") == 0) ret = do_action();
      else if (strcmp(pl_buf, "*") == 0) ret = do_action();
      else if (*pl_buf == '[' || *pl_buf == '?')
      {
        sprintf(pl_wrk, "%d", pickline);
        if (Blike(pl_buf, pl_wrk)) ret = do_action();
      }
      else if (pickline == pl_lookup(pl_buf, -1)) ret = do_action();
      if (ret) break;
    }
    if (*code_buf != 'f') oc_unlock();
    
    if (ret) continue;

    if (orphans)
    {
      sd_wait();

      if (fork() == 0)
      {
        ss_close();
        co_close();
        oc_close();
        od_close();

        execlp("orphan_picks", "orphan_picks", 0);
        krash("main", "load orphan_picks", 1);
      }
      pid = wait(&stats);
      if (pid < 0 || stats) krash("main", "orphan_picks failed", 1);
    }

    if (!found_count) eh_post(LOCAL_MSG, "No matching orders found");
    else 
    {
      sd_screen_off();
      sd_clear_screen();
      fix(select_comms);
      sd_text(select_comms);
      sd_screen_on();
  
      sprintf(work, "%d Orders", found_count);
      eh_post(ERR_CONFIRM, work);
    }
    if (*order_plus) strcat(on_buf,  order_plus);
    if (*group_plus) strcat(grp_buf, group_plus);
    if (*con_plus)   strcat(con_buf, con_plus);
  }
}

/*-------------------------------------------------------------------------*
 * Process the Requested Action
 *-------------------------------------------------------------------------*/
do_action()
{
#ifdef DEBUG
  fprintf(stderr, "do_action()  code=%c\n", *code_buf);
#endif

  switch (*code_buf)
  {
    case 'a': return activate_control();

    case 'c': return cancel_control();

    case 'f': return find_control();

    case 'g': return group_change_control();

    case 'h': return hold_control();
              
    case 'p': return pick_control();

    case 'l': return pickline_change_control();
              
    case 'x': return priority_change_control();
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
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
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Close All File But Screen Driver
 *-------------------------------------------------------------------------*/
close_all()
{
  if (fp)
  {
    fclose(fp);
    unlink(fp_name);
  }
  oc_close_save();
  od_close();
  co_close();
  ss_close();
  database_close();
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
  sd_tab(7, " ");
  sd_close();
  execlp("operm","operm",0);
  krash("leave", "operm load", 1);
}
/*-------------------------------------------------------------------------*
 *   activate function
 *-------------------------------------------------------------------------*/
activate_control()
{
  if (check_after_order(pickline)) return 1;

  find_order(activate_group);
  return 0;
}
activate_group()
{
  if (!after_queue)
  {
    after_priority = of_rec->of_pri;
    after_block    = OC_FIRST;
    
    if (of_rec->of_pri == 'h')      after_queue = OC_HIGH;
    else if (of_rec->of_pri == 'm') after_queue = OC_MED;
    else                            after_queue = OC_LOW;
  }
  of_rec->of_status   = 'q';
  of_rec->of_pri      = after_priority;
  of_rec->of_datetime = time(0);
  order_update(of_rec);
  
  oc_enqueue(order_block, after_block, after_queue);
  
  after_block = order_block;
}
 
/*-------------------------------------------------------------------------*
 * cancel group function
 *-------------------------------------------------------------------------*/
cancel_control()
{
  find_order(cancel_group);
  return 0;
}
cancel_group(block)
register long block;
{
  TOrderEventMessage x;

  x.m_pickline = oc->oi_tab[block - 1].oi_pl;
  x.m_order    = oc->oi_tab[block - 1].oi_on;

  if (oc->oi_tab[block - 1].oi_queue == OC_UW)
  {
    message_put(0, OrderCancelRequest, &x, sizeof(TOrderMessage));
    return 0;
  }
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
 *  find group control
 *-------------------------------------------------------------------------*/
find_control()
{
  char work[64];
  static long last_count;
#ifdef DEBUG
  fprintf(stderr, "find_control()  found_count=%d\n", found_count);
#endif

  last_count = found_count;
  
  tmp_name(fp_name);
  fp = fopen(fp_name, "w");
  if (fp == 0) krash("find_control", "open tmp", 1);

  find_order(find_group);
  
  fclose(fp);
  sd_msg(" ");

  if (last_count == found_count) 
  {
    unlink(fp_name);
    return 0;
  }
  sprintf(work, "%d", pickline);
  chng_pkln(work);
  
  sprintf(work, "%d Orders Found", found_count - last_count);
  eh_post(LOCAL_MSG, work);
  
  display_picks();

  return 0;
}
find_group(block)
register long block;
{
  register struct oi_item *o;
  char stat[16], orphan;
  
  o = &oc->oi_tab[block - 1];

  if (o->oi_flags & ORPHANS) orphan = '*';
  else orphan = 0x20;

  switch (of_rec->of_status)
  {
    case 'h': strcpy(stat, "Held");      break;
    case 'c': strcpy(stat, "Completed"); break;
    case 'x': strcpy(stat, "Cancelled"); break;
    case 'u': strcpy(stat, "Underway");  break;
    case 'q': strcpy(stat, "Queued");    break;
    default:  strcpy(stat, "Unknown");   break;
  }
             
/*         1         2         3         4         5         6         7     
 *12345678901234567890123456789012345678901234567890123456789012345678901234567
 * Pickline Priority Group    Order Lines  Units  Status     Order Reference
 *.xxxxxxxx....x.....xxxxxx xxxxxxx..xxxx...xxxx..xxxxxxxxxx.xxxxxxxxxxxxxxx
 */
  if (sp->sp_pl_by_name == 'y' && pl[of_rec->of_pl - 1].pl_pl)
  {
    fprintf(fp,
        "%c%-8.8s    %c     %-6.*s %7.*d  %4d   %4d  %-10s %-15.15s\n",
        orphan, pl[of_rec->of_pl - 1].pl_name,
        of_rec->of_pri, GroupLength, of_rec->of_grp, rf->rf_on,
        of_rec->of_on, of_rec->of_no_picks, of_rec->of_no_units,
        stat, of_rec->of_con);
  }
  else
  {
    fprintf(fp,
      " %c  %2d       %c     %-6.*s %7.*d  %4d   %4d  %-10s %-15.15s\n",
      orphan, of_rec->of_pl,
      of_rec->of_pri, GroupLength, of_rec->of_grp, rf->rf_on,
      of_rec->of_on, of_rec->of_no_picks, of_rec->of_no_units,
      stat, of_rec->of_con);
  }
  return 0;
}

/*-------------------------------------------------------------------------*
 *  group change control
 *-------------------------------------------------------------------------*/
group_change_control()
{
  space_fill(new_grp_buf, GroupLength);

  find_order(change_group);
  
  strip_space(new_grp_buf, GroupLength);
  return 0;
}
change_group(block)
register long block;
{
  memcpy(oc->oi_tab[block - 1].oi_grp, new_grp_buf, GroupLength);
  memcpy(of_rec->of_grp, new_grp_buf, GroupLength);
  
  of_rec->of_datetime = time(0);
  od_update(block);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  hold function
 *-------------------------------------------------------------------------*/
hold_control()
{
  after_block = OC_LAST;

  find_order(hold_group);
  return 0;  
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
 *  pick control
 *-------------------------------------------------------------------------*/
pick_control()
{
  if (check_after_order(pickline)) return 1;
  find_order(pick_group);
  return 0;
}
pick_group()
{
  if (!after_queue)
  {
    after_priority = 'h';
    after_queue    = OC_HIGH;
    after_block    = OC_FIRST;
  }
  if (found_count == 1)                   /* check after in selection        */
  {
    if (oc->oi_tab[after_block - 1].oi_queue != OC_HIGH && 
        oc->oi_tab[after_block - 1].oi_queue != OC_MED  &&
        oc->oi_tab[after_block - 1].oi_queue != OC_LOW) 
    {
      after_priority = 'h';
      after_queue    = OC_HIGH;
      after_block    = OC_FIRST;
    }
  }
#ifdef DEBUG
  fprintf(stderr, "after_block=%d order_block=%d after_queue=%d\n", 
    after_block, order_block, after_queue);
#endif
  
  of_rec->of_pri      = after_priority;
  of_rec->of_datetime = time(0);
  od_update(order_block);
  
  oc_enqueue(order_block, after_block, after_queue);
  
  after_block = order_block;
}
/*-------------------------------------------------------------------------*
 * pickline change function
 *-------------------------------------------------------------------------*/
pickline_change_control()
{
  if (check_after_order(new_pickline)) return 1;
  find_order(pickline_group);
  return 0;
}
pickline_group()
{
  if (!after_queue)
  {
    after_queue    = OC_HIGH;
    after_priority = 'h';
    after_block    = OC_LAST;
  }
  orphans |= od_move(order_block, new_pickline);

  of_rec->of_pl       = new_pickline;
  of_rec->of_pri      = after_priority;

  of_rec->of_status   = 'q';
  of_rec->of_datetime = time(0);
  order_update(of_rec);

  oc_enqueue(order_block, after_block, after_queue);
        
  after_block = order_block;
}
/*-------------------------------------------------------------------------*
 * change priority function
 *-------------------------------------------------------------------------*/
priority_change_control()
{
  after_priority = *new_pri_buf;
  
  if (after_priority == 'h')      after_queue = OC_HIGH;
  else if (after_priority == 'm') after_queue = OC_MED;
  else                            after_queue = OC_LOW;

  find_order(change_priority);

  return;
}
change_priority()
{
  of_rec->of_pri      = after_priority;
  of_rec->of_datetime = time(0);

  od_update(order_block);

  if (of_rec->of_status == 'h')
  {
    oc_enqueue(order_block, OC_LAST, OC_HOLD);
  }
  else oc_enqueue(order_block, OC_LAST, after_queue);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Search Queues For Groups
 *-------------------------------------------------------------------------*/
find_order(func)
long (*func)();
{
  if (*order_plus == '-' || *group_plus == '-' || *con_buf == '-')
  {
    if (!find_backward(func)) return 0;
  }
  return find_forward(func);
}
/*-------------------------------------------------------------------------*
 *  Search Forward For Groups
 *-------------------------------------------------------------------------*/
find_forward(func)
long (*func)();
{
  register struct oi_item  *o;
  register long k, flag, need_read;
  register long block, where;
  char on_wrk[OrderLength + 1];
  char grp_wrk[GroupLength + 1];
  char con_wrk[CustomerNoLength + 1];
  char work[2];
  
#ifdef DEBUG
  fprintf(stderr, "find_forward(%x)\n", func);
#endif

  count = 0;

  for (k = OC_COMPLETE; k <= OC_HOLD; k++)
  {
    flag = 1;                               /* queue ok initially;           */
    if (*con_buf || *code_buf == 'f' || *code_buf == 'g') 
    {
      need_read = 1;                        /* need order record             */
    }
    else need_read = 0;
    
    switch (k)
    {
      case OC_COMPLETE:
           
           if (*q_buf != '*')
           {
             if (Blike(q_buf, "x")) need_read = 1;
             else if (!Blike(q_buf, "c")) flag = 0;
           }
           if (*pri_buf != '*') need_read = 1;
           break;
           
      case OC_UW:

           if (!Blike(q_buf, "u")) flag = 0;
           if (*pri_buf != '*') need_read = 1;
           break;
  
      case OC_HIGH:
       
           if (!Blike(q_buf,   "q")) flag = 0;
           if (!Blike(pri_buf, "h")) flag = 0;
           break;
           
      case OC_MED:
      
           if (!Blike(q_buf,   "q")) flag = 0;
           if (!Blike(pri_buf, "m")) flag = 0;
           break;
      
      case OC_LOW:
      
           if (!Blike(q_buf,   "q")) flag = 0;
           if (!Blike(pri_buf, "l")) flag = 0;
           break;

      case OC_HOLD:

           if (!Blike(q_buf, "h")) flag = 0;
           if (*pri_buf != '*') need_read = 1;
           break;
    }
#ifdef DEBUG
  fprintf(stderr, "queue = %d  flag = %d\n", k, flag);
#endif

    if (!flag) continue;                   /* queue is not useful            */

    block = oc->oc_tab[pickline - 1].oc_queue[k].oc_first;
    
    while (block)
    {
      if (found_count + count >= max_count) break;

      where = block;
      o = &oc->oi_tab[block - 1];
      block = o->oi_flink;

      if (*on_buf && !plus_found)          /* check order number match      */
      {
        sprintf(on_wrk, "%0*d", rf->rf_on, o->oi_on);
        if (!Blike(on_buf, on_wrk)) continue;
      }
      if (*grp_buf && !plus_found)
      {
        memcpy(grp_wrk, o->oi_grp, rf->rf_grp);
        grp_wrk[rf->rf_grp] = 0;
        if (!Blike(grp_buf, grp_wrk)) continue;
      }
      if (need_read)
      {
        od_read (where);                  /* get order without lock          */
      
        work[0] = of_rec->of_pri; work[1] = 0;
        if (!Blike(pri_buf, work)) continue;
        work[0]  = of_rec->of_status; work[1] = 0;
        if (!Blike(q_buf, work)) continue;
        if (*con_buf && !plus_found)
        {
          memcpy(con_wrk, of_rec->of_con, rf->rf_con);
          con_wrk[rf->rf_con] = 0;
          if (!Blike(con_buf, con_wrk)) continue;
        }
      }
      if (*order_plus == '+' || *group_plus == '+' || *con_plus == '+')
      {
        plus_found = 1;
      }
      if (*code_buf == 'f' || *code_buf == 'g' || o->oi_queue == OC_UW)
      {
        found_count++;

        (*func)(where);                    /* do not dequeue item            */
        continue;
      }
      count++;
      oc_dequeue(where);
      oc_enqueue(where, OC_LAST, OC_WORK);
    }
  }
  if (!count) return;
          
  while (order_block = oc->oc_tab[pickline - 1].oc_queue[OC_WORK].oc_first)
  {
    found_count++;

    begin_work();
    oc_dequeue(order_block);
    od_get(order_block);
    (*func)(order_block);
    commit_work();
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Search Backward For Groups
 *-------------------------------------------------------------------------*/
find_backward(func)
long (*func)();
{
  register struct oi_item  *o;
  register long k, flag, need_read;
  register long block, where, found, count;
  char on_wrk[OrderLength + 1];
  char grp_wrk[GroupLength + 1];
  char con_wrk[CustomerNoLength + 1];
  char work[2];
  
#ifdef DEBUG
  fprintf(stderr, "find_backward(%x)\n", func);
#endif

  found = count = 0;

  for (k = OC_HOLD; k >= OC_COMPLETE; k--)
  {
    flag = 1;                               /* queue ok initially;           */
    if (*con_buf || *code_buf == 'f' || *code_buf == 'g') 
    {
      need_read = 1;                        /* need order record             */
    }
    else need_read = 0;
    
    switch (k)
    {
      case OC_COMPLETE:
           
           if (*q_buf != '*')
           {
             if (Blike(q_buf, "x")) need_read = 1;
             else if (!Blike(q_buf, "c")) flag = 0;
           }
           if (*pri_buf != '*') need_read = 1;
           break;
           
      case OC_UW:

           if (!Blike(q_buf, "u")) flag = 0;
           if (*pri_buf != '*') need_read = 1;
           break;
  
      case OC_HIGH:
       
           if (!Blike(q_buf,   "q")) flag = 0;
           if (!Blike(pri_buf, "h")) flag = 0;
           break;
           
      case OC_MED:
      
           if (!Blike(q_buf,   "q")) flag = 0;
           if (!Blike(pri_buf, "m")) flag = 0;
           break;
      
      case OC_LOW:
      
           if (!Blike(q_buf,   "q")) flag = 0;
           if (!Blike(pri_buf, "l")) flag = 0;
           break;

      case OC_HOLD:

           if (!Blike(q_buf, "h")) flag = 0;
           if (*pri_buf != '*') need_read = 1;
           break;
    }
#ifdef DEBUG
  fprintf(stderr, "queue = %d  flag = %d\n", k, flag);
#endif

    if (!flag) continue;                   /* queue is not useful            */

    block = oc->oc_tab[pickline - 1].oc_queue[k].oc_last;
    
    while (block)
    {
      if (count >= max_count) break;

      where = block;
      o = &oc->oi_tab[block - 1];
      block = o->oi_blink;

      if (*on_buf && !found)
      {
        sprintf(on_wrk, "%0*d", rf->rf_on, o->oi_on);
        if (!Blike(on_buf, on_wrk)) continue;
      }
      if (*grp_buf && !found)
      {
        memcpy(grp_wrk, o->oi_grp, rf->rf_grp);
        grp_wrk[rf->rf_grp] = 0;
        if (!Blike(grp_buf, grp_wrk)) continue;
      }
      if (need_read)
      {
        od_read(where);                  /* get order without lock           */
      
        work[0] = of_rec->of_pri; work[1] = 0;
        if (!Blike(pri_buf, work)) continue;
        work[0]  = of_rec->of_status; work[1] = 0;
        if (!Blike(q_buf, work)) continue;
        if (*con_buf && !found)
        {
          memcpy(con_wrk, of_rec->of_con, rf->rf_con);
          con_wrk[rf->rf_con] = 0;
          if (!Blike(con_buf, con_wrk)) continue;
        }
      }
      count++;
      found = 1;
    }
  }
  if (!count) return 0;

  sprintf(on_buf, "%0.*d", rf->rf_on, o->oi_on);
  memset(grp_buf, 0, BUF_SIZE);
  memset(con_buf, 0, BUF_SIZE);

  order_plus[0] = '+';
  group_plus[0] = con_plus[0] = 0;
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Check After Group/Order
 *-------------------------------------------------------------------------*/
check_after_order(who)
register long who;                         /* pickline                       */
{      
  char work[GroupLength + 1];

  if (*aft_on_buf)
  {
    after_order = atol(aft_on_buf);
    after_block = oc_find(who, after_order);
    if (!after_block) 
    {
      eh_post(ERR_ORDER, aft_on_buf);
      return 1;
    }
  }
  else if (*aft_grp_buf)
  {  
    find_last_order(who, aft_grp_buf);
    if (!after_block)
    {
      eh_post(GRP_NOT_FND, aft_grp_buf);
      return 1;
    }
  }
  else 
  {
    after_order    = 0;
    after_queue    = 0;
    return 0;
  }
  if (sp->sp_tl_mode == 'a') enter_high_order();
  
  after_queue = oc->oi_tab[after_block - 1].oi_queue;
  
#ifdef DEBUG
  fprintf(stderr, "after_order=%d  after_block= %d after_queue=%d\n",
    after_order, after_block, after_queue);
#endif

  if (after_queue == OC_HIGH) {after_priority = 'h'; return 0;}
  if (after_queue == OC_MED)  {after_priority = 'm'; return 0;}
  if (after_queue == OC_LOW)  {after_priority = 'l'; return 0;}

  eh_post(ERR_GRP_NQ, aft_grp_buf);
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Find Last Queued Order Of A Group
 *-------------------------------------------------------------------------*/
find_last_order(who, p)
register long who;
register char *p;
{
  register long k;
  
  after_block = 0;

  for (k = OC_LOW; k >= OC_HIGH; k--)
  {
    after_block = oc->oc_tab[who - 1].oc_queue[k].oc_last;
    
    while (after_block)
    {
      if (memcmp(p, oc->oi_tab[after_block - 1].oi_grp, GroupLength) == 0) 
      {
        return;
      }
      after_block = oc->oi_tab[after_block - 1].oi_blink;
    }
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Synchronize After Order Across Picklines
 *-------------------------------------------------------------------------*/
sync_after_order()
{
  register long k, block;
  register struct oi_item *x;

  if (check_after_order()) return 1;     /* failed                           */

  if (!after_order) return 0;

  for (k = 2; k <= sp->sp_picklines; k++)
  {
    if (!(pl[k - 1].pl_pl)) continue;
  
    block = oc_find(k, after_order);
    if (!block)
    {
      eh_post(ERR_ORDER, aft_on_buf);
      return 1;
    }
    x = &oc->oi_tab[block - 1];
    if (x->oi_queue != OC_HIGH) 
    {
      eh_post(ERR_ORDER, aft_on_buf);
      return 1;
    }
  }
  sprintf(aft_on_buf, "%d", after_order);

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
/*-------------------------------------------------------------------------*
 *  Check Patterns
 *
 *  p is entry      q  is pattern
 *  ----------      -------------
 *   x              x must be in pattern
 *   [xyz]          x, y, and z in pattern.
 *
 *  return 1;       mismatch
 *  return 0;       input of OK.
 *-------------------------------------------------------------------------*/
checkpat(p, q)
register char *p, *q;
{
  register long plen;
  char mask[128];
  
  plen = strlen(p);
  
  if (plen == 1)                          /* input is one byte               */
  {
    if (memchr(q, *p, strlen(q))) return 0;
    else return 1;
  }
  memset(mask, 0, 128);
  
  plen -= 2;
  if (plen < 1) return 1;                /* too  short                       */

  if (*p != '[') return 1;               /* must have [...                   */
  p++;
  if (p[plen] != ']') return 1;          /* must have ...]                   */
  
  while (*q) {mask[*q] = 1; q++;}        /* flag acceptable letters          */
  while (plen > 0) 
  {
    if (!mask[*p]) return 1;             /* not found or duplicate           */
    mask[*p] = 0;
    p++;
    plen--;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Display Picks
 *-------------------------------------------------------------------------*/
display_picks()
{
#ifdef DEBUG
  fprintf(stderr, "display_picks()\n");
#endif

  savefp = 0;

  fp = fopen(fp_name, "r");
  if (fp == 0) return 0;
  
  display_screen();

  if (fp)
  {
    fclose(fp);
    unlink(fp_name);
  }
  return 0;
}
display_screen()
{
  register long k;

  struct fld_parms fldyn = {23,25,5,1,&ONE, "Print? (y/n)",'a'};
  char buf[2];

  for (k = 6; k <= 23; k++)
  {
    sd_cursor(0, k, 1);
    sd_clear_line();
  }
  sd_cursor(0, 6, 2);
  sd_text(
  "Pickline Priority Group    Order Lines  Units  Status     Order Reference");

  savefp = 0;
  sd_cursor(0, 7, 1);
  show(fp, 14, 1);

  memset(buf, 0, 2);
  
  while(1)
  {
    sd_cursor(0,23,28);
    sd_text("(Exit, Forward, or Backward)");
    t = sd_input(&fldyn, sd_prompt(&fldyn, 0), 0, buf,0);

    switch(sd_print(t, code_to_caps(*buf)))  /* F041897 */
    {
      case(0): leave();
   
      case(1): sd_cursor(0, 7, 1);
               sd_clear_rest();
               sd_cursor(0, 7, 1);
               show(fp, 14, 1);
               break;

      case(2): sd_cursor(0, 7, 1);
               sd_clear_rest();
               sd_cursor(0, 7, 1);
               show(fp, 14, 2);
               break;
 
      case(3): return;
               
      case(4): fclose(fp); fp = 0;
               print_all();
               return 0;
    
      case(6): eh_post(ERR_YN,0);
               break;
    }
  }
}
/*------------------------------------------------------------------------*
 * function to display x number of lines of data on the screen               
 * Arguments:                                                               
 *           fp : the data file pointer.                                    
 *           lines : the number of lines to be displayed.                   
 *           i : the indicator of either going forward or                   
 *           backward on the file.                                      
 *------------------------------------------------------------------------*/
show(fp, lines, i)
register FILE *fp;
register long lines, i;
{
  register long pos, size;
  char str[1920];
  short j;

  memset(str, 0, 1920);
  
  pos = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);
  
  if (i == 2)
  {
    pos = savefp - lines * WIDTH;
    if (pos < 0) pos = 0;                 /*if past the begining of file     */
    savefp = pos;

    fseek(fp, pos, 0); 
    fread(str, WIDTH, lines, fp);
    sd_text(str);
  }
  else if (i == 1)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;

    fseek(fp, pos, 0); 
    fread(str, WIDTH, lines, fp);
    sd_text(str);
  }
  return 0;
}
/*------------------------------------------------------------------------* 
 * print the data
 *------------------------------------------------------------------------*/
print_all()
{
  char print_file[16];
  long pid, stats;
  
  if(fork() == 0)                         /*child process                    */
  {
    ss_close();
    co_close();
    oc_close();
    od_close();

    execlp("prft", "prft", fp_name, tmp_name(print_file),
    "sys/report/order_display_rpt.h", 0);
    krash("print_all", "prft load", 0);
    exit(1);
  }
  pid = wait(&stats);
  if (pid < 0 || stats) krash("print_all", "prft failed", 1);
  return;
}

/* end of select_comms.c */
