/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Productivity shared segment pr_open definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Added to mfc.
 *  04/13/94   |  tjt  Modifies for UNIX.
 *-------------------------------------------------------------------------*/
#ifndef PR_OPEN_H
#define PR_OPEN_H

static char pr_open_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "pr.h"

long pr_fd   = 0;                         /* productivity file fd            */
long pr_key  = 104;                       /* productivity file key           */
long pr_id   = -1;                        /* productivity file id            */
long pr_size = 0;                         /* productivity file size          */

struct pr_record    *pr = 0;
struct pr_pl_item   *pp = 0;
struct pr_zone_item *pz = 0;

#endif

/* end of pr_open.h */

