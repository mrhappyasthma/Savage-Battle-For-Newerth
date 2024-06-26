// (C) 2001 S2 Games

// sv_clients.c

#include "server_game.h"

cvar_t spawntest_x =			{ "spawntest_x",			"5000" };
cvar_t spawntest_y =			{ "spawntest_y",			"5000" };
cvar_t sv_maxMoney =			{ "sv_maxMoney",			"10000" };
cvar_t sv_tithe =				{ "sv_tithe",				"0.7" };
cvar_t sv_giveOverflowToTeam =	{ "sv_giveOverflowToTeam",	"0" };
cvar_t sv_invincibleTime =		{ "sv_invincibleTime",		"2000" };
cvar_t sv_ressurectTime =		{ "sv_ressurectTime",		"7000" };
cvar_t sv_autoRespawn =			{ "sv_autoRespawn",			"0" };
cvar_t sv_simultaneousInputs =	{ "sv_simultaneousInputs",	"0", CVAR_CHEAT };
cvar_t sv_ignoreMoney =			{ "sv_ignoreMoney",			"0", CVAR_CHEAT };
cvar_t sv_fullSpectateTest =	{ "sv_fullSpectateTest",	"0" };

cvar_t sv_skillfullKillDist =	{ "sv_skillfullKillDist",	"500" };


/*==========================

  SV_RefreshClientInfo

  Update the appropriate ST_CLIENT_INFO state string

 ==========================*/

void	SV_RefreshClientInfo(int clientnum)
{
	char s[1024] = "";
	client_t *client = &sl.clients[clientnum];

	//update the client's state string with the values from the sharedClientInfo struct
	//keep the state names short to conserve bandwidth

	ST_SetState(s, "n", client->info.name, 1024);
	ST_SetState(s, "t", fmt("%i", client->info.team), 1024);
	ST_SetState(s, "o", fmt("%i", client->info.isOfficer), 1024);
	ST_SetState(s, "c", fmt("%i", client->info.clan_id), 1024);
	ST_SetState(s, "r", fmt("%i", client->info.isReferee), 1024);
	ST_SetState(s, "rd", fmt("%i", client->info.ready), 1024);

	cores.Server_SetStateString(ST_CLIENT_INFO + clientnum, s);
}


/*==========================

  SV_RefreshClientScores

  we just write a series of ints instead of states to save on bandwidth
  (states aren't really necessary since there's no spaces or special characters)

 ==========================*/

void	SV_RefreshClientScores()
{
	int n;	

	for (n=0; n<MAX_CLIENTS; n++)
	{
		char *s;
		playerScore_t *score = &sl.clients[n].ps.score;

		if (!sl.clients[n].active)
			continue;

		s = fmt(
			"%i %i %i %i %i %i %i",
			score->kills,
			score->deaths,
			(int)score->experience,
			score->level,
			score->money,
			score->loyalty,
			score->ping
		);

		//these strings are REQUEST ONLY, so we don't have to worry about spamming the client with redundant changes
		cores.Server_SetStateString( ST_CLIENT_SCORE + n, s );
	}
}


/*==========================

  SV_SendScores

  A client will request scores when displaying the teamlist

  Commander requests team scores every couple of seconds all the time,
  more frequently for players he has selected  

 ==========================*/

void	SV_SendScores(int sendTo, int teamfilter, int clientfilter)
{
	int n;

	for (n = 0; n < MAX_CLIENTS; n++)
	{		
		if (n == sendTo)
			continue;		//never send a client his own score
		if (!sl.clients[n].active)
			continue;

		if (clientfilter > -1)
		{
			if (n != clientfilter)
				continue;
		}
		else if (teamfilter > -1)
		{
			if (sl.clients[n].info.team != teamfilter)
				continue;
		}

		cores.Server_SendRequestString(sendTo, ST_CLIENT_SCORE + n);
	}
}


/*==========================

  SV_Chase

  set up client in chasecam mode

 ==========================*/

bool	SV_Chase(int clientnum, int objectToSpectate)
{
	client_t *client = &sl.clients[clientnum];

	if (!sl.objects[objectToSpectate].base.active)
		return false;
	
	client->ps.flags |= PS_CHASECAM;
	client->ps.chaseIndex = objectToSpectate;
	M_CopyVec3(sl.objects[objectToSpectate].base.pos, client->ps.chasePos);

	if (client->ps.health <= 0 || !client->info.team || client->ps.status != STATUS_PLAYER)
		client->ps.statusMessage |= STATUSMSG_SPECTATING_PLAYER;

	return true;
}


/*==========================

  SV_ChaseNextTeammate

  chasecam!

 ==========================*/

void	SV_ChaseNextTeammate(int clientnum)
{	
	client_t *client = &sl.clients[clientnum];
	int n = client->ps.chaseIndex;
	int team = client->info.team;
	int i = 0;

	while(1)
	{
		n++;
		if (n>=MAX_CLIENTS)
			n=0;		
		if (i>=MAX_CLIENTS)
			return;
		//if (n==clientnum)
		//	return;

		//if (n < MAX_CLIENTS)
		{
			if (sl.clients[n].active && n != clientnum)
			{
				if (sl.clients[n].info.team == team)
				{
					if (SV_Chase(clientnum, n))
						return;
				}
			}
		}/*
		else
		{
			if (sl.objects[n].base.active)
			{
				if (sl.objects[n].base.team == team)
				{
					client->ps.chaseIndex = n;
					return;
				}
			}
		}*/
		i++;
	}

	//chase ourselves if no one left to spectate
	SV_Chase(clientnum, clientnum);
}

void	SV_ClientVoiceChat(int clientnum, int category, int item)
{
	voiceMenu_t *menu;
	client_t *client = &sl.clients[clientnum];
	teamInfo_t *team = &sl.teams[client->info.team];

	if (category >= MAX_VOICECHAT_CATEGORIES || category < 0)
		return;
	if (item >= MAX_VOICECHAT_ITEMS || item < 0)
		return;
	
	//determine which voice chat menu to use
	if (team->commander == clientnum)
	{
		//use the commander voice chat
		menu = GetVoiceMenu(fmt("%s_commander", raceData[team->race].name));
		if (!menu)
		{
			cores.Console_Printf("SV_ClientVoiceChat: no voicechat menu found for %s commander\n", raceData[team->race].name);
			return;
		}
	}
	else
	{
		//normal player
		if (client->ps.unittype == -1)
			return;

		menu = GetVoiceMenu(GetObjectByType(client->ps.unittype)->voiceMenu);
		if (!menu)
			return;
	}

	//we have a valid menu, send the chat

	if (!menu->categories[category].active)
		return;
	if (item >= menu->categories[category].numItems)
		return;

	//don't rely on the client info to be correct on the client, send across the menu name to use
	if (menu->categories[category].items[item].more)
		return;

	if (menu->categories[category].items[item].flags & VOICE_GLOBAL)
	{
		SV_BroadcastMessage(clientnum, fmt("%s %s %i %i", SERVER_VOICECHAT_MSG, menu->name, category, item));
	}
	else
	{
		SV_BroadcastTeamMessage(clientnum, client->info.team, fmt("%s %s %i %i", SERVER_VOICECHAT_MSG, menu->name, category, item));
	}
}



/*==========================

  SV_PlayerClearTarget

  called when a goal has been completed

 ==========================*/

void	SV_PlayerClearTarget(serverObject_t *player, int goal)
{
	client_t *client = player->client;
	if (!client)
		return;

	sl.objects[player->base.index].goal = goal;

	if (!client->waypoint.active)
		return;

	//award loyalty
	client->ps.score.loyalty++;
	sl.teams[player->base.team].orderFollowed = true;

	//send waypoint command
	cores.Server_SendMessage(client->waypoint.clientnum, 
							 sl.teams[player->base.team].commander, 
							 fmt("%s %i", SERVER_WAYPOINT_COMPLETED, player->base.index));
	//send waypoint command to player
	cores.Server_SendMessage(client->waypoint.clientnum, player->base.index, 
							 fmt("%s %i", SERVER_WAYPOINT_COMPLETED, player->base.index));

	client->waypoint.active = false;
}


/*==========================

  SV_PlayerTargetObject

 ==========================*/

void	SV_PlayerTargetObject(client_t *giver, serverObject_t *player, serverObject_t *object, int goal)
{
	int giveridx;
	client_t *client = player->client;
	if (!client)
		return;
	if (!object)
		return;

	if (!giver)
		giveridx = -1;
	else
		giveridx = giver->index;

	if (client->index == object->base.index)
		return;
		
	player->targetType = TARGTYPE_OBJECT;
	player->objectTarget = object;
	player->goal = goal;	
	
	client->waypoint.active = true;
	client->waypoint.time_assigned = sl.gametime;
	client->waypoint.object = true;
	client->waypoint.object_index = object->base.index;
	client->waypoint.object_type = object->base.type;
	client->waypoint.goal = goal;
	client->waypoint.clientnum = giveridx;
	if (giveridx == -1)
		client->waypoint.commander_order = true;
	else
		client->waypoint.commander_order = sl.teams[giver->info.team].commander == giveridx;

	client->stats.ordersGiven++;
	sl.teams[client->info.team].orderGiven = true;

	//send waypoint to commander
	cores.Server_SendMessage(giveridx, 
							 sl.teams[player->base.team].commander, 
							 fmt("%s %i %i %i %i", SERVER_WAYPOINT_OBJECT, client->index, object->base.index, object->base.type, goal));
	//send waypoint to client
	cores.Server_SendMessage(giveridx, 
							  client->index, 
							 fmt("%s %i %i %i %i", SERVER_WAYPOINT_OBJECT, client->index, object->base.index, object->base.type, goal));
}


