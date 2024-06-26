// (C) 2003 S2 Games

// packet.c

// handles writing to / reading from a message buffer 

#include "core.h"

/*#define	CHECK_BUF(pkt) \
	if (pkt->buf < &pkt->__buf[HEADER_SIZE] || pkt->buf > &pkt->__buf[HEADER_SIZE] + MAX_PACKET_SIZE) \
	{ \
		Console_DPrintf("Packet buf was not set correctly\n"); \
	}
*/

#define CHECK_BUF(pkt) pkt->buf = &pkt->__buf[HEADER_SIZE];

void	Pkt_Clear(packet_t *pkt)
{
	pkt->curpos = 0;
	pkt->length = 0;
	pkt->overflowed = false;
}

bool	Pkt_WriteInt(packet_t *pkt, int i)
{
	union
	{
		int i;
		byte b[4];
	} u;

	byte *buf = &pkt->__buf[HEADER_SIZE];


	if (pkt->length + 4 > MAX_PACKET_SIZE)
	{
		pkt->overflowed = true;
		return false;
	}

	u.i = LittleInt(i);

	buf[pkt->curpos++] = u.b[0];
	buf[pkt->curpos++] = u.b[1];
	buf[pkt->curpos++] = u.b[2];
	buf[pkt->curpos++] = u.b[3];
	
	if (pkt->curpos > pkt->length)
		pkt->length = pkt->curpos;

	return true;
}

bool	Pkt_WriteFloat(packet_t *pkt, float f)
{
	return Pkt_WriteInt(pkt, *(int *)&f);
}

/*
bool	Pkt_WriteFloat(packet_t *pkt, float i)
{
	union
	{
		int i;
		byte b[4];
	} u;

	byte *buf = &pkt->__buf[HEADER_SIZE];

	if (pkt->length + 4 > MAX_PACKET_SIZE)
	{
		Console_DPrintf("Pkt_WriteFloat: overflow\n");
		return false;
	}

	u.i = i;

	buf[pkt->curpos++] = u.b[0];
	buf[pkt->curpos++] = u.b[1];
	buf[pkt->curpos++] = u.b[2];
	buf[pkt->curpos++] = u.b[3];
	
	if (pkt->curpos > pkt->length)
		pkt->length = pkt->curpos;

	return true;
}
*/
bool	Pkt_WriteShort(packet_t *pkt, short i)
{
	union
	{
		short i;
		byte b[2];
	} u;

	byte *buf = &pkt->__buf[HEADER_SIZE];

	if (pkt->length + 2 > MAX_PACKET_SIZE)
	{
		pkt->overflowed = true;
		return false;
	}

	u.i = LittleShort(i);

	buf[pkt->curpos++] = u.b[0];
	buf[pkt->curpos++] = u.b[1];

	if (pkt->curpos > pkt->length)
		pkt->length = pkt->curpos;

	return true;
}

bool	Pkt_WriteByte(packet_t *pkt, byte b)
{
	byte *buf = &pkt->__buf[HEADER_SIZE];

	if (pkt->length + 1 > MAX_PACKET_SIZE)
	{
		pkt->overflowed = true;
		return false;
	}

	buf[pkt->curpos++] = b;

	if (pkt->curpos > pkt->length)
		pkt->length = pkt->curpos;

	return true;
}

bool	Pkt_WriteString(packet_t *pkt, char *str)
{
	unsigned char *buf = &pkt->__buf[HEADER_SIZE];
	unsigned char *write = &buf[pkt->curpos];
	char *read = str;

	if (pkt->length + strlen(str) > MAX_PACKET_SIZE)
	{
		pkt->overflowed = true;
		return false;
	}

	while (*read)
	{
		*write++ = *read++;
		pkt->curpos++;
	}

	*write = 0;
	pkt->curpos++;

	if (pkt->curpos > pkt->length)
		pkt->length = pkt->curpos;

	return true;
}



bool	Pkt_Write(packet_t *pkt, char *data, int size)
{
	unsigned char *buf = &pkt->__buf[HEADER_SIZE];
	unsigned char *write = &buf[pkt->curpos];
	char *read = data;

	if (pkt->length + size > MAX_PACKET_SIZE)
	{
		pkt->overflowed = true;
		return false;
	}

	while (read - data < size)
	{
		*write++ = *read++;
		pkt->curpos++;
	}

	if (pkt->curpos > pkt->length)
		pkt->length = pkt->curpos;
	
	return true;
}

bool	Pkt_WriteArray(packet_t *pkt, char *array, int size)
{
	//write the length at the beginning
	Pkt_WriteShort(pkt, (short)size);
	Pkt_Write(pkt, array, size);

	return (!pkt->overflowed);
}

bool	Pkt_WriteCmd(packet_t *pkt, byte cmd)
{
	if (pkt->length + sizeof(byte) > MAX_PACKET_SIZE)
	{
		pkt->overflowed = true;
		return false;
	}

	Pkt_WriteByte(pkt, cmd);
	
	return true;
}


//fixme: compress coords
bool	Pkt_WriteCoord(packet_t *pkt, float coord)
{
	//return Pkt_WriteInt(pkt, *(int *)&coord);
	return Pkt_WriteShort(pkt, COORD2SHORT(coord));
}

