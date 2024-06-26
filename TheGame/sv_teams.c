/*
 * (C) 2002 S2 Games
 *
 * sv_teams.c - team-related functions 
 */

#include "server_game.h"

extern cvar_t spawntest_x;
extern cvar_t spawntest_y;
extern cvar_t sv_startingTeamGold;
extern cvar_t sv_startingTeamStone;
extern cvar_t sv_writeScores;
extern cvar_t sv_team1race;
extern cvar_t sv_team2race;

cvar_t sv_teamSwitchDelay =				{ "sv_teamSwitchDelay",				"30000" };
cvar_t sv_teamSwitchConnectFreedom =	{ "sv_teamSwitchConnectFreedom",	"30000" };

cvar_t sv_balancedTeams =				{ "sv_balancedTeams",				"1" };
cvar_t sv_balanceLenience =				{ "sv_balanceLenience",				"1" };

cvar_t sv_cullPlayerObjects = 		{ "sv_cullPlayerObjects",	"1" };

cvar_t sv_lockExclusion =			{ "sv_lockExclusion", "0", CVAR_CHEAT };

cvar_t sv_showNotifications = { "sv_showNotifications", "1" };

extern int numTeamUnits;
extern int allTeamUnits[];

bool	SV_UseCommander()
{
	if (sl.gametype == GAMETYPE_RTSS)
		return true;
	else
		return false;
}

void	SV_SendNotice(int clientnum, int message, int param, char *explanation)
{
	if (sv_showNotifications.integer && cores.Cvar_GetInteger("svr_dedicated"))
		cores.Console_Printf("To %i: (%i %i) %s\n", clientnum, message, param, explanation);
	cores.Server_SendMessage(-1,
							 clientnum,
							 fmt("%s %i %i %s", SERVER_NOTICE_MSG, 
								 				message, param,
												explanation));
}

teamInfo_t 	*SV_GetTeamInfo(int objidx)
{
	int team = -1;
	if (objidx < MAX_CLIENTS)
	{
		team = sl.clients[objidx].info.team;
	}
	else if (objidx < MAX_OBJECTS)
	{
		team = sl.objects[objidx].base.team;
	}
	if (team >= 0 && team < MAX_TEAMS)
		return &sl.teams[team];
	
	return NULL;
}

void	SV_SendNoticeToTeam(int team, int message, int param, char *explanation)
{
	int n;

	if (sv_showNotifications.integer && cores.Cvar_GetInteger("svr_dedicated"))
		cores.Console_Printf("To TEAM %i: (%i %i) %s\n", team, message, param, explanation);
	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (!sl.clients[n].active)
			continue;

		if (sl.clients[n].info.team == team)
			SV_SendNotice(n, message, param, explanation);
	}
}

void	SV_SendOfficerList(int team)
{
	int n;
	char officermsg[1024];

	strcpy(officermsg, SERVER_COMMANDER_OFFICER_LIST_MSG);
	for (n = 0; n < MAX_OFFICERS; n++)
		strcat(officermsg, fmt(" %i", sl.teams[team].officers[n]));

	for (n = 0; n < MAX_CLIENTS; n++)
	{
		if (sl.clients[n].info.team == team)
			cores.Server_SendMessage(-1, n, officermsg);
	}
}


/*==========================

  SV_PromoteClient

 ==========================*/

void	SV_PromoteClient(client_t *client, int slot)
{
	teamInfo_t *team = &sl.teams[client->info.team];
	int officerstate = raceData[team->race].officerState;

	if (slot < 0 || slot >= MAX_OFFICERS)
		return;

	//apply promotion
	SV_ApplyStateToObject(team->commander, client->index, officerstate, -1);
	team->officers[slot] = client->index;
	client->info.isOfficer = true;
	SV_RefreshClientInfo(client->index);

	SV_SendNotice(client->index, NOTICE_PROMOTE, client->index, "");
	SV_SendNotice(team->commander, NOTICE_PROMOTE, client->index, "");
	SV_SendOfficerList(team->index);
}


/*==========================

  SV_DemoteOfficer

 ==========================*/

void	SV_DemoteOfficer(int teamnum, int slot)
{
	teamInfo_t *team = &sl.teams[teamnum];
	int clientnum = team->officers[slot];

	//apply the demotion
	if (clientnum >= 0 && clientnum < MAX_CLIENTS
		&& teamnum == sl.clients[clientnum].info.team)
	{
		serverObject_t *targ;
		int officerstate = raceData[team->race].officerState;
		
		targ = &sl.objects[clientnum];
		SV_RemoveStateFromObject(targ->base.index, officerstate);
		team->officers[slot] = -1;
		sl.clients[clientnum].info.isOfficer = false;
		SV_RefreshClientInfo(clientnum);
	}

	SV_SendNotice(clientnum, NOTICE_DEMOTE, clientnum, "");
	SV_SendNotice(team->commander, NOTICE_DEMOTE, clientnum, "");
	SV_SendOfficerList(teamnum);
		
}


void	SV_DemoteClient(client_t *client)
{
	int index;

	for (index = 0; index < MAX_OFFICERS; index++)
	{
		if (sl.teams[SV_GetTeam(client->index)].officers[index] == client->index)
			SV_DemoteOfficer(SV_GetTeam(client->index), index);
	}
}

/*==========================

  SV_ForceOfficer_Cmd

  Debugging command to froce any particular client to overtake officer slot 1 on their team
  This should probably go away befor the game ships, otehrwise server operators could abuse it

 ==========================*/

void	SV_ForceOfficer_Cmd(int argc, char* argv[])
{
	int client;

	if (argc < 1)
		return;

	client = atoi(argv[0]);

	if (client < 0 || client >= MAX_CLIENTS || !sl.clients[client].active)
		return;

	SV_DemoteOfficer(SV_GetTeam(client), 1);
	SV_PromoteClient(&sl.clients[client], 1);
}


/*==========================

  SV_GiveState_Cmd

  Debugging command to give any particular client any state

 ==========================*/

void	SV_GiveState_Cmd(int argc, char* argv[])
{
	int client, state, duration;

	if (argc < 2)
		return;

	client = atoi(argv[0]);

	if (client < 0 || client >= MAX_CLIENTS || !sl.clients[client].active)
		return;

	if (!SetInt(&state, 1, (MAX_STATES - 1), stateNames, argv[1]))
		return;

	if (argc > 2)
		duration = atoi(argv[2]);
	else
		duration = 10000;

	SV_ApplyStateToObject(client, client, state, duration);
}


/*
 * Build Queue code
 */
void	SV_SendQueue(teamInfo_t *team)
{
	int i = 0;
	char queue[256] = {0};
	
	while (i < MAX_BUILD_QUEUES
			&& team->buildQueue[i].builderIndex > MAX_CLIENTS)
	{
		strcat(queue, fmt("%i %i ", team->buildQueue[i].builderIndex, team->buildQueue[i].objectType));
		i++;
	}
	
	SV_SendNotice(team->commander, NOTICE_QUEUED_RESEARCH, 0, queue);
}

