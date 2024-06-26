/*
 * (C) 2002 S2 Games
 *
 * cmdr_requests.c'
 */

#include "client_game.h"

// Handles requests from players for money (possibly other things?)
//=============================================================================

//=============================================================================
// CMDR_GiveSelectedPlayerMoney_Cmd
//=============================================================================
void	CMDR_GiveSelectedPlayerMoney_Cmd(int argc, char *argv[])
{
	int amount;

	//give amount of argv[0], or default to 200
	amount = atoi(argv[0]);
	if (!amount)
		amount = 200;

	//server interperets an invalid client number as targeting the currently selected player
	corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_COMMANDER_GIVEMONEY_MSG, MAX_CLIENTS, amount));
}
//=============================================================================

void	CMDR_GivePlayerMoney(int client, int amount)
{
	corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_COMMANDER_GIVEMONEY_MSG, client, amount));
}

//=============================================================================
// CMDR_GivePlayerMoney_Cmd
//
// arg0 is the client number to give to
// arg1 is the amount (default 200)
//=============================================================================
void	CMDR_GivePlayerMoney_Cmd(int argc, char *argv[])
{
	int clientnum, amount = 0;

	if (argc < 1)
		return;

	clientnum = atoi(argv[0]);

	if (argc > 1)
		amount = atoi(argv[1]);
	if (!amount)
		amount = 200;

	CMDR_GivePlayerMoney(clientnum, amount);
}

void	CMDR_GiveTeamUnitPlayerMoney_Cmd(int argc, char *argv[])
{
	int amount = 0;
	if (argc)
	{
		if (argc > 1)
			amount = atoi(argv[1]);
		if (!amount)
			amount = 200;
		CMDR_GivePlayerMoney(CL_TeamUnitToGlobalUnitNum(atoi(argv[0])), amount);
	}
}
//=============================================================================


#define MAX_REQUESTS 5

cvar_t	cl_cmdr_request_active[] = 
{
	{ "cl_cmdr_request1_active",	"" },
	{ "cl_cmdr_request2_active",	"" },
	{ "cl_cmdr_request3_active",	"" },
	{ "cl_cmdr_request4_active",	"" },
	{ "cl_cmdr_request5_active",	"" },
	{ "", "" }
};

cvar_t	cl_cmdr_request_money[] = 
{
	{ "cl_cmdr_request1_money",	"" },
	{ "cl_cmdr_request2_money",	"" },
	{ "cl_cmdr_request3_money",	"" },
	{ "cl_cmdr_request4_money",	"" },
	{ "cl_cmdr_request5_money",	"" },
	{ "", "" }
};

cvar_t	cl_cmdr_request_description[] = 
{
	{ "cl_cmdr_request1_description",	"" },
	{ "cl_cmdr_request2_description",	"" },
	{ "cl_cmdr_request3_description",	"" },
	{ "cl_cmdr_request4_description",	"" },
	{ "cl_cmdr_request5_description",	"" },
	{ "", "" }
};

cvar_t	cl_cmdr_request_time[] = 
{
	{ "cl_cmdr_request1_time",	"" },
	{ "cl_cmdr_request2_time",	"" },
	{ "cl_cmdr_request3_time",	"" },
	{ "cl_cmdr_request4_time",	"" },
	{ "cl_cmdr_request5_time",	"" },
	{ "", "" }
};

cvar_t	cl_cmdr_request_client[] = 
{
	{ "cl_cmdr_request1_client",	"" },
	{ "cl_cmdr_request2_client",	"" },
	{ "cl_cmdr_request3_client",	"" },
	{ "cl_cmdr_request4_client",	"" },
	{ "cl_cmdr_request5_client",	"" },
	{ "", "" }
};

cvar_t	cl_cmdr_request_clientnum[] = 
{
	{ "cl_cmdr_request1_clientnum",	"" },
	{ "cl_cmdr_request2_clientnum",	"" },
	{ "cl_cmdr_request3_clientnum",	"" },
	{ "cl_cmdr_request4_clientnum",	"" },
	{ "cl_cmdr_request5_clientnum",	"" },
	{ "", "" }
};

