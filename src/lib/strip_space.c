/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Replace rightmost spaces with nulls.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   7/17/93   |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char strip_space_c[] = "%Z% %M% %I% (%G% - %U%)";

strip_space(buf, length)
register char *buf;
register long length;
{
  register long n;

  for (n = length - 1; n >= 0; n--)
  {
    if (*(buf + n) > 0x20) break;
    *(buf + n) = 0;
  }
  return 0;
}

/* end of strip_space.c */
