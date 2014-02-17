/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Communications Input Menu
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 10/08/94    |  Added to mfc from custom.
 * 07/12/95    |  Fix status return.
 * 07/14/95    |  Fix stop commo.
 *-------------------------------------------------------------------------*/
static char com_menu_in_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "eh_nos.h"
#include "iodefs.h"
#include "sd.h"
#include "ss.h"
#include "com_menu_in.t"

extern leave();
void catcher(int signum); // Signal handler

short pid  = 0;
short stat = 0;

static short ONE = 1;
struct fld_parms fld1 = { 13,51,28,1,&ONE,"Select Function",'a' };

short rm;
unsigned char t;
char buf[2];

/*
 *      MAIN program flow
 */

main( argc, argv )
long    argc;
char*   argv[];
{
  putenv("_=com_menu_in");
  chdir(getenv("HOME"));

  /* setpgrp(); */

  sd_open(leave);
  ss_open();

/*
 *      Output Main Communications Menu
 */
  menu_out( );

/*
 *      This is the main loop for screen input and communications
 *      reception.
 */
  while ( 1 )
  {
    buf[0] = 0;

    t = sd_input(&fld1,sd_prompt(&fld1,0),&rm,buf,0);

    if( t == UP_CURSOR ) continue;

    if( t == EXIT )
    {
      if( pid )                           /* cannot exit while comm running  */
      {
        eh_post( LOCAL_MSG, "Comm Is Running" );
        continue;
      }
      leave();
    }
    buf[0] = tolower(buf[0]);

    if (buf[0] == 'r')      start_orders();
    else if (buf[0] == 's') stop_comm();
    else if (buf[0] == 'p') start_sku( );

    else eh_post(ERR_CODE, buf);
  }
}
/*-------------------------------------------------------------------------*
 *  Death of Child Catcher
 *-------------------------------------------------------------------------*/

void catcher(int signum)
{
  long lpid, stat;

  pid = 0;

  lpid = wait( &stat );

  switch( (stat >> 8) & 0xff )
  {
    case  0:

      eh_post( LOCAL_MSG, "Comm Has Terminated" );
      break;

    case  1:                              /* Error opening comm              */

      eh_post( ERR_OPEN, "comm line" );
      break;

    case  2:                              /* Error opening temp file         */

      eh_post( ERR_OPEN, "tempfile" );
      break;

    case  3:                              /* manual interrupt                */

      eh_post( LOCAL_MSG, "Comm Stopped" );
      break;

    case  4:                              /* premature EOF                   */

      eh_post( LOCAL_MSG, "Premature End of File" );
      break;

    case  99:

      eh_post( LOCAL_MSG, "Bad Parameter" );/* bad parameter                 */
      break;
  }
  //return 0;
}
/*-------------------------------------------------------------------------*
 *      Display communication menu
 *-------------------------------------------------------------------------*/
menu_out( )
{
  fix( com_menu_in );
  sd_screen_off( );
  sd_clear( 0 );
  sd_text( com_menu_in );
  sd_screen_on( );
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Start Order Input
 *-------------------------------------------------------------------------*/
start_orders( )
{
  if( pid )
  {
    eh_post( LOCAL_MSG, "Comm Is Running" );
    return 0;
  }
  if (sp->sp_order_input_anytime == 'n' && sp->sp_config_status == 'n')
  {
    eh_post(ERR_NO_CONFIG, 0);
    return;
  }
  eh_post( LOCAL_MSG, "Order Input Started" );

  signal( SIGCHLD, catcher );

  pid = fork( );
  if( pid == 0 )
  {
    ss_close();

    execlp("comrecv", "comrecv", "O", 0);
    krash("start_orders", "load comrecv", 1);
  }
  return  0;
}
/*-------------------------------------------------------------------------*
 *  Start SKU Batch File Input
 *-------------------------------------------------------------------------*/
start_sku( )
{
  if( pid )
  {
    eh_post( LOCAL_MSG, "Comm Is Running" );
    return  -1;
  }
  eh_post( LOCAL_MSG, "SKU Input Started" );

  signal( SIGCHLD, catcher );

  pid = fork( );
  if( pid == 0 )
  {
    ss_close();
    
    execlp( "comrecv", "comrecv", "S", 0);
    krash("start_sku", "load commo program", 1);
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Stop Communications Child Process
 *-------------------------------------------------------------------------*/
stop_comm( )
{
  if (!pid)
  {
    eh_post( LOCAL_MSG, "Comm Is Not Running");
    return 0;
  }
  kill( pid, SIGTERM );                   /* kill child                     */
  return  0;
}
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
leave()
{
  if (pid) stop_comm();                   /* only if shutdown occurs         */
  sp->sp_oi_mode = 0x20;
  ss_close( );
  sd_close( );
  execlp( "operm", "operm", 0 );
  krash("leave", "load operm", 1);
}

/* end of com_menu_in.c */
