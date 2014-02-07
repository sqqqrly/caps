#define DEBUG
/*------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Model Communications Interface To CAPS Engine.
 *
 *  Execution:      engine_test
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  02/7/94    |  tjt  Original Implementation.
 *  05/03/94   |  tjt  Revised for ASCII messages. 
 *-------------------------------------------------------------------------*/
static char engine_test_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "caps_messages.h"
#include "message_types.h"
#include "engine_messages.h"
#include "engine_mess_types.h"

extern catcher();

unsigned char list[] = {ShutdownRequest, APUResponse};

main(argc, argv)
long argc;
char **argv;
{
  putenv("_=engine_test");
  chdir(getenv("HOME"));
  
  open_all();

  while (get_input()) {;}

  message_close();
  exit(0);
}
/*-------------------------------------------------------------------------*
 *  Get Input From The Terminal - Replace With Message Get From APU
 *-------------------------------------------------------------------------*/
get_input()
{
  char buf[32];
  long code, len, pickline, zone, order, newtote;
  TCEMessage x;

  printf("Walgreen's APU Simulator\n\n");
  printf("  1 = Start Engine\n");
  printf("  2 = Stop Engine\n");
  printf("  3 = Enable Pickline\n");
  printf("  4 = Disable Pickline\n");
  printf("  5 = Lock Pickline\n");
  printf("  6 = Unlock Pickline\n");
  printf("  7 = Stop Order Feeding\n");
  printf("  8 = Cancel Order\n");
  printf("  9 = Hold Order\n");
  printf(" 10 = Activate Order\n");
  printf(" 11 = Redisplay Pickline\n");
  printf(" 12 = Redisplay Zone\n");
  printf(" 13 = Send Zone Display\n");
  printf(" 14 = Process Order File\n");
  printf(" 15 = Login To Zone\n");
  printf(" 16 = Logoff Zone\n");
  printf(" 17 = Release Order To Zone\n");
  printf(" 18 = Change Tote Number\n");
  printf(" 19 = Make An Order\n\n");

  printf("Enter Selection ---> ");

  gets(buf);

  code = atol(buf);
  len  = 0;
  
  sprintf(&x, "%04d%04d", code, len);

  if (code == 0) return 0;

  if (code == 19)
  {
    system("create_orders -x");
    return 1;
  }
  if (code < 1 || code > 18)
  {
    printf("*** Invalid Selection\n\n");
    return 1;
  }
  if (code >= 3 && code <= 11 || code == 17 || code == 18)
  {
    printf("Enter Pickline  ---> ");
    gets(buf);
    pickline = atol(buf);
    len = 2;
    sprintf(&x, "%04d%04d%02d", code, len, pickline);
  }
  if (code >= 8 && code <= 10 || code == 17 || code == 18)
  {
    printf("Enter Tote     ---> ");
    gets(buf);
    order = atol(buf);
 
    len   = 8;
    sprintf(&x, "%04d%04d%02d%06d", code, len, pickline, order);
  }
  if (code == 18)
  {
    printf("Enter New Tote ---> ");
    gets(buf);
    newtote = atol(buf);
    printf("Enter Cust No  ---> ");
    gets(buf);
    
    len   = 21;
    sprintf(&x, "%04d%04d%02d%06d%-7.7s%06d", 
      code, len, pickline, order, buf, newtote);
  }
  if (code == 12 || code == 13 || code == 15 || code == 16 || code == 17)
  {
    printf("Enter Zone      ---> ");
    gets(buf);
    zone = atol(buf);
    if (code == 17)
    {
      len = 12;
      sprintf(&x, "%04d%04d%02d%06d%04d", code, len, pickline, order, zone);
    }
    else
    {
      len  = 4;
      sprintf(&x, "%04d%04d%04d", code, len, zone);
    }
  }
  if (code == 13)
  {
    printf("               V.........V.....\n");
    printf("Enter Text --> ");
    gets(buf);
    len = 20;
    
    sprintf(&x, "%04d%04d%04d%-16.16s", code, len, zone, buf);
  }
#ifdef DEBUG
  fprintf(stderr, "APU Request\n");
  Bdump(&x, len + 8);
#endif

  message_put(0, APURequest, &x, len + 8);

  if (code != 7  && code != 11 && code != 12 && 
      code != 13 && code != 16 && code != 18)
  {
    pause();
    sleep(2);
  }
  return 1;
}
/*-------------------------------------------------------------------------*
 *  CAPS Message Catcher
 *-------------------------------------------------------------------------*/
