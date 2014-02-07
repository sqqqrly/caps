/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Reset batch number to one.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/1/93    |  tjt Added to mfc.
 *-------------------------------------------------------------------------*/
static char batch_no_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"

FILE *fd;

short count = 1;

main()
{
  putenv("_=batch_no_init");
  chdir(getenv("HOME"));

  fd = fopen(batch_no_name, "w");
 
  if (fd == 0)
  {
    printf("*** Open %s Failed\n\n", batch_no_name);
  }
  if (fwrite(&count, 2, 1, fd) != 1)
  {
    printf("*** Write Error\n\n");
  }
  fclose(fd);

  printf("Batch Number Reset To 1\n\n");
  
  return;
}

/* end of batch_no_init.c */