/*==========================

  SV_PlayerTargetPosition

 ==========================*/

void	SV_PlayerTargetPosition(client_t *giver, serverObject_t *player, float x, float y, int goal)
{
	client_t *client = player->client;
	int giveridx;
	if (!client)
		return;
	if (!giver)
		giveridx = -1;
	else
		giveridx = giver->index;
	
	if (goal == GOAL_NONE)			//special case, cancels the current waypoint
	{
		client->waypoint.active = false;
		cores.Server_SendMessage(giveridx,
								 sl.teams[client->info.team].commander,
								 fmt("%s %i", SERVER_WAYPOINT_CANCEL, client->index));
		cores.Server_SendMessage(giveridx,
								 client->index,
								 fmt("%s %i", SERVER_WAYPOINT_CANCEL, client->index));
		player->goal = GOAL_NONE;

		return;
	}

	player->targetType = TARGTYPE_LOCATION;
	player->posTarget[0] = x;
	player->posTarget[1] = y;
	player->posTarget[2] = 0;	//Z not used
	player->goal = goal;	
	
	client->waypoint.active = true;
	client->waypoint.time_assigned = sl.gametime;
	client->waypoint.object = false;
	M_CopyVec3(player->posTarget, client->waypoint.pos);
	client->waypoint.goal = goal;
	client->waypoint.clientnum = giveridx;
	client->waypoint.commander_order = giveridx == sl.teams[giver->info.team].commander;

	client->stats.ordersGiven++;
	sl.teams[client->info.team].orderGiven = true;


	//send waypoint command to commander
	cores.Server_SendMessage(giveridx, 
							 sl.teams[client->info.team].commander, 
							 fmt("%s %i %f %f %i", SERVER_WAYPOINT_POSITION, client->index, x, y, goal));
	//send waypoint command to player
	cores.Server_SendMessage(giveridx,
							 client->index , 
							 fmt("%s %i %f %f %i", SERVER_WAYPOINT_POSITION, client->index, x, y, goal));
}


/*==========================

  SV_PlayerTargetReached

 ==========================*/

void	SV_PlayerTargetReached(serverObject_t *obj)
{
//	cores.Console_DPrintf("Player %i reached waypoint!\n", obj->base.index);

	SV_PlayerClearTarget(obj, GOAL_NONE);
}


/*==========================

  SV_PlayerAtTarget

 ==========================*/

bool	SV_PlayerAtTarget(serverObject_t *obj)
{
	waypoint_t *waypoint;
	vec3_t *pos;

	waypoint = &sl.clients[obj->base.index].waypoint;

	if (waypoint->object 
		&& !sl.objects[waypoint->object_index].base.active)
	{
		return true;
	}
	else if (waypoint->goal == GOAL_REACH_WAYPOINT)
	{
		if (waypoint->object)
			pos = &sl.objects[waypoint->object_index].base.pos;
		else
			pos = &waypoint->pos;
		if (M_GetDistanceSqVec2(obj->base.pos, (*pos)) 
			< PLAYER_WAYPOINT_DISTANCE_FUDGE_FACTOR)
		{
				return true;
		}
	}
	return false;
}


/*==========================

  SV_UpdateClientWaypoint

 ==========================*/

void	SV_UpdateClientWaypoint(client_t *client)
{
	serverObject_t *obj = client->obj;	

	//find the current waypoint/command, copy goal into the obj->goal field
	if (!client->waypoint.active)
		obj->goal = GOAL_NONE;
	else
		obj->goal = client->waypoint.goal;

	// now handle it appropriately
	if (obj->goal == GOAL_NONE)
	{
		return;
	}
	else if (obj->goal == GOAL_REACH_WAYPOINT)
	{
		if (SV_PlayerAtTarget(obj))
		{
			SV_PlayerClearTarget(obj, GOAL_NONE);
		}
	}
	else if (obj->goal == GOAL_ATTACK_OBJECT)
	{
		if (obj->objectTarget->base.health <= 0)
		{
			SV_PlayerClearTarget(obj, GOAL_NONE);
		}
	}
	else if (obj->goal == GOAL_CONSTRUCT_BUILDING)
	{
		if (obj->objectTarget->base.percentToComplete <= 0)
		{
			SV_PlayerClearTarget(obj, GOAL_NONE);
		}
	}
	else if (obj->goal == GOAL_REPAIR)
	{
		if (obj->objectTarget->adjustedStats.fullhealth == obj->objectTarget->base.health)
		{
			SV_PlayerClearTarget(obj, GOAL_NONE);
		}
	}	
}


/*==========================

  SV_GiveMoney

  Adds currency to a player
  if <tithe> is true, a percent is taken from the total and given to the team

 ==========================*/

void	SV_GiveMoney(int client, int money, bool tithe)
{
	int	toPlayer, toTeam;
	teamInfo_t *team;

	//safety checks
	if (client < 0 || client > MAX_CLIENTS || money <= 0)
		return;

	team = &sl.teams[sl.objects[client].base.team];

	//divide money between player and team
	if (tithe)
	{
		toPlayer = money * sv_tithe.value;
		toTeam = money - toPlayer;
	}
	else
	{
		toPlayer = money;
		toTeam = 0;
	}

	//add money to player
	sl.clients[client].ps.score.money += toPlayer;

	//handle overflow
	if (sl.clients[client].ps.score.money > sv_maxMoney.integer)
	{
		int overflow = sl.clients[client].ps.score.money - sv_maxMoney.integer;

		//subtract the overflow
		toPlayer -= overflow;
		sl.clients[client].ps.score.money = sv_maxMoney.integer;

		if (sv_giveOverflowToTeam.integer)
			toTeam += overflow;
	}

	//add to player's stats
	sl.clients[client].stats.moneyGained += toPlayer;

	//add money to team
	team->resources[raceData[team->race].currency] += toTeam;
}


/*==========================

  SV_IsItemResearched

 ==========================*/

bool	SV_IsItemResearched(int item, int team)
{
	if (sl.gametype != GAMETYPE_RTSS)
		return true;

	if (Tech_IsResearched(item, sl.teams[team].research))
		return true;

	return false;
}


/*==========================

  SV_SpendMoney

 ==========================*/

bool	SV_SpendMoney(int clientnum, int amount)
{
	client_t *client = &sl.clients[clientnum];

	if (amount < 0)
		return false;

	if (sv_ignoreMoney.integer)
		return true;

	if (client->ps.score.money < amount)
		return false;

	client->ps.score.money -= amount;			//take out of the player's money
	client->stats.moneySpent += amount;		//add to the money spent stat

	return true;
}


/*==========================

  SV_GetTeam

 ==========================*/

int	SV_GetTeam(int objnum)
{
	if (objnum < 0 || objnum >= MAX_OBJECTS)
		return 0;

	if (objnum < MAX_CLIENTS)
		return sl.clients[objnum].info.team;
	else
		return sl.objects[objnum].base.team;
}


/*==========================

  SV_GetRace

 ==========================*/

int	SV_GetRace(int objnum)
{
	if (objnum < 0 || objnum >= MAX_OBJECTS)
		return 0;

	return sl.teams[SV_GetTeam(objnum)].race;
}

/*==========================

  SV_IsOfficer

 ==========================*/

bool	SV_IsOfficer(int objnum)
{
	int index;
	int team = SV_GetTeam(objnum);

	if (team < 1)
		return false;

	if (objnum < 0 || objnum >= MAX_CLIENTS)
		return false;

	for (index = 0; index < MAX_OFFICERS; index++)
	{
		if (sl.teams[team].officers[index] == objnum)
			return true;
	}

	return false;
}


/*==========================

  SV_SpawnClient

  Set up clients here, this is called every time they emerge from a structure

 ==========================*/

extern cvar_t	sv_xp_survival_interval;

