/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Substitute for the UNOS delay function.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/27/95   | tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char delay_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>

long delay_catcher();

delay(n)
register long n;
{
  static struct itimerval x = {{0, 0}, {0, 0}};
  
  if (n <= 0) return 0;
  
  if (n > 9) 
  {
    x.it_value.tv_sec = n / 10;
    n -= 10 * x.it_value.tv_sec;
  }
  else x.it_value.tv_sec = 0;

  x.it_value.tv_usec = n * 100000;
    
  signal(SIGALRM, delay_catcher);

  setitimer(ITIMER_REAL, &x, 0);
  pause();
  return 0;
}
long delay_catcher()
{
  return 0;
}
/*-------------------------------------------------------------------------*
 *  caps_interval replaces eccall using clock or alarm calls
 *-------------------------------------------------------------------------*/
caps_interval(n, func)
register long n;
register long *func();
{
  static struct itimerval y = {{0, 0}, {0, 0}};
  static struct itimerval x = {{0, 0}, {0, 0}};
  
  if (n <= 0) 
  {
    setitimer(ITIMER_REAL, &y, 0);
    return 0;
  }
  if (n > 9) 
  {
    x.it_value.tv_sec = n / 10; 
    n -= 10 * x.it_value.tv_sec;
  }
  else x.it_value.tv_sec = 0;

  x.it_value.tv_usec = n * 100000;
    
  signal(SIGALRM, func);

  setitimer(ITIMER_REAL, &x, 0);

  return 0;
}
/*-------------------------------------------------------------------------*
 *  microclock replacement
 *-------------------------------------------------------------------------*/
long microclock()
{
  struct timeb t;
  static long start_secs = 0;
  static long start_mill = 0;
  
  ftime(&t);
  
  if (start_secs <= 0)
  {
    start_secs = t.time;
    start_mill = t.millitm;
  }
  return ((t.time - start_secs) * 1000 + (t.millitm - start_mill));
}

/* end of delay.c */