cvar_t	cl_pendingrequest_active =		{ "cl_pendingrequest_active",		"" };
cvar_t	cl_pendingrequest_money	=		{ "cl_pendingrequest_money",		"" };
cvar_t	cl_pendingrequest_description =	{ "cl_pendingrequest_description",	"" };
cvar_t	cl_pendingrequest_time =		{ "cl_pendingrequest_time",			"" };

cvar_t	cl_cmdr_requestPersistTime =	{ "cl_cmdr_requestPersistTime",		"20" };
cvar_t	cl_cmdr_autoApproveRequests =	{ "cl_cmdr_autoApproveRequests",	"0" };

char *requestNames[] = 
{
	"",
	"money",
	"object",
	"powerup",
	"promote",
	"build",

	""
};


int	CMDR_GetNextNewestRequest(int timecap)
{
	int n, newest = 0, r = -1;

	for (n = 0; n < MAX_CLIENTS; n++)
	{
		if (!cl.cmdrRequests[n].active)
			continue;

		if (cl.cmdrRequests[n].timestamp >= timecap)
			continue;

		if (cl.cmdrRequests[n].timestamp > newest)
		{
			newest = cl.cmdrRequests[n].timestamp;
			r = n;
		}
	}

	return r;
}

bool	CMDR_ExpireRequests()
{
	int n;
	bool update = false;
	
	for (n = 0; n < MAX_CLIENTS; n++)
	{
		if (!cl.cmdrRequests[n].active)
			continue;

		if (cl.gametime - cl.cmdrRequests[n].timestamp > 
			(cl_cmdr_requestPersistTime.integer * 1000))
		{
			corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_EXPIRE_MSG, n));
			memset(&cl.cmdrRequests[n], 0, sizeof(cmdrRequest_t));
			cl.cmdrRequests[n].active = false;
			update = true;
		}
	}

	return update;
}

/*==========================

  CMDR_UpdateRequestCvars

  Sets the values of all the request vars so the gui can handle them

 ==========================*/

void	CMDR_UpdateRequestCvars()
{
	int		n = 0, r = 0, newest = cl.gametime + 1;	//so we don't ignore requests that were just received
	int		seconds;
	bool	update;

	update = CMDR_ExpireRequests();

	//set them all inactive by default
	for (n = 0; n < MAX_REQUESTS; n++)
		corec.Cvar_SetVarValue(&cl_cmdr_request_active[n], 0);

	r = CMDR_GetNextNewestRequest(newest);
	n = 0;
	while (n < MAX_REQUESTS && r >= 0 && r < MAX_CLIENTS)
	{
		//mark request as active
		corec.Cvar_SetVarValue(&cl_cmdr_request_active[n], 1);

		//cost of request
		if (cl.cmdrRequests[r].money > 0)
			corec.Cvar_SetVar(&cl_cmdr_request_money[n], fmt("$%i", cl.cmdrRequests[r].money));
		else
			corec.Cvar_SetVar(&cl_cmdr_request_money[n], "");
		
		//description of the request
		switch (cl.cmdrRequests[r].type)
		{
		case REQUEST_MONEY:
			corec.Cvar_SetVar(&cl_cmdr_request_description[n], "Gold");
			break;
		case REQUEST_OBJECT:
			corec.Cvar_SetVar(&cl_cmdr_request_description[n], fmt("%s", CL_ObjectType(cl.cmdrRequests[r].object)->description));
			break;
		case REQUEST_POWERUP:
			corec.Cvar_SetVar(&cl_cmdr_request_description[n], fmt("%s powerup", CL_ObjectType(cl.cmdrRequests[r].object)->description));
			break;
		case REQUEST_PROMOTE:
			corec.Cvar_SetVar(&cl_cmdr_request_description[n], "A promotion to officer");
			break;
		case REQUEST_STRUCTURE:
			corec.Cvar_SetVar(&cl_cmdr_request_description[n], fmt("Placement of a %s", CL_ObjectType(cl.cmdrRequests[r].object)->description));
			break;
		default:
			corec.Cvar_SetVar(&cl_cmdr_request_description[n], "!!! Unknown request type !!!");
			break;
		}
		
		//time remaining for request
		seconds = (cl.gametime - cl.cmdrRequests[r].timestamp) / 1000;
		seconds = cl_cmdr_requestPersistTime.integer - seconds;
		corec.Cvar_SetVarValue(&cl_cmdr_request_time[n], seconds);
		
		//requesting client's name and number
		corec.Cvar_SetVar(&cl_cmdr_request_client[n], cl.clients[r].info.name);
		corec.Cvar_SetVarValue(&cl_cmdr_request_clientnum[n], r);

		newest = cl.cmdrRequests[r].timestamp;
		r = CMDR_GetNextNewestRequest(newest);
		n++;
	}

	if (update)
	{
		CL_UpdatePendingRequestCvars();
		CL_InterfaceEvent(IEVENT_REQUEST);
	}
}


