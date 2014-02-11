/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Separate into many files
 *
 *  Method:         #name=xxxxxxx      (name of file to open).
 *
 *  Execution:      uncat <input
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  04/14/97   | tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char uncat[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <string.h>

FILE *fd;
char fd_name[64];
char buf[128];

main()
{
  while (fgets(buf, 128, stdin))
  { 
     if (memcmp(buf, "#name", 5) == 0)
     {
       if (fd) fclose(fd);
       memset(fd_name, 0, 64);
       memcpy(fd_name, buf + 6, strlen(buf) - 7);
       fd = fopen(fd_name, "w");
       if (fd) printf(".. %s\n", fd_name);
       else    printf("Can't Open %s\n", fd_name);
       continue;
     }
     if (fd) fputs(buf, fd);
  }
  if (fd) fclose(fd);
}

/* end of uncat.c */
