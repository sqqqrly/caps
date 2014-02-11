/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Report writer definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/16/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
#ifndef TEXT_SPEC_H
#define TEXT_SPEC_H

static char text_spec_h[] = "%Z% %M% %I% (%G% - %U%)";

/* 
 *
 *  Structure of Remarks, Pick Text, System Field Specifications
 *
 *      Format of remarks.s, picktext.s, and system_fields.s
 *
 *      name:   length: use             remarks.s & picktext.s
 *
 *      no:     name:   length: use     system_fields.s
 *
 *      Text File       Text No
 *      ---------       -------
 *      Op Codes         0 - 31
 *      System Fields   32 - 63
 *      Remarks Fields  64
 *      Pick Text Fld   65
 *
 *
 *      Text File       Use Code
 *      ---------       --------
 *      Remarks Text    x =  1 = No Print Flag
 *                      t =  2 = Tote Label Count
 *                      s =  4 = Shipping Label Count
 *                      p =  8 = Packing List Count
 *                      n = 32 = Numeric Field
 *      Pick Text       x =  1 = No Print Flag
 *                      b = 16 = Backorder Flag
 *      System Fields   n = 32 = Numeric Field
 *                      r = 64 = Result Field
 *      
 */

struct text_spec_item
{
  char text_name[17];                     /* Field Name                      */
  unsigned char text_no;                  /* Field Number                    */
  unsigned char text_len;                 /* Field Length                    */
  unsigned char text_use;                 /* Field Use                       */
  short         text_offset;              /* Field Offset                    */
};

/*
 *  Text Specification Defines
 */

#define NOPRINT 1
#define TLC     2
#define SLC     4
#define PLC     8
#define BACKORD 16
#define NUMERIC 32
#define RESULT  64

/*
 * Structure of Report Specifications
 *
 *      Op Code p1      p2
 *      ------- ------  ------  
 *      form    length  width   
 *      area    size    0
 *      repeat  repeat  lines   
 *
 *      arith   0       0
 *      field   length  offset
 *      field   0       0
 *
 *      if      0       0
 *      field   length  offset
 *      field   length  offset
 *
 *      print   row     col
 *      field   length  offset
 *
 *      Op Code         Assignment
 *      -------         ----------
 *      ops               0 - 31
 *      sysflds          32 - 63
 *      rmkstext         64
 *      picktext         65
 *      lits             66
 */

struct report_spec_item
{
  unsigned char  text_op;                 /* Operation Code                  */
  unsigned char  text_p1;                 /* Parm 1                          */
  unsigned short text_p2;                 /* Parm 2                          */
};

/*
 *  System Field Defines
 */

#define FORM    0
#define AREA    1
#define MOVE    2
#define ADD     3
#define SUB     4
#define MULT    5
#define DIV     6
#define IFEQ    7
#define IFUNEQ  8
#define IFLESS  9
#define IFMORE  10
#define IFLAST  11                        /* last page test                  */
#define ELSE    12
#define ENDIF   13
#define PRINT   26
#define COPIES  27                      /* generated if any copy count fields*/
#define IFPRT   28                        /* generated if no print in remarks*/
#define IFFLAG  29                       /* generated if a no print pick text*/
#define REPEAT  30                        /* generated at end of repeat area */
#define END     31                        /* appended to end of report spec  */
#define ON      32
#define CON     33
#define GROUP   34
#define PRIORITY 35
#define SKU     36
#define MOD     37
#define QUAN    38
#define DESCR   39
#define FGROUP  40
#define UM      41
#define IPQTY   42
#define CPACK   43
#define BSLOC   44
#define ABSLOC  45
#define ALTID   46
#define QTY     47
#define RESTOCK 48
#define LCAP    49
#define STKLOC  50
#define PLIDX   51
#define UDATE   52
#define MDATE   53
#define EDATE   54
#define PICKED  55
#define SHORT   56
#define PAGE    57
#define DOLLAR1 58
#define DOLLAR2 59
#define WORK1   60
#define WORK2   61
#define LINE    62
#define SEQUENCE        63
#define RMKS    64
#define PTEXT   65
#define LITS    66

#endif

/* end of text_spec.h */
