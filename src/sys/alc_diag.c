/* #define DEBUG */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Total function diagnostics.
 *
 *  alc_diag [test] ["-print"] ["diag_menu3"]
 *
 *  Test 1 - Steady - With Blank Switch Action.
 *  Test 2 - Flashing *0 With Start/Stop Action.
 *  Test 3 - Flashing 11's.
 *  Test 4 - Self Test 11.
 *  Test 5 - Self Test 12.
 *  Test H - Hardware index.
 *  Test B - Bay & Zone display.
 *  Test L - Line test.
 *  Test F - Fast line test.
 *  Test S - SKU & Pick Location Display.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/11/94   |  tjt  Modified From tlc_diag.
 *  07/06/94   |  tjt  88 tests replaced by - and *0.
 *  07/06/94   |  tjt  Add diag_menu3 caller.
 *  07/06/94   |  tjt  Add ac_soft_reset.
 *  08/22/94   |  tjt  Add self tests 11 & 12.
 *  09/15/94   |  tjt  Add location database.
 *  03/23/95   |  tjt  Add ZC2, PM2, and PM4.
 *  05/31/95   |  tjt  Add print to alc_init call.
 *  06/01/95   |  tjt  Add fast line test.
 *  07/21/95   |  tjt  Revise Bard calls.
 *  04/25/96   |  sg   Bug in test_bz buf size.
 *  08/23/96   |  tjt  Add begin and commit work.
 *  01/07/97   |  tjt  Add PM6.
 *  01/07/97   |  sg   Add Full Function.
 *  05/28/98   |  tjt  Add IO scanner.
 *-------------------------------------------------------------------------*/
static char alc_diag_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "file_names.h"
#include "co.h"
#include "st.h"
#include "ss.h"
#include "Bard.h"
#include "bard/pmfile.h"

long fd;                                  /* open port                       */

char test[2];                             /* test to perform                 */
long port;                                /* -1 = all; else port             */

long children = 0;                        /* children of primary task        */
long called   = 0;                        /* TRUE if caller waiting to kill  */

FILE *pfd = 0;                            /* print file                      */
char pname[16];                           /* print file name                 */
char parm[4] = {0};                       /* print parameter                 */

void stop_it(int notUsed);

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  char text[64];

  putenv("_=alc_diag");
  chdir(getenv("HOME"));
  
#ifdef DEBUG
  fprintf(stderr, "alc_diag: pid=%d  pgrp=%d\n", getpid(), getpgrp());
#endif

  if (argc < 2)
  {
    printf("Specify Test\r\n\n");
    exit(0);
  }
  setpgrp();

  signal(SIGTERM, stop_it);
  signal(SIGHUP, stop_it);
  signal(SIGQUIT, stop_it);
  
  test[0] = argv[1][0];
  if (strcmp(argv[2], "All") == 0) port = -1;
  else port = atol(argv[2]);
  
  ss_open();
  co_open();
  
  if (argc > 3)
  {
    if (strcmp(argv[3], "-print") == 0)
    {
      char dashP[] = "-p";
      strcpy(parm, dashP);
      tmp_name(pname);
      pfd = fopen(pname, "w");
    }
  }
  if (argc > 4)
  {
    if (strcmp(argv[4], "diag_menu3") == 0) called = 1;
  }
  printf("\r\n\nTotal Function Diagnotics Test %c\n\n", *test);
  
  switch (*test)
  {
    case 'F':  fast_line_test();
      			break;
        
    case 'L':  line_test();               /* initialize everything           */
			      break;
    
    case 'B':  test_bz();                 /* pickline, zone & bay display    */
      			break;
                
    case 'S':  test_sm();                 /* sku display                     */
			      break;
    
    case 'H':                             /* port numbering                  */
    case '1':                             /* test 1 - steady -               */
    case '2':                             /* test 2 - flashing *0            */
    case '3':                             /* test 3 - flashing 11's          */
    case '4':                             /* test 4 - self test 11           */
    case '5':                             /* test 5 - self test 12           */
      			run_test();
      			break;
                
      default:	break;
  }
  ss_close();
  co_close();
  
  if (pfd)
  {
    fclose(pfd); pfd = 0;
    sprintf(text, "%s %s", getenv("LPR"), pname);
    system(text);
  }
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Run Primative Tests
 *-------------------------------------------------------------------------*/
run_test()
{
  register long k;

  k = fork_k_ways();
  
  execlp("alc_diag_test", "alc_diag_test", po[k].po_name, test, 0);

  krash("run_test", "load alc_diag_test", 1);
}
/*--------------------------------------------------------------------------*
 *  Pickline, Zone & Bay Display                                              
 *--------------------------------------------------------------------------*/
