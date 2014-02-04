/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Post error message with numeric parm.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/7/93     |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char eh_postn_c[] = "%Z% %M% %I% (%G% - %U%)";

long eh_postn(err, number)
long err;
long number;
{
  char text[64];

  sprintf(text, "%d", number);
  return eh_post(err, text);              /* now call error post             */
}

/* end of eh_postn.h */
