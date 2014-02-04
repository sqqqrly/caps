/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Process Print (y/n/e/f/b)? response.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/20/93    |  Added to mfc.
 *-------------------------------------------------------------------------*/
static char sd_print_c[] = "%Z% %M% %I% (%G% - %U%)";
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
 *   sd_print returns a value as described above. the possible
 *   return codes are 0,1,2,3,4 and 6 (meaningless). the code
 *   for sd_print is based on the following logic table:
 *
 *
 *     terminating key          y   n   e   f   b
 *      -----------------------------------------
 *      enter                   4   3   0   1   2
 *      exit                    4   0   0   0   0
 *      forward or down_cursor  1   1   1   1   1
 *      backward or up_cursor   2   2   2   2   2
 *      -----------------------------------------
 */ 
#include "iodefs.h"

sd_print(t, c)
register unsigned char t;
register unsigned char c;
{
  if(c >= 'A' && c <= 'Z') c += 0x20;       /* lower case                   */

  switch(t)
  {
    case FORWRD:
    case DOWN_CURSOR: return(1);
    case BACKWD:
    case UP_CURSOR:   return(2);

    case EXIT:        if(c == 'y') return(4);
                      else return(0);

    case ENTER:       switch(c)
                      {
                        case 'e': return(0);
                        case 'f': return(1);
                        case 'b': return(2);
                        case 'n': return(3);
                        case 'y': return(4);
                        default: return(6);
                      }
  }
}

/* end of sd_print.c */
