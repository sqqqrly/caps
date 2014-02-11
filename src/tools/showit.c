#include <stdio.h>


main()
{
  long k, now, when;
  
  microclock();
  
  for (k = 0; k < 15; k++)
  {
    when = microclock();
    sleep(1);
    now  = microclock();
    
    printf("%2d: %10d %10d\n", k, now - when, now);
  }
} 
