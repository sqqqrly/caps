/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Record format screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/08/93   | tjt  Added mfc and rewritten.
 *  03/31/94   | tjt  Added rf_hold to hold all or 'k' priority.
 *-------------------------------------------------------------------------*/
static char record_format_srn_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "global_types.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "record_format_srn.t"

extern leave();

#define NUM_PROMPTS  24

typedef struct
{
  short row;
  short col;
  short len;
  char  type;
  short min;
  short max;
  char  *valid;
  char  *source;

} field_item;

struct rf_item x;

field_item field[NUM_PROMPTS] = {
  
  { 7, 36, 1, 'a', 0, 0, 0, &x.rf_rp},
  { 7, 70, 1, 'a', 0, 0, 0, &x.rf_rt},
  { 8, 36, 1, 'a', 0, 0, 0, &x.rf_ft},
  { 8, 70, 1, 'a', 0, 0, 0, &x.rf_eof},
  
  {10, 36, 2, 'n', 0, CustomerNoLength, 0, (char *)&x.rf_con},
  {10, 70, 1, 'n', 2, OrderLength,    0, (char *)&x.rf_on},
  {11, 36, 2, 'n', 0, PicklineMax,    0, (char *)&x.rf_pl},
  {11, 70, 1, 'n', 0, GroupLength,    0, (char *)&x.rf_grp},

  {12, 36, 1, 'n', 0, 1,              0, (char *)&x.rf_pri},
  {12, 70, 2, 'n', 0, SkuLength,      0, (char *)&x.rf_sku},
  {13, 36, 1, 'n', 0, ModuleLength,   0, (char *)&x.rf_mod},
  {13, 70, 1, 'n', 1, QuantityLength, 0, (char *)&x.rf_quan},
  {14, 36, 4, 'n', 0, RemarksLength,  0, (char *)&x.rf_rmks},
  {14, 70, 1, 'n', 0, StklocLength,   0, (char *)&x.rf_stkloc},
  
  {16, 36, 4, 'n', 0, RemarksLength,  0, (char *)&x.rf_box_pos},
  {16, 70, 2, 'n', 0, BoxLength,      0, (char *)&x.rf_box_len},
  {17, 36, 3, 'n', 0, 999,            0, (char *)&x.rf_box_count},
  {17, 70, 3, 'n', 0, PickTextLength, 0, (char *)&x.rf_pick_text},
  
  {19, 36, 1, 'a', 0, 0, "yns", (char *)&x.rf_dup_flag},
  {19, 70, 1, 'a', 0, 0, "yns", (char *)&x.rf_zero_quantity},
  {20, 36, 1, 'a', 0, 0, "hony",(char *)&x.rf_skip_sku},
  {20, 70, 1, 'a', 0, 0, "inky",(char *)&x.rf_hold},
  {21, 36, 1, 'a', 0, 0, "yn",  (char *)&x.rf_ignore_rmks},
  {21, 70, 1, 'a', 0, 0, "yn",  (char *)&x.rf_ignore_pick_text}};
  
short ONE = 1;
struct fld_parms fld1 = {23, 36, 3, 1, &ONE, "Save These Values? (y/n)", 'a'};

char buf[8];
long value;

main()
{
  register long k;
  unsigned char t;
  struct fld_parms fld;

  putenv("_=record_format_srn");
  chdir(getenv("HOME"));

  sd_open(leave);
  sd_echo_flag = 0x20;
  ss_open();
  co_open();
  
  fix(record_format_srn);
  sd_screen_off();
  sd_clear_screen();
  sd_text(record_format_srn);
  sd_screen_on();
  
  memcpy(&x, rf, sizeof(struct rf_item));
  
  for (k = 0; k < NUM_PROMPTS; k++) show_field(k);
  
/*
 *  Only Super Operator May Input Data
 */
  getparms(0);
  
  if (!SUPER_OP)
  {
    eh_post(ERR_SUPER, 0);
    sd_cursor(0, 23, 3);
    sd_text("* * *  Hit Any Key   * * *");
    t = sd_keystroke(NOECHO);
    leave();
  }
  k = 0;
  
  while (1)
  {
    fld.irow   = field[k].row;
    fld.icol   = field[k].col;
    fld.pcol   = 0;
    fld.arrow  = 0;
    fld.length = &field[k].len;
    fld.prompt = 0;
    fld.type   = field[k].type;
    
    get_field(k); 

    t = sd_input(&fld, 0, 0, buf, 0);
  
    put_field(k);
    
    if (t == EXIT) leave();
    
    if (field[k].type == 'a')
    {
      if (field[k].valid)
      {
        if (!memchr(field[k].valid, *buf, strlen(field[k].valid)))
        {
          eh_post(ERR_CODE, buf);
          continue;
        }
      }
    }
    else
    {
      if (value < field[k].min || value > field[k].max)
      {
        eh_post(ERR_CODE, buf);
        continue;
      }
    }
    switch(k)
    {
      case 0:  if (x.rf_rp == 0x20 || !x.rf_rp)
               {
                 eh_post(ERR_PREFACE, 0);
                 x.rf_rp = rf->rf_rp;
                 continue;
               }
               break;
               
      case 1:  if (x.rf_rt == 0x20) x.rf_rt = 0; break;
      
      case 2:  if (x.rf_ft == 0x20) x.rf_ft = 0; break;
      
      case 3:  if (x.rf_eof == 0x20) x.rf_eof = 0; break;
      
      case 13: if (x.rf_box_pos > x.rf_rmks)
               {
                 eh_post(ERR_CODE, buf);
                 continue;
               }
               break;
               
      case 14: if (x.rf_box_len && !x.rf_rmks)
               {
                 eh_post(ERR_CODE, buf);
                 continue;
               }
               break;
      
      case 15: if (x.rf_box_count && !x.rf_box_len)
               {
                 eh_post(ERR_CODE, buf);
                 continue;
               }
               break;

    }
    show_field(k);
    
    if (t == UP_CURSOR) {if (k > 0) k--;}
    else if (t == RETURN) break;
    else 
    {
      k++;
      if (k >= NUM_PROMPTS) k = 0;
    }
  }
  sd_prompt(&fld1, 0);
  memset(buf, 0, 2);
  t = sd_input(&fld1, 0, 0, buf, 0);
  if (t == EXIT) leave();
  *buf = tolower(*buf);
  if (*buf == 'y')
  {
    memcpy(rf, &x, sizeof(struct rf_item));
    system("ss_dump -sp= -rf=sys/rf_text 1>/dev/null 2>/dev/null");
  }
  leave();
}
/*-------------------------------------------------------------------------*
 *  Make Display Field
 *-------------------------------------------------------------------------*/
get_field(k)
{
  if (field[k].type == 'a') 
  {
    buf[0] = *field[k].source;
    buf[1] = 0;
  }
  else sprintf(buf, "%d", *((short *)field[k].source));
}
/*-------------------------------------------------------------------------*
 *  Put A Field
 *-------------------------------------------------------------------------*/
put_field(k)
{
  if (field[k].type == 'a') *field[k].source = *buf;
  else
  {
    value = 0;
    sscanf(buf, "%d", &value);
    *((short *)field[k].source) = value;
  }
}
/*-------------------------------------------------------------------------*
 *  Show A Field
 *-------------------------------------------------------------------------*/
show_field(k)
{
  get_field(k); 
  sd_cursor(0, field[k].row, field[k].col);
  sd_text(buf);
}

leave()
{
  sd_close();
  ss_close();
  co_close();
  execlp("syscomm", "syscomm", 0);
  krash("leave", "syscomm load", 1);
}

/* end of record_format_srn.c */

