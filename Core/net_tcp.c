/*
 * (C)2003 S2 Games
 *
 * net_tcp.c - tcp/ip net code for both win32 and unix
 */

#include "core.h"
#include "../keyserver/ssl-utils.h"
#include "../keyserver/auth_common.h"
#include "cookie.h"

#define BUF_LEN 1024
#define MAX_QUEUE_LENGTH 64

cvar_t tcp_timeout = { "tcp_timeout", "10" };

int		TCP_Listen(int s, int port)
{
	int val = 1;
	struct timeval timeo;
	//char mac[6];
	struct sockaddr_in sockaddr;
	
	if (s)
	{
		Console_DPrintf("we already have a socket, so not initializing\n");
		return s;
	}
	
	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		Console_Errorf("Error opening local server socket.\n");
		return -1;
	}
	
#ifndef _WIN32
	//mark the socket as SO_REUSEADDR so we can quickly kill and restart the daemon
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val) < 0)
	{
		Console_Errorf("unable to mark the listen socket as SO_REUSEADDR\n");
	}
#endif
	
	timeo.tv_sec  = tcp_timeout.value;
	timeo.tv_usec = 0;
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeo, sizeof (timeo)) < 0)
	{
		Console_Errorf("unable to set socket option SO_RCVTIMEO - %s\n", strerror(errno));
	}

	timeo.tv_sec  = tcp_timeout.value;
	timeo.tv_usec = 0;
	if (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeo, sizeof (timeo)) < 0)
	{
		Console_Errorf("unable to set socket option SO_SNDTIMEO - %s\n", strerror(errno));
	}
	
	
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons((unsigned short)port);
	sockaddr.sin_addr.s_addr = htons(INADDR_ANY);
	
	if (bind(s, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) != 0)
	{
#ifndef _WIN32
		Console_DPrintf("tcp socket - error trying to bind socket - error %i\n", errno);
#else
		Console_DPrintf("tcp socket - error trying to bind socket - error %i\n", WSAGetLastError());
#endif
		return -1;
	}
	
	set_nonblocking(s);
	
	if (listen(s, MAX_QUEUE_LENGTH) != 0)
	{
		Console_Printf("someone else is listening to this port already\n");
		return -1;
	}
	
	return s;
}

void	TCP_Close(int s)
{
#ifndef _WIN32
	close(s);
#else
	closesocket(s);
#endif
}

/* wait for a connection to occur on a socket created with establish() */
int TCP_GetConnection(int s, struct sockaddr_in *addr)
{
	int t, socklen;
	struct timeval timeo;
	
	socklen = sizeof(struct sockaddr_in);
	if ((t = accept(s, (struct sockaddr *)addr, &socklen)) < 0)
	/* accept connection if there is one */
		return(0);
	set_nonblocking(t);

	timeo.tv_sec  = tcp_timeout.value;
	timeo.tv_usec = 0;
	if (setsockopt(t, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeo, sizeof (timeo)) < 0)
	{
		Console_Errorf("unable to set socket option SO_RCVTIMEO - %s\n", strerror(errno));
	}

	timeo.tv_sec  = tcp_timeout.value;
	timeo.tv_usec = 0;
	if (setsockopt(t, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeo, sizeof (timeo)) < 0)
	{
		Console_Errorf("unable to set socket option SO_SNDTIMEO - %s\n", strerror(errno));
	}
	
	return(t);
}

int		TCP_Read(int s, char *buf, int buf_len)
{
	int len;
#ifdef _WIN32
	int error;
#endif

	if (!s)
		return -1;
	
	while (true)
	{
		if ((len = recv(s, buf, buf_len, 0)) > 0)
		{
			return len;
		}
	
		if (len < 0)
		{
#ifdef _WIN32
			error = WSAGetLastError();
			if (error != EAGAIN && error != WSAEWOULDBLOCK)
#else
			if (errno != EAGAIN)
#endif
			{
				/* signal an error to the caller */
				return 0;
			}
			return 0;
		}
		else if (len == 0)
		{
			return 0;
		}
	}
}
						
int		TCP_Write(int s, char *buf, int buf_len)
{
	int ret;
	
	if (!s)
		return -1;
	
	Console_DPrintf("sending %s\n", buf);
#ifdef _WIN32
	ret = send(s, buf, buf_len, 0);
#else //_WIN32

#ifdef MSG_NOSIGNAL
	ret = send(s, buf, buf_len, MSG_NOSIGNAL);
#else
	ret = send(s, buf, buf_len, 0);
#endif

#endif //_WIN32
	return ret;
}

int		TCP_ConnectToSockaddr(struct sockaddr_in sockaddr)
{
	int s;
	struct timeval timeo;
	
	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s <= 0)
	{
		Console_DPrintf("Error opening socket.\n");
		return false;
	}
	
	timeo.tv_sec  = tcp_timeout.value;
	timeo.tv_usec = 0;
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeo, sizeof (timeo)) < 0)
	{
		Console_Errorf("unable to set socket option SO_RCVTIMEO - %s\n", strerror(errno));
	}

	timeo.tv_sec  = tcp_timeout.value;
	timeo.tv_usec = 0;
	if (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeo, sizeof (timeo)) < 0)
	{
		Console_Errorf("unable to set socket option SO_SNDTIMEO - %s\n", strerror(errno));
	}
	
	if (connect(s, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) < 0)
	{
		Console_DPrintf("Server connect failed\n");
		return false;
	}
	
	return s;
}

int		TCP_Connect(char *hostname, int port)
{
	struct sockaddr_in sockaddr;
	struct hostent *server;
	
	server = gethostbyname(hostname);
	if (!server)
	{
		Console_DPrintf("Unable to resolve %s\n", hostname);
		return false;
	}
	
	sockaddr.sin_family = PF_INET;
	sockaddr.sin_port = htons((unsigned short)port);
	memcpy(&sockaddr.sin_addr, server->h_addr, server->h_length);

	return TCP_ConnectToSockaddr(sockaddr);
}

#ifndef _WIN32
void	_jon_signal_handler(int signal)
{
	if (signal == SIGPIPE)
		return;
	else
		Console_DPrintf("Weird signal %i\n", signal);
}
#endif

void	TCP_SetSignalHandlers()
{
#ifndef _WIN32
	__sighandler_t ret;

	Console_DPrintf("Ignoring signal %i\n", SIGPIPE);
	ret = signal(SIGPIPE, SIG_IGN);
	if (ret == SIG_ERR)
	{
		Console_DPrintf("error setting SIGPIPE to ignore!\n");
	}
#endif
}

void	TCP_Init()
{
	Cvar_Register(&tcp_timeout);
}
