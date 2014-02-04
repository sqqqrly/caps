/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Closes a area controller port.
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  2/10/94    |  tjt  Originial implementation.
 *-------------------------------------------------------------------------*/
static char ac_close_c[] = "%Z% %M% %I% (%G% - %U%)";

ac_close(port)
register long port;
{
  if (port > 0)  close(port);
  return 0;
}

/* end of ac_close.c */