void	SV_SpawnClient(int clientnum, vec3_t pos, vec3_t angle)
{	
	client_t *client = &sl.clients[clientnum];
	objectData_t *unit = &objData[client->ps.unittype];
	serverObject_t *obj = client->obj;
	int	index;
	
	if (unit->objclass != OBJCLASS_UNIT)
		return;

	//set up callbacks for waypoint system
	obj->targetPosition = SV_PlayerTargetPosition;
	obj->targetObject = SV_PlayerTargetObject;
	obj->clearTarget = SV_PlayerClearTarget;

	SV_SetClientStatus(client->index, STATUS_PLAYER);

	SV_AdjustStatsForCurrentLevel(obj, client->ps.score.level);
	client->ps.fullhealth = obj->adjustedStats.fullhealth;
	client->ps.maxstamina = obj->adjustedStats.maxStamina;
		
	//if the client was dead, give them a new life
	//otherwise we assume they were just in a garrison or command center, 
	//and they should retain their current health
	if (client->ps.health <= 0)
	{
		client->ps.health = client->ps.fullhealth;
	}

	M_CopyVec3(pos, client->ps.pos);

	//set physics mode
	if (unit->isVehicle)
	{
		client->ps.phys_mode = PHYSMODE_DRIVE;
		client->driveYaw = 0;
	}
	else
	{
		client->ps.phys_mode = PHYSMODE_WALK;
	}

	//add the spawn event
	SV_Phys_AddEvent(obj, EVENT_SPAWN, 0, 0);
	
	client->ps.flags &= ~PS_INSIDE_TRANSPORT;
	client->ps.flags &= ~PS_CHASECAM;
	client->ps.flags &= ~PS_JUMP_LOCKED;
	client->ps.flags &= ~PS_USE_LOCKED;
	client->ps.flags &= ~PS_ATTACK_LOCKED;
	client->ps.flags &= ~PS_BLOCK_LOCKED;
	client->ps.flags &= ~PS_NOT_IN_WORLD;	

	M_ClearVec3(client->ps.velocity);

	//set viewing angles
	M_CopyVec3(angle, client->ps.angle);
	client->ps.flags |= PS_EXTERNAL_ANGLES;

	//set stamina to full
	client->ps.stamina = client->ps.maxstamina;

	//give client the base ammo for anything they are carrying
	for (index = 0; index < MAX_INVENTORY; index++)
	{
		if (client->ps.inventory[index])
		{
			if (!IsWeaponType(client->ps.inventory[index]))
				continue;

			if (GetObjectByType(client->ps.inventory[index])->ammoMax >= 0)
				client->ps.ammo[index] = GetObjectByType(client->ps.inventory[index])->ammoStart;
			else
				client->ps.ammo[index] = -1;
		}
		else
			client->ps.ammo[index] = -1;
	}

	client->ps.mana = unit->maxMana;

	//clear their states
	for (index = 0; index < MAX_STATE_SLOTS; index++)
	{
		if (client->ps.stateExpireTimes[index] == -1)
		{
			client->ps.stateExpireTimes[index] = 0;
			client->ps.states[index] = 0;
		}
	}

	//apply the officer state if necessary
	for (index = 0; index < MAX_OFFICERS; index++)
	{
		if (sl.teams[SV_GetTeam(clientnum)].officers[index] == clientnum)
			SV_ApplyStateToObject(sl.teams[SV_GetTeam(clientnum)].commander, clientnum, raceData[SV_GetRace(clientnum)].officerState, -1);
	}

	//clear the corpse inventory
	client->corpseUnitType = 0;
	memset(client->corpseInventory, 0, sizeof(client->corpseInventory));
	memset(client->corpseAmmo, 0, sizeof(client->corpseAmmo));

	//give them some invincibility to prevent spawn killing
	if (!client->alreadySpawned)
		client->ps.invincibleTime = sl.gametime + sv_invincibleTime.integer;

	client->alreadySpawned = true;

	client->ps.item = 0;
	
	client->nextSurvivalReward = sl.gametime + sv_xp_survival_interval.integer;

	client->ps.respawnTime = 0;
	client->requestedRespawn = false;

	//activate the units spawn script, and all the items he's carrying as well
	SV_GameScriptExecute(client->obj, client->obj, client->ps.unittype, GS_ENTRY_SPAWN);
	for (index = 0; index < MAX_INVENTORY; index++)
		SV_GameScriptExecute(client->obj, client->obj, client->ps.inventory[index], GS_ENTRY_SPAWN);

	SV_UpdateClientObject(clientnum);
}

/*==========================

  SV_BroadcastObituary

 ==========================*/

void	SV_BroadcastObituary(serverObject_t *attacker, client_t *target, const char *explanation)
{
	SV_BroadcastNotice(NOTICE_OBITUARY, 0, fmt("%i %i %s", attacker->base.index, target->index, explanation));
}


bool	_isVowel(char c)
{
	switch(c)
	{
	case 'a': case 'A':
	case 'e': case 'E':
	case 'i': case 'I':
	case 'o': case 'O':
	case 'u': case 'U':
		return true;
	default:
		return false;
	}
}

/*==========================

  SV_NPCKilledPlayer

 ==========================*/

void	SV_NPCKilledPlayer(serverObject_t *attacker, client_t *target)
{
	//SV_BroadcastNotice(NOTICE_OBITUARY, 0, fmt("%s was killed by a %s", target->info.name, GetObjectByType(attacker->base.type)->description));
	if (target->info.clan_id)
		SV_BroadcastObituary(attacker, target, fmt("^clan %i^%s was killed by %s %s", 
			target->info.clan_id,
			target->info.name,
			_isVowel(GetObjectByType(attacker->base.type)->description[0]) ? "an" : "a",
			GetObjectByType(attacker->base.type)->description));
	else
		SV_BroadcastObituary(attacker, target, fmt("%s was killed by %s %s", 
			target->info.name,
			_isVowel(GetObjectByType(attacker->base.type)->description[0]) ? "an" : "a",
			GetObjectByType(attacker->base.type)->description));
}


/*==========================

  SV_PlayerKilledPlayer

 ==========================*/

extern cvar_t sv_fraglimit;
extern cvar_t sv_xp_honor;
extern cvar_t sv_xp_skill;
extern cvar_t sv_xp_hero;
extern cvar_t sv_xp_legend;

void	SV_PlayerKilledPlayer(client_t *attacker, client_t *target, int weapon)
{
	int score;

	if (attacker == target)				//suicide
		score = 0;
	else if (attacker->info.team == target->info.team)
		score = -1;						//team kill
	else
	{
		score = 1;
		attacker->stats.killsSinceRespawn++;
		switch(attacker->stats.killsSinceRespawn)
		{
			case 3:
				SV_BroadcastNotice(NOTICE_3_KILLS, attacker->index, "");
				SV_AddExperience(attacker, EXPERIENCE_BONUS, sv_xp_honor.integer, 1.0);
				break;
			case 5:
				SV_BroadcastNotice(NOTICE_5_KILLS, attacker->index, "");
				SV_AddExperience(attacker, EXPERIENCE_BONUS, sv_xp_skill.integer, 1.0);
				break;
			case 10:
				SV_BroadcastNotice(NOTICE_HERO, attacker->index, "");
				SV_AddExperience(attacker, EXPERIENCE_BONUS, sv_xp_hero.integer, 1.0);
				break;
			case 15:
				SV_BroadcastNotice(NOTICE_LEGEND, attacker->index, "");
				SV_AddExperience(attacker, EXPERIENCE_BONUS, sv_xp_legend.integer, 1.0);
				break;
			default:
				break;
		}		
		if (GetObjectByType(target->obj->base.type)->isSiegeWeapon)
			SV_AddExperience(attacker, EXPERIENCE_SIEGE_KILL, 0, GetObjectByType(target->obj->base.type)->expMult);
		else
			SV_AddExperience(attacker, EXPERIENCE_PLAYER_KILL, target->ps.score.level, GetObjectByType(target->obj->base.type)->expMult);
	}

	attacker->ps.score.kills += score;

	if (attacker != target)
	{
		if (attacker->info.team != target->info.team)
		{
			vec3_t dist;

			//gain reward set in config file
			if (sv_goodieBags.integer)
				SV_SpawnGoodieBag(target->obj, target->ps.pos, attacker);
			else
				SV_GiveMoney(attacker->index, GetObjectByType(target->ps.unittype)->killGoldReward, true);
			
			if (weapon < 0)
			{
				SV_BroadcastObituary(attacker->obj, target, 
						fmt("%s killed %s with %s %s", 
								attacker->info.clan_id ? 
									fmt("^clan %i^%s", attacker->info.clan_id, attacker->info.name) : attacker->info.name,
								target->info.clan_id ? 
									fmt("^clan %i^%s", target->info.clan_id, target->info.name) : target->info.name,
								_isVowel(stateData[-weapon].name[0]) ? "an" : "a",
								stateData[-weapon].name));
			}
			else
			{
				SV_BroadcastObituary(attacker->obj, target, 
						fmt("%s killed %s with %s %s", 
								attacker->info.clan_id ? 
									fmt("^clan %i^%s", attacker->info.clan_id, attacker->info.name) : attacker->info.name,
								target->info.clan_id ? 
									fmt("^clan %i^%s", target->info.clan_id, target->info.name) : target->info.name,
								weapon ? (_isVowel(objData[weapon].description[0]) ? "an" : "a") : "a",
								weapon ? objData[weapon].description : "melee attack"));
			}

			M_SubVec3(target->ps.pos, attacker->ps.pos, dist);
			//was the attacker far away from the target?  if so, tell him how skillful he is!
			if (M_DotProduct(dist, dist) >= sv_skillfullKillDist.value * sv_skillfullKillDist.value &&
				GetObjectByType(weapon)->objclass == OBJCLASS_WEAPON)
				SV_SendNotice(attacker->index, NOTICE_SKILLFUL, 0, "");
		}
		else
		{
			SV_BroadcastObituary(attacker->obj, target, 
				fmt("^rTEAM KILL: ^w%s killed %s", 
						attacker->info.clan_id ? 
							fmt("^clan %i^%s", attacker->info.clan_id, attacker->info.name) : attacker->info.name,
						target->info.clan_id ? 
							fmt("^clan %i^%s", target->info.clan_id, target->info.name) : target->info.name));
		}
	}
	else
	{
		if (attacker->info.clan_id)
			SV_BroadcastObituary(attacker->obj, target, fmt("^clan %i^%s made the ultimate sacrifice", attacker->info.clan_id, attacker->info.name));		
		else
			SV_BroadcastObituary(attacker->obj, target, fmt("%s made the ultimate sacrifice", attacker->info.name));		
	}

	if (sl.gametype == GAMETYPE_DEATHMATCH)
	{
		if (attacker->ps.score.kills >= sv_fraglimit.integer)
		{
			SV_EndGame(attacker->index);
		}
	}
}


/*==========================

  SV_KillClient

  kill the player and unlink them from the world, reset states and set a respawn time  

 ==========================*/

extern cvar_t sv_useRespawnWindow;
extern cvar_t sv_respawnTime;
extern cvar_t sv_xp_die;

