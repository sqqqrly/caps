/*-----------------------------------------------------------------------
 *  Custom Code:    EPSON  - Epson printer.
 *                  STAR   - Star printer (Kmart).
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Short and restock printer.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/27/93   |  tjt  Added to mfc.
 *  06/30/94   |  tjt  Add EPSON and STAR defines.
 *  06/30/94   |  tjt  Add remaining picks to short ticket.
 *  08/27/94   |  tjt  Remove TTYSETSTATE from open_printer.
 *  09/29/94   |  tjt  Add remining picks to short queue item.
 *  02/15/95   |  tjt  Add split flag to short notice.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  07/25/95   |  tjt  Short ticket format #3 for Willams-Sonoma.
 *  07/25/95   |  tjt  Run without sku support.
 *  08/23/96   |  tjt  Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char notice_printer_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Short/Restock Printer Daemon
 *
 *  notice_printer [device] -r[=1|2] -s[=1|2|3] [pickline] ..
 *
 *  i.e. notice_printer /dev/tty12 -r -s=2 1 3 5 7
 *
 *-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include "file_names.h"
#include "global_types.h"
#include "message_types.h"
#include "ss.h"

#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"
#include "bard/short_notice.h"
#include "bard/restock_notice.h"

#define FORM_CUT_EPSON  "\033m"
#define FORM_CUT_STAR   "\033d1"
#define FORM_CUT        "\n\n"

#ifdef  EPSON
#define FORM_CUT FORM_CUT_EPSON
#endif

#ifdef  STAR
#define FORM_CUT FORM_CUT_STAR
#endif

#define DELAY1          16                /* wait for printing               */
#define DELAY2          30                /* wait on empty queue             */

extern leave();

prodfile_item  sku_rec;
pmfile_item    pkm_rec;

short_notice_item sh;
restock_notice_item rs;

long rs_format;                           /* restock format                  */
long sn_format;                           /* short format                    */

FILE *fd = 0;                             /* printer fd                      */
char fd_name[40];                         /* printer name                    */

char pflag[PicklineMax + 1] = {0};        /* picklines covered               */

unsigned char list[] = {ShutdownRequest, 0};

main( argc, argv )
short argc;
char* argv[];
{
  register long j, k;

  putenv("_=notice_printer");
  chdir(getenv("HOME"));

  setpgrp();

  signal_catcher(0);                        /* catch various signals         */
  
  database_open();

  if (argc < 4) krash("main", "missing parms", 1);

  ss_open();
  
  message_open();
  message_select(list, 1);
  message_signal(SIGUSR1, leave);
  
  open_printer(argv[1]);

  for (k = 2; k < argc; k++)              /* gather pickline flags           */
  {
    if (memcmp(argv[k], "-r", 2) == 0 && !rs_format)
    {
      if (argv[k][2] == '=') rs_format = atol(&argv[k][3]);
      else rs_format = 1;
      
      restock_open( AUTOLOCK );
      restock_setkey( 1 );
      continue;
    }
    if (memcmp(argv[k], "-s", 2) == 0 && !sn_format)
    {
      if (argv[k][2] == '=') sn_format = atol(&argv[k][3]);
      else sn_format = 1;

      short_open( AUTOLOCK );
      short_setkey( 1 );
      continue;
    }
    j = atol(argv[k]);
    if (j < 1 || j > PicklineMax) krash("main", "Bad Pickline", 1);
    pflag[j] = 1;
  }
  if (sp->sp_sku_support != 'n')
  {
    prodfile_open( AUTOLOCK );
    prodfile_setkey( 1 );

    pmfile_open( AUTOLOCK );
    pmfile_setkey( 1 );
  }
/*
 *  M A I N   L O O P
 */
  while (1)
  {
    if (short_fd && sp->sp_sp_flag == 'y')
    {
      begin_work();
      while (!short_next(&sh, AUTOLOCK))
      {
#ifdef DEBUG
  fprintf(stderr, "Short Ticket\n");
  Bdump(&sh, sizeof(short_notice_item));
#endif
        if (!pflag[sh.s_sh_pl]) continue;
        if (sn_format) sn_ticket();
        short_delete();
        /* commit_work();
        begin_work(); 
        */
        snooze( DELAY1 );
      }
      commit_work();
      short_setkey( 1 );
    }
    if (restock_fd && sp->sp_rp_flag == 'y')
    {
      begin_work();
      while (!restock_next(&rs, AUTOLOCK))
      {
#ifdef DEBUG
  fprintf(stderr, "Restock Ticket\n");
  Bdump(&rs, sizeof(restock_notice_item));
#endif
        if (!pflag[rs.r_rs_pl]) continue;
        if (rs_format) rs_ticket();
        restock_delete();
        commit_work();
        begin_work();
        snooze( DELAY1 );
      }
      commit_work();
      restock_setkey( 1 );
    }
    snooze( DELAY2 );                       /* wait to start again           */
  }
  return;
}
/*-------------------------------------------------------------------------*
 *
 *  Formats the Short Notice Ticket.
 *
 *-------------------------------------------------------------------------*/
