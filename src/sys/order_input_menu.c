/*-------------------------------------------------------------------------*
 *  Custom:         CANTON - Warning on upload transactions.
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order input menu for commo or diskette.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/13/93   |  tjt  Added to mfc.
 *  07/13/95   |  tjt  Check transaction output conflict.
 *  07/14/95   |  tjt  Check commo is 'n'.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char order_input_menu_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "order_input_menu.t"
#include "eh_nos.h"
#include "language.h"

extern leave();

FILE *fp;

short ONE =1;
short NINETEEN = 19;

char menu[16]  = "com_menu_";
char caller[20]  = "order_input_menu";

char *command  = "doscp A:ORDERS.DAT otext/orders.dat 2>/dev/null";
char message[128];

struct fld_parms fld[] = {
  {12,57,32,1,&ONE,     "Enter Code",'a'},
  {13,57,32,1,&ONE,     "Purge Orders? (y/n)",'a'},
  {15,57,32,1,&NINETEEN,"Purge Date/Time",'a'},
  {18,57,32,1,&ONE,     "Drive Ready? (y/n)",'a'},
  {17,57,32,1,&ONE,     "Trans Uploaded? (y/n)", 'a'},
};

char blanks[42];

main()
{
  register long k;
  unsigned char t, incode[2], ans[2], yn[2];
  unsigned char timestr[30], datetime[15], intime[20];
  long systime, pid, status;

  putenv("_=order_input_menu");
  chdir(getenv("HOME"));
  
  sd_open(leave);
  ss_open();

  fix(order_input_menu);
  sd_screen_off();
  sd_clear_screen();
  sd_text(order_input_menu);
  sd_screen_on();
  
  sd_prompt(&fld[0], 0);
  memset(blanks, 0x20, sizeof(blanks));
  
  while(1)
  {
    for (k = 13; k <= 18; k++)
    {
      sd_cursor(0, k, 1);
      sd_clear_line();
    }
    while(1)                              /* initial prompt loop & checks    */
    {
      memset(incode, 0, 2);

      t = sd_input(&fld[0], 0, 0, incode, 0);
      if(t == EXIT)      leave();
      if(t == UP_CURSOR) continue;

      *incode = tolower(*incode);

      if(*incode == 'q') continue;        /* put the code for stop comms     */
                /* the meaning of the line, above, is unclear - tjt 8/13/93 */

      if (!*incode) break;

      if (*incode != 'c' && *incode != 'd' && *incode != 't')
      {
        eh_post(ERR_CODE, incode);
        continue;
      }
      if (*incode == 'c' || *incode == 'd')
      {
        if (sp->sp_oi_mode !=  0x20 || *incode == sp->sp_to_mode) /* F071395 */
        {
          eh_post(LOCAL_MSG, "Device/Port In Use");
          continue;
        }
      }
      if (*incode == 'd' || *incode == 't')
      {
        if (sp->sp_order_input_anytime == 'n' && sp->sp_config_status == 'n')
        {
          eh_post(ERR_NO_CONFIG, 0);
          continue;
        }
      }
      break;
    }
    sd_prompt(&fld[1], 0);

    while(1)                              /* purge prompts & setup           */
    {
      memset(ans, 0, 2);
      memset(yn, 0, 2);
      
      t = sd_input(&fld[1], 0, 0, yn, 0);
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
    if (t == UP_CURSOR) continue;

    if(*ans == 'y')
    {
      sd_prompt(&fld[2], 0);
      systime = time(0) - sp->sp_purge_window;
      strcpy(timestr, ctime(&systime));
      timestr[24] = 0;
      sd_cursor(0, 14, 32);
      sd_text("Default Date/Time:");
      sd_cursor(0, 14, 57);
      sd_text(&timestr[4]);
      sd_cursor(0, 16, 32);
      sd_text("Format is:");
      sd_cursor(0, 16, 57);
      sd_text("mm/dd/yyyy hh:mm:ss");

      while(1)                            /* actual purge done here          */
      {
        t = sd_input(&fld[2], 0, 0, intime, 0);
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
#ifdef CANTON
        while(1)
        {
          *ans = 0; *yn = 0;
          t = sd_input(&fld[4],sd_prompt(&fld[4],0),0,yn,0);
          if (t == EXIT)      leave();
          if (t == UP_CURSOR) break;

          *ans = code_to_caps(*yn);
          if(*ans == 'y' || *ans == 'n') break;
      
          eh_post(ERR_YN,0);
        }
        if (*ans != 'y') 
        {
          eh_post(LOCAL_MSG, "Order Purge ABORTED");
          break;
        }
#endif

        sd_wait();
        
        if ((pid = fork()) == 0)          /* start purge                     */
        {
          sprintf(datetime, "%d", systime);/* to ascii                       */
          ss_close();
          execlp("order_purge","order_purge", datetime, 0);
          setpgrp();                      /* so exit will not kill tty_server*/
          exit(99);
        }
        else pid = wait(&status);

        if (pid > 0 && !status) eh_post(ERR_CONFIRM, "Order purge");
        else eh_post(CRASH_MSG, "order_purge failed");
        break;
      }
      if (t == UP_CURSOR) continue;
    }
    if (*incode == 'c')                   /* RJE comms order input           */
    {
      if (sp->sp_commo_orders_in == 'n')  /* check commo feature lockout     */
      {
        eh_post(LOCAL_MSG, "No Communications Feature");
        continue;
      }
      sp->sp_oi_mode = 'c';
      if (sp->sp_commo_orders_in == 'k')
      {
        sd_close(); ss_close();
        execlp("com_kermit_in", "com_kermit_in", 0);
      }
      else
      {
        sd_close(); ss_close();
        execlp("com_menu_in", "com_menu_in", 0);
      }
      sd_open(leave); ss_open();
      sprintf(message, "Program %s not found");
      eh_post(LOCAL_MSG, message);
      sp->sp_oi_mode = 0x20;              /* needed if error occurred        */
      continue;
    }
    if (*incode == 'd')                   /* diskette order_input            */
    {
      while(1)
      {
        *ans = *yn = 0;
        t = sd_input(&fld[3],sd_prompt(&fld[3],0),0,yn,0);
        if (t == EXIT)      leave();
        if (t == UP_CURSOR) break;

        *ans = code_to_caps(*yn);
        if(*ans != 'y' && *ans != 'n')
        {
          eh_post(ERR_YN,0);
          continue;
        }
        if (*ans == 'n') break;               /* abort diskette in           */
        break;
      }                                   /* end forever                     */
      if (t == UP_CURSOR || *ans == 'n') continue;

      sd_wait();

      sp->sp_oi_mode = 'd';

      status = system(command);
      sp->sp_oi_mode = 0x20;              

      if (status) 
      {
        eh_post(CRASH_MSG, "Diskette input failed");
        continue;
      }
    }
    if (*incode == 't' || *incode == 'd')
    {
      if (*incode == 't') sd_wait();

      fp = fopen("otext/orders.dat", "r");
      if (fp == 0)
      {
        eh_post(LOCAL_MSG, "No otext/orders.dat file exists");
        continue;
      }
      fclose(fp);

      sp->sp_oi_mode = 't';
      status = system("order_input <otext/orders.dat 2>/dev/null");
      sp->sp_oi_mode = 0x20;              

      if (status) 
      {
        eh_post(CRASH_MSG, "Order_input has errors");
        continue;
      }
      else eh_post(ERR_CONFIRM, "Order input");
       
      sp->sp_oi_mode = 0x20;
    }
  }
}
/*-------------------------------------------------------------------------*
 * function to transfer control back to the calling program
 *-------------------------------------------------------------------------*/
leave()
{
  sd_close();
  ss_close();
  execlp("operm", "operm", 0);
  krash("order_input_menu", "load operm", 1);
}

/* end of order_input_menu.c */
