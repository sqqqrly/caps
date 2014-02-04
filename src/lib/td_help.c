/*
 *  td_help.c
 *
 *  Generic Help Package
 */
#include <stdio.h>
#include "td.h"

#define ROWS   5
#define COLS   48
#define MAX    100
#define BLOCK  COLS * MAX
#define FLAG   '#'
#define LF     0x0a

unsigned char help_lines[ROWS][COLS];
unsigned char help_mess[8];
#include "help.t"
unsigned char help_save[3168];
td_window help_save_window = {&td_crt,0,0,24,24,132,0,0,help_save,'t','x'};

td_help(who, number)                      /* find help text                  */
unsigned char *who;                       /* help file name                  */
register long number;                     /* field number in screen          */
{
  register unsigned char *p, *q;
  register long k;
  unsigned char text[BLOCK];              /* text block                      */
  unsigned char fd_name[80];
  FILE *fd;
   
  p = (unsigned char *)getenv("HELP");
  if (!p) p = (unsigned char *)getenv("DBPATH");
  if (p) sprintf(fd_name, "%s/%s", p, who);
  else strcpy(fd_name, who);

  fd = fopen(fd_name, "r");
  if (fd == 0)
  {
    td_message("*** Can't Find Any Help");
    return 1;
  }
  help_save_window.td_width  = td_crt.td_width;
  k = 24 * help_save_window.td_width;

  p = help_save;
  q = td_screen;

  memcpy(p, q, k);                        /* save screen                     */

  while (1)                               /* find field number               */
  {
    if (!fgets(text, 128, fd))            /* read next line                  */
    {
      text[0] = 0;                        /* insure no FLAG byte             */
      break;                              /* break on EOF                    */
    }
    if (text[0] == FLAG)                  /* found a numbered line           */
    {
      number--;
      if (number < 0) break;
    }
  }
  if (text[0] == FLAG)
  {
    fread(text, BLOCK, 1, fd);

    td_update(&help_window);
    help_display(text);
    td_update(&help_save_window);
    td_refresh();
  }
  fclose(fd);
  return 0;
}
help_display(text)                        /* show help text                  */
unsigned char *text;
{
  extern unsigned char *help_copy();

  unsigned char *where[MAX+1];
  register unsigned char *p;
  register long j, k, max;
  unsigned char c;
   
  p = text;
  k = 0;
   
  while (k < MAX && p < text + BLOCK)     /* find all text lines             */
  {
    if (*p == FLAG) break;
    where[k] = p;
    k++;
    p = help_copy(help_lines[0], p);
  }
  where[k] = 0; max = k;

  c = HOME;
   
  while (1)
  {
    switch (c)
    {
      case HOME:     k = 0;
        break;

      case DOWN:
      case NEXTPAGE: break;

      case UP:
      case PREVPAGE: k = (((k - 1) / ROWS) - 1) * ROWS;
        if (k < 0) k = 0;
        break;

        default:       return;
      }
    if (k < max)
    {
      memset(help_lines, 0x20, ROWS * COLS);

      for (j = 0; j < ROWS; j++)
      {
        if (k < max)
        {
          help_copy(help_lines[j], where[k]);
          k++;
        }
      }
      if (k < max) strncpy(help_mess, "(more)  ", 8);
      else memset(help_mess, 0x20, 8);
      td_update(help_more);
      td_update(help_text);
      td_refresh();
    }
    c = td_get_byte(&help_window);
  }
}
/*
 *  Copy to LF or NULL
 */
unsigned char *help_copy(p, q)
register unsigned char *p, *q;
{
  register long k;
   
  k = 0;

  while (*q)
  {
    if (*q == LF) {q++; break;}
    if (k < COLS) {*p++ = *q; k++;}
    q++;
  }
  return q;
}
         
/* end of td_help.c */

