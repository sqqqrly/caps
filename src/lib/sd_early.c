/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Process Print (y/n)? response.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/20/93    |  Added to mfc.
 *-------------------------------------------------------------------------*/
static char sd_early_c[] = "%Z% %M% %I% (%G% - %U%)";
/*-------------------------------------------------------------------------*
 *   sd_more, sd_early, and sd_print return a value on which a
 *   switch statement can be executed for the responses to
 *   More? (y/n), Print? (y/n), and Print? (y/n/e/f/b) querys.
 *   all return a code from 0 to 6,  with the following meanings:
 *
 *        0 = exit program;
 *        1 = next screen;
 *        2 = previous screen;
 *        3 = repeat screen (or exit, depending on program);
 *        4 = print file, and exit;
 *        5 = continue with program execution (only for sd_early);
 *        6 = meaningless;
 *
 *   sd_early returns a value as described above. the possible
 *   return codes are 0, 4, 5, or 6 (meaningless). the code for 
 *   sd_early is based on the following decision table:
 *
 *        terminating key         y   n
 *        -----------------------------
 *        enter                   4   5
 *        exit                    4   0
 *        forward or down_cursor  4   5
 *        backward or up_cursor   4   5
 *        -----------------------------
 */
#include "iodefs.h"

sd_early(t, c)
register unsigned char t;
register unsigned char c;
{
  if(c >= 'A' && c <= 'Z') c += 0x20;       /* lower case                   */

  switch(c)
  {
    case 'y': return(4);
    case 'n': switch(t)
              {
                case EXIT: return(0);
                default:  return(5);
              }

    default: if(t == EXIT) return(0);
             else          return(6);
  }
}

/* end of sd_early.c */
