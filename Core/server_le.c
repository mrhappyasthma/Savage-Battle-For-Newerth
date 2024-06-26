// (C) 2001 S2 Games

// server_le.c

// server network code

// GAME UPDATE MECHANISM:
//
// every server frame, we update the client with recently changed state strings,
// update their playerstate  and object data, and send any other misc messages
//
// playerstate/object sending is only done if the client has not exceeded his bandwidth limit
//
// the data that goes into the ps/object update packet is determined by the current state of all
// game objects (localServer.gameobjs[]), delta compressed from a previous frame.
//
// the previous frame we are delta compressing from is determined by a few factors.
// one, if the client has not said otherwise, the server will compare against the previous server frame.
// two, if the client received an out of order frame, or missed a frame, it will request an update from a previously known good frame.
// three, if the client's history of updates does not include the frame the server delta'd from, it will request an update from a previous known good frame
// 
// the client requesting a delta frame other than the previous server frame is something that is cleared
// every frame on the server.  when the request has been fullfilled, on the next frame the server will
// go back to updating from (localServer.framenum - 1)
//
// sometimes, a ps/object update won't fit into one packet.  if that happens, it is indicated at the end
// of the packet that the update is not complete, and the server will wait until the next frame (and
// (until Server_BandwidthOK() returns true) to send the rest of the update.  the client stores
// the partial update in a buffer for later reconstruction.


#include "core.h"
#include "zlib.h"
#include "bans.h"
#include "../keyserver/auth_common.h"


//#define NET_NO_DELTA_SEND

#define MAX_MTU_SIZE 1500

#define	MAX_PORT_SEARCH	20

cvar_t	svr_maxbps =			{ "svr_maxbps",	"20000", CVAR_SAVECONFIG };	//max allowable bytes per second per client
cvar_t	svr_debug =				{ "svr_debugMsgs",				"0" };
cvar_t	svr_debugUpdate =				{ "svr_debugUpdate",			"0" };
cvar_t	svr_dedicated =			{ "svr_dedicated",			"0", CVAR_SERVERINFO | CVAR_READONLY };
cvar_t	svr_gamefps =			{ "svr_gamefps",			"15", CVAR_SERVERINFO | CVAR_VALUERANGE | CVAR_SAVECONFIG, 1, SERVER_MAX_FPS };
cvar_t	svr_port =				{ "svr_port",				DEFAULT_SERVER_PORT_ASCII, CVAR_SAVECONFIG };
cvar_t	svr_maxclients =		{ "svr_maxclients",			"10", CVAR_SAVECONFIG | CVAR_VALUERANGE | CVAR_SERVERINFO, 1, MAX_CLIENTS };
cvar_t	svr_dump =				{ "svr_dump",				"0" };
cvar_t	svr_pure =				{ "svr_pure",				"0", CVAR_SERVERINFO | CVAR_SAVECONFIG };
cvar_t	svr_mapurl = 			{ "svr_mapurl", 			"", CVAR_SERVERINFO | CVAR_SAVECONFIG };
cvar_t	svr_requiredArchives = 	{ "svr_requiredArchives",	"", CVAR_SERVERINFO | CVAR_SAVECONFIG };
cvar_t	svr_allowRemoteSvcmds = { "svr_allowRemoteSvcmds",  "0", CVAR_SAVECONFIG };
#ifdef SAVAGE_DEMO
cvar_t	svr_demo =				{ "svr_demo",				"1", CVAR_READONLY | CVAR_SERVERINFO };
#else
cvar_t	svr_demo =				{ "svr_demo",				"0", CVAR_READONLY | CVAR_SERVERINFO };
#endif
cvar_t	svr_exitOnInvalidObject = { "svr_exitOnInvalidObject", "1" };
cvar_t	svr_clientTimeout =		{ "svr_clientTimeout", "400" };		//specified in frames, not seconds!
cvar_t	svr_clientConnectionTimeout = { "svr_clientConnectionTimeout", "1350" }; //90 seconds, very generous
#ifdef SAVAGE_DEMO
cvar_t	svr_name =				{ "svr_name", "Unnamed Savage DEMO Server", CVAR_SERVERINFO | CVAR_SAVECONFIG };
#else	//SAVAGE_DEMO
cvar_t	svr_name =				{ "svr_name", "Unnamed Savage Server", CVAR_SERVERINFO | CVAR_SAVECONFIG };
#endif	//SAVAGE_DEMO
cvar_t	svr_password =			{ "svr_password", "", CVAR_SAVECONFIG };
cvar_t	svr_adminPassword =		{ "svr_adminPassword", "", CVAR_SAVECONFIG };
cvar_t	svr_firewalled =		{ "svr_firewalled", "1", CVAR_READONLY | CVAR_SERVERINFO };
cvar_t	svr_broadcast =			{ "svr_broadcast", "1", CVAR_SAVECONFIG };		//set to 0 to not broadcast the game to the master server
cvar_t	svr_world =				{ "svr_world", "", CVAR_SERVERINFO | CVAR_READONLY };
cvar_t	svr_sendStats =			{ "svr_sendStats", "1", CVAR_SERVERINFO | CVAR_SAVECONFIG};
cvar_t	svr_defaultBanTime =	{ "svr_defaultBanTime", "300000"};
cvar_t	svr_debugInputs =		{ "svr_debugInputs", "0" };

//cvar_t	heartbeatserver_ip =	{ "heartbeatserver_ip", "66.205.194.48" };
cvar_t	heartbeatserver_ip =	{ "heartbeatserver_ip", "masterserver.savage.s2games.com" };
cvar_t	heartbeatserver_port =	{ "heartbeatserver_port", "11236" };

static int server_timestamp = 0;
static int server_milliseconds = 0;

static inputState_t nullinput;
static baseObject_t nullobj;
static playerState_t nullplayerstate;

extern cvar_t default_world;

extern cvar_t net_forceIP;

#define	FOR_ALL_CLIENTS \
{ \
	int _clientloop; \
	for (_clientloop = 0; _clientloop < MAX_CLIENTS; _clientloop++) \
	{ \
		if (!localServer.clients[_clientloop].active) \
			continue;
		
#define END_CLIENT_LOOP \
	} \
}

#define	CLIENT (localServer.clients[_clientloop])
#define CLIENT_NUM (_clientloop)

server_t localServer;

bool	Server_AddClient(netconnection_t *net, int clientid, bool restarting, int forceindex, bool demoPlayer, clientConnectionInfo_t *oldClient);
void	Server_AddStringToQueue(int clientnum, int id);
void	Server_UpdateStateStrings(bool first);
bool	Server_ReadPackets();
void 	NavRep_Update();

//brute-force count
void	Server_CountClients()
{
	int n;

	localServer.numClients = 0;

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (localServer.clients[n].active)
			localServer.numClients++;
	}
}


int	Server_GetNumClients()
{
	return localServer.numClients;
}

bool	Server_IsClientActive(int clientnum)
{
	if (clientnum < MAX_CLIENTS && clientnum >= 0)
	{
		return localServer.clients[clientnum].active;
	}

	return false;
}



/*==========================

  Server_TalkToMaster

  should we talk to the master server?

 ==========================*/

bool	Server_TalkToMaster()
{
	if (svr_broadcast.integer == 2)		//force broadcasting
		return true;

	if (svr_firewalled.integer)			//can't get incoming connections
		return false;
	if (!svr_broadcast.integer)			//explicitly not broadcasting
		return false;

	return true;
}


void	Server_DisconnectClient(int clientnum, const char *reason)
{		
	if (localServer.clients[clientnum].cstate == CCS_IN_GAME)
		sv_api.ClientDisconnect(clientnum, reason);

	if (Server_TalkToMaster())
	{
		Pkt_Clear(&ncMasterServer.send);

		Pkt_WriteCmd(&ncMasterServer.send, CPROTO_PLAYER_DISCONNECT);
		Pkt_WriteByte(&ncMasterServer.send, 0);  //version 0
		Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[0]);
		Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[1]);
		Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[2]);
		Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[3]);
		Pkt_WriteShort(&ncMasterServer.send, (short)Server_GetPort());
		Pkt_WriteString(&ncMasterServer.send, localServer.clients[clientnum].net.cookie);
		
		Net_SendPacket(&ncMasterServer);
	}

	Net_ClearReliablePackets(&localServer.clients[clientnum].net);

	Pkt_Clear(&localServer.clients[clientnum].net.send);
	Pkt_Clear(&localServer.clients[clientnum].net.reliableSend);
 
	Pkt_WriteCmd(&localServer.clients[clientnum].net.send, SNET_KICK);
	Pkt_WriteString(&localServer.clients[clientnum].net.send, (char *)reason);
	Net_SendPacket(&localServer.clients[clientnum].net);

	localServer.clients[clientnum].active = false;
	localServer.clients[clientnum].cstate = CCS_DISCONNECTED;

	//remove bigstring allocation, if any
	if (localServer.clients[clientnum].bigstring)
		Tag_Free(localServer.clients[clientnum].bigstring);	

	Server_CountClients();	

	if (localClient.cstate > CCS_DISCONNECTED && clientnum == 0)
	{
		//full disconnect
		Game_Error(fmt("Local Disconnect! (%s)", reason));
	}

	Console_Printf("Client %s (num=%i, reqname=%s) disconnected (%s)\n", localServer.clients[clientnum].net.recvAddrName, clientnum, localServer.clients[clientnum].requestedName, reason);
}


void	Server_SetStatus(int status)
{
	localServer.status = status;
}

int		Server_GetStatus()
{
	return localServer.status;
}

void	Server_ClientList_Cmd(int argc, char *argv[])
{
	int n;
	int num = -1;

	if (argc)
		num = atoi(argv[0]);

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (num > -1 && n != num)
			continue;

		if (localServer.clients[n].active)
		{
			Console_Printf("Client %i:\n  addr = %s (id %i), reqname = %s, cstate = %i, maxp = %i\n",
							n,
							localServer.clients[n].net.recvAddrName,
							localServer.clients[n].clientid,
							localServer.clients[n].requestedName,
							localServer.clients[n].cstate,
							localServer.clients[n].maxPacketSize
							);
		}
	}
}


/*==========================

  Server_Kick_Cmd

  Kick a client by their client number

 ==========================*/

void	Server_Kick_Cmd(int argc, char *argv[])
{
	int clientnum;
	char *reason;

	if (!argc)
		return;

	clientnum = atoi(argv[0]);
	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return;

	if (!argv[1])
		reason = "No reason given";
	else
		reason = argv[1];

	if (localServer.clients[clientnum].active)
	{
		if (localServer.clients[clientnum].net.guid)
			Bans_Add(fmt("%i", localServer.clients[clientnum].net.guid), svr_defaultBanTime.integer);
		Bans_Add(localServer.clients[clientnum].net.recvAddrName, svr_defaultBanTime.integer);
		Server_DisconnectClient(clientnum, fmt("Kicked from server (%s)", reason));
	}
}

void	Server_BanClient_Cmd(int argc, char *argv[])
{
	int clientnum, time = 0;
	char *reason;

	if (!argc)
	{
		Console_Printf("No client number specified!\n");
		return;
	}

	clientnum = atoi(argv[0]);
	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return;

	if (argc > 1)
		time = atoi(argv[1]);

	if (argc <= 2)
		reason = "No reason given";
	else
		reason = argv[2];

	if (localServer.clients[clientnum].active)
	{
		if (localServer.clients[clientnum].net.guid)
			Bans_Add(fmt("%i", localServer.clients[clientnum].net.guid), time);
		Bans_Add(localServer.clients[clientnum].net.recvAddrName, time);
		Server_DisconnectClient(clientnum, fmt("Banned from server (%s)", reason));
	}
	
}

