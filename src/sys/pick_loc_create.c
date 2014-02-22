/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Pick location analysis report creation.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/28/93   |  tjt  Added to mfc.
 *  06/14/94   |  tjt  Add bays with no modules.
 *  07/22/95   |  tjt  Revise Bard calls.
 *  10/02/95   |  tjt  Replace UNOS quicksort with UNIX sort.
 *  08/23/96   |  tjt  Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char pick_loc_create_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      pick_loc_create.c                                               */
/*                                                                      */
/*      this routine creates the pick location analysis report,         */
/*      in the following steps:                                         */
/*                                                                      */
/*      the data is extracted from the files                            */
/*                                                                      */
/*      the output file is then sorted, using the input arguments.      */
/*                                                                      */
/*      the sorted file is then formatted for the report.               */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "iodefs.h"
#include "ss.h"
#include "co.h"
#include "pick_loc_report.t"              /* for database_report program     */
#include "imt.h"
#include "Bard.h"
#include "bard/prodfile.h"
#include "bard/pmfile.h"

prodfile_item pf;
pmfile_item pm;

#define RECORD_SIZE     73

/* arguments to main:

        argv[1] is the operator's pickline number (or 0 for all picklines).
        argv[2] is the selected ranking criteria.
        argv[3] is the selected sorting criteria.
        argv[4] is a code which informs which data values to use:
                1 = use current period rates.
                2 = use cumulative period rates.
        argv[5] unused;
        argv[6] is 'y' if this program exits to printer program, or
                'n' if report is to be displayed on the screen.
*/

long pid, status;

