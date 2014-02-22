/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Product file query report.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/28/93   |  tjt  Added to mfc.
 *  07/01/94   |  tjt  Add query name to report.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *  05/18/01   |  aha  Added fix for pos[] array size, was 199 now 1000.
 *-------------------------------------------------------------------------*/
static char pfq_report_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

/************************************************************************/
/*                                                                      */
/*      product file query report                                       */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ctype.h>
#include "iodefs.h"
#include "ss.h"
#include "sd.h"
#include "eh_nos.h"
#include "language.h"
#include "pfq_report.t"

short ONE = 1;

struct fld_parms fld =
{23,20,1,1,&ONE,"Print? (y/n)          (Exit, Forward, or Backward)",'a'};

long k, m;                                /* working subscripts              */
char *p;
long block = 0;                           /* current display block           */
long max = 0;                             /* end of position table           */
long pos[1000] = {0};                     /* position of block table         */
char buf[1400];                           /* input buffer                    */
long size;                                /* size of input file              */
long pid, status;
FILE *fp;
short rm, ret;
char c;
unsigned char t;
char print_file[16];

main(argc,argv)
int argc;
char **argv;
{
  extern leave();

  putenv("_=pfq_report");
  chdir(getenv("HOME"));

  sd_open(leave);
  ss_open();
  
  fix(pfq_report);
  sd_screen_off();
  sd_clear_screen();
  sd_text(pfq_report);
  sd_screen_on();
  
        /* determine if file exists or is zero length */

  fp = fopen(argv[1],"r");

  if(!fp) leave();
   
  fseek(fp, 0, 2);
  size = ftell(fp);
  fseek(fp, 0, 0);
  
  if(size <= 0)
  {
    fclose(fp);
    unlink(argv[1]);
    leave();
  }
/*
 *  build position table
 */

/*  MR1.0 initial while loop checks for all white except space and disregards.
 *  Needed to keep initial screen from having blank lines at top.
 */
  while(1)
  {
    c = fgetc(fp);
    if (!iscntrl(c))                      /* good data                       */
    {
      fseek(fp, - 1, 1);                  /* prev pos is start               */
      break;
    }
  }
    
  while(1)
  {
    pos[max] = ftell(fp);                 /* start of block                  */
    if (max >= 999) break;                /* file too big                    */
    max++;

    for (m=0; m<16; m++)                  /* read up to 16 lines             */
    {
      p = fgets(buf, 512, fp);
      if (!p) break;                      /* is EOF                          */
      buf[strlen(buf) - 1] = 0;
    }

    if (!p)
    {
      pos[max] = size;
      max--;
      break;                              /* is EOF                          */
    }
  }
/*
 *  Display block of data
 */
  while(1)                                /* main loop to display data       */
  {
    sd_cursor(0,6,1);
    sd_clear_rest();
    fseek(fp, pos[block], 0);
    k = fread(buf, 1, (pos[block + 1] - pos[block]), fp);
    buf[k] = 0;

    sd_text(buf);
    sd_clear_rest();                      /* clear remainder of screen       */

    buf[0] = buf[1] = 0;
    sd_prompt(&fld,0);
    while(1)                              /* loop to process print prompt    */
    {
      t = sd_input(&fld,0,&rm,buf,0);
      switch(ret=(sd_print(t, code_to_caps(buf[0]))))  /* F041897 */
      {
        case(0):

          fclose(fp);
          unlink(argv[1]);
          leave();

        case(1):

          if (block < max) block++;
          break;

        case(2):

         if (block > 0) block--;
          break;

        case(3):

          fclose(fp);
          unlink(argv[1]);
          leave();

        case(4):

          fclose(fp);

          if(fork()==0)                     /* child process                 */
          {
            execlp("prft","prft", argv[1], tmp_name(print_file), 
             "sys/report/pfq_print_report.h", argv[2], 0);
            krash("main", "prft load", 1);
          }
          pid = wait(&status);
          if (pid < 0 || status) krash("main", "prft failed", 1);
          leave();

        case(6):                          /* error                           */

         eh_post(ERR_YN,0);
      }
      if(ret != 6)                        /* not meaningless                 */
      break;
    }
  }
}
leave()
{
  sd_close();
  ss_close();
  execlp("product_file_query", "product_file_query", 0);
  krash("leave", "product_file_query load", 1);
}

/* end of pfq_report.c */
