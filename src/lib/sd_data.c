/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Common screen driver data.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/10/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char sd_data_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "iodefs.h"

long sd_server    = 0;                    /* task id of tty_server           */
long sd_row       = 0;                    /* current row position            */
long sd_col       = 0;                    /* current col position            */
long sd_width     = 80;                   /* screen width - 80 or 132        */
long sd_echo_flag = FILL;                 /* echo input bytes                */
long (*sd_func)() = 0;                    /* message processing function     */

unsigned char sd_buf[81] = {0};           /* input message                   */

/* end of sd_data.c */
