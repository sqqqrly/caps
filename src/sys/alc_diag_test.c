/* #define DEBUG */
/*-----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Single Port Total Function Test.
 *
 *  alc_diag_test  [port name]  [test] [-v] [-s]
 *
 *  Test 1 - Steady - With Blank Switch Action.
 *  Test 2 - Flashing 0* With Start/Stop Switch Action.
 *  Test 3 - Flashing 11's.
 *  Test 4 - Self Test 11.
 *  Test 5 - Self Test 12.
 *  Test H - Hardware Index.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/21/94   |  tjt  Original implementation.
 *  07/06/94   |  tjt  88 tests replaced by - and *0.
 *  07/06/94   |  tjt  Add ac_soft_reset.
 *  08/22/94   |  tjt  Add self tet 11 & 12.
 *  03/23/95   |  tjt  Add ZC2, PM2, and PM4.
 *  06/07/97   |  tjt  Add PM6.
 *-------------------------------------------------------------------------*/
static char alc_diag_test_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "file_names.h"
#include "co.h"

extern stopit();

FILE *md;
char md_name[40];

long fd;
char port_name[32];
char test;
long verbose = 0;
char *sparm = 0;
long low_cm = 1;
long high_cm = 50;
long nth = 1;

typedef struct
{
  unsigned char ac;
  unsigned char mod;
  unsigned char type;
} ac_rec_item;

ac_rec_item *t;
long max;

