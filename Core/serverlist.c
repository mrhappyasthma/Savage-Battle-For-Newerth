#include "core.h"
#include "packet.h"

#include "../keyserver/ssl-utils.h"

#define MAX_USERNAME_LENGTH 40 //set in the database on the master server

bool 	wait_for_masterserver_gamelist(int socket, server_info_t *servers, int max_servers);
bool 	send_masterserver_data(int socket, char *buf, int buflen);

int		masterserverbrowser_client_connect();

cvar_t	masterserver_ip = 	{ "masterserver_ip", "masterserver.savage.s2games.com" };
cvar_t	masterserver_port = { "masterserver_port", "11236" };
cvar_t	gamelist_ip = 		{ "gamelist_ip", "masterserver.savage.s2games.com" };
cvar_t	gamelist_port = 	{ "gamelist_port", "80" };
cvar_t	gamelist_requestsPerSecond = { "gamelist_requestsPerSecond", "30" };

static 	bool waiting_for_gamelist = false;
static 	int masterserver_userlist_sock = 0;
static 	int masterserver_userinfo_sock = 0;
static 	int _num_matchingusers = 0;
static 	int	newServerInfo = false;

typedef struct
{
	bool initialized;
	int _max_servers;
	int *_num_servers;
	server_info_t *serverlist;
} our_server_info_t;

our_server_info_t ourServerInfo;

static 	user_info_t *userlist;

//the current server
extern cvar_t server_port;
extern cvar_t server_address;

int	num_gameservers_queued = 0;
server_info_t *gameserver_queue = NULL;
int num_gameservers_allocated = 0;

struct sockaddr_in sockaddr;
bool resolved = false;

bool MasterServer_Resolve()
{
	struct hostent *server;

	server = gethostbyname(masterserver_ip.string);
	if (!server)
	{
		Console_DPrintf("Unable to resolve %s\n", masterserver_ip.string);
		return false;
	}

	sockaddr.sin_family = PF_INET;
	sockaddr.sin_port = htons((unsigned short)masterserver_port.integer);
	memcpy(&sockaddr.sin_addr, server->h_addr, server->h_length);

	Console_Printf("%s resolved to %s\n", masterserver_ip.string, inet_ntoa(*(struct in_addr *)server->h_addr));
	
	return true;
}

void	gameserver_add_request_to_queue(char *ip_address, int port)
{
	num_gameservers_queued++;
	if (num_gameservers_queued > num_gameservers_allocated)
	{
		num_gameservers_allocated += 10;
		gameserver_queue = Tag_Realloc(gameserver_queue, sizeof(server_info_t) * num_gameservers_allocated, MEM_NET);
	}
	Console_DPrintf("Queueing request to %s:%i to slot %i\n", ip_address, port, num_gameservers_queued-1);
	strncpySafe(gameserver_queue[num_gameservers_queued-1].ip_address, ip_address, 16);
	gameserver_queue[num_gameservers_queued-1].port = port;
}

void	send_gameserver_requests()
{
	static int accum_msec = 0;

	accum_msec += Host_FrameMilliseconds();
	if (accum_msec < 1000 / gamelist_requestsPerSecond.value)
		return;

	accum_msec = 0;

	if (num_gameservers_queued <= 0)
		return;

	Console_DPrintf("Sending request from slot %i to %s:%i\n", num_gameservers_queued-1, gameserver_queue[num_gameservers_queued-1].ip_address, gameserver_queue[num_gameservers_queued-1].port);

	Host_GetInfo(gameserver_queue[num_gameservers_queued-1].ip_address,
				 gameserver_queue[num_gameservers_queued-1].port, false);
	num_gameservers_queued--;
	
	if (num_gameservers_queued < num_gameservers_allocated - 10)
	{
		num_gameservers_allocated -= 10;
		gameserver_queue = Tag_Realloc(gameserver_queue, sizeof(server_info_t) * num_gameservers_allocated, MEM_NET);
	}
}

