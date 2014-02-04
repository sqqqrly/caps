/*---------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Transaction file routines.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *   7/17/93   |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char xf_routines_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      xf_routines.c                                                   */
/*                                                                      */
/*      these routines write transaction file records and pending       */
/*      transaction file records to the appropriate files. the current  */
/*      files used are:                                                 */
/*                      dat/ptf                                         */
/*                      dat/ppf                                         */
/*      also, these routines allow a record to be placed into a         */
/*      structure defined in xf_structs.c so they can be easily         */
/*      used by report programs.                                        */
/*                                                                      */
/*      see the file xf_structure_notes for a complete item by item     */
/*      listing of the contents of each type of xaction file record.    */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "xf_structs.h"
#include "pxf_structs.h"
#include "xf_defs.h"
#include "iodefs.h"

extern FILE *ptf_fd;
extern FILE *ppf_fd;

/* xf_dts():
 *
 *   this routine prints the date/time stamp, the menu selection
 *   code, and the operator name to the transaction file.
 */

#define COLON ':'

xf_dts(code)
char code;
{
  long *dts, now;
  
  time(&now);

  dts = localtime(&now);

  fprintf(ptf_fd,"%02d%02d%02d%02d%02d%02d%c%-8.8s",
  dts[5], dts[4]+1, dts[3], dts[2], dts[1], dts[0], code, getenv("OPERATOR"));
}

/* xf_SKU():

   this routine writes the SKU to the transaction file.
*/

xf_SKU(buf)
char *buf;
{
  fprintf(ptf_fd,"%-15.15s",buf);
}

/* xf_PM():
 *
 *   this routine writes the associated SKU and the PM number
 *   to the transaction file.
 */

xf_PM(skubuf,pmbuf)
char *skubuf;
int  pmbuf;
{
  fprintf(ptf_fd,"%-15.15s%5.5d",skubuf,pmbuf);
}

/* xf_SKU_image():
 *
 *   this routine writes the SKU image to the transaction file.
 */

xf_SKU_image(idesc,ifmgrp,iuofm,iipq,icpk,ibsl,iabsl)
char *idesc,*ifmgrp,*iuofm,*iipq,*icpk,*ibsl,*iabsl;
{
  fprintf(ptf_fd,"%-25.25s%-5.5s%-3.3s%3.3s%3.3s%-6.6s%-6.6s",
  idesc,ifmgrp,iuofm,iipq,icpk,ibsl,iabsl);
}

/* xf_PM_image():
 *
 *   this routine writes the pick module image to the transaction file.
 */

xf_PM_image(iqty,irspoint,ilcap,istkloc,ipindex,iiflag)
char *istkloc, *iiflag;
int  iqty, irspoint, ilcap, ipindex;
{
  fprintf(ptf_fd,"%5.5d%5.5d%5.5d%-6s%1.1d%1.1s",
  iqty,irspoint,ilcap,istkloc,ipindex,iiflag);
}

/* xf_PM_his();
 *
 *   this routine writes the pick module movement history to the
 *   transaction file.
 */

xf_PM_his(cuu,cmu,cul,cml,cur,cmr)
int cuu, cmu, cul, cml, cur, cmr;
{
  fprintf(ptf_fd,"%7d%7d%7d%7d%7d%7d",cuu,cmu,cul,cml,cur,cmr);
}

/* pxf_dts():
 *
 *  this routine prints the date/time stamp, the menu selection
 *   code, and the operator name to the pending transaction file.
 */

pxf_dts(code)
char code;
{
  long *dts, now;
  
  time(&now);

  dts = localtime(&now);

  fprintf(ppf_fd,"%02d%02d%02d%02d%02d%02d%c%-8.8s",
  dts[5], dts[4]+1, dts[3], dts[2], dts[1], dts[0], code, getenv("OPERATOR"));
}

/* pxf_SKU():
 *
 *   this routine writes the SKU to the pending transaction file.
 */

pxf_SKU(buf)
char *buf;
{
  fprintf(ppf_fd,"%-15.15s",buf);
}

/* pxf_PM():
 *
 *   this routine writes the PM number and possibly the bay number
 *   to the pending transaction file. it is used to write both
 *   pending insert PM xactions and pending delete PM xactions.
 */

pxf_PM(pmbuf)
char *pmbuf;
{
  fprintf(ppf_fd,"%5.5s",pmbuf);
}

/* pxf_id_PM():
 *
 *   this routine is used to write a pending transaction file record
 *   for both insert and delete PM.
 */

