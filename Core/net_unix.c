// (C) 2003 S2 Games

// net_unix.c

// network driver layer for BSD socket machines

#include "core.h"
#include "net_bsdsock.h"

int _num_netdrivers = 1;

_net_driver_t _netdrivers[] = 
{
	{
		"BSD Socket UDP/IP",
	
		BSDSock_Init,
		BSDSock_ShutDown,

		BSDSock_OpenPort,
		BSDSock_SetSendAddr,
		BSDSock_NewSocket,
		
		BSDSock_ReceivePacket,
		BSDSock_PreProcessPacket,
		BSDSock_SendPacket,
		BSDSock_SendReliablePacket,
		BSDSock_CloseNetConnection,
		BSDSock_Converse,

		BSDSock_BroadcastPacket,

		BSDSock_CheckPacketTimeouts,
		BSDSock_ClearReliablePackets
	}
};

