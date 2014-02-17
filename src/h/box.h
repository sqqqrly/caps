/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Box structures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/6/93    |  tjt  Added to mfc.
 *  02/02/95   |  tjt  Removed unused transaction part.
 *  10/06/95   |  tjt  Revised for INFORMIX.
 *-------------------------------------------------------------------------*/
#ifndef BOX_H
#define BOX_H

static char box_h[] = "%Z% %M% %I% (%G% - %U%)";

#define  BOX_UNUSED  'U'                  /* box order database status       */
#define  BOX_OPEN    'O'
#define  BOX_CLOSED  'C'
#define  BOX_QUEUED  'Q'
#define  BOX_PRINTED 'P'

/*-------------------------------------------------------------------------*
 *  Queue Item - same as h/bard/packing_list.h, tote_label.h, ship_label.h
 *-------------------------------------------------------------------------*/

typedef struct
{
  long           paper_ref;
  long           paper_time;
  short          paper_copies;
  short          paper_pl;
  long           paper_order;
  short          paper_zone;
  
} paper_item;

/*-------------------------------------------------------------------------*
 *  Box Packing List Queue Item - same as h/bard/box_packing_list.h
 *-------------------------------------------------------------------------*/

typedef struct
{
  long           paper_ref;
  long           paper_time;
  short          paper_copies;
  short          paper_pl;
  long           paper_order;
  long           paper_box_number;
  char           paper_printer[8];

} box_paper_item;

#endif

/* end of box.h */
