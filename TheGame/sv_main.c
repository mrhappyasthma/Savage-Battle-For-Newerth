// (C) 2001 S2 Games

// sv_main.c

#include "server_game.h"

serverLocal_t sl = { 0 };
heldServerData_t held;
coreAPI_server_t cores;


cvar_t sv_allowGuestReferee =				{ "sv_allowGuestReferee", "1", CVAR_SERVERINFO };
cvar_t sv_refereePassword	=				{ "sv_refereePassword", "" };					//must be set to allow passworded refs
cvar_t sv_minPlayers =						{ "sv_minPlayers", "2" };						//minimum players before setup countdown starts
cvar_t sv_nextStatus =						{ "sv_nextStatus", "1", CVAR_READONLY };		//1 == GAME_STATUS_SETUP
cvar_t sv_respawnNPCs =						{ "sv_respawnNPCs",			"1" };
cvar_t sv_respawnNPCInterval =				{ "sv_respawnNPCInterval", "40000" };
cvar_t sv_fastTech =						{ "sv_fastTech", "0", CVAR_CHEAT };
cvar_t sv_hitStructureRewardScale =			{ "sv_hitStructureRewardScale", "2", CVAR_CHEAT };
cvar_t sv_rangedHitStructureRewardScale =	{ "sv_rangedHitStructureRewardScale", "0.2", CVAR_CHEAT };
cvar_t sv_unitAngleLerp =					{ "sv_unitAngleLerp", "4", CVAR_CHEAT };
cvar_t sv_aiIgnoreSlope =					{ "sv_aiIgnoreSlope", "1", CVAR_CHEAT };
cvar_t sv_aiPredict =						{ "sv_aiPredict", "0", CVAR_CHEAT };
cvar_t sv_startingTeamStone =				{ "sv_startingTeamStone", "1500", CVAR_VALUERANGE, 0, 10000 };
cvar_t sv_startingTeamGold =				{ "sv_startingTeamGold", "1500", CVAR_VALUERANGE, 0, 10000 };
cvar_t sv_writeScores =						{ "sv_writeScores", "0" };
cvar_t sv_repairCost =						{ "sv_repairCost", "1", CVAR_CHEAT };
cvar_t sv_npcs =							{ "sv_npcs", "1" };
cvar_t sv_autojoin =						{ "sv_autojoin", "1" };
cvar_t sv_buildingInitialHealthPercent =	{ "sv_buildingInitialHealthPercent", "0.05", CVAR_CHEAT };
cvar_t sv_placementLeniency =				{ "sv_placementLeniency", "20", CVAR_CHEAT };
cvar_t sv_gametype =						{ "sv_gametype", "RTSS" };		//can be RTSS or DM
cvar_t sv_timeLimit =						{ "sv_timeLimit", "3600000" };	//60 minutes
cvar_t sv_easterEggs =						{ "sv_easterEggs",	"0" };
cvar_t sv_activeEasterEggs =				{ "sv_activeEasterEggs",	"0",	CVAR_READONLY | CVAR_SERVERINFO };
cvar_t sv_clientConnectMoney =				{ "sv_clientConnectMoney",	"1000", CVAR_VALUERANGE, 0, 10000 };
cvar_t sv_demolishInterval =				{ "sv_demolishInterval",	"10000", CVAR_CHEAT };
cvar_t sv_motd1 =							{ "sv_motd1", "Welcome to Savage!", CVAR_SAVECONFIG };
cvar_t sv_motd2 =							{ "sv_motd2", "", CVAR_SAVECONFIG };
cvar_t sv_motd3 =							{ "sv_motd3", "", CVAR_SAVECONFIG };
cvar_t sv_motd4 =							{ "sv_motd4", "", CVAR_SAVECONFIG };

cvar_t	sv_serverNotes =			{ "sv_serverNotes",	"" };
#ifdef SAVAGE_DEMO
cvar_t	sv_team1race =				{ "sv_team1race",	"human",	CVAR_READONLY };
cvar_t	sv_team2race =				{ "sv_team2race",	"human",	CVAR_READONLY };
#else	//SAVAGE_DEMO
cvar_t	sv_team1race =				{ "sv_team1race",	"human" };
cvar_t	sv_team2race =				{ "sv_team2race",	"human" };
#endif	//SAVAGE_DEMO

cvar_t	sv_todSpeed = 				{ "sv_todSpeed",				"0.003", CVAR_VALUERANGE, 0, 0.05 };
cvar_t	sv_todStart = 				{ "sv_todStart",				"500", CVAR_VALUERANGE, 0, 1440 };

cvar_t sv_chatFloodInterval =		{ "sv_chatFloodInterval",		"5" };
cvar_t sv_chatFloodCount =			{ "sv_chatFloodCount",			"10" };
cvar_t sv_chatFloodPenaltyTime =	{ "sv_chatFloodPenaltyTime",	"10" };

cvar_t sv_doSetup =					{ "sv_doSetup", "1" };
cvar_t sv_doWarmup =				{ "sv_doWarmup", "0" };
cvar_t sv_warmupTime =				{ "sv_warmupTime", "15000", CVAR_VALUERANGE, 5000, 120000 };
cvar_t sv_setupTime =				{ "sv_setupTime", "120000", CVAR_VALUERANGE, 5000, 240000 };
cvar_t sv_endGameTime =				{ "sv_endGameTime", "60000", CVAR_VALUERANGE, 10000, 240000};

cvar_t sv_nextMap =					{ "sv_nextMap",		"eden" };
cvar_t sv_nextMapCmd =				{ "sv_nextMapCmd",	"" };
cvar_t sv_playerChosenMap =			{ "sv_playerChosenMap", "eden" };

cvar_t sv_officerCommandRadius =	{ "sv_officerCommandRadius", "600", CVAR_CHEAT };

cvar_t sv_readyPercent =			{ "sv_readyPercent", "0.60", CVAR_VALUERANGE, 0.01, 1.0 };

cvar_t sv_debug =					{ "sv_debug", "0" };
cvar_t sv_debugBuilding =			{ "sv_debugBuilding", "1" };

cvar_t sv_respawnMultiplier =	{ "sv_respawnMultiplier", "1", CVAR_VALUERANGE, 1, 10 };
cvar_t sv_newPatchAvailable =	{ "sv_newPatchAvailable", "0" };
cvar_t sv_autoPatchMyServer =	{ "sv_autoPatchMyServer", "1" };
cvar_t sv_showClanAbbrev =		{ "sv_showClanAbbrev", "0" };

extern cvar_t sv_teamDamage;
extern cvar_t p_speed;
extern cvar_t p_gravity;
extern cvar_t p_jumpheight;
extern cvar_t sv_balancedTeams;

char connectingPlayerInformation[4096] = {0};
char clanInformation[4096] = {0};

int	lastHeartbeatTime = 0;

byte numPeonNames;

void	SV_ClientEcho(int clientnum, const char *msg)
{
	cores.Server_SendMessage(-1, clientnum, fmt("echo %s", msg));
}

extern void SV_AI_Destroy(struct ai_s* ai);

/*==========================

  SV_HoldPersistentData

  save some data that can be referenced after a server restart

 ==========================*/

void	SV_HoldPersistentData()
{
	int n;

	memset(&held, 0, sizeof(held));

	//write all data that needs to be saved across a server restart, like client team info
	held.active = true;

	for (n=0; n<MAX_CLIENTS; n++)
	{
		int team;

		if (!sl.clients[n].active)
			continue;

		held.clients[n].team = team = sl.clients[n].info.team;
		held.clients[n].commander = sl.teams[team].commander == n ? true : false;

		if (sl.clients[n].info.isOfficer)
		{
			int slot;

			for (slot = 0; slot < MAX_OFFICERS; slot++)
			{
				if (sl.teams[team].officers[slot] == n)
				{
					held.clients[n].officerslot = slot;
					break;
				}
			}

			if (slot == MAX_OFFICERS)
			{				
				cores.Console_Printf(fmt("Error with client %i: isOfficer == true, but client wasn't found in officer list!", n));
			}
			else
			{
				held.clients[n].officer = true;
			}
		}

		held.clients[n].nextNagTime = sl.clients[n].nextNagTime;
	}

	held.gametype = sl.gametype;

	if (sl.referee)
	{
		held.referee = sl.referee->index;
		held.guestRef = sl.guestReferee;
	}
}



/*==========================

  SV_NextMap

 ==========================*/

void	SV_NextMap()
{
	//all data stored in serverLocal_t gets cleared on a restart, so call this function to save some information
	SV_HoldPersistentData();

	cores.Cvar_SetVarValue(&sv_nextStatus, GAME_STATUS_SETUP);	//make sure we reset server status on the map change

	//simply restart everything.  the core engine will handle saving previous clients and reconnecting them
	//make sure to call BufPrintf here instead of Cmd_Exec so we can run the rest of the game loop before restarting

	//cores.Cmd_BufPrintf(fmt("world %s", cores.World_GetName()));
	cores.Cmd_BufPrintf("do sv_nextMapCmd\n");
	cores.Cmd_BufPrintf(fmt("world %s\n", sv_nextMap.string));
}

/*==========================

 SV_PlayerChosenMap

 ==========================*/

void	SV_PlayerChosenMap()
{
	SV_HoldPersistentData();

	cores.Cvar_SetVarValue(&sv_nextStatus, GAME_STATUS_SETUP);

	cores.Cmd_BufPrintf("world %s\n", sv_playerChosenMap.string);
}

/*==========================

  SV_RestartMatch

 ==========================*/

void	SV_RestartMatch(int nextStatus)
{
	SV_HoldPersistentData();
	cores.Cvar_SetVarValue(&sv_nextStatus, nextStatus);

	cores.Cmd_BufPrintf("resetgame\n");
}

void	SV_RestartMatch_Cmd(int argc, char *argv[])
{
	SV_SetGameStatus(GAME_STATUS_RESTARTING, sl.gametime + 1000, NULL);	
}

void	SV_ExtendTime(int msec)
{
	if (!sl.endTime)
		return;
	
	sl.endTime += msec;
	ST_SetState(sl.statusString, "endTime", fmt("%i", sl.endTime), sizeof(sl.statusString));

	cores.Server_SetStateString(ST_GAME_STATUS, sl.statusString);
}

void	SV_SetGameStatus(int status, int endTime, const char *optParams)
{
	if (status < 0 || status >= NUM_GAME_STATUS)
		core.Game_Error(fmt("Unkown game status %i\n", status));	

	sl.statusString[0] = 0;
	ST_SetState(sl.statusString, "status", fmt("%i", status), sizeof(sl.statusString));
	sl.status = status;
	ST_SetState(sl.statusString, "endTime", fmt("%i", endTime), sizeof(sl.statusString));
	sl.endTime = endTime;

	if (optParams)
	{
		if (strlen(optParams) + strlen(sl.statusString) > sizeof(sl.statusString)-1)
			core.Game_Error(fmt("optParams too long in SV_SetGameStatus\n"));

		strcat(sl.statusString, optParams);
	}

	//update all clients with the new game status
	cores.Server_SetStateString(ST_GAME_STATUS, sl.statusString);
}


int		SV_FindPlayer(char *name)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!sl.clients[i].active)
				continue;

		if (stricmp(sl.clients[i].info.name, name)==0)
				return i;
	}
	return -1;
}


/*==========================

  SV_FindPlayerSubString

  search for a player using a portion of his name, case insensitive

  the last client found will be the return value
  numfound will be set to the number of matches

 ==========================*/

int		SV_FindPlayerSubString(char *name, int *numfound)
{
	int clientnum = -1;
	int i;
	char substr[CLIENT_NAME_LENGTH];

	*numfound = 0;

	strncpySafe(substr, name, CLIENT_NAME_LENGTH);
	strlwr(substr);

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		char name[CLIENT_NAME_LENGTH];

		if (!sl.clients[i].active)
			continue;

		strcpy(name, sl.clients[i].info.name);
		strlwr(name);
		if (strstr(name, substr))
		{		
			clientnum = i;
			(*numfound)++;
		}
	}

	return clientnum;
}


/*==========================

  SV_BroadcastMessage

 ==========================*/

void	SV_BroadcastMessage(int sender, const char *msg)
{
	cores.Console_Printf(">> %s> %s\n", sl.clients[sender].info.name, msg);
	
	cores.Server_BroadcastMessage(sender, (char *)msg);
}


/*==========================

  SV_BroadcastTeamMessage

 ==========================*/

void	SV_BroadcastTeamMessage(int sender, int team, const char *msg)
{
	int n;	
	
	cores.Console_Printf("(team %i) <%s> %s\n", team, sl.clients[sender].info.name, msg);
	
	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (!sl.clients[n].active)
			continue;

		if (sl.clients[n].info.team == team)
			cores.Server_SendMessage(sender, n, (char *)msg);
	}
}

void	SV_BroadcastChatMessage(int sender, const char *msg)
{
	int n;

	if (cores.Cvar_GetInteger("svr_dedicated"))
		cores.Console_Printf("<%s> %s\n", sender < 0 || sender > MAX_CLIENTS ? "SERVER" : sl.clients[sender].info.name, msg);
	
	if (sender < 0 || sender > MAX_CLIENTS)
	{
		for (n=0; n<MAX_CLIENTS; n++)
		{
			if (sl.clients[n].active)
				SV_ClientEcho(n, fmt("<ADMIN> %s\n", msg));
		}
		//cores.Server_BroadcastMessage(-1, fmt("%s SERVER> %s", SERVER_CHAT_MSG, msg));
	}
	else
	{
		cores.Server_BroadcastMessage(sender, fmt("%s %s> %s", SERVER_CHAT_MSG, sl.clients[sender].info.name, msg));
	}
}

void	SV_SVChat_Cmd(int argc, char *argv[])
{
	int n;
	char msg[512] = "";

	if (!argc)
		return;

	strncpySafe(msg, argv[0], sizeof(msg));

	for (n=1; n<argc; n++)
	{
		if (strlen(msg)+strlen(argv[n]) >= sizeof(msg)-1)
			break;
		strcat(msg, " ");
		strcat(msg, argv[n]);		
	}

	SV_BroadcastChatMessage(-1, msg);
}

/*void	SV_Remote_Cmd(int argc, char *argv[])
{
	int n;
	char args[1024] = "";

	if (!argc)
		return;

	for (n=0; n<argc; n++)
	{
		strcat(args, argv[n]);
		strcat(args, " ");
	}

	cores.Server_BroadcastMessage(0, fmt("remote %s", args));
}*/

int	_clientStructOffset = 0;
#define CAST_INT(ptr) (*(int *)(ptr))

int		_SV_SortByScore(const void *client1, const void *client2)
{
	client_t *cl1 = &sl.clients[*(int *)client1];
	client_t *cl2 = &sl.clients[*(int *)client2];

	if (CAST_INT((char*)cl1 + _clientStructOffset) > CAST_INT((char*)cl2 + _clientStructOffset))
		return -1;

	return 1;
}


