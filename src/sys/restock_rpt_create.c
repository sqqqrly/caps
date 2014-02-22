/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Restock report creation.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/01/93   |  tjt  Added to mfc.
 *  03/29/95   |  tjt  Fix brf_open from "wac" to "a+".
 *  07/22/95   |  tjt  Revise Bard calls.
 *-------------------------------------------------------------------------*/
static char restock_rpt_create_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      restock_rpt_create.c                                            */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file_names.h"
#include "iodefs.h"
#include "ss.h"
#include "co.h"
#include "restock_report.t"             /* for use by database_report routine*/
#include "Bard.h"
#include "bard/pmfile.h"

extern FILE *brf_fd;

pmfile_item pm;

#define RECORD_SIZE     116               /* SQL output record size          */

/* arguments to restock_report_create:

        argv[1] is the 'all' or 'under restock only' selection code.
        argv[2] is the sort by code, either 'b', 's', or 'p'.
        argv[3] unused.
        argv[4] is the pickline number.
        argv[5] is 'y' if the report is to be printed, 'n' if displayed.
*/

long pm_tab_size;                         /* size of config.pm file          */

long pid, status;

main(argc,argv)
int argc;
char **argv;
{
  char *strtok();
  char *c1, *c2, *c3, *c4, *c5, *c6, *c7, *c8, *c9, *c10, *c11, *c12;
  
  FILE *fp0;
  FILE *fp1;
  FILE *fp2;
  char data_file[3][16];                  /* file 0 is the SQL query         */
                                        /* file 1 is results of query */
                                        /* file 2 is final file to write */
  char print_file[16];                    /* file to pass to print formatter */
  short i,j,k,n;                          /* general purpose                 */
  char c;                                 /* general purpose                 */
  short batch;                            /* batch number                    */
  char batch_text[5];                     /* ascii of batch                  */
  char save_batch_text[5];                /* same as batch_text              */
  short rs_qty=0;                         /* restock quantity (units)        */
  short rs_cp=0;                          /* restock quantity (cases)        */
  static char setup[] = 
  "select pfsku, pmodno, qty, bsloc, stkloc,\
  rsflag, descr, um, restock, absloc, cpack, lcap\n\
  from prodfile, pmfile\nwhere ";

/*-------------------------------------------------------------------------*
 *                                                                          
 *          ACTUAL SQL FORMAT:                                              
 *                                                                        
 *   select pfsku, pmodno, qty, bsloc, stkloc rsflag,descr, um ,restock,    
 *          restock, absloc, cpack, lcap                                    
 *   from prodfile,pmfile                                                   
 *   where .. (depends on query)                                               
 *
 *               000-014         pfsku
 *                   015         |
 *               016-024         pmodno
 *                   025         |
 *               026-034         qty (on hand)
 *                   035         |
 *               036-041         bsloc (back up stock location)
 *                   042         |
 *               043-048         stkloc (CAPS)
 *                   049         |
 *                   050         rsflag (restock pending)   
 *                   051         |                          
 *               052-076         descr
 *                   077         |
 *               078-080         um (unit of measure)
 *                   081         |
 *               082-090         restock (point)
 *                   091         |
 *               092-097         absloc (alternate back up)
 *                   098         |
 *               099-104         cpack (case pack)
 *                   105         |
 *               106-114         lcap (lane capacity - not on report)
 *                   115         \n (end of record)
 *
 *-------------------------------------------------------------------------*/
 
  char buf[150];                          /* general purpose buffer          */
  char wbuf[2][6];                        /* general purpose                 */

  unsigned char pickline;
  

  putenv("_=restock_rpt_create");
  chdir(getenv("HOME"));

  open_all();

  pickline = atol(argv[4]);

  for(i = 0; i < 3; i++) tmp_name(data_file[i]);
  
        /* create SQL query in file 0 */

  fp0 =fopen(data_file[0], "w");

  fprintf(fp0, setup);
  switch(*argv[1])                        /* all or under restock only       */
  {
  case 'a':                               /* all items                       */
    fprintf(fp0,"pmsku = pfsku\n");
    break;
  case 'u':                               /* under restock only              */
    fprintf(fp0,"qty < restock and pmsku = pfsku\n");
    break;
  }
  fprintf(fp0,"order by ");
  switch(*argv[2])                        /* sorting code                    */
  {
  case 'b':                               /* back up stock location          */
    fprintf(fp0,"bsloc, pmodno\n");
    break;
  case 's':                               /* CAPS stock location             */
    fprintf(fp0,"stkloc\n");
    break;
  case 'p':                               /* pick module number              */
    fprintf(fp0,"pmodno\n");
    break;
  }
  fclose(fp0);

  sprintf(buf,"SQL %s %s", data_file[0], data_file[1]);
  system(buf);

          /* determine batch number */

  fp0 = fopen(batch_no_name,"rw");
  if(fp0==0) krash("main", "open batch_no", 1);

  fread(&batch, 2, 1, fp0);               /* 2 bytes                         */
  fclose(fp0);
  //fseek(fp0, 0, 0);                       /* to beginning                    */

  if(++batch > 9999)
 	 batch=1;                  /* reset                           */

  fp0 = fopen(batch_no_name,"w");
  if(fp0==0) krash("main", "open batch_no", 1);
  fwrite(&batch, 2, 1, fp0);              /* update                          */
  fclose(fp0);

  sprintf(batch_text, "%05d", batch);

  fp1 = fopen(data_file[1],"r");          /* SQL result file                 */
  fp2 = fopen(data_file[2],"wct");        /* final file to write             */

  while (fgets(buf, RECORD_SIZE, fp1) > 0)
  {
    c1 = strtok(buf, "|");
    c2 = strtok(0, "|");
    c3 = strtok(0, "|");
    c4 = strtok(0, "|");
    c5 = strtok(0, "|");
    c6 = strtok(0, "|");
    c7 = strtok(0, "|");
    c8 = strtok(0, "|");
    c9 = strtok(0, "|");
    c10 = strtok(0, "|");
    c11 = strtok(0, "|");
    c12 = strtok(0, "|");
        
    if(!check_pl(c2, pickline))          /* no good, forget it              */
    continue;                             /* read another                    */

                /* calculate restock quantity and cases */

    rs_qty = atoi(c12) - atoi(c3);
    if(rs_qty < 0) rs_qty = 0;            /* set to zero if less             */
                                                /* case pack */
    n=atoi(c11);                          /* case pack                       */
    if (n <= 0) n = 1;                    /* default to 1                    */

    rs_cp = rs_qty / n;                   /* cases to restock                */
    rs_qty = rs_cp * n;                   /* restock in cases only !!        */
                
    if(*c6 == 'y')                      /* restock pending                 */
      strcpy(wbuf[0],"yes");
    else
      strcpy(wbuf[0]," no");

                /* print the record */

    fprintf(fp2,
/*
"%-15.15s           %5.5s   %5.5s   %-6.6s   %5d    %-6.6s      %-3.3s    \n",
*/
"%-15.15s          %5.5s  %5.5s     %-6.6s  %5d   %-6.6s       %-3.3s  \n",
     c1, c2, c3, c4, rs_qty, c5, wbuf[0]);
        
    fprintf(fp2,
    "%-25.25s  %-3.3s (%5.5s) (%-6.6s) (%5d)  (%3.3s)%14c\n",  
     c7, c8, c9, c10, rs_cp, c11, ' '); 

    fprintf(fp2, "%79.79s\n", " ");

/* write batch receipts file record, but only if restock                     */
/*   printing is disabled and restock quantity > 0.                          */

    if(*c6 != 'y' && rs_qty)
    {
      add_batch_rec(batch_text, c1, c5, c2 ,rs_qty);

      pm.p_pmodno = atol(c2);

      begin_work();
      if (!pmfile_read(&pm, LOCK))
      {
        pm.p_rsflag = 'y';
        pmfile_replace(&pm);
      }
      commit_work();
    }
  }                                       /* all records read                */
  fclose(fp1);
  fclose(fp2);

#ifndef DEBUG
  unlink(data_file[0]);                   /* DELETE FILE 0                   */
  unlink(data_file[1]);                   /* DELETE FILE 1                   */
#endif
        /* determine how to exit */

  close_all();
  if(*argv[5]=='y')                       /* print report                    */
  {
    if (fork() == 0)
    {
      execlp("prft"," prft",data_file[2],tmp_name(print_file),
        "sys/report/restock_print.h",batch_text,0);
      krash("main", "prft load", 1);
    }
    pid = wait(&status);
    if (pid < 0 || status) krash("main", "prft failed", 1);
      
    execlp("restock_rpt_input","restock_rpt_input", 0);
    krash("restock_rpt_input failed to load");
  }
/* F082587 - sd_text() didn't like "81" so changed to "80" */

  execlp("database_report","database_report",
  restock_report,data_file[2],"80","11","12","sys/report/restock_print.h",
    "restock_rpt_input",batch_text,"4","57","0",0);
  krash("main", "database_report load", 1);
}                                         /* end of MAIN                     */

