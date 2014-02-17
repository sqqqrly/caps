/*-------------------------------------------------------------------------*
 *  Custom Code:    DAYTIMER - revised pm file output - sku + stkloc.
 *-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Create transaction file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/19/93   |  tjt  Added to mfc.
 *  04/28/95   |  tjt  Modify pm output format with leadig zeros etc.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  07/29/95   |  tjt  Revised to separate file dumps.
 *  08/23/96   |  tjt  Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char create_output_file_c[] = "%Z% %M% %I% (%G% - %U%)";

/*****************************************************************
 *   create_output_file.c                                        *
 *                                                               *
 *   Creates an output file of SKU and Pick Module data for      *
 *   transmitting the product file data to host computer.        *
 *   The data may be transmitted via diskette or the             *
 *   communications network. The record size for both types      *
 *   of data are 128 bytes.                                      *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_names.h"
#include "ss.h"
#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"


pmfile_item pm;
prodfile_item sk;

FILE *pf_fd;                              /* SKU output file                 */
FILE *pm_fd;                              /* Pick Module output file         */

char pf_buf[128];                         /* output buffer for sku record    */
char pm_buf[128];                     /* output buffer for pick module record*/
char term_rcd[128];                       /* terminating record - all Z's    */
short i,ret;

main()
{
  putenv("_=create_trans");
  chdir(getenv("HOME"));
  
  database_open();

  ss_open();

  memset(pm_buf, 0x20, 128);
  memset(pf_buf, 0x20, 128);
  memset(term_rcd, 'Z', 128);

/*-------------------------------------------------------------------------*
 *  Dump Product File
 *-------------------------------------------------------------------------*/
#ifndef DAYTIMER
  pf_fd = fopen(pf_data_name, "w");        /* open SKU output file           */
  if (pf_fd == 0) exit(1);

  prodfile_open(AUTOLOCK);
  prodfile_setkey(1);
  
  begin_work();
  while (!prodfile_next(&sk, NOLOCK))
  {
    commit_work();
    
    sprintf(pf_buf,
      "A%-15.15s%-25.25s%-5.5s%-3.3s%3d%3d%-6.6s%-6.6s%-25.25s",
      sk.p_pfsku,  sk.p_descr, 
      sk.p_fgroup, sk.p_um, 
      sk.p_ipqty,  sk.p_cpack,
      sk.p_bsloc,  sk.p_absloc,
      sk.p_altid);

    memset(pf_buf + 92, 0x20, 36);
/*  pf_buf[127] = '\n';                F072995  avoid LF+CR on PC            */
    
    ret = fwrite (pf_buf, 1, 128, pf_fd); /* write sku record                */
    
    if (ret != 128)
    {
      close_all();
      exit(4);
    }
    begin_work();
  }
  commit_work();
  
  prodfile_close();
  fwrite (term_rcd, 128, 1, pf_fd);  /* write terminating record on sku file */
  fclose(pf_fd);
#endif

/*-------------------------------------------------------------------------*
 * Dump Pick Module File
 *-------------------------------------------------------------------------*/
  pm_fd = fopen(pm_data_name,"w");         /* open Pick Module output file   */
  if (pm_fd == 0) exit(1);

  pmfile_open(AUTOLOCK);
  pmfile_setkey(2);
  
  begin_work();
  
  while(!pmfile_next(&pm, NOLOCK))
  {
    commit_work();
    
#ifdef DAYTIMER
    for (i = 0; i < 6; i++) pm.p_stkloc[i] = toupper(pm.p_stkloc[i]);

    sprintf(pm_buf,
      "%*.*s%-6.6s       ",     
      rf->rf_sku, rf->rf_sku, pm.p_pmsku, pm.p_stkloc);

    ret = fwrite (pm_buf, 1, 25, pm_fd);   /* write PM record              */
    if (ret != 25)
    {
      close_all();
      exit(5);
    }
#else
    sprintf(pm_buf,
      "%5d%5d%-15.15s%5d%5d%5d%5d%-6.6s%d%c%7d%7d%7d%7d%7d%7d%c%d%c",
      pm.p_pmodno,   pm.p_pmodno,    pm.p_pmsku,
      pm.p_qty,      pm.p_restock,   pm.p_rqty,   pm.p_lcap,
      pm.p_stkloc,   pm.p_plidx,     pm.p_piflag,
      pm.p_cuunits,  pm.p_cmunits,
      pm.p_culines,  pm.p_cmlines,
      pm.p_curecpt,  pm.p_cmrecpt,
      pm.p_rsflag,   pm.p_acflag);

    memset(pm_buf + 96, 0x20, 32);

/*  pm_buf[127] = '\n';                  F072905 avoid LF+CR on PC           */

    ret = fwrite (pm_buf, 1, 128, pm_fd);  /* write PM record                */
    if (ret != 128)
    {
      close_all();
      exit(5);
    }
#endif
    begin_work();
  }
  commit_work(); 
   
#ifndef DAYTIMER
  fwrite (term_rcd, 128, 1, pm_fd);  /* write terminating record on pm file  */
#endif
  pmfile_close();
  fclose(pm_fd);
  
  close_all();
  exit(0);
}

/* close all file before returning to calling program   */
close_all()
{
  ss_close();
  prodfile_close();
  pmfile_close();
  database_close();
  if (pf_fd) fclose(pf_fd);
  if (pm_fd) fclose(pm_fd);
  return 0;
}

/* end of create_output_file.c */