void	Server_Reset(bool softrestart, bool first)
{		
	int n;
	int oldPort = localServer.port;
	int oldMaxClients = localServer.maxClients;
	void *net_object_pool = localServer.net_object_pool;
	unsigned int pool_size = localServer.pool_size;

	//free any memory used by state strings
	for (n=0; n<MAX_STATE_STRINGS; n++)
	{
		if (localServer.stateStrings[n].string)
			Tag_Free(localServer.stateStrings[n].string);
	}
	
	//clear everything
	memset(&localServer, 0, sizeof(server_t));
	
	if (!first)
	{
		//remove static surfaces that were allocated during the last game
		for (n=0; n<MAX_OBJECTS; n++)
		{
			WO_DeleteObject(n);
		}
	}

	//clear dynamic objects
	WT_ResetDynamic(false);
	//initialize object grid
	WOG_Reset(&localServer.grid, localServer.gameobjs);
	//initialize path
	Cmd_Exec("navrep_process");

	localServer.port = oldPort;
	localServer.net_object_pool = net_object_pool;
	localServer.pool_size = pool_size;
	localServer.maxClients = oldMaxClients;

	Server_SetStatus(SVS_STARTED);

	//build special state strings
	Server_UpdateStateStrings(true);

	localServer.active = true;

	if (dedicated_server.integer)
	{
		Cvar_Set("svr_dedicated", "1");
		localServer.dedicated = true;
	}
	else
	{
		Cvar_Set("svr_dedicated", "0");		
	}

	//reset server game code
	sv_api.Reset(first);
}

void	Server_ResetGame_Cmd(int argc, char *argv[])
{
	//restart the game without reloading the map and skipping some other init functions

	if (localServer.status != SVS_STARTED)
		return;

	Server_Start(world.name, true);
}

void	Server_RegisterVars()
{
	Cvar_Register(&svr_maxbps);
	Cvar_Register(&svr_name);
	Cvar_Register(&svr_debug);
	Cvar_Register(&svr_debugUpdate);
	Cvar_Register(&svr_gamefps);
	Cvar_Register(&svr_dedicated);
	Cvar_Register(&svr_port);
	Cvar_Register(&svr_maxclients);
	Cvar_Register(&svr_dump);
	Cvar_Register(&svr_mapurl);
	Cvar_Register(&svr_requiredArchives);
	Cvar_Register(&svr_pure);
	Cvar_Register(&svr_demo);
	Cvar_Register(&svr_dump);	
	Cvar_Register(&svr_clientTimeout);
	Cvar_Register(&svr_clientConnectionTimeout);
	Cvar_Register(&svr_password);
	Cvar_Register(&svr_firewalled);
	Cvar_Register(&svr_broadcast);
	Cvar_Register(&svr_world);
	Cvar_Register(&svr_adminPassword);
	Cvar_Register(&svr_sendStats);
	Cvar_Register(&svr_defaultBanTime);
	Cvar_Register(&svr_allowRemoteSvcmds);
	Cvar_Register(&svr_debugInputs);


	Cmd_Register("clientlist",	Server_ClientList_Cmd);
	Cmd_Register("kick",		Server_Kick_Cmd);
	Cmd_Register("kicknum",		Server_Kick_Cmd);
	Cmd_Register("resetgame",	Server_ResetGame_Cmd);
	Cmd_Register("banclient",	Server_BanClient_Cmd);
}

void	Server_Init()
{
	if (DLLTYPE == DLLTYPE_EDITOR)
		return;

	Cvar_Register(&heartbeatserver_ip);
	Cvar_Register(&heartbeatserver_port);
	
	memset(&localServer, 0, sizeof(server_t));
	memset(&nullobj, 0, sizeof(nullobj));
	memset(&nullplayerstate, 0, sizeof(nullplayerstate));
	memset(&nullinput, 0, sizeof(nullinput));

	Net_Server_Init();
	
	Net_SetSendAddr(&ncMasterServer, heartbeatserver_ip.string, heartbeatserver_port.integer);

//	sv_api.Init();
}




/*==========================

  Server_NotFirewalled

  Allow anyone to connect to this server (no authentication required)
  This will always get set for LAN games who don't have a direct connection to the internet

 ==========================*/

void	Server_NotFirewalled()
{
	Console_Printf("Got firewall response\n");
	Cvar_SetVarValue(&svr_firewalled, 0);

	//send the initial heartbeat
	Server_SendHeartbeat();
}

void	Server_BroadcastCvar(cvar_t *var)
{
	FOR_ALL_CLIENTS	
		Pkt_WriteCmd(&CLIENT.net.reliableSend, SNET_CVAR);
		Pkt_WriteString(&CLIENT.net.reliableSend, var->name);
		Pkt_WriteString(&CLIENT.net.reliableSend, var->string);
	END_CLIENT_LOOP
}


/*==========================

  Server_Disconnect

  notify all clients that we're disconnecting and deactivate the server

 ==========================*/

void	Server_Disconnect()
{
	if (!localServer.active)
		return;

	sv_api.Shutdown();

	Server_SendShutdown();

	localServer.active = false;

	FOR_ALL_CLIENTS
		Pkt_Clear(&CLIENT.net.send);
		Pkt_WriteCmd(&CLIENT.net.send, SNET_DISCONNECTING);
		Net_SendPacket(&CLIENT.net);
	END_CLIENT_LOOP
	/*FOR_ALL_CLIENTS
		Server_DisconnectClient(CLIENT_NUM);
	END_CLIENT_LOOP
	*/

/*
	flushtime = System_GetTime();

	//give us a couple of seconds to flush any incoming data
	while (System_GetTime() - flushtime < 2)
	{
//			if (!localServer.dedicated)
//				while (Net_ReceivePacket(&ncLocalClient) > 0)
//					Net_PreProcessPacket(&ncLocalClient);

		while (Server_ReadPackets());

//		if (localClient.cstate)
//			Client_Frame();

	}
*/

	localServer.status = SVS_DISCONNECTED;

	Net_CloseCookieSocket();

	World_Destroy();

	//free all memory allocated by the server
	Tag_FreeAll(MEM_SERVER);
	memset(&localServer, 0, sizeof(localServer));

	Console_Printf("****************\nSERVER SHUT DOWN\n****************\n");

	Net_CloseNetConnection(&ncListen);
}



clientConnectionInfo_t	previousClients[MAX_CLIENTS];

void	Server_SaveClients()
{
	int n;

	//we're transitioning all the clients over to the next map
	//or a restarted match

	memset(previousClients, 0, sizeof(previousClients));

	for (n=0; n<MAX_CLIENTS; n++)
	{
		clientConnectionInfo_t *client = &localServer.clients[n];

		if (!client->active)
			continue;
		if (client->cstate == CCS_VIRTUAL)		//don't keep virtual clients
			continue;

		//clear all state string info, because state strings get cleared on a restart

		if (client->bigstring)
			Tag_Free(client->bigstring);
		client->bigstringIndex = 0;
		client->bigstringLength = 0;
		client->bigstring = NULL;
		client->stateStringQueueSize = 0;

		previousClients[n] = localServer.clients[n];
	}
}

void	Server_SendWorldData(int clientnum);

void	Server_RestartClients(bool softrestart)
{
	int n;	

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (previousClients[n].active)
		{
			Server_AddClient(&previousClients[n].net, previousClients[n].clientid, true, n, previousClients[n].demoPlayer, &previousClients[n]);
		}
		else
		{
			memset(&localServer.clients[n], 0, sizeof(clientConnectionInfo_t));
		}
	}	
}

#ifdef SAVAGE_DEMO
bool	Server_IsDemoPlayer(int clientnum)
{
	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return false;
	if (!localServer.clients[clientnum].active)
		return false;

	return localServer.clients[clientnum].demoPlayer;
}
#endif


/*==========================

  Server_Start

  

 ==========================*/

void	Server_Start(const char *wName, bool softrestart)
{
	bool first = true;	
	char *ip = NULL;

	Cvar_SetVar(&svr_world, wName);	
	
	if (localServer.active)
	{
		first = false;

		//notify all clients that we're restarting
		//note: we shouldn't have to worry about this reliable packet getting lost,
		//since we restore their netConnection in Server_AddClient()
		FOR_ALL_CLIENTS
		{			
			Pkt_Clear(&CLIENT.net.reliableSend);
			Pkt_Clear(&CLIENT.net.send);

			Pkt_WriteCmd(&CLIENT.net.reliableSend, SNET_MESSAGE);
			Pkt_WriteByte(&CLIENT.net.reliableSend, 0);
			Pkt_WriteByte(&CLIENT.net.reliableSend, -1);
			Pkt_WriteString(&CLIENT.net.reliableSend, softrestart ? "softrestart" : "fullrestart");

			Net_SendPacket(&CLIENT.net);
		}
		END_CLIENT_LOOP

		sv_api.Shutdown();
	
		//save the information about all clients who were connected to the previous game
		Server_SaveClients();
		localServer.active = false;		
	}

	if (svr_requiredArchives.string[0])
	{
		char *tmp, *slash, *end;
		char currentLocalFilename[512] = {0};
		
		tmp = svr_requiredArchives.string;
		while (tmp)
		{
			slash = strchr(tmp, '/');
			end = strchr(tmp, ' ');
			if (!end)
				end = tmp + strlen(tmp);
	
			//rid ourselves of subdirs
			if (slash && slash < end)
				tmp = slash;
			
			strncpySafe(currentLocalFilename, fmt("/%s",tmp), MIN(512, end - tmp + 2));
		
			if (!strstr(currentLocalFilename, ".s2z")
				|| Archive_IsOfficialArchiveName(currentLocalFilename))
			{
				Console_Printf("Invalid server requiredArchive %s\n", currentLocalFilename);
			}
			else
			{
				//mark it as official so it gets used in the hash compare with the client
				if (!Archive_RegisterArchive(currentLocalFilename, ARCHIVE_NORMAL | ARCHIVE_OFFICIAL))
				{
					Console_Printf("Failed to load archive %s\n", currentLocalFilename);
				}
			}

			if (end && end[0])
				tmp = end + 1;
			else
				break;
		}
	}
	
	server_milliseconds = 1;		//restart the game time
	localServer.framenum = 1;		//reset the frame counter

	if (!softrestart)
	{	
		if (World_Load(svr_world.string, false))
		{
			Console_Printf("Loaded world \"%s\"\n", svr_world.string);
			Console_Printf("Server started\n");
		}
		else
		{
		/*  if (dedicated_server.integer && !World_Load(default_world.string, false))
				return;
			*/
			return;
		}

		sv_api.Init();
	}
							
	Server_Reset(softrestart, first);               //clear leftover server data

	if (first)		//i.e. not restarting
	{
		int n;

		//next Server_Frame() will allocate the memory we need to store client histories and set maxclients

		for (n=0; n<MAX_PORT_SEARCH; n++)
		{
			//search for a free listen port
			if (!Net_InitNetConnection(&ncListen, 0, svr_port.integer + n))
				continue;
			
			localServer.port = svr_port.integer + n;
			break;
		}

		if (n == MAX_PORT_SEARCH)
		{
			localServer.active = false;
			Game_Error("Couldn't start network\n");
			return;
		}

		if (Net_Server_Listen_For_Cookies() <= 0)
		{
			Console_Printf("error trying to listen on tcp/ip port\n");
		}

		//see if we are firewalled, will call SVR_NotFirewalled() if we aren't
		Cvar_SetVarValue(&svr_firewalled, 1);

		//localServer.num_net_objects = MAX_OBJECTS * 2;
		
		//localServer.clients = Tag_Malloc(sizeof(clientConnectionInfo_t) * MAX_CLIENTS, MEM_SERVER);
		

		if (!localServer.dedicated)
		{	
			//Host_Connect(net_localaddr);	//connect our local client to the server
			//Host_Connect(fmt("%s:%i", net_localaddr, svr_port.integer));	//connect our local client to the server
			if (strcmp(net_forceIP.string, "0.0.0.0") == 0)
				ip = "127.0.0.1";
			else
				ip = net_forceIP.string;
			Host_Connect(fmt("%s:%i", ip, localServer.port));	//connect our local client to the server
		}

		//expose games to the master browser when set to broadcast
		if (svr_broadcast.integer)
			MasterServer_TestFirewall();			
	}
	else
	{
		//reconnect all clients to the server
		//this will retain their client indexes
		Server_RestartClients(softrestart);
	}		
}


