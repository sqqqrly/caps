/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Format data to a print file.
 *
 *  Path to Header: $HOME/sys/report/argv[3]  or
 *                  $HOME/language/$LANGUAGE/report/argv[3]
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  09/15/93   |  tjt  Added to mfc.
 *  12/11/94   |  tjt  Add FORMFEED recognition (G. Hawk)
 *  08/03/95   |  tjt  Add ignore FORMFEED if first in file.
 *  02/06/97   |  tjt  Add LANGUAGE parameter.
 *-------------------------------------------------------------------------*/
static char prft_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 * prft.c
 * 
 * format the data file and store it in the printfile
 *
 * Arguments
 *  
 *         argv[1] is the name of the data file.
 *         argv[2] is the name of the printfile.
 *         argv[3] is the name of the headerfile.
 *         argv[4+] is additional parameters (%v).
 *
 *-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ss.h"
#include "co.h"
#include "iodefs.h"

#define maxhead 5
#define maxcolm 5
#define maxfoot 5

#define maxparm 7
#define maxsect 3

#define maxline 256
#define maxinfo 4096

#define delim1  ':'
#define delim2  '^'

#define maxdat 256

#define snamelen 16
#define cnamelen 40

struct parms
{
  short width;
  short leng;
  short top;
  short med;
  short bot;
  short bet;
  short chnk;
  char  *header;
  char  *column;
  char  *footer;
  char  *pageptr;
} p ={0,0,0,0,0,0,0,0,0,0,0};

FILE *fp1 = 0, *fp2 = 0, *fp3 = 0;

#undef LF

#define LF       '\n'
#define BLANK    0x20
#define FORMFEED "\f"

short ITH = 4;         /* the position of the first variable in the argv list*/

char buf1[maxline], buf2[maxinfo];       /* report parameters                */

long pid, status;

char spec_name[64];                      /* full path name                   */

main(argc, argv)
long argc;
char **argv;
{
  char text[80], *p;

  setpgrp();                              /* krash this task only            */
  putenv("_=prft");                       /* name to environment             */
  chdir(getenv("HOME"));                  /* to home directory               */
   
  ss_open();

  if (argc < 4)
  {
    krash("prft", "missing parms", 0);
    exit(1);
  }
  if ((fp1 = fopen(argv[1], "r")) == NULL)
  {
    sprintf(text, "Open Data File %s", argv[1]);
    krash("prft", text, 0);
    exit(1);
  }
  if ((fp2 = fopen(argv[2], "w")) == NULL)
  {
    sprintf(text, "Open Print File %s", argv[2]);
    krash("prft", text, 0);
    fclose(fp1);
    unlink(argv[1]);
    exit(1);
  }
  p = getenv("LANGUAGE");
    
  if (p)                                  /* language parameter exists       */
  {
    if (*p)                               /* has a value                     */
    {
      sprintf(spec_name, "language/%s/report/%s.h", p, basename(argv[3]));
      fp3 = fopen(spec_name, "r");
#ifdef DEBUG
  fprintf(stderr, "fp3=%x [%s]\n", spec_name);
#endif
    }
  }
  if (!fp3)                               /* use default version             */
  {
    sprintf(spec_name, "sys/report/%s.h", basename(argv[3])); /* default     */
    fp3 = fopen(spec_name, "r");
#ifdef DEBUG
  fprintf(stderr, "fp3=%x [%s]\n", spec_name);
#endif
  }
  if (fp3 == NULL)
  {
    sprintf(text, "Open Spec File %s", spec_name);
    krash("prft", text, 0);
    fclose(fp1);
    fclose(fp2);
    exit(1);
  }
#ifdef DEBUG
  fprintf(fp2, "argv[1] = %s\n", argv[1]);
  fprintf(fp2, "argv[2] = %s\n", argv[2]);
  fprintf(fp2, "argv[3] = %s\n", argv[3]);
  fprintf(fp2, "spec    = %s\n", spec_name);
#endif
  
  headin(argv, argc);
  prnt();

  ss_close();
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);

  sprintf(text,"%s %s", getenv("LPR"), argv[2]);
  system(text);
  unlink(argv[1]);                         /* delete data file             */

  exit(0);
}
/*-------------------------------------------------------------------------*
 * prnt
 * 
 * store the headings and the footer if any and the data in the
 * printfile
 *
 *-------------------------------------------------------------------------*/

