#include <stdio.h>

main(argc, argv)
long argc;
char **argv;
{
  unlink(argv[1]);
}