/*==========================

  Server_SendMessage

  Called from the server game code to send messages to all clients currently in the game

  ==========================*/

void	Server_SendMessage(int sender, int client, char *msg)
{
	if (Server_IsClientActive(client))
	{
		if (localServer.clients[client].cstate == CCS_IN_GAME)
		{			
			Pkt_WriteCmd(&localServer.clients[client].net.reliableSend, SNET_MESSAGE);
			Pkt_WriteByte(&localServer.clients[client].net.reliableSend, 0);
			Pkt_WriteByte(&localServer.clients[client].net.reliableSend, (byte)sender);
			Pkt_WriteString(&localServer.clients[client].net.reliableSend, msg);			
		}
	}
}

void	Server_BroadcastMessage(int sender, char *msg)
{
	FOR_ALL_CLIENTS
		Server_SendMessage(sender, CLIENT_NUM, msg);
	END_CLIENT_LOOP
}

void	Server_SendUnreliableMessage(int sender, int client, char *msg)
{
	if (!Server_IsClientActive(client))
		return;

	if (localServer.clients[client].reliableHack)
	{
		Server_SendMessage(sender, client, msg);
	}
	else
	{
		if (localServer.clients[client].cstate == CCS_IN_GAME)
		{
			Pkt_WriteCmd(&localServer.clients[client].net.send, SNET_MESSAGE);
			Pkt_WriteByte(&localServer.clients[client].net.send, 0);
			Pkt_WriteByte(&localServer.clients[client].net.send, (byte)sender);
			Pkt_WriteString(&localServer.clients[client].net.send, msg);
		}		
	}
}

/*=========================
 *
 * Stats code
 *
 *========================*/

static packet_t stats_packet;

bool    Server_StartStats(char *server_stats)
{
	Pkt_Clear(&stats_packet);
	Console_Printf("the server stats string is %s\n", server_stats);

	Net_Server_ClearStats();
	Server_WriteStatsHeader(&stats_packet);
	Pkt_WriteString(&stats_packet, server_stats);
	if (Net_Server_AddToStats(&stats_packet))
		return true;

	return false;
}

bool    Server_AddClientStats(char *cookie, char *user_stats)
{
	Pkt_Clear(&stats_packet);
	Pkt_WriteString(&stats_packet, cookie);
	Pkt_WriteString(&stats_packet, user_stats);
	if (Net_Server_AddToStats(&stats_packet))
		return true;

	return false;
}

bool    Server_SendStats()
{
	if (!Server_TalkToMaster())
		return false;

	if (!svr_sendStats.integer)
		return false;
		
	if (Net_Server_SendStats())
		return true;

	return false;
}

/*==========================

  Server_SendStateStrings

  send over all state strings that have been modified to the client reliably

  if the data won't fit in this packet, it will be queued for the next frame

 ==========================*/

void	Server_SendStateStrings(int clientnum)
{	
	int	numSent = 0;		//number of completed strings sent this frame
	int n;
	clientConnectionInfo_t *client = &localServer.clients[clientnum];
	int bytesRemaining = client->maxPacketSize - client->net.reliableSend.curpos - 10;

	if (bytesRemaining <= 0)
		return;

	if (client->bigstring)
	{
		int numbytes;
		//we're continuing a big string message
		
		Pkt_WriteCmd(&client->net.reliableSend, SNET_BIG_STRING_CONTINUE);
		Pkt_WriteShort(&client->net.reliableSend, (short)client->bigstringIndex);

		numbytes = MIN(bytesRemaining, (int)strlen(client->bigstringOffset)+1);
		Pkt_WriteInt(&client->net.reliableSend, client->bigstringOffset - client->bigstring);	//write offset
		Pkt_WriteShort(&client->net.reliableSend, (short)numbytes);
		Pkt_Write(&client->net.reliableSend, client->bigstringOffset, numbytes);			//write data
		
		if (client->bigstringOffset + numbytes > client->bigstring + client->bigstringLength)
		{
			//done sending, remove from queue and free memory			
			Tag_Free(client->bigstring);
			client->bigstringIndex = 0;
			client->bigstringLength = 0;
			client->bigstring = NULL;
			client->stateStringQueueSize--;
			numSent++;
			Mem_Move(client->stateStringQueue, &client->stateStringQueue[1], client->stateStringQueueSize * sizeof(int));
		}
		else
		{
			client->bigstringOffset += numbytes;
			return;
		}
	}

	for (n=0; n<client->stateStringQueueSize; n++)
	{
		int index = client->stateStringQueue[n];
		stateString_t *state = &localServer.stateStrings[index];

		if (client->net.reliableSend.curpos + state->length > client->maxPacketSize - 10)
		{
			//send 'big string' message
			//client will get many big string messages on an initial update, even though the strings
			//might not actually be big.  might want to improve this mechanism
			client->bigstring = Tag_Strdup(localServer.stateStrings[index].string, MEM_SERVER);
			client->bigstringOffset = client->bigstring;
			client->bigstringLength = strlen(client->bigstring);
			client->bigstringIndex = index;
			
			Pkt_WriteCmd(&client->net.reliableSend, SNET_BIG_STRING_START);
			Pkt_WriteShort(&client->net.reliableSend, (short)client->bigstringIndex);
			Pkt_WriteInt(&client->net.reliableSend, client->bigstringLength);

			break;
		}
		else
		{
			//send 'small string' message
			Pkt_WriteCmd(&client->net.reliableSend, SNET_SMALL_STRING);
			Pkt_WriteShort(&client->net.reliableSend, (short)index);
			Pkt_WriteString(&client->net.reliableSend, localServer.stateStrings[index].string);
			numSent++;
			client->stateStringQueueSize--;
		}
	}

	if (n > 0)
	{
		if (client->stateStringQueueSize < 0)		//won't ever happen unless there's dumbness going on with the above code
		{
			Game_Error("State string queue size < 0\n");
		}
		if (client->stateStringQueueSize)
			Mem_Move(client->stateStringQueue, &client->stateStringQueue[n], client->stateStringQueueSize * sizeof(int));
	}
}




/*==========================

  Server_GetFreeClientSlot

  return an empty client slot, or -1 if the server is full

 ==========================*/

int		Server_GetFreeClientSlot()
{
	int n;

	for (n=0; n<localServer.maxClients; n++)
	{
		if (!localServer.clients[n].active)
		{			
			return n;
		}
	}

	return -1;
}

bool	Server_IsClientConnected(netaddr_t *clientAddr, int clientid, int *clientnum)
{
	FOR_ALL_CLIENTS
		if (Net_IsSameIP(clientAddr, &CLIENT.net.recvAddr) && (clientid == CLIENT.clientid))
		{			
			*clientnum = CLIENT_NUM;
			return true;
		}
	END_CLIENT_LOOP	

	return false;
}



/*
bool	Server_CNET_CLEAR_UPDATE(int clientnum, packet_t *pkt)
{
	clientConnectionInfo_t *client = &localServer.clients[clientnum];

	if (client->cstate != CCS_IN_GAME)
		return false;

	Server_ClearUpdate(clientnum);

	return true;
}
*/

void	Server_SendClientList(int clientnum);

char    *Server_CheckForVIP(char *cookie)
{
	if (strncmp(cookie, SLOTHYS_COOKIE, strlen(SLOTHYS_COOKIE)) == 0)
		return "Slothy from S2 Games";
	else if (strncmp(cookie, JESSES_COOKIE, strlen(JESSES_COOKIE)) == 0)
		return "Jesse from S2 Games";
	else if (strncmp(cookie, JASONS_COOKIE, strlen(JASONS_COOKIE)) == 0)
		return "Jason from S2 Games";
	else if (strncmp(cookie, TRAVISS_COOKIE, strlen(TRAVISS_COOKIE)) == 0)
		return "Travis from S2 Games";
	else if (strncmp(cookie, TREVORS_COOKIE, strlen(TREVORS_COOKIE)) == 0)
		return "Trevor from S2 Games";
	else if (strncmp(cookie, SAMMYS_COOKIE, strlen(SAMMYS_COOKIE)) == 0)
		return "Sammy from S2 Games";
	else if (strncmp(cookie, WILLIES_COOKIE, strlen(WILLIES_COOKIE)) == 0)
		return "Willie from S2 Games";
	else if (strncmp(cookie, MARCS_COOKIE, strlen(MARCS_COOKIE)) == 0)
		return "Marc from S2 Games";
	else if (strncmp(cookie, RICKS_COOKIE, strlen(RICKS_COOKIE)) == 0)
		return "Rick from S2 Games";
	return NULL;
}


/*==========================

  Server_AddClient

  adds a client to the list of users managed by this server

  if 'restarting' is specified, we are transitioning the client from the previous map or match to the new one

  forceidx and oldClient are only looked at if 'restarting' is true

 ==========================*/

bool	Server_AddClient(netconnection_t *net, int clientid, bool restarting, int forceindex, bool demoPlayer, clientConnectionInfo_t *oldClient)
{
	int clientnum;
	clientConnectionInfo_t *client;

/*	
	if (Server_IsClientConnected(&connInfo->net.recvAddr, &clientnum))
	{
		Console_Printf("Server_AddClient(): Readding %s (dead connection assumed)\n", connInfo->net.sendAddrName);
		//call the game code disconnect function to make sure proper cleanup takes place, but don't actually
		//disconnect the client
		sv_api.ClientDisconnect(clientnum, "Restoring dead connection");
	}
	else
	{
	}
*/
	if (restarting)
	{
		//copy over the old client info
		clientnum = forceindex;
		client = &localServer.clients[clientnum];
		*client = *oldClient;
		memset(&client->curInputState, 0, sizeof(inputState_t));
	}
	else
	{
		//create a new client info
		clientnum = Server_GetFreeClientSlot();
		client = &localServer.clients[clientnum];
		memset(client, 0, sizeof(clientConnectionInfo_t));
		client->maxPacketSize = 1300;
		client->bps = 20000;
		client->netfps = 20;
	}	

	if (clientnum == -1)
	{
		//fixme: client needs to be notified that the server is full
		return false;
	}	

	Console_Printf("Client \"%s\" connected, assigned to slot %i\n", net->recvAddrName, clientnum);

	client->net = *net;
	client->clientid = clientid;

	//give this client a piece of ls.net_object_pool
	client->net_objects = &localServer.net_object_pool[clientnum * 2048];
	client->num_net_objects = 2048;

	//point the client to his exclusion list
	client->exclusionList = localServer.clientExclusionLists[clientnum];

	client->lastMsgFrame = localServer.framenum;
	client->active = true;
	client->restarting = restarting;
	client->nextUpdateTime = Host_Milliseconds();
	client->demoPlayer = demoPlayer;

	Server_CountClients();	
	
	client->just_connected = true;	

	client->cstate = CCS_CONNECTING;		//will remain in this state until we receive a CNET_REQUEST_STATE_STRINGS message
	
	return true;
}

/*==========================

  Server_AllocateVirtualClientSlot

 ===========================*/

