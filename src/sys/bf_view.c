/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Basic Function Controller Simulator.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  02/12/94   |  tjt  Original implementation.
 *  05/22/94   |  tjt  Display revised.
 *  03/24/95   |  tjt  Revised.
 *  05/07/95   |  tjt  Add find_module to position cursor. 
 *  07/21/95   |  tjt  Revise Bard calls.
 *  08/23/96   |  tjt  Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char bf_view_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "iodefs.h"
#include "ss.h"
#include "co.h"
#include "sd.h"
#include "eh_nos.h"
#include "caps_messages.h"
#include "message_types.h"
#include "bf_view.t"

#include "Bard.h"
#include "bard/pmfile.h"
long pm_fd = 0;

extern leave();
void do_view(int signum);

#define ROW  24
#define COL  132
#define MOD  50

unsigned char screen[ROW][COL];           /* current screen image            */
unsigned char image[ROW][COL];            /* new screen image                */

long image_flag = 0;                      /* refresh is needed               */
long row        = 0;                      /* cursor for input                */
long col        = 0;                      /* cursor for input                */
long first      = 0;                      /* first open screen               */

char *offon[2]                  = {"Off", "On"};

/*-------------------------------------------------------------------------*
 *  View Parmeters
 *-------------------------------------------------------------------------*/
 
long v_first_module;                      /* current least module            */
long v_last_module;                       /* current last module             */
long v_show_bay;                          /* 0 = zone   n = bay number       */
long v_view;                              /* 0 = off,   1 = on               */
long v_busy;                              /* 0 = off,   1 = on               */

long v_row = 6;                           /* current cursor position         */
long v_col = 2;

char v_title[MOD][120];                   /* box title                       */
unsigned char  v_type[MOD];               /* module type                     */
unsigned short v_module[MOD];             /* module number                   */
unsigned char  v_tc[MOD];                 /* tc of module                    */
unsigned char  v_pi[MOD];                 /* pi of module                    */
unsigned char  v_port[MOD];               /* port of module                  */

long v_max;                               /* number of modules               */
long v_mod;                               /* current module                  */

main(argc, argv)
long argc;
char **argv;
{
  unsigned char work[5];
  register long j, k;

  putenv("_=bf_view");
  chdir(getenv("HOME"));

  ss_open();
  co_open();
  sd_open(leave);
  if (sp->sp_sku_support == 'y') 
  {
    database_open();
    pmfile_open(READONLY);
    pmfile_setkey(1);
  }
  open_screen();

  do_input();

  leave();
}
/*-------------------------------------------------------------------------*
 *  Get Keyboard Input
 *-------------------------------------------------------------------------*/
