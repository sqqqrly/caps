/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Suitcase utility routines.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/23/94   |  tjt  Added to mfc.
 *  12/13/94   |  tjt  Added tests 4 & 5.
 *-------------------------------------------------------------------------*/
static char alc_suitcase_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "file_names.h"

void stop_it(int signum);

long fd = 0;                              /* ac port                         */
long address;
char test[2];
long pid, status;

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=alc_suitcase");
  chdir(getenv("HOME"));

  signal(SIGTERM, stop_it);
  signal(SIGQUIT, stop_it);
  signal(SIGHUP, stop_it);
  
  printf("\r\n\n");
  
  strncpy(test, argv[1], 1);               /* test to perform                */
        
  switch (*test)
  {
    case 'L':   line_test();
                break;
                
    case 'H':
    case '1':   
    case '2':
    case '3':   
    case '4':
    case '5':   tests();
                break;
                
    case 'A':   fd = ac_open(alc_suitcase_name);
                if (fd <= 0)
                {
                  printf("\r\n%s\r\n", "Open Suitcase Line Failed");
                  pause();
                  stop_it(0); // Warning: signal handler is getting zero. Not a valid signum.
                }
                sscanf(argv[2], "%d", &address);
                ac_readdress(fd, address);
                printf("\r\n%s\r\n", "Readdress Is Complete");
                break;
                
    case 'D':   printf("\r\n%s\r\n", "Download Requres About 2 Minutes");
                fd = ac_open(alc_suitcase_name);
                ac_download(fd, 9999, alc_firmware_name);
                printf("\r\n%s\r\n", "Download Done");
                break;

    default:    printf("\r\n%s\r\n", "Bad Test Selection");
                break;
  }
  pause();
  stop_it(0); // Warning: signal handler is getting zero. Not a valid signum.
  exit(0);
}

/*--------------------------------------------------------------------------*/
/*  Test 0 through Test 3 and Test H
/*--------------------------------------------------------------------------*/
tests()
{
  if ((pid = fork()) == 0)
  {
    execlp("alc_diag_test", "alc_diag_test", alc_suitcase_name, test, "-s", 0);
    krash("tests", "load alc_diag_test", 1);
  }
  wait(&status);
  pid = 0;

  return 0;
}
/*--------------------------------------------------------------------------*/
/*  Initialize Port                                                         */
/*--------------------------------------------------------------------------*/
line_test()
{
  long status;
  
  if ((pid = fork()) == 0)
  {
    execlp("alc_init", "alc_init", alc_suitcase_name, "-v", "-s", 0);
    exit(1);
  }
  wait(&status);

  if (status) printf("\r\n%s\r\n", "Suitcase Line Test Failed");
  else printf("\r\n%s\r\n", "Suitcase Line Test Is Complete");
    
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Terminate Processing On Shutdown Event                                  
 *-------------------------------------------------------------------------*/
void stop_it(int signum)
{
  if (pid) kill(pid, SIGTERM);
  wait(&status);
  
  if (fd) 
  {
    ac_reset(fd);
    ac_close(fd);                         /* reset, flush, and close port    */
  }
  exit(0);
}

/* end of alc_suitcase.c */



