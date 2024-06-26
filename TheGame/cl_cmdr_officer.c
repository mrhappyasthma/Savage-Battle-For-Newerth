/*
 * (C) 2003 S2 Games
 *
 * cl_cmdr_officer.c'
 */

#include "client_game.h"

// Commands to manage commanders "field officers"
// These will be FPS players that are given special powers as well as an aura
// effect.  Also includes special interface for interacting with them.
//=============================================================================

cvar_t	cl_cmdr_officers[] =
{
	{"cl_cmdr_officer1", ""},
	{"cl_cmdr_officer2", ""},
	{"cl_cmdr_officer3", ""},
	{""}
};


/*==========================

  CMDR_GetMaxOfficers

  Returns the maximum officers a team can currently support

 ==========================*/

int	CMDR_GetMaxOfficers()
{
	int max, index, numonteam = 0;

	for (index = 0; index < MAX_CLIENTS; index++)
	{
		if (cl.clients[index].info.active &&
			cl.clients[index].info.team == cl.info->team)
			numonteam++;
	}

	max = numonteam / MIN_CLIENTS_PER_OFFICER;

	return MIN(MAX_OFFICERS, MAX(max,1));
}


/*==========================

  CMDR_UnitIsOfficer

 ==========================*/

bool	CMDR_UnitIsOfficer(int unitnum)
{
	int i;

	for (i = 0; i < MAX_OFFICERS; i++)
	{
		if (cl.officers[i] == unitnum)
			return true;
	}
	return false;
}

void	CMDR_PromoteToOfficer(int slot, int client)
{
	int	maxOfficers = CMDR_GetMaxOfficers();
	int index;

	//validate client
	if (client < 0 || client >= MAX_CLIENTS)
		return;

	//already promoted?
	for (index = 0; index < MAX_OFFICERS; index++)
	{
		if (cl.officers[index] == client)
			return;
	}

	//no slot specified, find first available
	if (slot < 0)
	{
		for (index = 0; index < maxOfficers; index++)
		{
			if (cl.officers[index] < 0)
				break;
		}
		slot = index;
	}

	//validate slot
	if (slot >= maxOfficers || slot < 0)
		return;

	//demote existing officer, if any
	if (cl.officers[slot] >= 0)
		corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_DEMOTE_MSG, slot));

	//send the promotion
	corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_COMMANDER_PROMOTE_MSG, slot, client));

}


/*==========================

  CMDR_PromoteToOfficer_cmd

  syntax: promoteToOfficer [officer slot] [client name | client number]
  If client is not specified, the currently selected player will be promoted
  If slot is not specified, the first available slot is filled  

 ==========================*/

void	CMDR_PromoteToOfficer_cmd(int argc, char *argv[])
{
	int	client = -1;
	int slot = -1;

	//no client specified, try selected
	if (argc < 2)
	{
		//must have one and only one selected
		if (cl.selection.numSelected != 1)
			return;

		client = cl.selection.array[0];
	}
	else
	{
		client = atoi(argv[1]);
	}

	//slot specified?
	if (argc > 0)
		slot = atoi(argv[0]);

	CMDR_PromoteToOfficer(slot, client);
}

void	CMDR_PromoteOfficerTeamUnit_cmd(int argc, char *argv[])
{
	if (argc)
	{
		CMDR_PromoteToOfficer(-1, CL_TeamUnitToGlobalUnitNum(atoi(argv[0])));
	}
}

void	CMDR_DemoteOfficer(int officernum)
{
	if (officernum < 0 || officernum >= MAX_OFFICERS)
		return;

	if (cl.officers[officernum] >= 0)
		corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_COMMANDER_DEMOTE_MSG, officernum));
	cl.officers[officernum] = -1;
}


/*==========================

  CMDR_DemoteOfficer_cmd

  syntax: demoteOfficer [officer slot]
  If slot is specified, just clear that spot as an officer
  If no slot is specified, demote selected player if they are an officer

 ==========================*/

void	CMDR_DemoteOfficer_cmd(int argc, char *argv[])
{
	int officernum;

	//demote specified officer
	if (argc)
	{
		officernum = atoi(argv[0]);
		CMDR_DemoteOfficer(officernum);
	}
	//demote slected player
	else
	{
		int index;

		//must have only one client selected
		if (cl.selection.numSelected != 1)
			return;

		//check selected unit against officer list
		for (index = 0; index < MAX_OFFICERS; index++)
		{
			if (cl.officers[index] == cl.selection.array[0])
			{
				CMDR_DemoteOfficer(index);
				break;
			}
		}
	}
}

void	CMDR_DemoteOfficerUnit(int unitnum)
{
	int i;

	for (i = 0; i < MAX_OFFICERS; i++)
	{
		if (cl.officers[i] == unitnum)
		{
			CMDR_DemoteOfficer(i);
			return;
		}
	}
}

void	CMDR_DemoteOfficerTeamUnit_cmd(int argc, char *argv[])
{
	if (argc)
	{
		CMDR_DemoteOfficerUnit(CL_TeamUnitToGlobalUnitNum(atoi(argv[0])));
	}
}

void	CMDR_DemoteOfficerUnit_cmd(int argc, char *argv[])
{
	if (argc)
	{
		CMDR_DemoteOfficerUnit(atoi(argv[0]));
	}
}


/*==========================

  CMDR_Officer_cmd

  syntax: officer <number>
  selects specified officer, if one is set
  execute twice quickly to move to that officers possition
  hold CTRL to promote selected player (if one and only one selected)
  hold ALT to demote selected player (if one and only one selected is officer)

 ==========================*/