prnt()
{
  extern short linecnt();
  char  line[maxdat], pwork[4];
  short i, j, k, ret;
  short nmofchnk = 0,linenum = 0,lastblnk = 0;
  short headlins = 0,colmlins = 0,footlins = 0;
  short headsize = 0, colmsize = 0, footsize = 0;
  short pcount = 1;

  if (p.header) headlins = linecnt(p.header);  /* lines in heading  */
  if (p.column) colmlins = linecnt(p.column);  /* lines column head */
  if (p.footer) footlins = linecnt(p.footer);  /* lines in footer */

  linenum = p.leng - p.top - headlins - p.med - colmlins - p.bot - footlins;
  
  if((p.chnk + p.bet) > 1)                 /* number of chunks per page */
  {
    nmofchnk = (linenum + p.bet) / (p.chnk + p.bet);
    lastblnk = (linenum + p.bet) - (nmofchnk * (p.chnk + p.bet));
    linenum  -= lastblnk;
  }
  if(p.pageptr)                                 /* page number to footer   */
  {
    sprintf(pwork, "%3d", pcount);
    memcpy(p.pageptr, pwork, 3);
  }
  if (p.header) headsize = strlen(p.header);
  if (p.column) colmsize = strlen(p.column);
  if (p.footer) footsize = strlen(p.footer);
  
  while(1)                                /* loop forever                    */
  {
    blnklins(p.top/2);                    /* put the top of page blank lines */

    if(p.header) fwrite(p.header,headsize,1,fp2);/* put the header in the pr */
    blnklins(p.med);                      /* lines after header              */

    if (p.column) fwrite(p.column,colmsize,1,fp2); /* column headings        */
    if (p.bot) blnklins(p.bot);

    i = 0;                                /* current line number             */

    while (i < linenum)                   /* do one page of data             */
    {
      for( j = 0; j < p.chnk; j++)        /* do one data set                 */
      {
        if (!fgets(line, maxdat - 1, fp1))
        {
          blnklins(linenum - i);         /* end of print job                 */

          if(p.footer)
          {
            blnklins(lastblnk);
            if (p.footer) fwrite(p.footer, footsize, 1, fp2);
          }
          /* fwrite(FORMFEED, 1, 1, fp2);   */
          return 0;
        }
        else
        {
          if (*line == *FORMFEED)           /* F121194 */
          {
            if (i == 0 && pcount == 1) continue;/* ignore first F080395      */
            break;
          }
          k = strlen(line);
          if (k > p.width)
          {
            k = p.width;
            line[k - 1] = '\n';
          }
          fwrite(line, k, 1, fp2);
          i++;                          /* count each line                   */
        }
      }                                 /* end of a chunk of data            */
      if (*line == *FORMFEED)           /* F121194                           */
      {
        if (i == 0 && pcount == 1) continue;/* ignore first F080395          */
        break;
      }
      if (p.bet && i < linenum)
      {
        blnklins(p.bet);                /* lines between chunks              */
        i += p.bet;
      }
    }                                   /* full page of data                 */
    if (i < linenum) blnklins(linenum - i);  /* F121194                      */
    if(p.footer) 
    {
      blnklins(lastblnk);
      fwrite(p.footer, footsize - 1, 1, fp2);
    }
    fwrite(FORMFEED, 1, 1, fp2);
    fwrite("\r", 1, 1, fp2);
    ++pcount;
    if(p.pageptr)                       /* page number to footer         */
    {
      sprintf(pwork, "%3d", pcount);
      memcpy(p.pageptr, pwork, 3);
    }
  }
}
/* function to count the lines of a string */

short linecnt(strptr)

char *strptr;
{

  short count;
  char *q;

  count = 0;
  q = strptr;
  
  while( *q )
  if(*q++ == '\n') ++count;
  return(count);
}

/*-------------------------------------------------------------------------*
 * function to put blank lines in the print file
 *-------------------------------------------------------------------------*/
blnklins(num)
short num;
{
  short i;

  if(num) for(i = 0; i < num; i++) fwrite("\n", 1, 1, fp2);
}

/*-------------------------------------------------------------------------*
 *  pfmt
 *
 *  determines the value of the parameters and assigns
 *  values to pointers to header, column header, and footer.
 *
 *  Arguments none
 *
 *  Returns   none
 *
 *-------------------------------------------------------------------------*/

