/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Total Function Monitor / Simulator.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  02/12/94   |  tjt  Original implementation.
 *  05/22/94   |  tjt  Display revised.
 *  03/16/95   |  tjt  Add PM4 display.
 *  03/17/95   |  tjt  Add ZC2 and PM2 display.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  05/20/96   |  tjt  Add box full module.
 *  06/28/96   |  tjt  Recognize box full switch.
 *  08/23/96   |  tjt  Add begin and commit.
 *  01/07/97   |  tjt  Add PM6.
 *  01/15/96	|  sg   Add full function ZC.
 *  05/26/98   |  tjt  Add IO Module Scanner.
 *-------------------------------------------------------------------------*/
static char ac_view_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "iodefs.h"
#include "ss.h"
#include "co.h"
#include "of.h"
#include "sd.h"
#include "eh_nos.h"
#include "message_types.h"
#include "ac_view.t"
#include "zone_status.h"
#include "oracle_defines.h"

#include "Bard.h"
#include "bard/pmfile.h"

extern leave();
extern void do_view();

#define ROW  24
#define COL  80                           /* screen width (80 or 132)        */
#define MOD  30                           /* modules per screen (30 or 50)   */
#define ML   6                            /* modules per line (6 or 10)      */
#define PB   30                           /* input base (30 or 53)           */

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
long v_max;                               /* number of modules               */
long v_mod;                               /* current module                  */

main(argc, argv)
long argc;
char **argv;
{
  unsigned char work[5];
  register long j, k;

  putenv("_=ac_view");
  const char *homeDir = getenv("HOME");
  chdir(homeDir);

  ss_open();
  co_open();
  oc_open();
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
  register struct hw_item *p;
  register struct bay_item *b;
  register struct zone_item *z;
  register long j;
  char text[16];
  static short ONE = 1;
  static struct fld_parms fld = {19, PB+20, PB, 1, &ONE, "Enter Code", 'a'};
  
  sd_prompt(&fld, 0);
  sd_cursor(0, 19, PB+20);
  c = sd_keystroke(ECHO);
  c = tolower(c);
  
  while (1)
  {
    switch (c)
    {
      case EXIT:                          /* exit function                   */
                        
        if (v_view) v_view = 0;           /* refresh is running              */
        alarm(0);                         /* reset any alarm                 */
        leave();                          /* exit program                    */
                                                                
      case 's':                           /* superpicker message             */
      
        if (sp->sp_total_function != 's') break;
        v_view = 1;
        eh_post(LOCAL_MSG, "Superpicker Started", 0);
        do_view();
        message_put(0, StartSuperpicker, 0, 0);
        break;
      
      case 'm':                           /* stop superpicker                */

        if (sp->sp_total_function != 's') break;
        eh_post(LOCAL_MSG, "Superpicker Stopped", 0);
        v_view = 1;
        do_view();
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

      case 'e':                           /* toggle ext switch               */
      
        if (!v_max) break;
        p = &hw[v_first_module + v_mod];
        if (p->hw_type == PM2 || p->hw_type == PM4 || p->hw_type == PM6 ||
            p->hw_type == ZC  || p->hw_type == ZC2 || p->hw_type == BF) 
        {
          j = bay[p->hw_bay - 1].bay_port - 1;

          sprintf(text, "%04d10%03d1%c",
            p->hw_controller, p->hw_mod_address, j);
          message_put(0, ACSwitchPacketEvent, text, 11);

          sprintf(text, "%04d10%03d0%c",
            p->hw_controller, p->hw_mod_address, j);
          message_put(0, ACSwitchPacketEvent, text, 11);
        }
        do_view();
        break;
                        
      case 'n':                           /* toggle next switch              */
      
        if (!v_max) break;
        p = &hw[v_first_module + v_mod];

        if (p->hw_type == ZC || p->hw_type == ZC2 || p->hw_type == BF)
        {
          j = bay[p->hw_bay - 1].bay_port - 1;

          sprintf(text, "%04d10%03d2%c",
            p->hw_controller, p->hw_mod_address, j);
          message_put(0, ACSwitchPacketEvent, text, 11);

          sprintf(text, "%04d10%03d0%c",
            p->hw_controller, p->hw_mod_address, j);
          message_put(0, ACSwitchPacketEvent, text, 11);
        }
        do_view();
        break;
                        
      case 'x':                           /* toggle short switch             */
      
        if (!v_max) break;
        p = &hw[v_first_module + v_mod];
        if (p->hw_type == PM2 || p->hw_type == PM4 || p->hw_type == PM6)
        {
          j = bay[p->hw_bay - 1].bay_port - 1;

          if (!p->hw_switch)
          {
            sprintf(text, "%04d10%03d1%c",
              p->hw_controller, p->hw_mod_address, j);
            message_put(0, ACSwitchPacketEvent, text, 11);
          }
          else
          {
            sprintf(text, "%04d10%03d0%c",
              p->hw_controller, p->hw_mod_address, j);
            message_put(0, ACSwitchPacketEvent, text, 11);
          }
        }
        p->hw_switch ^= 2;                /* toggle switch                   */

        do_view();
        break;
                        
      case 'y':                           /* toggle both switches            */
      
        if (!v_max) break;
        p = &hw[v_first_module + v_mod];
        if (p->hw_type == ZC ||p->hw_type == ZC2)	/* F011596 */
        {
          j = bay[p->hw_bay - 1].bay_port - 1;

          sprintf(text, "%04d10%03d3%c",
            p->hw_controller, p->hw_mod_address, j);
          message_put(0, ACSwitchPacketEvent, text, 11);

          sprintf(text, "%04d10%03d0%c",
            p->hw_controller, p->hw_mod_address, j);
          message_put(0, ACSwitchPacketEvent, text, 11);
        }
        do_view();
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
          v_row = 4  * (v_mod / ML) + 6;
          v_col = 13 * (v_mod % ML) + 2;
          sd_cursor(0, v_row, v_col);
        }
        break;
                        
      case DOWN_CURSOR:                   /* move down                       */

        if (!v_max) break;
        v_mod = v_mod + ML;
        if (v_mod >= v_max) v_mod = v_max - 1;
        find_module(-1);
        show_title();

        v_row = 4  * (v_mod / ML) + 6;
        v_col = 13 * (v_mod % ML) + 2;
        sd_cursor(0, v_row, v_col);
        break;
                        
      case UP_CURSOR:                     /* move up                         */

        if (!v_max) break;

        if (v_mod >= ML) v_mod -= ML;
        find_module(-1);
        show_title();
                                                        
        v_row = 4  * (v_mod / ML) + 6;
        v_col = 13 * (v_mod % ML) + 2;
        sd_cursor(0, v_row, v_col);
        break;
                        
      case RIGHT_CURSOR:                  /* move right                      */
      case TAB:                           /* tab right too                   */

        if (!v_max) break;

        v_mod = (v_mod + 1) % v_max;
        find_module(1);
        show_title();

        v_row = 4  * (v_mod / ML) + 6;
        v_col = 13 * (v_mod % ML) + 2;
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
            p = &hw[v_first_module - 1];

            if (p->hw_bay)
            {
              b = &bay[p->hw_bay - 1];
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
        if (v_last_module >= coh->co_light_cnt) break;
        v_first_module = v_last_module;
        v_view = 1;
        new_screen();
        break;

      default:
        break;
    }
    sd_cursor(0, v_row, v_col);

    c = sd_keystroke(CURSOR * v_view);
    c = tolower(c);
  }
}
/*-------------------------------------------------------------------------*
 *  Update Current View
 *-------------------------------------------------------------------------*/
