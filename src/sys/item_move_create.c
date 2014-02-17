/*----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Item movement analysis report create.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   10/30/93  |  tjt  Added to mfc.
 *   04/13/95  |  tjt  Bug when rank different than sort.
 *   07/21/95  |  tjt  Revise Bard calls.
 *   09/24/95  |  tjt  Fix small bug in imt fread.
 *   10/02/95  |  tjt  Check unos quicksort to UNIX sort.
 *   08/23/96  |  tjt  Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char item_move_create_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      item_move_create.c                                              */
/*                                                                      */
/*      this routine creates the item movement analysis report,         */
/*      in the following steps:                                         */
/*                                                                      */
/*      for all mods in the correct pickline, the                       */
/*      pfsku, pmodno, lines, units, receipts,                          */
/*      and case pack and write data as fields to output file.          */
/*      cartons is figured as units/case pack, and units/line as        */
/*      lines divided by units.                                         */
/*                                                                      */
/*      the output file is then sorted, using the input arguments.      */
/*                                                                      */
/*      the sorted file is then formatted for the report.               */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "file_names.h"
#include "iodefs.h"
#include "co.h"
#include "Bard.h"
#include "bard/pmfile.h"
#include "bard/prodfile.h"

#include "item_move_report.t"            /* for database_report             */

#define RECORD_SIZE     62

long pid, status;

pmfile_item   pm;
prodfile_item pf;

        /* if ranking = sorting, files 0,1,and 2 are used. if
           ranking is not the same as sorting, files 0,1,2,3 and
           4 are used.
        */

FILE *fp0;
FILE *fp1;
FILE *fp2;
FILE *fp3;
FILE *fp4;

char data_file[5][16];               /* file 0 is the initially create file*/
                                /* file 1 is file 0 sorted */
                                /* file 2 is the final output file...
                                   or the file with ranking. */
                                /* file 3 is the sorted file-with-ranks */
                                /* file 4 is the final output file */

short wotfile=0;                        /* which file is final output      */
char print_file[16];                    /* required to print report        */

long lines, units, receipts, cartons, ul;
short case_pack;
  
char wbuf[2][8];                        /* for ranking comparison          */
char rank_buf[8];                       /* passed to report or printer     */

short rank,sort;
char cmd_buf[100];
char rec_buf[RECORD_SIZE+5];

short pickline;                         /* selected pickline               */
short current;                          /* current = 1 if true             */

char c;                                 /* general purpose                 */
short i,j,k,n;                          /* general purpose                 */

/* arguments to main:

        argv[1] is the operator's pickline number (or 0 for all picklines).
        argv[2] is the selected ranking criteria.
        argv[3] is the selected sorting criteria.
        argv[4] is a code which informs which data values to use:
                1 = use current period rates.
                2 = use cumulative period rates.
        argv[5] not used
        argv[6] is 'y' if this program exits to printer program, or
                'n' if report is to be displayed on the screen.
*/