main (argc, argv)
long argc;
char **argv;
{
  register long k;

  putenv("_=alc_diag_test");
  chdir(getenv("HOME"));
  
#ifdef DEBUG
  fprintf(stderr, "alc_diag_test: pid=%d pgrp=%d\n", getpid(), getpgrp());
#endif
  
  if (argc < 3)
  {
    krash("main", "Missing Parmeters", 1);
    exit(1);
  }
  signal(SIGTERM, stopit);
  signal(SIGINT,  stopit);
  signal(SIGHUP,  stopit);
  signal(SIGQUIT, stopit);

  strcpy(port_name, argv[1]);

  sprintf(md_name, "%s.%s", hw_name, basename(port_name));
    
  test = argv[2][0];

  for (k = 3; k < argc; k++)
  {
    if (strcmp(argv[k], "-v") == 0) verbose = 1;
    if (strcmp(argv[k], "-s") == 0) sparm = argv[k];
  }
  if (test != '4' && test != '5') line_test();

  fd = ac_open(argv[1]);
  if (fd <= 0)
  {
    krash("main", "Cannot Open Port", 1);
    exit(1);
  }
  switch (test)
  {
    case '1':  test_1(); break;
    case '2':  test_2(); break;
    case '3':  test_3(); break;
    case '4':  test_4(); break;
    case '5':  test_5(); break;
    case 'H':  test_h(); break;
    default:   printf("%s %c\r\n\n", "Invalid Parameter", test); break;
  }
  ac_close(fd);
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Test 1 - Steady - With Switch Action
 *-------------------------------------------------------------------------*/
test_1()
{
  register ac_rec_item *x;
  register long k;
  char buf[32];
  long ac, mod, what;
  
  if (verbose) printf("\r%s - %s\n", "Test 1 Started", port_name);

  ac_soft_reset(fd, 9999);

  for (k = 0, x = t; k < max; k++, x++)
  {
    if (x->type == BL)
    {
      sprintf(buf, "%04d09%03d01", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (x->type == PM2 || x->type == PM4)
    {
      sprintf(buf, "%04d09%03d----", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));

      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (x->type == PM6)
    {
      sprintf(buf, "%04d09%03d------", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));

      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (x->type == ZC2)
    {
      sprintf(buf, "%04d09%03d----------------", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    
      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
  }
  while (1)
  {
    ac_read(fd, buf);
    
    if (memcmp(buf + 4, "10", 2) != 0) continue;

    sscanf(buf, "%04d", &ac);
    sscanf(buf + 6, "%03d", &mod);
    what = buf[9] - '0';

    for (k = 0, x = t; k < max; k++, x++)
    {
      if (x->ac == ac && x->mod == mod) break;
    }
    if (k >= max) continue;

    if ((x->type == ZC2 && what == 3) || 
        (x->type == PM2 && what == 2) ||
        (x->type == PM4 && what == 2) ||
        (x->type == PM6 && what == 2))
    {
       sprintf(buf, "%04d10%03dB", ac, mod);
       ac_write(fd, buf, strlen(buf));
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Test 1a - Steady 88's  - NO LONGER ACTIVE
 *-------------------------------------------------------------------------*/
test_1a()
{
  register ac_rec_item *x;
  register long k;
  char buf[32];
  long ac, mod, what;
  
  ac_soft_reset(fd, 9999);

  for (k = 0, x = t; k < max; k++, x++)
  {
    if (x->type == BL)
    {
      sprintf(buf, "%04d09%03d01", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (t[k].type == PM2 || t[k].type == PM4)
    {
      sprintf(buf, "%04d09%03d8888", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));

      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (t[k].type == PM6)
    {
      sprintf(buf, "%04d09%03d888888", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));

      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (x->type == ZC2)
    {
      sprintf(buf, "%04d09%03d8888888888888888", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    
      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
  }
  while (1)
  {
    ac_read(fd, buf);
    
    if (memcmp(buf + 4, "10", 2) != 0) continue;

    sscanf(buf, "%04d", &ac);
    sscanf(buf + 6, "%03d", &mod);
    what = buf[9] - '0';

    if (what)
    {
       sprintf(buf, "%04d10%03dB", ac, mod);
       ac_write(fd, buf, strlen(buf));
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Test 2 - Flashing *0 - Any switch starts/stop test in CM
 *-------------------------------------------------------------------------*/
test_2()
{
  register ac_rec_item *x;
  register long k, ret, pass;
  char buf[32];
  char flag[1000]; 
  long ac, mod, what;
  
  if (verbose) printf("\r%s - %s\n", "Test 2 Started", port_name);

  memset(flag, 0, 1000);

  ac_soft_reset(fd, 9999);

#ifdef DEBUG
  fprintf(stderr, "ac + mod + type\n");
  Bdumpf(t, 3 * max, stderr);
#endif  
  for (k = 0, x = t; k < max; k++, x++)
  {
    if (x->type == PM2 || x->type == PM4 || x->type == PM6)
    {
      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));

      sprintf(buf, "%04d10%03dN", x->ac, x->mod);  /* no LED on              */
      ac_write(fd, buf, strlen(buf));
    }
    else if (x->type == ZC2)
    {
      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
  }
  pass = 0;

  while (1)
  {
#ifdef DEBUG
  fprintf(stderr, "Top of Loop\n");
  fflush(stderr);
#endif

    pass = (pass + 1) % 4;

    for (k = 0, x = t; k < max; k++, x++)
    {
      if (pass == 0 || pass == 2)
      {
        if (flag[x->ac])
        {
          sprintf(buf, "%04d10%03dB", x->ac, x->mod);
          ac_write(fd, buf, strlen(buf));
        }
        continue;
      }
      if (flag[x->ac] != 1) continue;

      if (x->type == BL)
      {
        sprintf(buf, "%04d09%03d01", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if ((x->type == PM2 || x->type == PM4) && pass == 1)
      {
        sprintf(buf, "%04d09%03d0000", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if ((x->type == PM2 || x->type == PM4) && pass == 3)
      {
        sprintf(buf, "%04d09%03d****", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if ((x->type == PM6) && pass == 1)
      {
        sprintf(buf, "%04d09%03d000000", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if ((x->type == PM6) && pass == 3)
      {
        sprintf(buf, "%04d09%03d******", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if (x->type == ZC2 && pass == 1)
      {
        sprintf(buf, "%04d09%03d0000000000000000", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if (x->type == ZC2 && pass == 3)
      {
        sprintf(buf, "%04d09%03d****************", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
    }
    for (k = 0; k < 1000; k++) 
    {
      if (flag[k] == 2) flag[k] = 0;
    }
    if (pass) continue;

    while (1)
    {
      ret = ac_readout(fd, buf, 1);
      if (ret <= 0) break;
      
      if (memcmp(buf + 4, "10", 2) != 0) continue;

      sscanf(buf, "%04d", &ac);
      sscanf(buf + 6, "%03d", &mod);
      what = buf[9] - '0';

      if (!what) continue;

      for (k = 0, x = t; k < max; k++, x++)
      {
        if (x->ac == ac && x->mod == mod) break;
      }
      if (k >= max) continue;
 
      if (!flag[x->ac]) flag[x->ac] = 1;
      else              flag[x->ac] = 2;
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Test 2a - Flashing 88's  -  NO LONGER ACTIVE
 *-------------------------------------------------------------------------*/
test_2a()
{
  register ac_rec_item *x;
  register long k, ret, pass;
  char buf[32];
  long ac, mod, what;
  
  ac_soft_reset(fd, 9999);

  for (k = 0, x = t; k < max; k++, x++)
  {
    if (t[k].type == PM2 || t[k].type == PM4 || t[k].type == PM6)
    {
      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (x->type == ZC2)
    {
      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
  }
  pass = 0;

  while (1)
  {
    pass ^= 1;

    for (k = 0, x = t; k < max; k++, x++)
    {
      if (pass)
      {
        sprintf(buf, "%04d10%03dB", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if (x->type == BL)
      {
        sprintf(buf, "%04d09%03d01", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if (t[k].type == PM2 || t[k].type == PM4)
      {
        sprintf(buf, "%04d09%03d  88", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if (t[k].type == PM6)
      {
        sprintf(buf, "%04d09%03d888888", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if (x->type == ZC2)
      {
        sprintf(buf, "%04d09%03d8888888888888888", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
    }
    while (1)
    {
      ret = ac_readout(fd, buf, 2);
      if (ret <= 0) break;
      
      if (memcmp(buf + 4, "10", 2) != 0) continue;

      sscanf(buf, "%04d", &ac);
      sscanf(buf + 6, "%03d", &mod);
      what = buf[9] - '0';

      if (!what)
      {
        sprintf(buf, "%04d10%03dB", ac, mod);
        ac_write(fd, buf, strlen(buf));
        continue;
      }
      for (k = 0, x = t; k < max; k++, x++)
      {
        if (x->ac == ac && x->mod == mod) break;
      }
      if (x->type == PM2 || x->type == PM4) 
      {
        sprintf(buf, "%04d09%03d  PP", ac, mod);
      }
      else if (x->type == PM6) 
      {
        sprintf(buf, "%04d09%03d    PP", ac, mod);
      }
      else
      {
        if (what == 1)      sprintf(buf, "%04d09%03d%-16s", ac, mod, "EE");
        else if (what == 2) sprintf(buf, "%04d09%03d%-16s", ac, mod, "NN");
        else                sprintf(buf, "%04d09%03d%-16s", ac, mod, "BB");
      }
      ac_write(fd, buf, strlen(buf));
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Test 3 - Flashing 11's
 *-------------------------------------------------------------------------*/
test_3()
{
  register ac_rec_item *x;
  register long k, n, ret, pass, base;
  char buf[32];
  long ac, mod, what;
  
  if (verbose) printf("\r%s - %s\n", "Test 3 Started", port_name);

  ac_soft_reset(fd, 9999);

  for (k = 0, x = t; k < max; k++, x++)
  {
    if (t[k].type == PM2 || t[k].type == PM4 || t[k].type == PM6)
    {
      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (x->type == ZC2)
    {
      sprintf(buf, "%04d10%03dE", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
  }
  base = pass = 0;

  while (1)
  {
    pass ^= 1;
    if (!pass) base = (base + 1) % 10;
    
    for (k = 0, x = t; k < max; k++, x++)
    {
      if (pass)
      {
        sprintf(buf, "%04d10%03dB", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if (x->type == BL)
      {
        sprintf(buf, "%04d09%03d01", x->ac, x->mod);
        ac_write(fd, buf, strlen(buf));
      }
      else if (t[k].type == PM2 || t[k].type == PM4)
      {
        n = 11 * ((base + k) % 10);

        sprintf(buf, "%04d09%03d  %02d", x->ac, x->mod, n);
        ac_write(fd, buf, strlen(buf));
      }
      else if (t[k].type == PM6)
      {
        n = 11 * ((base + k) % 10);

        sprintf(buf, "%04d09%03d    %02d", x->ac, x->mod, n);
        ac_write(fd, buf, strlen(buf));
      }
      else if (x->type == ZC2 && (base & 1))
      {
        sprintf(buf, "%04d09%03d%02d  %02d  %02d  %02d  ", x->ac, x->mod,
        11 * ((base + k) % 10), 11 * ((base + k + 1) % 10),
        11 * ((base + k + 2) % 10), 11 * ((base + k + 3) % 10));
        ac_write(fd, buf, strlen(buf));
      }
      else if (x->type == ZC2)
      {
        sprintf(buf, "%04d09%03d  %02d  %02d  %02d  %02d", x->ac, x->mod,
        11 * ((base + k) % 10), 11 * ((base + k + 1) % 10),
        11 * ((base + k + 2) % 10), 11 * ((base + k + 3) % 10));
        ac_write(fd, buf, strlen(buf));
      }
    }
    while (1)
    {
      ret = ac_readout(fd, buf, 2);
      if (ret <= 0) break;
      
      if (memcmp(buf + 4, "10", 2) != 0) continue;

      sscanf(buf, "%04d", &ac);
      sscanf(buf + 6, "%03d", &mod);
      what = buf[9] - '0';

      if (!what)
      {
        sprintf(buf, "%04d10%03dB", ac, mod);
        ac_write(fd, buf, strlen(buf));
        continue;
      }
      for (k = 0, x = t; k < max; k++, x++)
      {
        if (x->ac == ac && x->mod == mod) break;
      }
      if (x->type == PM2 || x->type == PM4) 
      {
        sprintf(buf, "%04d09%03d  PP", ac, mod);
      }
      else if (x->type == PM6) 
      {
        sprintf(buf, "%04d09%03d    PP", ac, mod);
      }
      else
      {
        if (what == 1)      sprintf(buf, "%04d09%03d%-16s", ac, mod, "EE");
        else if (what == 2) sprintf(buf, "%04d09%03d%-16s", ac, mod, "NN");
        else                sprintf(buf, "%04d09%03d%-16s", ac, mod, "BB");
      }
      ac_write(fd, buf, strlen(buf));
    }
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Test 4 - Self Test 11.
 *-------------------------------------------------------------------------*/
test_4()
{
  static char buf[] = "999911";
  
  if (verbose) printf("\r%s - %s\n", "Test 4 Started", port_name);

  ac_write(fd, buf, 6);

  pause();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Test 5 - Self Test 12.
 *-------------------------------------------------------------------------*/
test_5()
{
  static char buf[] = "999912";
  
  if (verbose) printf("\r%s - %s\n", "Test 5 Started", port_name);

  ac_write(fd, buf, 6);

  pause();
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Test H - Hardware Index
 *-------------------------------------------------------------------------*/
test_h()
{
  register ac_rec_item *x;
  register long k;
  char buf[32];
  
  if (verbose) printf("\r%s - %s\n", "Test H Started", port_name);

  ac_soft_reset(fd, 9999);

  for (k = 0, x = t; k < max; k++, x++)
  {
    if (x->type == BL)
    {
      sprintf(buf, "%04d09%03d01", x->ac, x->mod);
      ac_write(fd, buf, strlen(buf));
    }
    else if (t[k].type == PM2 || t[k].type == PM4)
    {
      sprintf(buf, "%04d09%03d%4d", x->ac, x->mod, x->mod + 1);
      ac_write(fd, buf, strlen(buf));
    }
    else if (t[k].type == PM6)
    {
      sprintf(buf, "%04d09%03d%6d", x->ac, x->mod, x->mod + 1);
      ac_write(fd, buf, strlen(buf));
    }
    else if (x->type == ZC2)
    {
      sprintf(buf, "%04d09%03dCM%-3d%5d      ", 
        x->ac, x->mod, x->ac, x->mod + 1);
      ac_write(fd, buf, strlen(buf));
    }
  }
  pause();
  return 0;
}

/*-------------------------------------------------------------------------*
 * Line Test Of One Port
 *-------------------------------------------------------------------------*/
line_test()
{
  register long k, bl_count, zc_count, pm_count;
  long pid, status;

  if (verbose)
  {
    printf("\r\n");
    printf("        Port        AC Count  BL Count  ZC Count  PM Count");
    printf("\n\r");
    printf("  ----------------  --------  --------  --------  --------\n\r");
  }
  if (fork() == 0)
  {
    execlp("alc_init", "alc_init", port_name, "-v", sparm, 0);
    printf("Program alc_init Not Found\r\n\n");
    exit(1);
  }
  pid = wait(&status);
    
  if (status || pid < 0)
  {
    printf("*** %s\r\n\n", "Line Test Failed");
    exit(1);
  }
  md = fopen(md_name, "r");
  if (md == 0)
  {
    printf("%s\r\n\n", "Can't Open Port Map");
    exit(1);
  }
  fseek(md, 0, 2);
  max = ftell(md) / 3;
  fseek(md, 0, 0);
  
  t = (ac_rec_item *)malloc(max * sizeof(ac_rec_item));

  for (k = 0; k < max; k++)
  {
    fread(&t[k], 3, 1, md);
  }
  bl_count = zc_count = pm_count = 0;

  for (k = 0; k < max; k++)
  {
    switch (t[k].type)
    {
      case BL:  bl_count++; break;
      case ZC2: zc_count++; break;
      case PM2: pm_count++; break;
      case PM4: pm_count++; break;
      case PM6: pm_count++; break;
      default:  break;
    }
  }
  if (verbose)
  {
    printf("  %-16s   %5d     %5d     %5d     %5d\r\n", 
      port_name, t[max - 1].ac, bl_count, zc_count, pm_count);
  }
  fclose(md);
  md = 0;
  return;
}
/*-------------------------------------------------------------------------*
 *  Stop Test
 *-------------------------------------------------------------------------*/
stopit()
{
  if (md) fclose(md);
  if (fd) {ac_reset(fd); ac_close(fd);}

#ifdef DEBUG
  fprintf(stderr, "alc_diag_test: leaving pid=%d\n", getpid());
#endif

  exit(0);
}

/* end of alc_diag_test.c */
