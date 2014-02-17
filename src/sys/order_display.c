/*-------------------------------------------------------------------------*  
 *  Custom Version: HAWK - lot numbers
 *-------------------------------------------------------------------------*  
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order display.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/28/93   |  tjt  Added to mfc.
 *  05/13/94   |  tjt  Fix order number 1 .. 6 digits right justified.
 *  05/27/94   |  tjt  Cancel go to complete queue.
 *  07/27/94   |  tjt  Fix order number 1 .. 7 digits right justified.
 *  12/10/94   |  tjt  Sorted and totaled by group code.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/04/95   |  tjt  Add sp_pl_by_name.
 *  06/30/95   |  tjt  Add customer order number query.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  07/25/95   |  tjt  Increase screen size.
 *  07/31/96   |  tjt  Remove unprinted temp file.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  04/18/97   |  tjt  Add language and code_to_caps.
 *-------------------------------------------------------------------------*/
static char order_display_c[] = "%Z% %M% %I% (%G% - %U%)";

/****************************************************************************/
/*                                                                          */
/*                               Order Display                              */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "language.h"
#include "order_display.t"
#include "ss.h"
#include "eh_nos.h"
#include "of.h"
#include "co.h"
#include "Bard.h"

#define NUM_PROMPTS     8
#define BUF_SIZE        16

#define WIDTH           80

/*
 *  Global Variables
 */

short ONE    = 1;
short LPL    = 8;
short LGROUP = 4;
short LORDER = 5;
short LZONE  = 3;

static struct fld_parms fld[] = {
  { 6,25,5,1,&LPL,     "Enter Pickline",'a'},
  { 7,25,5,1,&LZONE,   "Enter Zone", 'n'},
  { 8,25,5,1,&ONE,     "Enter Priority",'a'},
  { 6,60,40,1,&ONE,    "Enter Status",'a'},
  { 7,60,40,1,&LORDER, "Enter Order",'a'},
  { 8,60,40,1,&LGROUP, "Enter Group",'a'},
  {10,25,5,1,&ONE,     "Print? (y/n)",'a'},
  {23,25,5,1,&ONE,     "Print? (y/n)",'a'}
};

char temp_file[16], print_file[16];
FILE *fp = 0;
long savefp = 0;

char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */

long pickline = 0;
long zn       = 0;
long order    = 0;

#define priority        buf[2][0]
#define status          buf[3][0]
#define reference       buf[4]
#define group           buf[5]

