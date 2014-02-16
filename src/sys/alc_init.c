#define DEBUG
/* #define TIMER  */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Initialize a total function port.
 *                  Appends hw_item for port and counts for port.
 *  
 *  Execution:      alc_init [device]  [-v] [-s] [-m] [-p]
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/30/93   |  tjt  Add to mfc.
 *  07/06/94   |  tjt  Add ac_soft_reset.
 *  07/07/94   |  tjt  Add firmware date of XX/XX/XX is no check.
 *  07/07/94   |  tjt  Add error messages (-m option).
 *  03/16/95   |  tjt  Add PM4 as an option.
 *  03/17/95   |  tjt  Add ZC2 and PM2.
 *  05/31/95   |  tjt  Add print errors (-p option).
 *  06/03/95   |  tjt  Revised.
 *  01/07/96   |  sg   Add full function type == 'F' as a ZC module.
 *  05/20/96   |  tjt  Add box full module.
 *  06/07/97   |  tjt  Add PM6.
 *  05/20/98   |  tjt  Add IO module for type == 'S'.
 *  05/26/98   |  tjt  Add lookup of device name.
 *  05/26/98   |  tjt  Add prom date 01/15/97 is Full Function.
 *-------------------------------------------------------------------------*/
static char alc_init_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "message_types.h"
#include "file_names.h"
#include "global_types.h"
#include "alc_packet.h"
#include "ss.h"
#include "co.h"

void leave(int notUsed);
int errno;

#define NoError        0                  /* successful action               */
#define AddressError   1                  /* port has an addressign error    */
#define DateError      2                  /* wrong firmware date             */
#define ParityError    3                  /* parity on writeable firmware    */
#define OtherError     4                  /* unexpected stuff on the line    */
#define NoResponse     5                  /* no repsonse at all on the line  */

long TIMEOUT = 0;                         /* response timeout                */

FILE *fd;
char firmdate[8];                         /* firmware date                   */

long pd;                                  /* port descriptor                 */
char pd_name[40];                         /* port device name                */
long pd_no = -1;									/* F052698 - table position		  */

FILE *md, *sd;                            /* hardware map descriptor         */
char md_name[40];                         /* hardware map name               */

FILE *ed;
char ed_name[16];                         /* error message file              */

long          errors = 0;                 /* messages in file                */
long          verbose = 0;                /* to screen                       */
long          suitcase = 0;               /* suitcase addressing             */
long          send_errors = 0;          	/* send error messages             */
long          print_errors = 0;        	/* print error report              */

#ifdef TIMER
  long          start;
#endif

  main(argc, argv)
  long argc;
  char **argv;
  {
    long k, ret = 0;
    char text[80];
    char c;
    
    setpgrp();
    putenv("_=alc_init");                 /* name to environment             */
    chdir(getenv("HOME"));                /* to home directory               */

    signal(SIGINT, leave);
    signal(SIGTERM, leave);
    signal(SIGHUP, leave);
  
#ifdef TIMER
    start  = time(0);
#endif

    ss_open();
    co_open();										/* F052698								  */
    
    if (sp->sp_total_function == 's') leave(0);	/* simulated - return OK	  */
    if (sp->sp_full_function == 's') leave(0);	/* F052098						  */
    
    TIMEOUT = sp->sp_init_timeout;

    tmp_name(ed_name);

    ed = fopen(ed_name, "w");
    if (ed == 0)
    {
      krash("alc_init", "tmp file", 0);
      exit(1);
    }
    if (argc < 2)
    {
      message("Missing Port Argument");
      leave(1);
    }
    fd = fopen(alc_firmdate_name, "r");
    if (!fd)
    {
      message("Cannot Open Firmware Date File");
      leave(1);
    }
    fread(firmdate, 1, 8, fd);
    fclose(fd);
    fd = 0;

    strcpy(pd_name, argv[1]);

    for (k = 2; k < argc; k++)
    {
      if (strcmp(argv[k], "-v") == 0) verbose      = 1;
      if (strcmp(argv[k], "-s") == 0) suitcase     = 1;
      if (strcmp(argv[k], "-m") == 0) send_errors  = 1;
      if (strcmp(argv[k], "-p") == 0) print_errors = 1;
    }
    if (send_errors) message_open();

    for (k = 0; k < sp->sp_ports; k++)		/* F052698 - lookup port name		  */
    {
      if (strcmp(pd_name, po[k].po_name) == 0)
      {
        pd_no = k;
        break;
      }
    }
    if (pd_no < 0)								/* F052698 - error message			  */
    {
      message("Bad Port Name");
      leave(1);
    }
#ifdef DEBUG
    fprintf(stderr, "alc_init %s %8.8s\n\n", pd_name, firmdate);
#endif

    pd = ac_open(pd_name);                /* open total function port        */

    if (pd <= 0)                          /* can't open port                 */
    {
      message("Cannot Open Port");        /* fails immediately               */
      leave(1);
    }
    sprintf(md_name, "%s.%s", hw_name, basename(pd_name));

#ifdef DEBUG
    fprintf(stderr, "MAIN ret = %d\n", ret);
#endif

    ret = init_port();                    /* open and initialize port        */
  
#ifdef DEBUG
    fprintf(stderr, "main after init_port ret = %d\n", ret);
#endif
    ac_close(pd);                         /* close total function port       */

    if (ret)                              /* failed to inintialize           */
    {
      message("Initialization---- Failed");
      leave(1);
    }
    if (errors) message("Initialization Successful After Errors");
  
    leave(0);
  }
