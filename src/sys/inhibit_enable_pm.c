#define DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Inhibit and Enable Module Picking
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/24/93   |  tjt Added to mfc.
 *  07/26/94   |  shf Fix mod prompt line.
 *  11/15/94   |  tjt Fix More prompt.
 *  11/15/94   |  tjt Fix display bug.
 *  06/03/95   |  tjt Add pickline input by name.
 *  10/02/95   |  tjt Bug in display
 *  10/02/95   |  tjt Add PM2 and PM4.
 *  04/23/96   |  tjt Add stkloc field.
 *  04/03/97   |  tjt Add sleep to wait for ofc to do action.
 *  04/18/97   |  tjt Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char inhibit_enable_pm_c[] = "%Z% %M% %I% (%G% - %U%)";

/****************************************************************************/
/*                                                                          */
/*                      inhibit/enable pick module                          */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>
#include "global_types.h"
#include "getparms.h"
#include "message_types.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "language.h"
#include "inhibit_enable_pm.t"
#include "co.h"
#include "eh_nos.h"
#include "st.h"

extern catcher();
extern leave();

#define NUM_PROMPTS     8
#define BUF_SIZE        20

#define BASE    10
#define LINES   24                    /* number of lines to display on screen*/
#define WIDTH   80

long savefp=0;                            /* for use by show subroutine      */

short ONE = 1;
short LPL = 9;
short LSKU = 0;
short LMOD = 0;
short LSL  = 0;
static struct fld_parms fld[] = {

  {6,55,19,1,&LPL,  "Enter Pickline",'a'},
  {7,55,19,1,&ONE,  "Display Inhibited Modules? (y/n)",'a'},
  {6,39,19,1,&LSKU, "Enter Sku",'a'},
  {7,39,19,1,&LMOD, "Enter Module",'n'},  
  {8,39,19,1,&ONE,  "Inhibit? (y/n)",'a'},
  {23,16,1,1,&ONE,  "More? (y/n)",'a'},
  {8,55,19,1,&ONE,  "Enable All? Are You Sure? (y/n)",'a'},
  {6,39,19,1,&LSL,  "Enter Stkloc",'a'},
  
};
FILE *fp;

char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */
char yn[2];

char temp_file[16];                       /* temporary file name             */
TModule pm_nums[3];                       /* for display                     */
short lines = 0;                          /* # of lines to display           */

long num;                                 /* pickline number                 */
char sku[16];                             /* SKU number                      */
TModule PM_num;                           /* pick module number              */