main()
{
  extern leave();
  register long i, n, si, sx, index, flag, gototop, done;
  unsigned char t, text[64];
  short rm;

  putenv("_=order_display");
  chdir(getenv("HOME"));
  
  open_all();                             /* open all files                  */

  LGROUP = rf->rf_grp;                    /* group length                    */
  LORDER = rf->rf_on;                     /* order number length             */
   
  if (sp->sp_use_con == 'y' || sp->sp_use_con == 'b')
  {
    if (rf->rf_con >= LORDER) LORDER = rf->rf_con + 1;
  }
  while(1)
  {
    fix(order_display);
    sd_screen_off();
    sd_clear_screen();                    /* clear screen                    */
    sd_text(order_display);
    sd_screen_on();
  
    for(i = 0; i < NUM_PROMPTS; i++)      /*clear buffers                    */
    {
      for(n = 0; n < BUF_SIZE; n++) buf[i][n] = 0;
    }
    if(SUPER_OP) rm = 0;
    else rm = 1;

    if (LGROUP < 1) sx = NUM_PROMPTS - 3;
    else sx = NUM_PROMPTS - 2;
    
    if(IS_ONE_PICKLINE || !SUPER_OP) rm = 1;
  
    for(i = rm; i < sx; i++)
    {
      sd_prompt(&fld[i], 0);
    }
    sd_cursor(0, 6, 66);
    sd_text("(H Q U C X)");
    sd_cursor(0, 8, 28);
    sd_text("(H M L)");

    si = rm;
    index = rm;
    while(1)
    {
      gototop = done = flag = 0;

      t = sd_input(&fld[index], 0, 0,buf[index],0);
      if(t == EXIT) leave();

      if(t == UP_CURSOR)
      {
        if(index > si) index--;
        continue;
      }
      else if(t == DOWN_CURSOR || t == TAB)
      {
        index++;
        if(index >= sx) index = si;
        continue;
      }
      if (IS_ONE_PICKLINE) {pickline = op_pl; flag = 1;}
      else
      {
        if (SUPER_OP)
        {
          pickline = pl_lookup(buf[0], 0);
          if (pickline < 0)
          {
            eh_post(ERR_PL, buf[0]);
            index = 0;
            continue;
          }
          flag = 1;
        }
        else pickline = flag = op_pl;
      }
      if (*buf[1] != 0)
      {
        zn = atol(buf[1]);
        flag = 1;
      }
      else zn = 0;
      
      if (priority != 0)
      {
        priority= tolower(priority);
        if(priority != 'h' && priority != 'm' && priority != 'l')
        {
          eh_post(ERR_OF_PRI, 0);
          index = 1;
          continue;
        }
        flag = 1;                         /* have pickline or priority       */
      }
      if (status != 0)
      {
        status = tolower(status);
        if (status != 'h' && status != 'q' && status != 'x' &&
            status != 'c' && status != 'u')
        {
          eh_post(ERR_OF_STATUS, 0);
          index = 2;
          continue;
        }
        flag = 1;                       /* have pickline, priority, or status*/
      }
      if(*reference == 0 && *group == 0 && !flag)
      {
        eh_post(ERR_ENTER, 0);
        index = si;
        continue;
      }
      if (*group) space_fill(group, GroupLength);
      
      if (flag = store_data())                   /* have found something     */
      {
        sprintf(text, "%d Orders Match The Search", flag);
        eh_post(LOCAL_MSG, text);
#ifdef HAWK
        sort_and_total();
#endif
        break;
      }
      eh_post(ERR_OF_MATCH, 0);
      index = si;
    }
    while(1)
    {
      done = 0;
      t = sd_input(&fld[6],sd_prompt(&fld[6], 0), 0,buf[6],0);

      switch(sd_early(t, code_to_caps(*buf[6])))  /* F041897 */
      {
        case(0): leave();
            
        case(4): print_all();
                 *temp_file = 0;          /* mark file as deleted F073196    */
                 leave();

        case(5): done = 1;
                 break;

        case(6): eh_post(ERR_YN,0);
                 break;
      }
      if(done) break;
    }
    sd_cursor(0, 6, 1);
    sd_clear_rest();

    sd_cursor(0,6,2);
    sd_text("Pickline Zone Priority Group   Order Lines  Units  \
Status     Order Reference");
    
    sd_cursor(0, 7, 1);
    fp = fopen(temp_file, "r");
    show(fp,14,2);

    while(1)
    {
      gototop = 0;

      sd_cursor(0,23,28);
      sd_text("(Exit, Forward, or Backward)");
      t = sd_input(&fld[7],sd_prompt(&fld[7], 0), 0,buf[7],0);

      switch(sd_print(t, code_to_caps(*buf[7])))  /* F041897 */
      {
        case(0): leave();
   
        case(1): sd_cursor(0,13,1);
                 sd_clear_rest();
                 sd_cursor(0,7,1);
                 show(fp,14,1);           /*display next 14 lines            */
                 continue;

        case(2): sd_cursor(0,13,1);
                 sd_clear_rest();
                 sd_cursor(0,7,1);
                 show(fp,14,2);           /*display previous 14 lines        */
                 continue;
 
        case(3): gototop = 1;
                 break;
               
        case(4): fclose(fp); 
                 fp = 0;                 /* F073196 */
                 print_all();
                 *temp_file = 0;         /* F073196 */
                 leave();
    
        case(6): eh_post(ERR_YN,0);
                 continue;
      }
      if(gototop) break;
    }
    fclose(fp); fp = 0;                  /* delete tmp file F073196          */
    unlink(temp_file);
    *temp_file = 0;
  }
}
/*------------------------------------------------------------------------*
 * store the data in a temporary file
 *------------------------------------------------------------------------*/
