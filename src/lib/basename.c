/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Base part of a filename.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/12/93   |  tjt  original implementation.
 *-------------------------------------------------------------------------*/
static char basename_c[] = "%Z% %M% %I% (%G% - %U%)";

char *basename(p)
register char *p;
{
  static filename[64];
  register char *q;
  
  q = filename;
  
  while (*p)
  {
    if (*p == '/') q = filename;
    else if (*p == '.') break;
    else *q++ = *p;
    
    p++;
  }
  *q = 0;
  return filename;
}

/* end of basename.c */

