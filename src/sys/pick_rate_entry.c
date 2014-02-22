/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Manual entry of pick rates as 'zone nn = xx.xxx'
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/11/93   |  tjt  Added to mfc.
 *  06/04/95   |  tjt  Add pickline input by name.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char pick_rate_entry_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*                          Pick_rate_entry.c                           */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file_names.h"
#include "global_types.h"
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "language.h"
#include "pick_rate_entry.t"

extern leave();

#define NUM_PROMPTS     7
#define BUF_SIZE        33
#define DISPLAY_SIZE    9                 /* # of lines of files to show     */

#define WIDTH 80

short ONE = 1;
short TWO = 2;
short L30 = 30;
short LPL = 8;

struct fld_parms fld[] ={

  {11,45,8,1,&ONE,"Enter Code",'a'},
  {12,45,8,1,&L30,"Enter Source Pick Rate Name",'a'},
  {13,45,8,1,&L30,"Enter Destination Pick Rate Name",'a'},
  {12,45,8,1,&LPL,"Enter Pickline",'a'},
  {13,45,8,1,&L30,"Enter Configuration Name",'a'},
  {15,45,8,1,&ONE,"Are You Sure? (y/n)",'a'},
  {23,16,1,1,&ONE,"More? (y/n)         (Exit, Forward, or Backward)",'a'}

};
long savefp;

struct pick_rate_token
{
  long   zone;
  float  rate;
};

char *device, printer[20];                /* F050187                         */


