/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Product file formatted query report input screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/28/93   |  tjt  Added to mfc.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/03/95   |  tjt  Add pickline input by name.
 *  04/24/96   |  tjt  Extensively rewritten.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char pff_inquiry_input_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*  product file formatted inquiry input  screen 7.4                    */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "getparms.h"
#include "pff_defs.h"
#include "pff_inquiry_input.t"
#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"
#include "language.h"

unsigned char get_parms();
unsigned char get_pickline();
unsigned char get_code();
unsigned char get_range();
unsigned char get_sort();

long leave();

prodfile_item pf;
pmfile_item pm;

#define NUM_PROMPTS  5
#define LAST_PROMPT  4                    /* ignore configuration            */
#define BUF_SIZE     32

char buf[NUM_PROMPTS][BUF_SIZE];
char yn[2];

short ONE   = 1;
short THREE = 3;
short L30   = 30;
short LPL   = 8;
short RL    = 10;

static struct fld_parms fld[] = {

  { 9,40,10,1,&LPL,  "Enter Pickline",'a'},
  {10,40,10,1,&ONE,  "Enter Code",'a'},
  {11,40,10,1,&RL,   "Enter Range",'a'},
  {12,40,10,1,&THREE,"Enter Sorted By",'a'},
  {14,40,10,1,&ONE,  "Print? (y/n)",'a'} 

};
char valid_sort[11][4] = {

    "p",                                  /* PFF_P                           */
    "i",                                  /* PFF_L                           */
    "s",                                  /* PFF_S                           */
    "sp",                                 /* PFF_SP                          */
    "sl",                                 /* PFF_SL                          */
    "f",                                  /* PFF_F                           */
    "fs",                                 /* PFF_FS                          */
    "fp",                                 /* PFF_FP                          */
    "fl",                                 /* PFF_FL                          */
    "fsp",                                /* PFF_FSP                         */
    "fsl",                                /* PFF_FSL                         */

};
char rng_buf[2][16];                      /* key ranges                      */
char sort_buf[4];

