#include <stdio.h>

FILE *fp;

char stuff[64];

main()
{
  fp = popen("kermit >junk", "w");
  
  while (1)
  {
    printf("Enter Cmd --> ");
    gets(stuff);
    if (*stuff == '$') break;
    fputs(stuff, fp);
    fflush(fp);
    
  }
  pclose(fp);
}
