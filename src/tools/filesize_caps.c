/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Prints sizes of shared segments.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/1/93    |  tjt  Added to mfc
 *-------------------------------------------------------------------------*/
static char filesize_caps_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "ss.h"
#include "co.h"
#include "of.h"
#include "pr.h"

main()
{
  register total;

  putenv("_filesize_caps");
  chdir(getenv("HOME"));
  
  ss_open();
  co_open();
  oc_open();
  pr_open();

  total = ss_size + co_size + oc_size + pr_size;
  
  printf("\n\n");
  printf("    CAPS Shared Segment File Sizes\n");
  printf("    ------------------------------\n");
  printf("    ss segment is %7d bytes\n", ss_size);
  printf("    co segment is %7d bytes\n", co_size);
  printf("    oc segment is %7d bytes\n", oc_size);
  printf("    pr segment is %7d bytes\n", pr_size);
  printf("                  -------\n");
  printf("    total size is %7d bytes\n\n", total); 
   

  pr_close();
  co_close();
  oc_close();
  ss_close();


  return;
}

/* end of filesize_caps.c */
