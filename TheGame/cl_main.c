// (C) 2001 S2 Games
//
// cl_main.c

#include "client_game.h"
#include "cl_translator.h"


coreAPI_client_t corec;
clientLocal_t cl;

cvar_t	cl_alwaysThirdPerson =	{ "cl_alwaysThirdPerson",	"0", CVAR_SAVECONFIG };
cvar_t	cl_mousepos_x =			{ "cl_mousepos_x",			"0" };
cvar_t	cl_mousepos_y =			{ "cl_mousepos_y",			"0" };
cvar_t	cl_lerpObjects =		{ "cl_lerpObjects",			"1", CVAR_SAVECONFIG };
cvar_t	cl_cameraDistance =		{ "cl_cameraDistance",		"40", CVAR_CHEAT };
cvar_t	cl_cameraPosLerp =		{ "cl_cameraPosLerp",		"6", CVAR_CHEAT  };
cvar_t	cl_cameraAngleLerp =	{ "cl_cameraAngleLerp",		"9999", CVAR_SAVECONFIG };
cvar_t	cl_cameraDistLerp =		{ "cl_cameraDistLerp",		"8", CVAR_SAVECONFIG };
cvar_t	cl_zoomLerp =			{ "cl_zoomLerp",			"8" };
cvar_t	cl_meleeConstrain =		{ "cl_meleeConstrain",		"1", CVAR_SAVECONFIG };
cvar_t	cl_thirdPersonYSens =	{ "cl_thirdPersonYSens",	"0.5", CVAR_SAVECONFIG };
cvar_t	cl_firstPersonYSens =	{ "cl_firstPersonYSens",	"0.9", CVAR_SAVECONFIG };
cvar_t	cl_offsetz =			{ "cl_offsetz",				"7", CVAR_CHEAT };
cvar_t	cl_showIevents =		{ "cl_showIevents",			"0" };
cvar_t	cl_chaseDistance =		{ "cl_chaseDistance",		"50", CVAR_VALUERANGE, 50, 200 };
cvar_t	cl_thirdPerson =		{ "cl_thirdPerson",			"1", CVAR_READONLY };
cvar_t	cl_requestScores =		{ "cl_requestScores",		"0",  };
cvar_t	cl_usePlayerSoundPosition = {"cl_usePlayerSoundPosition", "0" };

cvar_t	cl_showClanAbbrevInUserlist = 	{ "cl_showClanAbbrevInUserlist", "1" };
cvar_t	cl_showClanIconInUserlist = 	{ "cl_showClanIconInUserlist", "1" };

cvar_t	cl_connectionProblemIcon_x = { "cl_connectionProblemIcon_x", "0" };
cvar_t	cl_connectionProblemIcon_y = { "cl_connectionProblemIcon_y", "460" };
cvar_t	cl_connectionProblemIcon_width = { "cl_connectionProblemIcon_width", "64" };
cvar_t	cl_connectionProblemIcon_height = { "cl_connectionProblemIcon_height", "64" };

//GUI status override variables
cvar_t	cl_inGameMenu =			{ "cl_inGameMenu",			"0" };
cvar_t	cl_showLobby =			{ "cl_showLobby",			"0" };

cvar_t	cl_skipRender =			{ "cl_skipRender",			"0", CVAR_CHEAT };
cvar_t	cl_skipObjects =		{ "cl_skipObjects",			"0", CVAR_CHEAT };
cvar_t	cl_skipStuff =			{ "cl_skipStuff",			"0", CVAR_CHEAT };
cvar_t	cl_effects =			{ "cl_effects",				"1" };

cvar_t	cl_freeMouse =			{ "cl_freeMouse",			"0" };

//temp hack so jesse can make screenshots
cvar_t 	cl_oldCommanderLook = { "cl_oldCommanderLook", "0" };

cvar_t	cl_doubleClickTime = { "cl_doubleClickTime", "200", CVAR_SAVECONFIG };

cvar_t	cl_voiceIconTime =	{ "cl_voiceIconTime", "5000", CVAR_SAVECONFIG };

//cvars exposed for the GUI
cvar_t	cl_team1_score =			{ "cl_team1_score",				"0", CVAR_READONLY };
cvar_t	cl_team2_score =			{ "cl_team2_score",				"0", CVAR_READONLY };
cvar_t	cl_team1commandername =		{ "cl_team1commandername",		"", CVAR_READONLY };
cvar_t	cl_team2commandername =		{ "cl_team2commandername",		"", CVAR_READONLY };

cvar_t	cl_playVoiceChats =			{ "cl_playVoiceChats",			"1", CVAR_SAVECONFIG };

cvar_t	cl_svAddress =				{ "cl_svAddress",				"", CVAR_READONLY };
cvar_t	cl_svName =					{ "cl_svName",					"",	CVAR_READONLY };

cvar_t	player_health =				{ "player_health",				"0", CVAR_READONLY };
cvar_t	player_healthpercent =		{ "player_healthpercent",		"0", CVAR_READONLY };
cvar_t	player_ammo =				{ "player_ammo",				"0", CVAR_READONLY };
cvar_t	player_maxammo =			{ "player_maxammo",				"0", CVAR_READONLY };
cvar_t	player_money =				{ "player_money",				"0", CVAR_READONLY };
cvar_t	player_stamina =			{ "player_stamina",				"0", CVAR_READONLY };
cvar_t	player_staminapercent =		{ "player_staminapercent",		"0", CVAR_READONLY };
cvar_t	player_overheatpercent =	{ "player_overheatpercent",		"0", CVAR_READONLY };
cvar_t	player_level =				{ "player_level",				"0", CVAR_READONLY };
cvar_t	player_percentNextLevel =	{ "player_percentNextLevel",	"0", CVAR_READONLY };
cvar_t	player_loyalty =			{ "player_loyalty",				"0", CVAR_READONLY };
cvar_t	player_x =					{ "player_x",					"0", CVAR_READONLY };
cvar_t	player_y =					{ "player_y",					"0", CVAR_READONLY };
cvar_t	player_z =					{ "player_z",					"0", CVAR_READONLY };
cvar_t	player_secondsToRespawn =	{ "player_secondsToRespawn",	"", CVAR_READONLY };
cvar_t	player_task =				{ "player_task",				"", CVAR_READONLY };
cvar_t	player_item =				{ "player_item",				"0", CVAR_READONLY };
cvar_t	player_wishitem =			{ "player_wishitem",			"0", CVAR_READONLY };
cvar_t	player_race =				{ "player_race",				"0", CVAR_READONLY };
cvar_t	player_racename =			{ "player_racename",			"", CVAR_READONLY };
cvar_t	player_team =				{ "player_team",				"0", CVAR_READONLY };
cvar_t	player_currentunit =		{ "player_currentunit",			"", CVAR_READONLY };
cvar_t	player_mana =				{ "player_mana",				"0", CVAR_READONLY };
cvar_t	player_referee =			{ "player_referee",				"0", CVAR_READONLY };
cvar_t	player_focus =				{ "player_focus",				"0", CVAR_READONLY };

cvar_t	game_timeLimitMinutes =		{ "game_timeLimitMinutes",		"0", CVAR_READONLY };
cvar_t	game_timeLimitSeconds =		{ "game_timeLimitSeconds",		"0", CVAR_READONLY };
cvar_t	game_restartSeconds =		{ "game_restartSeconds",		"0", CVAR_READONLY };
cvar_t	game_serverStatus =			{ "game_serverStatus",			"", CVAR_READONLY };
cvar_t	game_status =				{ "game_status",				"", CVAR_READONLY };

cvar_t  team1_race =				{ "team1_race", "",		CVAR_READONLY };
cvar_t  team1_racename =			{ "team1_racename", "", CVAR_READONLY };
cvar_t  team2_race =				{ "team2_race", "",		CVAR_READONLY };
cvar_t  team2_racename =			{ "team2_racename", "", CVAR_READONLY };

cvar_t	world_width =				{ "world_width",				"0", CVAR_READONLY };
cvar_t	world_height =				{ "world_height",				"0", CVAR_READONLY };
cvar_t	voice_item[10] =
{
	{ "voice_item1", "", CVAR_READONLY },
	{ "voice_item2", "", CVAR_READONLY },
	{ "voice_item3", "", CVAR_READONLY },
	{ "voice_item4", "", CVAR_READONLY },
	{ "voice_item5", "", CVAR_READONLY },
	{ "voice_item6", "", CVAR_READONLY },
	{ "voice_item7", "", CVAR_READONLY },
	{ "voice_item8", "", CVAR_READONLY },
	{ "voice_item9", "", CVAR_READONLY },
	{ "voice_item10", "", CVAR_READONLY }	
};

cvar_t	team_commandcenter_health =			{ "team_commandcenter_health",			"0",	CVAR_READONLY };
cvar_t	team_commandcenter_maxhealth =		{ "team_commandcenter_maxhealth",		"0",	CVAR_READONLY };
cvar_t	team_commandcenter_healthpercent =	{ "team_commandcenter_healthpercent",	"0",	CVAR_READONLY };

cvar_t	team_numworkers =		{ "team_numWorkers",	"",	CVAR_READONLY };
cvar_t	team_maxworkers =		{ "team_maxWorkers",	"",	CVAR_READONLY };
cvar_t	team_idleworkers =		{ "team_idleWorkers",	"",	CVAR_READONLY };
cvar_t	team_commander =		{ "team_commander",		"",	CVAR_READONLY };
cvar_t	team_commandername =	{ "team_commandername",	"",	CVAR_READONLY };

cvar_t	cl_downloadIcons = 			{ "cl_downloadIcons", 			"1", CVAR_SAVECONFIG };
cvar_t	cl_statsPopupDelay =		{ "cl_statsPopupDelay",			"4000" };

cvar_t	_toggleManageScreen =		{ "_toggleManageScreen", 		"0" };

cvar_t	cl_currentDirectory =	{ "cl_currentDirectory",	"", CVAR_READONLY };

cvar_t	cl_placeObjectTestRate =	{ "placeObjectTestRate",		"15", CVAR_CHEAT };

cvar_t	cl_cmdrNoticePersistTime =	{ "cl_cmdrNoticePersistTime",	"15000" };


extern cvar_t 	cl_targetedObject;
extern cvar_t 	cl_targetedTerrainX;
extern cvar_t 	cl_targetedTerrainY;
extern cvar_t 	cl_targetedTerrainZ;

extern cvar_t	gamelist_forcemap;
extern cvar_t	gamelist_forceraces;
extern cvar_t	gamelist_forcetime;
extern cvar_t	gamelist_forcegametype;

gui_element_t *lobby_chatbox;

char *interfaceEventNames[] =
{
	"trigger",
	"inventory",	//contents of the players inventory changed
	"unit",			//players unit type changed
	"money",		//players gained/lost money
	"death",		//player died
	"spawn",		//player spawned from a command center/outpost
	"selection",	//commander selection changed
	"resources",	//number of resources player/commander has changed
	"health",
	"ammo",
	"research",
	"states",
	"request",
	"deployment",
	"officers",
	"commander",
	"vote",
	""
};

char lastPrivMsg[MAX_NAME_LEN] = { 0 };

extern cvar_t cl_fogOfWarColor_r;
extern cvar_t cl_fogOfWarColor_g;
extern cvar_t cl_fogOfWarColor_b;
extern cvar_t cl_fogOfWarColor_a;


extern char *__builddate;

vec3_t zerovec3 = { 0, 0, 0};


/*==========================

  CL_Dead

 ==========================*/

bool	CL_Dead()
{
	if (cl.predictedState.health <= 0)
		return true;
	return false;
}

bool	CL_ClientNameIsCommander(char *name)
{
	int commander;

	commander = cl.teams[cl.info->team].commander;

	if (cl.clients[commander].info.active 
		&& strcmp(cl.clients[commander].info.name, name) == 0)
		return true;
	return false;
}
	
bool	CL_ClientNameIsOfficer(char *name)
{
	int i, j;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!cl.clients[i].info.active
			|| cl.info->team != cl.clients[i].info.team)
			continue;

		if (strcmp(cl.clients[i].info.name, name) == 0)
		{
			for (j = 0; j < MAX_OFFICERS; j++)
				if (cl.officers[j] == i)
					return true;
		}
	}
	return false;
}

bool	CL_ClientNameIsSpectator(char *name)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!cl.clients[i].info.active
			|| cl.clients[i].info.team != 0)
			continue;

		if (strcmp(cl.clients[i].info.name, name) == 0)
			return true;
	}
	return false;
}

/*
 * caches the clan icon
 */
void	CL_GetClanIcon(int clan)
{
	corec.File_CacheClanIcon(clan);
}

void	CL_GetClientInfo(int clientnum)
{
	corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_GETCOOKIE_MSG, clientnum));
}

void	CL_GetClientInfo_cmd(int argc, char *argv[])
{
	if (argc > 0)
		CL_GetClientInfo(atoi(argv[0]));
}

void	CL_GetSelectionInfo_cmd(int argc, char *argv[])
{
	if (cl.selection.numSelected > 0)
	{
		CL_GetClientInfo(cl.selection.array[0]);
	}
}

int     CL_TeamUnitToGlobalUnitNum(int unitnum)
{
	int i, j;
	
	if (unitnum >= 0 && unitnum < MAX_CLIENTS)
	{
		i = 0;
		j = 0;
		while (i < MAX_CLIENTS)
		{
			if (cl.clients[i].info.active 
				&& cl.clients[i].info.team == cl.info->team)
			{
				if (unitnum == j)
				{
					return i;
				}
				j++;
			}
			i++;
		}
	}
	return -1;
}

char	*CL_RaceSnd(const char *id)
{
	return Snd(fmt("%s_%s", raceData[cl.race].name, id));
}

char	*CL_RaceMsg(const char *id)
{
	return core.Str_Get(msgTable, fmt("%s_%s", raceData[cl.race].name, id));
}

void	CL_Play2d(const char *filename, float volume, int chan)
{
	residx_t sound;

	if (!filename[0])
		return;

	sound = corec.Res_LoadSound(filename);
	if (sound)
		corec.Sound_Play(sound, -1, NULL, volume, chan, 0);
}

void	CL_Play2d_Cmd(int argc, char *argv[])
{
	float volume = 1.0;
	if (!argc)
		return;

	if (argc > 1)
		volume = atof(argv[1]);

	CL_Play2d(argv[0], volume, CHANNEL_AUTO);
}

bool	_allWhiteSpace(char *string)
{
	int i = 0, len;
	len = strlen(string);
	while (i < len)
	{
		if (string[i] != ' '
			&& string[i] != '\v'
			&& string[i] != '\t'
			&& string[i] != '\n')
			return false;

		i++;
	}
	return true;
}




/*==========================

  CL_VoiceChat

  play a voice chat message

 ==========================*/

void	CL_VoiceChat(int clientnum, const char *menuname, int category, int itemnum)
{
	char *s[3];
	int r;
	voiceMenu_t *menu = GetVoiceMenu(menuname);
	voiceCategory_t *cat;
	voiceItem_t *item;

	if (!menu)
		return;

	if (category >= MAX_VOICECHAT_CATEGORIES || category < 0)
		return;

	cat = &menu->categories[category];

	if (!cat->active)
		return;

	if (itemnum < 0 || itemnum >= cat->numItems)
		return;

	item = &cat->items[itemnum];
	if (item->more)
		return;
	if (!item->numSounds)
		return;
	
	r = rand() % item->numSounds;

	if (item->vs[r].sound[0] && cl_playVoiceChats.integer)
	{		
		corec.Sound_StopHandle(cl.clients[clientnum].voiceHandle);

		cl.clients[clientnum].voiceHandle = corec.Sound_Play(corec.Res_LoadSound(fmt("%s/%s", menu->directory, item->vs[r].sound)), -1, NULL, 1.0, CHANNEL_AUTO, PRIORITY_HIGH);
	}

	if (item->flags & VOICE_GLOBAL)
		s[0] = "msg_public";
	else
		s[0] = "msg_team";

	s[1] = cl.clients[clientnum].info.name;
	s[2] = item->vs[r].text;

	corec.GUI_Notify(3, s);

	cl.clients[clientnum].voiceIconTime = cl.gametime + cl_voiceIconTime.integer;
}

void	CL_VoiceChatHide()
{
	cl.voiceActive = false;
	corec.Cmd_Exec("hide voice_chat_panel");
}


/*==========================

  CL_UpdateVoiceCategory

 ==========================*/

void	CL_UpdateVoiceCategory()
{
	int n;

	for (n=0; n<cl.voiceCategory->numItems; n++)
	{
		corec.Cvar_SetVar(&voice_item[n], fmt("%i. %s", cl.voiceCategory->items[n].number, cl.voiceCategory->items[n].desc));
	}

	for ( ; n<10; n++)
	{
		corec.Cvar_SetVar(&voice_item[n], "");
	}
}


/*==========================

  CL_VoiceChatNumberPress

  executed when a number key is pressed while the voice chat menu is shown

 ==========================*/

bool	CL_VoiceChatNumberPress(int number)
{
	int n;

	if (!cl.voiceMenu || !cl.voiceCategory)		//shouldn't ever be true when this function is called
		return false;

	for (n=0; n<cl.voiceCategory->numItems; n++)
	{
		voiceItem_t *item = &cl.voiceCategory->items[n];

		if (item->number != number)
			continue;
		
		if (item->more)
		{
			cl.voiceCategory = &cl.voiceMenu->categories[item->more];
			CL_UpdateVoiceCategory();
			return true;
		}
		else
		{
			corec.Cmd_Exec(fmt("vc %i %i", cl.voiceCategory - cl.voiceMenu->categories, n));
			CL_VoiceChatHide();
			return true;
		}		
	}

	return true;
}


/*==========================

  CL_VoiceChatActivate

  executed when V key is pressed

 ==========================*/

void	CL_VoiceChatActivate()
{
	//make sure we're looking at the correct voice chat menu
	if (cl.isCommander)
		cl.voiceMenu = GetVoiceMenu(fmt("%s_commander", raceData[cl.race].name));
	else
		cl.voiceMenu = GetVoiceMenu(CL_ObjectType(cl.predictedState.unittype)->voiceMenu);

	if (!cl.voiceMenu)
	{
		cl.voiceActive = false;
		return;
	}

	cl.voiceCategory = &cl.voiceMenu->categories[0];
	cl.voiceActive = true;
	CL_UpdateVoiceCategory();
}


void	CL_VoiceChatActivate_Cmd(int argc, char *argv[])
{
	CL_VoiceChatActivate();
}

/*==========================

  CL_UpdateVoiceChat

  called once per frame

 ==========================*/

void	CL_UpdateVoiceChat()
{	
	if (!cl.voiceActive)
		return;

	corec.Cmd_Exec("show voice_chat_panel");
}


void	CL_SendChatMessage(char *message)
{
	char msg[1024];

	if (_allWhiteSpace(message))
		return;
	
	BPrintf(msg, 1023, "%s %s", CLIENT_CHAT_MSG, message);

	CL_Play2d(Snd("msg_send"), 1.0, CHANNEL_AUTO);

	corec.Client_SendMessageToServer(msg);
}

void	CL_Chat_Cmd(int argc, char *argv[])
{
	char msg[1024];
	int n, size = 0;

	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: chat [message]\n"));

		return;
	}

	strcpy(msg, "");
	for (n=0; n<argc; n++)
	{
		if (size + strlen(argv[n]) >= 1024)
			break;
		
		strcat(msg, argv[n]);
		strcat(msg, " ");
		size += strlen(argv[n]) + 1;
	}
	
	if (msg[0] == '/')
		corec.GUI_Exec(&msg[1]);
	else
		CL_SendChatMessage(msg);
}

