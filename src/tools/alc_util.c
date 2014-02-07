/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description: Area controller port utility program.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/21/94   |  tjt  Revised from tlc_util.
 *-------------------------------------------------------------------------*/
static char alc_util_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "td.h"
#include "file_names.h"

#define MAX 360                           /* response buffer area modulo 12  */

unsigned char ibuf[40];                   /* input command line              */
unsigned char obuf[12][78];               /* output display window           */
char number[6];                           /* response value                  */

#include "alc_util.t"

td_driver_item parms[] = {{ac, 0}, {cmd, 0}, {text, 0}};

td_driver_call get_command = {parms, 0, 3, LOAD, REVDIM, F4,
  {td_Fexit, td_Fexit, td_Fexit, td_Fexit,
    td_Fexit, td_Fexit, td_Fexit, td_Fexit}};

td_driver_item ac_parm[] = {{message,  0}, {response, 0}};

td_driver_call get_ac_addr = {&ac_parm[1], 0, 1, LOAD, REVDIM, F4,
  {td_Fexit, 0, 0, td_Fexit, td_Fexit, 0, 0, 0}};

td_driver_call ac_clear = {ac_parm, 0, 2, CLEARONLY};


unsigned char table[MAX][78];             /* large display area              */
long max;                                 /* last display line               */
long count;                               /* count of display lines          */
long line;                                /* current display line            */
unsigned long start;                      /* microclock time                 */

long port;                                /* current open port file desc     */
long port_count;                          /* messages serviced               */

main(argc, argv)
long argc;
char **argv;
{
  extern get_response();
  extern quit();

  register unsigned char c;

  putenv("_=alc_util");
  chdir(getenv("HOME"));

  if (argc < 2)
  {
    printf("Must Specify Port Name\n\n");
    return;
  }
  port = ac_open(argv[1]);
  if (port <= 0)
  {
    printf("Can't Open %s\n\n", argv[1]);
    return;
  }
  td_open(80);
  td_init(&alc_util_window);
  td_tab(2, "2   HARD");
  td_tab(3, "3  CLEAR");
  td_tab(4, "4   SEND");
  td_tab(5, "5 DOWNLD");
  td_tab(6, "6 PAGEUP");
  td_tab(7, "7 PAGEDN");
  td_tab(8, "8 READDR");
      
  clear();                                /* clear everything                */

  while (1)
  {
    td_driver(&get_command, 0);           /* get next command/function key   */
    c = get_command.td_c;
    td_message(" ");
      
    if (c == F1) break;
    else if (c == F2)    ac_reset(port);
    else if (c == F3)    {clear(); pageup();}
    else if (c == F4)
    {
      strip_space(ibuf + 6, 32);

      start = microclock();
      ac_write(port, ibuf, strlen(ibuf));
      get_response();
    }
    else if (c == F5)    {download(); td_driver(&ac_clear, 0);}
    else if (c == F6)    get_response();
    else if (c == F7)    pagedown();
    else if (c == F8)    {readdress(); td_driver(&ac_clear, 0);}
  }
  td_close();
}
/*-------------------------------------------------------------------------*
 * Clear Displays
 *-------------------------------------------------------------------------*/
clear()
{
  memset(table, 0x20, 78 * MAX);          /* response display buffer         */
  memset(ibuf, '0', 6);
  memset(ibuf + 6, 0x20, 32);
  count = line = max = 0;
}
/*-------------------------------------------------------------------------*
 *  Readdress All Modules On Line
 *-------------------------------------------------------------------------*/
readdress()
{
  register unsigned char c;
  long ret, n;
  float elapsed;
  char err_mess[40];
   
  strcpy(number, "1");

  td_display(message, "First TC Addr In Port:");
  td_driver(&get_ac_addr, 0);
  c = get_ac_addr.td_c;
   
  if (c == F1 || c == F5) return c;
   
  sscanf(number, "%d", &n);

  if (n < 1 || n > 9998)
  {
    td_message("*** Invalid Address");
    return 0;
  }
  start = microclock();

  ret = ac_readdress(port, n);
  if (ret < 0) td_message("*** Line Is Noisy");
  else if (ret > 0)
  {
    sprintf(err_mess, "*** AC %d Failed To Address", ret);
    td_message(err_mess);
  }
  else
  {
    elapsed = (float)(microclock() - start) / 1048576;
    sprintf(err_mess, "Complete in %8.3f seconds", elapsed);
    td_message(err_mess);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Download All Firmware
 *-------------------------------------------------------------------------*/
download()
{
  register unsigned char c;
  float elapsed;
  long n;
  char err_mess[40];

  strcpy(number, "9999");

  td_display(message, "TC Address To Download:");
  td_driver(&get_ac_addr, 0);
  c = get_ac_addr.td_c;

  if (c == F1 || c == F5) return c;
   
  sscanf(number, "%d", &n);

  if (n < 1 || n > 9999)
  {
    td_message("*** Invalid Address");
    return 0;
  }
  td_message("*** Requires About 2 Minutes");

  start = microclock();

  ac_download(port, n, alc_firmware_name);

  elapsed = (float)(microclock() - start) / 1048576;
  sprintf(err_mess, "Complete in %8.3f seconds", elapsed);
  td_message(err_mess);

  return;
}
/*-------------------------------------------------------------------------*
 *  Display Next Page 
 *-------------------------------------------------------------------------*/
pageup()
{
  register long k;

  line += 12;
  if (line > max)
  {
    line = max;
    if (line < 12) line = 12;
  }
  for (k = 0; k < 12; k++)
  {
    memcpy(obuf[k], table[line - 12 + k], 78);
  }
  td_update(display);
  td_refresh();
  return;
}
/*-------------------------------------------------------------------------*
 *  Display Previous Page 
 *-------------------------------------------------------------------------*/
pagedown()
{
  register long k;

  line -= 12;
  if (line < 12) line = 12;
   
  for (k = 0; k < 12; k++)
  {
    memcpy(obuf[k], table[line - 12 + k], 78);
  }
  td_update(display);
  td_refresh();
  return;
}
/*-------------------------------------------------------------------------*
 *  Get Some Port Response
 *-------------------------------------------------------------------------*/
get_response()
{
  register long k, ret, len;
  float elapsed;
  unsigned char buffer[512];
  unsigned char work[84];
   
  while (1)
  {
    len = ret = ac_readout(port, buffer, 1);
    if (ret < 0) break;

    if (!ret) len = strlen(buffer) - 1;
    buffer[len] = 0;

    if (max >= MAX) max = 0;              /* circular buffering              */
    count++;                              /* count input responses           */
         
    elapsed = (float)(microclock() - start) / 1048.576;

    if (!ret)
    {
      sprintf(work, "%4d: %8.3fms  %4.4s %2.2s %s",
        count, elapsed, buffer, buffer+4, 
        "* * * * * * *  Error Packet Below * * * * * * *");
      
      memcpy(table[max], work, strlen(work));
      max = (max + 1) % MAX;
    }
    if (len > 6)
    {
      sprintf(work, "%4d: %8.3fms  %4.4s %2.2s %-50.50s",
        count, elapsed, buffer, buffer+4, buffer+6);
      memcpy(table[max], work, strlen(work));
      max = (max + 1) % MAX;
      
      for (k = 56; k < len; k += 50)
      {
        sprintf(work, "%26s%-50.50s", " ", buffer + k);
        memcpy(table[max], work, strlen(work));
        max = (max + 1) % MAX;
      }
    }
  }
  pageup();
  return;
}

/* end of alc_util.c */
