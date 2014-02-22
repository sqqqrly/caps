/*------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Product file query screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/01/93   |  tjt  Added to mfc.
 *  07/01/94   |  tjt  Added query name to the report.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *  05/18/01   |  aha  Added fix BugLogID #27.
 *-------------------------------------------------------------------------*/
static char product_file_query_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/************************************************************************/
/*                                                                      */
/*      product_file_query.c                                            */
/*                                                                      */
/*      the 'blank' filename is composed of the directory name,         */
/*      'query' plus the operator name. it is considered as any other   */
/*      file except that it may be created even if it already exists.   */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include "file_names.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "language.h"
#include "product_file_query.t"

#define NUM_PROMPTS     6
#define BUF_SIZE        31
#define DISPLAY_SIZE    9                 /* # of lines of files to show     */

#define WIDTH 80

static short ONE = 1;
static short L30 = 30;

struct fld_parms fld[] ={

  {11,45,8,1,&ONE,"Enter Code",'a'},
  {12,45,8,1,&L30,"Enter Source Query Name",'a'},
  {13,45,8,1,&L30,"Enter Destination Query Name",'a'},
  {14,45,8,1,&ONE,"Are You Sure? (y/n)",'a'},
  {14,45,8,1,&ONE,"Print? (y/n)",'a'},
  {23,16,1,1,&ONE,"More? (y/n)      (Exit, Forward, or Back)",'a'}

};

long savefp=0;                            /* used by show function           */

char blank_file[16];                      /* default file                    */
char data_file0[16];                      /* file 0 holds result of SQL      */
char data_file1[16];
char filename0[33];                       /* primary query file              */
char filename1[33];
char cmd_buf[100];
char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */

long pid, status;

