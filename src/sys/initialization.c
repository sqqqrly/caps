#define DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Screen to control initialization and configuration.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/18/93   |  tjt  Add and revised fro mfc.
 *  05/16/94   |  tjt  Add check_db before initialize.
 *  07/12/94   |  tjt  Add check caps busy.
 *  03/23/95   |  tjt  Add mark ports closed on failure.
 *  05/09/95   |  tjt  Add over 12 ports and print report.
 *  05/20/96   |  tjt  Add box full module.
 *  05/26/98   |  tjt  Add IO/BF to screen display.
 *  10/17/99   |  aha  Modified output to leave room to print 6 character
 *                     wide port name.
 *  10/08/01   |  aha  Added fix for simulation mode
 *-------------------------------------------------------------------------*/
#ident "(C) Kingway Material Handling Company %M% %H%"
static char initialization_c[] = "%Z% %M% %I% (%G% - %U%)";

/****************************************************************************/
/*                                                                          */
/*                          Initialization Screen                           */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "global_types.h"
#include "iodefs.h"
#include "message_types.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "initialization.t"
#include "eh_nos.h"

extern catcher();

#ifdef DEBUG
FILE *DF;
#endif

FILE *fd;
char fd_name[16];

long simulated = 0;

unsigned char list[] =
{ClientMessageEvent, InitErrorMessageEvent, InitializeEvent, ConfigureEvent};

struct port_item old[PortMax];             /* previous hardware summary     */
struct port_item old_tot = {0};
struct port_item new_tot = {0};

short ONE = 1;
short THIRTY = 30;

struct fld_parms fld1 = {21,35,5,1,&ONE,   "Initialize?     (y/n)", 'a'};
struct fld_parms fld2 = {22,35,5,1,&THIRTY,"Enter Configuration Name", 'a'};
struct fld_parms fld3 = {22,20,5,1,&ONE,   "Print? (y/n)", 'a'};

unsigned char t;
char buf[2];
char bif[32];
char parm[32];
char tocenter[80];
long pid, status;
long failed = 0;

long TIMEOUT = 500;
long now, start, time_to_go;

#define MAX 100
#define LINES 12
long max = 0;
unsigned char mtab[MAX][80];              /* message table                   */

