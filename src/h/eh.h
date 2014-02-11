/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Definition of error files.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/7/93     |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
#ifndef EH_H
#define EH_H

static char eh_h[] = "%Z% %M% %I% (%G% - %U%)";

#define EH_MAX  256

/*-------------------------------------------------------------------------*
 *  Definition of eh_mess file
 *-------------------------------------------------------------------------*/
 
typedef struct
{
  char eh_nos[3];                         /* error number                    */
  char eh_log;                            /* x = write to log file           */
  char eh_broadcast;                      /* x = broadcast to all ttys       */
  char eh_text[40];                       /* error message text              */
  char eh_lf;                             /* line feed                       */

} eh_item;

#endif

/* end of eh.h */
