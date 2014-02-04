/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    String compare without case.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/03/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char inanycase_c[] = "%Z% %M% %I% (%G% - %U%)";

long inanycase(p, q)
register char *p, *q;
{
  while (*p && *q)
  {
     if (toupper(*p) > toupper(*q)) return 1;
     if (toupper(*p) < toupper(*q)) return -1;
     p++;
     q++;
  }
  if (*p) return 1;
  if (*q) return -1;
  return 0;
}

/* end of inanycase.c */ 
