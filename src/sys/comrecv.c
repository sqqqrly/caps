/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Receive Order File Or SKU Maintenance File.
 *
 *  comrecv    [O | S]   where, O = Orders, S = SKU Maintenance
 *
 *  Exit values:
 *
 *      0       OK
 *      1       Error on open comm line
 *      2       Error on open data file
 *      3       Interrupted
 *      4       Premature EOF
 *     99       Invalid parameter passed 
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/10/94    |  Added to mfc from custom.
 * 12/13/94    |  Fix   close orders.dat
 *-------------------------------------------------------------------------*/
static char comrecv_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "sd.h"
#include "ss.h"
#include "file_names.h"

#define EOT 0x04                         /* EOF with ^icanon                */
#define LF  0x0a

extern  leave();

FILE*   sku_fd;
FILE*   tmp_fd;
FILE*   com_fd;

main( argc, argv )
long    argc;
char*   argv[];
{
  int     ret;

  putenv("_=comrecv");
  chdir(getenv("HOME"));
  
  com_open();
  if ( com_fd == 0) exit( 1 );

  signal( SIGTERM, leave );

  ss_open( );
  sd_open( leave );
  
  switch( *argv[1] )
  {
    case 'O':                             /* Order load                      */

      ret = receive_orders();
      break;

    case 'S':                             /* SKU load                        */

      ret = receive_sku();
      break;

    default:
      ret = 99;                           /* bad parameter passed            */
  }
  ss_close( );
  sd_close( );
  if ( com_fd ) {fclose( com_fd ); com_fd = 0;}
  if ( tmp_fd ) {fclose( tmp_fd ); tmp_fd = 0;}
  if ( sku_fd ) {fclose( sku_fd ); sku_fd = 0;}
  
  exit( ret );
}
/*-------------------------------------------------------------------------*
 * Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  ss_close();
  sd_close();

  if ( com_fd ) {fclose( com_fd ); com_fd = 0;}
  if ( tmp_fd ) {fclose( tmp_fd ); tmp_fd = 0;}
  if ( sku_fd ) {fclose( sku_fd ); sku_fd = 0;}

  exit( 3 );                              /* return code 3 = interrupt       */
}
/*-------------------------------------------------------------------------*
 * Open communication port
 *-------------------------------------------------------------------------*/
com_open()
{
  FILE *ttys;
  char  line[80];
  char *com_ttydev;
  char *strtok();
  char *ret;
  
  if( (ttys = fopen( "/etc/ttys", "r" )) == 0 ) return 0;

  while( ret = fgets( line, sizeof(line), ttys) )
  {
    if( !strcmp( strtok( line, ":" ), "com" ) ) break;
  }
  fclose( ttys );
  if (!ret) return 0;
  
  strtok( 0, ":" );                       /* Skip past 2nd field             */
  com_ttydev = strtok( 0, ":" );          /* 3rd field is device name        */

  if( (com_fd = fopen( com_ttydev, "r" )) == 0 ) return 0;

  return 0;
}
/*-------------------------------------------------------------------------*
 *      Receive order function
 *-------------------------------------------------------------------------*/
receive_orders()
{
  char    chbuf;
  int     count;
  char    command[128], text[64];
  char   t_name[] = "otext/orders.dat";

  if( (tmp_fd = fopen( t_name, "w" )) == 0 ) return 2;

  while( 1 )                              /* find a record preface           */
  {
    if( (chbuf = getc( com_fd )) < 0 ) return  4;

    chbuf &= 0x7f;
    
    if( chbuf == rf->rf_rp ) break;
  }
  count = 1;

  while( 1 )
  {
    if ( chbuf != LF) putc( chbuf, tmp_fd );

    if( chbuf == rf->rf_eof )             /* If end of file                  */
    {
      fflush( tmp_fd );
      fclose( tmp_fd );
      tmp_fd = 0;
      
      sd_cursor(0, 21, 28);
      sd_text("Order Input Started");
      sprintf( command, "bin/order_input <%s", t_name );
      system( command );
      sd_cursor(0, 22, 28);
      sd_text("Order Input Completed");

      break;
    }
    if( (chbuf = getc( com_fd )) < 0 ) return  4;

    chbuf &= 0x7f;

    if( chbuf == rf->rf_rp )
    {
      count++;
      putc( '\n', tmp_fd );
      sprintf(text, "Transmitted Orders = %d", count);
      sd_cursor(0, 20, 28);
      sd_text(text);
    }
  }
  return  0;
}
/*-------------------------------------------------------------------------*
 *      Receive SKU function
 *-------------------------------------------------------------------------*/
receive_sku()
{
  char    chbuf;
  char    t_name[40];

  tmp_name(t_name);
  if( (tmp_fd = fopen( t_name, "w+" )) == 0 ) return  2;

  while( (chbuf = getc( com_fd )) != EOT )
  {
    putc( chbuf, tmp_fd );
  }
  fseek( tmp_fd, 0, 0 );

  if( (sku_fd  = fopen( sku_batch_name, "a" )) == 0) return 2;

  while( (chbuf = getc( tmp_fd )) != EOF )
  {
    putc( chbuf, sku_fd );
  }
  return  0;
}

/* end of comrecv.c */
