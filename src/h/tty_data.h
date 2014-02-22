/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Screen driver keystroke definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *    7/7/93   |  tjt  Rewrite of screen driver.
 *-------------------------------------------------------------------------*/
#ifndef SD_DATA_H
#define SD_DATA_H

static char sd_data_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "iodefs.h"

unsigned char sd_in_byte[128] = {

  0, 0, 0, 0, 0, 0, 0, 0,
  DELETE, TAB, DOWN_CURSOR, UP_CURSOR, RIGHT_CURSOR, RETURN, 0, 0,
  0, 0, SEND, 0, 0, 0, 0, 0,
  0, 0, 0, ESCAPE,  0, 0, 0, 0,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f};

unsigned char sd_escape_byte[128] = {

  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x00 - 0x07                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x08 - 0x0f                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x10 - 0x17                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x18 - 0x1f                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x20 - 0x27                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x27 - 0x2f                     */
  0, F_KEY_1, F_KEY_2, F_KEY_3, F_KEY_4, F_KEY_5, F_KEY_6, F_KEY_7,
  F_KEY_8, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x40 - 0x47                     */
  0, 0, BACKWD, FORWRD, 0, 0, 0, 0,       /* 0x48 - 0x4f                     */
  PRINT, 0, 0, 0, 0, 0, 0, 0,             /* 0x50 - 0x57                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x58 - 0x5f                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x60 - 0x67                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x68 - 0x6f                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x70 - 0x77                     */
  0, 0, 0, SEND, 0, 0, 0, 0};             /* 0x78 - 0x7f                     */


unsigned char cursor_on[]     = {0x1b, 0x60, '1', 0};
unsigned char cursor_off[]    = {0x1b, 0x60, '0', 0};
unsigned char scroll_on[]     = {0x1b, 'O', 0};
unsigned char scroll_off[]    = {0x1b, 'N', 0};
unsigned char screen_on[]     = {0x1b, 0x60, '9', 0};
unsigned char screen_off[]    = {0x1b, 0x60, '8', 0};
unsigned char screen_80[]     = {0x1b, 0x60, ':', 0};
unsigned char screen_132[]    = {0x1b, 0x60, ';', 0};
unsigned char clear_screen[]  = {0x1b, '+', 0};
unsigned char clear_line[]    = {0x1b, 'T', 0};
unsigned char clear_forward[] = {0x1b, 'Y', 0};
unsigned char host_line_off[] = {0x1b, 'F', '\r', 0};
unsigned char revtabs_on[]    = {0x1b, 'A', '1', 't', 0};
unsigned char revtabs_off[]   = {0x1b, 'A', '1', '0', 0};

unsigned char keytabs[8][12] = {

  {0x1b,'z','0','1',' ',' ','E','X','I','T','\r',0},
  {0x1b,'z','1','2',' ',' ','H','E','L','P','\r',0},
  {0x1b,'z','2','3',' ',' ',' ','L','O','G','\r',0},
  {0x1b,'z','3','4',' ','P','R','I','N','T','\r',0},
  {0x1b,'z','4','5',' ','F','O','R','W','D','\r',0},
  {0x1b,'z','5','6',' ','B','K','W','R','D','\r',0},
  {0x1b,'z','6','7','\r',0,0,0,0,0,0,0},
  {0x1b,'z','7','8','\r',0,0,0,0,0,0,0,}};
   
unsigned char keyclear[8][5] = {

  {0x1b,'z','0','\r',0},
  {0x1b,'z','1','\r',0},
  {0x1b,'z','2','\r',0},
  {0x1b,'z','3','\r',0},
  {0x1b,'z','4','\r',0},
  {0x1b,'z','5','\r',0},
  {0x1b,'z','6','\r',0},
  {0x1b,'z','7','\r',0}};
   
unsigned char keyvalue[8][7] = {

  {0x1b,'z','@',0x1b,'1',0x7f,0},
  {0x1b,'z','A',0x1b,'2',0x7f,0},
  {0x1b,'z','B',0x1b,'3',0x7f,0},
  {0x1b,'z','C',0x1b,'4',0x7f,0},
  {0x1b,'z','D',0x1b,'5',0x7f,0},
  {0x1b,'z','E',0x1b,'6',0x7f,0},
  {0x1b,'z','F',0x1b,'7',0x7f,0},
  {0x1b,'z','G',0x1b,'8',0x7f,0}};
 
unsigned char reset_keyvalue[8][8] = {

  {0x1b,'z','@',0x01,'@','\r',0x7f,0},
  {0x1b,'z','A',0x01,'A','\r',0x7f,0},
  {0x1b,'z','B',0x01,'B','\r',0x7f,0},
  {0x1b,'z','C',0x01,'C','\r',0x7f,0},
  {0x1b,'z','D',0x01,'D','\r',0x7f,0},
  {0x1b,'z','E',0x01,'E','\r',0x7f,0},
  {0x1b,'z','F',0x01,'F','\r',0x7f,0},
  {0x1b,'z','G',0x01,'G','\r',0x7f,0}};
 
unsigned char sd_attr[16][4] = {

  {0x1b, 'H', '2', 0},                    /* upper left corner               */
  {0x1b, 'H', '3', 0},                    /* upper right corner              */
  {0x1b, 'H', '1', 0},                    /* lower left corner               */
  {0x1b, 'H', '5', 0},                    /* lower right corner              */
  {0x1b, 'H', '0', 0},                    /* upper tee                       */
  {0x1b, 'H', '4', 0},                    /* left  tee                       */
  {0x1b, 'H', '9', 0},                    /* right tee                       */
  {0x1b, 'H', '=', 0},                    /* lower tee                       */
  {0x1b, 'H', '8', 0},                    /* cross                           */
  {0x1b, 'H', ':', 0},                    /* horizontal bar                  */
  {0x1b, 'H', '6', 0},                    /* vertical bar                    */

  {0x1b, 'G', '0', 0},                    /* mormal video                    */
  {0x1b, 'G', '8', 0},                    /* underline video                 */
  {0x1b, 'G', 'p', 0},                    /* dim                             */
  {0x1b, 'G', '2', 0},                    /* blinking video                  */
  {0x1b, 'G', 't', 0}};                   /* reverse dim video               */

#endif

/* end of tty_data.h */
