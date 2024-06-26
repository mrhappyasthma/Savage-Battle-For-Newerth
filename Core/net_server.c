/*
 * (C)2003 S2 Games
 *
 * net_server.c - server-specific tcp/ip net code for both win32 and unix
 */

#include "core.h"
#include "../keyserver/ssl-utils.h"
#include "cookie.h"
#include "net_tcp.h"

#ifdef _WIN32
	SOCKET server_listen_sock = 0;
#else
	int server_listen_sock = 0;
#endif

#define KEYSERVER_COOKIE_SIZE 10
	
char cookie[KEYSERVER_COOKIE_SIZE+1];

//#define STATS_SERVER_ADDRESS "statsserver.savage.s2games.com"
//#define STATS_SERVER_PORT	 21337
cvar_t	statserver_ip = 	{ "statserver_ip", "statsserver.savage.s2games.com" };
cvar_t	statserver_port =	{ "statserver_port", "21337" };

#define BUF_LEN 1024

#ifdef _WIN32
	SOCKET t = 0;
#else
	static int t = 0;
#endif

#define ls localServer
	
extern cvar_t svr_port;
extern cvar_t svr_firewalled;
extern cvar_t svr_adminPassword;
extern cvar_t svr_allowRemoteSvcmds;

#define MAX_KEYSERVER_ADDR 8

//struct hostent statsserver;
//struct hostent keyserver;
char *keyserver_addr_list[MAX_KEYSERVER_ADDR];
int keyserver_h_length = 0;

//buffer to hold all the stats. yes, it's big
static char stats_buffer[65536] = {0};
static int stats_buffer_pos = 0;

static struct sockaddr_in statsserver_sockaddr;
static bool statsserver_resolved = false;

void Net_Server_Resolve()
{
	struct hostent *statsserver;
	//statsserver = gethostbyname(STATS_SERVER_ADDRESS);
	statsserver = gethostbyname(statserver_ip.string);

	if (!statsserver)
	{
		//Console_DPrintf("Unable to resolve %s\n", STATS_SERVER_ADDRESS);
		Console_DPrintf("Unable to resolve %s\n", statserver_ip.string);
		return;
	}
	
	statsserver_sockaddr.sin_family = PF_INET;
	//statsserver_sockaddr.sin_port = htons((unsigned short)STATS_SERVER_PORT);
	statsserver_sockaddr.sin_port = htons((unsigned short)statserver_port.integer);
	memcpy(&statsserver_sockaddr.sin_addr, statsserver->h_addr, statsserver->h_length);
	statsserver_resolved = true;
}

void	Net_Server_FirewallTest_cmd(int argc, char *argv[])
{
	MasterServer_TestFirewall();
}

void	Net_CopyHostEnt()
{
	
}

void	Net_Server_Init()
{
	struct hostent *ent;

	server_listen_sock = 0;
	cookie[KEYSERVER_COOKIE_SIZE+1] = 0;

	ent = gethostbyname(KEYSERVER_IP);
	if (!ent)
	{
		Console_Errorf("Unable to resolve the keyserver %s\n", KEYSERVER_IP);
		keyserver_addr_list[0] = NULL;
	}
	else
	{
		int n = 0;

		//copy address info
		while (n < MAX_KEYSERVER_ADDR)
		{
			if (!ent->h_addr_list[n])
				break;
			
			keyserver_addr_list[n] = Tag_Strdup(ent->h_addr_list[n], MEM_SYSTEM);
			
			n++;
		}
		
		keyserver_h_length = ent->h_length;
	}

	Cmd_Register("testfirewall", Net_Server_FirewallTest_cmd);
	Cvar_Register(&statserver_ip);
	Cvar_Register(&statserver_port);
}

int		Net_Server_Listen_For_Cookies()
{
	Console_Printf("listening for tcp connections on port %i\n", ls.port);
	server_listen_sock = TCP_Listen(server_listen_sock, ls.port);

	return server_listen_sock;
}

void	Net_CloseCookieSocket()
{
	TCP_Close(server_listen_sock);
	server_listen_sock = 0;
}

static int outputSocket = 0;

void	SendOutputToSocket(char *s)
{
	TCP_Write(outputSocket, s, strlen(s));
}

