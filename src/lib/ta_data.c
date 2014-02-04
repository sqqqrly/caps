/*
 *  ta_data.c
 *
 *  ANSI Terminal Driver Using Windows and Panes
 *
 */
#include <stdio.h>
#include "td.h"
/*
 *  Table to Check and Encode First Byte
 */
unsigned char ta_in_byte[128] = {

  0, 0, 0, 0, 0, 0, 0, 0,                 /* 00-07                           */
  LEFT, TAB, 0, 0, 0, ENTER, 0, 0,        /* 08-0f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 10-17                           */
  0, 0, 0, ESC, 0, 0, 0, 0,               /* 18-1f                           */
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
  'x', 'y', 'z', '{', 0, '}', 0, DELETE}; /* 78-7f                           */

/*
 *  Table to Check and Encode Second Byte if Escape
 */
unsigned char ta_in_escape[128] = {

  0, 0, 0, 0, 0, 0, 0, 0,                 /* 00-07                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 08-1f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 10-17                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 18-1f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 20-27                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 28-2f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 30-37                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 38-3f                           */
  0,UP,DOWN,RIGHT,LEFT,0,SHIFTHOME,NEXTPAGE,/* 40-47                         */
  HOME,PREVPAGE,0,0,INSERT,F1,F2,F3,      /* 48-4f                           */
  F4,F5,F6,F7,F8,OPENLINE,DELINE,CLRLINE, /* 50-57                           */
  SEND, 0, 0, 0, 0, 0, 0, 0,              /* 58-5f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 60-67                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 68-6f                           */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 70-77                           */
  0, 0, 0, 0, 0, 0, 0, 0};                /* 78-7f                           */

/*
 *  Table of Graphics and Video Attributes
 */
unsigned char ta_attr[16][12] = {

  {0xda, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* upper left corner               */
  {0xbf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* upper right corner              */
  {0xc0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* lower left corner               */
  {0xd9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* lower right corner              */
  {0xc2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* upper tee                       */
  {0xc3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* left tee                        */
  {0xb4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* right tee                       */
  {0xc1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* lower tee                       */
  {0xc5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* cross                           */
  {0xc4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* horizonal line                  */
  {0xb3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},/* vertical line                   */
  {0x1b,'[','0',';','3','7',';','4','4','m',0x20,0},  /* normal video        */
  {0x20,0x1b,'[','1',';','3','5',';','4','4','m',0},  /* underline video     */
  {0x20,0x1b,'[','1',';','3','4',';','4','4','m',0},  /* dim video           */
  {0x1b,'[','5','m',0x20, 0, 0, 0, 0, 0, 0, 0},       /* blink video         */
  {0x20,0x1b,'[','0',';','3','0',';','4','6','m',0}}; /* reverse dim video   */
/*
 *  Key Tabs
 */
unsigned char ta_tab_key[8][9] = {
  {"1 EXIT  "},
  {"        "},
  {"        "},
  {"        "},
  {"        "},
  {"        "},
  {"        "},
  {"        "}};
  

unsigned char ta_tab_clear[] = {

  0x1b,'[','2','5',';','1','H',0x1b,'[','K',0};

/*
 *  Display Functions
 */
unsigned char ta_control[11][6] = {

  {0x1b,'[','2','J', 0, 0},               /* clear screen                    */
  {0, 0, 0, 0, 0, 0},                     /* width 80 columns                */
  {0, 0, 0, 0, 0, 0},                     /* width 132 columns               */
  {0, 0, 0, 0, 0, 0},                     /* cursor off                      */
  {0, 0, 0, 0, 0, 0},                     /* cursor on                       */
  {0, 0, 0, 0, 0, 0},                     /* screen off                      */
  {0, 0, 0, 0, 0, 0},                     /* screen on                       */
  {0, 0, 0, 0, 0, 0},                     /* scroll off                      */
  {0, 0, 0, 0, 0, 0},                     /* scroll on                       */
  {0, 0, 0, 0, 0, 0},                     /* tabs off - normal               */
  {0, 0, 0, 0, 0, 0}};                    /* tabs on reverse dim             */

unsigned char w370_func[] =
"0;1;0|74/1B5B46;65/1B5B47;75/1B5B49;64/1B5B4C;\
42/1B5B4D;51/1B5B4E;52/1B5B4F;53/1B5B50;54/1B5B51;37/1B5B52;\
38/1B5B53;39/1B5B54;40/1B5B55;41/1B5B56;43/1B5B57;44/1B5B58";


/* end of ta_data.c */