void	SV_KillClient(client_t *client, serverObject_t *killer, vec3_t damageSource, int weapon, int attackDamage, int damageFlags)
{
	//teamInfo_t *team = &sl.teams[client->info.team];
	int i, lvl = client->ps.score.level;

	client->ps.animState = AS_DEATH_GENERIC;
	client->ps.animState2 = 0;
	client->ps.weaponState = 0;
	client->ps.health = 0;
	client->obj->base.health = 0;
	
	client->ps.phys_mode = PHYSMODE_DEAD;
	client->ps.item = 0;
	client->ps.stunFlags = 0;
	client->ps.stunTime = 0;
	client->ps.attackFlags = 0;
	client->ps.wpStateStartTime = 0;
	client->ps.overheatCounter = 0;	
	client->ps.score.deaths++;
	client->stats.killsSinceRespawn = 0;

	//set death velocity
	if (client->ps.flags & PS_INSIDE_TRANSPORT)
	{
		//special case if inside transport vehicle
		//give a random death velocity
		M_SetVec3(client->ps.velocity,
					M_Randnum(-300,300),
					M_Randnum(-300,300),
					M_Randnum(100,600)
					);
	}

	client->alreadySpawned = false;

	//apply exp penalty
	SV_SubtractExperience(client, sv_xp_die.integer);

	//save the corpse info
	client->corpseUnitType = client->ps.unittype;
	memcpy(client->corpseInventory, client->ps.inventory, sizeof(client->corpseInventory));
	memcpy(client->corpseAmmo, client->ps.ammo, sizeof(client->corpseAmmo));

	//a client loses all the items he was carrying when he dies
	for (i = 0; i < MAX_INVENTORY; i++)
	{
		client->ps.inventory[i] = 0;
		client->ps.ammo[i] = 0;
	}

	//lose all resources carried
	/*for (i = 0; i < MAX_RESOURCE_TYPES; i++)
	{
		client->obj->resources[i] = 0;
	}*/

	//destroy client's linked object, if it exists
	if (client->obj->link)
	{
		if (client->obj->link->kill)
			client->obj->link->kill(client->obj->link, NULL, client->obj->link->base.pos, 0, 0);
	}

	memset(client->ps.states, 0, sizeof(client->ps.states));
	memset(client->ps.stateExpireTimes, 0, sizeof(client->ps.stateExpireTimes));

	if (killer->client)
		SV_PlayerKilledPlayer(killer->client, client, weapon);
	else
		SV_NPCKilledPlayer(killer, client);

	Phys_AddEvent(&client->ps, EVENT_DEATH, SV_EncodePositionOnObject(client->obj, damageSource), (byte)MIN(255, attackDamage));

	if (sl.gametype == GAMETYPE_DEATHMATCH)
		client->ps.respawnTime = sl.gametime + 1000;
#ifdef SAVAGE_DEMO
	else if (sl.status == GAME_STATUS_NORMAL)
	{
		if (cores.Server_IsDemoPlayer(client->index) && cores.Milliseconds() >= client->nextNagTime)
		{
			client->ps.statusMessage |= STATUSMSG_NAG;
			client->nextNagTime = cores.Milliseconds() + 1200000;		
			client->ps.respawnTime = sl.gametime + 60000;			
		}
		else
		{
			int time = sl.teams[client->info.team].nextRespawnWindow - sl.gametime;
			if (time < 1000)
				time = 1000;
			client->ps.respawnTime = sl.gametime + time;
		}
	}
#else
	else if (sv_useRespawnWindow.integer && sl.status == GAME_STATUS_NORMAL)
	{
		int time = sl.teams[client->info.team].nextRespawnWindow - sl.gametime;
		if (time < 1000)
			time = 1000;
		client->ps.respawnTime = sl.gametime + time;
	}
#endif
	else
		client->ps.respawnTime = sl.gametime + 1000;	//no respawn time during warmup

	client->timeOfDeath = sl.gametime;
}

bool	SV_WouldObjectBeBlocked(const vec3_t pos, byte objtype)
{
	traceinfo_t trace;
	vec3_t bmin,bmax;

	M_CopyVec3(GetObjectByType(objtype)->bmin, bmin);
	M_CopyVec3(GetObjectByType(objtype)->bmax, bmax);

	cores.World_TraceBox(&trace, pos, pos, bmin, bmax, 0);
	if (trace.fraction < 1)
		return true;

	return false;
}


/*==========================

  SV_PlayerUnitRequest

  Process a request to change units, return success or failure

 ==========================*/

bool	SV_PlayerUnitRequest(client_t *client, int unittype)
{
	float			healthpercent;
	int				refund = 0, index, team;
	objectData_t	*unitData, *oldUnitData;
	bool			refundable = true;

	team = client->info.team;

	if (!client->info.team)
		return false;

	if (client->ps.health <= 0)
		refundable = false;

	if (client->obj->base.active)		//can't give items on the field
		return false;

	if(client->ps.unittype == unittype)
		return false;

	if (!SV_IsItemResearched(unittype, team) && !g_allUnitsAvailable.integer)
	{
		SV_SendNotice(client->index, NOTICE_GENERAL, 0, "This unit has not been researched by the commander");
		return false;
	}

	unitData = GetObjectByType(unittype);
	oldUnitData = GetObjectByType(client->ps.unittype);

	if (!Tech_HasEntry(unittype) || unitData->objclass == OBJCLASS_NULL)
		return false;

	if (sl.teams[team].race != unitData->race)
		return false;

	//sell back any items that the new unit can not hold
	for (index = 0; index < MAX_INVENTORY; index++)
	{
		//sell if...
		//the new unit can not purchase OR
		//the new unit can't hold the item in it's present slot...
		//unless it's a forced item, which can't be sold
		if (!unitData->canPurchase ||
			!(unitData->allowInventory[index] & (1 << GetObjectByType(client->ps.inventory[index])->objclass)))
		{
			int	count = 0;

			while (client->ps.inventory[index] && client->ps.inventory[index] != GetObjectTypeByName(oldUnitData->forceInventory[index]))
			{
				SV_GiveBackFromClient(client, index);
				count++;

				if (count > 10)
				{
					cores.Console_DPrintf("WARNING: Could not sell back slot %i from client %i\n", index, client->index);
					break;
				}
			}
		}
	}

	//get refund for current unit
	healthpercent = client->ps.health / (float)client->ps.fullhealth;
	if (refundable)
		refund = oldUnitData->playerCost;
	client->ps.score.money += refund;

	if (!SV_SpendMoney(client->index, unitData->playerCost))			//player can't afford item
	{
		//take back the money, cause we didn't end up selling
		client->ps.score.money -= refund;

		if (unitData->playerCost < 0)
			return false;

		SV_SendNotice(client->index, NOTICE_GENERAL, 0, "You cannot afford this unit");
		return false;
	}

	//remove items that were forced onto this unit before switching
	for (index = 0; index < MAX_INVENTORY; index++)
	{
		int		objnum = GetObjectTypeByName(oldUnitData->forceInventory[index]);

		if (objnum == client->ps.inventory[index])
		{
			client->ps.inventory[index] = 0;
			client->ps.ammo[index] = 0;
		}
	}


	client->ps.unittype = unittype;
	client->obj->base.type = unittype;
	SV_AdjustStatsForCurrentLevel(client->obj, client->ps.score.level);
	client->ps.fullhealth = client->obj->adjustedStats.fullhealth;
	client->ps.health = client->ps.fullhealth * healthpercent;

	//set up their inventory
	client->ps.mana = unitData->maxMana;

	for (index = 0; index < MAX_INVENTORY; index++)
	{
		int				objnum = GetObjectTypeByName(unitData->forceInventory[index]);
		objectData_t	*obj = GetObjectByType(objnum);

		if (!objnum)
			continue;

		if (SV_IsItemResearched(objnum, SV_GetTeam(client->index)) || g_allWeaponsAvailable.integer)
		{
			client->ps.inventory[index] = objnum;
			client->ps.ammo[index] = obj->ammoMax >= 0 ? obj->ammoStart : -1;
		}
		else
		{
			client->ps.inventory[index] = 0;
			client->ps.ammo[index] = 0;
		}
	}

	return true;
}


/*==========================

  SV_SelectPlayerUnit

  Changes the player's unit type

 ==========================*/

void SV_SelectPlayerUnit(int clientnum, const char *unitname)
{
	client_t *client;
	byte type = 0;

	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return;

	client = &sl.clients[clientnum];

	if (!client->info.team)
		return;

	type = GetObjectTypeByName(unitname);

	SV_PlayerUnitRequest(client, type);
}


bool	SV_IsValidSpawnObject(int team, int objindex, int clientnum)
{
	serverObject_t	*obj;
	objectData_t	*bld;

	if (objindex < 0 || objindex >= MAX_OBJECTS)
		return false;

	obj = &sl.objects[objindex];
	bld = GetObjectByType(obj->base.type);

	if (obj->base.index == sl.teams[team].command_center)
		return true;

	if (!obj->base.active || obj->base.health <= 0)
		return false;

	if (obj->base.flags & BASEOBJ_UNDER_CONSTRUCTION)
		return false;

	if (obj->base.team != team)
		return false;

	if ((IsBuildingType(obj->base.type) || (IsItemType(obj->base.type) && obj->base.owner == clientnum)) && bld->spawnFrom)
		return true;

	return false;
}

bool	SV_IsValidEntranceObject(int team, int objindex)
{
	serverObject_t	*obj;
	objectData_t	*bld;

	if (objindex < 0 || objindex >= MAX_OBJECTS)
		return false;

	obj = &sl.objects[objindex];
	bld = GetObjectByType(obj->base.type);

	if (obj->base.index == sl.teams[team].command_center)
		return true;

	if (!obj->base.active || obj->base.health <= 0)
		return false;

	if (obj->base.flags & BASEOBJ_UNDER_CONSTRUCTION)
		return false;

	if (obj->base.team != team)
		return false;

	if ((bld->objclass == OBJCLASS_BUILDING) && bld->canEnter)
		return true;

	return false;
}