int	Server_AllocVirtualClientSlot()
{
	clientConnectionInfo_t *client;

	int clientnum = Server_GetFreeClientSlot();
	if (clientnum == -1)
		return -1;

	client = &localServer.clients[clientnum];

	memset(client, 0, sizeof(clientConnectionInfo_t));	
	client->cstate = CCS_VIRTUAL;
	client->active = true;

	return clientnum;
}


/*==========================

  Server_SendDenyPacket

  Deny a client connection

 ==========================*/

void	Server_SendDenyPacket(char *reason)
{
	Pkt_Clear(&ncListen.send);
	Pkt_WriteCmd(&ncListen.send, CPROTO_NO);
	Pkt_WriteString(&ncListen.send, reason);
	Net_SendPacket(&ncListen);
}

/*==========================

  Server_CNET_KEEPALIVE

  this function is a NOP, but receiving it causes lastMsgFrame to get set for this client

 ==========================*/

bool	Server_CNET_KEEPALIVE(int clientnum, packet_t *pkt)
{
if (svr_debug.integer) {
	Console_DPrintf("Client %i: CNET_KEEPALIVE\n", clientnum);
}
	return true;
}


/*==========================

  Server_SetRequestOnly

  set a state string to be request only

 ==========================*/

void	Server_SetRequestOnly(int id)
{
	if (id < 0 || id >= MAX_STATE_STRINGS)
		return;

	localServer.stateStrings[id].flags |= STATESTRING_REQUEST_ONLY;
}

extern cvar_t	con_echoClient;

void	Server_SendOutputToClient(char *s)
{
	Server_SendUnreliableMessage(-1, con_echoClient.integer, fmt("conecho %s", s));
}


/*==========================

  Server_CNET_MESSAGE

 ==========================*/

bool	Server_CNET_MESSAGE(int clientnum, packet_t *pkt)
{
	char buf[1024];

	if (svr_debug.integer)
	{
		Console_DPrintf("Client %i: CNET_MESSAGE\n", clientnum);
	}

	memset(buf, 0, sizeof(buf));

	Pkt_ReadString(pkt, buf, 1023);

	if (strncmp(buf, "svcmd ", 6)==0)
	{
		if (svr_adminPassword.string[0] && strncmp(&buf[6], svr_adminPassword.string, strlen(svr_adminPassword.string))==0)
		{
			int	prevEchoClient = con_echoClient.integer;

			//no matter what con_echoClient is set to, we want to send the output from this command to the client that issued it
			Cvar_SetVarValue(&con_echoClient, clientnum);
			Cmd_Exec(fmt("%s", &buf[7 + strlen(svr_adminPassword.string)]));

			//the status message that the client executed the command should only go to the server, though
			Cvar_SetVarValue(&con_echoClient, -1);
			Console_Printf("Client %i executed svcmd %s\n", clientnum, &buf[7 + strlen(svr_adminPassword.string)]);

			//resume echoing to whichever client was previously receiving the console output
			Cvar_SetVarValue(&con_echoClient, prevEchoClient);
		}
		else
		{
			Console_Printf("svcmd attempted and failed by client %i\n", clientnum);
		}

		return true;
	}
	else if (strncmp(buf, "ssrq ", 5)==0)
	{
		//state string request (valid only for state strings with STATESTRING_REQUEST_ONLY flag set
		const char *s = GetNextWord(buf);
		int start = atoi(s);
		int num;
		int n;
		s = GetNextWord(s);
		num = atoi(s);
		

		if (start < 0 || start >= MAX_STATE_STRINGS)
			return false;
		if (num <= 0)
			return false;
		
		for (n=start; n<start+num; n++)
		{
			if (n>=MAX_STATE_STRINGS)
				break;

			if (localServer.stateStrings[n].flags & STATESTRING_REQUEST_ONLY)
				Server_AddStringToQueue(clientnum, n);
		}
	}

	//don't allow them to send game messages until they're actually in the game
	if (localServer.clients[clientnum].cstate == CCS_IN_GAME)
	{
		if (sv_api.ClientMessage(clientnum, buf))
			return true;
	}

	Console_DPrintf("Unknown message from client %i: %s\n", clientnum, buf);

	return true;
}


/*==========================

  Server_CNET_INPUTSTATE

 ==========================*/

bool	Server_CNET_INPUTSTATE(int clientnum, packet_t *pkt)
{
	int numInputs,n;
	int prev_old_diff;
//	int now;
	clientConnectionInfo_t *client = &localServer.clients[clientnum];
//	byte deltatime;

if (svr_debug.integer) {
	Console_DPrintf("Client %i: CNET_INPUTSTATE\n", clientnum);
}

	prev_old_diff = client->old_diff_frame;

	client->old_diff_frame = Pkt_ReadInt(pkt);

	numInputs = Pkt_ReadByte(pkt);

	memset(&client->curInputState, 0, sizeof(client->curInputState));
	client->curInputState.sequence = Pkt_ReadInt(pkt);
	client->curInputState.gametime = Pkt_ReadInt(pkt);

	for (n=0; n<numInputs; n++)
	{
		Net_ReadStructDiff(pkt, &client->curInputState, inputStateDesc);

		if (svr_debugInputs.integer)
		{
			Console_Printf("Client %i: pitch: %i  yaw: %i  mvmt: %i  gametime: %i  delta: %i\n", 
						clientnum, 
						client->curInputState.pitch,
						client->curInputState.yaw,
						client->curInputState.movement,
						client->curInputState.gametime,
						client->curInputState.delta_msec);
		}

		client->inputStateNum++;		
		if (client->inputStateNum == MAX_CLIENT_INPUTSTATES)
		{
			//Console_Printf("Got too many inputstates from client %i\n", clientnum);
			//read the rest of the packet even though we won't use it
		}
		else if (client->inputStateNum < MAX_CLIENT_INPUTSTATES)
		{
			sv_api.ProcessClientInput(clientnum, client->curInputState);
		}

		//derive sequence and gametime from existing data so we don't waste bandwidth
		client->curInputState.sequence++;
		client->curInputState.gametime += client->curInputState.delta_msec;
	}

	

	//add to ping counts
	if (client->old_diff_frame != prev_old_diff && client->old_diff_frame)	//client just acknowledged a new frame
	{
		networkUpdate_t *prevoldupd = &client->updates[prev_old_diff % MAX_SV_UPDATE_HISTORY];
		if (prevoldupd->framenum == prev_old_diff)
		{
			client->pingCounts[client->old_diff_frame % NUM_PING_COUNTS] = 
				server_milliseconds - client->updates[client->old_diff_frame % MAX_SV_UPDATE_HISTORY].time;
		}
		else
		{
			//old update fell out of buffer
			client->pingCounts[client->old_diff_frame % NUM_PING_COUNTS] = 999;
		}
	}



//	Console_DPrintf("recv input: %i\n", in.gametime);	

	return true;
}


const char*	Server_GetClientIP(int clientnum)
{
	if (clientnum < 0 || clientnum > localServer.maxClients)
		return "";

	if (!localServer.clients[clientnum].active)
		return "";

	return localServer.clients[clientnum].net.recvAddrName;
}

/*==========================

  Server_GetClientPing

  called by the game code to get a client's current ping

 ==========================*/

int	Server_GetClientPing(int clientnum)
{
	int n;
	int pingsum = 0;

	if (clientnum < 0 || clientnum > localServer.maxClients)
		return 0;

	if (!localServer.clients[clientnum].active)
		return 0;

	for (n=0; n<NUM_PING_COUNTS; n++)
	{
		pingsum += localServer.clients[clientnum].pingCounts[n];
	}

	return MIN(pingsum / NUM_PING_COUNTS, 999);
}

/*==========================

  Server_CNET_DISCONNECT

 ==========================*/

bool	Server_CNET_DISCONNECT(int clientnum, packet_t *pkt)
{
if (svr_debug.integer) {
	Console_DPrintf("Client %i: CNET_DISCONNECT\n", clientnum);
}

	Server_DisconnectClient(clientnum, "");

	return true;
}



/*==========================

  Server_CNET_OK_TO_START

 ==========================*/

bool	Server_CNET_OK_TO_START(int clientnum, packet_t *pkt)
{
	char hashString[4096];
	unsigned char world_s2z_hash[MAX_HASH_SIZE+1];
	int world_s2z_hashlen;
	clientConnectionInfo_t *client = &localServer.clients[clientnum];
	
	if (svr_debug.integer) {
		Console_DPrintf("Client %i: CNET_OK_TO_START\n", clientnum);
	}

	if (localServer.clients[clientnum].cstate != CCS_LOADING)
		return false;
	
	world_s2z_hashlen = Pkt_ReadArray(pkt, world_s2z_hash, MAX_HASH_SIZE);

	if (!World_CheckHashes(world_s2z_hashlen, world_s2z_hash))
	{		
		Server_DisconnectClient(clientnum, "World file does not match server");
		return true;
	}

	Pkt_ReadString(&client->net.recv, hashString, 4096);
	
	if (!File_CompareHashStrings(hashString, client->xorbits))
	{
		if (svr_pure.integer)
		{
			Server_DisconnectClient(clientnum, "This is a Pure Server and your files don't match the server.");
			return false;
		}
	}

	//start sending the client game data
	localServer.clients[clientnum].cstate = CCS_IN_GAME;

	Pkt_Clear(&client->net.reliableSend);	

	//make sure even Server_SendUnreliableMessage() gets sent reliably during ClientConnect
	//fixme: the proper way to do this would for the client to send an ACK before ClientConnect
	localServer.clients[clientnum].reliableHack = true;

	//notify the game that a new client has connected
	sv_api.ClientConnect(clientnum, 
						localServer.clients[clientnum].netSettings, 
						localServer.clients[clientnum].net.clan_id, 
						localServer.clients[clientnum].restarting);

	localServer.clients[clientnum].reliableHack = false;

	localServer.clients[clientnum].restarting = false;

	localServer.clients[clientnum].gameConnectTime = server_timestamp;

	Console_Printf("Client %i (%s @ %s) has entered the game\n", clientnum, localServer.clients[clientnum].requestedName, localServer.clients[clientnum].net.recvAddrName);

	return true;
}



/*==========================

  Server_CNET_NETSETTINGS

 ==========================*/

bool	Server_CNET_NETSETTINGS(int clientnum, packet_t *pkt)
{
	char settings[1024];
	char *maxp;
	char *name;
	char *bps;
	char *netframes;

	if (svr_debug.integer)
	{
		Console_DPrintf("Client %i: CNET_NETSETTINGS\n", clientnum);
	}

	Pkt_ReadString(pkt, settings, 1024);		
	
	//we can handle some net settings right here
	maxp = ST_GetState(settings, "maxPacketSize");
	if (maxp[0])
	{
		int mp = atoi(maxp);

		if (mp > MAX_MTU_SIZE)
			mp = MAX_MTU_SIZE;
		else if (mp < 512)
			mp = 512;

		localServer.clients[clientnum].maxPacketSize = mp;
		Console_Printf("Client %i set maxPacketSize to %i\n", clientnum, mp);
	}

	bps = ST_GetState(settings, "netBPS");
	if (bps[0])
	{
		int bp = atoi(bps);

		if (bp > svr_maxbps.integer)
			bp = svr_maxbps.integer;
		if (bp < 2000)
			bp = 2000;

		localServer.clients[clientnum].bps = bp;
		Console_Printf("Client %i set bytes per second to %i\n", clientnum, bp);
	}

	netframes = ST_GetState(settings, "netFrames");
	if (netframes[0])
	{
		int nf = atoi(netframes);

		if (nf > 60)
			nf = 60;
		else if (nf < 1)
			nf = 1;

		localServer.clients[clientnum].netfps = nf;
		Console_Printf("Client %i set netframes to %i\n", clientnum, nf);
	}

	name = ST_GetState(settings, "name");
	if (name != "")
	{
		strncpySafe(localServer.clients[clientnum].requestedName, name, CLIENT_NAME_LENGTH);
	}

	if (localServer.clients[clientnum].cstate == CCS_IN_GAME)
		sv_api.NewNetSettings(clientnum, settings);

	strncpySafe(localServer.clients[clientnum].netSettings, settings, 1024);

	return true;
}