/*==========================

  CMDR_Decline_Cmd

  Remove the request from the commanders buffer and
  notify the player that they have been denied

 ==========================*/

void	CMDR_Decline_Cmd(int argc, char *argv[])
{
	int clientnum;

	if (argc < 1)
		return;

	clientnum = atoi(argv[0]);
	if (clientnum < 0 || clientnum >= MAX_CLIENTS || !cl.cmdrRequests[clientnum].active)
		return;

	memset (&cl.cmdrRequests[clientnum], 0, sizeof(cmdrRequest_t));
	cl.cmdrRequests[clientnum].active = false;	//just to be safe
	corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_DECLINE_MSG, clientnum));
	CMDR_UpdateRequestCvars();
	CL_UpdatePendingRequestCvars();
	CL_InterfaceEvent(IEVENT_REQUEST);
}


/*==========================

  CMDR_Approve_Cmd

  Removes the request from the commanders buffer and
  sends a request for the server to grant the request

 ==========================*/

void	CMDR_Approve_Cmd(int argc, char *argv[])
{
	int clientnum;

	if (argc < 1)
		return;

	clientnum = atoi(argv[0]);
	if (clientnum < 0 || clientnum >= MAX_CLIENTS || !cl.cmdrRequests[clientnum].active)
		return;

	switch (cl.cmdrRequests[clientnum].type)
	{
	case REQUEST_MONEY:
		corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_COMMANDER_GIVEMONEY_MSG, clientnum, cl.cmdrRequests[clientnum].money));
		break;

	case REQUEST_OBJECT:
		corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_COMMANDER_GIVETHING_MSG, clientnum, cl.cmdrRequests[clientnum].object));
		break;

	case REQUEST_POWERUP:
		corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_COMMANDER_ACTIVATE_MSG, cl.cmdrRequests[clientnum].object, clientnum));
		break;

	case REQUEST_PROMOTE:
		CMDR_PromoteToOfficer(-1, clientnum);
		break;

	case REQUEST_STRUCTURE:
		cl.cmdrLookAtPoint[X] = cl.objects[clientnum].visual.pos[0];
		cl.cmdrLookAtPoint[Y] = cl.objects[clientnum].visual.pos[1];
		if (IsBuildingType(cl.cmdrRequests[clientnum].object))
			CL_StartPlaceObject(cl.cmdrRequests[clientnum].object);
		break;

	default:
		break;
	}

	memset (&cl.cmdrRequests[clientnum], 0, sizeof(cmdrRequest_t));
	cl.cmdrRequests[clientnum].active = false;	//just to be safe
	corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_APPROVE_MSG, clientnum));
	CMDR_UpdateRequestCvars();
	CL_UpdatePendingRequestCvars();
	CL_InterfaceEvent(IEVENT_REQUEST);
}


/*==========================

  CMDR_AddRequestToQueue

  Updates the commanders request buffer with a new request

 ==========================*/

void	CMDR_AddRequestToQueue(int clientnum, int requestType, int param)
{
	objectData_t	*obj;

	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return;

	if (requestType < 1 || requestType >= NUM_REQUEST_TYPES)
		return;

	obj = CL_ObjectType(param);

	cl.cmdrRequests[clientnum].active = true;
	cl.cmdrRequests[clientnum].type = requestType;
	cl.cmdrRequests[clientnum].timestamp = cl.gametime;
	if (requestType == REQUEST_MONEY)
		cl.cmdrRequests[clientnum].money = param;
	else if (requestType == REQUEST_OBJECT)
		cl.cmdrRequests[clientnum].money = obj->playerCost - cl.clients[clientnum].score.money;
	else
		cl.cmdrRequests[clientnum].money = 0;

	if (requestType == REQUEST_OBJECT || 
		requestType == REQUEST_POWERUP || 
		requestType == REQUEST_STRUCTURE)
		cl.cmdrRequests[clientnum].object = param;
	else
		cl.cmdrRequests[clientnum].object = 0;

	CMDR_UpdateRequestCvars();
	CL_UpdatePendingRequestCvars();
	CL_InterfaceEvent(IEVENT_REQUEST);

	if (cl_cmdr_autoApproveRequests.integer)
		corec.Cmd_Exec(fmt("approve %i", clientnum));
}