main(argc,argv)
int argc;
char **argv;
{
  extern leave();

  putenv("_=item_move_create");
  chdir(getenv("HOME"));

  pickline = atol(argv[1]);               /* selected pickline               */
  current = (*argv[4] == '1');            /* current = 1 if true             */

  open_all();
  
  for(i=0;i<5;i++) 
  {
    tmp_name(data_file[i]);
#ifdef DEBUG
  fprintf(stderr, "data_file[%d] = %s\n", i, data_file[i]);
#endif
  }

  for(i=0;i<2;i++)
  {
    for(n=0;n<8;n++) wbuf[i][n] = 0;
  }
  fp0 = fopen(data_file[0], "w");
  
  begin_work();
  while (!pmfile_next(&pm, NOLOCK))
  {
    if (pickline)
    {
      if (pickline != find_pickline(pm.p_pmodno)) continue;
    }
    memcpy(pf.p_pfsku, pm.p_pmsku, 15);
    
    if (prodfile_read(&pf, NOLOCK)) 
    {
      commit_work();
      begin_work();
      continue;
    }
    if (current)
    {
      lines    = pm.p_culines;
      units    = pm.p_cuunits;
      receipts = pm.p_curecpt;
    }
    else                                  /* cumulative                      */
    {
      lines    = pm.p_cmlines;
      units    = pm.p_cmunits;
      receipts = pm.p_cmrecpt;
    }
    case_pack = pf.p_cpack;
                                /* write record to output file */

    if(units && case_pack) cartons = units/case_pack;
    else cartons = 0;
        
    if(lines && units) ul = units/lines;
    else ul = 0;
        
    fprintf(fp0,"%-15.15s|%5d|%7d|%7d|%7d|%7d|%7d\n",
    pm.p_pmsku, pm.p_pmodno, receipts, cartons, lines, units, ul);
    
    commit_work();
    begin_work();
  }
  commit_work();
  
  fclose(fp0);

        /* sort file 0 by arguments...file 0 has the following format:

                00-14                   pmsku           field 1
                16-20                   pmodno          field 2
                22-28                   receipts        field 3
                30-36                   cartons         field 4
                38-44                   lines           field 5
                46-52                   units           field 6
                54-60                   ul              field 7
                61                      \n (end of record)
        */

  if(*argv[2]=='l')                       /* rank by lines                   */
    sort=5;
  else if(*argv[2]=='u')                  /* rank by units                   */
    sort=6;
  else                                    /* rank by cartons                 */
    sort=4;
#ifdef INTEL
  sprintf(cmd_buf, "sort \"-t|\" +%dnr -%d %s > %s",
    sort - 1, sort, data_file[0],  data_file[1]);
#else
  sprintf(cmd_buf, "quicksort \"d=|\" f=%dd %s > %s",
    sort, data_file[0],  data_file[1]);
#endif
  system(cmd_buf);

#ifndef DEBUG
  unlink(data_file[0]);                   /* DELETE FILE 0                   */
#endif

  if(*argv[2]==*argv[3])                  /* ranking equals sorting          */
  {
    rank=0;
    fp1 = fopen(data_file[1], "r");
    fp2 = fopen(data_file[2], "w");       /* final output                    */

    while (fread(rec_buf, RECORD_SIZE, 1, fp1) > 0)
    {
                        /* extract 'rank by' portion for ranking check */
      if(*argv[2]=='l')                   /* rank by lines                   */
      {
        movebytes(rec_buf+38,wbuf[1],7);
        wbuf[1][7] = 0;
      }
      else if(*argv[2]=='u')              /* rank by units                   */
      {
        movebytes(rec_buf+46,wbuf[1],7);
        wbuf[1][7] = 0;
      }
      else                                /* rank by cartons                 */
      {
        movebytes(rec_buf+30,wbuf[1],7);
        wbuf[1][7] = 0;
      }
      if(streql(wbuf[0],wbuf[1])) rank--; /* rank equal                      */

      strcpy(wbuf[0],wbuf[1]);

      fprintf(fp2,
      "%04d   %15.15s    %5.5s      %7.7s  %7.7s  %7.7s  %7.7s %7.7s\n",
      ++rank,rec_buf+0,rec_buf+16,rec_buf+22,rec_buf+30,rec_buf+38,rec_buf+46,
      rec_buf+54);
    }
    fclose(fp1);
    fclose(fp2);

#ifndef DEBUG
    unlink(data_file[1]);                 /* DELETE FILE 1                   */
#endif
    wotfile=2;
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
        movebytes(rec_buf+38,wbuf[1],7);
        wbuf[1][7] = 0;
      }
      else if(*argv[2]=='u')              /* rank by units                   */
      {
        movebytes(rec_buf+46,wbuf[1],7);
        wbuf[1][7] = 0;
      }
      else                                /* rank by cartons                 */
      {
        movebytes(rec_buf+30,wbuf[1],7);
        wbuf[1][7] = 0;
      }
      if(streql(wbuf[0],wbuf[1])) rank--; /* rank equal                      */

      strcpy(wbuf[0],wbuf[1]);
                        /* print record, with ranking and newline at end */
      fprintf(fp2,"%61.61s%04d\n",rec_buf, ++rank);  /* F041395 */
    }
    fclose(fp1);
    fclose(fp2);

#ifndef DEBUG
    unlink(data_file[1]);                 /* DELETE FILE 1                   */
#endif
                /* now sort ranked file */

    if(*argv[3]=='l')                     /* sort by lines                   */
      sort=5;
    else if(*argv[3]=='u')                /* sort by units                   */
      sort=6;
    else                                  /* sort by cartons                 */
      sort=6;
#ifdef INTEL
    sprintf(cmd_buf,"sort \"-t|\" +%dnr -%d %s > %s",
      sort - 1, sort, data_file[2], data_file[3]);  
