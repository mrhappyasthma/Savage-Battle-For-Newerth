// (C) 2003 S2 Games

// net_bsdsock.h


void		BSDSock_Init();
void		BSDSock_ShutDown();

bool		BSDSock_OpenPort(netconnection_t *nc, int port);
bool		BSDSock_SetSendAddr(netconnection_t *nc, char *addr, int clientport);
bool    	BSDSock_NewSocket(netconnection_t *nc);

int     	BSDSock_ReceivePacket(netconnection_t *nc);
bool    	BSDSock_PreProcessPacket(netconnection_t *nc);
bool		BSDSock_SendPacket(netconnection_t *nc);
bool		BSDSock_SendReliablePacket(netconnection_t *nc);
void		BSDSock_CloseNetConnection(netconnection_t *nc);
void		BSDSock_Converse(netconnection_t *nc);	

bool    	BSDSock_BroadcastPacket(netconnection_t *nc, int port);
void        BSDSock_CheckPacketTimeouts(netconnection_t *nc);

bool		BSDSock_Connect(char *addr);

void 		BSDSock_GetAvgTraffic(float *sent, float *received);

void    	BSDSock_ClearReliablePackets(netconnection_t *nc);