bool	process_cmd(int s, char *buf, int buf_len, struct sockaddr_in from)
{
	char our_buf[1024];
	char *pos, *player_cookie, *clan, *user_id, *guid;
	int i;
	
	if (strcmp(buf, KS_HI_CMD) == 0)
	{
		if (!generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE))
		{
			Console_Printf("error generating cookie!\n");
			return false;
		}
		BPrintf(our_buf, 1024, "%s %s", GS_HI_CMD, cookie);
		TCP_Write(s, our_buf, strlen(our_buf));
		return true;
	}
	else if (strncmp(buf, REMOTE_SVCMD_CMD, strlen(REMOTE_SVCMD_CMD)) == 0)
	{
		char *cmdstart, *adminP;
		
		if (!svr_allowRemoteSvcmds.integer)
		{
			Console_Printf("remote svcmd on the server tcp port, but svcmds are disabled (change svr_allowRemoteSvcmds to 1 to enable)\n");
			return false;
		}
		
		if (strlen(buf) < strlen(REMOTE_SVCMD_CMD) + KEYSERVER_COOKIE_SIZE + 1)
		{
			Console_Printf("REMOTE_SVCMD_CMD packet too short!\n");
			return false;
		}
		//pos will now point to the first char of any possible string
		pos = &buf[strlen(REMOTE_SVCMD_CMD)+1];
		//now see if there is a space
		if (pos[KEYSERVER_COOKIE_SIZE] != ' ')
		{
			Console_Printf("ALLOW_COOKIE_CMD - invalid packet!\n");
			return false;
		}
		if (strncmp(pos, cookie, KEYSERVER_COOKIE_SIZE) != 0)
		{
			Console_Printf("cookie doesn't match - we gave them %s, received cookie %s!\n", cookie, pos);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}
		pos += KEYSERVER_COOKIE_SIZE + 1;
		if (!strchr(pos, ' '))
		{
			Console_Printf("Invalid string for a remote (tcp) svcmd: %s\n", buf);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}
		adminP = fmt("%s ", svr_adminPassword.string);
		if (!svr_adminPassword.string[0])
		{
			Console_Printf("Admin Password isn't set, not running remote svcmd %s\n", buf);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}

		if (strncmp(adminP, pos, strlen(adminP)) != 0)
		{
			Console_Printf("Invalid admin password in remote svcmd: %s\n", buf);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}
		cmdstart = strchr(pos, ' ') + 1;
		if (!cmdstart || !cmdstart[0])
		{
			Console_Printf("Invalid remote svcmd: %s\n", buf);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}
		Console_Printf("remote attempt to do a svcmd %s*\n", cmdstart);

		outputSocket = s;
		Console_SetOutputCallback(SendOutputToSocket);
		Cmd_Exec(fmt("%s", cmdstart));
		Console_SetOutputCallback(NULL);
		outputSocket = 0;
		return false;
	}
	else if (strncmp(buf, ALLOW_COOKIE_CMD, strlen(ALLOW_COOKIE_CMD)) == 0)
	{
		if (strlen(buf) < strlen(ALLOW_COOKIE_CMD) + KEYSERVER_COOKIE_SIZE + COOKIE_SIZE + 2)
		{
			Console_Printf("ALLOW_COOKIE_CMD packet too short!\n");
			return false;
		}
		//pos will now point to the first char of the KS cookie
		pos = &buf[strlen(ALLOW_COOKIE_CMD)+1];
		//now see if there is a space
		if (pos[KEYSERVER_COOKIE_SIZE] != ' ')
		{
			Console_Printf("ALLOW_COOKIE_CMD - invalid packet!\n");
			return false;
		}
		if (strncmp(pos, cookie, KEYSERVER_COOKIE_SIZE) != 0)
		{
			Console_Printf("cookie doesn't match - we gave them %s, received cookie %s!\n", cookie, pos);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}
		player_cookie = pos + KEYSERVER_COOKIE_SIZE+1;
		player_cookie[COOKIE_SIZE] = 0;
		clan = player_cookie + COOKIE_SIZE + 1;
		
		if (strchr(clan, ' '))
			user_id = strchr(clan, ' ') + 1;
		else
			user_id = "0";

		if (strchr(user_id, ' '))
			guid = strchr(user_id, ' ') + 1;
		else
			guid = "0";

		//only accept connections from the keyserver IP
		i = 0;
		while (keyserver_addr_list[i])
		{
			if ((memcmp(&from.sin_addr, keyserver_addr_list[i], keyserver_h_length) == 0))
				break;
			i++;
		}			
	
		if (!keyserver_addr_list[i])
		{
			Console_Printf("Error: allowcookie msg from %s, which is not a valid keyserver IP\n", inet_ntoa(from.sin_addr));
			/* //commented out in case this causes errors
			TCP_Close(t);
			t = 0;
			*/
		}

		Console_DPrintf("user_id %i is trying to connect\n", atoi(user_id));

		if (!Net_AddAcceptableCookie(player_cookie, atoi(clan), atoi(user_id), atoi(guid)))
		{
			Console_Printf("failure trying to add acceptable cookie '%s'\n", player_cookie);
		}
		return false;
	}
	else if (strncmp(buf, STATESTRING_CMD, strlen(STATESTRING_CMD)) == 0)
	{
		if (strlen(buf) < strlen(STATESTRING_CMD) + KEYSERVER_COOKIE_SIZE + 2)
		{
			Console_Printf("STATESTRING_CMD packet too short!\n");
			return false;
		}
		//pos will now point to the first char of the KS cookie
		pos = &buf[strlen(STATESTRING_CMD)+1];
		//now see if there is a space
		if (pos[KEYSERVER_COOKIE_SIZE] != ' ')
		{
			Console_Printf("STATESTRING_CMD - invalid packet!\n");
			return false;
		}
		if (strncmp(pos, cookie, KEYSERVER_COOKIE_SIZE) != 0)
		{
			Console_Printf("cookie doesn't match - we gave them %s, received cookie %s!\n", cookie, pos);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}
		pos += KEYSERVER_COOKIE_SIZE+1;
		//pos will now point to the first char of the state string
		strncpy(our_buf, pos, strlen(pos)+1);
		
		//only accept connections from the keyserver IP
		i = 0;
		while (keyserver_addr_list[i])
		{
			if ((memcmp(&from.sin_addr, keyserver_addr_list[i], keyserver_h_length) == 0))
				break;
			i++;
		}			
	
		if (!keyserver_addr_list[i])
		{
			Console_Printf("Error: statestring msg from %s, which is not a valid keyserver IP\n", inet_ntoa(from.sin_addr));
			/* //commented out in case this causes errors
			TCP_Close(t);
			t = 0;
			*/
		}

		sv_api.KeyserverString(our_buf);
		return true;
	}
	else if (strncmp(buf, NEWPATCH_CMD, strlen(NEWPATCH_CMD)) == 0)
	{
		if (strlen(buf) < strlen(NEWPATCH_CMD) + KEYSERVER_COOKIE_SIZE + 1)
		{
			Console_Printf("NEWPATCH_CMD packet too short!\n");
			return false;
		}
		//pos will now point to the first char of the KS cookie
		pos = &buf[strlen(NEWPATCH_CMD)+1];
		//now see if there is a space
		if (pos[KEYSERVER_COOKIE_SIZE] != ' ')
		{
			Console_Printf("NEWPATCH_CMD - invalid packet!\n");
			return false;
		}
		if (strncmp(pos, cookie, KEYSERVER_COOKIE_SIZE) != 0)
		{
			Console_Printf("cookie doesn't match - we gave them %s, received cookie %s!\n", cookie, pos);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}
		pos += KEYSERVER_COOKIE_SIZE+1;

		//only accept connections from the keyserver IP
		i = 0;
		while (keyserver_addr_list[i])
		{
			if ((memcmp(&from.sin_addr, keyserver_addr_list[i], keyserver_h_length) == 0))
				break;
			i++;
		}			
	
		if (!keyserver_addr_list[i])
		{
			Console_Printf("Error: newpatch msg from %s, which is not a valid keyserver IP\n", inet_ntoa(from.sin_addr));
			/* //commented out in case this causes errors
			TCP_Close(t);
			t = 0;
			*/
		}
		
		sv_api.NewPatchAvailable();
		return true;
	}
	else if (strncmp(buf, NOTFIREWALLED_CMD, strlen(NOTFIREWALLED_CMD)) == 0)
	{
		if (strlen(buf) < strlen(NOTFIREWALLED_CMD) + KEYSERVER_COOKIE_SIZE + 1)
		{
			Console_Printf("NOTFIREWALLED_CMD packet too short!\n");
			return false;
		}
		//pos will now point to the first char of the KS cookie
		pos = &buf[strlen(NOTFIREWALLED_CMD)+1];
		//now see if there is a space
		if (pos[KEYSERVER_COOKIE_SIZE] != ' ')
		{
			Console_Printf("NOTFIREWALLED_CMD - invalid packet!\n");
			return false;
		}
		if (strncmp(pos, cookie, KEYSERVER_COOKIE_SIZE) != 0)
		{
			Console_Printf("cookie doesn't match - we gave them %s, received cookie %s!\n", cookie, pos);
			//scramble the cookie to make life harder for them
			generate_client_cookie(cookie, KEYSERVER_COOKIE_SIZE);
			return false;
		}
		pos += KEYSERVER_COOKIE_SIZE+1;

		//only accept connections from the keyserver IP
		i = 0;
		while (keyserver_addr_list[i])
		{
			if ((memcmp(&from.sin_addr, keyserver_addr_list[i], keyserver_h_length) == 0))
				break;
			i++;
		}			
	
		if (!keyserver_addr_list[i])
		{
			Console_Printf("Error: notfirewalled msg from %s, which is not a valid keyserver IP\n", inet_ntoa(from.sin_addr));
			/* //commented out in case this causes errors
			TCP_Close(t);
			t = 0;
			*/
		}
		
		Server_NotFirewalled();
		return true;
	}
	else
	{
		Console_DPrintf("unknown command received: '%s'\n", buf);
	}
	return false;
}