/*==========================

  SV_SetUrgencyLevel

  abstract way of queueing different music tracks on the client

 ==========================*/

void	SV_SetUrgencyLevel(int level)
{
	cores.Server_SetStateString(ST_URGENCY, fmt("%i", level));
}

/*==========================

  SV_SetAward

  only called from SV_GiveAwards

 ==========================*/

void	SV_SetAward(char *buf, const char *award, int clientindex)
{
	if (clientindex < 0 || clientindex >= MAX_CLIENTS)
		return;

	ST_SetState(buf, award, fmt("%i", clientindex), 4096);
}

/*==========================

  SV_GiveAwards

  hand out end of game awards

  don't give awards out to clients who aren't on a team or haven't been connected for very long

 ==========================*/

void	SV_GiveAwards(int winningTeam)
{
	int n;
	char awards[4096] = "";
	int mostOrdersGiven = 0, mostOrdersGivenIndex = -1;
	int mostOrdersFollowed = 0, mostOrdersFollowedIndex = -1;
	int mostKills = 0, mostKillsIndex = -1;
	int unbreakable = 0, unbreakableIndex = -1;
	int leastNPCsKilled = 9999999, leastNPCsKilledIndex = -1;
	int mostDeaths = 0, mostDeathsIndex = -1;
	int mostBuildingDamage = 0, mostBuildingDamageIndex = -1;
	int mostNPCsKilled = 0, mostNPCsKilledIndex = -1;
	int mostPeonsKilled = 0, mostPeonsKilledIndex = -1;
	int mostGoldEarned = 0, mostGoldEarnedIndex = -1;
	int mostGoldSpent = 0, mostGoldSpentIndex = -1;
	int leastKills = 9999999, leastKillsIndex = -1, leastKillsNum = 0;
	int mostExperience = 0, mostExperienceIndex = -1;
	int mostMedicXP = 0, mostMedicXPIndex = -1;

	for (n = 0; n < MAX_CLIENTS; n++)
	{
		client_t *client = &sl.clients[n];
		if (client->active && client->info.team && sl.gametime - client->info.connectTime > 300000)
		{
			if (SV_ClientIsCommander(n))
			{
				if (winningTeam == client->info.team)
					SV_SetAward(awards, "Best Commander", n);
			}
			else 
			{
				int medxp;
				int tmp;

				if (client->ps.score.kills == 0 && client->ps.score.deaths && client->stats.buildingKills == 0)
					SV_SetAward(awards, "Newbie Award", n);

				if (client->ps.score.deaths > mostDeaths && client->ps.score.deaths > 10)
				{
					mostDeaths = client->ps.score.deaths;
					mostDeathsIndex = n;
				}

				if (client->ps.score.kills > mostKills)
				{
					mostKills = client->ps.score.kills;
					mostKillsIndex = n;
				}

				if (client->ps.score.kills < leastKills)
				{
					leastKills = client->ps.score.kills;
					if (leastKillsNum)
						leastKillsIndex = -1;
				}

				tmp = client->ps.score.kills - client->ps.score.deaths;
				if (tmp > 10 && tmp > unbreakable)
				{
					unbreakable = tmp;
					unbreakableIndex = n;
				}
				
				if (client->stats.buildingDamage > mostBuildingDamage)
				{
					mostBuildingDamage = client->stats.buildingDamage;
					mostBuildingDamageIndex = n;
				}

				if (client->stats.npcKills > mostNPCsKilled)
				{
					mostNPCsKilled = client->stats.npcKills;
					mostNPCsKilledIndex = n;
				}

				if (client->stats.aiUnitKills > mostPeonsKilled)
				{
					mostPeonsKilled = client->stats.aiUnitKills;
					mostPeonsKilledIndex = n;
				}

				if (client->stats.moneyGained > mostGoldEarned)
				{
					mostGoldEarned = client->stats.moneyGained;
					mostGoldEarnedIndex = n;
				}

				if (client->stats.moneySpent > mostGoldSpent)
				{
					mostGoldSpent = client->stats.moneySpent;
					mostGoldSpentIndex = n;
				}

				if (client->stats.ordersGiven > mostOrdersGiven)
				{
					mostOrdersGiven = client->stats.ordersGiven;
					mostOrdersGivenIndex = n;
				}

				if (client->ps.score.loyalty > mostOrdersFollowed)
				{
					mostOrdersFollowed = client->ps.score.loyalty;
					mostOrdersFollowedIndex = n;
				}

				if (client->ps.score.experience > mostExperience)
				{
					mostExperience = client->ps.score.experience;
					mostExperienceIndex = n;
				}

				medxp = client->stats.xpTypes[EXPERIENCE_PLAYER_HEAL] +
					client->stats.xpTypes[EXPERIENCE_PLAYER_REVIVE];

				if (medxp > mostMedicXP && medxp > 30)
				{
					mostMedicXP = medxp;
					mostMedicXPIndex = n;									
				}
			}
		}
	}
	
	
	if (sl.hero)
	{		
		char *cmdcenterName = GetObjectByType(sl.objects[sl.teams[sl.winningTeam].command_center].base.type)->description;
		SV_SetAward(awards, fmt("Hero (Last Hit on %s)", cmdcenterName), sl.hero->index);
	}

	SV_SetAward(awards, "Veteran (Most Experience)", mostExperienceIndex);
	SV_SetAward(awards, fmt("Homewrecker (%i Building Damage)", mostBuildingDamage), mostBuildingDamageIndex);
	SV_SetAward(awards, fmt("Most In Demand (Given %i Orders)", mostOrdersGiven), mostOrdersGivenIndex);
	SV_SetAward(awards, fmt("Teacher's Pet (%i Orders Followed)", mostOrdersFollowed), mostOrdersFollowedIndex);
	SV_SetAward(awards, fmt("Sadist (%i Kills)", mostKills), mostKillsIndex);
	SV_SetAward(awards, "Best Healer", mostMedicXPIndex);
	SV_SetAward(awards, "Unbreakable", unbreakableIndex);

	SV_SetAward(awards, fmt("Biggest MMORPG Fan (%i NPC Kills)", mostNPCsKilled), mostNPCsKilledIndex);
	SV_SetAward(awards, fmt("Downsizer (%i Worker Kills)", mostPeonsKilled), mostPeonsKilledIndex);

	if (leastNPCsKilled == 0)
		SV_SetAward(awards, "Vegan Award (No NPC Kills)", leastNPCsKilledIndex);
	else
		SV_SetAward(awards, "Ethical Animal Treatment Award", leastNPCsKilledIndex);

	//bad awards
	SV_SetAward(awards, fmt("Reaper's Best Buddy", mostDeaths), mostDeathsIndex);
	if (leastKills == 0)
		SV_SetAward(awards, "Ghandi Award", leastKillsIndex);
	else
		SV_SetAward(awards, "Mostly Harmless", leastKillsIndex);		//don't say how much..make it a little bit ambiguous

	SV_SetAward(awards, fmt("Entrepreneur (Earned %i Gold)", mostGoldEarned), mostGoldEarnedIndex);
	SV_SetAward(awards, fmt("Big Spender (Spent %i Gold)", mostGoldSpent), mostGoldSpentIndex);

	cores.Server_SetStateString(ST_AWARDS, awards);

	cores.Server_SendStats();
}

void	SV_WritePlayerStats(int clientnum, int winningTeam, char *user_stats, int maxlen)
{
	client_t *client = &sl.clients[clientnum];
	
	user_stats[0] = 0;
				
	ST_SetState(user_stats, "Name", sl.clients[clientnum].info.name, maxlen);
	if (SV_ClientIsCommander(clientnum))
	{
		ST_SetState(user_stats, "Commander", "1", maxlen);
		ST_SetState(user_stats, "Commander XP", fmt("%.0f", client->ps.score.experience), maxlen);
	}
	else
	{
		ST_SetState(user_stats, "XP", fmt("%.0f", client->ps.score.experience), maxlen);
	}
	
	if (winningTeam > 0)
	{
		if (winningTeam == client->info.team)
			ST_SetState(user_stats, "Won", "1", maxlen);
		else
			ST_SetState(user_stats, "Lost", "1", maxlen);
	}
	ST_SetState(user_stats, "Deaths", fmt("%i", client->ps.score.deaths), maxlen);
	ST_SetState(user_stats, "Kills", fmt("%i", client->ps.score.kills), maxlen);
	ST_SetState(user_stats, "Enemy Damage", fmt("%i", client->stats.playerDamage), maxlen);
	ST_SetState(user_stats, "Building Damage", fmt("%i", client->stats.buildingDamage), maxlen);
	ST_SetState(user_stats, "Buildings Destroyed", fmt("%i", client->stats.buildingKills), maxlen);
	ST_SetState(user_stats, "NPC Kills", fmt("%i", client->stats.npcKills), maxlen);
	ST_SetState(user_stats, "NPC Damage", fmt("%i", client->stats.npcDamage), maxlen);
	ST_SetState(user_stats, "AI Kills", fmt("%i", client->stats.aiUnitKills), maxlen);
	ST_SetState(user_stats, "AI Damage", fmt("%i", client->stats.aiUnitDamage), maxlen);
	ST_SetState(user_stats, "Gold Earned", fmt("%i", client->stats.moneyGained), maxlen);
	ST_SetState(user_stats, "Gold Spent", fmt("%i", client->stats.moneySpent), maxlen);
	ST_SetState(user_stats, "Orders Given", fmt("%i", client->stats.ordersGiven), maxlen);
	ST_SetState(user_stats, "Orders Followed", fmt("%i", client->ps.score.loyalty), maxlen);
	ST_SetState(user_stats, "Impeached", fmt("%i", client->stats.impeached), maxlen);
	ST_SetState(user_stats, "Kicked", fmt("%i", client->stats.kicked), maxlen);
}

void	SV_WriteStatsPacket(int winningTeam)
{
	int n;
	char user_stats[4096] = { 0 };

	if (winningTeam > 0)
		ST_SetState(user_stats, "complete", "1", 4096);

	switch (sl.gametype)
	{
			case GAMETYPE_RTSS:
					ST_SetState(user_stats, "gametype", "rtss", 4096);
					break;
			case GAMETYPE_DEATHMATCH:
					ST_SetState(user_stats, "gametype", "deathmatch", 4096);
					break;
			default:
					cores.Console_Printf("Error: unknown game type!\n");
					break;
	}

	ST_SetState(user_stats, "map", cores.World_GetName(), 4096);
	ST_SetState(user_stats, "winningTeam", fmt("%i", winningTeam), 4096);
	ST_SetState(user_stats, "team1race", team1_racename.string, 4096);
	ST_SetState(user_stats, "team2race", team2_racename.string, 4096);

	//send the global game stats to start the packet
	cores.Server_StartStats(user_stats);

	for (n = 0; n < MAX_CLIENTS; n++)
	{
		client_t *client = &sl.clients[n];
		if (client->active)
		{
			SV_WritePlayerStats(n, winningTeam, user_stats, 4096);

			cores.Server_AddClientStats(cores.Server_GetClientCookie(n), user_stats);

			cores.Console_DPrintf("setting client %i stats to '%s'\n", n, user_stats);
			cores.Server_SetStateString(ST_CLIENT_FINAL_STATS + n, user_stats);
		}
		else
		{
			cores.Server_SetStateString(ST_CLIENT_FINAL_STATS + n, "");
		}
	}
	cores.Server_SendStats();
}

void	_SV_WriteRankings(file_t *f, int intOffset, const char *heading)
{
	int n,numClients,team;
	int order[MAX_CLIENTS];

	core.File_Printf(f, "=========================\n%s\n=========================\n\n", heading);

	for (team=1; team<=2; team++)
	{
		numClients=0;
		for (n=0; n<MAX_CLIENTS; n++)
		{
			if (sl.clients[n].active && sl.clients[n].info.team == team)
				order[numClients++] = n;
		}

		core.File_Printf(f, "TEAM %i\n------\n", team);

		_clientStructOffset = intOffset;
		qsort(order, numClients, sizeof(int), _SV_SortByScore);

		for (n=0; n<numClients; n++)
		{
			char spaces[256];
			client_t *client = &sl.clients[order[n]];
			char *name = sl.clients[order[n]].info.name;
			int score = CAST_INT((char *)client + intOffset);
			
			memset(spaces, ' ', 255);
			spaces[(CLIENT_NAME_LENGTH+2) - strlen(name)] = 0;
			
			strset(spaces, ' ');

			core.File_Printf(f, "%i.  %s%s%i\n", n+1, name, spaces, score);			
		}

		core.File_Printf(f, "\n\n");
	}

	core.File_Printf(f, "\n");	
}

void	SV_WriteScoresToFile()
{	
	char fname[64];
	file_t *f;

	core.File_GetNextFileIncrement(5, "scores", "txt", fname, 63);
	f = core.File_Open(fname, "wt");
	if (!f)
	{
		cores.Console_Printf("Couldn't write scores file\n");
		return;
	}

	_SV_WriteRankings(f, offsetof(client_t, ps.score.kills), "Kills - Enemy Players");
	_SV_WriteRankings(f, offsetof(client_t, stats.playerDamage), "Damage - Enemy Players");
	_SV_WriteRankings(f, offsetof(client_t, stats.buildingKills), "Kills - Enemy Buildings");
	_SV_WriteRankings(f, offsetof(client_t, stats.buildingDamage), "Damage - Enemy Buildings");
	_SV_WriteRankings(f, offsetof(client_t, stats.npcKills), "Kills - NPCs");
	_SV_WriteRankings(f, offsetof(client_t, stats.npcDamage), "Damage - NPCs");
	_SV_WriteRankings(f, offsetof(client_t, stats.aiUnitKills), "Kills - Enemy AI Units");
	_SV_WriteRankings(f, offsetof(client_t, stats.aiUnitDamage), "Damage - Enemy AI Units");
	_SV_WriteRankings(f, offsetof(client_t, stats.moneyGained), "Money Gained");
	_SV_WriteRankings(f, offsetof(client_t, stats.moneySpent), "Money Spent");
	
	core.File_Close(f);
}

