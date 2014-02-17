/*------------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Backup / Restore Menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Change for caps_ttymodes.h
 *  08/05/94   |  tjt  Fix various bugs.
 *  04/24/96   |  tjt  Rewritten for script files.
 *  04/25/00   |  aha  Modified to call improved shell scripts.
 *-------------------------------------------------------------------------*/
static char backupmm_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/****************************************************************************/
/*                                                                          */
/*                     Backup/Restore Menu     (5.0)                        */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "backupmm.t"

static short ONE = 1;

struct fld_parms fld0 = {19,56,25,1,&ONE,"Tape or Diskette? (t/d)",'a'};
struct fld_parms fld1 = {20,56,25,1,&ONE,"Enter Code",'a'};

char command[128];

extern void leave();

main()
{
  unsigned char t;
  char buf[2], td[2];

  putenv("_=backupmm");
  chdir(getenv("HOME"));

  open_all();

  fix(backupmm);                         /* caps header & backup menu       */
  sd_clear_screen();
  sd_text(backupmm);


  while(1)
  {
    memset(td, 0, 2);
    sd_prompt(&fld0, 0);
    t = sd_input(&fld0, 0, 0, td, 0);
    if (t == EXIT) leave();

    if (*td == 't')
    {
      sd_cursor(0,22,25);
      sd_text("Please Insert Tape Into Drive     ");
    }
    else if (*td == 'd')
    {
      sd_cursor(0,22,25);
      sd_text("Please Insert Diskette Into Drive ");
    }
    else
    {
      eh_post(ERR_CODE, td);
      continue;
    }
    while (1)
    {
      memset(buf, 0, 2);
      sd_prompt(&fld1, 0);
      t = sd_input(&fld1, 0, 0, buf, 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) break;

      switch(tolower(buf[0]))
      {
        case 't': if (*td != 't')
                  {
                    eh_post(ERR_CODE, td);
                    break;
                  }
                  sd_wait();
                  system("backupmm_tape");

                  sd_cursor(0, 19, 1);
                  sd_clear_rest();
                  eh_post(ERR_CONFIRM, "Backup");
                  break ;

        case 'a':

                  sd_wait();
                  if (*td == 't') system("backupmm_tape_sku");
                  else            system("backupmm_sku");
                  sd_cursor(0, 19, 1);
                  sd_clear_rest();
                  eh_post(ERR_CONFIRM, "Backup");
                  break ;

        case 'c':                         /* Backup Configuration            */

                  sd_wait();
                  if (*td == 't') system("backupmm_tape_config");
                  else            system("backupmm_config");
                  sd_cursor(0, 19, 1);
                  sd_clear_rest();
                  eh_post(ERR_CONFIRM, "Backup");
                  break ;


        case 'e':                         /* Backup Pickrate Entry Files     */

                  sd_wait();
                  if (*td == 't') system("backupmm_tape_pick");
                  else            system("backupmm_pick");
                  sd_cursor(0, 19, 1);
                  sd_clear_rest();
                  eh_post(ERR_CONFIRM, "Backup");
                  break ;

        case 'b':                         /* Restore SKU                     */

                  sd_wait();
                  if (*td == 't') system("restoremm_tape_sku");
                  else            system("restoremm_sku");
                  sd_cursor(0, 19, 1);
                  sd_clear_rest();
                  eh_post(ERR_CONFIRM, "Restore");
                  break ;

        case 'd':                         /* Restore config                  */

                  sd_wait();
                  if (*td == 't') system("restoremm_tape_config");
                  else            system("restoremm_config");
                  sd_cursor(0, 19, 1);
                  sd_clear_rest();
                  eh_post(ERR_CONFIRM, "Restore");
                  break ;

        case 'f':                         /* Restore pick rate               */
                  sd_wait();
                  if (*td == 't') system("restoremm_tape_pick");
                  else            system("restoremm_pick");
                  sd_cursor(0, 19, 1);
                  sd_clear_rest();
                  eh_post(ERR_CONFIRM, "Reload");
                  break ;

        default:  eh_post(ERR_CODE, buf);
                  continue;
      }                                   /* end switch                      */
      break;
    }
  }                                       /* end forever                     */
}                                         /* end main                        */

/*
 *open all files
 */
open_all()
{
  sd_open(leave);
  ss_open();
}
/*
 *close all files
 */
close_all()
{
  ss_close();
  sd_close();
}
/*
 *transfer control back to calling program
 */
void leave()
{
  close_all();
  execlp("syscomm","syscomm",0);
  krash("leave", "syscomm load", 1);
}

/* end of backupmm.c */
