/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Record structure for boxes database table 
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/08/2001  | aha  Created file.
 *-------------------------------------------------------------------------*/
#ifndef __BOXESH__
#define __BOXESH__
static char boxes_h[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"


typedef struct
{
   short          b_box_pl;
   long           b_box_on;
   long           b_box_number;
   char           b_box_status[2];
   char           b_box_last[2];
   short          b_box_lines;
   short          b_box_units;

} boxes_item;

#define boxes_size 18

#endif
/* end of boxes.h */