void	CL_ChatSelected_Cmd(int argc, char *argv[])
{
	char msg[1024];
	int n, size = 0;

	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: chatSelected [message]\n"));
		return;
	}

	strcpy(msg, "");
	for (n=0; n<argc; n++)
	{
		if (size + strlen(argv[n]) >= 1024)
			break;
		
		strcat(msg, argv[n]);
		strcat(msg, " ");
		size += strlen(argv[n]) + 1;
	}

	if (_allWhiteSpace(msg))
		return;
	
	CL_Play2d(Snd("msg_send"), 1.0, CHANNEL_AUTO);
	
	if (msg[0] == '/')
		corec.GUI_Exec(&msg[1]);
	else
		CL_MessageSelected(msg);
}

void	CL_ChatTeam_Cmd(int argc, char *argv[])
{
	char msg[1024];
	int n, size = 0;

	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: chatTeam [message]\n"));
		return;
	}

	strcpy(msg, "");
	for (n=0; n<argc; n++)
	{
		if (size + strlen(argv[n]) >= 1024)
			break;
		
		strcat(msg, argv[n]);
		strcat(msg, " ");
		size += strlen(argv[n]) + 1;
	}

	if (_allWhiteSpace(msg))
		return;
	
	CL_Play2d(Snd("msg_send"), 1.0, CHANNEL_AUTO);
	
	if (msg[0] == '/')
		corec.GUI_Exec(&msg[1]);
	else
		corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_CHAT_TEAM_MSG, msg));
}

void	CL_ChatPrivate_Cmd(int argc, char *argv[])
{
	char msg[1024];
	int n, size = 0;
	char *s[3];

	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: chatPrivate [name] [message]\n"));
		return;
	}

	strcpy(msg, "");
	for (n=1; n<argc; n++)
	{
		if (size + strlen(argv[n]) >= 1024)
			break;
		
		strcat(msg, argv[n]);
		strcat(msg, " ");
		size += strlen(argv[n]) + 1;
	}
	
	if (_allWhiteSpace(msg))
		return;
	
	//locally display the private message that was sent

	s[0] = "msg_sent";
	s[1] = argv[0];
	s[2] = msg;
	corec.GUI_Notify(3, s);

	CL_Play2d(Snd("msg_send"), 1.0, CHANNEL_AUTO);

	if (msg[0] == '/')
		corec.GUI_Exec(&msg[1]);
	else
		corec.Client_SendMessageToServer(fmt("%s %s %s", CLIENT_CHAT_PRIVATE_MSG, argv[0], msg));
}

void	CL_SendReply_cmd(int argc, char *argv[])
{
	char msg[1024];
	char name[128];
	int n, size = 0;
	char *s[3];

	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: chatPrivate [name] [message]\n"));
		return;
	}

	if (!lastPrivMsg[0])
	{
		corec.Console_Printf(getstring("nobody to reply to\n"));
		return;
	}

	strcpy(msg, "");
	for (n=0; n<argc; n++)
	{
		if (size + strlen(argv[n]) >= 1024)
			break;
		
		strcat(msg, argv[n]);
		strcat(msg, " ");
		size += strlen(argv[n]) + 1;
	}
	
	if (_allWhiteSpace(msg))
		return;
	
	//locally display the private message that was sent

	s[0] = "msg_public";
	BPrintf(name, 127, "To %s->", lastPrivMsg);
	s[1] = name;
	s[2] = msg;
	corec.GUI_Notify(3, s);

	CL_Play2d(Snd("msg_send"), 1.0, CHANNEL_AUTO);

	if (msg[0] == '/')
		corec.GUI_Exec(&msg[1]);
	else
		corec.Client_SendMessageToServer(fmt("%s %s %s", CLIENT_CHAT_PRIVATE_MSG, lastPrivMsg, msg));
	
}

void	CL_ChatCommander_Cmd(int argc, char *argv[])
{
	char msg[1024];
	//char name[128];
	int n, size = 0;
	char *s[3];

	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: chatCommander [message]\n"));
		return;
	}

	strcpy(msg, "");
	for (n=0; n<argc; n++)
	{
		if (size + strlen(argv[n]) >= 1024)
			break;
		
		strcat(msg, argv[n]);
		strcat(msg, " ");
		size += strlen(argv[n]) + 1;
	}
	
	if (_allWhiteSpace(msg))
		return;
	
	//locally display the private message that was sent

	s[0] = "msg_public";
	s[1] = "To COMMANDER";
	s[2] = msg;
	corec.GUI_Notify(3, s);

	CL_Play2d(Snd("msg_send"), 1.0, CHANNEL_AUTO);

	if (msg[0] == '/')
		corec.GUI_Exec(&msg[1]);
	else
		corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_CHAT_COMMANDER_MSG, msg));
}

void	CL_Team_Cmd(int argc, char *argv[])
{
  	int team;
  	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: team [team_number]\n"));
		return;
	}

	team = atoi(argv[0]);
	CL_SendTeamJoin(team);
}

void	CL_Unit_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: unit [unit_name]\n"));
		return;
	}

	corec.Client_SendMessageToServer(fmt("unit %s", argv[0]));
}

void	CL_Spawn_Cmd(int argc, char *argv[])
{
	corec.Client_SendMessageToServer("spawnfrom -1");
}

void	CL_SpawnFrom_Cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	corec.Client_SendMessageToServer(fmt("spawnfrom %i", atoi(argv[0])));
}

void	CL_SendPurchaseRequest(int obj_type, int builder_index)
{
	if (cl.isCommander)
	{
		corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_COMMANDER_PURCHASE_MSG, obj_type, builder_index));
		//CL_InterfaceEvent(IEVENT_RESEARCH);
	}
}

void	CL_SendActivateRequest(int source, int obj_type, int target)
{
	if (cl.isCommander)
		corec.Client_SendMessageToServer(fmt("%s %i %i %i", CLIENT_COMMANDER_ACTIVATE_MSG, source, obj_type, target));
}

void	CL_RegisterTestCommands();




void	CL_NotifyMessage(const char *msg, const char *soundfile)
{
	int n;
	static unsigned int toggle = 0;

	toggle++;

	if (msg[0] && msg[0] != '\n')
	{
		for (n = 0; n < cl.ui_numNotifications; n++)
		{
			GUI_ScrollBuffer_Printf(cl.ui_notifications[n], (char *)msg, 4, "");
		}
		
		corec.Console_Printf(fmt("%s%c", msg, (msg[strlen(msg)-1]!='\n') ? '\n' : 0));
	}

	if (soundfile)
	{
		if (soundfile[0])
			CL_Play2d(soundfile, 1.0, CHANNEL_NOTIFICATION + (toggle % 3));
	}
	else
	{
		//don't clobber the notification channels with the default notify sound
		CL_Play2d(Snd("general_notification"), 1.0, CHANNEL_AUTO);
	}

	if (cl_showLobby.integer && msg[0])
	{
		char *s[2];
		//if we're in lobby, print all notifications to the chat buffer		
		s[0] = "server";
		s[1] = (char *)msg;

		corec.GUI_Notify(2, s);
	}

	//synthesized voice support
	corec.Speak(msg);
}

void	CL_ObituaryMessage(int attacker, int target, const char *msg)
{
	char *str;
	char *soundfile;

	//CL_NotifyMessage(msg,NULL);
	if (cl.clients[target].info.team != cl.info->team)
	{
		//someone on the other team died
		str = fmt("^icon %s^  %s", Tex("enemy_death"), msg);
		soundfile = Snd("enemy_death");		
	}
	else
	{
		str = fmt("^icon %s^  %s", Tex("friend_death"), msg);
		soundfile = Snd("friend_death");		
	}

	CL_NotifyMessage(fmt("%s%s", msg, msg[strlen(msg)-1] == '\n' ? "" : "\n"), soundfile);
}

bool    CL_MouseDown1()
{   
	//don't process the mouseclick if the GUI is in use
	if (INT_Locked())
		return true;

	if (cl.isCommander)
	{
		cl.cmdrPendingInput |= CMDR_MOUSEDOWN1;
		return true;
	}

	return false;
}

bool    CL_MouseUp1()
{
	if (INT_Locked())
		return true;

	if (cl.isCommander)
	{
		cl.cmdrPendingInput |= CMDR_MOUSEUP1;
		return true;
	}

	return false;
}

//right-click
bool 	CL_MouseDown2()
{    
	//don't process the mouseclick if the GUI is in use
	if (INT_Locked())
		return true;

	if (cl.isCommander)
	{
		cl.cmdrPendingInput |= CMDR_MOUSEDOWN2;
		return true;
	}

	return false;
}

bool 	CL_MouseUp2()
{
	if (INT_Locked())
		return true;

	if (cl.isCommander)
	{
		cl.cmdrPendingInput |= CMDR_MOUSEUP2;
		return true;
	}

	return false;
}


bool	CL_InputEvent(int key, char rawchar, bool down)
{
	if (rawchar != 0)
		corec.Console_Printf("Key event: %i %c %i\n", key, rawchar, down);
	else
		corec.Console_Printf("Key event: %i %i %i\n", key, rawchar, down);

	if (down && cl.voiceActive)
	{
		if (key >= '0' && key <= '9')
		{
			return CL_VoiceChatNumberPress(key - '0');			
		}
		if (key == KEY_ESCAPE)
		{
			CL_VoiceChatHide();
			return true;
		}
	}

	switch (key)
	{
		case KEY_LBUTTON:
			if (down)
				return CL_MouseDown1();
			else
				return CL_MouseUp1();

		case KEY_RBUTTON:
			if (down)
				return CL_MouseDown2();
			else
				return CL_MouseUp2();

		case KEY_ESCAPE:
			if (cl.isCommander && down)
				return CL_CommanderEscapeKey();
			else
				return false;		
	}
	return false;
}

void	CL_Kill_Cmd(int argc, char *argv[])
{
	if (cl.isCommander)
		return;

	if (cl.status != STATUS_PLAYER)
		return;

	corec.Client_SendMessageToServer(CLIENT_KILL_MSG);
}

void	CL_Eject_Cmd(int argc, char *argv[])
{
	if (cl.isCommander)
		return;

	if (cl.status != STATUS_PLAYER || !CL_ObjectType(cl.predictedState.unittype)->canEject)
		return;

	corec.Client_SendMessageToServer(CLIENT_EJECT_MSG);
}

void	CL_ControlProxy_Cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	corec.Client_SendMessageToServer(fmt("%s %i", "cproxy", atoi(argv[0])));
}

void	CL_FullProxy_Cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	corec.Client_SendMessageToServer(fmt("%s %i", "proxy", atoi(argv[0])));
}

void	CL_ViewProxy_Cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	corec.Client_SendMessageToServer(fmt("%s %i", "vproxy", atoi(argv[0])));
}

void	CL_ReleaseProxy_Cmd(int argc, char *argv[])
{
	corec.Client_SendMessageToServer("rproxy");
}

void	CL_LookProxy_Cmd(int argc, char *argv[])
{
	vec3_t dir,endpos;
	traceinfo_t trace;

	Cam_ConstructRay(&cl.camera, cl_mousepos_x.integer, cl_mousepos_y.integer, dir);
	M_PointOnLine(cl.camera.origin, dir, 99999, endpos);
	corec.World_TraceBox(&trace, cl.camera.origin, endpos, zero_vec, zero_vec, 0);

	if (trace.fraction < 1)
		if (trace.index < MAX_CLIENTS && trace.index >= 0)
			corec.Client_SendMessageToServer(fmt("%s %i", "proxy", trace.index));

}

void	CL_Give_Cmd(int argc, char *argv[])
{
	byte type;

	if (!argc)
		return;

	type = CL_GetObjectTypeByName(argv[0]);

	if (type)	
		corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_GIVE_MSG, type));
}

void	CL_GiveBack_Cmd(int argc, char *argv[])
{
	int slot;

	if (!argc)
		return;

	slot = atoi(argv[0]);


	if (slot >= 0 && slot < MAX_INVENTORY)
		corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_GIVEBACK_MSG, slot));
}

void	CL_HideAllInterface()
{
	corec.GUI_HideAllPanels();
}



/*==========================

  CL_EnterLobby

  bring up the lobby interface

 ==========================*/

void	CL_EnterLobby()
{
	corec.GUI_ResetFocus();
	cl.uistate = UISTATE_NORMAL;
	CL_HideAllInterface();

	cl.guimode = true;
	cl.draw_world = false;
	corec.Console_DPrintf("Setting mouse mode to FREE\n");
	corec.Input_SetMouseMode(MOUSE_FREE);
	corec.Cmd_Exec("setBindProfile 0");
	corec.GUI_Exec(fmt("exec %s/ui_status_lobby.cfg", corec.Cvar_GetString("gui_basepath")));
}


/*==========================

  CL_EnterInGameMenu

  bring up the in game menu

 ==========================*/

void	CL_EnterInGameMenu()
{
	corec.GUI_ResetFocus();
	cl.uistate = UISTATE_NORMAL;
	CL_HideAllInterface();

	cl.guimode = true;
	corec.Input_SetMouseMode(MOUSE_FREE);
	corec.Cmd_Exec("setBindProfile 0");
	corec.GUI_Exec(fmt("exec %s/ui_status_ingame_menu.cfg", corec.Cvar_GetString("gui_basepath")));
}


/*==========================

  CL_EnterLoadout

  bring up the loadout (unit select) menu

 ==========================*/

void	CL_EnterLoadout()
{
	corec.GUI_ResetFocus();
	cl.uistate = UISTATE_NORMAL;
	CL_HideAllInterface();

	cl.guimode = true;
	cl.draw_world = false;

	corec.Input_SetMouseMode(MOUSE_FREE);
	corec.Cmd_Exec("setBindProfile 0");
	corec.GUI_Exec(fmt("exec %s/ui_status_unit_select.cfg", corec.Cvar_GetString("gui_basepath")));
}


/*==========================

  CL_StatusChange

  Switch to a new server decreed status

  Also used to initialize / remove fog of war when switch in / out of commander mode

 ==========================*/

void	CL_StatusChange(int status)
{
	//make sure the GUI doesn't get 'stuck' if switching status
	corec.GUI_ResetFocus();
	cl.uistate = UISTATE_NORMAL;

	CL_HideAllInterface();
	corec.Console_DPrintf("status is %i\n", status);

	if (status == STATUS_LOBBY)
	{	
		CL_EnterLobby();
	}
	else if (status == STATUS_ENDGAME)
	{
		cl.guimode = false;
		cl.draw_world = true;

		corec.Console_DPrintf("Setting mouse mode to RECENTER\n");
		corec.Cmd_Exec("setBindProfile 0");
		corec.Input_SetMouseMode(MOUSE_RECENTER);		

		if (cl.statsPopupTime == -1)	//already been displayed
			corec.GUI_Exec(fmt("exec %s/ui_status_endgame.cfg", corec.Cvar_GetString("gui_basepath")));
	}
	else if (status == STATUS_UNIT_SELECT)
	{	
		CL_EnterLoadout();
	}
	else if (status == STATUS_SPAWNPOINT_SELECT)
	{
		cl.guimode = true;
		cl.draw_world = false;
		corec.Console_DPrintf("Setting mouse mode to FREE\n");
		corec.Input_SetMouseMode(MOUSE_FREE);
		corec.GUI_Exec(fmt("exec %s/ui_status_spawnpoint_select.cfg", corec.Cvar_GetString("gui_basepath")));
	}
	else if (status == STATUS_PLAYER)
	{
		int n;

		cl.draw_world = true;
		cl.guimode = false;
		if (CL_ObjectType(cl.predictedState.unittype)->isVehicle)
		{
			corec.Console_DPrintf("Setting mouse mode to FREE_INPUT\n");
			corec.Input_SetMouseMode(MOUSE_FREE_INPUT);
		}
		else
		{
			corec.Console_DPrintf("Setting mouse mode to RECENTER\n");
			corec.Input_SetMouseMode(MOUSE_RECENTER);
		}

		corec.GUI_Exec(fmt("exec %s/ui_status_player.cfg", corec.Cvar_GetString("gui_basepath")));

		//clear all the standard buttons, to prevent stuck keys
		for (n = 1; n <= 8; n++)
			corec.Cvar_SetValue(fmt("button%i", n), 0);
		corec.Cvar_SetValue("move_forward", 0);
		corec.Cvar_SetValue("move_back", 0);
		corec.Cvar_SetValue("move_left", 0);
		corec.Cvar_SetValue("move_right", 0);
		corec.Cvar_SetValue("move_up", 0);
		corec.Cvar_SetValue("move_down", 0);
		//cl.wishitem = 0;	//always default to melee on spawn
		//cl.playerstate.item = cl.predictedState.item = 0;

		//make sure the userlist widgets aren't interactive
		corec.GUI_Exec("select team1_userlist_names; param interactive false");
		corec.GUI_Exec("select team2_userlist_names; param interactive false");

		core.Cvar_SetValue("gfx_nearclip", 2);
		core.Cvar_SetValue("gfx_farclip", corec.WR_FarClip());

		corec.Cvar_SetValue("ter_ambient_r", cl.oldAmbient_r);
		corec.Cvar_SetValue("ter_ambient_g", cl.oldAmbient_g);
		corec.Cvar_SetValue("ter_ambient_b", cl.oldAmbient_b);

		//we don't want fog of war for players
		corec.WR_ClearDynamap();
		corec.World_UseColormap(true);

		//cancel requests
		if (cl.pendingRequest.active)
		{
			memset(&cl.pendingRequest, 0, sizeof(cmdrRequest_t));
			cl.pendingRequest.active = false;

			corec.Client_SendMessageToServer(fmt("%s", CLIENT_COMMANDER_CANCEL_MSG));
			CL_UpdatePendingRequestCvars();
			CL_InterfaceEvent(IEVENT_REQUEST);
		}
	}
	else if (status == STATUS_SPECTATE)
	{
		cl.draw_world = true;
		cl.guimode = false;
		corec.Input_SetMouseMode(MOUSE_RECENTER);

		corec.GUI_Exec(fmt("exec %s/ui_status_spectate.cfg", corec.Cvar_GetString("gui_basepath")));

		core.Cvar_SetValue("gfx_nearclip", 2);
		core.Cvar_SetValue("gfx_farclip", corec.WR_FarClip());
		core.Cvar_SetValue("item", 0);			//select item 0 in our inventory

		corec.Cvar_SetValue("ter_ambient_r", cl.oldAmbient_r);
		corec.Cvar_SetValue("ter_ambient_g", cl.oldAmbient_g);
		corec.Cvar_SetValue("ter_ambient_b", cl.oldAmbient_b);

		//we don't want fog of war for spectators
		corec.WR_ClearDynamap();
		corec.World_UseColormap(true);

	}
	else if (status == STATUS_COMMANDER)
	{
		cl.draw_world = true;
		cl.guimode = true;
		corec.Console_DPrintf("Setting mouse mode to FREE\n");
		corec.Input_SetMouseMode(MOUSE_FREE);			
		corec.GUI_Exec(fmt("exec %s/ui_status_commander.cfg", corec.Cvar_GetString("gui_basepath")));
		if (cl.info->team == 1)
			corec.GUI_Exec("select team1_userlist_names; param interactive true");
		else
			corec.GUI_Exec("select team2_userlist_names; param interactive true");		

		if (cl.lastStatus != STATUS_COMMANDER)
		{
			cl.oldAmbient_r = corec.Cvar_GetValue("ter_ambient_r");
			cl.oldAmbient_g = corec.Cvar_GetValue("ter_ambient_g");
			cl.oldAmbient_b = corec.Cvar_GetValue("ter_ambient_b");
		}
		if (!cl_oldCommanderLook.value)
		{
			corec.Cvar_SetValue("ter_ambient_r", 0.2);
			corec.Cvar_SetValue("ter_ambient_g", 0.2);
			corec.Cvar_SetValue("ter_ambient_b", 0.2);
		
			corec.World_UseColormap(false);

			cl.fogCleared = false;		//so we completely initialize it
			//Rend_ResetFogOfWar();
		}
		else
		{
			//corec.GUI_Exec("cl_fogOfWarDistance 10000");

			corec.World_UseColormap(true);
			corec.WR_ClearDynamap();
		}

		cl.potentialSelection.numSelected = 0;
		CL_ProcessPotentialSelection();
		CL_CommanderInitializeCamera();
	}

	cl.lastStatus = status;
}