bool	SV_AddQueuedRequest(teamInfo_t *team, int builder_index, byte type)
{
	int i = 0;

	while (i < MAX_BUILD_QUEUES
			&& team->buildQueue[i].builderIndex > MAX_CLIENTS)  //no need to queue up build requests on clients
	{
		i++;
	}
	if (i >= MAX_BUILD_QUEUES)
		return false;

	if (!IsBuildingType(sl.objects[builder_index].base.type))
	{
		cores.Console_DPrintf("Attempt to queue up a build request on a non-building object.\n");
		return false;
	}
	
	team->buildQueue[i].builderIndex = builder_index;
	team->buildQueue[i].objectType = type;

	SV_SendQueue(team);
	return true;
}

bool	SV_RemoveQueuedRequest(int builderIndex, byte type)
{
	int i = 0;
	teamInfo_t *team = SV_GetTeamInfo(builderIndex);

	while (i < MAX_BUILD_QUEUES
			&& team->buildQueue[i].builderIndex != builderIndex
			&& team->buildQueue[i].objectType != type)
	{
		i++;
	}
	if (i >= MAX_BUILD_QUEUES)
		return false;
	else
	{
		memmove(&team->buildQueue[i], &team->buildQueue[i+1], (MAX_BUILD_QUEUES - (i)) * sizeof(build_queue_t));
		team->buildQueue[MAX_BUILD_QUEUES-1].builderIndex = -1;
		team->buildQueue[MAX_BUILD_QUEUES-1].objectType = -1;

		SV_SendQueue(team);
		return true;
	}

}
	
int		SV_GetQueuedRequest(int builderIndex)
{
	int i = 0;
	int type;
	teamInfo_t *team = SV_GetTeamInfo(builderIndex);

	while (i < MAX_BUILD_QUEUES
			&& team->buildQueue[i].builderIndex != builderIndex)
	{
		i++;
	}
	if (i >= MAX_BUILD_QUEUES)
		return -1;
	else
	{
		type = team->buildQueue[i].objectType;
		SV_RemoveQueuedRequest(builderIndex, (byte)team->buildQueue[i].objectType);
		return type;
	}
}

/*
 * Random team-related functions
 */

void	SV_BroadcastNotice(int message, int param, char *explanation)
{
	cores.Console_Printf(fmt("(%i %i) %s\n", message, param, explanation));
	cores.Server_BroadcastMessage(-1, fmt("%s %i %i %s", SERVER_NOTICE_MSG, message, param, explanation));
}


int		SV_CountTeamPlayers(int teamnum)
{
	int n;
	teamInfo_t *team = &sl.teams[teamnum];

	team->num_players = 0;

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (sl.clients[n].active && sl.clients[n].info.team == teamnum)
			team->num_players++;
	}

	return team->num_players;
}

bool	SV_ClientIsCommander(int clientnum)
{
	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return false;

	if (sl.teams[SV_GetTeam(clientnum)].commander == clientnum)
		return true;

	return false;
}

void	SV_SendUnreliableTeamMessage(teamInfo_t *teamptr, int sender, char *msg, bool sendToCommander)
{
	int n;

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (sl.clients[n].active)
		{
			if (teamptr->commander == n && !sendToCommander)
				continue;

			if (sl.clients[n].info.team == teamptr->index)
				cores.Server_SendUnreliableMessage(sender, n, msg);
		}
	}
}

void	SV_SendTeamMessage(teamInfo_t *teamptr, int sender, char *msg, bool sendToCommander, bool reliable)
{
	int n;

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (sl.clients[n].active)
		{
			if (teamptr->commander == n && !sendToCommander)
				continue;

			if (sl.clients[n].info.team == teamptr->index)
			{
				if (reliable)
					cores.Server_SendMessage(sender, n, msg);
				else
					cores.Server_SendUnreliableMessage(sender, n, msg);
			}
		}
	}
}

void	SV_MessageSelected(teamInfo_t *teamptr, int sender, char *msg)
{
	int i;

	for (i = 0; i < teamptr->selection.numSelected; i++)
	{
		//are they a human player?
		if (teamptr->selection.array[i] < MAX_CLIENTS)
		{
			//are they on the same team?
			if (sl.clients[teamptr->selection.array[i]].info.team == teamptr->index)
			{
				cores.Server_SendMessage(sender, i, fmt("%s %s> %s", SERVER_CHAT_PRIVATE_MSG, sl.clients[sender].info.name, msg));
			}
		}
	}
}

void	SV_SendPrivateMessage(int sender, int receiver, char *msg)
{
	cores.Server_SendMessage(sender, receiver, fmt("%s %s> %s", SERVER_CHAT_PRIVATE_MSG, sl.clients[sender].info.name, msg));
}



/*==========================

  SV_SendCompleteTechUpdate

  send client info about everything the team has researched

 ==========================*/

void	SV_SendCompleteTechUpdate(teamInfo_t *team, int clientnum)
{
	int n;
	char s[1024] = "";
	bool shouldsend = false;

	if (sl.gametype != GAMETYPE_RTSS)
		return;

	strcpy(s, SERVER_COMMANDER_ALL_RESEARCH_MSG);

	//send info about researched items
	for (n = 0; n < MAX_OBJECT_TYPES; n++)
	{
		if (team->research[n].count)
		{
			strncat(s, fmt(" %i %i", n, team->research[n].count), 1023);
			shouldsend = true;
		}
	}

	if (shouldsend)
		cores.Server_SendMessage(-1, clientnum, s);

	SV_SendCommanderTeamResources(team->index, clientnum);
}




/*==========================

  SV_SetExclusionList

  set objects to exclude from sending to a client based on fog of war

 ==========================*/

#define MAX_EXCLUSION_SEARCH_OBJECTS 256

//take this serverObject and make all nearby objects visible to this client
void	SV_MakeObjectsVisible(int clientnum, int team, serverObject_t *obj, bool showNPCs)
{
	int j, i;
	int allUnits, numObjects;
	float fogdist;
	float dist;
	int objects[MAX_EXCLUSION_SEARCH_OBJECTS];
	client_t *client = &sl.clients[clientnum];

	allUnits = -1;
	i = obj->base.index;

	fogdist = GetObjectByType(obj->base.type)->viewDistance;

	if (sl.gametime < obj->fogRevealTime)  //object is being explicitly revealed for a few seconds
	{
		client->exclusionList[i] = false;			
	}
		
	if (!(obj->base.flags & BASEOBJ_WORLDOBJ_REPRESENTS)
		&& !cores.World_IsLinked(&obj->base))
	{
		if (obj->base.team == team
			|| SV_FindClosestObjects(allTeamUnits, numTeamUnits, i, team, fogdist, &dist) >= 0)
			client->exclusionList[i] = false;
	}
	else
	{
		if (obj->base.health <= 0
			|| team != obj->base.team)
			return;

		numObjects = SV_FindObjectsInRadius(&allUnits, 1, fogdist, 
						obj->base.pos, objects, MAX_EXCLUSION_SEARCH_OBJECTS);

		j = 0;

		while (j < numObjects)
		{
			if (showNPCs || sl.objects[objects[j]].base.team != TEAM_UNDECIDED)
				client->exclusionList[objects[j]] = false;
			j++;
		}
		client->exclusionList[i] = false;		
	}
}