main(argc,argv)
char **argv;
short argc;
{
  register long k, flag;

  putenv("_=initialization");             /* program name                    */
  chdir(getenv("HOME"));

  if (argc > 1) strcpy(parm, argv[1]);    /* save calling parm               */

#ifdef DEBUG
  DF = fopen("debug/init", "w");
#endif

  sd_open(catcher);
  message_select(list, sizeof(list));
  ss_open();
  co_open();

  TIMEOUT = sp->sp_rp_timeout;

  tmp_name(fd_name);

  memcpy(old, po, sizeof(struct port_item) * coh->co_ports);

  fix(initialization);
  sd_screen_off();
  sd_clear_screen();                      /* clear screen                    */
  sd_text(initialization);
  sd_screen_on();

  if (sp->sp_running_status == 'y')
  {
    strcpy(tocenter, "*** Warning - CAPS Is Running ***");
    sd_cursor(0, 18, 40 - strlen(tocenter) / 2);
    sd_text(tocenter);
    sleep(5);
    leave(0);
  }
  if (sp->sp_in_process_status != 'x')
  {
    strcpy(tocenter, "*** Warning - CAPS Is Busy ***");
    sd_cursor(0, 18, 40 - strlen(tocenter) / 2);
    sd_text(tocenter);
    sleep(5);
    leave(0);
  }
  if (sp->sp_full_function  == 's'    ||
      sp->sp_basic_function == 's'    ||
      sp->sp_total_function == 's' )
  {
    strcpy(tocenter, "*** Warning - CAPS Hardware Is Simulated ***");
    sd_cursor(0, 18, 40 - strlen(tocenter) / 2);
    sd_text(tocenter);
    simulated = 1;
  }
  flag = 1;                                 /* prompt for name               */
  if (strcmp(parm, "reconfigure") == 0)
  {
    strcpy(tocenter, "*** Reconfiguration In Progress ***");
    sd_cursor(0, 19, 40 - strlen(tocenter) / 2);
    sd_text(tocenter);
  }
  else if (strcmp(parm, "recover") == 0)
  {
    strcpy(tocenter, "*** System Recovery In Progress ***");
    sd_cursor(0, 19, 40 - strlen(tocenter) / 2);
    sd_text(tocenter);
    strcpy(bif, coh->co_config_name);
    flag = 0;                             /* use current name                */
  }
  sp->sp_in_process_status = 'x';         /* set nothing in progress         */
  sp->sp_init_status = 'n';               /* set not initialized             */
  sp->sp_config_status = 'n';             /* set not configured              */

  strcpy(tocenter, "*** Checking Databases In Progress ***");
  sd_cursor(0, 7, 40 - strlen(tocenter) / 2);
  sd_text(tocenter);
  sd_wait();

  system("etc/check_db >dat/log/initialize.log 2>&1");

/*-------------------------------------------------------------------------*
 * Initialize
 *-------------------------------------------------------------------------*/

  while (sp->sp_init_status != 'y')
  {
    fd = fopen(fd_name, "w");
    if (fd == 0) krash("main", "tmp file", 1);

    failed = 0;

    time_to_go = TIMEOUT;
    start = time(0);

    memset(mtab, 0, 80 * MAX);
    max = 0;

    strcpy(tocenter, "*** Initialization In Progress ***");
    sd_cursor(0, 7, 40 - strlen(tocenter) / 2);
    sd_text(tocenter);

    sd_wait();

    message_put(0, InitializeRequest, 0, 0);

    while (time_to_go > 0)
    {
#ifdef DEBUG
  fprintf(DF, "TIMEOUT=%d time_to_go=%d\n", TIMEOUT, time_to_go);
#endif
      sleep(time_to_go);
      if (sp->sp_init_status == 'y') break;

      if (failed) break;
      else
      {
        now = time(0);
        time_to_go -= (now - start);
        start = now;
      }
    }
    if (sp->sp_init_status == 'y') break;

#ifdef DEBUG
  fprintf(DF, "TIMEOUT=%d time_to_go=%d failed=%d\n",
    TIMEOUT, time_to_go, failed);
#endif
    if (time_to_go <= 0)
    {
      eh_post(LOCAL_MSG, "*** CAPS Is Not Responding ***");
      failed = 1;
    }
    show_errors();

    while (1)                          /* Initialize? question          */
    {
      memset(buf, 0, 2);
      sd_prompt(&fld1, 0);
      t = sd_input(&fld1, 0, 0, buf, 0);
      if (t == EXIT) leave(0);

      *buf = toupper(*buf);
      if (*buf == 'Y') break;
      if (*buf == 'N') leave(0);
      eh_post(ERR_YN, 0);
    }
  }

  if (!simulated)
     {
       if (fork() == 0)
          {
            close_all();
            execlp("compare_hw_maps", "compare_hw_maps", "-m", 0);
            krash("main", "compare_hw_maps load", 1);
          }

       pid = wait(&status);

       show_errors();
       show_init();
     }

/*-------------------------------------------------------------------------*
 *  Configuration Name
 *-------------------------------------------------------------------------*/
  while (sp->sp_config_status != 'y')
  {
    if (flag)
    {
      sd_prompt(&fld2, 0);
      t = sd_input(&fld2, 0, 0, bif, 0);
      if (t == UP_CURSOR) continue;
      if (t == EXIT) leave(0);
    }
    if (!(*bif)) continue;

    memset(mtab, 0, 80 * MAX);
    max = 0;

    failed = 0;

    start = time(0);
    time_to_go = TIMEOUT;

    sd_wait();

    message_put(0, ConfigureRequest, bif, strlen(bif));

    while (time_to_go > 0)
    {
      sleep(time_to_go);
      if (sp->sp_config_status == 'y') break;
      if (failed) break;
      else
      {
        now = time(0);
        time_to_go -= (now - start);
        start = now;
      }
    }
    if (sp->sp_config_status == 'y') break;

    if (time_to_go <= 0)
    {
      eh_post(LOCAL_MSG, "*** CAPS Is Not Responding ***");
    }
    if (strcmp(parm, "recover") == 0)
    {
      eh_post(CRASH_MSG, "*** Recovery Needs Valid Configuration");
    }
    flag = 1;
  }
  show_init();

  if (fork() == 0)
  {
    close_all();
    execlp("save_hw_maps", "save_hw_maps", "-m", 0);
    krash("main", "save_hw_maps load", 1);
  }
  pid = wait(&status);

/*-------------------------------------------------------------------------*
 *  Reconfigure Orders
 *-------------------------------------------------------------------------*/
  if (strcmp(parm, "initialize") != 0)
  {
    if (strcmp(parm, "recover") == 0)
    {
      sd_cursor(0, 21, 1); sd_clear_line();
      strcpy(tocenter, " *** Checking and Fixing Order Indices ***");
      sd_cursor(0, 21, 40 - strlen(tocenter) / 2);
      sd_text(tocenter);
      system ("of_diags -r 1>dat/log/of_diag.log 2>&1");
    }
    strcpy(tocenter, "*** Reconfiguring The Order Databases ***");
    sd_cursor(0, 20, 40 - strlen(tocenter) / 2);
    sd_text(tocenter);
    sd_wait();

    if (fork() == 0)
    {
      close_all();
      if (strcmp(parm, "reconfigure") == 0)
      {
        execlp("reconfigure_orders", "reconfigure_orders", 0);
        krash("main", "reconfigure_orders load", 1);
      }
      execlp("recover_orders", "recover_orders", 0);
      krash("main", "recover_orders load", 1);
    }
    pid = wait(&status);

    if (pid < 0 || status) krash("main", "reconfigure/recover failed", 1);

    message_put(0, SystemRecoveryEvent, 0, 0);

    *parm = toupper(*parm);
    eh_post(ERR_CONFIRM, parm);
  }
  sd_cursor(0, 22, 1); sd_clear_line();
  strcpy(tocenter, "*** Hit Any Key To Exit ***");
  sd_cursor(0, 22, 40 - strlen(tocenter) / 2);
  sd_text(tocenter);
  sd_echo_flag = 0;
  sd_keystroke();
  leave(0);
}
/*-------------------------------------------------------------------------*
 *  Show Errors
 *-------------------------------------------------------------------------*/
