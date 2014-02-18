/*----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Manual order input.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/12/93   |  tjt  Added to mfc.
 *  05/24/94   |  tjt  Fix transaction output flag.
 *  12/23/94   |  tjt  Add null lot to transaction.
 *  08/03/95   |  tjt  Add allow priority 'k'.
 *  08/03/95   |  tjt  Add allow pickline 0.
 *-------------------------------------------------------------------------*/
static char ordr_entry_c[] = "%Z% %M% %I% (%G% - %U%)";

/****************************************************************************/
/*                                                                          */
/*                             Order_Entry Screen                           */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "getparms.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "of.h"
#include "order_entry.t"
#include "eh_nos.h"
#include "xt.h"

extern leave();

#define NUM_PROMPTS     18
#define BUF_SIZE        51
#define row_nums        18
#define top_rows        5
#define BUF1_SIZE       17
#define MAX_ITEMS       oc->of_max_picks

FILE *fd = 0;
char fd_name[16];

short rmks_len[8] = {0};                  /* remarks field length            */
struct text_spec_item *rmks = 0;          /* remarks specification           */
long  rmks_count = 0;                     /* number of remarks               */
long  rmks_remaining = 0;
short pt_len[2] = {0};                    /* pick text length                */
long  pt_count = 0;                       /* number of pick text             */
long  pt_remaining = 0;
long  pick_size;                          /* size of pick prompts            */

short zero = 0;                           /* no length                       */
short LCON = 15;
short LON  = 0;
short LPRI = 1;
short LGRP = 5;
short LPL  = 2;
short LSKU = 15;
short LMOD = 5;
short LQUAN = 5;

struct fld_parms fld[] ={
  {6,20,1,1,&LCON,"Customer No.",'a'},
  {6,20,1,1,&LON, "Order Number",'n'},
  {6,20,1,1,&LPRI,"Priority",'a'},
  {6,20,1,1,&LGRP,"Group",'a'},
  {6,20,1,1,&LPL, "Pickline",'n'},
  {6,20,1,1,&rmks_len[0],"Remarks",'a'},
  {6,20,1,1,&rmks_len[1],"Remarks",'a'},
  {6,20,1,1,&rmks_len[2],"Remarks",'a'},
  {6,20,1,1,&rmks_len[3],"Remarks",'a'},
  {6,20,1,1,&rmks_len[4],"Remarks",'a'},
  {6,20,1,1,&rmks_len[5],"Remarks",'a'},
  {6,20,1,1,&rmks_len[6],"Remarks",'a'},
  {6,20,1,1,&rmks_len[7],"Remarks",'a'},
  {6,20,1,1,&LSKU,"Sku",'a'},
  {6,20,1,1,&LMOD,"Module Number",'n'},
  {6,20,1,1,&LQUAN,"Quantity",'n'},
  {6,20,1,1,&pt_len[0],"Pick Text",'a'},
  {6,20,1,1,&pt_len[1],"Pick Text",'a'}
};
char buf[NUM_PROMPTS][BUF_SIZE];          /* array of buffers                */
struct buf1_item
{
  char buf1[4][BUF1_SIZE];
};
struct buf1_item *pick;                   /* point to picks                  */
long pick_tab_size = 0;

long have_data = 0; 

