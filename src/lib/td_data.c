/*
 *  td_data.c
 *
 *  Terminal Driver Using Windows and Panes
 *
 */
#include <stdio.h>
#include "td.h"

/*
 *  Table to Check and Encode First Byte
 */
unsigned char td_in_byte[256] = {

  0, 0, 0, 0, 0, 0, 0, 0,                 /* 00-07                           */
  LEFT, TAB, DOWN, UP, RIGHT, ENTER, 0, 0,/* 08-0f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 10-17                           */
  0, 0, 0, ESC, 0, 0, HOME, 0,            /* 18-1f                           */
  0x20, '!', 0x22, '#', '$', '%', '&', 0x27,/* 20-27                         */
  '(', ')', '*', '+', ',', '-', '.', '/', /* 28-2f                           */
  '0', '1', '2', '3', '4', '5', '6', '7', /* 30-37                           */
  '8', '9', ':', ';', '<', '=', '>', HELP,/* 38-3f                           */
  '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', /* 40-47                           */
  'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', /* 48-4f                           */
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', /* 50-57                           */
  'X', 'Y', 'Z', '[', 0, ']', 0, '_',     /* 58-5f                           */
  0, 'a', 'b', 'c', 'd', 'e', 'f', 'g',   /* 60-67                           */
  'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', /* 68-6f                           */
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', /* 70-77                           */
  'x', 'y', 'z', '{', 0, '}', 0, DELETE,  /* 78-7f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 80-87                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 88-8f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 90-87                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 98-9f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* a0-a7                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* a8-af                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* b0-b7                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* b8-bf                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* c0-c7                           */
  0, 0, FORMUP, FORMDOWN, 0, 0, 0, 0,     /* c8-cf                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* d0-d7                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* d8-df                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* e0-e7                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* e8-ef                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* f0-f7                           */
  0, 0, 0, FORMHOME, 0, 0, 0, 0};         /* f8-ff                           */

/*
 *  Table to Check and Encode Second Byte if Escape
 */
unsigned char td_in_escape[128] = {

  0, F1, F2, F3, F4, F5, F6, F7,          /* 00-07                           */
  F8, 0, 0, 0, 0, 0, 0, 0,                /* 08-1f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 10-17                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 18-1f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 20-27                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 28-2f                           */
  0, 0, 0, 0, 0, 0, 0, SEND,              /* 30-37                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 38-3f                           */
  0, 0, 0, 0, 0, OPENLINE, 0, 0,          /* 40-47                           */
  0, SHIFTAB, PREVPAGE, NEXTPAGE, 0, 0, 0, 0,/* 48-4f                        */
  PRINT, 0, DELINE, 0, 0, 0, 0, 0,        /* 50-57                           */
  0, CLRLINE, 0, 0, 0, 0, 0, 0,           /* 58-5f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 60-67                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 68-6f                           */
  0, INSERT, OVERSTRIKE, 0, 0, 0, 0, 0,   /* 70-77                           */
  0, 0, 0, SHIFTHOME, 0, 0, 0, 0};        /* 78-7f                           */
/*
 *  Table of Graphics and Video Attributes
 */
unsigned char td_attr[16][4] = {

  {ESC, 'H', '2', 0},                     /* upper left corner               */
  {ESC, 'H', '3', 0},                     /* upper right corner              */
  {ESC, 'H', '1', 0},                     /* lower left corner               */
  {ESC, 'H', '5', 0},                     /* lower right corner              */
  {ESC, 'H', '0', 0},                     /* upper tee                       */
  {ESC, 'H', '4', 0},                     /* left tee                        */
  {ESC, 'H', '9', 0},                     /* right tee                       */
  {ESC, 'H', '=', 0},                     /* lower tee                       */
  {ESC, 'H', '8', 0},                     /* cross                           */
  {ESC, 'H', ':', 0},                     /* horizonal line                  */
  {ESC, 'H', '6', 0},                     /* vertical line                   */
  {ESC, 'G', '0', 0},                     /* normal video                    */
  {ESC, 'G', '8', 0},                     /* underline video                 */
  {ESC, 'G', 'p', 0},                     /* dim video                       */
  {ESC, 'G', '2', 0},                     /* blink video                     */
  {ESC, 'G', 't', 0}};                    /* reverse dim video               */