void	CMDR_Officer_cmd(int argc, char *argv[])
{
	int officernum;

	if (argc < 1)
		return;

	officernum = atoi(argv[0]);

	if (officernum < 0 || officernum >= CMDR_GetMaxOfficers())
	{
		corec.Console_DPrintf("Invalid officer number: %i\n", officernum);
		return;
	}

	//ctrl promotes
	if (corec.Input_IsKeyDown(KEY_CTRL))
	{
		CMDR_PromoteToOfficer_cmd(argc, argv);
		return;
	}

	//shift does... something?
	if (corec.Input_IsKeyDown(KEY_SHIFT))
	{
		return;
	}

	//alt demotes
	if (corec.Input_IsKeyDown(KEY_ALT))
	{
		CMDR_DemoteOfficer_cmd(argc, argv);
		return;
	}

	//twice in a row centers on the officer
	if (cl.gametime - cl.lastOfficerCmdTime < 500 && officernum == cl.lastOfficerSelected)
	{
		cl.cmdrLookAtPoint[X] = cl.objects[cl.officers[officernum]].base.pos[0];
		cl.cmdrLookAtPoint[Y] = cl.objects[cl.officers[officernum]].base.pos[1];
		return;
	}

	//just select the officer
	cl.lastOfficerSelected = officernum;
	cl.lastOfficerCmdTime = cl.gametime;

	CL_ClearUnitSelection(&cl.selection);
	CL_AddToUnitSelection(&cl.selection, cl.officers[officernum]);
	CL_SendSelectionToServer();
	CMDR_RefreshSelectionIcons();
}


/*==========================

  CMDR_ChatOfficer_cmd

  Syntax: chatOfficer <number> <message>
  If officer number is -1, message broadcasts to all officers

 ==========================*/

void	CMDR_ChatOfficer_cmd(int argc, char *argv[])
{
	char msg[1024];
	//char name[CLIENT_NAME_LENGTH+1]
	int n, size = 0;
	char *s[3];
	int	officernum;

	//only commanders can use this
	if (!cl.isCommander)
		return;

	//check args
	if (argc < 2)
	{
		corec.Console_Printf("syntax: chatOfficer <number> <message>\n");
		return;
	}

	//get target
	officernum = atoi(argv[0]);
	if (officernum >= CMDR_GetMaxOfficers())
		return;

	//build message
	strcpy(msg, "");
	for (n = 1; n < argc; n++)
	{
		if (size + strlen(argv[n]) >= 1024)
			break;
		
		strcat(msg, argv[n]);
		strcat(msg, " ");
		size += strlen(argv[n]) + 1;
	}
	
	//locally display the private message that was sent
	s[0] = "msg_public";
	if (officernum >= 0)
	{
		s[1] = "To OFFICER %s->";
		strcpy(s[1], fmt(s[1], cl.clients[cl.officers[officernum]].info.name));
	}
	else
	{
		s[1] = "To ALL OFFICERS->";
	}
	s[2] = msg;
	corec.GUI_Notify(3, s);

	//send the message
	if (officernum < 0)
	{
		for (n = 0; n < CMDR_GetMaxOfficers(); n++)
		{
			if (cl.officers[n] > 0 && cl.officers[n] < MAX_CLIENTS)
				corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_CHAT_PRIVATE_MSG, cl.clients[cl.officers[n]].info.name, msg));
		}
	}
	else
	{
		if (cl.officers[n] > 0 && cl.officers[n] < MAX_CLIENTS)
			corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_CHAT_PRIVATE_MSG, cl.clients[cl.officers[officernum]].info.name, msg));
	}
}


int		CMDR_FindNextOfficer()
{
	static int lastOfficer = 0;
	int i = lastOfficer + 1;

	do
	{
		if (cl.officers[i] != 0)
		{
			lastOfficer = i;
			return i;
		}
		i++;

		if (i >= MAX_OFFICERS)
			i = 0;
	}
	while (i != lastOfficer);

	return -1;
}


void	CMDR_SelectNextOfficer_cmd(int argc, char *argv[])
{
	int officer;
	
	if (!cl.isCommander)
		return;
	
	officer = CMDR_FindNextOfficer();

	if (officer != -1)
	{
		CL_ClearUnitSelection(&cl.potentialSelection);
		CL_AddToUnitSelection(&cl.potentialSelection, officer);
		CL_ProcessPotentialSelection(&cl.potentialSelection);
		cl.cmdrLookAtPoint[X] = cl.objects[officer].visual.pos[0];
		cl.cmdrLookAtPoint[Y] = cl.objects[officer].visual.pos[1];
	}
}


/*==========================

  CMDR_InitOfficers

  Register commands/cvars relevant to officers

 ==========================*/

void	CMDR_InitOfficers()
{
	corec.Cmd_Register("promoteToOfficer",			CMDR_PromoteToOfficer_cmd);
	corec.Cmd_Register("demoteOfficer",				CMDR_DemoteOfficer_cmd);
	corec.Cmd_Register("officer",					CMDR_Officer_cmd);
	corec.Cmd_Register("chatOfficer",				CMDR_ChatOfficer_cmd);
	corec.Cmd_Register("promoteOfficerTeamUnit",	CMDR_PromoteOfficerTeamUnit_cmd);
	corec.Cmd_Register("demoteOfficerTeamUnit",		CMDR_DemoteOfficerTeamUnit_cmd);
	corec.Cmd_Register("demoteOfficerUnit",			CMDR_DemoteOfficerUnit_cmd);
	corec.Cmd_Register("findofficer",				CMDR_SelectNextOfficer_cmd);

	RegisterVarArray(cl_cmdr_officers);
}