/*==========================

  CL_CheckStatusChange

  checks for changes in GUI status and executes the appropriate script

  if cl_inGameMenu is set to 1, the main menu will always be displayed
  otherwise, if cl_showLobby is set to 1, the lobby will be displayed
  otherwise, the current client status will take effect

 ==========================*/

void	CL_CheckStatusChange()
{
	//first check the GUI override variables
	//these are checked in order of precedence (important)

	if (!corec.Client_IsPlayingDemo())
	{
		if (cl_inGameMenu.integer)
		{
			if (cl_inGameMenu.modified)
			{
				CL_EnterInGameMenu();
				cl_inGameMenu.modified = false;
			}

			return;
		}
		else if (cl_showLobby.integer)
		{
			if (cl_showLobby.modified || cl_inGameMenu.modified)
			{
				CL_EnterLobby();				
				cl_showLobby.modified = false;
				cl_inGameMenu.modified = false;
			}
			
			return;		
		}
	}
	else
	{
		cl_inGameMenu.modified = false;
		cl_showLobby.modified = false;
	}
	
	//if status changed, or cl_showLobby and cl_inGameMenu have been turned off
	if (cl.status != cl.lastStatus || cl_inGameMenu.modified || cl_showLobby.modified)
	{
		CL_StatusChange(cl.status);
		cl_inGameMenu.modified = false;
		cl_showLobby.modified = false;
	}	
}


/*==========================

  CL_UpdatePendingRequestCvars

  Updates cvars relevant to a player's own request

 ==========================*/

extern cvar_t	cl_pendingrequest_active;
extern cvar_t	cl_pendingrequest_money;
extern cvar_t	cl_pendingrequest_description;
extern cvar_t	cl_pendingrequest_time;

void	CL_UpdatePendingRequestCvars()
{
	int seconds;

	corec.Cvar_SetVarValue(&cl_pendingrequest_active, cl.pendingRequest.active);
	if (cl.pendingRequest.active)
	{
		corec.Cvar_SetVar(&cl_pendingrequest_money, fmt("$%i", cl.pendingRequest.money));
		seconds = (cl.gametime - cl.pendingRequest.timestamp) / 1000;
		corec.Cvar_SetVar(&cl_pendingrequest_time, fmt("%02i:%02i", seconds / 60, seconds % 60));
		switch (cl.pendingRequest.type)
		{
		case REQUEST_MONEY:
			corec.Cvar_SetVar(&cl_pendingrequest_description, "Money");
			break;

		case REQUEST_PROMOTE:
			corec.Cvar_SetVar(&cl_pendingrequest_description, "Promotion");
			break;

		case REQUEST_OBJECT:
		case REQUEST_POWERUP:
		case REQUEST_STRUCTURE:
			corec.Cvar_SetVar(&cl_pendingrequest_description, fmt("%s", CL_ObjectType(cl.pendingRequest.object)->description));
			break;

		default:
			corec.Cvar_SetVar(&cl_pendingrequest_description, requestNames[cl.pendingRequest.type]);
			break;
		}
	}

}

/*==========================

  CL_UpdateCvars

  Updates cvars that the GUI uses to display info
  Also generates appropriate ievents when data chagnes

  ==========================*/

extern void CL_UpdateClientInfoVars();

void	CL_UpdateCvars()
{
	objectData_t *unit;	
	int healthPercent;
	objectData_t	*wd;
	int index, maxpop = 0, numpop = 0, idlepop = 0;

	CL_UpdateClientInfoVars();

	unit = CL_ObjectType(cl.predictedState.unittype);
	cl.objects[cl.clientnum].visual.type = cl.predictedState.unittype;

	//level and experience
	corec.Cvar_SetVarValue(&player_level, cl.predictedState.score.level);
	corec.Cvar_SetVarValue(&player_percentNextLevel, Exp_GetPercentNextLevel(cl.race, cl.predictedState.score.level, cl.predictedState.score.experience));

	//request info
	if (cl.isCommander)
		CMDR_UpdateRequestCvars();
	CL_UpdatePendingRequestCvars();

	//commandcenter status
	corec.Cvar_SetVarValue(&team_commandcenter_health, cl.objects[cl.teams[cl.info->team].command_center].visual.health);
	corec.Cvar_SetVarValue(&team_commandcenter_maxhealth, cl.objects[cl.teams[cl.info->team].command_center].visual.fullhealth);
	corec.Cvar_SetVarValue(&team_commandcenter_healthpercent, (team_commandcenter_health.integer / team_commandcenter_maxhealth.value) * 100);

	if (cl.teams[cl.info->team].commander != team_commander.integer)
	{
		corec.Cvar_SetVarValue(&team_commander, cl.teams[cl.info->team].commander);
		corec.Cvar_SetVar(&team_commandername, cl.clients[cl.teams[cl.info->team].commander].info.name);
	}

	//worker status
	for (index = 1; index < MAX_OBJECT_TYPES; index++)
	{
		objectData_t	*obj;

		obj = CL_ObjectType(index);
		
		if (!obj->touched)
			break;

		if (obj->race != cl.race)
			continue;

		if (!obj->isWorker)
			continue;

		maxpop += obj->maxPopulation;
		numpop += cl.research[index].count;
	}

	for (index = MAX_CLIENTS; index < MAX_OBJECTS; index++)
	{
		if (!cl.objects[index].visual.active)
			continue;

		if (cl.objects[index].visual.team != cl.clients[cl.clientnum].info.team)
			continue;

		if (!IsWorkerType(cl.objects[index].visual.type))
			continue;

		if (cl.objects[index].visual.animState == AS_IDLE && cl.objects[index].animStateTime >= 1000)
			idlepop++;
	}

	corec.Cvar_SetVarValue(&team_numworkers, numpop);
	corec.Cvar_SetVarValue(&team_maxworkers, maxpop);
	corec.Cvar_SetVarValue(&team_idleworkers, idlepop);

	//health
	if (cl.predictedState.fullhealth)
		healthPercent = (cl.predictedState.health / (float)cl.predictedState.fullhealth) * 100.0;
	else
		healthPercent = 0;

	if (healthPercent != player_healthpercent.value)
		corec.Cvar_SetVarValue(&player_healthpercent, healthPercent);		

	if (cl.predictedState.health != player_health.integer)
	{
		corec.Cvar_SetVarValue(&player_health, cl.predictedState.health);
		CL_InterfaceEvent(IEVENT_HEALTH);
	}

	if (cl.predictedState.score.money != player_money.integer)
	{
		corec.Cvar_SetVarValue(&player_money, cl.predictedState.score.money);
		CL_InterfaceEvent(IEVENT_MONEY);
	}

	//stamina
	if (cl.predictedState.maxstamina)
		corec.Cvar_SetVarValue(&player_staminapercent, (100 * cl.predictedState.stamina / cl.predictedState.maxstamina));	
	else
		corec.Cvar_SetVarValue(&player_staminapercent, 0);
	corec.Cvar_SetVarValue(&player_stamina, cl.predictedState.stamina);

	//loyalty
	corec.Cvar_SetVarValue(&player_loyalty, cl.predictedState.score.loyalty);

	//weapon overheat
	wd = CL_ObjectType(cl.predictedState.inventory[cl.predictedState.item]);
	if (wd->overheatTime)
		corec.Cvar_SetVarValue(&player_overheatpercent, 100 * cl.predictedState.overheatCounter / (float)wd->overheatTime);
	else
		corec.Cvar_SetVarValue(&player_overheatpercent, 0);
	
	//show players current ammo
	if (cl.predictedState.ammo[cl.predictedState.item] < 0)
	{
		corec.Cvar_SetVar(&player_ammo, "");
		corec.Cvar_SetVar(&player_maxammo, "");
	}
	else
	{
		corec.Cvar_SetVarValue(&player_ammo, cl.predictedState.ammo[cl.predictedState.item]);
		corec.Cvar_SetVarValue(&player_maxammo, CL_ObjectType(cl.predictedState.inventory[cl.predictedState.item])->ammoMax);
	}

	//mana
	if (unit->maxMana > 0)
		corec.Cvar_SetVarValue(&player_mana, cl.predictedState.mana);

	//coordinates
	corec.Cvar_SetVarValue(&player_x, cl.objects[cl.clientnum].visual.pos[X]);
	corec.Cvar_SetVarValue(&player_y, cl.objects[cl.clientnum].visual.pos[Y]);
	corec.Cvar_SetVarValue(&player_z, cl.objects[cl.clientnum].visual.pos[Z]);

	//inventory selection
	corec.Cvar_SetVarValue(&player_item, cl.predictedState.item);	
	if (player_wishitem.integer != cl.wishitem)
	{
		corec.Cvar_SetVarValue(&player_wishitem, cl.wishitem);
		CL_InterfaceEvent(IEVENT_INVENTORY);
	}

	//focus
	if (cl.predictedState.focus != player_focus.value)
		corec.Cvar_SetVarValue(&player_focus, cl.predictedState.focus);

	if (strcmp(cl.ui_spawnbutton->text, "SPAWN") != 0)
		GUI_Button_ShowText(cl.ui_spawnbutton->element, getstring("SPAWN"));

	//respawn timer

	if (!cl.predictedState.statusMessage)
	{		
		switch(cl.serverStatus)
		{
			case GAME_STATUS_SETUP:
				corec.Cvar_SetVar(&game_status, "setup");
				if (cl.info->team)
				{
					if (!cl.info->ready)
						corec.Cvar_SetVar(&player_secondsToRespawn, getstring("Press F3 to indicate you're ready!"));
					else
						corec.Cvar_SetVar(&player_secondsToRespawn, getstring("Waiting for other players..."));
				}
				break;
			case GAME_STATUS_WARMUP:
				corec.Cvar_SetVar(&game_status, "warmup");
				corec.Cvar_SetVar(&player_secondsToRespawn, getstring("Prepare yourself for battle!"));
				break;
			case GAME_STATUS_NORMAL:
				corec.Cvar_SetVar(&game_status, "normal");
				corec.Cvar_SetVar(&player_secondsToRespawn, "");
				break;
			case GAME_STATUS_ENDED:
				corec.Cvar_SetVar(&game_status, "end");
				corec.Cvar_SetVar(&player_secondsToRespawn, "");
				break;
			case GAME_STATUS_RESTARTING:
				corec.Cvar_SetVar(&game_status, "restarting");
				corec.Cvar_SetVar(&player_secondsToRespawn, "");
				break;
			case GAME_STATUS_NEXTMAP:
			case GAME_STATUS_PLAYERCHOSENMAP:
				corec.Cvar_SetVar(&game_status, "nextmap");
				corec.Cvar_SetVar(&player_secondsToRespawn, "");
				break;
			default:
				corec.Cvar_SetVar(&game_status, "other");
				corec.Cvar_SetVar(&player_secondsToRespawn, "");
				break;
		}
	}
	else
	{
		if /*(cl.predictedState.statusMessage & STATUSMSG_WAITING_TO_RESPAWN)*/
			(cl.predictedState.respawnTime > cl.gametime)
		{
			int secs = (int)((cl.predictedState.respawnTime - cl.gametime) / 1000.0 + 1);

			corec.Cvar_SetVar(&player_secondsToRespawn, fmt(getstring("Respawn in %i"), secs));
			if (cl.ui_spawnbutton)
				GUI_Button_ShowText(cl.ui_spawnbutton->element, fmt("%i", secs));
			
		}
		else if (cl.predictedState.respawnTime /*cl.predictedState.statusMessage & STATUSMSG_ATTACK_TO_RESPAWN*/)
		{
			corec.Cvar_SetVar(&player_secondsToRespawn, getstring("Press ATTACK (default Left Mouse) to respawn"));
		}
		else
		{
			corec.Cvar_SetVar(&player_secondsToRespawn, "");
		}
	}



	//unit type
	if (cl.predictedState.unittype != cl.oldPlayerState.unittype)
	{
		if (CL_ObjectType(cl.predictedState.unittype)->objclass == OBJCLASS_UNIT)
		{
			corec.Cvar_SetVar(&player_currentunit, cl.objNames[cl.predictedState.unittype]);
			CL_InterfaceEvent(IEVENT_UNITTYPE);
		}
		else
		{
			corec.Cvar_SetVar(&player_currentunit, "");
		}
	}

	//team resources
	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
	{
		if (resourceData[index].name[0])
			corec.Cvar_SetValue(fmt("resource_%s", resourceData[index].name), cl.resources[index]);
	}

	//officer names
	if (cl.isCommander)
	{
		int index;

		for (index = 0; index < MAX_OFFICERS; index++)
		{
			if (cl.officers[index] >= 0 && cl.officers[index] < MAX_CLIENTS)
				corec.Cvar_SetVar(&cl_cmdr_officers[index], cl.clients[cl.officers[index]].info.name);
		}
	}

	if (cl.endTime)
	{
		int timeleft = cl.endTime - cl.gametime + 1000;		

		if (timeleft < 0)
		{
			corec.Cvar_SetVar(&game_timeLimitMinutes, "0");
			corec.Cvar_SetVar(&game_timeLimitSeconds, "00");
		}
		else
		{
			int secs;
			corec.Cvar_SetVarValue(&game_timeLimitMinutes, (int)(timeleft / 60000));
			secs = (int)(timeleft / 1000) % 60;

			if (secs < 10)
				corec.Cvar_SetVar(&game_timeLimitSeconds, fmt("0%i", secs));
			else
				corec.Cvar_SetVarValue(&game_timeLimitSeconds, secs);
		}
	}

	CL_VoteUpdateCvars(0);

	//the server status cvar is used for a couple of things in addition to server status, like spectating
	
	if (cl.predictedState.statusMessage & STATUSMSG_SPECTATING_PLAYER)
	{
		corec.Cvar_SetVar(&game_serverStatus, fmt(getstring("Spectating %s"), cl.clients[cl.predictedState.chaseIndex].info.name));
	}
	else if (cl.predictedState.statusMessage & STATUSMSG_WAITING_TO_RESPAWN || cl.predictedState.statusMessage & STATUSMSG_ATTACK_TO_RESPAWN)
	{
		//we're spectating someone
		
		{
			if (cl.info->team)
			{
				int t = cl.gametime % 9000;

				if (t > 6000)
					corec.Cvar_SetVar(&game_serverStatus, getstring("Press JUMP (default SPACE) to Spectate"));
				else if (t > 3000)
					corec.Cvar_SetVar(&game_serverStatus, getstring("Press LOADOUT (default L) to pick another unit"));
				else
				{
					if (CL_ObjectType(cl.predictedState.unittype)->revivable)
						corec.Cvar_SetVar(&game_serverStatus, getstring("Wait for a healer to revive you"));
				}
			}

			//corec.Cvar_SetVar(&game_serverStatus, fmt("Following %s", CL_ObjectType(cl.objects[cl.predictedState.chaseIndex].visual.type)->description));
		}		
	}
	else if (cl.draw_world && !cl.info->team)
	{
		corec.Cvar_SetVar(&game_serverStatus, getstring("You are spectating.  Press ESC to pick a team."));
	}
	else
	{
		switch(cl.serverStatus)
		{
			case GAME_STATUS_SETUP:		
				if (cl.endTime)
					corec.Cvar_SetVar(&game_serverStatus, fmt(getstring("Game Setup Mode - ends in %i seconds"), (cl.endTime - cl.gametime) / 1000 + 1));
				else
					corec.Cvar_SetVar(&game_serverStatus, getstring("Game Setup Mode"));
				break;
			case GAME_STATUS_WARMUP:
				corec.Cvar_SetVar(&game_serverStatus, fmt(getstring("WARMUP - game starts in %i seconds"), (cl.endTime - cl.gametime) / 1000 + 1));
				break;
			case GAME_STATUS_RESTARTING:
				corec.Cvar_SetVar(&game_serverStatus, fmt(getstring("Restarting Match...")));
				break;
			case GAME_STATUS_NEXTMAP:
				corec.Cvar_SetVar(&game_serverStatus, fmt(getstring("Loading Next Map...")));
				break;
			case GAME_STATUS_PLAYERCHOSENMAP:
				corec.Cvar_SetVar(&game_serverStatus, fmt(getstring("Loading Chosen Map...")));
				break;
			default:
				corec.Cvar_SetVar(&game_serverStatus, "");
				break;
		}
	}
}

void	CL_UpdateTeamScores()
{
	int i, team1_score, team2_score;

	team1_score = 0;
	team2_score = 0;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!cl.clients[i].info.active)
			continue;

		switch (cl.clients[i].info.team)
		{
			case TEAM_UNDECIDED:
				break;
			case 1:
				team1_score += cl.clients[i].score.kills;
				break;
			case 2:
				team2_score += cl.clients[i].score.kills;
				break;
		}
	}
}


void	CL_GetClanIcons()
{
	int i;
	
	if (!cl_downloadIcons.integer)
		return;
	
	/*//only download clan icons if they're dead, since it's blocking for now
	if (CL_Dead() 
		|| cl.status != STATUS_PLAYER)
	{*/
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (cl.clients[i].info.clan_id != 0)
			{
				CL_GetClanIcon(cl.clients[i].info.clan_id);
			}
		}
	//}
}

void	CL_ShowGameStats()
{
	corec.GUI_Exec("exec /gui/standard/ui_status_endgame.cfg");

	corec.Sound_StopMusic();
	cl.nextMusic[0] = 0;

	//stop BG music and play victory music if we won (play it as a sound effect so we can use the sfx volume)
	if (cl.winStatus == VICTORY)
	{		
		CL_Play2d("music/victory.ogg", 1.0, CHANNEL_MUSIC);
		corec.Cmd_Exec("exec #gui_basepath#/ui_endgame_victory.cfg");
	}
	else if (cl.winStatus == DEFEAT)
	{
		CL_Play2d("music/defeat.ogg", 1.0, CHANNEL_MUSIC);
		corec.Cmd_Exec("exec #gui_basepath#/ui_endgame_defeat.cfg");
	}
}



/*==========================

  CL_UpdateMouse

  Called every frame

 ==========================*/

void	CL_UpdateMouse()
{
	objectData_t *unitData = CL_ObjectType(cl.predictedState.unittype);

	if (cl_freeMouse.integer)
	{
		corec.Input_SetMouseMode(MOUSE_FREE);
		return;
	}

	corec.Cvar_SetValue("minpitch", -89);
	corec.Cvar_SetValue("maxpitch", 89);
	corec.Cvar_SetValue("ysensitivity", cl_firstPersonYSens.value);	

	//scale sensitivity based on zoom
	corec.Input_SetSensitivityScale(cl.camera.fovx / 90);	

	//update the mouse pos
	corec.Input_GetMousePos(&cl.mousepos);

	if ((cl.status == STATUS_PLAYER || cl.status == STATUS_SPECTATE)
		&& !cl_inGameMenu.integer && !cl_showLobby.integer)
	{
		if (!unitData->isVehicle)
		{
			if (corec.Input_GetMouseMode() != MOUSE_RECENTER)
				corec.Input_SetMouseMode(MOUSE_RECENTER);
		}
		else
		{
			if (cl.predictedState.health > 0 && cl.status == STATUS_PLAYER)
			{
				float w, h;
				float minx, miny, maxx, maxy;

				corec.Input_SetMouseMode(MOUSE_FREE_INPUT);

				w = corec.Vid_GetScreenW();
				h = corec.Vid_GetScreenH();

				minx = w * unitData->minAimX;
				maxx = w * unitData->maxAimX;
				miny = h * unitData->minAimY;
				maxy = h * unitData->maxAimY;

				if (cl.mousepos.x > maxx)
					cl.mousepos.x = maxx;
				else if (cl.mousepos.x < minx)
					cl.mousepos.x = minx;

				if (cl.mousepos.y > maxy)
					cl.mousepos.y = maxy;
				else if (cl.mousepos.y < miny)
					cl.mousepos.y = miny;
						
				corec.Input_SetMouseXY(cl.mousepos.x, cl.mousepos.y);				
			}
			else
			{
				//corec.Console_DPrintf("Setting mouse mode to RECENTER\n");
				corec.Input_SetMouseMode(MOUSE_RECENTER);
			}
		}

		if (IsMeleeType(cl.predictedState.inventory[cl.predictedState.item]))
		{
			if (cl_meleeConstrain.integer)
			{
				corec.Cvar_SetValue("minpitch", -25);
				corec.Cvar_SetValue("maxpitch", 30);
			}
			else
			{
				corec.Cvar_SetValue("minpitch", -70);
				corec.Cvar_SetValue("maxpitch", 70);
			}
			corec.Cvar_SetValue("ysensitivity", cl_thirdPersonYSens.value);
		}
	}
	corec.Cvar_SetVarValue(&cl_mousepos_x, cl.mousepos.x);
	corec.Cvar_SetVarValue(&cl_mousepos_y, cl.mousepos.y);
}