sn_ticket()
{
  struct hw_item *h;
  long sn_cases, now;
  char str[30];
  
  time(&now);

  if (sp->sp_sku_support != 'n')
  {
    pkm_rec.p_pmodno = sh.s_sh_mod;
    if( pmfile_read( &pkm_rec, NOLOCK ) )    return;

    strncpy( sku_rec.p_pfsku, pkm_rec.p_pmsku, sizeof(sku_rec.p_pfsku) );
    if( prodfile_read( &sku_rec, NOLOCK ) )  return;
  }
/*
 *      Calculate case
 */
  if( sku_rec.p_cpack > 0 )
  {
    sn_cases = pkm_rec.p_lcap / sku_rec.p_cpack;
  }
  else
  {
    sn_cases = pkm_rec.p_lcap;
  }

/*
 *  Format Detail Lines - Standard Full Function
 */
  if (sn_format == 1 && sp->sp_sku_support != 'n')   
  {
/* 
 *          1         2         3         4
 * 1234567890123456789012345678901234567890
 *                  SHORT NOTICE
 *  CAPS NO: XXXXX                LINE XX
 * ORDER NO: XXXXXXXXXXXXXX
 *
 *      SKU: xxxxxxxxxxxxxxx
 *     DESC: xxxxxxxxxxxxxxxxxxxxxxxx
 *  ORDERED: xxx  
 *   PICKED: xxx  
 *    SHORT: xxx
 *
 * STOCK LOCATIONS          LANE CAPACITY
 *
 *   PRIMARY BACKUP: xxxxxx CASES: xxxxx
 * SECONDARY BACKUP: xxxxxx UNITS: xxxxx
 *   STOCK LOCATION: xxxxxx CASE PACK: xxx
 *
 * RESTOCKED BY .............UNITS.........
 */
    fprintf (fd, "\n%14cSHORT NOTICE\n\n", ' ');
    fprintf (fd, " CAPS NO: %0*d                LINE %2d\n", rf->rf_on,
      sh.s_sh_on, sh.s_sh_pl);
    if (rf->rf_con > 0)
    {
      fprintf (fd, "ORDER NO: %-15.15s\n",   sh.s_sh_con);
    }
    fprintf(fd,  "\n");
    fprintf (fd, "     SKU: %-15.15s\n", sku_rec.p_pfsku);
    fprintf (fd, "    DESC: %-25.25s\n", sku_rec.p_descr);
    fprintf (fd, " ORDERED: %d\n",   sh.s_sh_ordered);
    fprintf (fd, "  PICKED: %d\n",   sh.s_sh_picked);
    fprintf (fd, "   SHORT: %d\n\n", sh.s_sh_ordered - sh.s_sh_picked);
    fprintf (fd, "STOCK LOCATIONS          LANE CAPACITY\n\n");
    fprintf (fd, "  PRIMARY BACKUP: %-6.6s CASES: %d\n",
      sku_rec.p_bsloc, sn_cases);
    fprintf (fd, "SECONDARY BACKUP: %-6.6s UNITS: %d\n",
      sku_rec.p_absloc, pkm_rec.p_lcap);
    fprintf (fd, "  STOCK LOCATION: %-6.6s CASE PACK: %d\n\n",
      pkm_rec.p_stkloc, sku_rec.p_cpack);
    fprintf (fd, "RESTOCKED BY.......... UNITS.........\n");
    fprintf (fd, "\n\n\n\n\n\n\n\n\n\n");
  }
/*
 *  Format Detail Lines - Basic Function Format
 */
  else if (sn_format == 2 && sp->sp_sku_support != 'n')
  {
    fprintf(fd, "\n");
    fprintf(fd, "            SHORT NOTICE\n");
    fprintf(fd, "     %s", ctime(&now));
    fprintf(fd, "           Pickline No.%d\n\n", sh.s_sh_pl);
    fprintf(fd, "Caps Order # %.*d\n",  rf->rf_on, sh.s_sh_on);
    fprintf(fd, "Caps Group # %*.*s\n", rf->rf_grp, rf->rf_grp, sh.s_sh_grp);
    fprintf(fd, "Picker     # %d\n",    sh.s_sh_picker);
    
    if (pkm_rec.p_piflag == 'y')
    {
      fprintf(fd, "       **********************\n");
      fprintf(fd, "       *** PICK INHIBITED ***\n");
      fprintf(fd, "       **********************\n");
    }
    fprintf(fd, "\n\n");

    fprintf(fd, "  Ordered : %3d %c\n",sh.s_sh_ordered, sh.s_sh_split);
    fprintf(fd, "  Picked  : %3d\n",   sh.s_sh_picked);
    fprintf(fd, "  Short   : %3d\n\n", sh.s_sh_ordered - sh.s_sh_picked);
    fprintf(fd, "PM#  : %05d\n",       pkm_rec.p_pmodno);
    fprintf(fd, "SKU  : %-15.15s\n",   sku_rec.p_pfsku);
    fprintf(fd, "Desc : %-25.25s\n\n", sku_rec.p_descr);

    fprintf(fd, "\n");
    fprintf(fd, "          STOCK LOCATIONS\n\n");
    fprintf(fd, "Caps Stock Loc   : %-6.6s\n",   pkm_rec.p_stkloc);
    fprintf(fd, "Load Number      : %-4.4s\n\n", sh.s_sh_con);
    fprintf(fd, "Primary Backup   : %-6.6s\n",   sku_rec.p_bsloc);
    fprintf(fd, "Secondary Backup : %-6.6s\n\n", sku_rec.p_absloc);

    if (sp->sp_remaining_picks != 'n')
    {
      fprintf(fd, "Total Quantity Yet To Pick : %d\n\n", sh.s_sh_remaining);
    }
    fprintf(fd, "Units        :.................\n\n");
    fprintf(fd, "Box Number   :.................\n\n");
    fprintf(fd, "Restocked By :.................\n\n\n\n\n\n\n\n\n\n\n");
  }
  else if (sn_format == 3)
  {
    fprintf(fd, "\n");
    fprintf(fd, "        SHORT NOTICE\n");
    fprintf(fd, "  %s", ctime(&now));
    fprintf(fd, "    Pickline : %d\n\n",  sh.s_sh_pl);
    fprintf(fd, "  Caps Order : %.*d\n",  rf->rf_on, sh.s_sh_on);
    fprintf(fd, "       Group : %*.*s\n\n", rf->rf_grp,rf->rf_grp,sh.s_sh_grp);
    fprintf(fd, "        PM # : %d\n",     sh.s_sh_mod);
    fprintf(fd, "    Location : %-8.8s\n", sh.s_sh_con);
    fprintf(fd, "     Ordered : %d\n",   sh.s_sh_ordered);
    fprintf(fd, "     Picked  : %d\n",   sh.s_sh_picked);
    fprintf(fd, "       Short : %d\n",   sh.s_sh_ordered - sh.s_sh_picked);
    fprintf(fd, " Yet To Pick : %d\n\n\n\n\n", sh.s_sh_remaining);
  }
  else return 0;
  fprintf(fd, "%s", FORM_CUT);
  fflush(fd);

  sp->sp_sh_printed += 1;
  return;
}