void	SV_EndGame(int winningTeam)
{
	char params[256] = "";
	int n;

	if (sl.status != GAME_STATUS_NORMAL)
		return;		//game can't end in any other mode

	if (sl.gametype == GAMETYPE_RTSS)
	{
		int n;
		int losingTeam = winningTeam == 2 ? 1 : 2;
		int losingCmdCenter = sl.teams[losingTeam].command_center;
		//int winningCmdCenter = sl.teams[winningTeam].command_center;

		for (n = 0; n < MAX_CLIENTS; n++)
		{
			if (sl.clients[n].active)
				SV_SetClientStatus(n, STATUS_ENDGAME);
		}
		
		sl.winningTeam = winningTeam;
	
		SV_SendNoticeToTeam(winningTeam, NOTICE_VICTORY, losingCmdCenter, GameMsg("notice_rtss_victory"));
		SV_SendNoticeToTeam(losingTeam, NOTICE_DEFEAT, losingCmdCenter, GameMsg("notice_rtss_defeat"));

		//put everyone's camera on the destroyed command center
		ST_SetState(params, "lookat", fmt("%i", losingCmdCenter), sizeof(params));
		ST_SetState(params, "winner", fmt("%i", winningTeam), sizeof(params));

		M_CopyVec3(sl.objects[losingCmdCenter].base.pos, sl.globalPos);

		if (sv_writeScores.integer)
		{
			SV_WriteScoresToFile();
		}
		SV_WriteStatsPacket(winningTeam);
		SV_GiveAwards(winningTeam);
	}
	else if (sl.gametype == GAMETYPE_DEATHMATCH)
	{
		int n;
		int winner = winningTeam;		//winningTeam is interpreted as client number

		for (n=0; n<MAX_CLIENTS; n++)
		{
			if (!sl.clients[n].active)
				continue;

			if (n == winner)
			{
				SV_SendNotice(n, NOTICE_VICTORY, winner, GameMsg("notice_deathmatch_winner"));
			}
			else
			{
				SV_SendNotice(n, NOTICE_DEFEAT, winner, fmt("%s %s", sl.clients[winner].info.name, GameMsg("notice_deathmatch_loser")));
			}
		}

		//put everyone's camera on the winner
		ST_SetState(params, "lookat", fmt("%i", winner), sizeof(params));

		if (sv_writeScores.integer)
		{
			SV_WriteScoresToFile();
		}
	}

	if (sv_newPatchAvailable.integer)
	{
		for (n = 0; n < MAX_CLIENTS; n++)
		{
			if (sl.clients[n].active)
				SV_ClientEcho(n, "The server is going down temporarily to update itself.  It will be back shortly.");
		}
	}

	SV_SetGameStatus(GAME_STATUS_ENDED, sl.gametime + sv_endGameTime.integer, params);
}


void	SV_EndGame_Cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	SV_EndGame(atoi(argv[0]));
}

void	SV_GiveMoney_Cmd(int argc, char *argv[])
{
	int n;

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (sl.clients[n].active)
		{
			if (sl.clients[n].info.team)
				SV_GiveMoney(n, (argc < 1) ? 2000 : atoi(argv[0]), false);				
		}
	}
}


void	SV_WriteScores_Cmd(int argc, char *argv[])
{
	SV_WriteScoresToFile();
	SV_WriteStatsPacket(0);
}

void	SV_WaypointPos_Cmd(int argc, char *argv[])
{
	if (argc < 2)
		return;
	sl.objects[0].targetPosition(&sl.clients[0], &sl.objects[0], atof(argv[0]), atof(argv[1]), GOAL_REACH_WAYPOINT);
}

void	SV_WaypointObj_Cmd(int argc, char *argv[])
{
	int idx,goal;
	if (!argc)
		return;
	idx = atoi(argv[0]);
	if (idx < 0 || idx >= MAX_OBJECTS)
		return;

	if (sl.objects[atoi(argv[0])].base.team != sl.clients[0].info.team)
		goal = GOAL_ATTACK_OBJECT;
	else
		goal = GOAL_REACH_WAYPOINT;
	sl.objects[0].targetObject(&sl.clients[0], &sl.objects[0], &sl.objects[atoi(argv[0])], goal);
}


/*==========================

  SV_CheckDuplicateName

  this will rename a client if he has the same name as someone else

 ==========================*/

void	SV_CheckDuplicateName(int clientnum, const char *name)
{
	int n;

	strncpySafe(sl.clients[clientnum].info.name, name, CLIENT_NAME_LENGTH);

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (!sl.clients[n].active)
			continue;
		if (n == clientnum)
			continue;

		if (strcmp(name, sl.clients[n].info.name)==0)
		{
			BPrintf(sl.clients[clientnum].info.name, CLIENT_NAME_LENGTH, "%s(%i)", name, n);
			return;
		}
	}
}

void    SV_ClientRename(int clientnum, const char *requestedName)
{
	unsigned int i = 0, j = 0;
	char name[CLIENT_NAME_LENGTH] = {0};
	char *clanAbbrev = NULL;
	bool nonwhite = false;

	if (sv_showClanAbbrev.integer && sl.clients[clientnum].info.clan_id)
	{
		clanAbbrev = ST_GetState(clanInformation, fmt("c%ia", sl.clients[clientnum].info.clan_id));
		strcpy(name, fmt("%s%s", clanAbbrev, strchr(clanAbbrev, '^') ? "^w" : "")); //add a ^w if the clan name has colors in it
	}

	j = strlen(name);
	//strip invalid chars from the nick
	while (i < strlen(requestedName) && strlen(name) < CLIENT_NAME_LENGTH-1)
	{
		switch (requestedName[i])
		{
			//case ' ':
			case '\n':
			case '\v':
			case '\r':
			case '\t':
			case '[':
			case ']':
			case '{':
			case '}':
			case '^':
			case '`':
			case '~':
			case '"':
					break;
			default:
					if ((requestedName[i] >= 'a' && requestedName[i] <= 'z')
						|| (requestedName[i] >= 'A' && requestedName[i] <= 'Z')
						|| (requestedName[i] >= '0' && requestedName[i] <= '9'))
					{
						nonwhite = true;
					}
					name[j++] = requestedName[i];
		}
		i++;
	}
	name[j] = '\0';
	
	SV_CheckDuplicateName(clientnum, name);
    cores.Console_Printf("player %i is now known as %s\n", clientnum, name);
}


void	SV_SetReady(client_t *client)
{
	if (sl.status != GAME_STATUS_SETUP && sl.status != GAME_STATUS_ENDED)
		return;								//ready status irrelevant during any other status mode

	if (client->info.ready)
		return;
	if (!client->info.team)
		return;

	client->info.ready = true;
	SV_RefreshClientInfo(client->index);

	SV_BroadcastNotice(NOTICE_GENERAL, client->index, fmt("%s is ready!", client->info.name));
}


void	SV_SetReferee(client_t *client, bool guest)
{
	if (client == sl.referee && guest == sl.guestReferee)
		return;

	//remove the current referee
	if (sl.referee)
	{
		sl.referee->info.isReferee = false;
		SV_RefreshClientInfo(sl.referee->index);
	}

	if (!client)
	{
		sl.referee = NULL;
		return;
	}

	client->info.isReferee = true;
	sl.referee = client;
	sl.guestReferee = guest;

	SV_RefreshClientInfo(client->index);

	SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("%s has become the referee!", client->info.name));

	//don't tell the client they're ref if they're a guest and not in setup mode
	if (guest && sl.status != GAME_STATUS_SETUP)
		return;

	if (guest)
		SV_ClientEcho(client->index, "Guest referee status granted\n");
	else
		SV_ClientEcho(client->index, "Referee status granted\n");
	SV_ClientEcho(client->index, "Type \"/ref\" to see a list of commands\n");
}

void	SV_RefereeCommand(client_t *client, int argc, char *argv[])
{
	if (sl.referee != client)
	{
		SV_ClientEcho(client->index, "You are not a referee.  To become referee, type \"refpwd <password>\"\n");		
		return;
	}

	if (sl.guestReferee && sl.status != GAME_STATUS_SETUP)
	{
		SV_ClientEcho(client->index, "Guest referee commands not allowed during match!  Wait until next game setup time.\n");
		return;
	}

	if (!argc)
	{
		//list possible referee commands
		SV_ClientEcho(client->index, "Referee commands\n");
		SV_ClientEcho(client->index, "================\n");
		SV_ClientEcho(client->index, "ref restartmatch\n");
		SV_ClientEcho(client->index, "ref startmatch\n");
#ifndef SAVAGE_DEMO		
		SV_ClientEcho(client->index, "ref setrace <team> <race>\n");
#endif
		SV_ClientEcho(client->index, "ref world <mapname>\n");
		SV_ClientEcho(client->index, "ref impeach <team>\n");
		SV_ClientEcho(client->index, "ref setcmdr <player>\n");
		SV_ClientEcho(client->index, "ref stopvote\n");
		return;
	}

	if (strcmp(argv[0], "restartmatch")==0)
	{
		/*if (sl.guestReferee)
		{
			SV_ClientEcho(client->index, "Command not allowed for guest refs\n");
		}
		else*/
		{
			cores.Cmd_BufPrintf("restartmatch\n");
		}
	}
	else if (strcmp(argv[0], "stopvote")==0)
	{
		if (sl.guestReferee)
		{
			SV_ClientEcho(client->index, "Command not allowed for guest refs\n");
		}
		else
		{
			cores.Cmd_BufPrintf("stopvote\n");
		}
	}
	else if (strcmp(argv[0], "startmatch")==0)
	{
		if (sl.status == GAME_STATUS_SETUP)
		{
			SV_SetGameStatus(GAME_STATUS_WARMUP, sl.gametime + sv_warmupTime.integer, NULL);
			SV_BroadcastNotice(NOTICE_GENERAL, 0, "Referee started the match!");
		}
		else
		{
			SV_ClientEcho(client->index, "startmatch only allowed during game setup mode\n");
		}
	}
#ifndef SAVAGE_DEMO
	else if (strcmp(argv[0], "setrace")==0)
	{
		if (argc < 3)
		{
			SV_ClientEcho(client->index, "Type \"ref setrace <team> <race>\"\n");
		}
		else
		{
			int team = atoi(argv[1]);

			if (team == 1 || team == 2)
			{
				int n;

				for (n=1; n<MAX_RACES; n++)
				{
					if (stricmp(argv[2], raceData[n].name)==0)
					{
						cores.Cvar_Set(fmt("sv_team%irace", team), argv[2]);
						break;
					}
				}
				
				if (n == MAX_RACES)
					SV_ClientEcho(client->index, fmt("%s is not a valid race\n", raceData[n].name));
				else
					SV_ClientEcho(client->index, fmt("Team %i will be set to %s on the next restart (type \"/ref restartmatch\")\n", team, raceData[n].name));

				SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("The referee has set team %i to %s!", team, raceData[n].name));
			}
			else
			{
				SV_ClientEcho(client->index, "team must be 1 or 2\n");
			}
		}
	}
#endif	//SAVAGE_DEMO
	else if (strcmp(argv[0], "world")==0)
	{
		if (argc < 2)
		{
			SV_ClientEcho(client->index, "Type \"ref world <mapname>\"");
		}
		else
		{
			if (core.File_Exists(fmt("/world/%s.s2z", argv[1])))
			{
				SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("The referee is changing the map to %s!", argv[1]));
				cores.Cvar_SetVar(&sv_playerChosenMap, fmt("%s", argv[1]));	
				SV_SetGameStatus(GAME_STATUS_PLAYERCHOSENMAP, sl.gametime + 2000, NULL);
			}
			else
			{
				SV_ClientEcho(client->index, "That world doesn't exist on this server\n");
			}
		}
	}
	else if (strcmp(argv[0], "impeach")==0)
	{
		if (argc < 2)
		{
			SV_ClientEcho(client->index, "Type \"ref impeach <team>\"");
		}
		else
		{
			int team = atoi(argv[1]);

			if (team == 1 || team == 2)
			{				
				if (sl.teams[team].commander > -1)
				{
					//client_t *cmdr = &sl.clients[sl.teams[team].commander];
					SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("The referee impeached %s!", client->info.name));
					SV_CommanderResign(client->index, true);
				}
			}
		}
	}
	else if (strcmp(argv[0], "setcmdr")==0)
	{
		if (argc < 2)
		{
			SV_ClientEcho(client->index, "Type \"ref setcmdr <player>\"");
		}
		else			
		{		
			int numfound;
			int clientnum = SV_FindPlayerSubString(argv[1], &numfound);

			if (clientnum > -1 && clientnum < MAX_CLIENTS)
			{				
				if (sl.clients[clientnum].info.team > 0)
				{
					SV_SetCommander(SV_GetTeam(clientnum), &sl.clients[clientnum], true);
					SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("The referee has made %s the commander of team %i!", sl.clients[clientnum].info.name, sl.clients[clientnum].info.team));
				}
			}
		}
	}
}

/*==========================

  SV_FirstClientConnection

  set up the correct server status for the first client connection

  this is the ONLY place the following status modes get set:

  GAME_STATUS_SETUP
    - during this time the referee may change server options
	- initially "endTime" for this mode will be 0 until a client connects, at which point it will be set to sl.gametime + sv_setupTime

  GAME_STATUS_WARMUP
    - during this time the referee may only change server options if he is not a guest referee
	- "endTime" will be set to sl.gametime + sv_warmupTime, which defaults to 30 seconds

  GAME_STATUS_NORMAL
    - this is the normal play mode.  the referee may only change server options if he is not a guest referee
	- "endTime" will be set to sl.gametime + sv_timeLimit, or 0 if no timelimit specified

 ==========================*/


void	SV_FirstClientConnection(client_t *client)
{
	int endtime;

	if (sv_nextStatus.integer == GAME_STATUS_SETUP && !sv_doSetup.integer)
		cores.Cvar_SetVarValue(&sv_nextStatus, GAME_STATUS_WARMUP);

	if (sv_nextStatus.integer == GAME_STATUS_WARMUP && !sv_doWarmup.integer)
		cores.Cvar_SetVarValue(&sv_nextStatus, GAME_STATUS_NORMAL);


	switch(sv_nextStatus.integer)
	{
		case GAME_STATUS_SETUP:
			//setup time lasts indefinitely on the first connection
			//the countdown starts when numclients == sv_minPlayers.integer
			endtime = 0;
			break;

		case GAME_STATUS_WARMUP:
			endtime = sl.gametime + sv_warmupTime.integer;
			break;

		case GAME_STATUS_NORMAL:
			//make sure timelimit is at least 1 minute
			if (sv_timeLimit.integer < 0)
				cores.Cvar_SetVarValue(&sv_timeLimit, 0);
			else if (sv_timeLimit.integer < 60000 && sv_timeLimit.integer > 0)
				cores.Cvar_SetVarValue(&sv_timeLimit, 60000);			

			endtime = sv_timeLimit.integer ? sl.gametime + sv_timeLimit.integer : 0;

			SV_SetUrgencyLevel(URGENCY_LOW);
			break;

		default:
			core.Game_Error("sv_nextStatus invalid\n");
			break;
	}

	SV_SetGameStatus(sv_nextStatus.integer, endtime, NULL);

	if (sv_allowGuestReferee.integer && sl.status == GAME_STATUS_SETUP)
	{
		//set this client as the referee
		SV_SetReferee(client, true);
	}
}



/*==========================

  SV_AllClientsDisconnected

  server became empty, restart it

 ==========================*/

void	SV_AllClientsDisconnected()
{
	SV_RestartMatch(GAME_STATUS_SETUP);
}


