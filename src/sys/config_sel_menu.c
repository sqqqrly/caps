/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Configuration Selection Menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/03/96   |  tjt Original implementatin.
 *-------------------------------------------------------------------------*/
static char config_sel_menu_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "iodefs.h"
#include "file_names.h"
#include "ss.h"
#include "sd.h"
#include "eh_nos.h"
#include "config_sel_menu.t"

extern leave();
double atof();

FILE *fd;
char fd_name[] = "sys/config_list";

#define NAMES 12
#define NSIZE 32
#define NROW  16

char names[NAMES][NSIZE] = {0};
long nmax = 0;

#define NUM_PROMPTS     6
#define BUF_SIZE        8

/*
 * Global Variables
 */

short ONE  = 1;
short TWO  = 2;
short FIVE = 5;
short SIX  = 6;
short NLEN = 25;

struct fld_parms fld[] ={
  { 7,48,3,1, &ONE,  "Based on lines or units", 'a'},
  { 8,48,3,1, &FIVE, "Number of Orders to be Considered",'n'},
  { 9,48,3,1, &SIX,  "Total Time Available", 'a'},
  {11,48,3,1, &ONE,  "Use Current Productivity Rates? (y/n)",'a'},
  {12,48,3,1, &ONE,  "Use Cummulative Productivity Rates? (y/n)",'a'},
  {13,48,3,1, &ONE,  "Use Pick Rate File? (y/n)",'a'}
};

struct fld_parms name_parms = 
  {NROW, 2, 0, 0, &NLEN, 0, 'a'};

char buf[NUM_PROMPTS][BUF_SIZE];

main()
{
  register long i, index, orders;
  register unsigned char t;
  double hours;
  
  putenv("_=config_sel_menu");
  chdir(getenv("HOME"));

  open_all();

  fix(config_sel_menu);
  sd_screen_off();
  sd_clear_screen();                       /* clear screen                   */
  sd_text(config_sel_menu);
  sd_screen_on();

  for(i = 0; i < NUM_PROMPTS; i++) memset(buf[i], 0, BUF_SIZE);
  
  load_names();
  show_names();
  
  strcpy(buf[0], "l");
  strcpy(buf[2], "7.25");
  strcpy(buf[3], "n");
  strcpy(buf[4], "n");
  strcpy(buf[5], "n");
  
  for (i = 0; i < NUM_PROMPTS; i++) 
  {
    sd_prompt(&fld[i], 0);
    sd_cursor(0, fld[i].irow, fld[i].icol);
    sd_text(buf[i]);
  }
  index = 0;
  
  while(1)
  {
    t = sd_input(&fld[0], 0, 0, buf[0], 0);

    if (t == EXIT) leave();
/*
 *  Check lines or units
 */
    *buf[0] = tolower(*buf[0]);
    if (*buf[0] != 'l' && *buf[0] != 'u')
    {
      eh_post(ERR_CODE, buf[0]);
      continue;
    }
    if (t == UP_CURSOR) break;
    if (t != TAB && t != DOWN_CURSOR && t != RETURN) continue;
    
    while (1)
    {
      t = sd_input(&fld[1], 0, 0, buf[1], 0);

      if (t == EXIT) leave();
/*
 *  Process number of orders prompt
 */
      strip_space(buf[1], BUF_SIZE);
      orders = atol(buf[1]);
      
      if (orders < 0)
      {
        eh_post(ERR_CODE, buf[1]);
        continue;
      }
      if (t == UP_CURSOR) break;
      if (t != TAB && t != DOWN_CURSOR && t != RETURN) continue;

      while (1)
      {
        t = sd_input(&fld[2], 0, 0, buf[2], 0);

        if (t == EXIT) leave();
/*
 *  Determine hours available
 */
        strip_space(buf[2], BUF_SIZE);
        hours = atof(buf[2]);

        if (hours < 1.0 || hours > 24.0)
        {
          eh_post(ERR_CODE, buf[2]);
          continue;
        }
        if (t == UP_CURSOR) break;
        if (t != TAB && t != DOWN_CURSOR && t != RETURN) continue;
/*
 * determine which rates to use
 */
        while (1)
        {
          t = sd_input(&fld[3], 0, 0, buf[3], 0);

          if (t == EXIT) leave();

          *buf[3] = tolower(*buf[3]);

          if (*buf[3] != 'y' && *buf[3] != 'n')
          {
            eh_post(ERR_CODE, buf[3]);
            continue;
          }
          if (*buf[3] == 'y')
          {
            strcpy(buf[4], "n");
            sd_cursor(0, fld[4].irow, fld[4].icol);
            sd_text(buf[4]);
            
            strcpy(buf[5], "n");
            sd_cursor(0, fld[5].irow, fld[5].icol);
            sd_text(buf[5]);
          }
          if (t == UP_CURSOR) break;
          if (t == RETURN) report();
          if (t != TAB && t != DOWN_CURSOR && t != RETURN) continue;
          
          while (1)
          {
            t = sd_input(&fld[4], 0, 0, buf[4], 0);

            if (t == EXIT) leave();

            *buf[4] = tolower(*buf[4]);

            if (*buf[4] != 'y' && *buf[4] != 'n')
            {
              eh_post(ERR_CODE, buf[4]);
              continue;
            }
            if (*buf[4] == 'y')
            {
              strcpy(buf[3], "n");
              sd_cursor(0, fld[3].irow, fld[3].icol);
              sd_text(buf[3]);
            
              strcpy(buf[5], "n");
              sd_cursor(0, fld[5].irow, fld[5].icol);
              sd_text(buf[5]);
            }
            if (t == UP_CURSOR) break;
            if (t == RETURN) report();
            if (t != TAB && t != DOWN_CURSOR && t != RETURN) continue;

            while (1)
            {
              t = sd_input(&fld[5], 0, 0, buf[5], 0);

              if (t == EXIT) leave();

              *buf[5] = tolower(*buf[5]);

              if (*buf[5] != 'y' && *buf[5] != 'n')
              {
                eh_post(ERR_CODE, buf[3]);
                continue;
              }
              if (*buf[5] == 'y')
              {
                strcpy(buf[3], "n");
                sd_cursor(0, fld[3].irow, fld[3].icol);
                sd_text(buf[3]);
            
                strcpy(buf[4], "n");
                sd_cursor(0, fld[4].irow, fld[4].icol);
                sd_text(buf[4]);
              }
              if (t == UP_CURSOR) break;
              if (t == RETURN) report();
            }
          }
        }
      }
    }
  }
}
/*-------------------------------------------------------------------------*
 *  Load configuration file names
 *-------------------------------------------------------------------------*/
