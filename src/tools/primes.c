/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Print Prime Numbers
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/20/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char primes_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH 26000

main()
{
  register long j, k;
  register unsigned char *p;
  
  p = (unsigned char *)malloc(LENGTH);
  if (!p) 
  {
    printf("Memory Allocation\n");
    exit(1);
  }
  memset(p, 1, LENGTH);
  
  for (j = 2; j < LENGTH / 2; j++)
  {
    for (k = j + j; k < LENGTH; k += j)
    {
      p[k] = 0;
    }
  }
  for (k = 0, j = 3; j < LENGTH; j++)
  {
    if (!p[j]) continue;

    printf("%7d", j);
    k++;
    
    if (!(k % 10)) printf("\n");
  }
  printf("\n\n");
  return 0;
}