/*==========================

  SV_NewNetSettings

  called when the player changes his net settings (maxpacketsize, name, etc), 
  as well as during SV_ClientConnect()

 ==========================*/
void	SV_NewNetSettings(int clientnum, const char *settings)
{
	if (ST_FindState(settings, "name"))
	{
		SV_ClientRename(clientnum, ST_GetState(settings, "name"));
	}

	//set client's aspect ratio for siege weapon controls
	sl.clients[clientnum].aspect = atof(ST_GetState(settings, "aspect"));

	SV_RefreshClientInfo(clientnum);
}

/*==========================

  SV_InitClientData

 ==========================*/

void	SV_InitClientData(int clientnum, const char *netsettings, unsigned int clan_id, bool restarting, bool virtualClient)
{
	client_t	*client;
	int			index;

	//initialize their player state and client info
	memset(&sl.clients[clientnum], 0, sizeof(client_t));


	client = sl.objects[clientnum].client;
	index = sl.objects[clientnum].base.index;
	memset(&sl.objects[clientnum], 0, sizeof(serverObject_t));

	sl.objects[clientnum].base.index = index;
	sl.objects[clientnum].client = client;

	sl.clients[clientnum].info.connectTime = sl.gametime;
	sl.clients[clientnum].info.clan_id = clan_id;

	sl.clients[clientnum].active = true;
	sl.clients[clientnum].isVirtual = virtualClient;
	sl.clients[clientnum].index = clientnum;		//should never change
	sl.clients[clientnum].lastInput.gametime = sl.gametime;
	sl.clients[clientnum].obj = &sl.objects[clientnum];
	sl.clients[clientnum].ps.score.level = 1;
	sl.clients[clientnum].ps.flags = PS_NOT_IN_WORLD;
	sl.clients[clientnum].ps.clientnum = clientnum;

	sl.clients[clientnum].lastVoteCalled = 0;
	sl.clients[clientnum].lastChatMsg = 0;
	sl.clients[clientnum].mute = false;


	//this is also done in SV_NewNetSettings
	if (ST_FindState(netsettings, "name"))
	{
		SV_ClientRename(clientnum, ST_GetState(netsettings, "name"));
	}

	cores.Console_Printf("Player %s connected, UID %u, GUID %u\n", sl.clients[clientnum].info.name, cores.Server_GetClientUID(clientnum), cores.Server_GetClientGUID(clientnum));

	//name = cores.Server_CheckForVIP(cores.Server_GetClientCookie(clientnum));
	//if (name)
	//fixme: do something special for vips
	

	//put them in the lobby
	SV_SetClientStatus(clientnum, STATUS_LOBBY);

	sl.numClients++;

	if (restarting && held.active)
	{
		//restore their previous team membership and any special status
		SV_ClientJoinTeam(client, held.clients[clientnum].team, true, true);

		if (held.clients[clientnum].officer)
			SV_PromoteClient(client, held.clients[clientnum].officerslot);

		if (held.clients[clientnum].commander)
			SV_SetCommander(held.clients[clientnum].team, client, true);
		
		if (held.referee == clientnum)
			SV_SetReferee(&sl.clients[held.referee], held.guestRef);

		sl.clients[clientnum].nextNagTime = held.clients[clientnum].nextNagTime;
	}
	else
	{
		//announce that a new client has connected
		SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("%s has joined the game", ST_GetState(netsettings, "name")));	

		/*
		if (sv_autojoin.integer && sl.status != GAME_STATUS_NORMAL)
		{
			if (sl.teams[1].num_players > sl.teams[2].num_players)
				SV_ClientJoinTeam(client, 2, false, false);
			else
				SV_ClientJoinTeam(client, 1, false, false);
		}
		//else
		*/

		//put them on spectators
		SV_ClientJoinTeam(client, 0, false, false);

		sl.clients[clientnum].nextNagTime = cores.Milliseconds() + 1200000;			//obnoxious demo nag screen / long respawn
	}

	SV_NewNetSettings(clientnum, netsettings);

	if (!virtualClient)
	{
		if (sl.numClients == 1)
			SV_FirstClientConnection(&sl.clients[clientnum]);

		if (sl.numClients == sv_minPlayers.integer &&
			sl.status == GAME_STATUS_SETUP &&
			sl.endTime == 0 &&
			sv_setupTime.integer)
		{
			//start setup time countdown
			SV_SetGameStatus(GAME_STATUS_SETUP, sl.gametime + sv_setupTime.integer, NULL);
		}
	}
}

/*==========================

  SV_ClientConnect

  this is called when the player first connects to the server

 ==========================*/

void	SV_ClientConnect(int clientnum, const char *netsettings, unsigned int clan_id, bool restarting)
{
	SV_InitClientData(clientnum, netsettings, clan_id, restarting, false);
}


/*=======================

  SV_ClientDisconnect

  called when a player disconnects from the server

 =======================*/

void	SV_ClientDisconnect(int clientnum, const char *reason)
{
	int			i;
	char		user_stats[4096] = {0};
	client_t	*client;

	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
	{
		core.Game_Error(fmt("SV_ClientDisconnect received an invalid clientnum (%i)\n", clientnum));
		return;
	}

	client = &sl.clients[clientnum];

	//send just this single player's stats to the stats server
	if (sl.status == GAME_STATUS_NORMAL)
	{
		cores.Server_StartStats(user_stats);
		SV_WritePlayerStats(clientnum, 0, user_stats, 4096);
		cores.Server_AddClientStats(cores.Server_GetClientCookie(clientnum), user_stats);
		cores.Server_SendStats();
	}

	//demote the player if they are an officer
	SV_DemoteClient(client);
		
	//a client loses all the items he was carrying when they disconnect
	//make sure they aren't using up delpoyment slots
	for (i = 0; i < MAX_INVENTORY; i++)
	{
		client->ps.inventory[i] = 0;
		client->ps.ammo[i] = 0;
	}

	//join team 0 to have them properly leave their current team
	SV_ClientJoinTeam(client, TEAM_UNDECIDED, true, true);
	SV_FreeClientObject(clientnum);
	sl.clients[clientnum].active = false;

	//remove their clientinfo string
	cores.Server_SetStateString(ST_CLIENT_INFO + clientnum, "");

	if (reason[0])
		SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("%s disconnected (%s)", sl.clients[clientnum].info.name, reason));
	else
		SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("%s disconnected", sl.clients[clientnum].info.name));

	sl.numClients--;
	
	if (sl.referee == &sl.clients[clientnum])
	{
		if (sv_allowGuestReferee.integer)
		{
			int n;
			//pick a new ref

			for (n=0; n<MAX_CLIENTS; n++)
			{
				if (sl.clients[n].active)
				{
					SV_SetReferee(&sl.clients[n], true);
					break;					
				}
			}

			if (n == MAX_CLIENTS)
				SV_SetReferee(NULL, true);
		}
		else
		{
			SV_SetReferee(NULL, true);
		}
	}

	if (sl.numClients == 0)
		SV_AllClientsDisconnected();
}


void	SV_ClientStartGameRequest(client_t *client)
{
	if (sl.gameStarted)
		SV_PutClientIntoGame(client);
}

void	SV_SelectedPlayerInventoryChanged(int team)
{
	static int lastInventory[MAX_TEAMS][MAX_INVENTORY];
	client_t *client;
	char tmp[1024];
	int i;
	
	if (sl.teams[team].selection.numSelected != 1
		|| sl.teams[team].selection.array[0] > MAX_CLIENTS)
	{
		if (lastInventory[team][0] >= 0)
			for (i = 0; i < MAX_INVENTORY; i++)
				lastInventory[team][i] = -1;
		return;
	}

	client = &sl.clients[sl.teams[team].selection.array[0]];
	for (i = 0; i < MAX_INVENTORY; i++)
		if (client->ps.inventory[i] != lastInventory[team][i])
			break;
	
	if (i >= MAX_INVENTORY) //i.e. nothing has changed...
		return;

	BPrintf(tmp, 1024, "%s %i", SERVER_COMMANDER_UNITINFO_MSG, sl.teams[team].selection.array[0]);
	for (i = 0; i < MAX_INVENTORY; i++)
		if (i == 0 || client->ps.inventory)
			strcat(tmp, fmt(" %i", client->ps.inventory[i]));
	cores.Server_SendMessage(-1, sl.teams[team].commander, tmp);

	//now store the last inventory we sent the commander
	for (i = 0; i < MAX_INVENTORY; i++)
		lastInventory[team][i] = client->ps.inventory[i];
}

void	SV_RefreshTimeOfDay()
{
	char s[128] = "";

	ST_SetState(s, "start", fmt("%i", sv_todStart.integer), sizeof(s));
	ST_SetState(s, "speed", fmt("%f", sv_todSpeed.value), sizeof(s));
	
	cores.Server_SetStateString(ST_TIME_OF_DAY, s);
}

void	SV_RefreshMOTD()
{
	char s[1024] = "";

	ST_SetState(s, "1", sv_motd1.string, sizeof(s));
	ST_SetState(s, "2", sv_motd2.string, sizeof(s));
	ST_SetState(s, "3", sv_motd3.string, sizeof(s));
	ST_SetState(s, "4", sv_motd4.string, sizeof(s));

	cores.Server_SetStateString(ST_MOTD, s);
}

void	SV_TimeLimitUp()
{
	bool overtime = false;

	//determine who won

	if (sl.gametype == GAMETYPE_RTSS)
	{
		teamInfo_t *team1 = &sl.teams[1];
		teamInfo_t *team2 = &sl.teams[2];
		serverObject_t *cmdcenter1 = &sl.objects[team1->command_center];
		serverObject_t *cmdcenter2 = &sl.objects[team2->command_center];

		if (cmdcenter1->base.health > cmdcenter2->base.health)
			SV_EndGame(1);
		else if (cmdcenter2->base.health > cmdcenter1->base.health)
			SV_EndGame(2);
		else
		{
			overtime = true;

			/*
			SV_BroadcastNotice(NOTICE_SUDDEN_DEATH, 0, "");

			cmdcenter1->base.health = 1;
			cmdcenter2->base.health = 1;
			*/
		}

	}
	else if (sl.gametype == GAMETYPE_DEATHMATCH)
	{
		int n;
		int highestKills = -999999;
		int clientnum = -1;
		
		for (n=0; n<MAX_CLIENTS; n++)
		{
			if (!sl.clients[n].active)
				continue;

			if (sl.clients[n].ps.score.kills > highestKills)
			{
				highestKills = sl.clients[n].ps.score.kills;
				clientnum = n;
			}
			else if (sl.clients[n].ps.score.kills == highestKills)
			{
				overtime = true;
				break;
			}
		}

		if (!overtime)
		{
			SV_EndGame(clientnum);
		}
	}

	if (overtime)
	{
		//2 minute overtime
		SV_BroadcastNotice(NOTICE_OVERTIME, 0, "");
		SV_SetGameStatus(GAME_STATUS_NORMAL, sl.gametime + 120000, NULL);
		SV_SetUrgencyLevel(URGENCY_HIGH);
	}
}

int	SV_GetOriginater(int objindex)
{
	serverObject_t	*obj;

	if (objindex < 0 || objindex >= MAX_OBJECTS)
		return -1;
	
	obj = &sl.objects[objindex];

	while (obj->owner)
		obj = obj->owner;

	return obj->base.index;
}

void	SV_SetEasterEggs()
{
	time_t		t;
	struct tm	*datetime;
	int	a, d, e, m, n;

	sl.activeEasterEggs = 0;

	if (!sv_easterEggs.integer)
		return;

	time(&t);
	datetime = localtime(&t);
	/*cores.Cvar_SetVar(&sv_datetime, 
		fmt("%04i/%02i/%02i %02i:%02i:%02i", 
		datetime->tm_year + 1900, datetime->tm_mon + 1, datetime->tm_mday, 
		datetime->tm_hour, datetime->tm_min, datetime->tm_sec));*/

	//fourth of july
	if (datetime->tm_mon == 6 && datetime->tm_mday == 4)
		sl.activeEasterEggs |= EGG_FOURTH_OF_JULY;

	//easter (this is friggin complicated)
	a = datetime->tm_year;
	d = (((255 - 11 * (a % 19)) - 21) % 30) + 21;
	e = d + (d > 48) + 6 - ((a + (int)floor(a / 4) + d + (d > 48) + 1) % 7);
	m = 3 + (e >= 31);
	n = (e >= 31) ? e - 30 : e + 1;
	if (datetime->tm_mon == m && datetime->tm_mday == n)
		sl.activeEasterEggs |= EGG_EASTER;

	//christmas
	if (datetime->tm_mon == 11 && datetime->tm_mday == 25)
		sl.activeEasterEggs |= EGG_CHRISTMAS;

	//halloween
	if (datetime->tm_mon == 9 && datetime->tm_mday == 31)
		sl.activeEasterEggs |= EGG_HALLOWEEN;

	//cinco de mayo, ¡ARIBA!
	if (datetime->tm_mon == 5 && datetime->tm_mday == 5)
		sl.activeEasterEggs |= EGG_CINCO_DE_MAYO;

	cores.Cvar_SetVarValue(&sv_activeEasterEggs, sv_easterEggs.integer ? sl.activeEasterEggs : 0);
}


/*==========================

  SV_CheckTimes

  check all the relevant time fields and do something appropriate if the time is up

 ==========================*/

extern cvar_t sv_respawnTime;

void	SV_CheckTimes()
{
	int n;

	//respawn cycle
	for (n=1; n<MAX_TEAMS; n++)
	{
		if (sl.gametime >= sl.teams[n].nextRespawnWindow)
		{
			sl.teams[n].nextRespawnWindow = sl.gametime + sv_respawnTime.integer;			
		}
	}

	//server status transition time
	if (sl.gametime >= sl.endTime && sl.endTime)
	{
		switch(sl.status)
		{
			case GAME_STATUS_SETUP:
				//go into warmup
				SV_SetGameStatus(GAME_STATUS_WARMUP, sl.gametime + sv_warmupTime.integer, NULL);
				return;
			case GAME_STATUS_WARMUP:
				SV_RestartMatch(GAME_STATUS_NORMAL);
				return;
			case GAME_STATUS_NORMAL:
				//timelimit is up
				SV_TimeLimitUp();
				return;
			case GAME_STATUS_ENDED:
				if (sv_newPatchAvailable.integer && sv_autoPatchMyServer.integer)
				{
					cores.Cmd_Exec("quit"); 
					return;
				}
				else
				{
					SV_SetGameStatus(GAME_STATUS_NEXTMAP, sl.gametime + 1000, NULL);
				}
				return;
			case GAME_STATUS_NEXTMAP:
				SV_NextMap();
				return;
			case GAME_STATUS_PLAYERCHOSENMAP:
				SV_PlayerChosenMap();				
				return;
			case GAME_STATUS_RESTARTING:
				SV_RestartMatch(GAME_STATUS_SETUP);
				return;
		}
	}

	//if enough clients are ready in setup or end of game, we can short circuit the time check
	if (sl.status == GAME_STATUS_SETUP || sl.status == GAME_STATUS_ENDED)
	{
		int n;
		int numReady = 0;
		
		if (!sv_newPatchAvailable.integer || !sv_autoPatchMyServer.integer)
		{
			for (n=0; n<MAX_CLIENTS; n++)
			{
				if (!sl.clients[n].active)
					continue;
	
				if (sl.clients[n].info.ready)
					numReady++;
			}
	
			if ((float)numReady / sl.numClients >= sv_readyPercent.value)
			{
				if (sl.status == GAME_STATUS_SETUP)
				{
					//go into warmup
					SV_SetGameStatus(GAME_STATUS_WARMUP, sl.gametime + sv_warmupTime.integer, NULL);				
				}
				else
				{
					SV_SetGameStatus(GAME_STATUS_NEXTMAP, sl.gametime + 2000, NULL);
				}
			}
		}
	}

	//urgency (music) reset time
	if (sl.urgencyResetTime && sl.gametime >= sl.urgencyResetTime)
	{
		char buf[32];
		if (atoi(cores.Server_GetStateString(ST_URGENCY, buf, sizeof(buf))) >= URGENCY_HIGH)
		{
			SV_SetUrgencyLevel(URGENCY_LOW);
		}
	}
}