static bool    _getRestOfLine(char *str, char *to_buf, int maxlen)
{
	int i;
	char *tmp;

	if (*str && *str == '\n')
		str++;
	
	tmp = str;
	while (*tmp && *tmp != '\n')
		tmp++;
	
	i = tmp - str;
	if (maxlen < i)
		i = maxlen - 1;
	strncpy(to_buf, str, i);
	to_buf[i] = 0;
	return tmp - str;
}


void	Net_Server_TCPListenFrame()
{
	char buf[BUF_LEN + 1], line[BUF_LEN + 1];
	char *bufptr;
	int ret;
	static struct sockaddr_in from;
	static double connect_start = 0;
	
	//is this connecting dangling?
	if ( t > 0 && System_Milliseconds() - connect_start > 6000) //6 second timeout?
	{
		Console_DPrintf("closing listen socket because of the 6 second timeout!\n");
		TCP_Close(t);
		t = 0;
	}

	if (t <= 0)
	{
		t = TCP_GetConnection(server_listen_sock, &from);
		if (t)
		{
			connect_start = System_Milliseconds();
		}
	}
	if (t)
	{
		//ip = inet_ntoa(from.sin_addr);

		ret = TCP_Read(t, buf, BUF_LEN);
		if (ret)
		{
			buf[ret] = 0;
			Console_DPrintf("got %i bytes on the listen socket from %s\n", ret, inet_ntoa(from.sin_addr));
			Console_DPrintf("%s\n", buf);

			bufptr = buf;
			
			while (t && _getRestOfLine(bufptr, line, BUF_LEN))
			{
				if (!process_cmd(t, line, strlen(line), from))
				{
					Console_DPrintf("closing listen socket!\n");
					TCP_Close(t);
					t = 0;
				}
				bufptr += strlen(line);
			}
		}
	}
}

