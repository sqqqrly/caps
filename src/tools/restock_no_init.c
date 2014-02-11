/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Reset restock number to one.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/5/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char restock_no_init_c[] = "%Z% %M% %I% (%G% - %U%)";

/*  restock_no_init.c
 *
 *   Reset to 1
 */
#include <stdio.h>
#include "file_names.h"

FILE *fd;
long count = 1;

main()
{
  putenv("_=restock_no_init");
  chdir(getenv("HOME"));

  fd = fopen(restock_no_name, "w");
 
  if (fd == 0)
  {
    printf("Restock_no Open Failed\n\n");
  }
  fwrite(&count, 4, 1, fd);

  fclose(fd);

  printf("Restock_no reset to 1\n\n");
  
  return;
}

/* end of restock_no_init.c */
