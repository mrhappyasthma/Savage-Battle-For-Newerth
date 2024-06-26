void    TCP_SetSignalHandlers();
void	TCP_Init();

//server-only functions
int		TCP_Listen(int socket, int port);
int 	TCP_GetConnection(int s, struct sockaddr_in *addr);

//client/server functions
int		TCP_Connect(char *hostname, int port);
int     TCP_ConnectToSockaddr(struct sockaddr_in sockaddr); //to avoid the name resolution
void	TCP_Close(int s);
int		TCP_Read(int s, char *buf, int buf_len);
int		TCP_Write(int s, char *buf, int buf_len);
