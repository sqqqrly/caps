/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Retrieve current CAPS configuration filename in use.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  12/07/01   |  aha  Created program.
 *-------------------------------------------------------------------------*/
static char get_config_filename_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss.h"
#include "co.h"

#define KERNELNAME  "bin/kernel"

main()
{
  FILE *td;
  unsigned short int k       = 0,
                     results = 0;
  struct zone_item *z;
  char kernel_check[12],
       command[80],
       tmpname[16];

  memset(kernel_check, 0x0, 12);
  memset(command, 0x0, 80);
  memset(tmpname, 0x0, 16);

  putenv("_=check_caps");                  /* name to environ   */
  chdir(getenv("HOME"));                   /* to home directory  */

  tmp_name(tmpname);
  sprintf(command, "%s > %s",
          "ps -ef|grep -v grep|grep ' bin/kernel'|cut -c54-63",
          tmpname);
  system(command);

  td = fopen(tmpname, "r");
  if (td == 0)
     {
       exit(-1);
     }

  fgets(kernel_check, 11, td);

  if (td)
     {
       fclose(td);
       unlink(tmpname);
     }

  if (strcmp(kernel_check, KERNELNAME) != 0)
     {
       exit(-1);
     }

  co_open();

  printf("%s", coh->co_config_name);

  close_all();
  exit(0);
} /* end of main() */

/*---------------------------------------------------------------------------
 * Closed all opened shared memory segments
 *---------------------------------------------------------------------------*/
close_all()
{
  co_close();
  return;
} /* end of close_all() */


/* end of get_config_filename.c */
