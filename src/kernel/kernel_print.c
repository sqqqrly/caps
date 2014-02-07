/*  #define MICROCLOCK  */
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Print kernel log file.
 *
 *                   kernel_print -stqv [file_name]
 *
 *                   s = selection table
 *                   t = tasks
 *                   q = message queue contents
 *                   v = message traffic
 *                   p = peer to peer messages
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  7/5/93     |  tjt  Original Implementation
 *-------------------------------------------------------------------------*/
static char kernel_print_c[] = "%Z% %M% %I% (%G% - %U%)";

#include <stdio.h>
#include "message.h"
#include "./kernel.h"
#include "./kernel_data.h"
#include "./kernel_types.h"
#include "message_types.h"

FILE *fd;
char fd_name[64];									/* name of log file					*/

long sflag = 0;									/* selection table					*/
long tflag = 0;									/* logged tasks						*/
long qflag = 0;									/* message queue						*/
long vflag = 0;									/* message traffic					*/

long ssize;											/* selection table size				*/
long tsize;											/* task table size					*/
long qsize;											/* queue table size					*/
long qend;											/* end of queue						*/

main(argc, argv)
long argc;
char **argv;
{
	register long k;
	register char *p;

	strcpy(fd_name, KernelLogFile);			/* default log file name			*/

	for (k = 1; k < argc; k++)					/* over all arguments				*/
	{
		p = argv[k];		
		if (*p != '-') strcpy(fd_name, argv[k]);
		else
		{
			p++;
			while (*p)
			{
				if (*p == 's')			sflag = 1;
				else if (*p == 't')	tflag = 1;
				else if (*p == 'q')	qflag = 1;
				else if (*p == 'v')	vflag = 1;
            else if (*p == 'p')  vflag = 2;
            p++;
			}
		}
	}
	fd = fopen(fd_name, "r");					/* kernel log file					*/
	if (fd == 0)
	{
		fprintf(stderr, "*** Invalid File Name\n\n");
		exit(1);
	}
	ssize = sizeof(TMessageSelectorItem) * MessageTypes;
	tsize = sizeof(TTaskItem) * MessageTasks;
	
   fread(ms, ssize, 1, fd);               /* selector tables               */
   fread(t_task, tsize, 1, fd);           /* task table                    */
	fseek(fd, ssize + tsize, 0);				/* past tables							*/
	fread(&qsize, 4, 1, fd);					/* queue size							*/
	fread(&qend, 4, 1, fd);						/* queue end							*/
   tq = (unsigned char *)malloc(qsize);   /* message queue                 */
   fread(tq, qsize, 1, fd);               /* message queue                 */
	
   if (tflag) print_tasks();
   if (sflag) print_selection();
	if (vflag) print_log();
}
/*------------------------------------------------------------------------*
 *  Print Tasks
 *-------------------------------------------------------------------------*/
print_tasks()
{
  register long k, totin, totout;
  register TTaskItem *t;
  
  totin = totout = 0;

  for (k = 0, t = t_task; k < MessageTasks; k++, t++)
  {
    if (!t->t_pid) continue; 

    fprintf(stderr, "%2d: pid=%5d %-20s signal=%2d queued=%d in=%d put=%d\n",
      k + 1, t->t_pid, t->t_name, t->t_signal, t->t_count, 
      t->t_snd_count, t->t_rcv_count);

    totout += t->t_snd_count;
    totin  += t->t_rcv_count;
  }
  fprintf(stderr, "\nTotal Messages To   Kernel: %7d\n", totin);
  fprintf(stderr, "Total Messages From Kernel: %7d\n\n", totout);
}
/*-------------------------------------------------------------------------*
 *  Print Selection
 *-------------------------------------------------------------------------*/
print_selection()
{
  register long j, k;
  register unsigned long mask;
  register TTaskItem *t;
  register TMessageSelectorItem *m;
  register long bit, byte;
  
  for (k = 0, t = t_task; k < MessageTasks; k++, t++)
  {
    if (!t->t_pid) continue;   

    fprintf(stderr, "%2d: pid=%5d %-20s signal=%2d queued=%d in=%d put=%d\n",
     k + 1, t->t_pid, t->t_name, t->t_signal, t->t_count, 
     t->t_snd_count, t->t_rcv_count);
  
    fprintf(stderr, "    Messages=");

    bit  = Bit[k];
    byte = Byte[k];

    for (j = 0, m = ms; j < MessageTypes; j++, m++)
    {
      if (ms[j][byte] & bit) fprintf(stderr, "%d,", j);
    }
    fprintf(stderr, "\n\n");
  }
  return 0;
}
/*-------------------------------------------------------------------------*
 *  Print Message Queue
 *-------------------------------------------------------------------------*/
print_log()
{
#ifdef MICROCLOCK
   float x;
#endif

	register long count;
	long when;
	TMessageItem buf;
	
	count = 0;
	
	while(1)
	{
		if (fread(&when, 4, 1, fd) != 1) break;	/* read message time			*/
		if (fread(&buf, 4, 1, fd) != 1) break;		/* fixed part					*/
		if (buf.m_length > 0)
		{
			if (fread(&buf.m_message, buf.m_length, 1, fd) != 1) break;
		}
		count++;
      
      if (vflag == 2)
      {
        if (buf.m_sender == KernelDestination) continue;
        if (buf.m_destination == KernelDestination) continue;

        if (buf.m_type >= TTYServerOpen && buf.m_type <= InputField) continue;
        if (buf.m_type >= TTYServerOpenEvent) continue;
      }
		if (when <= 1) fprintf(stderr, "Sending ..\n");
#ifdef MICROCLOCK
      else 
      {
		  x = when / 1048.51;
        fprintf(stderr, "\n%5d:  %10.3f ms", count, x);
      }
#else
		else fprintf(stderr, "\n%5d: %s", count, ctime(&when));
#endif
		Bdump(&buf, buf.m_length + 4);
	}
}


/* end of kernel_print.c */
