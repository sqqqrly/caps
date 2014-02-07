/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Multiport communications test.  This one program does
 *                  everything.
 *
 *  Execution:      commtest [-m | -s] [count] [device .. ]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/25/95   |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char commtest_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <termio.h>

long master = 0;
long count;
long start;
long status;

main(argc, argv)
long argc;
char **argv;
{
  register long k;

  if (argc < 4)
  {
    printf("*** Need parameters\n\n");
    exit(1);
  }
  if (strcmp(argv[1], "-m") == 0) master = 1;
  else if (strcmp(argv[1], "-s") != 0)
  {
    printf("*** Must be either -m or -s parameter\n\n");
    exit(1);
  }
  count = atoi(argv[2]);                   /* number of messages             */

  start = time(0);

  for (k = 3; k < argc; k++)
  {
    if (fork() == 0) 
    {
      if (master) master_task(argv[k]);
      else        slave_task(argv[k]);
    }
  }
  while (wait(&status) > 0) {;}

  printf("All Have Terminated in %d Seconds\n", time(0) - start);
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Master Task
 *-------------------------------------------------------------------------*/
master_task(name)
register long *name;
{
  register long k;
  char buf[80], *ret;
  FILE *fd, *sd;
  
  sd = fopen(name, "w");
  if (sd == 0)
  {
    printf("*** Can't Open Port %s\n\n", name);
    exit(1);
  }
  fd = fopen(name, "r");
  if (fd == 0)
  {
    printf("*** Can't Open Port %s\n\n", name);
    exit(1);
  }
  sprintf(buf, "stty 9600 cs8 -cstopb -parenb ixon ixoff ixany -onlcr <%s",
    name);
  system(buf);

  ioctl(fileno(fd), TCFLSH, 0);
  ioctl(fileno(sd), TCFLSH, 1);

  printf("Started %s\n", name);
  start = time(0);
  
  for (k = 0; k < count; k++)
  {
    sprintf(buf, "Message %6d\n", k);
    fputs(buf, sd);
    fflush(sd);
    printf("%6d: %s", k, buf);
    sleep(1);
/*
    ret = fgets(buf, 64, fd);
    printf("%6d: ret=%d %s", k, ret, buf);
*/
  }
  fclose(fd);
  fclose(sd);
  
  printf("Master Device %s in %d Seconds\n", name, time(0) - start);

  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Slave Task
 *-------------------------------------------------------------------------*/
slave_task(name)
register long *name;
{
  register long k;
  char buf[80], c, *ret;
  FILE *fd, *sd;
  
  sd = fopen(name, "w");
  if (sd == 0)
  {
    printf("*** Can't Open Port %s\n\n", name);
    exit(1);
  }
  fd = fopen(name, "r");
  if (fd == 0)
  {
    printf("*** Can't Open Port %s\n\n", name);
    exit(1);
  }
  sprintf(buf, "stty 9600 cs8 -cstopb -parenb ixon ixoff ixany -onlcr <%s", 
    name);
  system(buf);

  ioctl(fileno(fd), TCFLSH, 0);
  ioctl(fileno(sd), TCFLSH, 1);

  ret = fgets(buf, 64, fd);
  printf("Started %s\n", name);
  /* fprintf(sd, "OK\n"); */
  start = time(0);

  for (k = 1; k < count; k++)
  {
    ret = fgets(buf, 64, fd);
    printf("%6d: ret=%d %s", k, ret, buf);
/*
    fprintf(sd, "OK\n");
    fflush(sd);
*/
  }
  fclose(fd);

  printf("Slave Device %s in %d Seconds\n", name, time(0) - start);

  exit(0);
}
  
/* end of commtest.c */
  
  
