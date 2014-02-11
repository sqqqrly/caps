/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    CAPS Engine Messages Types To/From APU.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 * 12/22/93    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
#ifndef ENGINE_MESSAGE_TYPES_H
#define ENGINE_MESSAGE_TYPES_H

static char engine_message_types_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Messages From APU To The CAPS Engine
 *-------------------------------------------------------------------------*/
 
#define EngineStart                   1
#define EngineStop                    2
#define EngineEnablePickline          3
#define EngineDisablePickline         4
#define EngineLockPickline            5
#define EngineUnlockPickline          6
#define EngineStopZoneFeeding         7
#define EngineCancelOrder             8
#define EngineHoldOrder               9
#define EngineActivateOrder          10
#define EngineRedisplayPickline      11
#define EngineRedisplayZone          12
#define EngineZoneDisplay            13
#define EngineProcessOrders          14
#define EngineLogin                  15
#define EngineLogout                 16
#define EngineOrderRelease           17
#define EngineChangeTote             18
#define EnginePurgeOrders            19

/*-------------------------------------------------------------------------*
 *  Messages From The CAPS Engine To The APU
 *-------------------------------------------------------------------------*/

#define EngineStartEvent             32
#define EngineStopEvent              33
#define EngineEnablePicklineEvent    34
#define EngineDisablePicklineEvent   35
#define EngineLockPicklineEvent      36
#define EngineUnlockPicklineEvent    37
#define EngineStopZoneFeedingEvent   38
#define EngineOrderEvent             39
#define EngineCancelOrderEvent       40
#define EngineHoldOrderEvent         41
#define EngineActivateOrderEvent     42
#define EnginePickEvent              43
#define EngineZoneStatusEvent        44
#define EngineLoginEvent             45
#define EngineLogoutEvent            46
#define EngineOrderReleaseEvent      47
#define EngineChangeToteEvent        48
#define EngineProcessOrdersEvent     49

/*-------------------------------------------------------------------------*
 *  Event Response Codes
 *-------------------------------------------------------------------------*/
 
#define CE_OK        '0'                   /* action completed               */
#define CE_REJECT    '1'                   /* action impossible              */
#define CE_BUSY      '2'                   /* caps busy start/stop           */
#define CE_FILE      '3'                   /* no config/current file         */
#define CE_ORDER     '4'                   /* order not found                */
#define CE_ZONE      '5'                   /* zone not found/appropriate     */
#define CE_HW        '6'                   /* start failed on hardware       */
#define CE_CONFIG    '7'                   /* error in configuration         */

#endif
 
/* end of engine_message_types.h */



