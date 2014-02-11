#include <stdio.h>


#define MAX 5000

char table[MAX][15] = {0};

long compare();

main()
{
  register long j, k;
  long start, count;
  char work[15];
  
  start = microclock();
  
  for (k = 0; k < MAX; k++)
  {
    sprintf(table[k], "%010d", k + 1);
  }
  printf("Generate: %d milliseconds\n\n", microclock() - start);

  count = 0;
  start = microclock();
  
  for (k = 1; k <= MAX; k++)
  {
    sprintf(work, "%010d", k);
    
    for (j = 0; j < MAX; j++)
    {
      count++;
      if (memcmp(work, table[j], 10) == 0) break;
    }
  }
  printf("Serial Lookup: %d milliseconds\n", microclock() - start);
  printf("Count: %d\n", count);
  
  start = microclock();
  
  for (k = 1; k <= MAX; k++)
  {
    sprintf(work, "%010d", k);
    
    bsearch(work, table, MAX, 15, compare);
  }
  printf("Binary Lookup: %d milliseconds\n", microclock() - start);
}
long compare(p, q)
register *p, *q;
{
  return memcmp(p, q, 10);
}

/* end of lookup.c */
