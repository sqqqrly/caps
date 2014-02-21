#define DATETIME
/*-------------------------------------------------------------------------*
 *  Custom Versions: CANTON   - shows modslot
 *                   UPDATE   - allow change of order quantity.
 *                   DATETIME - show datetime of last action.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Display order picks.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/28/93   |  tjt Added mfc.
 *  05/13/94   |  tjt Fix order number 1 .. 6 digits left justified. 
 *  05/25/94   |  tjt Add ignore pick text flag.
 *  06/07/94   |  tjt Fix bug in B_startkey for pick retrieval.
 *  07/27/94   |  tjt Fix order number 1 .. 7 digits left justified. 
 *  01/23/95   |  tjt Add new IS_ONE_PICKLINE.
 *  01/24/95   |  tjt Add display of lot number + pick text.
 *  06/03/95   |  tjt Add pickline input by name.
 *  06/29/95   |  tjt Add customer number as parm.
 *  07/18/95   |  tjt Add change ordered quantity.
 *  07/21/95   |  tjt Revise Bard calls.
 *  09/08/95   |  tjt Fix canceled display.
 *  09/22/95   |  tjt Fix up arrow on get_parms returned bad block number.
 *  03/08/96   |  tjt Add datetime of last action option.
 *  07/26/96   |  tjt Add adjust remaining picks on change quantity.
 *  07/26/96   |  tjt Fix change quantity not allowed when underway.
 *  08/23/96   |  tjt Add begin and commit work.
 *  12/02/96   |  tjt Revise location to go.
 *  04/18/97   |  tjt Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char order_picks_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global_types.h"
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "order_picks.t"
#include "eh_nos.h"
#include "co.h"
#include "of.h"
#include "st.h"
#include "language.h"
#include "Bard.h"
#include "bard/pmfile.h"

pmfile_item pm;

#define WIDTH 80

short LPL = 8;                            /* max pickline length             */
short ONE = 1;
short LORDER = 5;
short LSKU = 15;
short LMOD = 5;
short LSL  = 6;
short LQUAN = 4;

struct fld_parms fld1 = { 6,27,1,1,  &LPL,    "Enter Pickline",'a'};
struct fld_parms fld2 = { 7,27,1,1,  &LORDER, "Enter Order Number",'a'};
struct fld_parms fld3 = {23,22,2,1,  &ONE,    "Print? (y/n)",'a'};
struct fld_parms fld4 = {23,28,2,1,  &ONE,    "Change Quantity? (y/n)", 'a'};
struct fld_parms fld5 = {23,22,2,1,  &LSKU,   "Enter SKU", 'a'};
struct fld_parms fld6 = {23,22,2,1,  &LMOD,   "Enter Module", 'a'};
struct fld_parms fld7 = {23,22,2,1,  &LSL,    "Enter Location", 'a'};
struct fld_parms fld8 = {23,62,40,1, &LQUAN,  "Enter Quantity", 'n'};

char buf1[9] = {0};
char buf2[CustomerNoLength + 1] = {0};
char buf3[2] = {0};
char buf4[2] = {0};
char buf5[SkuLength + 1] = {0};
char buf6[QuantityLength + 1];

FILE *fp = 0;
long delete = 1;                          /* delete work file                */
char temp_file[16],print_file[16];        /* temporary file name             */

long savefp = 0;

