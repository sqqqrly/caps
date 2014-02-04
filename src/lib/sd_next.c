/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Calculate row modifier to next row.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/20/93    |  tjt Added to mfc.
 *-------------------------------------------------------------------------*/
static char sd_next_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "sd.h"

sd_next(f)
register struct fld_parms *f;
{
  register long x;

  return (sd_row + 2 - f->irow);         /* row modifier to next row        */
}

/* end of sd_next.c */
