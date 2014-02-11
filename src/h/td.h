/*-------------------------------------------------------------------------*
 *  Copyright (c) 1989 - 1993 PTW Systems, Inc. - All rights reserved.
 *
 *  THIS IS UNPUBLISHED SOURCE CODE OF PTW SYSTEMS, INC.
 *  This copyright notice, above, does not evidence any actual
 *  or intended publication of such source code.
 *-------------------------------------------------------------------------*/
/*
 *  td.h
 *
 *  Terminal Driver Using Windows
 */
#ifndef TDH

/*
 *  Definition of a Window
 */
typedef struct td_window_item
{
  struct td_window_item *td_pwin;         /* window pane above               */
  short td_row;                           /* row in pane above               */
  short td_col;                           /* col in pane above               */
  short td_size;                          /* length of pane                  */
  short td_length;                        /* lines in window                 */
  short td_width;                         /* columns in window               */
  short td_base;                          /* position in pane                */
  short td_pbase;                         /* preferred base above            */
  unsigned char *td_image;                /* window data area                */
  char td_type;                           /* data type                       */
  char td_form;                           /* data store length               */
  short td_flag;                          /* driver edit flag                */

} td_window;

/*
 *  Screen Driver Table
 */
typedef struct
{
  td_window *td_field;                    /* window item                     */
  long (*td_edit)();                      /* window edit                     */

} td_driver_item;

/*
 *  Screen Driver Calling Parameter Structure
 */
typedef struct
{
  td_driver_item *td_dtab;                /* window list                     */
  short td_start;                         /* start field                     */
  short td_max;                           /* number of windows               */
  short td_initial;                       /* initial action                  */
  unsigned char  td_vattr;                /* video attribute                 */
  unsigned char  td_last_cr;              /* reassign last CR                */
  long  (*td_func[8])();                  /* function keys                   */
  short td_k;                             /* current field                   */
  unsigned char td_c;                     /* current exit byte               */

} td_driver_call;

extern unsigned char td_control[11][6];

#define td_screen_clear()  td_put_string(td_control[0])
#define td_set80()         td_put_string(td_control[1])
#define td_set132()        td_put_string(td_control[2])
#define td_cursor_off()    td_put_string(td_control[3])
#define td_cursor_on()     td_put_string(td_control[4])
#define td_screen_off()    td_put_string(td_control[5])
#define td_screen_on()     td_put_string(td_control[6])
#define td_scroll_off()    td_put_string(td_control[7])
#define td_scroll_on()     td_put_string(td_control[8])
#define td_tabs_off()      td_put_string(td_control[9])
#define td_tabs_on()       td_put_string(td_control[10])

#undef  NULL
#define NULL            0x00
#define SOH             0x01
#define BEEP            0x07              /* normal keys                     */
#define BS              0x08
#define CR              0x0d
#define ESC             0x1b
#define SPACE           0x20
#define DELETE          0x7f

#define F1              0x81              /* function keys                   */
#define F2              0x82
#define F3              0x83
#define F4              0x84
#define F5              0x85
#define F6              0x86
#define F7              0x87
#define F8              0x88

#define LEFT            0x89              /* input exit keys                 */
#define RIGHT           0x8a
#define UP              0x8b
#define DOWN            0x8c
#define FORMUP          0x8d              /* control prev page               */
#define FORMDOWN        0x8e              /* control next page               */
#define FORMHOME        0x8f              /* control shift home              */
      
#define ENTER           0x90              /* special input functions         */
#define TAB             0x91
#define SHIFTAB         0x92
#define HOME            0x93
#define SHIFTHOME       0x94
#define PRINT           0x95
#define SEND            0x96
#define OPENLINE        0x97
#define DELINE          0x98
#define CLRLINE         0x99
#define INSERT          0x9a
#define OVERSTRIKE      0x9b
#define PREVPAGE        0x9c
#define NEXTPAGE        0x9d
#define HELP            0x9f

#define ULC             0xe0              /* graphics symbols                */
#define URC             0xe1
#define LLC             0xe2
#define LRC             0xe3
#define UTEE            0xe4
#define LTEE            0xe5
#define RTEE            0xe6
#define BTEE            0xe7
#define CROSS           0xe8
#define HORT            0xe9
#define VERT            0xea
#define NORMAL          0xeb
#define UNDERLINE       0xec
#define DIM             0xed
#define BLINK           0xee
#define REVDIM          0xef


/*
 *    8  4  2  1
 *    x  x  x  x
 *    |  |  |  |
 *    |  |  |  +---- PASSED / FAILED
 *    |  |  +------- EXIT        (depends on flags, below) 
 *    |  +---------- EDITS       (do edits until fails)
 *    +------------- IFPASSED    (exit only on all passed)
 */

#define FAILED          0                 /* edit/function returns           */
#define PASSED          1
#define EXIT            2                 /* immediate exit                  */
#define ABORT           2                 /* immediate exit                  */
#define EDITS           4
#define IFPASSED        8
#define DONE            6                 /* exit + edits                    */
#define COMPLETE        14                /* exit + edits + ifpassed         */

/*
 *    8  4  2  1
 *    x  x  x  x
 *    |  |  |  |
 *    |  |  |  +---- RESETFLAGS  (clear active edit flags)
 *    |  |  +------- EXIT        (exit after initialization only)
 *    |  +---------- CLEARFIELDS (clear all fields)
 *    +------------- LOADFIELDS  (load all fields)
 */
#define RESETFLAGS      1
#define CLEARFIELDS     4
#define LOADFIELDS      8
#define CLEAR           5                 /* clear + reset                   */
#define CLEARONLY       6                 /* clear + exit                    */
#define LOAD            9                 /* load  + reset                   */
#define LOADONLY        10                /* load  + exit                    */

extern td_window     td_crt;
extern unsigned char td_screen[3168];
extern long          td_rowx;
extern long          td_colx;
extern long          td_change;
extern long          td_col_zero;
extern long          td_no_wrap;

#undef  putchar
#define putchar(x)         fputc(x, stdout)

#define td_refresh()       td_refresh_screen(1)
#define td_display(x,y)    {td_put(x,y);td_update(x);td_refresh_screen(1);}
#define td_init(x)         {td_screen_off();td_update(x);\
                             td_refresh_screen(1);td_screen_on();}
#define td_error(x)        {putchar(BEEP);putchar(BEEP);td_message(x);}
#define td_clearf(x)       {td_clear(x);td_update(x);td_refresh_screen(1);}
#define td_key(x)          td_get_window(x,0,0)
#define td_keyin(x,y)      {td_get_window(x,0,0);td_get(x,y);}
#define td_len(x)          td_window_size(x)

#define td_Fnull           0
extern long td_Fexit();
extern long td_Fabort();
extern long td_Fexec();
extern long td_Fdone();
extern long td_Fup();
extern long td_Fdown();

#define TDH
#endif

/* end of td.h  */