catcher(who, type, buf, len)
register long who, type;
register TCEMessage *buf;
register long len;
{
  switch (type)
  {
    case ShutdownRequest:
      
      message_close();
      exit(0);
    
    case APUResponse: 
     
      process_event(buf);
      break;
     
   default: break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Process Event
 *-------------------------------------------------------------------------*/
process_event(buf)
register unsigned char *buf;
{
  long code, len;
  
  sscanf(buf,     "%04d", &code);
  sscanf(buf + 4, "%04d", &len);

  printf("\n\n");

#ifdef DEBUG
  fprintf(stderr, "APU Event:\n");
  Bdump(buf, len + 8);
#endif

  switch (code)
  {
    case EngineStartEvent:                  /* 32 */
     
      printf("Engine Start %c\n", buf[8]); 
      break; 

    case EngineStopEvent:                   /* 33 */
     
      printf("Engine Stop %c\n", buf[8]); 
      break;
       
    case EngineEnablePicklineEvent:         /* 34 */
     
      printf("Enable Pickline %2.2s %c\n", buf + 8, buf[10]);
      break;
       
    case EngineDisablePicklineEvent:        /* 35 */
     
      printf("Disable Pickline %2.2 %c\n", buf + 8, buf[10]);
      break;
       
    case EngineLockPicklineEvent:           /* 36 */

      printf("Lock Pickline %2.2s %c\n", buf + 8, buf[10]);
      break;

    case EngineUnlockPicklineEvent:         /* 37 */

      printf("Unlock Pickline %2.2s %c\n", buf + 8, buf[10]);
      break;

    case EngineStopZoneFeedingEvent:        /* 38 */
    
      printf("Stop Zone Feeding %2.2s %c\n", buf + 8, buf[10]); 
      break;
     
    case EngineOrderEvent:
    
      printf("Order %2.2s %6.6s %c\n", buf+8, buf+10, buf+16);
      break;

    case EngineCancelOrderEvent:            /* 40 */

      printf("Cancel Order %2.2s %6.6s %c\n", buf + 8, buf + 10, buf[16]);
      break;
       
    case EngineHoldOrderEvent:               /* 41 */

      printf("Hold Order %2.2s %6.6s %c\n", buf + 8, buf + 10, buf[16]);
      break;
       
    case EngineActivateOrderEvent:           /* 42 */

      printf("Act Order %2.2s %6.6s %c\n", buf + 8, buf + 10, buf[16]);
      break;
       
    case EnginePickEvent:                    /* 43 */

      printf("Pick ");
      if (len > 0) printf("%*.*s", len, len, buf + 8);
      printf("\n");
      break;

    case EngineZoneStatusEvent:              /* 44 */

      printf("Zone Status %4.4s %c %6.6s\n", buf + 9, buf[8], buf + 13);   
      break;
    
    case EngineLoginEvent:                   /* 45 */
    
      printf("Zone Login %4.4s %c\n", buf + 8, buf[12]);
      break;

    case EngineLogoutEvent:                  /* 46 */
    
      printf("Zone Logout %4.4s %c\n", buf + 8, buf[12]);
      break;

    case EngineOrderReleaseEvent:            /* 47 */
    
      printf("Order Release %2.2s %6.6s %4.4s %c\n", 
        buf + 8, buf + 10, buf + 16, buf[20]);
      break;
      
    case EngineChangeToteEvent:              /* 48 */

      printf("EngineChangeToteEvent %21.21s\n", buf+8);
      break;

    default:
    
      printf("Unknown Code=%d Len=%d [%*.*s]\n",
        code, len, len, len, buf + 8);
      break;
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Open All Files
 *-------------------------------------------------------------------------*/
open_all()
{
  message_open();
  message_select(list, sizeof(list));
  message_signal(SIGUSR1, catcher);
  
  return 0;
}

/* end of engine_test.c */
