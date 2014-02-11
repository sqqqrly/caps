#define TCBL
#define MIOM
/*-------------------------------------------------------------------------
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  CAPS Message Structures.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/01/93   |  tjt  Original implementation.
 *  05/20/94   |  tjt  Added worst message.
 *-------------------------------------------------------------------------*/
#ifndef CAPS_MESSAGES_H
#define CAPS_MESSAGES_H

static char caps_messages_h[] = "%Z% %M% %I% (%G% - %U%)";

#include "global_types.h"
#include "message.h"

/*-------------------------------------------------------------------------*
 *  Error Messages
 *-------------------------------------------------------------------------*/

typedef  struct
{
  unsigned char  m_error;
  unsigned char  m_text[63];

}  TErrorMessage;

/*-------------------------------------------------------------------------*
 *  Keyboard and Screen Messages
 *-------------------------------------------------------------------------*/
 
typedef  struct
{
  unsigned char m_row;
  unsigned char m_col;
  unsigned char m_text[80];

}  TDisplayTextMessage;

typedef  struct
{
  unsigned char m_row;
  unsigned char m_col;
  unsigned char m_type;
  unsigned char m_echo;
  unsigned char m_text[80];

}  TInputFieldMessage;

typedef  struct
{
  unsigned char m_terminator;
  unsigned char m_text[80];

}  TFieldMessage;

/*-------------------------------------------------------------------------*
 *  Port Number Message
 *-------------------------------------------------------------------------*/
 typedef struct
 {
   TPort   m_port;
 
 } TPortMessage;
/*-------------------------------------------------------------------------*
 *  Pickline Only Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline      m_pickline;

}  TPicklineMessage;

/*-------------------------------------------------------------------------*
 *  Zone Only Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TZone          m_zone;

}  TZoneMessage;

/*-------------------------------------------------------------------------*
 *  Zone Display Message
 *-------------------------------------------------------------------------*/

typedef struct
{
  TZone         m_zone;
  char          m_text[16];

}  TZoneDisplayMessage;

/*-------------------------------------------------------------------------*
 *  Zone Status Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline      m_pickline;
  TZone          m_zone;
  TOrder         m_order;
  unsigned char  m_status;

}  TZoneStatusMessage;

/*-------------------------------------------------------------------------*
 *  Zone Test Message
 *-------------------------------------------------------------------------*/
 
typedef struct
{
  TZone         m_zone;
  short         m_test;

}  TZoneTestMessage;

/*-------------------------------------------------------------------------*
 *  Module Only Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TModule        m_module;
   
}  TModuleMessage;

/*-------------------------------------------------------------------------*
 *  Pickline and Order Number Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;

}  TOrderMessage;

/*-------------------------------------------------------------------------*
 * Order Split Message 
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  TOrder      m_neworder;
  TCon        m_con;
  
}  TOrderSplitMessage;

/*-------------------------------------------------------------------------*
 *  Pickline, Order and Zone Message 
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  TZone       m_zone;
   
}  TZoneOrderMessage;

/*-------------------------------------------------------------------------*
 *  Order Event Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  TZone       m_zone;
  TGroup      m_grp;
  TCon        m_con;
  
}  TOrderEventMessage;

/*-------------------------------------------------------------------------*
 *  Pickline, Order and Box Message 
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  TBoxNumber  m_box;
  TBoxNumber  m_last;


   
}  TBoxOrderMessage;

/*-------------------------------------------------------------------------*
 *  Pickline, Order and Controller Message 
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  TController m_tc;
   
}  TControllerOrderMessage;

/*-------------------------------------------------------------------------*
 * Print  Message 
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  short       m_copies;
  TBoxNumber  m_box;
   
}  TPrintMessage;

/*-------------------------------------------------------------------------*
 *  Module Pick Request Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  TModule     m_module;
  TQuantity   m_ordered;
  long        m_reference;
  short       m_case_pack;
  char        m_um;
  char        m_count;
  
}  TPickRequestMessage;

/*-------------------------------------------------------------------------*
 *  Module Pick Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  TModule     m_module;
  TQuantity   m_picked;
  long        m_reference;
  
}  TPickMessage;

/*-------------------------------------------------------------------------*
 *  Module Pick And Box Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TOrder      m_order;
  TModule     m_module;
  TQuantity   m_picked;
  TBoxNumber  m_box;
  long        m_reference;
  long        m_picker_id;

}  TPickBoxMessage;

/*-------------------------------------------------------------------------*
 *  Pickline and Group Message
 *-------------------------------------------------------------------------*/

