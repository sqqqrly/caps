/*-------------------------------------------------------------------------*
 *  Source Code:    %M%           
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Add, Change Delete SKU.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/02/93   |  tjt  Added to mfc.
 *  06/30/94   |  tjt  Fix redisplay prompts on adds.
 *  06/30/94   |  tjt  Fix default case and inner pack to zero.
 *  06/30/94   |  tjt  Add immediate autocasing changes.
 *  01/30/95   |  tjt  Fix display of module on delete. 
 *  12/06/95   |  tjt  Add multiple stkloc on change.
 *  02/04/97   |  tjt  Add case and pack max to 15,000.
 *  04/18/97   |  tjt  Add langauge.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char acd_sku_screen_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "language.h"
#include "add_sku_screen.t"
#include "change_sku_screen.t"
#include "delete_sku_screen.t"
#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"

prodfile_item  sku_rec;                  /* SKU record                      */
pmfile_item    pkm_rec;                  /* Pick module record              */

static short L01 = 1;
static short L03 = 3;
static short L05 = 5;
static short L06 = 6;
static short L25 = 25;

char text[80];

/*------------------------------------------------------------------------*
    M A I N
 *------------------------------------------------------------------------*/
main(argc,argv)
long argc;
char **argv;
{
  putenv("_=acd_sku_screen");
  chdir(getenv("HOME"));

  open_all();
  
  if (streql("add", argv[1]))          add_sku();
  else if (streql("change", argv[1]))  change_sku();
  else if (streql("delete", argv[1]))  delete_sku();
  else
  {
    close_all();
    execlp("pfead_prodfile", "pfead_prodfile", 0);
    krash("main", "pfead_prodfile load", 1);
  }
}

add_sku()
{
#define NUM_PROMPTS     10
#define BUF_SIZE        26

  static struct fld_parms fld[] = {
    {6,40,1,1,0,"Enter SKU",'a'},
    {7,40,1,1,&L25,"Enter Description",'a'},
    {8,40,1,1,&L25,"Enter Alt. Identification", 'a'},
    {9,40,1,1,&L05,"Enter Family Group",'a'},
    {10,40,1,1,&L03,"Enter Unit of Measure",'a'},
    {11,40,1,1,&L05,"Enter Inner Pack Qty",'n'},
    {12,40,1,1,&L05,"Enter Case Pack",'n'},
    {13,40,1,1,&L06,"Enter Back Up Stock Location",'a'},
    {14,40,1,1,&L06,"Enter Alt. Back Up Stock Loc.",'a'},
    {16,40,1,1,&L01,"Assign Module(s)? (y/n)",'a'},
  };
        
  short rm;
  short ret;
  short pack, casep;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];        /* array of buffers                */
  char parm[8];
  
  short i;
  short n;

        /* set lengths into field structures */

  fld[0].length = &rf->rf_sku;

  fix(add_sku_screen);
  sd_clear_screen();                      /* clear screen                    */
  sd_screen_off();
  sd_text(add_sku_screen);
  sd_screen_on();

  while(1)                                /* begin massive loop              */
  {
    sd_cursor(0, 6, 1);
    sd_clear_rest();                      /* clear rest                      */

    memset( buf, 0, sizeof(buf) );

    for(i = 0; i < 9; i++) sd_prompt(&fld[i],0);

    i = 0;                                /* restore index                   */

                /* main loop to gather input */

    while(1)
    {
      t = sd_input(&fld[i],0,&rm,buf[i],0);

      if(t == EXIT) leave();
      else if(t == UP_CURSOR && i > 0) i--;
      else if(t==DOWN_CURSOR || t==TAB)
      {
        if(i < 8)
        i++;
        else
        i = 0;
      }
      else if(t == RETURN)
      {
        strip_space(buf[0],15);
        if(!(*buf[0]))                    /* nothing entered                 */
        {
          eh_post(ERR_SKU_INV,"");
          i = 0;
          continue;
        }
        space_fill( buf[0], sizeof(sku_rec.p_pfsku) );
        strncpy( sku_rec.p_pfsku, buf[0],
        sizeof(sku_rec.p_pfsku) );
        prodfile_setkey( 1 );
        if(!prodfile_read(&sku_rec, NOLOCK))
        {
          strip_space(buf[0],15);
          eh_post(ERR_SKU_DUP,buf[0]);
          i = 0;
          continue;
        }
        casep = atoi(buf[6]);
        if (casep > 15000)
        {
          eh_post(ERR_VALUE, buf[6]);
          i = 6;
          continue;
        }
        pack = atoi(buf[5]);
        if (pack > 15000)
        {
          eh_post(ERR_VALUE, buf[5]);
          i = 5;
          continue;
        }
        if (casep > 1 && pack > 1)
        {
          if (pack > casep)
          {
            eh_post(ERR_PACK, 0);
            i = 5;
            continue;
          }
        }
        strncpy(sku_rec.p_pfsku,  buf[0], 15);
        strncpy(sku_rec.p_descr,  buf[1], 25);
        strncpy(sku_rec.p_altid,  buf[2], 25);
        strncpy(sku_rec.p_fgroup, buf[3], 5);
        strncpy(sku_rec.p_um,     buf[4], 3);
        strncpy(sku_rec.p_bsloc,  buf[7], 6);
        strncpy(sku_rec.p_absloc, buf[8], 6);

        sku_rec.p_ipqty = pack;
        sku_rec.p_cpack = casep;
        
        begin_work();
        prodfile_write(&sku_rec);
        commit_work();
#ifdef DELL
        description_update(buf[0], buf[1]);
#endif
        sprintf(text, "SKU %-.*s Added", rf->rf_sku, sku_rec.p_pfsku);
        log_entry(text);
        
        eh_post(ERR_CONFIRM, "SKU Add");
        break;
      }
    }
                /* display assign modules request */

    while(1)
    {
      t = sd_input(&fld[9],sd_prompt(&fld[9],0),&rm,buf[9],0);
      if(t == EXIT) leave();
      else if(t == RETURN)
      {
        if (code_to_caps(*buf[9]) == 'y') break;
        if (code_to_caps(*buf[9]) == 'n') break;
        eh_post(ERR_YN,0);
      }
    }
    if(code_to_caps(*buf[9]) == 'y')
    {
      if (sp->sp_unassigned_pm == 'y') strcpy(parm, "assign");
      else strcpy(parm, "add");
      
      close_all();
      
      execlp("acd_pm_screen", "acd_pm_screen", parm, buf[0], 0);
      krash("add_sku", "acd_pm_screen load", 1);
    }
  }                                       /* end massive while(1)loop        */
}


