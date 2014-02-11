/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Internal Kernel Structures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/02/93    |  tjt  Orignal implementation.
 *-------------------------------------------------------------------------*/
#ifndef KERNEL_H
#define KERNEL_H
#include <stdio.h>

static char kernel_h[] = "%Z% %M% %I% (%G% - %U%)";

#define  KernelSelectorBytes  8           /* related to MessageTasks         */

#define  KPage                8192        /* queue allocation increments     */

#define  KernelQueueMin       KernelSelectorBytes + 3 /* least queue item    */
#define  KernelQueueMax       KernelQueueMin + 255    /* worst queue item    */

#define  ShutdownTimeout      4           /* watchdog timeout during shutdown*/
#define  ShutdownCount       15           /* number of timeouts 4*15 = 60 sec*/

#define  KernelLogFile        "kernel.log"

extern   long KernelTimeout;              /* watchdog timeout = 60 seconds   */
/*-------------------------------------------------------------------------*
 *  Message Selector Bit Matrix
 *-------------------------------------------------------------------------*/

typedef unsigned char TMessageSelectorItem[KernelSelectorBytes];

/*-------------------------------------------------------------------------*
 *  Message Queue Item
 *-------------------------------------------------------------------------*/
 
typedef  struct
{
  unsigned char  tq_tasks[KernelSelectorBytes];
  unsigned char  tq_sender;
  unsigned char  tq_type;
  unsigned char  tq_length;
  unsigned char  tq_text[MessageText];

}  TMessageQueueItem;

/*-------------------------------------------------------------------------*
 *  Kernel Task Control Table
 *-------------------------------------------------------------------------*/
 
typedef  struct
{
  unsigned short t_pid;                   /* task pid                        */
  unsigned char  t_name[32];              /* task name                       */
  unsigned short t_count;                 /* messages in queue               */
  long           t_signal;                /* signal to task                  */
  long           t_offset;                /* offset to message queue         */
  long           t_snd_time;              /* time last message to task       */
  long           t_rcv_time;              /* time last message from task     */
  long           t_snd_count;             /* messages sent to task           */
  long           t_rcv_count;             /* messages received from task     */
   
} TTaskItem;

/*-------------------------------------------------------------------------*
 *  Global Variables As Externals
 *-------------------------------------------------------------------------*/

extern FILE                   *fd;        /* logging file                    */

extern TMessageSelectorItem   ms[];       /* message selector table          */
extern unsigned char          Byte[];     /* map task to selector byte       */
extern unsigned char          Bit[];      /* map taks to selector bit        */
extern unsigned char          *tq;        /* task/message queue table        */

extern long                   tq_size;    /* allocated items                 */
extern long                   tq_end;     /* current next item               */
extern long                   kerneltimer;/* watchdog timer interval         */
extern long                   kernelshutdown;/* 0=no, 1=request; 2=event     */

extern TTaskItem              t_task[];   /* user task table                 */

#endif

/* end of kernel.h */
