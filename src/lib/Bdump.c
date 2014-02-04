/*-------------------------------------------------------------------------*
 *  Copyright (c) 1989 - 1993 PTW Systems, Inc. - All rights reserved.
 *
 *  THIS IS UNPUBLISHED SOURCE CODE OF PTW SYSTEMS, INC.
 *  This copyright notice, above, does not evidence any actual
 *  or intended publication of such source code.
 *-------------------------------------------------------------------------*/
/*
 *  Bdump.c
 *
 *  Alphahex Dump of Area
 */
#include <stdio.h>

extern long Bdumpf();                    /* sco complains without this       */

long Bdump(p, n) 
register unsigned char *p;
register long n;
{
  return Bdumpf(p, n, stderr);
}
long Bdumpf(p, n, fd)
register unsigned char *p;
register long n;
register FILE *fd;
{
  register long k, m;
   
  m = 0;
   
  while (n > 0)
  {
    fprintf(fd, "%5d: ", m);
    for (k = 0; k < 16; k++)
    {
      if (k < n) fprintf(fd, "%02x ", p[k]);
      else fprintf(fd, "   ");
    }
    fprintf(fd, "   ");
    for (k = 0; k < 16; k++)
    {
      if (k < n)
      {
        if (*p > 0x20 && *p < 0x7f) fprintf(fd, "%c", *p);
        else fprintf(fd, ".");
        p++;
      }
      else break;
    }
    n -= 16;
    m += 16;
    fprintf(fd, "\n");
  }
  return 0;
}

/* end of Bdump.c */