/*==========================

  CL_CheckScoreRequest

  update scores every 2 seconds

 ==========================*/

void	CL_CheckScoreRequest()
{
	if (!cl_requestScores.integer)
		return;

	if (cl.gametime - cl.lastScoreRequestTime >= 2000)
	{
		//corec.Console_Printf("sending score req\n");
		corec.Client_SendMessageToServer(CLIENT_SCORE_ALL_MSG);
		cl.lastScoreRequestTime = cl.gametime;
	}
}

#define MUSIC_FADE_TIME	7000

void	CL_UpdateMusic()
{
	if (!cl.nextMusicTime)
		return;	

	if (corec.Milliseconds() >= cl.nextMusicTime)
	{
		cl.musicVolume = corec.Cvar_GetValue("sound_musicVolume");

		corec.Sound_StopMusic();

		if (cl.nextMusic[0])
			corec.Sound_PlayMusic(cl.nextMusic);

		cl.nextMusicTime = 0;
	}
	else
	{
		if (cl.nextMusicTime - corec.Milliseconds() < 500)
			corec.Sound_StopMusic();
		else
		{
		//	corec.Sound_SetChannelVolume(CHANNEL_MUSIC, cl.musicVolume * ((float)((cl.nextMusicTime-500) - corec.Milliseconds()) / (MUSIC_FADE_TIME-500))));		
		}
	}	
}


/*==========================

  CL_Frame

  The bulk of the game code is called from this function.
  It is called every frame, and its responsibilities are to
  render the world and objects, perform any client-side
  prediction, and update the player state

  if we received an update from the server this frame, this is called immediately after it

 ==========================*/

void	CL_Frame(int gametime)
{
	float timeOfDay;
	static float lastGUIUpdate;

	PERF_BEGIN;

	if (!cl.gotObjectTypes)
	{
		core.Game_Error("CL_Frame() called without object types set\n");
	}	

	Perf_ClearClient();

	//cl.frametime = (gametime - cl.gametime) / 1000.0;
	cl.frametime = corec.FrameSeconds();

	cl.gametime = gametime;
	cl.systime = corec.Milliseconds();

	cl.frame++;

	cl.screenscalex = corec.Vid_GetScreenW() / 640.0;
	cl.screenscaley = corec.Vid_GetScreenH() / 480.0;

	if (cl.status == STATUS_COMMANDER && !cl_showLobby.integer && !cl_inGameMenu.integer)
		corec.Cmd_Exec("setBindProfile 1");
	else
		corec.Cmd_Exec("setBindProfile 0");

	corec.Sound_ClearLoopingSounds();

	CL_ResetParticleAllocCount();

	CL_GetClanIcons();	

	if (cl.voteEndTime)
	{
		if (cl.voteEndTime > cl.gametime)
		{
			//makes sure vote panel stays on screen through status changes
			corec.GUI_Exec("show vote_panel");
		}
		else
		{
			corec.GUI_Exec("hide vote_panel");
			cl.voteEndTime = 0;
		}
	}

	corec.Cvar_SetVar(&cl_currentDirectory, core.File_GetCurrentDir());

	//set up all drawing commands to use screen coordinates
	TL_SetCoordSystem(SCREEN_COORDS);
	corec.Draw_SetShaderTime((float)corec.Milliseconds() / 1000.0);		//set time for animated textures drawn in the interface

	timeOfDay = ((gametime * cl.todSpeed + cl.todStart) - 1440 * (int)((gametime * cl.todSpeed + cl.todStart) / 1440));

	TL_SetTimeOfDay(timeOfDay);


	//check for a GUI status change
	CL_CheckStatusChange();

	CL_CheckScoreRequest();

	CL_UpdateMusic();

	CL_UpdateVoiceChat();
	
	//clear the scene now so we can add objects in functions other than Rend_Render()
	if (!cl_skipRender.integer)
		corec.Scene_Clear();

	CL_UpdateMouse();


	//retrieve the 3d mouse position and the current buttons pressed from the core engine
	memcpy(&cl.lastInputstate, &cl.inputstate, sizeof(inputState_t));
	corec.Client_GetInputState(corec.Client_GetCurrentInputStateNum(), &cl.inputstate);
		
	//check for expired notify points
	if (cl.isCommander)
	{
		int	index;

		for (index = 0; index < cl.noticeCount; index++)
		{
			if (cl.noticeQueue[index].expireTime <= gametime)
			{
				cl.noticeCount--;
				memcpy(&cl.noticeQueue[index], &cl.noticeQueue[index+1], sizeof(commanderNotice_t) * cl.noticeCount);
			}
		}
	}

	if (!cl_skipObjects.integer)
	{
		//process all objects
		CL_ProcessObjects();
	}

	if (cl_effects.integer)
		CL_ProcessEffects();

	CL_ProcessPlayerState();

	if (cl.statsPopupTime > 0 && cl.gametime >= cl.statsPopupTime)
	{
		CL_ShowGameStats();

		cl.statsPopupTime = -1; //set it to -1 so it won't repeatedly call ShowGameStats(), but the game knows it should be dispalying stats
	}
	
	if (!cl_skipStuff.integer)
	{
		CL_UpdateEnvironmentEffects();

		CL_UpdateTeamScores();
		CL_UpdateCvars();

		//process simple particle effects
		CL_ParticleFrame();
	}
	
	//let the gui figure its stuff out
//	corec.GUI_CheckMouseAgainstUI(cl.mousepos.x, cl.mousepos.y);

	if (!cl_skipRender.integer)
	{
		if (cl.draw_world)
		{
			//use the current playerState_t to determine our camera position and angle
			Cam_DetermineViewpoint();
	
			//make sure selectable objects are added to scene before we do the commander logic
			Rend_AddObjects();

			if (cl.isCommander && !cl.winStatus)
			{
				CL_CommanderFrame();				
			}

			if (cl.isCommander)
				Rend_AddFogOfWar();

			//render the world and objects
			Rend_Render();
		} else {
			corec.Draw_Quad2d(0, 0, corec.Vid_GetScreenW(), corec.Vid_GetScreenH(), 0, 0, 1, 1, res.blackShader);
		}

		if (cl.isCommander && _toggleManageScreen.integer)
		{
			if (cl.gametime - lastGUIUpdate > 500)
			{
				CMDR_RefreshManageUsers();
				lastGUIUpdate = cl.gametime;
			}
		}
	}

	//make sure to set the toplayer drawing code to use GUI coords at the end of this loop
	//this is because GUI elements will be drawn outside of CL_Frame from the core engine, and
	//the widgets may call TL_* functions
	TL_SetCoordSystem(GUI_COORDS);

	cl.oldPlayerState = cl.predictedState;

	cl.techModified = false;		//if it was true this frame, certain functions will have caught it

	cl.lastwishitem = cl.wishitem;

	// send out placeobject query if necessary
	{
		if ( cl.cmdrPlaceObjectDirty && (cl.systime > cl.cmdrPlaceObjectTime) )
		{
			cl.cmdrPlaceObjectTime = cl.systime + 1000.0f/cl_placeObjectTestRate.value;
			corec.Client_SendMessageToServer(fmt("%s %i %.0f %.0f %.0f %.0f %.0f %.0f %d", 
										CLIENT_COMMANDER_CAN_SPAWN_BUILDING_MSG, 
										cl.cmdrPlaceObjectType,
										cl.cmdrPlaceObjectPos[0], cl.cmdrPlaceObjectPos[1], cl.cmdrPlaceObjectPos[2],
										cl.cmdrPlaceObjectAngle[0], cl.cmdrPlaceObjectAngle[1], cl.cmdrPlaceObjectAngle[2],
										cl.cmdrPlaceObjectReqId));
		}
	}

	PERF_END(PERF_CLIENTFRAME);

	if (showGamePerf.integer)
		Perf_Print();
}



/*==========================

  CL_GetWidget

  ==========================*/

void	*CL_GetWidget(char *itemname, char *classname)
{
	void *ret = corec.GUI_GetClass(UI(itemname), classname);

	if (!ret)
		core.Game_Error(fmt("GUI Error: Couldn't find %s %s (%s)\n", classname, itemname, UI(itemname)));

	return ret;
}

void	*CL_GetPanel(char *panelname)
{
	gui_panel_t *ret = corec.GUI_GetPanel(UI(panelname));

	if (!ret)
		core.Game_Error(fmt("GUI Error: Couldn't find panel %s (%s)\n", panelname, UI(panelname)));

	return ret;
}

/*==========================

  CL_GetWidgetPointers

  Saves pointers to gui widgets that the game code expects to exist

 ==========================*/

void	CL_GetWidgetPointers()
{	
	int n;

	cl.ui_goldgain = CL_GetWidget("textbuffer_goldgain", "textbuffer");

	cl.ui_numNotifications = 0;
	for (n=0; n<MAX_NOTIFICATION_BUFFERS; n++)
	{
		gui_scrollbuffer_t *widget;
		char *widgetname = fmt("textbuffer_notifications_%i", n + 1);

		if (UI(widgetname)[0])
		{
			widget = CL_GetWidget(widgetname, "scrollbuffer");
			if (widget)
				cl.ui_notifications[cl.ui_numNotifications++] = widget;
		}
	}

	cl.ui_numChatBoxes = 0;
	for (n=0; n<MAX_CHATBOXES; n++)
	{
		gui_textbox_t *widget;
		char *widgetname = fmt("textbox_chat_%i", n + 1);

		if (UI(widgetname)[0])
		{
			widget = CL_GetWidget(widgetname, "textbox");
			if (widget)
				cl.ui_chatBoxes[cl.ui_numChatBoxes++] = widget;
		}
	}

	cl.ui_money_icon = CL_GetWidget("graphic_money", "graphic");

	cl.ui_playerTaskPanel = CL_GetPanel("panel_player_task");
	cl.ui_leftarrow = CL_GetWidget("graphic_task_left", "graphic");
	cl.ui_rightarrow = CL_GetWidget("graphic_task_right", "graphic");
	
	cl.ui_money_icon->curFrame = 0;
	cl.ui_money_icon->freezeFrame = 0;
	cl.ui_money_icon->endFrame = 0;
	cl.ui_money_icon->numloops = 0;

	cl.ui_spawnbutton = corec.GUI_GetClass("unit_select_buttons:spawn", "button");		//this can be NULL, we're careful about doing checks against it

	CMDR_InitGUIWidgets();
}


/*==========================

  CL_Restart

  Called when the game starts, or when a new world is loaded

 ==========================*/

void	CL_Restart()
{
	int i;
	vec3_t nul;
	char svinfo[8192];

	memset(&cl, 0, sizeof(clientLocal_t));
	corec.Client_GameObjectPointer(&cl.objects[0].base, sizeof(cl.objects[0]), MAX_OBJECTS);

	//set server name and address vars
	corec.Client_GetStateString(ST_SERVER_INFO, svinfo, sizeof(svinfo));

	corec.Cvar_SetVar(&cl_svAddress, fmt("%s:%s", corec.Cvar_GetString("server_address"), corec.Cvar_GetString("server_port")));
	corec.Cvar_SetVar(&cl_svName, ST_GetState(svinfo, "svr_name"));

	corec.Cvar_Set("cl_chat_msg", "");

	CL_GetWidgetPointers();
	CL_CommanderReset();
	
	CL_HideAllInterface();
	
	cl.realClientnum = corec.Client_GetOwnClientNum();	//this is guaranteed not to change during the course of a game

	//the following clientnum related values will get overridden as soon as we receive a server frame
	//(see CL_BeginServerFrame)
	cl.clientnum = cl.realClientnum;
	cl.player = &cl.objects[cl.realClientnum];
	cl.info = &cl.clients[cl.realClientnum].info;

	corec.World_GetBounds(nul, cl.worldbounds);
	corec.Cvar_SetVarValue(&world_width, cl.worldbounds[0]);
	corec.Cvar_SetVarValue(&world_height, cl.worldbounds[1]);

	cl.lastStatus = -1;
	cl.winStatus = 0;
	
	cl.frame = 0;
	cl.isCommander = false;
	for (i = 0; i < MAX_OFFICERS; i++)
		cl.officers[i] = -1;

	cl.showSelectionRect = false;
	CL_ClearUnitSelection(&cl.selection);
	
	for (i = 0; i < MAX_RESOURCE_TYPES; i++)
		cl.resources[i] = 0;

	for (i = 0; i < MAX_CLIENTS; i++)
		cl.clients[i].waypoint.active = false;

	//clear out old effects
	CL_ResetEffects();
	CL_ResetBeams();
	CL_ResetParticles();

	memset(cl.objects, 0, sizeof(cl.objects));
	//set indexes
	for (i = 0; i < MAX_OBJECTS; i++)
	{
		cl.objects[i].base.index = i;
	}
 
	cl.oldAmbient_r = corec.Cvar_GetValue("ter_ambient_r");
	cl.oldAmbient_g = corec.Cvar_GetValue("ter_ambient_g");
	cl.oldAmbient_b = corec.Cvar_GetValue("ter_ambient_b");

	//save out the near clip plane value to restore after we have been commander
	cl.player_nearclip = 2;
	cl.player_farclip = corec.Cvar_GetValue("gfx_farclip");	//this is the value that will have been loaded from the world .cfg
	
	cl.status = -1; //STATUS_LOBBY;

	cl.guimode = true;
	cl.uistate = UISTATE_NORMAL;

	cl.cmdrPlaceObjectTime = 0;
	cl.cmdrPlaceObjectDirty = false;
	cl.cmdrPlaceObjectValid = false;
	cl.cmdrPlaceObjectReqId = 0;

	//retrigger showing the lobby on a restart, otherwise it will be hidden
	//when it should be visible
	corec.Cvar_SetVarValue(&cl_showLobby, cl_showLobby.integer);
	corec.Cvar_SetVarValue(&cl_inGameMenu, cl_inGameMenu.integer);
}


/*==========================

  CL_Shutdown

  called right before a restart, or on a disconnect

 ==========================*/

void	CL_Shutdown()
{
	int n;

	for (n=0; n<MAX_OBJECTS; n++)
	{
		if (n < MAX_CLIENTS)
			corec.Geom_FreeSkeleton(&cl.clients[n].weaponSkel);
		CL_FreeClientObject(n);
	}

	corec.Geom_FreeSkeleton(&cl.wpSkeleton);
}


/*==========================

  CL_DrawForeground

  Draws anything that we want to be drawn on top of the GUI

 ==========================*/

void	CL_DrawForeground()
{
	
	CMDR_RotatingModelFrame();

	CL_Draw2dParticles();

	if (corec.Client_ConnectionProblems())
	{
		corec.GUI_SetRGBA(1,1,1,1);
		corec.GUI_Quad2d_S(cl_connectionProblemIcon_x.integer, cl_connectionProblemIcon_y.integer, cl_connectionProblemIcon_width.integer, cl_connectionProblemIcon_height.integer, res.connectionProblemShader);
	}
	
}

char 	*CL_GetBuild()
{
	return __builddate;
}

int		CL_AddPositionWaypoint(int sender, int clientnum, vec3_t pos, int goal)
{
	pointinfo_t pi;
	clientInfo_t *client = &cl.clients[clientnum];

	if (goal == GOAL_NONE)
	{
		CL_DestroyWaypoint(clientnum);		
	}
	else
	{
		client->waypoint.active = true;
		client->waypoint.goal = goal;
		client->waypoint.object = false;
		client->waypoint.time_assigned = cl.gametime;
		client->waypoint.fake_waypoint = false;
		client->waypoint.clientnum = sender;
		client->waypoint.commander_order = (cl.teams[cl.info->team].commander == sender);


		corec.World_SampleGround(pos[X], pos[Y], &pi);
		client->waypoint.pos[X] = pos[X];
		client->waypoint.pos[Y] = pos[Y];
		client->waypoint.pos[Z] = pi.z;

		if (clientnum == cl.clientnum)
		{
			corec.GUI_Exec("fadein player_task_panel 500");
			cl.ui_playerTaskPanel->pos[1] = -100;

			if (client->waypoint.commander_order || sender == -1)
				corec.Cvar_SetVar(&player_task, fmt("%s", getstring(CL_GetGoalString(goal))));
			else
			{
				corec.Cvar_SetVar(&player_task, fmt("^b[O]:^w %s", getstring(CL_GetGoalString(goal))));
				cl.clients[sender].voiceIconTime = cl.gametime + cl_voiceIconTime.integer;
			}
		}
	}

	if (cl.lastWaypointUpdate != cl.gametime)
	{		
		if ((cl.isCommander && sender == cl.clientnum) || (!cl.isCommander))
			CL_PlayGoalSound(sender, goal, 0);	
	}

	cl.lastWaypointUpdate = cl.gametime;

	return 0;
}

int		CL_AddObjectWaypoint(int sender, int clientnum, int object, int objecttype, int goal)
{
	clientInfo_t *client = &cl.clients[clientnum];

	if (goal == GOAL_NONE)
	{
		CL_DestroyWaypoint(clientnum);
	}
	else
	{
		client->waypoint.active = true;
		client->waypoint.goal = goal;
		client->waypoint.object = true;
		client->waypoint.object_type = objecttype;
		client->waypoint.object_index = object;
		client->waypoint.time_assigned = cl.gametime;
		client->waypoint.fake_waypoint = false;
		client->waypoint.clientnum = sender;
		client->waypoint.commander_order = (cl.teams[cl.info->team].commander == sender);

		if (clientnum == cl.clientnum)
		{
			corec.GUI_Exec("fadein player_task_panel 500");
			cl.ui_playerTaskPanel->pos[1] = -100;

			if (client->waypoint.commander_order || sender == -1)
				corec.Cvar_SetVar(&player_task, fmt("%s", getstring(CL_GetGoalString(goal))));
			else
			{
				corec.Cvar_SetVar(&player_task, fmt("^b[O]^w %s", getstring(CL_GetGoalString(goal))));
				cl.clients[sender].voiceIconTime = cl.gametime + cl_voiceIconTime.integer;
			}
		}

		if (cl.lastWaypointUpdate != cl.gametime)
		{
			if ((cl.isCommander && sender == cl.clientnum) || (!cl.isCommander))
				CL_PlayGoalSound(sender, goal, objecttype);
		}
	}


	cl.lastWaypointUpdate = cl.gametime;

	return 0;
}

bool	CL_DestroyWaypoint(int clientnum)
{
	cl.clients[clientnum].waypoint.active = false;

	if (clientnum == cl.clientnum)
	{
		corec.GUI_Exec("fadeout player_task_panel 250");
		//corec.GUI_Exec("hide player_task_panel");
		corec.Cvar_SetVar(&player_task, "");
	}

	return true;
}

void	CL_CompleteWaypoint(int sender, int clientnum)
{
	CL_DestroyWaypoint(clientnum);

	if (sender > -1)
		cl.clients[sender].voiceIconTime = cl.gametime + cl_voiceIconTime.integer;

	CL_PlayGoalSound(sender, GOAL_COMPLETED, 0);
}

void	CL_CancelWaypoint(int sender, int clientnum)
{
	CL_DestroyWaypoint(clientnum);

	if (sender > -1)
		cl.clients[sender].voiceIconTime = cl.gametime + cl_voiceIconTime.integer;

	CL_PlayGoalSound(sender, GOAL_NONE, 0);
}

