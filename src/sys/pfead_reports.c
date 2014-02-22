/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Product File Reports.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   12/11/93  |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char pfead_reports_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      product file reports menu     screen 7.0C                       */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "pfead_reports.t"

extern leave();

static short ONE = 1;

struct fld_parms fld1 = {22, 45, 24, 1, &ONE, "Enter Code",'a'};

main()
{
  short rm;
  short ret;
  unsigned char t;
  char buf[4];
  
  putenv("_=pfead_reports");
  chdir(getenv("HOME"));
  
  open_all();
  
  fix(pfead_reports);
  sd_screen_off();
  sd_clear_screen();
  sd_text(pfead_reports);
  sd_screen_on();
  
  while(1)
  {
    memset(buf, 0, sizeof(buf));

    t = sd_input(&fld1,(sd_prompt(&fld1,0)),&rm,buf,0);

    if(t ==  EXIT) leave();
    if(t != RETURN) continue;                  /* invalid keys               */

    *buf = tolower(*buf);
 
    if (sp->sp_running_status == 'y')
    {
      if (*buf == 'q' || *buf == 'i')
      {
        eh_post(ERR_IS_CONFIG, 0);        /* system is running message       */
      }
    }
    if (sp->sp_config_status != 'y')
    {
      if (*buf == 'a' || *buf == 'm')
      {
        eh_post(ERR_NO_CONFIG, 0);
        continue;
      }
    }
    switch(*buf)
    {
      case 'a':  
        
        loadprog("stockout_input", 0);
        break;

      case 'i':
        
        loadprog("pff_inquiry_input", 0);
        break;

      case 'k':
        
        loadprog("i_o_sku_data", 0);
        break;
       
      case 'l':
        
        loadprog("pick_loc_input", 0);
        break;

      case 'm':
        
        loadprog("item_move_input", 0);
        break;
        
      case 'q':
        
        loadprog("product_file_query", 0);
        break;

      case 'r':
        
        loadprog("restock_rpt_input", 0);
        break;

      case 's':
        
        loadprog("stock_stat_input", 0);
        break;

      case 't':
        
        loadprog("e_d_res_tick_prnt", 0);
        break;

#ifdef DELL        
      case 'b':
      
        loadprog("location_report", 0);
        break;
        
      case 'p':
     
        loadprog("parts_report", 0);
        break;
#endif      

      default:
    
        eh_post(ERR_CODE, buf);
        break;
    }                                      /* end switch                     */
  } 
}
/*-------------------------------------------------------------------------*
 *  Load Program And Exit
 *-------------------------------------------------------------------------*/
loadprog(prog, parm)
register char *prog, *parm;
{
  char text[80];

  close_all();
  execlp(prog, prog, parm, 0);
  open_all();    
  sprintf(text, "Program %s not found", prog);
  eh_post(CRASH_MSG, text);
}

open_all()
{
  sd_open(leave);
  ss_open();
}
close_all()
{
  ss_close();
  sd_close();
}
/*
 * transfer control back to menu
 */
leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 0);
}

/* end of pfead_reports.c */
