
#include "core.h"

#define STATS_VERSION0	0
#define STATS_VERSION1	1
#define STATS_VERSION2	2

bool	Server_WriteStatsHeader(packet_t *pkt)
{
	Pkt_WriteByte(pkt, STATS_VERSION2); //protocol version
	Pkt_WriteShort(pkt, (short)Server_GetPort());
	
	return true;
}