/*==========================

  CMDR_PlayerRequest_Cmd

  Command for a player to request something from the commander
  syntax: playerRequest <request type> <amount/type>

 ==========================*/

void	CMDR_PlayerRequest_Cmd(int argc, char *argv[])
{
	int n;
	int	requestType = 0, param = 0;
	objectData_t	*obj;

	//make sure there is a commander
	if (cl.teams[cl.info->team].commander < 0)
	{
		char *s[2];

		s[0] = "server";
		s[1] = "Your team has no commander.\n";
		corec.GUI_Notify(2, s);
		CL_Play2d(CL_RaceSnd("declined"), 1.0, CHANNEL_AUTO);
		return;
	}

	//arg0 must be set (type of request)
	if (argc < 1)
	{

		corec.Console_Printf("Invalid request.  Syntax:\n");
		corec.Console_Printf("request <id> <amount/type>\n");
		
		//show the valid request types
		corec.Console_Printf("Valid request ids are:\n");
		for (n = 1; n < NUM_REQUEST_TYPES; n++)
		{
			if (!strlen(requestNames[n]))
				break;
			corec.Console_Printf("%s\n", requestNames[n]);
		}
		return;
	}

	//see what kind of request this is, and validate the param
	SetInt(&requestType, 1, NUM_REQUEST_TYPES-1, requestNames, argv[0]);
	switch(requestType)
	{
	//player just wants an arbitrary sum of money
	case REQUEST_MONEY:
		if (argc < 2)
		{
			corec.Console_Printf("Must specify amount\n");
			return;
		}
		param = atoi(argv[1]);

		//don't let players request more than the team has left
		if (param < 1)
		{
			corec.Console_Printf("Must specify amount\n");
			return;
		}
		if (param > cl.resources[raceData[cl.race].currency])
		{
			char *s[2];

			s[0] = "server";
			s[1] = "Your team does not have that much gold\n";

			corec.GUI_Notify(2, s);
			return;
		}
		break;

	//player wants a particular item/weapon/unit
	//don't let them request something that is not availble
	//don't let them request something the team cannot afford
	case REQUEST_OBJECT:
		if (argc < 2)
		{
			corec.Console_Printf("Must specify object type\n");
			return;
		}
		SetInt(&param, FIRST_OBJECT_TYPE, LAST_OBJECT_TYPE, cl.objNames, argv[1]);
		obj = CL_ObjectType(param);

		//don't let them request an object type other than item/weapon/unit for their race
		if ((!IsWeaponType(param) && !IsUnitType(param) && !IsItemType(param)) || obj->race != cl.race)
		{
			corec.Console_Printf("Invalid object\n");
			return;
		}
		break;

	//player wants a powerup
	case REQUEST_POWERUP:
		if (argc < 2)
		{
			corec.Console_Printf("Must specify powerup type\n");
			return;
		}
		SetInt(&param, FIRST_OBJECT_TYPE, LAST_OBJECT_TYPE, cl.objNames, argv[1]);
		obj = CL_ObjectType(param);

		//make sure they are requestign a powerup and that it is relevant to their race
		if (!IsUpgradeType(param) || obj->race != cl.race)
		{
			corec.Console_Printf("Invalid powerup\n");
			return;
		}

		//don't let them request one that the team cannot afford
		for (n = 0; n < MAX_RESOURCE_TYPES; n++)
		{
			if (obj->activateCost[n] > cl.resources[n])
			{
				corec.Console_Printf("Your team cannot afford this right now.\n");
				return;
			}
		}
		break;

	//player wants to be an officer
	//make sure there is an officer slot free
	case REQUEST_PROMOTE:
		break;

	//player is suggesting a location to build
	//make sure the request is for a valid building of their race
	//make sure the team can afford the structure
	//make sure the building is available to build
	case REQUEST_STRUCTURE:
		if (argc < 2)
		{
			corec.Console_Printf("Must specify building type\n");
			return;
		}
		SetInt(&param, FIRST_OBJECT_TYPE, LAST_OBJECT_TYPE, cl.objNames, argv[1]);
		obj = CL_ObjectType(param);

		//don't let them request an object type other than item/weapon/unit for their race
		if (!IsBuildingType(param) || obj->race != cl.race)
		{
			corec.Console_Printf("Invalid building\n");
			return;
		}
		break;

	default:
		corec.Console_Printf("Invalid request.  Syntax:\n");
		corec.Console_Printf("request <id> <amount/type>\n");
		corec.Console_Printf("Valid request ids are:\n");
		for (n = 1; n < NUM_REQUEST_TYPES; n++)
		{
			if (!strlen(requestNames[n]))
				break;
			corec.Console_Printf("%s\n", requestNames[n]);
		}
		return;
	}

	corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_REQUEST_MSG, requestType, param));
	
	//setup the pending request info
	cl.pendingRequest.active = true;
	cl.pendingRequest.type = requestType;
	cl.pendingRequest.timestamp = cl.gametime;
	
	if (requestType == REQUEST_MONEY)
		cl.pendingRequest.money = param;
	else if (requestType == REQUEST_OBJECT)
		cl.pendingRequest.money = obj->playerCost - cl.predictedState.score.money;
	else
		cl.pendingRequest.money = 0;

	if (requestType == REQUEST_OBJECT || 
		requestType == REQUEST_POWERUP || 
		requestType == REQUEST_STRUCTURE)
		cl.pendingRequest.object = param;
	else
		cl.pendingRequest.object = 0;

	CL_UpdatePendingRequestCvars();
	CL_InterfaceEvent(IEVENT_REQUEST);
}


