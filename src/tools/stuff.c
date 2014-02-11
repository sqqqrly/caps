#include <stdio.h>

typedef struct
{
  short a;
  long  b;
  char  c;
  short d;
  long  e;
  short f;
  short g;
  long  h;
  
  
} stuff;

stuff x;
char *y;

main()
{
  y = (char *)&x;
  
  printf("sizeof x = %d\n", sizeof(x));
  printf("&x = %8x  offset = %d\n", &x,   (char *)&x - y);
  printf("&a = %8x  offset = %d\n", &x.a, (char *)&x.a - y);
  printf("&b = %8x  offset = %d\n", &x.b, (char *)&x.b - y);
  printf("&c = %8x  offset = %d\n", &x.c, (char *)&x.c - y);
  printf("&d = %8x  offset = %d\n", &x.d, (char *)&x.d - y);
  printf("&e = %8x  offset = %d\n", &x.e, (char *)&x.e - y);
  printf("&e = %8x  offset = %d\n", &x.f, (char *)&x.f - y);
  printf("&e = %8x  offset = %d\n", &x.g, (char *)&x.g - y);
  printf("&e = %8x  offset = %d\n", &x.h, (char *)&x.h - y);
  

}
