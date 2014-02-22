/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Copy diskette to sku batch file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/26/93   |  tjt  Added mfc.
 *  06/28/94   |  tjt  Revised for DOS diskettes.
 *-------------------------------------------------------------------------*/
static char sku_diskette_read_c[] = "%Z% %M% %I% (%G% - %U%)";

/*
 *   sku_diskette_read.c
 *
 *   Reads IBM Format Diskette and outputs
 *   SKU input to SKU batch file for EOD processing
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss.h"
#include "file_names.h"

FILE *in_fd, *out_fd;

char work[16];

#define READ_SIZE 128
#define WRITE_SIZE 92

long i,ret;
unsigned char byte;
char terminator = '\n';
unsigned char buf[READ_SIZE];
char command[80];

main()
{
  putenv("_=sku_diskette_read");
  chdir(getenv("HOME"));

  ss_open();
  ss_close();
  
  tmp_name(work);

  sprintf(command, "doscp A:SKU.DAT %s", work);
  ret = system(command);

  if (ret) exit(1);

  in_fd = fopen(work, "r");                /* open ibm diskette work file	  */ 
  if (in_fd == 0) exit(1);

  out_fd = fopen(sku_batch_name, "a+");   /* SKU batch pending file          */
  if (out_fd == 0)
  {
    fclose(in_fd);
    exit(2);
  }

/*-------------------------------------------------------------------------*
 *  Main Loop
 *-------------------------------------------------------------------------*/
  while(1)
  {
    ret = fread(buf, 1, READ_SIZE, in_fd);  /* read input record             */
    if (ret != READ_SIZE)
    {
      close_all();
      exit(3);
    }
    byte = *buf;                            /* first byte of record          */

    if (byte != 'a' && byte != 'c' && byte != 'd' && byte != 'z'
    && byte != 'A' && byte != 'C' && byte != 'D' && byte != 'Z')
    {
      close_all();
      exit(4);
    }
    else if (byte == 'z' || byte == 'Z')  /* terminating record              */
    {
      close_all();
      exit(0);
    }
    ret = fwrite (buf, 1, WRITE_SIZE, out_fd);/* output to SKU batch file   */
    if (ret != WRITE_SIZE)
    {
      close_all();
      exit(5);                            /* sku output write error          */
    }
  }
  exit(0);
}

/* close all files before returning to calling program  */
close_all()
{
  fclose(in_fd);
  fclose(out_fd);
  unlink(work);
  return;
}

/* end of sku_diskette_read.c */
