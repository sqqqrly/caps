/* #define DEBUG  */
#define TANDY
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Add, Assign, Change, Deassign, and Move PM Screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/2/93    |  tjt  Added to mfc.
 *  06/30/94   |  tjt  Fix exit return on assign to add sku.
 *  06/30/94   |  tjt  Add immediate autocasing changes.
 *  07/15/94   |  tjt  Add dup stkloc messages show other pm.
 *  08/29/94   |  tjt  Add check pm maximum value.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  11/13/95   |  tjt  Realtime update.
 *  12/02/96   |  tjt  Add units and lines to location.
 *  12/18/97   |  tjt  Add allocation to pmfile.
 *  05/18/01   |  aha  Added fix to change default value of piflag.
 *-------------------------------------------------------------------------*/
static char acd_pm_screen_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <fcntl.h>
#include "iodefs.h"
#include "eh_nos.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "st.h"
#include "language.h"
#include "add_pm_screen.t"
#include "assign_pm_screen.t"
#include "change_pm_screen.t"
#include "deassign_pm_screen.t"
#include "move_pm_screen.t"
#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"

prodfile_item  sku_rec;
pmfile_item    pkm_rec, work, new;

long have_pm_rec;
long have_move_rec;

short L01  = 1;
short L03  = 3;
short L04  = 4;
short L05  = 5;
short LSL  = 6;
short LSKU = 15;
short LMOD = 5;

long PROMPTS = 11;
#define BUFS      12
#define BUFSIZE   16

#define ADD_START     3
#define ASSIGN_START  3
#define CHANGE_START  5
#define MOVE_START    5

char buf[BUFS][BUFSIZE];                  /* array of buffers                */
char pbuf[3][BUFSIZE];

struct fld_parms fld[] = {
    { 6,45,20,1,&LMOD,"PM Number",'n'},
    { 7,45,20,1,&LSL, "Stock Location",'a'},
    { 8,45,20,1,&LSKU,"SKU",'a'},
    { 9,45,20,1,&L05, "Quantity on Hand",'n'},
    {10,45,20,1,&L05, "Allocation", 'n'},
    {11,45,20,1,&L05, "Restock Point",'n'},
    {12,45,20,1,&L05, "Restock Quantity", 'n'},
    {13,45,20,1,&L05, "Lane Capacity",'n'},
    {14,45,20,1,&L01, "Pick Location Index",'n'},
    {15,45,20,1,&L01, "Pick Inhibit Flag",'a'},
    {16,45,20,1,&L01, "Autocasing Flag",'a'},
    {17,45,20,1,&L04, "Display", 'a'},
    
  };
  
struct fld_parms fld1 = {18,45,20,1,&L01, "Proceed? (y/n)", 'a'};
struct fld_parms fld2 = {20,45,20,1,&LMOD,"New PM Number",'n'};
struct fld_parms fld3 = {21,45,20,1,&LSL, "New Stock Location",'a'};

long from_add = 0;
char text[80];

main(argc,argv)
long argc;
char **argv;
{
  putenv("_=acd_pm_screen");
  chdir(getenv("HOME"));
  
  open_all();

  if (sp->sp_multibin_lights == 'y') PROMPTS = 12;
  
  if (argc >= 3) from_add = 1;
  
  if      (streql("add",      argv[1])) add_pm(argv[2]);
  else if (streql("assign",   argv[1])) assign_pm(argv[2]);
  else if (streql("change",   argv[1])) change_pm();
  else if (streql("deassign", argv[1])) deassign_pm();
  else if (streql("move",     argv[1])) move_pm();
  
  leave();
}
/*--------------------------------------------------------------------------*
 This routine relates PM numbers to SKUs. A PM may not be
 assigned if the entered SKU is not in the database.

 If this screen is entered from the add_sku_screen, the SKU
 is entered automatically (the first time), and it will exit
 to the add_SKU_screen. if a down arrow or tab is hit from an
 empty field, the previous entered data will be entered (if any)
 *--------------------------------------------------------------------------*/

