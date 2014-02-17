#define DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    View, print, purge message log.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  11/26/95   |  tjt  Added to mfc.
 *  04/19/96   |  tjt  Add clear log count on purge.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char message_view_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "sd.h"
#include "ss.h"
#include "iodefs.h"
#include "message_view.t"
#include "eh_nos.h"
#include "language.h"
#include "Bard.h"
#include "bard/maint_log.h"

extern leave();

FILE *fd;
char  fd_name[16];

short ONE =1;
short NINETEEN = 19;

struct fld_parms fld[] = {
  { 7,45,20,1, &ONE,      "Find or Purge? (f/p)", 'a'},
  { 8,45,20,1, &NINETEEN, "Starting Date/Time",   'a'},
  { 9,45,20,1, &NINETEEN, "Ending   Date/Time",   'a'},
  {12,45,20,1, &ONE,      "Are You Sure? (y/n)",  'a'},
};

maint_log_item x;

main()
{
  unsigned char t, incode[2], ans[2], yn[2];
  unsigned char start_time[20], end_time[20], datetime[32];
  long start, end;
  long count;

  putenv("_=message_view");
  chdir(getenv("HOME"));
  
  sd_open(leave);
  ss_open();
  database_open();
  log_open(AUTOLOCK);
  log_setkey(1);
  
  fix(message_view);
  sd_screen_off();
  sd_clear_screen();
  sd_text(message_view);
  sd_screen_on();
  
  sd_prompt(&fld[0], 0);
  sd_prompt(&fld[1], 0);
  sd_prompt(&fld[2], 0);

  sd_cursor(0, 10, 20);
  sd_text("Format is:");
  sd_cursor(0, 10, 45);
  sd_text("mm/dd/yyyy hh:mm:ss");

  while(1)
  {
    memset(incode, 0, 2);
    memset(start_time, 0, 20);
    memset(end_time, 0, 20);
    
    t = sd_input(&fld[0], 0, 0, incode, 0);
    if(t == EXIT)      leave();
    if(t == UP_CURSOR) continue;

    *incode = tolower(*incode);

    if (*incode != 'f' && *incode != 'p')
    {
      eh_post(ERR_CODE, incode);
      continue;
    }
    while(1)                  
    {
      t = sd_input(&fld[1], 0, 0, start_time, 0); /* starting time           */
      if(t == EXIT)      leave();
      if(t == UP_CURSOR) break;

      if(*start_time)
      {
        if(!time_convert(start_time, &start))
        {
#ifdef DEBUG
  fprintf(stderr, "start:%s = %d (now = %d)\n", start_time, start, time(0));
#endif
          eh_post(ERR_TIME, start_time);
          continue;
        }
      }
      else start = 0;

      while(1)                          
      {
        t = sd_input(&fld[2], 0, 0, end_time, 0); /* ending time             */
        if(t == EXIT)      leave();
        if(t == UP_CURSOR) break;

        if(*end_time)
        {
          if(!time_convert(end_time, &end))
          {
#ifdef DEBUG
  fprintf(stderr, "end: %s = %d (now = %d)\n", end_time, start, time(0));
#endif
            eh_post(ERR_TIME, end_time);
            continue;
          }
        }
        else end = 0x7fffffff;
        
        if (t == UP_CURSOR) continue;
        break;
      }
      while (*incode == 'p')
      {
        sd_prompt(&fld[3], 0);
        memset(ans, 0, 2);
        memset(yn,  0, 2);

        t = sd_input(&fld[3], 0, 0, yn, 0);
        if(t == EXIT)      leave();
        if(t == UP_CURSOR) break;

        *ans = code_to_caps(*yn);

        if (*ans != 'y' && *ans != 'n')
        {
          eh_post(ERR_YN, ans);
          continue;
        }
        break;
      }
      if (*incode == 'p' && *ans == 'n') continue;
      break;
    }
    sd_wait();

    x.m_log_time = start;
    log_startkey(&x);
    
    x.m_log_time = end;
    log_stopkey(&x);
    
    if (*incode == 'p')
    {
      begin_work();
      
      sp->sp_log_count = 0;
      
      while (!log_next(&x, LOCK))
      {
        log_delete();
        commit_work();
        begin_work();
      }
      commit_work();
    
      eh_post(ERR_CONFIRM, "Log purge");
      continue;
    }
    tmp_name(fd_name);
    fd = fopen(fd_name, "w");
    if (fd == 0) krash("message_view", "tmp file", 1);
    count = 0;
        
    while (!log_next(&x, NOLOCK))
    {
      strcpy(datetime, ctime(&x.m_log_time));
      
      fprintf(fd, "%15.15s %63.63s\n", datetime + 4, x.m_log_text);
      count++;
    }
    fclose(fd);
    
    if (count < 1)
    {
      eh_post(ERR_CONFIRM, "No Data - Find");
      unlink(fd_name);
      continue;
    }
    sd_close();
    ss_close();
    log_close();
    database_close();
  
    execlp("database_report", "database_report",
    message_view, fd_name, "80", "6", "16",
    "sys/report/report_report.h", "operm", 
    "    Message Log", "0", "0", "0", 0);
  
    krash("message_view", "load database_report", 1);
  }
}
/*-------------------------------------------------------------------------*
 * function to transfer control back to the calling program
 *-------------------------------------------------------------------------*/
leave()
{
  sd_close();
  ss_close();
  log_close();
  database_close();
  execlp("operm", "operm", 0);
  krash("message_view", "load operm", 1);
}

/* end of message_view.c */
