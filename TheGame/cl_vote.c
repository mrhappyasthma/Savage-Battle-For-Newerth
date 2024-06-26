// (C) 2003 S2 Games
//
// cl_vote.c

#include "client_game.h"

cvar_t	vote_show =			{ "vote_show",			"0",	CVAR_READONLY };
cvar_t	vote_description =	{ "vote_description",	"",		CVAR_READONLY };
cvar_t	vote_team =			{ "vote_team",			"0",	CVAR_READONLY };
cvar_t	vote_min =			{ "vote_min",			"0",	CVAR_READONLY };
cvar_t	vote_need =			{ "vote_need",			"0",	CVAR_READONLY };
cvar_t	vote_yes =			{ "vote_yes",			"0",	CVAR_READONLY };
cvar_t	vote_no =			{ "vote_no",			"0",	CVAR_READONLY };
cvar_t	vote_seconds =		{ "vote_seconds",		"0",	CVAR_READONLY };
cvar_t	vote_passpercent =	{ "vote_passpercent",	"0",	CVAR_READONLY };



/*==========================

  CL_VoteUpdateCvars

 ==========================*/

void	CL_VoteUpdateCvars(int id)
{
	char	buffer[1024];
	int		total = 0, voted, index;
	float	yes;

	//this is the only cvar that needs updating every frame, and only if a vote is active
	if (cl.voteEndTime > cl.gametime)
		corec.Cvar_SetVarValue(&vote_seconds, ((cl.voteEndTime - cl.gametime) / 1000 + 1));

	//everything beyond here relates to a specific state string update
	if (!id)
		return;

	switch (id)
	{
	case ST_VOTE_DESCRIPTION:
		corec.Client_GetStateString(ST_VOTE_DESCRIPTION, buffer, 1024);
		corec.Cvar_SetVar(&vote_description, buffer);
		break;

	case ST_VOTE_END_TIME:
		corec.Client_GetStateString(ST_VOTE_END_TIME, buffer, 1024);
		cl.voteEndTime = atoi(buffer);
		
		if (cl.voteEndTime > cl.gametime)
		{
			CL_NotifyMessage("A vote has been called", Snd("vote_called"));
			corec.Cvar_SetVarValue(&vote_show, 1);
		}
		else
		{
			corec.Cvar_SetVarValue(&vote_show, 0);
		}
		CL_InterfaceEvent(IEVENT_VOTE);
		break;

	case ST_VOTES_YES:
		corec.Client_GetStateString(ST_VOTES_YES, buffer, 1024);
		corec.Cvar_SetVarValue(&vote_yes, atoi(buffer));
		
		if (vote_yes.integer > 0)
			CL_Play2d(Snd("vote_yes"), 1.0, CHANNEL_GUI);
		CL_InterfaceEvent(IEVENT_VOTE);
		break;

	case ST_VOTES_NO:
		corec.Client_GetStateString(ST_VOTES_NO, buffer, 1024);
		corec.Cvar_SetVarValue(&vote_no, atoi(buffer));
		
		if (vote_no.integer > 0)
			CL_Play2d(Snd("vote_no"), 1.0, CHANNEL_GUI);
		CL_InterfaceEvent(IEVENT_VOTE);
		break;

	case ST_VOTES_MIN:
		corec.Client_GetStateString(ST_VOTES_MIN, buffer, 1024);
		corec.Cvar_SetVarValue(&vote_min, atof(buffer));
		break;

	case ST_VOTES_NEED:
		corec.Client_GetStateString(ST_VOTES_NEED, buffer, 1024);
		corec.Cvar_SetVarValue(&vote_need, atof(buffer));
		break;

	case ST_VOTE_TEAM:
		corec.Client_GetStateString(ST_VOTE_TEAM, buffer, 1024);
		corec.Cvar_SetVarValue(&vote_team, atoi(buffer));
		break;
	}

	//if someone voted, update the pass/fail progess bar
	if (vote_show.integer &&
		(id == ST_VOTES_YES || id == ST_VOTES_NO || id == ST_VOTE_END_TIME))
	{
		for (index = 0; index < MAX_CLIENTS; index++)
		{
			if (!cl.clients[index].info.active)
				continue;

			if ((cl.clients[index].info.team == vote_team.integer) ||
				(!vote_team.integer && (cl.clients[index].info.team > 0)))
				total++;
		}

		//determine the percentage of yes votes, then divide this by the minimum pass percentage
		//this will give us a full bar exactly when there are enough votes to pass
		voted = vote_yes.integer + vote_no.integer;
		if (voted)
		{
			yes = vote_yes.integer / (float)voted;
			yes /= vote_min.value;
			yes = MIN(yes, 1.0);	//cap at 100%
		}
		else
		{
			yes = 0;
		}

		//now scale the bar to show how many more people need to vote,
		//if that has not yet been reached
		if ((voted / (float)total) < vote_min.value)
			yes *= (voted / (float)(total * vote_min.value));

		corec.Cvar_SetVarValue(&vote_passpercent, yes);
	}

}


/*==========================

  CL_Vote_Cmd

 ==========================*/

void	CL_Vote_Cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_VOTE_MSG, argv[0]));
}


/*==========================

  CL_CallVote_Cmd

 ==========================*/

void	CL_CallVote_Cmd(int argc, char *argv[])
{
	char args[1024];

	ConcatArgs(argv, argc, args);

	corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_CALLVOTE_MSG, args));
}



/*==========================

  CL_VoteInit

 ==========================*/

void	CL_VoteInit()
{
	corec.Cmd_Register("vote",				CL_Vote_Cmd);
	corec.Cmd_Register("callvote",			CL_CallVote_Cmd);

	corec.Cvar_Register(&vote_show);
	corec.Cvar_Register(&vote_description);
	corec.Cvar_Register(&vote_team);
	corec.Cvar_Register(&vote_min);
	corec.Cvar_Register(&vote_need);
	corec.Cvar_Register(&vote_yes);
	corec.Cvar_Register(&vote_no);
	corec.Cvar_Register(&vote_seconds);
	corec.Cvar_Register(&vote_passpercent);
}
