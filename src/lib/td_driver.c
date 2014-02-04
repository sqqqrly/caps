/*-------------------------------------------------------------------------
 *  Copyright (c) 1989 - 1993 PTW Systems, Inc. - All rights reserved.
 *
 *  THIS IS UNPUBLISHED SOURCE CODE OF PTW SYSTEMS, INC.
 *  This copyright notice, above, does not evidence any actual
 *  or intended publication of such source code.
 *-------------------------------------------------------------------------*/
/*
 *  td_driver.c
 *
 *  General Purpose multifield screen driver
 *
 *   Initial:        NULL        = no initial action
 *                   RESETFLAGS  = clear edit flags
 *                   CLEAR       = clear and reset edit flags
 *                   LOAD        = load and reset edit flags
 *                   CLEARONLY   = clear, reset flags and exit
 *                   LOADONLY    = clear, reset flags and exit
 *
 *
 *   Return:         FAILED      (0) = Failed Edit, Abort, etc.
 *                   PASSED      (1) = Passed Edits
 *                   td_c        = exit key
 *                   td_k        = exit field
 * 
 *   Function Keys:  NULL        (0) = no action
 *                   PASSED      (1) = test field edit
 *                   EXIT/ABORT  (2) = exit always (return FAILED)
 *                   DONE        (6) = exit and test edits (return PASS/FAIL)
 *                   COMPLETE    (14) = exit if all edits pass (return PASSED)
 *
 *                      function key reassignment by using
 *                         func(w) {td_window *w; w->td_c = x; return PASSED;}
 *
 *   Edit Returns:   FAILED      (0) = failed edit
 *                   PASSED      (1) = passed edit
 *                   EXIT/ABORT  (2) = exit always (return FAILED)
 *                   DONE        (6) = exit and test edits (return PASS/FAIL)
 *                   COMPLETE    (14) = exit if all edits passed (return PASSED
 */
#include <stdio.h>
#include "td.h"

#define K       table->td_k
#define C       table->td_c

long td_Fexit()   {return ABORT;}         /* useful returns                  */
long td_Fabort()  {return ABORT;}
long td_Fexec()   {return COMPLETE;}
long td_Fdone()   {return DONE;}
long td_Fup(w, t)    int w; td_driver_call *t; {t->td_c=FORMUP; return 1;}
long td_Fdown(w, t)  int w; td_driver_call *t; {t->td_c=FORMDOWN; return 1;}
long td_Fhome(w, t)  int w; td_driver_call *t; {t->td_c=FORMHOME; return 1;}