/*==========================

  CMDR_CancelRequest_Cmd

  Send a message to the commander 

  ==========================*/

void	CMDR_CancelRequest_Cmd(int argc, char *argv[])
{
	if (!cl.pendingRequest.active)
		return;

	memset(&cl.pendingRequest, 0, sizeof(cmdrRequest_t));
	cl.pendingRequest.active = false;

	corec.Client_SendMessageToServer(fmt("%s", CLIENT_COMMANDER_CANCEL_MSG));
	CL_UpdatePendingRequestCvars();
	CL_InterfaceEvent(IEVENT_REQUEST);
}


/*==========================

  CMDR_InitRequests

 ==========================*/

void	CMDR_InitRequests()
{
	corec.Cmd_Register("giveSelectedPlayerMoney",	CMDR_GiveSelectedPlayerMoney_Cmd);
	corec.Cmd_Register("givePlayerMoney",			CMDR_GivePlayerMoney_Cmd);
	corec.Cmd_Register("giveTeamUnitMoney",			CMDR_GiveTeamUnitPlayerMoney_Cmd);

	corec.Cmd_Register("playerRequest",	CMDR_PlayerRequest_Cmd);
	corec.Cmd_Register("cancelRequest",	CMDR_CancelRequest_Cmd);
	corec.Cmd_Register("decline",		CMDR_Decline_Cmd);
	corec.Cmd_Register("approve",		CMDR_Approve_Cmd);

	corec.Cvar_Register(&cl_pendingrequest_active);
	corec.Cvar_Register(&cl_pendingrequest_money);
	corec.Cvar_Register(&cl_pendingrequest_description);
	corec.Cvar_Register(&cl_pendingrequest_time);

	corec.Cvar_Register(&cl_cmdr_requestPersistTime);
	corec.Cvar_Register(&cl_cmdr_autoApproveRequests);

	RegisterVarArray(cl_cmdr_request_active);
	RegisterVarArray(cl_cmdr_request_money);
	RegisterVarArray(cl_cmdr_request_description);
	RegisterVarArray(cl_cmdr_request_time);
	RegisterVarArray(cl_cmdr_request_client);
	RegisterVarArray(cl_cmdr_request_clientnum);
}
