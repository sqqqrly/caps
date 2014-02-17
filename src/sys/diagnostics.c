/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Menu for diagnostics functions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/07/93   |  tjt  Added to mfc.
 *  02/18/95   |  tjt  Add port disable.
 *  07/14/96   |  tjt  Fix hard drive space display.
 *-------------------------------------------------------------------------*/
static char diagnostics_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "diagnostics.t"

extern leave();

#define  BLANK 0x20

static short ONE = 1;

struct fld_parms fld1 = {18,45,25,1,&ONE,"Enter Selection",'a'};

FILE *trk_diag;

main(argc, argv)
long argc;
char **argv;
{
  short rm;
  long status;
  unsigned char t;
  static char buf[2] = {0,0};
  
  putenv("_=diagnostics");
  chdir(getenv("HOME"));

  ss_open();
  co_open();
  sd_open(leave);

  fix(diagnostics);
  sd_screen_off();
  sd_clear_screen();
  sd_text(diagnostics);
  sd_screen_on();

  while(1)
  {
    t = sd_input(&fld1,(sd_prompt(&fld1,0)),&rm,buf,0);

    if(t == EXIT) leave();
    
    *buf = tolower(*buf);

    switch(*buf)
    {
      case 't':                            /* basic function tests           */

        if (sp->sp_basic_function == 'n')
        {
          eh_post(LOCAL_MSG, "Basic function not enabled");
          break;
        }
        if (count_ports(IsBasicFunction) < 1)
        {
          eh_post(LOCAL_MSG, "No ports available for diagnostics");
          continue;
        }
        loadprog("diag_menu0", 0);
        break;

      case 'f':                            /* full function tests            */
      case 'o':
      
        if (sp->sp_full_function == 'n')
        {
          eh_post(LOCAL_MSG, "Full function not enabled");
          break;
        }
        if (count_ports(IsFullFunction) < 1)
        {
          eh_post(LOCAL_MSG, "No ports available for diagnostics");
          continue;
        }
        loadprog("diag_menu1", buf);
        break;

      case 'c':

        if (sp->sp_full_function == 'n')
        {
          eh_post(LOCAL_MSG, "Carousels not enabled");
          break;
        }
        if (sp->sp_running_status == 'y')
        {
          eh_post(ERR_IS_CONFIG, 0);
          continue;
        }
        loadprog("diag_menu2", 0);
        break;

      case 'a':                            /* area controller tests          */

        if (sp->sp_total_function == 'n')
        {
          eh_post(LOCAL_MSG, "Total function not enabled");
          break;
        }
        if (count_ports(IsTotalFunction) < 1)
        {
          eh_post(LOCAL_MSG, "No ports available for diagnostics");
          continue;
        }
        loadprog("diag_menu3", 0);
        break;

      case 'h':

        sd_cursor(0, 20, 5);              /* F071496 */
        sd_clear_rest();
        sd_text(".. ");
#ifdef INTEL
        system("df /u");
#else
        system("space");
#endif
        fflush(stdout);
        break;

      case 'd':
          
        loadprog("diskette_diags", 0);
        break;

      case 's':
          
        if (sp->sp_basic_function == 'n')
        {
          eh_post(LOCAL_MSG, "Basic function not enabled");
          break;
        }
        loadprog("suitcase_menu", 0);
        break;
        
      case 'u':
      
        if (sp->sp_total_function == 'n')
        {
          eh_post(LOCAL_MSG, "Total function not enabled");
          break;
        }
        loadprog("alc_suitcase_menu", 0);
        break;

      case 'z':
      
        if (sp->sp_running_status != 'y')
        {
          eh_post(ERR_NO_CONFIG, 0);
          continue;
        }
        loadprog("test_menu", 0);
        break;
        
      default:   
      
        eh_post(ERR_CODE, buf);
        break;
    }
  }                                       /* end of while loop               */
}
/*-------------------------------------------------------------------------*
 *  Count Available Ports For Diagnostics
 *-------------------------------------------------------------------------*/
count_ports(type)
register unsigned long type;
{
  register long k, count;
  
  for (k = count = 0; k < coh->co_ports; k++)
  {
    if (po[k].po_status != 'x') continue;
    if (!(po[k].po_flags & type)) continue;
    count++;
  }
  return count;
}
loadprog(prog, parm)
register char *prog, *parm;
{
  char text[64];

  ss_close();
  co_close();
  sd_close();
  execlp(prog, prog, parm, 0);
  
  ss_open();
  co_open();
  sd_open(leave);
  sprintf(text, "Program %s not found", prog);
  eh_post(CRASH_MSG, text);
}
leave()
{
  sd_close();
  ss_close();
  co_close();
  execlp("syscomm", "syscomm", 0);
  krash("leave", "syscomm load", 1);
}

/* end of diagnostics.c */
