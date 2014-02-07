/*
 * get_sub.c
 *
 * extract subroutine  get_sub "name" <in >out
 */
#include <stdio.h>

long to_end = 0;

main(argc, argv)
long argc;
char **argv;
{
  char buf[128];
  long flag, len;
  
  flag = 0;
  len = strlen(argv[1]);
  
  if (argc > 2) to_end = 1;

  while(fgets(buf, 128, stdin) > 0)
  {
    if (flag) 
    {
      fputs(buf, stdout);
      if (buf[0] == '}' && !to_end) exit(0);
      continue;
    }
    if (strlen(buf) < len) continue;
    if (strncmp(argv[1], buf, len) == 0) 
    {
      flag = 1;
      fputs(buf, stdout);
      continue;
    }
  }
  exit(0);
}


/* end of get_sub.c */

