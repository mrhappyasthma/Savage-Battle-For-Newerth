// (C) 2003 S2 Games

// sv_vclient.c

// virtual clients
// currently used for testing purposes, but could be used as a starting point for bot code


#include "server_game.h"


client_t *SV_ConnectVirtualClient()
{
	client_t *client;
	char netsettings[1024] = "";

	int clientnum = cores.Server_AllocVirtualClientSlot();
	
	if (clientnum == -1)
		return NULL;

	client = &sl.clients[clientnum];

	ST_SetState(netsettings, "name", fmt("Virtual Client %i", clientnum), 1024);

	SV_InitClientData(clientnum, netsettings, 0, false, true);

	return client;
}

void	SV_AddVClient_Cmd(int argc, char *argv[])
{
	if (!SV_ConnectVirtualClient())
		cores.Console_Printf("All client slots are in use\n");
}

void	SV_InitVirtualClients()
{
	cores.Cmd_Register("addvclient", SV_AddVClient_Cmd);
}