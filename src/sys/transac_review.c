/*-------------------------------------------------------------------------*
 *  Custom Code:    SONOMA - scanned order number from tmp.order_scan.x
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Tranaction review.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/14/93   |  tjt Added to mfc
 *  05/04/94   |  tjt Add 6 digit order number
 *  07/28/96   |  tjt Add 7 digit order number.
 *  07/28/94   |  tjt Fix bug in change_xt.
 *  01/23/95   |  tjt Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt Add pickline input by name.
 *  07/22/95   |  tjt Revise Bard calls.
 *  08/23/96   |  tjt Add begin and commit work.
 *  04/18/97   |  tjt Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char transac_review_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "transac_review.t"
#include "ss.h"
#include "eh_nos.h"
#include "co.h"
#include "st.h"
#include "xt.h"
#include "of.h"
#include "getparms.h"
#include "language.h"
#include "Bard.h"

extern leave();

#define NUM_PROMPTS     6
#define BUF_SIZE        16

#define WIDTH           80

struct trans_item xt;

/*
 * Global Variables
 */
short ONE = 1;
short LPL = 8;
short LORDER = 5;
short LSKU = 15;
short LQUAN = 4;

char sku_prompt[40];

struct fld_parms fld[] = {
  {6,45,20,1,&LPL,   "Enter Pickline",'a'},
  {7,45,20,1,&LORDER,"Enter Order Number",'a'},
  {20,35,2,1,&ONE,   "Change Quantity Picked? (y/n)",'a'},
  {21,35,2,1,&LSKU,   sku_prompt,'a'},
  {22,35,2,1,&LQUAN, "Enter Quantity Picked",'n'},
  {23,17,2,1,&ONE,   "More? (y/n)",'a'}
};
char heading4[] = 
"  Date Time     Grp  Reference         Order PL C SKU/SL   Module Quan Pick\
  Zn";
char heading6[] =
"  Date Time     Group  Reference       Order PL C SKU/SL   Module Quan Pick\
  Zn";
  
short ret,rm,i,k,n,j,gototop,err,index0,si,pickline,found, shorts, picks;
long order, block, savefp;
char ord[6],temp_file[16],temp1[5];
char sku_mod1[16], quan1[5], quan2[5];
char *p;
FILE *fp;
unsigned char t;
char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */

struct st_item *s;