change_sku()
{
#undef  NUM_PROMPTS
#undef  BUF_SIZE
#define NUM_PROMPTS     9
#define BUF_SIZE        26

  static struct fld_parms fld[] = {
    {6,25,1,1,0,"Enter SKU",'a'},
    {11,52,1,0,&L25,"Description",'a'},
    {12,52,1,0,&L25,"Alt. Identification", 'a'},
    {13,52,1,0,&L05,"Family Group",'a'},
    {14,52,1,0,&L03,"Unit of Measure",'a'},
    {15,52,1,0,&L05,"Inner Pack Qty.",'n'},
    {16,52,1,0,&L05,"Case Pack",'n'},
    {17,52,1,0,&L06,"Back Up Stock Location",'a'},
    {18,52,1,0,&L06,"Alt. Back Up Stock Loc.",'a'}
  };
        

  short pack, casep;
  short rm;
  short ret = 0;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];        /* array of buffers                */
  char cbuf[NUM_PROMPTS-1][BUF_SIZE];     /* copy buffers                    */
  char work[40];
  
  short i;
  short n;

        /* set lengths into field structures */

  fld[0].length = &rf->rf_sku;

  fix(change_sku_screen);
  sd_clear_screen();                      /* clear screen                    */
  sd_screen_off();
  sd_text(change_sku_screen);
  sd_screen_on();

  while(1)                                /* begin massive loop              */
  {
    sd_cursor(0,6,1);
    sd_clear_rest();

    memset( buf, 0, sizeof(buf) );     /* clear input buffers             */
    memset( cbuf, 0, sizeof(cbuf) );   /* clear copy buffers              */

                /* get initial entered value (SKU) */
    sd_prompt(&fld[0],0);
    while(1)
    {
      t = sd_input(&fld[0],0,&rm,buf[0],0);
      if(t == EXIT) leave();
      else if(t == RETURN)
      {
        space_fill( buf[0], sizeof(sku_rec.p_pfsku) );
        strncpy( sku_rec.p_pfsku, buf[0], sizeof(sku_rec.p_pfsku) );
        prodfile_setkey( 1 );
        if (prodfile_read(&sku_rec, NOLOCK))
        {
          eh_post(ERR_SKU_INV,buf[0]);
          continue;
        }
        strncpy(cbuf[0], sku_rec.p_descr, 25);
        strncpy(cbuf[1], sku_rec.p_altid, 25);
        strncpy(cbuf[2], sku_rec.p_fgroup, 5);
        strncpy(cbuf[3], sku_rec.p_um, 3);
        sprintf(cbuf[4], "%d", sku_rec.p_ipqty);
        sprintf(cbuf[5], "%d", sku_rec.p_cpack);
        strncpy(cbuf[6], sku_rec.p_bsloc, 6);
        strncpy(cbuf[7], sku_rec.p_absloc, 6);
        
        break;                            /* from input loop                 */
      }
      else
      ;                                   /* continue input                  */
    }                                     /* end SKU input                   */
                /* display change prompts */

    for(i = 1; i < 9; i++)
    sd_prompt(&fld[i],0);

                /* display 'old' and 'new' messages */

    pmfile_setkey( 2 );
    memcpy(pkm_rec.p_pmsku, sku_rec.p_pfsku, 15);
    pmfile_startkey(&pkm_rec);
    n = 0;
    
    while (!pmfile_next(&pkm_rec, NOLOCK))
    {
#ifdef CANTON
      sprintf(work, "Module: %d   ModSlot: %6.6s", 
        pkm_rec.p_pmodno, pkm_rec.p_stkloc);
#else
      sprintf(work, "Stkloc: %6.6s  Module: %d", 
        pkm_rec.p_stkloc, pkm_rec.p_pmodno);
#endif
      sd_cursor(0, 6 + n, 52);
      sd_text(work);
      n = (n + 1) % 4;
    }
    if (!n) pkm_rec.p_pmodno = 0;             /* not assigned                */

    sd_cursor(0,10,25);
    sd_text(".......... Old ..........");
    sd_cursor(0,10,52);
    sd_text(".......... New ..........");

                /* display old data */

    for(i = 0; i < 8; i++)
    {
      sd_cursor(0,i + 11,25);
      sd_text(cbuf[i]);
    }
    i = 1;                                /* reset index                     */

                /* main loop to gather input */

    while(1)
    {
      t = sd_input(&fld[i],0,&rm,buf[i],cbuf[i-1]);

      if(t == EXIT) leave();
      else if(t == UP_CURSOR && i > 1) i--;
      else if(t == DOWN_CURSOR || t == TAB)
      {
        if(*buf[i])                       /* new data entered                */
        ;
        else
        {
          sd_cursor(0,fld[i].irow,fld[i].icol);
          sd_text(cbuf[i-1]);
          strcpy(buf[i],cbuf[i-1]);
        }
        if(i < 8) i++;
        else i = 1;
      }
      else if(t == RETURN)
      {
        if (*buf[6])  casep = atoi(buf[6]);
        else casep = atoi(cbuf[5]);
        if (casep > 15000)
        {
          eh_post(ERR_VALUE, buf[6]);
          i = 6;
          continue;
        }
        if (*buf[5]) pack = atoi(buf[5]);
        else pack = atoi(cbuf[4]);
        if (pack > 15000)
        {
          eh_post(ERR_VALUE, buf[5]);
          i = 5;
          continue;
        }
        if (casep > 1 && pack > 1)
        {
          if (pack > casep)
          {
            eh_post(ERR_PACK, 0);
            i = 5;
            continue;
          }
        }
        break;
      }
      else
      ;                                   /* invalid keys                    */
    }
                /* insert changes to Database */
    
    begin_work();
    if (!prodfile_read(&sku_rec, LOCK))
    {
      strncpy(sku_rec.p_pfsku,  buf[0], 15);
      strncpy(sku_rec.p_descr,  buf[1], 25);
      strncpy(sku_rec.p_altid,  buf[2], 25);
      strncpy(sku_rec.p_fgroup, buf[3], 5);
      strncpy(sku_rec.p_um,     buf[4], 3);
      strncpy(sku_rec.p_bsloc,  buf[7], 6);
      strncpy(sku_rec.p_absloc, buf[8], 6);

      sku_rec.p_ipqty = pack;
      sku_rec.p_cpack = casep;
        
      update_autocasing();
    
      prodfile_update(&sku_rec);
      commit_work();
#ifdef DELL
      description_update(buf[0], buf[1]);
#endif
    }
    else commit_work();

    sprintf(text, "SKU %-.*s Changed", rf->rf_sku, sku_rec.p_pfsku);
    log_entry(text);
    
    eh_post(ERR_CONFIRM, "SKU Change");

  }                                       /* end massive while(1)loop        */
}