main()
{
  FILE *fpr = 0;
  FILE *fpw = 0;
  FILE *err_fd = 0;
  long savefp;
  char err_file[40];
  char filebuf[40];
  short rm;
  unsigned char t;
  char buf[NUM_PROMPTS][BUF_SIZE];        /* array of buffers                */
  char filename[2][45];                   /* holds filenames                 */
  char cmd_buf[100];
  char c;
  char command[40];

  short i;
  short n;
  short ret=1;
  short done=0;
  short loop_done=0;
  short clear_flag=0;                     /* for when display < 10 lines     */
                                          /* 1 = clear input lines only      */
  short dflag=0;                          /* used by display sequence        */
  short count=0;                          /* count of lines to display       */

  putenv("_=pick_rate_entry");
  chdir(getenv("HOME"));
  
  open_all();
  
  fix(pick_rate_entry);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(pick_rate_entry);
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
      if(t == EXIT) leave();

      else if (t != RETURN)
      continue;

                        /* check input */

      *buf[0] = tolower(*buf[0]);
      
      switch(*buf[0])
      {
        case 'c':                         /* create a file                   */
          
          ce_file(filename[0],buf[1],1,1);
          edit(filename,"w");
          break;

        case 'e':                         /* change a file                   */
          
          ce_file(filename[0],buf[1],1,0);
          ce_sure(buf[5]);
          if(*buf[5]=='n')
          {
            done=1;
            break;
          }
          edit(filename,"r");
          break;

        case 'v':
          
          ce_file(filename[0], buf[1], 1, 0);
          tmp_name(err_file);
          err_fd = fopen(err_file, "w");
          if(err_fd == 0)
          {
            krash("main", "open tmp", 1);
          }
          fprintf(err_fd,"Errors in pick rate validation:\n");
          
          ce_validate(buf[1], err_fd, filebuf, err_file);
/*
 * print errors 
 */
          sd_cursor(0,12,1);              /* after input line                */
          sd_clear_rest();

          if(err_fd)
          {
            fseek(err_fd, 0, 2);

            if(ftell(err_fd) > 32)
            {
              eh_post(ERR_PICK_RATE, 0);
              fclose(err_fd);
              sprintf(command,"%s %s", getenv("LPR"), err_file);
              system(command);
            }
            else
            {
              fclose(err_fd);
              unlink(err_file);
              eh_post(ERR_CONFIRM, "validation");
            }
          }
          clear_flag = 1;
          err_fd = 0;
          done=1;
          break;

        case 'd':                         /* delete a file                   */
          
          ce_file(filename[0],buf[1],1,0);
          ce_sure(buf[5]);
          if(*buf[5]=='n')
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
          while((c=getc(fpr))!=EOF)
          putc(c,fpw);
          fclose(fpr);
          fclose(fpw);
          done=1;
          break;

        case 'l':
                                /* there are 9 lines available for display */
          count=ce_format(cmd_buf);
          if (!count)
          {
            done=1;
            unlink(cmd_buf);              /* delete display                  */
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
            buf[6][n] = 0;
            sd_prompt(&fld[6],0);         /* More?                           */
            while(1)
            {
              t=sd_input(&fld[6],0,&rm,buf[6],0);
              done=0;
              switch(sd_more(t, code_to_caps(*buf[6])))  /* F041897 */
              {
                case(0): leave();
                          break;
                case(1):
                         dflag=1;         /* forward                         */
                         done=1;
                         break;
                case(2):
                         dflag=2;        /* backwrd                         */
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
      if(done) break;
    }
  }
}

/*****************************************/
/* functions for config_entry     */
/*****************************************/
open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  getparms(0);
}
close_all()
{
  ss_close();
  co_close();
  sd_close();
}

leave()                                   /* exit program                    */
{
  close_all();
  execlp("pnmm", "pnmm", 0);
  krash("leave", "pnmm load", 1);
}

edit(filename,mode)                       /* execute editor                  */
char *filename;
char *mode;
{
  close_all();
  execlp("input_editor","input_editor", "pick_rate_entry", filename, mode, 0);
  krash("edit", "input_editor load", 1);
}

ce_file(filebuf,buf,i,u)                  /* function returns usable filename*/
char *filebuf;
char *buf;
short i;
short u;                                  /* u = 0 return existing file      */
                /* u = 1 return non-existing file */
{
  FILE *ret = 0;
  short n;

  for(n=0;n<BUF_SIZE;n++) buf[n] = 0;
  sd_prompt(&fld[i],0);
  while(1)
  {
    ce_name(buf,i);                       /* get a filename                  */
    sprintf(filebuf, "%s/%s", pick_rate_text, buf);
    ret = fopen(filebuf, "r");
    if(ret) fclose(ret);
    if((ret && !u) || (!ret && u))  return;
    else
    {
      eh_post(ERR_FILE_INVALID,buf);
    }
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
    if (t == EXIT) leave();
      
    if (*buf) return;                    /* if a name entered               */
    eh_post(ERR_FILE_INVALID, 0);
  }
}

ce_sure(buf)                              /* input are you sure message      */
char *buf;
{
  unsigned char t, yn[2];
  short rm, n;

  memset(buf, 0, BUF_SIZE);
  memset(yn,  0, 2);

  sd_prompt(&fld[5],0);

  while(1)
  {
    t = sd_input(&fld[5],0,&rm,yn,0);
    if (t == EXIT) leave();
    else if (t == RETURN)
    {
      *buf = code_to_caps(*yn);          /* F041897 */
      if(*buf != 'n' && *buf != 'y')
      {
        eh_post(ERR_YN, 0);
        continue;
      }
      return;
    }
  }
}

ce_format(cmd_buf)                  /* returns true count of lines to display*/
char *cmd_buf;                            /* returns filename in cmd_buf     */
{
  FILE *fp = 0;
  FILE *sp = 0;
  char temp_file[2][33];
  char c;
  char tbuf[2][33];
  short count, save_count, odd, i, n;

  count=0;        
  odd  =0;
  tmp_name(temp_file[0]);                 /* original listing                */
  tmp_name(temp_file[1]);                 /* final listing                   */

  sprintf(cmd_buf,"ls %s >%s", pick_rate_text, temp_file[0]);
  system(cmd_buf);                        /* create listing                  */

  fp = fopen(temp_file[0], "r");
  while((c=getc(fp)) != EOF)
  {
    if (c == NEWLINE) count++;
  }
  save_count = count/2;                   /* output count (more or less)     */
  
  fseek(fp, 0, 0);                        /* to beginning                    */

  if(count % 2 == 1)                      /* odd number, and at least one    */
  {
    odd=1;
    count--;                              /* make count even (may be 0)      */
  }
  sp = fopen(temp_file[1], "w");
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
  unlink(temp_file[0]);                   /* delete original file            */
  strcpy(cmd_buf,temp_file[1]);           /* return formatted file           */
  return(save_count+odd);              /* return line count of formatted file*/
}
/*
 * Validate a Pick_rate 
 */
ce_validate(name, err_fd, filebuf, err_file)

char *name;
FILE *err_fd;
char *filebuf;
char *err_file;

{
  extern short tokenize();

  char buf[5][BUF_SIZE];                  /* array of buffers                */
  unsigned char t;
  short i, j, rm, n, pickline;
  long size;
  char prt_file[40];
  FILE *ret;

  struct pick_rate_token *rate;
  FILE *prt_fd = 0;

#ifdef DEBUG
  fprintf(stderr, "ce_validate(name=%x err_fd=%x filebuf=%x err_file=%x\n",
    name, err_fd, filebuf, err_file);
    
  if (name) fprintf(stderr, "name=[%s]\n", name);
#endif

/*
 * prompt for pickline and configuration name (only against current config)
 */
  if(SUPER_OP)
  {
    i = 3;
    rm = 1;
  }
  else i = 4;

  for(j = i; j < 5; j++) sd_prompt(&fld[j], rm);

  memset(buf, 0, 5 * BUF_SIZE);
  
  strncpy(buf[4], coh->co_config_name, 30);
  
  while(1)
  {
    t = sd_input(&fld[i],rm,&rm,buf[i],0);
    if (t == EXIT)
    {
      if(err_fd)
      {
        fclose(err_fd);
        unlink(err_file);
      }
      if (prt_fd) fclose(prt_fd);
      leave();
    }
    if (t == UP_CURSOR)
    {
      if(i == 4) i = 3;
      else i = 4;
      continue;
    }
    else if(t == DOWN_CURSOR || t == TAB)
    {
      if(i == 3) i = 4;
      else i = 3;
      continue;
    }
    if(SUPER_OP)                          /* check pickline                  */
    {
      pickline = pl_lookup(buf[3], op_pl);
      if (pickline <= 0)
      {
        eh_post(ERR_PL,buf[3]);
        i = 3;
        continue;
      }
      sprintf(buf[3], "%d", pickline);
      chng_pkln(buf[3]);
    }
    else pickline = op_pl;

    if (strcmp(buf[4], coh->co_config_name) != 0)
    {
      eh_post(LOCAL_MSG, "Only Current Config Allowed");
      i = 4;
      continue;
    }
    break;
  }
  sprintf(prt_file, "%s/%s", pick_rate, name);
 
  prt_fd = fopen(prt_file, "w+");         /* write/read pick_rate file       */
  if (prt_fd == 0)
  {
    fprintf(err_fd, "Pick rate validation failed on ");
    fprintf(err_fd, "open pick_rate_file\n");
    return ;                              /* file open error                 */
  }
  if (!tokenize(name, prt_fd, err_fd))    /* tokenize pick_rate file         */
  {
    if (prt_fd) fclose(prt_fd);
    return;
  }
/*
 * set pointers to pick rate file
 */
  fseek(prt_fd, 0, 2);
  size = ftell(prt_fd);
  rate = (struct pick_rate_token *)malloc(size);
  fseek(prt_fd, 0, 0);
  fread(rate, size, 1, prt_fd);

  if (!check_config (rate, pickline, prt_fd, err_fd)) /* check pick_rate file*/
  {
    fprintf(err_fd,"Pick rate file zones do not match configuration\n");
  }
  free(rate);
  return;
}
/*
 * tokenize the pick_rate file
 */
short tokenize(name, prt_fd, err_fd)

char *name;
FILE *prt_fd;
FILE *err_fd;

{
  extern short gettoken();

  short j,ret;
  short temp_zone[ZoneMax];
  FILE *fd = 0;
  struct pick_rate_token prt;
  char temp_name[40];

#ifdef DEBUG
  fprintf(stderr, "tokenize(name=%x prt_fd=%x err_fd=%x)\n",
    name, prt_fd, err_fd);

  if (name) fprintf(stderr, "name=[%s]\n", name);
#endif

  sprintf(temp_name,"%s/%s", pick_rate_text, name);
  fd = fopen(temp_name, "r");
  if(fd == 0)
  {
    fprintf(err_fd, "Pick rate validation failed on ");
    fprintf(err_fd, "no pick_rate file\n");
    return(0);                            /* file open error                 */
  }
  for(j = 0; j < ZoneMax; j++) temp_zone[j] = 0;

  ret = gettoken(&prt,temp_zone,fd,err_fd);
  while(ret == 1 || ret == 3)
  {
    if (ret == 1)
    {
      fwrite(&prt, sizeof(struct pick_rate_token), 1, prt_fd);
    }
    ret = gettoken(&prt,temp_zone, fd, err_fd);
  }
  if (ret == 2)                           /* error in pick rate file         */
  {
    if(fd) fclose(fd);
    return(0);
  }
  if (fd) fclose(fd);
  return(1);
}
/*
 * get a token
 */
short gettoken(prt,temp_zone,fd,err_fd)

struct pick_rate_token *prt;
short temp_zone[ZoneMax];
FILE *fd;
FILE *err_fd;

{
  extern short isfloat();
  extern short isnumeric();
  register char *p;
  short k,j;
  char *line,line1[81];

#ifdef DEBUG
  fprintf(stderr, "gettoken(prt=%x temp_zone=%x fd=%x err_fd=%x\n",
    prt, temp_zone, fd, err_fd);
#endif
  
  if (!fgets(line1, 80, fd)) return 0;

  line1[strlen(line1) - 1] = 0;

  line = line1;
  iswhite(&line);                         /* pass white space                */

  if(*line == 0) return (3);              /* blank line                      */

  p = line;
  while (*p) {*p = tolower(*p); p++;}

  if (strncmp(line, "zone", 4))
  {
    fprintf(err_fd,"Pick rate validation failed on ");
    fprintf(err_fd,"'%s' - ",line1);
    fprintf(err_fd," 'zone' expected\n");
    return(2);                            /* invalid format                  */
  }
  line += 4;                              /* advance the pointer             */
  iswhite(&line);
  if(!isnumeric(prt, &line))
  {
    fprintf(err_fd,"Pick rate validation failed on ");
    fprintf(err_fd," '%s' - ",line1);
    fprintf(err_fd,"zone number expected\n");
    return(2) ;                           /* invalid format                  */
  }
  if(temp_zone[prt->zone - 1])
  {
    fprintf(err_fd,"Pick rate validation failed on ");
    fprintf(err_fd,"'%s' - ",line1);
    fprintf(err_fd,"duplicate zone number\n");
    return(2) ;                           /* zone is duplicate               */
  }
  iswhite(&line);
  if(*line != '=')
  {
    fprintf(err_fd,"Pick rate validation failed on ");
    fprintf(err_fd,"'%s' - ",line1);
    fprintf(err_fd,"'=' expected\n");
    return(2);                            /* invalid format                  */
  }
  line++;
  iswhite(&line);
  if(!isfloat(prt,&line))
  {
    fprintf(err_fd,"Pick rate validation failed on ");
    fprintf(err_fd,"'%s' - ",line1);
    fprintf(err_fd,"number expected\n");
    return(2);                            /* invalid format                  */
  }
  temp_zone[prt->zone - 1] = 1;           /* remember the zone               */

  iswhite(&line);
  if(*line != 0)
  {
    fprintf(err_fd,"Pick rate validation failed on ");
    fprintf(err_fd," '%s' - ",line1);
    fprintf(err_fd,"end of line expected\n");
    return(2) ;                           /* invalid end of line             */
  }
  return(1) ;
}

/*
 * pass the white space
 */
iswhite(line)
char **line;
{
  while(**line == SPACE) (*line)++;
}

/*
 * check number to be numeric
 */
short isnumeric(prt,line)

char **line;
struct pick_rate_token *prt;

{
  char temp[10];
  short j;

  j = 0;

  while(**line >= '0' && **line <= '9')
  {
    temp[j] = **line;
    (*line)++;
    j++;
  }
  if(!j || j > 4)                         /* not numeric or large number     */
  return(0);

  temp[j] = 0;
  prt->zone = atoi(temp);
  return(1);
}

/*
 * check number to be float
 */
short isfloat(prt,line)

struct pick_rate_token *prt;
char **line;

{
  char temp[80],temp1[80];
  float kf;
  long j,n,m,k;
  long a;

  prt->rate = 0;
  
  j = m = n = 0;
  while((**line <= '9' && **line >= '0') || **line == '.')
  {
    if(n)
    {
      temp1[m] = **line;                  /* store fractional part           */
      m++;
    }
    else if(**line != '.')
    {
      temp[j] = **line;                   /* store whole part                */
      j++;
    }
    if(**line == '.')                     /* decimal point                   */
    n++;

    (*line)++;
  }
  if(n > 1 || j >8) return(0);            /* invalid format                  */

  if (j > 0)
  {
    temp[j] = 0;
    a = atol(temp);
    prt->rate = a;
  }
  if (m > 0)
  {
    temp1[m] = 0;
    k = atol(temp1);
    kf = k;
  
    while (m > 0) {kf = kf / 10.0; m--;}
  
    prt->rate += kf;
  }
  if(prt->rate == 0) return(0);

  return(1);
}
/*
 * check the pick_rate file against configuration
 */
check_config(rate, pickline, prt_fd, err_fd)

struct pick_rate_token *rate;
short pickline;
FILE *prt_fd;
FILE *err_fd;

{
  long size;
  short count, j, n, i, zcount = 0;

#ifdef DEBUG
  fprintf(stderr, "check_config(rate=%x pickline=%d prt_fd=%x err_fd=%x)\n",
    rate, pickline, prt_fd, err_fd);
#endif
  
  fseek(prt_fd, 0, 2);
  size = ftell(prt_fd);
  fseek(prt_fd, 0, 0);
  
  size = size / sizeof(struct pick_rate_token);    /* number of zones  */

  if (pickline == 0)
  {
    for (i = 0; i < coh->co_zone_cnt; i++)
    {
      if(zone[i].zt_zone) zcount++;
    }
  }
  else
  {
    for (i = 0; i < coh->co_zone_cnt; i++)
    {
      if (zone[i].zt_pl == pickline) zcount++;
    }
  }
  if (size != zcount) return(0);

  count = 0;

  for (j = 0; j < size; j++)
  {
    for (n = 0; n < coh->co_zone_cnt; n++)
    {
      if((pickline == 0 || zone[n].zt_pl == pickline)
        && rate[j].zone == zone[n].zt_zone)
      {
        count++;
        break;
      }
    }
  }
  if (count != size) return 0;           /* zones not in configuration      */
   
  return(1);
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

  if(index == 2)
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

/* end of pick_rate_entry.c */