/* MAIN PROGRAM */
main()
{
  putenv("_=transac_review");
  chdir(getenv("HOME"));

  open_all();

  LORDER = rf->rf_on;                     /* order number length             */

  if (sp->sp_use_con == 'y' || sp->sp_use_con == 'b')
  {
    if (rf->rf_con > LORDER) LORDER = rf->rf_con + 1;
  }
  if (sp->sp_use_stkloc == 'y') 
  {
    LSKU = rf->rf_stkloc;                 /* stkloc length                   */
    strcpy(sku_prompt, "Enter Location");
  }
  else if (rf->rf_sku) 
  {
    LSKU = rf->rf_sku;                    /* sku number                      */
    strcpy(sku_prompt, "Enter SKU");
  }
  else 
  {
    LSKU = rf->rf_mod;                    /* module length                   */
    strcpy(sku_prompt, "Enter Module");
  }
  LQUAN = rf->rf_quan;                    /* quantity length                 */

  fix(transac_review);
  sd_screen_off();
  sd_clear_screen();
  sd_text(transac_review);
  sd_screen_on();
  
  while(1)
  {
    sd_cursor(0,6,1);
    sd_clear_rest();

    memset(buf, 0, NUM_PROMPTS * BUF_SIZE);

    if((SUPER_OP) && (!(IS_ONE_PICKLINE))) rm = 0;
    else rm = 1;

    index0 = si = rm;
    
    for(i = rm; i < 2; i++) sd_prompt(&fld[i], 0);

    while(1)
    {
      t = sd_input(&fld[index0], 0, 0, buf[index0], 0);

      sd_cursor(0,8,1);                   /*clear the bottom part            */
      sd_clear_rest();

      if(t == EXIT) leave();

      if (t == DOWN_CURSOR || t == TAB)
      {
        if(index0 == 0) index0 = 1;
        continue;
      }
      else if (t == UP_CURSOR)
      {
        if (index0 == 1 && si == 0) index0 = 0;
        continue;
      }
      if (t == F_KEY_5)
      {
        if (block > 0)
        {
          block = oc->oi_tab[block - 1].oi_flink;
          if (block > 0)
          {
            order = oc->oi_tab[block - 1].oi_on;
          }
        }
        sprintf(buf[1], "%d", order);
        sprintf(buf[0], "%d", pickline);
      }
      if (t == F_KEY_6)
      {
        if (block > 0)
        {
          block = oc->oi_tab[block - 1].oi_blink;
          if (block > 0)
          {
            order = oc->oi_tab[block - 1].oi_on;
          }
        }
        sprintf(buf[1], "%d", order);
        sprintf(buf[0], "%d", pickline);
      }
      if(IS_ONE_PICKLINE) pickline = op_pl;
      else
      {
        if(SUPER_OP)
        {
          if(!*buf[0])
          {
            eh_post(ERR_PL,buf[0]);
            index0 = 0;
            continue;
          }
          if(sp->sp_config_status == 'y') check_pkln();
          else pickline = atol(buf[0]);

          if(err)                         /*invalid pickline was entered     */
          {
            index0 = 0;
            continue;
          }
        }
        else pickline = op_pl;
      }                       
#ifdef SONOMA
  
      if (!(*buf[1]))
      {
        FILE *td;
        char td_name[32];
    
        sprintf(td_name, "tmp/order_scan.%d", pickline);
        td = fopen(td_name, "r");
        if (td > 0)
        {
          fread(buf[1], rf->rf_con + 1, 1, td);
          fclose(td);
        }
      }
#endif
      order = check_on(pickline, buf[1]);

      err = 0;
  
      block = oc_find(pickline, order);

      if (block)                           /* order was found                */
      {
        od_read(block);                    /* get order without lock         */
        sd_cursor(0, 6, 1);
        sd_clear_line();
        sd_cursor(0, 6, 15);
        sd_text("Order Number ");
        sd_text(buf[1]);
        sd_text("     Status ");
        switch(of_rec->of_status)
        {
          case 'c': sd_text("Completed");
                    break;

          case 'h': sd_text("Held");
                    break;

          case 'q': sd_text("Queued");
                    break;

          case 'u': sd_text("Underway");
                    break;
        }
        found = shorts = picks = 0;

        if (of_rec->of_status == 'c' ||
            of_rec->of_status == 'x' ||
            of_rec->of_status == 'u')
        {
          check_xtfile_all();
   /*     check_xtfile();  */
        }
        else
        {
          for(i = rm; i < 2; i++) memset(buf[i], 0, BUF_SIZE);
          continue;
        }
        if (found) break;
        else
        {
          sd_cursor(0, 8, 28);
          sd_text("No Transactions");
          index0 = si;
          for(i = rm;i < 2;i++)
          for(n = 0;n < BUF_SIZE;n++) buf[i][n] = 0;
          for (i = rm; i < 2; i++)
          sd_prompt(&fld[i], 0);
          continue;
        }
      }
      else
      {
        eh_post(ERR_ORDER,buf[1]);
        index0 = si;
        continue;
      }
    }                                      /* end while - has found data     */
    sd_cursor(0, 7, 1); sd_clear_line();

    sd_cursor(0, 8, 1);
    if (rf->rf_grp > 4) sd_text(heading6);
    else                sd_text(heading4);

    sd_cursor(0, 9, 1);
    fseek(fp, 0, 0);
    show(fp, 10, 2);

    while(1)                                /* display and change loop       */
    {
      sd_cursor(0, 23, 35);
      sd_text("(Exit, Forward, or Backward)");

      gototop = 0;
      sd_prompt(&fld[5], 0);
      t = sd_input(&fld[5], 0, 0, buf[5], 0);
             
      if (t == F_KEY_7)
      {
        get_change();
        continue;
      }
      switch(sd_more(t,code_to_caps(*buf[5])))  /* F041897 */
      {
        case(0): leave();

        case(1): sd_cursor(0, 9, 1);
                 sd_clear_rest();
                 sd_cursor(0, 9, 1);
                 show(fp, 10, 2);
                 break;

        case(2): sd_cursor(0, 9, 1);
                 sd_clear_rest();
                 sd_cursor(0, 9, 1);
                 show(fp, 10, 1);
                 break;

        case(3): gototop = 1;
                 break;

        case(6): eh_post(ERR_YN,0);
                 break;
      }
      if (gototop) break;
    }                                      /* end of display loop            */
  }
}
/*-------------------------------------------------------------------------*
 *  Get Any Chnage
 *--------------------------------------------------------------------------*/