main()
{
  unsigned char t;
  
  putenv("_=pff_inquiry_input");
  chdir(getenv("HOME"));
  
  open_all();                             /* open ss, eh, co                 */

  /* initialize and show screen display */

  fix(pff_inquiry_input);
  sd_screen_off();
  sd_clear_screen();                      /* clear entire screen             */
  sd_text(pff_inquiry_input);
  sd_screen_on();
  
  sd_cursor(0, 6, 10);
  sd_text("F = Family Group                       L = CAPS Stock Location");
  sd_cursor(0, 7, 10);
  sd_text("P = Pick Module Number                 S = SKU");

  while(1)
  {
    memset(buf, 0, NUM_PROMPTS * BUF_SIZE); /* clear buffers                 */

    get_parms();                            /* get all parmeters             */

    sd_prompt(&fld[4], 0);
    memset(yn, 0, 2);
    
    while(1)
    {
      t = sd_input(&fld[4], 0, 0, yn, 0);
      if (t == EXIT) leave();
      
      *buf[4] = code_to_caps(*yn);

      if (*buf[4] == 'y')
      {
        sd_wait();
        close_all();
        execlp("pff_inquiry_create", "pff_inquiry_create",
                buf[0], buf[1], sort_buf, rng_buf[0], rng_buf[1],
                "", "print", 0);

        krash("main", "pff_inquiry_create load", 1);
      }
      else if (*buf[4] == 'n')
      {
        sd_wait();
        close_all();
        execlp("pff_inquiry_create","pff_inquiry_create",
                buf[0], buf[1], sort_buf, rng_buf[0], rng_buf[1],
                "", "report", 0);

        krash("main", "pff_inquiry_create load", 1);
      }
      eh_post(ERR_YN,0);
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Gather All Parameters
 *-------------------------------------------------------------------------*/
unsigned char get_parms()
{
  unsigned char t;
  
  if((SUPER_OP) && (!(IS_ONE_PICKLINE)))
  {
    sd_prompt(&fld[0], 0); 
  }
  sd_prompt(&fld[1], 0);
  sd_cursor(0, 11, 1);
  sd_clear(2);
  
  while(1)
  {
    t = get_pickline();
    
    while (1)
    {
      t = get_code();
      if (t == UP_CURSOR) break;
      
      while (1)
      {
        begin_work();
        t = get_range();
        commit_work();
        
        if (t == UP_CURSOR) break;
        
        while (1)
        {
          t = get_sort();
          if (t == UP_CURSOR) break;
          if (t == RETURN) return t;
        }                                  /* end of get_sort                */
      }                                    /* end of get_range               */
    }                                      /* end of get_code                */
  }                                        /* end of get_pickline            */
}                                          /* end of get_parms               */
/*-------------------------------------------------------------------------*
 *  Get Pickline Value
 *-------------------------------------------------------------------------*/
unsigned char get_pickline()
{
  unsigned char t;
  long n;
  
  sprintf(buf[0], "%d", op_pl);
 
  if (!(SUPER_OP) || (IS_ONE_PICKLINE)) return TAB;
  
  while (1)
  {
    t = sd_input(&fld[0], 0, 0, buf[0], 0);
    if (t == EXIT) leave();
    
    n = pl_lookup(buf[0], op_pl);         /* numeric of pickline             */
    if (n > 0)
    {
      sprintf(buf[0], "%d", n);
      chng_pkln(buf[0]);
      return TAB;
    }
    eh_post(ERR_PL, buf[0]);
  }
}
/*-------------------------------------------------------------------------*
 *  Get Action Code
 *-------------------------------------------------------------------------*/
unsigned char get_code()
{
  unsigned char t;
  
  while (1)
  {
    t = sd_input(&fld[1], 0, 0, buf[1], 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return UP_CURSOR;

    *buf[1] = tolower(*buf[1]);           /* lower case                      */

    switch (*buf[1])
    {
      case 'f': RL = 11;
                break;

      case 'l': RL = 2 * rf->rf_stkloc + 1;
                break;

      case 'p': RL = 2 * rf->rf_mod + 1;
                break;

      case 's': RL = 2 * rf->rf_sku + 1;
                break;

      default:  eh_post(ERR_CODE,buf[1]);
                continue;
    }
    sd_prompt(&fld[2], 0);
    sd_prompt(&fld[3], 0);
    return TAB;
  }  
}
/*-------------------------------------------------------------------------*
 *  Get Range
 *-------------------------------------------------------------------------*/
unsigned char get_range()
{
  char t, *p;
  long range_parms;
  
  memset(rng_buf, 0, 32);
  
  while (1)
  {
    t = sd_input(&fld[2], 0, 0, buf[2], 0);
    if (t == EXIT) {commit_work(); leave();}
    if (t == UP_CURSOR) return UP_CURSOR;
    
    if (!*buf[2])                           /* null is all                   */
    {
      strcpy(rng_buf[0], "0");
      strcpy(rng_buf[1], "0");
      return TAB;
    }
    p = (char *)memchr(buf[2], '-', RL);
    if (p)
    {
      memcpy(rng_buf[0], buf[2], p - buf[2]);
      strcpy(rng_buf[1], p + 1);
    }
    else strcpy(rng_buf[0], buf[2]);
    
    switch (*buf[1])                      /* get code                        */
    {
      case 'p':                           /* pick module number              */

                pmfile_setkey(1); 
                pm.p_pmodno = atol(rng_buf[0]);
                if (pmfile_read(&pm, NOLOCK))
                {
                  eh_post(ERR_PFF_ITEM,rng_buf[0]);
                  continue;
                }
                if (*rng_buf[1])
                {
                  pm.p_pmodno = atol(rng_buf[1]);
                  if (pmfile_read(&pm, NOLOCK))
                  {
                    eh_post(ERR_PFF_ITEM,rng_buf[1]);
                    continue;
                  }
                  if (atol(rng_buf[0]) >= atol(rng_buf[1]))
                  {
                    eh_post(ERR_RANGE,0);
                    continue;
                  }
                }
                else strcpy(rng_buf[1], "0");
                return TAB;

      case 's':                        /* SKU                            */

                prodfile_setkey(1);  
                strcpy(pf.p_pfsku, rng_buf[0]);
                space_fill(pf.p_pfsku, SkuLength);
                if (prodfile_read(&pf, NOLOCK))
                {
                  eh_post(ERR_PFF_ITEM,rng_buf[0]);
                  continue;
                }
                if (*rng_buf[1])
                {
                  if (strcmp(rng_buf[0], rng_buf[1]) >= 0)
                  {
                    eh_post(ERR_RANGE,0);
                    continue;
                  }
                  strcpy(pf.p_pfsku, rng_buf[1]);
                  space_fill(pf.p_pfsku, SkuLength);
                  if (prodfile_read(&pf, NOLOCK))
                  {
                    eh_post(ERR_PFF_ITEM,rng_buf[1]);
                    continue;
                  }
                }
                else strcpy(rng_buf[1], "0");
                return TAB;

      case 'l':                           /* CAPS location                   */

                pmfile_setkey(3);
                strcpy(pm.p_stkloc, rng_buf[0]);
                space_fill(pm.p_stkloc, StklocLength);
                if (pmfile_read(&pm, NOLOCK))
                {
                  eh_post(ERR_PFF_ITEM,rng_buf[0]);
                  continue;
                }
                if (*rng_buf[1])
                {
                  if (strcmp(rng_buf[0], rng_buf[1]) >= 0)
                  {
                    eh_post(ERR_RANGE,0);
                    continue;
                  }
                  strcpy(pm.p_stkloc, rng_buf[0]);
                  space_fill(pm.p_stkloc, StklocLength);
                  if (pmfile_read(&pm, NOLOCK))
                  {
                    eh_post(ERR_PFF_ITEM,rng_buf[1]);
                    continue;
                  }
                }
               else strcpy(rng_buf[1], "0");
                 return TAB;

     case 'f':                             /* Family Group                   */

                prodfile_setkey(2);
                strcpy(pf.p_fgroup, rng_buf[0]);
                space_fill(pf.p_fgroup, 5);
                if (prodfile_read(&pf, NOLOCK))
                {
                  eh_post(ERR_PFF_ITEM,rng_buf[0]);
                  continue;
                }
                if (*rng_buf[1])
                {
                  if (strcmp(rng_buf[0], rng_buf[1]) >= 0)
                  {
                    eh_post(ERR_RANGE,0);
                    continue;
                  }
                  strcpy(pf.p_fgroup, rng_buf[0]);
                  space_fill(pf.p_fgroup, 5);
                  if (prodfile_read(&pf, NOLOCK))
                  {
                    eh_post(ERR_PFF_ITEM,rng_buf[1]);
                    continue;
                  }
                }
                else strcpy(rng_buf[1], "0");
                return TAB;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Get Sort By
 *-------------------------------------------------------------------------*/
unsigned char get_sort()
{
  register char t;
  long sort_mod;
  
  while (1)
  {
    t = sd_input(&fld[3], 0, 0, buf[3], 0);
    if (t == EXIT) leave();
    if (t == UP_CURSOR) return UP_CURSOR;

    if (!*buf[3])
    {
      switch (*buf[1])
      {
        case 's': sort_mod = PFF_S;
                  break;

        case 'f': sort_mod = PFF_F;
                  break;

        case 'l': sort_mod = PFF_L;
                  break;

        case 'p': sort_mod = PFF_P;
                  break;
      }
      sprintf(sort_buf, "%d", sort_mod);
      return t;
    }
    else
    {
      buf[3][0] = tolower(buf[3][0]);       /* lower case                    */
      buf[3][1] = tolower(buf[3][1]);       /* lower case                    */
      buf[3][2] = tolower(buf[3][2]);       /* lower case                    */
      
      for (sort_mod = 0; sort_mod < 11; sort_mod++)
      {
        if (strcmp(buf[3], valid_sort[sort_mod]) == 0)
        {
          sprintf(sort_buf, "%d", sort_mod);
          return t;
        }
      }
      eh_post(ERR_PFF_SRT,0);
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  co_open();
  getparms(0);
  pmfile_open(READONLY);
  prodfile_open(READONLY);
}
/*-------------------------------------------------------------------------*
 *  Close All Files
 *-------------------------------------------------------------------------*/
close_all()
{ 
  co_close();
  ss_close();
  pmfile_close();
  prodfile_close();
  sd_close();
  database_close();
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
long leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 0);
}

/* end of pff_inquiry_input.c */