void	SV_CheckCvars()
{
	//time of day
	if (sv_todStart.modified || sv_todSpeed.modified)
	{
		SV_BroadcastNotice(NOTICE_GENERAL, 0, "The time of day was modified");
		SV_RefreshTimeOfDay();
		sv_todStart.modified = false;
		sv_todSpeed.modified = false;
	}

	if (sv_motd1.modified || sv_motd2.modified || sv_motd3.modified || sv_motd4.modified)
	{
		SV_RefreshMOTD();
		sv_motd1.modified = false;
		sv_motd2.modified = false;
		sv_motd3.modified = false;
		sv_motd4.modified = false;
	}
}

/*==========================

  SV_Frame

  the main game processing function

 ==========================*/

void	SV_Frame(int gametime)
{
	int n;
	float fps = cores.Cvar_GetValue("svr_gamefps");

	//calculate frame_msec and frame_sec, used for physics simulation delta time
	//this will not necessarily always be the current gametime minus the last gametime
	//if the server frame was particularly long, then we need to cap it

	sl.frame_msec = gametime - sl.gametime;
	if (sl.frame_msec > (1000.0 / fps) * 4)
		sl.frame_msec = (1000.0 / fps) * 4;
	sl.frame_sec = sl.frame_msec / 1000.0;
	sl.gametime = gametime;

	SV_SetEasterEggs();

	//check various time variables
	SV_CheckTimes();

	//check any cvars we need to track the value of
	SV_CheckCvars();

	//advance non client objects
	SV_AdvanceObjects();

	//advance clients
	SV_AdvanceClients();
	
	//do per-frame team maintenance
	SV_TeamFrame();

	//handle vote system
	SV_VoteFrame();

	for (n = 0; n < MAX_TEAMS; n++)
		SV_SelectedPlayerInventoryChanged(n);
}


void	SV_SetGameType(int gametype)
{
	if (gametype < 0 || gametype >= NUM_GAMETYPES)
		gametype = GAMETYPE_RTSS;

	sl.gametype = gametype;
 	cores.Server_SetStateString(ST_GAMETYPE, fmt("%i", gametype));
}


/*==========================

  SV_Shutdown

  called on SV_Reset or Server_Disconnect.
  a chance to clean up/shutdown any objects...

 ==========================*/


void SV_Shutdown()
{
	int n;

	// term ai if necessary
	for (n = 0; n < MAX_OBJECTS; n++)
	{
		if (sl.objects[n].ai)
		{
			SV_AI_Destroy(sl.objects[n].ai);
			sl.objects[n].ai = NULL;
		}
	}
}


/*==========================

  SV_BuildObjectTypesString

  builds a state string that matches up the client object definitions with the server

 ==========================*/

void	SV_BuildObjectTypesString()
{
	int n;
	char statestring[8192] = "";

	for (n=0; n<MAX_OBJECT_TYPES; n++)
	{
		objectData_t *def = GetObjectByType(n);

		if (!def->objclass)
			continue;
		if (!def->name[0])
			continue;

		if (!ST_SetState(statestring, fmt("%i", n), def->name, sizeof(statestring)))
			core.Game_Error("Object type state string is too long!\n");
	}

	cores.Server_SetStateString(ST_OBJECT_TYPES, statestring);
}


/*==========================

  SV_Reset

  called on a new map or on a restart

  'first' is set to true if this is the first time the server has been run

 ==========================*/

void	SV_Reset(bool first)
{
	vec3_t blah;
	int n;
	int gametype;

	//tell the core engine where to find shared game data
	cores.Server_GameObjectPointer(&sl.objects[0], sizeof(sl.objects[0]), MAX_OBJECTS);
	cores.Server_ClientExclusionListPointer(sl.clients[0].exclusionList, sizeof(client_t), MAX_CLIENTS);
	//set certain strings to request only so they don't get sent all the time
	for (n=ST_CLIENT_SCORE; n < ST_CLIENT_SCORE + MAX_CLIENTS; n++)
	{
		cores.Server_SetRequestOnly(n);
	}

	SV_BuildObjectTypesString();

	//clear the non held-over server data
	memset(&sl, 0, sizeof(sl));
	sl.lastActiveObject = MAX_CLIENTS;

	//set the server to 'setup' mode the first time it's run
	if (first)
	{
		cores.Cvar_SetVarValue(&sv_nextStatus, GAME_STATUS_SETUP);		
	}

	cores.World_GetBounds(blah, sl.worldBounds);

	//reserve the first MAX_CLIENTS objects for client representations
	for (n=0; n<MAX_CLIENTS; n++)
	{
		sl.clients[n].obj = &sl.objects[n];
		sl.objects[n].client = &sl.clients[n];
		sl.clients[n].index = n;
	}

	//set indexes correctly
	for (n=0; n<MAX_OBJECTS; n++)
	{
		sl.objects[n].base.index = n;
	}

	//set game type
	gametype = StringToGametype(sv_gametype.string);

	//clear all held over data if we're changing game types
	if (held.active && gametype != held.gametype)
	{
		memset(&held, 0, sizeof(held));
		cores.Cvar_SetVarValue(&sv_nextStatus, GAME_STATUS_SETUP);
	}
	
	SV_SetGameType(gametype);

	//game is always empty on SV_Reset()
	//sv_nextStatus will take effect on the first client connection
	SV_SetGameStatus(GAME_STATUS_EMPTY, 0, NULL);	

	//initialize basic team data and spawn in command centers and other reference objects
	SV_SpawnReferenceObjects();
	SV_ResetTeams();

	//set time of day
	SV_RefreshTimeOfDay();

	//set MOTD
	SV_RefreshMOTD();

	SV_SetUrgencyLevel(URGENCY_NONE);
}




/*==========================

  SV_GiveBackFromClient

  attempts to sell the item from the specified slot in a clients inventory

 ==========================*/

void	SV_GiveBackFromClient(client_t *client, int slot)
{
	int itemnum, force;
	objectData_t	*item, *unit;
	int	payback = 0;

	if (!cores.Cvar_GetInteger("svr_allowCheats"))
	{
		//must be at loadout screen
		if (client->ps.status != STATUS_UNIT_SELECT)
			return;
	}

	unit = GetObjectByType(client->ps.unittype);

	//must be a valid item in a valid slot
	if (slot >= MAX_INVENTORY || slot < 0)
		return;

	if (!client->ps.inventory[slot])
		return;

	//get the item
	itemnum = client->ps.inventory[slot];
	item = GetObjectByType(itemnum);

	force = GetObjectTypeByName(unit->forceInventory[slot]);
	if (force)
	{
		client->ps.inventory[slot] = force;
		client->ps.ammo[slot] = (GetObjectByType(force)->ammoMax > 0) ? GetObjectByType(force)->ammoStart : -1;
	}
	else if (IsItemType(itemnum) && client->ps.ammo[slot] > 1)
	{
		client ->ps.ammo[slot]--;
	}
	else
	{
		client->ps.inventory[slot] = 0;
		client->ps.ammo[slot] = 0;
	}
	
	//shift inventory
	/*if (slot > 1)
	{
		for (index = 2; index < MAX_INVENTORY - 1 - pass; index++)
		{
			if (client->ps.inventory[index])
				continue;
			client->ps.inventory[index] = client->ps.inventory[index + 1];
			client->ps.ammo[index] = client->ps.ammo[index + 1];
		}
		client->ps.inventory[index] = 0;
		client->ps.ammo[index] = 0;
	}*/

	payback += item->playerCost;
	
	SV_GiveMoney(client->index, payback, false);
}


/*==========================

  SV_GiveClient

  give an item to a client if they can afford it and it has been researched

 ==========================*/

void	SV_GiveClient(client_t *client, int item)
{
	int				index, cost, slots, wpPoints = 0;
	int				count = 0, fullcount = 0;
	playerState_t	*ps = &client->ps;
	objectData_t	*itemData;
	
	if (!client->info.team)
		return;

	if (!cores.Cvar_GetInteger("svr_allowCheats"))
	{
		if (client->ps.status != STATUS_UNIT_SELECT)
			return;

		if(!GetObjectByType(sl.objects[client->index].base.type)->canPurchase)
			return;
	}

	//make sure this is a valid item for the player to carry
	slots = CanPutInInventory(ps->unittype, item);
	if (!slots)
		return;

	itemData = GetObjectByType(item);

	//enforce maxHold limit
	if (itemData->maxHold > 0)
	{
		for (index = 0; index < MAX_INVENTORY; index++)
		{
			if (ps->inventory[index] == item)
			{
				count++;
				if (ps->ammo[index] >= itemData->ammoMax )
					fullcount++;
			}
		}

		if (fullcount >= itemData->maxHold)
		{
			SV_ClientEcho(client->index, fmt("You can only hold %i %ss\n", itemData->maxHold, itemData->description));
			return;
		}
	}

	//check for weapon points
	for (index = 0; index < MAX_INVENTORY; index++)
	{
		wpPoints += GetObjectByType(ps->inventory[index])->weapPointValue;
	}
	if (wpPoints + itemData->weapPointValue > GetObjectByType(sl.objects[client->index].base.type)->maxWeapPoints)
	{
		SV_ClientEcho(client->index, "You can not hold any more weapons\n");
		return;
	}

	//enforce maxDeployment
	if (itemData->maxDeployment > 0 && sl.teams[SV_GetTeam(client->index)].deployedItems[item] >= itemData->maxDeployment)
	{
		SV_ClientEcho(client->index, fmt("Your team has already deployed too many %ss\n", itemData->description));
		return;
	}

	//checks for availability (object is researched and its required buildings exist)
	if (!g_allWeaponsAvailable.integer)
	{
		if (!SV_IsItemResearched(item, SV_GetTeam(client->index)))
			return;

		if (!Tech_IsAvailable(item, sl.teams[SV_GetTeam(client->index)].research))
		{
			SV_ClientEcho(client->index, "This item is not currently avaialble\n");
			return;
		}
	}

	cost = GetObjectByType(item)->playerCost;

	//look for a slot to put the object in 
	for (index = 0; index < MAX_INVENTORY; index++)
	{
		if (!(slots & (1 << index)))
			continue;

		//allow items to 'stack' by increasing ammo
		if (IsItemType(item) &&
			ps->inventory[index] == item && ps->ammo[index] < itemData->ammoMax)
		{
			if (SV_SpendMoney(client->index, cost))
				ps->ammo[index]++;
			else
				SV_ClientEcho(client->index, "You cannot afford this item\n");
			return;
		}

		//is the slot in use (by something other than the default item)?
		if (ps->inventory[index] &&
			ps->inventory[index] != GetObjectTypeByName(GetObjectByType(ps->unittype)->forceInventory[index]))
			continue;

		if (count >= itemData->maxHold)
			continue;

		if (!SV_SpendMoney(client->index, cost))
		{
			SV_ClientEcho(client->index, "You cannot afford this item\n");
			return;
		}

		ps->inventory[index] = item;
		ps->ammo[index] = itemData->ammoStart;
		return;
	}

	SV_ClientEcho(client->index, "This item will not fit in your inventory!\n");
}


/*==========================

  SV_MessageType

  ==========================*/

bool	SV_MessageType(const char *msg, const char *type)
{		
	if ((strncmp(msg, type, strlen(type))==0) && (msg[strlen(type)] == ' ' || msg[strlen(type)] == '\0'))
		return true;

	return false;
}


bool	SV_CanChat(client_t *client)
{
	if (client->mute)
	{
		SV_ClientEcho(client->index, "Message rejected, you have been muted.\n");
		return false;
	}

	if (client->floodMuteEndTime >= sl.gametime)
	{
		SV_ClientEcho(client->index, fmt("Flood protection active: you can chat again in %i seconds.\n", (client->floodMuteEndTime - sl.gametime) / 1000));
		return false;
	}

	if (sl.gametime - client->lastChatMsg <= (sv_chatFloodInterval.integer * 1000))
	{
		client->chatFloodCount++;

		if (client->chatFloodCount >= sv_chatFloodCount.integer)
			client->floodMuteEndTime = sl.gametime + (sv_chatFloodPenaltyTime.integer * 1000);
	}
	else
	{
		client->chatFloodCount = 0;
	}

	client->lastChatMsg = sl.gametime;
	return true;
}

/*==========================

  SV_ClientMessage

  Handles all messages sent from clients

  returning FALSE here means the message is unrecognized, and the client will error out

 ==========================*/

