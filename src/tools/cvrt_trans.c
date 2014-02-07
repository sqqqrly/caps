/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Convert transaction file to seven digits.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/08/94   | tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char cvrt_trans[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

FILE *in, *out;

char buf[128];

main(argc, argv)
long argc;
char **argv;
{
  printf("Convert Transaction File\n\n");
  
  in = fopen("/mfc/dat/db/transaction.old", "r");
  
  if (in == 0)
  {
    printf("Can't open transaction.old file\n\n");
    exit(1);
  }
  out = fopen("/mfc/dat/db/transaction.dat", "w");
  if (out == 0)
  {
    printf("Can't Open transaction.dat\n\n");
    exit(1);
  }
  while (fread(buf, 64, 1, in) == 1)
  {
    fprintf(out, "%23.23s0%41.41s", buf, buf + 23);
    fflush(out);
  }
  fclose(in);
  fclose(out);
}

/* end of cvrt_trans.c */
