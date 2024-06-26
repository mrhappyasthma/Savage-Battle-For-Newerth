// (C) 2003 S2 Games

// net_win32.c

// network driver layer

#ifdef _WIN32

#include "core.h"
#include "net_winsock.h"

_num_netdrivers = 1;

_net_driver_t _netdrivers[] = 
{
	{
		"WinSock UDP/IP",
	
		WSock_Init,
		WSock_ShutDown,

		WSock_OpenPort,
		WSock_SetSendAddr,
		WSock_NewSocket,
		WSock_ReceivePacket,
		WSock_PreProcessPacket,
		WSock_SendPacket,
		WSock_SendReliablePacket,
		WSock_CloseNetConnection,
		WSock_Converse,

		WSock_BroadcastPacket,

		WSock_CheckPacketTimeouts,
		WSock_ClearReliablePackets,
	}
};

#endif  //_WIN32