main()
{
  extern leave();

  FILE *fpr;
  FILE *fpw;
  short rm;
  unsigned char t;

  char c;

  short i;
  short n;
  short ret=1;
  short done=0;
  short loop_done=0;
  short clear_flag=0;                     /* for when display < 10 lines     */
                                /* 1 = clear input lines only */
  short dflag=0;                          /* used by display sequence        */
  short count=0;                          /* count of lines to display       */

  putenv("_=product_file_query");
  chdir(getenv("HOME"));

  open_all();

  tmp_name(blank_file);

  fpr = fopen(blank_file,"wct");
  fclose(fpr);

  fix(product_file_query);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(product_file_query);
  sd_screen_on();

  while(1)                                /* loop for multiple screens       */
  {
    sd_cursor(0,11,1);
    if(!clear_flag)                       /* if no display                   */
    {
      sd_clear_rest();                    /* clear rem. screen               */
    }
    for(n = 0; n < BUF_SIZE; n++)
    buf[0][n] = 0;
    done=0;
    sd_prompt(&fld[0],0);
                /* main loop to gather input */

    while(1)
    {
      t = sd_input(&fld[0],0,&rm,buf[0],0);
      if(clear_flag)
      {
        sd_cursor(0,12,1);                /* after input line                */
        sd_clear_rest();
        clear_flag=0;
      }
      if(t == EXIT)
      {
        leave();
      }
      else if(t != RETURN)
      continue;
      else
      ;
                        /* check input */
      *buf[0] = tolower(*buf[0]);

      switch(*buf[0])
      {
        case 'c':                         /* create a file                   */

          ce_file(filename0, buf[1], 1, 1);
          edit(filename0, "w");
          break;

        case 'e':                         /* change a file                   */

          ce_file(filename0, buf[1], 1, 0);
          ce_yn(3, buf[3]);
          if(*buf[3]=='n')
          {
            done=1;
            break;
          }
          edit(filename0, "r");
          break;

        case 's':                         /* save a file                     */

          ce_file(filename0, buf[1], 1, 1);
          if (strcmp(filename0, blank_file) != 0)
          {
            done=1;
            break;
          }
          fpr = fopen(blank_file, "r");
          fpw = fopen(filename0,  "w");
          while((c=getc(fpr))!=EOF) putc(c,fpw);
          fclose(fpr);
          fclose(fpw);
          done=1;
          break;

        case 'd':                         /* delete a file                   */

          ce_file(filename0, buf[1], 1, 0);
          ce_yn(3, buf[3]);
          if(*buf[3] == 'n')
          {
            done=1;
            break;
          }
          unlink(filename0);
          if (streql(buf[1], blank_file))
          {
            fpr = fopen(blank_file, "w");
            fclose(fpr);                  /* create blank file again         */
          }
          done=1;
          break;

        case 'p':                         /* copy a file                     */

          ce_file(filename0, buf[1], 1, 0);
          ce_file(filename1, buf[2], 2, 1);
          fpr = fopen(filename0, "r");
          fpw = fopen(filename1, "w");
          while((c=getc(fpr))!=EOF) putc(c,fpw);
          fclose(fpr);
          fclose(fpw);
          done=1;
          break;

        case 'r':                         /* run query                       */

          ce_file(filename0, buf[1], 1, 0);
          sprintf(cmd_buf,"SQL %s %s", filename0, tmp_name(data_file0));
          ce_yn(4, buf[4]);               /* print question                  */
          if (*buf[4] == 'y')
          {
            sd_cursor(0,15,1);            /* out of the way                  */
            if(fork()==0)                 /* this is child                   */
            {
              sd_wait();
              close_all();
              system(cmd_buf); 
              
              execlp("prft","prft", data_file0,
                tmp_name(data_file1),
                "sys/report/pfq_print_report.h", buf[1], 0);
              krash("main", "prft load", 1);
            }
            else                          /* this is parent                  */
            {
              pid = wait(&status);
              if (pid < 0 || status) krash("main", "prft failed", 1);
              done=1;
              break;
            }
          }
          else                            /* response must be no to print    */
          {
            sd_wait();
            system(cmd_buf);
            close_all();
            sd_close();
            execlp("pfq_report", "pfq_report", data_file0, buf[1], 0);
            krash("main", "pfq_report load", 1);
          }

        case 'l':
                                /* there are 9 lines available for display */
          count = ce_format(cmd_buf);
          if(!count)
          {
            done=1;
            break;
          }
                                /* filename is in cmd_buf */
          fpr = fopen(cmd_buf, "r");
          if(count<=DISPLAY_SIZE)         /* no more prompt                  */
          {
            sd_cursor(0,13,1);
            show(fpr,count,2);
            clear_flag=1;
            done=1;
            fclose(fpr);
            unlink(cmd_buf);
            break;
          }
                                /* implied else here */
          dflag=2;
          while(1)                        /* loop to display data            */
          {
            loop_done=0;
            sd_cursor(0,13,1);
            ret=show(fpr,DISPLAY_SIZE,dflag);
            if(ret)
            sd_clear_rest();
            memset(buf[5], 0, BUF_SIZE);

            sd_prompt(&fld[5],0);         /* More?                           */
            while(1)
            {
              t=sd_input(&fld[5],0,&rm,buf[5],0);
              done=0;
              switch(sd_more(t, code_to_caps(*buf[5]))) /* F041897 */
              {
                case(0):  leave();
                          break;
                case(1):  dflag=2;        /* forward                         */
                          done=1;
                          break;
                case(2):  dflag=1;       /* backwrd                          */
                          done=1;
                          break;
                case(3):  done=1;
                          loop_done=1;
                          clear_flag=0;
                          break;
                case(6):  eh_post(ERR_YN,0);
                          break;
              }                           /* end switch                      */
              if(done)
              break;
            }                             /* end more input loop             */
            if(loop_done)
            break;
          }                               /* end display loop                */
          fclose(fpr);
          strcpy(filename0, cmd_buf);
          unlink(filename0);             /* delete display                  */
          done=1;
          break;

          default:
          eh_post(ERR_CODE,buf[0]);
          break;
        }
      if(done)
      break;
    }
  }
}

/*****************************************/
/* functions for product_file_query      */
/*****************************************/

ce_file(filebuf,buf,i,u)                  /* function returns usable filename*/
char *filebuf;
char *buf;
short i;
short u;                                  /* u = 0 return existing file      */
                /* u = 1 return non-existing file */
{
  FILE *ret;
  short n;

  memset(buf, 0, BUF_SIZE);
  sd_prompt(&fld[i],0);
  while(1)
  {
    ce_name(buf,i);                       /* get a filename                  */

    sprintf(filebuf, "%s/%s", query_name, buf);

    if (strcmp(buf, blank_file) == 0) return;

    ret = fopen(filebuf,"r");

    if (ret) fclose(ret);

    if((ret && !u) || (!ret && u))  return;
    else
    {
      eh_post(ERR_FILE_INVALID, buf);
    }
  }
}

