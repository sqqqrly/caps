/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Configuration definitons for co_open.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   |  tjt  Add to mfc.
 *  04/04/94   |  tjt  Added ac and tc views.
 *  04/13/94   |  tjt  Modified for UNIX.
 *-------------------------------------------------------------------------*/
#ifndef CO_OPEN_H
#define CO_OPEN_H

static char co_open_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "co.h"

/*-------------------------------------------------------------------------*
 *  Definition of Configuration Segment Global Variables
 *-------------------------------------------------------------------------*/

long co_fd   = 0;                         /* configuration file fd           */
long co_key  = 102;                       /* configuration file key          */
long co_id   = -1;                        /* configuration file id           */
long co_size = 0;                         /* configuration file size         */

unsigned char     *co    = 0;             /* pointer to configuration        */
struct co_header  *coh   = 0;             /* pointer to header               */
struct port_item  *po    = 0;             /* pointer to port table           */
struct pl_item    *pl    = 0;             /* pointer to pickline table       */
struct seg_item   *sg    = 0;             /* pointer to segment table        */
struct zone_item  *zone  = 0;             /* pointer to zone table           */
struct bay_item   *bay   = 0;             /* pointer to bay table            */
struct hw_item    *hw    = 0;             /* pointer to hardware table       */
struct pw_item    *pw    = 0;             /* pointer to pick table           */
struct mh_item    *mh    = 0;             /* pointer to module/hw table      */
struct st_item    *st    = 0;             /* pointer to sku table            */

struct hw_bl_view_item *blv = 0;          /* pointer to bay lamp view        */
struct hw_zc_view_item *zcv = 0;          /* pointer to zone controller view */
struct hw_pm_view_item *pmv = 0;          /* pointer to pick module view     */

#endif

/* end of co.h */