void	SV_SetExclusionListLarge_Commander(int clientnum)
{
	int i, team;
	client_t *client = &sl.clients[clientnum];

	for (i = 0; i < MAX_OBJECTS; i++)
		client->exclusionList[i] = true;

	team = sl.clients[clientnum].info.team;
	
	for (i = 0; i <= sl.lastActiveObject; i++)
	{
		serverObject_t *obj = &sl.objects[i];

		if (!obj->base.active
			|| obj->base.flags & BASEOBJ_UNDER_CONSTRUCTION)
			continue;

		SV_MakeObjectsVisible(clientnum, team, obj, true);
	}
}


/*==========================

  SV_SetExclusionList

  Set up an array of objects not to send to the client

 ==========================*/

void	SV_SetExclusionList(int clientnum)
{
	int i, team;
	float dist;
	float *pos;
	float farclip = cores.Cvar_GetValue("gfx_farclip");
	client_t *client = &sl.clients[clientnum];
	serverObject_t *self;

	if (sv_lockExclusion.integer)
		return;

	if (client->psHasProxy)
		self = client->psHasProxy->obj;
	else
		self = client->obj;

	if (SV_ClientIsCommander(clientnum))
	{
		SV_SetExclusionListLarge_Commander(clientnum);
		return;
	}

	team = sl.clients[clientnum].info.team;
	
	if (!sv_cullPlayerObjects.integer)
	{
		memset(client->exclusionList, 0, sizeof(exclusionList_t));
		return;
	}

	for (i = 0; i < MAX_OBJECTS; i++)
		client->exclusionList[i] = true;
	
	if (sl.winningTeam)
	{
		int losingTeam = (sl.winningTeam == 1) ? 2 : 1;
		pos = sl.objects[sl.teams[losingTeam].command_center].base.pos;
	}
	else
	{
		if (client->ps.flags & PS_CHASECAM)
			pos = sl.objects[client->ps.chaseIndex].base.pos;
		else
			pos = client->ps.pos;
	}


	for (i = 0; i <= sl.lastActiveObject; i++)
	{
		if (self->base.index == i)		//don't send our own object, because we derive it from playerstate
			continue;

		dist = MAX(sl.objects[i].base.bmax[0], sl.objects[i].base.bmax[1]) + farclip;

		/*
		if (sl.objects[i].base.team == team
			&& GetObjectByType(sl.objects[i].base.type)->revealHidden)
		{
			SV_MakeObjectsVisible(clientnum, team, &sl.objects[i], false);
		}
		else */ 
		if (M_GetDistanceSq(pos, sl.objects[i].base.pos) <= (dist * dist))
		{
			client->exclusionList[i] = false;
		}
		else if (sl.objects[i].base.flags & BASEOBJ_USE_TRAJECTORY)
		{
			if (M_GetDistanceSq(pos, sl.objects[i].base.traj.origin) <= (dist * dist))
				client->exclusionList[i] = false;
			/*
			else
				cores.Console_DPrintf("object %i is a projectile but origin is (%f, %f), pos is (%f, %f), player pos is (%f, %f)\n", i, 
								sl.objects[i].base.traj.origin[X],
								sl.objects[i].base.traj.origin[Y],
								sl.objects[i].base.pos[X],
								sl.objects[i].base.pos[Y],
								pos[X], pos[Y]);
			*/
		}
		else if (GetObjectByType(sl.objects[i].base.type)->spawnFrom || GetObjectByType(sl.objects[i].base.type)->commandCenter)
		{
			client->exclusionList[i] = false;
		}
		else if (client->waypoint.active && sl.objects[i].base.index == client->waypoint.object_index)
		{
			//don't exclude the client's goal
			client->exclusionList[i] = false;
		}
		else if (sl.objects[i].base.team == client->info.team)
		{
			//send teammates
			client->exclusionList[i] = false;
		}
	}
}


//this function may have unexpected results
void	SV_ResearchEverything(teamInfo_t *team)
{
	int n;

	for (n = 0; n < MAX_OBJECT_TYPES; n++)
	{
		team->research[n].count = 1;
	}
}


bool    SV_SetCommander(int team, client_t *client, bool silent)
{
	if (!SV_UseCommander())
		return false;

	if (client->ps.status != STATUS_UNIT_SELECT)
		return false;

	if (team > 0 && team < MAX_TEAMS && client->active)	
	{
		if (client->info.team == team)
		{			
			if (sl.teams[team].commander > -1)
			{
				//resign the old commander
				SV_CommanderResign(sl.teams[team].commander, silent);
			}

			if (!silent)
				SV_SendNoticeToTeam(team, NOTICE_GENERAL, 0, fmt("%s has become the commander of your team!\n", client->info.name));
			
			//demote the player if they are an officer
			SV_DemoteClient(client);
			
			SV_SetClientStatus(client->index, STATUS_COMMANDER);
			sl.teams[team].commander = client->index;		
			
			sl.teams[team].selection.numSelected = 0;	//clear the unit selection
			
			//notify all clients with the new commander info
			SV_RefreshTeamInfo(team);

			//SV_SendCommanderTeamResources(team, client);
			//SV_SendCompleteTechUpdate(&sl.teams[team], client);	//send a complete tech update to the player so he knows what's available
		}
		else
		{
			cores.Console_DPrintf("client %i tried to become the commander of a different team\n");			
			return false;
		}

		return true;
	}
		   
	return false;
}

void	SV_GroupMove(teamInfo_t *team, vec3_t position, int goal)
{
	vec3_t groundpos;
	vec3_t diff;
	vec3_t closest_pos;
	vec3_t adjusted_pos;
	int closest, tmpdist, dist = 99999999;
	float jitter_x = 9;
	float jitter_y = 9;
	int i;
		
	groundpos[X] = position[X];
	groundpos[Y] = position[Y];
	groundpos[Z] = 0;

	//find the closest unit to the destination
	i = 0;
	while (i < team->selection.numSelected)
	{
		M_SubVec3(groundpos, sl.objects[team->selection.array[i]].base.pos, diff);
		tmpdist = M_DotProduct(diff, diff);
		if (tmpdist < dist)
		{
			dist = tmpdist;
			closest = i;
			M_CopyVec3(sl.objects[team->selection.array[i]].base.pos, closest_pos);
		}

		cores.World_UnlinkObject(&sl.objects[team->selection.array[i]].base);

		i++;
	}

	//now that we know which is closest, offset everyone's pos based on their relative pos with the closest guy
	i = 0;
	while (i < team->selection.numSelected)
	{
		M_SubVec3(sl.objects[team->selection.array[i]].base.pos, closest_pos, diff);
		diff[Z]=0;
		//collapse the group a bit
		while ((dist = M_DotProduct(diff, diff)) > 30000)
		{
			M_MultVec2(diff, 0.20, diff);
		}
		//add a little bit of randomness to it
		if (diff[X] > -10 && diff[X] < 10)
			jitter_x = diff[X] / 3;
		if (diff[Y] > -10 && diff[Y] < 10)
			jitter_y = diff[Y] / 3;
		diff[X] += M_Randnum(-jitter_x, jitter_x);
		diff[Y] += M_Randnum(-jitter_y, jitter_y);
		M_AddVec3(groundpos, diff, adjusted_pos);

	/*	if (sl.objects[team->selection.array[i]].path)
		{
			cores.Path_FreePath(sl.objects[team->selection.array[i]].path);
			sl.objects[team->selection.array[i]].path = NULL;
		}*/
		SV_CommanderTargetPosition(team->commander, team->selection.array[i], adjusted_pos[0], adjusted_pos[1], goal);		
		sl.objects[team->selection.array[i]].nextDecisionTime = sl.gametime;// + (i * 150);
		i++;
	}

	i = 0;
	//link everything back in
	while (i < team->selection.numSelected)
	{
		cores.World_LinkObject(&sl.objects[team->selection.array[i]].base);
		i++;
	}
}


