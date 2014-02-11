#include <stdio.h>
#include "ss.h"


main(argc, argv)
long argc;
char **argv;
{
  unsigned char flag;

  putenv("_=set_remaining");
  chdir(getenv("HOME"));
  
  flag = argv[1][0];

  ss_open();

  printf("Remaining Picks Flag Is %c\n\n", sp->sp_remaining_picks);
  
  if (flag == 'y' || flag == 'n' || flag == 'u')
  {
    sp->sp_remaining_picks = flag;
    printf("Remaining Picks Flag Is %c\n\n", sp->sp_remaining_picks);
  }
  else printf("Bad Parameter\n\n");
  
  ss_close_save();
}
