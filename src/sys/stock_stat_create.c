/*--------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Create Stock Status Report
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/17/93   |  tjt Added to mdc.
 *-------------------------------------------------------------------------*/
static char stock_stat_create_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      stock_stat_create.c                                             */
/*                                                                      */
/*      This program creates the stock status report.                   */
/*      The report is created in 3 steps, as follows:                   */
/*                                                                      */
/*      step 1. An SQL query is created and run on the selection        */
/*              criteria (sorted by).                                   */
/*                                                                      */
/*      step 2. Only pick modules assigned to                           */
/*              the operators pickline are selected.                    */
/*                                                                      */
/*      step 3. The records in the prepared file are now processed,     */
/*              formatted for the report, and passed to the printer     */
/*              routine or the report routine for display on the CRT.   */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "ss.h"
#include "co.h"
#include "stock_stat_report.t"

extern leave();

/* lookup types. these are used to determine usable size of range buffers */

#define NO_LOOK         0
#define SKU_LOOK        1
#define STKLOC_LOOK     2

#define RECORD_SIZE     77          /* size of record returned from SQL query*/

/* arguments to main:

        argv[1] is the operator's pickline number.
        argv[2] is the selection mode (either s,l or p).
        argv[3] is the configuration filename.
        argv[4] is the first range buffer from the input screen.
        argv[5] is the second range buffer.
        argv[6] is the print/report screen flag, which tells this
                program where to end up (either "print" or "report").
*/
char *setup = "select pfsku, um, pmodno, stkloc, qty, piflag, rsflag, descr\n";


long pid, status;