void	gameserver_queue_init()
{
	num_gameservers_allocated = 50;
	gameserver_queue = Tag_Malloc(sizeof(server_info_t) * num_gameservers_allocated, MEM_NET);
}

int 	MasterServer_TestFirewall()
{
	int buflen;
	char buf[MAX_PACKET_SIZE];
	int masterserver_firewall_sock;

	masterserver_firewall_sock = masterserverbrowser_client_connect();

	if (!masterserver_firewall_sock)
	{
		Console_Printf("Couldn't connect to tcp port %i on %i to request a firewall test\n", masterserver_port.integer, masterserver_ip.integer);
		return -1;
	}

	set_nonblocking(masterserver_firewall_sock);
	
	Console_Printf("Requesting a firewall test\n");
	
	snprintf(buf, MAX_PACKET_SIZE, "FIREWALL_TEST %i", localServer.port);
	buflen  = strlen(buf);

	TCP_Write(masterserver_firewall_sock, buf, buflen);
	TCP_Close(masterserver_firewall_sock);
	
	return true;
} 

void	MasterServer_GetLanGames()
{
	Pkt_Clear(&ncLocalBrowser.send);
	Pkt_WriteCmd(&ncLocalBrowser.send, CPROTO_INFO_REQUEST);
	Pkt_WriteInt(&ncLocalBrowser.send, System_Milliseconds());
	Net_BroadcastPacket(&ncLocalBrowser, 11235);
	Net_BroadcastPacket(&ncLocalBrowser, 11236);
	Net_BroadcastPacket(&ncLocalBrowser, 11237);
	Net_BroadcastPacket(&ncLocalBrowser, 11238);
	Pkt_Clear(&ncLocalBrowser.send);
}

int 	MasterServer_GetGameList()
{
	static int lastCall = -1;
/*
	if (Host_Milliseconds() - lastCall < 1000)
	{
		newServerInfo = true;
		return 0;
	}
	lastCall = Host_Milliseconds();*/

	MasterServer_GetLanGames();
	
	num_gameservers_queued = 0;
	waiting_for_gamelist = true;
	newServerInfo = true;
	*ourServerInfo._num_servers = 0;
	return *ourServerInfo._num_servers;
} 

int		MasterServer_GetUserInfo(char *cookie, char *server, int port)
{
	int buflen;
	char buf[MAX_PACKET_SIZE];

	masterserver_userinfo_sock = masterserverbrowser_client_connect();

	if (!masterserver_userinfo_sock)
	{
		Console_Printf("Couldn't get a valid socket\n");
		return -1;
	}

	set_nonblocking(masterserver_userinfo_sock);
	
	if (!server)
		server = server_address.string;

	if (!port)
		port = server_port.integer;
	
	snprintf(buf, MAX_PACKET_SIZE, "USERCOOKIE %s %s %i", cookie, server, port);
	buflen  = strlen(buf);

	TCP_Write(masterserver_userinfo_sock, buf, buflen);
	
	return 0;
}

// process the response we get back from the masterserver
// eventually the masterserver can send us messages, like that we need to upgrade (with a URL to the patch file), etc.
bool		process_masterserver_userinfo_response(char *buf, int len, char *info, int maxlen)
{
	packet_t pkt;
	Pkt_Import(&pkt, buf, len, true);

	Pkt_ReadString(&pkt, info, maxlen);
		
	return true;
}

