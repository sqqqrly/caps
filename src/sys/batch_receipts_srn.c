/*---------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Batch Receipts Screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/02/93    |  tjt  Added to mfc.
 * 07/21/95    |  tjt  Revise Bard calls.
 * 08/23/96    |  tjt  Add commit work.
 * 04/18/97    |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char batch_receipts_srn_c[] = "%Z% %M% %I% (%G% - %U%)";
/************************************************************************/
/*                                                                      */
/*      batch_receipts_srn.c         screen 7.12.1                   */
/*                                                                      */
/*      this routine updates UNIFY, writes a 'batch transaction'        */
/*      record in the order/ptf file, and updates module table for      */
/*      each pick module noted in a previously created restock report,  */
/*      excluding exceptions entered by the operator. exceptions        */
/*      may take any of the following forms:                            */
/*                                                                      */
/*      SKU only entered:       no restocking performed for any of      */
/*                              the pick modules assigned to the SKU.   */
/*      SKU, PM, no qty:        no restocking was performed for the     */
/*                              entered pick module.                    */
/*      SKU, PM, quantity:      the pick module was restocked by the    */
/*                              quantity entered.                       */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "language.h"
#include "batch_receipts_srn.t"
#include "Bard.h"
#include "bard/pmfile.h"

extern FILE *brf_fd;

pmfile_item pm;

#define NUM_PROMPTS 6
#define BUF_SIZE 16

#define RECORD_SIZE     37               /* size of record in batch file    */
#define BATCH_SIZE      4              /* size of batch number part of record*/
#define REM_SIZE        RECORD_SIZE-BATCH_SIZE

short ONE = 1;
short FOUR = 4;
short FIVE = 5;
short LSL;
short LSKU;
short LMOD;

struct fld_parms fld[] = {

/* initial prompts */

  {6,30,1,1,&FIVE,"Enter Batch Number",'n'},
  {7,30,1,1,&ONE,"Is Batch Complete? (y/n)",'a'},

/* secondary prompts */

  {11,30,1,1,&LSKU,"Enter SKU",'a'},
  {12,30,1,1,&LSL, "Enter CAPS Stock Location",'a'},
  {13,30,1,1,&LMOD,"Enter PM Number",'n'},
  {14,30,1,1,&FIVE,"Enter Quantity Restocked",'n'}

};

/* processing of batch receipts is performed as follows:

        step 1: the batch receipts records for the entered batch
                number are read into memory. the batch receipts file
                is not accessed again until the records are processed.

        step 2: if there are exceptions, these are processed as they
                are entered. after exception entering is complete (or
                if there were no exceptions), the batch records (now
                in memory) are processed. the PM numbers for excepted
                records are written as longs to an exception file. the
                exception file is read into memory and during batch
                processing each record to be processed has its PM
                number checked against the excepted PMs in memory.

        step 3: the records in the batch receipts file for the entered
                batch are cleared to NULLs. the file is cleaned up at
                end of day maintenance.
*/
struct batch_record
{
  char batch_number[4];
  char batch_asku[15];
  char batch_stkloc[8];
  char batch_pmno[5];
  char batch_rsqty[5];
};

FILE *bug;                                /* DEBUG file                      */

char text[80];

