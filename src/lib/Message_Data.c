/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Message data values.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/14/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char Message_Data_c[] = "%Z% %M% %I% (%G% - %U%)";

long msgin        = 0;                    /* input queue to kernel           */
long msgout       = 0;                    /* output queue from kernel        */
long msgtask      = 0;                    /* id of this task 1 .. 32         */
long msgtype      = 0;                    /* message wait type               */
long msgsig       = 0;                    /* message signal                  */
long (*msgfunc)() = 0;                    /* message processor               */

/* end of Message_Data.c */