store_data()
{
  struct oc_item *z;
  long found;

  tmp_name(temp_file);                    /* get a temporary file name       */
  fp = fopen(temp_file, "w+");

  if(fp == 0)
  {
    krash("store_data", "tmp file", 1);
    return;
  }
  found = 0;

  if (pickline == 0 && SUPER_OP)          /* all picklines                  */
  {
    for (pickline = 1; pickline <= coh->co_pl_cnt; pickline++)
    {
      z = &oc->oc_tab[pickline - 1];
      if (z->oc_hold.oc_first)
      {
        found += check_queue(z->oc_hold.oc_first, 'h', 0);
      }
      if (z->oc_comp.oc_first)
      {
        found += check_queue(z->oc_comp.oc_first, 'x', 0);
        found += check_queue(z->oc_comp.oc_first, 'c', 0);
      }
      if (z->oc_uw.oc_first)
      {
        found += check_queue(z->oc_uw.oc_first, 'u', 0);
      }
      if (z->oc_high.oc_first)
      {
        found += check_queue(z->oc_high.oc_first, 'q', 'h');
      }
      if (z->oc_med.oc_first)
      {
        found += check_queue(z->oc_med.oc_first, 'q', 'm');
      }
      if (z->oc_low.oc_first)
      {
        found += check_queue(z->oc_low.oc_first, 'q', 'l');
      }
    }
  }
  else
  {
    z = &oc->oc_tab[pickline - 1];
    if (z->oc_hold.oc_first)
    {
      found += check_queue(z->oc_hold.oc_first, 'h', 0);
    }
    if (z->oc_comp.oc_first)
    {
      found += check_queue(z->oc_comp.oc_first, 'x', 0);
      found += check_queue(z->oc_comp.oc_first, 'c', 0);
    }
    if (z->oc_uw.oc_first)
    {
      found += check_queue(z->oc_uw.oc_first, 'u', 0);
    }
    if (z->oc_high.oc_first)
    {
      found += check_queue(z->oc_high.oc_first, 'q', 'h');
    }
    if (z->oc_med.oc_first)
    {
      found += check_queue(z->oc_med.oc_first, 'q', 'm');
    }
    if (z->oc_low.oc_first)
    {
      found += check_queue(z->oc_low.oc_first, 'q', 'l');
    }
  }
  fclose(fp);
  fp = 0;
  return found;
}
/*------------------------------------------------------------------------*
 * check the specified queue and store data if no err
 *------------------------------------------------------------------------*/
check_queue(block, list, p)
register long block;
register char list, p;
{
  register struct oi_item *o;
  char stat[16], znx[4], work[12], orphan;
  register long k, found, err;
  
  if (!block) return 0;
  if (status && status != list) return 0;
  if (list == 'q' && priority && priority != p) return 0;

  found = 0;

  begin_work();
  
  order = check_on(pickline, reference);
  
  oc_lock();

  while (block)
  {
    o = &oc->oi_tab[block - 1];
    block = o->oi_flink;

    if (o->oi_flags & ORPHANS) orphan = '*';
    else orphan = 0x20;

    if (list == 'h')      strcpy(stat, "Held");
    else if (list == 'c') strcpy(stat, "Completed");
    else if (list == 'x') strcpy(stat, "Cancelled");
    else if (list == 'u') strcpy(stat, "Underway");
    else                  strcpy(stat, "Queued");

    if (order && order != o->oi_on) continue;
    
    if (*group)
    {
      if (memcmp(o->oi_grp, group, GroupLength) != 0) continue;
    }
    strncpy(znx, "   ", 3);

    if (list == 'c' && zn) continue;
    if (list == 'x' && zn) continue;

    if (list != 'c' && list != 'x')
    {
#ifdef DEBUG
  fprintf(stderr, "pl=%d  on=%d  entry=%d   exit=%d\n", 
    o->oi_pl, o->oi_on, o->oi_entry_zone, o->oi_exit_zone);
  fflush(stderr);
#endif
      k = o->oi_entry_zone;
      if (list != 'u') k = zone[k - 1].zt_start_section;
      sprintf(znx, "%3d", k);
      if (zn && zn != k) continue;
    }
    of_rec->of_pl = o->oi_pl;
    of_rec->of_on = o->oi_on;
    if (order_read(of_rec, NOLOCK)) continue;

    if (list == 'c' && of_rec->of_status != 'c') continue;
    if (list == 'x' && of_rec->of_status != 'x') continue;

    if (priority && of_rec->of_pri != priority) continue;
             
/*         1         2         3         4         5         6         7     
 *12345678901234567890123456789012345678901234567890123456789012345678901234567
 * Pickline Zone Priority Group   Order Lines  Units  Status     Order Referenc
 *....xx....xxx.....x.....xxxx..xxxxxxx..xxxx...xxxx..xxxxxxxxxx.xxxxxxxxxxxxxx
 */
    if (sp->sp_pl_by_name == 'y' && pl[of_rec->of_pl - 1].pl_pl)
    {
      fprintf(fp,
        "%c%-8.8s %3.3s     %c     %-6.*s%7.*d  %4d   %4d  %-10s %-15.15s \n",
        orphan, pl[of_rec->of_pl - 1].pl_name, znx,
        of_rec->of_pri, GroupLength, of_rec->of_grp, rf->rf_on,
        of_rec->of_on, of_rec->of_no_picks, of_rec->of_no_units,
        stat, of_rec->of_con);
    }
    else
    {
      fprintf(fp,
     " %c  %2d    %3.3s     %c     %-6.*s%7.*d  %4d   %4d  %-10s %-15.15s \n",
        orphan, of_rec->of_pl, znx,
        of_rec->of_pri, GroupLength, of_rec->of_grp, rf->rf_on,
        of_rec->of_on, of_rec->of_no_picks, of_rec->of_no_units,
        stat, of_rec->of_con);
    }
    found += 1;
    commit_work();
    begin_work();
  }
  commit_work();
  oc_unlock(); 
  return found;
}
/*------------------------------------------------------------------------*
 *  Check Order Number
 *------------------------------------------------------------------------*/