main()
{
  short rm = 0;
  short ret,mi;
  short si,sr;
  unsigned char t;
  short i, j, k, nums, num;
  short n, done = 0;
  short FIRSTTIME = 1;
  short last_page, cur_page, new_page, last_row, col, last_item;
  register char *q;

  putenv("_=order_entry");
  chdir(getenv("HOME"));

  open_all();

/*
 * allocate pick table
 */
  pick_tab_size = oc->of_max_picks * sizeof(struct buf1_item);
  pick = (struct buf1_item *)malloc(pick_tab_size);

  LCON  = rf->rf_con;
  if (sp->sp_use_con != 'y') LON = rf->rf_on;
  LPRI  = rf->rf_pri;
  LGRP  = rf->rf_grp;
  LPL   = rf->rf_pl;
  if (rf->rf_sku)
  {
    LSKU = rf->rf_sku;
    LMOD = 0;
  }
  else
  {
    LMOD = rf->rf_mod;
    LSKU = 0;
  }
  LQUAN = rf->rf_quan;

  fix(order_entry);
  sd_screen_off();
  sd_clear_screen();                        /* clear screen                  */
  sd_text(order_entry);
  sd_screen_on();

/*
 *  setup remarks specification (400 bytes on screen)
 */
  if (rf->rf_rmks)
  {
    rmks_remaining = rf->rf_rmks;
    
    for (rmks_count = 0; rmks_count < 8; rmks_count++)
    {
      if (rmks_remaining <= 0) break;
      
      if (rmks_remaining > 50) rmks_len[rmks_count] = 50;
      else rmks_len[rmks_count] = rmks_remaining;
      
      rmks_remaining -= rmks_len[rmks_count];
    }
  }
  
/*
 *  setup pick text
 */
  pick_size = 2;

  if (rf->rf_pick_text)
  {
    pt_remaining = rf->rf_pick_text;
    
    for (pt_count = 0; pt_count < 2; pt_count++)
    {
      if (pt_remaining <= 0) break;
      
      if (pt_remaining > 50) pt_len[pt_count] = 50;
      else pt_len[pt_count] = pt_remaining;
      
      pt_remaining -= pt_len[pt_count];
      pick_size++;
    }
    
  }
   /* clear input buffers */

  memset(buf, 0, sizeof(buf));

  memset(pick, 0, pick_tab_size);

  if (!SUPER_OP && !DATA_OP) *fld[4].length = 0;

/* main loop to gather input */

  while(1)
  {
    sd_cursor(0,(top_rows + 1),1);
    sd_clear_rest();

    rm = 0;
    for (i = 0; i < 13; i++)
    {
      if (*fld[i].length)
      {
        sd_prompt(&fld[i],rm);
        if (i < 2 || i > 4)
        {
          for (j = 0;j < BUF_SIZE;j++) buf[i][j] = 0;
        }
        if (! FIRSTTIME && (i >= 2 && i <= 4))
        {
          sd_cursor(0,(top_rows + rm + 1), 20);
          sd_text(buf[i]);
        }
        rm++;
      }
    }
    FIRSTTIME = 0;
    sr = rm;
    rm = 0;
    i = 0;
    done = 0;
       
    while(1)
    {
      if (*fld[i].length)
      {
        t = sd_input(&fld[i],rm,&rm,buf[i],0);
         
        if (t == EXIT) 
        {
          process_orders();
          leave();
        }
        else if (t == UP_CURSOR && rm > 0)
        {
          i--;
          while(! *fld[i].length) i--;
          rm--;
        }
        else if (t == DOWN_CURSOR || t == TAB)
        {
          if(rm < sr - 1)
          {
            i++;
            while(! *fld[i].length) i++;
            rm++;
          }
          else
          {
            i = 0;
            rm = 0;
          }
        }
        else if(t == RETURN)
        {
          if(*fld[2].length)              /* check priority                  */
          {
            *buf[2] = tolower(*buf[2]);
            if(*buf[2] != 'h' && *buf[2] != 'm' && 
               *buf[2] != 'l' && *buf[2] != 'k')
            {
              eh_post(ERR_OF_PRI,0);
              i = 0;
              rm = 0;
              continue;
            }
          }
          if (*fld[4].length)              /*check pickline                  */
          {                          
            if (*buf[4] == 0) memset(buf[4], '0', *fld[4].length);
          }
          break;
        }
      }
      else if (i < 12) i++;
      else i = 0;
    }

/* gather the input for different sku's or pick modules */

    sr = sr - rmks_count;

    nums = (row_nums - sr) / (pick_size);

    last_row = top_rows + 1 + sr + nums * pick_size;

    rm = sr;
    new_page = 1;
    last_page = 0;
    cur_page = 0;
    n = 0;
    last_item = -1;

    while(1)
    {
      if(new_page)
      {
        sd_cursor(0,(sr + top_rows + 1),1);
        sd_clear_rest();                      /* clear the bottom of screen  */
        rm = sr;
        new_page = 0;
        last_page++;
        cur_page++;
      }
      while(1)
      {
        i = rm;
        for (j = 13; j < 18; j++)
        {
          if (*fld[j].length)
          {
            sd_prompt(&fld[j], i);
            i++;
          }
        }
        last_item++;

        while(1)
        {
          j = 13;
          t = DOWN_CURSOR;
          while (j >= 13 && j < 18)
          {
            if (j < 15) i = 0;
            else i = j - 14;

            if (*fld[j].length)
            {
              t = sd_input(&fld[j],rm,&rm,pick[n].buf1[i],0);
              if (t == RETURN)
              {
                do
                {
                  if (*fld[j].length) rm++;
                  j++;
                }
                while (j < 18);
                break;
              }
              if (t == EXIT) break;
            }
            else                          /* no field                        */
            {
              if (t == UP_CURSOR) j--;
              else j++;
              continue;
            }

            if (t == UP_CURSOR)           /* field                           */
            {
              if (j > 13) {rm--; j--;}
              else break;
            }
            else {rm++; j++;}
          }

          if(t == EXIT) 
          {
            process_orders();
            leave();
          }
          else if(t == RETURN)
          {
            i = 0;
            while(i <= last_item)
            {
              if(pick[i].buf1[1][0] && !pick[i].buf1[0][0])
              {
                eh_post(ERR_PM_MISSING,0);
                if (rm >= sr) rm -= pick_size;
                break;
              }
              else if(!pick[i].buf1[1][0] && pick[i].buf1[0][0])
              {
                eh_post(ERR_QUAN_MISSING,0);
                if (rm >= sr) rm -= pick_size;
                break;
              }
              i++;
            }
            if(i <= last_item)
            continue;
            done = 1;
            break;
          }
          else if(t == UP_CURSOR)
          {
            if(rm > sr)
            {
              rm -= pick_size;
              n--;
            }
            else if(n > 0)
            {
              show_text(n - nums, sr, rm, last_item, nums);
              printf("first show test");
              rm = sr;
              n = n - nums;
              cur_page--;
            }
          }

          else if(t == DOWN_CURSOR || t == TAB)
          {
            if(!pick[n].buf1[0][0])
            {
              eh_post(ERR_PM_MISSING,0);
              rm -= pick_size;
            }
            else if(!pick[n].buf1[1][0])
            {
              eh_post(ERR_QUAN_MISSING,0);
              rm -= pick_size;
            }
            else if(n == MAX_ITEMS)
            {
              done = 1;
              break;
            }
            else if((rm + top_rows + 1) < last_row )
            {
              n++;
              if((n-1) == last_item) break;
            }
            else if(cur_page < last_page)
            {
              printf("second show text");
              show_text((n+1), sr, rm, last_item, nums);
              printf("second show text");

              rm = sr;
              n++;
              cur_page++;
            }
            else if(cur_page == last_page)
            {
              new_page = 1;
              n++;
              break;
            }
          }
        }
        if(done) break;
        if(new_page) break;
      }
      if(done) break;
    }
    send_data(last_item + 1);

    if (sp->sp_to_flag != 'n' && sp->sp_to_manual == 'y')    /* F052494      */
    {
      write_transaction(last_item + 1);
    }
    memset(pick, 0, pick_tab_size);
  }
}