//wait for a response from the server and then return true if it says we can continue
int 	listen_for_masterserver_userinfo(int *socket, char *info, int maxlen)
{
	int len;
	char buf[MAX_PACKET_SIZE];
	bool done = false;

	while (true)
	{ 
		 if ((len = TCP_Read(*socket, buf, MAX_PACKET_SIZE)) > 0) 
		 { 
			 Console_DPrintf("received %i bytes from the server\n", len);
			 done = process_masterserver_userinfo_response(buf, len, info, maxlen);
			 if (!done)
			 {
				continue;
			 }
			 else
			 {
				TCP_Close(*socket); 
				*socket = 0;
				return true;
			 }
		 }
		 if (len == 0)
			return false;
		 if (len < 0) 
		 {
#ifdef _WIN32
			 int error;
			 error = WSAGetLastError();
			 if (error != EAGAIN && error != WSAEWOULDBLOCK)
#else
			 if (errno != EAGAIN)
#endif
			 {
				 Console_DPrintf("connection closed!\n");
				 /* signal an error to the caller */ 
				 TCP_Close(*socket); 
				 *socket = 0;
				 return -1;
			 }
		 }
	 }
}

int		MasterServer_GetUserInfo_Frame(char *info, int maxlen)
{
	if (masterserver_userinfo_sock)
	{
		if (listen_for_masterserver_userinfo(&masterserver_userinfo_sock, info, maxlen))
			return true;
	}

	return false;
}

int 	MasterServer_FindUsers(char *name)
{
	int buflen;
	char buf[MAX_PACKET_SIZE];

	masterserver_userlist_sock = masterserverbrowser_client_connect();

	if (!masterserver_userlist_sock)
	{
		Console_Printf("Couldn't get a valid socket\n");
		return -1;
	}

	set_nonblocking(masterserver_userlist_sock);
	
	//we'll want to change this to VACANT_LIST eventually
	snprintf(buf, MAX_PACKET_SIZE, "USER_LIST %s", name);
	buflen  = strlen(buf);

	TCP_Write(masterserver_userlist_sock, buf, buflen);
	
	_num_matchingusers = 0;
	return _num_matchingusers;
}

// process the response we get back from the masterserver
// eventually the masterserver can send us messages, like that we need to upgrade (with a URL to the patch file), etc.
bool		process_masterserver_userlist_response(char *buf, int len, user_info_t *users, int max_users, int *num_users, int *num_new_users)
{
	packet_t pkt;
	user_info_t *user = NULL;

	*num_new_users = 0;

	Pkt_Import(&pkt, buf, len, true);

	while (!Pkt_DoneReading(&pkt)
			&& *num_users < max_users)
	{
		user = &users[*num_users];

		Console_DPrintf("num_users is %i\n", *num_users);
		
		Pkt_ReadString(&pkt, user->username, MAX_USERNAME_LENGTH);
		if (user->username[0] == 0)
		{
			_num_matchingusers = *num_users;
			return true;
		}

		user->user_id = Pkt_ReadInt(&pkt);
		Pkt_ReadString(&pkt, user->server_ip, 15);
		user->port = Pkt_ReadShort(&pkt);
		
		*num_users += 1; //*num_users++ increments the pointer!
		*num_new_users += 1;
	}
	_num_matchingusers = *num_users;
	return false;
}