void 	CL_CommanderResign_Cmd(int argc, char *argv[])
{
	//temporarily commented out due to occassional desynchronization between client and server
	//if (cl.isCommander)
		CL_SendCommanderResignation();
}

void 	CL_CommanderRequest_Cmd(int argc, char *argv[])
{
	//temporarily commented out due to occassional desynchronization between client and server
	//if (!cl.isCommander)
		CL_SendCommanderRequest();
}

//=============================================================================
// CL_CL_ObjectType
// 
// Returns the address of an object, mapped to compensate for any client/server
// discrepencies
//=============================================================================
objectData_t	*CL_CL_ObjectType(int objtype)
{
	//return CL_ObjectType(clientObjectTable[objtype]);
	return CL_ObjectType(objtype);
}


bool	CL_ServerNotice(int notice, int param, char *explanation)
{	
	int obj, type;

	//first process the errors, which won't necessarily use the param for an object index
	switch (notice)
	{
		case ERROR_NOTCOMMANDER:
			CL_NotifyMessage(fmt(gettext("You are not the commander.\n")), NULL);
		  	return true;

		case ERROR_NEEDRESOURCE:
		{
			int resourceidx = atoi(explanation);

			if (resourceidx < 0 || resourceidx >= MAX_RESOURCE_TYPES)
			{
				corec.Console_DPrintf("invalid resource index received in ERROR_NEEDRESOURCE\n");
				return false;
			}

			CL_NotifyMessage(fmt(getstring("You need more %s\n"), resourceData[resourceidx].description), CL_RaceSnd(fmt("need_%s", resourceData[resourceidx].name)));
		  	return true;
		}
		case ERROR_NOTAVAILABLE:
			CL_NotifyMessage(fmt(getstring("This item is not available\n")), NULL);
			return true;

		case ERROR_PLAYERNOTFOUND:
			CL_NotifyMessage(fmt(getstring("The player %s was not found\n"), explanation), NULL);
		  	return true;
	}	

	obj = param;
	if (obj < 0 || obj >= MAX_OBJECTS)
		return false;

	switch (notice)
	{
		case NOTICE_ALERT_STRING:
		{						
			CL_NotifyMessage(Snd(fmt("%s_text", explanation)), Snd(explanation));
			break;
		}
		case NOTICE_VICTORY:
			CL_NotifyMessage(fmt("%s\n", gettext(explanation)), CL_RaceSnd("announcer_victory"));
			cl.winStatus = VICTORY;
			//corec.Sound_PlayMusic("music/victory.ogg", false);
			break;

		case NOTICE_DEFEAT:
			CL_NotifyMessage(fmt("%s\n", gettext(explanation)), CL_RaceSnd("announcer_defeat"));
			cl.winStatus = DEFEAT;
			break;

		case NOTICE_BASE_UNDER_ATTACK:
			CL_NotifyMessage(getstring("Our base is under attack!\n"), CL_RaceSnd("announcer_base_under_attack"));			
			break;

		case NOTICE_BEGIN_RESEARCH:
		{
			objectData_t *def;
			type = atoi(explanation);
			def = CL_ObjectType(type);
			if (type < MAX_OBJECT_TYPES)
			{
				cl.objects[obj].itemConstruction = type;
				CL_InterfaceEvent(IEVENT_RESEARCH);
			}

			if (def->objclass == OBJCLASS_ITEM || def->objclass == OBJCLASS_WEAPON)
				CL_NotifyMessage(fmt(getstring("Researching %s"), CL_ObjectType(type)->description), Snd("begin_research"));

			break;
		}
		case NOTICE_UPGRADECOMPLETE:
			cl.objects[obj].itemConstruction = 0;
			CL_AddNotification(obj, NOTICE_BUILDING_COMPLETE, false);
			CL_InterfaceEvent(IEVENT_RESEARCH);
			break;

		case NOTICE_CANCEL_RESEARCH:
			cl.objects[obj].itemConstruction = 0;
			CL_InterfaceEvent(IEVENT_RESEARCH);
			break;

		case NOTICE_QUEUED_RESEARCH:
			{
				char *s = explanation;
				int i, building, itemType;
				clientObject_t *obj;

				for (i = 0; i < MAX_OBJECTS; i++)
					cl.objects[i].numQueuedResearch = 0;
				
				while (true)
				{
					if (!s || !s[0])
						break;
					building = atoi(s);
					s = GetNextWord(s);
					if (!s || !s[0] || building < 0 || building > MAX_OBJECTS)
						break;
					obj = &cl.objects[building];
					itemType = atoi(s);
					if (obj->numQueuedResearch < MAX_CLIENT_QUEUED_RESEARCH)
					{
						obj->researchQueue[obj->numQueuedResearch] = itemType;
						obj->numQueuedResearch++;
					}
				}
	
			}
			break;

		case NOTICE_3_KILLS:
			if (obj == cl.clientnum)
			{
				CL_NotifyMessage(CL_RaceMsg("3_kills"), CL_RaceSnd("3_kills"));
			}
			else
			{
				CL_NotifyMessage(fmt("%s %s\n", cl.clients[obj].info.name, CL_RaceMsg("3_kills_other")), NULL);
			}
			break;
		case NOTICE_5_KILLS:
			if (obj == cl.clientnum)
			{
				CL_NotifyMessage(CL_RaceMsg("5_kills"), CL_RaceSnd("5_kills"));
			}
			else
			{
				CL_NotifyMessage(fmt("%s %s\n", cl.clients[obj].info.name, CL_RaceMsg("5_kills_other")), NULL);
			}
			break;

		case NOTICE_HERO:
			if (obj == cl.clientnum)
			{
				CL_NotifyMessage(CL_RaceMsg("10_kills"), CL_RaceSnd("10_kills"));
			}
			else
			{
				CL_NotifyMessage(fmt("%s %s\n", cl.clients[obj].info.name, CL_RaceMsg("10_kills_other")), NULL);
			}
			break;

		case NOTICE_LEGEND:
			if (obj == cl.clientnum)
			{
				CL_NotifyMessage(CL_RaceMsg("15_kills"), CL_RaceSnd("15_kills"));
			}
			else
			{
				CL_NotifyMessage(fmt("%s %s\n", cl.clients[obj].info.name, CL_RaceMsg("15_kills_other")), NULL);
			}
			break;

		case NOTICE_SKILLFUL:
			CL_NotifyMessage(CL_RaceMsg("skillful"), CL_RaceSnd("skillful"));			
			break;

		case NOTICE_OBITUARY:
		{
			int attacker,target;
			char *s = explanation;
			//obituary message has the format
			//<attacker> <target> <explanation>
			//attacker and target are specified in object indexes
			attacker = atoi(explanation);
			s = GetNextWord(s);
			target = atoi(s);
			s = GetNextWord(s);
			CL_ObituaryMessage(attacker, target, fmt("%s\n", gettext(s)));
			break;
		}
		case NOTICE_BUILDING_CLAIMED:		
			CL_NotifyMessage(fmt("%s\n", gettext(explanation)), CL_RaceSnd("building_claimed"));
			break;		
		case NOTICE_BUILDING_STOLEN:		
			CL_NotifyMessage(fmt("%s\n", gettext(explanation)), CL_RaceSnd("building_stolen"));
			break;
		case NOTICE_SUDDEN_DEATH:
			CL_NotifyMessage(getstring("SUDDEN DEATH!\n"), Snd("sudden_death"));
			break;
		case NOTICE_OVERTIME:
			CL_NotifyMessage(getstring("OVERTIME\n"), Snd("overtime"));
			break;
		case NOTICE_PROMOTE:
			if (param == cl.clientnum)
				CL_NotifyMessage(getstring("You have been given officer status!\n"), CL_RaceSnd("promoted"));
			else
				CL_NotifyMessage("", CL_RaceSnd("promoted"));
			break;
		case NOTICE_DEMOTE:
			if (param == cl.clientnum)
				CL_NotifyMessage(getstring("You are no longer an officer\n"), CL_RaceSnd("demoted"));
			else
				CL_NotifyMessage("", CL_RaceSnd("demoted"));
			break;
		case NOTICE_GENERAL:
		default:
			CL_NotifyMessage(fmt("%s\n", gettext(explanation)), NULL);
			break;
	}
	return true;
}

bool	CL_MessageType(const char *msg, const char *type)
{		
	if ((strncmp(msg, type, strlen(type))==0) && ((msg[strlen(type)] == ' ') || (msg[strlen(type)] == '\0')))
		return true;

	return false;
}



bool	CL_ServerMessage(int sender, char *msg)
{
#ifdef _DEBUG
  FILE* f = fopen("msg.dmp","a+");
  if (NULL != f && NULL != msg)
  {
    fprintf(f,"%s\n",msg);
    fclose(f);
  }
#endif

	if (strncmp(msg, "echo ", 5) == 0)
	{
		//this message gets printed to the console, also
		char *s[2];

		s[0] = "server";
		s[1] = gettext(&msg[5]);   //cl_translate(&msg[5]);

		corec.GUI_Notify(2, s);

		return true;
	}
	//=========================================================================
	else if (CL_MessageType(msg, "conecho"))
	{
		//this is the servers console output
		char	*s;

		s = GetNextWord(msg);
		corec.Console_Printf("^g> %s", s);

		return true;
	}
	//=========================================================================
	else if (CL_MessageType(msg, SERVER_CHAT_MSG))
	{
		char *s[3];
		int offset = 0;
	  
		corec.Console_Printf("%i> %s\n", sender, &msg[strlen(SERVER_CHAT_MSG)+1]);

		s[0] = "msg_public";
		s[1] = strdup(&msg[strlen(SERVER_CHAT_MSG)+1]);
		if (!strstr(s[1], "> "))
			goto truncated_message;
		
		offset = strstr(s[1], "> ") - s[1];
		s[1][offset] = 0;
		offset += 2;
		s[2] = &s[1][offset];
		
		CL_Play2d(Snd("msg_receive"), 1.0, CHANNEL_AUTO);

		corec.GUI_Notify(3, s);

		free(s[1]);
		
		return true;
	}
	//else if (CL_MessageType(msg, SERVER_SCORE_

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_CHAT_TEAM_MSG))
	{
		char *s[3];
		int offset = 0;
	  
		corec.Console_Printf("%i> %s\n", sender, &msg[strlen(SERVER_CHAT_TEAM_MSG)+1]);

		s[0] = "msg_team";
		s[1] = strdup(&msg[strlen(SERVER_CHAT_TEAM_MSG)+1]);
		if (!strstr(s[1], "> "))
			goto truncated_message;
		
		offset = strstr(s[1], "> ") - s[1];
		s[1][offset] = 0;
		offset += 2;
		s[2] = &s[1][offset];
		
		CL_Play2d(Snd("msg_receive"), 1.0, CHANNEL_AUTO);

		corec.GUI_Notify(3, s);

		free(s[1]);
		
		return true;
	}

	else if (CL_MessageType(msg, SERVER_VOICECHAT_MSG))
	{
		char *menu,*category,*item;		
		
		menu = GetNextWord(msg);
		category = GetNextWord(menu);
		item = GetNextWord(category);

		CL_VoiceChat(sender, FirstTok(menu), atoi(category), atoi(item));

		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_CHAT_PRIVATE_MSG))
	{
		char *s[3];
		int offset = 0;
	  
		corec.Console_Printf("%i> %s\n", sender, &msg[strlen(SERVER_CHAT_PRIVATE_MSG)+1]);

		s[0] = "msg_private";
		s[1] = strdup(&msg[strlen(SERVER_CHAT_PRIVATE_MSG)+1]);
		if (!strstr(s[1], "> "))
			goto truncated_message;
		
		offset = strstr(s[1], "> ") - s[1];
		s[1][offset] = 0;
		offset += 2;
		s[2] = &s[1][offset];

		strncpy(lastPrivMsg, s[1], MAX_NAME_LEN);
		
		CL_Play2d(Snd("msg_receive"), 1.0, CHANNEL_AUTO);
		
		corec.GUI_Notify(3, s);

		free(s[1]);

		CL_AddNotification(sender, NOTICE_CHAT, true);
		
		return true;
	}

	//=========================================================================
	// A client has asked the server for a client's cookie
	//=========================================================================
	else if (CL_MessageType(msg, SERVER_CLIENTCOOKIE_MSG))
	{
		char *s = GetNextWord(msg);
		int clientnum;
		char *cookie;

		clientnum = atoi(s);
		
		s = GetNextWord(s);
		cookie = s;
		corec.Console_DPrintf("Client %i's cookie is '%s'\n", clientnum, cookie);

		corec.MasterServer_GetUserInfo(cookie, NULL, 0);

		return true;
	}

	//=========================================================================
	// A client has asked the commander for a something
	//=========================================================================
	else if (CL_MessageType(msg, CLIENT_REQUEST_MSG))
	{
		int requestType, param;
		char *s = GetNextWord(msg);

		if (!cl.isCommander)
			return true;

		requestType = atoi(s);

		s = GetNextWord(s);
		param = atoi(s);

		CMDR_AddRequestToQueue(sender, requestType, param);
		return true;
	}

	//=========================================================================
	// A client has been denied! (clear his pending)
	//=========================================================================
	else if (CL_MessageType(msg, CLIENT_COMMANDER_DECLINE_MSG))
	{
		char *s[2];

		memset(&cl.pendingRequest, 0, sizeof(cmdrRequest_t));
		cl.pendingRequest.active = false;

		s[0] = "server";
		s[1] = getstring("Your request has been ^rdeclined!\n");
		corec.GUI_Notify(2, s);
		CL_Play2d(CL_RaceSnd("declined"), 1.0, CHANNEL_AUTO);

		CL_UpdatePendingRequestCvars();
		CL_InterfaceEvent(IEVENT_REQUEST);
		return true;
	}

	//=========================================================================
	// A client has asked the commander for a something
	//=========================================================================
	else if (CL_MessageType(msg, CLIENT_COMMANDER_APPROVE_MSG))
	{
		char *s[2];

		memset(&cl.pendingRequest, 0, sizeof(cmdrRequest_t));
		cl.pendingRequest.active = false;

		s[0] = "server";
		s[1] = getstring("Your request has been ^gapproved!\n");
		corec.GUI_Notify(2, s);
		CL_Play2d(CL_RaceSnd("approved"), 1.0, CHANNEL_AUTO);

		CL_UpdatePendingRequestCvars();
		CL_InterfaceEvent(IEVENT_REQUEST);
		return true;
	}

	//=========================================================================
	// A client has asked the commander for a something
	//=========================================================================
	else if (CL_MessageType(msg, CLIENT_COMMANDER_EXPIRE_MSG))
	{
		char *s[2];

		memset(&cl.pendingRequest, 0, sizeof(cmdrRequest_t));
		cl.pendingRequest.active = false;

		s[0] = "server";
		s[1] = getstring("Your request has expired...\n");
		corec.GUI_Notify(2, s);
		CL_Play2d(CL_RaceSnd("expired"), 1.0, CHANNEL_AUTO);

		CL_UpdatePendingRequestCvars();
		CL_InterfaceEvent(IEVENT_REQUEST);
		return true;
	}

	//=========================================================================
	// A client has canceled his request
	//=========================================================================
	else if (CL_MessageType(msg, CLIENT_COMMANDER_CANCEL_MSG))
	{
		memset (&cl.cmdrRequests[sender], 0, sizeof(cmdrRequest_t));
		cl.cmdrRequests[sender].active = false;	//just to be safe
		CMDR_UpdateRequestCvars();
		CL_UpdatePendingRequestCvars();
		CL_InterfaceEvent(IEVENT_REQUEST);
		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_COMMANDER_UNITINFO_MSG))
	{
		int i, unitnum, numItems;
		int items[MAX_INVENTORY];
		char *s;

		s = GetNextWord(msg);
		unitnum = atoi(s);
		numItems = MAX_INVENTORY;
		for (i = 0; i < numItems; i++)
		{
			s = GetNextWord(s);
			items[i] = atoi(s);
		}
		CMDR_UpdateUnitInventory(unitnum, numItems, items);
		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_WAYPOINT_POSITION))
	{
		char *pos;
		pointinfo_t pi;
		vec3_t target;
		int goal, cmd_clientnum;
		float x, y;

		pos = &msg[strlen(SERVER_WAYPOINT_POSITION)];
		if (!pos)
			goto truncated_message;
        cmd_clientnum = atoi(pos+1);
		pos = strchr(pos+1, ' ');
		if (!pos)
			goto truncated_message;
        x = atof(pos+1);
		pos = strchr(pos+1, ' ');
		if (!pos)
			goto truncated_message;
        y = atof(pos+1);
		pos = strchr(pos+1, ' ');
		if (!pos)
			goto truncated_message;
        goal = atoi(pos+1);

		//corec.Console_Printf("%i> player %i got message saying player %i was assigned goal of %i at position (%f, %f)\n", clientnum, cmd_clientnum, cl.clientnum, goal, x, y);

		target[X] = x;
		target[Y] = y;
		corec.World_SampleGround(x, y, &pi);
		target[Z] = pi.z;

		CL_AddPositionWaypoint(sender, cmd_clientnum, target, goal);		

		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_WAYPOINT_OBJECT))
	{
		char *pos;
		int goal, object, objecttype, cmd_clientnum;

		pos = &msg[strlen(SERVER_WAYPOINT_OBJECT)];
		if (!pos)
			goto truncated_message;
		
        cmd_clientnum = atoi(pos+1);
		pos = strchr(pos+1, ' ');
		if (!pos)
			goto truncated_message;
        object = atoi(pos+1);
		pos = strchr(pos+1, ' ');
		if (!pos)
			goto truncated_message;
        objecttype = atoi(pos+1);
		pos = strchr(pos+1, ' ');
		if (!pos)
			goto truncated_message;

        goal = atoi(pos+1);

		//corec.Console_Printf("%i> player %i received message saying player %i was assigned goal of %i for object %i\n", clientnum, cmd_clientnum, cl.clientnum, goal, object);
		
		CL_AddObjectWaypoint(sender, cmd_clientnum, object, objecttype, goal);
		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_WAYPOINT_CANCEL))
	{		
		int client = atoi(GetNextWord(msg));

		//corec.Console_Printf("%i> player %i waypoint cleared\n", clientnum, client);

		CL_CancelWaypoint(sender, client);

		return true;
	}
	else if (CL_MessageType(msg, SERVER_WAYPOINT_COMPLETED))
	{
		int client = atoi(GetNextWord(msg));

		CL_CompleteWaypoint(sender, client);

		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_COMMANDER_TEAM_RESOURCES_MSG))
	{
	  	char	*pos;
		int		n = 0;

		pos = &msg[strlen(SERVER_COMMANDER_TEAM_RESOURCES_MSG)];
		for (n = 0; n < MAX_RESOURCE_TYPES; n++)
		{
			if (cl.info && !(raceData[cl.teams[cl.info->team].race].resources & (1 << n)))
			{
				cl.resources[n] = 0;
				continue;
			}

			if (!pos)
				goto truncated_message;
			cl.resources[n] = atoi(pos+1);
			pos = strchr(pos+1, ' ');
		}

		CL_InterfaceEvent(IEVENT_RESOURCES);

		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_NOTICE_MSG))
	{
	  	char *pos;
		int notice, param;

		pos = &msg[strlen(SERVER_NOTICE_MSG)];
		if (!pos)
			goto truncated_message;
        notice = atoi(pos+1);
		pos = strchr(pos+1, ' ');
		if (!pos)
			goto truncated_message;
        param = atoi(pos+1);
		pos = strchr(pos+1, ' ');

		if (*pos)
			CL_ServerNotice(notice, param, pos+1);
		else
			return false;

		return true;
	}