/*
 *  Function Keys
 */
unsigned char td_func[] = {

  ESC, 'z', '@', ESC, 1, DELETE,
  ESC, 'z', 'A', ESC, 2, DELETE,
  ESC, 'z', 'B', ESC, 3, DELETE,
  ESC, 'z', 'C', ESC, 4, DELETE,
  ESC, 'z', 'D', ESC, 5, DELETE,
  ESC, 'z', 'E', ESC, 6, DELETE,
  ESC, 'z', 'F', ESC, 7, DELETE,
  ESC, 'z', 'G', ESC, 8, DELETE, 0};

/*
 *  Key Tabs
 */
unsigned char td_tab_key[] = {

  ESC, 'z', '0', '1', 0x20, 0x20, 0x20, 'E',  'X', 'I',  'T',   CR,
  ESC, 'z', '1', '2', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, CR,
  ESC, 'z', '2', '3', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, CR,
  ESC, 'z', '3', '4', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, CR,
  ESC, 'z', '4', '5', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, CR,
  ESC, 'z', '5', '6', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, CR,
  ESC, 'z', '6', '7', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, CR,
  ESC, 'z', '7', '8', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, CR, 0};

unsigned char td_tab_clear[] = {

  ESC, 'z', '0', CR,
  ESC, 'z', '1', CR,
  ESC, 'z', '2', CR,
  ESC, 'z', '3', CR,
  ESC, 'z', '4', CR,
  ESC, 'z', '5', CR,
  ESC, 'z', '6', CR,
  ESC, 'z', '7', CR, 0};

/*
 *  Display Functions
 */
unsigned char td_control[11][6] = {

  {ESC, '+', 0, 0, 0, 0},                 /* clear screen                    */
  {ESC, 0x60, ':', 0, 0, 0},              /* width 80 columns                */
  {ESC, 0x60, ';', 0, 0, 0},              /* width 132 columns               */
  {ESC, 0x60, '0', 0, 0, 0},              /* cursor off                      */
  {ESC, 0x60, '1', 0, 0, 0},              /* cursor on                       */
  {ESC, 0x60, '8', 0, 0, 0},              /* screen off                      */
  {ESC, 0x60, '9', 0, 0, 0},              /* screen on                       */
  {ESC, 'N', 0, 0, 0, 0},                 /* scroll off                      */
  {ESC, 'O', 0, 0, 0, 0},                 /* scroll on                       */
  {ESC, 'A', '1', '0', 0, 0},             /* tabs off - normal               */
  {ESC, 'A', '1', 't', 0, 0}};            /* tabs on reverse dim             */

/*
 *  Current Screen Areas
 */

td_window td_crt = {0,0,0,24,24,132,0,0,td_screen,'t','x'};

short td_first[24] = {0};                 /* first change in row             */
short td_last[24] = {0};                  /* last change in row              */
unsigned char td_screen[3168];            /* current screen                  */

long td_row_offset = 0;                   /* window in screen                */
long td_col_offset = 0;                   /* window in screen                */

long td_rowx = 0;                         /* current cursor                  */
long td_colx = 0;
long td_change = 0;                       /* data was input                  */
long td_col_zero = 1;                     /* UP/DOWN to column 0             */
long td_no_wrap = 1;                      /* no automatic next row           */

char td_last_message[50] = {0};           /* last sreen message              */

long td_lock_flag = 0;                    /* exclusion flag                  */
long td_need_refresh = 0;                 /* screen needs update             */

/* end of td_data.c */