/*-------------------------------------------------------------------------*
 *  Error Messages
 *-------------------------------------------------------------------------*/
  message(p)
  register char *p;
  {
    char text[80];

    sprintf(text, "Port: %s - %s\n", basename(pd_name), p);
    fprintf(ed, "%s", text);
    errors += 1;
  
    if (verbose)
    {
      printf("\r%s", text);
      fflush(stdout);
    }
    if (send_errors)
    {
      message_put(0, InitErrorMessageEvent, text, strlen(text));
    }
    return 0;
  }
/*-------------------------------------------------------------------------*
 *  Initialize Port
 *
 *  Try  NoError  AddressError  DateError  ParityError OtherError NoResponse
 *  ---  -------  ------------  ---------  ----------- ---------- ----------
 *   1     OK     readdress      download    again       again      again
 *   2     OK     readdress      again     download      again      again
 *   3     OK     readdress      again     download      again      again
 *   4     OK       FAILS        FAILS       FAILS       FAILS      FAILS
 *--------------------------------------------------------------------------*/
int  init_port()
  {
    int count, ret = 0, trys, loaded;

#ifdef DEBUG
    fprintf(stderr, "init_port()\n");
#endif

    trys   = 0;                           /* clear attempts                  */
    loaded = 0;                           /* no download attempt yet         */
  
    while (1)                             /* try several times               */
    {
      trys++;                             /* count attempts to fix           */

      ret = get_status();
      
      if (md) fclose(md);
#ifdef DEBUG
      fprintf(stderr, "init_port close %x - ret=%d\n", md, ret);
#endif
      md = 0;
      
      if (!ret) break;   	/* get status OK                   */

      if (trys > 3)
      {
#ifdef DEBUG
      fprintf(stderr, "before failed 3 trys\n");
#endif
        message("Failed After 3 Trys");
        return 1;                         /* failed to get status            */
      }
#ifdef DEBUG
      fprintf(stderr, "before switch ret\n");
#endif
      switch (ret)
      {
        case AddressError:  readdress_port();
        break;
                          
        case ParityError:   if (trys == 1) break;/* try again once 			  */
         
        case DateError:     if (!loaded) download_port();
                            loaded = 1;
                            trys = 0;
                            break;
                          
        default:            break;        /* just try again                  */
      }
    }
    return 0;
  }
