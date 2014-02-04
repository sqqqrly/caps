/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Write an entry in the database table.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/06/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char log_entry_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <string.h>
#include "ss.h"
#include "../h/oracle_defines.h"
#include "Bard.h"
#include "./bard/maint_log.h"

log_entry(p)
register char *p;                         /* null terminated text            */
{
  register long len;
  maint_log_item log;
  
  if (!log_fd) krash("log_entry", "log not open", 1);
  
  memset(&log, 0, sizeof(maint_log_item));
  
  log.m_log_ref  = ++sp->sp_log_count;
  log.m_log_time = time(0);
  
  len = strlen(p);
  if (len < 1) return 0;
  if (len > 63) len = 63;
  
  memcpy(log.m_log_text, p, len);
  
  log_write(&log);
  return 0;
}

/* end of log_entry.c */

