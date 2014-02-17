#define MIO
/*-------------------------------------------------------------------------*
 *  Source Code:    %M%
 *-------------------------------------------------------------------------*
 *  Version:        %I%
 *  Version Date:   %G% - %U%
 *  Source Date:    %H% - %T%
 *
 *  Description:    AC packet defintions.
 *
 *-------------------------------------------------------------------------*
 *                            Revision history                             |
 *-------------+-----------------------------------------------------------*
 * Change Date |            Author & Description                           |
 *-------------+-----------------------------------------------------------*
 *  03/7/94    |  tjt  Original implementation.
 *-------------------------------------------------------------------------*/
static char alc_packet_h[] = "%Z% %M% %I% (%G% - %U%)";

/*-------------------------------------------------------------------------*
 *  Null (Error) Packet
 *-------------------------------------------------------------------------*/
typedef struct
{
  unsigned char packet_ac_addr[4];
  unsigned char packet_command[2];
  unsigned char packet_err_count[2];
  unsigned char packet_error;
  
} TNullPacketItem;

/*-------------------------------------------------------------------------*
 *  Type Return Packet - 02
 *-------------------------------------------------------------------------*/
typedef struct
{
  unsigned char packet_ac_addr[4];
  unsigned char packet_command[2];
  unsigned char packet_map[256];
    
} TLengthPacketItem;

/*-------------------------------------------------------------------------*
 *  Status Return Packet - 03
 *-------------------------------------------------------------------------*/
typedef struct
{
  unsigned char packet_ac_addr[4];
  unsigned char packet_command[2];
  unsigned char packet_prom_date[8];
  unsigned char packet_prom_check[2];
  unsigned char packet_eeprom_date[8];
  unsigned char packet_eeprom_check[2];
  
} TStatusPacketItem;

/*-------------------------------------------------------------------------*
 *  Switch Action Return Packet - 10
 *-------------------------------------------------------------------------*/
typedef struct
{
  unsigned char packet_ac_addr[4];
  unsigned char packet_command[2];
  unsigned char packet_mod_addr[3];
  unsigned char packet_switch;
  unsigned char packet_port;
  
} TSwitchPacketItem;
/*-------------------------------------------------------------------------*
 *  Scanner Return Packet - 10
 *-------------------------------------------------------------------------*/
typedef struct
{
  unsigned char packet_ac_addr[4];
  unsigned char packet_command[2];
  unsigned char packet_mod_addr[3];
  unsigned char packet_text[24];
  unsigned char packet_port;
  
} TIOPacketItem;

#ifdef MIO
/*-------------------------------------------------------------------------*
 *  Multiple IO Input Packet - 10
 *-------------------------------------------------------------------------*/
typedef struct
{
  unsigned char packet_ac_addr[4];
  unsigned char packet_command[2];
  unsigned char packet_mod_addr[3];
  unsigned char packet_byte;
  unsigned char packet_port;

} TMIOPacketItem;
#endif
/*-------------------------------------------------------------------------*
 *  Union Of Return Packets
 *-------------------------------------------------------------------------*/

typedef union
{
  TNullPacketItem    NullPacket;
  TLengthPacketItem  LengthPacket;
  TStatusPacketItem  StatusPacket;
  TSwitchPacketItem  SwitchPacket;
  TIOPacketItem		IOPacket;
#ifdef MIO
  TMIOPacketItem                MIOPacket;
#endif
  
} TPacketItem;


/* end of alc_packet.h */