// process the response we get back from the masterserver
// eventually the masterserver can send us messages, like that we need to upgrade (with a URL to the patch file), etc.
int		process_masterserver_serverlist_response(int *num_new_games)
{
	packet_t pkt;
	server_info_t server;
	char ip[16];
	file_t *f;
	char *buf;
	int size;

	*num_new_games = 0;

	f = HTTP_OpenFileNonBlocking(fmt("http://%s:%i/gamelist_full.dat", gamelist_ip.string, gamelist_port.integer));
	if (!f)
		return -1;
	
	if (!File_IsBuffered(f))
	{
		Console_Printf("we don't handle non-buffered files\n");
		File_Close(f);
		return true;
	}
	
	size = File_GetBuffer(f, &buf);
	if (size == 0)
	{
		Console_Printf("Error getting gamelist\n");
		File_Close(f);
		return true;
	}
	else if (size < HEADER_SIZE)
	{
		Console_Printf("short game list packet (%i bytes)\n", size);
		File_Close(f);
		return true;
	}
		
	Pkt_Import(&pkt, buf, size, true);
	File_Close(f);

	while (!Pkt_DoneReading(&pkt))
	{
		server.ip_address[0] = Pkt_ReadByte(&pkt);
		server.ip_address[1] = Pkt_ReadByte(&pkt);
		server.ip_address[2] = Pkt_ReadByte(&pkt);
		server.ip_address[3] = Pkt_ReadByte(&pkt);
		if (Pkt_DoneReading(&pkt))
			return true;
		server.port = Pkt_ReadShort(&pkt);

		if (server.ip_address[0] == 0
			&& server.ip_address[1] == 0
			&& server.ip_address[2] == 0
			&& server.ip_address[3] == 0
			&& server.port == 0)
		{
			return true;
		}
		
		//now copy the human-readable version into the array
		strcpy(ip, inet_ntoa(*(struct in_addr *)server.ip_address)); 
		strcpy(server.ip_address, ip);

		gameserver_add_request_to_queue(server.ip_address, server.port);
	}
	return true;
}

//wait for a response from the server and then return true if it says we can continue
int 	listen_for_masterserver_gamelist(server_info_t *servers, int max_servers, int *num_servers)
{
	int i, num_new_games = 0;
	bool done = false;
	char *ip;

	done = process_masterserver_serverlist_response(&num_new_games);
	if (done == 1)
	{
		for (i = 0; i < *num_servers; i++)
		{
			ip = inet_ntoa(*(struct in_addr *)servers[i].ip_address);
			Console_DPrintf("game #%i at %s:%i\n", //, map %s, %s, %s, gametime: %02i:%02i:%02i\n", i, 
					i,
					servers[i].ip_address,
					servers[i].port);
		}
		waiting_for_gamelist = false;
		return num_new_games; 
	}
	return done > 0;
}

//wait for a response from the server and then return true if it says we can continue
int 	listen_for_masterserver_userlist(int *socket, user_info_t *users, int max_users, int *num_users)
{
	int len, num_new_users = 0;
	char buf[MAX_PACKET_SIZE];
	bool done = false;

	while (true)
	{ 
		 if ((len = TCP_Read(*socket, buf, MAX_PACKET_SIZE)) > 0) 
		 { 
			 Console_DPrintf("received %i bytes from the server\n", len);
			 done = process_masterserver_userlist_response(buf, len, users, max_users, num_users, &num_new_users);
			 if (!done)
			 {
				continue;
			 }
			 else
			 {
				TCP_Close(*socket); 
				*socket = 0;
				return num_new_users; 
			 }
		 }
		 if (len == 0)
			return num_new_users;
		 if (len < 0) 
		 {
#ifdef _WIN32
			 int error;
			 error = WSAGetLastError();
			 if (error != EAGAIN && error != WSAEWOULDBLOCK)
#else
			 if (errno != EAGAIN)
#endif
			 {
				 Console_DPrintf("connection closed!\n");
				 /* signal an error to the caller */ 
				 TCP_Close(*socket); 
				 *socket = 0;
				 return num_new_users; 
			 }
		 }
	 }
}

int		masterserverbrowser_client_connect()
{
	int s;

	if (!resolved || masterserver_ip.modified || masterserver_port.modified)
	{
		masterserver_ip.modified = false;
		masterserver_port.modified = false;
		resolved = MasterServer_Resolve(); //give it another shot?
		if (!resolved)
			Console_Errorf("Can't resolve %s\n", masterserver_ip.string);
	}
	
	s = TCP_ConnectToSockaddr(sockaddr);
	return s;
}