void	SV_GroupMoveToObject(teamInfo_t *team, int objidx, int goal)
{
	int n = 0;

	for (n = 0; n < team->selection.numSelected; n++)
		SV_CommanderTargetObject(team->commander, team->selection.array[n], objidx, goal);
}


/*==========================

  SV_CommanderTargetPosition

  clientnum - the client who issued the order
  object - thing we are giving the waypoint to

 ==========================*/

void    SV_CommanderTargetPosition(int clientnum, int object, float targetx, float targety, int goal)
{
	if (!SV_ClientIsCommander(clientnum) && !SV_IsOfficer(clientnum))
		return;
	
	if (sl.objects[object].targetPosition)
	{
		sl.objects[object].targetPosition(&sl.clients[clientnum], &sl.objects[object], targetx, targety, goal);
	}
}


/*==========================

  SV_CommanderTargetObject

  clientnum - the client who issued the order
  object - thing we are giving the waypoint to

 ==========================*/

void    SV_CommanderTargetObject(int clientnum, int object, int target, int goal)
{
	if (!SV_ClientIsCommander(clientnum) && !SV_IsOfficer(clientnum))
		return;

	if (target < 0 || target > MAX_OBJECTS)
	{
		cores.Console_DPrintf("WARNING: %s sent an invalid command to target object %i\n", sl.clients[clientnum].info.name, target);

		return;
	}

	if (sl.objects[object].targetObject)
	{
		sl.objects[object].targetObject(&sl.clients[clientnum], &sl.objects[object], &sl.objects[target], goal);
		if (object < MAX_CLIENTS)
		{
			sl.clients[object].stats.ordersGiven++;
			sl.teams[sl.objects[clientnum].base.team].orderGiven = true;		

			if (!SV_ClientIsCommander(clientnum))
				sl.clients[object].waypoint.commander_order = false;
			else 
				sl.clients[object].waypoint.commander_order = true;
		}
	}
}

//get a valid position to spawn around an object 
bool	SV_GetSpawnPointFromBuilding(serverObject_t *obj, vec3_t point, int objectType)
{	
	return SV_GetSpawnPointAroundObject(obj->base.type, obj->base.pos, obj->base.angle, objectType, point); 
}


bool	SV_ValidTeamNumber(int teamnum)
{
	switch (sl.gametype)
	{
		case GAMETYPE_DEATHMATCH:
			if (teamnum < 0 || teamnum > 1)
				return false;
			break;
		case GAMETYPE_RTSS:
		default:
			if (teamnum < 0 || teamnum > MAX_TEAMS)
				return false;
			break;		
	}
	
	return true;	
}

extern cvar_t sv_clientConnectMoney;

void	SV_ClientJoinTeam(client_t *client, int team, bool silent, bool force)
{
	int old_team;
	playerScore_t oldscore;

	if (team >= MAX_TEAMS)
		return;

	//disable team switching if it's a deathmatch game
	if (sl.gametype == GAMETYPE_DEATHMATCH && team)
		team = 1;

	//sanity check the new team number
	if (!SV_ValidTeamNumber(team))
	{
		cores.Console_Printf("Client tried to pick an invalid team (%i)\n", team);
		return;
	}
	
	//get the old team
	old_team = client->info.team;

	//are they trying to join their existing team?
	if (team && (old_team == team))
		return;

	//don't allow team switching when spawned out, except in setup mode
	if (client->obj->base.active && old_team && sl.status != GAME_STATUS_SETUP)
	{
		SV_ClientEcho(client->index, "You can't switch teams while on the field\n");
		return;
	}

	if (sv_balancedTeams.integer && !force)
	{
		if (team)
		{
			int teamPlayers = SV_CountTeamPlayers(team) + 1;		//current players plus me
			int otherPlayers = SV_CountTeamPlayers(team ^ 3);
			if ((team ^ 3) == old_team)
				otherPlayers--;			

			if (teamPlayers - otherPlayers > sv_balanceLenience.integer)
			{
				SV_ClientEcho(client->index, fmt("Team %i has too many players\n", team));
				if (old_team == 0)
					team = 0;			//we could probably just return here, but let's just make sure they get on spectators correctly
				else
					return;
			}
		}	
	}

	//sanity check it
	if (old_team < 0 || old_team >= MAX_TEAMS)
		old_team = 0;

	if (client->teamLockEndTime > sl.gametime && team != 0)
	{
		SV_ClientEcho(client->index, fmt("Switching teams too quickly!  You must wait %i more seconds to switch teams\n", (client->teamLockEndTime - sl.gametime) / 1000));
		return;
	}

	//if they were on a team, demote/resign them from the old team if necessary
	if (old_team > 0 && old_team < MAX_TEAMS)
	{
		//remove client from the old team
		if (sl.teams[old_team].commander == client->index)
			SV_CommanderResign(client->index, true);

		SV_DemoteClient(client);
	}

	//clear all their states
	memset(client->ps.states, 0, sizeof(client->ps.states));
	memset(client->ps.stateExpireTimes, 0, sizeof(client->ps.stateExpireTimes));
	SV_UpdateClientObject(client->index);

	//assign the player to this team
	client->info.team = team;
	sl.objects[client->index].base.team = team;

	cores.Console_Printf("player %i has left team %i and joined team %i\n", client->index, old_team, team);	
	
	//reset their playerstate and give them some starting money
	oldscore = client->ps.score;
	memset(&client->ps, 0, sizeof(playerState_t));

	client->ps.score = oldscore;

	client->ps.clientnum = client->index;

	//clear stats
	//memset(&client->stats, 0, sizeof(clientStats_t));

	//clear exclusion list
	memset(&client->exclusionList, 0, sizeof(exclusionList_t));

	SV_CountTeamPlayers(old_team);		
	SV_CountTeamPlayers(team);		//update the num_players count

	if (!silent)
	{
		if (team)
			SV_BroadcastNotice(NOTICE_GENERAL, client->index, fmt("%s joined team %i", client->info.name, team));
		else
		{
			if (old_team)
				SV_BroadcastNotice(NOTICE_GENERAL, client->index, fmt("%s joined the spectators", client->info.name, team));
		}
	}

	if (team)
	{
		SV_SendCompleteTechUpdate(&sl.teams[team], client->index);	//send a complete tech update to the player so he knows what's available
	
		SV_SendOfficerList(team);

		SV_SetDefaultUnit(client);
		SV_SetClientStatus(client->index, STATUS_UNIT_SELECT);
		M_CopyVec3(sl.objects[sl.teams[team].command_center].base.pos, client->ps.pos);
		M_CopyVec3(sl.objects[sl.teams[team].command_center].base.pos, client->obj->base.pos);
	
		//zero out their cash
		client->ps.score.money = 0;
		SV_GiveMoney(client->index, sv_clientConnectMoney.integer, false);
	}
	else
	{
		traceinfo_t trace;

		//start spectators in the center of the map
		cores.World_TraceBox(&trace,
							 vec3(sl.worldBounds[0] / 2, sl.worldBounds[1] / 2, 99999),
							 vec3(sl.worldBounds[0] / 2, sl.worldBounds[1] / 2, -99999),
							 zero_vec, zero_vec, 0);

		M_SetVec3(client->ps.pos, trace.endpos[0], trace.endpos[1], trace.endpos[2] + 500);		
		M_ClearVec3(client->ps.velocity);
		client->ps.phys_mode = PHYSMODE_FREEFLY;
		client->ps.unittype = 0;
		SV_SetClientStatus(client->index, STATUS_SPECTATE);
	}

	//set restrictions to prevent abuse
	if (sl.status == GAME_STATUS_NORMAL && (sl.gametime - client->info.connectTime) >= sv_teamSwitchConnectFreedom.integer)
	{
		if (team != old_team)
			client->teamLockEndTime = sl.gametime + sv_teamSwitchDelay.integer;
	}

	SV_RefreshClientInfo(client->index);
}

