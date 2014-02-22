/*-----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Skeleton Transaction Server.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/03/94   |  tjt  Original Implementation.
 *-------------------------------------------------------------------------*/
static char trans_server_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xt.h"
#include "caps_messages.h"
#include "message_types.h"

unsigned char list[] = {ShutdownRequest, TransactionEvent};

main(argc, argv)
long argc;
char **argv;
{
  long who, type, len;                      /* message paramters             */
  unsigned char buf[256];                   /* message buffer                */

  putenv("_=trans_server");
  chdir(getenv("HOME"));
  
  open_all();                               /* open everything               */

  while (1)                                 /* loop until shutdown message   */
  { 
    message_get(&who, &type, buf, &len);    /* get message                   */
    process_message(who, type, buf, len);   /* process message               */
  }
}
/*-------------------------------------------------------------------------*
 *  CAPS Message Catcher
 *-------------------------------------------------------------------------*/
process_message(who, type, buf, len)
register long who, type;
register TCapsMessageItem *buf;
register long len;
{
#ifdef DEBUG
  fprintf(stderr, "CAPS Message: who=%d type=%d len=%d\n", who, type, len);
  if (len > 0) Bdump(buf, len);
#endif

  switch (type)
  {
    case ShutdownRequest:
      
      close_all();
      message_close();
      exit(0);
    
    case TransactionEvent:
    
      process_transaction(buf);
      break;

    default: break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Transactions
 *-------------------------------------------------------------------------*/
process_transaction(buf)
register struct trans_item *buf;
{
  if (buf->xt_code == 'P' || buf->xt_code == 'S')
  {
    return 0;                              /* process pick or short          */
  }
  if (buf->xt_code == 'Z')
  {
    return 0;                              /* process zone event             */
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
open_all()
{
  message_open();
  message_select(list, sizeof(list));
  
  /* open output  */
  
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Close All Files
 *-------------------------------------------------------------------------*/
close_all()
{
  
  return 0;
}

/* end of trans_server.c */