/*	//=========================================================================
	//Server build a new structure
	//=========================================================================
	else if (CL_MessageType(msg, SERVER_COMMANDER_BUILT_STRUCTURE_MSG))
	{
		char *type;

		type = GetNextWord(msg);
		if (type)
		{
			char *numstructs = GetNextWord(type);
			if (numstructs)
			{
				int typenum = atoi(type);
				int inumstructs = atoi(numstructs);

				if (IsBuildingType(typenum))
					cl.research[typenum].count = inumstructs;
			}
		}

		cl.techModified = true;

		return true;
	}*/

	//=========================================================================
	//Server sent a list of the teams officers
	//=========================================================================
	else if (CL_MessageType(msg, SERVER_COMMANDER_OFFICER_LIST_MSG))
	{
		char *s;
		int n, officernum;

		//clear the current list
		for (n = 0; n < MAX_OFFICERS; n++)
			cl.officers[n] = -1;

		//read the new list
		s = GetNextWord(msg);
		for (n = 0; n < MAX_OFFICERS; n++)
		{
			officernum = atoi(s);

			if (officernum < 0 || officernum >= MAX_CLIENTS)
				officernum = -1;
			
			cl.officers[n] = officernum;
			s = GetNextWord(s);
			if (!*s)
				break;
		}

		CL_InterfaceEvent(IEVENT_OFFICERS);

		return true;
	}

	//=========================================================================
	//The team's commander has researched a new upgrade or built a structure
	//=========================================================================
	else if (CL_MessageType(msg, SERVER_COMMANDER_RESEARCH_UPDATE_MSG))
	{
		char *s = GetNextWord(msg);
		int objtype;

		if (s)
		{
			int count;
			objtype = atoi(s);

			if (objtype < 0 || objtype >= MAX_OBJECT_TYPES)
			{
				core.Console_DPrintf("Received bad research update message from server (bad object type)\n");
				return true;
			}

			s = GetNextWord(s);
			if (!s)
				return true;

			count = atoi(s);						
			
			if (count > cl.research[objtype].count)
			{
				char *specific = Snd(fmt("%s_constructed", CL_CL_ObjectType(objtype)->name));
				if (specific[0])
				{
					CL_NotifyMessage(fmt(getstring("%s has been constructed\n"), CL_CL_ObjectType(objtype)->description), specific);
				}
				else if (IsBuildingType(objtype))
				{					
					CL_NotifyMessage(fmt(getstring("%s has been constructed\n"), CL_CL_ObjectType(objtype)->description), CL_RaceSnd("new_building"));
				}
				else if (IsWeaponType(objtype))
				{
					CL_NotifyMessage(fmt(getstring("%s weapon is now available!\n"), CL_CL_ObjectType(objtype)->description), CL_RaceSnd("new_weapon"));
				}
				else if (IsItemType(objtype))
				{
					CL_NotifyMessage(fmt(getstring("%s item is now available!\n"), CL_CL_ObjectType(objtype)->description), CL_RaceSnd("new_item"));				
				}
				else if (IsUnitType(objtype))
				{
					if (!IsWorkerType(objtype))
						CL_NotifyMessage(fmt(getstring("%s unit is now available!\n"), CL_CL_ObjectType(objtype)->description), CL_RaceSnd("new_unit"));
				}
				else
				{
					CL_NotifyMessage(fmt(getstring("%s has been researched\n"), CL_CL_ObjectType(objtype)->description), NULL);
				}
			}

			cl.research[objtype].count = count;
		}

		cl.techModified = true;
		CL_InterfaceEvent(IEVENT_RESEARCH);

		return true;
	}

	//=========================================================================
	//Updated list of deployed items
	//=========================================================================
	else if (CL_MessageType(msg, SERVER_ITEM_DEPLOYMENT_MSG))
	{
	  	char *pos;
		int itemtype, deployed;

		pos = &msg[strlen(SERVER_ITEM_DEPLOYMENT_MSG)];

		while (*pos)
		{
			pos = GetNextWord(pos);
			itemtype = atoi(pos);
			pos = GetNextWord(pos);
			if (!(*pos))
				break;
			deployed = atoi(pos);

			if (itemtype < 0 || itemtype >= MAX_OBJECT_TYPES)
				continue;

			cl.deployment[itemtype] = deployed;
		}

		CL_InterfaceEvent(IEVENT_DEPLOYMENT);

		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_COMMANDER_ALL_RESEARCH_MSG))
	{
		char *s = msg;		
		int n;
		researchData_t research[MAX_OBJECT_TYPES];

		memset(research, 0, sizeof(research));
		
		for (n = 0; n <= MAX_OBJECT_TYPES; n++)
		{
			int i;

			s = GetNextWord(s);
			i = atoi(s);
			if (!s)
				break;
			if (i < 0 || i >= MAX_OBJECT_TYPES)
			{
				corec.Console_DPrintf("Received bad researched items message from server (bad object type)\n");
				return true;
			}
			
			s = GetNextWord(s);
			if (!s)
			{
				corec.Console_DPrintf("Received bad researched items message from server (incomplete)\n");
				break;
			}
			research[i].count = atoi(s);
		}

		memcpy(cl.research, research, sizeof(cl.research));
		cl.techModified = true;

		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_COMMANDER_CAN_SPAWN_BUILDING_MSG))
	{
		int reqid;
		bool valid;
		char* pos = &msg[strlen(SERVER_COMMANDER_CAN_SPAWN_BUILDING_MSG)];
		if (!pos)
			goto truncated_message;
		pos = GetNextWord(pos);
        reqid = atol(pos);
		pos = GetNextWord(pos);
		valid = *pos == 't';
		if ( reqid == cl.cmdrPlaceObjectReqId )
		{
			cl.cmdrPlaceObjectDirty = false;
			cl.cmdrPlaceObjectValid = valid;
		}

		//if ( !valid )
		{
			char notice[256];
			pos = GetNextWord(pos);
			if ( pos && *pos )
			{
				sprintf(notice, getstring("Placement: %s"), gettext(pos));
				CL_ServerNotice(-1, NOTICE_GENERAL, notice);
			}
		}

		return true;
	}

	//=========================================================================
	else if (CL_MessageType(msg, SERVER_COMMANDER_PLACE_LINK_MSG))
	{
		char	*pos = &msg[strlen(SERVER_COMMANDER_PLACE_LINK_MSG)];
		int		twin;
		
		if (!pos)
			goto truncated_message;

		pos = GetNextWord(pos);
        twin = atoi(pos);

		if (twin >= MAX_CLIENTS && twin < MAX_OBJECTS)
		{
			cl.cmdrPlaceObjectTwin = twin;
			cl.cmdrMode = CMDR_PLACING_LINK;
			cl.cmdrPlaceObjectType = cl.objects[twin].base.type;
		}
		return true;
	}

/*
#ifdef S2_INTERNAL_DEV_TESTING
	else if (strncmp(msg, "remote ", 7)==0)
	{
		corec.Cmd_Exec(&msg[7]);

		return true;
	}
#endif
*/	
	return false;

truncated_message:
	corec.Console_DPrintf("Truncated message.  Message ignored.\n");
	
	return true;
}

void    CL_SendCommanderRequest()
{
	if (!cl.isCommander)
		corec.Client_SendMessageToServer(CLIENT_COMMANDER_REQUEST_MSG);
}

void    CL_SendCommanderResignation()
{
	if (cl.isCommander)
		corec.Client_SendMessageToServer(CLIENT_COMMANDER_RESIGN_MSG);
}


//send the request to join a team
void    CL_SendTeamJoin(int team)
{
	corec.Client_SendMessageToServer(fmt("%s %i", CLIENT_TEAM_JOIN_MSG, team));
}


/*==========================

  CL_CycleInventory

  Select the next or previous item that is available

 ==========================*/

void	CL_CycleInventory(int inc)
{
	int startitem = cl.wishitem;
	objectData_t	*obj;

	if (cl.gametime < cl.nextInventoryCycle)
		return;

	if (startitem >= MAX_INVENTORY || startitem < 0)
	{
		cl.wishitem = 0;
		return;
	}

	while(1)
	{	
		cl.wishitem += inc;
		if (cl.wishitem >= MAX_INVENTORY)
			cl.wishitem = 0;
		else if (cl.wishitem < 0)
			cl.wishitem = MAX_INVENTORY-1;

		obj = CL_ObjectType(cl.predictedState.inventory[cl.wishitem]);

		if (cl.wishitem == startitem)
			break;
		if (!cl.predictedState.inventory[cl.wishitem])
			continue;		//nothing in this inventory slot
		if (obj->isSelectable &&
			((!obj->useMana && cl.predictedState.ammo[cl.wishitem]) || 
			(obj->useMana && cl.predictedState.mana >= obj->manaCost) ||
			(obj->objclass == OBJCLASS_MELEE)))
			break;		//a weapon/item that still has ammo/mana
	}

	cl.nextInventoryCycle = cl.gametime + 150;
}


/*==========================

  CL_InvNext_Cmd

 ==========================*/

void	CL_InvNext_Cmd(int argc, char *argv[])
{	
	CL_CycleInventory(1);
}


/*==========================

  CL_InvPrev_Cmd

 ==========================*/

void	CL_InvPrev_Cmd(int argc, char *argv[])
{	
	CL_CycleInventory(-1);
}


/*==========================

  CL_InvSwitch_Cmd

  Switch to an item/weapon in the inventory

 ==========================*/

void	CL_InvSwitch_Cmd(int argc, char *argv[])
{
	int wish;
	
	if (!argc)
		return;

	if (cl.gametime < cl.nextInventoryCycle)
		return;

	wish = atoi(argv[0]);
	if (wish < 0 || wish >= MAX_INVENTORY)
		return;

	if (cl.predictedState.inventory[wish] &&
		CL_ObjectType(cl.predictedState.inventory[wish])->isSelectable)
		cl.wishitem = wish;

	cl.nextInventoryCycle = cl.gametime + 150;
}

/*==========================

  CL_Inv_Cmd

  List the player's inventory

 ==========================*/

void	CL_Inv_Cmd(int argc, char *argv[])
{
	int	i;

	corec.Console_Printf("Inventory:\n");

	for (i = 0; i < MAX_INVENTORY; i++)
	{
		corec.Console_Printf("%i: %s [%i]\n", i, CL_CL_ObjectType(cl.playerstate.inventory[i])->description, cl.playerstate.ammo[i]);
	}
	corec.Console_Printf("Current selection: state: %i / predicted: %i / wish: %i\n", cl.playerstate.item, cl.predictedState.item, cl.wishitem);
}
//=============================================================================


void	CL_Admin_Postpone_Cmd(int argc, char *argv[])
{
	corec.Client_SendMessageToServer(CLIENT_ADMIN_POSTPONE_GAME_MSG);
}

void	CL_Admin_Setting_Cmd(int argc, char *argv[])
{
	corec.Console_Printf("this doesn't do anything yet..\n");
	corec.Client_SendMessageToServer(CLIENT_ADMIN_CHANGE_SETTING_MSG);
}

void	CL_Start_Game_Cmd(int argc, char *argv[])
{
	corec.Client_SendMessageToServer(CLIENT_START_GAME_MSG);
}

/*==========================

  CL_GetBuild_Cmd

  Prints the current build string in the console

 ==========================*/

void	CL_GetBuild_Cmd(int argc, char *argv[])
{
	corec.Console_Printf("Build: %s\n", CL_GetBuild());
}


/*==========================

  CL_IngameMenu_Cmd

  Activate the menu while the game is running
  execs ui_status_ingamemenu.cfg

 ==========================*/

void	CL_IngameMenu_Cmd(int argc, char *argv[])
{
	if (cl_inGameMenu.integer)
	{
		corec.Cvar_SetVarValue(&cl_inGameMenu, 0);
		return;
	}
	else if (cl_showLobby.integer || (cl.status == STATUS_LOBBY && !cl_inGameMenu.integer))
		corec.Cvar_SetVarValue(&cl_inGameMenu, 1);
	else
		corec.Cvar_SetVarValue(&cl_showLobby, 1);
}


/*==========================

  CL_HideIngameMenu_Cmd

 ==========================*/

void	CL_HideIngameMenu_Cmd(int argc, char *argv[])
{
	//CL_StatusChange(STATUS_EXIT_INGAME_MENU);
	corec.Cvar_SetVarValue(&cl_inGameMenu, 0);
}


/*==========================

  CL_ReloadGUI_Cmd

  Destroys all GUI objects and then re-initializes
  Also flushes any scripts in memory

  FIXME: doesn't load apropriate screen when executed from the main menu

 ==========================*/

void	CL_ReloadGUI_Cmd(int argc, char *argv[])
{
	corec.GUI_DestroyAll();
	corec.Cmd_FlushScriptBuffer();
	corec.Cmd_Exec("exec /ui_main.cfg");
	corec.Cmd_Exec("exec /ui_game.cfg");
	CL_GetWidgetPointers();
	INT_InitWidgetPointers();
	CL_CommanderReset();

	corec.GUI_HideAllPanels();
	CL_StatusChange(cl.status);
}


/*==========================

  CL_InterfaceEvent

  Causes the script associated with the event to be executed

 ==========================*/

void	CL_InterfaceEvent(interfaceEvents_enum event)
{
	if (cl_showIevents.integer)
		corec.Console_Printf("->ievent [%s]\n", interfaceEventNames[event]);
	if (ieventScriptTable[event][0] != 0)
		corec.Cmd_Exec(fmt("exec %s", ieventScriptTable[event]));
}



/*==========================

  CL_ClearIEventAssociations_Cmd

  Removes all interface event script associations

 ==========================*/

void	CL_ClearIEventAssociations_Cmd(int argc, char *argv[])
{
	int index;

	for (index = 0; index < NUM_IEVENTS; index++)
	{
		ieventScriptTable[index][0] = 0;
	}
}

/*==========================

  CL_SetIEventScript_Cmd

  Syntax: setIEventScript <event name> <script file>

 ==========================*/

void	CL_SetIEventScript_Cmd(int argc, char *argv[])
{
	int event;

	if (argc < 1)
		return;

	SetInt(&event, 0, NUM_IEVENTS-1, interfaceEventNames, argv[0]);

	if (argc < 2)
	{
		corec.Console_DPrintf("%s: <%s>\n", interfaceEventNames[event], ieventScriptTable[event]);
		return;
	}

	strcpy(ieventScriptTable[event], argv[1]);
	corec.Console_DPrintf("%s: <%s>\n", interfaceEventNames[event], ieventScriptTable[event]);
}


/*==========================

  CL_TriggerIEvent_Cmd

  Causses a specified interface event to trigger, or activates the "trigger" event by default
  Syntax: triggerIEvent [event type]

 ==========================*/

void	CL_TriggerIEvent_Cmd(int argc, char *argv[])
{
	int event;

	if (argc < 1)
	{
		CL_InterfaceEvent(IEVENT_TRIGGER);
		return;
	}

	SetInt(&event, 0, NUM_IEVENTS-1, interfaceEventNames, argv[0]);
	CL_InterfaceEvent(event);
}


/*==========================

  CL_IsOfficer

 ==========================*/

bool	CL_IsOfficer()
{
	int index;

	for (index = 0; index < MAX_OFFICERS; index++)
	{
		if (cl.officers[index] == cl.clientnum)
			return true;
	}

	return false;
}


void	CL_ParseNick_cmd(int argc, char *argv[])
{
	char *namestart;
	
	if (argc < 2)
	{
		corec.Console_DPrintf("error: you must specify a source string and a dest cvar for parseNick\n");
		return;
	}

	namestart = strchr(argv[0], '|');
	if (!namestart)
	{
		corec.Console_DPrintf("%s is not a proper irc nick format\n", argv[0]);
		return;
	}
	
	namestart++;
	corec.Cvar_Set(argv[1], namestart);
}




/*==========================

  CL_EnterBuilding_Cmd

  Player requests to enter a nearby structure

 ==========================*/

void	CL_EnterBuilding_Cmd(int argc, char *argv[])
{
	corec.Client_SendMessageToServer(CLIENT_ENTER_BUILDING_MSG);
}

int		CL_FindHighestStat(user_game_stats_t user_stats[], int offset, int numUsers)
{
	int i, value, highest = -9999999, highest_index = 0;

	for (i = 0; i < numUsers; i++)
	{
		if (user_stats[i].commander)
			continue;

		value = *(int *)((int)&user_stats[i] + offset);
		if (value > highest)
		{
			highest = value;
			highest_index = i;
		}
	}
	return highest_index;
}

int		CL_FindLowestStat(user_game_stats_t user_stats[], int offset, int numUsers)
{
	int i, value, lowest = 9999999, lowest_index = 0;

	for (i = 0; i < numUsers; i++)
	{
		if (user_stats[i].commander)
			continue;
		
		value = *(int *)((int)&user_stats[i] + offset);
		if (value < lowest)
		{
			lowest = value;
			lowest_index = i;
		}
	}
	return lowest_index;
}

static int _stat;

void    _stat_callback(const char *name, const char *value)
{
	gui_element_t *widget;
	int playerIndex;

	playerIndex = atoi(value);
	if (playerIndex < 0 || playerIndex > MAX_CLIENTS)
	{
		return;
	}

	widget = corec.GUI_GetObject(fmt("award%i_label", _stat));
	if (widget)
	{
		GUI_Label_ShowText(widget, (char *)name);

		widget = corec.GUI_GetObject(fmt("award%i", _stat));

		if (widget)
		{
			GUI_Label_ShowText(widget, fmt("<team %i> %s", 
								cl.clients[playerIndex].info.team, 
								cl.clients[playerIndex].info.name));
		}
	}

	_stat++;
}

void	CL_GameStats()
{
	char awards[4096] = {0};
	
	_stat = 0;
	
	corec.Client_GetStateString(ST_AWARDS, awards, 4096);
	
	ST_ForeachState(awards, _stat_callback);

	cl.statsPopupTime = cl.gametime + cl_statsPopupDelay.integer;	
}

int		CL_DetermineGoalForObject(int objidx)
{
	if (objidx >= 0)
	{
		byte type = cl.objects[objidx].base.type;
		byte team = cl.objects[objidx].base.team;
	
		if ((CL_CL_ObjectType(type)->objclass == OBJCLASS_BUILDING) && CL_CL_ObjectType(type)->isMine)
		{
			return GOAL_MINE;
		}
		else if (team == cl.info->team)
		{
			if (IsBuildingType(type))
			{
				objectData_t *def = CL_ObjectType(type);
	
				if (cl.objects[objidx].base.flags & BASEOBJ_UNDER_CONSTRUCTION)
				{
					return GOAL_CONSTRUCT_BUILDING;
				}
				else if (cl.objects[objidx].visual.health < cl.objects[objidx].visual.fullhealth)
				{
					return GOAL_REPAIR;
				}
				else if ((def->canEnter && def->objclass == OBJCLASS_BUILDING) || def->commandCenter)
				{
					return GOAL_ENTER_BUILDING;
				}
				else if (def->canRide)
				{
					return GOAL_ENTER_TRANSPORT;
				}				
				else
				{
					return GOAL_DEFEND;
				}
			}
			else
				return GOAL_FOLLOW;
		}
		else
			return GOAL_ATTACK_OBJECT;
	}
	return GOAL_NONE;
}

void	CL_SendOfficerCommand()
{
	int goal;
			
	if (cl_targetedObject.integer >= 0)
	{
		goal = CL_DetermineGoalForObject(cl_targetedObject.integer);
		corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_OFFICER_OBJECT_GOAL_MSG, goal, cl_targetedObject.integer));
	}
	else
	{
		goal = GOAL_REACH_WAYPOINT;
		corec.Client_SendMessageToServer(fmt("%s %i %.0f %.0f %.0f", CLIENT_OFFICER_LOCATION_GOAL_MSG, goal, cl_targetedTerrainX.value, cl_targetedTerrainY.value, cl_targetedTerrainZ.value));
	}
}

void	CL_SendOfficerCommand_cmd(int argc, char *argv[])
{
	CL_SendOfficerCommand();
}

/*==========================

  CL_MouseFree_cmd

  Change the mouse mode

 ==========================*/

