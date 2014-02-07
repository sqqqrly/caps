/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Print codes in sys/fl_table.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/21/93    |  tjt  Rewritten.
 *-------------------------------------------------------------------------*/
static char fl_dump_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "file_names.h"
#include "features.h"

char *name[8]  = {

  "Main Menu  1.0",
  "Operations Menu  2.0",
  "Systems Commands Menu  3.0",
  "Configuration Menu  4.0",
  "Productivity and Manpower Menu  5.0",
  "Unused Menu Options 6.0",
  "Product File Menu  7.0",
  "Label and Packing List Menu  8.0"};
   
main()
{
  register long j, k, mask;

  putenv("_=fl_dump");                    /* name to environ                 */
  chdir(getenv("HOME"));                  /* insure in home directory        */

  load_features();                        /* get the features table          */

  printf("Current Features in %s\n\n", fl_table_name);

  for (k = 0; k < 8; k++)
  {
    printf("%-36s = %s\n", name[k], fl.fword[k]);
  }
  printf("\nAll Done\n");
}

/* end of fl_dump.c */
