/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    General purpose screen display of database reports.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/03/93   |  tjt  Added to mfc
 *  10/02/95   |  tjt  Bug fix in fseek.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char database_report_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      database_report.c                                               */
/*                                                                      */
/*      this program replaces the following report programs:            */
/*                                                                      */
/*              item_move_anal_report.c                                 */
/*              pick_loc_anal_report.c                                  */
/*              ant_stockout_report.c                                   */
/*              stock_status_report.c                                   */
/*              restock_report.c                                        */
/*                                                                      */
/*                                                                      */
/*      arguments to main are:                                          */
/*                                                                      */
/*              argv[1] is the string to be used for display.           */
/*              argv[2] is the filename to open.                        */
/*                                                                      */
/*              argv[3] is the width of the lines (replaces width)      */
/*              argv[4] is the row where the display begins.            */
/*              argv[5] is the display size.                            */
/*                                                                      */
/*              argv[6] is the name of the print.h to be used for print.*/
/*              argv[7] is the program to return to.                    */
/*                                                                      */
/*              argv[8] is the first non-repeating display (or "0")     */
/*              argv[9] is the row where argv[8] starts.                */
/*              argv[10] is the col where argv[8] starts.               */
/*                                                                      */
/*              argv[8] thru argv[10] is repeated for every non-        */
/*              repeating display item.                                 */
/*              argv[8], argv[11] and so forth are passed to print.     */
/*              (up to argv[14])                                        */
/*              these arguments are passed in the order that they       */
/*              will be processed by print. if arg is to be printed     */
/*              but not displayed on the screen, send row, col args     */
/*              of ascii zero ("0");                                    */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "language.h"

extern leave();
short ONE = 1;

struct fld_parms fld =

{23,20,1,1,&ONE,"Print? (y/n)          (Exit, Forward, or Backward)",'a'};

short width;
long savefp=0;

char crash_mess[80];
char input_file[16], print_file[16];

FILE *bug;

main(argc,argv)
int argc;
char **argv;
{
  long pid, status;
  short display_start;
  short display_size;
  FILE *fp;
  short rm,ret;
  short i,n;
  unsigned char t;
  char buf[2];
  short dflag;
  short row,col;

  putenv("_=database_report");
  chdir(getenv("HOME"));

#ifdef DEBUG
  bug = fopen("rpt_bug", "w");

  for (i = 0; i < argc; i++)
  {
    fprintf(bug, "argv[%d] = %s\n", i, argv[i]);
  }
  fflush(bug);
#endif
  
  open_all();
  
  fix(argv[1]);
  fp            = fopen(argv[2], "r");
  width         = atoi(argv[3]);
  display_start = atoi(argv[4]);
  display_size  = atoi(argv[5]);
  strcpy(input_file, argv[7]);

  sd_screen_off();
  sd_clear_screen();
  sd_text(argv[1]);
  sd_screen_on();

        /* see if display special non-repeating elements */

  for(n = 8; !streql(argv[n],"0"); n += 3)
  {
    row=atoi(argv[n+1]);
    col=atoi(argv[n+2]);
    if(row && col)
    {
      sd_cursor(0,row,col);
      sd_text(argv[n]);
    }
  }

  dflag=1;                                /* display forward                 */
  while(1)                                /* main loop to display data       */
  {
    sd_cursor(0, display_start, 1);
    sd_clear_rest();                      /* clear remainder of screen       */
    sd_cursor(0, display_start, 1);
    show(fp, display_size, dflag);
    for(i=0; i < 2; i++) buf[i] = 0;

    while(1)                              /* loop to process print prompt    */
    {
      sd_prompt(&fld, 0);
      t = sd_input(&fld,0,&rm,buf,0);
      switch(ret=(sd_print(t, code_to_caps(buf[0]))))  /* F041897 */
      {
        case(0):
        fclose(fp);
        unlink(argv[2]);                 /* DELETE INPUT ARG                */
        close_all();
        sd_close();
        execlp(argv[7],argv[7],0);
        sprintf(crash_mess,"%s load",argv[7]);
        krash("main", crash_mess, 1);

        case(1):
          dflag=1;
          break;

        case(2):
          dflag=2;
          break;

        case(3):
          fclose(fp);
          unlink(argv[2]);
          close_all();
          sd_close();
          execlp(argv[7],argv[7],0);
          sprintf(crash_mess,"%s load",argv[7]);
          krash("main",crash_mess,1);

        case(4):
          fclose(fp);
          close_all();

                                /* print file and leave */

          if (fork() == 0)               /* child process                   */
          {
            close_all();
          
            if(streql(argv[8],"0"))
            {
              execlp("prft","prft",
                argv[2],tmp_name(print_file),
                argv[6],0);
              krash("main","prft load", 0);
            }
            else if(streql(argv[11],"0"))
            {
              execlp("prft","prft",
                argv[2],tmp_name(print_file),
                argv[6],argv[8],0);
              krash("main", "prft load", 0);
            }
            else if(streql(argv[14],"0"))
            {
              execlp("prft","prft",
                argv[2],tmp_name(print_file),
                argv[6],argv[8],argv[11],0);
              krash("main", "prft load", 0);
            }
            else                          /* pass 3 extra arguments          */
            {
              execlp("prft","prft",
                argv[2],tmp_name(print_file),
                argv[6],argv[8],argv[11],
                argv[14],0);
              krash("Main","prft load", 1);
            }
          }
          else                            /* parent process                  */
          {
            pid = wait(&status);
            close_all();
            sd_close();
            execlp(argv[7],argv[7],0);
            sprintf(crash_mess, "%s load",argv[7]);
            krash("main", crash_mess, 1);
          }
        case(6):                          /* error                           */
          eh_post(ERR_YN,0);
          break;
      }
      if(ret != 6)                        /* not meaningless                 */
      break;
    }
  }
}

