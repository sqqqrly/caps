/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Order file structures for od_open.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  8/16/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
#ifndef OD_OPEN_H
#define OD_OPEN_H

static char od_open_h[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

/*-------------------------------------------------------------------------*
 *  Order File Databases
 *-------------------------------------------------------------------------*/

struct of_header    *of_rec = 0;          /* order header database record    */
struct of_pick_item *op_rec = 0;          /* picks database record           */
struct of_rmks_item *or_rec = 0;          /* remarks database record         */

struct of_record    *of = 0;              /* pointer to order record         */

#endif

/* end of od_open.h  */
