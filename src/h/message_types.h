/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:  Definition of CAPS Message Codes.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  06/01/93   | tjt Original implementation.
 *  05/06/94   | tjt Add APURequest & APUResponse
 *  05/12/94   | tjt Add ZoneOffline & ZoneOnline + renumber.
 *  12/15/94   | tjt Add LotChange message.
 *  02/22/95   | tjt Add port disable/enable message.
 *  05/18/98   | tjt Add IO port input message.
 *-------------------------------------------------------------------------*/
#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

static char message_types_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Message Types
 *-------------------------------------------------------------------------*/

#define  Event                      128

#define  ClientMessage              1
#define  SystemMessage              2

#define  Initialize                 3
#define  Configure                  4
#define  Markplace                  5
#define  Restoreplace               6
#define  SystemRecovery             7
#define  Shutdown                   8

#define  ShortPrintDisable          9
#define  ShortPrintEnable           10
#define  RestockPrintDisable        11
#define  RestockPrintEnable         12

#define  PortInitialize             13
#define  PortMarkplace              14
#define  PortDisable                15
#define  PortEnable                 16
#define  PortRedisplay              17

#define  PicklineDisable            18
#define  PicklineEnable             19
#define  PicklineLock               20
#define  PicklineUnlock             21
#define  PicklineRedisplay          22
#define  PicklineStop               23

#define  ZoneStop                   24
#define  ZoneStart                  25
#define  ZoneComplete               26
#define  ZoneNext                   27
#define  ZoneRedisplay              28
#define  ZoneClear                  29
#define  ZoneDisplay                30
#define  ZoneTest                   31
#define  ZoneStatus                 32
#define  ZoneOffline                33
#define  ZoneOnline                 34

#define  ModuleInhibit              35
#define  ModuleEnable               36
#define  ModulePick                 37
#define  ModuleSplit                38
#define  ModulePickBox					39

#define  OrderInput                 39
#define  OrderManualEntry           40
#define  OrderPick                  41
#define  OrderRepick                42
#define  OrderHold                  43
#define  OrderActivate              44
#define  OrderPriority              45
#define  OrderToteAhead             46
#define  OrderRelease               47
#define  OrderUnderway              48
#define  OrderComplete              49
#define  OrderCancel                50
#define  OrderSplit                 51
#define  OrderPurge                 52

#define  GroupPick                  53
#define  GroupHold                  54
#define  GroupActivate              55
#define  GroupPriority              56
#define  GroupCancel                57

#define  BoxClose                   60
#define  BoxOpen                    61
#define  BoxNext                    62
#define  BoxSplit                   63
#define  BoxScan                    64

#define  LotChange                  68

#define  TCInputPacket              70
#define  TCErrorPacket              71
#define  ACSwitchPacket             72
#define  ACIOPacket						73
#define  ACErrorPacket              79

#define  InitErrorMessage           80

#define  BadgeScan                  90
#define  BadgeError                 91
#define  BadgePicker                92

#define  StartSuperpicker           98
#define  StopSuperpicker            99

#define  Transaction                100
#define  ToteLabel                  101
#define  ShipLabel                  102
#define  PackingList                103
#define  BoxPackingList             104
#define  ShortNotice                105
#define  RestockNotice              106

#define  APURequest                 110
#define  APUResponse                111

#define  TTYServerOpen              118
#define  TTYServerClose             119
#define  TTYServerWho               120
#define  ScreenClear                121
#define  ScreenDisplay              122
#define  ChangePrinter              123
#define  ChangeOperator             124
#define  KeyTab                     125
#define  Keystroke                  126
#define  InputField                 127
#define  ZoneNextRequest 	ZoneNext

/*-------------------------------------------------------------------------*
 *  System Messages and Events
 *-------------------------------------------------------------------------*/

#define  InitializeRequest          Initialize
#define  ConfigureRequest           Configure
#define  MarkplaceRequest           Markplace
#define  RestoreplaceRequest        Restoreplace
#define  SystemRecoveryRequest      SystemRecovery
#define  ShutdownRequest            Shutdown

