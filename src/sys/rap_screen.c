/*-----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Receipts, adjustments, and physical inventory.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/01/93   |   tjt  Added to dmc
 *  05/26/95   |   tjt  Added access by either sku, pm, or stkloc.
 *  07/22/95   |   tjt  Revise Bard calls.
 *-------------------------------------------------------------------------*/
static char rap_screen_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      rap_screen.c    receipts, adjustments, physical inventory       */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "file_names.h"
#include "receipts_screen.t"
#include "adjustments_screen.t"
#include "phys_inv_screen.t"
#include "global_types.h"

#include "Bard.h"
#include "bard/pmfile.h"
#include "bard/prodfile.h"
#include "bard/restock_notice.h"

extern leave();

FILE *rfd;

pmfile_item pm;
prodfile_item pf;

#define NUM_PROMPTS     4
#define BUF_SIZE        16

short LSKU;
short LMOD;
short ONE = 1;
short FIVE = 5;
short SIX = 6;
short rm  = 0;

struct fld_parms code_fld = {8,52,13,1,&ONE, "Enter Code",'a'};

struct fld_parms fld[] = {

  { 9,52,13,1,&LSKU,"Enter SKU",'a'},
  {10,52,13,1,&SIX, "Enter CAPS Stock Location",'a'},
  {11,52,13,1,&LMOD,"Enter PM Number",'n'},
  {12,52,13,1,&FIVE,"Enter Quantity",'n'}

};

unsigned char t;
char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */
char text[80];

main(argc,argv)
int argc;
char **argv;
{
  putenv("_=rap_screen");
  chdir(getenv("HOME"));

  open_all();

  LSKU = rf->rf_sku;
  LMOD = rf->rf_mod;

  if (strcmp("receipts",argv[1]) == 0)          receipts();
  else if (strcmp("adjustments",argv[1]) == 0)  adjustments();
  else if (strcmp("phys_inv",argv[1]) == 0)     phys_inv();
  leave();
}
/*-------------------------------------------------------------------------*
 *
 * receipts()
 *
 *  this routine adds the entered quantity to the indicated Pick
 *  Module record in the database.
 *-------------------------------------------------------------------------*/

