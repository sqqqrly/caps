/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Configuration entry screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/03/93    |  tjt  Added to mfc.
 * 05/07/95    |  tjt  Fix list file display.
 * 04/18/97    |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char config_entry_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      config_entry.c                                                  */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "co.h"
#include "language.h"
#include "config_entry.t"

#define NUM_PROMPTS     5
#define BUF_SIZE        33
#define DISPLAY_SIZE    9                 /* # of lines of files to show     */

#define WIDTH 80

char *ttyname();
char *device, printer[20];                /*F050187                          */
char command[50];                         /*F050187                          */
char qname[32] ="config/";                /*F063093                          */
char print_file[32];
static short ONE = 1;
static short L30 = 30;

struct fld_parms fld[] ={

  {11,46,7,1,&ONE,"Enter Code",'a'},
  {12,46,7,1,&L30,"Enter Source Configuration",'a'},
  {13,46,7,1,&L30,"Enter Destination Configuration",'a'},
  {14,46,7,1,&ONE,"Are You Sure? (y/n)",'a'},
  {23,16,1,1,&ONE,"More? (y/n)      (Exit, Forward, or Back)",'a'}

};

long savefp=0;                            /* used by show function           */

main()
{
  FILE *fpr;
  FILE *fpw;
  short rm;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];        /* array of buffers                */
  char filename[2][45];                   /* holds filenames                 */
  char cmd_buf[100];
  char c;

  short i;
  short n;
  short ret=1;
  short done=0;
  short loop_done=0;
  short clear_flag=0;                     /* for when display < 10 lines     */
                                          /* 1 = clear input lines only      */
  short dflag=0;                          /* used by display sequence        */
  short count=0;                          /* count of lines to display       */

  open_all();

  fix(config_entry);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(config_entry);
  sd_screen_on();

  while(1)                                /* loop for multiple screens       */
  {
                /* clear main buffer and screen */
    
    sd_cursor(0,11,1);
    if(!clear_flag) sd_clear_rest();      /* if no display                   */
    
    memset(buf[0], 0, NUM_PROMPTS * BUF_SIZE);
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
      if(t == EXIT) leave();
      else if(t != RETURN) continue;
                        
                        /* check input */

      *buf[0] = tolower(*buf[0]);
      switch(*buf[0])
      {
        case 'c':                         /* create a file                   */
        
          ce_file(filename[0],buf[1],1,1);
          edit(filename,"w");
          break;

        case 'r':                         /* print a file                    */
        
          ce_file(filename[0],buf[1],1,0);
          ce_sure(buf[3]);
          if(*buf[3]=='n')
          {
            done=1;
            break;
          }
          print(filename);
          done = 1;
          break;

        case 'e':                         /* change a file                   */
        
          ce_file(filename[0],buf[1],1,0);
          if(sp->sp_running_status == 'y' &&
          streql(buf[1],coh->co_config_name))
          {
            eh_post(ERR_ED_CONFIG,"Edit");
            continue;
          }
          ce_sure(buf[3]);
          if(*buf[3]=='n')
          {
            done=1;
            break;
          }
          edit(filename,"r");
          break;


        case 'v':
        
          ce_file(filename[0],buf[1],1,0);
          ce_validate(buf[1]);
          clear_flag = 1;
          done=1;
          break;

        case 'd':                         /* delete a file                   */
        
          ce_file(filename[0],buf[1],1,0);
          if (sp->sp_config_status == 'y' &&
          streql(buf[1], coh->co_config_name))
          {
            eh_post(ERR_ED_CONFIG, "Delete");
            continue;
          }
          ce_sure(buf[3]);
          if(*buf[3]=='n')
          {
            done=1;
            break;
          }
          unlink(filename[0]);
          done=1;
          break;

        case 'p':                         /* copy a file                     */
        
          ce_file(filename[0],buf[1],1,0);
          ce_file(filename[1],buf[2],2,1);
          fpr = fopen(filename[0], "r");
          fpw = fopen(filename[1], "w");
          while((c=getc(fpr))!=EOF) putc(c,fpw);
          fclose(fpr);
          fclose(fpw);
          done=1;
          break;

        case 'l':
                                /* there are 9 lines available for display */
          count=ce_format(cmd_buf);
          if (count < 1)
          {
            sd_cursor(0, 13, 7);
            sd_text("*** No configuration files");

            clear_flag = done = 1;
            unlink(cmd_buf);             /* delete display                  */
            break;
          }
                                /* filename is in cmd_buf */
          fpr = fopen(cmd_buf,"r");
          if(count<=DISPLAY_SIZE)         /* no more prompt                  */
          {
            sd_cursor(0,13,1);
            show(fpr,count,2);
            clear_flag = done = 1;
            fclose(fpr);
            unlink(cmd_buf);             /* delete display                  */
            break;
          }
          dflag=2;
          while(1)                        /* loop to display data            */
          {
            loop_done=0;
            sd_cursor(0,13,1);
            ret=show(fpr,DISPLAY_SIZE,dflag);
            if(ret)
            sd_clear_rest();
            for(n=0;n<BUF_SIZE;n++)
            buf[4][n] = 0;
            sd_prompt(&fld[4],0);         /* More?                           */
            while(1)
            {
              t=sd_input(&fld[4],0,&rm,buf[4],0);
              done=0;
              switch(sd_more(t, code_to_caps(*buf[4])))  /* F041897 */
              {
                case(0):
                  leave();
                case(1):
                  dflag=1;                /* forward                         */
                  done=1;
                  break;
                case(2):
                  dflag=2;                /* backwrd                         */
                  done=1;
                  break;
                case(3):
                  done=1;
                  loop_done=1;
                  clear_flag=0;
                  break;
                case(6):
                  eh_post(ERR_YN,0);
                  break;
              }                           /* end switch                      */
              if(done)
              break;
            }                             /* end more input loop             */
            if(loop_done)
            break;
          }                               /* end display loop                */
          fclose(fpr);
          unlink(cmd_buf);               /* delete display                  */
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
/* functions for configuration_entry     */
/*****************************************/

leave()                                   /* exit program                    */
{
  close_all();
  sd_close();
  execlp("confm","confm", 0);
  krash("leave", "comfm load", 1);
}

/* new Function to print files    */
print(filename)                           /* execute printfunction           */
char *filename;
{
  char temp1[16], temp2[16], title[40];
  long pid, stat, len;
  tmp_name(temp1);
  tmp_name(temp2);
  
  sprintf(command, "cp %s %s", filename, temp1);
  system(command);
  
  len = (20 - strlen(filename)) / 2;
  if (len <= 0) len = 1;
  
  sprintf(title, "%*s%s", len, " ", filename);
  
  if (fork() == 0)
  {
    execlp("prft", "prft", temp1, temp2, "sys/report/report_report.h",
      title, 0);
    krash("print", "load prft", 1);
  }
  pid = wait(&stat);
  return 0;
}

edit(filename,mode)                       /* execute editor                  */
char *filename;
char *mode;
{
  close_all();
  execlp("input_editor","input_editor",
    "config_entry", filename, mode, 0);
  krash("edit", "input_editor load", 1);
}

ce_file(filebuf,buf,i,u)                  /* function returns usable filename*/
char *filebuf;
char *buf;
short i;
short u;                                  /* u = 0 return existing file      */
                /* u = 1 return non-existing file */
{
  FILE *ret;
  short n;

  for(n=0;n<BUF_SIZE;n++) buf[n] = 0;
  sd_prompt(&fld[i],0);
  while(1)
  {
    ce_name(buf,i);                       /* get a filename                  */
    strcpy(filebuf,"config/");
    strcat(filebuf,buf);
    ret = fopen(filebuf,"r");
    if (ret) fclose(ret);
    if((ret && !u) || (!ret && u)) return;
    else eh_post(ERR_FILE_INVALID, buf);
  }
}

ce_name(buf,i)                            /* input a filename                */
char *buf;
short i;
{
  unsigned char t;
  short rm;

  while(1)
  {
    t=sd_input(&fld[i],0,&rm,buf,0);

    if(t==EXIT) leave();
    else if(t==RETURN)
    {
      if(*buf) return;                    /* if a name entered               */
      else eh_post(ERR_FILE_INVALID,0);
    }
  }
}

ce_sure(buf)                              /* input are you sure message      */
char *buf;
{
  unsigned char t, yn[2];
  short rm, n;

  memset(buf, 0, BUF_SIZE);
  memset(yn,  0, 2);
  
  sd_prompt(&fld[3],0);
  while(1)
  {
    t=sd_input(&fld[3],0,&rm,yn,0);
    if(t==EXIT) leave();
    else if(t==RETURN)
    {
      *buf = code_to_caps(*yn);          /* F041897 */
      if(*buf != 'n' && *buf != 'y') eh_post(ERR_YN,0);
      else return;
    }
  }
}

ce_format(cmd_buf)                  /* returns true count of lines to display*/
char *cmd_buf;                            /* returns filename in cmd_buf     */
{
  FILE *fp;
  FILE *sp;
  char temp_file[2][33];
  char c;
  char tbuf[2][33];
  short count, lines, i, n;

  count = 0;
  tmp_name(temp_file[0]);                 /* original listing                */
  tmp_name(temp_file[1]);                 /* final listing                   */

  sprintf(cmd_buf, "/bin/ls config >%s", temp_file[0]);
  system(cmd_buf);                        /* create listing                  */

  fp  = fopen(temp_file[0], "r");
  while((c = getc(fp)) != EOF)
  {
    if (c == NEWLINE) count++;
  }
  fseek(fp, 0, 0);                        /* to beginning                    */
  
  sp = fopen(temp_file[1], "w");
  
  for (lines = 0; count >= 2; count -= 2)
  {
    for (i = 0; i < 2; i++)
    {
      n=0;
      while((c=getc(fp))!=NEWLINE) tbuf[i][n++] = c;
      tbuf[i][n] = 0;
    }
    fprintf(sp,"       %-35s%-37s\n",tbuf[0],tbuf[1]);
    lines++;
  }
  if (count)
  {
    n=0;
    while((c=getc(fp))!=NEWLINE) tbuf[0][n++] = c;
    tbuf[0][n] = 0;
    fprintf(sp,"       %-72s\n",tbuf[0]);
    lines++;
  }
  fclose(fp);
  fclose(sp);
  unlink(temp_file[0]);                   /* delete original file            */
  strcpy(cmd_buf,temp_file[1]);           /* return formatted file           */
  return(lines);                       /* return line count of formatted file*/
}


/*
 * Validate a Configuration
 */
ce_validate(name)
char *name;
{
  char string[16];
  long pid, status;

  if (fork() == 0)
  {
    close_all();
    execlp("configure", "configure", name, "-v", 0);
    krash("ce_validate", "configure load", 1);
    exit(1);
  }
  pid = wait(&status);
  
  if (status || pid < 0) eh_post(ERR_CHK_CONFIG, "errors");
  else eh_post(ERR_CONFIRM, "Validation");
  
  return;
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
  register long pos, size;
  char str[1920];

  memset(str, 0, 1920);

  pos = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);

  if (index == 2)
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
  else if(index == 1)
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

open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
}

close_all()
{
  co_close();
  ss_close();
}

/* end of config_entry.c */
