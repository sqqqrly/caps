/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Test Kernel Speed.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *             |
 *             |
 *-------------------------------------------------------------------------*/
static char kernel_test_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "message.h"

#define MAX 10000

long start, end, who, type, len, buf;
unsigned char list[] = {1, 0};

main()
{
  register long k;

  putenv("_=kernel_test");
  chdir(getenv("HOME"));
  
  message_open();
  message_select(list, sizeof(list));

  start = time(0);

  for (k = 0; k < MAX; k++)
  {
    message_put(0, 1, 0, 0);
    message_get(&who, &type, &buf, &len);
  }
  end = time(0);

  printf("%d messages in %d seconds\n", k, end - start);
}

/* end of kernel_test.c  */