void	Net_Server_ClearStats()
{
	stats_buffer_pos = 0;
}

bool	Net_Server_AddToStats(packet_t *pkt)
{
#ifndef SAVAGE_DEMO
	if (pkt->length < 65536 - stats_buffer_pos)
	{
		Mem_Copy(&stats_buffer[stats_buffer_pos], &pkt->__buf[HEADER_SIZE], pkt->length);
		stats_buffer_pos += pkt->length;

		return true;
	}
#endif
	return false;
}	

//not using TCP_Connect because we want to avoid the DNS lookup if possible for this
bool	Net_Server_SendStats()
{
#ifndef SAVAGE_DEMO
	int s, ret;
		
	if (!statsserver_resolved)
		return false;

#ifdef unix
	if (fork() != 0)
		return true;
#endif
	
	s = TCP_ConnectToSockaddr(statsserver_sockaddr);
	if (s < 0)
	{
		Console_DPrintf("Error connecting to statsserver.\n");
		return false;
	}
	
	Console_Printf("reporting stats to master server\n");
	
	ret = TCP_Write(s, stats_buffer, stats_buffer_pos);
	if (ret < stats_buffer_pos)
	{
		Console_DPrintf("error writing stats to the socket - tried to write %i bytes, only %i written\n", stats_buffer_pos, ret);
	}

	TCP_Close(s);

#ifdef unix
	exit(0);
#endif //unix
	
	return true;
	
#else //SAVAGE_DEMO
	return false;
	
#endif //SAVAGE_DEMO
}