/*==========================

  SV_SpawnPlayerFrom

  spawn from command center or garrison

 ==========================*/

void SV_SpawnPlayerFrom(client_t *client, int objidx)
{
	serverObject_t *obj;

	if (!client->info.team)
		return;

	if (sl.teams[client->info.team].commander == client->index)
		return;

	if (client->ps.respawnTime)
	{
		//indicate that the client requested the respawn so that they can respawn right away if they already went to the loadout screen
		client->requestedRespawn = true;

		if (client->ps.respawnTime > sl.gametime)
		{
			SV_SetClientStatus(client->index, STATUS_SPECTATE);
			SV_ChaseNextTeammate(client->index);
			return;
		}
	}
	if (client->obj->base.active)
		return;

	//player just hit the "proceed button", wants to spawn from anywhere
	if (objidx == -1)
	{
		teamInfo_t *team = &sl.teams[client->info.team];
		vec3_t point;

		if (sl.gametype != GAMETYPE_RTSS)
		{
			int n;
			float *spawnPoint;
			int spawnidx = rand() % sl.numSpawnPoints;
			
			for (n=0; n<sl.numSpawnPoints; n++)
			{
				spawnPoint = sl.spawnPoints[(n+spawnidx) % sl.numSpawnPoints];

				if (SV_WouldObjectBeBlocked(spawnPoint, client->ps.unittype))
					continue;

				SV_SpawnClient(client->index, spawnPoint, zero_vec);
				return;
			}

			SV_SendNotice(client->index, NOTICE_GENERAL, 0, "All spawn locations are blocked!\n");
		}
		else
		{
			if (team->outpostList)		//has this team created ouposts?  (alternate spawn points)
			{
				SV_SetClientStatus(client->index, STATUS_SPAWNPOINT_SELECT);
			}
			else	//if not, just spawn them from the command center
			{
				if (!SV_GetSpawnPointFromBuilding(&sl.objects[team->command_center], point, client->ps.unittype))
				{					
					SV_SendNotice(client->index, NOTICE_GENERAL, 0, "All spawn locations are blocked!\n");
				}
				else
				{
					vec3_t viewangle = { 0, 0, sl.objects[team->command_center].base.angle[YAW] };

					SV_SpawnClient(client->index, point, viewangle);
				}
			}
		}
		return;
	}

	obj = &sl.objects[objidx];
	
	//check if this is a valid outpost
	if (SV_IsValidSpawnObject(client->info.team, objidx, client->index))
	{
		vec3_t point;
		vec3_t diff;
		float yaw;
		teamInfo_t *enemyTeam = &sl.teams[client->info.team ^ 3];

		if (!SV_GetSpawnPointFromBuilding(obj, point, client->ps.unittype))
		{
			SV_SendNotice(client->index, NOTICE_GENERAL, 0, "All spawnpoints around this object are blocked!");
			return;
		}

		//set them facing the enemy command center
		M_SubVec3(sl.objects[enemyTeam->command_center].base.pos, obj->base.pos, diff);
		yaw = M_GetVec2Angle(diff);

		SV_SpawnClient(client->index, point, vec3(0, 0, yaw));
	}
	else
	{
		SV_SendNotice(client->index, NOTICE_GENERAL, 0, "Cannot spawn from here!");
	}
}


/*==========================

  SV_IsPlayerStateSpecific

  filters out events that aren't relevant to other clients

  ==========================*/

bool	SV_IsPlayerStateSpecific(objEvent_t *event)
{
	switch(event->type)
	{
		case EVENT_QUAKE:
		case EVENT_WEAPON_HIT:
			return true;
		default:
			break;
	}

	return false;
}


/*==========================

  SV_FreeClientObject

  free the object that represented this client and unlink from the world

 ==========================*/

void	SV_FreeClientObject(int clientnum)
{	
	playerState_t *ps = &sl.clients[clientnum].ps;

	cores.World_UnlinkObject(&sl.objects[clientnum].base);
	cores.Server_FreeObject(&sl.objects[clientnum]);
	sl.objects[clientnum].base.active = false;
	ps->flags |= PS_NOT_IN_WORLD;
}



/*==========================

  SV_UpdateClientObject

  updates a baseObject_t representation of the client
  to be sent to other clients to render.

  done once every client input, and once during every server frame

 ==========================*/

void	SV_UpdateClientObject(int clientnum)
{
	int n;
	serverObject_t *obj = &sl.objects[clientnum];
	playerState_t *ps = &sl.clients[clientnum].ps;
	inputState_t *in = &sl.clients[clientnum].input;

	if (ps->status != STATUS_PLAYER || ps->flags & PS_INSIDE_TRANSPORT || ps->flags & PS_NOT_IN_WORLD)
	{
		if (obj->base.active)
		{
			SV_FreeClientObject(clientnum);			
		}		
		//still go through and update the baseobject fields even though we have "freed" the object
		//this needs to be done because functions like SV_DamageTarget() expect base.health to be up to date
	}
	else
	{
		obj->base.active = true;
	}

	obj->base.flags = 0;

	M_CopyVec3(ps->pos, obj->base.pos);

	if (ps->animState == AS_RESURRECTED)
	{
		//while they're in the resurrected state, push anyone that
		//comes inside the bounding box out of the way		

		obj->base.surfaceFlags = SURF_PUSH_ZONE;
	}
	else
	{
		if (ps->health <= 0)
		{
			obj->base.surfaceFlags = SURF_CORPSE;
			if (GetObjectByType(ps->unittype)->revivable)
				obj->base.surfaceFlags |= SURF_REVIVABLE;
		}
		else
		{
			//only update base angles when alive
			obj->base.angle[0] = ps->angle[0];
			obj->base.angle[1] = ps->angle[1];
			obj->base.angle[2] = ps->angle[2];
			obj->base.surfaceFlags = 0;
		}
	}
	
	if (ps->invincibleTime >= sl.gametime)
		obj->base.flags |= BASEOBJ_INVINCIBLE;
	if (ps->flags & PS_COMMANDER_SELECTED)
		obj->base.flags |= BASEOBJ_COMMANDER_SELECTED;
	
	obj->base.exflags = 0;

	//continuous weapon effect
	if (ps->weaponState == AS_WPSTATE_FIRE)
	{
		if (GetObjectByType(ps->inventory[ps->item])->continuous)
			obj->base.exflags |= BASEOBJEX_FIRING_CONTINUOUS;
	}

	obj->base.health = ps->health;
	obj->base.fullhealth = ps->fullhealth;

	obj->base.type = ps->unittype;

	//copy over relevant events
	obj->base.numEvents = 0;
	for (n=0; n<ps->numEvents; n++)
	{
		if (SV_IsPlayerStateSpecific(&ps->events[n]))
			continue;

		memcpy(&obj->base.events[obj->base.numEvents++], &ps->events[n], sizeof(objEvent_t));
	}
	
	obj->base.animState = ps->animState;
	obj->base.animState2 = ps->animState2;
	obj->base.team = sl.clients[clientnum].info.team;
	obj->base.weapon = ps->inventory[ps->item];	

	memcpy(obj->stateExpireTimes, ps->stateExpireTimes, sizeof(obj->stateExpireTimes));
	memcpy(obj->base.states, ps->states, sizeof(obj->base.states));

	if (obj->base.active)
	{
		SetObjectBounds(&obj->base);
		cores.World_LinkObject(&obj->base);
	}
}

bool	SV_TraceBox(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface)
{
	return cores.World_TraceBox(result, start, end, bmin, bmax, ignoreSurface);
}



/*==========================

  SV_ResetInventory

 ==========================*/

void	SV_ResetInventory(client_t *client)
{
	int				index, objtype;
	objectData_t	*unit = GetObjectByType(client->ps.unittype);
	objectData_t	*obj;

	for (index = 0; index < MAX_INVENTORY; index++)
	{
		objtype = GetObjectTypeByName(unit->forceInventory[index]);
		obj = GetObjectByType(objtype);

		if (!obj)
		{
			client->ps.inventory[index] = 0;
			client->ps.ammo[index] = 0;
			continue;
		}

		client->ps.inventory[index] = objtype;
		client->ps.ammo[index] = obj->ammoStart;
	}
}


/*==========================

  SV_SetDefualtUnit

  Searches objects for a unit that is always available and matches the client's race

 ==========================*/

bool	SV_SetDefaultUnit(client_t *client)
{
	int index, unittype = -1;

	//find a suitable unit for the player
	for (index = 0; index < MAX_OBJECT_TYPES; index++)
	{
		if (objData[index].objclass != OBJCLASS_UNIT)
			continue;

		if (!objData[index].alwaysAvailable)
			continue;

		if (objData[index].race != SV_GetRace(client->index))
			continue;

		unittype = index;
	}

	if (unittype == -1)
	{
		client->ps.unittype = 0;
		cores.Console_Printf("******** WARNING ********\nCould not find a suitable unit for client %i\n************************", client->index);
		return false;
	}

	client->ps.unittype = unittype;
	client->obj->base.type = unittype;
	
	SV_AdjustStatsForCurrentLevel(client->obj, client->ps.score.level);
	client->ps.fullhealth = client->obj->adjustedStats.fullhealth;
	client->ps.health = client->ps.fullhealth;

	SV_ResetInventory(client);
	return true;
}


void	SV_SetClientStatus(int clientnum, byte status)
{
	if (status == sl.clients[clientnum].ps.status)
		return;

	sl.clients[clientnum].ps.status = status;
	sl.clients[clientnum].ps.statusMessage = 0;
}