main()
{
  short rm,ret,i,j,k,n;                   /* general purpose                 */
  char c;                                 /* general purpose                 */
  unsigned char t;                        /* return from sd_input            */
  char buf[NUM_PROMPTS][BUF_SIZE];        /* holds operator input            */

  long PM_num;                            /* pick module for UNIFY access    */
  short good_batch;                       /* if set, correct entry           */
  long batch_start;                       /* start of batch records          */
  char batch_text[5];
  short found;                            /* gp flag                         */
  short rec_count;                        /* count of records in batch       */
  short ex_entered;                       /*if set least 1 exception entered */

  short s_rpt_flg=0;                      /* if set, modifiy sec. prompts    */

  char wbuf[50];                          /* holds batch num for compare     */
  char rec_buf[RECORD_SIZE+1];            /* holds complete record           */

  short etype;                            /* type of exception entered       */
  short exceptions;                       /* if set, exceptions entered      */
  struct batch_record *br;                /* for memory table                */
  FILE *fp;                               /* file descr. for exception file  */
  char tfile[16];                         /* temporary file containing ex.   */
  long *ex;                               /* pointer to exception table      */

  putenv("_=batch_receipts_srn");
  chdir(getenv("HOME"));
  
  open_all();

        /* set lengths into parameters structures */

  LSKU = rf->rf_sku;                      /* SKU length                      */
  LMOD = rf->rf_mod;                      /* Pick Module length              */
  LSL  = rf->rf_stkloc;                   /* Stock Location                  */
  
  if (LSKU < 1) LSKU = SkuLength;
  if (LMOD < 1) LMOD = ModuleLength;
  if (LSL  < 1) LSL  = StklocLength;
  
  fix(batch_receipts_srn);
  sd_clear_screen();                      /* clear entire screen             */
  sd_screen_off();
  sd_text(batch_receipts_srn);
  sd_screen_on();

        /* clear buffers */

  for(i=0;i<2;i++)
  {
    for(n=0; n<BUF_SIZE; n++)
    {
      buf[i][n] = 0;
    }
  }
  exceptions=0;                           /* clear flag                      */
  ex_entered=0;                           /* clear exceptions entered flag   */

        /************************************************************/
        /* process initial inputs for batch number and completeness */
        /************************************************************/

  for(i = 0; i < 2; i++) sd_prompt(&fld[i], 0);
  i = 0;                                  /* reset index                     */

  while(1)
  {
    t = sd_input(&fld[i],0,&rm,buf[i],0);
    if(t == EXIT) leave();
    else if(t == UP_CURSOR && i > 0)
    i--;
    else if(t == DOWN_CURSOR || t == TAB)
    {
      if(i < 1)
      i++;
      else
      i = 0;
    }
    else if(t == RETURN)
    {
                        /* validate entered batch number */

      good_batch = 0;
      brf_open("r");
      
      while(1)                            /* till end of file                */
      {
        if (fread(batch_text, BATCH_SIZE, 1, brf_fd) != 1) break;
        if (atoi(batch_text) == atoi(buf[0]))
        {
          good_batch=1;
          rec_count=1;                    /* set                             */
          batch_start = ftell(brf_fd) - BATCH_SIZE;
          fseek(brf_fd, REM_SIZE, 1);
          while (fread(rec_buf, RECORD_SIZE, 1, brf_fd) == 1)
          {
            movebytes(rec_buf,wbuf,BATCH_SIZE);
            wbuf[BATCH_SIZE] = 0;
            if(atoi(batch_text) == atoi(wbuf)) rec_count++;
            else break;
          }
          break;
        }
        fseek(brf_fd, REM_SIZE, 1);
      }
      brf_close();
      if(!good_batch)
      {
        eh_post(ERR_BATCH_NO,buf[0]);
        i=0;
        continue;
      }
                        /* batch num ok, validate yes/no response */

      *buf[1] = tolower(*buf[1]);
      if(code_to_caps(*buf[1]) != 'y' && code_to_caps(*buf[1]) != 'n')
      {
        eh_post(ERR_YN,0);
        i=1;
        continue;
      }
                        /* initial input ok */
      break;                              /* from input loop                 */
    }
  }                                       /* end first input                 */
        /* build memory table of batch records. this is used to validate
           the secondary input, and also used by the batch process function.
           batch receipts file records have the following format:

                0-3             batch number.
                4-18            associated SKU.
                19-24           CAPS stock location.
                25-29           ascii pick module.
                30-34           ascii restock quantity.
        */

  br = (struct batch_record *)malloc(rec_count * RECORD_SIZE);
  brf_open("r+");
  fseek(brf_fd, batch_start, 0);
  fread(br, RECORD_SIZE, rec_count, brf_fd);
  brf_close();

  if (code_to_caps(*buf[1]) == 'y')
  {
    process_batch(br,rec_count,0,0);
    clear_batch(batch_start,rec_count);
    leave();
  }
        /***********************************************/
        /* batch not complete. process exception input */
        /***********************************************/

  tmp_name(tfile);                   /* get temporary filename for exceptions*/
  sd_cursor(0,9,1);
  sd_text("Enter Exceptions");

  while(1)                                /* main loop for exceptions        */
  {
                /* see if we need to modify prompts */

    if(s_rpt_flg++ == 1)                  /* second time around              */
    {
      for(i = 2; i < NUM_PROMPTS; i++)
      {
        *(fld[i].prompt) = 0;
        fld[i].arrow     = 0;
      }
    }
                /* clear buffers */

    for (i = 2; i < NUM_PROMPTS; i++)
    {
      for (n = 0; n < BUF_SIZE; n++) buf[i][n] = 0;
    }
                /* display prompts */

    for(i = 2; i < NUM_PROMPTS; i++) sd_prompt(&fld[i], 0);
    
    i = 2;                                /* reset index                     */

        /* input loop is designed to allow only CAPS or PM number entry */

    while(1)                              /* input loop                      */
    {
      t = sd_input(&fld[i],0,&rm,buf[i],0);
      if(t == EXIT)
      {
        if(ex_entered)
        {
#ifdef DEBUG
          fprintf(bug,"about to process those which weren't exceptions\n");
#endif
          process_batch(br,rec_count,1,tfile);
          clear_batch(batch_start,rec_count);
          leave();
        }
        else
        {

          eh_post(ERR_BATCH_NO_PROC,0);
          leave();
        }
      }
      else if(t == UP_CURSOR && i > 2)
      {
        if(i==5)                          /* quantity                        */
        i=(*buf[4] || (!(*buf[4]) && !(*buf[3])))?4:3;
        else if(i==4)                     /* PM                              */
        i=(*buf[4])?2:3;
        else                              /* i is 3                          */
        i=2;
      }
      else if(t == DOWN_CURSOR || t == TAB)
      {
        if(i==2)
        i=(*buf[3] || (!(*buf[3]) && !(*buf[4])))?3:4;
        else if(i==3)
        i=(*buf[3])?5:4;
        else if(i==4)
        i=5;
        else                              /* i is 5                          */
        i=2;
      }
      else if(t == RETURN)
      {
                                /* validate all input. etype is set as shown:

                                        etype=1         SKU only
                                        etype=2         SKU and CAPS
                                        etype=3         SKU and PM
                                */

        etype=0;
        if(*buf[2])                       /* SKU entered                     */
        {
          strip_space(buf[2],15);
          found=0;
          for(k=0;k<rec_count;k++)
          {
            movebytes(br[k].batch_asku,wbuf,15);
            wbuf[15] = 0;
            strip_space(wbuf,15);
            if(streql(buf[2],wbuf))
            {
              found=1;
              break;
            }
          }
          if(!found)
          {
            eh_post(ERR_SKU_BATCH,0);
            i=2;
            continue;
          }
        }
        else                              /* SKU not entered                 */
        {
          eh_post(ERR_SKU_INV,0);
          i=2;
          continue;
        }
                                /* SKU processed. check CAPS */

        if(*buf[3])                       /* CAPS entered                    */
        {
          etype=2;
          found=0;
          for(k=0;k<rec_count;k++)
          {
            movebytes(br[k].batch_stkloc, wbuf, 8);
            wbuf[8] = 0;
            if(streql(buf[3],wbuf))
            {
              found=1;
              break;
            }
          }
          if(!found)
          {
            eh_post(ERR_CAPS_BATCH,0);
            i=3;
            continue;
          }
          movebytes(br[k].batch_asku,wbuf,15);
          wbuf[15] = 0;
          strip_space(wbuf,15);
          if(!streql(wbuf,buf[2]))
          {
            eh_post(ERR_CAPS_SKU,0);
            i=3;
            continue;
          }
        }
        else if(*buf[4])                  /* PM entered                      */
        {
          etype=3;
          found=0;
          for(k=0;k<rec_count;k++)
          {
            movebytes(br[k].batch_pmno,wbuf,5);
            wbuf[5] = 0;
            for(n=0;n<5;n++)
            {
              if(wbuf[n]==SPACE)
              wbuf[n]='0';
              else
              break;
            }
            if(atoi(buf[4])==atoi(wbuf))
            {
              found=1;
              break;
            }
          }
          if(!found)
          {
            eh_post(ERR_PM_BATCH,0);
            i=4;
            continue;
          }
          movebytes(br[k].batch_asku,wbuf,15);
          wbuf[15] = 0;
          strip_space(wbuf,15);
          if(!streql(wbuf,buf[2]))
          {
            eh_post(ERR_PM_SKU,0);
            i=4;
            continue;
          }
        }
        else if(*buf[5] && atoi(buf[5]))
        {
                                        /* SKU and non-zero qty entered */
          eh_post(ERR_CAPS_PM,0);
          i=3;
          continue;
        }
        else                              /* SKU only                        */
        etype=1;
        ex_entered=1;                     /* an exception entered            */
        break;                            /* from input loop                 */
      }
      else
      ;
    }                                     /* end 2 input loop                */

                /* all input validated. process exceptions */

    fp=fopen(tfile,"a+");                 /* exception file                  */
    if(etype==1)                   /* all PMs with this SKU are not restocked*/
    {
      for(k=0;k<rec_count;k++)
      {
        movebytes(br[k].batch_asku,wbuf,15);
        wbuf[15] = 0;
        strip_space(wbuf,15);
        if(!streql(buf[2],wbuf))          /* not this SKU                    */
        continue;
                                /* implied else here */
        movebytes(br[k].batch_pmno,rec_buf+25,5);
        movebytes("00000",rec_buf+30,5);  /* qty=0                           */
        movebytes(br[k].batch_number,rec_buf+0,4);
        process_record(rec_buf);
        movebytes(br[k].batch_pmno,wbuf,5);
        wbuf[5] = 0;
        for(j=0;j<5;j++)
        {
          if(wbuf[j]==SPACE)
          wbuf[j]='0';
          else
          break;
        }
        PM_num=atoi(wbuf);
        fwrite(&PM_num,4, 1, fp);
      }
    }
    else                                  /* etype is 2 or 3. k is correct   */
    {
      movebytes(br[k].batch_pmno,rec_buf+25,5);
      if(!(*buf[5]) || atoi(buf[5])==0)
      strcpy(wbuf,"00000");
      else                                /* positive quantity entered       */
      {
                                /* right-justify qty in wbuf */
        for(j=4,i=4;j>=0;j--)
        if(buf[5][j])                     /* not a NULL                      */
        wbuf[i--]=buf[5][j];
        while(i>=0)
        wbuf[i--]='0';                    /* ascii zero                      */
      }
      movebytes(wbuf,rec_buf+30,5);
      movebytes(br[k].batch_number,rec_buf+0,4);
      process_record(rec_buf);
      movebytes(br[k].batch_pmno,wbuf,5);
      wbuf[5] = 0;
      for(j=0;j<5;j++)
      {
        if(wbuf[j]==SPACE)
        wbuf[j]='0';
        else
        break;
      }
      PM_num=atoi(wbuf);
      fwrite(&PM_num, 4, 1, fp);
    }
    fclose(fp);                           /* close exceptions file           */
  }                                       /* end exceptions loop             */
}                                         /* end prog                        */

