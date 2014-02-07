/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Report status of CAPS system.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/08/01   |  aha  Created program.
 *-------------------------------------------------------------------------*/
static char check_caps_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ss.h"

#define KERNELNAME  "bin/kernel"

main()
{
  FILE *td;
  char kernel_check[12],
       command[80],
       tmpname[16];

  memset(kernel_check, 0x0, 12);
  memset(command, 0x0, 80);
  memset(tmpname, 0x0, 16);

  putenv("_=check_caps");                  /* name to environ   */
  chdir(getenv("HOME"));                  /* to home directory  */

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
       printf("System not started\n");
       exit(4);
     }

  ss_open();

  if (sp->sp_running_status == 'n')
     {
       printf("Markplace\n");
       close_all();
       exit(3);
     }
  else
     {
       if (sp->sp_in_process_status != 'x')
          {
            printf("System is Busy\n");
            close_all();
            exit(1);
          }
       else
          {
            if (sp->sp_config_status == 'y')
               {
                 printf("System is running\n");
                 close_all();
                 exit(2);
               }
          }
     }

  close_all();

  exit(0);
} /* end of main() */


/*---------------------------------------------------------------------------
 * Closed all opened shared memory segments
 *---------------------------------------------------------------------------*/
close_all()
{
  ss_close();
  return;
} /* end of close_all() */

/* end of check_caps.c */