typedef  struct
{
  TPickline   m_pickline;
  TGroup      m_grp;

}  TGroupMessage;

/*-------------------------------------------------------------------------*
 *  Badge Scanner Message
 *-------------------------------------------------------------------------*/

typedef struct
{
  TPickline  m_pickline;
  long       m_badge;

}  TBadgeMessage;

/*-------------------------------------------------------------------------*
 *  Terminal Controller Message
 *-------------------------------------------------------------------------*/

typedef struct
{
  unsigned char m_tc[4];
  unsigned char m_pi[2];
  unsigned char m_cmd[2];
  unsigned char m_order[6];
  unsigned char m_quan[4];
  unsigned char m_port;
  
}  TPacketMessage;

/*-------------------------------------------------------------------------*
 *  Total Function Switch Message
 *-------------------------------------------------------------------------*/
 
typedef struct
{
  unsigned char m_ac[4];
  unsigned char m_cmd[2];
  unsigned char m_mod[3];
  unsigned char m_switch;
  unsigned char m_port;
  
}  TSwitchMessage;
/*-------------------------------------------------------------------------*
 *  Total Function IO Scan Message
 *-------------------------------------------------------------------------*/
 
typedef struct
{
  unsigned char m_ac[4];
  unsigned char m_cmd[2];
  unsigned char m_mod[3];
  unsigned char m_text[24];
  unsigned char m_port;
  
}  TIOMessage;

#ifdef MIOM
/*-------------------------------------------------------------------------*
 *  Total Function Multiple IO Output Message
 *-------------------------------------------------------------------------*/
 
typedef struct
{
   TModule m_hwix;
  
}  TMIOOutputMessage;

#endif
/*-------------------------------------------------------------------------*
 *  Lot Change and Split Request
 *-------------------------------------------------------------------------*/
 
typedef struct
{
  TPickline     m_pickline;                 /* pickline                      */
  unsigned char m_key[15];                  /* SKU, stkloc, or module        */
  unsigned char m_keytype;                  /* 1=SKU, 2=stkloc, 3=module     */
  unsigned char m_lot[15];                  /* new lot number                */
  TQuantity     m_quantity;                 /* quantity split into old lot   */

} TLotMessage;

/*-------------------------------------------------------------------------*
 *  Worst Case Message
 *-------------------------------------------------------------------------*/
 
typedef struct
{
  unsigned char m_text[MessageText];
  
} TWorstMessage;

typedef struct
{
  TPort   		m_port;
  unsigned char 	m_text[11];
  long			m_length;
} TTcblMessage;

/*-------------------------------------------------------------------------*
 *  Union Of All CAPS Messages
 *-------------------------------------------------------------------------*/

typedef union
{
  TErrorMessage           ErrorMessage;
  TDisplayTextMessage     DisplayTextMessage;
  TInputFieldMessage      InputFieldMessage;
  TFieldMessage           FieldMessage;
  TPortMessage            PortMessage;
  TPicklineMessage        PicklineMessage;
  TZoneMessage            ZoneMessage;
  TZoneDisplayMessage     ZoneDisplayMessage;
  TZoneStatusMessage      ZoneStatusMessage;
  TZoneTestMessage        ZoneTestMessage;
  TModuleMessage          ModuleMessage;
  TZoneOrderMessage       ZoneOrderMessage;
  TBoxOrderMessage        BoxOrderMessage;
  TControllerOrderMessage TCOrderMessage;
  TPrintMessage           PrintMessage;
  TOrderMessage           OrderMessage;
  TOrderSplitMessage      OrderSplitMessage;
  TOrderEventMessage      OrderEventMessage;
  TPickRequestMessage     PickRequestMessage;
  TPickMessage            PickMessage;
  TPickBoxMessage         PickBoxMessage;
  TGroupMessage           GroupMessage;
  TBadgeMessage           BadgeMessage;
  TPacketMessage          PacketMessage;
  TSwitchMessage          SwitchMessage;
  TLotMessage             LotMessage;
  TWorstMessage		  WorstMessage;
#ifdef TCBL
  TTcblMessage            TcblMessage;
#endif
#ifdef MIOM
  TMIOOutputMessage	  MIOOutputMessage;
#endif
}  TCapsMessageItem;

#endif

/*  end of caps_messages.h  */