test_bz()
{
  register struct zone_item *z;
  register struct bay_item *b;
  register struct hw_item *h;
  register long j, k;
  char buf[32];
   
  if (sp->sp_config_status != 'y')
  {
    printf("CAPS Is Not Configured\r\n");
    return;
  }
  k = fork_k_ways();
  
  fd = ac_open(po[k].po_name);
  if (fd <= 0) return 0;
  
  ac_soft_reset(fd, 9999);                /* software reset                  */

  for (j = 0, h = hw; j < coh->co_hw_cnt; j++, h++)
  {
    if (!h->hw_bay) continue;

    b = &bay[h->hw_bay - 1];
    if (!(b->bay_flags & IsTotalFunction)) continue;
    if (b->bay_port != k + 1) continue;
  
    z = &zone[b->bay_zone - 1];

    if (h->hw_type == BL)
    {
      sprintf(buf, "%04d09%03d01",
        			h->hw_controller, h->hw_mod_address);
    }
    else if (h->hw_type == PM2 || h->hw_type == PM4)
    {
      sprintf(buf, "%04d09%03d%04d",
      			h->hw_controller, h->hw_mod_address, h->hw_mod);
    }
    else if (h->hw_type == PM6)
    {
      sprintf(buf, "%04d09%03d%06d",
      			h->hw_controller, h->hw_mod_address, h->hw_mod);
    }
    else if (h->hw_type == ZC2)
    {
      sprintf(buf, "%04d09%03dP%-4dZ%-4dB%-5d",
      			h->hw_controller, h->hw_mod_address,
      			z->zt_pl, z->zt_zone, b->bay_number);
    }
    else if (h->hw_type == ZC)				/* F010797 */
    {
      sprintf(buf, "%04d09%03dB%-4d",
      			h->hw_controller, h->hw_mod_address,
     				z->zt_pl, z->zt_zone, b->bay_number);
    }
    else continue;
    
    ac_write(fd, buf, strlen(buf));
  }
  if (called) pause();
  close_all();
  exit(0);
}
/*--------------------------------------------------------------------------*
 *  SKU & Stock Location Display                                               
 *--------------------------------------------------------------------------*/