/*==========================

  Server_CNET_REQUEST_STATE_STRINGS

  should only happen when the client is connecting

 ==========================*/

bool	Server_CNET_REQUEST_STATE_STRINGS(int clientnum, packet_t *pkt)
{
	int n;
	int i = 0;
	clientConnectionInfo_t *client = &localServer.clients[clientnum];

	if (client->cstate != CCS_CONNECTING)
		return false;

	client->cstate = CCS_AWAITING_STATE_STRINGS;

	Pkt_WriteCmd(&client->net.reliableSend, SNET_STARTING_STATE_STRINGS);

	//queue up all state strings for sending
	for (n=0; n<MAX_STATE_STRINGS; n++)
	{		
		if (localServer.stateStrings[n].string)
			localServer.clients[clientnum].stateStringQueue[i++] = n;
	}

	localServer.clients[clientnum].stateStringQueueSize = i;

	return true;
}


/*==========================

  Server_ProcessPacket

  process incoming data from a connected client

 ==========================*/

void	Server_ProcessPacket(int clientnum)
{
	//bool gotInputState = false;
	char tmp[COOKIE_SIZE+1];
	clientConnectionInfo_t *client = &localServer.clients[clientnum];

	if (DLLTYPE == DLLTYPE_EDITOR)
		return;

	while(!Pkt_DoneReading(&client->net.recv))
	{			
		byte cmd;
		bool ret;

		cmd = Pkt_ReadCmd(&client->net.recv);

		switch(cmd)
		{
			case CNET_KEEPALIVE:
			{
				ret = Server_CNET_KEEPALIVE(clientnum, &client->net.recv);
				break;
			}
			case CNET_MESSAGE:
			{
				ret = Server_CNET_MESSAGE(clientnum, &client->net.recv);
				break;
			}
			case CNET_INPUTSTATE:
			{
				ret = Server_CNET_INPUTSTATE(clientnum, &client->net.recv);
				break;
			}
			case CNET_DISCONNECT:
			{
				ret = Server_CNET_DISCONNECT(clientnum, &client->net.recv);
				break;
			}
			case CNET_OK_TO_START:
			{
				ret = Server_CNET_OK_TO_START(clientnum, &client->net.recv);
				break;
			}
			case CNET_NETSETTINGS:
			{
				ret = Server_CNET_NETSETTINGS(clientnum, &client->net.recv);
				break;
			}
			case CNET_REQUEST_STATE_STRINGS:
			{
				ret = Server_CNET_REQUEST_STATE_STRINGS(clientnum, &client->net.recv);
				break;
			}
			/*case CNET_CLEAR_UPDATE:
			{
				ret = Server_CNET_CLEAR_UPDATE(clientnum, &client->net.recv);
				break;
			}*/
			case CPROTO_LEARNPORT:
			{
				//if we got this far, no need to process this command
				//just read it and let it continue
				Pkt_ReadString(&client->net.recv, tmp, COOKIE_SIZE+1);
				ret = true;
				break;
			}
			default:
				ret = false;
				break;
		}

		if (!ret)
		{
			Console_DPrintf("Invalid packet data received from client %i - command was %i\n", clientnum, cmd);
			Pkt_Clear(&client->net.recv);
		}
	}
}


/*==========================

  Server_WriteClientCookies

  used by Server_SendHeartbeat()

 ==========================*/

void	Server_WriteClientCookies(netconnection_t *nc)
{
	int i, count = 0;

	//write the cookies for all the clients
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (localServer.clients[i].active)
		{
			count++;
		}
	}

	Pkt_WriteByte(&nc->send, (byte)count);
	
	//write the cookies for all the clients
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (localServer.clients[i].active)
		{
			Pkt_WriteString(&nc->send, localServer.clients[i].net.cookie);
		}
	}
}
	

/*==========================

  Server_SendShutdown

  tell the master server we're shutting down

 ==========================*/

void	Server_SendShutdown()
{
	if (!Server_TalkToMaster())
		return;
		
	Pkt_Clear(&ncMasterServer.send);

	Pkt_WriteCmd(&ncMasterServer.send, CPROTO_SERVER_SHUTDOWN);
	Pkt_WriteByte(&ncMasterServer.send, 0);  //version 0
	Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[0]);
	Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[1]);
	Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[2]);
	Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[3]);
	Pkt_WriteShort(&ncMasterServer.send, (short)Server_GetPort());
	
	Net_SendPacket(&ncMasterServer);
}


/*==========================

  Server_SendHeartbeat

  send heartbeat to the master server

 ==========================*/

void	Server_SendHeartbeat()
{	
	if (!Server_TalkToMaster())
		return;
		
	Pkt_Clear(&ncMasterServer.send);

	Pkt_WriteCmd(&ncMasterServer.send, CPROTO_HEARTBEAT);
	Pkt_WriteByte(&ncMasterServer.send, 3); //version 3
	Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[0]);
	Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[1]);
	Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[2]);
	Pkt_WriteByte(&ncMasterServer.send, ((char *)&net_localip)[3]);
	Pkt_WriteShort(&ncMasterServer.send, (short)Server_GetPort());

	Server_WriteClientCookies(&ncMasterServer);

	Net_SendPacket(&ncMasterServer);

	Console_Printf("Sent heartbeat\n");
}



/*==========================

  Server_SendInfo

  Send information about this server to an outside source

  This is unrelated to the ST_SERVER_INFO string

 ==========================*/

void	Server_SendInfo(netconnection_t *nc, bool extended)
{
	char infostring[1024] = "";
	
	//write the name of the server
	ST_SetState(infostring, "name", svr_name.string, 512);
					
	//write the name of the current map
	ST_SetState(infostring, "world", svr_world.string, 512);	
					
	//num clients
	ST_SetState(infostring, "cnum", fmt("%i", localServer.numClients), 512);
	
	//max clients
	ST_SetState(infostring, "cmax", fmt("%i", localServer.maxClients), 512);

	//gametime in seconds
	ST_SetState(infostring, "time", fmt("%i", server_timestamp / 1000), 512);

	//gametime in seconds
	ST_SetState(infostring, "fw", svr_firewalled.string, 512);

	//send this info
	Pkt_WriteString(&nc->send, infostring);

	//now have the game code build a game specific string and send that
	infostring[0] = 0;
	if (extended)
		sv_api.BuildExtendedInfoString(infostring, 1024);
	else
		sv_api.BuildInfoString(infostring, 1024);

	Pkt_WriteString(&nc->send, infostring);
	
	Net_SendPacket(nc);
}


/*==========================

  Server_ReadPackets

  read all data being sent to us over the network  

 ==========================*/

bool	Server_ReadPackets()
{
	int i, numRead = 0;
	byte cmd;
	bool done = false;
	char str[COOKIE_SIZE+1];
	
	while (Net_ReceivePacket(&ncListen) > 0)
	{
		int clientnum;
		int oldpos = ncListen.recv.curpos;
		int clientid = Pkt_ReadShort(&ncListen.recv) & 0xffff;

		numRead++;
		
		netStats.svBytesInAccum += ncListen.recv.length;

		if (Server_IsClientConnected(&ncListen.recvAddr, clientid, &clientnum))
		{
			clientConnectionInfo_t *client = &localServer.clients[clientnum];
			//Mem_Copy(&localServer.clients[clientnum].net, &ncListen, sizeof(netconnection_t));
			
			Pkt_Copy(&ncListen.recv, &client->net.recv);
			Mem_Copy(&client->net.recvAddr, &ncListen.recvAddr, sizeof(struct sockaddr_in));
			Mem_Copy(&client->net.sendAddr, &ncListen.recvAddr, sizeof(struct sockaddr_in));
			strncpySafe(localServer.clients[clientnum].net.recvAddrName, inet_ntoa(ncListen.recvAddr.sin_addr), MAX_ADDRESS_LENGTH);
			strncpySafe(localServer.clients[clientnum].net.sendAddrName, inet_ntoa(ncListen.recvAddr.sin_addr), MAX_ADDRESS_LENGTH);
			if (Net_PreProcessPacket(&client->net))
			{
				Server_ProcessPacket(clientnum);
			}			
						
			client->lastMsgFrame = localServer.framenum;
		}
		else
		{
			if (clientid != 0xffff)				//hack to maintain compatibility with old protocol
				ncListen.recv.curpos = oldpos;

			cmd = Pkt_ReadCmd(&ncListen.recv);
			switch (cmd)
			{
				case CPROTO_LEARNPORT:
					Pkt_ReadString(&ncListen.recv, str, COOKIE_SIZE+1);
					str[COOKIE_SIZE] = 0;
					
					i = 0;
					done = false;
					while (i < MAX_CLIENTS && !done)
					{
						if (localServer.clients[i].active)
						{
							if (strcmp(localServer.clients[i].net.cookie, str) == 0)
							{
								Console_Printf("Client %i told us to retune their port to %i (%i host order)\n", i, ncListen.recvAddr.sin_port, ntohs(ncListen.recvAddr.sin_port));
								localServer.clients[i].net.sendAddr.sin_port = ncListen.recvAddr.sin_port;
								localServer.clients[i].net.recvAddr.sin_port = ncListen.recvAddr.sin_port;
								done = true;
							}
						}
						i++;
					}
					break;
					
				case CPROTO_CONNECT_ME:
					{
						char buf[256];
						char *ip;
						netconnection_t net;										
						short protocol_ver;
						int connectclientid;
						bool democlient;
	
						Console_Printf("someone from %s is trying to connect\n", ncListen.recvAddrName);
						Pkt_ReadString(&ncListen.recv, buf, 256);
						if (strcmp(buf, "S2Connect")!=0)
						{
							Console_Printf("Net_ReadOutOfBandPackets(): Invalid connection packet received from %s\n", ncListen.recvAddrName);
							break;
						}
		
						Net_Converse(&ncListen);  //prepare to talk to the client
		
						Net_CopyNetConnection(&ncListen, &net);
						//copy this sock over so it sends from the receive port and makes firewalls happy
						net.sock = ncListen.sock;

						protocol_ver = Pkt_ReadShort(&net.recv);
						if (protocol_ver != NET_PROTOCOL_VERSION)
						{
							Console_Printf("Wrong protocol version.  You are using protocol %i, I am using %s protocol %i.\n", protocol_ver, protocol_ver > NET_PROTOCOL_VERSION ? "an older" : "a newer", NET_PROTOCOL_VERSION);
							Server_SendDenyPacket(fmt("Wrong protocol version.  You are using protocol %i, I am using %s protocol %i.\n", protocol_ver, protocol_ver > NET_PROTOCOL_VERSION ? "an older" : "a newer", NET_PROTOCOL_VERSION));

							break;
						}

						if ((ip = Ban_IsIPBanned(ncListen.recvAddrName)))
						{
							Console_Printf("Your IP Address (%s*) is banned from this server.\n", ip);
							Server_SendDenyPacket(fmt("Your IP Address (%s*) is banned from this server.\n", ip));
							break;
						}

						Pkt_ReadString(&net.recv, buf, 256);
						if (svr_password.string[0])
						{
							if (strcmp(buf, svr_password.string)!=0)
							{
								Console_Printf("Invalid password\n");
								Server_SendDenyPacket("Invalid password.  Enter the correct password at the console with \"set cl_password <passwd>\"");
	
								break;
							}
						}
	
						if (Server_GetNumClients() >= localServer.maxClients)
						{
							Console_Printf("Server is full, rejecting\n");
							Server_SendDenyPacket("Server is full");
	
							break;
	
						}
						
						Pkt_ReadString(&net.recv, net.cookie, COOKIE_SIZE+1);
						net.cookie[COOKIE_SIZE]=0;

						if (!Net_ClientCookie_OK(net.cookie, &net.clan_id, &net.user_id, &net.guid))
						{
							Console_Printf("Invalid cookie from client, rejecting\n");						
							Server_SendDenyPacket("Invalid connection string");
	
							break;
						}

						if (Ban_IsUserIdBanned(net.guid))
						{
							Console_Printf("User GUID %u is banned from this server.\n", net.guid);
							Server_SendDenyPacket("You are banned from this server.\n");
							break;
						}

						connectclientid = Pkt_ReadShort(&net.recv) & 0xffff;
						democlient = Pkt_ReadByte(&net.recv);

#ifndef SAVAGE_DEMO
						if (democlient)
						{
							Console_Printf("Invalid connection string from client\n");
							Server_SendDenyPacket("Invalid connection string");
						}
#endif

						Pkt_Clear(&net.recv);
						Pkt_Clear(&net.send);
						Pkt_Clear(&net.reliableSend);
						
						net.reliableSend_lastseq = 0;
						net.reliableRecv_lastseq = 0;
		
						net.unack_packets = NULL;
								
						//send confirmation packet
						Pkt_Clear(&ncListen.send);
						Pkt_WriteCmd(&ncListen.send, CPROTO_OK);
						Net_SendPacket(&ncListen);
		
						Server_AddClient(&net, connectclientid, false, 0, democlient, NULL);
							
						done = true;
						break;
					}
	
				case CPROTO_PING:
					{
						int clienttime;
						clienttime = Pkt_ReadInt(&ncListen.recv);
						Net_Converse(&ncListen);  //prepare to talk to the client
						Console_DPrintf("sending ping to %s\n", ncListen.sendAddrName);
						//send ping reply
						Pkt_Clear(&ncListen.send);
						Pkt_WriteCmd(&ncListen.send, CPROTO_PING_ACK);
						Pkt_WriteInt(&ncListen.send, clienttime);
						Net_SendPacket(&ncListen);
	
						done = true;
					}
					break;

				case CPROTO_INFO_REQUEST:
					{
						int clienttime;
						//time = System_GetTime();
						//if (time - lastinfo > 0.5)
						//{
							clienttime = Pkt_ReadInt(&ncListen.recv);
							Net_Converse(&ncListen);  //prepare to talk to the client
							Console_DPrintf("sending info to %s\n", ncListen.sendAddrName);
							//send ping reply
							Pkt_Clear(&ncListen.send);
							Pkt_WriteCmd(&ncListen.send, CPROTO_INFO);
							Pkt_WriteByte(&ncListen.send, NET_PROTOCOL_VERSION);
							Pkt_WriteInt(&ncListen.send, clienttime);
							Server_SendInfo(&ncListen, false);
							//lastinfo = time;
						//}
						done = true;
					}
					break;
					
				case CPROTO_EXT_INFO_REQUEST:
					{
						int clienttime;
						//time = System_GetTime();
						//if (time - lastinfo > 0.5)
						//{
							clienttime = Pkt_ReadInt(&ncListen.recv);
							Net_Converse(&ncListen);  //prepare to talk to the client
							Console_DPrintf("sending info to %s\n", ncListen.sendAddrName);
							//send ping reply
							Pkt_Clear(&ncListen.send);
							Pkt_WriteCmd(&ncListen.send, CPROTO_EXT_INFO);
							Pkt_WriteByte(&ncListen.send, NET_PROTOCOL_VERSION);
							Pkt_WriteInt(&ncListen.send, clienttime);
							Server_SendInfo(&ncListen, cmd == CPROTO_EXT_INFO_REQUEST);
							//lastinfo = time;
						//}
						done = true;
					}
					break;
										
				case CPROTO_SEND_HEARTBEAT:
					{
						Server_SendHeartbeat();
						done = true;
					}
					break;
			}
			if (!done)
			{
				Console_DPrintf("Packet received from non-client %s:%i, cmd %i\n", ncListen.recvAddrName, ncListen.recvAddr.sin_port, cmd);
			}
			Pkt_Clear(&ncListen.recv);
		}
	}
	return numRead;
}


