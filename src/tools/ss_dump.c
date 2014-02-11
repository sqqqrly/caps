/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Dumps system shared segment.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/21/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char ss_dump_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_names.h"
#include "ss.h"
#include "ss_init.h"

FILE *fd;
char fd_name[80];
char rf_name[80];
char suffix[40];

main(argc, argv)
long argc;
char **argv;
{
  register long k;

  putenv("_=ss_dump");                    /* name to environ                 */
  chdir(getenv("HOME"));                  /* insure in home directory        */
   
  ss_open();
   
  strcpy(fd_name, sp_text_name);          /* default file names              */
  strcpy(rf_name, rf_text_name);

  strcat(fd_name, ".dump");
  strcat(rf_name, ".dump");
  
  for (k = 1; k < argc; k++)
  {
    if (memcmp(argv[k], "-sp=", 4) == 0)      strcpy(fd_name, argv[k] + 4);
    else if (memcmp(argv[k], "-rf=", 4) == 0) strcpy(rf_name, argv[k] + 4);
  }
  printf("Dumping sp_text to [%s]\n", fd_name);
  printf("Dumping rf_text to [%s]\n", rf_name);

  if (*fd_name)
  {
    fd = fopen(fd_name, "w");
    if (fd == 0)
    {
      printf("*** Can't Open %s\n\n", fd_name);
      exit(1);
    }
    dump_part(fd, sp, ss_data);
    fclose(fd);
  }
  if (*rf_name)
  {
    fd = fopen(rf_name, "w");
    if (fd == 0)
    {
      printf("*** Can't Open %s\n\n", rf_name);
      exit(1);
    }
    dump_part(fd, rf, rf_data);
    fclose(fd);
  }
  ss_close();
   
  printf("All Done\n\n");
}
/*-------------------------------------------------------------------------*
 *  Dump A Portion
 *-------------------------------------------------------------------------*/
 
dump_part(fd, p, q)
FILE *fd;
unsigned char *p;
Tinit_item *q;
{
  register long k;
  unsigned long work;
  unsigned short word;
  
  while (1)
  {
    switch (q->init_type)
    {
      case 0:     return 0;
      
      case 'f':
      case 'c':   
        
        fprintf(fd, "%c     : %s\n", *p > 0 ? *p : 0x20, q->init_desc);
        break;
         
      case 's':   

        fprintf(fd, "%-*.*s : %s\n",
        q->init_length - 1, q->init_length - 1, p, q->init_desc);
        break;

      case 'z':
      case 'b':

        fprintf(fd, "%-5d : %s\n", *p, q->init_desc);
        break;
        
      case 'h':

        fprintf(fd, "%-5d : %s\n", *((unsigned short *)p), q->init_desc);
        break;
      
      case 'w':   

        fprintf(fd, "%-5d : %s\n", *((unsigned long *)p), q->init_desc);
        break;
    }
    p += q->init_length;
    q++;
  }
  return 0;
}

/* end of ss_dump.c */
