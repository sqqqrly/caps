/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Full Function Tests
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  02/08/94   |  tjt  Added to mfc.
 *  01/23/95   |  tjt  Add new IS_ONE_PICKLINE.
 *  06/04/95   |  tjt  Add pickline input by name.
 *  08/01/95   |  tjt  Add check is full function pickline.
 *  08/01/05   |  tjt  Add check is full function zone.
 *-------------------------------------------------------------------------*/
static char test_menu_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "caps_messages.h"
#include "message_types.h"
#include "eh_nos.h"
#include "getparms.h"
#include "test_menu.t"

extern leave();

unsigned char t;
char buf[2] = {0};
char range[16];
char pickline[9] = {0};
long low, high, line;
TZoneTestMessage y;

short ONE = 1;
short LPL = 8;
short SEVEN = 7;

struct fld_parms fld1 = {15,56,26,1,&ONE,"Enter Test Number or Code",'a'};
struct fld_parms fld2 = {16,56,26,1,&LPL,"Enter Pickline",'a'};
struct fld_parms fld3 = {17,56,26,1,&SEVEN,"Enter Zone Range",'a'};

main()
{
  register char *p;
  register long k, n;
  
  putenv("_=test_menu");
  chdir(getenv("HOME"));

  sd_open(leave);
  ss_open();
  co_open();
  getparms();
  
  fix(test_menu);
  sd_screen_off();
  sd_clear_screen();
  sd_text(test_menu);
  sd_screen_on();
  
  while(1)
  {
    buf[0] = buf[1] = 0;

    sd_prompt(&fld1, 0);
    t = sd_input(&fld1, 0, 0, buf, 0);
    if (t ==  EXIT) leave();

    *buf = tolower(*buf);

    switch (*buf)
    {
      case '1':
      case '2':
      case '3':
      case '4': 
      case '5':  y.m_test = *buf - '0'; break;
          
      case 'r':  y.m_test = 0; break;
      
      default:   eh_post(ERR_CODE, buf);
                 continue;
    }
    if (IS_ONE_PICKLINE) line = op_pl; 
    else
    {
      if (SUPER_OP)
      {
        sd_prompt(&fld2, 0);
        t = sd_input(&fld2, 0, 0, pickline, 0);
        if (t == EXIT) leave();

        line = pl_lookup(pickline, 0);

        if (line < 0 || line > coh->co_pl_cnt)
        {
          eh_post(ERR_PL, pickline);
          continue;
        }
        if (line && !(pl[line - 1].pl_flags & IsFullFunction))
        {
          eh_post(ERR_PL, pickline);
          continue;
        }
        if (!line)           
        {
          for (y.m_zone = 1; y.m_zone <= coh->co_zone_cnt; y.m_zone++)
          {
            if (!(zone[y.m_zone - 1].zt_zone)) continue;
            if (!(zone[y.m_zone - 1].zt_flags & IsFullFunction)) continue;
            
            message_put(0, ZoneTestRequest, &y, sizeof(TZoneTestMessage));
          }
          continue;
        }
      }
      else line = op_pl;
    }
    memset(range, 0, SEVEN);
    sd_prompt(&fld3, 0);
    t = sd_input(&fld3, 0, 0 ,range, 0);
    if (t == EXIT) leave();

    n = strlen(range);
    p = (char *)memchr(range, '-', n);
    if (p) *p++ = 0;
    
    high = coh->co_zone_cnt; low = 1;

    if (n > 0) high = low = atol(range);
    if (p)     high = atol(p);

    if (low > high || low < 1)
    {
      eh_post(ERR_RANGE, 0);
      continue;
    }
    if (n && (zone[low - 1].zt_pl != line || zone[high - 1].zt_pl != line))
    {
      eh_post(ERR_RANGE, 0);
      continue;
    }
    for (y.m_zone = low; y.m_zone <= high; y.m_zone++)
    {
      if (zone[y.m_zone - 1].zt_pl == line)
      {
        message_put(0, ZoneTestRequest, &y, sizeof(TZoneTestMessage));
      }
    }
  }
}
leave()
{
  sd_close();
  ss_close();
  co_close();
  execlp("diagnostics", "diagnostics", 0);
  krash("leave", "load operm", 1);
}


/* end of test_menu.c */