main(argc,argv)
int argc;
char **argv;
{
  extern leave();                    /* for power failure               */
  register long j, k, m;
  
        /* if ranking = sorting, files 0,1,and 2 are used. if
           ranking is not the same as sorting, files 0,1,2,3 and
           4 are used.
        */

  FILE *fp0;
  FILE *fp1;
  FILE *fp2;
  FILE *fp3;
  FILE *fp4;

  char data_file[5][16];                  /* file 0 is extracted raw data    */
                                          /* file 1 is sorted data           */
                                          /* file 2 is ranked data           */
                                          /* file 3 is the sorted ranked data*/
                                          /* file 4 is the final output file */

  short wotfile=0;                        /* which file is final output      */
  char print_file[16];                    /* required to print report        */

  char wbuf[2][8];                        /* for ranking comparison          */

  short rank,sort;
  char cmd_buf[100];
  char rec_buf[RECORD_SIZE+5];

  short pickline=atoi(argv[1]);           /* selected pickline               */
  short current=(*argv[4]=='1');          /* current = 1 if true             */

  char c;                                 /* general purpose                 */
  short i, n;                             /* general purpose                 */

  putenv("_=pick_loc_create");
  chdir(getenv("HOME"));

  open_all();

        /* obtain temporary filenames */

  for(i=0; i< 5; i++) tmp_name(data_file[i]);

        /* clear working buffers */

  for(i=0;i<2;i++)
  for(n=0;n<8;n++)
  wbuf[i][n] = 0;


  fp0 = fopen(data_file[0], "w");

  for (k = 0; k < coh->co_zone_cnt; k++)
  {
    if (pickline && zone[k].zt_pl != pickline) continue;

    j = zone[k].zt_first_bay;
    
    while (j > 0)
    {
      for (m = bay[j-1].bay_prod_first; m && m <= bay[j-1].bay_prod_last; m++)
      {
        begin_work();
        pm.p_pmodno = m;
        if (!pmfile_read(&pm, NOLOCK))
        {
          memcpy(pf.p_pfsku, pm.p_pmsku, 15);
          if (!prodfile_read(&pf, NOLOCK))
          {
            if (current)
            {
              fprintf(fp0,"%-15.15s|%-25.25s|%5d|%7d|%7d|%-6.6s|%d\n",
                pm.p_pmsku, pf.p_descr, pm.p_pmodno,
                pm.p_culines, pm.p_cuunits, pm.p_stkloc, pm.p_plidx);
            }
            else
            {
              fprintf(fp0,"%-15.15s|%-25.25s|%5d|%7d|%7d|%-6.6s|%d\n",
                pm.p_pmsku, pf.p_descr, pm.p_pmodno,
                pm.p_cmlines, pm.p_cmunits, pm.p_stkloc, pm.p_plidx);
            }
          }
        }
        commit_work();
      }
      j = bay[j].bay_next;
    }
  }
  fclose(fp0);

#ifdef DEBUG
  fprintf(stderr, "Extract Done %s\n", data_file[0]);
  fflush(stderr);
#endif
        
        /* sort file 0 by arguments...file 0 has the following format:

                00-14                   pmsku
                16-40                   descr
                42-46                   pmodno
                48-54                   lines
                56-62                   units
                64-69                   stkloc
                71                      plindex
                72                      \n (end of record)
        */

  if(*argv[2]=='l')                       /* rank by lines                   */
    sort=4;
  else if(*argv[2]=='u')                  /* rank by units                   */
    sort=5;
  else                                    /* rank by pick loc index          */
    sort=7;
#ifdef INTEL
  sprintf(cmd_buf,"sort \"-t|\" +%dnr -%d %s > %s",
    sort - 1, sort, data_file[0], data_file[1]);
#else
  sprintf(cmd_buf,"quicksort \"d=|\" f=%dd %s > %s",
    sort, data_file[0], data_file[1]);
#endif
  system(cmd_buf);
#ifdef DEBUG
  fprintf(stderr, "Sort Done %s\n", data_file[1]);
  fflush(stderr);
#endif

#ifndef DEBUG
  unlink(data_file[0]);                   /* DELETE FILE 0                   */
#endif
  if(*argv[2] == *argv[3])                /* ranking equals sorting          */
  {
    rank=0;
    fp1 = fopen(data_file[1], "r");
    fp2 = fopen(data_file[2], "w");       /* final output                    */

    while (fread(rec_buf, RECORD_SIZE, 1, fp1) > 0)
    {
                        /* extract 'rank by' portion for ranking check */
      if(*argv[2]=='l')                   /* rank by lines                   */
      {
        movebytes(rec_buf+48, wbuf[1],7);
        wbuf[1][7] = 0;;
      }
      else if(*argv[2]=='u')              /* rank by units                   */
      {
        movebytes(rec_buf+56, wbuf[1],7);
        wbuf[1][7] = 0;
      }
      else                                /* rank by pick loc index          */
      {
        wbuf[1][0] = rec_buf[71];
        wbuf[1][1] = 0;
      }
      if(streql(wbuf[0],wbuf[1]))         /* rank equal                      */
        rank--;
      strcpy(wbuf[0], wbuf[1]);

      fprintf(fp2,
      "%04d   %15.15s   %5.5s   %7.7s %7.7s    %6.6s          %c          \n",
      ++rank, rec_buf+0, rec_buf+42, rec_buf+48, rec_buf+56,
      rec_buf+64, *(rec_buf+71));
      
      fprintf(fp2, "       %25.25s%47c\n", rec_buf+16, ' ');
    }
    fclose(fp1);
    fclose(fp2);
#ifndef DEBUG
    unlink(data_file[1]);                 /* DELETE FILE 1                   */
#endif
    wotfile=2;
#ifdef DEBUG
  fprintf(stderr, "Rank Equal Done %s\n", data_file[2]);
  fflush(stderr);
#endif
  }
  else                                    /* ranking not equal to sorting    */
  {
                /* copy ranked file, adding rank indicators... */

    fp1 = fopen(data_file[1], "r");
    fp2 = fopen(data_file[2], "w");
    rank=0;

    while (fread(rec_buf, RECORD_SIZE, 1, fp1) > 0)
    {
                        /* extract 'rank by' portion for ranking check */
      if(*argv[2]=='l')                   /* rank by lines                   */
      {
        movebytes(rec_buf+48,wbuf[1],7);
        wbuf[1][7] = 0;
      }
      else if(*argv[2]=='u')              /* rank by units                   */
      {
        movebytes(rec_buf+56,wbuf[1],7);
        wbuf[1][7] = 0;
      }
      else                                /* rank by pick loc index          */
      {
        wbuf[1][0] = rec_buf[71];
        wbuf[1][1] = 0;
      }
      if(streql(wbuf[0],wbuf[1]))         /* rank equal                      */
      rank--;
      strcpy(wbuf[0], wbuf[1]);
                        /* print record, with ranking and newline at end */
      fprintf(fp2,"%72.72s%04d\n",rec_buf,++rank);
    }
    fclose(fp1);
    fclose(fp2);
#ifndef DEBUG
    unlink(data_file[1]);                 /* DELETE FILE 1                   */
#endif

#ifdef DEBUG
  fprintf(stderr, "Rank Unequal Done %s\n", data_file[2]);
  fflush(stderr);
#endif
                /* now sort ranked file */

    if(*argv[3]=='l')                     /* sort by lines                   */
    sort=4;
    else if(*argv[3]=='u')                /* sort by units                   */
    sort=5;
    else                                  /* sort by pick loc index          */
    sort=7;
#ifdef INTEL
    sprintf(cmd_buf,"sort \"-t|\" +%dn -%d %s > %s",
      sort - 1, sort, data_file[2], data_file[3]);
#else
    sprintf(cmd_buf,"quicksort \"d=|\" f=%da %s > %s",
      sort, data_file[2], data_file[3]);
#endif
    system(cmd_buf);
#ifndef DEBUG
    unlink(data_file[2]);
#endif
#ifdef DEBUG
  fprintf(stderr, "Sort Done %s\n", data_file[3]);
  fflush(stderr);
#endif
                /* now write final output */

    fp3 = fopen(data_file[3], "r");
    fp4 = fopen(data_file[4], "w");
    while (fread(rec_buf, RECORD_SIZE+4, 1, fp3) > 0)
    {
      fprintf(fp4,
    "%4.4s    %15.15s   %5.5s    %7.7s %7.7s   %6.6s          %c          \n",
      rec_buf+72, rec_buf+0, rec_buf+42, rec_buf+48, rec_buf+56,
      rec_buf+64,*(rec_buf+71));
      
      fprintf(fp4, "       %25.25s%47c\n", rec_buf+16, ' ');
    }
    fclose(fp3);
    fclose(fp4);
#ifndef DEBUG
    unlink(data_file[3]);                 /* DELETE FILE 3                   */
#endif
    wotfile=4;
#ifdef DEBUG
  fprintf(stderr, "Extract Done %s\n", data_file[4]);
  fflush(stderr);
#endif
  }
        /* get time for report display */
  showtime(pickline, current, cmd_buf);

  close_all();
  if(*argv[6]=='y')                       /* print file                      */
  {
    if (fork() == 0)
    {
      execlp("prft","prft",data_file[wotfile],
        tmp_name(print_file),"sys/report/pick_loc_print.h", cmd_buf,0);
      krash("main", "prft load", 1);
    }
    pid = wait(&status);
    if (pid < 0 || status) krash("main", "prft failed", 1);
      
    execlp("pick_loc_input","pick_loc_input",0);
    krash("main", "pick_loc_input load", 1);
  }
  execlp("database_report","database_report",
      pick_loc_report, data_file[wotfile], "80", "11", "12",
      "sys/report/pick_loc_print.h", "pick_loc_input",
      cmd_buf, "6", "10", "0", 0);
  krash("main", "database_report load", 1);
}