void	SV_PutClientIntoGame(client_t *client)
{				
	if (client->info.team)
	{
		SV_SetDefaultUnit(client);
		SV_SetClientStatus(client->index, STATUS_UNIT_SELECT);
	}
	else
	{
		//they're not on a team
		SV_SendNotice(client->index, NOTICE_GENERAL, 0, "Join a team before entering the game");
	}
}


void	SV_RemoveAttachedItems(client_t *client)
{
	int index;

	for (index = 0; index < sl.lastActiveObject; index++)
	{
		if (!sl.objects[index].base.active ||
			!sl.objects[index].owner ||
			!(sl.objects[index].base.flags & BASEOBJ_ATTACHED_TO_OWNER))
			continue;

		if (sl.objects[index].owner->base.index == client->index)
			SV_FreeObject(sl.objects[index].owner->base.index);
	}
}



/*==========================

  SV_PurgeInventory

  Remove any items with zero ammo, so they won't be replenished

 ==========================*/

void	SV_PurgeInventory(client_t *client)
{
	int	index;

	for (index = 0; index < MAX_INVENTORY; index++)
	{
		if (IsItemType(client->ps.inventory[index]) &&
			client->ps.ammo[index] == 0 &&
			GetObjectByType(client->ps.inventory[index])->ammoMax != -1)
			client->ps.inventory[index] = 0;
	}
}


/*==========================

  SV_UseButton

  Player is attempting to return to the equipment screen by "entering" a building
  OR trying to enter a vehicle transport

 ==========================*/


void	SV_UseButton(client_t *client)
{
	serverObject_t *building;
	objectData_t *buildingData;

	if (client->ps.status != STATUS_PLAYER)
		return;

	if (client->obj->base.health <= 0)
		return;
	
	//exit a vehicle if we are in one
	if (client->ps.flags & PS_INSIDE_TRANSPORT && client->ps.chaseIndex > -1)
	{
		serverObject_t *transport = &sl.objects[client->ps.chaseIndex];
		vec3_t point;
		
		if (SV_GetSpawnPointAroundObject(transport->base.type, transport->base.pos, transport->base.angle, client->ps.unittype, point))
		{
			int n;

			//exit them out of the vehicle
			client->ps.flags &= ~PS_INSIDE_TRANSPORT;
			client->ps.flags &= ~PS_CHASECAM;
			client->ps.chaseIndex = -1;
			for (n = 0; n < transport->numRiders; n++)
			{
				if (transport->riders[n] == client->index)
				{
					memcpy(&transport->riders[n], &transport->riders[n+1], (transport->numRiders - 1) - n);					
					transport->numRiders--;
				}
			}
			SV_SpawnClient(client->index, point, vec3(0,0,transport->base.angle[YAW]));
			return;
		}
		else
		{
			SV_SendNotice(client->index, NOTICE_GENERAL, 0, fmt("The area around the %s is not clear!\n", GetObjectByType(transport->base.type)->description));
			return;
		}
	}
	else
	{	
		//trying to enter a building?
		traceinfo_t trace;
		float dist = 0;
		vec3_t to;
		//trace against the collision surface rather than per poly by setting the bounds to non zero size
		vec3_t smallmin = { -60, -60, -60 };
		vec3_t smallmax = { 40, 40, 40 };

		vec3_t from = { client->ps.pos[0], client->ps.pos[1], client->ps.pos[2] + GetObjectByType(client->ps.unittype)->viewheight };
		M_PointOnLine(from, client->forward, dist, to);
		cores.World_TraceBoxEx(&trace, from, to, smallmin, smallmax, SURF_DYNAMIC|SURF_TERRAIN|SURF_STATIC, client->index);
		if (trace.index < 0 || trace.index >= MAX_OBJECTS)
			return;

		building = &sl.objects[trace.index];
		buildingData = GetObjectByType(building->base.type);

		if (building->base.team != client->info.team)
			return;

		//check for entering buildings first
		if (SV_IsValidEntranceObject(SV_GetTeam(client->index), trace.index))
		{
			if (sl.objects[trace.index].twin)
			{
				vec3_t	point;

				if (!sl.objects[trace.index].twin->base.active ||
					(sl.objects[trace.index].twin->base.flags & BASEOBJ_UNDER_CONSTRUCTION))
					return;

				if(SV_GetSpawnPointFromBuilding(sl.objects[trace.index].twin, point, client->ps.unittype))
				{
					M_CopyVec3(point, sl.objects[client->index].base.pos);
					M_CopyVec3(point, sl.objects[client->index].client->ps.pos);
				}
				return;
			}

			SV_SetClientStatus(client->index, STATUS_UNIT_SELECT);
			SV_RemoveAttachedItems(client);
			SV_PurgeInventory(client);
			return;
		}

		//check for entering a vehicle
		if (buildingData->canRide && building->numRiders < buildingData->maxRiders)
		{
			building->riders[building->numRiders++] = client->index;
			client->ps.chaseIndex = trace.index;
			client->ps.flags |= PS_INSIDE_TRANSPORT | PS_CHASECAM;
			return;
		}
	}
}


bool	SV_Eject(int clientnum)
{
	client_t *client = &sl.clients[clientnum];
	objectData_t *def;

	if (clientnum < 0 || clientnum > MAX_CLIENTS)
		return false;

	if (client->ps.status != STATUS_PLAYER)
		return false;

	if (client->obj->base.health <= 0)
		return false;
	
	//are we a unit that can destroy itself and spawn out?

	def = GetObjectByType(client->ps.unittype);

	if (def->canEject)
	{
		serverObject_t *corpse;

		corpse = SV_GenerateCorpse(client, AS_DEATH_GENERIC);
		SV_Phys_AddEvent(corpse, EVENT_DEATH, 0, 0);

		client->ps.unittype = GetObjectTypeByName(def->ejectUnit);
		if (!client->ps.unittype)
			cores.Console_DPrintf("Couldn't find eject unit type\n");
		
		SV_SpawnClient(client->index, client->ps.pos, client->ps.angle);
		SV_ResetInventory(client);

		//give player some velocity
		client->ps.velocity[0] = client->forward[0] * 50;
		client->ps.velocity[1] = client->forward[1] * 50;
		client->ps.velocity[2] = M_Randnum(100, 200);
		client->ps.flags |= PS_DAMAGE_PUSH;
		client->ps.flags |= PS_USE_LOCKED;

		//give them the correct full health for the unit type
		SV_AdjustStatsForCurrentLevel(client->obj, client->ps.score.level);
		client->ps.health = client->ps.fullhealth;
	}

	return false;
}


/*==========================

  SV_PlayerCorpseFrame

 ==========================*/

extern cvar_t p_gravity;

void	SV_PlayerCorpseFrame(serverObject_t *corpse)
{
	bool onground;
	traceinfo_t groundtrace;
	physicsParams_t p;
	phys_out_t p_out;

	// free the corpse after a little while
	if (sl.gametime >= corpse->nextDecisionTime)
		SV_FreeObject(corpse->base.index);

	Phys_SetupParams(corpse->base.type, corpse->base.team, NULL, corpse->base.pos, corpse->velocity, SV_TraceBox, GetObjectByType, sl.frame_sec, &p);

	onground = SV_Phys_ObjectOnGround(corpse, &groundtrace);

	if (onground && !corpse->flags)
	{
		//damper velocity until we slide to a stop

		int n;

	//	ps->velocity[2] = 0;
		float mult = p.frametime * 300;

		for (n=0; n<3; n++)
		{
			if (corpse->velocity[n] < 0)
			{
				corpse->velocity[n] += mult;
				if (corpse->velocity[n] > 0)
					corpse->velocity[n] = 0;
			}
			else
			{
				corpse->velocity[n] -= mult;
				if (corpse->velocity[n] < 0)
					corpse->velocity[n] = 0;
			}			
		}
	}
	else
	{
		corpse->velocity[2] -= PHYSICS_SCALE * DEFAULT_GRAVITY * p_gravity.value * p.frametime;		
	}
	
	if (ABS(corpse->velocity[0]) < 0.01)
		corpse->velocity[0] = 0;
	if (ABS(corpse->velocity[1]) < 0.01)
		corpse->velocity[1] = 0;
	if (ABS(corpse->velocity[2]) < 0.01)
		corpse->velocity[2] = 0;

	Phys_Slide(&p, &p_out);

	corpse->flags = 0;		//fixme
}



void	SV_HandleClientCollisions(int clientnum)
{
	int index;
	client_t *client = &sl.clients[clientnum];

	
	for (index = 0; index < client->phys_out.num_collisions; index++)
	{
		if (client->phys_out.collisions[index].index >= MAX_OBJECTS)
			continue;
		else
		{
			serverObject_t *touched = &sl.objects[client->phys_out.collisions[index].index];
			objectData_t *def = GetObjectByType(touched->base.type);

			if (def->dropoff || def->commandCenter)
			{
				if (SV_DropoffResources(client->obj, &sl.objects[client->phys_out.collisions[index].index]))
				{
					if (client->obj->goal == GOAL_DROPOFF_RESOURCES)
						SV_PlayerClearTarget(client->obj, GOAL_NONE);
				}
			}
		}
	}
}


/*==========================

  SV_ProcessClientInput

  the main player processing function
  called from the core engine

 ==========================*/

