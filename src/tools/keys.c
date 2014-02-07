#include <stdio.h>

main()
{
  register unsigned char c;

  system("stty raw");
  system("stty -echo");
  
  while (1)
  {
    c = getchar();
    printf("0x%02x [%c]\r\n", c, (c > 0x20 && c < 0x7f) ? c : '.');
    if (c == '$') break;
  }
  system("stty sane");
}