void	SV_RefreshTeamInfo(int teamnum)
{
	//build a new team info state string
	//keep the state names short to conserve bandwidth
	char s[1024] = "";

	if (teamnum < 1 || teamnum >= MAX_TEAMS)
		return;

	//race
	ST_SetState(s, "r", fmt("%i", sl.teams[teamnum].race), 1024);
	//commander clientnum
	ST_SetState(s, "c", fmt("%i", sl.teams[teamnum].commander), 1024);
	//command center index
	ST_SetState(s, "C", fmt("%i", sl.teams[teamnum].command_center), 1024);

	cores.Server_SetStateString(ST_TEAM_INFO + teamnum, s);
}

/*==========================

  SV_ResetTeams

  initialize info for teams 1 and 2 (team 0 is spectator / undecided)
  keep clients in their teams

 ==========================*/

void	SV_ResetTeams()
{  	
	int i;
	int index = -1;
	teamInfo_t *team;

	//init spectator team
	sl.teams[0].name = "Spectator";
	sl.teams[0].index = 0;
	sl.teams[0].commander = -1;

	//set everyone to team 0 (undecided / spectator)
	for (i=0; i<MAX_CLIENTS; i++)
	{
		if (sl.clients[i].active)
			sl.clients[i].info.team = 0;
	}

	SV_CountTeamPlayers(0);

	if (sl.gametype == GAMETYPE_DEATHMATCH)
	{
		team = &sl.teams[1];
		team->index = 1;
		team->name = "";
		//fixme: this should get set by the game creator
		team->race = 1;
		team->commander = -1;
		team->command_center = 0;

		SV_ResearchEverything(team);
	}
	else if (sl.gametype == GAMETYPE_RTSS)
	{
		//find references to command centers and spawn them
		for (i = 1; i < MAX_TEAMS; i++)
		{
			int o, resource;
			cvar_t *racevar;

			referenceObject_t refobj;
			serverObject_t *obj;

			team = &sl.teams[i];

			team->index = i;
			team->name = "";
			//fixme: this should get set by the game creator
			team->commander = -1;
			team->lastResourceAdd = sl.gametime;
			for (o = 0; o < MAX_OFFICERS; o++)
				team->officers[o] = -1;

			//set team's race
			switch (i)
			{
				case 1:
					racevar = &sv_team1race;					
					break;
				case 2:
					racevar = &sv_team2race;
					break;
			}

			team->race = GetRaceDataByName(racevar->string) - &raceData[0];
			if (!team->race)
			{
				team->race = 1;
				cores.Console_Printf("Unrecognized race %s\n", racevar->string);
			}
			
			//find generic command_center object to use as a spawnpoint for this race's command center
			index = cores.WO_FindReferenceObject(index+1, "command_center", &refobj);
			
			if (index == -1)
			{
				//this should be handled more gracefully really, but we shouldn't ever meet this condition unless the server is running a bogus map
				core.Game_Error("Not enough command centers in world");
				return;
			}

			//spawn the command center belonging to the team's race
			obj = SV_SpawnObject(team->index, GetObjectTypeByName(raceData[team->race].baseBuilding), refobj.pos.position, refobj.pos.rotation);

			sl.objects[obj->base.index].base.team = i;		//assign the command center to this team
			team->command_center = obj->base.index;			

			resource = GetResourceByName("stone");
			if (resource >= 0)
				team->resources[resource] = sv_startingTeamStone.integer;
			resource = GetResourceByName("gold");
			if (resource >= 0)
				team->resources[resource] = sv_startingTeamGold.integer;

			memset(team->research, 0, sizeof(team->research));
			//team->research[sl.objects[team->command_center].base.type].count = 1;

			SV_CountTeamPlayers(i);

			for (o = 0; o < MAX_OBJECT_TYPES; o++)
			{
				vec3_t spawnPoint;

				if (IsUnitType(o))
				{
					objectData_t *unit = GetObjectByType(o);

					if (unit->race == team->race
						&& unit->isWorker && unit->spawnAtStartNum > 0)
					{
						int sp;

						for (sp = 0; sp < unit->spawnAtStartNum; sp++)
						{
							if (SV_GetSpawnPointFromBuilding(obj, spawnPoint, o))
							{
								vec3_t zerovec = { 0,0,0 };
								//if that worked, let's spawn our item
								SV_SpawnObject(obj->base.team, o, spawnPoint, zerovec);
							}
						}
					}
				}
			}
		}
	}	

	//run the team frame once to sync everything up
	SV_TeamFrame();

	for (i=0; i < MAX_TEAMS; i++)
		SV_RefreshTeamInfo(i);
}


void	SV_CommanderElect(client_t *client, teamInfo_t *team)
{
	if (!SV_UseCommander())
		return;

	if (sl.gametime < client->ps.respawnTime)
		return;

	if (sl.gametime < client->canCommandTime)
	{
		SV_ClientEcho(client->index, fmt("You are restricted from command for %i more seconds\n", (client->canCommandTime - sl.gametime) / 1000));
		return;
	}

	//let's see if there is a commander already
	if (team->commander > -1
		&& team->commander != client->index
		&& team->commander < MAX_CLIENTS
		&& sl.clients[team->commander].active)
	{
		//automatically call a vote

		char *param = "elect";

		SV_CallVote(client->index, 0, &param);
		return;
	}

	//set this client to be commander
	SV_SetCommander(team->index, client, false);	
}