/* function to display the previous entered data*/

show_text(x,sr,rm,last_item,nums)
short x;                                  /* first pick to show              */
short sr;                                 /* first line of screnn            */
short rm;                                 /* current rm                      */
short last_item;                          /* last pick                       */
short nums;                               /* picks per screen                */
{
  short j = x;
  short rm1 = sr;
  short k = sr + top_rows + 1;

  sd_cursor(0, k, 1);
  sd_clear_rest();

  while(j < (nums + x) && j <= last_item)
  {
    if (LSKU) sd_prompt(&fld[13],rm1++);  /* sku                             */
    else sd_prompt(&fld[14],rm1++);       /* module                          */

    sd_prompt(&fld[15],rm1++);            /* quan                            */

    if (*fld[16].length) sd_prompt(&fld[16], rm1++);/* pick text 1           */
    if (*fld[17].length) sd_prompt(&fld[17], rm1++);/* pick text 2           */

    sd_cursor(0,k++,20);                  /* sku/mod value                   */
    sd_text(pick[j].buf1[0]);

    sd_cursor(0,k++,20);                  /* quan value                      */
    sd_text(pick[j].buf1[1]);

    if(*fld[16].length)
    {
      sd_cursor(0,k++,20);                /* pick text                       */
      sd_text(pick[j].buf1[2]);
    }

    if(*fld[17].length)
    {
      sd_cursor(0,k++,20);                /* pick text                       */
      sd_text(pick[j].buf1[3]);
    }
    j++;                                  /* next pick                       */
  }
}
/*
 * send the data to the order input
 */
