/*-------------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Print shorts in transaction file.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  08/13/93   |  tjt  Added to mfc.
 *  05/04/94   |  tjt  Six digit order number.
 *  08/23/96   |  tjt  Add begin and commit work.
 *-------------------------------------------------------------------------*/
static char transac_short_rpt_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Bard.h"
#include "ss.h"
#include "xt.h"

struct trans_item xt;

#define MAX_LINE        50
short   page, line;

main( argc, argv )
long    argc;
char*   argv[];
{
  char    *p, *getenv();

  putenv("_=transac_short_rpt");

  p = getenv("HOME");
  if (p) chdir( p );

/*
 * Main Loop
 */

  print_shorts( );

  exit( 0 );
}

print_shorts( )
{
  FILE*   lpr_fd;
  char lpr_name[16], command[80];
  short   ordr, ship, reqd;

  tmp_name(lpr_name);
  lpr_fd = fopen( lpr_name, "w" );

  database_open();

  xt_open();
  transaction_setkey(2);
  
  begin_work();
  
  while (!transaction_next(&xt, NOLOCK))
  {
    if( xt.xt_code != 'S' )         
    {
      commit_work();
      begin_work();
      continue;
    }
    if( ++line % MAX_LINE == 1 )    print_header( lpr_fd );

    ordr = atoi_n( xt.xt_quan1, sizeof(xt.xt_quan1) );
    ship = atoi_n( xt.xt_quan2, sizeof(xt.xt_quan2) );
    reqd = ordr - ship;

    fprintf( lpr_fd, "  %.15s %.2s/%.7s %.15s   %4d   %4d    %4d\n",
      xt.xt_sku_mod1, xt.xt_pl, xt.xt_on, xt.xt_con, ordr, ship, reqd);
    
    commit_work();
    begin_work();
  }
  commit_work();
  
  xt_close();
  database_close();
  
  fclose( lpr_fd );

  sprintf(command, "%s %s", "LPR", lpr_name);
  system(command);
  return 0;
}


print_header( fd )
FILE*   fd;
{
  long now;
  char str[32];

  if( page > 1 )  fprintf( fd, "\f" );

  time(&now);
  strcpy(str, ctime(&now));

  fprintf(fd, 
    "                            Short Report                 Page %d\n", 
    ++page);
    
  fprintf(fd,   "                     %s\n", str);

  fprintf(fd, 
    "  SKU/PM #         PL/Order  CustomerOrder   Ordered Shipped Short\n");

  fprintf(fd, 
    "  --------------- ---------- --------------- ------- ------- -----\n");
}

atoi_n( str, len )
char*   str;
short   len;
{
  char    tmpstr[8+1];

  strncpy( tmpstr, str, len );    tmpstr[len] = '\0';
  return  atoi( tmpstr );
}

/* end of transac_short_rpt.c  */
