/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    ANSI Screen driver keystroke definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *    7/7/93   |  tjt  Rewrite of screen driver.
 *  12/26/94   |  tjt  Revised for ansi terminals.
 *-------------------------------------------------------------------------*/
#ifndef ANSI_DATA_H
#define ANSI_DATA_H

static char ansi_data_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "iodefs.h"

unsigned char sd_in_byte[128] = {

  0, 0, 0, 0, 0, 0, 0, 0,
  DELETE, TAB, 0, 0, 0, RETURN, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
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
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x28 - 0x2f                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x30 - 0x37                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x38 - 0x3f                     */
  0,UP_CURSOR,DOWN_CURSOR,RIGHT_CURSOR,LEFT_CURSOR,0,0,FORWRD, /* 0x40 - 0x47*/
  SEND,BACKWD,0,0,0,F_KEY_1,F_KEY_2,F_KEY_3,                   /* 0x48 - 0x4f*/
  F_KEY_4,F_KEY_5,F_KEY_6,F_KEY_7,F_KEY_8,0,0,0,               /* 0x50 - 0x57*/
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x58 - 0x5f                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x60 - 0x67                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x68 - 0x6f                     */
  0, 0, 0, 0, 0, 0, 0, 0,                 /* 0x70 - 0x77                     */
  0, 0, 0, 0, 0, 0, 0, 0};                /* 0x78 - 0x7f                     */

unsigned char clear_screen[]  = {0x1b, '[', '2', 'J', 0};
unsigned char clear_line[]    = {0x1b, '[', 'K', 0};

unsigned char sd_attr[16][12] = {

  {0xda, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* upper left corner            */
  {0xbf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* upper right corner           */
  {0xc0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* lower left corner            */
  {0xd9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* lower right corner           */
  {0xc2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* upper tee                    */
  {0xc3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* left  tee                    */
  {0xb4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* right tee                    */
  {0xc1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* lower tee                    */
  {0xc5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* cross                        */
  {0xc4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* horizontal bar               */
  {0xb3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   /* vertical bar                 */

  {0x1b,'[','0',';','3','7',';','4','4','m',0x20,0},  /* normal video        */
  {0x1b,'[','1',';','3','5',';','4','4','m',0x20,0},  /* underline video     */
  {0x1b,'[','0',';','3','6',';','4','4','m',0x20,0},  /* dim                 */
  {0x1b,'[','5','m', 0x20, 0, 0, 0, 0, 0, 0, 0},      /* blinking video      */
  {0x1b,'[','0',';','3','4',';','4','6','m',0x20,0}}; /* reverse dim video   */

unsigned char w370_func[] =
"0;1;0|74/1B5B46;65/1B5B47;75/1B5B49;64/1B5B4C;\
42/1B5B4D;51/1B5B4E;52/1B5B4F;53/1B5B50;54/1B5B51;37/1B5B52;\
38/1B5B53;39/1B5B54;40/1B5B55;41/1B5B56;43/1B5B57;44/1B5B58";

#endif

/* end of ansi_data.h */