void	CL_MouseFree_cmd(int argc, char *argv[])
{
	cl.guimode = true;
	corec.Input_SetMouseMode(MOUSE_FREE);
}


/*==========================

  CL_MouseRecenter_cmd

  Change the mouse mode

 ==========================*/

void	CL_MouseRecenter_cmd(int argc, char *argv[])
{
	cl.guimode = false;
	corec.Input_SetMouseMode(MOUSE_RECENTER);
}


/*==========================

  CL_MouseLocate_cmd

  Move the mouse pointer to a specified location

 ==========================*/

void	CL_MouseLocate_cmd(int argc, char *argv[])
{
	int x, y;

	if (argc < 2)
	{
		corec.Console_Printf("Not enough parameters\n");
		return;
	}

	x = atoi(argv[0]);
	y = atoi(argv[1]);

	corec.GUI_ScaleToScreen(&x, &y);
	cl.mousepos.x = x;
	cl.mousepos.y = y;
	corec.Cvar_SetVarValue(&cl_mousepos_x, cl.mousepos.x);
	corec.Cvar_SetVarValue(&cl_mousepos_y, cl.mousepos.y);

	if (cl.guimode)
		corec.Input_SetMouseXY(cl.mousepos.x, cl.mousepos.y);
}

/*==========================

  CL_SetGameType

 ==========================*/

void	CL_SetGameType()
{
	char gametype[16];

	corec.Client_GetStateString(ST_GAMETYPE, gametype, 16);

	cl.gametype = atoi(gametype);

	corec.Console_DPrintf("Gametype is %i\n", cl.gametype);
}


/*==========================

  CL_SetTeamInfo

 ==========================*/

void	CL_SetTeamInfo(int team)
{
	char teamInfo[1024];
	int racenum;

	corec.Client_GetStateString(ST_TEAM_INFO + team, teamInfo, 1024);

	cl.teams[team].race = racenum = atoi(ST_GetState(teamInfo, "r"));
	cl.teams[team].commander = atoi(ST_GetState(teamInfo, "c"));
	cl.teams[team].command_center = atoi(ST_GetState(teamInfo, "C"));
	
	corec.Cvar_SetVar(&cl_team1commandername, cl.clients[cl.teams[1].commander].info.name);
	corec.Cvar_SetVar(&cl_team2commandername, cl.clients[cl.teams[2].commander].info.name);
	if (team == cl.info->team)
	{
		corec.Cvar_SetVarValue(&team_commander, cl.teams[team].commander);
		corec.Cvar_SetVar(&team_commandername, cl.clients[cl.teams[team].commander].info.name);
	}

	if (team == 1)
	{
		corec.Cvar_SetVarValue(&team1_race, racenum);
		corec.Cvar_SetVar(&team1_racename, raceData[racenum].name);
	}
	else if (team == 2)
	{
		corec.Cvar_SetVarValue(&team2_race, racenum);
		corec.Cvar_SetVar(&team2_racename, raceData[racenum].name);
	}

	if (!cl.info)
	{
		corec.Console_Printf("error!  cl.info is null in CL_SetTeamInfo!\n");
		return;
	}

	cl.isCommander = cl.info->team && (cl.teams[cl.info->team].commander == cl.clientnum);

	CL_InterfaceEvent(IEVENT_COMMANDER);
}


/*==========================

  CL_CountClients

 ==========================*/

void	CL_CountClients()
{
	int n;

	cl.numActiveClients = 0;

	for (n=0; n<MAX_CLIENTS; n++)
	{
		if (cl.clients[n].info.active)
			cl.numActiveClients++;
	}
}


/*==========================

  CL_SetClientStats

 ==========================*/

void	CL_SetClientStats(int i)
{
	int team;
	char stats[4096];
	char *sortValues[11];
	int numSortValues;
	gui_o_scrollbuffer_t *widget;

	corec.Client_GetStateString(ST_CLIENT_FINAL_STATS + i, stats, 4096);

	corec.Console_DPrintf("Client %i's stats are '%s'\n", i, stats);
	
	if (!stats[0])
	{
		corec.Console_DPrintf("No stats received for client %i\n", i);
		return;
	}
		
	team = cl.clients[i].info.team;

	widget = corec.GUI_GetClass(fmt("stats_team%i_panel:team%i_stats", team, team), "o_scrollbuffer");
	if (!widget)
		return;
	
	numSortValues = 12;
	sortValues[0] = cl.clients[i].info.name;
	sortValues[1] = fmt("%i", (int)cl.clients[i].score.experience);
	sortValues[2] = fmt("%i", cl.clients[i].score.level);
	sortValues[3] = ST_GetState(stats, getstring("Orders Given"));
	sortValues[4] = ST_GetState(stats, getstring("Orders Followed"));
	sortValues[5] = ST_GetState(stats, getstring("Kills"));
	sortValues[6] = ST_GetState(stats, getstring("Deaths"));
	sortValues[7] = ST_GetState(stats, getstring("Buildings Destroyed"));
	sortValues[8] = ST_GetState(stats, getstring("NPC Kills"));
	sortValues[9] = ST_GetState(stats, getstring("AI Kills"));
	sortValues[10] = ST_GetState(stats, getstring("Gold Earned"));
	sortValues[11] = ST_GetState(stats, getstring("Gold Spent"));

	GUI_OrderedScrollBuffer_Printf(widget,
					fmt("%s^col^%i^col^%i^col^%s^col^%s^col^%s^col^%s^col^%s^col^%s^col^%s^col^%s^col^%s^col^", 
							cl.clients[i].info.name, 
							(int)cl.clients[i].score.experience, 
							cl.clients[i].score.level, 
							ST_GetState(stats, getstring("Orders Given")), 
							ST_GetState(stats, getstring("Orders Followed")), 
							ST_GetState(stats, getstring("Kills")), 
							ST_GetState(stats, getstring("Deaths")), 
							ST_GetState(stats, getstring("Buildings Destroyed")), 
							ST_GetState(stats, getstring("NPC Kills")), 
							ST_GetState(stats, getstring("AI Kills")), 
							ST_GetState(stats, getstring("Gold Earned")), 
							ST_GetState(stats, getstring("Gold Spent"))), 
					fmt("%i", i), 
					numSortValues,
					(char **)sortValues);
}


/*==========================

  CL_SetClientScore

  string comes across as a series of ints

 ==========================*/

void	CL_SetClientScore(int clientnum)
{
	char clientScore[1024];
	playerScore_t *score = &cl.clients[clientnum].score;

	corec.Client_GetStateString(ST_CLIENT_SCORE + clientnum, clientScore, 1024);

	if (!clientScore[0])
		return;	

	sscanf(clientScore,
			"%i %i %f %i %i %i %i",
			&score->kills,
			&score->deaths,
			&score->experience,
			&score->level,
			&score->money,
			&score->loyalty,
			&score->ping
		);
		
}


/*==========================

  CL_SetClientInfo

 ==========================*/

void	CL_SetClientInfo(int clientnum)
{
	char clientInfo[1024];
	sharedClientInfo_t oldinfo;

	corec.Client_GetStateString(ST_CLIENT_INFO + clientnum, clientInfo, 1024);

	if (!clientInfo[0])
	{
		//string is blank, which means client is no longer active
		cl.clients[clientnum].info.active = false;
		CL_CountClients();
		return;
	}

	oldinfo = cl.clients[clientnum].info;

	cl.clients[clientnum].info.clan_id = atoi(ST_GetState(clientInfo, "c"));
	cl.clients[clientnum].info.isOfficer = atoi(ST_GetState(clientInfo, "o"));
	strncpySafe(cl.clients[clientnum].info.name, ST_GetState(clientInfo, "n"), CLIENT_NAME_LENGTH);
	cl.clients[clientnum].info.team = atoi(ST_GetState(clientInfo, "t"));
	cl.clients[clientnum].info.ready = atoi(ST_GetState(clientInfo, "rd"));
	cl.clients[clientnum].info.isReferee = atoi(ST_GetState(clientInfo, "r"));

	cl.clients[clientnum].info.active = true;

	if (clientnum == cl.clientnum)
	{
		//this is info about ourself
		corec.Cvar_SetVarValue(&player_referee, cl.info->isReferee);
		corec.Cvar_SetVarValue(&player_team, cl.info->team);
		cl.gotOurInfo = true;
	}

	CL_CountClients();
}


/*==========================

  CL_GameStatusChanged

  the server has transitioned to lobby mode, end game mode, or the normal game mode
  also called if server changes any other parameter in the string, like endTime

 ==========================*/

void	CL_GameStatusChanged()
{
	int timeleft;
	int status, i;
	bool isnew;
	char string[1024];	
	gui_element_t *widget;

	corec.Client_GetStateString(ST_GAME_STATUS, string, 1024);

	if (!string[0])
		return;

	status = atoi(ST_GetState(string, "status"));
	isnew = (status != cl.serverStatus);
	cl.serverStatus = status;

	cl.endTime = atoi(ST_GetState(string, "endTime"));		//time at which we transition to the next status
	if (cl.endTime)
	{
		timeleft = cl.endTime - cl.gametime;
		if (timeleft)
			corec.Console_DPrintf("Time remaining: %i minutes, %i seconds\n", timeleft / 60000, (timeleft / 1000) % 60);
	}

	switch (cl.serverStatus)
	{
		case GAME_STATUS_EMPTY:
			break;
		case GAME_STATUS_SETUP:			
			//display lobby interface if we just transitioned into setup mode
			if (isnew)
			{
				corec.Cvar_SetVarValue(&cl_showLobby, 1);
		//		corec.GUI_Exec(fmt("exec %s/ui_status_lobby.cfg", gui_basepath->string));
			}
			break;
		case GAME_STATUS_ENDED:
			cl.winObject = atoi(ST_GetState(string, "lookat"));			
	
			corec.WR_ClearDynamap();
			corec.World_UseColormap(true);
			cl.isCommander = false;
			corec.Sound_StopMusic();		//adds drama!
			//intentional fallthrough
		case GAME_STATUS_WARMUP:
		case GAME_STATUS_NORMAL:
			if (isnew)
			{
//				corec.Cvar_SetVarValue(&cl_showLobby, 0);
				if (cl.serverStatus == GAME_STATUS_NORMAL)
				{					
					for (i = 1; i < NUM_TEAMS; i++)
					{
						widget = corec.GUI_GetObject(fmt("stats_team%i_panel:team%i_stats", i, i));
						if (!widget)
							continue;
	
						corec.Console_DPrintf("Clearing team %i's stats widget\n", i);
						GUI_OrderedScrollBuffer_Clear(widget);
					}
				}
			}
			break;
		case GAME_STATUS_RESTARTING:
		case GAME_STATUS_PLAYERCHOSENMAP:
		case GAME_STATUS_NEXTMAP:
			break;
		default:
			core.Game_Error("Unknown server status \"%s\"", status);
	}

	switch (cl.serverStatus)
	{
		case GAME_STATUS_EMPTY:
			corec.Console_DPrintf("GAME_STATUS_EMPTY\n");
			break;
		case GAME_STATUS_SETUP:
			corec.Console_DPrintf("GAME_STATUS_SETUP\n");
			break;
		case GAME_STATUS_WARMUP:
			corec.Console_DPrintf("GAME_STATUS_WARMUP\n");
			break;
		case GAME_STATUS_NORMAL:
			corec.Console_DPrintf("GAME_STATUS_NORMAL\n");
			break;
		case GAME_STATUS_ENDED:
			corec.Console_DPrintf("GAME_STATUS_ENDED\n");
			break;
		case GAME_STATUS_RESTARTING:
			corec.Console_DPrintf("GAME_STATUS_RESTARTING\n");
			break;
		case GAME_STATUS_PLAYERCHOSENMAP:
			corec.Console_DPrintf("GAME_STATUS_PLAYERCHOSENMAP\n");
			break;
		case GAME_STATUS_NEXTMAP:
			corec.Console_DPrintf("GAME_STATUS_NEXTMAP\n");
			break;
		default:
			break;
	}
}

void	CL_SetTimeOfDay()
{
	char string[128];

	corec.Client_GetStateString(ST_TIME_OF_DAY, string, sizeof(string));

	cl.todStart = atof(ST_GetState(string, "start"));
	cl.todSpeed = atof(ST_GetState(string, "speed"));
}

void	CL_SetUrgency()
{
	int urgencylevel, oldlevel;
	char string[16];

	oldlevel = cl.urgencyLevel;

	corec.Client_GetStateString(ST_URGENCY, string, sizeof(string));

	urgencylevel = atoi(string);
	if (urgencylevel == cl.urgencyLevel)
		return;

	cl.urgencyLevel = urgencylevel;
	cl.nextMusicTime = corec.Milliseconds();
	cl.musicVolume = corec.Cvar_GetValue("sound_musicVolume");

	switch (cl.urgencyLevel)
	{
		case URGENCY_NONE:
			cl.nextMusic[0] = 0;			
			break;
		case URGENCY_LOW:
			//fade out old music
			if (oldlevel == URGENCY_HIGH)
			{				
				cl.nextMusicTime += MUSIC_FADE_TIME;
			}
			strcpy(cl.nextMusic, corec.Cvar_GetString("cl_music1"));			
			break;
		case URGENCY_HIGH:
			//play right away
			strcpy(cl.nextMusic, corec.Cvar_GetString("cl_music2"));			
			break;
		default:
			break;
	}
}


void	CL_SetObjectTypeCB(const char *key, const char *objname)
{
	//this is the extern from object_config.c which holds the actual object definitions
	extern objectData_t objData[MAX_OBJECT_TYPES];
	int n;

	int index = atoi(key);

	if (index >= MAX_OBJECT_TYPES || index < 0)
		core.Game_Error("Invalid object type index sent from server\n");

	for (n=0; n<MAX_OBJECT_TYPES; n++)
	{
		if (stricmp(objData[n].name, objname)==0)
		{
			if (!objData[n].objclass)
				core.Game_Error("Bad client side object data (%s)\n", objData[n].name);

			cl.objData[index] = &objData[n];
			cl.objNames[index] = objData[n].name;
			break;
		}
	}

	if (n==MAX_OBJECT_TYPES)
	{
#ifndef SAVAGE_DEMO
		core.Game_Error(fmt("The server is using an object we don't have (%s)\n", objname));	
#else
		core.Game_Error("Connect failed");
#endif
	}
}

objectData_t *_CL_GetObjectByType(int type)
{
	return CL_ObjectType(type);
}

void	CL_SetObjectTypes()
{
	//this is the extern from object_config.c which holds the actual object definitions
	extern objectData_t objData[MAX_OBJECT_TYPES];		
	int n;
	char string[8192];

	//clear array
	for (n=0; n<MAX_OBJECT_TYPES; n++)
	{
		//point them all to the 0th object definition...safer than having NULL pointers
		cl.objData[n] = &objData[0];
		cl.objNames[n] = "";
	}

	corec.Client_GetStateString(ST_OBJECT_TYPES, string, sizeof(string));

	ST_ForeachState(string, CL_SetObjectTypeCB);

	//regenerate the techtree
	Tech_Generate(_CL_GetObjectByType);

	cl.gotObjectTypes = true;
}

/*==========================

  CL_StateStringModified

 ==========================*/

void	CL_StateStringModified(int id)
{	
	switch(id)
	{
		case ST_OBJECT_TYPES:
			CL_SetObjectTypes();
			break;
		case ST_GAMETYPE:
			CL_SetGameType();
			break;

		case ST_VOTES_YES:
		case ST_VOTES_NO:
		case ST_VOTES_MIN:
		case ST_VOTES_NEED:
		case ST_VOTE_TEAM:
		case ST_VOTE_DESCRIPTION:
		case ST_VOTE_END_TIME:
			CL_VoteUpdateCvars(id);
			break;

		case ST_GAME_STATUS:
			CL_GameStatusChanged();
			break;

		case ST_TIME_OF_DAY:
			CL_SetTimeOfDay();
			break;

		case ST_AWARDS:
			CL_GameStats();
			break;

		case ST_URGENCY:
			CL_SetUrgency();

		default:			
			break;
	}

	if (id >= ST_TEAM_INFO && id < ST_TEAM_INFO + MAX_TEAMS)
		CL_SetTeamInfo(id - ST_TEAM_INFO);
	else if (id >= ST_CLIENT_INFO && id < ST_CLIENT_INFO + MAX_CLIENTS)
		CL_SetClientInfo(id - ST_CLIENT_INFO);
	else if (id >= ST_CLIENT_FINAL_STATS && id < ST_CLIENT_FINAL_STATS + MAX_CLIENTS)
		CL_SetClientStats(id - ST_CLIENT_FINAL_STATS);
	else if (id >= ST_CLIENT_SCORE && id < ST_CLIENT_SCORE + MAX_CLIENTS)
		CL_SetClientScore(id - ST_CLIENT_SCORE);
}



void	CL_Say_cmd(int argc, char *argv[])
{
	corec.Console_Printf(getstring("Saying %s\n"), argv[0]);
	corec.Speak(argv[0]);
}

void	CL_Ready_Cmd(int argc, char *argv[])
{
	corec.Client_SendMessageToServer(CLIENT_READY_MSG);
}

void	CL_Referee_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf(getstring("syntax: refpwd <password>\n"));
		return;
	}

	corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_REFEREE_MSG, argv[0]));
}

void	CL_RefereeCommand_Cmd(int argc, char *argv[])
{
	char args[1024];

	ConcatArgs(argv, argc, args);

	corec.Client_SendMessageToServer(fmt("%s %s", CLIENT_REFEREE_COMMAND_MSG, args));
}




/*==========================

  CL_Ignore_Cmd

  reject all incoming messages from the specified client

 ==========================*/

void	CL_Ignore_Cmd(int argc, char *argv[])
{
	int	index, clientnum = -1, found = 0;

	if (!argc)
		return;

	for (index = 0; index < MAX_CLIENTS; index++)
	{
		if (!stricmp(argv[0], cl.clients[index].info.name))
		{
			clientnum = index;
			found++;
		}
	}

	if (found > 1)
	{
		corec.Console_Printf(getstring("Multiple clients named %s, use the ignorenum command.\n"), argv[0]);
		corec.Console_Printf(getstring("Clients:\n--------\n"));
		for (index = 0; index < MAX_CLIENTS; index++)
		{
			if (!stricmp(argv[0], cl.clients[index].info.name))
				corec.Console_Printf("%2i: %s\n", index, cl.clients[index].info.name);
		}
		return;
	}

	if (found == 1)
	{
		cl.ignoring[clientnum] = true;
		corec.Console_Printf(getstring("Now ignoring %s.\n"), argv[0]);
		return;
	}
}


/*==========================

  CL_Unignore_Cmd

  stop rejecting all incoming messages from the specified client

 ==========================*/

void	CL_Unignore_Cmd(int argc, char *argv[])
{
	int	index, clientnum = -1, found = 0;

	if (!argc)
		return;

	for (index = 0; index < MAX_CLIENTS; index++)
	{
		if (!stricmp(argv[0], cl.clients[index].info.name))
		{
			clientnum = index;
			found++;
		}
	}

	if (found > 1)
	{
		corec.Console_Printf(getstring("Multiple clients named %s, use the unignorenum command.\n"), argv[0]);
		corec.Console_Printf(getstring("Clients:\n--------\n"));
		for (index = 0; index < MAX_CLIENTS; index++)
		{
			if (!stricmp(argv[0], cl.clients[index].info.name))
				corec.Console_Printf("%2i: %s\n", index, cl.clients[index].info.name);
		}
		return;
	}

	if (found == 1)
	{
		cl.ignoring[clientnum] = false;
		corec.Console_Printf(getstring("No longer ignoring %s.\n"), argv[0]);
		return;
	}
}


/*==========================

  CL_IgnoreNum_Cmd

  Set a player to ignore by their client number

 ==========================*/

