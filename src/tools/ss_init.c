/*---------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Initialize system shared segment.
 *                  Call ss_init [-sp=text] [-rf=text] [-ss] 
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/21/93   |  tjt  Rewritten.
 *  05/24/94   |  tjt  Fix rt, ft, and eof for space is null.
 *  06/05/95   |  tjt  Update ss segment anytime.
 *-------------------------------------------------------------------------*/
static char ss_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_names.h"
#include "ss.h"
#include "ss_init.h"

FILE *fd;
char fd_name[40];
char rf_name[40];

struct ss_item x;

long errors = 0;
long flag   = 0;                         /* flag ==1 is update ss segment */
long verbose = 0;

main(argc, argv)
long argc;
char **argv;
{
  register long k;
  register long n;

  putenv("_=ss_init");                    /* name to environ                 */
  chdir(getenv("HOME"));                  /* insure in home directory        */
   
  strcpy(fd_name, sp_text_name);          /* default file names              */
  strcpy(rf_name, rf_text_name);

  for (k = 1; k < argc; k++)
  {
    if (memcmp(argv[k], "-sp=", 4) == 0)       strcpy(fd_name, argv[k] + 4);
    else if (memcmp(argv[k], "-rf=", 4) == 0)  strcpy(rf_name, argv[k] + 4);
    else if (strcmp(argv[k], "-ss") == 0)      flag = 1;
    else if (strcmp(argv[k], "-v")  == 0)      verbose = 1;
  }
  if (flag)                                /* update real ss segment         */
  {
    ss_open();
    memcpy(&x, ssi, sizeof(struct ss_item));
  }
  else memset(&x, 0, sizeof(struct ss_item));  /* make new ss segment        */

  printf("Initializing System Shared Segment\n\n");

  fd = fopen(fd_name, "r");
  if (fd == 0)
  {
    printf("*** Can't Open %s\n\n", fd_name);
    exit(1);
  }
  n = load_part(fd, &x.sp_tab, ss_data);
  if (n != sizeof(struct sp_item))
  {
    printf("*** sp_item size is %d bytes but %d initialized\n\n",
    sizeof(struct sp_item), n);
    errors++;
  }
  fclose(fd);

  printf("\n\nInitializing Record Format Parameters\n\n");
  fd = fopen(rf_name, "r");
  if (fd == 0)
  {
    printf("*** Can't Open %s\n\n", rf_name);
    exit(1);
  }
  n = load_part(fd, &x.rf_tab, rf_data);
  if (n != sizeof(struct rf_item))
  {
    printf("*** rf_item size is %d but %d initialized\n",
    sizeof(struct rf_item), n);
    errors++;
  }
  fclose(fd);

  if (!errors)
  {
    if (x.rf_tab.rf_rt  == 0x20) x.rf_tab.rf_rt  = 0;
    if (x.rf_tab.rf_ft  == 0x20) x.rf_tab.rf_ft  = 0;
    if (x.rf_tab.rf_eof == 0x20) x.rf_tab.rf_eof = 0;
  }
  if (!flag)
  {
    fd = fopen(ss_name, "w");
    if (fd == 0)
    {
      printf("*** Can't Open %s\n\n", ss_name);
      exit(1);
    }
    if (fwrite(&x, sizeof(struct ss_item), 1, fd) != 1)
    {
      printf("*** Write error on %s\n\n", ss_name);
      exit(1);
    }
    fclose(fd);
  }
  if (errors) 
  {
    if (flag) ss_close();
    printf("*** Had %d Errors ***\n", errors);
    printf("All Done\n\n");
    exit(1);
  }
  if (flag)
  {
    printf("Saving To Shared Segment\n");
    memcpy(ssi, &x, sizeof(struct ss_item));
    ss_close_save();
  }
  else ss_remove();
  
/*
  sp = &x.sp_tab;
  rf = &x.rf_tab;
*/

  printf("All Done\n\n");

  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Load A Portion
 *-------------------------------------------------------------------------*/
 
load_part(fd, p, q)
FILE *fd;
unsigned char *p;
Tinit_item *q;
{
  //unsigned char *memchr();
  unsigned char buf[128], *r, *s, *t;
  unsigned long work, count, len;
  unsigned short word;
   
  count = 0;

  while (fgets(buf, 128, fd))
  {
    printf("%s", buf);                    /* echo input line                 */

    if (*buf == '#') continue;            /* is remarks                      */
      
    if (verbose)
    {
      printf("type=%c length%d [%s]\n", 
        q->init_type, q->init_length, q->init_desc);
    }
    if (flag && !q->init_refresh)         /* not a updateable parameter      */
    {
      p     += q->init_length;
      count += q->init_length;
      q++;
      continue;
    }
    switch (q->init_type)
    {
      case 0:     printf("*** Too Many Input Lines\n\n");
                  return 0;
      
      case 'f':   if (flag && *p != 0x20) printf("*** Updated\n");
                  *p = 0x20;
                  break;

      case 'c':   if (flag && *p != *buf) printf("*** Updated\n");
      
                  *p = *buf;              /* single byte                     */
                  len = strlen(q->init_desc);
                  
                  s = memchr(q->init_desc, '(', len);

                  if (!s) break;          /* no values to check              */
                     
                  t = memchr(s + 1, *buf, len - (s - buf));

                  if (!t)
                  {
                    printf("*** Value %c Not In %s\n\n", *p, t);
                    errors++;
                  }
                  break;
         
      case 's':   s = memchr(buf, ':', q->init_length - 1);
                  if (!s) len = q->init_length - 1;
                  else len = s - buf;

                  for (t = buf + len; t < buf + q->init_length; t++) *t = 0;

                  for (s = buf + len - 1; s >= buf; s--)
                  {
                    if (*s > 0x20 && *s < 0x7f) break;
                    *s = 0;
                  }
                  if (flag && memcmp(p, buf, q->init_length)) 
                    printf("*** Updated\n");
                  memcpy(p, buf, q->init_length);
                  break;

      case 'b':   work = cvrt(buf);
                  if (work < q->init_min || work > q->init_max)
                  {
                    printf("*** Value %d Outside Limits (%d, %d)\n\n",
                    work, q->init_min, q->init_max);
                    errors++;
                  }
					   if (flag && *p != work) printf("*** Updated\n");

                  *p = work;
                  break;
                     
      case 'h':   word = cvrt(buf);
                  if (word < q->init_min || word > q->init_max)
                  {
                    printf("*** Value %d Outside Limits (%d, %d)\n\n",
                      word, q->init_min, q->init_max);
                    errors++;
                  }
					   if (flag && memcmp(p, &word, 2)) printf("*** Updated\n");
                  memcpy(p, &word, 2);
                  break;
         
      case 'w':   work = cvrt(buf);
                  if (work < q->init_min || work > q->init_max)
                  {
                    printf("*** Value %d Outside Limits (%d, %d)\n\n",
                    work, q->init_min, q->init_max);
                    errors++;
                  }
					   if (flag && memcmp(p, &work, 4)) printf("*** Updated\n");
                  memcpy(p, &work, 4);
                  break;
                     
      case 'z':   memset(p, 0, q->init_length);
                  break;

      default:    break;
    }
    p     += q->init_length;
    count += q->init_length;
    q++;
  }
  if (q->init_type)
  {
    printf("*** Not Enough Input Lines\n\n");
  }
  return count;
}
cvrt(p)
unsigned char *p;
{
  register long x;
   
  x = 0;
   
  while (*p >= '0' && *p <= '9')
  {
    x = 10 * x + (*p - '0');
    p++;
  }
  return x;
}

/* emd of ss_init.c */