add_pm (buffer)
char *buffer;
{
  register struct st_item   *s;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k, pickline;
  long count;
  unsigned char t;
  
  count = have_pm_rec = 0;

  fix(add_pm_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(add_pm_screen);
  sd_screen_on();
  
  while(1)                                /* begin massive loop              */
  {
    sd_cursor(0, 6, 1);
    sd_clear_rest();

    memset( buf, NULL, sizeof(buf) );     /* clear input buffers             */
    display_prompts();
    memset(&pkm_rec, 0, sizeof(pmfile_item));
    pm_default_rec(&pkm_rec);
    pkm_rec.p_piflag = 'n';
    fetch_pm_rec(&pkm_rec);
    if(buffer) strcpy(buf[2], buffer);
    display_pm_rec();
    
    while (1)
    {
      t = get_new_pmodno();
      if (t == UP_CURSOR) continue;
      if (t == RETURN)    break;
      
      while (1)
      {
        t = get_new_stkloc();
        if (t == UP_CURSOR) break;
        if (t == RETURN)    break;
        
        while (1)
        {
          t = get_sku(&count);
          
          if (sp->sp_running_status == 'y' && *buf[2] > 0x20)
          {
            i = &pw[atol(buf[0]) - 1];
            h = &hw[i->pw_ptr - 1];
            
            if (h->hw_bay > 0)
            {
              b = &bay[h->hw_bay - 1];
              if (b->bay_zone > 0)
              {
                z = &zone[b->bay_zone - 1];
                pickline = z->zt_pl;
              
                s = sku_lookup(pickline, buf[2]);
                if (s) eh_post(ERR_SKU_DUP, buf[2]);
                else count = 0;
              }
            }
          }
          else pickline = 0;
          
          if (count > 0) eh_post(ERR_SKU_DUP, buf[2]);
          
          if (t == UP_CURSOR) break;
          if (t == RETURN)    break;
        
          while (1)
          {
            t = get_fields(ADD_START);
            if (t == UP_CURSOR) break;
            if (t == RETURN)    break;
          }                               /* end get_fields                  */
          if (t == RETURN) break;
        }                                 /* end get_sku                     */
        if (t == RETURN) break;
      }                                   /* end get_stkloc                  */
      if (t == RETURN) break;
    }                                     /* end get_pmodno                  */
    if (count > 0) eh_post(ERR_SKU_DUP, buf[2]);
    if (!get_proceed(0)) continue;

    store_pm_rec(&pkm_rec);
    
    begin_work();
    pmfile_write(&pkm_rec);
    commit_work();
        
    if (sp->sp_running_status == 'y' && pickline > 0)
    {
      co_lock();
        
      for (k = coh->co_st_cnt; k > 0; k--)
      {
        if (st[k - 1].st_pl > pickline ||
          memcmp(st[k - 1].st_sku, pkm_rec.p_pmsku, rf->rf_sku) > 0)
        {
           st[k] = st[k - 1];
           continue;
        }
        break;
      }
      s = st + k;

#ifdef DEBUG
  fprintf(stderr, "add(%.*s):  st=%x s=%x count=%d\n",
    rf->rf_sku, pkm_rec.p_pmsku, st, s, s - st);
#endif      
      
      memcpy(s->st_sku,    pkm_rec.p_pmsku,  SkuLength);
      memcpy(s->st_stkloc, pkm_rec.p_stkloc, StklocLength);
      strip_space(s->st_sku,    SkuLength);
      strip_space(s->st_stkloc, StklocLength);
      s->st_pl  = pickline;
      s->st_mod = pkm_rec.p_pmodno;
      s->st_mirror = 0;

      coh->co_st_cnt += 1;
      co_unlock();
      
      assign_picks(pkm_rec.p_pmsku, pkm_rec.p_pmodno);
    }
#ifdef DEBUG
  Bdump(st, 128);
#endif
    begin_work();
    sprintf(text, "SKU %-.*s Module %d Added",
      rf->rf_sku, pkm_rec.p_pmsku, pkm_rec.p_pmodno);
    log_entry(text);
    commit_work();
        
    eh_post(ERR_CONFIRM, "Module Add");
        
    update_autocasing(&pkm_rec);
        
    if (buffer) return;                   /* was add sku call                */
  } 
}
/*-------------------------------------------------------------------------*
 *  Assign SKU to a deassigned record
 *-------------------------------------------------------------------------*/
assign_pm (buffer)
char *buffer;
{
  register struct st_item   *s;
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k, pickline;
  long count;
  unsigned char t;
  
  memset( buf, NULL, sizeof(buf) );      /* clear input buffers             */
  count = 0;
  
  fix(assign_pm_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(assign_pm_screen);
  sd_screen_on();
  
  while(1)                                /* begin massive loop              */
  {
    sd_cursor(0, 6, 1);
    sd_clear_rest();
    display_prompts();
    memset(&pkm_rec, 0, sizeof(pmfile_item));
    memset( buf, NULL, sizeof(buf) );     /* clear input buffers             */
    if(buffer) 
    {
      strcpy(buf[2], buffer);
      sd_cursor(0, fld[2].irow, fld[2].icol);
      sd_text(buf[2]);
    }
    while (1)
    {
      have_pm_rec = 0;
      
      t = get_pmodno();
      if (t == UP_CURSOR) continue;
      
      if (have_pm_rec)
      {
        if (pkm_rec.p_pmsku[0] > 0x20)
        {
          eh_post(LOCAL_MSG, "Already Assigned");
          continue;
        }
        fetch_pm_rec(&pkm_rec);
        if (buffer) strcpy(buf[2], buffer);  /* from add sku screen          */
        display_pm_rec();
      }
      while (1)
      {
        if (have_pm_rec) t = get_new_stkloc();
        else
        {
          t = get_stkloc();
          if (have_pm_rec)
          {
            if (pkm_rec.p_pmsku[0] > 0x20)
            {
              eh_post(LOCAL_MSG, "Already Assigned");
              continue;
            }
            fetch_pm_rec(&pkm_rec);
            if (buffer) strcpy(buf[2], buffer);  /* from add sku screen      */
            display_pm_rec();
          }
        }
        if (t == UP_CURSOR) break;
        
        if (!have_pm_rec)
        {
          eh_post(LOCAL_MSG, "Need Module or Stkloc");
          continue;
        }
        while (1)
        {
          if (buffer) t = TAB;
          else        t = get_sku(&count);
          
          if (!(*buf[2]))
          {
            eh_post(LOCAL_MSG, "Must Have A Product");
            continue;
          }
          if (sp->sp_running_status == 'y')
          {
            i = &pw[atol(buf[0]) - 1];
            h = &hw[i->pw_ptr - 1];
            
            if (h->hw_bay > 0)
            {
              b = &bay[h->hw_bay - 1];
              if (b->bay_zone > 0)
              {
                z = &zone[b->bay_zone - 1];
                pickline = z->zt_pl;
                
                s = sku_lookup(pickline, buf[2]);
                if (s) eh_post(ERR_SKU_DUP, buf[2]);
                else count = 0;
              }
            }
          }
          else pickline = 0;

          if (count > 0) eh_post(ERR_SKU_DUP, buf[2]);

          if (t == UP_CURSOR) break;
          if (t == RETURN)    break;
          
          while (1)
          {
            t = get_fields(ASSIGN_START);
            if (t == UP_CURSOR) break;
            if (t == RETURN)    break;
          }                               /* end get_fields                  */
          if (t == RETURN) break;
        }                                 /* end get_sku                     */
        if (t == RETURN) break;
      }                                   /* end get_stkloc                  */
      if (t == RETURN) break;
    }                                     /* end get_pmodno                  */
    if (count > 0) eh_post(ERR_SKU_DUP, buf[2]);
    if (!get_proceed(0)) continue;

    begin_work();
    pmfile_setkey(1);
    if (!pmfile_read(&pkm_rec, LOCK))
    {
      store_pm_rec(&pkm_rec);
      pmfile_update(&pkm_rec);
    }
    commit_work();
        
    if (sp->sp_running_status == 'y' && pickline > 0)
    {
      co_lock();
        
      for (k = coh->co_st_cnt; k > 0; k--)
      {
        if (st[k - 1].st_pl > pickline ||
          memcmp(st[k - 1].st_sku, pkm_rec.p_pmsku, rf->rf_sku) > 0)
        {
           st[k] = st[k - 1];
           continue;
        }
        break;
      }
      s = st + k;

#ifdef DEBUG
  fprintf(stderr, "assign(%.*s):  st=%x s=%x count=%d\n",
    rf->rf_sku, pkm_rec.p_pmsku, st, s, s - st);
#endif      
      
      memcpy(s->st_sku,    pkm_rec.p_pmsku,  SkuLength);
      memcpy(s->st_stkloc, pkm_rec.p_stkloc, StklocLength);
      strip_space(s->st_sku,    SkuLength);
      strip_space(s->st_stkloc, StklocLength);
      s->st_pl  = pickline;
      s->st_mod = pkm_rec.p_pmodno;
      s->st_mirror = 0;

#ifdef DEBUG
  Bdump(st, 128);
#endif

      coh->co_st_cnt += 1;
      co_unlock();
      
      assign_picks(pkm_rec.p_pmsku, pkm_rec.p_pmodno);
    }
    sprintf(text, "SKU %-.*s Module %d Assigned",
      rf->rf_sku, pkm_rec.p_pmsku, pkm_rec.p_pmodno);
    begin_work();
    log_entry(text);
    commit_work();
        
    eh_post(ERR_CONFIRM, "Module Assign");
        
    update_autocasing(&pkm_rec);
        
    if (buffer) return;                   /* was add sku call                */
  } 
}

/*-------------------------------------------------------------------------*

 This screen allows the changing of PM record information,
 except for associated SKU. changes are allowed on deassigned
 pick modules.
 
 *-------------------------------------------------------------------------*/

change_pm()
{
  register long k;
  char *stab;                             /* pntr to SKU tab                 */
  unsigned char t;
  register struct st_item *s;
  
#ifdef DEBUG
  fprintf(stderr, "change_pm()\n");
  fflush(stderr);
#endif
  
  memset( buf, NULL, sizeof(buf) );      /* clear input buffers             */

  fix(change_pm_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(change_pm_screen);
  sd_screen_on();
  
  while(1)                                /* begin massive loop              */
  {
    while (1)
    {
      sd_cursor(0, 6, 1);
      sd_clear_rest();
      display_prompts();
      memset(&pkm_rec, 0, sizeof(pmfile_item));
      memset( buf, NULL, sizeof(buf) );   /* clear input buffers             */

      have_pm_rec = 0;
      
      t = get_pmodno();
      if (t == UP_CURSOR) continue;
      
#ifdef DEBUG
  fprintf(stderr, "have_pm_rec=%d\n", have_pm_rec);
  Bdump(&pkm_rec, 32);
  fflush(stderr);
#endif
      
      if (have_pm_rec)
      {
        fetch_pm_rec(&pkm_rec);
        display_pm_rec();
      }
      while (1)
      {
        if (have_pm_rec) t = get_new_stkloc();
        else
        {
          t = get_stkloc();
          if (have_pm_rec)
          {
            fetch_pm_rec(&pkm_rec);
            display_pm_rec();
          }
        }
        if (t == UP_CURSOR) break;
        
        if (!have_pm_rec)
        {
          eh_post(LOCAL_MSG, "Need Module or Stkloc");
          continue;
        }
        while (1)
        {
          t = get_fields(CHANGE_START);
          if (t == UP_CURSOR) break;
          if (t == RETURN)    break;
        }                                 /* end get_fields                  */
        if (t == RETURN) break;
      }                                   /* end get_stkloc                  */
      if (t == RETURN) break;
    }                                     /* end get_pmodno                  */
    if (!get_proceed(0)) continue;

    begin_work();
    pmfile_setkey(1);
    if (!pmfile_read(&pkm_rec, LOCK))
    {
      store_pm_rec(&pkm_rec);
      pmfile_update(&pkm_rec);
      sprintf(text, "Module %d Updated", pkm_rec.p_pmodno);
      log_entry();
      
      eh_post(ERR_CONFIRM, "Module Update");
    }
    commit_work();

    if (sp->sp_running_status == 'y')
    {
      s = mod_lookup(pkm_rec.p_pmodno);
      
      if (s) 
      {
        memcpy(s->st_stkloc, pkm_rec.p_stkloc, StklocLength);
        strip_space(s->st_stkloc, StklocLength);
      }
    }
    update_autocasing(&pkm_rec);
  }                                       /* end massive while(1)loop        */
}

/*-------------------------------------------------------------------------*

 Pick modules may be deassigned if they exist in the database.

 *-------------------------------------------------------------------------*/

deassign_pm()
{
  register long k;
  char *stab;                             /* pntr to SKU tab                 */
  unsigned char t;
  register struct st_item *s;
#ifdef TANDY
	long found=0;
#endif
  
  memset( buf, NULL, sizeof(buf) );      /* clear input buffers             */

  fix(deassign_pm_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(deassign_pm_screen);
  sd_screen_on();
  
  while(1)                                /* begin massive loop              */
  {
    sd_cursor(0, 6, 1);
    sd_clear_rest();
    display_prompts();
    memset(&pkm_rec, 0, sizeof(pmfile_item));
    memset( buf, NULL, sizeof(buf) );     /* clear input buffers             */

    while (1)
    {
      have_pm_rec = 0;
      
      t = get_pmodno();
      if (t == UP_CURSOR) continue;
      
      if (have_pm_rec)
      {
        if (pkm_rec.p_pmsku[0] <= 0x20)
        {
          eh_post(LOCAL_MSG, "Not Assigned");
          continue;
        }
#ifdef TANDY
  	memset(op_rec->pi_sku, 0, SkuLength);
  	memcpy(op_rec->pi_sku, &pkm_rec.p_pmsku, rf->rf_sku);
  
  	pick_setkey(4);
  	pick_startkey(op_rec);
  
  	begin_work();
  	while (!pick_next(op_rec, LOCK))
  	{
    		if (!(op_rec->pi_flags & PICKED )) 
		{
			commit_work();
			found = 1;
			break;
		}
	}
        if (found)
        {
          eh_post(LOCAL_MSG,"Active Picks for the Module");
	  found = 0;
          continue;
        }
#endif
        fetch_pm_rec(&pkm_rec);
        display_pm_rec();
      }
      while (!have_pm_rec)
      {
        t = get_stkloc();
        if (have_pm_rec)
        {
          if (pkm_rec.p_pmsku[0] <= 0x20)
          {
            eh_post(LOCAL_MSG, "Not Assigned");
            have_pm_rec = 0;
            continue;
          }
          fetch_pm_rec(&pkm_rec);
          display_pm_rec();
        }
        if (t == UP_CURSOR) break;
        if (t == RETURN)    break;
      }                                   /* end get_stkloc                  */
      if (!have_pm_rec)
      {
        eh_post(LOCAL_MSG, "Need Module or Stkloc");
        continue;
      }
      if (t == RETURN) break;
    }                                     /* end get_pmodno                  */
    if (!get_proceed(0)) continue;

    if (sp->sp_running_status == 'y')
    {
      co_lock();
      s = mod_lookup(pkm_rec.p_pmodno);

#ifdef DEBUG
  fprintf(stderr, "deassign(%.*s):  st=%x s=%x count=%d\n",
    rf->rf_sku, pkm_rec.p_pmsku, st, s, s - st);
#endif      
      
      if (s)
      {
        for (k = s - st; k < coh->co_st_cnt; k++, s++)
        {
          *s = *(s + 1);
        }
        memset(s, 0, sizeof(struct st_item));
        coh->co_st_cnt -= 1;
      }
      co_unlock();
      
      deassign_picks(pkm_rec.p_pmsku);
    }

#ifdef DEBUG
  Bdump(st, 128);
#endif
    begin_work();
    pmfile_setkey(1);
    if (!pmfile_read(&pkm_rec, LOCK))
    {
      if (sp->sp_unassigned_pm == 'y')
      {
        pm_default_rec(&pkm_rec);
        memset(pkm_rec.p_pmsku, 0, SkuLength);
        pmfile_update(&pkm_rec);
      }
      else pmfile_delete();

      sprintf(text, "SKU %-.*s Module %d Deassigned", 
        rf->rf_sku, pkm_rec.p_pmsku, pkm_rec.p_pmodno);
      log_entry(text);
    }
    commit_work();
    
    eh_post(ERR_CONFIRM, "Module Deleted");
  }                                       /* end massive while(1)loop        */
}
/*-------------------------------------------------------------------------*

 Pick modules may be moved if they exist in the database and the new
 module is not assigned.

 *-------------------------------------------------------------------------*/

move_pm()
{
  register long k;
  char *stab;                             /* pntr to SKU tab                 */
  unsigned char t;
  register struct st_item *s;
    
  memset( buf, NULL, sizeof(buf) );      /* clear input buffers             */

  fix(move_pm_screen);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(move_pm_screen);
  sd_screen_on();
  
  while(1)                                /* begin massive loop              */
  {
    sd_cursor(0, 6, 1);
    sd_clear_rest();
    display_prompts();
    memset(&pkm_rec, 0, sizeof(pmfile_item));
    memset( buf, NULL, sizeof(buf) );   /* clear input buffers             */

    while (1)
    {
      have_pm_rec = 0;
      
      t = get_pmodno();
      if (t == UP_CURSOR) continue;
      
      if (have_pm_rec)
      {
        if (pkm_rec.p_pmsku[0] <= 0x20)
        {
          eh_post(LOCAL_MSG, "Not Assigned");
          continue;
        }
        fetch_pm_rec(&pkm_rec);
        display_pm_rec();
      }
      while (1)
      {
        if (!have_pm_rec) 
        {
          t = get_stkloc();
          if (have_pm_rec)
          {
            if (pkm_rec.p_pmsku[0] <= 0x20)
            {
              eh_post(LOCAL_MSG, "Not Assigned");
              continue;
            }
            fetch_pm_rec(&pkm_rec);
            display_pm_rec();
          }
        }
        if (t == UP_CURSOR) break;

        if (!have_pm_rec)
        {
          eh_post(LOCAL_MSG, "Need Module or Stkloc");
          continue;
        }
        if (t == RETURN)    break;
        
        while (1)
        {
          t = get_fields(MOVE_START);
          if (t == UP_CURSOR) break;
          if (t == RETURN)    break;
        }
        if (t == RETURN) break;
      }
      if (t == RETURN) break;
    }                         
    get_move_rec();
    
    if (!get_proceed(4)) continue;

    new = pkm_rec;
    new.p_pmodno = work.p_pmodno;
    memcpy(new.p_stkloc, work.p_stkloc, StklocLength);
    
    if (sp->sp_running_status == 'y')
    {
      co_lock();
      s = mod_lookup(pkm_rec.p_pmodno);
      
      if (s) 
      {
        memcpy(s->st_stkloc, new.p_stkloc, StklocLength);
        strip_space(s->st_stkloc, StklocLength);
        s->st_mod = new.p_pmodno;
      }
      co_unlock();
      
      move_picks(pkm_rec.p_pmsku, new.p_pmodno);
    }
    update_autocasing(&new);
    
    begin_work();
    pmfile_setkey(1);
    if (!pmfile_read(&pkm_rec, LOCK))
    {
      if (sp->sp_unassigned_pm == 'y')
      {
        pm_default_rec(&pkm_rec);
        memset(pkm_rec.p_pmsku, 0, SkuLength);
        pmfile_update(&pkm_rec);
      }
      else pmfile_delete();
    }
    commit_work();
    
    begin_work();
    if (have_move_rec) 
    {
      if (!pmfile_read(&work, LOCK))
      {
        pmfile_update(&new);
      }
    }
    else pmfile_write(&new);
    commit_work();

    sprintf(text, "SKU %-.*s Module %d Moved To %d", 
      rf->rf_sku, pkm_rec.p_pmsku, pkm_rec.p_pmodno, new.p_pmodno);
    begin_work();
    log_entry(text);
    commit_work();

    eh_post(ERR_CONFIRM, "Module Move");
  }                                       /* end massive while(1)loop        */
}
/*-------------------------------------------------------------------------*
 *  Get Proceed? (y/n)
 *-------------------------------------------------------------------------*/
get_proceed(n)
long n;
{
  unsigned char t;
  char buf1[2];

  sd_prompt(&fld1, n);
  strcpy(buf1, "n");
  
  while (1)
  {
    t = sd_input(&fld1, n, 0, buf1, 0);
    if (t == EXIT) leave();
    *buf1 = tolower(*buf1);
    if (code_to_caps(*buf1) == 'y') return 1;
    if (code_to_caps(*buf1) == 'n') return 0;
    
    eh_post(ERR_YN, 0);
  }
}
/*-------------------------------------------------------------------------*
 *  Get Existing PM Record On Module Number
 *-------------------------------------------------------------------------*/
get_pmodno()
{
  unsigned char t;
  
  while(1)
  {
    t = sd_input(&fld[0], 0, 0, buf[0], 0);

    if (t == EXIT) leave();

    strip_space(buf[0], BUFSIZE);
    
    if (!(*buf[0])) return t;

    pkm_rec.p_pmodno = atol(buf[0]);   
    
    if( pkm_rec.p_pmodno <= 0 || pkm_rec.p_pmodno > sp->sp_modules)
    {
      eh_post(ERR_PM_INV, buf[0]);
      continue;
    }
    pmfile_setkey( 1 );
    if (!pmfile_read( &pkm_rec, NOLOCK ))
    {
      have_pm_rec = 1;
      return t;
    }
    eh_post(ERR_PM_INV, buf[0]);
  }
}
/*-------------------------------------------------------------------------*
 *  Get Existing PM Record On Module Number For Move
 *-------------------------------------------------------------------------*/
get_move_pmodno()
{
  unsigned char t;
  
  while(1)
  {
    t = sd_input(&fld2, 0, 0, pbuf[0], 0);

    if (t == EXIT) leave();
    
    strip_space(pbuf[0], BUFSIZE);
    
    if (!(*pbuf[0])) return t;

    work.p_pmodno = atol(pbuf[0]);   
    
    pmfile_setkey( 1 );
    if (!pmfile_read( &work, NOLOCK )) 
    {
      if (work.p_pmodno == pkm_rec.p_pmodno)
      {
        eh_post(ERR_PM_DUP, pbuf[0]);
        continue;
      }
      if (work.p_pmsku[0] > 0x20)
      {
        eh_post(LOCAL_MSG, "Already Assigned");
        continue;
      }
      have_move_rec = 1;
      sd_cursor(0, fld3.irow, fld3.icol);
      sprintf(pbuf[2], "%-.*s", LSL, work.p_stkloc);
      sd_text(pbuf[2]);
      return t;
    }
    eh_post(ERR_PM_INV, pbuf[0]);
  }
}
/*-------------------------------------------------------------------------*
 *  Get New PM Record On Module Number
 *-------------------------------------------------------------------------*/
get_new_pmodno()
{
  unsigned char t;
  
  while(1)
  {
    t = sd_input(&fld[0], 0, 0, buf[0], 0);

    if (t == EXIT) leave();
    
    strip_space(buf[0], BUFSIZE);
    
    if (!(*buf[0]))
    {
      eh_post(ERR_PM_INV, buf[0]);
      continue;
    }
    pkm_rec.p_pmodno = atol(buf[0]);   
    
    if( pkm_rec.p_pmodno <= 0 || pkm_rec.p_pmodno > sp->sp_modules)
    {
      eh_post(ERR_PM_INV, buf[0]);
      continue;
    }
    pmfile_setkey( 1 );
    if (!pmfile_read( &pkm_rec, NOLOCK ))
    {
      eh_post(ERR_PM_DUP, buf[0]);
      continue;
    }
    return t;
  }
}
/*-------------------------------------------------------------------------*
 *  Get Existing Stkloc
 *-------------------------------------------------------------------------*/
get_stkloc()
{
  unsigned char t;
  
  while(1)
  {
    t = sd_input(&fld[1], 0, 0, buf[1], 0);

    if (t == EXIT) leave();
    
    strip_space(buf[1], BUFSIZE);
    
    if (!(*buf[1])) return t;

    memcpy( pkm_rec.p_stkloc, buf[1], sizeof(pkm_rec.p_stkloc) );
    space_fill( pkm_rec.p_stkloc, sizeof(pkm_rec.p_stkloc) );

    pmfile_setkey( 3 );            
    pmfile_startkey( &pkm_rec );

    if (!pmfile_read( &pkm_rec, NOLOCK ))
    {
       have_pm_rec = 1;
       return t;
    }
    eh_post(ERR_CAPS_INV, buf[1]);
  }
}
/*-------------------------------------------------------------------------*
 *  Get Existing Stkloc For Move
 *-------------------------------------------------------------------------*/
get_move_stkloc()
{
  unsigned char t;
  
  while(1)
  {
    t = sd_input(&fld3, 0, 0, pbuf[1], 0);

    if(t == EXIT) leave();

    strip_space(pbuf[1], BUFSIZE);
    
    if (!(*pbuf[1])) return t;

    memcpy( work.p_stkloc, pbuf[1], sizeof(work.p_stkloc) );
    space_fill( work.p_stkloc, sizeof(work.p_stkloc) );

    pmfile_setkey( 3 );            
    pmfile_startkey( &work );

    if (!pmfile_read( &work, NOLOCK )) 
    {
      if (work.p_pmodno == pkm_rec.p_pmodno)
      {
        eh_post(ERR_PM_DUP, pbuf[1]);
        continue;
      }
      if (work.p_pmsku[0] > 0x20)
      {
        eh_post(LOCAL_MSG, "Already Assigned");
        continue;
      }
      have_move_rec = 1;
      sprintf(pbuf[0], "%d", work.p_pmodno);
      sd_cursor(0, fld2.irow, fld2.icol);
      sd_text(pbuf[0]);
      return t;
    }
    eh_post(ERR_CAPS_INV, pbuf[1]);
  }
}
/*-------------------------------------------------------------------------*
 *  Get New or Changed Stkloc
 *-------------------------------------------------------------------------*/
get_new_stkloc()
{
  unsigned char t;
  
  while(1)
  {
    t = sd_input(&fld[1], 0, 0, buf[1], 0);

    if (t == EXIT) leave();
    
    strip_space(buf[1], BUFSIZE);
    
    if (!(*buf[1])) return t;

    memcpy( work.p_stkloc, buf[1], sizeof(work.p_stkloc) );
    space_fill( work.p_stkloc, sizeof(work.p_stkloc) );

    pmfile_setkey( 3 );            
    pmfile_startkey( &work );

    if (!pmfile_read( &work, NOLOCK ))
    {
      if (have_pm_rec)
      {
        if (memcmp(pkm_rec.p_stkloc, work.p_stkloc, StklocLength) == 0) 
        {
          return t;
        }
      }
      eh_post(ERR_CAPS_DUP, buf[1]);
      continue;
    }
    return t;
  }
}
/*-------------------------------------------------------------------------*
 *  Get SKU - Sku Must Exist In Product File Or Be Null
 *-------------------------------------------------------------------------*/
get_sku(count)
register long *count;
{
  unsigned char t, dups;
  char message[64];
  
  *count = 0;
  
  sd_cursor(0, 18, 1); sd_clear_line();
  sd_cursor(0, 19, 1); sd_clear_line();
  sd_cursor(0, 20, 1); sd_clear_line();
  sd_cursor(0, 21, 1); sd_clear_line();
  sd_cursor(0, 22, 1); sd_clear_line();
  
  while(1)
  {
    t = sd_input(&fld[2], 0, 0, buf[2], 0);
    
    if (t == EXIT) leave();

    strip_space(buf[2], BUFSIZE);
    
    if (!(*buf[2])) return t;
        
    memcpy( sku_rec.p_pfsku, buf[2], sizeof(sku_rec.p_pfsku) );
    space_fill(sku_rec.p_pfsku, sizeof(sku_rec.p_pfsku));

    prodfile_setkey( 1 );

    if( prodfile_read( &sku_rec, NOLOCK ) )
    {
      eh_post(ERR_SKU_INV, buf[2]);
      continue;
    }
    pmfile_setkey( 2 );
    memcpy( work.p_pmsku, buf[2], sizeof(work.p_pmsku) );
    space_fill( work.p_pmsku, sizeof(work.p_pmsku));
          
    pmfile_startkey( &work );
    dups = 0;
        
    while (!pmfile_next( &work, NOLOCK ) )
    {
      sd_cursor(0, 19, 20);
      sd_text("Location Assignments");
            
      sprintf(message, "Stkloc: %-.*s  Module: %d",
        LSL, work.p_stkloc, work.p_pmodno);
      sd_cursor(0, 20 + (dups % 4), 20);
      sd_text(message);
      dups += 1;
    }
    if (count) *count = dups;
    return t;
  }
}
/*-------------------------------------------------------------------------*
 *  Get Data Fields
 *-------------------------------------------------------------------------*/
get_fields(min)
long min;
{
  register long k;
  unsigned char t;
  
  k = min; 
  
  while (1)
  {
    while (1)
    {
      t = sd_input(&fld[k], 0, 0, buf[k], 0);
      if (t == EXIT) leave();
      if (t == RETURN) break;
      if (t == UP_CURSOR)
      {
        k--;
        if (k < min) return t;
        continue;
      }
      k++;
      if (k >= PROMPTS) k = min;
    }
    *buf[9]  = tolower(*buf[9]);
    *buf[10] = tolower(*buf[10]);

    if (atol(buf[5]) > atol(buf[7]))        /* restock > lcap                */
    {
      eh_post(ERR_RESTOCK, 0);
      k = 5;
      continue;
    }
    if (atol(buf[6]) > atol(buf[7]))        /* rqty > lcap                   */
    {
      eh_post(LOCAL_MSG, "Restock qty exceeds lane capacity");
      k = 6;
      continue;
    }
    if (atol(buf[8]) < 0 || atol(buf[8]) > 9)
    {
      eh_post(ERR_CODE, buf[8]);
      k = 8;
      continue;
    }
    if (code_to_caps(*buf[9]) != 'y' && code_to_caps(*buf[9]) != 'n')
    {
      eh_post(ERR_YN, 0);
      k = 9;
      continue;
    }
    if (code_to_caps(*buf[10]) != 'y' && code_to_caps(*buf[10]) != 'n')
    {
      eh_post(ERR_YN, 0);
      k = 10;
      continue;
    }
    if (sp->sp_autocasing == 'n')
    {
      if (code_to_caps(*buf[10]) == 'y')
      {
        eh_post(LOCAL_MSG, "No Autocasing Feature");
        k = 10;
        continue;
      }
    }
    return RETURN;
  }
}
/*-------------------------------------------------------------------------*
 *  Update Autocasing
 *-------------------------------------------------------------------------*/
update_autocasing(x)
register pmfile_item *x;
{
  register struct pw_item *i;
  
  if (sp->sp_running_status != 'y') return 0;

  i = &pw[x->p_pmodno - 1];

  if (x->p_piflag == 'y') i->pw_flags |=  PicksInhibited;
  else                    i->pw_flags &= ~PicksInhibited;
  
  if (x->p_acflag == 'y')
  {
    memcpy( sku_rec.p_pfsku, pkm_rec.p_pmsku, sizeof(sku_rec.p_pfsku));
    prodfile_setkey( 1 );

    if(!prodfile_read( &sku_rec, NOLOCK ))
    {
      i->pw_case = sku_rec.p_cpack;
      i->pw_pack = sku_rec.p_ipqty;
      return 0;
    }
  }
  i->pw_case = 1000;
  i->pw_pack = 100;
  return  0;
}
/*-------------------------------------------------------------------------*
 *  Get Existing Pmfile Record For Move
 *-------------------------------------------------------------------------*/
get_move_rec()
{
  unsigned char t;

  sd_prompt(&fld2, 0);
  sd_prompt(&fld3, 0);
    
  memset(pbuf, 0, sizeof(pbuf));
  memset(&work, 0, sizeof(pmfile_item));
  
  while (1)
  {
    have_move_rec = 0;

    t = get_move_pmodno();
    if (t == UP_CURSOR) continue;
      
    while (!have_move_rec)
    {
      t = get_move_stkloc();
      if (t == UP_CURSOR) break;
    }
    if (t == UP_CURSOR) continue;
    
    if (!have_move_rec)
    {
      eh_post(LOCAL_MSG, "Need Valid Destination");
      continue;
    }
    if (t == RETURN) return 0;
  }
}
/*-------------------------------------------------------------------------*
 *  Display Prompts
 *-------------------------------------------------------------------------*/
display_prompts()
{
  register long k;

  for (k = 0; k < PROMPTS; k++)
  {
    sd_prompt(&fld[k], 0);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Display Buffer Values
 *-------------------------------------------------------------------------*/
display_pm_rec()
{
  register long k;

  for (k = 0; k < PROMPTS; k++)
  {
    sd_cursor(0, fld[k].irow, fld[k].icol);
    sd_text(buf[k]);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Store Values to PM Record
 *-------------------------------------------------------------------------*/
store_pm_rec(x)
register pmfile_item *x;
{
  x->p_pmodno = atol(buf[0]);

  memcpy(x->p_stkloc, buf[1], StklocLength);
  memcpy(x->p_pmsku,  buf[2], SkuLength);

  space_fill(buf[11], 2);
  memcpy(x->p_display, buf[11], 2);
  
  x->p_qty     = atol(buf[3]);
  x->p_alloc   = atol(buf[4]);
  x->p_restock = atol(buf[5]);
  x->p_rqty    = atol(buf[6]);
  x->p_lcap    = atol(buf[7]);
  x->p_plidx   = atol(buf[8]);
  x->p_piflag  = code_to_caps(*buf[9]);
  x->p_acflag  = code_to_caps(*buf[10]);
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Fetch Values to Buffers
 *-------------------------------------------------------------------------*/
fetch_pm_rec(x)
register pmfile_item *x;
{
  memset(buf, 0, sizeof(buf));
  
  if (x->p_pmodno > 0) sprintf(buf[0], "%d", x->p_pmodno);
  
  if (x->p_stkloc[0] > 0x20)
  {
    memcpy(buf[1], x->p_stkloc, StklocLength);
    strip_space(buf[1], StklocLength);
  }
  if (x->p_pmsku[0] > 0x20) 
  {
    memcpy(buf[2], x->p_pmsku, SkuLength);
    strip_space(buf[2], SkuLength);
  }
  if (x->p_qty)     sprintf(buf[3], "%d", x->p_qty);
  if (x->p_alloc)   sprintf(buf[4], "%d", x->p_alloc);
  if (x->p_restock) sprintf(buf[5], "%d", x->p_restock);
  if (x->p_rqty)    sprintf(buf[6], "%d", x->p_rqty);
  if (x->p_lcap)    sprintf(buf[7], "%d", x->p_lcap);

  sprintf(buf[8], "%d", x->p_plidx);
  
  *buf[9]  = caps_to_code(x->p_piflag);
  *buf[10] = caps_to_code(x->p_acflag);
  
  memcpy(buf[11], x->p_display, 2);
  strip_space(buf[11], 2);
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Clear PM Record to Default Values (except sku, pm and stkloc).
 *-------------------------------------------------------------------------*/
pm_default_rec(x)
register pmfile_item *x;
{
  x->p_piflag  = 'n';             /* aha 051801 */
  x->p_rsflag  = 'n';
  x->p_acflag  = 'n';

  x->p_qty     = 0;
  x->p_restock = 0;
  x->p_rqty    = 0;
  x->p_lcap    = 0;
  x->p_plidx   = 0;
  x->p_cuunits = 0;
  x->p_culines = 0;
  x->p_curecpt = 0;
  x->p_cmunits = 0;
  x->p_cmlines = 0;
  x->p_cmrecpt = 0;

  return 0;
}
/*-------------------------------------------------------------------------*
 *  Assign Picks
 *-------------------------------------------------------------------------*/
assign_picks(sku, mod)
char *sku;
long mod;
{
  long block;
  
  sd_wait();
  
  memset(op_rec->pi_sku, 0, SkuLength);
  memcpy(op_rec->pi_sku, sku, rf->rf_sku);
  
  pick_setkey(4);
  pick_startkey(op_rec);
  
  begin_work();
  while (!pick_next(op_rec, LOCK))
  {
    if (op_rec->pi_flags & (PICKED | NO_PICK | MIRROR)) continue;
    
    pw[op_rec->pi_mod - 1].pw_units_to_go += op_rec->pi_ordered;
    pw[op_rec->pi_mod - 1].pw_lines_to_go += 1;
    
    pl[op_rec->pi_pl - 1].pl_lines_to_go += 1;
    pl[op_rec->pi_pl - 1].pl_units_to_go += op_rec->pi_ordered;
    
    op_rec->pi_mod = mod;   
    
    pick_update(op_rec);
    commit_work();
    
    oc_lock();
    block = oc_find(op_rec->pi_pl, op_rec->pi_on);
    if (block) 
    {
      if (oc->oi_tab[block - 1].oi_queue >= OC_HIGH) 
      {
        oc->oi_tab[block - 1].oi_flags |= NEED_CONFIG;
      }
    }
    oc_unlock();
    begin_work();
  }
  commit_work();
  configure_orders();
  sd_cursor(0, 24, 1);
  sd_clear_line();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Deassign Picks
 *-------------------------------------------------------------------------*/
deassign_picks(sku)
char *sku;
{
  long block;
  
  sd_wait();
  
  memset(op_rec->pi_sku, 0, SkuLength);
  memcpy(op_rec->pi_sku, sku, rf->rf_sku);
  
  pick_setkey(4);
  pick_startkey(op_rec);
  
  begin_work();
  while (!pick_next(op_rec, LOCK))
  {
    if (op_rec->pi_flags & (PICKED | NO_PICK | MIRROR)) continue;
    
    pw[op_rec->pi_mod - 1].pw_units_to_go -= op_rec->pi_ordered;
    pw[op_rec->pi_mod - 1].pw_lines_to_go -= 1;
    
    pl[op_rec->pi_pl - 1].pl_lines_to_go -= 1;
    pl[op_rec->pi_pl - 1].pl_units_to_go -= op_rec->pi_ordered;
    
    op_rec->pi_mod = 0;   
    
    pick_update(op_rec);
    commit_work();
    
    oc_lock();
    block = oc_find(op_rec->pi_pl, op_rec->pi_on);
    if (block) 
    {
      if (oc->oi_tab[block - 1].oi_queue >= OC_HIGH)
      {
        oc->oi_tab[block - 1].oi_flags |= NEED_CONFIG;
      }
      else oc->oi_tab[block - 1].oi_flags |= ORPHANS;
    }
    oc_unlock();
    begin_work();
  }
  commit_work();
  configure_orders();
  sd_cursor(0, 24, 1);
  sd_clear_line();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Move Picks
 *-------------------------------------------------------------------------*/
move_picks(sku, mod)
char *sku;
long mod;
{
  long block;
  
  sd_wait();
  
  memset(op_rec->pi_sku, 0, SkuLength);
  memcpy(op_rec->pi_sku, sku, rf->rf_sku);
  
  pick_setkey(4);
  pick_startkey(op_rec);
  
  begin_work();
  while (!pick_next(op_rec, LOCK))
  {
    if (op_rec->pi_flags & (PICKED | NO_PICK | MIRROR)) continue;
    
    pw[op_rec->pi_mod - 1].pw_units_to_go -= op_rec->pi_ordered;
    pw[op_rec->pi_mod - 1].pw_lines_to_go -= 1;
    
    op_rec->pi_mod = mod;   
    
    pw[op_rec->pi_mod - 1].pw_units_to_go += op_rec->pi_ordered;
    pw[op_rec->pi_mod - 1].pw_lines_to_go += 1;
    
    pick_update(op_rec);
    commit_work();
    
    oc_lock();
    block = oc_find(op_rec->pi_pl, op_rec->pi_on);
    if (block) 
    {
      if (oc->oi_tab[block - 1].oi_queue >= OC_HIGH)
      {
        oc->oi_tab[block - 1].oi_flags |= NEED_CONFIG;
      }
    }
    oc_unlock();
    begin_work();
  }
  commit_work();
  configure_orders();
  sd_cursor(0, 24, 1);
  sd_clear_line();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Recongfigure Orders
 *-------------------------------------------------------------------------*/
configure_orders()
{
  register long k;
  register struct oi_item *o;
  
  for (k = 1, o = oc->oi_tab; k <= oc->of_size ; k++, o++)
  {
    if (!(o->oi_flags & NEED_CONFIG)) continue;
    
    o->oi_flags &= ~NEED_CONFIG;
    
    begin_work();
    od_config(k, 1, 0);
    commit_work();
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Close Files
 *-------------------------------------------------------------------------*/
close_all()
{
  prodfile_close( );
  pmfile_close( );
  log_close( );
  od_close();
  ss_close();
  co_close();
  oc_close();
  sd_close();
  database_close();
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  close_all();
  
  if(from_add)              /* we were assigning and came from add SKU screen*/
  {
    execlp("acd_sku_screen", "acd_sku_screen", "add", 0);
    krash("leave", "acd_sku_screen load", 1);
  }
  else
  {
    execlp("pfead_pmfile", "pfead_pmfile", 0);
    krash("leave", "pfead_pmfile load", 1);
  }
}

/*-------------------------------------------------------------------------*
 *  Open All 
 *------------------------------------------------------------------------*/
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  co_open();
  oc_open();
  od_open();
  prodfile_open( AUTOLOCK );
  pmfile_open( AUTOLOCK );
  log_open( AUTOLOCK );
  
  LSKU = rf->rf_sku;
  LMOD = rf->rf_mod;
  LSL  = rf->rf_stkloc;
  
  if (LSKU < 1) LSKU = SkuLength;
  if (LMOD < 1) LMOD = ModuleLength;
  if (LSL  < 1) LSL  = StklocLength;
  
  return 0;
}
/* end of acd_pm_screen.c */