void	CL_IgnoreNum_Cmd(int argc, char *argv[])
{
	int clientnum;

	if (!argc)
		return;

	clientnum = atoi(argv[0]);
	if (clientnum >= 0 && clientnum < MAX_CLIENTS && cl.clients[clientnum].info.active)
	{
		cl.ignoring[clientnum] = true;
		corec.Console_Printf(getstring("Now ignoring %s.\n"), cl.clients[clientnum].info.name);
	}
}


/*==========================

  CL_UnignoreNum_Cmd

  Un-ignore by their client number

 ==========================*/

void	CL_UnignoreNum_Cmd(int argc, char *argv[])
{
	int clientnum;

	if (!argc)
		return;

	clientnum = atoi(argv[0]);
	if (clientnum >= 0 && clientnum < MAX_CLIENTS && cl.clients[clientnum].info.active)
	{
		cl.ignoring[clientnum] = false;
		corec.Console_Printf(getstring("No longer ignoring %s.\n"), cl.clients[clientnum].info.name);
	}
}



/*==========================

  CL_VoiceChat_Cmd

 ==========================*/

void	CL_VoiceChat_Cmd(int argc, char *argv[])
{
	if (argc < 2)
		return;

	corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_VOICECHAT_MSG, atoi(argv[0]), atoi(argv[1])));
}

void	CL_ExtendedServerInfo(const char *ip, int port, int ping, const char *coreInfo, const char *gameInfo)
{
	gui_element_t *widget;
	gui_scrollbuffer_t *sbuffer;
	int time;

	widget = corec.GUI_GetObject("server_info_panel:server_name_label");
	if (widget)
		GUI_Label_ShowText(widget, fmt(getstring("Server Name: %s"), ST_GetState(coreInfo, "name")));

	widget = corec.GUI_GetObject("server_info_panel:ip_label");
	if (widget)
		GUI_Label_ShowText(widget, fmt("IP: %s:%i", ip, port));

	widget = corec.GUI_GetObject("server_info_panel:map_label");
	if (widget)
		GUI_Label_ShowText(widget, fmt(getstring("Map: %s"), gamelist_forcemap.string[0] ? gamelist_forcemap.string : ST_GetState(coreInfo, "map")));

	corec.GUI_Exec(fmt("select server_info_panel:current_map;  param image /world/%s_overhead.tga", gamelist_forcemap.string[0] ? gamelist_forcemap.string : ST_GetState(coreInfo, "world")));
	
	widget = corec.GUI_GetObject("server_info_panel:map_label");
	if (widget)
		GUI_Label_ShowText(widget, fmt(getstring("Map: %s"), gamelist_forcemap.string[0] ? gamelist_forcemap.string : ST_GetState(coreInfo, "world")));

	widget = corec.GUI_GetObject("server_info_panel:players_label");
	if (widget)
		GUI_Label_ShowText(widget, fmt(getstring("Players: %s/%s"), ST_GetState(coreInfo, "cnum"), ST_GetState(coreInfo, "cmax")));

	widget = corec.GUI_GetObject("server_info_panel:type_label");
	if (widget)
		GUI_Label_ShowText(widget, fmt(getstring("Type: %s"), gamelist_forcegametype.string[0] ? gamelist_forcegametype.string : ST_GetState(gameInfo, "gametype")));

	widget = corec.GUI_GetObject("server_info_panel:time_label");
	if (widget)
	{
		time = gamelist_forcetime.string[0] ? gamelist_forcetime.integer : atoi(ST_GetState(gameInfo, "timelimit"));
		GUI_Label_ShowText(widget, fmt(getstring("Time Limit: %02i:%02i"), time / 3600, time % 3600));
	}

	widget = corec.GUI_GetObject("server_info_panel:notes_label");
	if (widget)
		GUI_Label_ShowText(widget, fmt(getstring("Server Notes: %s"), ST_GetState(gameInfo, "notes")));

	sbuffer = corec.GUI_GetClass("server_info_panel:server_player_list", "scrollbuffer");
	if (sbuffer)
	{
		GUI_ScrollBuffer_Clear(sbuffer->element);
		GUI_ScrollBuffer_Printf(sbuffer, ST_GetState(gameInfo, "players"), 0, "");
	}
}


void	CL_ListClients_Cmd(int argc, char *argv[])
{
	int index;

	for (index = 0; index < MAX_CLIENTS; index++)
	{
		if (cl.clients[index].info.active)
			corec.Console_Printf("%2i: %s\n", index, cl.clients[index].info.name);
	}
}


/*==========================

  CL_ShowLoadout_Cmd

  Send a message to the server requesting to view the loadout screen
  This will indicate that we aren't going to wait to be revived

 ==========================*/

void	CL_ShowLoadout_Cmd(int argc, char *argv[])
{
	corec.Client_SendMessageToServer(CLIENT_SHOW_LOADOUT_MSG);
}

void	CL_RegenerateTechTree_Cmd(int argc, char *argv[])
{
	Tech_Generate(_CL_GetObjectByType);
	corec.Cmd_Exec("refreshCommanderGrid\n");
}

void	CL_ActivateChat_Cmd(int argc, char *argv[])
{
	int n;
	char chatcommand[32];

	if (!argc)
	{
		strcpy(chatcommand, "chat");
	}
	else
	{		
		strncpySafe(chatcommand, fmt("chat%s", argv[0]), sizeof(chatcommand));
	}

	for (n=0; n<cl.ui_numChatBoxes; n++)
	{
		if (corec.GUI_IsVisible(cl.ui_chatBoxes[n]->element))
		{
			//activate this chatbox
			corec.Cmd_Exec("set cl_chat_msg \"\"");
			corec.GUI_Select(cl.ui_chatBoxes[n]->element);
			corec.Cmd_Exec("param variable cl_chat_msg\n");
			//param commit_cmd "chat #cl_chat_msg#; set cl_chat_msg \"\""
			corec.Cmd_Exec(fmt("param commit_cmd \"%s #cl_chat_msg#; set cl_chat_msg \\\"\\\"\"\n", chatcommand));
			corec.Cmd_Exec("param abort_cmd \"set cl_chat_msg \\\"\\\"\"\n");

			corec.GUI_Focus(cl.ui_chatBoxes[n]->element);
		}
	}
}


/*==========================

  CL_Register

  Called as soon as the dll is loaded, use this to register commands/cvars as early as possible

  ==========================*/

void	CL_Register()
{
	int n;

	corec.Cmd_Register("ready",				CL_Ready_Cmd);
	corec.Cmd_Register("activateChat",		CL_ActivateChat_Cmd);
	corec.Cmd_Register("chat",				CL_Chat_Cmd);
	corec.Cmd_Register("c",					CL_Chat_Cmd);
	corec.Cmd_Register("chatTeam",			CL_ChatTeam_Cmd);
	corec.Cmd_Register("chatPrivate",		CL_ChatPrivate_Cmd);
	corec.Cmd_Register("chatCommander",		CL_ChatCommander_Cmd);
	corec.Cmd_Register("msg",				CL_ChatPrivate_Cmd);
	corec.Cmd_Register("chatSelected",		CL_ChatSelected_Cmd);
	corec.Cmd_Register("team",				CL_Team_Cmd);	
	corec.Cmd_Register("unit",				CL_Unit_Cmd);
	corec.Cmd_Register("spawnfrom",			CL_SpawnFrom_Cmd);
	corec.Cmd_Register("enterBuilding",		CL_EnterBuilding_Cmd);
	corec.Cmd_Register("commander_resign",	CL_CommanderResign_Cmd);
	corec.Cmd_Register("commander_request",	CL_CommanderRequest_Cmd);
	corec.Cmd_Register("kill",				CL_Kill_Cmd);
	corec.Cmd_Register("eject",				CL_Eject_Cmd);
	corec.Cmd_Register("playonce2d",		CL_Play2d_Cmd);
	corec.Cmd_Register("play2d",			CL_Play2d_Cmd);
	corec.Cmd_Register("listclients",		CL_ListClients_Cmd);

	corec.Cmd_Register("invnext",			CL_InvNext_Cmd);
	corec.Cmd_Register("invprev",			CL_InvPrev_Cmd);
	corec.Cmd_Register("invswitch",			CL_InvSwitch_Cmd);
	corec.Cmd_Register("inventory",			CL_Inv_Cmd);

	corec.Cmd_Register("start_game",		CL_Start_Game_Cmd);
	corec.Cmd_Register("admin_postpone",	CL_Admin_Postpone_Cmd);
	corec.Cmd_Register("admin_setting",		CL_Admin_Setting_Cmd);
	corec.Cmd_Register("give",				CL_Give_Cmd);
	corec.Cmd_Register("cproxy",			CL_ControlProxy_Cmd);
	corec.Cmd_Register("proxy",				CL_FullProxy_Cmd);
	corec.Cmd_Register("vproxy",			CL_ViewProxy_Cmd);
	corec.Cmd_Register("releaseProxy",		CL_ReleaseProxy_Cmd);
	corec.Cmd_Register("lookproxy",			CL_LookProxy_Cmd);
	corec.Cmd_Register("giveback",			CL_GiveBack_Cmd);
	corec.Cmd_Register("getbuild",			CL_GetBuild_Cmd);
	corec.Cmd_Register("ingameMenu",		CL_IngameMenu_Cmd);
	corec.Cmd_Register("hideIngameMenu",	CL_HideIngameMenu_Cmd);
	corec.Cmd_Register("reloadGUI",			CL_ReloadGUI_Cmd);
	corec.Cmd_Register("setIEventScript",			CL_SetIEventScript_Cmd);
	corec.Cmd_Register("clearIEventAssociations",	CL_ClearIEventAssociations_Cmd);
	corec.Cmd_Register("triggerIEvent",				CL_TriggerIEvent_Cmd);
	corec.Cmd_Register("parseNick", 				CL_ParseNick_cmd);
	corec.Cmd_Register("getinfo", 					CL_GetClientInfo_cmd);
	corec.Cmd_Register("getselectioninfo", 			CL_GetSelectionInfo_cmd);

	corec.Cmd_Register("mouseFree",		CL_MouseFree_cmd);
	corec.Cmd_Register("mouseRecenter",	CL_MouseRecenter_cmd);
	corec.Cmd_Register("mouseLocate",	CL_MouseLocate_cmd);

	corec.Cmd_Register("say", CL_Say_cmd);

	corec.Cmd_Register("vc", CL_VoiceChat_Cmd);

	corec.Cmd_Register("officer_cmd", CL_SendOfficerCommand_cmd);
	corec.Cmd_Register("re", CL_SendReply_cmd);
	corec.Cmd_Register("refpwd", CL_Referee_Cmd);
	corec.Cmd_Register("ref", CL_RefereeCommand_Cmd);

	corec.Cmd_Register("ignore",		CL_Ignore_Cmd);
	corec.Cmd_Register("unignore",		CL_Unignore_Cmd);
	corec.Cmd_Register("ignorenum",		CL_IgnoreNum_Cmd);
	corec.Cmd_Register("unignorenum",	CL_UnignoreNum_Cmd);

	corec.Cmd_Register("voicechat", CL_VoiceChatActivate_Cmd);

	corec.Cmd_Register("showLoadout",	CL_ShowLoadout_Cmd);

	corec.Cmd_Register("regenerateTechTree", CL_RegenerateTechTree_Cmd);

	corec.Cvar_Register(&cl_showClanAbbrevInUserlist);
	corec.Cvar_Register(&cl_showClanIconInUserlist);
	
	corec.Cvar_Register(&cl_alwaysThirdPerson);
	corec.Cvar_Register(&cl_firstPersonYSens);
	corec.Cvar_Register(&cl_thirdPersonYSens);
	corec.Cvar_Register(&cl_lerpObjects);
	corec.Cvar_Register(&cl_mousepos_x);
	corec.Cvar_Register(&cl_mousepos_y);
	corec.Cvar_Register(&cl_cameraDistance);
	corec.Cvar_Register(&cl_cameraAngleLerp);
	corec.Cvar_Register(&cl_cameraPosLerp);
	corec.Cvar_Register(&cl_cameraDistLerp);
	corec.Cvar_Register(&cl_offsetz);
	corec.Cvar_Register(&cl_doubleClickTime);
	corec.Cvar_Register(&cl_zoomLerp);
	corec.Cvar_Register(&cl_oldCommanderLook);
	corec.Cvar_Register(&cl_showIevents);
	corec.Cvar_Register(&cl_chaseDistance);
	corec.Cvar_Register(&cl_thirdPerson);
	corec.Cvar_Register(&cl_usePlayerSoundPosition);
	corec.Cvar_Register(&cl_connectionProblemIcon_x);
	corec.Cvar_Register(&cl_connectionProblemIcon_y);
	corec.Cvar_Register(&cl_connectionProblemIcon_width);
	corec.Cvar_Register(&cl_connectionProblemIcon_height);
	corec.Cvar_Register(&cl_voiceIconTime);
	corec.Cvar_Register(&cl_freeMouse);
	
	corec.Cvar_Register(&cl_team1_score);
	corec.Cvar_Register(&cl_team2_score);
	corec.Cvar_Register(&cl_team1commandername);
	corec.Cvar_Register(&cl_team2commandername);

	corec.Cvar_Register(&cl_playVoiceChats);

	corec.Cvar_Register(&cl_svName);
	corec.Cvar_Register(&cl_svAddress);

	corec.Cvar_Register(&player_health);
	corec.Cvar_Register(&player_healthpercent);
	corec.Cvar_Register(&player_money);
	corec.Cvar_Register(&player_stamina);
	corec.Cvar_Register(&player_staminapercent);
	corec.Cvar_Register(&player_overheatpercent);
	corec.Cvar_Register(&player_ammo);
	corec.Cvar_Register(&player_maxammo);
	corec.Cvar_Register(&player_level);
	corec.Cvar_Register(&player_percentNextLevel);
	corec.Cvar_Register(&player_loyalty);
	corec.Cvar_Register(&player_item);
	corec.Cvar_Register(&player_wishitem);
	corec.Cvar_Register(&player_race);
	corec.Cvar_Register(&player_racename);
	corec.Cvar_Register(&player_team);
	corec.Cvar_Register(&world_width);
	corec.Cvar_Register(&world_height);
	corec.Cvar_Register(&player_x);
	corec.Cvar_Register(&player_y);
	corec.Cvar_Register(&player_z);
	corec.Cvar_Register(&player_secondsToRespawn);
	corec.Cvar_Register(&player_task);
	corec.Cvar_Register(&player_currentunit);
	corec.Cvar_Register(&player_focus);
	corec.Cvar_Register(&player_mana);
	corec.Cvar_Register(&player_referee);
	corec.Cvar_Register(&team1_race);
	corec.Cvar_Register(&team1_racename);
	corec.Cvar_Register(&team2_race);
	corec.Cvar_Register(&team2_racename);

	corec.Cvar_Register(&team_commandcenter_health);
	corec.Cvar_Register(&team_commandcenter_maxhealth);
	corec.Cvar_Register(&team_commandcenter_healthpercent);

	corec.Cvar_Register(&team_numworkers);
	corec.Cvar_Register(&team_maxworkers);
	corec.Cvar_Register(&team_idleworkers);
	corec.Cvar_Register(&team_commander);
	corec.Cvar_Register(&team_commandername);
	
	corec.Cvar_Register(&_toggleManageScreen);

	corec.Cvar_Register(&game_timeLimitMinutes);
	corec.Cvar_Register(&game_timeLimitSeconds);
	corec.Cvar_Register(&game_restartSeconds);
	corec.Cvar_Register(&game_serverStatus);
	corec.Cvar_Register(&game_status);

	corec.Cvar_Register(&cl_currentDirectory);

	corec.Cvar_Register(&cl_skipRender);
	corec.Cvar_Register(&cl_skipStuff);
	corec.Cvar_Register(&cl_skipObjects);
	corec.Cvar_Register(&cl_effects);

	corec.Cvar_Register(&cl_requestScores);

	corec.Cvar_Register(&cl_downloadIcons);
	corec.Cvar_Register(&cl_statsPopupDelay);

	corec.Cvar_Register(&cl_placeObjectTestRate);

	corec.Cvar_Register(&cl_inGameMenu);
	corec.Cvar_Register(&cl_showLobby);

	corec.Cvar_Register(&cl_meleeConstrain);
	corec.Cvar_Register(&cl_cmdrNoticePersistTime);

	//voicechat
	for (n=0; n<10; n++)
	{
		corec.Cvar_Register(&voice_item[n]);
	}
	
	CL_InitAsk();
	CL_VoteInit();
}


/*==========================

  CL_Init

  Called on an initial connect to a server

 ==========================*/

void	CL_Init()
{
	extern objectData_t objData[MAX_OBJECT_TYPES];
	int index;
	char motd[1024];
	char *m;

	//register all cvars
	CL_Register();
	
	memset(&cl, 0, sizeof(clientLocal_t));
	corec.Client_GameObjectPointer(&cl.objects[0].base, sizeof(cl.objects[0]), MAX_OBJECTS);
//	corec.Client_TeamDataPointer(&cl.teams[0], sizeof(cl.teams[0]), MAX_TEAMS);

	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
	{
		if (!stricmp(resourceData[index].name, ""))
			continue;

		corec.Cmd_Exec(fmt("createvar resource_%s", resourceData[index].name));		
	}

	//initialize object types
	for (index=0; index<MAX_OBJECT_TYPES; index++)
	{
		//point them all to the 0th object definition...safer than having NULL pointers
		cl.objData[index] = &objData[0];
		cl.objNames[index] = "";
	}

	corec.Cvar_SetVar(&cl_currentDirectory, core.File_GetCurrentDir());

	CL_CommanderInit();
	
	corec.Cmd_Exec("exec /ui_game.cfg");
	CL_GetWidgetPointers();


	CL_InitEvents();
	CL_InitEffects();
	CL_InitEnvironment();
	CL_InitParticles();

	TL_Pool_Init();

	TL_InitSky();

	Rend_Init();

	CL_InitAnimation();

	CL_InitResources();

	CL_RegisterTestCommands();

	Phys_Init();

	cl.guimode = true;
	cl.uistate = UISTATE_NORMAL;

	if (corec.Client_IsPlayingDemo())
	{
		corec.Cvar_SetVarValue(&cl_showLobby, 0);
	}
	else
	{
		corec.Cvar_SetVarValue(&cl_showLobby, 1);
	}
	corec.Cvar_SetVarValue(&cl_inGameMenu, 0);

	//print message of the day
	corec.Client_GetStateString(ST_MOTD, motd, sizeof(motd));

	m = ST_GetState(motd, "1");
	if (m[0])
	{
		CL_NotifyMessage("---------", "");
		CL_NotifyMessage(m, "");
		CL_NotifyMessage(ST_GetState(motd, "2"), "");
		CL_NotifyMessage(ST_GetState(motd, "3"), "");
		CL_NotifyMessage(ST_GetState(motd, "4"), "");
		CL_NotifyMessage("---------", "");
	}
}

/*==========================

  CL_InitAPIs

  Sets up calls for core engine into client code

  ==========================*/

void	CL_InitAPIs(coreAPI_client_t *core_api, clientAPI_t *client_api)
{
	memcpy(&corec, core_api, sizeof(coreAPI_client_t));

	client_api->Init = CL_Init;
	client_api->GetBuild = CL_GetBuild;
	client_api->Frame = CL_Frame;
	client_api->Restart = CL_Restart;
	client_api->Shutdown = CL_Shutdown;
	client_api->DrawForeground = CL_DrawForeground;
	client_api->ServerMessage = CL_ServerMessage;
	client_api->InputEvent = CL_InputEvent;
	client_api->BeginServerFrame = CL_BeginServerFrame;
	client_api->EndServerFrame = CL_EndServerFrame;
	client_api->ObjectUpdated = CL_ObjectUpdated;
	client_api->ObjectFreed = CL_ObjectFreed;
	client_api->ObjectNotVisible = CL_ObjectNotVisible;
	client_api->StateStringModified = CL_StateStringModified;
	client_api->ExtendedServerInfo = CL_ExtendedServerInfo;
	client_api->PrecacheResources = CL_PrecacheResources;

	TL_Register();		//fixme
}