check_on(pickline, buf)
register long pickline;
register char *buf;
{
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
  register long ret;

  strip_space(con, CustomerNoLength);     /* remove any spaces               */

  if (!*con) return 0;                    /* nothing entered                 */

  order_setkey(2);                        /* pickline + con                  */
  
  of_rec->of_pl = pickline;
  memcpy(of_rec->of_con, con, CustomerNoLength);
  space_fill(of_rec->of_con, CustomerNoLength);  /* field is space filled    */
  
  if (!order_read(of_rec, NOLOCK)) ret = of_rec->of_on;
  else ret = -1;
  
  order_setkey(1);

  return ret;
}
/*------------------------------------------------------------------------* 
 * print the data
 *------------------------------------------------------------------------*/
print_all()
{
  long pid, stats;
  
  if(fork() == 0)                         /*child process                    */
  {
    ss_close();
    co_close();
    oc_close();
    od_close();

    execlp("prft", "prft", temp_file, tmp_name(print_file),
    "sys/report/order_display_rpt.h", 0);
    krash("print_all", "prft load", 0);
    exit(1);
  }
  pid = wait(&stats);
  if (pid < 0 || stats) krash("print_all", "prft failed", 1);
  return;
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
    sd_clear_rest();
    sd_text(str);
  }
  else if (i == 1)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;

    fseek(fp, pos, 0); 
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
  }
}
/*-------------------------------------------------------------------------*
 *  Sort and Total Within Group
 *-------------------------------------------------------------------------*/
#ifdef HAWK
sort_and_total()
{
  FILE *wd;
  char work_file[16], igroup[4], ibuf[80], command[80];
  long pid, stats, count, lines, units, tlines, tunits;

  tmp_name(work_file);

  sprintf(command("sort +0.23 -0.39  -o %s %s", work_file, temp_file);
  system(command);

  fp = fopen(temp_file, "w");
  wd = fopen(work_file, "r");

  memset(igroup, 0, 4);
  count = tlines = tunits = 0;
  
  while (fread(ibuf, 80, 1, wd) == 1)
  {
    if (memcmp(ibuf + 24, igroup, 4) != 0)   /* break on group               */
    {
      if (*igroup)
      {
        if (count > 1)
        {
          fprintf(fp, "%38c-----  -----%29c\n", 0x20, 0x20);
          fprintf(fp, "%32c%5d%6d%7d%29c\n", 0x20, count,tlines,tunits, 0x20);
        }
        fprintf(fp, "%c%78c\n", 0x0c, 0x20);
      }
      memcpy(igroup, ibuf + 24, 4);
      count = tlines = tunits = 0;
    }
    sscanf(ibuf + 39, "%d", &lines); tlines += lines;
    sscanf(ibuf + 46, "%d", &units); tunits += units;
    fwrite(ibuf, 80, 1, fp);
    count++;
  }
  if (*igroup && count > 1)
  {
    fprintf(fp, "%38c-----  -----%29c\n", 0x20, 0x20);
    fprintf(fp, "%32c%5d%6d%7d%29c\n", 0x20, count, tlines, tunits, 0x20);
  }
  fclose(fp);
  fclose(wd);
  unlink(work_file);

  return;
}
#endif
/*------------------------------------------------------------------------*
 * open all files
 *------------------------------------------------------------------------*/
leave()
{
  close_all();
  sd_close();
  execlp("operm", "operm", 0);
  krash("leave", "operm load", 1);
}
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  co_open();
  oc_open();
  od_open();
  getparms(0);
}
/*------------------------------------------------------------------------*
 *  close all_files
 *------------------------------------------------------------------------*/
close_all()
{
  if (fp) fclose(fp);
  if (*temp_file) unlink(temp_file);
  ss_close();
  co_close();
  oc_close();
  od_close();
  database_close();
}
/*------------------------------------------------------------------------*
 * transfer control back to calling program
 *------------------------------------------------------------------------*/

/* end of order_display.c */
