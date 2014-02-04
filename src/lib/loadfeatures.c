/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Load Features Table.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/21/93    |   tjt  Original Implementation
 *-------------------------------------------------------------------------*/
static char load_features_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"
#include "features.h"

Tfeatures_item fl;                        /* features data area              */

load_features()
{
  register long k;
  FILE *fd;
   
  fd = fopen(fl_table_name, "r");
  if (fd == 0)
  {
    return krash("load_features", "Can't open fl_table", 1);
  }
  for (k = 0; k < 8; k++)
  {
     if (fgets(fl.fword[k], 32, fd) <= 0)
     {
       return krash("load_features", "Read error on fl_table", 1);
     }
     fl.fword[k][strlen(fl.fword[k]) - 1] = 0;
  }
  fclose(fd);

  return 0;
}

/* end of loadfeatures.c */
