/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Transaction output.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/13/93   |  tjt  Added to mfc.
 *  08/09/94   |  tjt  Fix transaction directory.
 *  07/13/95   |  tjt  Add Check order input conflict too.
 *  07/14/95   |  tjt  Add Kermit output option.
 *  07/14/95   |  tjt  Add Check transaction file feature on purge.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char transac_output_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "file_names.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "xt.h"
#include "eh_nos.h"
#include "transac_output.t"
#include "language.h"

#include "Bard.h"

extern leave();

struct trans_item xt;

short ONE =1;
short NINETEEN = 19;

FILE *fd;
FILE *bug;

char menu[20]   = "com_menu_";
char caller[20] = "transac_output";

char *command   = "doscp dat/files/zt A:TRANSACT.DAT";
char message[128];


struct fld_parms fld[] = {
  {14,54,25,1,&ONE,     "Enter Code",'a'},
  {15,54,25,1,&ONE,     "Drive Ready?  (y/n)",'a'},
  {16,54,25,1,&ONE,     "Retransmit?   (y/n)",'a'},
  {17,54,25,1,&ONE,     "Purge Orders? (y/n)",'a'},
  {19,54,25,1,&NINETEEN,"Purge Date/Time",'a'},
  {15,54,25,1,&ONE,     "Are You Sure? (y/n)", 'a'}
};

