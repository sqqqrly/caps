/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Lot Control Manual Operations.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  12/22/94   |  tjt Original implementation.
 *  06/03/95   |  tjt Add pickline input by name.
 *  07/21/95   |  tjt Revise Bard calls.
 *  08/23/96   |  tjt Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char lot_control_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "iodefs.h"
#include "ss.h"
#include "sd.h"
#include "eh_nos.h"
#include "getparms.h"
#include "message_types.h"
#include "caps_messages.h"
#include "lot_control.t"

#include "Bard.h"
#include "bard/lot.h"

extern leave();

short ONE = 1;
short QL;
short SKU;
short LOT = 15;
short LPL = 8;

struct fld_parms fld1 = {14, 50, 25, 1, &ONE, "Enter Code", 'a'};
struct fld_parms fld2 = {15, 50, 25, 1, &LPL, "Enter Pickline", 'a'};
struct fld_parms fld3 = {16, 50, 25, 1, &SKU, "Enter SKU",  'a'};
struct fld_parms fld4 = {17, 50, 25, 1, &LOT, "Enter Lot",  'a'};
struct fld_parms fld5 = {23, 20,  2, 1, &ONE, "More? (y/n)", 'a'};
struct fld_parms fld6 = {18, 50, 25, 1, &QL,  "Quantity", 'n'};
struct fld_parms fld7 = {15, 50, 25, 1, &ONE, "Proceed? (y/n)", 'a'};
char code[2];
unsigned char t;

main (argc, argv)
long argc;
char **argv;
{
  putenv("_=lot_control");
  chdir(getenv("HOME"));
  
  database_open();
  ss_open();
  co_open();
  sd_open(leave);
  lot_open(AUTOLOCK);
  lot_setkey(1);
  
  fix(lot_control);
  sd_screen_off();
  sd_clear_screen();
  sd_text(lot_control);
  sd_screen_on();

  getparms(0);

  SKU = rf->rf_sku ? rf->rf_sku : 5;
  QL  = rf->rf_quan;
  
  while (1)
  {
    sd_prompt(&fld1, 0);
    t = sd_input(&fld1, 0, 0, code, 0);
    if (t == EXIT) leave();

    sd_cursor(0, 15, 1); sd_clear_line();
    sd_cursor(0, 16, 1); sd_clear_line();
    sd_cursor(0, 17, 1); sd_clear_line();
    sd_cursor(0, 18, 1); sd_clear_line();
    sd_cursor(0, 19, 1); sd_clear_line();
    sd_cursor(0, 20, 1); sd_clear_line();
    sd_cursor(0, 21, 1); sd_clear_line();
    sd_cursor(0, 22, 1); sd_clear_line();
    sd_cursor(0, 23, 1); sd_clear_line();

    switch(tolower(*code))
    {
      case 'a':  add_lot();
                 break;
                 
      case 'd':  delete_lot();
                 break;
                 
      case 'q':  show_lot();
                 break;
      
      case 'r':  report_lot();
                 break;

      case 's':  split_pick();
                 break;
                 
      case 'p':  purge_lot();
                 break;
                 
      default:   eh_post(ERR_CODE, code);
                 break;
    }
  }
  leave();
}
/*-------------------------------------------------------------------------*
 *  Add A Lot
 *-------------------------------------------------------------------------*/
