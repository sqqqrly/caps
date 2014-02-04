/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Transaction file definitions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/16/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
#ifndef XF_STRUCTS_H
#define XF_STRUCTS_H

static char xf_structs_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*                  structures for transaction reports                     */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/* structures for add,change,delete SKU, assign,deassign PM, receipts,
   adjustments, physical inventory.
*/

struct SKU_image
{
  char descr[25];                         /* SKU description                 */
  char famgrp[5];                         /* family group                    */
  char uofms[3];                          /* unit fo measure                 */
  char ipkqty[3];                         /* inner pack quantity             */
  char casepk[3];                         /* case pack                       */
  char bsloc[6];                          /* backup stock loc.               */
  char absloc[6];                         /* alt. backup stk. loc.           */
};

struct PM_image
{
  char qtyonh[5];                         /* quantity on hand                */
  char restock[5];                        /* restock point                   */
  char lanecap[5];                        /* lane capacity                   */
  char CAPSloc[6];                        /* CAPS stock loc.                 */
  char plindex;                           /* pick locaton index              */
  char piflag;                            /* pick inhibit flag               */
};

struct SKU_data
{
  char SKU[15];                           /* SKU                             */
  struct SKU_image SKU_after;
  struct SKU_image SKU_before;
};

struct PM_data
{
  char PM[5];                             /* PM                              */
  char aSKU[15];                          /* associated SKU                  */
  struct PM_image PM_after;
  struct PM_image PM_before;
};

struct AUTO_data                       /* F013187  AUTO_image structure added*/
{
  char auto_sku[15];                      /* SKU number-id                   */
  char auto_pm[5];                        /* absolute pm number on pmfile    */
  char auto_plidx;                        /* pick index number 0-9           */
  char auto_descr[25];                    /* SKU description field           */
};

struct xf_data
{
  char dts[12];                           /* date time stamp                 */
  char code;                              /* menu selection                  */
  char oprname[8];                        /* operator name                   */

  union skuorpm
  {
    struct AUTO_data auto_info;           /* F013187  added auto struct      */
    struct SKU_data sku_info;
    struct PM_data pm_info;
  }pmsku;
};

/* structures for insert and delete pick modules...                          */

struct ulr_data
{
  char cur_units[7];                      /* units - current                 */
  char cum_units[7];                      /* units - cumulative              */
  char cur_lines[7];                      /* lines - current                 */
  char cum_lines[7];                      /* lines - cumulative              */
  char cur_rcpts[7];                      /* receipts - current              */
  char cum_rcpts[7];                      /* receipts - cumulative           */
};

struct xf_PM_id
{
  char dts[12];                           /* date time stamp                 */
  char code;                              /* menu selection                  */
  char oprname[8];                        /* operator name                   */
  char dtf;                               /* delete transactions flag        */
  char PM_num[5];                         /* PM number                       */

  union insordel
  {
    char bay_num[2];                      /* bay number                      */
    struct ulr_data ulr;                  /* units-lines-receipts data       */
  }delins;
};

/***********************************************************************     */
/*                                                                           */
/*      xf_structure_notes                                                   */
/*                                                                           */
/*      detailed structure of ptf file records...                            */
/*                                                                           */
/*      abbreviations used:                                                  */
/*                                                                           */
/*              SKU             Stock Keeping Unit                           */
/*              PM              Pick Module                                  */
/*              rap             Receipts,Adjustments,Physical Inventory      */
/*              acd SKU         add,change,delete SKU                        */
/*              acd PM          assign,change,deassign PM                    */
/*              batch           Batch Receipts                               */
/*              ins             insert PM                                    */
/*              del             delete PM                                    */
/*              auto           auto assign,deassign,delete PM and/or SKU     */
/*                                                                           */
/***********************************************************************     */
/*
record types:                   bytes           contents
----------------------------------------------------------------------------
all:                            000-011         date/time stamp YYMMDDHHMMSS
                                012             menu selection code from 7.0
                                013-020         operator name
           
auto:                           021-035         SKU left justified
                                036-040         PM number
                                041-041         pick location index number
                                042-066         SKU description field

                                067-067         record terrminator


rap,acd PM,batch                021-035         associated SKU
                                036-040         PM number
        (old image)             041-045         quantity on hand
                                046-050         restock point
                                051-055         lane capacity
                                056-061         CAPS stock location
                                062             pick location index
                                063             pick inhibit flag

a PM:                           064             record terminator

rap,batch,c PM:(new image)      064-068         quantity on hand
                                069-073         restock point
                                074-078         lane capacity
                                079-084         CAPS stock location
                                085             pick location index
                                086             pick inhibit flag

rap,c PM:                       087             record terminator

batch:                          087-090         batch number
                                091             record terminator


record types:                   bytes           contents
----------------------------------------------------------------------------
d PM:                           064-070         current units
                                071-077         cumulative units
                                078-084         current lines
                                085-091         cumulative lines
                                092-098         current receipts
                                099-105         cumulative receipts
                                106             record terminator

ins:                            021-021         delete trans flag 'y' or 'n'
                                022-026         PM num before inserted
                                026             record terminator

del:                            021-021         delete trans flag 'y' or 'n'
                                022-026         PM number
                                026             record terminator

all:                            000-011         date/time stamp YYMMDDHHMMSS
                                012             menu selection code from 7.0
                                013-020         operator name

ac SKU:                         021-035         SKU number
                (old image)     036-060         description
                                061-065         family group
                                066-068         unit of measure
                                069-071         inner pack quantity
                                072-074         case pack
                                075-080         back up stock location
                                081-086         alternate back up stock loc.

a SKU:                          087             record terminator

c SKU:          (new image)     087-111         description
                                112-116         family group
                                117-119         unit of measure
                                120-122         inner pack quantity
                                123-125         case pack
                                126-131         back up stock location
                                132-137         alternate back up stock loc.
                                138             record terminator

d SKU:                          021-021        rev trans flag init to 'n'  
                                022-036        SKU number      
                                037-037        record terminator

*/

#endif

/* end of xf_structs.h */
