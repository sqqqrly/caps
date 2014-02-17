/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Total Function Message Table
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/06/94   |  tjt  Original implementation.
 *  03/16/95   |  tjt  Remove cluster picking.
 *-------------------------------------------------------------------------*/
#ifndef ALC_TEXT_H
#define ALC_TEXT_H

static char alc_text_h[] = "%Z% %M% %I% (%G% - %U%)";

typedef struct
{
  unsigned char alc_ahead[24];              /* start ahead zone              */
  unsigned char alc_complete[24];           /* start completed zone          */
  unsigned char alc_locked[24];             /* start lcoked zone             */
  unsigned char alc_ee_no_picks[24];        /* start early exit no picks     */
  unsigned char alc_ee_picks[24];           /* start early exit with picks   */
  unsigned char alc_late_entry[24];         /* start late entry              */
  unsigned char alc_uw_no_picks[24];        /* start underway no picks       */
  unsigned char alc_uw_picks[24];           /* start underway with picks     */
  unsigned char alc_waiting[24];            /* start waiting automatic feed  */
  unsigned char alc_pass_tote[24];          /* zone finish pass tote         */
  unsigned char alc_exit_tote[24];          /* zone finish exit tote         */
  unsigned char alc_hold_tote[24];          /* zone finish hold tote         */
  unsigned char alc_login[24];              /* zone login message            */
  unsigned char alc_pi_pick[24];            /* zone pick message             */
  
} alc_data_item;

#endif

/* end of alc_text.h */
