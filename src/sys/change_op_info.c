/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Change operator information screen.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/02/93   |  tjt  Added to mfc.
 *  04/18/97   |  tjt  Add language.h and code_to_caps
 *-------------------------------------------------------------------------*/
static char change_op_info_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include "file_names.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "language.h"
#include "change_op_info.t"
#include "Bard.h"
#include "bard/operator.h"

extern leave();

FILE *fd;                                 /* features file                   */

short L1  = 8;
short L2  = 32;
short L3  = 8;
short L4  = 2;
short L5  = 1;
short L6  = 26;
short L7  = 26;
short L8  = 26;
short L9  = 26;
short L10 = 26;
short L11 = 26;
short L12 = 26;

char level[2];
char code[2];
char name[9];

#define NUM 13

struct fld_parms fld[] ={
  { 7,35,1,0, &L1,  "Operator Name",'a'},
  { 9,35,1,0, &L1,  "New Operator Name", 'a'},
  {10,35,1,0, &L2,  "Location",'a'},
  {11,35,1,0, &L3,  "Printer",'a'},
  {12,35,1,0, &L4,  "Pickline",'n'},
  {13,35,1,0, &L5,  "Level",'a'},
  {15,49,1,0, &L6,  "Main Menu       (1.0)",'a'},
  {16,49,1,0, &L7,  "Operations Menu (2.0)",'a'},
  {17,49,1,0, &L8,  "System Commands (3.0)",'a'},
  {18,49,1,0, &L9,  "Configuration   (4.0)",'a'},
  {19,49,1,0, &L10, "Productivity    (5.0)",'a'},
  {20,49,1,0, &L11, "Product File    (7.0)",'a'},
  {21,49,1,0, &L12, "Labels          (8.0)",'a'},
};
operator_item op, opx;                    /* operator record                 */

unsigned char *buf[] = {name, op.o_op_name, op.o_op_desc,
  op.o_op_printer, op.o_op_pickline, level, op.o_op_mm,
  op.o_op_ops, op.o_op_sys, op.o_op_config, op.o_op_prod, 
  op.o_op_sku, op.o_op_label};

struct fld_parms fldyn ={22,35,15,1,&L5, "Change? (y/n)", 'a'};
char yn0[2];

char features[8][32];

