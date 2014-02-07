/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Processes fl_text to make fl_table.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   7/20/93   |  tjt  Rewritten.
 *-------------------------------------------------------------------------*/
static char fl_init_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Format of sys/fl_text
 *
 *  #  lines beginning with pound sign are remarks
 *
 *  nn = Screen nn        (lines beginning with a number are a group).
 *  A = feature selection (an upper case letter is a feature).
 *  B # feature selection (a pound sign means NOT available).
 *  For example,
 *
 *  01 = Main Menu Screen 1.0
 *  C = Configuration Entry
 *  E = Order Entry
 *  F # Product File Entry         (# means feature NOT available)
 *  G # Labels and Packing Lists   (# means feature NOT available)
 *  L = Logoff
 *  O = Operations Commands
 *  P = Productivity and Manpower
 *  S = System Commands
 *  02 = Operations Menu Screen 2.0
 *  A = Alter Swtich Action
 *  B = Order Input
 *
 *  A feature group is packed into one word where A == 1, B == 2, C == 4, ..
 *-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_names.h"
#include "features.h"

FILE *fd;                                 /* useful file descriptor          */

Tfeatures_item fl = {0};

main()
{
  register long j, k;
  char buf[128];
  
  putenv("_=fl_init");                    /* name to environ                 */
  chdir(getenv("HOME"));                  /* insure in home director         */

  fd = fopen(fl_text_name, "r");          /* features text                   */
  if (fd == 0)
  {
    printf("*** Can't open %s\n\n", fl_text_name);
    return;
  }
/*
 * Load and Process Features Text
 */
  printf("Convert %s To %s Format\n\n", fl_text_name, fl_table_name);
  
  while (fgets(buf, 128, fd) > 0)         /* while any data                  */
  {
    printf("%s", buf);                    /* show input data                 */

    if (*buf == '#') continue;            /* is remarks                      */

    else if (*buf >= '1' && *buf <= '8')  /* is a feature group              */
    {
      k = *buf - '1';
      j = 0;
    }
    else if (*buf >= 'A' && *buf <= 'Z')  /* is a feature                    */
    {
      if (buf[2] != '=') continue;        /* ignore this entry               */

      if (j > 0)
      {
        if (memchr(fl.fword[k], *buf, j))
        {
          printf("*** Duplicate Entry - %s\n", buf);
		    exit(1);
        }
      }
      fl.fword[k][j++] = *buf;
    }
    else
    {
      printf("*** Unrecognized - %s\n", buf);
      exit(1);
    }
  }
  fclose(fd);

  fd = fopen(fl_table_name, "w");
  if (fd == 0)
  {
    printf("*** Can't open sys/fl_table\n\n");
    exit(1);
  }
  for (k = 0; k < 8; k++)
  {
    fprintf(fd, "%s\n", fl.fword[k]);
  }
  fclose(fd);
   
  printf("All Done\n\n");
}
/*
 *  ascii to binary convert
 */
cvrt(p)
char *p;
{
  long i;

  i = 0;

  while (*p >= '0' && *p <= '9')
  {
    i = i * 10 + (*p++ & 0x0f);
  }
  return i;
}

/* end of fl_init.c */