void	SV_CommanderResign(int clientnum, bool silent)
{
	int teamnum = sl.clients[clientnum].info.team;
	teamInfo_t *team = &sl.teams[teamnum];

	//let's see if they are the commander of this team
	if (team->commander != clientnum)
	{
		cores.Console_Printf("Request to resign from commander from client %i, but they are not this team's commander.\n", clientnum);
		return;
	}

	team->commander = -1;
	SV_RefreshTeamInfo(team->index);

	//place them in the unit config screen
	SV_SetClientStatus(clientnum, STATUS_UNIT_SELECT);

	//set their exclusion list back to all 0's, so they can see everything again
	memset(sl.clients[clientnum].exclusionList, 0, sizeof(exclusionList_t));
		
	if (!silent)
		SV_SendNoticeToTeam(team->index, NOTICE_GENERAL, 0, fmt("%s has resigned as commander\n", sl.clients[clientnum].info.name));
}




void	SV_SendCommanderTeamResources(int team, int client)
{
	char	buffer[1024];
	int		index;

	strcpy(buffer, SERVER_COMMANDER_TEAM_RESOURCES_MSG);
	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
	{
		if (raceData[sl.teams[team].race].resources & (1 << index))
			strcat(buffer, fmt(" %i", sl.teams[team].resources[index]));
	}

	if (client == -1)
		SV_SendUnreliableTeamMessage(&sl.teams[team], -1, buffer, true);
	else
		cores.Server_SendUnreliableMessage(-1, client, buffer);
}

bool SV_GiveResourceToPlayer(int objindex, int resource, int amount)
{
	if (resource >= 0 && resource < MAX_RESOURCE_TYPES)
	{
		sl.objects[objindex].resources[resource] += amount;
		if (amount > GetObjectByType(sl.objects[objindex].base.type)->maxResources[resource])
			amount = GetObjectByType(sl.objects[objindex].base.type)->maxResources[resource];
		return true;
	}
	else
	{
		cores.Console_DPrintf("SV_GiveResourceToPlayer(): Unknown resource type %i\n", resource);
	}
	return false;
}

bool SV_GiveResourceToTeam(int team, int resource, int amount)
{
	if (resource >= 0 && resource < MAX_RESOURCE_TYPES)
	{
		if (raceData[sl.teams[team].race].resources & (1<<resource))
		{
			sl.teams[team].resources[resource] += amount;
			return true;
		}
	}
	else
	{
		cores.Console_DPrintf("SV_GiveResourceToTeam(): Unknown resource %i\n", resource);
	}

	return false;
}

//give resources to all teams
void SV_GiveResource_Cmd(int argc, char *argv[])
{
	int i = 0, t;

	if (argc < 1)
		return;

	if (!SetInt(&i, 0, GetNumResources(), resourceNames, argv[0]))
	{
		cores.Console_Printf("Invalid resource type\n");
		return;
	}

	for (t = 0; t < MAX_TEAMS; t++)
	{
		if (raceData[sl.teams[t].race].resources & (1<<i))
			sl.teams[t].resources[i] += (argc > 1) ? atoi(argv[1]) : 99999;
	}
}


/*==========================

  SV_ApplyStateToObject

  Adds a state to an object and makes adjustments

 ==========================*/

bool	SV_ApplyStateToObject(int inflictor, int target, int statenum, int time)
{
	serverObject_t	*obj;
	stateData_t		*state;
	client_t		*client;

	if (target < 0 || target >= MAX_OBJECTS)
		return false;

	if (statenum < 1 || statenum >= MAX_STATES)
		return false;

	obj = &sl.objects[target];
	client = (target < MAX_CLIENTS) ? &sl.clients[target] : NULL;
	state = &stateData[statenum];

	//check for a conflict and resolve for priority
	if (stateData[obj->base.states[state->slot]].priority > state->priority)
			return false;

	if (time > 0)
		obj->stateExpireTimes[state->slot] = sl.gametime + time;
	else
		obj->stateExpireTimes[state->slot] = time;

	obj->base.states[state->slot] = statenum;

	if (client)
	{
		memcpy(client->ps.states, obj->base.states, sizeof(client->ps.states));
		memcpy(client->ps.stateExpireTimes, obj->stateExpireTimes, sizeof(client->ps.stateExpireTimes));
	}

	obj->stateInflictors[state->slot] = inflictor;

	return true;
}


/*==========================

  SV_RemoveStateFromObject

  Clears an objects state

 ==========================*/

void	SV_RemoveStateFromObject(int objindex, int statenum)
{
	serverObject_t	*obj;
	stateData_t		*state;

	if (objindex < 0 || objindex >= MAX_OBJECTS)
		return;

	if (statenum < 0 || statenum >= MAX_STATES)
		return;

	obj = &sl.objects[objindex];
	state = &stateData[statenum];

	if (obj->base.states[state->slot] == statenum)
	{
		obj->base.states[state->slot] = 0;
		obj->stateExpireTimes[state->slot] = 0;

		if (obj->client)
		{
			obj->client->ps.states[state->slot] = 0;
			obj->client->ps.stateExpireTimes[state->slot] = 0;
		}
	}
}


/*==========================

  SV_ApplyStateToRadius

  Applies the specified state to all objects near specified object

 ==========================*/

void	SV_ApplyStateToRadius(int objindex, float radius, int state, int duration)
{
	int		index;
	vec3_t	pos, vec, org;
	float	dist;

	if (objindex < 0 || objindex >= MAX_OBJECTS)
		return;

	if (radius < 1 || !state)
		return;

	SV_GetPosition(objindex, org);

 	//test each object
	for (index = 0; index <= sl.lastActiveObject; index++)
	{
		//don't apply radstate to originater
		if (index == objindex)
			continue;

		//only active objects
		if (!sl.objects[index].base.active)
			continue;

		//skip things we know we can't apply state to
		if (sl.objects[index].base.team == 0 && !(stateData[state].radiusTargets & TARGET_NEUTRAL))
			continue;
		if (sl.objects[objindex].base.team != sl.objects[index].base.team && !(stateData[state].radiusTargets & TARGET_ENEMY))
			continue;
		if (sl.objects[objindex].base.team == sl.objects[index].base.team && !(stateData[state].radiusTargets & TARGET_ALLY))
			continue;

		if (IsUnitType(sl.objects[index].base.type) && !(stateData[state].radiusTargets & TARGET_UNIT))
			continue;
		if (IsBuildingType(sl.objects[index].base.type) && !(stateData[state].radiusTargets & TARGET_BUILDING))
			continue;

		if (sl.objects[index].base.type == sl.objects[objindex].base.type && !(stateData[state].radiusTargets & TARGET_SAME_TYPE))
			continue;

		//get the correct possition
		if (index < MAX_CLIENTS)
			M_CopyVec3(sl.clients[index].ps.pos, pos);
		else
			M_CopyVec3(sl.objects[index].base.pos, pos);
		
		//make sure the distance is within the radius

		//get distance from origin to target
		M_SubVec3(pos, org, vec);
 		dist = M_GetVec3Length(vec);

		//create a vector whose length is the radius of a sphere that could contain the objects bounding box
		vec[0] = MAX(ABS(sl.objects[index].base.bmin[0]), ABS(sl.objects[index].base.bmax[0]));
		vec[1] = MAX(ABS(sl.objects[index].base.bmin[1]), ABS(sl.objects[index].base.bmax[1]));
		vec[2] = MAX(ABS(sl.objects[index].base.bmin[2]), ABS(sl.objects[index].base.bmax[2]));
		
		//subtract the "bounding sphere" from the distance
		dist -= M_GetVec3Length(vec);
		if (dist < 0)
			dist = 0;
		
		//skip this object if it's out of range
		if (dist > radius)
			continue;

		SV_ApplyStateToObject(objindex, index, state, duration);
	}
}


