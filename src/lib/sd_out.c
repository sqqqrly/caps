/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Output a data field.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/20/93    |  tjt Added to mfc.
 *-------------------------------------------------------------------------*/
static char sd_out_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"

sd_out(f, r, sr, sc, buf)
register struct fld_parms *f;
register long r, sr, sc;
register unsigned char *buf;
{
  sd_cursor(0, f->irow + r, f->icol);     /* position cursor                 */
  sd_text(buf);                           /* output data field               */
  sd_cursor(0, sr, sc);                   /* restore  cursor                 */
  return 0;
}

/* end of sd_out.c */