void	SV_ProcessClientInput(int clientnum, inputState_t input)
{
	static int proxycall = false;
	client_t *client = &sl.clients[clientnum];
	int deltatime;
	vec3_t oldvel;//, change;
	int wasInAir;
	objectData_t	*unit;

	if (client->inputRedirect)
	{
		//redirect all our inputs to another client
		proxycall = true;
		SV_ProcessClientInput(client->inputRedirect->index, input);
		proxycall = false;
		if (!sv_simultaneousInputs.integer)
			return;
	}

	if (client->inputHasProxy && !proxycall)
		return;

	if (sl.status == GAME_STATUS_ENDED)
	{
		client->ps.inputSequence = input.sequence;
		return;
	}

	client->input = input;

	unit = &objData[client->obj->base.type];

	if (client->ps.flags & PS_INSIDE_TRANSPORT)
		return;

	if (client->ps.status == STATUS_PLAYER)
	{
		int cheatTime = 1200 * cores.Cvar_GetValue("timescale");
		if (cheatTime < 1200)
			cheatTime = 1200;

		//fixme: add an ApplyForces funcion to phys_ which we can use to apply gravity even if the client didn't send any inputstates

		//go through each input state and advance the playerstate by the delta time between each one.
		//if the total summed up deltatime is longer than a server frame (based on svr_gamefps),
		//then we have to truncate the remaining times so the client doesn't go faster than we want
		//him to

		//unlink us from the collision system so we don't collide against ourself
		cores.World_UnlinkObject(&client->obj->base);

		M_CopyVec3(client->ps.velocity, oldvel);
		wasInAir = !(client->ps.flags & PS_ON_GROUND);

		deltatime = client->input.delta_msec;
		client->physicsTime += deltatime;


		if (client->physicsTime > cheatTime)			//prevent speed cheats
			return;

		if (GetObjectByType(sl.objects[client->index].base.type)->isVehicle)
		{
			client->aimX = client->input.yaw;
			if (client->aimX / (float)(0x7fff) < unit->minAimX)
				client->aimX = unit->minAimX * 0x7fff;
			else if (client->aimX / (float)(0x7fff) > unit->maxAimX)
				client->aimX = unit->maxAimX * 0x7fff;

			client->aimY = client->input.pitch;
			if (client->aimY / (float)(0x7fff) < unit->minAimY)
				client->aimY = unit->minAimY * 0x7fff;
			else if (client->aimY / (float)(0x7fff) > unit->maxAimY)
				client->aimY = unit->maxAimY * 0x7fff;

			client->input.yaw = ANGLE2WORD(client->driveYaw - 360);
			client->input.pitch = 0;
		}
		
		client->ps.attackFlags &= ~PS_ATK_CLEAR_MELEE_HITS;
		Phys_AdvancePlayerState(&client->ps, client->info.team, &client->input, &client->lastInput, SV_TraceBox, GetObjectByType, &client->phys_out);
		//if (M_GetVec2Length(client->ps.intent) > 0)
		//	core.Console_DPrintf("Speed: %0.2f (%0.2f)\n", M_GetVec2Length(client->ps.intent), M_GetVec2Length(client->ps.velocity));
		if (client->ps.attackFlags & PS_ATK_CLEAR_MELEE_HITS)
			memset(client->obj->hitObjects, 0, sizeof(bool) * MAX_OBJECTS);

		HeadingFromAngles(client->ps.angle[PITCH], client->ps.angle[YAW], client->forward, client->right);
		if (GetObjectByType(sl.objects[client->index].base.type)->isVehicle)
		{
			HeadingFromAngles(0, client->ps.angle[YAW], client->forward, client->right);
			client->driveYaw = client->ps.angle[YAW];
		}

		SV_HandleClientCollisions(clientnum);		

		SV_DoOncePerInputStateEvents(client);

		SV_UpdateClientObject(clientnum);	//convert the playerstate into a baseObject to send to other clients, and update collision info

		if (client->ps.animState == AS_RESURRECTED)
		{
			/*
				the client is done resurrecting when
				the following two conditions are met:

				- they have been resurrecting for at least sv_ressurectTime milliseconds
				- their bounding box is not intersecting anything
			*/

			if (sl.gametime >= client->ressurectEndTime)
			{
				traceinfo_t trace;
				cores.World_TraceBoxEx(&trace, client->ps.pos, client->ps.pos, client->obj->base.bmin, client->obj->base.bmax, TRACE_PLAYER_MOVEMENT, client->index);
				if (trace.fraction == 1)
				{
					client->ps.animState = AS_IDLE;
				}
				else
				{
					//give them a little more invincibility time until everything is out of the way
					client->ps.invincibleTime = sl.gametime + 500;
				}
			}
		}


		if (client->ps.health > 0 && client->obj->base.active)
		{
			if (client->input.buttons & B_USE)
			{
				if (!(client->ps.flags & PS_USE_LOCKED))
				{
					SV_UseButton(client);
					client->ps.flags |= PS_USE_LOCKED;
				}
			}
			else
				client->ps.flags &= ~PS_USE_LOCKED;
		}
		else
		{
			//allow them to go into spectate mode
			if (client->input.movement & MOVE_UP && sl.gametime - client->timeOfDeath > 1500)
			{
				if (!(client->ps.flags & PS_JUMP_LOCKED))
				{
					//going into spectate mode gives up their life
					if (SV_WaitingToBeRevived(client))
						SV_RelinquishLife(client);

					SV_SetClientStatus(client->index, STATUS_SPECTATE);
					SV_ChaseNextTeammate(clientnum);

					client->ps.flags |= PS_JUMP_LOCKED;
				}
			}
		}
	}
	else if (client->ps.status == STATUS_SPECTATE)
	{
		if (!client->info.team)
		{
			//free fly spectate mode
			Phys_AdvancePlayerState(&client->ps, 0, &client->input, &client->lastInput, SV_TraceBox, GetObjectByType, &client->phys_out);
		}
		else
		{
			//angle hack, should be handled through Phys_AdvancePlayerState()
			client->ps.angle[PITCH] = WORD2ANGLE(client->input.pitch);
			client->ps.angle[ROLL] = 0;
			client->ps.angle[YAW] = WORD2ANGLE(client->input.yaw);

			if (client->input.movement & MOVE_UP)
			{								
				if (!(client->ps.flags & PS_JUMP_LOCKED))
				{
					SV_ChaseNextTeammate(clientnum);

					client->ps.flags |= PS_JUMP_LOCKED;
				}
			}
			else
				client->ps.flags &= ~PS_JUMP_LOCKED;
		}
	}

	client->lastInput = client->input;
}


/*==========================

  SV_SetControlProxy

  redirect controls from client -> other

  we are only allowed to become a proxy for virtual clients

 ==========================*/

void	SV_SetControlProxy(client_t *client, int othernum)
{
	client_t *other;

	if (othernum < 0 || othernum >= MAX_CLIENTS)
		return;
	if (!core.Cvar_GetValue("svr_allowCheats"))
		return;

	other = &sl.clients[othernum];

	if (!other->isVirtual || !other->active)
		return;

	SV_ReleaseProxy(client);

	other->inputHasProxy = client;
	client->inputRedirect = other;
}


/*==========================

  SV_SetViewProxy

  redirects playerstates from other -> client
  
  we are only allowed to become a proxy for virtual clients

 ==========================*/

void	SV_SetViewProxy(client_t *client, int othernum)
{
	client_t *other;

	if (othernum < 0 || othernum >= MAX_CLIENTS)
		return;
	if (!core.Cvar_GetValue("svr_allowCheats"))
		return;

	other = &sl.clients[othernum];

	if (!other->isVirtual || !other->active)
		return;

	SV_ReleaseProxy(client);

	client->psHasProxy = other;
	other->psRedirect = client;
}


/*==========================

  SV_SetFullProxy

  sets us as a view and control proxy in one call

 ==========================*/

void	SV_SetFullProxy(client_t *client, int othernum)
{
	client_t *other;

	if (othernum < 0 || othernum >= MAX_CLIENTS)
		return;
	if (!core.Cvar_GetValue("svr_allowCheats"))
		return;
	if (othernum == client->index)
	{
		SV_ReleaseProxy(client);
		return;
	}

	other = &sl.clients[othernum];

	if (!other->isVirtual || !other->active)
		return;

	SV_ReleaseProxy(client);

	other->inputHasProxy = client;
	client->inputRedirect = other;
	client->psHasProxy = other;
	other->psRedirect = client;	
}


/*==========================

  SV_ReleaseProxy

 ==========================*/

void	SV_ReleaseProxy(client_t *client)
{
	if (!core.Cvar_GetValue("svr_allowCheats"))
		return;

	if (client->inputRedirect)
	{
		client->inputRedirect->inputHasProxy = NULL;
		client->inputRedirect = NULL;
	}
	if (client->inputHasProxy)
	{
		client->inputHasProxy->inputRedirect = NULL;
		client->inputHasProxy = NULL;
	}
	if (client->psHasProxy)
	{
		client->psHasProxy->psRedirect = NULL;
		client->psHasProxy = NULL;
	}
	if (client->psRedirect)
	{
		client->psRedirect->psHasProxy = NULL;
		client->psRedirect = NULL;
	}
}


/*==========================
  
  SV_FilterPredictedEvents

  ==========================*/

void	SV_FilterPredictedEvents(playerState_t *ps)
{
#if 0
	int n;
	objEvent_t newevents[MAX_OBJECT_EVENTS];
	int numnew = 0, max = 0;

	max = ps->numEvents;
	if (ps->numEvents > MAX_OBJECT_EVENTS)
	{
		cores.Console_Printf("Warning!  This playerState claims to have %i events!\n", ps->numEvents);
		max = MAX_OBJECT_EVENTS;
	}
	
	for (n=0; n < max; n++)
	{
		objEvent_t *ev = &ps->events[n];

		switch(ev->type)
		{
			case EVENT_JUMP:
			case EVENT_JUMP_LAND:
			case EVENT_WEAPON_FIRE:
				continue;
			default:
				memcpy(&newevents[numnew], &ps->events[n], sizeof(objEvent_t));
				numnew++;
				break;
		}
	}
	
	if (numnew != ps->numEvents)
	{
		memcpy(ps->events, newevents, numnew * sizeof(objEvent_t));
		ps->numEvents = numnew;
	}
#endif
}


