/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Report status of Picker Logins in CAPS.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  12/07/01   |  aha  Created program.
 *-------------------------------------------------------------------------*/
static char check_logins_c[] = "%Z% %M% %I% (%G% - %U%)";
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
       printf("System not started\n");
       exit(4);
     }

  ss_open();
  co_open();

  if (sp->sp_config_status != 'y')
     {
       printf("System is not configured\n");
       close_all();
       exit(4);
     }
  else
     {
       if (sp->sp_in_process_status != 'x')
          {
            printf("System is Busy\n");
            close_all();
            exit(-1);
          }

       results = 0;

       for (k = 0; k < coh->co_zone_cnt; k++)
           {
             z = &zone[k];

             if (z->zt_picker)
                {
                  results = 1;
                  break;
                }
           } /* end of for loop to check each configured zone */

       if (results)
          {
            printf("Picker ID %ld is logged in at zone %d\n",
                   z->zt_picker, z->zt_zone);
            close_all();
            exit(1);
          }
       else
          {
            printf("No logins present\n");
            close_all();
            exit(0);
          }
     }
} /* end of main() */

/*---------------------------------------------------------------------------
 * Closed all opened shared memory segments
 *---------------------------------------------------------------------------*/
close_all()
{
  ss_close();
  co_close();
  return;
} /* end of close_all() */


/* end of check_logins.c */