int		MasterServer_GetGameList_Frame(server_info_t *servers, int max_servers, int *num_servers)
{
	int num_new_games = 0;

	ourServerInfo.initialized = true;
	ourServerInfo.serverlist = servers;
	ourServerInfo._num_servers = num_servers;
	ourServerInfo._max_servers = max_servers;

	if (waiting_for_gamelist)
		num_new_games = listen_for_masterserver_gamelist(servers, max_servers, num_servers);
	//*num_servers = _num_servers;
	
	if (newServerInfo)
	{
		Console_DPrintf("setting num_new_games to %i because we heard from game servers\n", newServerInfo);
		num_new_games += newServerInfo;
	}
	send_gameserver_requests();
	newServerInfo = false;

	return num_new_games;
}

int		MasterServer_GetUserList_Frame(user_info_t *users, int max_users, int *num_users)
{
	int num_new_users = 0;

	userlist = users;
	if (masterserver_userlist_sock)
		num_new_users = listen_for_masterserver_userlist(&masterserver_userlist_sock, users, max_users, num_users);
	//*num_servers = _num_servers;

	return num_new_users;
}

void	MasterServer_EraseGameInfo(char *address, int port)
{
	int i = 0;

	if (!ourServerInfo.initialized)
		return;
	
	while (ourServerInfo._num_servers && i < *ourServerInfo._num_servers)
	{
		if (strcmp(ourServerInfo.serverlist[i].ip_address, address) == 0
			&& ourServerInfo.serverlist[i].port == port)
		{
			Mem_Move(&ourServerInfo.serverlist[i], &ourServerInfo.serverlist[i+1], (*ourServerInfo._num_servers - i - 1) * sizeof(server_info_t));
			(*ourServerInfo._num_servers) = (*ourServerInfo._num_servers) - 1;

			newServerInfo = true;
		}
		i++;
	}
}

void	MasterServer_SetGameInfo(char *address, int port, int ping, const char *coreInfo, const char *gameInfo)
{
	server_info_t *server = NULL;
	int i = 0;

	if (!ourServerInfo.initialized)
		return;
	
	while (ourServerInfo._num_servers && i < *ourServerInfo._num_servers)
	{
		server = &ourServerInfo.serverlist[i];
		if (strcmp(server->ip_address, address) == 0 
			&& port == server->port)
		{
			strcpy(server->coreInfo, coreInfo);
			strcpy(server->gameInfo, gameInfo);
			server->ping = ping;
			return;
		}
		i++;
	}

	if (*ourServerInfo._num_servers >= ourServerInfo._max_servers)
	{
		Console_DPrintf("warning: Server list overflow - not adding server info for server %s:%i\n", address, port);
		return;
	}
	
	server = &ourServerInfo.serverlist[*ourServerInfo._num_servers];
	strcpy(server->ip_address, address);
	server->port = port;
	strcpy(server->coreInfo, coreInfo);
	strcpy(server->gameInfo, gameInfo);
	server->ping = ping;

	(*ourServerInfo._num_servers) += 1;
	
	newServerInfo = true;
}

bool	MasterServer_GetGameInfo(char *address, int port, int *ping, char **coreInfo, char **gameInfo)
{
	server_info_t *server = NULL;
	int i = 0;

	if (!ourServerInfo.initialized)
		return false;
	
	while (ourServerInfo._num_servers && i < *ourServerInfo._num_servers)
	{
		server = &ourServerInfo.serverlist[i];
		if (strcmp(server->ip_address, address) == 0 
			&& port == server->port)
		{
			*coreInfo = server->coreInfo;
			*gameInfo = server->gameInfo;
			*ping = server->ping;
			return true;
		}
		i++;
	}
	return false;
}

int	MasterServer_Init()
{
		
	Cvar_Register(&masterserver_ip);
	Cvar_Register(&masterserver_port);
	Cvar_Register(&gamelist_ip);
	Cvar_Register(&gamelist_port);
	Cvar_Register(&gamelist_requestsPerSecond);

	gameserver_queue_init();

	resolved = MasterServer_Resolve();

	ourServerInfo.initialized = false;
	
	return true;
}
