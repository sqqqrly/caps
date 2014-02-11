/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Send tail of transactions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/09/94   |  tjt  Implemented.
 *  11/22/94   |  tjt  Revised to Send All By Default.
 *-------------------------------------------------------------------------*/
static char resend_trans_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>

#include "ss.h"
#include "xt.h"
#include "message_types.h"
#include "caps_messages.h"

extern leave();

FILE *fd;
char fd_name[] = "dat/db/xt_work.dat";

struct trans_item buf;
#define SIZE 65

long where;
long count;

main(argc, argv)
long argc;
char **argv;
{
  register long tick;

  putenv("_=resend_trans");
  chdir(getenv("HOME"));
  
  fprintf(stderr, "Resend Transactions\n\n");

  system("cp dat/db/transaction.dat dat/db/xt_work.dat");

  ss_open();
  message_open(leave);

  fd = fopen(fd_name, "r");
  if (fd == 0) 
  {
    fprintf(stderr, "Can't Open %s\n\n", fd_name);
    leave();
  }
  fseek(fd, 0, 2);
  where = ftell(fd);
  
  count = where / SIZE;

  fprintf(stderr, "%6d Transactions In The File\n", count);

  if (argc > 1) count = atol(argv[1]);

  fprintf(stderr, "%6d Will Be Sent\n\n", count);

  where -= (SIZE * count);
  
  if (where < 0) where = 0;
  
  fseek(fd, where, 0);

  while (fread(&buf, sizeof(struct trans_item), 1, fd) == 1)
  {
    fprintf(stderr, "%15.15s %61.61s\n", 
      (char *)ctime(&buf.xt_time) + 4, buf.xt_group);
    fflush(stderr);

    message_put(0, TransactionEvent, &buf, sizeof(struct trans_item));
  }
  sp->sp_to_count = 0;
    
  fprintf(stderr, "All Done\n\n");
  leave();
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  if (fd) fclose(fd);
  ss_close();
  message_close();
  exit(0);
}

/* end of resend_trans.c */
