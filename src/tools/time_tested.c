/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Test Time Convert.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *             |
 *             |
 *-------------------------------------------------------------------------*/
static char time_tested_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>

main()
{
  long now, what;
  char buf[32];
  struct timeb milli;
  
  now = time(0);
  ftime(&milli);
  
  printf("Time Is %24.24s\n\n", ctime(&now));
  printf("Time:   now=%10d\n", now);
  printf("Milli:     =%10d.%03d\n", milli.time, milli.millitm);
  
  printf("Enter    mm/dd/yyyy hh:mm:ss\n");
  printf("Enter -->");
  gets(buf);

  time_convert(buf, &what);

  printf("Input Is %24.24s\n", ctime(&what));
  
  printf("now  = %d\n", now);
  printf("what = %d\n", what);
  
  printf("Difference = %d\n\n", now - what);

}
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Convert ASCII time to long format.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Added to mfc.
 *  07/14/96   |  tjt  Fix day is calculated one too many.
 *  07/14/96   |  tjt  Added adjustment for timezone and altzone.
 *-------------------------------------------------------------------------*/
static char time_convert_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 * convert ascii time to seconds since Jan 1, 1970
 *
 * Input:  mm/dd/yyyy hh:mm:ss
 *-------------------------------------------------------------------------*/
#include <time.h>

#ifndef INTEL
#define altzone (timezone - 3600)
#endif

#define altzone (timezone - 3600)
time_convert(string, seconds)
char *string;
long *seconds;
{
  long  hours,minutes;
  short years,iyear,imonth,days,iday,ihour,iminute,isecond,leap = 0;
  char month[3],day[3],year[5],hour[3],minute[3],second[3];
  struct tm *x;
  long y;
  
  y = time(0);
  
  x = (struct tm *)localtime(&y);
  
  strncpy(month,  string,    2);
  strncpy(day,    string+3,  2);
  strncpy(year,   string+6,  4);
  strncpy(hour,   string+11, 2);
  strncpy(minute, string+14, 2);
  strncpy(second, string+17, 2);

  month[2]  = 0;
  day[2]    = 0;
  year[4]   = 0;
  hour[2]   = 0;
  minute[2] = 0;
  second[2] = 0;

  imonth  = atoi(month);
  iday    = atoi(day);
  iyear   = atoi(year);
  ihour   = atoi(hour);
  iminute = atoi(minute);
  isecond = atoi(second);

  if(iyear % 4 == 0 && iyear % 100 != 0 || iyear % 400 == 0) leap = 1;

  if(iyear < 1970 || imonth < 1 || imonth > 12 || iday < 0 || ihour < 0 ||
  ihour > 23   || iminute < 0 || iminute > 60 || isecond < 0
  || isecond > 60) return(0);
    
  switch(imonth)
  {
    case (1):

      if(iday > 31) return(0);
      days = 0;
      break;

    case (2):

      if((!leap && iday > 28) || (leap && iday > 29)) return(0);
      days = 31;
      break;

    case (3):

      if(iday > 31) return(0);
      days = 59;
      break;

    case (4):

      if(iday > 30) return(0);
      days = 90;
      break;

    case (5):

      if(iday > 31) return(0);
      days = 120;
      break;

    case (6):

      if(iday > 31) return(0);
      days = 151;
      break;

    case (7):

      if(iday > 30) return(0);
      days = 181;
      break;

    case (8):

      if(iday > 31) return(0);
      days = 212;
      break;

    case (9):

      if(iday > 30) return(0);
      days = 243;
      break;

    case (10):

      if(iday > 31) return(0);
      days = 273;
      break;

    case (11):

      if(iday > 30) return(0);
      days = 304;
      break;

    case (12):

      if(iday > 31) return(0);
      days = 334;
    }

  if (leap && imonth > 2 ) days++;
  years = iyear - 1970;

/*  days is calculated by adding the days passed in the current year with
 *  the number of leap years passed and the number of the days in the 
 *  proceeding years
 */
  days = days + (years * 365) + ((years + 1)/4) + iday - 1; /* F071496 */

  hours = days * 24 + ihour;
  minutes = hours * 60 + iminute;
  *seconds = minutes * 60 + isecond;
  
  if (x->tm_isdst) *seconds += altzone;    /* F071496 */
  else *seconds += timezone;
    
  if(*seconds > y) return (0);
  return (1);
}

/* end of time_convert.c */