/********************************************************/
/*      functions for batch_receipts_srn.c           */
/********************************************************/

leave()
{
  close_all();
  execlp("pfead", "pfead", 0);
  krash("leave", "pfead load", 1);
}
open_all()
{
  database_open();
  sd_open(leave);
  ss_open();
  pmfile_open(AUTOLOCK);
  pmfile_setkey(1);
  log_open( AUTOLOCK );
}

close_all()
{
  ss_close();
  pmfile_close();
  log_close();
  sd_close();
  database_close();
}


/* process_batch():

        this routine processes a batch receipts transaction.
*/

process_batch(br,rec_count,eflag,efile)
struct batch_record *br;
short rec_count;
short eflag;                              /* exceptions flag                 */
char efile[15];                           /* filename for exceptions         */
{
  char c;                                 /* general purpose                 */
  FILE *fp;                               /* for exceptions file             */
  char outbuf[RECORD_SIZE+1];             /* holds batch receipt record      */
  char wbuf[6];                           /* general purpose                 */
  long esize;
  short i,j,k,n;
  short found;
  long *exp;                        /* F103188 - to read PM number for search*/
  long PM_num;
  char *ex;                           /* used to get exception file into mem.*/

  if(eflag)                               /* there are exceptions! there are!*/
  {
    fp = fopen(efile,"r");
    fseek(fp, 0, 2);
    esize = ftell(fp);
    ex = (char *)malloc(esize);
    fseek(fp, 0, 0);
    fread(ex, esize, 1, fp);
    fclose(fp);
    unlink(efile);                       /* DELETE EXCEPTION FILE           */
  }
  for(k=0; k < rec_count; k++)
  {
    if(eflag)                             /* there are exceptions! etc...    */
    {
      found=0;
      movebytes(br[k].batch_pmno,wbuf,5);
      wbuf[5] = 0;
      for(j=0;j<5;j++)
      {
        if(wbuf[j]==SPACE)
        wbuf[j]='0';
        else
        break;
      }
      PM_num=atoi(wbuf);
/*  
 * F103188 - the test on PM_num was incorrect in that a long was compared 
 * to a character
 */ 
      for(i=0;i<esize;i+=4)
      {
        exp = (long *)(ex+i);             /* F103188                         */
        if(PM_num == *exp)                /* excepted                        */
        {
          found=1;
#ifdef DEBUG
          fprintf(bug,"PM %d was excepted\n",PM_num);
#endif
          break;
        }
      }
      if(found)
      continue;
    }
                /* record not excepted or no exceptions */

    movebytes(br[k].batch_pmno,outbuf+25,5);
    movebytes(br[k].batch_rsqty,outbuf+30,5);
    movebytes(br[k].batch_number,outbuf+0,4);
    n=process_record(outbuf);
  }
}