bool	SV_ClientMessage(int clientnum, char *msg)
{
	char msgcopy[1024];
	char *argv[32];
	int argcount;
	client_t	*client;

	strncpySafe(msgcopy, msg, 1024);

	argcount = SplitArgs(msgcopy, argv, 32);

	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		cores.Console_DPrintf("ERROR: Message received from invalid clientnumber (%i)\n", clientnum);
	else
		client = &sl.clients[clientnum];

	//core.Console_Printf("[MSG] %i: %s\n", clientnum, argv[0]);

	//=========================================================================
	//proxies
	//=========================================================================
	if (SV_MessageType(msg, "cproxy"))
	{
		SV_SetControlProxy(&sl.clients[clientnum], atoi(argv[1]));

		return true;
	}

	else if (SV_MessageType(msg, "proxy"))
	{
		SV_SetFullProxy(&sl.clients[clientnum], atoi(argv[1]));

		return true;
	}

	else if (SV_MessageType(msg, "vproxy"))
	{
		SV_SetViewProxy(&sl.clients[clientnum], atoi(argv[1]));

		return true;
	}

	else if (SV_MessageType(msg, "rproxy"))
	{
		SV_ReleaseProxy(&sl.clients[clientnum]);

		return true;
	}

	if (sl.clients[clientnum].inputRedirect)
	{
		return SV_ClientMessage(sl.clients[clientnum].inputRedirect->index, msg);		
	}

	//=========================================================================
	//=========================================================================
	if (strcmp(argv[0], CLIENT_CHAT_MSG)==0)
	{
		if (!SV_CanChat(&sl.clients[clientnum]))
			return true;

		SV_BroadcastChatMessage(clientnum, &msg[strlen(CLIENT_CHAT_MSG)+1]);		
		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_CHAT_SELECTED_MSG))
	{
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		if (!SV_CanChat(&sl.clients[clientnum]))
			return true;

		if (team->commander != clientnum)
		{
			cores.Console_Printf("received CLIENT_CHAT_SELECTED_MSG from non commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
			return true;
		}
		SV_MessageSelected(team, clientnum, &msg[strlen(CLIENT_CHAT_SELECTED_MSG)+1]);		
		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_CHAT_TEAM_MSG))
	{
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		if (!SV_CanChat(&sl.clients[clientnum]))
			return true;

		SV_SendTeamMessage(team, clientnum, fmt("%s %s> %s", SERVER_CHAT_TEAM_MSG, sl.clients[clientnum].info.name, &msg[strlen(CLIENT_CHAT_TEAM_MSG)+1]), true, false);
		return true;
	}
	
	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_VOICECHAT_MSG))
	{
		if (!SV_CanChat(&sl.clients[clientnum]))
			return true;

		SV_ClientVoiceChat(clientnum, atoi(argv[1]), atoi(argv[2]));

		return true;
	}

	//=========================================================================
	// Client wants to add a request to the commanders queue
	// Just verifies that the team has a commander and then relays the message
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_GETCOOKIE_MSG))
	{
		int lookupclient = atoi(GetNextWord(msg));

		if (lookupclient < 0 || lookupclient >= MAX_CLIENTS)
			return true;
		
		if (!sl.clients[lookupclient].active)
			cores.Server_SendMessage(-1, clientnum, fmt("%s %i ", SERVER_CLIENTCOOKIE_MSG, lookupclient));
		else
			cores.Server_SendMessage(-1, clientnum, fmt("%s %i %s", SERVER_CLIENTCOOKIE_MSG, lookupclient, cores.Server_GetClientCookie(lookupclient)));
	
		return true;
	}
	
	//=========================================================================
	// Request individual player score
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_SCORE_PLAYER_MSG))
	{
		int lookupclient = atoi(GetNextWord(msg));

		if (lookupclient < 0 || lookupclient >= MAX_CLIENTS)
			return true;

		SV_SendScores(clientnum, -1, lookupclient);

		return true;
	}

	//=========================================================================
	// Request team scores
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_SCORE_TEAM_MSG))
	{
		int teamnum = atoi(GetNextWord(msg));

		if (teamnum < 0 || teamnum >= MAX_TEAMS)
			return true;

		SV_SendScores(clientnum, teamnum, -1);

		return true;
	}

	//=========================================================================
	// Request all scores
	//=========================================================================
	
	else if (SV_MessageType(msg, CLIENT_SCORE_ALL_MSG))
	{				
		SV_SendScores(clientnum, -1, -1);

		return true;
	}

	//=========================================================================
	// Client wants to add a request to the commanders queue
	// Just verifies that the team has a commander and then relays the message
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_REQUEST_MSG))
	{
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		if (!team->index)
			return true;

		if (team->commander >= 0 && team->commander < MAX_CLIENTS)
			cores.Server_SendMessage(clientnum, team->commander, msg);

		return true;
	}

	//=========================================================================
	// Client's request was declined, relay the message to the player
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_DECLINE_MSG))
	{
		char *c = GetNextWord(msg);
		int targclient = atoi(c);
		teamInfo_t *cmdrteam = SV_GetTeamInfo(clientnum);
		int clteam = SV_GetTeam(targclient);

		if (!cmdrteam->index || 
			targclient < 0 || targclient >= MAX_CLIENTS || 
			cmdrteam->index != clteam ||
			cmdrteam->commander != clientnum)
			return true;

		cores.Server_SendMessage(clientnum, targclient, CLIENT_COMMANDER_DECLINE_MSG);
		return true;
	}

	//=========================================================================
	// Client's request was approved, make transactions and send the good news!
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_APPROVE_MSG))
	{
		char *c = GetNextWord(msg);
		int targclient = atoi(c);
		teamInfo_t *cmdrteam = SV_GetTeamInfo(clientnum);
		int clteam = SV_GetTeam(targclient);

		if (!cmdrteam->index || 
			targclient < 0 || targclient >= MAX_CLIENTS || 
			cmdrteam->index != clteam ||
			cmdrteam->commander != clientnum)
			return true;

		cores.Server_SendMessage(clientnum, targclient, CLIENT_COMMANDER_APPROVE_MSG);
		return true;
	}

	//=========================================================================
	// Client's request was ignored, let them know
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_EXPIRE_MSG))
	{
		char *c = GetNextWord(msg);
		int targclient = atoi(c);
		teamInfo_t *cmdrteam = SV_GetTeamInfo(clientnum);
		int clteam = SV_GetTeam(targclient);

		if (!cmdrteam->index || 
			targclient < 0 || targclient > MAX_CLIENTS || 
			cmdrteam->index != clteam ||
			cmdrteam->commander != clientnum)
			return true;

		cores.Server_SendMessage(clientnum, targclient, CLIENT_COMMANDER_EXPIRE_MSG);
		SV_AddExperience(&sl.clients[clientnum], EXPERIENCE_COMMANDER_IGNORE_REQUEST, 0, 1.0);
		return true;
	}

	//=========================================================================
	// Client wants to cancel a request
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_CANCEL_MSG))
	{
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		if (!team->index)
			return true;

		if (team->commander >= 0 && team->commander < MAX_CLIENTS)
			cores.Server_SendMessage(clientnum, team->commander, msg);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_CHAT_COMMANDER_MSG))
	{
		teamInfo_t *team = SV_GetTeamInfo(clientnum);
		char *chatmsg = GetNextWord(msg);

		if (!SV_CanChat(&sl.clients[clientnum]))
			return true;

		if (!team->index)
		{			
			//can't chat to commander on team 0
			return true;
		}

		if (team->commander >= 0 && team->commander < MAX_CLIENTS)
			SV_SendPrivateMessage(clientnum, team->commander, chatmsg);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_CHAT_PRIVATE_MSG))
	{
		char *pos, *message;
		int receiverClientNum;

		if (!SV_CanChat(&sl.clients[clientnum]))
			return true;

		pos = &msg[strlen(CLIENT_CHAT_PRIVATE_MSG)+1];
		
		message = GetNextWord(pos);
		//my god this is beautiful code!
		message[-1] = '\0';

		receiverClientNum = SV_FindPlayer(pos);
		if (receiverClientNum >= 0)
		{
			SV_SendPrivateMessage(clientnum, receiverClientNum, message);
		}
		else
		{
			SV_SendNotice(clientnum, ERROR_PLAYERNOTFOUND, 0, pos);
		}
		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_TEAM_JOIN_MSG))
	{
		char *pos;
		byte team;

		pos = &msg[strlen(CLIENT_TEAM_JOIN_MSG)+1];
		team = atoi(pos);

		SV_ClientJoinTeam(client, team, false, false);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_SPAWN_BUILDING_MSG))
	{
		//parameters: type x y z anglex angley anglez
		int				argc, type;
		char			*argv[8];
		vec3_t			pos, angle;
		serverObject_t	*obj;

		argc = SplitArgs(msg, argv, 8);
		if (argc < 8)
			return true;

		type = atoi(argv[1]);
		pos[0] = atof(argv[2]);
		pos[1] = atof(argv[3]);
		pos[2] = atof(argv[4]);
		angle[0] = atof(argv[5]);
		angle[1] = atof(argv[6]);
		angle[2] = atof(argv[7]);

		if (!SV_ClientIsCommander(clientnum))
		{
			cores.Console_DPrintf("received CLIENT_COMMANDER_SPAWN_BUILDING_MSG from non commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
			return true;
		}
		
		obj = SV_CommanderPlaceBuilding(client, type, pos, angle, GetObjectByType(type)->linked ? true : false);
		if (obj)
		{
			if (!GetObjectByType(type)->selfBuild)
				SV_GroupMoveToObject(SV_GetTeamInfo(clientnum), obj->base.index, GOAL_CONSTRUCT_BUILDING);

			if (GetObjectByType(type)->linked)
				client->obj->link = obj;
		}

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_SPAWN_BUILDING_LINK_MSG))
	{
		//parameters: type x y z anglex angley anglez
		int				argc, type;
		char			*argv[8];
		vec3_t			pos, angle;
		serverObject_t	*obj;

		argc = SplitArgs(msg, argv, 8);
		if (argc < 8)
			return true;

		type = atoi(argv[1]);
		pos[0] = atof(argv[2]);
		pos[1] = atof(argv[3]);
		pos[2] = atof(argv[4]);
		angle[0] = atof(argv[5]);
		angle[1] = atof(argv[6]);
		angle[2] = atof(argv[7]);

		if (!SV_ClientIsCommander(clientnum))
		{
			cores.Console_DPrintf("received CLIENT_COMMANDER_SPAWN_BUILDING_LINK_MSG from non commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
			return true;
		}
		
		if (!client->obj->link)
		{
			cores.Console_DPrintf("received CLIENT_COMMANDER_SPAWN_BUILDING_LINK_MSG with no previous link\n");
			return true;
		}

		obj = SV_CommanderPlaceBuilding(client, type, pos, angle, false);
		if (obj)
		{
			client->obj->link->twin = obj;
			obj->twin = client->obj->link;
			client->obj->link = NULL;
		}
		else
		{
			SV_FreeObject(client->obj->link->base.index);
		}

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_CANCEL_LINK_MSG))
	{
		if (client->obj->link)
			SV_FreeObject(client->obj->link->base.index);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_CAN_SPAWN_BUILDING_MSG))
	{
		// type x y z anglex angley anglez reqid

		char reason[256];
		teamInfo_t *team = SV_GetTeamInfo(clientnum);
		char *s = GetNextWord(msg);
		byte type;
		vec3_t pos,angle;
		int reqid;
		type = atoi(s);
		s = GetNextWord(s);
		pos[0] = atof(s);
		s = GetNextWord(s);
		pos[1] = atof(s);
		s = GetNextWord(s);
		pos[2] = atof(s);
		s = GetNextWord(s);
		angle[0] = atof(s);
		s = GetNextWord(s);
		angle[1] = atof(s);
		s = GetNextWord(s);
		angle[2] = atof(s);
		s = GetNextWord(s);
		reqid = atol(s);

		if (team->commander == clientnum)
		{
			//in order to spawn a building, you must have at least one peon selected
			//if (team->selection.numSelected >= 1 || GetObjectByType(type)->selfBuild)
			{
				if (IsBuildingType(type))
				{
					if ( SV_BuildingCanFit(type, pos, angle, SV_GetTeam(clientnum), reason) )
					{
						// send success message
						if ( sv_debugBuilding.value )
						{
							cores.Server_SendUnreliableMessage(-1, clientnum, fmt("%s %i t Can build", SERVER_COMMANDER_CAN_SPAWN_BUILDING_MSG, reqid));
						}
						else
						{
							cores.Server_SendUnreliableMessage(-1, clientnum, fmt("%s %i t", SERVER_COMMANDER_CAN_SPAWN_BUILDING_MSG, reqid));
						}
						return true;
					}
				}
				else
				{
					strcpy(reason, "You didn't build a building!");
				}
			}
			//else
			//{
			//	strcpy(reason, "You haven't selected any peons!");
			//}
		}
		else
		{
			strcpy(reason, "You are not the commander!");
		}

		// send failure message
		if ( sv_debugBuilding.value )
		{
			cores.Server_SendUnreliableMessage(-1, clientnum, fmt("%s %i f %s", SERVER_COMMANDER_CAN_SPAWN_BUILDING_MSG, reqid, reason));
		}
		else
		{
			cores.Server_SendUnreliableMessage(-1, clientnum, fmt("%s %i f", SERVER_COMMANDER_CAN_SPAWN_BUILDING_MSG, reqid));
		}

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_LOCATION_GOAL_MSG))
	{		
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		if (team->commander != clientnum)
		{
			cores.Console_Printf("received CLIENT_COMMANDER_LOCATION_GOAL_MSG from non commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
			return true;
		}

		if (team->selection.numSelected > 0)
		{
			int goal;
			vec3_t pos;

			char *s = GetNextWord(msg);
			goal = atoi(s);
			if (goal < 0 || goal >= NUM_GOALS)
			{
				cores.Console_Printf("received invalid goal in CLIENT_COMMANDER_LOCATION_GOAL_MSG\n");
				return true;
			}

			s = GetNextWord(s);
			pos[0] = atoi(s);
			s = GetNextWord(s);
			pos[1] = atoi(s);
			s = GetNextWord(s);
			pos[2] = atoi(s);
		
			SV_GroupMove(team, pos, goal);
		}

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_OBJECT_GOAL_MSG))
	{
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		if (team->commander != clientnum)
		{
			cores.Console_Printf("received CLIENT_COMMANDER_OBJECT_GOAL_MSG from non commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
			return true;
		}

		if (team->selection.numSelected > 0)
		{
			int goal, objidx;
			char *s = GetNextWord(msg);

			goal = atoi(s);
			if (goal < 1 || goal >= NUM_GOALS)
			{
				cores.Console_Printf("received invalid goal in CLIENT_COMMANDER_OBJECT_GOAL_MSG\n");
				return true;
			}

			s = GetNextWord(s);
			objidx = atoi(s);
			
			SV_GroupMoveToObject(team, objidx, goal);
		}

		return true;
	}

	//=========================================================================
	//  Object command sent by an officer to nearby units
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_OFFICER_OBJECT_GOAL_MSG))
	{
#define MAX_OBJECT_SEARCH_AROUND_OFFICER 50
		int i, numObjects, allUnits[1];
		int objects[MAX_OBJECT_SEARCH_AROUND_OFFICER];
		serverObject_t *obj;
		int goal, objidx;
		char *s = GetNextWord(msg);
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		if (sl.gametime < client->nextOfficerCmdTime)		
			return true;
		
		client->nextOfficerCmdTime = sl.gametime + 1000;

		goal = atoi(s);
		if (goal < 1 || goal >= NUM_GOALS)
		{
			cores.Console_Printf("received invalid goal in CLIENT_OFFICER_OBJECT_GOAL_MSG\n");
			return true;
		}

		s = GetNextWord(s);
		objidx = atoi(s);

		if (!SV_IsOfficer(clientnum))
		{
			cores.Console_Printf("received invalid officer order from non-officer (%s, client %i)\n", sl.clients[clientnum].info.name, clientnum);
			return true;
		}

		allUnits[0] = -1;
		numObjects = SV_FindObjectsInRadius(allUnits, 1, sv_officerCommandRadius.value,
						                     sl.objects[clientnum].base.pos, objects, MAX_OBJECT_SEARCH_AROUND_OFFICER);

		i = 0;
		while (i < numObjects)
		{
			obj = &sl.objects[objects[i]];
			if (obj->base.team == team->index
				&& obj->base.health > 0
				&& obj->base.index < MAX_CLIENTS)
			{
				//if (!sl.clients[objects[i]].waypoint.active || !sl.clients[objects[i]].waypoint.commander_order)
					SV_CommanderTargetObject(clientnum, objects[i], objidx, goal);
			}
				
			i++;
		}
		return true;
	}

	//=========================================================================
	//  Location command sent by an officer to nearby units
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_OFFICER_LOCATION_GOAL_MSG))
	{
		int i, numObjects, allUnits[1];
		int objects[MAX_OBJECT_SEARCH_AROUND_OFFICER];
		serverObject_t *obj;
		int goal;
		vec3_t pos;
		teamInfo_t *team = SV_GetTeamInfo(clientnum);
		char *s = GetNextWord(msg);

		if (sl.gametime < client->nextOfficerCmdTime)		
			return true;
		
		client->nextOfficerCmdTime = sl.gametime + 1000;

		goal = atoi(s);
		if (goal < 1 || goal >= NUM_GOALS)
		{
			cores.Console_Printf("received invalid goal in CLIENT_OFFICER_OBJECT_GOAL_MSG\n");
			return true;
		}

		s = GetNextWord(s);
		pos[0] = atoi(s);
		s = GetNextWord(s);
		pos[1] = atoi(s);
		s = GetNextWord(s);
		pos[2] = atoi(s);

		if (!SV_IsOfficer(clientnum))
		{
			cores.Console_Printf("received invalid officer order from non-officer (%s, client %i)\n", sl.clients[clientnum].info.name, clientnum);
			return true;
		}

		allUnits[0] = -1;
		numObjects = SV_FindObjectsInRadius(allUnits, 1, sv_officerCommandRadius.value,
						                     sl.objects[clientnum].base.pos, objects, MAX_OBJECT_SEARCH_AROUND_OFFICER);

		i = 0;
		while (i < numObjects)
		{
			obj = &sl.objects[objects[i]];
			if (obj->base.team == team->index
				&& obj->base.health > 0
				&& obj->base.index < MAX_CLIENTS)
			{
				//if (!sl.clients[objects[i]].waypoint.active || !sl.clients[objects[i]].waypoint.commander_order)
					SV_CommanderTargetPosition(clientnum, objects[i], pos[X], pos[Y], goal);
			}
				
			i++;
		}
		return true;
	}
	//=========================================================================
	// The commander has requested a player receive money from the team coffers
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_GIVEMONEY_MSG))
	{
		char	*s = GetNextWord(msg);
		int		clienttarg = atoi(s), amount;
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		//check for commander
		if (team->commander != clientnum)
		{
			cores.Console_Printf("received CLIENT_COMMANDER_GIVEMONEY_MSG from non commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
			return true;
		}
		
		//invalid clientnum assumes that the commander is targeting the currently selected player
		if (clienttarg < 0 || clienttarg >= MAX_CLIENTS)
		{
			//validate target
			if (team->selection.numSelected == 1 && team->selection.array[0] < MAX_CLIENTS)
			{
				clienttarg = team->selection.array[0];
			}
			else
			{
				cores.Console_Printf("Must have a player selected\n");			
			}
		}

		s = GetNextWord(s);
		amount = atoi(s);
		if (team->resources[raceData[team->race].currency] < amount)
			amount = team->resources[raceData[team->race].currency];

		team->resources[raceData[team->race].currency] -= amount;
		SV_GiveMoney(clienttarg, amount, false);

		return true;
	}

	//=========================================================================
	// The commander has requested a player an object purchased with team money
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_GIVETHING_MSG))
	{
		char	*s = GetNextWord(msg);
		int		clienttarg = atoi(s), object;
		int		difference;
		teamInfo_t *team = SV_GetTeamInfo(clientnum);

		//check for commander
		if (team->commander != clientnum)
		{
			cores.Console_Printf("received CLIENT_COMMANDER_GIVETHING_MSG from non commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
			return true;
		}
		
		if (clienttarg < 0 || clienttarg >= MAX_CLIENTS)
			return true;

		s = GetNextWord(s);
		object = atoi(s);

		difference = objData[object].playerCost - sl.clients[clienttarg].ps.score.money;
		difference = MAX(0, difference);

		if (team->resources[raceData[team->race].currency] < difference)
			return true;

		switch (objData[object].objclass)
		{
		case OBJCLASS_UNIT:
			team->resources[raceData[team->race].currency] -= difference;
			SV_GiveMoney(clienttarg, difference, false);
			SV_PlayerUnitRequest(client, object);
			break;

		case OBJCLASS_WEAPON:
		case OBJCLASS_ITEM:
			team->resources[raceData[team->race].currency] -= difference;
			SV_GiveMoney(clienttarg, difference, false);
			SV_GiveClient(client, object);
			break;

		default:
			break;
		}

		return true;
	}

	//=========================================================================
	//client wants to purchase an upgrade or item that gets constructed inside a building
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_PURCHASE_MSG))
	{
		//parameters: type builder
		int				argc;
		char			*argv[3];
		int				team, builder_index, type;
		serverObject_t *builder;

		argc = SplitArgs(msg, argv, 3);
		if (argc < 3)
			return true;

		type = atoi(argv[1]);
		builder_index = atoi(argv[2]);

		if (!SV_ClientIsCommander(clientnum))
			return true;

		if (builder_index < MAX_CLIENTS || builder_index >= MAX_OBJECTS)
			return true;

		team = SV_GetTeam(clientnum);
		builder = &sl.objects[builder_index];

		if (builder->base.team != team)
			return true;

		SV_PurchaseObject(team, type, builder, false);

		return true;
	}

	//=========================================================================
	//A player has requested to change their current unit type
	//=========================================================================
	else if (SV_MessageType(msg, "unit"))
	{
		SV_SelectPlayerUnit(clientnum, &msg[5]);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, "spawnfrom"))
	{
		int obj = atoi(GetNextWord(msg));
		
		if (obj < -1 || obj >= MAX_OBJECTS)
			cores.Console_DPrintf("Received invalid spawnfrom index from client %i!\n", clientnum);
		else
			SV_SpawnPlayerFrom(client, obj);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (strcmp(msg, CLIENT_KILL_MSG)==0)
	{
		vec3_t dmgpos = { sl.clients[clientnum].ps.pos[0] + M_Randnum(-10,10),
							sl.clients[clientnum].ps.pos[1] + M_Randnum(-10,10),
							sl.clients[clientnum].ps.pos[2] + M_Randnum(-10,10) };
		SV_DamageTarget(&sl.objects[clientnum], &sl.objects[clientnum], dmgpos, 0, 999999, DAMAGE_NO_KNOCKBACK | DAMAGE_EXPLOSIVE);
		sl.clients[clientnum].ps.score.money = 0;
		//SV_KillClient(&sl.clients[clientnum], sl.clients[clientnum].obj, sl.clients[clientnum].ps.pos, 0, 0);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (strcmp(msg, CLIENT_EJECT_MSG)==0)
	{
		SV_Eject(clientnum);
		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_GIVE_MSG))
	{
		byte item = atoi(GetNextWord(msg));

		SV_GiveClient(client, item);
		return true;
	}

	//=========================================================================
	//A player is selling an object back to the base
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_GIVEBACK_MSG))
	{
		int slot = atoi(GetNextWord(msg));

		if (slot >= 0 && slot < MAX_INVENTORY)
			SV_GiveBackFromClient(client, slot);

		return true;
	}

	//=========================================================================
	//A player wishes to become commander
	//=========================================================================
	else if ((strcmp(msg, CLIENT_COMMANDER_REQUEST_MSG))==0)
	{
		if (sl.clients[clientnum].info.team != TEAM_UNDECIDED)
			SV_CommanderElect(client, &sl.teams[sl.clients[clientnum].info.team]);

		return true;
	}

	//=========================================================================
	//A commander wishes to leave his post
	//=========================================================================
	else if ((strcmp(msg, CLIENT_COMMANDER_RESIGN_MSG))==0)
	{
		if (SV_ClientIsCommander(clientnum))
			SV_CommanderResign(clientnum, false);
		else
		{
			cores.Console_Printf("Received resign request from a non-commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
		}

		return true;
	}

	//=========================================================================
	//A commander has requested a player be promoted
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_PROMOTE_MSG))
	{
		int team, slot, newofficer;
		char *s;

		if (!SV_ClientIsCommander(clientnum))
			return true;

		s = GetNextWord(msg);
		slot = atoi(s);
		s = GetNextWord(s);
		newofficer = atoi(s);
		team = SV_GetTeam(clientnum);

		if (newofficer < 0 || newofficer >= MAX_CLIENTS)
			return true;
		if (SV_GetTeam(newofficer) != team)
			return true;

		SV_DemoteOfficer(team, slot);
		SV_PromoteClient(&sl.clients[newofficer], slot);

		return true;
	}
	//=========================================================================
	//A commander has requested a player be demoted
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_DEMOTE_MSG))
	{
		int team, officernum;

		//must be commander to do this
		if (!SV_ClientIsCommander(clientnum))
			return true;

		officernum = atoi(GetNextWord(msg));
		team = sl.clients[clientnum].info.team;

		SV_DemoteOfficer(team, officernum);
		
		return true;
	}

	//=========================================================================
	//=========================================================================
	else if ((strcmp(msg, CLIENT_START_GAME_MSG))==0)
	{
		SV_ClientStartGameRequest(client);
		
		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_SELECTION_MSG))
	{
		int n;
		char *s = GetNextWord(msg);
		int numsel = atoi(s);
		teamInfo_t *team = SV_GetTeamInfo(clientnum);
		int clnum;

		if (numsel > MAX_SELECTABLE_UNITS || numsel < 0)
		{
			cores.Console_Printf("Invalid selection number received in CLIENT_COMMANDER_SELECTION_MSG\n");
			return true;
		}
		if (clientnum != team->commander)
		{
			cores.Console_Printf("Selection message received from a non commander!\n");
			SV_SendNotice(clientnum, ERROR_NOTCOMMANDER, 0, "");
			return true;
		}

		team->selection.numSelected = 0;

		for (clnum = 0; clnum < MAX_CLIENTS; clnum++)
		{
			client_t *client = &sl.clients[clnum];
			if (client->info.team == sl.clients[clientnum].info.team)
			{
				client->ps.flags &= ~PS_COMMANDER_SELECTED;
			}
		}
						
		for (n = 0; n < numsel; n++)
		{
			int objidx;

			s = GetNextWord(s);
			objidx = atoi(s);
			
			//for the server side selection array, only allow selection of mobile units on the commander's team.
			//other units like structures don't concern the server code, and we don't want any bugs where
			//the commander might be able to control units not on his team, etc
			if (objidx < 0 || objidx > MAX_OBJECTS)
			{
				cores.Console_Printf("Invalid selection object received in CLIENT_COMMANDER_SELECTION_MSG\n");

				return true;
			}			

			if (sl.objects[objidx].base.team != sl.clients[clientnum].info.team)
				continue;

			team->selection.array[team->selection.numSelected] = objidx;

			team->selection.numSelected++;

			if (objidx < MAX_CLIENTS)
			{
				sl.clients[objidx].ps.flags |= PS_COMMANDER_SELECTED;
				Phys_AddEvent(&sl.clients[objidx].ps, EVENT_COMMANDER_SELECTED, 0, 0);
			}
		}

		team->selection.mobileUnitsSelected = true;
				
		return true;
	}

	//=========================================================================
	//commander activated an upgrade
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_ACTIVATE_MSG))
	{
        char *pos;
		int targ, tech;

        pos = &msg[strlen(CLIENT_COMMANDER_ACTIVATE_MSG)+1];
		tech = atoi(pos);
		pos = strchr(pos, ' ')+1;
		targ = atoi(pos);

		SV_ActivateUpgrade(clientnum, targ, tech);
		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_DEMOLISH_MSG))
	{
		char	*pos = GetNextWord(msg);
		int		targ;
		serverObject_t	*targobj;

		targ = atoi(pos);

		if (targ < 0 || targ >= MAX_OBJECTS)
			return true;

		targobj = &sl.objects[targ];
		if (SV_GetTeam(targ) != SV_GetTeam(clientnum))
			return true;

		if (sl.teams[SV_GetTeam(targ)].nextDemolishTime > sl.gametime)
		{
			SV_ClientEcho(clientnum, "Trying to demolish too quickly.\n");
			return true;
		}
		
		if (IsBuildingType(targobj->base.type) && targ != sl.teams[SV_GetTeam(targ)].command_center)
		{
			int refund, i;
			objectData_t	*bldData = GetObjectByType(targobj->base.type);

			for (i = 0; i < MAX_RESOURCE_TYPES; i++)
			{
				if (bldData->cost[i])
				{

					refund = bldData->cost[i] * (targobj->base.health / (float)targobj->base.fullhealth);
					refund /= 2;
					sl.teams[SV_GetTeam(targ)].resources[i] += refund;
				}
			}

			SV_DamageTarget(targobj, targobj, targobj->base.pos, 0, 999999, 0);
			SV_AddExperience(&sl.clients[clientnum], EXPERIENCE_COMMANDER_DEMOLISH, 0, bldData->expMult);
			sl.teams[SV_GetTeam(targ)].nextDemolishTime = sl.gametime + sv_demolishInterval.integer;
		}
		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_SUICIDE_MSG))
	{
		char	*pos = GetNextWord(msg);
		int		targ;
		serverObject_t	*targobj;

		targ = atoi(pos);

		if (targ < 0 || targ >= MAX_OBJECTS)
			return true;

		targobj = &sl.objects[targ];
		if (SV_GetTeam(targ) != SV_GetTeam(clientnum))
			return true;

		if (IsWorkerType(targobj->base.type) || IsItemType(targobj->base.type))
		{
			if (targobj->kill)
				targobj->kill(targobj, targobj, targobj->base.pos, 0, 0);
			else
				SV_DamageTarget(targobj, targobj, targobj->base.pos, 0, 65535, 0);
		}

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_COMMANDER_STOP_RESEARCH_MSG))
	{
		char	*pos = GetNextWord(msg);
		int		targ, index;
		serverObject_t	*targobj;

		targ = atoi(pos);

		if (targ < MAX_CLIENTS || targ >= MAX_OBJECTS)
			return true;

		targobj = &sl.objects[targ];
		if (SV_GetTeam(targ) != SV_GetTeam(clientnum))
			return true;

		if (IsBuildingType(targobj->base.type) &&
			targobj->base.type != targobj->itemConstruction)
		{
			int type;

			//refund
			for (index = 0; index < MAX_RESOURCE_TYPES; index++)
				SV_GiveResourceToTeam(SV_GetTeam(targ), index, objData[targobj->itemConstruction].cost[index]);

			//stop the research
			SV_CancelConstruction(targ);

			//see if they have something in the build queue
			type = SV_GetQueuedRequest(targ);
			while (type >= 0)
			{
				if (Tech_IsAvailable(type, sl.teams[SV_GetTeam(targ)].research))
				{
					SV_PurchaseObject(SV_GetTeam(targ), type, targobj, false);
					break;
				}
				else
				{
					SV_RemoveQueuedRequest(targ, (byte)type);
				}

				type = SV_GetQueuedRequest(targ);
			}
		}

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_VOTE_MSG))
	{
		char	*s = GetNextWord(msg);

		switch(*s)
		{
			case 'y':				
			case 'Y':
			case '1':
				SV_ClientCastVote(&sl.clients[clientnum], VOTE_YES);
				break;
			case 'n':
			case 'N':
			case '0':
				SV_ClientCastVote(&sl.clients[clientnum], VOTE_NO);
				break;
			default:
				return true;
		}

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_CALLVOTE_MSG))
	{
		if (sl.status == GAME_STATUS_ENDED)
		{
			if (sv_newPatchAvailable.integer && sv_autoPatchMyServer.integer)
			{
				SV_ClientEcho(clientnum, "No votes can be called now, the server is scheduled to go down at the end of this match.");
				return true;
			}
		}
		
		SV_CallVote(clientnum, argcount-1, &argv[1]);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_REFEREE_MSG))
	{
		if (argcount < 2)
			return true;

		if (strlen(sv_refereePassword.string) > 1)
		{
			if (strcmp(sv_refereePassword.string, argv[1]) == 0)
			{
				SV_SetReferee(&sl.clients[clientnum], false);
			}
		}
		else
		{
			SV_ClientEcho(clientnum, "Invalid referee password\n");
		}

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_REFEREE_COMMAND_MSG))
	{
		SV_RefereeCommand(&sl.clients[clientnum], argcount-1, &argv[1]);

		return true;
	}

	//=========================================================================
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_READY_MSG))
	{
		SV_SetReady(&sl.clients[clientnum]);

		return true;
	}

	//=========================================================================
	// Client is giving up their chance of being revived and asking to show the loadout screen
	//=========================================================================
	else if (SV_MessageType(msg, CLIENT_SHOW_LOADOUT_MSG))
	{
		if (SV_WaitingToBeRevived(client) || (client->info.team && client->ps.status != STATUS_PLAYER))
		{		
			SV_RelinquishLife(client);
			SV_SetClientStatus(client->index, STATUS_UNIT_SELECT);
		}
		
		return true;
	}

	return false;			//we want the core engine to handle this message
}

