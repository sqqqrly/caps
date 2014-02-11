/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Summarizes response time.
 *
 *  Execution:      sumtime [start xx:xx] [stop xx:xx] [file]
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  12/09/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char sumtime_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *fd;
char fd_name[64];

char buf[256];

main(argc, argv)
long argc;
char **argv;
{
  register long len, k;
  register long count, ticks;
  long tab[10];
  
  float ms, worst, total;
  
  putenv("_=sumtime");
  
  printf("CAPS Response Statistics\n\n");
  
  if (argc < 4)
  {
    fprintf(stderr, "Parms are [start xx:xx] [stop xx:xx] [file]\n\n");
    exit(0);
  } 
  strcpy(fd_name, argv[3]);
  
  fd = fopen(fd_name, "r");
  if (fd == 0)
  {
    fprintf(stderr, "Can't Open %s\n\n", fd_name);
    exit(0);
  }
  printf("Starting Time: %s\n", argv[1]); 
  printf("Ending   Time: %s\n", argv[2]);
  
  count = 0;
  worst = 0.0;
  total = 0.0;
  
  for (k = 0; k < 10; k++) tab[k] = 0;
  
  while (fgets(buf, 250, fd) > 0)
  {  
    len = strlen(buf);
    buf[len - 1] = 0;

/*  printf("[%s] len=%d\n", buf, len);   */
    
    if (len < 29) continue;
    if (memcmp(buf + 24, "Time=", 5) != 0) continue;
    if (memcmp(buf, argv[1], 5) < 0) continue;
    if (memcmp(buf, argv[2], 5) > 0) break;
    
    ticks = atol(buf + 29);
    if (ticks < 0) continue;
    
    ms = (float)ticks / 1000.0;            /* to ms from microseconds        */
    if (ms > 30000.0) continue;            /* ignore over 30 seconds         */
    k = ms / 1000.0;                       /* truncated seconds              */
    
 /* printf("ms=%8.3f  k=%d\n", ms, k); */
    
    if (k > 9) k = 9;
    tab[k] += 1;
    count += 1;
    
    total += ms;
    if (ms > worst) worst = ms;
  }
  fclose(fd);
  
  printf("\n");
  printf("Zone Underway Events:     %d\n\n", count);
  
  if (count > 0)
  {
    printf("Average Response Time: %8.3f ms\n", total / (float)count );
    printf("Worst   Response Time: %8.3f ms\n", worst);
  }
  printf("\nDistribution of Response Time\n\n");
  printf("Seconds   Count  Percent\n");
  printf("-------   -----  -------\n");
  
  for (k = 0; k < 10; k++)
  {
    printf("%3d-%-2d   %4d   %5d\n",
      k, k + 1, tab[k], (tab[k] * 100) / count);
  }
  printf("\n\nAll Done\n\n");
  exit(0);
}

/* end of sumtime.c */
