/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Closes a basic function port.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/24/91    |  tjt  Originial implementation.
 *  7/30/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char tc_close_c[] = "%Z% %M% %I% (%G% - %U%)";

tc_close(port)
register long port;
{
  if (port > 0) close(port);
  return 0;
}

/* end of tc_close.c */
