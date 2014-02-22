#define DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Picker Login Status by Zone.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/01/01   |  aha  Created program.
 *-------------------------------------------------------------------------*/
static char picker_login_rpt_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "picker_login_rpt.t"
#include "global_types.h"
#include "Bard.h"
#include "bard/picker.h"
#include "language.h"

FILE *fp;
char filename[16];

#define MAXLINE         80

#ifdef DEBUG
FILE *DF;
#define DF_SIZE 4000000
#endif

#define REPORT_HEADER_FILE "sys/report/picker_login_rpt.h"
#define WIDTH        79
#define FIRST_LIST_REPORT_LINE 8

#undef PRINT

static short ONE  = 1;

struct fld_parms fld = 
       {23,17,1,1,&ONE,  "Print? (y/n)      (Exit, Forward, or Backward)",'a'};

static short rm = 0;  /* no one changes this, but we do need to be able to take
                         its address, so to avoid everyone declaring their own
                         useless copy, we make one global useless copy... */


main(argc, argv)
int argc;
char *argv[];
{
  char text[MAXLINE];

  putenv("_=picker_login_rpt");
  chdir(getenv("HOME"));

  open_all();

  memset(text, 0x0, MAXLINE);
#ifdef DEBUG
  fprintf(DF, "Picker_logn_rpt started.\n");
  fflush(DF);
#endif

/*
 ** do setup and create the data on the screen
 */
  memcpy(picker_login_rpt + 267, "    Zone Login Status    ", 25);
  picker_login_rpt[76] = '8';
      
  sd_screen_off();
  fix(picker_login_rpt);
  sd_clear_screen();
  sd_text(picker_login_rpt);
  sd_screen_on();
#ifdef DEBUG
  fprintf(DF, "Screen is set up.\n");
  fflush(DF);
#endif
  
  /*
  ** get a temporary file to use
  */
  tmp_name(filename);
  if (!(fp = fopen(filename, "w+")))
  {
    eh_post(ERR_OPEN, filename);
    leave();
  }
#ifdef DEBUG
  fprintf(DF, "File opened, filename = %s\n", filename);
  fflush(DF);
#endif

  /*
  ** show report specific titles and process report
  */
  
  sprintf(text, "%s%5s%s%21s%s%17s",
          "Zone No.",
          " ",
          "Employee No.",
          " ",
          "Name",
          " ");
#ifdef DEBUG
  fprintf(DF, "text = %s\n", text);
  fflush(DF);
#endif
  sd_cursor(0, 6, 7);
  sd_text(text);
  memset(text, 0x0, MAXLINE);
  sprintf(text, "%s%5s%s%5s%s",
          "--------", " ",
          "------------", " ",
          "-------------------------------------");
  sd_cursor(0, 7, 7);
  sd_text(text);
#ifdef DEBUG
  fprintf(DF, "Screen titles set up.\n");
  fflush(DF);
#endif

  list_zones(fp, filename);
      
  /*
  ** clean up and exit
  */

  leave();
}
/* end of main() */


/*
** create and show the report listing zone login status 
*/
list_zones(fp, filename)
FILE *fp;
char *filename;
{
  char name[40];
  unsigned short int length    = 0,
                     n         = 0,
                     num_space = 0;
  picker_item picker_record;
  struct zone_item *z;
  TZone zone_num = 0;
  long int picker_id = 0L;
#ifdef DEBUG
  fprintf(DF, "In list_zones().\n");
  fprintf(DF, "filename = %s\n", filename);
  fflush(DF);
#endif
 
  if (sp->sp_config_status != 'y')
     {
       eh_post(ERR_NO_CONFIG);
       return;
     }

  for (n = 0; n < coh->co_zone_cnt; n++)
      {
        memset(&picker_record, 0x0, sizeof(picker_item));

        z = &zone[n];

        zone_num  = z->zt_zone;
        picker_id = z->zt_picker;

        if (picker_id == 0L)
           {
             strcpy(name, "(none)");
           }
        else
           {
             begin_work();
             picker_query(&picker_record, picker_id);
             commit_work();

             strip_space(picker_record.p_last_name, 16);
             strip_space(picker_record.p_first_name, 16);
             strip_space(picker_record.p_middle_initial, 2);

             sprintf(name, "%s, %s %s", picker_record.p_last_name, 
                                        picker_record.p_first_name,
                                        picker_record.p_middle_initial);
           }

        length = strlen(name);

        if (length < 37)
           {
              num_space = 37 - length;
              memset(name + length + 1, 0x20, num_space);
              name[37] = '\0'; 
           }

        fprintf(fp, "%6s%8d%5s%12d%5s%-37.37s%6s\n",
                " ",
                zone_num, " ",
                picker_id, " ",
                name, " ");

        fflush(fp);
      } /* end of for loop for all zones */

  /*
  ** show report to user and print if desired
  */
  display_data(fp, REPORT_HEADER_FILE, FIRST_LIST_REPORT_LINE+rm,
               fld.irow - FIRST_LIST_REPORT_LINE, filename, 0, 0);

  return;
} /* end of list_zones() */


