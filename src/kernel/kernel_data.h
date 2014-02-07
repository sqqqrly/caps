/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Internal Kernel Data Structures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/02/93    |  tjt  orignal implementation.
 *             |
 *-------------------------------------------------------------------------*/
#ifndef KERNEL_DATA_H
#define KERNEL_DATA_H
#include <stdio.h>

static char kernel_data_h[] = "%Z% %M% %I% (%G% - %U%)";

long kerneltimer    = 60;                 /* watchdog & shutdown timer       */
long kernelshutdown = 0;                  /* shutdown in processs flag       */

/*-------------------------------------------------------------------------*
 *  Message Selector Bit Matrix
 *-------------------------------------------------------------------------*/

TMessageSelectorItem ms[MessageTypes];    /* task selector bits              */

unsigned char  Byte[MessageTasks] =

{0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,
 4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7};

unsigned char  Bit[MessageTasks] =

 {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

/*-------------------------------------------------------------------------*
 *  Kernel Message Queue
 *-------------------------------------------------------------------------*/

unsigned char        *tq;                 /* task queue                      */
long                 tq_size;             /* number of items                 */
long                 tq_end;              /* current last item               */
   
/*-------------------------------------------------------------------------*
 *  Kernel Task Control Table
 *-------------------------------------------------------------------------*/

TTaskItem      t_task[MessageTasks];      /* task table                      */

/*-------------------------------------------------------------------------*
 *  Kernel Logging File
 *-------------------------------------------------------------------------*/

FILE  *fd = 0;                            /* kernel logging file             */

#endif

/* end of kernel_data.h */