show_errors()
{
  register long j, k, ret, pos;
  char prt_name[16];
  long stat;

  fclose(fd);

  if (max > 0)
  {
    pos = 0;

    while (1)                               /*  print question               */
    {
      strcpy(buf, "n");
      sd_prompt(&fld3, 0);
      t = sd_input(&fld3, 0, 0, buf, 0);

      ret = sd_print(t, *buf);

      if (ret == 0) leave(0);
      else if (ret == 1)
      {
        pos += LINES;
        if (pos > max - LINES) pos = max - LINES;
        if (pos < 0) pos = 0;
      }
      else if (ret == 2)
      {
        pos -= LINES;
        if (pos < 0) pos = 0;
      }
      else if (ret == 3) break;
      else if (ret == 4)
      {
        if (fork() == 0)
        {
          execlp("prft", "prft", fd_name, tmp_name(prt_name),
            "sys/report/init_report.h", 0);
          krash("show_errors", "prft load", 1);
        }
        wait(&stat);
        return 0;
      }
      for (j = 0, k = pos; j < LINES && k < max; j++, k++)
      {
        sd_cursor(0, 9 + j, 1);
        sd_text(mtab[k]);
      }
    }
  }
  unlink(fd_name);
  fd = 0;
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Message Processing
 *-------------------------------------------------------------------------*/
catcher(who, type, buf, len)
register long who;
register long type;
register unsigned char *buf;
register long len;
{
  register long j, k;

  buf[len] = 0;

  switch(type)
  {
    case ShutdownEvent:

      close_all();
      exit(0);

    case ClientMessageEvent:

      failed = 1;
      eh_post(*buf, buf + 1);
      break;

    case InitErrorMessageEvent:

      if (max < 1)
      {
        for (j = 0; j < LINES; j++)
        {
          sd_cursor(0, 9 + j, 1);
          sd_clear_line();
        }
      }
      if (max < MAX && len > 1)
      {
        if (len > 0) len--;
        if (len > 79) len = 79;
        buf[79] = 0;
        if (len < 79) memset(buf + len, 0x20, 79 - len);
        memcpy(mtab[max], buf, 80);
        max++;
      }
      if (fd) fprintf(fd, "%s", buf);

      k = max > LINES ? max - LINES : 0;

      for (j = 0; j < LINES && k < max; j++, k++)
      {
        sd_cursor(0, 9 + j, 1);
        sd_text(mtab[k]);
      }
      break;

    case InitializeEvent:

      eh_post(ERR_CONFIRM, "Initialization");
      break;

    case ConfigureEvent:

      eh_post(ERR_CONFIG_DONE, buf);
      break;

  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Show Initialization Summary
 *-------------------------------------------------------------------------*/
show_init()
{
  register long j, k, pos, ret, len;
  char text[80], diff[6], prt_name[16], *name;
  long stat;

  fd = fopen(fd_name, "w");
  if (fd == 0) krash("show_init", "tmp file", 1);

  memset(mtab, 0, 80 * MAX);
  max = 0;

  memset(&old_tot, 0, sizeof(old_tot));
  memset(&new_tot, 0, sizeof(new_tot));

  for (j = 6; j < 24; j++)
  {
    sd_cursor(0, j, 1);
    sd_clear_line();
  }
  sprintf(text,
  "---------------- Current --------------  ----------- Previous -----------");
  sd_cursor(0, 6, 2); sd_text(text);
  fprintf(fd, " %s\n", text);

  sprintf(text,
  "Line    BL   ZC  PM/PI IO/BF TC  Lights   BL   ZC  PM/PI IO/BF TC  Lights");

  sd_cursor(0, 7, 2); sd_text(text);            /* F052698 - add IO/BF to headin
g */
  fprintf(fd, " %s\n", text);

  sprintf(text,
  "------ ---- ---- ----- ---- ---- ------  ---- ---- ----- ---- ---- ------");
  sd_cursor(0, 8, 2); sd_text(text);
  fprintf(fd, " %s\n", text);

  for (k = 0; k < coh->co_ports; k++)
  {
     if (!po[k].po_lights && !old[k].po_lights) continue;

     memset(diff, 0x20, 6);
     if (po[k].po_count[BL  - 1] != old[k].po_count[BL  - 1]) diff[0] = '*';
     if (po[k].po_count[ZC  - 1] != old[k].po_count[ZC  - 1]) diff[1] = '*';
     if (po[k].po_count[ZC2 - 1] != old[k].po_count[ZC2 - 1]) diff[1] = '*';
     if (po[k].po_count[PM  - 1] != old[k].po_count[PM  - 1]) diff[2] = '*';
     if (po[k].po_count[PI  - 1] != old[k].po_count[PI  - 1]) diff[2] = '*';
     if (po[k].po_count[PM2 - 1] != old[k].po_count[PM2 - 1]) diff[2] = '*';
     if (po[k].po_count[PM4 - 1] != old[k].po_count[PM4 - 1]) diff[2] = '*';
     if (po[k].po_count[PM6 - 1] != old[k].po_count[PM6 - 1]) diff[2] = '*';
     if (po[k].po_count[BF  - 1] != old[k].po_count[BF  - 1]) diff[3] = '*';
     if (po[k].po_count[IO  - 1] != old[k].po_count[IO  - 1]) diff[3] = '*';
     if (po[k].po_controllers    != old[k].po_controllers)    diff[4] = '*';
     if (po[k].po_lights         != old[k].po_lights)         diff[5] = '*';

     for (j = BL; j <= IO; j++)
     {
       old_tot.po_count[j - 1] += old[k].po_count[j - 1];
     }
     old_tot.po_controllers += old[k].po_controllers;
     old_tot.po_lights      += old[k].po_lights;

     for (j = BL; j <= IO; j++)
     {
       new_tot.po_count[j - 1] += po[k].po_count[j - 1];
     }
     new_tot.po_controllers += po[k].po_controllers;
     new_tot.po_lights      += po[k].po_lights;

     sprintf(text, "%3d   %4d%5d%6d%5d%5d%7d  %4d%c%4d%c%5d%c%4d%c%4d%c%6d%c",
       k+1,
       po[k].po_count[BL - 1],
       po[k].po_count[ZC - 1] + po[k].po_count[ZC2 - 1],
       po[k].po_count[PM - 1] + po[k].po_count[PI - 1] +
         po[k].po_count[PM2 - 1] + po[k].po_count[PM4 - 1] +
         po[k].po_count[PM6 - 1],
       po[k].po_count[BF - 1] + po[k].po_count[IO - 1],
       po[k].po_controllers, po[k].po_lights,

       old[k].po_count[BL - 1], diff[0],
       old[k].po_count[ZC - 1] + old[k].po_count[ZC2 - 1], diff[1],
       old[k].po_count[PM - 1] + old[k].po_count[PI - 1] +
         old[k].po_count[PM2 - 1] + old[k].po_count[PM4 - 1] +
         old[k].po_count[PM6 - 1], diff[2],
       old[k].po_count[BF - 1] + old[k].po_count[IO - 1], diff[3],
       old[k].po_controllers, diff[4], old[k].po_lights, diff[5]);

     if (sp->sp_port_by_name == 'y')      /* overlay port name               */
     {
       memset(text, 0x20, 6);             /* erase port number               */
       name = (char *)basename(po[k].po_name);
       len  = strlen(name);
       memcpy(text, name, len > 6 ? 6 : len);
     }
     strcpy(mtab[max++], text);
     fprintf(fd, " %s\n", text);
  }
  memset(diff, 0x20, 6);
  if (new_tot.po_count[BL  - 1] != old_tot.po_count[BL  - 1]) diff[0] = '*';
  if (new_tot.po_count[ZC  - 1] != old_tot.po_count[ZC  - 1]) diff[1] = '*';
  if (new_tot.po_count[ZC2 - 1] != old_tot.po_count[ZC2 - 1]) diff[1] = '*';
  if (new_tot.po_count[PM  - 1] != old_tot.po_count[PM  - 1]) diff[2] = '*';
  if (new_tot.po_count[PI  - 1] != old_tot.po_count[PI  - 1]) diff[2] = '*';
  if (new_tot.po_count[PM2 - 1] != old_tot.po_count[PM2 - 1]) diff[2] = '*';
  if (new_tot.po_count[PM4 - 1] != old_tot.po_count[PM4 - 1]) diff[2] = '*';
  if (new_tot.po_count[PM6 - 1] != old_tot.po_count[PM6 - 1]) diff[2] = '*';
  if (new_tot.po_count[BF  - 1] != old_tot.po_count[BF  - 1]) diff[3] = '*';
  if (new_tot.po_count[IO  - 1] != old_tot.po_count[IO  - 1]) diff[3] = '*';
  if (old_tot.po_controllers != new_tot.po_controllers) diff[4] = '*';
  if (old_tot.po_lights      != new_tot.po_lights)      diff[5] = '*';

  sprintf(text,
  "       ---- ---- ----- ---- ---- ------  ---- ---- ----- ---- ---- ------");
  strcpy(mtab[max++], text);
  fprintf(fd, " %s\n", text);

  sprintf(text, "Total  %3d%5d%6d%5d%5d%7d  %4d%c%4d%c%5d%c%4d%c%4d%c%6d%c",

    new_tot.po_count[BL - 1],
    new_tot.po_count[ZC - 1] + new_tot.po_count[ZC2 - 1],
    new_tot.po_count[PM - 1] + new_tot.po_count[PI - 1] +
      new_tot.po_count[PM2 - 1] + new_tot.po_count[PM4 - 1] +
      new_tot.po_count[PM6 - 1],
    new_tot.po_count[BF - 1] + new_tot.po_count[IO - 1],
    new_tot.po_controllers, new_tot.po_lights,

    old_tot.po_count[BL - 1], diff[0],
    old_tot.po_count[ZC - 1] + old_tot.po_count[ZC2 - 1], diff[1],
    old_tot.po_count[PM - 1] + old_tot.po_count[PI - 1] +
      old_tot.po_count[PM2 - 1] + old_tot.po_count[PM4 - 1] +
      old_tot.po_count[PM6 - 1], diff[2],
    old_tot.po_count[BF - 1] + old_tot.po_count[IO - 1], diff[3],
    old_tot.po_controllers, diff[4], old_tot.po_lights, diff[5]);

  strcpy(mtab[max++], text);
  fprintf(fd, " %s\n", text);

  fclose(fd);

  pos = 0;

  while (1)                                 /*  print question               */
  {
    for (j = 0; j < LINES; j++)
    {
      sd_cursor(0, 9 + j, 2);
      if (j + pos < max) sd_text(mtab[j + pos]);
      else sd_clear_line();
    }
    strcpy(buf, "n");
    sd_prompt(&fld3, 0);
    t = sd_input(&fld3, 0, 0, buf, 0);

    ret = sd_print(t, *buf);

    if (ret == 0) leave(0);
    else if (ret == 1)
    {
      pos += LINES;
      if (pos > max - LINES) pos = max - LINES;
      if (pos < 0) pos = 0;
    }
    else if (ret == 2)
    {
      pos -= LINES;
      if (pos < 0) pos = 0;
    }
    else if (ret == 3) break;
    else if (ret == 4)
    {
      if (fork() == 0)
      {
        execlp("prft", "prft", fd_name, tmp_name(prt_name),
          "sys/report/init_report.h", 0);
        krash("show_errors", "prft load", 1);
      }
      wait(&stat);
      return 0;
    }
  }
/* unlink(fd_name);  */
  fd = 0;
  return 0;
}
/*--------------------------------------------------------------------------*
 *  Graceful Exit Routines
 *--------------------------------------------------------------------------*/
close_all()
{
  co_close_save();
  ss_close_save();
  if (fd) fclose(fd);
}
leave()
{
  register long k;

  if (sp->sp_running_status != 'y')
  {
    for (k = 0; k < coh->co_ports; k++)
    {
      if (!po[k].po_id) continue;
      po[k].po_status = 'x';
    }
  }
  sd_close();
  close_all();
  unlink(fd_name);

#ifdef DEBUG
  fclose(DF);
#endif

  execlp("mmenu", "mmenu", 0);
  krash("leave", "load mmenu", 1);


}

/* end of initialization.c */
