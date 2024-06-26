// (C) 2003 S2 Games

// packet.h - specific to the heartbeat server

// message handling over the network

#include "common.h"

#define MAX_PACKET_SIZE	8096
#define HEADER_SIZE	5

typedef struct
{
	unsigned char	buf[MAX_PACKET_SIZE + HEADER_SIZE];
	int		curpos;
	int		length;
} packet_t;


void	Pkt_Clear(packet_t *pkt);

bool	Pkt_WriteInt(packet_t *pkt, int i);
bool	Pkt_WriteShort(packet_t *pkt, short i);
bool	Pkt_WriteByte(packet_t *pkt, byte b);
bool	Pkt_WriteString(packet_t *pkt, char *s);
bool	Pkt_WriteCmd(packet_t *pkt, byte cmd);
bool	Pkt_WriteCoord(packet_t *pkt, float coord);
bool	Pkt_WriteByteAngle(packet_t *pkt, float angle);
bool	Pkt_WriteWordAngle(packet_t *pkt, float angle);
bool	Pkt_WriteFloat(packet_t *pkt, float f);


int		Pkt_ReadInt(packet_t *pkt);
short	Pkt_ReadShort(packet_t *pkt);
byte	Pkt_ReadByte(packet_t *pkt);
void	Pkt_ReadString(packet_t *pkt, char *buf, int size);
byte	Pkt_ReadCmd(packet_t *pkt);
bool	Pkt_DoneReading(packet_t *pkt);
float	Pkt_ReadCoord(packet_t *pkt);
float	Pkt_ReadFloat(packet_t *pkt);
float	Pkt_ReadByteAngle(packet_t *pkt);
float	Pkt_ReadWordAngle(packet_t *pkt);

bool    Pkt_DoneReading(packet_t *pkt);
void	Pkt_Copy(packet_t *from, packet_t *to);
void    Pkt_Import(packet_t *pkt, char *buf, int len);
