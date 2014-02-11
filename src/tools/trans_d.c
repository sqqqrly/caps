#define DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Model Transaction Processing Daemon.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/26/96   | tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char trans_d_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

#include "message_types.h"
#include "caps_messages.h"
#include "xt.h"

long who;                                 /* sender identification           */
long type;                                /* caps message number             */
long length;                              /* message length                  */

struct trans_item xt;                     /* transaction message             */

unsigned char list[] = {TransactionEvent, ConfigureEvent, RestoreplaceEvent,
                        MarkplaceEvent, ShutdownRequest};
             
long running = 0;                         /* caps is picking                 */

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=trans_d");
  chdir(getenv("HOME="));

  message_open();
  message_select(list, sizeof(list));
  
  while (1)
  {
    message_get(&who, &type, &xt, &length);
  
#ifdef DEBUG
  fprintf(stderr, "%.*s\n", sizeof(struct trans_item) - 8, xt.xt_con);
#endif
    
    switch (type)
    {
      case ShutdownRequest: leave();
      
      case ConfigureEvent:
      case RestoreplaceEvent: running = 1;
                              break;

      case MarkplaceEvent:   running = 0;
                             break;
      
      case TransactionEvent: process_trans(&xt);
                             break;
                      
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Process Transactions
 *-------------------------------------------------------------------------*/
process_trans(xt)
register struct trans_item *xt;
{
  switch (xt->xt_code)
  {
    case 'B':                              /* box close event                */
              /* TODO - box full event */
              break;

    case 'P':                              /* full pick event                */
              /* TODO - pick without short */
              break;
              
    case 'S':                              /* short pick event               */
              /* TODO - pick with short */
              break;

    case 'C':                              /* order complete event           */
              /* TODO - order complete */
              break;

    case 'U':                              /* order underway event           */
              /* TODO - order underway */
              break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  message_close();
  exit(0);
}

/* end of trans_d.c */
