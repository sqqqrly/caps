/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Catches most signals and reports to errlog.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/01/96   | tjt  Original implementation.
 *  12/19/96   | tjt  Modified to ignore some and die on others.
 *-------------------------------------------------------------------------*/
static char signal_catcher_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <signal.h>

/*-------------------------------------------------------------------------*
 *  Signal Catch and Report
 *-------------------------------------------------------------------------*/
SignalHandler( Signal )
long Signal;
{
  static char *name[] = 
  {
   {"Unknown"},             {"SIGHUP - hangup"},      {"SIGINT - interrupt"},
   {"SIGQUIT - quit"},      {"SIGILL - illegal inst"},{"SIGTRAP - trap"},
   {"SIGIOT - iot inst"},   {"SIGEMT - emt inst"},    {"SIGFPE - floating pt"},
   {"SIGKILL - not caught"},{"SIGBUS - bus error"},   {"SIGSEGV - seg viol"},
   {"SIGSYS - bad arg"},    {"SIGPIPE - no reader"},  {"SIGALRM - timeout"},
   {"SIGTERM - terminate"}, {"SUGUSR1 - user sig 1"}, {"SIGUSR2 - user sig 2"}
  };
  char text[32];

  sprintf(text, "Signal=%d %s", Signal, Signal > 17 ? name[0] : name[Signal]);

  krash("SignalCatcher", text, 0);
  
  switch ( Signal ) 
  {
    case SIGHUP:                          /* caught once then ignored        */
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
    case SIGALRM:
    case SIGUSR1:
    case SIGUSR2: signal( Signal, SignalHandler );
                  break;

    case SIGTRAP: signal( SIGTRAP, SignalHandler );  /* always caught        */
                  break;
                  
    case SIGILL:
    case SIGIOT:
    case SIGFPE:
    case SIGBUS:
    case SIGSEGV:
    case SIGSYS:
    case SIGPIPE: signal( Signal, SignalHandler );
                  break;
                  
    default:      signal( Signal, SignalHandler );  /* should never happen   */
                  break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Arm Signal Catching -- 0 is ignore all but krash.
 *                      -- 1 is ignore all.
 *                      -- 2 is catch  all.
 *                      -- 4 is catch  all but krash.
 *-------------------------------------------------------------------------*/
signal_catcher(no_krash)
unsigned long no_krash;
{
  register long k;
  
  no_krash = no_krash & 3;
  
  if (no_krash < 2)
  {
    for (k = 1; k < 18; k++)
    {
      if (!no_krash && k == SIGTRAP) signal(SIGTRAP, SIG_DFL);
      else signal(k, SIG_IGN);
    }  
    return 0;
  }
  if (no_krash) signal( SIGTRAP, SignalHandler );
  
  signal( SIGHUP,  SignalHandler );
  signal( SIGINT,  SignalHandler );
  signal( SIGQUIT, SignalHandler );
  signal( SIGILL,  SignalHandler );
  signal( SIGIOT,  SignalHandler );
  signal( SIGFPE,  SignalHandler );
  signal( SIGBUS,  SignalHandler );
  signal( SIGSEGV, SignalHandler );
  signal( SIGSYS,  SignalHandler );
  signal( SIGPIPE, SignalHandler );
  signal( SIGALRM, SignalHandler );
  signal( SIGTERM, SignalHandler );
  signal( SIGUSR1, SignalHandler );
  signal( SIGUSR2, SignalHandler );

  return 0;
}

/* end of SignalHandler.c */