void	SV_NewPatchAvailable()
{
	int i;
	
	if (sv_autoPatchMyServer.integer)
	{

		if (cores.Server_GetNumClients() == 0)
			cores.Cmd_Exec("quit");
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (!sl.clients[i].active)
				continue;

			SV_ClientEcho(i, "This server will be going down temporarily at the end of this game to update with a new patch.\n");
		}
		cores.Cvar_SetVarValue(&sv_newPatchAvailable, 1);
	}
}

char 	*SV_BuildPlayerListString()
{
	static char s[1024];
	char *line = NULL;
	int team, i, j;

	s[0] = 0;

	for (team = 1; team < MAX_TEAMS; team++)
	{
		line = fmt("Team %i (%s):\n", team, raceData[sl.teams[team].race].name);
		if (strlen(s) + strlen(line) < 1024)
			strcat(s, line);

		j = 0;
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (!sl.clients[i].active || sl.clients[i].info.team != team)
				continue;
			line = fmt("%s%s\n", sl.clients[i].info.name, SV_ClientIsCommander(i) ? " (Commander)" : "");
			if (strlen(s) + strlen(line) < 1024)
				strcat(s, line);
			j++;
		}
		if (j == 0)
		{
			line = "--empty team--\n";
			if (strlen(s) + strlen(line) < 1024)
				strcat(s, line);
		}
	}
	return s;
}

