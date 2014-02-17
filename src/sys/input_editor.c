/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Interface to Tom's Screen Editor.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/30/95   | tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char input_editor_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sd.h"

long pid, status;

main (argc, argv)
long argc;
char **argv;
{
  register long k;
  
  putenv("_=input_editor");
  chdir(getenv("HOME"));
  
#ifdef DEBUG
  for (k = 0; k < argc; k++)
  {
    fprintf(stderr, "argv[%d] = [%s]\n", k, argv[k]);
  }
#endif
    
  sd_open();
  sd_tty_close();
  sd_close();
  sleep(1);
  
  if (fork() == 0)
  {
    execlp("tsa", "tsa", "-nobak", argv[2], 0);
    krash("main", "load tsa", 1);
  }
  pid = wait(&status);
  
  sd_open();
  sd_tty_open();
  sd_close();
  
  execlp(argv[1], argv[1], 0);
  krash("main", "load return prog", 0);
}

/* end of input_editor.c */

