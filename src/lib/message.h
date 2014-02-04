/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Kernel Message Structures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 06/01/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
#ifndef MESSAGE_H
#define MESSAGE_H

static char message_h[] = "%Z% %M% %I% (%G% - %U%)";

extern long msgin;                        /* input queue to kernel           */
extern long msgout;                       /* output queue from kernel        */
extern long msgtask;                      /* id of this task 1 .. 64         */
extern long msgtype;                      /* message type for wait           */
extern long msgsig;                       /* message signal                  */
extern long (*msgfunc)();                 /* message processor               */
/*-------------------------------------------------------------------------*
 *  Message Defines And Structure
 *-------------------------------------------------------------------------*/
#define  MessageKeyOut  301               /* output queue number             */
#define  MessageKeyMin  302               /* first input queue               */
#define  MessageKeyMax  365               /* last  input queue               */
#define  MessageKeyZero 366               /* used for queue zero             */
#define  MessageTasks   64                /* number of tasks 1 .. 64         */
#define  MessageTypes   259               /* message types 1 .. 255          */
#define  MessageText    255               /* text bytes                      */
/*-------------------------------------------------------------------------*
 * Kernel Message Parameters
 *-------------------------------------------------------------------------*/
typedef union
{
  unsigned short    m_pid;
  long              m_signal;
  unsigned char     m_select[MessageText];
  unsigned char     m_text[MessageText];
   
}  TKernelMessageItem;
/*-------------------------------------------------------------------------*
 *  Kernel Message Structure
 *-------------------------------------------------------------------------*/
typedef struct                            /* message structure               */
{
  unsigned char  m_sender;                /* id of sender                    */
  unsigned char  m_destination;           /* id of destination or one        */
  unsigned char  m_type;                  /* message type code               */
  unsigned char  m_length;                /* length of message text          */
   
  TKernelMessageItem   m_message;         /* various messages                */

}  TMessageItem;
/*-------------------------------------------------------------------------*
 *  Kernel Message Log Structure
 *-------------------------------------------------------------------------*/
typedef  struct
{
  long           m_time;                  /* time of receipt                 */
  TMessageItem   m_log;                   /* copy of the message             */

}  TMessageLogItem;

#endif

/*  end of message.h  */
