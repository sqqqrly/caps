/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Space fill right part of null terminated field.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/17/93   |  tjt  Rewritten.
 *  07/10/96   |  tjt  Replaced strlen with loop in case no null.   
 *-------------------------------------------------------------------------*/
static char space_fill_c[] = "%Z% %M% %I% (%G% - %U%)";

space_fill(buf, length)
register char *buf;
register long length;
{
  register long i;
  register unsigned char *p;
  
  if (length <= 0) return 0;              /* invalid parm                    */

  for (i = 0, p = buf; i < length; i++, p++) /* find byte less than space    */
  {
    if (*p < 0x20) break;                 /* get length of field             */
  }
  if (i < length);                        /* check any fill is needed        */
  {
    memset(buf + i, 0x20, length - i);
  }
  return 0;
}

/* end of space_fill.c */