do_input()
{
  register unsigned char c;
  register struct hw_item *h;
  register struct bay_item *b;
  register struct zone_item *z;
  register long j;
  char text[16];
  
  while (1)
  {
    sd_cursor(0, v_row, v_col);

    c = sd_keystroke(CURSOR * v_view);
    c = tolower(c);
    
    switch (c)
    {
      case EXIT:                          /* exit function                   */
                        
        if (v_view) v_view = 0;           /* refresh is running              */
        alarm(0);                         /* reset any alarm                 */
        leave();                          /* exit program                    */
                                                                
      case 's':                           /* superpicker message             */
      
        if (sp->sp_basic_function != 's') break;
        eh_post(LOCAL_MSG, "Superpicker Started", 0);
        message_put(0, StartSuperpicker, 0, 0);
        break;
      
      case 'm':                          /* stop superpicker                */

        if (sp->sp_basic_function != 's') break;
        eh_post(LOCAL_MSG, "Superpicker Stopped", 0);
        message_put(0, StopSuperpicker, 0, 0);
        break;

      case 'z':                           /* zone display                    */
      
        if (v_view)
        {
          while (v_busy) sleep(1);
          v_view = 0;
          alarm(0);
        }
        get_zone();
        v_view = 1;
        new_screen();
        break;
                                                        
      case 'b':                           /* display by bay                  */

        if (v_view)
        {
          while (v_busy) sleep(1);
          v_view = 0;
          alarm(0);
        }
        get_bay();
        v_view = 1;
        new_screen();
        break;

      case 'f':                           /* box full switch                 */
      
        if (!v_max) break;
        if (sp->sp_basic_function != 's') break;
        if (sp->sp_box_full == 'n') break;
        
        sprintf(text, "%04d00000000000000%c", v_tc[v_mod], v_port[v_mod]);
        message_put(0, TCInputPacket, text, sizeof(TPacketMessage));
        
        break;
                        
      case 'x':                           /* short switch                    */
      
        if (!v_max) break;
        if (sp->sp_basic_function != 's') break;
        if (v_type[v_mod] != ZC) break;
        
        sprintf(text, "%04d00000000000001%c", v_tc[v_mod], v_port[v_mod]);
        message_put(0, TCInputPacket, text, sizeof(TPacketMessage));
        
        break;
                        
      case 'y':                           /* recall switch                   */
      
        if (!v_max) break;
        if (sp->sp_basic_function != 's') break;
        if (v_type[v_mod] != ZC) break;
                
        sprintf(text, "%04d00000000000002%c", v_tc[v_mod], v_port[v_mod]);
        message_put(0, TCInputPacket, text, sizeof(TPacketMessage));
        
        break;
                        
      case 'n':                           /* next switch                     */
      
        if (!v_max) break;
        if (sp->sp_basic_function != 's') break;
        if (v_type[v_mod] != ZC) break;
        
        sprintf(text, "%04d00000000000003%c", v_tc[v_mod], v_port[v_mod]);
        message_put(0, TCInputPacket, text, sizeof(TPacketMessage));
        
        break;
                        
      case 'p':                           /* pick switch                     */
      
        if (!v_max) break;
        if (sp->sp_basic_function != 's') break;
        if (v_type[v_mod] != PI) break;
        
        sprintf(text, "%04d%02d000000000004%c", 
          v_tc[v_mod], v_pi[v_mod], v_port[v_mod]);
        message_put(0, TCInputPacket, text, sizeof(TPacketMessage));
        
        break;
                        
      case RETURN:
      
        v_mod = 0;
        show_title();
        v_row = 6; v_col = 2;
        break;

      case LEFT_CURSOR:                   /* move left                       */

        if (!v_max) break;
        if (v_mod > 0)
        {
          v_mod--;
          find_module(-1);
          show_title();
          v_row = 4  * (v_mod / 10) + 6;
          v_col = 13 * (v_mod % 10) + 2;
          sd_cursor(0, v_row, v_col);
        }
        break;
                        
      case DOWN_CURSOR:                   /* move down                       */

        if (!v_max) break;
        v_mod = v_mod + 10;
        if (v_mod >= v_max) v_mod = v_max;
        find_module(-1);
        show_title();

        v_row = 4  * (v_mod / 10) + 6;
        v_col = 13 * (v_mod % 10) + 2;
        sd_cursor(0, v_row, v_col);
        break;
                        
      case UP_CURSOR:                    /* move up                         */

        if (!v_max) break;

        if (v_mod >= 10) v_mod -= 10;
        find_module(-1);
        show_title();
                                                        
        v_row = 4  * (v_mod / 10) + 6;
        v_col = 13 * (v_mod % 10) + 2;
        sd_cursor(0, v_row, v_col);
        break;
                        
      case RIGHT_CURSOR:                  /* move right                      */
      case TAB:                           /* tab right too                   */

        if (!v_max) break;

        v_mod = (v_mod + 1) % v_max;
        find_module(1);
        show_title();

        v_row = 4  * (v_mod / 10) + 6;
        v_col = 13 * (v_mod % 10) + 2;
        sd_cursor(0, v_row, v_col);
        break;
        
      case BACKWD:                        /* previous screen                 */

        if (!v_max) break;

        if (v_show_bay)
        {
          if (v_show_bay > 1) v_show_bay--;
          setup_first_module(&bay[v_show_bay - 1]);
        }
        else
        {
          if (v_first_module > 0) 
          {
            h = &hw[v_first_module - 1];
            if (h->hw_bay)
            {
              b = &bay[h->hw_bay - 1];
              z = &zone[b->bay_zone - 1];
              setup_first_module(&bay[z->zt_first_bay - 1]);
            }
            else break;
          }
          else break;
        }
        v_view = 1;
        new_screen();
        break;
                                                        
      case FORWRD:                        /* next screen                     */

        if (!v_max) break;
        if (v_show_bay)
        {
          if (v_show_bay < coh->co_bay_cnt) v_show_bay++;
          setup_first_module(&bay[v_show_bay - 1]);
        }
        else
        {
          if (v_last_module >= coh->co_light_cnt) break;
          v_first_module = v_last_module;
        }
        v_view = 1;
        new_screen();
        break;

      default:  

        break;
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Update Current View
 *-------------------------------------------------------------------------*/
void do_view(int signum)
{
  register long k, flag, count;
  register unsigned char *d;
  
  if (v_view) v_busy = 1;

  for (k = 0; k < v_max; k++)
  {
    if (v_type[k] == BL)      d = blv[v_module[k] - 1].hw_display;
    else if (v_type[k] == ZC) d = zcv[v_module[k] - 1].hw_display;
    else if (v_type[k] == PI) d = pmv[v_module[k] - 1].hw_display;
    else continue;
    
    make_box(k, v_type[k], 0, d);
  }
  show_title();
  refresh();
  sd_cursor(0, v_row, v_col);
  
  if (v_view) 
  {
    signal(SIGALRM, do_view);
    alarm(1);
  }      
  else alarm(0);

  v_busy = 0;
}
/*-------------------------------------------------------------------------*
 *  Setup New Screen
 *-------------------------------------------------------------------------*/
new_screen()
{
  register struct hw_item   *h;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long j, k, new_zone, new_bay;
  char last_bay[8], last_zone[8], last_pickline[8], last_module[8];
  pmfile_item pm;
  
  if (v_view)
  {
    while (v_busy) sleep(1);
  }
  memset(v_title, 0, sizeof(v_title));
  memset(screen, SPACE, ROW * COL);
  memset(image,  SPACE, ROW * COL);
  memcpy(image,  bf_view, COL);
  sd_clear_screen();
  
  memset(v_type,   0, sizeof(v_type));
  memset(v_module, 0, sizeof(v_module));
  memset(v_tc,     0, sizeof(v_tc));
  memset(v_pi,     0, sizeof(v_pi));
  new_bay = new_zone = 0;

  for (j = k = 0, h = &hw[v_first_module]; k < MOD; k++, h++)
  {
    if (v_first_module + k >= coh->co_light_cnt) break;
    if (v_show_bay && v_first_module + k > v_last_module) break;
    if (j >= MOD) break;

    strcpy(last_bay, "?");
    strcpy(last_zone, "?");
    strcpy(last_pickline, "?");
    strcpy(last_module, "?");
    
    if (h->hw_bay) 
    {
      b = &bay[h->hw_bay - 1];
 
      if (!(b->bay_flags & IsBasicFunction)) break;

      sprintf(last_bay, "%d", h->hw_bay);

      if (b->bay_zone) 
      {
        z = &zone[b->bay_zone - 1];

        if (!new_zone) new_zone = z->zt_zone;
        if (z->zt_zone != new_zone) break;

        sprintf(last_zone, "%d", z->zt_zone);
        sprintf(last_pickline, "%d", z->zt_pl);
      }
      if (b->bay_number != new_bay)
      {
        new_bay = b->bay_number;
        
        if (j % 10) j = j + 10 - (j % 10);
        if (j >= MOD) break;
        
        v_type[j]   = BL;
        v_module[j] = b->bay_number;
        v_tc[j]     = h->hw_controller;
        v_port[j]   = b->bay_port - 1;
        
        sprintf(v_title[j], 
          "Pickline: %s  Zone: %s  Bay: %s  Module: BL%d",
            last_pickline, last_zone, last_bay, b->bay_number);
        j++;
        
        v_type[j]   = ZC;
        v_module[j] = b->bay_number;
        v_tc[j]     = h->hw_controller;
        v_port[j]   = b->bay_port - 1;
        
        sprintf(v_title[j], 
          "Pickline: %s  Zone: %s  Bay: %s  Module: TC%d",
            last_pickline, last_zone, last_bay, b->bay_number);
        j++;
      }
    }
    v_type[j]   = PI;
    v_module[j] = h->hw_mod;
    v_tc[j]     = h->hw_controller;
    v_pi[j]     = h->hw_mod_address;
    v_port[j]   = b->bay_port - 1;
    
    sprintf(last_module, "PI%d", h->hw_mod);
    
    if (sp->sp_sku_support == 'y')
    {
      pm.p_pmodno = h->hw_mod;
      
      begin_work();
      if (!pmfile_read(&pm, NOLOCK))
      {
        sprintf(v_title[j], 
  "Pickline: %s  Zone: %s  Bay: %s  Module: PM%d  SKU: %*.*s  Stkloc: %6.6s",
          last_pickline, last_zone, last_bay, h->hw_mod,
          rf->rf_sku, rf->rf_sku, pm.p_pmsku, pm.p_stkloc);
        j++;
        commit_work();
        continue;
      }
      commit_work();
    }
    sprintf(v_title[j], 
          "Pickline: %s  Zone: %s  Bay: %s  Module: %s",
            last_pickline, last_zone, last_bay, last_module);
    j++;
  }
  v_last_module = v_first_module + k;
  v_max = j;
  v_mod = 0;
  v_row = 6; 
  v_col = 2;

  do_view(0); // Signal handler being passed zero!
}
/*-------------------------------------------------------------------------*
 *  Find Valid Module
 *-------------------------------------------------------------------------*/
find_module(n)
register long n;
{
  while (1)
  {
    if (v_type[v_mod]) return 0;
    if (v_mod > v_max) v_mod = 0;
    if (v_mod <= 0) return 0;
    v_mod += n;
  }
}
/*-------------------------------------------------------------------------*
 *  Show Module Title Information
 *-------------------------------------------------------------------------*/
show_title()
{
  if (v_mod < 0 || v_mod >= v_max) return 0;
  if (!v_title[v_mod][0]) return 0;
  
  memset(image[3], 0x20, 132);
  memcpy(image[3], v_title[v_mod], strlen(v_title[v_mod]));
  refresh(); 
}
/*-------------------------------------------------------------------------*
 *  Get Bay Information
 *-------------------------------------------------------------------------*/
get_bay()
{
  register long k;
  register struct zone_item *z;
  register struct bay_item  *b;

  static short FOUR = 4;
  static struct fld_parms fld = {19, 73, 53, 1, &FOUR, "Enter Bay", 'n'};
  unsigned char t, buf[4];
  
  if (!first)
  {
    sd_cursor(0, 1, 1);
    sd_clear_rest();
    sd_cursor(0, 1, 1);
    sd_text(bf_view);
  }
  first = 0;
  
  while (1)
  {
    sd_prompt(&fld, 0);
    t = sd_input(&fld, 0, 0, buf, 0);
 
    if (t == UP_CURSOR) {v_max = v_show_bay = 0; return 0;}
    if (t == EXIT) leave();

    k = atol(buf);
    if (k < 1 || k > coh->co_bay_cnt) 
    {
      eh_post(LOCAL_MSG, "Bay Out Of Range");
      continue;
    }
    b = &bay[k - 1];
    if (!(b->bay_flags & IsBasicFunction))
    {
      eh_post(LOCAL_MSG, "Not Basic Function");
      continue;
    }
    v_show_bay = k;
    break;
  }
  setup_first_module(b);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Setup First Module In Display
 *-------------------------------------------------------------------------*/
setup_first_module(b)
register struct bay_item *b;
{
  v_first_module = coh->co_light_cnt;
  v_last_module  = 1;
  
  if (b->bay_mod_first)
  { 
    v_first_module = mh[b->bay_mod_first - 1].mh_ptr - 1;
  }
  if (b->bay_mod_last)
  {
    v_last_module = mh[b->bay_mod_last - 1].mh_ptr - 1;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Get Zone Information
 *-------------------------------------------------------------------------*/
get_zone()
{
  register long k;
  register struct zone_item *z;
  register struct bay_item  *b;

  static short THREE = 3;
  static struct fld_parms fld = {19, 73, 53, 1, &THREE, "Enter Zone", 'n'};
  unsigned char t, buf[4];
  
  if (!first)
  {
    sd_cursor(0, 1, 1);
    sd_clear_rest();
    sd_cursor(0, 1, 1);
    sd_text(bf_view);
  }
  first = 0;
  
  while (1)
  {
    sd_prompt(&fld, 0);
    t = sd_input(&fld, 0, 0, buf, 0);
 
    if (t == UP_CURSOR) {v_max = v_show_bay = 0; return 0;}
    if (t == EXIT) leave();

    k = atol(buf);
    if (k < 1 || k > coh->co_zone_cnt) 
    {
      eh_post(ERR_ZONE, buf);
      continue;
    }
    z = &zone[k - 1];
    if (!z->zt_zone)
    {
      eh_post(ERR_ZONE, buf);
      continue;
    }
    if (!(z->zt_flags & IsBasicFunction))
    {
      eh_post(ERR_ZONE, buf);
      continue;
    }
    v_show_bay = 0;
    break;
  }
  setup_first_module(&bay[z->zt_first_bay - 1]);
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Refresh Screen
 *-------------------------------------------------------------------------*/
refresh()
{
  register long j;
  register unsigned char *p, *q;
        
  p = screen[0];
  q = image[0];

  for (j = 1; j <= ROW; j++, p += COL, q += COL)
  {
    if (memcmp(p, q, COL) == 0) continue;
    memcpy(p, q, COL);
    sd_cursor(0, j, 1);
    sd_text_2(p, COL);
  }
}
/*-------------------------------------------------------------------------*
 *  Make Box
 *-------------------------------------------------------------------------*/
make_box(n, type, switches, display)
register long n;
register long type;
register long switches;
register unsigned char *display;
{
  unsigned char work[16];
  register long row, col, j, k, width;
        
#ifdef DEBUG
  fprintf(stderr, "make_box(n=%d type=%d swt=%d)\n",
    n, type, switches);
#endif

  if (n < 0 || n >= MOD) return 0;

  row =  4 * (n / 10) + 4;
  col = 13 * (n % 10);
        
  if (type == BL)      width = 5;
  else if (type == ZC) width = 13;
  else if (type == PI) width = 5;
  else return 0;
  
  for (k = 0; k < 13; k++)
  {
    if (k == 0)
    {
      image[row][col]      = ULC;
      image[row + 1][col]  = VERT;
      image[row + 2][col]  = LLC;
    }
    else if (k == width - 1)
    {
      image[row][col + k]     = URC;
      image[row + 1][col + k] = VERT;
      image[row + 2][col + k] = LRC;
    }
    else if (k >= width)
    {
      image[row][col + k]     = SPACE;
      image[row + 1][col + k] = SPACE;
      image[row + 2][col + k] = SPACE;
    }
    else
    {
      image[row][col + k]     = HORT;
      image[row + 1][col + k] = SPACE;
      image[row + 2][col + k] = HORT;
    }
  }
  if (type == BL)
  {
    if (display[0] == '1') image[row + 1][col + 2] = '*';
    else                   image[row + 1][col + 2] = CROSS;
  }
  else if (type == ZC)
  {
      memcpy(&image[row + 1][col + 2], display, 10);
  }
  else if (type == PI)
  {
    if (display[0] == '1')
    {
      image[row + 1][col + 1] = BLINK;
      image[row + 1][col + 2] = '*';
      image[row + 1][col + 3] = NORMAL;
    }
    else
    {
      image[row + 1][col + 1] = NORMAL;
      image[row + 1][col + 2] = 0x20;
      image[row + 1][col + 3] = NORMAL;
    }
  }
}
/*-------------------------------------------------------------------------*
 * Open Screen
 *-------------------------------------------------------------------------*/
open_screen()
{
  sd_screen_off();
  sd_clear_screen();
  sd_screen_132();
  sd_screen_off();
  sleep(2);
  v_first_module = coh->co_light_cnt;
  sd_cursor(0, 1, 1);
  sd_text(bf_view);
  sd_screen_on();
  first = 1;
}
/*-------------------------------------------------------------------------*
 * Close Screen
 *-------------------------------------------------------------------------*/
close_screen()
{
  sd_clear_screen();
  sd_screen_off();
  sd_screen_80();
  sd_screen_off();
  sleep(2);
  sd_clear_screen();
  sd_screen_on();
  sd_close();
}
/*-------------------------------------------------------------------------*
 *  Exit Program
 *-------------------------------------------------------------------------*/
leave()
{
  if (sp->sp_sku_support == 'y')
  {
    pmfile_close();
    database_close();
  }
  ss_close();
  co_close();
  close_screen();
  execlp("operm", "operm", 0);
  krash("leave", "diags load", 1);
}

/* end of bf_view.c */