/*-------------------------------------------------------------------------*
 *  Get status and length of all controllers on port
 *-------------------------------------------------------------------------*/
  get_status()
  {
    long k, ret = 0, count, max, mods;
    unsigned char buf[32], text[80];
    TStatusPacketItem x;
    TLengthPacketItem y;
    unsigned char map[256];
  
    struct
    {
      unsigned char ac;
      unsigned char mod;
      unsigned char type;
    } ac_rec;

#ifdef DEBUG
    fprintf(stderr, "get_status()\n");
#endif

    max = count = 0;

    ac_reset(pd);                         /* hard reset port                 */
    ac_soft_reset(pd, 9999);              /* software reset                  */
  
    if (!suitcase)                        /* first count controllers         */
    {
      ac_write(pd, "999903", 6);
      memset(map, 0, sizeof(map));
  
      while (1)                           /* wait for all responses          */
      {
        ret = ac_readout(pd, &x, TIMEOUT);/* get a message                   */

#ifdef DEBUG
        fprintf(stderr, "get_status() 1  ret=%d\n", ret);
        if (ret > 0) Bdump(&x, ret);
#endif

        if (ret < 0) break;               /* was a timeout                   */

        if (!ret)                         /* got a bad packet                */
        {
          message("Get Status: Noisy Line");
          message(&x);
          return OtherError;              /* ignore rest if one noisy        */
        }
        k = cvrt(x.packet_ac_addr, 4);    /* get responding address          */

        if (k >= 256 || map[k])
        {
          message("Get Status:    Address Error");
          message(&x);
          return AddressError;
        }
#ifdef DEBUG
   fprintf(stderr, "Found Command Module # %d\n", k);
#endif
        map[k] = 1;
        max++;
      }
    }
    errno = 0;
    md = fopen(md_name, "w");
#ifdef DEBUG
    fprintf(stderr, "md_name=%s md=%x errno=%d\n", md_name, md, errno);
#endif
    
    if (md == 0)
    {
      sprintf(text, "Invalid Hardware Map File Name %s", md_name);
      message(text);
      leave(1);
    }
    ac_write(pd, "999904", 6);            /* block all communications        */
    sleep(6);
    ac_write(pd, "999903", 6);            /* request status                  */
  
    while (1)                             /* wait for all responses          */
    {
	ret = 0;
      ret = ac_readout(pd, &x, TIMEOUT);  /* get a message                   */

#ifdef DEBUG
      fprintf(stderr, "get_status() 2  ret=%d\n", ret);
      if (ret > 0) Bdump(&x, ret);
#endif

      if (ret < 0) break;                 /* was a timeout                   */

      if (!ret)                           /* got a bad packet                */
      {
        message("Get Status: Noisy Line");
        message(&x);
        return OtherError;                /* ignore rest if one noisy        */
      }
      ac_rec.ac = cvrt(x.packet_ac_addr, 4);/* get responding address        */

      if (suitcase) count = ac_rec.ac;    /* suitcase has any address        */
      else count++;

#ifdef DEBUG
      fprintf(stderr, "get_status()  max=%d count=%d  ac_addr=%d\n",
      max, count, ac_rec.ac);
#endif
    
      if (ac_rec.ac != count)             /* check address                   */
      {
        message("Get Status:      Address Error");
        message(&x);
        return AddressError;
      }
      if (memcmp(x.packet_command, "03", 2) != 0)
      {
        message("Get Status: Unexpected Packet");
        message(&x);
        return OtherError;                /* unexpected response             */
      }
      if (memcmp(x.packet_prom_date, "01/15/97", 8) == 0)	/* F052698		  */
      {
        po[pd_no].po_flags |= IsFullFunction;	/* flag based on prom date	  */
      }
      if (firmdate[0] != 'X')
      {
        if (memcmp(x.packet_eeprom_date, firmdate, 8) != 0)
        {
          message("Get Status: Bad Firmware Date");
          message(&x);
          return DateError;               /* firmware out of date            */
        }
      }
      if (memcmp(x.packet_prom_check, "00", 2) != 0)
      {
        message("Get Status: Readonly Firmware Error");
        message(&x);
        return OtherError;                /* readonly firmware error         */
      }
      if (memcmp(x.packet_eeprom_check, "00", 2) != 0)
      {
        message("Get Status: Writeable Firmware Error");
        message(&x);
        return ParityError;               /* writeable firmware error        */
      }
      sprintf(buf, "%04d02", count);      /* get length                      */
      ac_write(pd, buf, 6);
    
      ret = ac_readout(pd, &y, TIMEOUT);  /* get length response             */
      mods = ret - 6;                     /* number of modules on controller */

#ifdef DEBUG
      fprintf(stderr, "get_length()  cm=%d  ret=%d  mods=%d\n", 
                       count, ret, mods);
      if (ret > 0) Bdump(&y, ret);
#endif
    
      if (ret < 0)                        /* was a timeout                   */
      {
        message("Get Status: Length Not Returned");
        return OtherError;
      }
      if (!ret)                           /* got a bad packet                */
      {
        message("Get Status: Noisy Line");
        message(&y);
        return OtherError;                /* ignore rest if one noisy        */
      }
      ac_rec.ac = cvrt(y.packet_ac_addr, 4);/* get responding address        */

      if (ac_rec.ac != count)             /* check address                   */
      {
        message("Get Status: Address Error");
        return AddressError;
      }
      if (memcmp(y.packet_command, "02", 2) != 0)
      {
        message("Get Status: Unexpected Packet");
        return OtherError;                /* unexpected response             */
      }
      for (ac_rec.mod = 0; ac_rec.mod < mods; ac_rec.mod++)
      {
        switch(y.packet_map[ac_rec.mod])
        {
          case 'B': ac_rec.type = BL;  break;	/* F052698 - total or full	  */
          case 'F': ac_rec.type = ZC;  break;	/* F010796 - ff zone control */
          case 'Z': ac_rec.type = ZC2; break;
          case 'P': ac_rec.type = PM2; break;	/* F052698 - total or full	  */
          case 'Q': ac_rec.type = PM4; break;
          case 'U': ac_rec.type = BF; break;
          case 'H': ac_rec.type = PM6; break;
          case 'S': ac_rec.type = IO;	break;	/* F052098 - scanner			  */
          default:  ac_rec.type = 0;   break;
        }
#ifdef DEBUG
        Bdump(&ac_rec, 3);
#endif        
        fwrite(&ac_rec, 3, 1, md);
        
        if (ac_rec.type == ZC2)
        {
          sprintf(buf, "%04d09%03dINITIALIZE      ", ac_rec.ac, ac_rec.mod);
          ac_write(pd, buf, 25);
        }
        else if (ac_rec.type == ZC)			/* F010796 */
        {
          sprintf(buf, "%04d09%03d__L__", ac_rec.ac, ac_rec.mod);
          ac_write(pd, buf, 14);
        }
      }
      sprintf(buf, "%04d05", count);
      ac_write(pd, buf, 6);               /* unblock to next ac              */
      sleep(6);
      sprintf(buf, "%04d03", count + 1);
      ac_write(pd, buf, 6);               /* request status                  */
    }
    if (md) fclose(md);
#ifdef DEBUG
   fprintf(stderr, "get_status() close %x\n", md);
#endif
    md = 0;
    if (suitcase) return NoError;
  
    if (count != max)
    {
      message("Get Status:        Address Error");
      message(&x);
      return AddressError;
    }
#ifdef DEBUG
   fprintf(stderr, "return NoError\n");
#endif
    return NoError;
  }