/*==========================

  Server_UpdatePlayerState

  called by the game code to set the playerstate that will be sent over the network to a client

 ==========================*/

void	Server_UpdatePlayerState(int clientnum, playerState_t *ps)
{
	if (!ps)
		return;

	Mem_Copy(&localServer.clients[clientnum].playerState, ps, sizeof(playerState_t));	
}




/*
//delta compare objects to determine 
void	Server_SendObjectUpdates(int clientnum, networkUpdate_t *compareAgainst, networkUpdate_t *out)
{
	clientConnectionInfo_t *client = &localServer.clients[clientnum];
	networkUpdate_t *update = &client->update;

	int indexToCompare = 0;
	int n = 0;
	int newStartIndex = (update->endindex + 1) % MAX_UPDATE_OBJECTS;
	int newEndIndex = newStartIndex;
	

	for (n=0; n<MAX_OBJECTS; n++)
	{
		baseObject_t *oldObject = compareAgainst[indexToCompare];

		if (oldObject->index 


		indexToCompare++;
	}
}
*/

/*===================

  Server_GenerateUpdate

  make a copy of all the unreliable data (objects + playerstate) we should be sending to the client

 ===================*/

networkUpdate_t *Server_GenerateUpdate(int clientnum)
{
	//bool first = true;
	int n;	
	clientConnectionInfo_t *client = &localServer.clients[clientnum];	
	networkUpdate_t *out = &client->updates[localServer.framenum % MAX_SV_UPDATE_HISTORY];

	out->framenum = localServer.framenum;
	out->time = server_timestamp;
	out->playerstate = client->playerState;
	out->num_objects = 0;
	out->start_object = client->cur_obj;

	for (n=0; n<localServer.num_gameobjs; n++)
	{
		baseObject_t *copy;
		baseObject_t *obj = localServer.gameobjs[n];
		
		//if (n == clientnum)					//don't send the client its own object
		//	continue;
		if (!obj->active)					//don't send inactive objects
			continue;
		if (!obj->type)						//don't send objects without a type set
			continue;
		if (client->exclusionList[n])		//don't send objects in the client's exclusion list
			continue;

		//from now on operate on 'copy' instead of the original object
		copy = &client->net_objects[client->cur_obj % client->num_net_objects];
		*copy = *obj;

		if (copy->flags & BASEOBJ_USE_TRAJECTORY && !copy->numEvents)
		{
			//zero out position, we don't need it
			M_ClearVec3(copy->pos);
		}	

		out->num_objects++;

		client->cur_obj++;
	}

	if (svr_debugUpdate.integer)
	{
		Console_Printf("Generated update with %i objects\n", out->num_objects);
	}

	return out;
}





/*==========================

  Server_SendPlayerStateDiff

 ==========================*/

void	Server_SendPlayerStateDiff(int clientnum, playerState_t *old)
{	
	clientConnectionInfo_t *client = &localServer.clients[clientnum];
	playerState_t *cur = &client->playerState;
	structDiff_t diff;
	int n;
	int numbytes = Net_GetStructDiff(old, cur, playerStateDesc, &diff);

	Pkt_WriteCmd(&client->net.send, SNET_UPDATE_PLAYERSTATE);
	Net_WriteStructDiff(&client->net.send, cur, playerStateDesc, &diff);

	//write any events (no delta compression used or needed here)
	Pkt_WriteByte(&client->net.send, cur->numEvents);
	for (n = 0; n < cur->numEvents; n++)
	{
		if (cur->events[n].param)
			cur->events[n].type |= EVENT_SENT_PARAM1;
		if (cur->events[n].param2)
			cur->events[n].type |= EVENT_SENT_PARAM2;
		Pkt_WriteByte(&client->net.send, cur->events[n].type);

		if (cur->events[n].type & EVENT_SENT_PARAM1)
			Pkt_WriteByte(&client->net.send, cur->events[n].param);

		if (cur->events[n].type & EVENT_SENT_PARAM2)
			Pkt_WriteByte(&client->net.send, cur->events[n].param2);
	}

	//clear events, we only want to send them once
	cur->numEvents = 0;
}



/*==========================

  Server_SendObjectDiffs

  objects that exist in the new update that were not in the old update are treated as new objects
  objects that don't exist in the new update that were present in the old update are treated as removed or "not visible" objects
  zero bytes get written if an object didn't change at all

 ==========================*/

void	Server_SendObjectDiffs(int clientnum, networkUpdate_t *oldu, networkUpdate_t *newu, int oldstart, int newstart)
{
	clientConnectionInfo_t *client = &localServer.clients[clientnum];
	packet_t *pkt = &client->net.send;
	int oldNumObjects = 0;
	int oldMarker = oldstart;
	int newMarker = newstart;

	if (!oldu)
		oldNumObjects = 0;
	else
		oldNumObjects = oldu->num_objects;

	//write the command
	Pkt_WriteCmd(pkt, SNET_UPDATE_OBJECTS);

	//step through each object in the old and new updates to compare them
	//we need to step entirely through both updates before we're done
	while(newMarker < newu->num_objects || oldMarker < oldNumObjects)
	{
		int oldInc, newInc;
		baseObject_t *oldObject = NULL, *newObject = NULL;
		structDiff_t diff;
		bool diffing;
		int writesize;
		int newIndex, oldIndex;

		if (newMarker < newu->num_objects)
		{
			newObject = &client->net_objects[(newu->start_object + newMarker) % client->num_net_objects];
			newIndex = newObject->index;
		}
		else
		{
			//went off the end of new update
			//set new index high so that we'll go down the newIndex > oldIndex path
			newIndex = MAX_OBJECTS;
		}

		if (oldMarker < oldNumObjects)
		{
			oldObject = &client->net_objects[(oldu->start_object + oldMarker) % client->num_net_objects];
			oldIndex = oldObject->index;
		}
		else
		{
			//went off the end of old update			
			//set old index high so that we'll go down the newIndex < oldIndex path
			oldIndex = MAX_OBJECTS;
		}

		if (newIndex == oldIndex)
		{
			//comparing the same object between frames
			diffing = true;
			//oldobject will already have been set above
			oldInc = 1;
			newInc = 1;
		}
		else if (newIndex > oldIndex)
		{
			//the object at oldIndex doesn't exist in the new update, so tell the client to free it
			diffing = false;
			oldInc = 1;
			newInc = 0;
		}
		else	//newIndex < oldIndex
		{
			//a new object exists in this update that wasn't in the old update
			diffing = true;
			//set oldobject to zeroed out data
			//todo: a very nice optimization could be to DIFF this object against the previous one
			//in the frame if their types match.  for weapons that fire many projectiles, often
			//subsequent objects will have the same type and be similar in their data.
			oldObject = &nullobj;
			oldInc = 0;
			newInc = 1;
		}

		if (diffing)
		{
			//get the diff of this object compared with the same object on the previous update
			//store the size of the diff.  will be 0 bytes if the object didn't change (but may have extra bytes from events)
			writesize = Net_GetStructDiff(oldObject, newObject, baseObjectDesc, &diff);
			if (newObject->numEvents)
				writesize += 4 + newObject->numEvents * sizeof(objEvent_t);	//estimated space taken up by events (guaranteed to be >= max event bytes)
		}
		else
		{
			writesize = 2;
		}

		if (writesize)		//need to write some data
		{
			if (pkt->curpos + writesize >= client->maxPacketSize-20)
			{
				//packet would overflow.  delay sending the rest of the data until the next server frame
				client->update_old_marker = oldMarker;
				client->update_new_marker = newMarker;
				client->update_old = oldu;
				client->update_new = newu;
 				Pkt_WriteShort(pkt, OBJECT_CONTINUED);

				if (svr_debugUpdate.integer)
				{
					Console_Printf("Overflowed, continuing update on next frame\n");
				}
				return;
			}

			if (diffing)
			{
				short index_flags = newObject->index;
				if (newObject->numEvents)
					index_flags |= OBJECT_HAS_EVENTS;
								
				Pkt_WriteShort(pkt, index_flags);
				Net_WriteStructDiff(pkt, newObject, baseObjectDesc, &diff);

				//write any events
				if (newObject->numEvents)
				{
					int e;

					Pkt_WriteByte(pkt, newObject->numEvents);
					for (e=0; e<newObject->numEvents; e++)
					{
						if (newObject->events[e].param)
							newObject->events[e].type |= EVENT_SENT_PARAM1;
						if (newObject->events[e].param2)
							newObject->events[e].type |= EVENT_SENT_PARAM2;
						Pkt_WriteByte(pkt, newObject->events[e].type);
						
						if (newObject->events[e].type & EVENT_SENT_PARAM1)
							Pkt_WriteByte(pkt, newObject->events[e].param);
						if (newObject->events[e].type & EVENT_SENT_PARAM2)
							Pkt_WriteByte(pkt, newObject->events[e].param2);
					}
				}
			}
			else
			{
				//the object is no longer visible or got freed
				if (!localServer.gameobjs[oldObject->index]->active)
				{
					Pkt_WriteShort(pkt, (short)(oldObject->index | OBJECT_FREE));
				}
				else
				{
					//the object is still active, so it must have left visibility
					Pkt_WriteShort(pkt, (short)(oldObject->index | OBJECT_NOT_VISIBLE));
				}
			}
		}

		oldMarker += oldInc;
		newMarker += newInc;
	}

	client->update_old = NULL;
	client->update_new = NULL;
	Pkt_WriteShort(pkt, OBJECT_END_UPDATE);
	client->num_updates_sent++;
	//set next update time based on client's net FPS
	client->nextUpdateTime = Host_Milliseconds() + 1000 / (client->netfps+2);		//+2 is a slight fudge so equal fps settings can match up better
}