/* process_record():

        this routine processes a single record for either an exception
or a batch process. if an exception, the passed buffer should resemble
a batch receipt record, in terms of position of PM (required) qty, and
batch number (required for product file maint. report).

*/

process_record(buf)
char *buf;                                /* passed RECORD_SIZE buffer       */
{
  long quan,iquan,save_quan,PM_num;
  char wbuf[6];
  short i, n;                             /* general purpose                 */
  char c;                                 /* general purpose                 */

  movebytes(buf+25,wbuf,5);               /* get pick module number          */
  wbuf[5] = 0;
  for(n=0;n<5;n++)
  {
    if(wbuf[n]==SPACE)
    wbuf[n]='0';
    else
    break;
  }
  pm.p_pmodno = atoi(wbuf);
  begin_work();
  if (pmfile_read(&pm, LOCK))
  {
    commit_work();
    return 1;                             /* not found                       */
  }
  //commit_work();
  movebytes(buf+30,wbuf,5);               /* get passed quantity             */
  wbuf[5] = 0;
  for(n=0;n<5;n++)
  {
    if(wbuf[n]==SPACE)
    wbuf[n]='0';
    else
    break;
  }
  iquan = atol(wbuf);
  pm.p_qty += iquan;                      /* update quantity on hand         */

        /* update history counts */

  pm.p_curecpt += iquan;
  pm.p_cmrecpt += iquan;

        /* set restock pending flag to no */

  pm.p_rsflag = 'n';

  pmfile_update(&pm);
  commit_work();
  
  sprintf(text, "%s %d %s %s %s %d", 
    "Module", pm.p_pmodno, "Batch", wbuf, "Receipt", iquan);
  log_entry(text);
  
  return(0);                              /* good return                     */
}

/* clear_batch()

        this routine clears the batch records in the order/brf file
to NULLs.

*/

clear_batch(pos,count)
long pos;
short count;
{
  short i;
  short k=count * RECORD_SIZE;

  brf_open("w");
  brf_lock();
  fseek(brf_fd, pos, 0);
  for(i=0;i<k;i++)
  putc(0,brf_fd);
  brf_unlock();
  brf_close();
}

/* end of batch_receipts_srn.c */