test_sm()
{
  register struct bay_item *b, *c;
  register struct hw_item *h, *z;
  register struct pw_item *i;
  register long j, k;
  pmfile_item x;
  char buf[32];
  long ac, mod, what;
  
  if (sp->sp_config_status != 'y')
  {
    printf("CAPS Is Not Configured\r\n");
    return;
  }
  k = fork_k_ways();
  
  fd = ac_open(po[k].po_name);
  if (fd <= 0) return 0;
  
  ac_soft_reset(fd, 9999);

  database_open();
  
  pmfile_open(READONLY);
  pmfile_setkey(1);

  for (j = 0, h = hw; j < coh->co_hw_cnt; j++, h++)
  {
    if (!h->hw_bay) continue;
    b = &bay[h->hw_bay - 1];

    if (b->bay_port != k + 1) continue;

    if (h->hw_type == PM2 || h->hw_type == PM4)
    {
      sprintf(buf, "%04d09%03dXXXX", h->hw_controller, h->hw_mod_address);
      ac_write(fd, buf, strlen(buf));

      sprintf(buf, "%04d10%03dE", h->hw_controller, h->hw_mod_address);
      ac_write(fd, buf, strlen(buf));
      
      h->hw_current = 0;
    }
    else if (h->hw_type == PM6)
    {
      sprintf(buf, "%04d09%03dXXXXXX", h->hw_controller, h->hw_mod_address);
      			ac_write(fd, buf, strlen(buf));

      sprintf(buf, "%04d10%03dE", h->hw_controller, h->hw_mod_address);
      			ac_write(fd, buf, strlen(buf));
      
      h->hw_current = 0;
    }
  }
  while (1)
  {
    ac_read(fd, buf);
    
    if (memcmp(buf + 4, "10", 2) != 0) continue;

    sscanf(buf, "%04d", &ac);
    sscanf(buf + 6, "%03d", &mod);
    what = buf[9] - '0';

    for (j = 0, h = hw; j < coh->co_hw_cnt; j++, h++)
    {
      if (!h->hw_bay) continue;
      if (h->hw_type != PM2 &&
      	 h->hw_type != PM4 &&
      	 h->hw_type != PM6) continue;
          
      b = &bay[h->hw_bay - 1];
      if (b->bay_port != k + 1) continue;
      
      if (h->hw_controller == ac && h->hw_mod_address == mod) break;
    }
    if (j >= coh->co_hw_cnt) continue;

#ifdef DEBUG
    fprintf(stderr, "ac=%d mod=%d module=%d bay=%d zone=%d zc=%d\n",
    ac, mod, h->hw_mod, h->hw_bay, b->bay_zone, b->bay_zc);
#endif
    
    if (what)
    {
      sprintf(buf, "%04d10%03dB", ac, mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (h->hw_type == PM6)
    {
      sprintf(buf, "%04d09%03d______", ac, mod);
      ac_write(fd, buf, strlen(buf));
      sprintf(buf, "%04d10%03dF", ac, mod);
      ac_write(fd, buf, strlen(buf));
    }
    else
    {
      sprintf(buf, "%04d09%03d____", ac, mod);
      ac_write(fd, buf, strlen(buf));
      sprintf(buf, "%04d10%03dF", ac, mod);
      ac_write(fd, buf, strlen(buf));
    }
    z = 0;
    
    if (!b->bay_zc)
    {
      for (j = 0, c = bay; j < coh->co_bay_cnt; j++, c++)
      {
#ifdef DEBUG
        fprintf(stderr, "bay=%d zone=%d zc=%d\n",
        c->bay_number, c->bay_zone, c->bay_zc);
#endif
        
        if (!c->bay_zc) continue;
        if (c->bay_zone != b->bay_zone) continue;
        z = &hw[c->bay_zc - 1];
        break;
      }
      if (!z) continue;
    }
    else z = &hw[b->bay_zc - 1];

    if (b->bay_flags & VertLights)
    {
      if (what)
      {
        if (!h->hw_current) h->hw_current = h->hw_first;
        else
        {
          h->hw_current += 1;
          i = &pw[h->hw_current - 1];
          if ((i - pw) < b->bay_width)
          {
            h->hw_current = h->hw_first;
          }
        }
      }
      x.p_pmodno = h->hw_current;
    }
    else if (b->bay_flags & HortLights)
    {
      if (what)
      {
        if (!h->hw_current) h->hw_current = h->hw_first;
        else
        {
          h->hw_current += b->bay_width;
          i = &pw[h->hw_current - 1];
          if (i > &pw[b->bay_prod_last - 1])
          {
            h->hw_current = h->hw_first;
          }
        }
      }
      x.p_pmodno = h->hw_current;
    }
    else x.p_pmodno = h->hw_mod;
    
    if (what)										/* Switch Down - show SKU			  */
    {
      begin_work();
      if (!pmfile_read(&x, NOLOCK))
      {
        if (z->hw_type == ZC)					/* full function					     */
        {
          sprintf(buf, "%04d09%03d%5.5s",
        			z->hw_controller, z->hw_mod_address, x.p_pmsku);
        }
        else									/* Switch Up								  */
        {
          sprintf(buf, "%04d09%03d%-10.10sSKU   ",
        			z->hw_controller, z->hw_mod_address, x.p_pmsku);
        }
      }
      else
      {
        if (z->hw_type == ZC)					/* full function						  */
        {
          sprintf(buf, "%04d09%03d%-----",
        				z->hw_controller, z->hw_mod_address);
        }
        else
        {
          sprintf(buf, "%04d09%03d%NOT FOUND       ",
        				z->hw_controller, z->hw_mod_address);
        }
      }
      commit_work();
    }
    else												/* Switch Up - mod					  */
    {
      if (z->hw_type == ZC)
      {
        sprintf(buf, "%04d09%03d%-5d",
			z->hw_controller, z->hw_mod_address, x.p_pmodno);
      }
      else											/* Switch Up - show stkloc + mod	  */
      {
        begin_work();
        if (!pmfile_read(&x, NOLOCK))
        {
          sprintf(buf, "%04d09%03d%-6.6s    M%-5d",
				  z->hw_controller, z->hw_mod_address, x.p_stkloc, x.p_pmodno);
        }
        else
        {
          sprintf(buf, "%04d09%03d%NOT FOUND       ",
        		z->hw_controller, z->hw_mod_address);
        }
        commit_work();
      }
    }
    ac_write(fd, buf, strlen(buf));
  }
  stop_it(SIGINT);                              /* should never get here !!!       */
}
/*--------------------------------------------------------------------------*/
/*  Initialize All Ports                                                    */
/*--------------------------------------------------------------------------*/
fast_line_test()
{
  FILE *md;
  register long k, bl_count, zc_count, pm_count, bf_count;
  register long total_count, total_bl, total_zc, total_pm, total_bf;
  long pid, status;
  char md_name[40];
  
  struct
  {
    unsigned char ac;
    unsigned char mod;
    unsigned char type;
  } ac_rec;
  
  total_count = total_bl = total_zc = total_pm = total_bf = 0;
  
  printf("\r\n");
  printf("       Port       AC Count BL Count ZC Count PM Count  BF & IO\n\r");
  printf(" ---------------- -------- -------- -------- -------- --------\n\r");

  if (pfd)
  {
    fprintf(pfd, "\n\n\n");
    fprintf(pfd,
    "       Port       AC Count BL Count ZC Count PM Count  BF & IO\n");
    fprintf(pfd,
    " ---------------- -------- -------- -------- -------- --------\n");
  }
  printf("\r  ... Wait\r");
  fflush(stdout);
  
  for (k = 0; k < sp->sp_ports; k++)
  {
    if (!(po[k].po_flags & IsTotalFunction)) continue;
    if (port >= 0 && k != port) continue;
    
    if (fork() == 0)
    {
      strcpy(md_name, po[k].po_name);
      ss_close();
      co_close();
      if (pfd) fclose(pfd);
      
      execlp("alc_init", "alc_init", md_name, "-v", parm, 0);
      printf("Program alc_init Not Found\r\n\n");
      exit(1);
    }
    children++;
  }
  while(children > 0)
  {
    pid = wait(&status);
    children--;
    if (status || pid < 0) krash("line_test", "alc_init failed", 1);
  }
  for (k = 0; k < sp->sp_ports; k++)
  {
    if (!(po[k].po_flags & IsTotalFunction)) continue;
    if (port >= 0 && k != port) continue;

    fd = ac_open(po[k].po_name);
    if (fd > 0)
    {
      ac_reset(fd);
      ac_close(fd);
    }
    sprintf(md_name, "%s.%s", hw_name, basename(po[k].po_name));
    
    md = fopen(md_name, "r");
    if (md == 0) krash("line_test", md_name, 1);
    
    bl_count = zc_count = pm_count = bf_count = 0;

    while(fread(&ac_rec, 3, 1, md) == 1)
    {
      switch (ac_rec.type)
      {
        case BL:  bl_count++; break;
        case ZC:  zc_count++; break;
        case ZC2: zc_count++; break;
        case PM2: pm_count++; break;
        case PM4: pm_count++; break;
        case PM6: pm_count++; break;
        case BF:  bf_count++; break;
        case IO:  bf_count++; break;
        default:  break;
      }
    }
    printf(" %-16s %8d %8d %8d %8d %8d\r\n",
    		basename(po[k].po_name), ac_rec.ac,
    		bl_count, zc_count, pm_count, bf_count);
  
    if (pfd)
    {
      fprintf(pfd, " %-16s %8d %8d %8d %8d %8d\r\n",
      basename(po[k].po_name), ac_rec.ac,
      bl_count, zc_count, pm_count, bf_count);
    }
    total_count += ac_rec.ac;
    total_bl    += bl_count;
    total_zc    += zc_count;
    total_pm    += pm_count;
    total_bf    += bf_count;
  }
  printf(" Totals:          %8d %8d %8d %8d %8d\r\n\n",
  total_count, total_bl, total_zc, total_pm, total_bf);

  if (pfd)
  {
    fprintf(pfd, " Totals:          %8d %8d %8d %8d %8d\n",
    total_count, total_bl, total_zc, total_pm, total_bf);
  }
  return;
}
/*--------------------------------------------------------------------------*/
/*  Initialize All Ports                                                    */
/*--------------------------------------------------------------------------*/
line_test()
{
  FILE *md;
  register long k, bl_count, zc_count, pm_count, bf_count;
  register long total_count, total_bl, total_zc, total_pm, total_bf;
  long pid, status;
  char md_name[40];
  
  struct
  {
    unsigned char ac;
    unsigned char mod;
    unsigned char type;
  } ac_rec;
  
#ifdef DEBUG
  fprintf(stderr, "line_test() ports=%d pfd=%d\n", sp->sp_ports, pfd);
  fflush(stderr);
#endif
  
  total_count = total_bl = total_zc = total_pm = total_bf = 0;
  
  printf("\r\n");
  printf("       Port       AC Count BL Count ZC Count PM Count  BF & IO\n\r");
  printf(" ---------------- -------- -------- -------- -------- --------\n\r");

  if (pfd)
  {
    fprintf(pfd, "\n\n\n");
    fprintf(pfd,
    "       Port       AC Count BL Count ZC Count PM Count  BF & IO\n");
    fprintf(pfd,
    " ---------------- -------- -------- -------- -------- --------\n");
  }
  printf("\r  ... Wait\r");
  fflush(stdout);

#ifdef DEBUG
  fprintf(stderr, "loop\n");
  fflush(stderr);
#endif

  for (k = 0; k < sp->sp_ports; k++)
  {
#ifdef DEBUG
    fprintf(stderr, "\n\rk=%d\n", k);
    fflush(stderr);
#endif

    if (!(po[k].po_flags & IsTotalFunction)) continue;
    if (port >= 0 && k != port) continue;
    
#ifdef DEBUG
    fprintf(stderr, "alc_init %s\n", po[k].po_name);
    fflush(stderr);
#endif

    if (fork() == 0)
    {
      strcpy(md_name, po[k].po_name);
      ss_close();
      co_close();
      if (pfd) fclose(pfd);

      execlp("alc_init", "alc_init", md_name, "-v", parm, 0);
      printf("Program alc_init Not Found\r\n\n");
      exit(1);
    }
#ifdef DEBUG
    fprintf(stderr, "children=%d\n", children);
#endif

    children++;
    pid = wait(&status);
    children--;

    if (status || pid < 0) krash("line_test", "alc_init failed", 1);
    
    fd = ac_open(po[k].po_name);
    if (fd > 0)
    {
      ac_reset(fd);
      ac_close(fd);
    }
    sprintf(md_name, "%s.%s", hw_name, basename(po[k].po_name));
    
    md = fopen(md_name, "r");
    if (md == 0) krash("line_test", md_name, 1);
    
    bl_count = zc_count = pm_count = bf_count = 0;

    while(fread(&ac_rec, 3, 1, md) == 1)
    {
      switch (ac_rec.type)
      {
        case BL:  bl_count++; break;
        case ZC:  zc_count++; break;
        case ZC2: zc_count++; break;
        case PM2: pm_count++; break;
        case PM4: pm_count++; break;
        case PM6: pm_count++; break;
        case BF:  bf_count++; break;
        case IO:  bf_count++; break;
        default:  break;
      }
    }
    printf(" %-16s %8d %8d %8d %8d %8d\r\n",
    basename(po[k].po_name), ac_rec.ac,
    bl_count, zc_count, pm_count, bf_count);
  
    if (pfd)
    {
      fprintf(pfd, " %-16s %8d %8d %8d %8d %8d\r\n",
      		basename(po[k].po_name), ac_rec.ac,
      		bl_count, zc_count, pm_count, bf_count);
    }
    total_count += ac_rec.ac;
    total_bl    += bl_count;
    total_zc    += zc_count;
    total_pm    += pm_count;
    total_bf    += bf_count;
  }
  printf(" Totals:          %8d %8d %8d %8d %8d\r\n\n",
  		total_count, total_bl, total_zc, total_pm, total_bf);

  if (pfd)
  {
    fprintf(pfd, " Totals:          %8d %8d %8d %8d %8d\r\n\n",
    	total_count, total_bl, total_zc, total_pm, total_bf);
  }
  return;
}
/*--------------------------------------------------------------------------*/
/*  Fork For Each Port                                                      */
/*--------------------------------------------------------------------------*/
fork_k_ways()
{
  register long k;
  long status;
  
  children = 0;
  
  for (k = 0; k < sp->sp_ports; k++)
  {
    if (!(po[k].po_flags & IsTotalFunction)) continue;
    if (port >= 0 && k != port) continue;
    
    if (fork() == 0) {fd = 0; return k;}
    children++;
  }
  while (children)
  {
    wait(&status);
    children--;
  }
  printf("Test %s Done\r\n\n", test);
  stop_it(SIGINT);
}
/*--------------------------------------------------------------------------*
 *  Close All Open Files
 *--------------------------------------------------------------------------*/
close_all()
{
#ifdef DEBUG
  fprintf(stderr, "alc_diag: close_all()\n");
#endif

  if (pmfile_fd)
  {
    pmfile_close();
    database_close();
  }
  if (fd) {ac_reset(fd); ac_close(fd);}
  if (pfd)
  {
    fclose(pfd);
    unlink(pname);
  }
  pfd = 0;
  fd = 0;
  ss_close();
  co_close();

  return 0;
}
/*--------------------------------------------------------------------------*/
/*  Terminate Processing On Shutdown Event                                  */
/*--------------------------------------------------------------------------*/
void stop_it(int notUsed)
{
#ifdef DEBUG
  fprintf(stderr, "alc_diag: stop_it()  pid=%d\n",getpid());
#endif

  close_all();

  if (children) kill (0, SIGTERM);        /* kill children and self !!!      */

  exit(0);
}

/* end of alc_diag.c */
