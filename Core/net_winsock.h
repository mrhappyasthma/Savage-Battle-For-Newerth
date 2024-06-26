// (C) 2003 S2 Games

// net_winsock.h


void		WSock_Init();
void		WSock_ShutDown();

bool		WSock_OpenPort(netconnection_t *nc, int port);
bool		WSock_SetSendAddr(netconnection_t *nc, char *addr, int clientport);
bool    	WSock_NewSocket(netconnection_t *nc);

int			WSock_ReceivePacket(netconnection_t *nc);
bool		WSock_PreProcessPacket(netconnection_t *nc);
bool		WSock_SendPacket(netconnection_t *nc);
bool		WSock_SendReliablePacket(netconnection_t *nc);
void		WSock_CloseNetConnection(netconnection_t *nc);
void		WSock_Converse(netconnection_t *nc);	

bool    	WSock_BroadcastPacket(netconnection_t *nc, int port);

void		WSock_CheckPacketTimeouts(netconnection_t *nc);

void		WSock_ClearReliablePackets(netconnection_t *nc);

char		*WSock_GetErrorString(int err);
