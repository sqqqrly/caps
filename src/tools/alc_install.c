/*-----------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Single Port Area Controller Test.
 *
 *  alc_install
 *
 *  Test 1 - Steady 88's.
 *  Test 2 - Flashing 11's.
 *  Test 3 - Flashing Index.
 *  Test 4 - Readdress
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/03/94   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char alc_install_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <string.h>
#include <signal.h>

long fd;
char fd_name[32];
char test;

long low_cm = 0;
long high_cm = 0;
long nth = 1;
long pid;
long status;

void leave();

char work[16];

main (argc, argv)
long argc;
char **argv;
{
  putenv("_=alc_install");
  chdir(getenv("HOME"));
  
  signal(SIGTERM, SIG_IGN);
  signal(SIGQUIT, leave);
  signal(SIGINT,  leave);
  signal(SIGHUP,  leave);
  
  printf("ALC Installation Test\n\n");

  while (1)
  {
    get_parms();
  }
}
/*-------------------------------------------------------------------------*
 *  Get Parameters
 *-------------------------------------------------------------------------*/
get_parms()
{
  long n;

  while (1)
  {
    printf("Enter Port Number    ---> ");
    gets(work);
    if (strlen(work) < 1) continue;

    sscanf(work, "%d", &n);
    if (n < 4 || n > 11) 
    {
      printf("Must 4 .. 11\n\n");
      continue;
    }
    sprintf(fd_name, "/dev/tty%d", n);
    fd = ac_open(fd_name);
    if (fd <= 0)
    {
      printf("Cannot Open Port %s\n\n", fd_name);
      continue;
    }
    break;
  }
  printf("\n");
  printf("  1 - Flashing 88's\n");
  printf("  2 - Flashing 11's\n");
  printf("  3 - Flashing Index\n");
  printf("  4 - Readdress\n\n");
  
  while (1)
  {
    printf("Enter Test Number    ---> ");
    gets(work);
    test = work[0];
    
    if (test < '1' || test > '4')
    {
      printf("Invalid Test Number %s\n\n", work);
      continue;
    }
    break;
  }
  if (test != '4')
  {
    while (1)
    {
      printf("Enter Low  CM Number ---> ");
      gets(work);
      sscanf(work, "%d", &low_cm);
      if (low_cm < 1)
      {
        printf("Invalid CM Number %s\n\n", work);
        continue;
      }
      break;
    }
    high_cm = low_cm;

    while (1)
    {  
      printf("Enter High CM Number ---> ");
      gets(work);
      if (!work[0]) break;
      sscanf(work, "%d", &high_cm);
      if (high_cm < low_cm)
      {
        printf("Invalid CM Number %s\n\n", work);
        continue;
      }
      break;
    }
    while (1)
    {
      printf("Enter Increment      ---> ");
      gets(work);
      if (!work[0]) break;
      sscanf(work, "%d", &nth);
      if (nth < 1 || nth > 10)
      {
        printf("Must Be 1 .. 10\n\n");
        continue;
      }
      break;
    }
  }
  printf("\n");
  printf("Port:      %s\n", fd_name);
  printf("Test:      %c\n", test);
  if (test != '4')
  {
    printf("Low CM:    %d\n", low_cm);
    printf("High CM:   %d\n", high_cm);
    printf("Increment: %d\n\n", nth);
    if ((pid = fork() == 0))
    {
      signal(SIGTERM, leave);
      run_test();
    }
    gets(work);
    kill(pid, SIGTERM);
    wait(&status);
    return 0;
  }
  n = ac_readdress(fd, 1); 
  printf("%d CM readdressed\n", n);

  gets(work);
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
void leave()
{
  ac_close(fd);
  printf("\nAll Done\n\n");
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Installation Tests
 *-------------------------------------------------------------------------*/
run_test()
{
  register long j, k, ret, len;
  char buf[32];
  char type[256];
  long ac, mod, what, cycle, value, eleven, count;
  
  ac_write(fd, "999901", 6);               /* soft reset                     */
  sleep(5);

  while ((len = ac_readout(fd, type, 2)) > 0) 
  {
    type[len] = 0;
    printf("Flushing Input Line: [%s]\n", type);
  }
  eleven = cycle = 0;
  

  while (1)
  {
    cycle = (cycle + 1) % nth;             /* cycle counter                  */
    if (!cycle) eleven = (eleven + 1) % 10;/* elevens counter                */
    
    if (!count) printf("\r%-79s\n", "*** No CM's Found ***");
    else printf("\r%79s\r", " ");
    fflush(stdout);
    
    count = 0;

    for (k = low_cm; k <= high_cm; k++)
    {
      sprintf(buf, "%04d02", k);
      ac_write(fd, buf, 6);
      ret = len = ac_readout(fd, type, 3);

      if (ret < 0) break;
   
      if (!ret) len = strlen(type);
      
      if (memcmp(type + 4, "02", 2) != 0 || ret <= 0)
      {
        type[len] = 0;
        if (ret <= 0) printf("Bad Parity: [%s]\n", type);
        else printf("Unexpected Packet: [%s]\n", type);
        continue;
      }
      printf("%2.2s(%d) ", buf + 2, len - 6);
      fflush(stdout);
      count++;
      
      for (j = 6; j < len; j++)
      {
        if (type[j] != 'B')
        {
          sprintf(buf, "%04d10%03dE", k, j - 6);
          ac_write(fd, buf, strlen(buf));
        }
      }
      for (j = 6; j < len; j++)
      {
        if (type[j] == 'B')
        {
          sprintf(buf, "%04d09%03d01", k, j - 6);
          ac_write(fd, buf, strlen(buf));
        }
        else if (type[j] == 'P' && (!((j - 6 + cycle) % nth)))
        {
          if (test == '1')      value = 88;
          else if (test == '2') value = (((j - 6)/nth + eleven) % 10) * 11;
          else                  value = j - 6;

          sprintf(buf, "%04d09%03d  %02d", k, j - 6, value % 100);
          ac_write(fd, buf, strlen(buf));
        }
        else if (type[j] == 'Z')
        {
          sprintf(buf, "%04d09%03dCM%-3d%5d      ", k, j - 6, k, len - 6);
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
        if (type[mod + 6] == 'P') sprintf(buf, "%04d09%03d  PP", ac, mod);
        else
        {
          if (what == 1)      sprintf(buf, "%04d09%03d%-16s", ac, mod, "EE");
          else if (what == 2) sprintf(buf, "%04d09%03d%-16s", ac, mod, "NN");
          else                sprintf(buf, "%04d09%03d%-16s", ac, mod, "BB");
        }
        ac_write(fd, buf, strlen(buf));
      }
      for (j = 6; j < len; j++)
      {
        if (type[j] == 'P' && (!((j - 6 + cycle) % nth)))
        {
          sprintf(buf, "%04d10%03dB", k, j - 6);
          ac_write(fd, buf, strlen(buf));
        }
        else
        {
          sprintf(buf, "%04d10%03dB", k, j - 6);
          ac_write(fd, buf, strlen(buf));
        }
      }
    }
  }
}

/* end of alc_install.c */