void do_view()
{
  register long k, flag, count;
  register struct hw_item *p;
  register unsigned char *d;
  
  if (v_view) v_busy = 1;

  for (k = 0, p = &hw[v_first_module]; k < v_max; k++, p++)
  {
    if (p->hw_type == BL)       d = blv[p->hw_mod - 1].hw_display;
    else if (p->hw_type == ZC)  d = zcv[p->hw_mod - 1].hw_display; /*F011596*/
    else if (p->hw_type == ZC2) d = zcv[p->hw_mod - 1].hw_display;
    else if (p->hw_type == PM2) d = pmv[p->hw_mod - 1].hw_display;
    else if (p->hw_type == PM4) d = pmv[p->hw_mod - 1].hw_display;
    else if (p->hw_type == PM6) d = pmv[p->hw_mod - 1].hw_display;
    else if (p->hw_type == BF)  d = 0;
    else if (p->hw_type == IO)  d = 0;
    else continue;
    
    make_box(k, p->hw_type, p->hw_switch, d, p);
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
  register struct hw_item   *p;
  register struct bay_item  *b;
  register struct zone_item *z;
  register long k, new_zone, new_bay;
  char last_bay[8], last_zone[8], last_pickline[8], last_module[8];
  pmfile_item pm;
  
#ifdef DEBUG
  fprintf(stderr, "new_screen(); %d - %d\n",
  v_first_module, v_last_module);
#endif
  
  if (v_view)
  {
    while (v_busy) sleep(1);
  }
  memset(v_title, 0, sizeof(v_title));
  memset(screen, SPACE, ROW * COL);
  memset(image, SPACE, ROW * COL);
  memcpy(image,  ac_view, COL);
  sd_clear_screen();
  
  new_zone = new_bay = 0;

  for (k = 0, p = &hw[v_first_module]; k < MOD; k++, p++)
  {
    if (v_first_module + k >= coh->co_light_cnt) break;

/*    
    if (v_show_bay && v_first_module + k > v_last_module) break;
*/
    strcpy(last_bay, "?");
    strcpy(last_zone, "?");
    strcpy(last_pickline, "?");
    strcpy(last_module, "?");
    
    if (p->hw_type == BL)       sprintf(last_module, "BL%d", p->hw_mod);
    else if (p->hw_type == ZC)  sprintf(last_module, "ZC%d", p->hw_mod);
    else if (p->hw_type == ZC2) sprintf(last_module, "ZC%d", p->hw_mod);
    else if (p->hw_type == PM2) sprintf(last_module, "PM%d", p->hw_mod);
    else if (p->hw_type == PM4) sprintf(last_module, "PM%d", p->hw_mod);
    else if (p->hw_type == PM6) sprintf(last_module, "PM%d", p->hw_mod);
    else if (p->hw_type == IO)  sprintf(last_module, "IO%d", p->hw_mod);
    
    if (p->hw_bay)
    {
      b = &bay[p->hw_bay - 1];
 
      if (!(b->bay_flags & IsTotalFunction)) break;

      if (!new_bay) new_bay = p->hw_bay;
      if (new_bay != p->hw_bay) break;
      
      sprintf(last_bay, "%d", p->hw_bay);

      if (b->bay_zone)
      {
        z = &zone[b->bay_zone - 1];

        if (!new_zone) new_zone = z->zt_zone;
        if (z->zt_zone != new_zone) break;

        sprintf(last_zone, "%d", z->zt_zone);
        sprintf(last_pickline, "%d", z->zt_pl);
      }
    }
    if ((sp->sp_sku_support == 'y') &&
    (p->hw_type == PM2 || p->hw_type == PM4 || p->hw_type == PM6))
    {
      pm.p_pmodno = p->hw_mod;
      
      begin_work();
      if (!pmfile_read(&pm, NOLOCK))
      {
        sprintf(v_title[k],
        "Pickline: %s  Zone: %s  Bay: %s  Module: PM%d  SKU: %*.*s  Stkloc: %6\
.6s",
        last_pickline, last_zone, last_bay, p->hw_mod,
        rf->rf_sku, rf->rf_sku, pm.p_pmsku, pm.p_stkloc);
          
        commit_work();
        continue;
      }
      commit_work();
    }
    sprintf(v_title[k],
    "Pickline: %s  Zone: %s  Bay: %s  Module: %s",
    last_pickline, last_zone, last_bay, last_module);
  }
  v_last_module = v_first_module + k;
  v_max = k;
  v_mod = 0;
  v_row = 6;
  v_col = 2;

  do_view();
}
/*-------------------------------------------------------------------------*
 *  Find Real Module
 *-------------------------------------------------------------------------*/
find_module(n)
register long n;
{
  while (1)
  {
    if (v_title[v_mod][0]) return 0;
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
  
  memset(image[3], 0x20, COL);
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
  static struct fld_parms fld = {20, PB+20, PB, 1, &FOUR, "Enter Bay", 'n'};
  unsigned char t, buf[4];
  
  if (!first)
  {
    sd_cursor(0, 1, 1);
    sd_clear_rest();
    sd_cursor(0, 1, 1);
    sd_text(ac_view);
  }
  first = 0;
  memset(buf, 0, 4);
  
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
    if (!(b->bay_flags & IsTotalFunction))
    {
      eh_post(LOCAL_MSG, "Not Total Function Bay");
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
  
#ifdef DEBUG
  fprintf(stderr, "setup_first_module(%d)\n", b->bay_number);
#endif
  
  if (b->bay_mod_first)
  {
    v_first_module = mh[b->bay_mod_first - 1].mh_ptr;
  }
  if (b->bay_zc  && b->bay_zc  < v_first_module) v_first_module = b->bay_zc;
  if (b->bay_bl  && b->bay_bl  < v_first_module) v_first_module = b->bay_bl;
  v_first_module--;

  if (b->bay_mod_last)
  {
    v_last_module = mh[b->bay_mod_last - 1].mh_ptr;
  }
  if (b->bay_zc  && b->bay_zc  > v_last_module) v_last_module = b->bay_zc;
  if (b->bay_bl  && b->bay_bl  > v_last_module) v_last_module = b->bay_bl;
  v_last_module--;
  
#ifdef DEBUG
  fprintf(stderr, "v_first_module=%d  v_last_module=%d\n",
  v_first_module, v_last_module);
#endif
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
  static struct fld_parms fld = {20, PB+20, PB, 1, &THREE, "Enter Zone", 'n'};
  unsigned char t, buf[4];
  
  if (!first)
  {
    sd_cursor(0, 1, 1);
    sd_clear_rest();
    sd_cursor(0, 1, 1);
    sd_text(ac_view);
  }
  first = 0;
  memset(buf, 0, 4);
  
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
    if (!(z->zt_flags & IsTotalFunction))
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
make_box(n, type, switches, display, p)
register long n;
register long type;
register long switches;
register unsigned char *display;
register struct hw_item *p;					/* F052898 */
{
  unsigned char work[16];
  register long row, col, j, k, width;
  register struct bay_item *b;				/* F052898 */
  register struct zone_item *z;
  
#ifdef DEBUG
  fprintf(stderr, "make_box(n=%d type=%d swt=%d)\n",
  n, type, switches);
  if (type == ZC2) Bdump(display,16);
  else if (type == ZC) Bdump(display, 5);
#endif

  if (n < 0 || n >= MOD) return 0;

  row =  4 * (n / ML) + 4;
  col = 13 * (n % ML);
        
  if (type == BL)       width = 5;
  else if (type == ZC)  width = 9;
  else if (type == IO)	width = 12;
  else if (type == ZC2) width = 13;
  else if (type == PM2) width = 9;
  else if (type == PM4) width = 11;
  else if (type == PM6) width = 10;
  else if (type == BF)  width = 12;
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
    memcpy(&image[row + 1][col + 2], display, 5);
    if (switches & 1) image[row + 2][col + 1] = 'X';
    if (switches & 2) image[row + 2][col + 7] = 'X';
  }
  else if (type == ZC2)
  {
    memcpy(&image[row + 1][col + 2], display, 10);
    memcpy(&image[row + 2][col + 4], display + 10, 6);

    if (switches & 1) image[row + 2][col + 1] = 'X';
    if (switches & 2) image[row + 2][col + 11] = 'X';
  }
  else if (type == PM2)
  {
    image[row + 1][col + 2] = display[2];
    image[row + 1][col + 3] = display[3];

    if (switches & 1) display[0] &= 0x7f;

    if (display[0] & 0x80)
    {
      image[row + 1][col + 5] = BLINK;
      image[row + 1][col + 6] = '*';
      image[row + 1][col + 7] = NORMAL;
    }
    else
    {
      image[row + 1][col + 5] = NORMAL;
      image[row + 1][col + 6] = 0x20;
      image[row + 1][col + 7] = NORMAL;
    }
    if (switches & 1) image[row + 2][col + 7] = 'X';
  }
  else if (type == PM4)
  {
    image[row + 1][col + 2] = display[0] & 0x7f;
    image[row + 1][col + 3] = display[1];
    image[row + 1][col + 4] = display[2];
    image[row + 1][col + 5] = display[3];

    if (switches & 1) display[0] &= 0x7f;

    if (display[0] & 0x80)
    {
      image[row + 1][col + 7] = BLINK;
      image[row + 1][col + 8] = '*';
      image[row + 1][col + 9] = NORMAL;
    }
    else
    {
      image[row + 1][col + 7] = NORMAL;
      image[row + 1][col + 8] = 0x20;
      image[row + 1][col + 9] = NORMAL;
    }
    if (switches & 1) image[row + 2][col + 9] = 'X';
  }
  else if (type == PM6)
  {
    image[row + 1][col + 2] = display[0] & 0x7f;
    image[row + 1][col + 3] = display[1];
    image[row + 1][col + 4] = display[2];
    image[row + 1][col + 5] = display[3];
    image[row + 1][col + 6] = display[4];
    image[row + 1][col + 7] = display[5];

    if (switches & 1) display[0] &= 0x7f;

    if (switches & 1) image[row + 2][col + 8] = 'X';
  }
  else if (type == BF)
  {
    memcpy(&image[row + 1][col + 2], "BOX FULL", 8);

    if (switches & 2) image[row + 2][col + 10] = 'X';
  }
  else if (type == IO)
  {
    memcpy(&image[row + 1][col + 2], "SCANNER ", 8);
    
    if (p->hw_bay && p->hw_state)
    {
      b = &bay[p->hw_bay - 1];
      if (b->bay_zone)
      {
        z = &zone[b->bay_zone - 1];
        
        if (z->zt_status == ZS_UNDERWAY || z->zt_status == ZS_EARLY)
        {
          if (!(oc->oi_tab[z->zt_order - 1].oi_flags & NEED_BOX))
          {
            memcpy(&image[row + 1][col + 2],
          					oc->oi_tab[z->zt_order - 1].oi_con, 8);
          }
        }
      }
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
  if (COL == 132) sd_screen_132();
  sd_screen_off();
  if (COL == 132) sleep(2);
  v_first_module = coh->co_light_cnt;
  sd_cursor(0, 1, 1);
  sd_text(ac_view);
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
  if (COL == 132) sd_screen_80();
  sd_screen_off();
  if (COL == 132) sleep(2);
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
  oc_close();
  close_screen();
  execlp("operm", "operm", 0);
  krash("leave", "diags load", 1);
}

/* end of ac_view.c */
