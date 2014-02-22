/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Product file formatted query report.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  01/28/93   |  tjt  Added to mfc.
 *  04/18/97   |  tjt  Add language.h and code_to_caps
 *-------------------------------------------------------------------------*/
static char program_name_c[] = "%Z% %M% %I% (%G% - %U%)";

/************************************************************************/
/*                                                                      */
/*      product file formatted inquiry report   screen 7.5              */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "language.h"
#include "pff_inquiry_report.t"

#define WIDTH   80                        /* for use by show routine         */

short ONE = 1;

struct fld_parms fld =

{23,20,1,1,&ONE,"Print? (y/n)          (Exit, Forward, or Backward)",'a'};


long savefp=0;

main(argc,argv)
int argc;
char **argv;
{
  extern leave();
  long pid, status;
  FILE *fp = fopen(argv[1],"r");
  short rm,ret;
  short i;
  unsigned char t;
  char buf[2];
  short dflag;
  char print_file[15];

  putenv("_=pff_inquiry_report");
  chdir(getenv("HOME"));

  open_all();

  fix(pff_inquiry_report);
  sd_screen_off();
  sd_clear_screen();
  sd_text(pff_inquiry_report);
  sd_screen_on();
  
  dflag=1;                                /* display forward                 */
  while(1)                                /* main loop to display data       */
  {
    sd_cursor(0,6,1);
    show(fp,16,dflag);
    sd_clear_rest();                      /* clear remainder of screen       */
    for(i=0;i<2;i++)
    buf[i]=0;
    sd_prompt(&fld,0);
    while(1)                              /* loop to process print prompt    */
    {
      t = sd_input(&fld,0,&rm,buf,0);
      switch(ret=(sd_print(t, code_to_caps(buf[0]))))  /* F041897 */
      {
        case(0):
        fclose(fp);
        unlink(argv[1]);
        leave();

        case(1):
        dflag=1;
        break;

        case(2):
        dflag=2;
        break;

        case(3):
        fclose(fp);
        unlink(argv[1]);
        leave();

        case(4):

        fclose(fp);
        close_all();

                                /* print file and leave */

        execlp("pff_print","pff_print", argv[1], 0);
        krash("main", "pff_print load", 1);

        case(6):                          /* error                           */
        eh_post(ERR_YN,0);
      }
      if(ret != 6)                        /* not meaningless                 */
      break;
    }
  }
}

/****************************************************************************/
/*function to display x number of lines of data on the screen               */
/* Arguments:                                                               */
/*           fp : the data file pointer.                                    */
/*           lines : the number of lines to be displayed.                   */
/*           i : the indicator of either going forward or                   */
/*           backward on the file.                                          */
/*                                                                          */
/* returns : 1 if successfull                                               */
/*           0 if failed                                                    */
/****************************************************************************/

show(fp,lines,index)
FILE *fp;
short lines,index;
{
  register long pos, size;
  char str[1920];

  memset(str, 0, 1920);
  
  pos = ftell(fp);
  fseek(fp, 0, 2);
  size = ftell(fp);
  
  if(index == 2)
  {
    pos = savefp - lines * WIDTH;
    if(pos < 0) pos = 0;
    savefp = pos;

    fseek(fp, pos, 0);  
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else if(index == 1)
  {
    if (pos >= size) pos = savefp;
    savefp = pos;
    
    fseek(fp, pos, 0);  
    fread(str, WIDTH, lines, fp);
    sd_clear_rest();
    sd_text(str);
    return(1);
  }
  else return(0);
}
leave()
{
  close_all();
  execlp("pff_inquiry_input", " pff_inquiry_input",0);
  krash("leave", "pff_inquiry_input load", 1);
}


open_all()
{
  sd_open(leave);
  ss_open();
}

close_all()
{
  ss_close();
  sd_close();
}

/* end of pff_inquiry_report.c */