send_data(last_item)
short last_item;
{
  short i, j, k;

  have_data = 1;
  
  fprintf(fd, "%c", rf->rf_rp);           /* record preface                  */

  for(j = 0; j < 5; j++)
  {
    if(*fld[j].length)                    /* output header fields            */
    {
      if(fld[j].type == 'a')
      {
        fprintf(fd, "%s", buf[j]);
        space_fill(*fld[j].length - strlen(buf[j]));
      }
      else
      {
        zero_fill(*fld[j].length - strlen(buf[j]));
        fprintf(fd, "%s", buf[j]);
      }
    }
  }
  for (j = 0; j < rmks_count; j++)
  {
    fprintf(fd, "%s", buf[j + 5]);
    space_fill(rmks_len[j] - strlen(buf[j + 5]));
  }
  space_fill(rmks_remaining);

  for(i = 0; i < last_item; i++)
  {
    if(pick[i].buf1[0][0] == 0 && pick[i].buf1[1][0] == 0)
    continue;
         
    if (LSKU)                             /* sku output                      */
    {
      fprintf(fd, "%s", pick[i].buf1[0]);
      space_fill(rf->rf_sku - strlen(pick[i].buf1[0]));
    }
    else
    {
      zero_fill(rf->rf_mod - strlen(pick[i].buf1[0]));
      fprintf(fd, "%s", pick[i].buf1[0]);
    }
    zero_fill(rf->rf_quan - strlen(pick[i].buf1[1]));
    fprintf(fd, "%s", pick[i].buf1[1]);

    for (j = 0; j < pt_count; j++)        /* pick text                       */
    {
      fprintf(fd, "%s", pick[i].buf1[j + 2]);
      space_fill(pt_len[j] - strlen(pick[i].buf1[j + 2]));
   }
   space_fill(pt_remaining);
  }
  if (rf->rf_rt != SPACE) fprintf(fd, "%c", rf->rf_rt);

  return;
}
/*
 * function to zerofill the string
 */
zero_fill(n)
long n;
{
  while (n > 0)
  {
    fprintf(fd, "0");
    n--;
  }
  return;
}
space_fill(n)
long n;
{
  while (n > 0)
  {
    fprintf(fd, " ");
    n--;
  }
  return;
}
/*
 * write the order to the transaction file
 */
write_transaction(last_item)
short last_item;
{
  register long k;

  xt_open();

  for(k = 0; k < last_item; k++)
  {
    if (rf->rf_sku)
    {
      xt_build(buf[0],buf[3], atol(buf[1]), atol(buf[4]), 'A', 
        pick[k].buf1[0], 0, 0, atol(pick[k].buf1[1]), 0, 0, 0);
    } 
    else
    {
      xt_build(buf[0], buf[3], atol(buf[1]), atol(buf[4]), 'A',
         0, atol(pick[k].buf1[0]), 0, atol(pick[k].buf1[1]), 0, 0, 0);
    }
  }
  xt_close();

  return;
}
process_orders()
{
  long status;
  
  if (!have_data) leave();
  have_data = 0;
  
  if(rf->rf_eof == SPACE)
  {
    if(rf->rf_rt == SPACE) fprintf(fd,"%c",rf->rf_rp);

    else  fprintf(fd,"%c%c",rf->rf_rp,rf->rf_rt);
  }
  else
  {
    fprintf(fd,"%c",rf->rf_eof);
  }
  fclose(fd);
  fd = 0;
  
  if (fork() == 0)
  {
    close_all();
    execlp("order_input", "order_input", fd_name, 0);
    eh_post(CRASH_MSG, "Program order_input Not Found");
    exit(1);
  }
  sd_wait();
  wait(&status);

  if (status) eh_post(LOCAL_MSG, "Orders Have Errors");
  else eh_post(ERR_CONFIRM, "Order input");

  sd_cursor(0, 21, 26);
  sd_text("* * *   Hit Any Key   * * *");
  sd_keystroke(NOECHO);
}
open_all()
{
  sd_open(leave);
  tmp_name(fd_name);
  fd = fopen(fd_name, "w");
  if (fd == 0) krash("main", "open tmp", 1);
  ss_open();
  oc_open();
  getparms(0);
}
close_all()
{
  if (fd) fclose(fd);
  oc_close();
  ss_close();
}

/*
 * function to transfer control back to the calling program
 */
leave()
{
  char command[80];

  close_all();

  sprintf(command, "cp %s tmp/last_order", fd_name);
  system(command);
  
  unlink(fd_name);
  sd_close();
  execlp("mmenu", "mmenu", 0);
  krash("leave", "mmemu load", 1);
}

/* end of order_entry.c */