add_lot()
{
  char sku[15], lot[15];
  lot_item x;
  long pickline, ok;
  
  if (sp->sp_running_status == 'y')
  {
    eh_post(ERR_IS_CONFIG, 0);
    return 0;
  }
  if (sp->sp_config_status != 'y')
  {
    eh_post(ERR_NO_CONFIG, 0);
    return 0;
  }
  sd_prompt(&fld3, 0);
  sd_prompt(&fld4, 0);

  memset(sku, 0, 15);
  memset(lot, 0, 15);

  while (1)
  {
    pickline = get_pickline();
    
    while (1)
    {
      t = sd_input(&fld3, 0, 0, sku, 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;
      
      if (!sku_lookup(pickline, sku))
      {
        eh_post(ERR_SKU_INV, sku);
        continue;
      }
      while (1)
      {
        t = sd_input(&fld4, 0, 0, lot, 0);
        if (t == EXIT) leave();
        if (t == UP_CURSOR) break;
        
        strip_space(sku, 15);
        strip_space(lot, 15);
    
        x.l_lot_time = 0;
        x.l_lot_pl   = pickline;
        memcpy(x.l_lot_sku, sku, 15);
        lot_startkey(&x);
    
        x.l_lot_time = 0x7fffffff;
        lot_stopkey(&x);

        ok = 1;
    
        begin_work();
        while (!lot_next(&x, NOLOCK))
        {
          if (memcmp(x.l_lot_number, lot, 15) == 0) 
          {
            ok = 0;
            break;
          }
        }
        commit_work();
        
        if (ok)
        {
          x.l_lot_time = time(0);
          x.l_lot_pl   = pickline;
          memcpy(x.l_lot_sku, sku, 15);
          memcpy(x.l_lot_number, lot, 15);
          lot_write(&x);
          eh_post(ERR_CONFIRM, "Add Lot");
          return 0;
        }
        eh_post(LOCAL_MSG, "Duplicate Lot Number");
      }
    }
  }
}

/*-------------------------------------------------------------------------*
 *  Delete A Lot
 *-------------------------------------------------------------------------*/
delete_lot()
{
  char sku[15], lot[15];
  lot_item x;
  long pickline;
  
  if (sp->sp_running_status == 'y')
  {
    eh_post(ERR_IS_CONFIG, 0);
    return 0;
  }
  sd_prompt(&fld3, 0);
  sd_prompt(&fld4, 0);

  memset(sku, 0, 15);
  memset(lot, 0, 15);

  while (1)
  {
    pickline = get_pickline();
    
    while (1)
    {
      t = sd_input(&fld3, 0, 0, sku, 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;
      
      while (1)
      {
        t = sd_input(&fld4, 0, 0, lot, 0);
        if (t == EXIT) leave();
        if (t == UP_CURSOR) break;
        
        strip_space(sku, 15);
        strip_space(lot, 15);
    
        x.l_lot_time = 0;
        x.l_lot_pl   = pickline;
        memcpy(x.l_lot_sku, sku, 15);
        lot_startkey(&x);
    
        x.l_lot_time = 0x7fffffff;
        lot_stopkey(&x);

        begin_work();
        while (!lot_next(&x, LOCK))
        {
          if (memcmp(x.l_lot_number, lot, 15) == 0) 
          {
            lot_delete();
            eh_post(ERR_CONFIRM, "Delete Lot");
            commit_work();
            return 0;
          }
        }
        commit_work();
        
        eh_post(LOCAL_MSG, "Lot Number Not Found");
      }
    }
  }
}

/*-------------------------------------------------------------------------*
 *  Display A Lot
 *-------------------------------------------------------------------------*/
show_lot()
{
  char ans[2], sku[15], text[80];
  long pickline, k;
  lot_item x;
  
  memset(sku, 0, 15);
  sd_prompt(&fld3, 0);
  
  while (1)
  {
    pickline = get_pickline();
    
    t = sd_input(&fld3, 0, 0, sku, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return 0;
    
    strip_space(sku, 15);
    
    x.l_lot_time = 0;
    x.l_lot_pl   = pickline;
    memcpy(x.l_lot_sku, sku, 15);
    lot_startkey(&x);
    
    x.l_lot_time = 0x7fffffff;
    lot_stopkey(&x);

    sd_cursor(0, 15, 1); sd_clear_line();

    sd_cursor(0, 16, 10);
    sd_text("        Date              PL SKU             Lot Number");
    sd_cursor(0, 17, 10);
    sd_text(" ------------------------ -- --------------- --------------");
    k = 0;

    while (!lot_next(&x, NOLOCK))
    {
      sprintf(text, " %24.24s %2d %-15.15s %-15.15s", ctime(&x.l_lot_time),
        x.l_lot_pl, x.l_lot_sku, x.l_lot_number);

      if (k >= 5)
      {
        sd_prompt(&fld5, 0);
        sd_input(&fld5, 0, 0, ans, 0);
        if (tolower(*ans) != 'y') return 0;
        k = 0;
        sd_cursor(0, 18, 1);
        sd_clear_rest();
      }
      sd_cursor(0, 18 + k, 10);
      sd_text(text);
      k++;
    }
    return 0;
  }
}

/*-------------------------------------------------------------------------*
 *  Split A Pick
 *-------------------------------------------------------------------------*/
split_pick()
{
  char sku[15], lot[15], quan[5];
  long pickline;
  TLotMessage x;
  
  if (sp->sp_running_status != 'y')
  {
    eh_post(LOCAL_MSG, "CAPS Is Not Running");
    return 0;
  }
  sd_prompt(&fld3, 0);
  sd_prompt(&fld4, 0);
  sd_prompt(&fld6, 0);
  
  memset(sku, 0, 15);
  memset(lot, 0, 15);
  memset(quan, 0, 5);
  
  while (1)
  {
    pickline = get_pickline();
    
    while (1)
    {
      t = sd_input(&fld3, 0, 0, sku, 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;
      
      if (!sku_lookup(pickline, sku))
      {
        eh_post(ERR_SKU_INV, sku);
        continue;
      }
      while (1)
      {
        t = sd_input(&fld4, 0, 0, lot, 0);
        if (t == EXIT) leave();
        if (t == UP_CURSOR) break;
        
        while (1)
        {
          t = sd_input(&fld6, 0, 0, quan, 0);
          if (t == EXIT) leave();
          if (t == UP_CURSOR) break;

          strip_space(sku, 15);
          strip_space(lot, 15);
    
          x.m_pickline = pickline;
          x.m_quantity = atol(quan);
          x.m_keytype  = '1';
          memcpy(x.m_key, sku, 15);
          memcpy(x.m_lot, lot, 15);
          
          message_put(0, LotChangeRequest, &x, sizeof(TLotMessage));
          
          eh_post(ERR_CONFIRM, "Split Lot");
          return 0;
        }
      }
    }
  }
}

/*-------------------------------------------------------------------------*
 *  Report A Lot
 *-------------------------------------------------------------------------*/
report_lot()
{
  FILE *fd;
  char fd_name[16], rd_name[16];
  long status;
  lot_item x;
  
  lot_setkey(1);
  
  tmp_name(rd_name);
  tmp_name(fd_name);
  fd = fopen(fd_name, "w");
  if (fd == 0) krash("report_lot", "open tmp", 1);

  while (!lot_next(&x, AUTOLOCK))
  {
    fprintf(fd, "          %24.24s    %2d    %-15.15s %-15.15s\n",
     ctime(&x.l_lot_time), x.l_lot_pl, x.l_lot_sku, x.l_lot_number);

  }
  fclose(fd);
  if (fork() == 0)
  {
    execlp("prft", "prft", fd_name, rd_name, "sys/report/lot_report.h", 0);
    krash("report_lot", "load prft", 1);
  }
  wait(&status);
  unlink(fd_name);
  eh_post(ERR_CONFIRM, "Lot Report");
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Purge A Lot
 *-------------------------------------------------------------------------*/
purge_lot()
{
  lot_item x, y;
  char ans[4];
  
  if (sp->sp_running_status == 'y')
  {
    eh_post(ERR_IS_CONFIG, 0);
    return 0;
  }
  sd_prompt(&fld7, 0);
  t = sd_input(&fld7, 0, 0, ans, 0);
  if (t == EXIT) leave();
  if (t == UP_CURSOR) return 0;
  
  if (tolower(*ans) != 'y') return 0;

  lot_setkey(1);
  
  memset(&y, 0, sizeof(y));

  while (!lot_next(&x, AUTOLOCK))
  {
    if (x.l_lot_pl == y.l_lot_pl && memcmp(x.l_lot_sku, y.l_lot_sku, 15) == 0)
    {
      lot_delete();
      continue;
    }
    y = x;
  }
  eh_post(ERR_CONFIRM, "Lot Purge");
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Pickline
 *-------------------------------------------------------------------------*/
get_pickline()
{
  unsigned char t, buf[9];
  long pickline;
  
  if (sp->sp_picklines == 1) return 1;
  if (!SUPER_OP)             return op_pl;
  
  sd_prompt(&fld2, 0);
  memset(buf, 0, 9);
  
  while (1)
  {
    t = sd_input(&fld2, 0, 0, buf, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return 0;

    pickline = pl_lookup(buf, op_pl);
    if (pickline <= 0)
    {
      eh_post(ERR_PL, buf);
      continue;
    }
    break;
  }
  return pickline;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  ss_close();
  co_close();
  sd_close();
  lot_close();
  database_close();

  execlp("mmenu", "mmenu", 0);
  krash("leave", "mmenu load", 1);
}
/* end of lot_control.c */



