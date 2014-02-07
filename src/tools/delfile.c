#include <stdio.h>

main(argc, argv)
long argc;
char **argv;
{
  if (argc > 1) unlink(argv[1]);
}