get_change()
{
  while (1)
  {
    sd_cursor(0, 23, 1); sd_clear_line();

    sd_prompt(&fld[3], 0);
    sd_prompt(&fld[4], 0);
      
    memset(buf[3], 0, BUF_SIZE);
    memset(buf[4], 0, BUF_SIZE);

    while (1)                              /* get responses to changes       */
    {
      t = sd_input(&fld[3], 0, 0, buf[3], 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;
      if (t == F_KEY_7) break;
      
      while (1)
      {
        t = sd_input(&fld[4], 0, 0, buf[4], 0);
        if (t == EXIT) leave();
        if (t == UP_CURSOR) break;
        if (t == RETURN) break;
        if (t == F_KEY_7) break;
      }
      if (t == UP_CURSOR) continue;
      if (t == F_KEY_7) break;
      
      if (!change_xt()) break;
    }
    sd_cursor(0, 21, 1); sd_clear_line();
    sd_cursor(0, 22, 1); sd_clear_line();
    return;
  }
}

/*-------------------------------------------------------------------------*
 *  Check Order Number
 *-------------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------------*
 *  Check Customer Order Number
 *-------------------------------------------------------------------------*/
check_con(pickline, con)
register long pickline;
register char *con;
{
  strip_space(con, CustomerNoLength);     /* remove any spaces               */

  if (!*con) return 0;                    /* nothing entered                 */

  order_setkey(2);                        /* pickline + con                  */
  
  of_rec->of_pl = pickline;
  memcpy(of_rec->of_con, con, CustomerNoLength);
  space_fill(of_rec->of_con, CustomerNoLength);  /* field is space filled    */
  
  if (!order_read(of_rec, NOLOCK)) return of_rec->of_on;

  return 0;
}

/*-------------------------------------------------------------------------*
 *  check the pickline requested
 *-------------------------------------------------------------------------*/
check_pkln()
{
  char work[4];

  err = 0;
  pickline = pl_lookup(buf[0], op_pl);

  if(pickline <= 0)
  {
    eh_post(ERR_PL,buf[0]);
    err = 1;
    i = 0;
  }
  else 
  {
    sprintf(work, "%d", pickline);
    chng_pkln(work);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  check transaction file for shorts
 *-------------------------------------------------------------------------*/
check_xtfile()
{
  char work[8];

  tmp_name(temp_file);                    /* get a temporary file name       */
  fp = fopen(temp_file, "w+");

  if(fp == 0) krash("check_xtfile", "tmp file");

  sprintf(work, "%07d", order);           /* F072894 */
  memcpy(xt.xt_on, work, 7);
  
  sprintf(work, "%02d", pickline);
  memcpy(xt.xt_pl, work, 2);
  
  xt.xt_ref = 0;
  
  transaction_startkey(&xt);
  xt.xt_ref = 0x7fffffff;
  transaction_stopkey(&xt);
  
  begin_work();
  
  while (!transaction_next(&xt, NOLOCK))
  {
    if(xt.xt_code != 'S') continue;

    found = 1;

    strncpy(sku_mod1, xt.xt_sku_mod1, 15);
    sku_mod1[15] = 0;

    if (!rf->rf_sku)
    {
      j = 10;
      while( sku_mod1[j] == '0')
      {
        sku_mod1[j] = SPACE;
        j++;
      }
    }
    strncpy(quan1, xt.xt_quan1, 4);
    quan1[4] = 0;
    j = 0;
    while(quan1[j] == '0' && j < 3)
    {
      quan1[j] = SPACE;
      j++;
    }
    strncpy(quan2, xt.xt_quan2, 4);
    quan2[4] = 0;
    j = 0;
    while(quan2[j] == '0' && j < 3)
    {
      quan2[j] = SPACE;
      j++;
    }
    fprintf(fp,"     %s%14c%s%16c%s%21c\n",
      sku_mod1, ' ', quan1, ' ', quan2, ' ');
  }
  commit_work();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  check transaction file for all tranactions
 *-------------------------------------------------------------------------*/
check_xtfile_all()
{
  char work[8];

  tmp_name(temp_file);                    /* get a temporary file name       */
  fp = fopen(temp_file, "w+");

  if(fp == 0) krash("check_xtfile", "tmp file");

  sprintf(work, "%07d", order);           /* F072894 */
  memcpy(xt.xt_on, work, 7);
  
  sprintf(work, "%02d", pickline); 
  memcpy(xt.xt_pl, work, 2);
  
  xt.xt_ref = 0;
  
  transaction_startkey(&xt);
  xt.xt_ref = 0x7fffffff;
  transaction_stopkey(&xt);

  while(!transaction_next(&xt, NOLOCK))
  {
    found = 1;

    if (xt.xt_code == 'S') shorts = 1;
    if (xt.xt_code == 'P') picks  = 1;
    
    if (!rf->rf_sku) 
    {
      for (j = 0; j < 15; j++)
      {
        if (xt.xt_sku_mod1[j] != '0') break;
        xt.xt_sku_mod1[j] = 0x20;
      }
    }
    if (xt.xt_pl[0] == '0') xt.xt_pl[0] = 0x20;

    for (j = 0; j < 4; j++)
    {
      if (xt.xt_quan1[j] != '0') break;
      xt.xt_quan1[j] = 0x20;
    }
    for (j = 0; j < 4; j++)
    {
      if (xt.xt_quan2[j] != '0') break;
      xt.xt_quan2[j] = 0x20;
    }
    for (j = 0; j < 3; j++)
    {
      if (xt.xt_zone[j] != '0') break;
      xt.xt_zone[j] = 0x20;
    }
/*
         1         2         3         4        5          6         7
1234567890123456789012345678901234567890123456789012345678901234567890123456789
  Date Time     Grp  Reference         Order PL C SKU      Module Quan Pick  Zn
xxxxxxxxxxxxxxx xxxx xxxxxxxxxxxxxxx xxxxxxx xx x xxxxxxxxxxxxxxx xxxx xxxx xxx
123456789012345 1234 123456789012345 1234567 12 1 123456789012345 1234 1234 123
*/    
    if (rf->rf_grp <= 4)
    {
      fprintf(fp, 
     "%15.15s %4.4s %15.15s %7.7s %2.2s %c %15.15s %4.4s %4.4s %3.3s\n",
      (char *)ctime(&xt.xt_time) + 4, xt.xt_group, xt.xt_con, xt.xt_on,
      xt.xt_pl, xt.xt_code, xt.xt_sku_mod1, xt.xt_quan1, xt.xt_quan2,
      xt.xt_zone);
    }
/*
         1         2         3         4        5          6         7
1234567890123456789012345678901234567890123456789012345678901234567890123456789
  Date Time     Group  Reference       Order PL C SKU      Module Quan Pick  Zn
xxxxxxxxxxxxxxx xxxxxx xxxxxxxxxxxxx xxxxxxx xx x xxxxxxxxxxxxxxx xxxx xxxx xxx
123456789012345 123456 1234567890123 1234567 12 1 123456789012345 1234 1234 123
*/    
    else
    {
      fprintf(fp, 
     "%15.15s %6.6s %13.13s %7.7s %2.2s %c %15.15s %4.4s %4.4s %3.3s\n",
      (char *)ctime(&xt.xt_time) + 4, xt.xt_group, xt.xt_con, xt.xt_on,
      xt.xt_pl, xt.xt_code, xt.xt_sku_mod1, xt.xt_quan1, xt.xt_quan2,
      xt.xt_zone);
    }
  }                                         /* F072894 */
}
/*-------------------------------------------------------------------------*
 * change the contents of xt file if no error
 *-------------------------------------------------------------------------*/
change_xt()
{
  long where;
  char work[20];

  if (!*buf[4])
  {
    eh_post(ERR_QTY_INV, 0);
    err = 1;
    i = 4;
    return err;
  }
  sprintf(work, "%07d", order);             /* F072894 */
  memcpy(xt.xt_on, work, 7);
  
  sprintf(work, "%02d", pickline);
  memcpy(xt.xt_pl, work, 2);
  
  transaction_startkey(&xt);

  if (!rf->rf_sku)
  {
    n = atol(buf[3]);
    sprintf(work, "          %05d", n);
  }    
  else sprintf(work, "%-15.15s", buf[3]);  /* F072894   */
  
  where = err = 0;

  begin_work();
  while (!transaction_next(&xt, LOCK))
  {
    where = (where + 1) % 10;

    if (xt.xt_code != 'S' && xt.xt_code != 'P') continue;

    if (memcmp(xt.xt_sku_mod1, work, 15) != 0)  continue;

    memcpy(temp1, xt.xt_quan1, 4);
    temp1[4] = 0;
    n = atoi(temp1);
    j = atoi(buf[4]);
    if (j > n)
    {
      eh_post(ERR_XR_PICK,0);           /*picked is more than ordered      */
      err = 1;
      i = 4;
      return err;
    }
    sprintf(work, "%04d", j);
    memcpy(xt.xt_quan2, work, 4);       /*change the quantity picked       */
    transaction_update(&xt);
    
    sprintf(work, "%4d", j);
    sd_cursor(0, where + 8, 72);
    sd_text(work);                      /* update screen                   */
         
    if (sp->sp_use_stkloc == 'y')
    {
      s = stkloc_lookup(buf[3]);
      n = s->st_mod;
    }
    else if (rf->rf_sku > 0)            /* get module from sku             */
    {
      s = sku_lookup(pickline, buf[3]);
      n = s->st_mod;
    }
    else n = atoi(buf[3]);              /* module number                   */

    pick_setkey(1);

    op_rec->pi_pl  = pickline;
    op_rec->pi_on  = order;
    op_rec->pi_mod = n;

    if (!pick_read(op_rec, LOCK))
    {
      op_rec->pi_picked = atol(buf[4]);
      pick_replace(op_rec);
    }
    commit_work();
    eh_post(ERR_CONFIRM,"Quantity change");
    return 0;
  }
  commit_work();
  eh_post(ERR_PICK, buf[3]);
  err = 1;
  i = 3;
  return err;
}
/*-------------------------------------------------------------------------*
 * function to display x number of lines of data on the screen               
 * Arguments:                                                               
 *           fp : the data file pointer.                                    
 *           lines : the number of lines to be displayed.                   
 *           i : the indicator of either going forward or                   
 *           backward on the file.                                          
 *                                                                          
 */
show(fp,lines,m)
FILE *fp;
short lines,m;
{
  register long pos, size;
  char str[1920];

  memset(str, 0, 1920);
  
  pos = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);

  if(m == 1)
  {
    pos = savefp - lines * WIDTH;
    if(pos < 0)  pos = 0;
    savefp = pos;

    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
  }
  else if(m == 2)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;

    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
  }
  return 0;
}
/*
 * open all files
 */
open_all()
{
  database_open();
  sd_open(leave);
  sd_tab(7, "7 CHANGE");
  ss_open();
  oc_open();
  od_open();
  co_open();
  xt_open();

  transaction_setkey(2);
  getparms(0);
}
/*
 * transfer control back to the calling program
 */
close_all()
{
  if (fp)
  {
    fclose(fp);
    unlink(temp_file);
  }
  sd_tab(7, "        ");
  ss_close();
  oc_close();
  od_close();
  co_close();
  xt_close();
  sd_close();
  database_close();
}
leave()
{
  close_all();
  execlp("operm", "operm", 0);
  krash("transac_review", "load operm", 1);
}

/* end of transac_review.c */
