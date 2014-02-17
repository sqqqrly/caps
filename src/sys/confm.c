/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Configuration entry menu.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  10/03/93   |  tjt  Added to mfc.
 *  08/04/94   |  tjt  Add bay and modules to print.
 *  08/04/94   |  tjt  Save all config to config/name.current.
 *  08/11/94   |  tjt  Fix zone_list bug.
 *  06/02/95   |  tjt  Add pickline input by name.
 *  04/16/96   |  tjt  Remove cluster
 *  04/17/96   |  tjt  Add matrix and carousel.
 *  03/14/97   |  tjt  Add demand feed attribute.
 *  03/21/97   |  tjt  Add MM.
 *  04/18/97   |  tjt  Add language.h and code_to_caps.
 *-------------------------------------------------------------------------*/
static char confm_c[] = "%Z% %M% %I% (%G% - %U%)";
/****************************************************************************/
/*                                                                          */
/*                             Configuration Menu                           */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include "iodefs.h"
#include "getparms.h"
#include "sd.h"
#include "confm.t"
#include "ss.h"
#include "co.h"
#include "eh_nos.h"
#include "language.h"

#define NP 3
#define NS 9
#define LPICKLINE 8

short ONE  = 1;
short PLEN = LPICKLINE;

struct fld_parms fld1 = {20,38,20,1,&ONE,"Enter Code",'a'};
struct fld_parms fld2 = {21,38,20,1,&PLEN,"Enter Pickline",'a'};
struct fld_parms fld3 = {23,20,1,1,&ONE,"Print? (y/n)",'a'};

char tname[16];
FILE *t_fd;
char tbuf[81];
long pos[50];
short page;
char *eof;
char command[50];

short i, j, k, rm, ret;
unsigned char t;
char buf[NP][NS];
long line;

main()
{
  extern leave();

  putenv("_=confm");
  chdir(getenv("HOME"));

  open_all();

  fix(confm);
  sd_screen_off();
  sd_clear_screen();
  sd_text(confm);
  sd_screen_on();
     
  while(1)
  {

    memset(buf, 0, NP * NS);

    t = sd_input(&fld1,(sd_prompt(&fld1,0)),&rm,buf[0],0);

    if(t ==  EXIT) leave();

    if(!LEGAL(toupper(*buf[0])))
    {
      eh_post(ERR_CODE, buf[0]);
      continue;
    }

    if (tolower(*buf[0]) == 'e')
    {
      close_all();
      execlp("config_entry", "config_entry", 0);
      krash("main", "config_entry load", 1);
    }
    if (sp->sp_config_status == 'n')
    {
      eh_post(ERR_NO_CONFIG, 0);
      continue;
    }
/*
 *  Setup proper pickline
 */
    line = op_pl;                         /* default                         */

    while (SUPER_OP)
    {
      t = sd_input(&fld2,sd_prompt(&fld2, 0),&rm,buf[1],0);
      if (t == EXIT) leave();

      line = pl_lookup(buf[1], 0);       /* ALL is SUPER OP default          */
      if (line < 0)
      {
        eh_post(ERR_PL, buf[1]);
        memset(buf[1], 0, NS);
        continue;
      }
      break;
    }
/*
 *  Display Configuration
 */
    build(tolower(*buf[0]), line);        /* edit display file               */
    display();                            /* display configuration           */
 
    fix(confm);
    sd_screen_off();
    sd_clear_screen();
    sd_text(confm);
    sd_screen_on();
  }
}
leave()
{
  close_all();
  execlp("mmenu", "mmenu", 0);
  krash("leave", "mmenu load", 1);
}
/*
 * open all files
 */
open_all()
{
  sd_open(leave);
  ss_open();
  co_open();
  getparms("LEGAL4");
}

/* 
 * close all files
 */
close_all()
{
  ss_close();
  co_close();
  sd_close();
}

/*
 * transfer control back to calling program
 */
/*
 *  Display Configuration
 */