void	Server_ClearUpdate(int clientnum)
{
	clientConnectionInfo_t *client = &localServer.clients[clientnum];

	client->update_old = NULL;
	client->update_new = NULL;	
	client->old_diff_frame = 0;
}


/*==========================

  Server_SendUpdateDiff

  sends all data needed to reconstruct object states and player state

 ==========================*/

void	Server_SendUpdateDiff(int clientnum)
{
	clientConnectionInfo_t *client = &localServer.clients[clientnum];	
	networkUpdate_t *oldUpdate;
	networkUpdate_t *newUpdate;
	int oldstart = 0, newstart = 0;
	bool continuing = false;

	if (client->update_new)		//continuing fragmented update from last frame
	{
		oldUpdate = client->update_old;
		oldstart = client->update_old_marker;

		newUpdate = client->update_new;
		newstart = client->update_new_marker;

		continuing = true;		
	}
	else
	{
		oldUpdate = NULL;
			
		if (client->old_diff_frame)
		{
			//the client wants a diff from some previously received frame.
			//this is the path we will be taking most of the time as
			//we receive acknowledgements of packets from the client.
			//if the frame the client requests is no longer in our
			//update history, then we have to send them a full update
			oldUpdate = &client->updates[client->old_diff_frame % MAX_SV_UPDATE_HISTORY];
			if (client->old_diff_frame != oldUpdate->framenum)
			{
				Console_DPrintf("Server lost update %i for client %i\n", client->old_diff_frame, clientnum);
				oldUpdate = NULL;
			}
		/*	for (n=0; n<MAX_SV_UPDATE_HISTORY; n++)
			{
				if (client->old_diff_frame == client->updates[n].framenum)
				{									
					oldUpdate = &client->updates[n];
					break;				
				}
			}*/
		}	
		
		//generate all the object data we want to send
		newUpdate = Server_GenerateUpdate(clientnum);

		if (oldUpdate)
		{
			if ((client->cur_obj - oldUpdate->start_object >= client->num_net_objects || oldUpdate->start_object > newUpdate->start_object) ||
				(!oldUpdate->framenum || oldUpdate->framenum > localServer.framenum))
			{
				//the old update is no longer valid, send data uncompressed
				oldUpdate = NULL;
			}
		}
	}

	if (!continuing)
	{
		client->continuedCount = 0;

		//write frame info
		Pkt_WriteCmd(&client->net.send, SNET_FRAME);
		Pkt_WriteInt(&client->net.send, server_timestamp);			//current server time
		//write old update frame number
		//the client needs to have this update in its buffer for diffing
		//if it fell out of the client's buffer, they'll request a full update
		Pkt_WriteInt(&client->net.send, oldUpdate ? oldUpdate->framenum : 0);
		//write new update frame number
		Pkt_WriteInt(&client->net.send, newUpdate->framenum);
	
		//send changes in playerstate	
		Server_SendPlayerStateDiff(clientnum, oldUpdate ? &oldUpdate->playerstate : &nullplayerstate);

		//send changes in game objects
		Server_SendObjectDiffs(clientnum, oldUpdate, newUpdate, 0, 0);
	}
	else
	{
		client->continuedCount++;

		Pkt_WriteCmd(&client->net.send, SNET_FRAME_CONTINUED);
		//write the update we're continuing
		Pkt_WriteInt(&client->net.send, newUpdate->framenum);
		//we increment a counter during the continued update so the client can detect if any packets get lost / unordered
		Pkt_WriteByte(&client->net.send, (byte)client->continuedCount);

		//send changes in game objects
		Server_SendObjectDiffs(clientnum, oldUpdate, newUpdate, oldstart, newstart);
	}
}






/*==========================

  Server_BandwidthOK

  returns false if we shouldn't send the client any more data this frame

  ==========================*/

bool	Server_BandwidthOK(int clientnum)
{
	//cycle through the last second of sent messages and total up their size
	//if we exceed the client's configured bytes per second, return false	

	//better than totaling up the bytes each second, as several large packets 
	//causing the connection to freeze will get cycled out sooner
	int n;
	int total = 0;

	clientConnectionInfo_t *client = &localServer.clients[clientnum];

#ifndef _DEBUG
	//ignore bandwidth estimation for local client
	if (localClient.cstate && localClient.clientnum == clientnum)
		return true;
#endif

	if (Host_Milliseconds() < client->nextUpdateTime)
		return false;

	for (n=0; n<svr_gamefps.integer; n++)
	{
		total += client->sendSize[n];
	}

	if (total > client->bps)
		return false;

	return true;
}


/*==========================

  Server_WriteGameUpdate

  called every frame for every client in the CCS_IN_GAME state

 ==========================*/

void	Server_WriteGameUpdate(int clientnum)
{		
	//send state string data reliably
	Server_SendStateStrings(clientnum);

	if (!Server_BandwidthOK(clientnum))
		return;			//exceeded client's bandwidth

	Server_SendUpdateDiff(clientnum);
	
	#ifdef NET_NO_DELTA_SEND
		Server_ClearUpdate(clientnum);
	#endif	
}

bool	Net_CompressPacketTest(netconnection_t *nc)
{
	/*byte outbuf[4096];
	long destlen;

	if (!svr_compressPackets.integer)
		return true;

	if (compress(outbuf, &destlen, &nc->send.__buf[HEADER_SIZE], nc->send.length) != Z_OK)
		return false;

	Console_DPrintf("compressed packet from %i to %i\n", nc->send.length, destlen);*/

	return true;
}


void	Server_SendPackets(int clientnum)
{	
	clientConnectionInfo_t *client = &localServer.clients[clientnum];
	netconnection_t *net = &localServer.clients[clientnum].net;

	Net_CheckPacketTimeouts(net);

	netStats.svBytesOutAccum += net->reliableSend.length;
	netStats.svBytesOutAccum += net->send.length;
	client->sendSize[localServer.framenum % svr_gamefps.integer] = net->reliableSend.length + net->send.length;
	
	if (net->reliableSend.length)
	{	  		
		if (net->reliableSend.length > MAX_MTU_SIZE)
		{
			//no way to send this packet
			//this will essentially drop an unreliable packet, which is BAD!
			//the client should really be dropped in this case, but we'll
			//keep them connected just in case the message really didn't
			//need to be reliable (could cause weirdness for the client)			
			Console_Printf("Reliable packet overflow for client %i\n", clientnum);
			Pkt_Clear(&net->reliableSend);
		}	
		else
		{
			Net_SendReliablePacket(net);
		}
	}

	if (net->send.length)
	{
		if (net->send.length > MAX_MTU_SIZE)
		{
			//drop the packet
			Pkt_Clear(&net->send);
		}
		else
		{
			if (!Net_SendPacket(net))
			{
				Console_DPrintf("Error sending packet to %s\n", net->sendAddrName);
			}
		}
	}

	if (net->reliableSend.length || net->send.length)
	{
		client->lastSendTime = Host_Milliseconds();
	}
}

unsigned int Server_GetClientGUID(int client_id)
{
	if (localServer.clients[client_id].active && localServer.clients[client_id].net.guid)
		return localServer.clients[client_id].net.guid;
	return 0;
}

unsigned int Server_GetClientUID(int client_id)
{
	if (localServer.clients[client_id].active && localServer.clients[client_id].net.user_id)
		return localServer.clients[client_id].net.user_id;
	return 0;
}

char *Server_GetClientCookie(int client_id)
{
	if (localServer.clients[client_id].active && localServer.clients[client_id].net.cookie)
		return localServer.clients[client_id].net.cookie;
	return NULL;
}


/*==========================

  Server_CheckClientTimeout

  if a client hasn't sent us a message in a while, drop him from the server

 ==========================*/

void	Server_CheckClientTimeout(int clientnum)
{
	int timeoutTime;
	clientConnectionInfo_t *client = &localServer.clients[clientnum];

	if (client->cstate == CCS_VIRTUAL)		//not a real client
		return;

	if (client->cstate == CCS_IN_GAME)
	{
		timeoutTime = svr_clientTimeout.integer;
	}
	else
	{
		//don't time them out as quickly if they're in the middle of connecting
		timeoutTime = svr_clientConnectionTimeout.integer;
	}

	if (localServer.framenum - client->lastMsgFrame > timeoutTime)
	{
		Console_Printf("Client %i timed out - framenum was %i, current frame is %i\n", clientnum, client->lastMsgFrame, localServer.framenum);
		Server_DisconnectClient(clientnum, "Server timed us out");
	}
}

/*==========================

  Server_WriteClientPackets

  called every game frame for every client connected to the server

  data will normally only get sent to the client when they're in the
  CCS_AWAITING_STATE_STRINGS or CCS_IN_GAME state

 ==========================*/

void	Server_WriteClientPackets(int clientnum)
{
	clientConnectionInfo_t *client = &localServer.clients[clientnum];

	switch(client->cstate)
	{
		case CCS_VIRTUAL:			//not a real client
			return;
		case CCS_AWAITING_STATE_STRINGS:
			Server_SendStateStrings(clientnum);
			
			if (client->stateStringQueueSize == 0)
			{
				//all strings have been sent
				//client can now be loading a map and precaching resources
				//we can put the client into the game once we receive a CNET_OK_TO_START message
				client->cstate = CCS_LOADING;
				Pkt_WriteCmd(&client->net.reliableSend, SNET_FINISHED_STATE_STRINGS);
				Pkt_WriteByte(&client->net.reliableSend, (byte)clientnum);
				client->xorbits = 0; //M_Randnum(-MAX_INT, MAX_INT-1);
				Pkt_WriteInt(&client->net.reliableSend, client->xorbits);
			}
			break;
		case CCS_IN_GAME:
			if (!client->update_new)
			{
				//if update_new, the client is still in the middle of getting sent a fragmented update
				Server_WriteGameUpdate(clientnum);
			}
			client->just_connected = false;
			break;
		default:
			break;
	}

	Server_SendPackets(clientnum);

	//reset the input state count
	client->inputStateNum = 0;	

	//set the current input time to the current server time
	client->curInputTime = server_timestamp;
}