main()
{
  register long i, k, len;
  unsigned char t;
  
  putenv("_=change_op_info");        /* program name                    */
  chdir(getenv("HOME"));

  database_open();

  sd_open(leave);
  ss_open();

  operator_open(AUTOLOCK);
  operator_setkey(1);
  
  fix(change_op_info);
  sd_clear_screen();
  sd_screen_off();
  sd_text(change_op_info);
  sd_screen_on();
/*
  sd_echo_flag = 0x20;
*/
  fd = fopen(fl_table_name, "r");
  
  if (fd == 0) krash("main", "fl_table open", 1);
  
  for (k = 0; k < 8; k++)
  { 
    memset(features[k], 0, 32);
    fgets(features[k], 32, fd);
    len = strlen(features[k]) - 1;
    features[k][len] = 0;
  }
  fclose(fd);

/*
 *  Get all input
 */
  i = 0;

  while (1)
  {
    t = sd_input(&fld[i], 0, 0, buf[i], 0);
    if (t ==  EXIT) leave();
  
    strip_space(buf[i], *fld[i].length + 1);

/*
 *  Process Field Input
 */
    switch (i)                            /* case on each field              */
    {
      case 0:                             /* operator name                   */
      
        memcpy(op.o_op_name, name, sizeof(op.o_op_name));

        if (operator_read(&op, NOLOCK))
        {
          eh_post(ERR_OP_NAME, name);
          continue;
        }
        display();                        /* show data for operator          */
        t = TAB;                          /* force to next field             */
        break;
    
      case 1: break;                      /* name change                     */

      case 2: break;                      /* operator location               */
      
      case 3: break;                      /* printer - no edit               */
      
      case 4: break;                      /* pickline - no edit              */
      
      case 5:                             /* operator level                  */
      
        *level = toupper(*level);
        
        if (*level != 'S' && *level != 'P' && *level != 'D')
        {
          eh_post(ERR_CODE, level);
          display();
          continue;
        }
        if (level[0] != op.o_op_level[0])
        {
          memcpy(op.o_op_mm,     features[0], 32);
          memcpy(op.o_op_ops,    features[1], 32);
          memcpy(op.o_op_sys,    features[2], 32);
          memcpy(op.o_op_config, features[3], 32);
          memcpy(op.o_op_prod,   features[4], 32);
          memcpy(op.o_op_sku,    features[6], 32);
          memcpy(op.o_op_label,  features[7], 32);
          
          if (*level == 'P')
          {
            memset(op.o_op_mm, 0, 32);
            memset(op.o_op_sys, 0, 32);
            strcpy(op.o_op_mm, "CELGLOP");
          }
          else if (*level == 'D')
          {
            memset(op.o_op_mm, 0, 32);
            memset(op.o_op_ops, 0, 32);
            memset(op.o_op_sys, 0, 32);
            memset(op.o_op_config, 0, 32);
            memset(op.o_op_prod, 0, 32);
            memset(op.o_op_label, 0, 32);
            strcpy(op.o_op_mm, "FL");
          }
        }
        op.o_op_level[0] = level[0];
        display();
        break;
        
      case 6:                              /* main menu features           */
        
        cp(op.o_op_mm, features[0]);

        if (*code)
        {
          eh_post(ERR_PROFILE, code);
          continue;
        }
        break;
      
      case 7:                              /* ops menu features              */
        
        cp(op.o_op_ops, features[1]);

        if (*code)
        {
          eh_post(ERR_PROFILE, code);
          continue;
        }
        break;
      
      case 8:                              /* sys menu features              */
        
        cp(op.o_op_sys, features[2]);

        if (*code)
        {
          eh_post(ERR_PROFILE, code);
          continue;
        }
        break;
      
      case 9:                              /* config menu features         */
        
        cp(op.o_op_config, features[3]);

        if (*code)
        {
          eh_post(ERR_PROFILE, code);
          continue;
        }
        break;
      
      case 10:                             /* prod menu features           */
        
        cp(op.o_op_prod, features[4]);

        if (*code)
        {
          eh_post(ERR_PROFILE, code);
          continue;
        }
        break;
      
      case 11:                              /* sku menu features           */
        
        cp(op.o_op_sku, features[6]);

        if (*code)
        {
          eh_post(ERR_PROFILE, code);
          continue;
        }
        break;
      
      case 12:                             /* label menu features            */
        
        cp(op.o_op_label, features[7]);

        if (*code)
        {
          eh_post(ERR_PROFILE, code);
          continue;
        }
        break;
      
    }
    if (t == UP_CURSOR)
    {
      if (i > 1) i--;
      continue;
    }
    if (t == RETURN)
    {
      sd_prompt(&fldyn, 0);
      memset(yn0, 0, 2);
      
      while (1)
      {
        t = sd_input(&fldyn, 0, 0, yn0, 0);
        if (t == EXIT) leave();
        if (code_to_caps(*yn0) == 'y') break;
        if (code_to_caps(*yn0) == 'n') break;
        eh_post(ERR_YN, 0);
      }
      strip_space(op.o_op_name, 9); 
      strip_space(op.o_op_desc, 32);
      strip_space(op.o_op_printer, 9);
      strip_space(op.o_op_pickline, 3);
      strip_space(op.o_op_mm, 32);
      strip_space(op.o_op_ops, 32);
      strip_space(op.o_op_sys, 32);
      strip_space(op.o_op_config, 32);
      strip_space(op.o_op_prod, 32);
      strip_space(op.o_op_sku, 32);
      strip_space(op.o_op_label, 32);

      if (code_to_caps(*yn0) == 'y')
      {
        if (strncmp(name, op.o_op_name, 8) != 0)
        {
          if (change_logon(name, op.o_op_name) < 0)
          {
            eh_post(ERR_OP_NAME, op.o_op_name);
            i = 1;
            continue;
          }
        }
        memcpy(opx.o_op_name, name, sizeof(op.o_op_name));

        begin_work();
        if (!operator_read(&opx, LOCK)) operator_update(&op);
        commit_work();
      }
      fix(change_op_info);                /* start again                     */
      sd_clear_screen();
      sd_text(change_op_info);
      memset(&op, 0, sizeof(operator_item));
      memset(name, 0, sizeof(name));
      i = 0;
      continue;
    }
    i++;
    if (i >= NUM) i = 2;
  }                                       /* end of while(1)                 */
}                                         /* end of main                     */
/*
 *  Display Data for an Operator
 */
display()
{
  register long k;
  
  level[0] = op.o_op_level[0];
  
  for (k = 1; k < NUM; k++)
  {
    sd_cursor(0, fld[k].irow, fld[k].icol);
    sd_clear_line();
    sd_cursor(0, fld[k].irow, fld[k].icol);
    sd_text(buf[k]);
  }
  return;
}
/* 
 *  Check Valid Profile
 */
cp(x, y)
register char *x;                         /* profile                         */
register char *y;                         /* legal codes                     */
{
  char work[32];
  register long len;
  register char *p, *q;
  
  memcpy(work, x, 32);                    /* copy new profile                */
  memset(x, 0, 32);                       /* clear new                       */
  len = strlen(y);                        /* length of legit                 */
  
  p = work; q = x;
  memset(code, 0, 2);
  
  while (*p)
  {
    *p = toupper(*p);

    if (len)
    {
      if (!memchr(y, *p, len)) *code = *p++;
      else *q++ = *p++;
    }
    else *code = *p++;
  }
  display();
  return 0;
}
leave()
{
  operator_close();
  ss_close();
  sd_close();
  database_close();
  execlp("syscomm", "syscomm", 0);
  krash ("leave", "syscomm load", 1);
}

/* end of change_op_info.c */
