/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Format DOS diskette(s).
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/8/93    |  tjt  Added to mfc.
 *-------------------------------------------------------------------------*/
static char diskette_diags_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "eh_nos.h"
#include "diskette_diags.t"

extern leave();

short ONE = 1;

struct fld_parms fld1 = {8,55,20,1,&ONE,"Diskette Drive Ready? (y/n)",'a'};
struct fld_parms fld2 = {9,55,20,1,&ONE,"Format Another? (y/n)", 'a'};

short rm;
unsigned char t;

char buf[2];

main()
{
  putenv("_=diskette_diags");
  chdir(getenv("HOME"));
    
  ss_open();
  sd_open(leave);

  fix(diskette_diags);
  sd_screen_off();
  sd_clear_screen();
  sd_text(diskette_diags);
  sd_screen_on();

  while (1)
  {  
    memset(buf, 0, 2);
      
    t = sd_input(&fld1,sd_prompt(&fld1,0),&rm,buf,0);

    if(t == EXIT) leave();
    if (t == UP_CURSOR) continue;
    
    *buf = tolower(*buf);
    if (*buf == 'n') continue;
    if (*buf != 'y')
    {
      eh_post(ERR_CODE, buf[0]);
      continue;
    }
/*
 *  Format Diskette
 */
    system("dosformat -fq a:");
    
    eh_post(ERR_CONFIRM, "Format diskette");
  
    memset(buf, 0, 2);
    
    t = sd_input(&fld2,sd_prompt(&fld2,0),&rm,buf,0);

    if(t == EXIT) leave();
    if (t == UP_CURSOR) continue;
    
    *buf = tolower(*buf);
    if (*buf == 'n') leave();
    if (*buf == 'y') continue;
    
    eh_post(ERR_CODE, buf);
  }  
}                                         /* end main                        */
leave()
{
  sd_close();
  ss_close();
  execlp("diagnostics", "diagnostics", 0);
  krash("leave", "diagnostics load", 1);
}

/* end diskette_diags.c */

