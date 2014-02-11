/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Productivity shared segment structure definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/16/93   | tjt  Added to mfc.
 *  07/23/93   | tjt  Revised for global_types and dat directory.
 *  04/13/94   | tjt  Modifed for UNIX.
 *-------------------------------------------------------------------------*/
#ifndef PR_H
#define PR_H

static char pr_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Productivity Header
 *-------------------------------------------------------------------------*/
 
struct pr_record
{
  long pr_picklines;                      /* number of picklines             */
  long pr_zones;                          /* number of zones                 */
};

/*-------------------------------------------------------------------------*
 *  Definition of Productivity Segment  (28 Bytes per Pickline)
 *-------------------------------------------------------------------------*/

struct pr_pl_item
{
  long pr_pl_cur_completed;               /* current completed orders        */
  long pr_pl_cum_completed;               /* cummulative completed orders    */
  long pr_pl_cum_start;                   /* cummulative start date/time     */
  long pr_pl_cur_start;                   /* current start date/time         */
  long pr_pl_cum_elapsed;                 /* cummulative elaped time         */
  long pr_pl_cur_elapsed;                 /* current elapsed time            */
  long pr_pl_current;                     /* recent start date/time          */

};
 
/*-------------------------------------------------------------------------*
 *  Zone Productivity   (56 bytes per Zone)
 *-------------------------------------------------------------------------*/

struct pr_zone_item
{
  long pr_zone_cum_orders;                /* cummulative order count         */
  long pr_zone_cum_lines;                 /* cummulative line count          */
  long pr_zone_cum_units;                 /* cummulative piece count         */
  long pr_zone_cum_ah_cnt;                /* cummulative ahead count         */
  long pr_zone_cum_ahead;                 /* cummulative ahead time          */
  long pr_zone_cum_active;                /* cummulative idle time           */

  long pr_zone_cur_orders;                /* current order count             */
  long pr_zone_cur_lines;                 /* current line count              */
  long pr_zone_cur_units;                 /* current piece count             */
  long pr_zone_cur_ah_cnt;                /* current ahead count             */
  long pr_zone_cur_ahead;                 /* current ahead time              */
  long pr_zone_cur_active;                /* current idle time               */

};

extern long pr_fd;                        /* productivity file fd            */
extern long pr_id;                        /* productivity file id            */
extern long pr_size;                      /* productivity file size          */

struct pr_record    *pr;                  /* pointer to header part          */
struct pr_pl_item   *pp;                  /* pointer to pickline             */
struct pr_zone_item *pz;                  /* pointer to zone                 */

#endif

/* end of pr.h */