/*==========================

  Server_UpdateStateStrings

  Update ST_SERVER_INFO and ST_CVAR_SETTINGS state strings

  Called every frame as well as during Server_Reset()

 ==========================*/

void	Server_UpdateStateStrings(bool first)
{
	static char s[8192] = "";

	if (cvar_transmitModified || first)
	{
		if (!Cvar_GetTransmitCvars(s, sizeof(s)))
			Game_Error("ST_CVAR_SETTINGS buffer overflow");

		Server_SetStateString(ST_CVAR_SETTINGS, s);
		
		cvar_transmitModified = false;
	}

	if (cvar_serverInfoModified || first)
	{
		if (!Cvar_GetServerInfo(s, sizeof(s)))
			Game_Error("ST_SERVER_INFO buffer overflow");

		Server_SetStateString(ST_SERVER_INFO, s);

		cvar_serverInfoModified = false;
	}
}


/*==========================

  Server_CheckMaxClients

  see if the value of maxclients has changed, or needs to be initially set

 ==========================*/

void	Server_CheckMaxClients()
{
	//the following sets up the number of clients allowed to connect to the server
	//doing the check here allows us to change maxclients on the fly
	if (localServer.active && (svr_maxclients.integer != localServer.maxClients || !localServer.maxClients))
	{		
		if (!localServer.maxClients)
		{			
			//allocate big array of objects
			localServer.pool_size = sizeof(baseObject_t) * 2048 * svr_maxclients.integer;
			localServer.net_object_pool = Tag_Malloc(localServer.pool_size, MEM_SERVER);

			Console_Printf("Allocated %i bytes for client data\n", localServer.pool_size);
		}
		else
		{
			int n;
			int newPoolSize = sizeof(baseObject_t) * 2048 * svr_maxclients.integer;
			if (newPoolSize > localServer.pool_size)		//only grow the array, don't shrink in case there are clients connected who still need the data
			{
				localServer.pool_size = newPoolSize;
				localServer.net_object_pool = Tag_Realloc(localServer.net_object_pool, localServer.pool_size, MEM_SERVER);
			
				Console_Printf("Increasing client data pool to %i bytes\n", localServer.pool_size);
			}
			
			for (n=0; n<localServer.maxClients; n++)
			{
				clientConnectionInfo_t *client = &localServer.clients[n];

				//give this client a piece of ls.net_object_pool
				client->net_objects = &localServer.net_object_pool[n * 2048];
				client->num_net_objects = 2048;
			}
		}

		localServer.maxClients = svr_maxclients.integer;		
		Console_Printf("Max clients set to %i\n", svr_maxclients.integer);
	}
}


/*==========================

  Server_CheckContinuedUpdates

  check for updates that were too large to send in one packet
  make sure at least 15 milliseconds have elapsed between sends
  to lower the risk of clogging the connection up

 ==========================*/

void	Server_CheckContinuedUpdates()
{
	FOR_ALL_CLIENTS
	{
		if (CLIENT.update_new)			//the client has a fragmented update pending
		{
			if (Host_Milliseconds() - CLIENT.lastSendTime >= 15 && Server_BandwidthOK(CLIENT_NUM))
			{
				int lastSendSize = CLIENT.sendSize[localServer.framenum % svr_gamefps.integer];

				Server_SendUpdateDiff(CLIENT_NUM);
				Server_SendPackets(CLIENT_NUM);

				//make sure bandwidth estimation is still correct
				CLIENT.sendSize[localServer.framenum % svr_gamefps.integer] += lastSendSize;
			}
		}
	}
	END_CLIENT_LOOP
}



/*==========================

  Server_Frame

  the main server processing function

 ==========================*/

void	_Server_Frame()
{
	int n;
	static int accum_msec = 0;

	if (DLLTYPE == DLLTYPE_EDITOR)
		return;
	if (!world.loaded)
		return;
	if (!localServer.active)
		return;

	Server_CheckMaxClients();
	
	//allow clients to connect through the authentication server
	Net_Server_TCPListenFrame();

	//see if it's time to expire the ban on any users...
	Bans_Frame();

	//process incoming data
	//this will mostly be inputstates from clients, with the
	//occasional chat message or other text based command.
	//issue any ProcessInput() calls and calculate pings here, too
	Server_ReadPackets();

	//advance server time	
	accum_msec += Host_FrameMilliseconds();	
	server_milliseconds += Host_FrameMilliseconds();

	//check for any fragmented updates that we can send without waiting for the next game frame
	Server_CheckContinuedUpdates();

	//send heartbeat to the master server to let it know we're alive
	if (server_milliseconds > localServer.nextHeartbeatTime)
	{
		localServer.nextHeartbeatTime = server_milliseconds + 30000;
		Server_SendHeartbeat();
	}
	
	//issue a console warning if the frame time is too long
	if (Host_FrameSeconds() > ((1.0 / svr_gamefps.value) * 1.5) && localServer.framenum)
	{
		Console_DPrintf("Warning: Long server frame (%i msec)\n", Host_FrameMilliseconds());
	}

	if (accum_msec < 1000 / svr_gamefps.integer)
		return;

	accum_msec = 0;

	//
	//everything from now on happens at svr_gamefps speed
	//
			
	//set the timestamp that will be sent to clients this frame
	server_timestamp = (int)server_milliseconds;

	//update ST_SERVER_INFO and ST_CVAR_SETTINGS state strings if necessary
	Server_UpdateStateStrings(false);

	// update navrep

	NavRep_Update();

	{
		OVERHEAD_INIT;
		//run game logic
		sv_api.Frame(server_timestamp);		
		OVERHEAD_COUNT(OVERHEAD_SERVER_GAMELOGIC);
	}

	//increment frame counter
	localServer.framenum++;
	
	{
		OVERHEAD_INIT;

		//send data back to each client and make sure they haven't timed out
		FOR_ALL_CLIENTS
			Server_WriteClientPackets(CLIENT_NUM);
			Server_CheckClientTimeout(CLIENT_NUM);
		END_CLIENT_LOOP

		OVERHEAD_COUNT(OVERHEAD_SERVER_SENDPACKETS);
	}

	//clear object events
	for (n = MAX_CLIENTS; n < MAX_OBJECTS; n++)
	{			
		localServer.gameobjs[n]->numEvents = 0;			
	}
}


void	Server_Frame()
{
	OVERHEAD_INIT;
	_Server_Frame();
	OVERHEAD_COUNT(OVERHEAD_SERVER_FRAME);
}


/*==========================

  Server_GameObjectPointer

  called by the game code so we can set a pointer to game code maintained object data

 ==========================*/

void	Server_GameObjectPointer(void *base, int stride, int num_objects)
{
	int n, offset;

	offset = 0;
	if (!stride)
		System_Error("Server_GameObjectPointer: stride not specified\n");

	if (num_objects > MAX_OBJECTS)
		System_Error("Server_GameObjectPointer: exceeded MAX_OBJECTS (%i)\n", MAX_OBJECTS);

	for (n=0; n<num_objects; n++)
	{
		(char *)localServer.gameobjs[n] = (char *)base + offset;

		//memset(localServer.gameobjs[n], 0, stride);

		offset += stride;
	}

	localServer.num_gameobjs = num_objects;
}





/*==========================

  Server_ClientExclusionListPointer

  called by the game code so we can set a pointer to client exclusion list data

 ==========================*/

void	Server_ClientExclusionListPointer(void *base, int stride, int num_clients)
{
	int n, offset;

	offset = 0;
	if (!stride)
		System_Error("Server_ClientExclusionListPointer: stride not specified\n");

	if (num_clients != MAX_CLIENTS)
		System_Error("Server_ClientExclusionListPointer: num_clients must be %i\n", MAX_CLIENTS);

	for (n=0; n<num_clients; n++)
	{
		//clientConnectionInfo_t *client = &localServer.clients[n];

		(char *)localServer.clientExclusionLists[n] = (char *)base + offset;		

		offset += stride;
	}
}


short		Server_GetPort()
{
	return localServer.port;
}



/*==========================

  Server_AddStringToQueue

  add a state string to send to a client

 ==========================*/

void	Server_AddStringToQueue(int clientnum, int id)
{
	clientConnectionInfo_t *client = &localServer.clients[clientnum];

	if (!localServer.stateStrings[id].string)
		return;		//nothing to send

	client->stateStringQueue[client->stateStringQueueSize] = id;
	client->stateStringQueueSize++;
	if (client->stateStringQueueSize >= MAX_STATE_STRINGS)
	{
		//overflowed queue, drop client...HIGHLY UNLIKELY			
		Server_DisconnectClient(clientnum, "Overflowed state string queue");
	}
}


/*==========================

  Server_SetStateString

  API function

 ==========================*/

void	Server_SetStateString(int id, const char *string)
{
	int len;
	stateString_t *state;

	if (id < 0 || id >= MAX_STATE_STRINGS)
	{
		Game_Error("Invalid state string index given for string %s (%i)\n", string, id);
		return;
	}

	state = &localServer.stateStrings[id];

	if (state->string)
		if (strcmp(string, state->string)==0)
			return;			//string wasn't modified

	len = strlen(string);

	if (state->memsize)
	{
		if (len+1 > state->memsize)
		{
			//allocate more memory
			state->string = Tag_Realloc(state->string, len+1, MEM_SERVER);
			state->memsize = len+1;
		}
	}
	else
	{
		state->string = Tag_Malloc(len+1, MEM_SERVER);
		state->memsize = len+1;
	}

	strcpy(state->string, string);
	state->modifyCount++;
	state->lastModifiedFrame = localServer.framenum;
	state->length = len;	

	if (state->flags & STATESTRING_REQUEST_ONLY)
		return;

	//add to all connected clients' send queues
	//note: don't remove duplicates in the queue, otherwise "big string" messages
	//can become corrupt over a few frames
	FOR_ALL_CLIENTS
	{
		Server_AddStringToQueue(CLIENT_NUM, id);
	}
	END_CLIENT_LOOP
}




/*==========================

  Server_SendRequestString

 ==========================*/

void	Server_SendRequestString(int clientnum, int id)
{
	if (id < 0 || id >= MAX_STATE_STRINGS)
		return;

	if (!(localServer.stateStrings[id].flags & STATESTRING_REQUEST_ONLY))
		return;

	Server_AddStringToQueue(clientnum, id);
}


/*==========================

  Server_GetStateString

  API function

 ==========================*/

char	*Server_GetStateString(int id, char *buf, int bufsize)
{
	strncpySafe(buf, localServer.stateStrings[id].string, bufsize);

	return buf;
}


void	Server_FreeObject(void *objdata)
{
	return;
}

void	Server_SpawnObject(void *obj, int index)
{
	return;
}



int		WOG_FindClosestObjects_Server(int *objectTypes, int num_objectTypes, float min_dist, float max_dist, vec3_t pos, int team, float *closest_dist)
{
	return WOG_FindClosestObjects(&localServer.grid, objectTypes, num_objectTypes, min_dist, max_dist, pos, team, closest_dist);
}

int		WOG_FindObjectsInRadius_Server(int *objectTypes, int num_objectTypes, float radius, vec3_t pos, int *objects, int numObjects)
{
	return WOG_FindObjectsInRadius(&localServer.grid, objectTypes, num_objectTypes, radius, pos, objects, numObjects);
}
