/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Microtimer In-Line Macros
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  1/15/94    |  add to mfc.
 *-------------------------------------------------------------------------*/
static char microtimer_h[] = "%Z% %M% %I% (%G% - %U%)";

unsigned long microtimer[10][2];

char *break_timer(x)
register unsigned long x;
{
  register long a, b, c;
  static unsigned char work[16];

  a = x >> 20;
  b = ((x & 0xfffff) * 1000) >> 20; 
  c = ((x & 0x3ff) * 1000) >> 10;
  
  sprintf(work,"%d:%d.%03d", a, b, c);

  return work;
}
#define start_timer(x)  microtimer[x][0] = microclock()
#define stop_timer(x)   microtimer[x][1] = microclock()
#define show_start(x)   break_timer(microtimer[x][0])
#define show_stop(x)    break_timer(microtimer[x][1])
#define show_elapsed(x) break_timer(microtimer[x][1] - microtimer[x][0])

/* end of microtimer.c */