/*-------------------------------------------------------------------------*
 *
 *  Computes Restock Quantity and Cases, and Lane Capacity Cases
 *  and formats the Restock Ticket.
 *
 *-------------------------------------------------------------------------*/
rs_ticket()
{
  long lc_cases;
  long rs_cases;
  long rs_qty;
  long now;
  char str[30];
  
  time(&now);

  if (sp->sp_sku_support == 'n') return 0;

  pkm_rec.p_pmodno = rs.r_rs_mod;
  if( pmfile_read( &pkm_rec, NOLOCK ) )    return 0;

  strncpy( sku_rec.p_pfsku, pkm_rec.p_pmsku, sizeof(sku_rec.p_pfsku) );
  if( prodfile_read( &sku_rec, NOLOCK ) )  return 0;

/*
 *      Compute Restock Quantity and Cases
 */
  rs_qty = pkm_rec.p_lcap - pkm_rec.p_restock + rs.r_rs_quantity;

  if( sku_rec.p_cpack > 0 )
  {
    rs_cases = rs_qty / sku_rec.p_cpack;
    lc_cases = pkm_rec.p_lcap / sku_rec.p_cpack;
  }
  else
  {
    rs_cases = rs_qty;
    lc_cases = pkm_rec.p_lcap;
  }

/* 
 *  Format Detail Lines - Standard Full Function
 */
  if (rs_format == 1)
  {
    fprintf (fd,"\n%8cRESTOCK TICKET    NO. %d\n\n",
      ' ', rs.r_rs_number);

    fprintf (fd," SKU: %-15.15s\n", sku_rec.p_pfsku);
    fprintf (fd,"DESC: %-25.25s\n\n", sku_rec.p_descr);
    fprintf (fd,"RESTOCK QTY: %d           CASES: %d\n\n",
      rs_qty, rs_cases);
    fprintf (fd, "STOCK LOCATIONS          LANE CAPACITY\n\n");
    fprintf (fd, "  PRIMARY BACKUP: %-6.6s CASES:     %d\n",
      sku_rec.p_bsloc, lc_cases);
    fprintf (fd, "SECONDARY BACKUP: %-6.6s UNITS:     %d\n",
      sku_rec.p_absloc, pkm_rec.p_lcap);
    fprintf (fd, "  STOCK LOCATION: %-6.6s CASE PACK: %d\n\n",
      pkm_rec.p_stkloc, sku_rec.p_cpack);
    fprintf (fd, "RESTOCKED BY.......... UNITS.........\n");
    fprintf (fd, "\n\n\n\n\n\n\n\n\n\n");
  }
/*
 *  Format Detail Lines - Basic Function Format
 */
  else if (rs_format == 2)
  {
    fprintf(fd, "RESTOCK NOTICE");
    fprintf(fd, " %s  \n", ctime(&now));
    fprintf(fd, "Sku: %-8.8s",       sku_rec.p_pfsku);
    fprintf(fd, "Desc: %-25.25s\n",     sku_rec.p_descr);
    fprintf(fd, "Restock Quantities\n");
    fprintf(fd, "  Units      : %5d\n",   rs_qty);
    fprintf(fd, "  Cases      : %5d\n", rs_cases);
    fprintf(fd, "Lane Capacity           Stock Location\n");
    fprintf(fd, "  Units      : %5d",   pkm_rec.p_lcap);
    fprintf(fd, " Caps Stock Loc  : %-6.6s\n", pkm_rec.p_stkloc);
    fprintf(fd, "  Cases      : %5d",   lc_cases);
    fprintf(fd, " Primary Backup  : %-6.6s\n",   sku_rec.p_bsloc);
    fprintf(fd, "  Case Pack  : %5d", sku_rec.p_cpack);
    fprintf(fd, " Secondary Backup: %-6.6s\n\n", sku_rec.p_absloc);
    fprintf(fd, "Units        :.................\n\n");
    fprintf(fd, "Restocked By :.................\n\n\n");
  }
  else return 0;
  fprintf(fd, "%s", FORM_CUT);
  fflush(fd);

  sp->sp_rs_printed += 1;
  
  return;
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
open_printer(name)
unsigned char *name;
{
  strcpy(fd_name, name);                  /* get printer name                */
  fd = fopen(fd_name, "w");               /* open printer                    */

  if (fd == 0) krash("main", "open printer", 1);
  
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Sleep For a Fixed Time
 *--------------------------------------------------------------------------*/
snooze ( what )
register long what;
{
  register long done, now;
  
  now  = time(0);
  done = now + what;

  while (done > now)
  {
    sleep(done - now);
    now = time(0);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  ss_close();
  message_close();
  if (fd) fclose(fd);
  short_close();
  restock_close();
  if (sp->sp_sku_support != 'n')
  {
    prodfile_close();
    pmfile_close();
  }
  database_close();
  exit(0);
}
  
/* end of notice_printer.c */
