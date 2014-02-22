/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Product file formatted query report print.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/29/93   |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char pff_print_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      pff_print.c                                                     */
/*                                                                      */
/*      formats report file for printing...                             */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"

#define WIDTH           80         /* characters per line (including NEWLINE)*/
#define SKU_LINES       6               /* current number of lines for an SKU*/
#define PM_LINES        10                /* current number of lines for a PM*/
long pid, status;

main(argc,argv)
int argc;
char **argv;                           /* argv[1] contains filename to format*/
{
  FILE *fpr;
  FILE *fpw;
  char print_name[16];                    /* output file                     */
  char temp_file[16];                     /* used by prft to print           */
  short i;
  char buf[5];
  char c;

  putenv("_=pff_print");
  chdir(getenv("HOME"));

#ifdef DEBUG
  fprintf(stderr, "pff_print: file=%s\n", argv[1]);
#endif
  
  for(i=0;i<5;i++)
  buf[i]=0;                            /* clear comparison buffer         */
  tmp_name(print_name);

  fpr = fopen(argv[1], "r");
  fpw = fopen(print_name, "w");

  while (fread(buf, 4, 1, fpr) > 0)       /* read 4 bytes                    */
  {
    fseek(fpr, -4, 1);                    /* go back!                        */
    if(streql(buf,"SKU "))                /* this is SKU rec.                */
    {
      fprintf(fpw, "----------------------------------------");
      fprintf(fpw, "----------------------------------------\n\n");

      xfer(fpr,fpw,SKU_LINES*WIDTH);      /* xfer to output                  */
    }
    else                                  /* this is PM rec.                 */
    {
      xfer(fpr,fpw,PM_LINES*WIDTH);       /* xfer to output                  */
    }
  }
  fclose(fpr);
  fclose(fpw);
#ifndef DEBUG
  unlink(argv[1]);
#endif
  if (fork() == 0)
  {
    execlp("prft","prft",print_name,tmp_name(temp_file),
    "sys/report/pff_print_report.h",0);
    krash("main", "prft load", 1);
  }
  pid = wait(&status);
  if (pid < 0 || status) krash("main", "prft failed", 1);
    
  execlp("pff_inquiry_input","pff_inquiry_input",0);
  krash("main", "pff_inquiry_input load", 1);
}                                         /* END OF MAIN                     */

xfer(fpr,fpw,count)
FILE *fpr;
FILE *fpw;
short count;
{
  char buf[PM_LINES*WIDTH];
  fread(buf, count, 1, fpr);
  fwrite(buf, count, 1, fpw);
}

/* end of pff_print.c */
