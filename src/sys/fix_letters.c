/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Fetches maps of large letters.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/3/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char fix_letters_c[] = "%Z% %M% %I% (%G% - %U%)";

/*  fix_letters.c
 *
 *  checks and builds letter files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LF 0x0a  

FILE *in_fd;
FILE *out_fd;

char oname[40];
long high, wide;

char *table;                              /* compressed letters              */
char *input;                              /* entire input letters            */
char c;                                   /* current input byte              */
long next;                                /* next table byte                 */
long in;                                  /* current input byte              */
char what;                                /* current letter                  */
long len;
long j, k;

main(argc,argv)
long argc;
char **argv;
{
/*  argv[1] is name of letter file i.e. letter5  */
/*  output is /cap/sys/letterx.t                 */

  putenv("_=fix_letters");
  chdir(getenv("HOME"));

  high = argv[1][6] & 0x0f;
  wide = 5;
  if (high == 9) wide = 7;
  printf("Build letter table for %s, high=%d, wide=%d\n\n",
  argv[1], high, wide);
/*
 *  open letter raw file with LFs for editing
 */
  in_fd = fopen(argv[1], "r");
  if (in_fd == 0)
  {
    printf("Can't open %s\n\n", argv[1]);
    return;
  }
/*
 * open output file in /caps/sys
 */
  strcpy(oname, "sys/");                  /* F063093                         */
  strcat(oname, argv[1]);
  strcat(oname, ".t");
  out_fd = fopen(oname, "wctu");
  if (out_fd == 0)
  {
    printf("Can't open %s\n\n", oname);
    return;
  }
/*
 *  allocate tables
 */
  table = (char *)malloc(2500);
  input = (char *)malloc(3000);
  len = fread(input, 1, 3000, in_fd);      /* read entire file               */
  for (j = len; j <3000; j++) input[j] = 0;/* clear rest of input            */
  fclose(in_fd);
/*
 *  next is next position in output table and length of table
 */
  next = 0;                               /* next output byte                */
  in   = 0;                               /* current input byte              */
  printf("Input File  = %s\n", argv[1]);
  printf("Output File = %s\n", oname);
  printf("Length of %s is %d.\n", argv[1], len);
/*
 *
 */
  while (input[in])                       /* while any data                  */
  {
    what = 0;                             /* value of letter                 */
    for (j = 0; j < high; j++)            /* for all rows of letter          */
    {
      for (k = 0; k < wide; k++)          /* for all cols of letter          */
      {
        c = input[in++];                  /* get input byte                  */
        printf("%c", c);                  /* display current byte            */
        table[next++] = c;                /* move to table                   */
        if (c == 0x20) continue;          /* space is ok                     */
        if (c == LF) abort_me();             /* bad input !!!                   */
        if (!c) abort_me();                  /* premature end                   */
        if (!what)
        {
          what = c;                       /* first non-space                 */
          continue;
        }
        if (c != what) abort_me();           /* different letter                */
      }
      c = input[in++];
      if (c != LF) abort_me();               /* bad input !!!                   */
      printf("\n");
    }
    c = input[in++];
    if (c != LF) abort_me();                 /* bad input                       */
    printf("\n");
  }
  fwrite(table, next, 1, out_fd);
  fclose(out_fd);
  printf("All Done\n\n");

  free(table);                            /* F121886                         */
  free(input);                            /* F121886                         */

  return;
}
/*
 * abort job
 */
abort_me()
{
  printf("\nInput Error\n\n");
  printf("Error at Input byte = %d\n", in);
  printf("Row = %d, Column = %d\n", j + 1, k + 1);
  printf("Byte c=%c, d=%d, x=%x\n\n", c, c, c);
  printf("Job Aborted ! \n\n");
  exit(0);
}

/* end of fix_letters.c */
