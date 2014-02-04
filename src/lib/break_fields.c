/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/16/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char break_fields_c[] = "%Z% %M% %I% (%G% - %U%)";

/*  break_fields.c
 *
 *  Divides a null terminated string into fields
 *
 *  Returns:
 *
 *    >0 = Number of fields found
 *     0 = Remarks only
 *    <0 = Error in Field
 */
long break_fields(p, array, max)
char *p;                                  /* pointer to string               */
char **array;                             /* arrary of pointer to fields     */
long max;                                 /* maximumu fields to find         */
{
  extern char *whitespace();

  long j, k;                              /* field counter and return        */

  k = 0;                                  /* clear field count               */

  while(*p && k < max)                    /* while any string                */
  {
    p = whitespace(p);                    /* wipeout whitespace              */
    if (databyte(*p) || *p == ':')        /* if any field                    */
    {
      array[k++] = p;                     /* save pointer to first byte      */
      while(databyte(*p)) p++;            /* scan over data bytes            */
      p = whitespace(p);                  /* wipeout whitespace              */
      if (*p == ':')                      /* wipeout terminator              */
      {
        *p++ = 0;                         /* wipeout colon                   */
        continue;
      }
    }
    if (!*p) break;                       /* end of line                     */
    return -1;
  }

  for (j=k; j<max; j++) array[j] = p;     /* point rest to null              */

  return k;
}

/*
 *  Test Whitespace (space, tab, or remarks)
 */
char *whitespace(q)
char *q;
{
  char *r;

  while (*q)                              /* while any string                */
  {
    while (*q == 0x20 || *q == '\t')
    {
      *q++ = 0;                           /* wipeout space or tab            */
      continue;
    }
    if (*q == '/' && *(q+1) == '*')       /* remarks                         */
    {
      r = q;                              /* save remarks start              */
      q += 2;
      while (*q)
      {
        if (*q == '*' && *(q+1) == '/')   /* end of remarks                  */
        {
          q += 2;
          while (r < q) *r++ = 0;         /* wipeout remarks                 */
          r--;
          break;
        }
        q++;
        continue;
      }
      if (*r) return r;
      continue;
    }
    return q;
  }
  return q;
}
/*
 *  Test Valid Data Byte
 */
databyte(q)
char q;
{
  if (q >= 'a' && q <= 'z') return 1;
  if (q >= 'A' && q <= 'Z') return 1;
  if (q >= '0' && q <= '9') return 1;
  if (q == '-') return 1;
  if (q == '_') return 1;
  return 0;
}
   
/* end of breakfields.c */