/*==========================

  SV_AdjustObjectForNewUpgrade

  applies the effects of a new upgrade to an active object

 ==========================*/

void	SV_AdjustObjectForNewUpgrade(int upgrade, int objindex)
{
	objectData_t	*dat = &objData[upgrade];
	baseObject_t	*obj;

	if (!dat)
		return;

	if (objindex < 0 || objindex >= MAX_OBJECTS)
		return;

	obj = &sl.objects[objindex].base;
}



/*==========================

  SV_GiveUpgradeToTeam

 ==========================*/

void	SV_GiveUpgradeToTeam(int team, int upgrade)
{
	teamInfo_t *teamptr = &sl.teams[team];

	if (!((1<<objData[upgrade].objclass) & (UPG|WPN|UNT|ITM|MLE)))
		return;

	if (team < MAX_TEAMS && team >= 0)
	{
		teamptr->research[upgrade].count++;
		if (objData[upgrade].duration)
			teamptr->research[upgrade].expireTime = sl.gametime + objData[upgrade].duration;

		//send this information to the team
		SV_SendTeamMessage(teamptr, -1, fmt("%s %i %i", SERVER_COMMANDER_RESEARCH_UPDATE_MSG, upgrade, teamptr->research[upgrade].count), true, true);
		
		cores.Console_Printf("Team %i now has upgrade %i available (x%i).\n", team, upgrade, teamptr->research[upgrade].count);
	}
}


/*==========================

  SV_CountNumberUnderConstruction

 ==========================*/

int	SV_CountNumberUnderConstruction(int team, int item)
{
	int n, count = 0;

	for (n = 0; n <= sl.lastActiveObject; n++)
	{
		if (sl.objects[n].base.active
			&& sl.objects[n].base.team == team
			&& sl.objects[n].itemConstruction == item
			&& sl.objects[n].itemConstructionAmountLeft > 0)
			count++;
	}
	return count;
}


/*==========================

  SV_ActivateUpgrade

 ==========================*/

void	SV_ActivateUpgrade(int source, int target, int tech)
{
	objectData_t	*upgrade = GetObjectByType(tech);
	serverObject_t	*obj = &sl.objects[target], *cmndr = &sl.objects[source];
	int	targflags = 0, n;

	//validate commander
	if (sl.teams[cmndr->base.team].commander != source)
		return;

	if (objData[obj->base.type].isVehicle)
		return;

	//set team orientation
	if (!obj->base.team)
		targflags |= TARGET_NEUTRAL;
	else
		targflags |= (SV_GetTeam(target) == SV_GetTeam(source)) ? TARGET_ALLY : TARGET_ENEMY;

	//set object type
	if (IsBuildingType(obj->base.type))
		targflags |= TARGET_BUILDING;
	else if (IsUnitType(obj->base.type))
		targflags |= TARGET_UNIT;

	//can commander afford this?
	for (n = 0; n < MAX_RESOURCE_TYPES; n++)
	{
		if (sl.teams[SV_GetTeam(source)].resources[n] < upgrade->activateCost[n])
		{
			SV_ClientEcho(source, fmt("You need more %s to activate this power", resourceData[n].name));
			return;
		}
	}

	//compare target flags against unit
	if ((targflags & upgrade->targetFlags & TARGET_TEAMS) && (targflags & upgrade->targetFlags & TARGET_TYPES))
	{
		if (SV_GameScriptExecute(obj, obj, tech, GS_ENTRY_USE))
		{
			//success, deduct resources and exit
			for (n = 0; n < MAX_RESOURCE_TYPES; n++)
				sl.teams[SV_GetTeam(source)].resources[n] -= upgrade->activateCost[n];

			SV_Phys_AddEvent(obj, EVENT_POWERUP, 0, (byte)tech);
			SV_AddExperience(cmndr->client, EXPERIENCE_COMMANDER_POWERUP, 0, upgrade->expMult);
			return;
		}
		else
		{
			//SV_Phys_AddEvent(obj, EVENT_FIZZLE, 0, (byte)tech);
		}

	}

	//cores.Console_Printf("Couldn't hit target\n");
}


/*==========================

  SV_CanPurchaseObject

 ==========================*/

bool	SV_CanPurchaseObject(int team, int objtype, int buildertype, bool ignorecost)
{
	int				index;
	teamInfo_t		*teaminfo = &sl.teams[team];
	objectData_t	*objdata;

	//check race
	objdata = GetObjectByType(objtype);
	if (objdata->race != teaminfo->race)
		return false;

	//check availability
	if (!Tech_IsResearchable(objtype, teaminfo->research))
	{
		SV_SendNotice(sl.teams[team].commander, ERROR_NOTAVAILABLE, 0, "");
		return false;
	}

	//make sure the proper builder is selected (if any)
	if (!Tech_IsBuilder(objtype, buildertype))
		return false;

	//check for resources
	if (ignorecost)
		return true;

	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
	{
		if (teaminfo->resources[index] < GetObjectByType(objtype)->cost[index])
		{
			SV_SendNotice(teaminfo->commander, ERROR_NEEDRESOURCE, 0, fmt("%i", index));
			return false;
		}
	}

	return true;
}


/*==========================

  SV_PurchaseObject

 ==========================*/

bool	SV_PurchaseObject(int team, int type, serverObject_t *builder, bool ignorecost)
{
	int			index, builderType = 0;

	//things that are not buildings must have a builder
	if (!builder && !IsBuildingType(type))
		return false;

	if (builder)
		builderType = builder->base.type;

	if (!SV_CanPurchaseObject(team, type, builderType, ignorecost))
		return false;

	if (builder)
	{
		//we may already be constructing an item
		if (builder->itemConstruction)
		{
			if (GetObjectByType(type)->isWorker)
			{
				cores.Console_Printf("Queueing build request for object %i\n", builder->base.index);
				SV_AddQueuedRequest(&sl.teams[team], builder->base.index, (byte)type);
				return false;
			}
			else
				return false;
		}

		SV_ConstructObject(builder, type);
	}

	//withdraw resources
	if (!ignorecost)
	{
		for (index = 0; index < MAX_RESOURCE_TYPES; index++)
		{
			sl.teams[team].resources[index] -= GetObjectByType(type)->cost[index];
		}
	}

	cores.Console_DPrintf("Purchasing item %s\n", GetObjectByType(type)->name);

	return true;
}


/*==========================

  SV_UpdateTeamResearch

 ==========================*/

