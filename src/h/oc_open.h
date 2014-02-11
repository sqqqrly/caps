/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order file structures for oc_open.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
#ifndef OC_OPEN_H
#define OC_OPEN_H

static char oc_open_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "of.h"

/*-------------------------------------------------------------------------*
 *  Order Index File 
 *-------------------------------------------------------------------------*/

long oc_fd   = 0;                         /* order index file fd             */
long oc_key  = 103;                       /* order index file key            */
long oc_id   = -1;                        /* order index file id             */
long oc_size = 0;                         /* order index file size           */

struct oc_rec *oc = 0;                    /* Order Control Segment           */

#endif

/* end of oc_open.h  */
