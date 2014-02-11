/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Make a dummy firmware for download.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  3/7/94     |  tjt   Original implementation.
 *-------------------------------------------------------------------------*/
static char make_ac_firmware_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

#include "file_names.h"

/*-------------------------------------------------------------------------*
 *  Firmware Load:   (eeprom addresses 8000-9fff)
 *  
 *  Address Length Contents
 *  ------- ------ --------
 *   8000       1    0x00
 *   8001    4080    0x00 
 *   9FF1       8    07/01/33             eeprom date                    
 *   9FF9       4    xxxx                  address of controller          
 *   9FFD       1    0x00                  checksum                       
 *   9FFE       2    9FF0                  highest used address           
 *-------------------------------------------------------------------------*/
 
FILE *fd;
extern unsigned char *hex();

main()
{
  register long k;

  putenv("_=make_ac_firmware");
  chdir(getenv("HOME"));
   
/*-------------------------------------------------------------------------*
 *  Make  A Firmware Date File
 *-------------------------------------------------------------------------*/

  fd = fopen(alc_firmdate_name, "w");
  if (fd == 0)
  {
    krash("main", "Can't Open Firmware Date File", 1);
  }
  fprintf(fd, "07/01/33\n");
  fclose(fd);

/*-------------------------------------------------------------------------*
 * Make A Firmware File
 *-------------------------------------------------------------------------*/
  fd = fopen(alc_firmware_name, "w");
  if (fd == 0)
  {
    krash("main", "Can't Open Firmware File", 1);
  }
  for (k = 0x8000; k < 0x9ff0; k += 8)
  {
    fprintf(fd, "008%4.4s0000000000000000\n", hex(k));
  }
  fprintf(fd, "0099FF00030372F30312F3333\n");
  fprintf(fd, "0039FFD00F09F\n");
   
  fclose(fd);
  
  printf("All Done\n\n");

  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Hex Address In Upper Case
 *-------------------------------------------------------------------------*/
unsigned char *hex(k)
register long k;
{
  static unsigned char work[8];
  
  sprintf(work, "%04x", k);

  work[0] = toupper(work[0]);
  work[1] = toupper(work[1]);
  work[2] = toupper(work[2]);
  work[3] = toupper(work[3]);

  return work;
}

/* end of make_ac_firmware.c */
