/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    CAPS Engine Messages To and From APU.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  05/03/93   |  tjt  Modifed to ASCII format.
 *-------------------------------------------------------------------------*/
#ifndef ENGINE_MESSAGES_H
#define ENGINE_MESSAGES_H

static char engine_messages_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Message Header Part
 *-------------------------------------------------------------------------*/
 
 typedef struct
 {
   unsigned char  CE_message_type[4];     /* engine message type             */
   unsigned char  CE_message_len[4];      /* length of message below         */
   
 } TCEHeader;

/*-------------------------------------------------------------------------*
 *  Pickline Message (Enable, Disable, Lock, Unlock, Redisplay Pickline)
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline number (zero is all)   */
  
} TCEPicklineRequest;

/*-------------------------------------------------------------------------*
 *  Order Message  (Cancel, Hold, Activate Order)
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline line number            */
  unsigned char CE_order[6];              /* order number                    */

} TCEOrderRequest;

/*-------------------------------------------------------------------------*
 *  Order Release Message 
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline line number            */
  unsigned char CE_order[6];              /* order number                    */
  unsigned char CE_zone[4];               /* zone number                     */
  
} TCEOrderRelease;

/*-------------------------------------------------------------------------*
 *  Zone Message   (Redisplay)
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_zone[4];               /* zone number                     */

} TCEZoneRequest;

/*-------------------------------------------------------------------------*
 *  Display Message To The Zone Controller   (Zone Display)
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_zone[4];               /* zone number                     */
  unsigned char CE_display[16];           /* message to picker               */

} TCEZoneDisplayRequest;

/*-------------------------------------------------------------------------*
 *  Change Tote Request
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline line number            */
  unsigned char CE_order[6];              /* order number                    */
  unsigned char CE_customer_number[7];    /* new customer order number       */
  unsigned char CE_new_order[6];          /* new tote number                 */

} TCEChangeToteRequest;


/*-------------------------------------------------------------------------*
 *  Response  (StartEvent, StopEvent) 
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_response;              /* '0' = Success, else  Failure    */
  
} TCESystemEvent;

/*-------------------------------------------------------------------------*
 *  Pickline Message Response
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline line number            */
  unsigned char CE_response;              /* '0' = Success, else Failure     */

} TCEPicklineEvent;

/*-------------------------------------------------------------------------*
 *  Zone Message Response
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_zone[4];               /* zone number                     */
  unsigned char CE_response;              /* '0' = Success, else Failure     */

} TCEZoneEvent;

/*-------------------------------------------------------------------------*
 *  Order Message Events
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline line number            */
  unsigned char CE_order[6];              /* order number                    */
  unsigned char CE_action;                /* H, U, C, or X, A                */
  
} TCEOrderEvent;

/*-------------------------------------------------------------------------*
 *  Order Release Message Response
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline line number            */
  unsigned char CE_order[6];              /* order number                    */
  unsigned char CE_zone[4];               /* zone number                     */
  unsigned char CE_response;              /* '0' = Success, else Failure     */
  
} TCEOrderReleaseEvent;

/*-------------------------------------------------------------------------*
 *  Zone Status Response
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_status;                /* status  U = underway A = ahead  */
                                          /* L = late entry E = early exit   */
                                          /* X = locked     F = finished     */
                                          /* W = waiting for picker action   */
  unsigned char CE_zone[4];               /* zone number                     */
  unsigned char CE_order[6];              /* order numnber                   */
  
} TCEZoneStatusEvent;

/*-------------------------------------------------------------------------*
 *  Pick Message   (Pick Event)
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline line number            */
  unsigned char CE_zone[4];               /* pick zone                       */
  unsigned char CE_module[5];             /* module number                   */
  unsigned char CE_order[6];              /* order number                    */
  unsigned char CE_ordered[4];            /* quantity ordered                */
  unsigned char CE_picked[4];             /* quantity actually picked        */

} TCEPickEvent;

/*-------------------------------------------------------------------------*
 *  Change Tote Event
 *-------------------------------------------------------------------------*/
typedef struct
{
  TCEHeader     CE_Header;                /* header part                     */
  unsigned char CE_pickline[2];           /* pickline line number            */
  unsigned char CE_order[6];              /* order number                    */
  unsigned char CE_customer_number[7];    /* new customer order number       */
  unsigned char CE_new_order[6];          /* new tote number                 */
  unsigned char CE_response;              /* '0' = Success, else Failure     */
  
} TCEChangeToteEvent;

/*-------------------------------------------------------------------------*
 *  Union Of All Messages
 *-------------------------------------------------------------------------*/
typedef union
{
  TCEHeader                 CESystemRequest;
  TCEPicklineRequest        CEPicklineRequest;
  TCEOrderRequest           CEOrderRequest;
  TCEOrderRelease           CEOrderRelease;
  TCEZoneRequest            CEZoneRequest;
  TCEZoneDisplayRequest     CEZoneDisplayRequest;
  TCEChangeToteRequest      CEChangeToteREquest;
  
  TCESystemEvent            CESystemEvent;
  TCEPicklineEvent          CEPicklineEvent; 
  TCEZoneEvent              CEZoneEvent;
  TCEOrderEvent             CEOrderEvent;
  TCEOrderReleaseEvent      CEOrderReleaseEvent;
  TCEPickEvent              CEPickEvent;
  TCEZoneStatusEvent        CEZoneStatusEvent;
  TCEChangeToteEvent        CEChangeToteEvent;
  
} TCEMessage;
 
#endif
 
/* end of engine_messages.h */
