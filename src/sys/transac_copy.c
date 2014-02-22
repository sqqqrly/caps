/*------------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Copy tranaction file, xt, to transmission file, zt.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/13/93   |  tjt  Added to mfc.
 *  05/04/94   |  tjt  Six digit order number
 *  07/28/94   |  tjt  Seven digit order number
 *  08/09/94   |  tjt  Transaction directory.
 *  07/22/95   |  tjt  Revise Bard calls.
 *-------------------------------------------------------------------------*/
static char transac_copy_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "Bard.h"
#include "global_types.h"
#include "file_names.h"
#include "ss.h"
#include "xt.h"

FILE *zt_fd, *fd;

struct trans_item xt;

long k, block;
/*
 *  Default Field Lengths
 */
short con = 0;
short on  = 5;
short pl  = 1;
short sku = 15;
short pm  = 4;
short qn  = 3;

char LF;
char mode = 0;                            /* 0 = 880, 1 = 660                */
char mode_id[3];                          /* 660 or anything                 */

main()
{
  putenv("_=transac_copy");
  chdir(getenv("HOME"));
  
  database_open();

  ss_open();
  zt_fd = fopen(zt_name, "w");            /* xmit file                       */
  if (zt_fd == 0) return;

  if (!mode)                              /* 880 output mode                 */
  {
    con  = rf->rf_con;
    on   = rf->rf_on;
    pl   = rf->rf_pl;
    sku  = rf->rf_sku;
    pm   = rf->rf_mod;
    qn   = rf->rf_quan;
  }
/*
 * Main Loop
 */
  xt_open();
  transaction_setkey(1);
  
  begin_work();
  while (!transaction_next(&xt, LOCK))
  {
    if (mode)                             /* 660 mode                        */
    {
      if (xt.xt_code == 'S') xt.xt_code = 'E';
    }
    fwrite(&xt.xt_code, 1, 1, zt_fd);     /* output code                     */

    if (con > 0)                          /* output customer no              */
    {
      fwrite(xt.xt_con, con, 1, zt_fd);
    }
    fwrite(&xt.xt_on[OrderLength - on], on, 1, zt_fd);/* order number        */

    if (pl > 0)                           /* output pickline                 */
    {
      fwrite(&xt.xt_pl[PicklineLength - pl], pl, 1, zt_fd);
    }
    if (sku > 0)                          /* output sku                      */
    {
      fwrite(xt.xt_sku_mod1, sku, 1, zt_fd);
    }
    else                                  /* output pm                       */
    {
      fwrite(&xt.xt_sku_mod1[SkuLength - pm], pm, 1, zt_fd);
    }
    fwrite(&xt.xt_quan1[QuantityLength - qn], qn, 1, zt_fd); /* quan ordered */

    fwrite(&xt.xt_quan2[QuantityLength - qn], qn, 1, zt_fd);/* quan picked   */

    if (sp->sp_lot_control != 'n')
    {
      fwrite(xt.xt_lot, LotLength, 1, zt_fd);    /* output lot number        */
    }
    fwrite("\n", 1, 1, zt_fd);            /* add a LF                        */
  
    transaction_delete();
    commit_work();
    begin_work();
  }
  commit_work();
  fclose(zt_fd);

  xt_close();

  sp->sp_to_count = 0;

  ss_close();

  database_close();

  exit(0);
}

/* end of transac_copy.c  */
