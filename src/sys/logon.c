#define  DEBUG
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    Initial logon process to build environment.
 *                  Load tty_server as child and op_logon for actual
 *                  logon to caps.
 *
 *  Execution:      logon [user]
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  07/13/93   |  tjt  Rewrite for tty_server
 *  12/26/94   |  tjt  Add ANSI Terminal.
 *-------------------------------------------------------------------------*/
static char logon_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include <signal.h>
#include "file_names.h"

extern catcher();
extern leave();
extern char *getenv();

#define   MAX  32
#define   LINE 256

char home[32] = {0};
char language[32] = {0};
long have_language = 0;

FILE *fd;

char *list[MAX + 1] = {0};                /* environment pointers            */
char tab[MAX][LINE + 1] = {0};            /* environment parms               */
char tty_name[64];                        /* name of tty device              */
char server_name[64];                     /* name of server program          */
char text[128];

char *tty_server[] = {server_name, tty_name, 0};
char *op_logon[]   = {"bin/op_logon", 0};

long ready_to_go = 0;                      /* tty server has started         */

main(argc, argv)
long argc;
char **argv;
{
  register long j, k, len;
  register char *p, *q;
   
  putenv("_=logon");
  
  umask(0111);                             /* files created as rw.rw.rw      */
  
  signal(SIGHUP, leave);
  signal(SIGINT, leave);
  signal(SIGQUIT, leave);
  signal(SIGTERM, leave);
  signal(SIGTRAP, leave);

  p = getenv("LANGUAGE");                 /* usually LANGUAGE=               */
  if (p)
  {
    have_language = 1;                    /* flag was in environment         */
    if (*p) strcpy(language, p);          /* language in the environment     */
  }
/*--------------------------------------------------------------------------*
 *  All children of logon are in the same process group for krash message
 *--------------------------------------------------------------------------*/
  
  //setpgrp();                              /* process group for krash         */
                                          
  p = getenv("HOME");                     /* insure in HOME anyway           */
  if (p) chdir(p);

/*------------------------------------------------------------------------*/
/*  Build environment
/*------------------------------------------------------------------------*/

  fd = fopen(dotinit_name, "r");          /* open sys/dotinit file           */
  if (fd == 0)                            /* open failed                     */
  {
    krash("logon", "Open sys/dotinit", 0);
    leave();
  }
  k = 0;

  p = (char *)ttyname(fileno(stdout));    /* get name of tty device          */
  if (p)
  {
    strcpy(tty_name, p);                  /* save for tty_server call        */
    sprintf(tab[k], "TTY=%s", tty_name);  /* to environment                  */
    list[k] = tab[k];
    k++;
  }
  p = getenv("LOGNAME");
  if (p)
  {
    sprintf(tab[k], "LOGNAME=%s", p);     /* to environment                  */
    list[k] = tab[k];
    k++;
  }
  strcpy(server_name, "bin/tty_server");

  p = getenv("TERM");
  if (p)
  {
    if (strcmp(p, "ansi") == 0 || strcmp(p, "wyse370") == 0)
    {
      strcpy(server_name, "bin/ansi_server2");

      if (memcmp(tty_name, "/dev/ttyp", 9) == 0)
      {
        strcpy(server_name, "bin/ansi_server2");
      }  
    }
    sprintf(tab[k], "TERM=%s", p);
    list[k] = tab[k];
    k++;
  }
  while (k < MAX)
  {
    if (fgets(tab[k], LINE, fd) == 0) break;
    if (tab[k][0] == '#') continue;
    j = strlen(tab[k]) - 1;
    if (j < 1) continue;
    tab[k][j] = 0;
    
    if (memcmp(tab[k], "HOME=", 5) == 0)
    {
      strcpy(home, &tab[k][5]);           /* save home directory             */
    }
    else if (memcmp(tab[k], "LANGUAGE=", 9) == 0)
    {
      if (have_language)                  /* use environment parm            */
      {
        sprintf(tab[k], "LANGUAGE=%s", language);
      }
      else strcpy(language, &tab[k][9]);  /* save actual language            */
    }
    else if (memcmp(tab[k], "PATH=", 5) == 0)
    {
      if (*language)
      {
        sprintf(text, "PATH=:%s/language/%s/bin:%s",
          home, language, &tab[k][5]);
        strcpy(tab[k], text);
      }
    }
    list[k] = tab[k];
    k++;
  }
  list[k] = 0;
   
  fclose(fd);

/*-------------------------------------------------------------------------*
 *  tty_server started as child so will die if parent dies !!!
 *-------------------------------------------------------------------------*/
  ready_to_go = 0;                        /* tty server has NOT started      */
  signal(SIGUSR1, catcher);					/* catch when tty_server loaded    */
  
  if (fork() == 0)                        /* start tty_server as child       */
  {
    if (*language)
    {
      strcpy(text, server_name);
      sprintf(server_name, "language/%s/%s", language, text);
    }
#ifdef DEBUG
  fprintf(stderr, "server=%s  tty=%s\n", tty_server[0], tty_server[1]);
#endif
    
    execve(tty_server[0], tty_server, list);

    krash("logon", "tty_server not found", 0);
    leave();
  }
  
/*-------------------------------------------------------------------------*
 *  op_logon is beginning of caps 
 *-------------------------------------------------------------------------*/

  while (!ready_to_go) pause();          /* give tty_server time to start    */
                                         /* signal will break sleep          */
  execve(op_logon[0], op_logon, list);

  krash("logon", "op_logon not found", 0);
  leave();
}
/*-------------------------------------------------------------------------*
 *  catch ready signal from tty_server - only to assure loaded.
 *-------------------------------------------------------------------------*/
catcher()
{
   ready_to_go = 1;
	return 0;										/* do nothing - only catch	      */
}
/*-------------------------------------------------------------------------*
 *  Grateful Death
 *-------------------------------------------------------------------------*/
leave()
{
  printf("\r\n\nUnable To Logon To CAPS\n\n");
  printf("\r.. Goodbye\n\n\r");
  exit(1);
}


/* end of logon.c */