ce_name(buf,i)                            /* input a filename                */
register char *buf;
register long i;
{
  unsigned char t;

  while(1)
  {
    t = sd_input(&fld[i], 0, 0, buf, 0);
    if (t == EXIT) leave();
    else if (t == RETURN)
    {
      if (!*buf) strcpy(buf, blank_file);
      return;
    }
  }
}

ce_yn(prompt,buf)                         /* input yes or no response        */
register long prompt;
register char *buf;
{
  unsigned char t, yn[2];

  memset(buf, 0, BUF_SIZE);
  memset(yn, 0, 2);
  
  sd_prompt(&fld[prompt], 0);
  while(1)
  {
    t = sd_input(&fld[prompt],0, 0,yn,0);
    if (t == EXIT) leave();
    else if (t == RETURN)
    {
      *buf = code_to_caps(*yn);          /* F041897 */
      if(*buf != 'n' && *buf != 'y')
      {
        eh_post(ERR_YN,0);
      }
      else return;
    }
  }
}

/* ce_format:

        this routine lists the files in the query directory and formats
them to show 2 filenames on a line.

*/
ce_format(cmd_buf)                  /* returns true count of lines to display*/
char *cmd_buf;                            /* returns filename in cmd_buf     */
{
  FILE *fp;
  FILE *sp;
  char temp_file[2][33];
  char c;
  char tbuf[2][33];
  short count, save_count, odd, i, n;

  count=0;        odd=0;
  tmp_name(temp_file[0]);                 /* original listing                */
  tmp_name(temp_file[1]);                 /* final listing                   */

  sprintf(cmd_buf, "/bin/ls %s >%s", query_name, temp_file[0]);
  system(cmd_buf);                        /* create listing                  */
  fp = fopen(temp_file[0], "r");
  while((c=getc(fp)) != EOF)
  if(c==NEWLINE)
  count++;
  count--;                                /* subtract one for directory name */
  save_count = count/2;                   /* output count (more or less)     */
  fseek(fp, 0, 0);                        /* to beginning                    */
  while((c=getc(fp)) != NEWLINE)
  ;                                       /* skip directory name             */
  if(count % 2 == 1)                      /* odd number, and at least one    */
  {
    odd=1;
    count--;                              /* make count even (may be 0)      */
  }
  sp = fopen(temp_file[1],"wct");
  while(count)
  {
    for(i=0;i<2;i++)
    {
      n=0;
      while((c=getc(fp))!=NEWLINE)
      tbuf[i][n++]=c;
      tbuf[i][n] = 0;
    }
    fprintf(sp,"       %-35s%-37s\n",tbuf[0],tbuf[1]);
    count-=2;
  }
  if(odd)                                 /* could be first                  */
  {
    n=0;
    while((c=getc(fp))!=NEWLINE)
    tbuf[0][n++]=c;
    tbuf[0][n] = 0;
    fprintf(sp,"       %-72s\n",tbuf[0]);
  }
  fclose(fp);
  fclose(sp);
  unlink(temp_file[0]);                  /* delete original file            */
  strcpy(cmd_buf,temp_file[1]);           /* return formatted file           */
  return(save_count+odd);              /* return line count of formatted file*/
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

show(fp,lines,index)
FILE *fp;
short lines,index;
{
  long pos, size;
  char str[1920];

  memset(str, 0, 1920);
  
  pos = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);

  if(index == 1)
  {
    pos = savefp - lines * WIDTH;
    if(pos < 0) pos = 0;
    savefp = pos;

    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else if(index == 2)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;

    fseek(fp, pos, 0);
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else return(0);
}

leave()                                   /* exit program                    */
{
  close_all();
  sd_close();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
}
open_all()
{
  sd_open(leave);
  ss_open();
}

close_all()
{
  ss_close();
  unlink(blank_file);
}


edit(filename,mode)                       /* execute editor                  */
char *filename;
char *mode;
{
  close_all();
  sd_close();                            /* aha 051801 */
  execlp("input_editor","input_editor",
  "product_file_query", filename, mode, 0);
  krash("edit", "input_editor load", 1);
}

/* end of product_file_query.c */
