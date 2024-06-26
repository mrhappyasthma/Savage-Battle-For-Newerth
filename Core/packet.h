// (C) 2003 S2 Games

// packet.h

// message handling over the network

#include "core.h"

void	Pkt_Clear(packet_t *pkt);

bool	Pkt_Write(packet_t *pkt, char *data, int size);
bool	Pkt_WriteInt(packet_t *pkt, int i);
bool	Pkt_WriteShort(packet_t *pkt, short i);
bool	Pkt_WriteByte(packet_t *pkt, byte b);
bool	Pkt_WriteString(packet_t *pkt, char *s);
bool    Pkt_WriteArray(packet_t *pkt, char *array, int size);
bool	Pkt_WriteCmd(packet_t *pkt, byte cmd);
bool	Pkt_WriteCoord(packet_t *pkt, float coord);
bool	Pkt_WriteByteAngle(packet_t *pkt, float angle);
bool	Pkt_WriteWordAngle(packet_t *pkt, float angle);
bool	Pkt_WriteFloat(packet_t *pkt, float f);

int		Pkt_Read(packet_t *pkt, char *out, int size);
int		Pkt_ReadInt(packet_t *pkt);
short	Pkt_ReadShort(packet_t *pkt);
byte	Pkt_ReadByte(packet_t *pkt);
void	Pkt_ReadString(packet_t *pkt, char *buf, int size);
int     Pkt_ReadArray(packet_t *pkt, char *str, int maxsize);
byte	Pkt_ReadCmd(packet_t *pkt);
bool	Pkt_DoneReading(packet_t *pkt);
float	Pkt_ReadCoord(packet_t *pkt);
float	Pkt_ReadFloat(packet_t *pkt);
float	Pkt_ReadByteAngle(packet_t *pkt);
float	Pkt_ReadWordAngle(packet_t *pkt);

void	Pkt_Copy(packet_t *from, packet_t *to);
void    Pkt_Import(packet_t *pkt, char *buf, int len, bool includesHeader);
void    Pkt_Concat(packet_t *to, packet_t *from);
