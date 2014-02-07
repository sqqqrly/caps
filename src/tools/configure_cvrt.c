/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Convert old full function configuration to new.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/28/93   |  tjt  Original implementation.
 *  05/25/94   |  tjt  Add usage error message.
 *-------------------------------------------------------------------------*/
static char configure_cvrt_c[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *
 *  Convert Configuration Text.
 *
 *    config  [old config name]  [new config name]
 *
 *    return 0 = valid configuration
 *           1 = invalid parms
 *           2 = not enough space
 *           3 = has errors
 *-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global_types.h"

extern unsigned char gnc();
#define END    0x7f

FILE *fd = 0;
char fd_name[64];                         /* input configuration name        */

FILE *cd;
char cd_name[64];                         /* output configuration name       */

FILE *ed;
char ed_name[16];                         /* error file name                 */

#define LINE 128

unsigned char buffer[LINE + 1] = {0};     /* input line biffer               */
unsigned char backup[LINE] = {0};         /* backup buffer                   */

unsigned char *next = buffer;             /* next input character            */
unsigned char *last = backup;             /* next backup character           */

long line   = 0;                          /* input line number               */
long errors = 0;                          /* error message count             */

typedef struct
{
  TBay     bay_number;                    /* number of this bay              */
  TModule  first_module;                  /* first module in bay             */
  TModule  last_module;                   /* last module in bay              */
  TModule  bay_lamp;                      /* bay lamp number                 */
  TModule  zone_controller;               /* zone controller number          */
  short    barrier;                       /* bay has a barrier               */
   
} bay_item;

bay_item *bay;                            /* bay table                       */
long     last_bl = 0;                     /* highest bay lamp                */
long     last_zc = 0;                     /* highest zone controller         */
long     last_bno = 0;                    /* highest bay number              */

main(argc, argv)
long argc;
char **argv;
{
/*-------------------------------------------------------------------------*
 *  Open input file and shared segment file.
 *-------------------------------------------------------------------------*/
 
  if (argc < 3)                           /* must have config file names     */
  {
    fprintf(stderr, "Usage: configure_cvrt [old_name]  [new_name]\n");
    fprintf(stderr, "       DO NOT specify directory.\n");
    fprintf(stderr, "       Both names MUST be in /mfc/config\n\n");
    leave(1);
  }
  chdir(getenv("HOME"));                  /* to home directory               */

  strcpy(fd_name, "config/");             /* make config file name           */
  strcat(fd_name, argv[1]);

  strcpy(cd_name, "config/");             /* make config segment name        */
  strcat(cd_name, argv[2]);
  
  open_all_files();                       /* open all files                  */
  
  collect();                              /* gather bay information          */
  validate();                             /* check bay information           */
  copy_rest();                            /* copy all but bays               */

  if (errors) leave(3);
  leave(0);
}
/*-------------------------------------------------------------------------*
 *  Error Message
 *-------------------------------------------------------------------------*/
message(p)
register char *p;
{
  register long k;
   
  k = next - buffer + 5;

  fprintf(ed, "*** %s\n", p);
  fprintf(ed, "%*.*sV\n", k, k, " ");
  fprintf(ed, "%4d: %s\n", line, buffer);

  next = buffer;
  last = backup;
  *next = 0;
   
  errors++;
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Collect Input Configuration Bays
 *-------------------------------------------------------------------------*/

collect()
{
  while (1)
  {
    if (get_symbol('\n'))  continue;      /* is null line                    */
    if (get_symbol(END))   break;
    if (get_mbl())         continue;
    if (get_bay())         continue;

    next = buffer;
    last = backup;                        /* reject line                     */
    *next = 0;
   
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Validate Bay Information
 *-------------------------------------------------------------------------*/

validate()
{
  register long k, n, last_zc;
  register bay_item *b;

  last_zc = 0;

  for (k = 0, b = bay; k < last_bl; k++, b++)
  {
    n = b->zone_controller;
    if (!n) continue;
      
    if (n < last_zc)
    {
      fprintf(ed, "*** Zone Controller ZC%d Out Of Order\n", n);
      errors++;
      continue;
    }
    for (last_zc++; last_zc < n; last_zc++)
    {
      fprintf(ed, "*** Zone Controller ZC%d Is Missing\n", last_zc);
      errors++;
    }
  }
  for (k = 0, b = bay; k < last_bl; k++, b++)
  {
    if (!b->bay_number) b->bay_number = ++last_bno;

    n = b->bay_lamp;
    if (n) continue;

    fprintf(ed, "*** Bay Lamp BL%d Is Missing\n", k + 1);
    errors++;
  }
  for (k = 1, b = bay; k <= last_bl; k++, b++)
  {
    fprintf(cd, "B%d=", k);
    if (b->first_module)
    {
      fprintf(cd, "%d,", b->last_module - b->first_module + 1);
    }
    fprintf(cd, "BL");
    if (b->zone_controller) fprintf(cd, ",ZC");
    if (b->barrier) fprintf(cd, ",X");

    if (b->first_module) fprintf(cd, "   * Old Bay %d", b->bay_number);

    fprintf(cd, "\n");
  }
}

/*-------------------------------------------------------------------------*
 *  Copy All But Bay Definitions
 *-------------------------------------------------------------------------*/
 
copy_rest()
{
  fseek(fd, 0, 0);                        /* reset input                     */

  while (1)
  {
    gnl();                                /* get a line of input             */
    last = backup;
      
    if (*buffer == END) break;            /* is end of file                  */
      
    if (convert_mbl()) continue;          /* convert mbl + discard bays      */
    if (convert_zone()) continue;
      
    fprintf(cd, "%s", buffer);            /* copy other data                 */
  }
}

/*-------------------------------------------------------------------------*
 *  Get Bay Definition
 *-------------------------------------------------------------------------*
 *  B[0-9]+ = [0-9]+ (-[0-9]+)?, BL[0-9]+ (, ZC[0-9]+)?      
 *-------------------------------------------------------------------------*/
get_bay()
{
  long bno, mod_low, mod_high, bl, zc, bar;
  register bay_item *b;
  
  if (!get_symbol('B')) return 0;         /* not a bay identifier            */

  if (!get_bay_no(&bno)) return 0;

  if (!get_symbol('=')) return message("Equals Expected");

  if (!get_module_range(&mod_low, &mod_high)) return 0;

  if (!get_symbol(',')) return message("Comma Expected");
  
  if (!get_bay_lamp(&bl)) return 0;
  
  if (get_symbol(','))
  {
    if (!get_zone_controller(&zc)) return 0;
  }
  else zc = 0;
  
  if (!get_symbol('\n')) return message("Unrecognized");

  if (bno > 20000) {bar = 1; bno -= 20000;}
  else bar = 0;
  
  b = &bay[bl - 1];                       /* bay is bay lamp number !!!      */
  if (b->bay_lamp) return message("Duplicate Bay Lamp");
  
  b->bay_number      = bno;               /* save bay information            */
  b->first_module    = mod_low;
  b->last_module     = mod_high;
  b->bay_lamp        = bl;
  b->zone_controller = zc;
  b->barrier         = bar;
  
  if (bl > last_bl)   last_bl  = bl;      /* save highest bay lamp           */
  if (bno > last_bno) last_bno = bno;     /* save highest bay number         */
  
  return 1;
}
      
/*-------------------------------------------------------------------------*
 *  Get Master Bay Lamp
 *-------------------------------------------------------------------------*/

get_mbl()
{
  register long n;
  register bay_item *b;

  if (!get_symbol('B')) return 0;
  if (!get_symbol('L')) {*last++ = 'B'; return 0;}
   
  n = get_number();
   
  if (n <= 0) return message("Master Bay Number Expected");
   
  b = &bay[n - 1];
   
  if (b->bay_lamp) return message("Duplicate Assignment");

  b->bay_lamp = n;

  next = buffer;                          /* ignore rest of line             */
  last = backup;
  *next = 0;
   
  return 1;
}
      
/*-------------------------------------------------------------------------*
 *  Convert Master Bay Lamp - Discard Bays
 *-------------------------------------------------------------------------*/

convert_mbl()
{
  register long j, n;
  register bay_item *b;
  
  if (!get_symbol('B')) return 0;

  if (!get_symbol('L'))
  {
    next = buffer;
    last = backup;
    *next = 0;
    return 1;
  }
  n = get_number();
   
  if (n <= 0) return message("Master Bay Number Expected");
   
  fprintf(cd, "MB%d=", n);
  
  if (!get_symbol('=')) return message("Equals Expected");

  while (1)
  {
    if (!get_symbol('B')) return message("Bay Lamp Expected");
    if (!get_symbol('L')) return message("Bay Lamp Expected");

    n = get_number();
    if (n <= 0)      return message("Bay Lamp Number Expected");
    if (n > last_bl) return message("Bay Is Undefined");

    fprintf(cd, "B%d", n);
      
    if (get_symbol('\n')) break;
    if (!get_symbol(',')) return message("Unrecognized");
      
    fprintf(cd, ",");
  }
  fprintf(cd, "\n");
  return 1;
}
/*-------------------------------------------------------------------------*
 *  Convert Zones And Renumber Bays
 *-------------------------------------------------------------------------*/

convert_zone()
{
  register bay_item *b;
  register long k, n;

  if (!get_symbol('Z')) return 0;

  n = get_number();
   
  if (n <= 0) return message("Zone Number Expected");
   
  fprintf(cd, "Z%d=", n);
  
  if (!get_symbol('=')) return message("Equals Expected");

  while (1)
  {
    if (get_symbol('\n')) break;
    if (get_symbol(','))  {fprintf(cd, ","); continue;}
    if (get_symbol('-'))  {fprintf(cd, "-"); continue;}
    if (!get_symbol('B')) return message("Bay Expected");

    n = get_number();
    if (n > 20000) n -= 20000;
      
    if (n <= 0) return message("Bay Number Expected");

    for (k = 1, b = bay; k <= last_bl; k++, b++)
    {
      if (b->bay_number == n) break;
    }
    if (k > last_bl) return message("Bay Not Defined");

    fprintf(cd, "B%d", k);
  }
  fprintf(cd, "\n");
  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get A Bay Number -  B[0-9]+ 
 *-------------------------------------------------------------------------*/

get_bay_no(x)
register long *x;
{
  *x = 0;

  *x = get_number();
   
  if (*x <= 0) return message("Bay Number Expected");

  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get A Module Range  -  [0-9]? (-[0-9]+)?
 *-------------------------------------------------------------------------*/

get_module_range(x, y)
register long *x, *y;
{
  *x = get_number();
   
  if (*x <= 0) return message("Module Number Expected");
   
  if (get_symbol('-'))
  {
    *y = get_number();
    if (*y <= 0) return message("Module Number Expected");
  }
  else *y = *x;
  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get A Bay Lamp - BL[0-9]+
 *-------------------------------------------------------------------------*/

get_bay_lamp(x)
register long *x;
{
  if (get_symbol('T'))
  {
    message("Basic Function Configuration - No Conversion");
    leave(3);
  }
  if (!get_symbol('B')) return message("Bay Lamp Expected");
  if (!get_symbol('L')) return message("Bay Lamp Expected");

  *x = get_number();
   
  if (*x <= 0) return message("Bay Lamp Number Expected");
  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get A Zone Contoller  -  ZC[0-9]+
 *-------------------------------------------------------------------------*/

get_zone_controller(x)
register long *x;
{
  if (!get_symbol('Z')) return 0;
  if (!get_symbol('C')) return message("Zone Controller Expected");

  *x = get_number();
   
  if (*x <= 0) return message("Zone Controller Number Expected");
  return 1;
}

/*-------------------------------------------------------------------------*
 *  Get a symbol
 *-------------------------------------------------------------------------*/

get_symbol(x)
register unsigned char x;
{
  register unsigned char c;
   
  bypass_remarks();                       /* skip over any remarks           */

  c = gnc();                              /* get next byte                   */

  if (c >= 'a' && c <= 'z') c -= 0x20;    /* to uppercase                    */

  if (c == x) return 1;

  *last++ = c;                            /* unget current byte              */
  return 0;
}

/*-------------------------------------------------------------------------*
 *   Get a number - Return of zero is nothing found.
 *-------------------------------------------------------------------------*/

get_number()
{
  register unsigned char c;
  register long x;
   
  bypass_remarks();                       /* skip over any remarks           */

  x = 0;                                  /* clear number                    */

  while (1)                               /* convert number                  */
  {
    c = gnc();
    if (c < '0' || c > '9') break;
      
    x = 10 * x + (c - '0');
  }
  *last++ = c;                            /* unget last byte                 */

  return x;
}

/*-------------------------------------------------------------------------*
 *  Bypass whitespace and remarks
 *-------------------------------------------------------------------------*/
bypass_remarks()
{
  register unsigned char c;

  while (1)
  {
    c = gnc();
    if (c == 0x20 || c == 0x09) continue;
    break;
  }
  if (c == '*')
  {
    while (1)
    {
      c = gnc();
      if (c == END || c == '\n') break;
    }
  }
  *last++ = c;
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Get next character
 *-------------------------------------------------------------------------*/
unsigned char gnc()
{
  register long ret;
  register unsigned char c;
  
  if (last > backup) return *(--last);    /* pop a backup character          */

  if (!*next) gnl();                      /* get another line                */

  c = *next++;                            /* get the byte                    */

  return c;                               /* return a byte                   */
}

/*-------------------------------------------------------------------------*
 *  Get a line of input
 *-------------------------------------------------------------------------*/
gnl()
{
  next = buffer;                          /* reset pointer                   */

  memset(buffer, 0, LINE + 1);            /* clear the buffer                */

  if (!fgets(buffer, LINE, fd))           /* get next line                   */
  {
    memset(buffer, END, LINE);            /* fill with end of file           */
  }
  line++;                                 /* count number of lines           */
  return 0;
}

/*-------------------------------------------------------------------------*
 *  Close files and exit.
 *-------------------------------------------------------------------------*/
leave(n)
register long n;
{
  char command[80];

  if (fd) fclose(fd);                     /* close input file                */
  if (cd) fclose(cd);                     /* close output file               */
  if (ed) fclose(ed);                     /* close error file                */

  if (n)
  {
    sprintf(command, "rm %s; cat %s; rm %s", cd_name, ed_name, ed_name);
    system(command);
  }
  exit(n);                                /* exit with code                  */
}

/*-------------------------------------------------------------------------*
 *  Open files and setup needed pointers
 *-------------------------------------------------------------------------*/
open_all_files()
{
  register long size;

  tmp_name(ed_name);                      /* error file name                 */
  ed = fopen(ed_name, "w");               /* open error file                 */
  if (ed == 0) leave(1);                  /* open errors failed              */
  
  fd = fopen(fd_name, "r");               /* open config input file          */
  if (fd == 0) leave(1);                  /* open input failed               */

  cd = fopen(cd_name, "w");               /* open config output file         */
  if (cd == 0) leave(1);                  /* open output failed              */

  bay = (bay_item *)malloc(sizeof(bay_item) * BayMax);
  if (!bay) leave(2);
  memset(bay, 0, BayMax * sizeof(bay_item));
  
  return 0;
}

/* end of configure_cvrt.c */