void	SV_UpdateTeamResearch(int t)
{
	teamInfo_t	*team = &sl.teams[t];
	researchData_t research[MAX_OBJECT_TYPES];
	int n;

	//brute force techtree update
	//this got rid of icky code
	//here we send the team an update about anything physical that got constructed (buildings or workers)

	memset(research, 0, sizeof(research));

	//update the temporary research array with PHYSICAL objects (upgrade types get added in SV_GiveUpgradeToTeam)
	for (n = MAX_CLIENTS; n <= sl.lastActiveObject; n++)
	{
		baseObject_t *base = &sl.objects[n].base;

		if (!base->active || base->health <= 0)
			continue;
		if (base->team != t)
			continue;
		if (base->flags & BASEOBJ_UNDER_CONSTRUCTION)		//still under construction
			continue;			

		if (!Tech_HasEntry(base->type))				//it's not part of the techtree, so don't update it
			continue;

		research[base->type].count++;
	}

	//now compare this with the old research array
	//send update messages for anything that has changed
	//fixme: could pack this more efficiently
	for (n = 0; n < MAX_OBJECT_TYPES; n++)
	{
		if (!IsBuildingType(n) && !IsWorkerType(n))		//ignore anything that's not a physical item we can construct
			continue;

		if (research[n].count != team->research[n].count)
		{
			//update the team's research array and send a message to the team
			team->research[n].count = research[n].count;
			SV_SendTeamMessage(team, -1, fmt("%s %i %i", SERVER_COMMANDER_RESEARCH_UPDATE_MSG, n, research[n].count), true, true);
		}			
	}
}

extern cvar_t sv_xp_commander_order_interval;
extern cvar_t sv_xp_commander_order_given;
extern cvar_t sv_xp_commander_order_followed;

void	SV_TeamFrame()
{
	int		t, n, x;
	bool	sendResourceUpdate;
	bool	sendDeploymentUpdate;
	char	buffer[1024];
	int		c;

	if (sl.gametype != GAMETYPE_RTSS)
		return;


	for (t = 1; t < MAX_TEAMS; t++)
	{
		teamInfo_t	*team = &sl.teams[t];

		//replenish technology resources
		if (sl.gametime - team->lastResourceAdd > 1000)
		{
			int bld, res, store[MAX_RESOURCE_TYPES];

			memset(store, 0, sizeof(int) * MAX_RESOURCE_TYPES);

			team->lastResourceAdd = sl.gametime;

			//step through all the objects
			for (bld = 0; bld < MAX_OBJECT_TYPES; bld++)
			{
				//skip non-buildings
				if (!IsBuildingType(bld))
					continue;

				//skip things that aren't in the techtree
				if (!team->research[bld].count)
					continue;

				//step through the resources and add any that are flagged
				for (res = 0; res < MAX_RESOURCE_TYPES; res++)
				{
					store[res] += GetObjectByType(bld)->store[res] * team->research[bld].count;
					if (GetObjectByType(bld)->generate & (1<<res))
					{
						team->resources[res]++;
						if (team->commander > 0 && team->commander < MAX_CLIENTS)
							SV_AddExperience(&sl.clients[team->commander], EXPERIENCE_COMMANDER_GATHER, 1, 1.0);
					}
				}
			}

			//cap off resources
			for (res = 0; res < MAX_RESOURCE_TYPES; res++)
			{
				if (team->resources[res] > store[res])
					team->resources[res] = store[res];
			}
		}

		//check for sending resource update
		sendResourceUpdate = false;		

		for (n = 0; n < MAX_RESOURCE_TYPES; n++)
		{
			if (team->resources[n] != team->oldResources[n])
			{
				sendResourceUpdate = true;
				team->oldResources[n] = team->resources[n];
			}
		}
		if (sendResourceUpdate)
			SV_SendCommanderTeamResources(t, -1);

		//update the current deployment array
		memset(team->deployedItems, 0, sizeof(int) * MAX_OBJECT_TYPES);
		for (n = 0; n < sl.lastActiveObject; n++)	//deployed items
		{
			baseObject_t *obj = &sl.objects[n].base;
			if (!obj->active)
				continue;
			if (obj->team != team->index)
				continue;
			if (GetObjectByType(obj->type)->maxDeployment <= 0)
				continue;

			team->deployedItems[obj->type]++;
		}
		for (n = 0; n < MAX_CLIENTS; n++)	//items in players' inventorys
		{
			client_t *client = &sl.clients[n];
			if (!client->active)
				continue;
			if (client->info.team != team->index)
				continue;

			for (x = 0; x < MAX_INVENTORY; x++)
			{
				if (client->ps.inventory[x])
				{
					if (GetObjectByType(client->ps.inventory[x])->maxDeployment > 0)
					{
						team->deployedItems[sl.clients[n].ps.inventory[x]] += sl.clients[n].ps.ammo[x];
					}
				}
			}
		}

		//check for sending a deployment update
		sendDeploymentUpdate = false;
		memset(buffer, 0, 1024);
		c = 0;
		strcpy(buffer, SERVER_ITEM_DEPLOYMENT_MSG);
		c += strlen(SERVER_ITEM_DEPLOYMENT_MSG);
		for (n = 0; n < MAX_OBJECT_TYPES; n++)
		{
			if (team->deployedItems[n] != team->oldDeployedItems[n])
			{
				sendDeploymentUpdate = true;
				team->oldDeployedItems[n] = team->deployedItems[n];
				if (c + strlen(fmt(" %i %i", n, team->deployedItems[n])) >= 1023)
					break;	//we'll catch the rest next time
				strcat(buffer, fmt(" %i %i", n, team->deployedItems[n]));
			}
		}
		if (sendDeploymentUpdate)
			SV_SendTeamMessage(team, -1, buffer, true, false);		//not that important that it get sent reliably

		//give out orders followed/given experience
		if (team->nextOrderReward < sl.gametime && team->commander >= 0 && team->commander < MAX_CLIENTS)
		{
			if (team->orderGiven)
				SV_AddExperience(&sl.clients[team->commander], EXPERIENCE_COMMANDER_ORDER_GIVEN, 0, 1.0);

			if (team->orderFollowed)
				SV_AddExperience(&sl.clients[team->commander], EXPERIENCE_COMMANDER_ORDER_FOLLOWED, 0, 1.0);

			team->orderGiven = team->orderFollowed = false;
			team->nextOrderReward = sl.gametime + sv_xp_commander_order_interval.integer;
		}

		SV_UpdateTeamResearch(t);
	}
}

void	SV_InitTeams()
{
	int i;	

	cores.Cvar_Register(&sv_cullPlayerObjects);

	cores.Cvar_Register(&sv_teamSwitchDelay);
	cores.Cvar_Register(&sv_teamSwitchConnectFreedom);
	cores.Cvar_Register(&sv_balancedTeams);
	cores.Cvar_Register(&sv_balanceLenience);
	cores.Cvar_Register(&sv_lockExclusion);
	cores.Cvar_Register(&sv_showNotifications);

	cores.Cmd_Register("forceOfficer",	SV_ForceOfficer_Cmd);
	cores.Cmd_Register("givestate",		SV_GiveState_Cmd);

	for (i = 0; i < MAX_TEAMS; i++)
	{
		memset(&sl.teams[i], 0, sizeof(build_queue_t));
	}
}