/* check_pl():

        this routine checks the pickline in the SQL query result record
        to be within the view of the operator (in operator's pickline,
        and if all, non-zero). returns 1 if yes, 0 if no.
*/

check_pl(buf, pickline)               /* check that module is in operator pl*/
char *buf;
short pickline;
{
  long PM_num;
  
  if (!pickline) return 1;

  PM_num = atol(buf);

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

add_batch_rec(batch,bsku,bstkloc,bpmno,rsqty)
char *batch,*bsku,*bstkloc,*bpmno;
long rsqty;
{
  brf_open("a+");                         /* open batch receipts file        */
  brf_lock();

  fprintf(brf_fd,"%4.4s%15.15s%6.6s%5.5s%05d",
    batch, bsku, bstkloc, bpmno, rsqty);

  brf_unlock();
  brf_close();                            /* close batch receipts file       */
}

open_all()
{
  database_open();
  ss_open();
  co_open();
  pmfile_open(AUTOLOCK);
  pmfile_setkey(1);
  return 0;
}

close_all()
{
  ss_close();
  co_close();
  pmfile_close();
  database_close();
  return 0;
}

leave()
{
  close_all();
  execlp("pfead", "pfead", 1);
  krash("leave", "pfead load", 1);
}

/* end of restock_rpt_create.c */