display()
{
  char temp1[16], title[20];
  long len, pid, stat;
  
  t_fd = fopen(tname, "r");
   
  for (k=0; k<50; k++) pos[k] = 0;        /* page pointers                   */

  page = 1;
  while (1)
  {
    for (k=0; k<16; k++)
    {
      eof = fgets(tbuf, 81, t_fd);
      if (!eof) break;                   /* end of file                     */
      tbuf[strlen(tbuf) - 1] = 0;
    }
    if (!eof) break;
    pos[page++] = ftell(t_fd);           /* save page address               */
  }
  page = 0;

  while(1)
  {
    sd_cursor(0, 6, 1);
    sd_clear_rest();
    fseek(t_fd, pos[page], 0);

    for (k=0; k<16; k++)
    {
      eof = fgets(tbuf, 81, t_fd);
      if (!eof) break;
      tbuf[strlen(tbuf) - 1] = 0;
      sd_cursor(0, 6 + k, 1);
      sd_text(tbuf);
    }
    sd_cursor(0, 23, 25);
    sd_text("(Exit, Forward, Backward)");
    t = sd_input(&fld3,sd_prompt(&fld3,0),&rm,buf[2],0);
    ret = sd_print(t, code_to_caps(buf[2][0]));  /* F041897 */
    if (ret == 0)                         /* exit display                    */
    {
      fclose(t_fd);
      unlink(tname);
      return;
    }
    if (ret == 1)                         /* forward                         */
    {
      if (pos[page + 1] > 0) page++;
      continue;
    }
    if (ret == 2)                         /* backward                        */
    {
      if (page > 0) page--;
      continue;
    }
    if (ret == 3)                         /* no print and quit               */
    {
      fclose(t_fd);
      unlink(tname);
      return;
    }
    if (ret == 4)                         /* print  and quit                 */
    {
      fclose(t_fd);

/*  F050187 system printers */

      tmp_name(temp1);
      
      len = (20 - strlen(coh->co_config_name)) / 2;
      if (len <= 0) len = 1;
      sprintf(title, "%*s%s", len, " ", coh->co_config_name);
      
      if (fork() == 0)
      {
        execlp("prft", "prft", tname, temp1, "sys/report/report_report.h",
         title, 0);
        krash("display", "load prft", 1);
      }
      pid = wait(&stat);
      unlink(tname);
      return;
    }
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  Build Configuration Display File
 *-------------------------------------------------------------------------*/
build(code, pickline)
char code;
short pickline;
{
  short first, last;
  long last_port;
  char command[80];
  
  tmp_name(tname);
   
  t_fd = fopen(tname, "wct");

  first = last = pickline;
  
  if (pickline == 0)
  {
    first = 1;
    last  = coh->co_pl_cnt;
  }
  get_name();
   
  last_port = -1;

  for (k = first; k <= last; k++)
  {
    if (pl[k - 1].pl_pl == 0) continue;
    get_pl(k);
    if (code == 'a' || code == 'b') get_bay(k, &last_port);
    if (code == 'a' || code == 'z') get_zone(k);
    if (code == 'a' || code == 'm') get_mbl(k);
    if (code == 'a' || code == 'o') get_flow(k);
    if (code == 's') get_pl(k);
  }
  fclose(t_fd);

  if (code == 'a')
  {
    sprintf(command, "cp %s config/%s.current", tname, coh->co_config_name);
    system(command);
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  display configuration name
 *-------------------------------------------------------------------------*/
get_name()
{
  fprintf(t_fd, "*\n* %s\n*\n", coh->co_config_name);
  return;
}
/*-------------------------------------------------------------------------*
 *  display pickline name
 *-------------------------------------------------------------------------*/
get_pl(line)
short line;
{
  char sam[4];

  fprintf(t_fd, "P%d='%s'\n", line, pl[line - 1].pl_name);
  fprintf(t_fd, "%3.3s\n", pl[line - 1].pl_sam);
  
  return;
}
/*-------------------------------------------------------------------------*
 *  display bay assignments
 *-------------------------------------------------------------------------*/
get_bay(line, last_port)
short line;
long *last_port;
{
  register struct bay_item *b;
  register struct zone_item *z;
  register long k, comma, mbl;
  char buf[80], work[16];
  
  for (k = mbl = 0, b = bay; k < coh->co_bay_cnt; k++, b++)
  {
    if (!b->bay_zone) continue;
    z = &zone[b->bay_zone - 1];
    if (z->zt_pl != line) continue;
    
    if (b->bay_port != *last_port)
    {
      *last_port = b->bay_port;

      fprintf(t_fd, "\n<%d = ", b->bay_port - 1);

      if (b->bay_flags & IsFullFunction)       fprintf(t_fd, "FF\n\n");
      else if (b->bay_flags & IsBasicFunction) fprintf(t_fd, "BF\n\n");
      else if (b->bay_flags & IsTotalFunction) fprintf(t_fd, "AC\n\n");
      else if (b->bay_flags & IsDummy)         fprintf(t_fd, "DF\n\n");
    }
    comma = 0;

    sprintf(buf, "B%d=", b->bay_number);
    if (b->bay_flags & HasModules)
    {
      if ((b->bay_flags & HasBoxFull) && (b->bay_flags & IsFullFunction))
      {
        sprintf(work, "%d", b->bay_prod_last - b->bay_prod_first + 2);
        strcat(buf, work);
      }
      else
      {
        sprintf(work, "%d", b->bay_prod_last - b->bay_prod_first + 1);
        strcat(buf, work);
      }
      if (b->bay_flags & Multibin)
      {
        sprintf(work, ",%d", b->bay_mod_last - b->bay_mod_first + 1);
        strcat(buf, work);
      }
      comma = 1;
    }
    if (b->bay_flags & HasBayLamp)
    {
      if (comma) strcat(buf, ",");
      strcat(buf, "BL");
      comma = 1;
    }
    if (b->bay_flags & HasZoneController)
    {
      if (comma) strcat(buf, ",");
      strcat(buf, "ZC");
      comma = 1;
    }
    if (b->bay_flags & HasBoxFull)
    {
      if (comma) strcat(buf, ",");
      strcat(buf, "BF");
      comma = 1;
    }
    if (b->bay_flags & IsBasicFunction)
    {
      if (comma) strcat(buf, ",");
      sprintf(work, "TC");
      strcat(buf, work);
      comma = 1;
    }
    if (b->bay_flags & MultiModule)
    {
      strcat(buf, "S");
      comma = 0;
    }
    if (b->bay_flags & Multibin)
    {
      if (comma) strcat(buf, ",");
      if (b->bay_flags & VertLights)      strcat(buf, "M");
      else if (b->bay_flags & IsCarousel) strcat(buf, "C");
      else                                strcat(buf, "H");
      
      if (b->bay_flags & IsPut)           strcat(buf, "P");
      if (b->bay_flags & IsPI)            strcat(buf, "I");
      if (b->bay_flags & HasBayBarrier)   strcat(buf, "X");
    }
    else if (b->bay_flags & (HasBayBarrier | IsPut | IsPI))
    {
      if (comma) strcat(buf, ",");
      if (b->bay_flags & IsPut)           strcat(buf, "P");
      if (b->bay_flags & IsPI)            strcat(buf, "I");
      if (b->bay_flags & HasBayBarrier)   strcat(buf, "X");
      comma = 1;
    }
    if (b->bay_flags & IsMasterBayLamp) mbl++;

    if (b->bay_flags & HasModules)
    {
      fprintf(t_fd, "%-24s    * Bay %3d:  Modules %d - %d\n",
        buf, b->bay_number - mbl, b->bay_mod_first, b->bay_mod_last);
    }
    else fprintf(t_fd, "%s\n", buf);
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  display zone assignments
 *-------------------------------------------------------------------------*/
get_zone(line)
short line;
{
  register struct bay_item *b;
  register struct zone_item *z;
  register long j, k;
  
  for (k = 0, z = zone; k < coh->co_zone_cnt; k++, z++)
  {
    if (z->zt_pl != line) continue;
    fprintf(t_fd, "Z%d=", z->zt_zone);
    j = z->zt_first_bay;
    fprintf(t_fd, "B%d", j);

    b = &bay[j - 1];
    
    while (b->bay_next)
    {
      fprintf(t_fd, ",B%d", b->bay_next);
      b = &bay[b->bay_next - 1];
    }
    fprintf(t_fd, "\n");
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  display master bay lamps
 *-------------------------------------------------------------------------*/
get_mbl(line)
short line;
{
  register struct bay_item *b, *m;
  register struct zone_item *z;
  register long j, k, comma;
  
  for (k = 1, m = bay; k <= coh->co_bay_cnt; k++, m++)
  {
    if (!m->bay_zone) continue;
    if (!(m->bay_flags & IsMasterBayLamp)) continue;
    z = &zone[m->bay_zone - 1];
    if (z->zt_pl != line) continue;
    
    fprintf(t_fd, "MBL%d = ", m->bay_number);
    
    comma = 0;

    for (j = 1, b = bay; j <= coh->co_bay_cnt; j++, b++)
    {
      if (b->bay_mbl != k) continue;
      if (comma) fprintf(t_fd, ",");
      fprintf(t_fd, "B%d", b->bay_number);
      comma = 1;
    }
    fprintf(t_fd, "\n");
  }
  return;
}
/*-------------------------------------------------------------------------*
 *  display order flow
 *-------------------------------------------------------------------------*/
get_flow(line)
short line;
{
  register struct seg_item *s;
  register struct zone_item *z;
  register long j, k, n, first, last;
  
  for (k = 1, s = sg; k <= coh->co_seg_cnt; k++, s++)
  {
    if (s->sg_pl != line) continue;

    fprintf(t_fd, "N%d,", s->sg_snode);   /* first node                      */

    first = last = n = s->sg_first_zone;  /* zone number                     */
    fprintf(t_fd, "Z%d", first);          /* display zone                    */
    
    while (1)
    {
      n = zone[n - 1].zt_feeding;
      if (!n) break;
      
      if (n > last + 1)
      {
        fprintf(t_fd, "-Z%d,Z%d", last, n);
        first = n;
      }
      last = n;
    }
    if (first != last) fprintf(t_fd, "-Z%d", last);
    fprintf(t_fd, ",N%d\n", s->sg_enode);
  }
  if (pl[line -1].pl_flags & EarlyExitModeLast)/* early exit                 */
  {
    fprintf(t_fd, "EEMODE = LAST\n");
  }
  if (pl[line -1].pl_flags & EarlyExitModeNext)/* early exit                 */
  {
    fprintf(t_fd, "EEMODE = NEXT\n");
  }
  zone_list(line, "EE", EarlyExit);
  zone_list(line, "LE", LateEntry);
  zone_list(line, "ST", Steering);
  zone_list(line, "JZ", JumpZone);
  zone_list(line, "SI", DemandFeed);
  
  return;
}
/*------------------------------------------------------------------------*
 *  Do Zone Lists
 *-------------------------------------------------------------------------*/
zone_list(line, p, flag)
register long line;
register char *p;
register unsigned long flag;
{
  register long k, last, count;
  register struct zone_item *z;

  if (pl[line - 1].pl_flags & flag)
  {
    fprintf(t_fd, "%s=", p);

    count = last = 0;

    for (k = 0, z = zone; k< coh->co_zone_cnt; k++, z++)
    {
      if (z->zt_pl != line) continue;

      if (!(z->zt_flags & flag))
      {
        if (count > 1) fprintf(t_fd, "-Z%d", last);
        count = 0;
        continue;
      }
      if (!count)
      {
         if (last) fprintf(t_fd, ",");
         fprintf(t_fd, "Z%d", z->zt_zone - count);
      }
      count++;
      last = z->zt_zone;
    }
    if (count > 0) fprintf(t_fd, "-Z%d", last);
    fprintf(t_fd, "\n");
  }
}

/* end of confm.c */