//fixme: compress angles
bool	Pkt_WriteByteAngle(packet_t *pkt, float angle)
{
	return Pkt_WriteByte(pkt, ANGLE2BYTE(angle));
}

bool	Pkt_WriteWordAngle(packet_t *pkt, float angle)
{
	return Pkt_WriteShort(pkt, ANGLE2WORD(angle));
}

int		Pkt_ReadInt(packet_t *pkt)
{
	union
	{
		int i;
		byte b[4];
	} u;	

	byte *buf = &pkt->__buf[HEADER_SIZE];

	if (pkt->curpos + (signed)sizeof(int) > pkt->length)
	{
		Console_DPrintf("Pkt_ReadInt: packet too short\n");
		return 0;
	}

	u.b[0] = buf[pkt->curpos];
	u.b[1] = buf[pkt->curpos+1];
	u.b[2] = buf[pkt->curpos+2];
	u.b[3] = buf[pkt->curpos+3];
		
	pkt->curpos += sizeof(int);

	return LittleInt(u.i);
}


short	Pkt_ReadShort(packet_t *pkt)
{
	union
	{
		short i;
		byte b[2];
	} u;

	byte *buf = &pkt->__buf[HEADER_SIZE];

	if (pkt->curpos + (signed)sizeof(short) > pkt->length)
	{
		Console_DPrintf("Pkt_ReadShort: packet too short\n");
		return 0;
	}

	u.b[0] = buf[pkt->curpos];
	u.b[1] = buf[pkt->curpos+1];

	pkt->curpos += sizeof(short);

	return LittleShort(u.i);
}

byte	Pkt_ReadByte(packet_t *pkt)
{
	byte *buf = &pkt->__buf[HEADER_SIZE];

	if (pkt->curpos + (signed)sizeof(byte) > pkt->length)
	{
		Console_DPrintf("Pkt_ReadByte: packet too short\n");
		return 0;
	}

	return buf[pkt->curpos++];
}

void	Pkt_ReadString(packet_t *pkt, char *str, int size)
{
	char *s = str;
	unsigned char *buf = &pkt->__buf[HEADER_SIZE];

	while((pkt->curpos < pkt->length) && (s < str + (size - 1)))
	{
		*s = buf[pkt->curpos];

		if (*s == 0)
			break;

		s++;
		pkt->curpos++;
	}

	//even if str isn't big enough to hold this entire string, increment curpos past it
	if (s >= str + (size - 1))
	{
		while (pkt->curpos < pkt->length && buf[pkt->curpos])
			pkt->curpos++;
	}
	
	*s = 0;		//make sure the string is null-terminated
	pkt->curpos++;
}

int		Pkt_Read(packet_t *pkt, char *out, int size)
{
	char *s = out;	
	unsigned char *buf = &pkt->__buf[HEADER_SIZE];	

	while((pkt->curpos < pkt->length) 			
			&& (s < out + size))
	{
		*s = buf[pkt->curpos];

		s++;
		pkt->curpos++;
	}

	return s - out;
}


int		Pkt_ReadArray(packet_t *pkt, char *str, int maxsize)
{
	int size = Pkt_ReadShort(pkt);

	return Pkt_Read(pkt, str, MIN(maxsize, size));
}



byte	Pkt_ReadCmd(packet_t *pkt)
{
	return Pkt_ReadByte(pkt);
}

float	Pkt_ReadCoord(packet_t *pkt)
{
	float dbg = SHORT2COORD(Pkt_ReadShort(pkt));
	return dbg;
	//return *(float *)&i;
}

float	Pkt_ReadFloat(packet_t *pkt)
{
	int i = Pkt_ReadInt(pkt);

	return *(float *)&i;
}

float	Pkt_ReadByteAngle(packet_t *pkt)
{
	return BYTE2ANGLE(Pkt_ReadByte(pkt));
}

float	Pkt_ReadWordAngle(packet_t *pkt)
{
	return WORD2ANGLE(Pkt_ReadShort(pkt));
}

bool	Pkt_DoneReading(packet_t *pkt)
{
	if (pkt->curpos < pkt->length)
		return false;
	else
		return true;
}

void	Pkt_Copy(packet_t *from, packet_t *to)
{
	Mem_Copy(to->__buf, from->__buf, from->length + HEADER_SIZE);
	to->curpos = from->curpos;
	to->length = from->length;
	to->overflowed = from->overflowed;
}

void    Pkt_Import(packet_t *pkt, char *buf, int len, bool includesHeader)
{
	if (len < MAX_PACKET_SIZE + HEADER_SIZE)
	{
		if (includesHeader)
			memcpy(pkt->__buf, buf, len);
		else
			memcpy(&pkt->__buf[HEADER_SIZE], buf, len);
		pkt->curpos = 0;
		pkt->length = len;
	}
}

void	Pkt_Concat(packet_t *to, packet_t *from)
{
	memcpy(&to->__buf[to->curpos + HEADER_SIZE], &from->__buf[HEADER_SIZE], from->length);
	to->length += from->length;
	to->curpos += from->length;
}