#define  PortInitializeRequest      PortInitialize
#define  PortMarkplaceRequest       PortMarkplace
#define  PortDisableRequest         PortDisable
#define  PortEnableRequest          PortEnable
#define  PortRedisplayRequest       PortRedisplay
#define  PortDisableEvent           Event + PortDisable
#define  PortEnableEvent            Event + PortEnable

#define  ShortPrintDisableRequest   ShortPrintDisable
#define  ShortPrintEnableRequest    ShortPrintEnable
#define  RestockPrintDisableRequest RestockPrintDisable
#define  RestockPrintEnableRequest  RestockPrintEnable

#define  ClientMessageEvent         Event + ClientMessage
#define  SystemMessageEvent         Event + SystemMessage

#define  InitializeEvent            Event + Initialize
#define  ConfigureEvent             Event + Configure
#define  MarkplaceEvent             Event + Markplace
#define  RestoreplaceEvent          Event + Restoreplace
#define  SystemRecoveryEvent        Event + SystemRecovery
#define  ShutdownEvent              Event + Shutdown

#define  PortInitializeEvent        Event + PortInitialize
#define  PortMarkplaceEvent         Event + PortMarkplace

#define  ShortPrintDisableEvent     Event + ShortPrintDisable
#define  ShortPrintEnableEvent      Event + ShortPrintEnable
#define  RestockPrintDisableEvent   Event + RestockPrintDisable
#define  RestockPrintEnableEvent    Event + RestockPrintEnable

/*-------------------------------------------------------------------------*
 *  Pickline Requests and Events
 *-------------------------------------------------------------------------*/
 
#define  PicklineDisableRequest     PicklineDisable
#define  PicklineEnableRequest      PicklineEnable
#define  PicklineLockRequest        PicklineLock
#define  PicklineUnlockRequest      PicklineUnlock
#define  PicklineRedisplayRequest   PicklineRedisplay
#define  PicklineStopRequest        PicklineStop

#define  PicklineDisableEvent       Event + PicklineDisable
#define  PicklineEnableEvent        Event + PicklineEnable
#define  PicklineLockEvent          Event + PicklineLock
#define  PicklineUnlockEvent        Event + PicklineUnlock
#define  PicklineRedisplayEvent     Event + PicklineRedisplay
#define  PicklineStopEvent          Event + PicklineStop

/*-------------------------------------------------------------------------*
 *  Zone Requests and Events
 *-------------------------------------------------------------------------*/

#define  ZoneStopRequest            ZoneStop
#define  ZoneStartRequest           ZoneStart
#define  ZoneRedisplayRequest       ZoneRedisplay
#define  ZoneClearRequest           ZoneClear
#define  ZoneDisplayRequest         ZoneDisplay
#define  ZoneTestRequest            ZoneTest
#define  ZoneOfflineRequest         ZoneOffline
#define  ZoneOnlineRequest          ZoneOnline

#define  ZoneStopEvent              Event + ZoneStop
#define  ZoneStartEvent             Event + ZoneStart
#define  ZoneCompleteEvent          Event + ZoneComplete
#define  ZoneNextEvent              Event + ZoneNext
#define  ZoneRedisplayEvent         Event + ZoneRedisplay
#define  ZoneClearEvent             Event + ZoneClear
#define  ZoneTestEvent              Event + ZoneTest
#define  ZoneStatusEvent            Event + ZoneStatus

/*-------------------------------------------------------------------------*
 *  Module Requests and Events
 *-------------------------------------------------------------------------*/

#define  ModuleInhibitRequest       ModuleInhibit
#define  ModuleEnableRequest        ModuleEnable
#define  ModulePickRequest          ModulePick
#define  ModulePickBoxRequest			ModulePickBox

#define  ModuleInhibitEvent         Event + ModuleInhibit
#define  ModuleEnableEvent          Event + ModuleEnable
#define  ModulePickEvent            Event + ModulePick
#define  ModuleSplitEvent           Event + ModuleSplit
#define  ModuleRestockEvent         Event + ModuleRestock
#define	ModulePickBoxEvent			Event + ModulePickBox
/*-------------------------------------------------------------------------*
 *  Order Requests and Events
 *-------------------------------------------------------------------------*/