/*-------------------------------------------------------------------------*
 *  Readdress The Entire Port
 *-------------------------------------------------------------------------*/
  readdress_port()
  {
    ac_reset(pd);                         /* hard reset port                 */
    ac_soft_reset(pd, 9999);              /* software reset                  */
  
    message("Readdressing");

    ac_readdress(pd, 1);                  /* readdress entire port           */
    return 0;
  }
/*-------------------------------------------------------------------------*
 *  Download Entire Port
 *-------------------------------------------------------------------------*/
  download_port()
  {
    ac_reset(pd);                         /* hard reset port                 */
    ac_soft_reset(pd, 9999);              /* software reset                  */
  
    message("Downloading New Firmware");

    ac_download(pd, 9999, alc_firmware_name);
    return 0;
  }
/*-------------------------------------------------------------------------*
 *  Convert to Binary
 *-------------------------------------------------------------------------*/
  cvrt(p, n)
  register unsigned char *p;
  register long n;
  {
    register long x;
   
    x = 0;
   
    while (n > 0)
    {
      if (*p < '0' || *p > '9') return -1;
      x = 10 * x + (*p++ - '0');
      n--;
    }
    return x;
  }
/*-------------------------------------------------------------------------*
 *  Graceful Exit
 *-------------------------------------------------------------------------*/
  void leave(int x)
  {
    long pid =0 , stat =0 ;
    char text[80], prt_name[16];

    ss_close();
    co_close();									/* F052698								  */
    if (send_errors) message_close();
  
    if (ed) fclose(ed);
    if (fd) fclose(fd);
    if (md) fclose(md);
  
    if (errors && print_errors)
    {
      if ((pid = fork()) == 0)
      {
        execlp("prft", "prft", ed_name, tmp_name(prt_name),
        "sys/report/init_report.h", 0);
        krash("leave", "load prft", 0);
        exit(1);
      }
      pid = wait(&stat);
    }
    else unlink(ed_name);
  
    exit(x);
  }

/* end of alc_init.c */

   
