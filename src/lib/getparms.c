/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Get environemntal paramters
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/11/93   |  tjt  Original implementation.
 *  05/19/94   |  tjt  Add default values.
 *-------------------------------------------------------------------------*/
static char getparms_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

extern char *getenv();

extern SUPER_OP;
extern DATA_OP;
extern op_pl;
extern char op_legal[32];
extern char op_printer[16];
extern op_refresh;

getparms(p)
char *p;
{
  register char *q;
   
  q = getenv("LEVEL");                    /* LEVEL=S is SUPER_OP             */
  if (q)
  {
    if (*q == 'S')      SUPER_OP = 1;
    else if (*q == 'D') DATA_OP  = 1;
  }
  else SUPER_OP = DATA_OP = 0;            /* default to pickline operator    */

  q = getenv("PICKLINE");                 /* get current pickline            */
  if (q) op_pl = atol(q);
  else op_pl = 1;                         /* default to pickline 1           */
  
  if (p)                                  /* get legal features              */
  {
    q = getenv(p);
    if (q) strncpy(op_legal, q, 31);
    else memset(op_legal, 0, 31);         /* default to nothing is legal     */
  }

  q = getenv("PRINTER");                  /* get operator printer name       */
  if (q) strncpy(op_printer, q, 8);
  else strcpy(op_printer, "default");     /* default to 'default'            */
  
  q = getenv("REFRESH");                  /* refresh interval                */
  if (q) op_refresh = atol(q);
  else op_refresh = 1;                    /* default to 1 second             */
  
  return 0;
}

/* end of getparms.c */
