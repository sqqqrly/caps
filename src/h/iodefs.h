/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Keyboard byte definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/10/93   |  tjt  Revised.
 *  04/19/97   |  tjt  Change attributes and function keys.
 *-------------------------------------------------------------------------*/
#ifndef IODEFS_H
#define IODEFS_H

static char iodefs_h[] = "%Z% %M% %I% (%G% - %U%)";

#define streql(x, y)       (strcmp((x), (y)) == 0)
#define movebytes(x, y, z)  memcpy((y), (x), (z))

/*-------------------------------------------------------------------------*
 * General Purpose Key Definitions            
 *-------------------------------------------------------------------------*/

#define BEEP            0x07
#define LEFT_CURSOR     0x08
#define TAB             0x09
#define LINE_FEED       0x0a
#define LF              0x0a
#define NEWLINE         0x0a
#define DOWN_CURSOR     0x0a
#define UP_CURSOR       0x0b
#define RIGHT_CURSOR    0x0c
#define RETURN          0x0d
#define ENTER           0x0d
#define CR              0x0d
#define ESCAPE          0x1b
#define SPACE           0x20
#define FILL            0x5f
#define DELETE          0x7F
#define SEND            0xfd
#define LAST_CHAR       0xaf              /* allow lanaguage bytes           */

/*-------------------------------------------------------------------------*
 * Function Key Definitions  
 *-------------------------------------------------------------------------*/
 
#define F_KEY_1         0xf1
#define F_KEY_2         0xf2
#define F_KEY_3         0xf3
#define F_KEY_4         0xf4
#define F_KEY_5         0xf5
#define F_KEY_6         0xf6
#define F_KEY_7         0xf7
#define F_KEY_8         0xf8

#define EXIT            F_KEY_1
#define HELP            F_KEY_2
#define LOG             F_KEY_3
#define PRINT           F_KEY_4
#define FORWRD          F_KEY_5
#define BACKWD          F_KEY_6
#define INSERT_LINE     F_KEY_7
#define DELETE_LINE     F_KEY_8

/*-------------------------------------------------------------------------*
 *  Screen Attributes
 *-------------------------------------------------------------------------*/

#define ULC            0xe0
#define URC            0xe1
#define LLC            0xe2
#define LRC            0xe3
#define UTEE           0xe4
#define LTEE           0xe5
#define RTEE           0xe6
#define BTEE           0xe7
#define CROSS          0xe8
#define HORT           0xe9
#define VERT           0xea
#define NORMAL         0xeb
#define UNDERLINE      0xec
#define DIM            0xed
#define BLINK          0xee
#define REVDIM         0xef

#endif

/* end of iodefs.h */