/*--------------------------------------------------------------------------**
** display the data of a report to the user
**--------------------------------------------------------------------------*/
display_data(fp, header_file, beg_line, num_lines, filename, msg1, msg2)
FILE *fp;                               /* the data to display               */
char *header_file;                      /* header file for printed report    */
int beg_line;                  /* the first line to use on the user's monitor*/
int num_lines;                  /* the number of lines to display at one time*/
char *filename;                         /* the name of the file with the data*/
char *msg1;                         /* a string to include on printed report */
char *msg2;                         /* a string to include on printed report */
{
  char buffer[8];                       /* input buffer space                */
  int row_modifier = 0;                 /* for sd_input                      */
  unsigned char terminator;             /* key that caused sd_input to return*/

  fseek(fp, 0, 0);
  sd_cursor(0, beg_line, 1);
  show(fp, num_lines, 1);

  for (;;)
  {
    buffer[0] = buffer[1] = '\0';
    sd_prompt(&fld, 0);
    terminator = sd_input(&fld,
                          row_modifier,
                          &row_modifier,
                          buffer, 
                          0);

    switch(sd_print(terminator, code_to_caps(buffer[0])))
    {
      case(3) :                         /* user answered NO                  */
      case(0) : unlink(filename);       /* user pressed EXIT                 */
                fclose(fp);
                return;

      case(1) : sd_cursor(0,beg_line,1);/* scroll forward                    */
                sd_clear_rest();
                sd_cursor(0,beg_line,1);
                show(fp,num_lines,1);
                break;

      case(2) : sd_cursor(0,beg_line,1);/* scroll backward                   */
                sd_clear_rest();
                sd_cursor(0,beg_line,1);
                show(fp,num_lines,2);
                break;

      case(4) : print(filename, header_file, msg1, msg2); /* user ans.d YES */
                fclose(fp);
                return;
    }
  }
} /* end of display_data() */


/*--------------------------------------------------------------------------*/
/* function to display x number of lines of data on the screen              */
/* Arguments:                                                               */
/*           fp : the data file pointer.                                    */
/*           lines : the number of lines to be displayed.                   */
/*           i : the indicator of either going forward or                   */
/*           backward on the file.                                          */
/*--------------------------------------------------------------------------*/
show(fp,lines,index)
FILE *fp;
short lines,index;
{
  long position, size;
  static long savefp = 0;
  char str[1920];

  memset(str, 0, 1920);

  position = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);

  if (index == 2)
  {
    position = savefp - lines * WIDTH;
    if(position < 0) position = 0;
    savefp = position;

    fseek(fp, position, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else if(index == 1)
  {
    if (position >= size) position = savefp;
    savefp = position;

    fseek(fp, position, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else return(0);
} /* end of show() */


/*--------------------------------------------------------------------------**
** print the data
**--------------------------------------------------------------------------*/
print(filename, header_file, msg1, msg2)
char *filename;                         /* name of the file with the data    */
char *header_file;                      /* name of the header file for report*/
char *msg1;                         /* a string to be included on the report */
char *msg2;                         /* a string to be included on the report */
{
  char print_file[128];
  int pid, status;

  if (fork() == 0)                      /* if child process                  */
  {
    close_all();
    execlp("prft", "prft", filename, tmp_name(print_file),
		  header_file, msg1, msg2, 0);
    krash("print", "prft load", 0);
    exit(1);
  }
  else                                  /* parent process                    */
  {
    pid = wait(&status);
    if (pid < 0 || status) krash("print", "prft failed", 1);
  }
} /* end of print() */

/*
 *transfer control back to calling program
 */
leave()
{
  close_all();
  unlink(filename);
  database_close();
  sd_close();
  execlp("picker_acctability","picker_acctability",0);
  krash("leave", "picker_acctability load", 1);
} /* end of leave() */


/*
 *open all files
 */
open_all()
{
  char command[MAXLINE];

  memset(command, 0x0, MAXLINE);

  database_open();

  sd_open(leave);
  ss_open();
  co_open();

  picker_open(READONLY);
  picker_setkey(1);

#ifdef DEBUG
  DF = fopen("debug/pick_login.bug", "w");
  if (ftell(DF) > DF_SIZE)
     {
        fclose(DF);
        sprintf(command, "%s  %s  %s",
        "mv -f debug/pick_login.bug",
        "debug/pick_login.bug.save",
        "1>/dev/null 2>&1");
        system(command);
        DF = fopen("debug/pick_login.bug", "w");
     }
#endif
  
  return;
} /* end of open_all() */


/*
 *close all files
 */
close_all()
{
  picker_close();
  ss_close();
  co_close();
#ifdef DEBUG
  if (DF) fclose(DF);
#endif
  return;
} /* end of close_all() */


/* end of picker_login_rpt.c */