/****************************************************************************/
/*function to display x number of lines of data on the screen               */
/* Arguments:                                                               */
/*           fp : the data file pointer.                                    */
/*           lines : the number of lines to be displayed.                   */
/*           i : the indicator of either going forward or                   */
/*           backward on the file.                                          */
/*                                                                          */
/* returns : 1 if successfull                                               */
/*           0 if failed                                                    */
/****************************************************************************/

show(fp, lines, index)
FILE *fp;
short lines,index;
{
  register long pos, size;
  char str[1924];

  memset(str, 0, 1924);

  pos = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);

#ifdef DEBUG
  fprintf(bug, "show(lines=%d, index=%d) savefp=%d pos=%d size=%d\n", 
    lines, index, savefp, pos, size);
  fflush(bug);
#endif
  
  if(index == 2)
  {
    pos = savefp - lines * width;
    if(pos < 0) pos = 0;
    savefp = pos;

    fseek(fp, pos, 0);                     /* position the file pointer      */
    fread(str, width, lines, fp);
    sd_clear_rest();

#ifdef DEBUG
  fprintf(bug, "index=%d  row=%d  col=%d  pos=%d\n", 
    index, sd_row, sd_col, pos);
  Bdumpf(str, width * lines, bug);  
  fflush(bug);
#endif

    sd_text(str);
    return(1);
  }
  else if(index == 1)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;
    fseek(fp, pos, 0);
    fread(str, width, lines, fp);
    sd_clear_rest();

#ifdef DEBUG
  fprintf(bug, "index=%d  row=%d  col=%d  pos=%d\n", 
    index, sd_row, sd_col, pos);
  Bdumpf(str, width * lines, bug);
  fflush(bug);
#endif

    sd_text(str);
    return(1);
  }
  else return(0);
}

open_all()
{
  ss_open();
  co_open();
  sd_open(leave);
}

close_all()
{
  co_close();
  ss_close();
}

leave()
{
  close_all();
  sd_close();
  execlp(input_file, input_file, 0);
  sprintf(crash_mess, "%s load", input_file);
  krash("leave", crash_mess, 1);
}

/* end of database_report.c */