/*==========================

  SV_RelinquishLife

  remove the client from the world and put a corpse in his place
  
  set his status to STATUS_UNIT_SELECT

 ==========================*/

void	SV_RelinquishLife(client_t *client)
{
	if (client->ps.status != STATUS_PLAYER)
		return;
	if (!client->obj->base.active)
		return;

	SV_GenerateCorpse(client, client->ps.animState);
	SV_FreeClientObject(client->index);
	SV_SetDefaultUnit(client);	
}


/*==========================

  SV_CheckRespawnTimer

 ==========================*/

void	SV_CheckRespawnTimer(client_t *client)
{
	if (!client->ps.respawnTime)
		return;
	if (sl.gametime < client->ps.respawnTime)
	{
		if (!client->ps.statusMessage)
		{
			client->ps.statusMessage |= STATUSMSG_WAITING_TO_RESPAWN;
			client->ps.statusMessage &= ~STATUSMSG_ATTACK_TO_RESPAWN;
		}
		return;
	}
	if (sl.status == GAME_STATUS_ENDED)
		return;

	if (client->requestedRespawn)
	{
		SV_SpawnPlayerFrom(client, -1);
	}
	else
	{		
		client->ps.statusMessage |= STATUSMSG_ATTACK_TO_RESPAWN;
		client->ps.statusMessage &= ~STATUSMSG_WAITING_TO_RESPAWN;

		if (client->input.buttons & B_ATTACK)
		{
			SV_RelinquishLife(client);
			SV_SetClientStatus(client->index, STATUS_UNIT_SELECT);			
		}
		else
		{
			return;		//wait until they hit attack
		}
	}

	client->ps.respawnTime = 0;
}

void	SV_CheckChaseCam(client_t *client)
{
	serverObject_t *chaseObj;

	//make sure the chase object is still alive and figure out what to do if it isn't	

	if (!(client->ps.flags & PS_CHASECAM))
		return;

	chaseObj = &sl.objects[client->ps.chaseIndex];

	if (chaseObj->base.active)
	{
		M_CopyVec3(chaseObj->base.pos, client->ps.chasePos);
		client->ps.chasePos[2] += GetObjectByType(chaseObj->base.type)->viewheight;
		return;
	}

	if (client->ps.status == STATUS_PLAYER)
	{
		//if we're still in the world then just take us out of chasecam mode
		client->ps.flags &= ~PS_CHASECAM;		
	}
	else if (client->ps.status == STATUS_SPECTATE)
	{
		SV_ChaseNextTeammate(client->index);
	}
}


/*==========================

  SV_AdvanceClients

  ==========================*/

void	SV_AdvanceClients()
{
	int n;

	for (n = 0; n < MAX_CLIENTS; n++)
	{
		client_t *client = &sl.clients[n];

		if (!sl.clients[n].active)
			continue;

		if (client->isVirtual)
		{
			//give an input to ProcessInput
			inputState_t in;
			memset(&in, 0, sizeof(in));
			in.gametime = sl.gametime;
			in.delta_msec = sl.frame_msec;
			SV_ProcessClientInput(n, in);
		}

		if (sl.gametime >= client->physicsClearTime)
		{
			client->physicsTime = 0;
			client->physicsClearTime = sl.gametime + 1000;
		}

		if (sl.status == GAME_STATUS_ENDED)
		{
			//clear the exclusion list
			memset(client->exclusionList, 0, sizeof(exclusionList_t));
			//set position to the global position determined at the end of the game
			M_CopyVec3(sl.globalPos, client->ps.pos);

			SV_UpdateClientObject(n);
		}
		else
		{
			if (sl.teams[client->info.team].commander != n)
			{
				SV_CheckChaseCam(client);
				//check respawn timer
				SV_CheckRespawnTimer(client);
			}
			//cull objects based on fog of war
			SV_SetExclusionList(n);
			//update client's waypoint goal
			SV_UpdateClientWaypoint(client);
			//perform events
			SV_DoOncePerFrameEvents(client);
			//convert the playerstate into a baseObject to send to other clients
			SV_UpdateClientObject(n);
		}

		SV_FilterPredictedEvents(&client->ps);

		//get ping
		if (sl.gametime > client->nextPingTime)
		{
			client->ps.score.ping = cores.Server_GetClientPing(n);
			client->nextPingTime = sl.gametime + 500;
		}

		//copy the playerstate to the core
		//this is guaranteed to happen after all SV_ProcessClientInputs() callbacks
		if (client->psRedirect)
		{
			cores.Server_UpdatePlayerState(client->psRedirect->index, &client->ps);
			memcpy(client->psRedirect->exclusionList, client->exclusionList, sizeof(exclusionList_t));
		}
		else
		{
			if (!client->psHasProxy)
			{
				if (sv_fullSpectateTest.integer
					&& client->ps.flags & PS_CHASECAM 
					&& client->ps.chaseIndex < MAX_CLIENTS 
					&& client->ps.chaseIndex >= 0
					&& client->ps.chaseIndex != n)
				{
					playerState_t spectatePS = sl.clients[client->ps.chaseIndex].ps;

					//retain the status message
					spectatePS.statusMessage = client->ps.statusMessage;

					cores.Server_UpdatePlayerState(n, &spectatePS);
				}	
				else
				{
					cores.Server_UpdatePlayerState(n, &client->ps);
				}
			}
		}

		client->ps.numEvents = 0;		//clear events so they don't get sent again
		client->ps.flags &= ~PS_EXTERNAL_ANGLES; //unlock the external angles
	}

	SV_RefreshClientScores();
}


bool	SV_WaitingToBeRevived(client_t *client)
{
	if (client->ps.health > 0)
		return false;
	if (client->ps.status != STATUS_PLAYER)
		return false;
	if (!client->obj->base.active)
		return false;

	return true;
}

/*==========================

  SV_ReviveCorpse

 ==========================*/

bool	SV_ReviveCorpse(int corpsenum, float percent)
{
	serverObject_t	*corpse;
	int	unittype, ammo[MAX_INVENTORY], inventory[MAX_INVENTORY];

	if (corpsenum < 0 || corpsenum >= MAX_CLIENTS)
		return false;

	corpse = &sl.objects[corpsenum];

	if (!corpse->base.active)
		return false;

	if (!IsUnitType(corpse->base.type) || !GetObjectByType(corpse->base.type)->revivable)
		return false;

	if (corpse->base.health > 0)
		return false;

	unittype = 0;
	
	if (!unittype)
	{
		unittype = sl.clients[corpsenum].corpseUnitType;
		memcpy(inventory, sl.clients[corpsenum].corpseInventory, sizeof(inventory));
		memcpy(ammo, sl.clients[corpsenum].corpseAmmo, sizeof(ammo));
	}

	//reviving a client still atached to their body
	if (sl.objects[corpsenum].client)
	{
		client_t	*client = sl.objects[corpsenum].client;

		client->ps.animState = AS_RESURRECTED;
		client->ps.animState2 = AS_IDLE;
		client->ps.weaponState = 0;
		client->ps.health = client->obj->adjustedStats.fullhealth * percent;
		client->obj->base.health = client->ps.health;
	
		client->ps.phys_mode = PHYSMODE_WALK;
		client->ps.item = 0;
		client->ps.stunFlags = 0;
		client->ps.stunTime = 0;
		client->ps.attackFlags = 0;
		client->ps.wpStateStartTime = 0;
		client->ps.overheatCounter = 0;	
		client->stats.killsSinceRespawn = 0;
		client->ps.invincibleTime = sl.gametime + sv_ressurectTime.integer + 500;
		client->ressurectEndTime = sl.gametime + sv_ressurectTime.integer;
		client->ps.respawnTime = 0;
		client->ps.flags &= ~PS_CHASECAM;
		client->ps.statusMessage = 0;
		
		M_ClearVec3(client->ps.velocity);

		client->ps.unittype = unittype;
		memcpy(client->ps.inventory, inventory, sizeof(inventory));
		memcpy(client->ps.ammo, ammo, sizeof(ammo));

		Phys_AddEvent(&client->ps, EVENT_RESURRECTED, 0, 0);

		SV_UpdateClientObject(client->index);
	}

	return true;
}


/*==========================

  SV_ReviveClient_cmd

 ==========================*/

void	SV_ReviveClient_cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	SV_ReviveCorpse(atoi(argv[0]), .5);
}


/*==========================

  SV_InitClients

 ==========================*/

void	SV_InitClients()
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		sl.clients[i].active = false;
		sl.clients[i].info.team = TEAM_UNDECIDED;		
	}
	
	cores.Cmd_Register("revive",	SV_ReviveClient_cmd);

	cores.Cvar_Register(&spawntest_x);
	cores.Cvar_Register(&spawntest_y);
	cores.Cvar_Register(&sv_maxMoney);
	cores.Cvar_Register(&sv_tithe);
	cores.Cvar_Register(&sv_invincibleTime);
	cores.Cvar_Register(&sv_ressurectTime);
	cores.Cvar_Register(&sv_skillfullKillDist);	
	cores.Cvar_Register(&sv_autoRespawn);
	cores.Cvar_Register(&sv_simultaneousInputs);
	cores.Cvar_Register(&sv_ignoreMoney);
	cores.Cvar_Register(&sv_fullSpectateTest);
}