main()
{
  register long k;
  unsigned char t, incode[2], ans[2], yn[2], again[2];
  unsigned char timestr[30], datetime[15], intime[20];
  long systime;
  long pid, status;

  putenv("_=transac_output");
  chdir(getenv("HOME"));

  open_all();

  fix(transac_output);
  sd_screen_off();
  sd_clear_screen();
  sd_text(transac_output);
  sd_screen_on();
  
  sd_prompt(&fld[0], 0);

  while(1)
  {
    for (k = 15; k <= 20; k++)
    {
      sd_cursor(0, k, 1);
      sd_clear_line();
    }
    while(1)
    {
      memset(incode, 0, 2);

      t = sd_input(&fld[0], 0, 0, incode, 0);
      if(t == EXIT)      leave();
      if(t == UP_CURSOR) continue;

      if (!*incode) break;

      *incode = tolower(*incode);
        
      if (*incode != 'c' && *incode != 'd' &&
          *incode != 'p' && *incode != 'e')
      {
        eh_post(ERR_CODE, incode);
        continue;
      }
      break;
    }
    if (*incode == 'p')
    {
      if (sp->sp_to_flag != 'y' && sp->sp_to_flag != 'b')
      {
        eh_post(LOCAL_MSG, "No Transaction File Feature");
        continue;
      }
      sd_wait();
      
      if((pid = fork()) == 0)
      {
        ss_close();
        execlp("transac_short_rpt", "transac_short_rpt", 0);
        exit(1);
      }
      else pid = wait(&status);

      if (pid > 0 && !status) eh_post(ERR_CONFIRM, "Short Printing");
      else eh_post(CRASH_MSG, "tranac_short_rpt failed");
      continue;
    }
    if (*incode == 'e')
    {
      if (sp->sp_to_flag != 'y' && sp->sp_to_flag != 'b')
      {
        eh_post(LOCAL_MSG, "No Transaction File Feature");
        continue;
      }
      while(1)
      {
        memset(ans, 0, 2);                /* are you sure response           */
        memset(yn, 0, 2);
        
        t = sd_input(&fld[5],sd_prompt(&fld[5],0), 0, yn, 0);
        if (t == EXIT)      leave();
        if (t == UP_CURSOR) break;

        *ans = code_to_caps(*yn);

        if(*ans != 'y' && *ans != 'n')
        {
          eh_post(ERR_YN,0);
          continue;
        }
        if(*ans == 'y')
        {
          eh_post(LOCAL_MSG, "Purging Transaction File");

          database_open();
          xt_open();
          transaction_setkey(0);
          
          begin_work();
          while (!transaction_next(&xt, LOCK)) 
          {
            transaction_delete();
            commit_work();
            begin_work();
          }
          commit_work();
          xt_close();
          database_close();
          
          sp->sp_to_count = 0;
          
          eh_post(PURGE_TRANS, 0);
        }
        break;
      }                                   /* end while(1)                    */
      continue;
    }                                     /* end if                          */
    if (*incode == 'c' || *incode == 'd')
    {
      if (sp->sp_to_mode != 0x20 || sp->sp_oi_mode == *incode) /* F071395    */
      {
        eh_post(LOCAL_MSG, "Device/Port In Use");          
        continue;
      }
    }
    if (*incode == 'd')
    {
      while(1)
      {
        memset(ans, 0, 2);
        memset(yn, 0, 2);
        
        t = sd_input(&fld[1],sd_prompt(&fld[1],0), 0, yn, 0);
        if (t == EXIT)     leave();
        if (t == UP_CURSOR) break;

        *ans = code_to_caps(*yn);
        
        if(*ans != 'y' && *ans != 'n')
        {
          eh_post(ERR_YN,0);
          continue;
        }
        break;
      }
      if (t == UP_CURSOR) continue;
      if (*ans == 'n') continue;          /* abort diskette in               */
    }
    while(1)
    {
      memset(again, 0, 2);                /* retransmit response             */
      memset(yn, 0, 2);
      
      t = sd_input(&fld[2],sd_prompt(&fld[2],0), 0, yn, 0);
      if (t == EXIT)     leave();
      if (t == UP_CURSOR) break;

      *again = code_to_caps(*yn);
      if (*again != 'y' && *again != 'n')
      {
        eh_post(ERR_YN, 0);
        continue;
      }
      break;
    }
    if (t == UP_CURSOR) continue;

    while(1)
    {
      memset(ans, 0, 2);                  /* purge response                  */
      memset(yn, 0, 2);
      
      t = sd_input(&fld[3],sd_prompt(&fld[3],0), 0, yn, 0);
      if(t == EXIT)      leave();
      if(t == UP_CURSOR) break;

      *ans = code_to_caps(*yn);
      if(*ans != 'y' && *ans != 'n')
      {
        eh_post(ERR_YN,0);
        continue;
      }
      break;
    }
    if(*ans == 'y')
    {
      sd_prompt(&fld[4],0);
      systime = time(0) - sp->sp_purge_window;
      strcpy(timestr, ctime(&systime));
      timestr[24] = 0;
      sd_cursor(0,18,25);
      sd_text("Default Date/Time:");
      sd_cursor(0,18,54);
      sd_text(&timestr[4]);
      sd_cursor(0,20,25);
      sd_text("Format is:");
      sd_cursor(0,20,54);
      sd_text("mm/dd/yyyy hh:mm:ss");

      while(1)
      {
        t = sd_input(&fld[4], 0, 0, intime, 0);
        if(t == EXIT)      leave();
        if(t == UP_CURSOR) break;

        if(*intime != 0)
        {
          if(!time_convert(intime, &systime))
          {
            eh_post(ERR_TIME, intime);
            continue;
          }
        }
        eh_post(LOCAL_MSG, "Purging Orders");

        if ((pid = fork()) == 0)
        {
          sprintf(datetime, "%d", systime);
          ss_close();
          execlp("order_purge", "order_purge", datetime, 0);
          exit(1);
        }
        else pid = wait(&status);

        if (pid > 0 && !status) eh_post(ERR_CONFIRM, "Order purge");
        else eh_post(CRASH_MSG, "order_purge failed");
        break;
      }
      if (t == UP_CURSOR) continue;
    }
    if (*again == 'n')                    /* transmit current file           */
    {
      eh_post(LOCAL_MSG, "Extracting Transactions");

      if ((pid = fork()) == 0)
      {
        ss_close();
        execlp("transac_copy", "transac_copy", 0);
        exit(1);
      }
      else pid = wait(&status);

      if (pid < 0 || status)
      {
        eh_post(LOCAL_MSG, "transac_copy failed");
        continue;
      }
      eh_post(ERR_CONFIRM, "Transaction File");
    }
/*
 *  Start Transaction Output Operations
 */
    if (*incode == 'c')                   /* comm output                     */
    {
      if (sp->sp_commo_trans_out == 'n')
      {
        eh_post(LOCAL_MSG, "No Communication Feature");
        continue;
      }
      eh_post(LOCAL_MSG, "Sending Transactions");
      sd_close();

      sp->sp_to_mode = 'c';
      
      if (fork() == 0)
      {
        if (sp->sp_commo_trans_out == 'k') 
        {
          ss_close();
          execlp("com_kermit_out", "com_kermit_out", 0);
        }
        else 
        {
          ss_close();
          execlp("comsend",  "comsend",  0);
        }
        ss_open();
        sd_open();
        sp->sp_to_mode = 0x20;
        eh_post(LOCAL_MSG, "Program Not Found");
        continue;
      }
      pid = wait(&status);
      sp->sp_to_mode = 0x20;
      sd_open();
      
      if (pid < 0 || status) 
      {
        eh_post(LOCAL_MSG, "Communications Failed");
      }
      else eh_post(ERR_CONFIRM, "Transactions Output");
      continue;
    }
    if (*incode == 'd')
    {
      sd_wait();

      sp->sp_to_mode = 'd';

      sprintf(message, command);
      status = system(message);
      sp->sp_to_mode = 0x20;
      
      if (status) eh_post(LOCAL_MSG, "Diskette output failed");
      else eh_post(ERR_CONFIRM, "Tranaction Output");
    }
  }
}
/* function to transfer control back to the calling program*/
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
leave()
{
  close_all();
  execlp("operm", "operm", 0);
  krash("leave", "load operm", 1);
}


/* end of transac_output.c */