main()
{
  register long k;

  putenv("_=inhibit_enable_pm");
  chdir(getenv("HOME"));

  ss_open();
  co_open();
  getparms(0);
  sd_open(catcher);

  LSKU = rf->rf_sku;                      /* sku number length               */
  LMOD = rf->rf_mod;                      /* pick module length              */
  LSL  = rf->rf_stkloc;                   /* stkloc length                   */
  
  fix(inhibit_enable_pm);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(inhibit_enable_pm);
  sd_screen_on();
  
  memset(buf, 0, NUM_PROMPTS * BUF_SIZE);          /* clear input buffers */

  while(1)
  {
    sd_cursor(0, 5, 1);
    sd_clear_rest();

    get_pickline();            
    
    while (1)
    {
      if (check_display() == UP_CURSOR) break;

      while (1)
      {
        if (get_parms() == UP_CURSOR) break;
        if (*buf[1] == 'y') display_inhibited();
      }
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Get Pickline Value
 *-------------------------------------------------------------------------*/
get_pickline()
{
  register unsigned char t;

  if (IS_ONE_PICKLINE || !SUPER_OP) 
  {
    num = op_pl;
    return 0;
  }
  sd_prompt(&fld[0], 0);
  memset(buf[0], 0, BUF_SIZE);
  
  while(1)                                /* input pickline number           */
  {
    t = sd_input(&fld[0], 0, 0, buf[0], 0);
    if(t == EXIT) leave();

    num = pl_lookup(buf[0], 0);

    if (!num) return 0;

    if (num < 0)
    {
      eh_post(ERR_PL,buf[0]);             /* out of range                    */
      continue;
    }
    sprintf(buf[0], "%d", num);
    chng_pkln(buf[0]);                /* record new pickline             */
    return 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Get SKU/Module Parameters
 *-------------------------------------------------------------------------*/
get_parms()
{
  sd_cursor(0, 6, 1); sd_clear_line();
  sd_cursor(0, 7, 1); sd_clear_line();
  sd_cursor(0, 8, 1); sd_clear_line();

  if (sp->sp_use_stkloc == 'y' && rf->rf_stkloc > 0)
  {
    if (get_stkloc() == UP_CURSOR) return UP_CURSOR;
    if (!PM_num)
    {
      if (get_module() == UP_CURSOR) return UP_CURSOR;
    }
  }
  else if (rf->rf_sku > 0)
  {
    if (get_sku() == UP_CURSOR) return UP_CURSOR;
    if (!PM_num)
    {
      if (get_module() == UP_CURSOR) return UP_CURSOR;
    }
  }
  else if (get_module() == UP_CURSOR) return UP_CURSOR;

  if (get_action() == UP_CURSOR) return UP_CURSOR;

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get SKU
 *-------------------------------------------------------------------------*/
get_sku()
{
  register unsigned char t;
  register struct st_item *p;

  sd_prompt(&fld[2],0);

  PM_num = 0;
  memset(buf[2], 0, BUF_SIZE);
  
  while(1)
  {
    t = sd_input(&fld[2], 0, 0, buf[2], 0);
    if(t == EXIT) leave();
    if(t == UP_CURSOR) return UP_CURSOR;

    if (!buf[2][0]) return 0;

    p = sku_lookup(num > 0 ? num : op_pl, buf[2]);
    if (!p)
    {
      eh_post(ERR_SKU_INV, buf[2]);
      continue;
    }
    PM_num = p->st_mod;
    return 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Get Stock Location
 *-------------------------------------------------------------------------*/
get_stkloc()
{
  register unsigned char t;
  register struct st_item *p;
  long pickline;
  char what[4];
  
  sd_prompt(&fld[7],0);

  PM_num = 0;
  memset(buf[7], 0, BUF_SIZE);
  
  while(1)
  {
    t = sd_input(&fld[7], 0, 0, buf[7], 0);
    if(t == EXIT) leave();
    if(t == UP_CURSOR) return UP_CURSOR;

    if (!buf[7][0]) return 0;

    p = stkloc_lookup(buf[7]);
    if (!p)
    {
      eh_post(ERR_SKU_INV, buf[7]);
      continue;
    }
    pickline = num > 0 ? num : op_pl;
    
    if (p->st_pl != pickline)
    {
      sprintf(what, "%d", pickline);
      eh_post(ERR_PL, what);
      continue;
    }
    PM_num = p->st_mod;
    return 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Get Module Number
 *-------------------------------------------------------------------------*/
get_module()
{
  register unsigned char t;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k;
  
  sd_prompt(&fld[3],0);
  memset(buf[3], 0, BUF_SIZE);
  
  while (1)
  {
    t = sd_input(&fld[3], 0, 0, buf[3], 0);
    if(t == EXIT) leave();
    if(t == UP_CURSOR) return UP_CURSOR;

    PM_num = atol(buf[3]);

    if (!PM_num) return 0;

    if(PM_num > coh->co_prod_cnt || PM_num < 1) 
    {
      eh_post(ERR_PM_INV, 0);
      continue;
    }
    if (!num) return 0;

    i = &pw[PM_num - 1];
    k = i->pw_ptr;

    if (k > 0)
    {
      h = &hw[k - 1];
      if (h->hw_bay)
      {
        b = &bay[h->hw_bay - 1];
        if (b->bay_zone)
        {
          z = &zone[b->bay_zone - 1];
          if (z->zt_pl == num) return 0;
        }
      }
    }
    eh_post(ERR_PM_INV, 0);
  }
}
/*-------------------------------------------------------------------------*
 *  Get Action Code
 *-------------------------------------------------------------------------*/
get_action()
{
  register unsigned char t;
  register struct pw_item *i;
  
  sd_prompt(&fld[4], 0);
  memset(buf[4], 0, BUF_SIZE);
  memset(yn, 0, 2);
  
  while(1)
  {
    t = sd_input(&fld[4], 0, 0, yn, 0);
    if(t == EXIT)       leave();
    if (t == UP_CURSOR) return UP_CURSOR;

    *buf[4] = code_to_caps(*yn);          /* F041897 */

    if(*buf[4] != 'y' && *buf[4] != 'n')
    {
      eh_post(ERR_YN, 0);
      continue;
    }
    if (!PM_num)
    {
      if (*buf[4] == 'y')
      {
        eh_post(LOCAL_MSG, "Cannot Inhibit All");
        continue;
      }
      return enable_all(num);
    }
    i = &pw[PM_num - 1];
        
    if (*buf[4] == 'y')
    {
      if (i->pw_flags & PicksInhibited)
      {
        eh_post(LOCAL_MSG, "Already Inhibited");
        continue;
      }
      message_put(0, ModuleInhibitRequest, &PM_num, sizeof(TModule));
      sleep(1);
      return 0;
    }
    if (!(i->pw_flags & PicksInhibited))
    {
      eh_post(LOCAL_MSG, "Already Enabled");
      continue;
    }
    message_put(0, ModuleEnableRequest, &PM_num, sizeof(TModule));
    sleep(1);
    return 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Check Display Inhibited List
 *-------------------------------------------------------------------------*/
check_display()
{
  register unsigned char t;
  
  sd_cursor(0, 5, 1);                   /* clear bottom of screen         */
  sd_clear_rest();

  sd_prompt(&fld[1], 0);
  memset(buf[1], 0, BUF_SIZE);
  memset(yn, 0, 2);
  
  while(1)                         
  {
    t = sd_input(&fld[1], 0, 0, yn, 0);
    if(t == EXIT) leave();
    if(t == UP_CURSOR) return UP_CURSOR;

    *buf[1] = code_to_caps(*yn);          /* F041897 */

    if (*buf[1] == 'n') return 0;

    if(*buf[1] == 'y') return display_inhibited();

    eh_post(ERR_YN,0);
  }
}
/*-------------------------------------------------------------------------*
 *  Display Inhibited On Screen
 *-------------------------------------------------------------------------*/
display_inhibited()
{
  register unsigned char t;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k, n, done;
  
#ifdef DEBUG
  fprintf(stderr, "display_inhibited()\n");
#endif
  
  if (sp->sp_use_stkloc == 'y')
  {
    sd_cursor(0,BASE, 6);
    sd_text("Stkloc/Module         Stkloc/Module         Stkloc/Module");
  }
  else if (rf->rf_sku == 0)              /*if not sku support                */
  {
    sd_cursor(0, BASE, 25);
    sd_text("Module      Module      Module");
  }
  else                                    /* sku  support                    */
  {
    sd_cursor(0,BASE, 6);
    sd_text("Sku/Module            Sku/Module            Sku/Module");
  }
            /* prepare formatted data in temporary file */
            /* create data file */

  lines = 0;
  tmp_name(temp_file);
  fp = fopen(temp_file, "w");             /* open file to write              */
  n=0;

  for (k = 1, i = pw; k <= coh->co_prod_cnt; k++, i++)
  {
    if (!(i->pw_flags & PicksInhibited)) continue;
          
    h = &hw[i->pw_ptr - 1];
    
    if (num)                              /* check against pickline          */
    {
      if (!h->hw_bay) continue;
      b = &bay[h->hw_bay - 1];
      if (!b->bay_zone) continue;
      z = &zone[b->bay_zone - 1];

      if (z->zt_pl != num) continue;
    }
    if (rf->rf_sku)
    {
      if (!mod_lookup(k)) continue;
    }
    pm_nums[n++] = k;
    if(n == 3)
    {
      if (sp->sp_use_stkloc == 'y') put_stkloc(n);
      else if (rf->rf_sku)          put_skus(n);
      else
      {
        fprintf(fp,"%25c%5d       %5d       %5d%25c\n",
          ' ', pm_nums[0], pm_nums[1], pm_nums[2]);
      }
      n = 0;
      lines++;
    }
  }
  if (n > 0)                              /* either 1 or 2 unprinted modules */
  {
    lines++;
    if (sp->sp_use_stkloc == 'y') put_stkloc(n);
    else if (rf->rf_sku)          put_skus(n);
    else
    {
      fprintf(fp,"%25c%5d", ' ', pm_nums[0]);
      if(--n > 0)                         /* one more module                 */
      fprintf(fp, "       %5d%37c\n",pm_nums[1], ' ');
      else fprintf(fp,"%49c\n", ' ');
    }
  }
  fclose(fp);

            /* display formatted data */

  fp = fopen(temp_file, "r");             /* open file to read               */
  sd_cursor(0, BASE + 1, 1);
  show(fp,LINES,2);                       /* display lines                   */

  if (lines > LINES)                      /* need more prompt                */
  {
    done = 0;

    while(1)
    {
      sd_prompt(&fld[5], 0);
      memset(buf[5], 0, BUF_SIZE);

      t = sd_input(&fld[5], 0, 0, buf[5], 0);

      switch(sd_more(t, code_to_caps(buf[5][0])))  /* F041897 */
      {
        case(0): leave();

        case(1): sd_cursor(0, BASE + 1, 1);
                 show(fp,LINES,1);
                 sd_clear_rest();
                 break;

        case(2): sd_cursor(0, BASE + 1, 1);
                 show(fp,LINES,2);
                 sd_clear_rest();
                 break;

        case(3): done = 1;
                 break;

        case(6): eh_post(ERR_YN,0);
      }
      if (done) break;
    }
  }
  fclose(fp);
#ifndef DEBUG
  unlink(temp_file);
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

show(fp,lines,index)
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
    return(1);
  }
  else if (index == 1)
  {
    if (pos >= size) pos  = savefp;
    savefp = pos;

    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else return(0);
}
/*-------------------------------------------------------------------------*
 *  Output 1-3 Sku/Modules
 *-------------------------------------------------------------------------*/
put_skus(n)
register long n;
{
  register long i;

  fprintf(fp, "     ");
  for (i = 0; i < n; i++) out_sku(pm_nums[i]);
  if (n == 1)      fprintf(fp, "%52c\n", ' ');
  else if (n == 2) fprintf(fp, "%30c\n", ' ');
  else             fprintf(fp, "        \n");
  return 0;
}
out_sku(mod)
long mod;
{
  register struct st_item *p;
  register long i;

  p = mod_lookup(mod);
  if (!p) return;

  for (i = 0; i < rf->rf_sku; i++)
  {
    fprintf(fp, "%c", p->st_sku[i]);
  }
  fprintf(fp, "/%-5d", mod);
   
  for (i=0; i < 16 - rf->rf_sku; i++) fprintf(fp, "%c", 0x20);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Output 1-3 Stkloc/Modules
 *-------------------------------------------------------------------------*/
put_stkloc(n)
register long n;
{
  register long i;

  fprintf(fp, "     ");
  for (i = 0; i < n; i++) out_stkloc(pm_nums[i]);
  if (n == 1)      fprintf(fp, "%52c\n", ' ');
  else if (n == 2) fprintf(fp, "%30c\n", ' ');
  else             fprintf(fp, "        \n");
  return 0;
}
out_stkloc(mod)
long mod;
{
  register struct st_item *p;
  register long i;

  p = mod_lookup(mod);
  if (!p) return;

  for (i = 0; i < rf->rf_stkloc; i++)
  {
    fprintf(fp, "%c", p->st_stkloc[i]);
  }
  fprintf(fp, "/%-5d", mod);
   
  for (i=0; i < 16 - rf->rf_stkloc; i++) fprintf(fp, "%c", 0x20);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Enable All Modules
 *-------------------------------------------------------------------------*/
enable_all(pickline)
register long pickline;
{
  register unsigned char t;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  TModule k;
  
  sd_prompt(&fld[6], 0);
  memset(buf[6], 0, BUF_SIZE);
  memset(yn, 0, 2);
  
  while(1)                         
  {
    t = sd_input(&fld[6], 0, 0, yn, 0);
    if(t == EXIT) leave();
    if(t == UP_CURSOR) return UP_CURSOR;

    *buf[6] = code_to_caps(*yn);          /* F041897 */

    if (*buf[6] == 'n') return 0;
    if (*buf[6] == 'y') break;

    eh_post(ERR_YN,0);
  }
  for (k = 1, i = pw; k <= coh->co_prod_cnt; k++, i++)
  {
    if (!(i->pw_flags & PicksInhibited)) continue;

    h = &hw[i->pw_ptr - 1];
    
    if (pickline)
    {
      if (!h->hw_bay) continue;
      b = &bay[h->hw_bay - 1];
      if (!b->bay_zone) continue;
      z = &zone[b->bay_zone - 1];
      if (z->zt_pl != pickline) continue;
    }
    message_put(0, ModuleEnableRequest, &k, sizeof(TModule));
  }
  sleep(3);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Message Catcher
 *-------------------------------------------------------------------------*/
catcher(who, type, buf, len)
register long who, type, len;
register unsigned char *buf;
{
  buf[len] = 0;

  switch (type)
  {
    case ShutdownEvent: leave(0);
    
    case ClientMessageEvent: eh_post(*buf, buf + 1);

    default: break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  ss_close();
  co_close();
  sd_close();
  execlp("operm", "operm", 0);
  krash("operm", "load");
}

/* end of inhibit_enable_pm.c */
