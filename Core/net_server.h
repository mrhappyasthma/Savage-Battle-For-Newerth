void    Net_Server_Init();
int    	Net_Server_Listen_For_Cookies();
void    Net_CloseCookieSocket();
void    Net_Server_TCPListenFrame();

void    Net_Server_ClearStats();
bool    Net_Server_AddToStats(packet_t *pkt);
bool    Net_Server_SendStats();

void	Net_Server_Resolve();

