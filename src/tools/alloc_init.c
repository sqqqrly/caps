/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Clear allocation Field
 *
 *  Execution:      alloc_init  [-bal=nnnn]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  12/18/96   |  tjt  Original implementation.
 *  01/18/97   |  tjt  Add balance reset.
 *-------------------------------------------------------------------------*/
static char alloc_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "Bard.h"
#include "bard/pmfile.h"
#include "oracle_defines.h"

pmfile_item pm;

long balance = -1;

main(argc, argv)
long argc;
char **argv;
{
  long count = 0;
  
  putenv("_=alloc_init");
  chdir(getenv("HOME"));
  
  if (argc > 1)
  {
    if (memcmp(argv[1], "-bal=", 5) == 0)
    {
      balance = atol(&argv[1][5]);
    }
  }
  printf("Clear allocation field in pmfile\n");
  if (balance >= 0) printf("Set balance to %d\n\n", balance);
  
  database_open();
  pmfile_open(AUTOLOCK);
  pmfile_setkey(1);
  
  begin_work();
  
  while (!pmfile_next(&pm, LOCK))
  {
    pm.p_alloc = 0;
    if (balance >= 0) 
    {
       if (pm.p_pmsku[0] > 0x20) pm.p_qty = balance;
       else                      pm.p_qty = 0;
    }
    count++;
    pmfile_update(&pm);
    commit_work();
    begin_work();
  }
  commit_work();
  
  pmfile_close();
  database_close();
  
  printf("%d modules updated.\n", count);
  printf("All Done\n\n");
}

/* end of alloc_init.c */
