/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Definition of structures for ss_open.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Added to mfc.
 *  04/12/94   |  tjt  Modified for UNIX. 
 *-------------------------------------------------------------------------*/
#ifndef SS_OPEN_H
#define SS_OPEN_H

static char ss_open_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "ss.h"

/*-------------------------------------------------------------------------*
 *  System Segment Pointers (Setup by ss_open)
 *-------------------------------------------------------------------------*/

struct ss_item    *ssi  = 0;              /* points to entire ss             */
struct sp_item    *sp   = 0;              /* points to system parameters     */
struct rf_item    *rf   = 0;              /* points to record format         */

/*-------------------------------------------------------------------------*
 *   Definition of System Segment Global Variables
 *-------------------------------------------------------------------------*/

long ss_key    = 101;                     /* system segment key              */
long ss_fd     = 0;                       /* system segment file fd          */
long ss_id     = -1;                      /* system segment id               */
long ss_size   = 0;                       /* system segment length           */

#endif

/* end of ss_open.h                                                          */