main(argc,argv)
short argc;
char **argv;
{
  extern leave();
  extern unsigned short get_parms();
  long block;
  
  putenv("_=order_picks");
  chdir(getenv("HOME"));
  
  block = atol(argv[2]);

  open_all();
  
  tmp_name(temp_file);                    /* get a temporary file name       */
  
  LORDER = rf->rf_on;                     /* order number length             */
  if (sp->sp_use_con == 'y' || sp->sp_use_con == 'b')
  {
    if (rf->rf_con >= LORDER) LORDER = rf->rf_con + 1;
  }
  LQUAN = rf->rf_quan;

  LSKU = rf->rf_sku;
  LMOD = rf->rf_mod;
  LSL  - rf->rf_stkloc;
   
  fix(order_picks);
  sd_screen_off();
  sd_clear_screen();
  sd_text(order_picks);
  sd_screen_on();

  while(1)
  {
    begin_work();
    get_picks(block);
    commit_work();
    show_picks(block);
    
#ifdef UPDATE
    begin_work();
    if (change_ordered(block)) 
    {
      commit_work(); 
      continue;
    }
    commit_work();
#endif
    block = get_parms();
    savefp = 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  if(fp)
  {
    fclose(fp);
    if (delete) unlink(temp_file);
  }
  close_all();
  database_close();
  sd_close();
  execlp("order_stat", "order_stat", 0);
  krash("leave", "load order_stat", 1);
}
/*-------------------------------------------------------------------------*
 *  Get Pickline and Order Number
 *-------------------------------------------------------------------------*/
unsigned short get_parms()
{
  long pickline, order;
  long block;
  unsigned char t;
  char work[8];
  sd_cursor(0, 6, 1);
  sd_clear_rest();

  memset(buf1, 0, sizeof(buf1));          /* clear pickline                  */
  memset(buf2, 0, sizeof(buf2));          /* clear order number              */

  sd_prompt(&fld2, 0);                    /* prompt for order                */
  
  while(1)
  {
    if (IS_ONE_PICKLINE) pickline = op_pl;
    else if (!SUPER_OP)  pickline = op_pl;
    else
    {
      sd_prompt(&fld1, 0);

      while(1)
      {
        t = sd_input(&fld1,0, 0, buf1, 0);
        if(t == EXIT) leave();

        pickline = pl_lookup(buf1, op_pl);

        if (pickline <= 0)
        {
          eh_post(ERR_PL, buf1);
          continue;
        }
        sprintf(work, "%d", pickline);
        chng_pkln(work);             /* change pickline on the screen   */
        break;
      }                                   /* end while(1)pickline            */
    }                                     /* end SUPER OP                    */
    t = sd_input(&fld2, 0, 0, buf2, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) continue;
    if (t == RETURN)
    {
      if (*buf2 == '#') 
      {
        if (sp->sp_use_con == 'n') block = check_con(pickline, buf2 + 1);
        else                       block = oc_find(pickline, atol(buf2 + 1));
      }
      else
      {
        if (sp->sp_use_con == 'n') block = oc_find(pickline, atol(buf2));
        else                       block = check_con(pickline, buf2);
      }
      if (block) return block;

      eh_post(ERR_ORDER, buf2);
    }                                     /* end while(1)order number        */
  }                                       /* end while(1)parms               */
}
/*-------------------------------------------------------------------------*
 *  Check Customer Order Number
 *-------------------------------------------------------------------------*/
check_con(pickline, con)
register long pickline;
register char *con;
{
  register long block;

  strip_space(con, CustomerNoLength);     /* move any spaces                 */

  if (!*con) return 0;                    /* nothing entered                 */

  order_setkey(2);                        /* pickline + con                  */
  
  of_rec->of_pl = pickline;
  memcpy(of_rec->of_con, con, CustomerNoLength);
  space_fill(of_rec->of_con, CustomerNoLength);  /* field is space filled    */
  
  block = 0;
  
  begin_work();
  if (!order_read(of_rec, NOLOCK))
  {
    block = oc_find(of_rec->of_pl, of_rec->of_on);
  }
  commit_work();
  return block;
}

/*-------------------------------------------------------------------------*
 *  Get Pick Information
 *-------------------------------------------------------------------------*/
get_picks(block)
register long block;
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k;
  
  if (fp) fclose(fp);

  fp = fopen(temp_file, "w+");

  if(fp == 0) krash("get_picks", "tmp file", 1);
  
  order_setkey(1);

  od_read(block);                          /* read order header - nolock     */
  
  op_rec->pi_pl  = of_rec->of_pl;
  op_rec->pi_on  = of_rec->of_on;
  op_rec->pi_mod = 0;
  pick_startkey(op_rec);

  op_rec->pi_mod = ProductMax;
  pick_stopkey(op_rec);

  while (!pick_next(op_rec, NOLOCK))
  {
    k = op_rec->pi_mod;

    if (k >= 1 && k <= coh->co_prod_cnt)
    {
      i = &pw[k - 1];
      h = &hw[i->pw_ptr - 1];

      if (h->hw_bay)
      {
        b = &bay[h->hw_bay - 1];
        if (b->bay_zone)
        {
          z = &zone[b->bay_zone - 1];
          if (z->zt_pl == op_rec->pi_pl)
          {
            print(k, z->zt_zone);
            continue;
          }
        }
      }
    }
    print(k, 0);
  }
  print(-1, -1);                          /* close last print line           */
}
/*-------------------------------------------------------------------------*
 *  Show Picks On Screen
 *-------------------------------------------------------------------------*/
