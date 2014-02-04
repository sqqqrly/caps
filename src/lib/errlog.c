/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  CAPS Error Logging Function.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/07/93    |  tjt Original implementation
 *-------------------------------------------------------------------------*/
static char errlog_c[] = "%Z% %M% %I% (%G% - %U%)";

/*
 *  errlog.c
 *
 *  Common Error Logging Routine 
 *
 *    Called as return errlog(message);
 *
 *    message is any null terminated text.
 *
 */
#include <stdio.h>
#include "file_names.h"

long errlog(p)
register char *p;                         /* message text                    */
{
  char who[128];                          /* program identification          */
  FILE *fd;
  char *q;
  char datetime[24];
  long now;

  fd = fopen(errlog_name, "a+");          /* open to append                  */

  now = time(0);                          /* current time                    */
  memcpy(datetime, ctime(&now), 24);      /* date time text                  */
   
  q = (char *)getenv("_");                /* program name                    */

  if (q) sprintf(who, "%d %s", getpid(), q);
  else   sprintf(who, "%d", getpid());    /* pid only                        */

  if (fd)
  {
    fprintf(fd, "%24.24s PID:%s %s\n", datetime, who, p);
    fclose(fd);
  }
  fprintf(stderr, "\r%24.24s PID:%s %s\n", datetime, who, p);
  fflush(stderr);

  return 0;
}

/* end of errlog.c */

