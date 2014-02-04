/*-------------------------------------------------------------------------*
 *  Copyright (c) 1989 - 1993 PTW Systems, Inc. - All rights reserved.
 *
 *  THIS IS UNPUBLISHED SOURCE CODE OF PTW SYSTEMS, INC.
 *  This copyright notice, above, does not evidence any actual
 *  or intended publication of such source code.
 *-------------------------------------------------------------------------*/
/*
 *  Bdate.c
 *
 *  Collection of Date Subroutines.
 *
 *    1/1/1970 has a value of 0.  Stored as GMT seconds / 86400.
 *    dates until about 2038.
 *
 *  Convert mo/da/yr To Internal Date Form.
 *
 *    date is in the form mo/da/yr (any delimitor is allowed).
 *    packed is the internal unsigned short date from 1900. 
 *
 *    return    0;      valid date.
 *    return   -1;      invalid date.
 *    return   -2;      future date.
 *    return   -3;      null date.
 */
#include <time.h>

long Bbreak_date(date, packed)
register unsigned char *date;
register unsigned short *packed;
{
  extern long altzone, timezone;
   
  static long y = 0;

  static short julian[]   = {0,0,31,59,90,120,151,181,212,243,273,304,334,365};
  static short days[]     = {0,31,29,31,30,31,30,31,31,30,31,30,31};
   
  register long k, leaps, years;
  register unsigned char *p;
  register struct tm *x;
  long mo, da, yr;
  unsigned char work[20];
  short value;

  strcpy(work, "         -1 -1 -1 ");

  p = work;

  for (k = 0; k < 8; k++)                 /* remove non-numerics             */
  {
    if (!date[k]) break;
    if (date[k] < '0' || date[k] > '9') *p = 0x20;
    else *p = date[k];
    p++;
  }
  sscanf(work, "%2d%2d%2d", &mo, &da, &yr);

  if (mo < 0)                             /* is a null field                 */
  {
    value = 0;
    memcpy(packed, (unsigned char *)&value, 2);/* return zero value          */
    return -3;
  }
  if (da < 0 || yr < 0) return -1;        /* missing field                   */

  if (yr < 70) yr += 100;                 /* based at 1970                   */
  if (yr >= 138) return -1;               /* only through 12/31/2037         */
   
  leaps = yr / 4;                         /* number of leaps since 1900      */
  years = yr - 4 * leaps;                 /* years after leap year           */

  if (mo > 12 || mo < 1) return -1;       /* invalid month                   */
  if (da < 1) return -1;                  /* invalid day per se              */
  if (da > days[mo]) return -1;           /* invalid day for month           */
  if (mo == 2 && da == 29 && years) return -1;/* only in leap year           */
   
  value = 1461 * leaps + 365 * years + julian[mo] + da - 25569;
  if (years || (!years && mo > 2)) value += 1;

  Bexpand_date(value, date);              /* return good format              */

  memcpy(packed, (unsigned char *)&value, 2);/* return value                 */
   
  if (!y)
  {
    y = time(0);                          /* current time                    */
    x = (struct tm *)localtime(&y);       /* needed to set timezone          */
    if (x->tm_isdst) y -= (timezone + 3600);        /* local daylight time             */
    else y -= timezone;                   /* local standard time             */
    y = y / 86400;
  }
  if (value > y) return -2;               /* future date                     */
  return 0;
}

/*
 *  Convert Internal Date To mo/da/yr
 *
 *    pack is pointer to date in internal from.
 *    date is pointer to result string as mm/dd/yy.
 */
Bexpand_date(pack, date)
unsigned short pack;
unsigned char *date;
{
  char work[12];
  register struct tm *x;
  long value;
   
  value = 86400 * pack;                   /* to system time                  */

  if (!value)                             /* is null date                    */
  {
    memset(date, 0x20, 8);
    return 0;
  }
  x = (struct tm *)gmtime(&value);        /* break into parts                */

  if (x->tm_year > 99) x->tm_year -= 100;

  sprintf(work, "%02d/%02d/%02d", x->tm_mon + 1, x->tm_mday, x->tm_year);
  strncpy(date, work, 8);
  return 0;
}


/*
 *  Convert Internal Date To mo/da/yr
 *
 *    pack is pointer to date in internal from.
 *    date is pointer to result string as yymmdd
 */
long Bexpand_yymmdd(pack, date)
unsigned short pack;
unsigned char *date;
{
  char work[12];
  register struct tm *x;
  long value;
   
  value = 86400 * pack;                   /* to system time                  */

  if (!value)                             /* is null date                    */
  {
    memset(date, 0x20, 6);
    return 0;
  }
  x = gmtime(&value);                    /* break into parts                */

  if (x->tm_year > 99) x->tm_year -= 100;

  sprintf(work, "%02d%02d%02d", x->tm_year, x->tm_mon + 1, x->tm_mday);
  memcpy(date, work, 6);
  return 0;
}


/*
 *  Calculate Elapsed Days As (To - From)
 *
 *    from points to internal from date.
 *    to points to internal to date.
 *
 *    return (to - from).
 */
long Belapsed_days(from, to)
unsigned short from;
unsigned short to;
{
  return (to - from);
}

/*
 *  Get Current Date.
 *
 *    date points to current date as mm/dd/yy.
 *
 *    return (today in internal form);
 */
long Bget_date(date)
unsigned char *date;
{
  extern long timezone;
  unsigned char work[12];
  register struct tm *x;                  /* time structure                  */
  long y;                                 /* time value                      */
   
  y = time(0);                            /* get current clock time          */
  x = (long *)localtime(&y);              /* break into time structure       */
   
  if (date)
  {
    sprintf(work, "%02d/%02d/%02d", x->tm_mon + 1, x->tm_mday, x->tm_year);
    strncpy(date, work, 8);               /* copy date                       */
  }
  if (x->tm_isdst) y -= (timezone + 3600);
  else y -= timezone;
  
  return (y / 86400);                     /* internal date form              */
}

/* end of Bdate.c */