pxf_id_PM(pmbuf,code)
char *pmbuf;
char code;
{
  long num;

  ppf_open("a+");
  ppf_lock();

  pxf_dts(code);                          /* date stamp, code, op name       */
  putc('n',ppf_fd);                       /* transaction flag                */
  num=atol(pmbuf);
  fprintf(ppf_fd,"%5.5d\n",num);

  ppf_unlock();
  ppf_close();
}

/* xf_date_conv():
 *
 *   this routine converts a date in the form YYMMDDHHMMSS to the form
 *   MMM DD, YYYY  HH:MM:SS.
 */

xf_date_conv(idate,odate)
char *idate;                              /* input date YYMMDDHHMMSS         */
char *odate;                            /* output date MMM DD, YYYY  HH:MM:SS*/
{
  short i;
  short num;
  char buf[3];                            /* temporary buffer for atoi       */

        /* convert month */

  movebytes(idate+2,buf,2);               /* get month                       */
  buf[2] = NULL;
  num = atoi(buf);                        /* compute month number            */

  switch(num)
  {
    case(1):
    strcpy(odate,"JAN");
    break;
    case(2):
    strcpy(odate,"FEB");
    break;
    case(3):
    strcpy(odate,"MAR");
    break;
    case(4):
    strcpy(odate,"APR");
    break;
    case(5):
    strcpy(odate,"MAY");
    break;
    case(6):
    strcpy(odate,"JUN");
    break;
    case(7):
    strcpy(odate,"JUL");
    break;
    case(8):
    strcpy(odate,"AUG");
    break;
    case(9):
    strcpy(odate,"SEP");
    break;
    case(10):
    strcpy(odate,"OCT");
    break;
    case(11):
    strcpy(odate,"NOV");
    break;
    case(12):
    strcpy(odate,"DEC");
    break;
  };

  *(odate + 3) = 0x20;                    /* space betwixt month, day        */

        /* copy day of month */

  movebytes(idate + 4, odate + 4, 2);
  strcpy(odate + 6,", ");                 /* betwixt day, year               */

        /* compute year */

  movebytes(idate, buf, 2);
  num = atoi(buf);
  if(num >= 84)                           /* 20th century                    */
  strcpy(odate + 8, "19");
  else
  strcpy(odate + 8, "20");
  strcpy(odate + 10, buf);                /* year part                       */
  strcpy(odate + 12, "  ");               /* betwixt year and time           */

        /* convert time */

  movebytes(idate + 6, odate + 14, 2);
  *(odate + 16) = COLON;
  movebytes(idate + 8, odate + 17, 2);
  *(odate + 19) = COLON;
  movebytes(idate + 10, odate + 20, 2);
  *(odate + 22) = NULL;                   /* null terminate string           */
}

/************************************************************************/
/*                                                                      */
/*      U T I L I T Y  R O U T I N E S                                  */
/*                                                                      */
/************************************************************************/

/* pxf_find_rec():
 *
 *   this routine determines if a record already exists in the pending
 *   transaction file for the requested operation. returns 1 if such
 *   a record found and zero if not.
 */

pxf_find_rec(buf,code)
char *buf;
char code;
{
  char c;
  char comp_buf[16];
  short comp_size;
  short i;

  ppf_open("r+");
  ppf_lock();

  while((c = getc(ppf_fd)) != EOF)
  {
    ungetc(c,ppf_fd);                     /* put it back !!!                 */

    for(i=0;i<12;i++)                     /* read past dts                   */
    getc(ppf_fd);

    c = getc(ppf_fd);                     /* get code                        */
    if(c != code)                         /* not this record                 */
    {
      while((c = getc(ppf_fd)) != '\n')
      ;
      continue;
    }
                /* implied else here */

    for(i=0;i<8;i++)                      /* read past op name               */
    getc(ppf_fd);

    c = getc(ppf_fd);                     /* get xaction delete flag         */
    if(c == 'y')                          /* transaction deleted             */
    {
      while((c = getc(ppf_fd)) != '\n')
      ;
      continue;
    }
                /* implied else here */

    if(code == DSKUCODE)
    comp_size = 15;
    else
    comp_size = 5;                        /* size of PM number               */

    for(i=0;i<comp_size;i++)              /* read in SKU or PM               */
    comp_buf[i] = getc(ppf_fd);

    if (!strncmp(comp_buf,buf,i))
    {
      ppf_unlock();
      ppf_close();
      return(1);                          /* record exists                   */
    }
    else
    {
      while((c = getc(ppf_fd)) != '\n')
      ;
      continue;
    }
  }
  ppf_unlock();
  ppf_close();
  return(0);                              /* no record found                 */
}

/* end of xf_routines.c */
