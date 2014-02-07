/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Convert sp_text to new format.  
 *
 *                  cvrt_sp <in > out
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/04/96   | tjt  Original implemeatation
 *-------------------------------------------------------------------------*/
static char cvrt_sp_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

main()
{
  register long j, k;
  char buf[128];
  
  for (k = 1; k < 256; k++)
  {
    if (!gets(buf))
    {
      if (k != 103)
      {
        fprintf(stderr, 
          "File should have 102 lines.  Has %d lines.\n\n", k - 1);
      
        fprintf(stderr, "*** Failed");
        exit(1);
      }
      break;
    }
    if (k == 78)
    {
      for (j = 0; j < 12; j++) printf("0     :new\n");
    }
    else if (k == 87) printf("0     :new\n");
    printf("%s\n", buf);
  }
  fprintf(stderr, "All Done\n\n");
  exit(0);
}