/*==========================

  SV_BuildInfoString

  Passes back a string to send to clients asking for info requests from the server.

  Core sends server name, num clients, max clients, and world name, so that info
  shouldn't be put here.  This is for passing back some game specific info.

 ==========================*/

void	SV_BuildInfoString(char *buf, int size)
{
	int needCmdr;

	buf[0] = 0;
	
	ST_SetState(buf, "gametype", GetGametypeName(sl.gametype), size);

	if (sl.teams[1].commander == -1 || sl.teams[2].commander == -1)
		needCmdr = 1;
	else
		needCmdr = 0;

	ST_SetState(buf, "needcmdr", fmt("%i", needCmdr), size);

	if (sv_teamDamage.integer)
		ST_SetState(buf, "td", "1", size);

	ST_SetState(buf, "pass", cores.Cvar_GetString("svr_password")[0] ? "1" : "0", size);

	if (p_speed.value != p_speed.default_value || p_jumpheight.value != p_jumpheight.default_value || p_gravity.value != p_gravity.default_value)
		ST_SetState(buf, "pspd", "1", size);
	
	ST_SetState(buf, "race1", raceData[sl.teams[1].race].name, size);
	ST_SetState(buf, "race2", raceData[sl.teams[2].race].name, size);
	
	ST_SetState(buf, "demo", cores.Cvar_GetString("svr_demo"), size);

	ST_SetState(buf, "pure", cores.Cvar_GetString("svr_pure"), size);
	ST_SetState(buf, "bal", sv_balancedTeams.string, size);
}

/*==========================

  SV_BuildExtendedInfoString

  Passes back a string to send to clients asking for detailed info requests from the server.

  Core sends server name, num clients, max clients, and world name, so that info
  shouldn't be put here.  This is for passing back some game specific info.

 ==========================*/

void	SV_BuildExtendedInfoString(char *buf, int size)
{
	SV_BuildInfoString(buf, size);
	
	ST_SetState(buf, "timelimit", fmt("%i", sv_timeLimit.integer/1000), size);
	ST_SetState(buf, "notes", sv_serverNotes.string, size);
	
	ST_SetState(buf, "players", SV_BuildPlayerListString(), size);
}


/*==========================

  SV_GetPosition

  Sets <outpos> to the location of object <objnum> using the correct field
  returns 1 if successful, 0 if the object was invalid or inactive

  ==========================*/

bool	SV_GetPosition(int objnum, vec3_t outpos)
{
	if (objnum < 0 || objnum >= MAX_OBJECTS)
		return false;

	if (!sl.objects[objnum].base.active)
		return false;

	if (objnum < MAX_CLIENTS)
		M_CopyVec3(sl.clients[objnum].ps.pos, outpos);
	else
		M_CopyVec3(sl.objects[objnum].base.pos, outpos);

	return true;
}


/*==========================

  SV_Mute_Cmd

 ==========================*/

void	SV_Mute_Cmd(int argc, char *argv[])
{
	int clientnum;

	if (!argc)
		return;

	clientnum = atoi(argv[0]);
	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return;

	if (sl.clients[clientnum].active)
	{
		sl.clients[clientnum].mute = true;
		SV_SendNotice(clientnum, NOTICE_GENERAL, 0, "You have been muted.\n");
	}
}


/*==========================

  SV_UnMute_Cmd

 ==========================*/

void	SV_UnMute_Cmd(int argc, char *argv[])
{
	int clientnum;

	if (!argc)
		return;

	clientnum = atoi(argv[0]);
	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return;

	if (sl.clients[clientnum].active)
	{
		sl.clients[clientnum].mute = false;
		SV_SendNotice(clientnum, NOTICE_GENERAL, 0, "Your voice has been restored.");
	}
}

void	SV_SVMem_Cmd(int argc, char *argv[])
{
	cores.Console_Printf("sizeof(serverLocal_t) == %i\n", sizeof(serverLocal_t));
}


void	SV_Init()
{	
	//clear all data
	memset(&sl, 0, sizeof(sl));
	sl.lastActiveObject = MAX_CLIENTS;

	memset(&held, 0, sizeof(held));

	//register cvars
	cores.Cvar_Register(&sv_allowGuestReferee);
	cores.Cvar_Register(&sv_refereePassword);
	cores.Cvar_Register(&sv_nextMap);
	cores.Cvar_Register(&sv_respawnNPCInterval);		
	cores.Cvar_Register(&sv_respawnNPCs);
	cores.Cvar_Register(&sv_fastTech);
	cores.Cvar_Register(&sv_hitStructureRewardScale);
	cores.Cvar_Register(&sv_rangedHitStructureRewardScale);
	cores.Cvar_Register(&sv_unitAngleLerp);
	cores.Cvar_Register(&sv_aiIgnoreSlope);
	cores.Cvar_Register(&sv_startingTeamGold);
	cores.Cvar_Register(&sv_startingTeamStone);
	cores.Cvar_Register(&sv_writeScores);
	cores.Cvar_Register(&sv_aiPredict);
	cores.Cvar_Register(&sv_repairCost);
	cores.Cvar_Register(&sv_npcs);
	cores.Cvar_Register(&sv_autojoin);
	cores.Cvar_Register(&sv_debug);
	cores.Cvar_Register(&sv_debugBuilding);
	cores.Cvar_Register(&sv_respawnMultiplier);
	cores.Cvar_Register(&sv_newPatchAvailable);
	cores.Cvar_Register(&sv_autoPatchMyServer);
	cores.Cvar_Register(&sv_buildingInitialHealthPercent);
	cores.Cvar_Register(&sv_placementLeniency);
	cores.Cvar_Register(&sv_gametype);
	cores.Cvar_Register(&sv_readyPercent);
	cores.Cvar_Register(&sv_officerCommandRadius);
	cores.Cvar_Register(&sv_timeLimit);
	cores.Cvar_Register(&sv_easterEggs);
	cores.Cvar_Register(&sv_activeEasterEggs);
	cores.Cvar_Register(&sv_doSetup);
	cores.Cvar_Register(&sv_doWarmup);
	cores.Cvar_Register(&sv_warmupTime);
	cores.Cvar_Register(&sv_setupTime);
	cores.Cvar_Register(&sv_endGameTime);
	cores.Cvar_Register(&sv_nextStatus);
	cores.Cvar_Register(&sv_team1race);
	cores.Cvar_Register(&sv_team2race);
	cores.Cvar_Register(&sv_todStart);
	cores.Cvar_Register(&sv_todSpeed);
	cores.Cvar_Register(&sv_nextMap);
	cores.Cvar_Register(&sv_nextMapCmd);
	cores.Cvar_Register(&sv_playerChosenMap);
	cores.Cvar_Register(&sv_minPlayers);
	cores.Cvar_Register(&sv_clientConnectMoney);
	cores.Cvar_Register(&sv_chatFloodInterval);
	cores.Cvar_Register(&sv_chatFloodCount);
	cores.Cvar_Register(&sv_chatFloodPenaltyTime);
	cores.Cvar_Register(&sv_serverNotes);
	cores.Cvar_Register(&sv_demolishInterval);
	cores.Cvar_Register(&sv_showClanAbbrev);
	cores.Cvar_Register(&sv_motd1);
	cores.Cvar_Register(&sv_motd2);
	cores.Cvar_Register(&sv_motd3);
	cores.Cvar_Register(&sv_motd4);

	//register commands
	cores.Cmd_Register("restartmatch",	SV_RestartMatch_Cmd);
	cores.Cmd_Register("giveresource",	SV_GiveResource_Cmd);
	cores.Cmd_Register("endgame",		SV_EndGame_Cmd);
	cores.Cmd_Register("givemoney",		SV_GiveMoney_Cmd);
	cores.Cmd_Register("writescores",	SV_WriteScores_Cmd);
	cores.Cmd_Register("mute",			SV_Mute_Cmd);
	cores.Cmd_Register("unmute",		SV_UnMute_Cmd);
	cores.Cmd_Register("svmem",			SV_SVMem_Cmd);
	cores.Cmd_Register("svchat",		SV_SVChat_Cmd);

#ifdef S2_INTERNAL_DEV_TESTING
//	cores.Cmd_Register("remote", SV_Remote_Cmd);
//	cores.Cmd_Register("waypointpos", SV_WaypointPos_Cmd);
//	cores.Cmd_Register("waypointobj", SV_WaypointObj_Cmd);
#endif

	//init tech tree
	Tech_Generate(GetObjectByType);

	numPeonNames = 1; //start at 1 because 0 is reserved for "not named"

	//count the number of peon names
	while (peonNames[numPeonNames][0])
		numPeonNames++;

	Phys_Init();

	SV_InitBuildings();
	SV_InitWeapons();
	SV_InitClients();
	SV_InitVirtualClients();
	SV_InitItems();
	SV_InitEvents();
	SV_InitTeams();
	SV_InitVotes();
	SV_InitExperience();
	SV_InitGameScript();
}

void 	SV_NotFirewalled()
{
	cores.Console_Printf("we are apparently not firewalled, hooray!\n");
}

void	SV_KeyserverString(char *pos)
{
	char *clan_id_text, *abbrev;

	cores.Console_DPrintf("Received keyserver string: '%s'\n", pos);
	
	clan_id_text = ST_GetState(pos, "clan");
	abbrev = ST_GetState(pos, "ca");
	//cookie = ST_GetState(pos, "cookie");

	ST_SetState(clanInformation, fmt("c%sa", clan_id_text), abbrev, 4096);
	//ST_SetState(connectingPlayerInformation, fmt("c%sa", clan_id_text), ST_GetState(pos, "ca"));
	cores.Server_SetStateString(ST_CLAN_INFO, clanInformation);
}

void	SV_InitAPIs(coreAPI_server_t *core_api, serverAPI_t *server_api)
{
	memcpy(&cores, core_api, sizeof(coreAPI_server_t));	

	server_api->Init = SV_Init;
	server_api->ClientConnect = SV_ClientConnect;
	server_api->ClientDisconnect = SV_ClientDisconnect;
	server_api->NewNetSettings = SV_NewNetSettings;	
	server_api->ProcessClientInput = SV_ProcessClientInput;
	server_api->Frame = SV_Frame;
	server_api->Reset = SV_Reset;
	server_api->Shutdown = SV_Shutdown;
	server_api->ClientMessage = SV_ClientMessage;
	server_api->BuildInfoString = SV_BuildInfoString;
	server_api->BuildExtendedInfoString = SV_BuildExtendedInfoString;
	server_api->KeyserverString = SV_KeyserverString;
	server_api->NewPatchAvailable = SV_NewPatchAvailable;
	server_api->NotFirewalled = SV_NotFirewalled;
}

