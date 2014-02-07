/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear item movement times.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/5/93    |  tjt  Added to mfc
 *-------------------------------------------------------------------------*/
static char init_init_c[] = "%Z% %M% %I% (%G% - %U%)";

/*  imt_init.c
 *
 *  Clear Item Movement Date/Times
 */
#include <stdio.h>
#include "imt.h"

main()
{
   register long k;

   putenv("_=imt_init");
   chdir(getenv("HOME"));

   printf("Clear Item Movement Data/Times\n");

   for (k = 0; k < PicklineMax; k++)
   {
      imt[k].imt_cur = 0;
      imt[k].imt_cum = 0;
   }
   imt_unload();
   imt_load();				/* test reload			*/
   return;
}

/* end of imt_init.c */