headin(v, c)
char **v;
int c;
{
  char delim, *arr1[maxparm];
  register char *q;
  short k;

  fgets(buf1, maxline, fp3);

  break_fields(buf1, arr1, maxparm);      /* break the first line            */

  p.width = atoi(arr1[0]);                /* width of the report page        */

  p.leng = atoi(arr1[1]);                 /* length of each report page      */

  p.chnk = atoi(arr1[2]);                 /* lines that have to stay together*/

  p.bet = atoi(arr1[3]);                  /* blank lines between items       */

  p.top = atoi(arr1[4]);                  /* blank lines between pages       */

  p.med = atoi(arr1[5]);                  /* blank lines after heading       */

  p.bot = atoi(arr1[6]);                  /* blank lines after col head      */

  k = fread(buf2, 1, maxinfo, fp3);       /* read the rest of the header     */

  buf2[k] = 0;
  fix1(buf2, v, c);                       /* replace the variables          */

  q = buf2;

  while(*q)
  {
    if (*q == delim2)
    {
      *q++ = 0;
      if(*q == 'h')                       /* skip the heading line           */
      {
        while(*q != LF) q++;
        p.header = ++q;
      }
      else if(*q =='c')                   /* skip the column heading         */
      {
        while(*q != LF) q++;
        p.column = ++q;
      }
      else if(*q == 'f')                  /* skip the first line             */
      {
        while(*q != LF) q++;
        p.footer = ++q;
      }
    }
    q++;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *   Replace Macros With Literal Values
 *-------------------------------------------------------------------------*/
fix1(string, v, c)
char *string;
char **v;
short c;
{
  char str[32];
  short  j, k, i, n;
  char *q;
  long now;
  
  time(&now);
  strcpy(str, ctime(&now));              /* store time in str               */
  str[24] = 0;
  
  while(*string)
  {
    if (*string != '%') {string++; continue;}

    switch(*(string+1))
    {
      case 'm' : strncpy(string, str+4, 3); break;

      case 'd' : strncpy(string,str+8,2); break;

      case 'y' : strncpy(string,str + 20,4); break;

      case 'w' : strncpy(string,str,3); break;
                      
      case 't' : strncpy(string,str + 11,8); break;

      case 'n' : p.pageptr = string; break;
        
      case 's' : 
      
        if (*(string + 2) == 'l')
        strncpy(string, sp->sp_name, strlen(sp->sp_name));

        else if(*(string + 2) == 'c')
        {
          for(j = 0; j < (snamelen - strlen(sp->sp_name))/ 2; j++)
          {
            *(string++) = BLANK;
          }
          strncpy(string, sp->sp_name, strlen(sp->sp_name));
        }

        else if(*(string + 2) == 'r')
        {
          for(j = 0; j < (snamelen - strlen(sp->sp_name)); j++)
          {
            *(string++) = BLANK;
          }
          strncpy(string, sp->sp_name, strlen(sp->sp_name));
        }
        break;
            
      case 'c' : if(*(string + 2) == 'l')
        strncpy(string, sp->sp_company, strlen(sp->sp_company));

        else if(*(string + 2) == 'c')
        {
          for(j = 0; j < (cnamelen - strlen(sp->sp_company))/ 2; j++)
          {
            *(string++) = BLANK;
          }
          strncpy(string, sp->sp_company, strlen(sp->sp_company));
        }

        else if(*(string + 2) == 'r')
        {
          for(j = 0; j < (cnamelen - strlen(sp->sp_company)); j++)
          {
            *(string++) = BLANK;
          }
          strncpy(string, sp->sp_company, strlen(sp->sp_company));
        }
        break;

      case 'l' :                    /* clear to the end of the line in string*/

        i = 0;
        while(*(string + i) != '\n')
        {
          *(string + i) = BLANK;
          i++;
        }
        q = (char *)getenv("PICKLINE");
        if (q) strncpy(string, q, strlen(q));
        break;

      case 'p' : 

        if (sp->sp_config_status == 'y')
        {
          co_open();
          k = atol(getenv("PICKLINE"));

          strncpy(string, pl[k - 1].pl_name, strlen(pl[k - 1].pl_name));
          co_close();
          break;
        }
        strncpy(string, "Pickline", 8);
        break;

      case 'v' :  if(c > ITH)
        {
          *(string + 1) = BLANK;
          strncpy(string,v[ITH],strlen(v[ITH]));
          ITH++;
        }
      }
    string++;
  }
  return 0;
}

/* end of prft.c */
