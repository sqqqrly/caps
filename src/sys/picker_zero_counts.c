/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/11/93    |  tjt Added to mfc.
 * 07/22/95    |  tjt Revise Bard calls.
 * 04/18/97    |  tjt Add language.h and code_to_caps.
 * 06/05/01    |  aha Modified for section productivity.
 * 07/25/01    |  aha Modified for new picker table fields.
 *-------------------------------------------------------------------------*/
static char picker_zero_counts_c[] = "%Z% %M% %I% (%G% - %U%)";
#include "caps_copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "iodefs.h"
#include "sd.h"
#include "co.h"
#include "ss.h"
#include "eh_nos.h"
#include "message_types.h"
#include "caps_messages.h"
#include "global_types.h"
#include "picker_zero_counts.t"
#include "picker_acct.h"
#include "Bard.h"
#include "bard/picker.h"
#include "bard/picker_order.h"
#include "language.h"
#include "sections.h"

extern leave();

#define CURRENT      0
#define LAST         0
#define NUM_PROMPTS  1

static short ONE = 1;
#define MAX_BUF_SIZE 1

struct fld_parms fld[NUM_PROMPTS] = {
   { 9,67, 8,1,&ONE,"Zero Current Accountability Counts (y/n)",'a'}
	};

static char buffers[NUM_PROMPTS][MAX_BUF_SIZE+1];  /* input buffer space */

static short rm = 0;  /* no one changes this, but we do need to be able to take
                         its address, so to avoid everyone declaring their own
                         useless copy, we make one global useless copy... */

/*------------------------------------------------------------------------ */

main()
{
  short i;
  unsigned char t;
  int index = 0;

  putenv("_=picker_zero_counts");
  chdir(getenv("HOME"));
  
  /*
  ** do setup and create the data on the screen
  */
  open_all();

  sd_screen_off();
  fix(picker_zero_counts);
  sd_clear_screen();
  sd_text(picker_zero_counts);
  sd_screen_on();
  
/*
 ** show and process the menu
 */
  sd_prompt(&fld[CURRENT], rm);

  while(1)
  {
    /*
    ** get user input until user presses RETURN on last field
    ** (or presses EXIT at any time)
    */
    t = sd_input(&fld[index],rm,&rm,buffers[index],0);

    switch (t) {
      case EXIT:
        leave();
      case UP_CURSOR:
        if (index > 0) index--;
        continue;
      case DOWN_CURSOR:
      case TAB:
        if (index < LAST)
          index++;
        else
          index = 0;
        continue;
      case RETURN:
        if (index < LAST)
        {
          index++;
          continue;
        }
    }

    /*
    ** now check out all the given information and verify it as ok 
    */
    if (buffers[CURRENT][0] == '\0')
    {
      eh_post(ERR_YN, 0);
      continue;
    }
    if ((buffers[CURRENT][0] != '\0')
        && (code_to_caps(buffers[CURRENT][0]) != 'n')
        && (code_to_caps(buffers[CURRENT][0]) != 'y'))
    {
      eh_post(ERR_YN, "");
      index = CURRENT;
      continue;
    }
    
    /*
    ** all ok, so perform the desired action
    */
    if (code_to_caps(buffers[CURRENT][0]) == 'y') zero_current();
    else leave();

    /*
    ** now clear the input fields and loop again, getting more user input
    */
    index = CURRENT;
    buffers[CURRENT][0] = '\0';
    sd_prompt(&fld[CURRENT], rm);
  }
}


/*
** zero current counts
*/
zero_current()
{
  struct zone_item *z = 0;
  unsigned short int i = 0;
  char text[80],
       datemsg[20];
  picker_item picker;
  picker_order_item picker_order;
  section_prod_log_item splog;
  unsigned long int now = 0L;
  time_t * nowptr = 0;
  TZone zone_num = 0;

  datemsg[19] = '\0';
  text[79]    = '\0';

  memset(&splog, 0x0, sizeof(section_prod_log_item));

  /*
  ** zero the total configured time values
  */
  if (!zero_pa_file())
    return; 

  /*
  ** clear out all picker records
  */
  begin_work();
  while ( picker_next( &picker, LOCK ) == 0 )
  {
    picker.p_zone = 0;
    picker.p_status = 0;
    picker.p_cur_order_count = 0;
    picker.p_cur_lines = 0;
    picker.p_cur_units = 0;
    picker.p_cur_time  = 0;
    picker.p_cum_order_count = 0;
    picker.p_cum_lines = 0;
    picker.p_cum_units = 0;
    picker.p_cum_time  = 0;
    picker_update( &picker );

    commit_work();
    begin_work();
  }
  commit_work();

  /*
  ** clear out all picker_order records
  */
  begin_work();
  while ( !picker_order_next( &picker_order, LOCK ))
  {
    if (picker_order.p_completion_time)
    {               
      if (picker_order.p_order_status & PA_CURRENT)
      {
        picker_order.p_order_status ^= (PA_CURRENT);
        picker_order_update( &picker_order );
      }
    }
    commit_work();
    begin_work();
  }
  commit_work();

  sprintf(text, "dbaccess caps sql/copy_section_prod.sql %s",
          "1>/dev/null 2>&1");
  system(text);
 
  nowptr = (time_t *)&now;
  strftime(datemsg, 20, "%Y-%m-%d %T", localtime(nowptr));

  strcpy(splog.spl_record_date, datemsg);


  sprintf(text, "dbaccess caps src/h/informix/section_prod.sql %s",
          "1>/dev/null 2>&1");
  system(text);

  for (i = 0; i <= coh->co_zone_cnt; i++)
      {
         z = &zone[i];
         zone_num = z->zt_zone;
         z->zt_picker = 0;
         memset(z->zt_picker_name, 0, 12);
         z->zt_picker_name[11] = '\0';
         message_put(0, ZoneOfflineRequest, &zone_num, sizeof(TZone));
      }

  eh_post(ERR_CONFIRM, "Zero current counts");
}

/*
** zero the data in the picker_accountability file for total config.d times.
** returns 0 on failure.
*/
int zero_pa_file(void)
{
  int fd;
  PA_FILESTRUCT data;
  int ok;
  long now;

  ok = open_pa_file(fd);      /* open the file and read the current contents */
  if (ok)
    ok = read_pa_file(fd, &data);
  if (ok)
  {                                 /* do the actual work to zero the fields */
    time(&now);
    data.time_at_zero_current = now;
    data.current_config_time = 0;
    if (sp->sp_config_status == 'y')
      data.last_start_time = now;
    else
      data.last_start_time = 0;
      
    data.time_at_zero_cumulative = now;
    data.cumulative_config_time = 0;

    ok = write_pa_file(fd, &data);        /* write the new contents */
  }
  if (ok)
    ok = close_pa_file(fd);               /* close the file */

  if (!ok)                                /* tell user if an error occurred */
  {
    krash("zero_pa_file", "failed", 1);
  }
  return ok;
}

leave()
{
  close_all();
  execlp("picker_acctability","picker_acctability",0);
  krash("leave", "picker_acctability load", 1);
}

/*
 *open all files
 */
open_all()
{
  database_open();

  sd_open(leave);
  ss_open();
  co_open();

  picker_open(AUTOLOCK); 
  picker_setkey(0);

  picker_order_open(AUTOLOCK); 
  picker_order_setkey(0);


  return 0;
}

/*
 *close all files
 */
close_all()
{ 
  picker_close();
  picker_order_close();
  co_close();
  ss_close();
  sd_close();
  database_close();
  return 0;
}

/*
 *transfer control back to calling program
 */
/* end of picker_zero_count.c */