load_names()
{
  register long n;

  fd = fopen(fd_name, "r");
  
  memset(names, 0, sizeof(names));
  nmax = 0;
  
  if (fd == 0) return 0;
  
  while (fgets(names[nmax], NSIZE - 1, fd) > 0)
  {
    n = strlen(names[nmax]) - 1;
    names[nmax][n] = 0;                  /* remove line feed                 */
    nmax++;
    if (nmax >= NAMES) break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 * Unload configuration file names
 *-------------------------------------------------------------------------*/
unload_names()
{
  register long n;
  
  fd = fopen(fd_name, "w");
  
  for (n = 0; n < NAMES; n++)
  {
    if (*names[n]) fprintf(fd, "%s\n", names[n]);
  }
  fclose(fd);
  return 0;
}
/*-------------------------------------------------------------------------*
 * Show configuration file names
 *-------------------------------------------------------------------------*/
show_names()
{
  register long j, k, n;
  
  sd_cursor(0, NROW - 1, 1);
  sd_clear_rest();
  sd_cursor(0, NROW - 1, 2);
  sd_text(
  "- - - - - - - -   C o n f i g u r a t i o n   N a m e s   - - - - - - - -");
  
  for (n = 0; n < NAMES; n++)
  {
    sd_cursor(0, (n / 3) + NROW, (n % 3) * 26 + 2);
    sd_text(names[n]);
  }

}
/*-------------------------------------------------------------------------*
 * Edit configuration names
 *-------------------------------------------------------------------------*/
edit_names()
{
  register unsigned char t;
  register long j, k, n;
  FILE *fd;
  char fd_name[32];
  
  n = 0;
  
  while (1)
  {
    name_parms.irow =  NROW + (n / 3);
    name_parms.icol = (n % 3) * 26 + 2;
    
    t = sd_input(&name_parms, 0, 0, names[n], 0);
    
    if (t == EXIT) leave();
      
    space_fill(names[n], NSIZE);
    sd_cursor(0, name_parms.irow, name_parms.icol);
    sd_text_2(names[n], NLEN);
    
    strip_space(names[n], NSIZE);
    
    for (k = 0; k < NAMES; k++) 
    {
      if (*names[k]) nmax = k + 1;
    }
    for (k = 0; k < NAMES; k++)
    {
      if (*names[k])
      {
        sprintf(fd_name, "config/%s", names[k]);
        fd = fopen(fd_name, "r");
        if (fd == 0)
        {
          eh_post(ERR_CODE, names[k]);
          n = k;
          break;
        }
        fclose(fd);
      }
    }
    if (k < NAMES) continue;             /* was a bad file name              */
    
    for (k = 0; k < NAMES; k++)
    {
      if (!(*names[k])) continue;
      
      for (j = k + 1; j < NAMES; j++)
      {
        if (!(*names[j])) continue;
        if (memcmp(names[k], names[j], NSIZE) != 0) continue;
        eh_post(LOCAL_MSG, "Duplicate Config Name");
        n = j;
        break;
      }
      if (j < NAMES) break;
    }
    if (k < NAMES) continue;
    
    switch (t)
    {
      case UP_CURSOR:   if (n > 2) n = n - 3;
                        break;
                        
      case DOWN_CURSOR: n = (n + 3) % NAMES;
                        break;
                        
      case TAB:         n = (n + 1) % NAMES;
                        break;
                        
      case RETURN:      return 0;
      
      case BACKWD:      n = 0;
                        break;

      case FORWRD:      n = nmax;
                        if (nmax >= NAMES) n = NAMES - 1;
                        break;
    }
  }
}
/*-------------------------------------------------------------------------*
 * transfer control to report screen
 *-------------------------------------------------------------------------*/
report()
{
  if (*buf[3] != 'y' && *buf[4] != 'y' && *buf[5] != 'y') 
  {
    eh_post(ERR_CODE, buf[3]);
    return 0;
  }
  edit_names();
  unload_names();
  
  close_all();
  execlp("config_select", "config_select", 
    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], 0);
  krash("report", "config_select load", 1);
}
/*-------------------------------------------------------------------------*
 *  Open all files
 *-------------------------------------------------------------------------*/
open_all()
{
  sd_open(leave);
  ss_open();
  return 0;
}
/*-------------------------------------------------------------------------*
 * close all files
 *-------------------------------------------------------------------------*/
close_all()
{
  sd_close();
  ss_open();
  return 0;
}
/*-------------------------------------------------------------------------*
 * transfer control back to calling program
 *-------------------------------------------------------------------------*/
leave()
{
  close_all();
  execlp("pnmm", "pnmm", 0);
  krash("leave", "pnmm load", 1);
}

/* end of config_sel_menu.c */





