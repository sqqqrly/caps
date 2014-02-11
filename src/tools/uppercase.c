/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Change File to Uppercase.
 *
 *  Execution:      uppercase <in  >out
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/01/95   |  tjt  Original Implmentation.
 *-------------------------------------------------------------------------*/
static char uppercase_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>

main()
{
  unsigned char c;
  
  while ((c = getchar()) != 0xff) printf("%c", toupper(c));

}

/* end of uppercase.c */