#else
    sprintf(cmd_buf,"quicksort \"d=|\" f=%dd %s > %s",
      sort, data_file[2], data_file[3]);  
#endif
    system(cmd_buf);

#ifndef DEBUG
    unlink(data_file[2]);                 /* DELETE FILE 2                   */
#endif
                /* now write final output */

    fp3 = fopen(data_file[3], "r");
    fp4 = fopen(data_file[4], "w");

    while (fread(rec_buf, RECORD_SIZE+4, 1, fp3) > 0)   /* F041395 */
    {
      fprintf(fp4,
      "%4.4s   %15.15s    %5.5s      %7.7s  %7.7s  %7.7s  %7.7s %7.7s\n",
      rec_buf+61,rec_buf+0,rec_buf+16,rec_buf+22,rec_buf+30,rec_buf+38,
      rec_buf+46,rec_buf+54);
    }
    fclose(fp3);
    fclose(fp4);

#ifndef DEBUG
    unlink(data_file[3]);                /* DELETE FILE 3                   */
#endif
    wotfile=4;
  }
        /* get time for report display */
  showtime(pickline, current, cmd_buf);

        /* set 'ranked by' output argument */
  if(*argv[2]=='l')      strcpy(rank_buf,"lines");
  else if(*argv[2]=='u') strcpy(rank_buf,"units");
  else                   strcpy(rank_buf,"cartons");

  close_all();
  if(*argv[6]=='y')                       /* print file                      */
  {
    if (fork() == 0)                      /* child process                   */
    {
      execlp("prft","prft", data_file[wotfile],
      tmp_name(print_file),"sys/report/item_move_print.h",
      rank_buf,cmd_buf,0);

      krash("main", "prft load", 1);
    }
    pid = wait(&status);
    if (pid < 0 || status) krash("main", "prft failed", 1);
      
    execlp("item_move_input", "item_move_input", 0 );
    krash("main", "item_move_input load", 1);
  }
  else                                    /* display file                    */
  {
/* F082587 - sd_text() didn't like "81" so changed to "80" */

    execlp("database_report","database_report",
    item_move_report, data_file[wotfile],"80","11","11",
    "sys/report/item_move_print.h","item_move_input",
    rank_buf,"0","0",cmd_buf,"6","10","0",0);

    krash("main", "database_report load", 1);
  }
}
find_pickline(prod)
register long prod;
{
  register struct hw_item   *h;
  register struct pw_item   *i;
  register struct bay_item  *b;
  register struct zone_item *z;
  
  if (prod < 1 || prod > coh->co_prod_cnt) return 0;
  
  i = &pw[prod - 1];

  h = &hw[i->pw_ptr - 1];
  if (!h->hw_bay) return 0;               /* no bay for light                */
  
  b = &bay[h->hw_bay - 1];
  if (!b->bay_zone) return 0;             /* no zone for bay                 */
  
  z = &zone[b->bay_zone - 1];
  
  return z->zt_pl;                        /* pickline or zero                */
}

showtime(pickline, current, buf)
unsigned char pickline;
short current;
char *buf;                                /* where to return times string    */
{
  char wbuf[2][50];          /* buf 0 for start time, buf 1 for end(now) time*/
  FILE *fpt;                              /* for file of times by pickline   */
  long c_time, now;

  if (!pickline)
  {
    strcpy(buf, "     ");
    return;
  }
  fpt = fopen(imt_name, "r");
  if(fpt == 0) krash("showtime", "open imt", 1);
 
        /* first 4 bytes are current time, next 4 cumulative time */
        
  fseek(fpt, (pickline - 1) * 8, 0);
  if(current) fread(&c_time, 4, 1, fpt);
  else
  {
    fseek(fpt, 4, 1);
    fread(&c_time,4, 1, fpt);
  }
  fclose(fpt);
  
  time(&now);
  
  strncpy(wbuf[0], ctime(&c_time), 24);
  strncpy(wbuf[1], ctime(&now), 24);

  sprintf(buf, "From Period %20.20s To %20.20s", wbuf[0]+4, wbuf[1]+4);
}
open_all()
{
  database_open();

  co_open();

  pmfile_open(READONLY);
  pmfile_setkey(0);

  prodfile_open(READONLY);
  prodfile_setkey(1);
}
close_all()
{
  co_close();
  pmfile_close();
  prodfile_close();
  database_close();
}
leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "mmemu load", 1);
}

/* end of item_move_create.c */