receipts()                               /* function processes receipts input*/
{

  long quan;                              /* current quantity                */
  long save_quan;
  short cases;                            /* # of cases equiv to quan        */
  short eaches;                           /* items over case quantity        */
  long i;
  
  fix(receipts_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(receipts_screen);
  sd_screen_on();
  
  for(i = 0; i < NUM_PROMPTS; i++) sd_prompt(&fld[i],0);

  while(1)                                /* begin massive loop              */
  {
    begin_work();
    get_fields();

    quan = atol(buf[3]);

    save_quan = pm.p_qty;

    pm.p_qty     += quan;                 /* update quantities               */
    pm.p_curecpt += quan;
    pm.p_cmrecpt += quan;

    pm.p_rsflag  = 'n';                   /* clear restock flag              */

    if (pf.p_cpack > 0)
    {
      cases  = quan/ pf.p_cpack;          /* compute                         */
      eaches = quan - cases * pf.p_cpack;
      sprintf(text, "Received %d Cases + %d Items", cases, eaches);
    }
    sprintf(text, "Received %d Items", quan);

    sd_cursor(0, 14, 1);
    sd_clear_line();

    sd_cursor(0,14, 10);
    sd_text(text);

    pm.p_rsflag  = 'n';                   /* clear restock flag              */
    check_restock_notice();
      
    pmfile_update(&pm);

    sprintf(text, "Old Balance: %d   Quantity: %d   New Balance: %d",
      save_quan, quan, pm.p_qty);
    
    sd_cursor(0, 15, 10);
    sd_clear_line();
    sd_cursor(0, 15, 10);
    sd_text(text);

    eh_post(ERR_CONFIRM, "Receipt");

    sprintf(text, "Module %d received %d", pm.p_pmodno, quan);
    log_entry(text);

    commit_work();
  }
}
/*-------------------------------------------------------------------------*
 *
 *  adjustments()
 *
 *  this routine adjusts the quantity on hand in the indicated Pick 
 *  Module record in the database.
 *--------------------------------------------------------------------------*/

adjustments()
{
  long quan;                              /* current quantity                */
  long save_quan;
  long i;
  char code[2];
  
  fix(adjustments_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(adjustments_screen);
  sd_screen_on();
  
  for(i = 0; i < NUM_PROMPTS; i++) sd_prompt(&fld[i],0);

  memset(code, 0, 2);

  while(1)                                /* begin massive loop              */
  {
    while(1)
    {
      t = sd_input(&code_fld, sd_prompt(&code_fld), 0, code, 0);

      if (t == EXIT) leave();

      *code = tolower(*code);
      
      if (*code == 'd' || *code == 'i') break;
      
    }
    begin_work();
    get_fields();
    
    quan = atol(buf[3]);

    save_quan = pm.p_qty;
    
    if(*code == 'i')                      /* increase                        */
    {
      pm.p_qty += quan;
  
      sprintf(text, "Module %d adjustment increase %d",
       pm.p_pmodno, quan);
      log_entry(text);
    }
    else                                  /* decrease                        */
    {
      pm.p_qty -= quan;

      sprintf(text, "Module %d adjustment decrease %d",
       pm.p_pmodno, quan);
      log_entry(text);

    }
    pm.p_rsflag  = 'n';                   /* clear restock flag              */
    check_restock_notice();
      
    pmfile_update(&pm);
    commit_work();
    
    sprintf(text, "Old Balance: %d   Quantity: %d   New Balance: %d",
      save_quan, quan, pm.p_qty);
    
    sd_cursor(0, 15, 10);
    sd_clear_line();
    sd_cursor(0, 15, 10);
    sd_text(text);
    
    if(pm.p_qty < 0)
    {
      eh_post(ERR_CONFIRM,"Adj below zero");
    }
    else eh_post(ERR_CONFIRM, "Adjustment");
  }
}
/*-------------------------------------------------------------------------*
 *
 *   phys_inv():
 *
 *  this routine replaces the quantity on hand in the indicated
 *  Pick Module record with the entered quantity.
 *------------------------------------------------------------------------*/

phys_inv()
{
  long i, quan, save_quan;

  fix(phys_inv_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(phys_inv_screen);
  sd_screen_on();
  
  for(i = 0; i < NUM_PROMPTS; i++) sd_prompt(&fld[i],0);

  while(1)                                /* begin massive loop              */
  {
    begin_work();
    get_fields();

    quan = atol(buf[3]);

    save_quan = pm.p_qty;

    pm.p_qty  = quan;                     /* update quantity                 */

    pm.p_rsflag  = 'n';                   /* clear restock flag              */
    check_restock_notice();
    pmfile_update(&pm);

    sprintf(text, "Module %d inventory %d",
       pm.p_pmodno, quan);
    log_entry(text);
    commit_work();

    sprintf(text, "Old Balance: %d   Quantity: %d   New Balance: %d",
      save_quan, quan, pm.p_qty);
    
    sd_cursor(0, 15, 10);
    sd_clear_line();
    sd_cursor(0, 15, 10);
    sd_text(text);

    eh_post(ERR_CONFIRM, "Inventory");
  }                                       /* end massive while(1)loop        */
}
/*-------------------------------------------------------------------------*
 *  Get Input Fields
 *-------------------------------------------------------------------------*/
get_fields()
{
  register long i;
  
  memset(buf, 0, NUM_PROMPTS * BUF_SIZE);

  i = 0;                                  /* restore index                   */

  while(1)
  {
    t = sd_input(&fld[i],0, 0, buf[i],0);

    if (t == EXIT) {commit_work(); leave();}

    else if(t == UP_CURSOR && i > 0)
    {
      if(i == 3)                          /* last line                       */
      {
        if(*buf[2]) i = 2;                /* PM entered                      */
        else i = 1;                       /* to CAPS line                    */
      }
      else if(i == 2)                     /* PM entered?                     */
      {
        if(*buf[2]) i = 0;
        else i = 1;
      }
      else i = 0;                         /* i = 1                           */
    }
    else if(t == DOWN_CURSOR || t == TAB)
    {
      if(i == 0)                          /* SKU line                        */
      {
        if(*buf[1] || !(*buf[2]))         /* CAPS                            */
        i = 1;                            /* to CAPS                         */
        else i = 2;                       /* to PM                           */
      }
      else if(i == 1)                     /* CAPS line                       */
      {
        if(*buf[1])                       /* CAPS                            */
        i = 3;                            /* to quan                         */
        else
        i = 2;                            /* to PM                           */
      }
      else if(i == 2)
      i = 3;
      else                                /* i = 3                           */
      i = 0;
    }
    else if (t == RETURN)
    {
      strip_space(buf[0], SkuLength);
      strip_space(buf[1],6);
      
      if(!*buf[0] && !*buf[1] && !*buf[2])        /* no data                 */
      {
        eh_post(ERR_CAPS_PM,0);
        i = 1;
        continue;
      }
      if(*buf[1])                         /* stock location                  */
      {
        pmfile_setkey(3);

        strcpy(pm.p_stkloc, buf[1]);
        space_fill(pm.p_stkloc, 6);
          
        if (pmfile_read(&pm, LOCK))
        {
          eh_post(ERR_CAPS_INV,0);
          i = 1;
          continue;
        }
      }
      else if (*buf[2])                   /* PM entered - CAPS not entered   */
      {
        pmfile_setkey(1);
          
        pm.p_pmodno = atol(buf[2]);

        if (pmfile_read(&pm, LOCK))
        {
          eh_post(ERR_PM_INV,0);
          i = 2;
          continue;
        }
      }
      else                                 /* use SKU                        */
      {
        pmfile_setkey(2);
          
        memcpy(pm.p_pmsku, buf[0], SkuLength);
        space_fill(pm.p_pmsku, SkuLength);
        
        if (pmfile_read(&pm, LOCK))
        {
          eh_post(ERR_SKU_INV, buf[0]);
          i = 1;
          continue;
        }
      }
      if (!*buf[0]) memcpy(buf[0], pm.p_pmsku, SkuLength);

      if(!(*buf[0]))
      {
        eh_post(ERR_SKU_INV, "");
        i = 0;
        continue;
      }
      strcpy(pf.p_pfsku, buf[0]);
      space_fill(pf.p_pfsku, SkuLength);

      if (prodfile_read(&pf, NOLOCK))
      {
        eh_post(ERR_CAPS_SKU, buf[0]);
        i = 1;
        continue;
      }
      if (memcmp(pm.p_pmsku, pf.p_pfsku, SkuLength) != 0)
      {
        if (*buf[2]) {eh_post(ERR_PM_SKU, 0); i = 2;}
        else {eh_post(ERR_CAPS_SKU, 0); i = 1;}
        continue;
      }
                                                /* good CAPS - SKU combo */
      if (atol(buf[3]) <= 0)              /* received qty                    */
      {
        eh_post(ERR_QTY_INV,0);
        i = 3;                            /* qty field                       */
        continue;
      }
      return 0;
    }                                     /* end if RETURN                   */
  }                                       /* end while                       */
}
/*-------------------------------------------------------------------------*
 *  Check Restock Notice
 *-------------------------------------------------------------------------*/
check_restock_notice()
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;

  restock_notice_item rs;
  long r_number, pickline, cases, qty;
  
  if (sp->sp_restock_notice != 'y') return 0; /* feature not selected        */
  
  if (pm.p_qty >= pm.p_restock) return 0; /* not below restock               */
  if (pm.p_rsflag == 'y')       return 0; /* notice already printed          */
  if (pm.p_rqty <= 0)           return 0; /* no restock quantity             */
  
  pickline = 0;
  
  if (pm.p_pmodno <= coh->co_products)
  {
    i = &pw[pm.p_pmodno - 1];
    h = &hw[i->pw_ptr - 1];
    if (h->hw_bay > 0)
    {
      b = &bay[h->hw_bay - 1];
      if (b->bay_zone > 0)
      {
        pickline = zone[b->bay_zone - 1].zt_pl;
      }
    }
  }
  memset(&rs, 0, sizeof(restock_notice_item));

  fseek(rfd, 0, 0);
  fread(&r_number, 4, 1, rfd);
  rs.r_rs_number = r_number;
  r_number++;
  if (r_number > 99999) r_number = 1;
  fseek(rfd, 0, 0);
  fwrite(&r_number, 4, 1, rfd);
  
  rs.r_rs_ref      = ++sp->sp_rs_count;
  rs.r_rs_time     = time(0);
  rs.r_rs_pl       = pickline;
  rs.r_rs_mod      = pm.p_pmodno;
  rs.r_rs_quantity = pm.p_restock - pm.p_qty;
  restock_write(&rs);

  pm.p_rsflag = 'y';

  if (sp->sp_to_flag != 'n')
  {
    qty = pm.p_lcap - pm.p_qty;
    if (i->pw_case > 0) cases = qty / i->pw_case;
    else cases = 0;

    xt_build(" ", " ", 0, pickline, 'K',
      pm.p_pmsku, pm.p_pmodno, pm.p_stkloc, qty, cases, 0,0);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
open_all()
{
  rfd = fopen(restock_no_name, "r+");
  if (rfd == 0) krash("open_all", "open restock numbers", 1);
  
  database_open();
  sd_open(leave);
  ss_open();
  co_open();
  prodfile_open(AUTOLOCK);
  prodfile_setkey(1);
  pmfile_open(AUTOLOCK);
  log_open(AUTOLOCK);
  restock_open(AUTOLOCK);
  return 0;
}

close_all()
{
  if (rfd) fclose(rfd);
  ss_close();
  co_close();
  pmfile_close();
  prodfile_close();
  restock_close();
  log_close();
  sd_close();
  database_close();
  return 0;
}

leave()
{
  close_all();
  execlp("pfead_inventory", "pfead_inventory", 0);
  krash("main", "pfead_inventory load", 1);
}

/* end of rap_screen.c */
