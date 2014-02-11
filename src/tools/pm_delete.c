/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Remove/Insert a part of the pmfile file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/10/94   |  tjt  Original impletmentation
 *-------------------------------------------------------------------------*/
static char pm_delete_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

#include "Bard.h"
#include "bard/pmfile.h"
#include "oracle_defines.h"

long db;
pmfile_item pm;

char mode[8];

main()
{
  putenv("_=pm_delete");
  chdir(getenv("HOME"));
  
  pmfile_open(AUTOLOCK);
  pmfile_setkey(0);
  
  while (1)
  {
    printf("Enter (D)elete or (I)nsert\n");
    gets(mode);
    *mode = tolower(*mode);
    if (*mode == 'i' || *mode == 'd') break;
  }
  if (*mode == 'i') insert_pm();
  else delete_pm();
  
  pmfile_close();
  exit(0);
}
/*-------------------------------------------------------------------------*
 * Insert Modules
 *-------------------------------------------------------------------------*/
insert_pm()
{
  char first[8], last[8], ans[8];
  long low, high, shift;

  printf("Enter First Module to Insert --> "); gets(first);
  printf("Enter Last  Module to Insert --> "); gets(last);

  low  = atoi(first);
  high = atoi(last);
  
  printf("First = %d\n", low);
  printf("Last  = %d\n", high);
  printf("Proceed? --> ");
  gets(ans);
  
  if (*ans != 'y' && *ans != 'Y') return 0;
  
  shift = high - low + 1;

  begin_work();
  while (!pmfile_next(&pm, LOCK))
  {
    if (pm.p_pmodno < low) continue;

    pm.p_pmodno += shift;
    pmfile_update(&pm);
    commit_work();
    begin_work();
  }
  commit_work();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Delete Modules
 *-------------------------------------------------------------------------*/
delete_pm()
{
  char first[8], last[8], ans[8];
  long low, high, shift;

  printf("Enter First Module to Delete --> "); gets(first);
  printf("Enter Last  Module to Delete --> "); gets(last); 
  
  low  = atoi(first);
  high = atoi(last);
  
  printf("First = %d\n", low);
  printf("Last  = %d\n", high);
  printf("Proceed? --> ");
  gets(ans);
  
  if (*ans != 'y' && *ans != 'Y') return 0;
  
  shift = high - low + 1;

  while (!pmfile_next(&pm, LOCK))
  {
    if (pm.p_pmodno <  low) continue;
    if (pm.p_pmodno <= high)
    {
      pmfile_delete();
      continue;
    }
    pm.p_pmodno -= shift;
    pmfile_update(&pm);
  }
}


/* end of pm_delete.c */