delete_sku()
{
#undef  NUM_PROMPTS
#undef  BUF_SIZE
#define NUM_PROMPTS     2
#define BUF_SIZE        16

  static struct fld_parms fld[] = {
    { 6,35,10,1,0,"Enter SKU",'a'},
    {10,35,10,1,&L01,"Are You Sure? (y/n)",'a'}
  };
        

  short rm,ret,i,n;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];        /* array of buffers                */
  char asku_buf[16];                      /* holds associated SKU            */
  long pm_buf[11];                        /* array of pm numbers             */
  short pm_count;
  short d_count;
  char message[50];
  char text[64];
  rm = 0;

        /* set lengths into field structures */

  fld[0].length = &rf->rf_sku;

  fix(delete_sku_screen);
  sd_clear_screen();                      /* clear screen                    */
  sd_screen_off();
  sd_text(delete_sku_screen);
  sd_screen_on();
  
  while(1)                                /* begin massive loop              */
  {
    sd_cursor(0,6,1);
    sd_clear_rest();

    memset( buf, 0, sizeof(buf) );     /* clear input buffers             */

    sd_prompt(&fld[0],0);

                /* main loop to gather input */

    while(1)
    {
      t = sd_input(&fld[0],0,&rm,buf[0],0);

      sd_cursor(0,7,1);
      sd_clear_rest();

      if (t == EXIT) leave();
      else if (t != RETURN) continue;

      if(!(*buf[0]))                    /* no SKU entered                  */
      {
        eh_post(ERR_SKU_INV,"");
        continue;
      }
      space_fill( buf[0], sizeof(sku_rec.p_pfsku) );
      strncpy( sku_rec.p_pfsku, buf[0], sizeof(sku_rec.p_pfsku) );
      prodfile_setkey( 1 );
      if( prodfile_read( &sku_rec, NOLOCK ) )
      {
        strip_space(buf[0],15);
        eh_post(ERR_SKU_INV,buf[0]);
        continue;
      }
                /* display description and pick modules still assigned */

      strip_space( sku_rec.p_descr, sizeof(sku_rec.p_descr) );
      sd_cursor(0,7,10);
      sd_text("Description: ");
      sd_text( sku_rec.p_descr );         /* show description                */

          /* determine if any pick modules assigned to this SKU */

      pm_count = 0;                       /* initialize                      */
      strncpy( pkm_rec.p_pmsku, buf[0], sizeof(pkm_rec.p_pmsku) );

      pmfile_setkey( 2 );
      pmfile_startkey( &pkm_rec );

      while ( !pmfile_next( &pkm_rec, NOLOCK ) )
      {
        sd_cursor(0, 9, 10);
        sd_text("Pick Module(s) Still Assigned ");

        sd_cursor(0, 10 + (pm_count % 10), 10);
        sprintf(text, "Stkloc: %6.6s  Module: %d", 
          pkm_rec.p_stkloc, pkm_rec.p_pmodno);
        sd_text(text);
        pm_count++;
      }
      if (pm_count > 0)
      {
        eh_post(ERR_SKU_DEL, buf[0]);
        continue;
      }
      break;
    }
    rm = pm_count;
    
    while(1)
    {
      t = sd_input(&fld[1],sd_prompt(&fld[1],rm),&rm,buf[1],0);
      if(t == EXIT) leave();
      else if(t == RETURN)
      {
        if(*buf[1] == 'y' || *buf[1] == 'n') break;
        eh_post(ERR_YN,0);
      }
    }
    begin_work();
    if(!prodfile_read( &sku_rec, LOCK )) prodfile_delete();
    commit_work();
    
    sprintf(text, "SKU %-.*s Deleted", rf->rf_sku, sku_rec.p_pfsku);
    log_entry(text);
 
    eh_post(ERR_CONFIRM, "SKU Deleted");
  }                                       /* end massive while(1)loop        */
}
update_autocasing()
{
  register struct pw_item *i;
    
  if (sp->sp_running_status != 'y') return 0;
  
  pmfile_setkey( 2 );
  memcpy(pkm_rec.p_pmsku, sku_rec.p_pfsku, 15);
  while (!pmfile_next(&pkm_rec, NOLOCK))
  {
    i = &pw[pkm_rec.p_pmodno - 1];

    if (pkm_rec.p_acflag == 'y')
    {
      i->pw_case = sku_rec.p_cpack;
      i->pw_pack = sku_rec.p_ipqty;
    }
    else 
    {
      i->pw_case = 1000;
      i->pw_pack = 100;
    }
  }
  return  0;
}
leave()
{
  close_all();
  execlp("pfead","pfead", 0);
  krash("leave", "pfead load", 1);
}

open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  co_open();
  prodfile_open( AUTOLOCK );
  pmfile_open( AUTOLOCK );
  log_open( AUTOLOCK );
  return 0;
}


close_all()
{
  prodfile_close( );
  pmfile_close( );
  log_close( );
  ss_close();
  co_close();
  sd_close();
  database_close();
  return 0;
}


/* end of acd_sku_screen.c  */
