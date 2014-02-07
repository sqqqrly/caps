/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Dump specific data from HW table in CAPS shared memory
 *                  into a text file and load it into the database table
 *                  hw_dump for analysis.  CAPS must be initialized and
 *                  configured to use hw_dump.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/01/03   |  aha  Created program.
 *-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "caps_copyright.h"
#include "ss.h"
#include "co.h"

#define KERNELNAME  "bin/kernel"
#define DUMPFILE    "sql/hw_dump.dat"

static char hw_dump_c[] = "%Z% %M% %I% (%G% - %U%)";
FILE *hwf;

main()
{
  FILE *td;
  short j    = 0,
        hwix = 0;
  char kernel_check[12],
       command[80],
       h_type[4],
       tmpname[16];

  memset(kernel_check, 0x0, 12);
  memset(command, 0x0, 80);
  memset(tmpname, 0x0, 16);

  putenv("_=hw_dump");                  /* name to environ   */
  chdir(getenv("HOME"));                /* to home directory  */

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
       printf("System not started.\n");
       exit(4);
     }

  ss_open();
  co_open();

  if (sp->sp_config_status != 'y')
     {
       printf("System is running.\n");
       close_all();
       exit(2);
     }

  hwf = fopen(DUMPFILE, "w");

  if (hwf == 0)
     {
       printf("Cannot open sql/hw_dump.dat for writing.\n");
       close_all();
       exit(3);
     }

  for (j = 0; j < coh->co_light_cnt; j++)
      {
         memset(h_type, 0x0, 4);

         switch (hw[j].hw_type)
                {
                   case BL:   strcpy(h_type, "BL");
                              break;

                   case ZC:   strcpy(h_type, "ZC");
                              break;

                   case PM:   strcpy(h_type, "PM");
                              break;

                   case PI:   strcpy(h_type, "PI");
                              break;

                   case ZC2:  strcpy(h_type, "ZC2");
                              break;

                   case PM2:  strcpy(h_type, "PM2");
                              break;

                   case PM4:  strcpy(h_type, "PM4");
                              break;

                   case PM6:  strcpy(h_type, "PM6");
                              break;

                   case BF:   strcpy(h_type, "BF");
                              break;

                   case IO:   strcpy(h_type, "IO");
                              break;

                   default:   break;
                }  /* end of switch statement for hardware type */

         hwix = j + 1;

         fprintf(hwf, "%d|%d|%d|%d|%d|%d|%s|\n",
                 j,
                 hwix,
                 hw[j].hw_mod,
                 hw[j].hw_mod_address,
                 hw[j].hw_bay,
                 hw[j].hw_controller,
                 h_type);
         fflush(hwf);

      }  /* end of for loop to read hw table */

  fclose(hwf);

  memset(command, 0x0, 80);

  sprintf(command, "%s%s",
          "/u/mfc/script/ld_hw_dump_file ",
          "1>/u/mfc/dat/log/ld_hdf.log 2>&1");

  system(command);

  sleep(5);

  unlink(DUMPFILE);

  close_all();

  exit(0);
} /* end of main() */


/*---------------------------------------------------------------------------
 * Closed all opened shared memory segments
 *---------------------------------------------------------------------------*/
close_all()
{
  ss_close();
  co_close();

  if (hwf)
     {
       fclose(hwf);
     }

  return;
} /* end of close_all() */

/* end of hw_dump.c */