td_driver(table, help)
register td_driver_call *table;           /* all call parmeters              */
unsigned char *help;                      /* help file name                  */
{
  register td_driver_item *ptab;          /* driver item                     */
  register long k, max, fret, eret;       /* working values                  */
  register td_window *w;                  /* current window                  */
  long (*edit)(), source, new;            /* working values                  */
  long (*func)();                         /* function key                    */
  td_window *x;                           /* window above                    */
  long abs_row;                           /* absolute row                    */
   
/*
 *  Mark Low Fields as Edited
 */
  max = table->td_max - 1;                /* last table item                 */

  if (table->td_initial & RESETFLAGS)
  {
    ptab = table->td_dtab;                /* point to table                  */

    for (k = 0; k <= max; k++, ptab++)
    {
      w = ptab->td_field;

      if (k < table->td_start || !ptab->td_edit)
      {
        w->td_flag = PASSED;              /* mark passed edited              */
      }
      else w->td_flag = FAILED;           /* mark needs edit                 */
    }
  }
  ptab = table->td_dtab;
  w = ptab[table->td_start].td_field;     /* starting window                 */
  w->td_base = 0;                         /* top of window                   */
  if (td_align(w, 0, 1)) td_refresh();    /* align screen on start           */
/*
 *  Initialize Screen Image and Data Fields
 */
  if (table->td_initial & (CLEARFIELDS | LOADFIELDS))
  {
    for (k = 0; k <= max; k++, ptab++)
    {
      w = ptab->td_field;                 /* get field window                */

      if (table->td_initial & CLEARFIELDS)/* clear all fields                */
      {
        td_clear(w);                      /* clear screen field              */
      }
      w->td_base = 0;                     /* top of window                   */
      td_update(w);                       /* update field                    */
    }
    td_refresh();                         /* refresh screen                  */
  }
  if (table->td_initial & EXIT) return 0; /* display only                    */
  K = table->td_start;                    /* first field of table            */
  abs_row = td_rowx = 0;                  /* initial positions               */

  while (1)
  {
    ptab    = &table->td_dtab[K];         /* point to table item             */
    w       = ptab->td_field;             /* input window                    */
    edit    = ptab->td_edit;              /* edit subroutine                 */

    if (abs_row != w->td_row || td_rowx >= w->td_length) td_rowx = 0;
    abs_row = w->td_row;

    if (table->td_vattr) td_video(w, table->td_vattr);
    C = td_get_window(w, td_rowx, 0);
    if (table->td_vattr) td_video(w, NORMAL);

    if (td_change)                        /* field has changed               */
    {
      if (edit) w->td_flag = FAILED;      /* flag needs edit                 */
    }
    else if (C == UP && K > 0)
    {
      td_rowx = 0; K--;                   /* UP is allowed                   */
      continue;
    }
    if (C == HELP)
    {
      if (help) td_help(help, K);         /* do help screen                  */
      else td_message("*** No Help Available");
      continue;
    }
    if (K == max && C == ENTER)           /* special map CR                  */
    {
      if (table->td_last_cr) C = table->td_last_cr;
    }
    if (C >= F1 && C <= F8)               /* key before field                */
    {
      func = table->td_func[C - F1];      /* key action address              */
      if (func)
      {
        fret = (*func)(w, table);         /* key isnot null                  */
        if (fret == ABORT) return FAILED; /* exit now !!!                    */
        if (fret == NULL) continue;       /* process field again             */
        fret &= COMPLETE;                 /* remove low bit                  */
      }
      else fret = NULL;                   /* no function routine             */
    }
    else fret = NULL;                     /* not a function key              */

    if (w->td_flag != PASSED)             /* field needs edit                */
    {
      if (edit)
      {
        eret = (*edit)(w, table);         /* edit field                      */
        w->td_flag = eret & PASSED;       /* pass/fail flag                  */
        eret -= w->td_flag;               /* remove any flag                 */

        if (eret == ABORT) return FAILED; /* immediate exit                  */

        if (w->td_flag == FAILED)         /* failed edit test                */
        {
          if (fret == DONE) return FAILED;/* exit with failed                */
         
          putchar(BEEP);                  /* failure signal                  */
          if (C == UP && K > 0)
          {
            td_rowx = 0; K--;             /* UP is allowed                   */
          }
          continue;                       /* try input again                 */
        }
      }
      w->td_flag = PASSED;
      fret |= eret;                       /* combine flags                   */
    }
    if (fret & EDITS)                     /* exit requested                  */
    {
      td_change = 0;                      /* no input keyed                  */
      ptab = table->td_dtab;              /* list of fields                  */

      for (K = 0; K <= max; K++, ptab++)  /* find first to edit              */
      {
        w = ptab->td_field;               /* window of field                 */
        if (w->td_flag == FAILED)         /* needs edit                      */
        {
          edit = ptab->td_edit;
          if (edit)
          {
            w->td_flag = (*edit)(w, table) & PASSED ? PASSED : FAILED;

            if (w->td_flag == FAILED)     /* failed edit                     */
            {
              if (fret == DONE) return FAILED;
              putchar(BEEP);
              break;
            }
          }
          else w->td_flag = PASSED;
        }
      }
      if (K > max) return PASSED;         /* passed all edits                */
      continue;                           /* go to input field               */
    }
    switch (C)                            /* process terminator              */
    {
      case UP:       td_rowx = 0;         /* force top of window             */
      case LEFT:     if (K > 0) K--;      /* previous window                 */
        break;

      case RIGHT:
      case DOWN:     td_rowx = 0;         /* force top of window             */
      case ENTER:    if (K < max) K++;    /* next window                     */
        break;

      case FORMDOWN:
      case FORMUP:
      case FORMHOME: x = w->td_pwin;      /* window above                    */
        if (x->td_length <= 24) break;

        new = ((x->td_base + 23) / 24) * 24;/* base mod 24                   */

        if (C == FORMDOWN)      new += 24;
        else if (C == FORMUP)   new -= 24;
        else new = 0;

        if (new < 0) new = 0;

        if (new == x->td_base) break;
        ptab = table->td_dtab;

        for (k = 0; k <= max; k++, ptab++)/* find in window                  */
        {
          w = ptab->td_field;
          if (w->td_row >= new) break;
        }
        if (k > max) k = max;
        K = k;
        break;

        default:       break;
      }
  }
}

/* end of td_driver.c */



