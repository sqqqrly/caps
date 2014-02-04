/*
 *  td_pr.c
 *
 *  Keystroke Record and Playback
 *
 *  To Record:
 *
 *    PLAY=NO         (!= YES is ignored)
 *    RECORD=filename
 *
 *  To Playback:
 *
 *    PLAY=YES
 *    RECORD=anything  (is ignored)
 *    main_menu  <filename
 *
 */
#include <stdio.h>
#include "td.h"

static long td_pr_count = 0;
static long td_pr_flag  = 0;

static FILE *td_pr_fd   = 0;

/*
 * Open File For Record
 */
long td_pr_open()
{
  unsigned char *p;

  td_pr_fd    = 0;                        /* file not open yet               */
  td_pr_flag  = 0;                        /* normal keyboard input           */

  p = (unsigned char *)getenv("PLAY");    /* open file for playback          */
  if (p)
  {
    if (strcmp(p, "YES") == 0)
    td_pr_flag = 2;                       /* PLAY=YES to playback            */
  }
  p = (unsigned char *)getenv("RECORD");  /* open file to record keys        */
  if (p && *p)
  {
    td_pr_fd = fopen(p, "a+");            /* append more keystrokes          */
    if (td_pr_fd == 0) return 0;
    td_pr_flag = 1;                       /* record keystrokes               */
  }
  return 0;
}
/*
 *  Close Record/Playback File
 */
long td_pr_close()
{
  if (td_pr_fd)
  {
    fputc(0x5c, td_pr_fd);                /* ESCAPE LF                       */
    fputc(0x0a, td_pr_fd);
    fclose(td_pr_fd);
  }
  td_pr_flag = 0;
  return 0;
}
/*
 *  Record Key Strokes For Playback
 */
long td_record(c)
unsigned char c;
{
  if (c < 0x20)
  {
    if (c == 0) return 0;                 /* ignore nulls                    */
    td_pr_count++;
    fputc('^', td_pr_fd);                 /* CONTROL C                       */
    c += 0x40;
  }
  fputc(c, td_pr_fd);
  td_pr_count++;
   
  if (td_pr_count >= 40)
  {
    fputc(0x5c, td_pr_fd);                /* BACKSLASH LF                    */
    fputc(0x0a, td_pr_fd);
    td_pr_count = 0;
  }
  return 0;
}
/*
 *  Playback Key Strokes
 */
unsigned char td_playback()
{
  unsigned char c;
   
  if (td_pr_flag == 2)                    /* playback from file              */
  {
    while (1)
    {
      c = getchar();
      if (c == 0x5c)                      /* is a backslash                  */
      {
        c = getchar();
        continue;
      }
      else if (c == '^')                  /* is a control symbol             */
      {
        c = getchar();
        c -= 0x40;
      }
      break;
    }
  }
  else                                    /* keyboard input                  */
  {
    while (1)
    {
      c = getchar();
      if (td_pr_flag == 1) td_record(c);  /* record keystrokes               */
      break;
    }
  }
  return c;
}

/* end of td_pr.c */

