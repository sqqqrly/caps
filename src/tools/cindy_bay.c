/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Generate box modules.
 *
 *  Execution:      cindy_bay >[filename]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/05/97   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char cindy_bay_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

typedef struct
{
  char  id[4];
  short shelves;
  short first;
  short bins;
} bay_def;

bay_def bay[12] = {{"D00", 4, 1, 8},
                   {"D20", 4, 1, 8},
                   {"D40", 4, 1, 8},
                   {"D60", 4, 1, 8},
                   {"D80", 4, 1, 8},
                   {"D90", 3, 6, 5},
                   {"D90", 3, 1, 5},
                   {"D70", 4, 1, 8},
                   {"D50", 4, 1, 8},
                   {"D30", 4, 1, 8},
                   {"D10", 4, 1, 8},
                   {"DB0", 2, 1, 8}};

long max = 12;

main(argc, argv)
long argc;
char **argv;
{
  register long j, k, m, n, mod, qty, restock, rqty, lcap;
  
  fprintf(stderr, "Generate pmfile in ascii load format\n\n");
  
  mod = 0;
  qty = 100;
  restock = 90;
  rqty = 50;
  lcap = 1000;
  
  for (j = 0; j < max; j++)
  {
    for (m = bay[j].shelves; m > 0; m--)
    {
      for (n = 0; n < bay[j].bins; n++)
      {
         mod++;
         /* mod|sku|qty|alloc|restock|rqty|lcap|stkloc|disp|idx|pi|  */
         printf("%d||%d|0|%d|%d|%d|%3.3s%1d%02d||0|n|0|0|0|0|0|0|n|n|\n",
           mod, qty, restock, rqty, lcap, bay[j].id, m, n + bay[j].first);
      }
    }
  }
}

/* end of cindy_bay.c */

