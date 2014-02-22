/*----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Control print operations
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/8/93    |  tjt  Added to mfc.
 *  03/14/94   |  tjt  Modified for berkley lpc, lpq, and lprm.
 *  12/06/95   |  tjt  Modified for SCO UNIX.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char printer_control_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "message_types.h"
#include "language.h"
#include "berkeley_control.t"

extern leave();

#define NUM_PROMPTS     3
#define BUF_SIZE        5

short ONE = 1;
short FOUR = 4;
short EIGHT = 8;

struct fld_parms fld[] = {

  {14,50,27,1,&ONE,  "Enter Code",'a'},
  {16,50,27,1,&FOUR, "Enter File Number",'n'},
  {23,20,1,1,&ONE,   "More? (y/n)",'a'}

};
struct fld_parms fld1 = {16,50,27,1, &EIGHT, "Enter Printer", 'a'};

char command[80];
char printer[32];
FILE *t_fd;
char tname[16];

char x[25][80];
long max;
char *q;

main()
{
  short rm;
  short ret;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];        /* array of buffers                */

  short i;
  short n;

  putenv("_=printer_control");
  chdir(getenv("HOME"));

  open_all();

  while(1)                                /* begin massive loop              */
  {
    fix(berkeley_control);
    sd_screen_off();
    sd_clear_screen();                    /* clear screen                    */
    sd_text(berkeley_control);
    sd_screen_on();

                /* clear input buffers */

    for(i = 0; i < NUM_PROMPTS; i++)
    for(n = 0; n < BUF_SIZE; n++)
    buf[i][n] = 0;


                /* main loop to get code */

    while(1)
    {
      t = sd_input(&fld[0],sd_prompt(&fld[0],0),&rm,buf[0],0);

      if (t == EXIT) leave();
      if (t == UP_CURSOR) continue;

      *buf[0] = tolower(*buf[0]);      
      
      if (*buf[0] == 'a' || *buf[0] == 'c' || 
          *buf[0] == 'k' || *buf[0] == 's' || 
          *buf[0] == 'r' || *buf[0] == 'd')  break;
      
      eh_post(ERR_CODE, buf[0]);
    }
    if (*buf[0] == 'c')
    {
      t = sd_input(&fld1, sd_prompt(&fld1, 0), 0, op_printer, 0);
      if (t == EXIT) leave();
      if (t == UP_CURSOR) continue;
      
      message_put(sd_server, ChangePrinterEvent, op_printer, 
        strlen(op_printer));

      sprintf(printer, "PRINTER=%s", op_printer);
      putenv(printer);
      continue;
    }
                /* code entered. display second prompt */

    if(buf[0][0] != 'd')                  /* not display                     */
    {
      while (*buf[0] == 'k')
      {
        sd_prompt(&fld[1],0);
        t = sd_input(&fld[1],0,&rm,buf[1],0);
        if(t == EXIT) leave();
        else if(t == RETURN) break;
      }
      switch(buf[0][0])    
      {
        case 'a':                         /* abort current print job         */
        
          sd_wait();
          sd_cursor(0, 18, 27);
          sd_text(".");
          sprintf(command, "cancel %s", op_printer);
          system(command);
          sleep(2);
          break;

        case 'k':                         /* kill print file                 */
          
          sd_wait();
          sd_cursor(0, 18, 27);
          sd_text(".");
          sprintf(command, "cancel %s-%s", op_printer, buf[1]);
          system(command);
          sleep(2);
          break;
          
        case 's':                         /* stop print file                 */
          
          sd_wait();
          sd_cursor(0, 18, 27);
          sd_text(".");
          sprintf(command, "disable %s", op_printer);
          system(command);
          sleep(2);
          break;
          
        case 'r':                         /* restart printing                */
          
          sd_wait();
          sd_cursor(0, 18, 27);
          sd_text(".");
          sprintf(command, "enable %s", op_printer);
          system(command);
          sleep(2);
          break;
      }
      continue;
    }
    else                                  /* display request                 */
    {
      sd_cursor(0,16,2);
      sd_text("Job ID                  Operator          Size   Date");
    }
/*
 *  Get Printer Queue Data
 */
    tmp_name(tname);                      /* get temp name                   */
    sprintf(command, "lpstat -o %s >%s", op_printer, tname);
    system(command);

    t_fd = fopen(tname, "r");
    if (t_fd == 0) continue;              /* open failed                     */
    
    memset(x, 0, sizeof(x));
    
    for (max = 0; max < 25; max++)
    {
      if (!fgets(x[max], 80, t_fd)) break;
      n = strlen(x[max]) - 1;
      x[max][n] = 0;
    }
    n = 0;

    fclose(t_fd);
    unlink(tname);

    while(1)
    {
      sd_cursor(0, 17, 1);
      sd_clear_rest();

      for (i = 0; i < 5 && n + i < max; i++)
      {
        sd_cursor(0, 17 + i, 2);
        sd_text(x[i + n]);
      }
      sd_cursor(0,23,23);
      sd_text("(Exit, Forward, or Backward)");
      sd_prompt(&fld[2],0);

      t = sd_input(&fld[2],0,&rm,buf[2],0);

      ret = sd_more(t, code_to_caps(buf[2][0]));  /* F041897 */
                        
      if (ret == 0) leave();               /* exit                           */
      
      else if (ret == 1)                  /* forward                         */
      {
        if (n + 5 < max) n += 5;
      }
      else if (ret == 2)                  /* backward                        */
      {
        if (n > 0) n -= 5;
      }
      else if (ret == 3) break;           /* quit display                    */
    }
  }                                       /* end massive while(1)loop        */

}
open_all()
{
  ss_open();
  sd_open(leave);
  getparms(0);
}
close_all()
{
  sd_close();
  ss_close();
}
leave()
{
  close_all();
  execlp("syscomm", "syscomm", 0);
  krash("leave", "syscomm load", 1);
}

/* end of printer_control.c */
