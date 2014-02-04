/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Specify message selections.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/14/93    |  tjt  Original implementation
 *-------------------------------------------------------------------------*/
static char message_select_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "kernel_types.h"

message_select(list, count)
register unsigned char *list;
register long count;
{
  register long j, k, ret;

  if (count < 1)
  {
    krash("message_select", "Count Less Than 1", 1);
  }
  for (k = 0; k < count; k += j)
  {
    j = count - k;
    if (j > MessageText) j = MessageText;

    ret = message_put(KernelDestination, KernelSelect, list + k, j);
    if (ret < 0) return -1;
  }
  return 0;
}

/* end of message_select.c */
