/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Transaction format screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/8/93    | tjt  Added mfc and rewritten.
 *  06/15/94   | tjt  Added order input event.
 *  06/15/94   | tjt  Fix screen sequence reordered.
 *  12/23/94   | tjt  Add lot split/end transaction.
 *-------------------------------------------------------------------------*/
static char transac_format_srn_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "iodefs.h"
#include "global_types.h"
#include "getparms.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "transac_format_srn.t"

extern leave();

#define NUM_PROMPTS  14

typedef struct
{
  short row;
  short col;
  char  *valid;
  char  *source;

} field_item;

struct sp_item x;

field_item field[NUM_PROMPTS] = {
  
  { 6, 56, "ynbq", &x.sp_to_flag},
  { 8, 56, "yn",  &x.sp_to_complete},
  { 9, 56, "yn",  &x.sp_to_cancel},
  {10, 56, "yn",  &x.sp_to_underway},
  {11, 56, "yn",  &x.sp_to_repick},
  {12, 56, "yn",  &x.sp_to_manual},
  {13, 56, "yn",  &x.sp_to_order_queued},
  {14, 56, "yn",  &x.sp_to_short},
  {15, 56, "yn",  &x.sp_to_pick_event},
  {16, 56, "yn",  &x.sp_to_orphan},
  {17, 56, "yn",  &x.sp_to_restock},
  {18, 56, "yn",  &x.sp_to_orders_done},
  {19, 56, "yn",  &x.sp_to_lot_split},
  {20, 56, "yn",  &x.sp_to_box_close}};
  
short ONE = 1;
struct fld_parms fld1 = {22, 56, 20, 1, &ONE, "Save These Values? (y/n)", 'a'};

char buf[8];
long value;

main()
{
  register long k;
  unsigned char t;
  struct fld_parms fld;

  putenv("_=transac_format_srn");
  chdir(getenv("HOME"));

  sd_open(leave);
  sd_echo_flag = 0x20;
  ss_open();
  co_open();
  
  fix(transac_format_srn);
  sd_screen_off();
  sd_clear_screen();
  sd_text(transac_format_srn);
  sd_screen_on();
  
  memcpy(&x, sp, sizeof(struct sp_item));
  
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
    fld.length = &ONE;
    fld.prompt = 0;
    fld.type   = 'a';
    
    get_field(k); 

    t = sd_input(&fld, 0, 0, buf, 0);
  
    put_field(k);
    
    if (t == EXIT) leave();
    
    if (!memchr(field[k].valid, *buf, strlen(field[k].valid)))
    {
       eh_post(ERR_CODE, buf);
       continue;
    }
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
    memcpy(sp, &x, sizeof(struct sp_item));
    system("ss_dump -sp=sys/sp_save -rf= 1>/dev/null 2>/dev/null");
  }
  leave();
}
/*-------------------------------------------------------------------------*
 *  Make Display Field
 *-------------------------------------------------------------------------*/
get_field(k)
{
  *buf = *field[k].source;
  buf[1] = 0;
}
/*-------------------------------------------------------------------------*
 *  Put A Field
 *-------------------------------------------------------------------------*/
put_field(k)
{
  *field[k].source = *buf;
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

/* end of transac_format_srn.c */

