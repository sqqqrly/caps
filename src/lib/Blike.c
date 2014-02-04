/*-------------------------------------------------------------------------*
 *  Copyright (c) 1989 - 1993 PTW Systems, Inc. - All rights reserved.
 *
 *  THIS IS UNPUBLISHED SOURCE CODE OF PTW SYSTEMS, INC.
 *  This copyright notice, above, does not evidence any actual
 *  or intended publication of such source code.
 *-------------------------------------------------------------------------*/
/*
 *  Blike
 *
 *  Pattern Matching
 *
 *    p is a null terminated pattern using ? and/or *
 *    q is data null terminated value. 
 *
 */
long Blike(p, q)
register unsigned char *p, *q;
{
  register long flag;

  if (*p && *q)
  {
    if (*p == '*')
    {
      p++;
      if (!*p) return 1;

      while (*q)
      {
        if (Blike(p, q)) return 1;
        q++;
      }
      return 0;
    }
    else if (*p == '[')
    {
      p++;
      flag = 1;
      if (*p == '!') {flag = 0; p++;}
      while (*p)
      {
        if (*p == ']') {flag ^= 1; break;}
        if (*(p+1) == '-')
        {
          if (*q >= *p && *q <= *(p+2)) break;
          p += 3;
          continue;
        }
        else if (*p == *q) break;
        p++;
      }
      if (!flag) return 0;
      while (*p && *p != ']') p++;
      return Blike(p+1, q+1);
    }
    else if (*p == '?') return Blike(p+1, q+1);
    else if (*p == *q)  return Blike(p+1, q+1);
    return 0;
  }
  if (*p == '*')  return 1;
  if (!*p && !*q) return 1;
  return 0;
}

/* end of Blike.c */