#define  OrderPickRequest           OrderPick
#define  OrderRepickRequest         OrderRepick
#define  OrderHoldRequest           OrderHold
#define  OrderAcitvateRequest       OrderActivate
#define  OrderPriorityRequest       OrderPriority
#define  OrderReleaseRequest        OrderRelease
#define  OrderCancelRequest         OrderCancel
#define  OrderSplitRequest          OrderSplit
#define  OrderPurgeRequest          OrderPurge

#define  OrderInputEvent            Event + OrderInput
#define  OrderManualEntryEvent      Event + OrderManualEntry
#define  OrderRepickEvent           Event + OrderRepick
#define  OrderToteAheadEvent        Event + OrderToteAhead
#define  OrderUnderwayEvent         Event + OrderUnderway
#define  OrderCompleteEvent         Event + OrderComplete
#define  OrderCancelEvent           Event + OrderCancel
#define  OrderSplitEvent            Event + OrderSplit
#define  OrderPurgeEvent            Event + OrderPurge

/*-------------------------------------------------------------------------*
 *  Group Requests and Events
 *-------------------------------------------------------------------------*/

#define  GroupPickRequest           GroupPick
#define  GroupHoldRequest           GroupHold
#define  GroupActivateRequest       GroupActivate
#define  GroupPriorityRequest       GroupPriority
#define  GroupCancelRequest         GroupCancel

#define  GroupCancelEvent           Event + GroupCancel

/*-------------------------------------------------------------------------*
 *  Box Requests and Events
 *-------------------------------------------------------------------------*/

#define  BoxCloseRequest            BoxClose
#define  BoxOpenRequest             BoxOpen
#define  BoxNextRequest             BoxNext
#define  BoxSplitRequest            BoxSplit

#define  BoxCloseEvent              Event + BoxClose
#define  BoxOpenEvent               Event + BoxOpen
#define  BoxNextEvent               Event + BoxNext
#define  BoxSplitEvent              Event + BoxSplit
#define  BoxScanEvent               Event + BoxScan

/*------------------------------------------------------------------------*
 *  Lot Requests And Events
 *------------------------------------------------------------------------*/
 
#define  LotChangeRequest           LotChange

#define  LotChangeEvent             Event + LotChange

/*------------------------------------------------------------------------*
 *  Device Specific Events
 *------------------------------------------------------------------------*/

#define  TCInputPacketEvent         Event + TCInputPacket
#define  TCErrorPacketEvent         Event + TCErrorPacket

#define  ACSwitchPacketEvent        Event + ACSwitchPacket
#define  ACErrorPacketEvent         Event + ACErrorPacket
#define  ACIOPacketEvent				Event + ACIOPacket

#define  InitErrorMessageEvent      Event + InitErrorMessage

#define  BadgeScanEvent             Event + BadgeScan
#define  BadgeErrorEvent            Event + BadgeError
#define  BadgePickerEvent           Event + BadgePicker

/*-------------------------------------------------------------------------*
 *  Output Transaction Event
 *-------------------------------------------------------------------------*/
 
#define  TransactionEvent           Event + Transaction
#define  ToteLabelEvent             Event + ToteLabel
#define  ShipLabelEvent             Event + ShipLabel
#define  PackingListEvent           Event + PackingList
#define  BoxPackingListEvent        Event + BoxPackingList
#define  ShortNoticeEvent           Event + ShortNotice
#define  RestockNoticeEvent         Event + RestockNotice

/*-------------------------------------------------------------------------*
 *  TTY Server Requests and Events
 *-------------------------------------------------------------------------*/

#define  TTYServerOpenRequest       TTYServerOpen
#define  TTYServerWhoRequest        TTYServerWho
#define  KeyTabRequest              KeyTab
#define  KeystrokeRequest           Keystroke
#define  InputFieldRequest          InputField

#define  TTYServerOpenEvent         Event + TTYServerOpen
#define  TTYServerCloseEvent        Event + TTYServerClose
#define  TTYServerWhoEvent          Event + TTYServerWho
#define  ScreenClearEvent           Event + ScreenClear
#define  ScreenDisplayEvent         Event + ScreenDisplay
#define  ChangePrinterEvent         Event + ChangePrinter
#define  ChangeOperatorEvent        Event + ChangeOperator
#define  KeystrokeEvent             Event + Keystroke
#define  InputFieldEvent            Event + InputField

#endif

/* end of message_types.h */