show_picks(block)
register long block;
{
  char pkln_name[12], pkln_num[4], order_num[8], status[10];
  char lines[16], units[16], priority[2];
  char group[GroupLength + 1], con[CustomerNoLength];
  unsigned char t;
  long i, pid, stat;
  
  memset(group, 0, sizeof(group));
  memset(con, 0, sizeof(con));
  
  sd_cursor(0, 6, 1);
  sd_clear_rest();

  if (pl[of_rec->of_pl - 1].pl_pl)
  {
    strncpy(pkln_name, pl[of_rec->of_pl - 1].pl_name, 8);
    pkln_name[8] = 0;
  }
  else  strcpy(pkln_name,"Pickline");

  sprintf(pkln_num, " %d", of_rec->of_pl);
  sprintf(order_num, "%0*d", rf->rf_on, of_rec->of_on);
  sprintf(priority, "%c", of_rec->of_pri);
  
  switch(of_rec->of_status)
  {
    case 'q': strcpy(status, "Queued  "); break;
    case 'h': strcpy(status, "Hold    "); break;
    case 'c': strcpy(status, "Complete"); break;
    case 'x': strcpy(status, "Canceled"); break;
    case 'u': strcpy(status, "Underway"); break;
    default:  strcpy(status, "Unknown "); break;
  }
 /*
  *          1         2         3         4         5         6         7
  * 123456789012345678901234567890123456789012345678901234567890123456789012345
  * xxxxxxxxxx Order Number: xxxxxxx  Status: xxxxxxxx Priority: x Group: xxxxx
  *            Reference:    xxxxxxxxxxxxxxx           Lines: xx   Units: xx
  */
  
  sd_cursor(0, 6,  1); sd_text(pkln_name);
  sd_cursor(1, 0,  1); sd_text(pkln_num);
  sd_cursor(0, 6, 12); sd_text("Order Number: "); sd_text(order_num);
  sd_cursor(0, 6, 35); sd_text("Status: ");       sd_text(status);
  sd_cursor(0, 6, 52); sd_text("Priority: ");     sd_text(priority);
  
  if (rf->rf_grp > 0)
  {
    sd_cursor(0, 6, 64); 
    sd_text("Group: ");        
    sd_text_2(of_rec->of_grp, rf->rf_grp);
    memset(group, of_rec->of_grp, rf->rf_grp);
  }
  if (rf->rf_con > 0)
  {
    sd_cursor(0, 7, 12); 
    sd_text("Reference:    "); 
    sd_text_2(of_rec->of_con, rf->rf_con);
    memcpy(con, of_rec->of_con, rf->rf_con);
  }
#ifdef DATETIME
  sd_cursor(0, 8, 12);
  sd_text_2(ctime(&of_rec->of_datetime), 24);
#endif

  if (oc->oi_tab[block - 1].oi_flags & ORPHANS)
  {
    sd_cursor(0, 7, 43);
    sd_text("*Orphans");
  }
  sprintf(lines, "Lines: %d", of_rec->of_no_picks);
  sd_cursor(0, 7, 52);
  sd_text(lines);
  
  sprintf(units, "Units: %d", of_rec->of_no_units);
  sd_cursor(0, 7, 64);
  sd_text(units);

  sd_cursor(0, 10, 1); sd_text("Zone");

  if (sp->sp_use_stkloc == 'y')
  {
    sd_cursor(1, 0, 1);
    for(i = 0; i < 3; i++)
    {
      sd_cursor(0, 10, 6 + 25 * i); sd_text("Stock Location  Qty Shrt");
    }
  }
#ifdef CANTON
  else if (rf->rf_sku)                    /* if sku support                  */
  {
    sd_cursor(1, 0, 1);
    for(i = 0; i < 3; i++)
    {
      sd_cursor(0, 10, 6 + 25 * i); sd_text("ModSlt      Sku      Qty");
    }
  }
#else
  else if (rf->rf_sku)                   /* if sku support                  */
  {
    sd_cursor(1, 0, 1);
    for(i = 0; i < 3; i++)
    {
      sd_cursor(0, 10, 6 + 25 * i); sd_text("Sku             Qty Shrt");
    }
  }
#endif
  else                                    /* not sku support                 */
  {
    for(i = 0; i < 3; i++)
    {
      sd_cursor(0,10, 15 + 25 * i); sd_text("Module Qty Shrt");
    }
  }
  sd_cursor(0,11,1);
  fseek(fp, savefp, 0);
  show(fp,10,1);                          /* display data                    */

  while(1)
  {
    sd_cursor(0,23,30);
    sd_text("(Exit, Forward, or Backward)");

    t = sd_input(&fld3, sd_prompt(&fld3,0), 0, buf3, 0);
    switch(sd_print(t, code_to_caps(*buf3)))  /* F041897 */
    {
      case(0) : leave();
                break;

      case(1) : sd_cursor(0,11,1);
      show(fp,10,1);                      /* show the next set of data       */
      break;

      case(2) : sd_cursor(0,11,1);
      show(fp,10,2);                      /* show the previous set of data   */
      break;

      case(3) : return;
      break;

      case(4) :                           /*   Print the data and leave      */

      if(fork() == 0)                     /* if child pprocess               */
      {
        fclose(fp);
#ifdef CANTON
        if(rf->rf_sku)                    /* if sku support                  */
        {
          close_all();
          execlp("prft", "prft", temp_file,
            tmp_name(print_file),
            "sys/report/order_picks_sku2.h",pkln_name, pkln_num,
            order_num, status, priority, group, lines, units, 0);
          krash("show_picks", "prft load", 0);
          exit(1);
        }
#else
        if(rf->rf_sku)                    /* if sku support                  */
        {
          close_all();
          execlp("prft", "prft", temp_file,
            tmp_name(print_file),
            "sys/report/order_picks_sku.h",pkln_name, pkln_num,
            order_num, status, priority, group, con, lines, units, 0);
          krash("show_picks", "prft load", 0);
          exit(1);
        }
#endif
        else                              /* not sku support                 */
        {
          execlp("prft", "prft", temp_file,
            tmp_name(print_file),
            "sys/report/order_picks_pm.h", pkln_name, pkln_num,
            order_num, status, priority, con, group, lines, units, 0);
          krash("show_picks", "prft load", 0);
          exit(1);
        }
      }
      pid = wait(&stat);
      if (pid < 0 || stat) krash("show_picks", "prft failed", 1);
      delete = 0;
      leave();
    
      case(6) : eh_post(ERR_YN,0);
      break;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Change Picks
 *-------------------------------------------------------------------------*/
change_ordered(block)
register long block;
{
  register struct pl_item *p;
  register struct pw_item *i;
  register struct st_item *s;
  struct fld_parms *fld;
  unsigned char t;
  long quan, diff;
  
  if (oc->oi_tab[block - 1].oi_queue = OC_UW) return 0;
  
  sd_cursor(0, 23, 1);
  sd_clear_line();

  sd_prompt(&fld4, 0);
  
  while (1)
  {
    t = sd_input(&fld4, 0, 0, buf4, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return 0;
    
    *buf4 = tolower(*buf4);
    if (*buf4 == 'n') return 0;
    if (*buf4 == 'y') break;
    eh_post(ERR_CODE, buf4);
  }
  op_rec->pi_pl = oc->oi_tab[block - 1].oi_pl;
  op_rec->pi_on = oc->oi_tab[block - 1].oi_on;

  if (sp->sp_use_stkloc == 'y') fld = &fld7;   /* stock location             */
  else if (rf->rf_sku > 0)      fld = &fld5;   /* sku                        */
  else                          fld = &fld6;   /* module                     */
  
  sd_cursor(0, 23, 1);
  sd_clear_line();

  sd_prompt(&fld,  0);
  sd_prompt(&fld8, 0);

  strcpy(buf5, "n");
  
  while (1)
  {
    t = sd_input(&fld, 0, 0, buf5, 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return 0;
    
    if (sp->sp_use_stkloc == 'y')
    {
      s = stkloc_lookup(buf5);

      if (!s)
      {
        eh_post(ERR_CAPS_INV, buf5);
        continue;
      }
      op_rec->pi_mod = s->st_mod;
    }
    else if (rf->rf_sku)
    {
      s = sku_lookup(op_rec->pi_pl, buf5);
      if (!s)
      {
        eh_post(ERR_SKU_INV, buf5);
        continue;
      }
      op_rec->pi_mod = s->st_mod;
    }
    else op_rec->pi_mod = atol(buf5);
    
    if (pick_read(op_rec, NOLOCK))
    {
      eh_post(LOCAL_MSG, "Pick Not Found");
      continue;
    }
    if (op_rec->pi_flags & PICKED)
    {
      eh_post(LOCAL_MSG, "Already Picked");
      continue;
    }
    i = &pw[op_rec->pi_mod - 1];
    p = &pl[op_rec->pi_pl - 1];
    
    while (1)
    {
      t = sd_input(&fld8, 0, 0, buf6, 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;
      
      quan = atol(buf6);
      
      if (quan >= op_rec->pi_ordered)
      {
        eh_post(LOCAL_MSG, "Can only decrease quantity");
        continue;
      }
      diff = op_rec->pi_ordered - quan;
      
      i->pw_units_to_go -= diff;
      p->pl_units_to_go -= diff;
      
      if (!pick_read(op_rec, LOCK))
      {
        op_rec->pi_ordered = quan;
        pick_replace(op_rec);
      }
      return 1;
    }  
  }
}


/*--------------------------------------------------------------------------*
 * Open / Close All Files
 *--------------------------------------------------------------------------*/
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  oc_open();
  od_open();
  co_open();
#ifdef CANTON
  pmfile_open(READONLY);
  pmfile_setkey(2);
#endif
  getparms(0);
  return 0;
}
close_all()
{
  co_close();
  ss_close();
  oc_close();
  od_close();
#ifdef CANTON
  pmfile_close();
#endif
  return 0;
}
/****************************************************************************/
/*function to display x number of lines of data on the screen               */
/* Arguments:                                                               */
/*           fp : the data file pointer.                                    */
/*           lines : the number of lines to be displayed.                   */
/*           i : the indicator of either going forward or                   */
/*           backward on the file.                                          */
/*                                                                          */
/* returns : 1 if successfull                                               */
/*           0 if failed                                                    */
/****************************************************************************/

show(fp, lines, index)
FILE *fp;
short lines,index;
{
  register long pos, size;
  char str[1920];

  memset(str, 0, 1920);

  pos = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);

  if(index == 2)
  {
    pos = savefp - lines * WIDTH;
    if(pos < 0) pos = 0;
    savefp = pos;
    
    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
  }
  else if(index == 1)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;

    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
  }
  else return(0);
}
/*--------------------------------------------------------------------------*
 * print the module in the file
 *--------------------------------------------------------------------------*/
print(module, zne)
long module, zne;
{
  register struct st_item *s;
  register long j, k;
  static long last_zone = 9999;
  static long n = 0;
  static char p_text[3][PickTextLength + 1] = {0};

  if(zne != last_zone || n == 3)
  {
    switch (n)
    {
      case 1: fprintf(fp,"%50c\n", ' ');  break;
      case 2: fprintf(fp,"%25c\n", ' ');  break;
      case 3: fprintf(fp,"\n");         break;
    }
    if (p_text[0][0])
    {
      fprintf(fp, "     %-24.24s %-24.24s %-24.24s\n",
      p_text[0], p_text[1], p_text[2]);

      memset(p_text, 0, sizeof(p_text));
    }
    n = 0; last_zone = 9999;
      
    if (module == -1 && zne == -1) return;/* last call                       */

    if(zne > 0) fprintf(fp,"%3d ", zne);   /* write the zone                 */
    else fprintf(fp,"  ? ");               /* write ?                        */
  }
  if (sp->sp_use_stkloc == 'y' && rf->rf_stkloc > 0)
  {
    s = mod_lookup(op_rec->pi_mod);
    
    if (s) fprintf(fp, " %-15.*s", rf->rf_stkloc, s->st_stkloc);
    else   fprintf(fp, " %-15.15s", op_rec->pi_sku);
    
    if (op_rec->pi_flags & PICKED)
    {
      fprintf(fp, "%4d%c%4d", op_rec->pi_ordered,
        (op_rec->pi_flags & NO_PICK) ? '*' : ' ',
        op_rec->pi_ordered - op_rec->pi_picked);
    }
    else
    {
      fprintf(fp, "%4d%c    ", op_rec->pi_ordered,
        (op_rec->pi_flags & NO_PICK) ? '*' : ' ');
    }
  }
#ifdef CANTON
  else if (rf->rf_sku)
  {
    strcpy(pm.p_pmsku, op_rec->pi_sku, SkuLength);
    space_fill(pm.p_pmsku, SkuLength);
    if (pmfile_read(&pm, NOLOCK)) memset(pm.p_stkloc, 0x20, StklocLength);

    fprintf(fp, " %-6.6s %-13.13s %3d", 
      pm.p_stkloc, op_rec->pi_sku, op_rec->pi_ordered);
  }
#else
  else if (rf->rf_sku)
  {
    fprintf(fp, " %-15.15s", op_rec->pi_sku);

    if (op_rec->pi_flags & PICKED)
    {
      fprintf(fp, "%4d%c%4d", op_rec->pi_ordered,
        (op_rec->pi_flags & NO_PICK) ? '*' : ' ',
        op_rec->pi_ordered - op_rec->pi_picked);
    }
    else
    {
      fprintf(fp, "%4d%c    ", op_rec->pi_ordered,
        (op_rec->pi_flags & NO_PICK) ? '*' : ' ');
    }
  }
#endif
  else
  {
    fprintf(fp,"         %5d  ", op_rec->pi_mod);
  
    if (op_rec->pi_flags & PICKED)
    {
      fprintf(fp, "%4d%c%4d", op_rec->pi_ordered,
        (op_rec->pi_flags & NO_PICK) ? '*' : ' ',
        op_rec->pi_ordered - op_rec->pi_picked);
    }
    else
    {
      fprintf(fp, "%4d%c    ", op_rec->pi_ordered,
        (op_rec->pi_flags & NO_PICK) ? '*' : ' ');
    }
  }
  if (rf->rf_pick_text && rf->rf_ignore_pick_text != 'y')
  {
    memcpy(p_text[n], op_rec->pi_pick_text, rf->rf_pick_text);
  }
  if (sp->sp_lot_control == 'y')
  {
    j = strlen(p_text[n]);
    k = LotLength;
    if (j + k > PickTextLength) k = PickTextLength - j;
    if (k > 0) memcpy(&p_text[n][j], op_rec->pi_lot, k);
  }
  n++;
  last_zone = zne;
}

/* end of order_picks.c */