showtime(pickline,current,buf)
unsigned char pickline;
short current;
char *buf;                                /* where to return times string    */
{
  char wbuf[2][50];          /* buf 0 for start time, buf 1 for end(now) time*/
  long c_time, now;

  time(&now);

  if(!pickline)                          /* check for all picklines requested*/
  {
    strcpy(buf,"        ");
    return;
  }
  imt_load();
  if(current) c_time = imt[pickline - 1].imt_cur;
  else c_time = imt[pickline - 1].imt_cum;
  
  strcpy(wbuf[0], ctime(&c_time));
  strcpy(wbuf[1], ctime(&now));

  sprintf(buf,"From Period %20.20s To %20.20s",wbuf[0]+4, wbuf[1]+4);
}

open_all()
{
  database_open();
  ss_open();
  co_open();
  pmfile_open(READONLY);
  pmfile_setkey(1);
  prodfile_open(READONLY);
  prodfile_setkey(1);
  return 0;
}

close_all()
{
  ss_close();
  co_close();
  pmfile_close();
  prodfile_close();
  database_close();
  return 0;
}

leave()
{
  close_all();
  execlp("mmenu", "mmenu", 0);                    /* exit to main menu       */
  krash("leave", "mmenu load", 1);
}

/* end of pick_loc_create.c */