main(argc,argv)
int argc;
char **argv;
{
  char *strtok();
  char *c1, *c2, *c3, *c4, *c5, *c6, *c7, *c8;
  FILE *fp0;
  FILE *fp1;
  FILE *fp2;
  FILE *fp3;
  char data_file[3][16];                /* file 0 holds SQL query           */
                                        /* file 1 holds result of SQL query */
                                        /* file 2 has selected recs. from 1 */
                                        /* file 3 is final output file      */

  char cmd_buf[100];                      /* holds system commands           */

  char print_file[16];                    /* required for prft routine       */

  char rng_buf[2][16];                    /* lower and upper ranges          */
  char rec_buf[RECORD_SIZE];              /* holds rec from SQL result       */
  char PM_buf[5];                         /* ascii of a PM number            */
  long PM_num;                            /* derived from PM_buf             */

  char selection = *argv[2];              /* code, either p,s or l           */
  char expand_sel[10];                    /* full name of UNIFY field        */
  short lookup;                           /* holds values defined above      */
  char obuf[2][4];                        /* used for final report           */

  short found=0;                          /* first record found flag         */
  short all_list=0;                       /* 'All' records requested flag    */

  FILE *fpc;                              /* used to access config.pm file   */
  unsigned char pickline;                 /* operator pickline               */

  char c;                                 /* general purpose                 */
  FILE *fpr;
  FILE *fpw;
  short ret,i,j,k,n;                      /* general purpose                 */

  putenv("_=stock_stat_create");
  chdir(getenv("HOME"));

  open_all();

        /****************************************/
        /* clear command and range buffers.     */
        /* get temporary file names.            */
        /****************************************/

  pickline = atol(argv[1]);

  for(i=0;i<100;i++)  cmd_buf[i] = 0;
  for(i=0;i<2;i++)
  {
    for(n=0;n<16;n++) rng_buf[i][n] = 0;
  }
  tmp_name(data_file[0]);
  tmp_name(data_file[1]);
  tmp_name(data_file[2]);

        /************************************************/
        /*      create SQL query in file 0.             */
        /************************************************/

  fp0 = fopen(data_file[0], "w");         /* SQL query                       */
  fprintf(fp0, setup);                    /* initial text                    */

  switch(selection)
  {
    case 'p':                             /* pick module                     */
      fprintf(fp0, " from pmfile, prodfile\nwhere ");
      strcpy(expand_sel,"pmodno");
      break;

    case 'l':                             /* stock location                  */
      fprintf(fp0, " from pmfile, prodfile\nwhere ");
      strcpy(expand_sel,"stkloc");
      break;

    case 's':                             /* SKU                             */
      fprintf(fp0, " from prodfile, pmfile\nwhere ");
      strcpy(expand_sel,"pfsku");
      break;

    }

  if(!streql("0",argv[4]))                /* if 'All' not chosen             */
  {
    fprintf(fp0,"%s ",expand_sel);        /* selection                       */
    if(streql("0",argv[5]))               /* 2nd buf empty                   */
    {
      if(selection == 'p')                /* pick module selection           */
      fprintf(fp0,"= %s ",argv[4]);
      else                                /* pfsku, fgroup, stkloc           */
      {
        fprintf(fp0,"= '%s' ",argv[4]);
      }
    }
    else                                  /* range request                   */
    {
      if(selection != 'p')                /* not pick module select          */
      {
        fprintf(fp0," between '%s' and '%s' ", argv[4], argv[5]);
      }
      else                                /* PM select                       */
      fprintf(fp0, " between %s and %s ", argv[4], argv[5]);
    }

    fprintf(fp0,"and ");
  }
  else
  {
    all_list=1;                           /* set 'All' requested flag        */
  }
  fprintf(fp0,"pmsku = pfsku\norder by %s",expand_sel);
  fprintf(fp0,"\n");                     /* termination sequence            */
  fclose(fp0);

        /********************************************************/
        /*      run SQL query. data received in file 1          */
        /********************************************************/

  sprintf(cmd_buf,"SQL %s %s", data_file[0], data_file[1]);
  system(cmd_buf);

  fp1 = fopen(data_file[1], "r");
  fp2 = fopen(data_file[2], "w");

  while (fgets(rec_buf, RECORD_SIZE, fp1) > 0)
  {
    c1 = strtok(rec_buf, "|");
    c2 = strtok(0, "|");
    c3 = strtok(0, "|");
    c4 = strtok(0, "|");
    c5 = strtok(0, "|");
    c6 = strtok(0, "|");
    c7 = strtok(0, "|");
    c8 = strtok(0, "|");
            
    if (!(check_pl(c3, pickline))) continue;

        /**********************************************/
        /* process records - create SKU and PM report */
        /**********************************************/

    (*c6 == 'y')? strcpy(obuf[0],"Yes") : strcpy(obuf[0],"No ");
    (*c7 == 'y')? strcpy(obuf[1],"Yes") : strcpy(obuf[1],"No ");


    fprintf(fp2,
   "%-15.15s%6c%-3.3s%8c%5.5s      %-6.6s    %5.5s      %3.3s       %3.3s  \n",
    c1, 0x20, c2, 0x20, c3, c4, c5, obuf[0], obuf[1]);

    fprintf(fp2, "%-25.25s%54c\n%79c\n", c8, 0x20, 0x20);
  }
  fclose(fp1);
  fclose(fp2);
#ifdef DEBUG
  unlink(data_file[0]);
  unlink(data_file[1]);                   /* DELETE FILE 2                   */
#endif
  close_all();
  if(streql(argv[6],"report"))            /* go to report program            */
  {
    execlp("database_report","database_report",
      stock_stat_report, data_file[2], "80", "9", "15",
     "sys/report/stock_stat_print.h",
     "stock_stat_input","0",0);
    krash("main", "database_report load", 1);
  }
  else                                    /* print in background             */
  {
    if (fork() == 0)
    {
      execlp("prft", "prft", data_file[2], tmp_name(print_file),
        "sys/report/stock_stat_print.h",0);
      krash("main", "prft load", 1);
    }
    else
    {
      pid = wait(&status);
      if (pid < 0 || status) krash("main", "prft failed", 1);
      
      execlp("stock_stat_input","stock_stat_input", 0);
      krash("main", "stock_stat_input load", 1);
    }
  }
}                                         /* END OF MAIN                     */

/********************************************************/
/*      functions for stock_status_create               */
/********************************************************/


/* check_pl():

        this routine checks the pickline in the SQL query result record
        to be within the view of the operator (in operator's pickline,
        and if all, non-zero). returns 1 if yes, 0 if no.
*/

check_pl(buf, pickline)               /* check that module is in operator pl */
char *buf;
short pickline;
{
  long PM_num;

  PM_num = atol(buf);

  if (!pickline) return 1;
  
  if (pickline == find_pickline(PM_num)) return 1;

  return 0;                               /* ERROR                           */
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

open_all()
{
  ss_open();
  co_open();
}

close_all()
{
  ss_close();
  co_close();
}

leave()
{
  close_all();
  execlp("mmenu", "mmenu", 0);
  krash("leave", "mmemu load", 1);
}

/* end of stock_stat_create.c */
