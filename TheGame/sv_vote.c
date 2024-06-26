// (C) 2003 S2 Games

// sv_vote.c


#include "server_game.h"

cvar_t	sv_clientVoteTime =		{ "sv_clientVoteTime",		"60000" };
cvar_t	sv_impeachPenaltyTime =	{ "sv_impeachPenaltyTime",	"60000" };
cvar_t	sv_minVotePercent =		{ "sv_minVotePercent",		".50" };
cvar_t	sv_allowMapVotes = { "sv_allowMapVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowMsgVotes = { "sv_allowMsgVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowKickVotes = { "sv_allowKickVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowMuteVotes = { "sv_allowMuteVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowRestartVotes = { "sv_allowRestartVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowTimeVotes = { "sv_allowTimeVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowImpeachVotes = { "sv_allowImpeachVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowConcedeVotes = { "sv_allowConcedeVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowRefVotes = { "sv_allowRefVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_allowRaceVotes = { "sv_allowRaceVotes", "1", CVAR_SAVECONFIG };
cvar_t	sv_disableVoting = { "sv_disableVoting", "0", CVAR_SAVECONFIG };

#define DECLARE_VOTE(name) \
	bool SV_##name##VoteStart(int initiatingClient, int argc, char *argv[]); \
	void SV_##name##VotePassed();


DECLARE_VOTE(ImpeachCommander)
DECLARE_VOTE(Concede)
DECLARE_VOTE(KickPlayer)
DECLARE_VOTE(KickNum)
DECLARE_VOTE(MutePlayer)
DECLARE_VOTE(ChangeMap)
DECLARE_VOTE(RestartGame)
DECLARE_VOTE(ChangeSetting)
DECLARE_VOTE(Message)
DECLARE_VOTE(Time)
DECLARE_VOTE(NextMap)
DECLARE_VOTE(Ref)
DECLARE_VOTE(Restart)
DECLARE_VOTE(Race)


votable_t votables[] =
{
/*	{
		"Elect New Commander",
		"elect",
		SV_ElectCommanderVoteStart,
		SV_ElectCommanderVotePassed,
		VOTABLE_TEAM,
		0.67
	},*/
	{
		"Impeach Commander",
		"impeach",
		SV_ImpeachCommanderVoteStart,
		SV_ImpeachCommanderVotePassed,
		VOTABLE_TEAM,
		0.67,
		&sv_allowImpeachVotes
	},
	{
		"Concede",
		"concede",
		SV_ConcedeVoteStart,
		SV_ConcedeVotePassed,
		VOTABLE_TEAM,
		0.67,
		&sv_allowConcedeVotes
	},
	{
		"Change Map",
		"world",
		SV_ChangeMapVoteStart,
		SV_ChangeMapVotePassed,
		VOTABLE_ALL,
		0.50,
		&sv_allowMapVotes
	},
	{
		"Next Map",
		"nextmap",
		SV_NextMapVoteStart,
		SV_NextMapVotePassed,
		VOTABLE_ALL,
		0.50,
		&sv_allowMapVotes
	},
	{
		"Kick Player",
		"kick",
		SV_KickPlayerVoteStart,
		SV_KickPlayerVotePassed,
		VOTABLE_ALL,
		0.50,
		&sv_allowKickVotes
	},
	{
		"Kick Player by ID",
		"kicknum",
		SV_KickNumVoteStart,
		SV_KickPlayerVotePassed,
		VOTABLE_ALL,
		0.50,
		&sv_allowKickVotes
	},
	{
		"Mute Player",
		"mute",
		SV_MutePlayerVoteStart,
		SV_MutePlayerVotePassed,
		VOTABLE_ALL,
		0.50,
		&sv_allowMuteVotes
	},
	{
		"Random Message",
		"msg",
		SV_MessageVoteStart,
		SV_MessageVotePassed,
		VOTABLE_ALL,
		0.50,
		&sv_allowMsgVotes,
	},
	{
		"Extend Time",
		"time",
		SV_TimeVoteStart,
		SV_TimeVotePassed,
		VOTABLE_ALL,
		0.50,
		&sv_allowTimeVotes
	},
	{
		"Set Referee",
		"ref",
		SV_RefVoteStart,
		SV_RefVotePassed,
		VOTABLE_ALL,
		0.66,
		&sv_allowRefVotes
	},
	{
		"Restart Match",
		"restartmatch",
		SV_RestartVoteStart,
		SV_RestartVotePassed,
		VOTABLE_ALL,
		0.67,
		&sv_allowRestartVotes
	},
	{
		"Change Team's Race",
		"race",
		SV_RaceVoteStart,
		SV_RaceVotePassed,
		VOTABLE_TEAM,
		0.67,
		&sv_allowRaceVotes
	},
	{
		"",
		"",
	}
};


//=================================================
// Impeach commander
//=================================================

bool	SV_ImpeachCommanderVoteStart(int initiatingClient, int argc, char *argv[])
{
	int cmdr;
	client_t *cmdrclient;
	int team = sl.clients[initiatingClient].info.team;

	if (!team)
		return false;

	cmdr = sl.teams[team].commander;
	if (cmdr == -1)
		return false;

	cmdrclient = &sl.clients[cmdr];

	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Impeach commander: %s\ncalled by: %s", cmdrclient->info.name, sl.clients[initiatingClient].info.name));
	sl.voteInfo.affectedClient = cmdrclient;

	return true;
}

void	SV_ImpeachCommanderVotePassed()
{
	if (sl.voteInfo.team->commander == sl.voteInfo.affectedClient->index)
	{
		SV_CommanderResign(sl.voteInfo.affectedClient->index, true);
		sl.clients[sl.voteInfo.affectedClient->index].stats.impeached++;
		sl.clients[sl.voteInfo.affectedClient->index].canCommandTime = sl.gametime + (sv_impeachPenaltyTime.integer);
	}

	SV_BroadcastNotice(NOTICE_GENERAL, 0, fmt("%s has been impeached!", sl.voteInfo.affectedClient->info.name));
}


//=================================================
// Set Referee
//=================================================

extern cvar_t sv_allowGuestReferee;

bool	SV_RefVoteStart(int initiatingClient, int argc, char *argv[])
{
	client_t *client = &sl.clients[initiatingClient];

	if (!sv_allowGuestReferee.integer)
	{
		SV_ClientEcho(initiatingClient, "Guest referees are not allowed on this server\n");
		return false;
	}

	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Set referee: %s", client->info.name));

	sl.voteInfo.affectedClient = client;

	return true;
}

void	SV_RefVotePassed()
{
	SV_SetReferee(sl.voteInfo.affectedClient, true);
}


//=================================================
// Restart Match
//=================================================

bool	SV_RestartVoteStart(int initiatingClient, int argc, char *argv[])
{	
	client_t *client = &sl.clients[initiatingClient];
	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Restart match.\ncalled by: %s", client->info.name));

	return true;
}

void	SV_RestartVotePassed()
{
	cores.Cmd_BufPrintf("restartmatch\n");
}


//==================================================
// Race
//==================================================

bool	SV_RaceVoteStart(int initiatingClient, int argc, char *argv[])
{
	int r;

	client_t *client = &sl.clients[initiatingClient];
	if (!argc)
	{		
		return false;
	}
	if (sl.status != GAME_STATUS_SETUP)
	{
		SV_ClientEcho(initiatingClient, "Race votes are only valid during GAME SETUP\n");
		return false;
	}
	if (!client->info.team)
	{
		SV_ClientEcho(initiatingClient, "You must join a team to call a race vote\n");
		return false;
	}

	for (r=1; r<MAX_RACES; r++)
	{
		if (stricmp(argv[0], raceData[r].name)==0)
			break;
	}
	if (r == MAX_RACES)
	{
		SV_ClientEcho(initiatingClient, "Invalid race name\n");
		return false;
	}

	if (sl.teams[client->info.team].race == r)
	{
		return false;
	}

	strcpy(sl.voteInfo.command, argv[0]);
	sl.voteInfo.team = &sl.teams[client->info.team];

	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Team %i: Change Race to %s\ncalled by: %s", client->info.team, argv[0], client->info.name));

	return true;
}

void	SV_RaceVotePassed()
{
	cores.Cmd_BufPrintf("set sv_team%irace %s; restartmatch\n", sl.voteInfo.team->index, sl.voteInfo.command);	
}


//==================================================
// Kick player
//==================================================
bool	SV_KickPlayerVoteStart(int initiatingClient, int argc, char *argv[])
{
	int numfound = 0;
	int clientnum;
	client_t *client = &sl.clients[initiatingClient];

	if (!argc)
		return false;

	clientnum = SV_FindPlayerSubString(argv[0], &numfound);

	if (numfound == 1)
	{
		//vote isn't ambiguous, so start it
		sl.voteInfo.affectedClient = &sl.clients[clientnum];
		cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Kick: %s\ncalled by: %s",sl.clients[clientnum].info.name, client->info.name));
		//cores.Server_SetStateString(ST_VOTE_TARGET, sl.clients[clientnum].info.name);
		return true;
	}

	return false;
}

bool	SV_KickNumVoteStart(int initiatingClient, int argc, char *argv[])
{
	int numfound = 0;
	int clientnum;
	client_t *client = &sl.clients[initiatingClient];

	if (!argc)
		return false;

	clientnum = atoi(argv[0]);
	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return false;
	if (!sl.clients[clientnum].active)
		return false;

	//vote isn't ambiguous, so start it
	sl.voteInfo.affectedClient = &sl.clients[clientnum];
	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Kick: %s\ncalled by: %s",sl.clients[clientnum].info.name, client->info.name));
	//cores.Server_SetStateString(ST_VOTE_TARGET, sl.clients[clientnum].info.name);
	return true;
}

void	SV_KickPlayerVotePassed()
{
	sl.clients[sl.voteInfo.affectedClient->index].stats.kicked++;
	cores.Cmd_Exec(fmt("kicknum %i \"You have been voted out of the game\"\n", sl.voteInfo.affectedClient->index));
}


//==================================================
// Mute player
//==================================================
bool	SV_MutePlayerVoteStart(int initiatingClient, int argc, char *argv[])
{
	int numfound = 0;
	int clientnum;
	client_t *client = &sl.clients[initiatingClient];

	if (!argc)
		return false;

	clientnum = SV_FindPlayerSubString(argv[0], &numfound);

	if (numfound == 1)
	{
		if (sl.referee)
		{
			if (sl.referee->index == initiatingClient)
			{
				SV_ClientEcho(initiatingClient, "Can not mute the referee");
				return false;
			}
		}

		//vote isn't ambiguous, so start it
		sl.voteInfo.affectedClient = &sl.clients[clientnum];
		cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Mute: %s\ncalled by: %s", sl.clients[clientnum].info.name, client->info.name));
		//cores.Server_SetStateString(ST_VOTE_TARGET, sl.clients[clientnum].info.name);
		return true;
	}

	return false;
}

void	SV_MutePlayerVotePassed()
{
	cores.Cmd_Exec(fmt("mute %i\n", sl.voteInfo.affectedClient->index));
}


//==================================================
// Change map
//==================================================

bool	SV_ChangeMapVoteStart(int initiatingClient, int argc, char *argv[])
{
	client_t *client = &sl.clients[initiatingClient];

	if (!argc)
		return false;

	if (!core.File_Exists(fmt("/world/%s.s2z", argv[0])))
		return false;

	strcpy(sl.voteInfo.command, argv[0]);

	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Load map: %s\ncalled by: %s", argv[0], client->info.name));
	//cores.Server_SetStateString(ST_VOTE_TARGET, argv[0]);

	return true;
}

void	SV_ChangeMapVotePassed()
{
	cores.Cvar_Set("sv_playerChosenMap", fmt("%s", sl.voteInfo.command));	
	SV_SetGameStatus(GAME_STATUS_PLAYERCHOSENMAP, sl.gametime + 2000, NULL);
}


//==================================================
// Next map
//==================================================

extern cvar_t sv_nextMap;

bool	SV_NextMapVoteStart(int initiatingClient, int argc, char *argv[])
{
	client_t *client = &sl.clients[initiatingClient];

	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Load next map.\ncalled by: %s", client->info.name));
	//cores.Server_SetStateString(ST_VOTE_TARGET, sv_nextMap.string);

	return true;
}

void	SV_NextMapVotePassed()
{
	SV_SetGameStatus(GAME_STATUS_NEXTMAP, sl.gametime + 2000, NULL);
}

//==================================================
// Concede
//==================================================

bool	SV_ConcedeVoteStart(int initiatingClient, int argc, char *argv[])
{
	int team = sl.clients[initiatingClient].info.team;

	if (!team)
		return false;

	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Concede match.\ncalled by: %s", sl.clients[initiatingClient].info.name));	

	return true;
}

void	SV_ConcedeVotePassed()
{
	SV_EndGame(sl.voteInfo.team->index == 1 ? 2 : 1);
}

//==================================================
// Message
//==================================================

bool	SV_MessageVoteStart(int initiatingClient, int argc, char *argv[])
{
	char msg[1024];

	if (!argc)
		return false;

	ConcatArgs(argv, argc, msg);

	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("%s\ncalled by: %s", msg, sl.clients[initiatingClient].info.name));

	return true;
}

void	SV_MessageVotePassed()
{
}

//==================================================
// Time
//==================================================

bool	SV_TimeVoteStart(int initiatingClient, int argc, char *argv[])
{
	if (sl.status != GAME_STATUS_SETUP &&
		sl.status != GAME_STATUS_NORMAL)
		return false;

	cores.Server_SetStateString(ST_VOTE_DESCRIPTION, fmt("Extend timelimit.\ncalled by: %s", sl.clients[initiatingClient].info.name));

	return true;
}

void	SV_TimeVotePassed()
{
	SV_ExtendTime(600000);		//extend time by 10 mins
}


/*==========================

  SV_EnoughVotes

 ==========================*/

bool	SV_EnoughVotes()
{
	int		totalVoters;
	float	percent;

	//work out percentage who voted
	if (sl.voteInfo.votable->votetype == VOTABLE_TEAM)
		totalVoters = sl.voteInfo.team->num_players;
	else
		totalVoters = sl.numClients;
	percent = (float)(sl.votesYes + sl.votesNo) / totalVoters;

	//if the timer has not expired, only return true if the result is locked
	if (sl.gametime < sl.voteEndTime)
	{
		if (percent > sv_minVotePercent.value &&
			((sl.votesYes / (float)totalVoters) > sl.voteInfo.votable->minPercent ||
			(sl.votesNo / (float)totalVoters) > (1.0 - sl.voteInfo.votable->minPercent)))
			return true;

		return false;
	}

	if (percent > sv_minVotePercent.value)
		return true;

	return false;
}


/*==========================

  SV_ClientCastVote

 ==========================*/

void	SV_ClientCastVote(client_t *client, int vote)
{
	if (!sl.voteInProgress)
		return;

	if (client->voteStatus)
		return;		//already voted

	if (sl.voteInfo.votable->votetype == VOTABLE_TEAM)
		if (&sl.teams[client->info.team] != sl.voteInfo.team)
		{
			SV_ClientEcho(client->index, fmt("Only players on team %i are allowed to vote on this\n", sl.voteInfo.team->index));
			return;
		}

	client->voteStatus = vote;

	switch(vote)
	{
		case VOTE_YES:
			sl.votesYes++;
			cores.Server_SetStateString(ST_VOTES_YES, fmt("%i", sl.votesYes));
			SV_ClientEcho(client->index, "You voted yes\n");
			break;

		case VOTE_NO:
			sl.votesNo++;
			cores.Server_SetStateString(ST_VOTES_NO, fmt("%i", sl.votesNo));
			SV_ClientEcho(client->index, "You voted no\n");
			break;

		default:
			break;
	}

	if (SV_EnoughVotes())
		SV_TallyVote();
}


/*==========================

  SV_StopVote

 ==========================*/

void	SV_StopVote()
{
	int n;

	sl.voteInProgress = 0;
	sl.voteEndTime = 0;

	cores.Server_SetStateString(ST_VOTE_END_TIME, "0");
	cores.Server_SetStateString(ST_VOTES_YES, "0");
	cores.Server_SetStateString(ST_VOTES_NO, "0");

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (!sl.clients[n].active)
			continue;

		sl.clients[n].voteStatus = 0;
	}
}

void	SV_StopVote_Cmd(int argc, char *argv[])
{
	if (!sl.voteInProgress)
		return;

	SV_StopVote();
}


/*==========================

  SV_CallVote

 ==========================*/

void	SV_CallVote(int initiatingClient, int argc, char *argv[])
{
	int n = 0;	

	if (sv_disableVoting.integer)
	{
		SV_ClientEcho(initiatingClient, "Voting is not allowed on this server.\n");
		return;
	}

	if (sl.voteInProgress)
	{
		if (initiatingClient > -1)
			SV_ClientEcho(initiatingClient, "Vote cannot be started since one is already in progress.\n");
		return;
	}

	if (sl.clients[initiatingClient].mute)
	{
		SV_ClientEcho(initiatingClient, "Callvote rejected, you have been muted.\n");
		return;
	}

	if (sl.numClients > 1)
	{
		if (sl.gametime - sl.clients[initiatingClient].lastVoteCalled <= sv_clientVoteTime.integer)
		{
			SV_ClientEcho(initiatingClient, fmt("You must wait %i seconds before you can call another vote.\n", 
												(sv_clientVoteTime.integer - (sl.gametime - sl.clients[initiatingClient].lastVoteCalled))/1000));
			return;
		}
	}

	//is this a valid vote?

	if (!argc)
	{
		//echo list of vote commands
		SV_ClientEcho(initiatingClient, "Vote commands:\n");
		SV_ClientEcho(initiatingClient, "==============\n");
		while (votables[n].command[0])
		{
			if (votables[n].cvar->integer)
				SV_ClientEcho(initiatingClient, fmt("callvote %s   //%s\n", votables[n].command, votables[n].description));
			n++;
		}

		return;
	}

	while (votables[n].command[0])
	{
		if (stricmp(argv[0], votables[n].command)==0)
		{
			if (!votables[n].cvar->integer)
			{
				SV_ClientEcho(initiatingClient, fmt("\"%s\" vote is not allowed on this server.\n", votables[n].command));
				return;
			}

			memset(&sl.voteInfo, 0, sizeof(sl.voteInfo));

			if (argc > 1)
				sl.voteInProgress = votables[n].voteStartFunc(initiatingClient, argc-1, &argv[1]);
			else
				sl.voteInProgress = votables[n].voteStartFunc(initiatingClient, 0, NULL);

			if (sl.voteInProgress)
			{
				char desc[1024];
				cores.Server_GetStateString(ST_VOTE_DESCRIPTION, desc, sizeof(desc));
				cores.Console_Printf("Client %i called a vote:\n%s\n", initiatingClient, desc);

				sl.votesYes = sl.votesNo = 0;
				sl.voteInfo.votable = &votables[n];
				sl.voteInfo.team = &sl.teams[sl.clients[initiatingClient].info.team];

				sl.voteEndTime = sl.gametime + 30000;
				cores.Server_SetStateString(ST_VOTE_END_TIME, fmt("%i", sl.voteEndTime));
				cores.Server_SetStateString(ST_VOTES_YES, "0");
				cores.Server_SetStateString(ST_VOTES_NO, "0");
				cores.Server_SetStateString(ST_VOTE_TEAM, fmt("%i", (sl.voteInfo.votable->votetype == VOTABLE_ALL) ? 0 : sl.voteInfo.team->index));
				cores.Server_SetStateString(ST_VOTES_MIN, fmt("%f", sv_minVotePercent.value));
				cores.Server_SetStateString(ST_VOTES_NEED, fmt("%f", sl.voteInfo.votable->minPercent));

				sl.clients[initiatingClient].lastVoteCalled = sl.gametime;
			}

			return;
		}

		n++;
	}
	
}


/*==========================

  SV_TallyVote  

 ==========================*/

void	SV_TallyVote()
{
	if (SV_EnoughVotes())
	{
		float yesPercent = (float)sl.votesYes / (sl.votesYes + sl.votesNo);

		if (yesPercent >= sl.voteInfo.votable->minPercent)
		{
			//vote passed
			sl.voteInfo.votable->votePassedFunc();
			SV_BroadcastNotice(NOTICE_GENERAL, 0, "Vote passed");
		}
		else
		{
			//vote failed
			SV_BroadcastNotice(NOTICE_GENERAL, 0, "Vote failed");
		}
	}
	else
	{
		//not enough people voted
		SV_BroadcastNotice(NOTICE_GENERAL, 0, "Vote failed: not enough votes");
	}

	SV_StopVote();
}


/*==========================

  SV_VoteFrame

 ==========================*/

void	SV_VoteFrame()
{
	if (!sl.voteInProgress)
		return;

	if (sl.voteInfo.affectedClient && !sl.voteInfo.affectedClient->active)
	{
		//client quit, so vote is no longer pertinent
		SV_StopVote();
		return;
	}

	if (sl.gametime >= sl.voteEndTime)
		SV_TallyVote();
}



/*==========================

  SV_InitVotes

 ==========================*/

void	SV_InitVotes()
{
	cores.Cvar_Register(&sv_clientVoteTime);
	cores.Cvar_Register(&sv_impeachPenaltyTime);
	cores.Cvar_Register(&sv_minVotePercent);
	cores.Cvar_Register(&sv_allowMapVotes);
	cores.Cvar_Register(&sv_allowMsgVotes);
	cores.Cvar_Register(&sv_allowKickVotes);
	cores.Cvar_Register(&sv_allowMuteVotes);
	cores.Cvar_Register(&sv_allowRestartVotes);
	cores.Cvar_Register(&sv_allowTimeVotes);
	cores.Cvar_Register(&sv_allowImpeachVotes);
	cores.Cvar_Register(&sv_allowConcedeVotes);
	cores.Cvar_Register(&sv_allowRefVotes);
	cores.Cvar_Register(&sv_allowRaceVotes);
	cores.Cvar_Register(&sv_disableVoting);

	cores.Cmd_Register("stopvote", SV_StopVote_Cmd);
}
