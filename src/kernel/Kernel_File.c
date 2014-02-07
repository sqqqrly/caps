/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:     Process log file start and stop messages.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/21/93   | tjt - Original implementation.
 *-------------------------------------------------------------------------*/
static char Kernel_File_c[] = "%Z% %M% %I% (%G% - %U%)";

#include "message.h"
#include "./kernel.h"

extern FILE *fd;

/*-------------------------------------------------------------------------*
 *  Process Kernel Start Log Message
 *-------------------------------------------------------------------------*/

kernel_start_log(buf)
register TMessageItem *buf;
{
  char name[64], message[128];

  if (fd) return -1;                      /* already open                    */

  if (buf->m_length)
  {
    strncpy(name, buf->m_message.m_text, buf->m_length);
    name[buf->m_length] = 0;
  }
  else strcpy(name, KernelLogFile);

  fd = fopen(name, "w");                  /* open new log file               */
  if (fd == 0)
  {
    sprintf(message, "Open Log File %s Failed", name);
    krash("kernel_start_log", message, 0);
    return - 1;
  }
  fwrite(ms, sizeof(TMessageSelectorItem), MessageTypes, fd);
  fwrite(t_task, sizeof(TTaskItem), MessageTasks, fd);
  fwrite(&tq_size, 4, 1, fd);
  fwrite(&tq_end, 4, 1, fd);
  fwrite(tq, tq_size, 1, fd);

  return 0;
}

/*-------------------------------------------------------------------------*
 *  Process Kernel Stop Log Message
 *-------------------------------------------------------------------------*/

kernel_stop_log(buf)
register TMessageItem *buf;
{
  if (!fd) return -1;                     /* file is not open                */
  fclose(fd);                             /* close file                      */
  fd = 0;                                 /* mark as closed                  */
  return 0;
}

/* end of Kernel_File.c */
