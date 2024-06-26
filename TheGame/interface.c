// (C) 2002 S2 Games

// interface.c

// this module is initialized at engine startup

// INT_Frame() is called all the time, regardless of the client connection state

// this module should have NO access to anything in the game code,
// but the game code can access this module


#include "interface.h"



void GUI_InitClasses();

#define INT_UISTATE_NORMAL 0x0000
#define INT_UISTATE_LOCKED 0x0001
#define INT_UISTATE_LOGIN  0x0002
#define INT_UISTATE_CDKEY  0x0004
#define INT_UISTATE_CREATEACCOUNT  0x0008

coreAPI_interface_t corei;

cvar_t	int_mousepos_x = { "int_mousepos_x", "0" };
cvar_t	int_mousepos_y = { "int_mousepos_y", "0" };
cvar_t	int_cursorsize = { "int_cursorsize", "32" };
cvar_t 	int_skipOutOfBand = { "int_skipOutOfBand", "0" };
cvar_t 	int_skip = { "int_skip", "0" };
cvar_t 	int_skipGUI = { "int_skipGUI", "0" };
cvar_t	buddy_name	=			{ "buddy_name", 				"" };
cvar_t	buddy_displayname	=	{ "buddy_displayname", 		"" };
cvar_t	buddy_gameip	=		{ "buddy_gameip", 				"" };
cvar_t	buddy_gameport	=		{ "buddy_gameport",			"11235" };

cvar_t	gamelist_forcemap = 	{ "gamelist_forcemap",  "" };
cvar_t	gamelist_forceraces = 	{ "gamelist_forceraces",  "" };
cvar_t	gamelist_forcetime = 	{ "gamelist_forcetime",  "" };
cvar_t	gamelist_forcegametype = 	{ "gamelist_forcegametype",  "" };

cvar_t	gamelist_empty =		{ "gamelist_empty", 	"1" };
cvar_t	gamelist_full =			{ "gamelist_full", 		"1" };
cvar_t	gamelist_pure =			{ "gamelist_pure", 		"both" };
cvar_t	gamelist_demo =			{ "gamelist_demo", 		"1" };
cvar_t	gamelist_players =		{ "gamelist_players", 	"1" };
cvar_t	gamelist_needscommander =	{ "gamelist_needscommander", 	"0" };
cvar_t	gamelist_race = 		{ "gamelist_race",  "Any" };
cvar_t	gamelist_gametype = 	{ "gamelist_gametype",  "" };
cvar_t	gamelist_maxping = 		{ "gamelist_maxping",  "0" };
cvar_t	gamelist_passworded = 	{ "gamelist_passworded",  "1" };
cvar_t	gamelist_teamdamage = 	{ "gamelist_teamdamage",  "1" };
cvar_t	gamelist_pspeed = 		{ "gamelist_pspeed",  "1" };

cvar_t	int_playIntro =			{ "int_playIntro",	"1" };

cvar_t	int_crosshairsize =		{ "int_crosshairsize",	"32",	CVAR_SAVECONFIG };
cvar_t	int_crosshairR =		{ "int_crosshairR",		"1",	CVAR_SAVECONFIG };
cvar_t	int_crosshairG =		{ "int_crosshairG",		"1",	CVAR_SAVECONFIG };
cvar_t	int_crosshairB =		{ "int_crosshairB",		"1",	CVAR_SAVECONFIG };
cvar_t	int_crosshairA =		{ "int_crosshairA",		"0.7",	CVAR_SAVECONFIG };
cvar_t	int_crosshairR2 =		{ "int_crosshairR2",	"1",	CVAR_SAVECONFIG };
cvar_t	int_crosshairG2 =		{ "int_crosshairG2",	"1",	CVAR_SAVECONFIG };
cvar_t	int_crosshairB2 =		{ "int_crosshairB2",	"1",	CVAR_SAVECONFIG };
cvar_t	int_crosshairA2 =		{ "int_crosshairA2",	"0.5",	CVAR_SAVECONFIG };
cvar_t	int_crosshairShader =	{ "int_crosshairShader", "/textures/cursors/xhair_circle.tga",	CVAR_SAVECONFIG };


gui_o_scrollbuffer_t *gamelist_scrollbuffer_widget = NULL;
gui_scrollbuffer_t *buddysearch_scrollbuffer_widget = NULL;
gui_scrollbuffer_t *irc_userlist = NULL;
gui_scrollbuffer_t *buddylist = NULL;
gui_scrollbuffer_t *irc_window = NULL;
gui_scrollbuffer_t *buddy_msgwindow = NULL;
gui_button_t *buddy_joingame = NULL;

static residx_t	arrowShader;
static residx_t	cursorShader = 0;
static int			cursorHotspotX = 0;
static int			cursorHotspotY = 0;

static int	int_uistate;  //used to track if the uistate is LOCKED or not, currently

#define MAX_SERVERS 1000
static server_info_t servers[MAX_SERVERS];
static int num_servers;

#define MAX_BUDDYSEARCH_USERS 50
static user_info_t buddysearch[MAX_BUDDYSEARCH_USERS];
static int num_matchingusers;

static bool authenticated = false;
static bool draw_transfer_progress = false;


extern cvar_t buddy_name, buddy_displayname, buddy_gameip, buddy_gameport;

static mousepos_t mousepos;

void	INT_IrcOutput(char *string)
{
#ifndef SAVAGE_DEMO
	GUI_ScrollBuffer_Printf(irc_window, string, 5, "");
#endif
}

void	INT_IrcMsgOutput(char *user, char *string, bool incoming)
{
	char *realnick;
	
	//not the most elegant of solutions, but it'll work for now
	if (buddy_name.string[0] == 0 && corei.Input_GetMouseMode() == MOUSE_FREE)
	{
		corei.Cvar_SetVar(&buddy_name, user);
		corei.GUI_Exec(fmt("exec %s/ui_status_buddy_msg.cfg", corei.Cvar_GetString("gui_basepath")));
	}

	realnick = strchr(user, '|');
	if (realnick)
		realnick++;
	else
		realnick = user;
	
	if (strcmp(buddy_name.string, user) == 0)
	{
		if (incoming)
			GUI_ScrollBuffer_Printf(buddy_msgwindow, fmt("<%s> %s\n", realnick, string), 5, "");
		else
			GUI_ScrollBuffer_Printf(buddy_msgwindow, fmt(">%s< %s\n", realnick, string), 5, "");
	}
	else
	{
		INT_IrcOutput(fmt("-->(private)<%s> %s\n", realnick, string));
	}
}

void	INT_StopIntro()
{
	corei.Cmd_Exec("hide intro;	select intro:movie;	param image unload");
	corei.Cmd_Exec("hide preintro; select preintro:movie; param image unload");
	corei.Cvar_SetVarValue(&int_playIntro, 0);
	corei.GUI_Exec(fmt("exec %s/ui_status_main_menu.cfg", corei.Cvar_GetString("gui_basepath")));
}

bool	INT_Locked()
{
	if (int_uistate & INT_UISTATE_LOCKED)
		return true;

	return false;	
}

bool    INT_MouseDown1()
{
	if (corei.GUI_WidgetWantsMouseClicks(MOUSE_LBUTTON, int_mousepos_x.value, int_mousepos_y.value, true))
		return false;

	if (int_uistate & INT_UISTATE_LOCKED)
		return false;	
	
	if (corei.GUI_CheckMouseAgainstUI(int_mousepos_x.value, int_mousepos_y.value))
	{			
		int_uistate |= INT_UISTATE_LOCKED;
		corei.GUI_SendMouseDown(MOUSE_LBUTTON, int_mousepos_x.value, int_mousepos_y.value);  //send mouse down to the UI			
	}

	return true;
}

bool    INT_MouseUp1()
{
	if (int_uistate & INT_UISTATE_LOCKED)
	{
		int_uistate &= ~INT_UISTATE_LOCKED;
		corei.GUI_SendMouseUp(MOUSE_LBUTTON, int_mousepos_x.value, int_mousepos_y.value);
	}

	return true;
}

//right-click
bool 	INT_MouseDown2()
{    
	if (corei.GUI_WidgetWantsMouseClicks(MOUSE_LBUTTON, int_mousepos_x.value, int_mousepos_y.value, true))
		return false;

	if (int_uistate & INT_UISTATE_LOCKED)
		return true;

	if (corei.GUI_CheckMouseAgainstUI(int_mousepos_x.value, int_mousepos_y.value))
	{			
		int_uistate |= INT_UISTATE_LOCKED;
		corei.GUI_SendMouseDown(MOUSE_RBUTTON, int_mousepos_x.value, int_mousepos_y.value);  //send mouse down to the UI			
	}

	return true;
}

bool 	INT_MouseUp2()
{
	if (int_uistate & INT_UISTATE_LOCKED)
	{
		int_uistate &= ~INT_UISTATE_LOCKED;
		corei.GUI_SendMouseUp(MOUSE_RBUTTON, int_mousepos_x.value, int_mousepos_y.value);
	}


	return true;
}

bool	INT_InputEvent(int key, char rawchar, bool down)
{

	// FRENCH
	//if (key == 0)
	//	key = rawchar;

	if (corei.Input_GetMouseMode() != MOUSE_FREE)
	{		
		return false;
	}
	
	if (int_playIntro.integer)
	{
		switch (key)
		{
			//case KEY_LBUTTON:
			case KEY_ESCAPE:
			case KEY_SPACE:
			case KEY_ENTER:
				INT_StopIntro();
				return true;
		}
	}
	else
	{
		switch (key)
		{
			case KEY_LBUTTON:
		  		if (down)
		  			return INT_MouseDown1();
				else
		  			return INT_MouseUp1();

			case KEY_RBUTTON:
		  		if (down)
		  			return INT_MouseDown2();
				else
		  			return INT_MouseUp2();
		}
	}
	return false;
}

void	INT_HideAllInterface()
{
	corei.GUI_HideAllPanels();
}

void	INT_UpdateIRCUserList()
{
	int i;
	char **users = corei.IRC_GetUserList();
	char *visibleNick;

	if (!irc_userlist)
		return;

	if (!corei.GUI_IsVisible(irc_userlist->element))
		return;
	
	GUI_ScrollBuffer_Clear(irc_userlist->element);
	
	i = 0;
	while (users[i])
	{
		visibleNick = strchr(users[i], '|');
		if (visibleNick)
		{
			visibleNick++;
			GUI_ScrollBuffer_Printf(irc_userlist, visibleNick, 0, users[i]);
		}
		i++;
	}
}

void	INT_UpdateBuddyList()
{
	int i;
	char **users = corei.IRC_GetBuddyList();
	char *visibleNick;

	if (!buddylist)
		return;
	
	GUI_ScrollBuffer_Clear(buddylist->element);
	
	i = 0;
	while (users[i])
	{
		visibleNick = strchr(users[i], '|');
		if (visibleNick)
		{
			visibleNick++;
			GUI_ScrollBuffer_Printf(buddylist, visibleNick, 0, users[i]);
		}
		i++;
	}

	if (buddy_joingame
	    && strlen(buddy_gameip.string))
	{
		corei.GUI_Show(buddy_joingame->element);
	}
    else
	{
		corei.GUI_Hide(buddy_joingame->element);
    }
}

bool	INT_FilterGame(char *ip, int port, int ping, char *coreInfo, char *gameInfo)
{
	if (!gamelist_empty.integer && atoi(ST_GetState(coreInfo, "cnum")) <= 0)
			return false;
	if (!gamelist_full.integer && atoi(ST_GetState(coreInfo, "cnum")) >=  atoi(ST_GetState(coreInfo, "cmax")))
			return false;
	//if (gamelist_players.integer && atoi(ST_GetState(coreInfo, "cnum")) <= 0)
	//		return false;
	if (!gamelist_players.integer && atoi(ST_GetState(coreInfo, "cnum")) > 0)
			return false;
	if (gamelist_needscommander.integer && atoi(ST_GetState(gameInfo, "needcmdr")) <= 0)
			return false;
	if (stricmp(gamelist_race.string, "human v human") == 0 && 
			(stricmp(ST_GetState(gameInfo, "race1"), "human") != 0
			|| stricmp(ST_GetState(gameInfo, "race2"), "human") != 0))
			return false;
	if (stricmp(gamelist_race.string, "beast v beast") == 0 && 
			(stricmp(ST_GetState(gameInfo, "race1"), "beast") != 0
			|| stricmp(ST_GetState(gameInfo, "race2"), "beast") != 0))
			return false;
	if (stricmp(gamelist_race.string, "human v beast") == 0 && 
			!((stricmp(ST_GetState(gameInfo, "race1"), "human") == 0
				&& stricmp(ST_GetState(gameInfo, "race2"), "beast") == 0)
				|| (stricmp(ST_GetState(gameInfo, "race1"), "beast") == 0
				&& stricmp(ST_GetState(gameInfo, "race2"), "human") == 0)))
			return false;
	if (gamelist_gametype.string[0] && strcmp(ST_GetState(gameInfo, "gametype"), gamelist_gametype.string) != 0 )
			return false;
	if (gamelist_maxping.integer && ping > gamelist_maxping.integer)
			return false;
	if (!gamelist_passworded.integer && atoi(ST_GetState(gameInfo, "pass")) > 0)
			return false;
	if (!gamelist_teamdamage.integer && atoi(ST_GetState(gameInfo, "td")) > 0)
			return false;
	if (!gamelist_pspeed.integer && atoi(ST_GetState(gameInfo, "pspd")) > 0)
			return false;
	if (stricmp(gamelist_pure.string, "pure only") == 0 && atoi(ST_GetState(gameInfo, "pure")) == 0)
			return false;
	if (stricmp(gamelist_pure.string, "impure only") == 0 && atoi(ST_GetState(gameInfo, "pure")) > 0)
			return false;
	if (!gamelist_demo.integer && atoi(ST_GetState(gameInfo, "demo")) > 0)
			return false;
#ifdef SAVAGE_DEMO
	if (atoi(ST_GetState(gameInfo, "demo")) == 0)
		return false;
#endif	//SAVAGE_DEMO
	return true;
}

#define NUM_COLUMNS 7
#define MAX_STR_LENGTH 64
void	INT_UpdateGameList()
{
	int i, num_new_servers;
	char gamestring[256];
	char values[NUM_COLUMNS][MAX_STR_LENGTH];
	char *valptrs[NUM_COLUMNS];
	char *racestring;
	char *race1, *race2;

	for (i=0; i<NUM_COLUMNS; i++)
		valptrs[i] = values[i];

	num_new_servers = corei.MasterServer_GetGameList_Frame(servers, MAX_SERVERS, &num_servers);
	num_new_servers += gamelist_empty.modified + gamelist_full.modified + gamelist_players.modified 
			+ gamelist_needscommander.modified + gamelist_race.modified 
			+ gamelist_gametype.modified + gamelist_maxping.modified + gamelist_teamdamage.modified
			+ gamelist_pspeed.modified + gamelist_passworded.modified + gamelist_pure.modified 
			+ gamelist_demo.modified;
	gamelist_empty.modified = false;
	gamelist_full.modified = false;
	gamelist_players.modified = false;
	gamelist_needscommander.modified = false;
	gamelist_race.modified = false;
	gamelist_gametype.modified = false;
	gamelist_maxping.modified = false;
	gamelist_passworded.modified = false;
	gamelist_teamdamage.modified = false;
	gamelist_pspeed.modified = false;
	gamelist_pure.modified = false;
	gamelist_demo.modified = false;

	if (num_new_servers > 0)
	{
		corei.Cmd_Exec("o_scrollbuffer clear connect_panel:connectmenu");
		for (i = 0; i < num_servers; i++)
		{
			if (gamelist_scrollbuffer_widget && servers[i].coreInfo[0])
			{
				int gametime = atoi(ST_GetState(servers[i].coreInfo, "time"));

				if (!INT_FilterGame(servers[i].ip_address, servers[i].port, servers[i].ping, servers[i].coreInfo, servers[i].gameInfo))
					continue;

				if (gamelist_forcetime.string[0])
					gametime = gamelist_forcetime.integer;

				racestring = "";
				if (gamelist_forceraces.string[0])
					racestring = gamelist_forceraces.string;
				else
				{
					race1 = ST_GetState(servers[i].gameInfo, "race1");
					race2 = ST_GetState(servers[i].gameInfo, "race2");
					if (race1[0] && race2[0])
						racestring = fmt("%c v %c", toupper(race1[0]), toupper(race2[0]));
				}

				BPrintf(gamestring, 256, "%-40s^col^%-20s^col^%s/%s^col^%s^col^%02i:%02i^col^%s^col^%i",
							ST_GetState(servers[i].coreInfo, "name"),							
							gamelist_forcemap.string[0] ? gamelist_forcemap.string : ST_GetState(servers[i].coreInfo, "world"),
							ST_GetState(servers[i].coreInfo, "cnum"),
							ST_GetState(servers[i].coreInfo, "cmax"),
							racestring,
							gametime / 3600,
							(gametime % 3600) / 60,
							gamelist_forcegametype.string[0] ? gamelist_forcegametype.string : ST_GetState(servers[i].gameInfo, "gametype"),
							servers[i].ping);

				strcpy(values[0], ST_GetState(servers[i].coreInfo, "name"));
				strcpy(values[1], gamelist_forcemap.string[0] ? gamelist_forcemap.string : ST_GetState(servers[i].coreInfo, "world"));
				strcpy(values[2], ST_GetState(servers[i].coreInfo, "cnum")),
				strcpy(values[3], ST_GetState(servers[i].gameInfo, "race1")),
				strcpy(values[4], fmt("%i", gametime));
				strcpy(values[5], gamelist_forcegametype.string[0] ? gamelist_forcegametype.string : ST_GetState(servers[i].gameInfo, "gametype"));
				strcpy(values[6], fmt("%i", servers[i].ping));

				GUI_OrderedScrollBuffer_Printf(gamelist_scrollbuffer_widget, gamestring,
						fmt("%s:%i", servers[i].ip_address, servers[i].port), NUM_COLUMNS, valptrs);
			}
		}
	}
}

/*==========================

  INT_UpdateUserSearchList

 ==========================*/

void	INT_UpdateUserSearchList()
{
	int i, num_new_users;
	char userstring[128];

	num_new_users = corei.MasterServer_GetUserList_Frame(buddysearch, MAX_BUDDYSEARCH_USERS, &num_matchingusers);

	if (num_new_users > 0)
	{
		corei.Cmd_Exec("scrollbuffer clear buddysearch_results");
		for (i = 0; i < num_matchingusers; i++)
		{
			if (buddysearch_scrollbuffer_widget)
			{
				BPrintf(userstring, 128,
						fmt("UID %i, %s",
							buddysearch[i].user_id,
							buddysearch[i].username));
				GUI_ScrollBuffer_Printf(buddysearch_scrollbuffer_widget, userstring, 0,
						fmt("_%i|%s", buddysearch[i].user_id, buddysearch[i].username));
			}
		}
	}
}

/*==========================
 *
 *   INT_UpdateUserInfo
 *
 *    ==========================*/
static int  uinfo_field = 0;

static void _printInfo(const char *key, const char *value)
{
	gui_element_t *obj;
	
	corei.Console_Printf("key: %s, value: %s\n", key, value);   
	
	if (strcmp(key, "username") == 0 || strcmp(key, "uid") == 0
		|| strcmp(key, "clan") == 0 || strcmp(key, "clabbrev") == 0
		|| strcmp(key, "guid") == 0)
		return;
	
	obj = corei.GUI_GetObject(fmt("userinfo_panel:uinfo%i_label", uinfo_field));
	if (obj)
	{
		if (strcmp(key, "Name") == 0)
			GUI_Label_ShowText(obj, "Plays as:");
		else
			GUI_Label_ShowText(obj, fmt("%s:", key));
		corei.GUI_Show(obj);
	}
	
	obj = corei.GUI_GetObject(fmt("userinfo_panel:uinfo%i_text", uinfo_field));
	if (obj)
	{
		GUI_Label_ShowText(obj, (char *)value);
		corei.GUI_Show(obj);
	}
	
	uinfo_field++;
}
	
void    INT_UpdateUserInfo()
{
	char userinfo[4096];
	int ret;
	gui_element_t *obj;
	
	ret = corei.MasterServer_GetUserInfo_Frame(userinfo, 4096);
	
	if (ret > 0)
	{
		corei.Console_Printf("User info for user_id %s:\n", ST_GetState(userinfo, "uid"));

		corei.Cvar_Set("userinfo_buddyname", fmt("%s|%s", ST_GetState(userinfo, "uid"), ST_GetState(userinfo, "username")));
	
		obj = corei.GUI_GetObject("userinfo_panel:uinfo0_label");
		if (obj)
			GUI_Label_ShowText(obj, "Login:");
		obj = corei.GUI_GetObject("userinfo_panel:uinfo0_text");
		if (obj)
			GUI_Label_ShowText(obj, fmt("%s (UID %s) (GUID %s)", ST_GetState(userinfo, "username"), ST_GetState(userinfo, "uid"), ST_GetState(userinfo, "guid")));
	
		uinfo_field = 1;
	
		//check for empty string
		if (ST_GetState(userinfo, "clabbrev")[0])
		{
			obj = corei.GUI_GetObject("userinfo_panel:uinfo1_label");
			if (obj)
			{
				corei.GUI_Show(obj);
				GUI_Label_ShowText(obj, "Main Clan:");
			}
			obj = corei.GUI_GetObject("userinfo_panel:uinfo1_text");
			if (obj)
			{
				corei.GUI_Show(obj);
				GUI_Label_ShowText(obj, fmt("^clan %s^%s", ST_GetState(userinfo, "clan"), ST_GetState(userinfo, "clabbrev")));
			}
	
			uinfo_field = 2;
		}
		ST_ForeachState(userinfo, _printInfo);
	
		corei.GUI_Exec("show userinfo_panel");
	
		while ((obj = corei.GUI_GetObject(fmt("userinfo_panel:uinfo%i_label", uinfo_field))))
		{
			corei.GUI_Hide(obj);
	
			obj = corei.GUI_GetObject(fmt("userinfo_panel:uinfo%i_text", uinfo_field));
			corei.GUI_Hide(obj);
	
			uinfo_field++;
		}
	}
}

void	INT_LoadingStart()
{
}


/*==========================

  INT_LoadingFrame

  Called while connecting to a server and loading resources

 ==========================*/

void	INT_LoadingFrame(const char *currentResource)
{
	int ypos = 150;
	int offset = -26;
	int cstate;
	const char *resource;	
	residx_t mapoverlay;
	residx_t minimap;

	corei.Draw_SetColor(black);
	corei.Draw_Quad2d(0,0,corei.Vid_GetScreenW(),corei.Vid_GetScreenH(),0,0,1,1,corei.GetWhiteShader());	

	corei.Draw_SetColor(white);

	//if (!res.minimap)

	cstate = corei.Client_GetConnectionState();

	if (draw_transfer_progress)
	{
		resource = fmt("%s (%s/%s at %s, %s)", 
						corei.Cvar_GetString("transfer_resource"),
						corei.Cvar_GetString("transfer_bytesTransferred"),
						corei.Cvar_GetString("transfer_size"),
						corei.Cvar_GetString("transfer_speed"),
						corei.Cvar_GetString("transfer_remaining"));
	}
	else
		resource = currentResource;

	if (cstate >= CCS_LOADING)
	{
		minimap = corei.Res_LoadShaderEx(corei.Cvar_GetString("world_overhead"), SHD_FULL_QUALITY);		

		//draw minimap
		corei.GUI_Quad2d_S(256,75+offset,512,512,minimap);				
	}

	mapoverlay = corei.Res_LoadShaderEx("/textures/loading/nfm_mapoverlay.tga", SHD_FULL_QUALITY);
	corei.GUI_Quad2d_S(256,75+offset,512,512,mapoverlay);

	corei.Draw_SetColor(vec4(0.45,0.28,0.08,1));

	if (cstate >= CCS_LOADING)
	{	
		corei.GUI_DrawString(512 - corei.GUI_StringWidth(corei.World_GetName(), 32, 32, 1, 512, corei.GetNicefontShader()) / 2,445+offset,corei.World_GetName(),32,32,1,512,corei.GetNicefontShader());
	}

	corei.Draw_SetColor(vec4(0.45/2,0.28/2,0.08/2,1));
	corei.GUI_DrawString(256+120,480+offset, resource, 12, 12,1,380,corei.GetNicefontShader());

	//for (n=CCS_AUTHENTICATING; n<=cstate; n++)
	{
		int i;
		char *string;
		int	iconStates[4] = { 0,0,0,0 };
		ivec2_t iconPositions[4] =
		{
			{ 250, 572+offset },
			{ 250 + ((96+47)*1), 572+offset },
			{ 250 + ((96+47)*2), 572+offset },
			{ 250 + ((96+47)*3), 572+offset }
		};
		char *iconShaders[4] =
		{
			"/textures/loading/nfm_auth",
			"/textures/loading/nfm_connect",
			"/textures/loading/nfm_waiting",
			"/textures/loading/nfm_loading"
		};
		ivec2_t starPositions[4] =
		{
			{ 202, 587+offset },
			{ 202 + ((192-48)*1), 587+offset },
			{ 202 + ((192-48)*2), 587+offset },
			{ 202 + ((192-48)*3), 587+offset }			
		};

		if (cstate <= CCS_AUTHENTICATING)
		{
			string = "Authenticating";
			iconStates[0] = 1;
		}
		else if (cstate <= CCS_CONNECTING)
		{
			string = "Establishing Connection";
			iconStates[0] = 2;
			iconStates[1] = 1;
		}
		else if (cstate <= CCS_AWAITING_STATE_STRINGS)
		{
			string = "Awaiting State Strings";
			iconStates[0] = 2;
			iconStates[1] = 2;
			iconStates[2] = 1;
		}
		else if (cstate <= CCS_LOADING)
		{
			string = "Loading";
			iconStates[0] = 2;
			iconStates[1] = 2;
			iconStates[2] = 2;
			iconStates[3] = 1;
		}
		else
		{
			iconStates[0] = 2;
			iconStates[1] = 2;
			iconStates[2] = 2;
			iconStates[3] = 2;

			if (cstate == CCS_AWAITING_FIRST_FRAME)
			{
				string = "Awaiting First Frame";
			}
			else
			{
				string = "";
			}
		}

		corei.Draw_SetColor(white);

		corei.GUI_DrawString(512 - corei.GUI_StringWidth(string, 32, 32, 1, 512, corei.GetNicefontShader()) / 2,680,string,32,32,1,512,corei.GetNicefontShader());

		corei.GUI_DrawString(332,724,"Connecting to server.  Press Escape to cancel.",16,16,1,640,corei.GetNicefontShader());	

		for (i=0; i<4; i++)
		{
			if (!iconStates[i])
				corei.Draw_SetColor(vec4(0.5,0.5,0.5,1));
			else
				corei.Draw_SetColor(vec4(1,1,1,1));

			corei.GUI_Quad2d_S(iconPositions[i][0], iconPositions[i][1], 96, 96, 
					corei.Res_LoadShaderEx(fmt("%s%s.tga", iconShaders[i], iconStates[i] == 2 ? "_ok" : ""), SHD_FULL_QUALITY | SHD_NO_MIPMAPS));

			corei.Draw_SetColor(white);
			if (iconStates[i] == 2)
			{
				corei.GUI_Quad2d_S(starPositions[i][0], starPositions[i][1], 192, 64,
					corei.Res_LoadShaderEx(fmt("/textures/loading/1_nl_loadingstar.tga"), SHD_FULL_QUALITY | SHD_NO_MIPMAPS));
			}

		}
				
	}
}


void	INT_ShowProgressMeter(const char *currentResource, float progress, int totalSize, int currentAmount, int totalTime)
{
	static int lastCalculation = 0;
	int now;
	float denom;
	
	if (!currentResource || totalTime <= 0)
	{
		draw_transfer_progress = false;
		return;
	}

	now = corei.Milliseconds();
	if (now - lastCalculation < 1000)
		return;

	lastCalculation = now;

	corei.Console_DPrintf("Downloading %s, %i done (%i out of %i bytes, at %fk/s)\n", currentResource, (int)(progress * 100), currentAmount, totalSize, (float)(currentAmount)/totalTime);

	draw_transfer_progress = true;
	corei.Cvar_SetValue("transfer_progressPercent", progress * 100);
	corei.Cvar_SetValue("transfer_size", totalSize);
	corei.Cvar_SetValue("transfer_bytesTransferred", currentAmount);

	if (currentAmount > 0)
	{
		corei.Cvar_Set("transfer_speed", fmt("%5i k/s", (int)((float)currentAmount/totalTime)));
		denom = (((float)currentAmount/totalTime) * 1000); //since time is in millis, we need to multiply the denominator by 1k
		if (denom)
			corei.Cvar_Set("transfer_remaining", fmt("%02i:%02i remaining", (int)((((float)totalSize - currentAmount) / denom)/60), (int)((((float)totalSize - currentAmount) / denom)) % 60));
	}
	else
	{
		corei.Cvar_Set("transfer_speed", "0 k/s");
		corei.Cvar_Set("transfer_remaining", "");
	}

	corei.Cvar_Set("transfer_progress", fmt("%10i / %10i bytes", currentAmount, totalSize));

	corei.Cvar_Set("transfer_resource", fmt("downloading %s", currentResource));
}

/*==========================

  INT_Frame

  called all the time every frame, whether we're connected to a game or not

 ==========================*/

void	INT_Frame()
{
	static bool menuhidden = true;

	if (int_skip.integer)
		return;

	if (int_playIntro.integer)
	{
		menuhidden = true;
	}
#ifndef SAVAGE_DEMO
	else if (int_uistate & INT_UISTATE_CDKEY)
	{
		corei.GUI_Exec("show cdkey_panel; hide login_panel; hide create_user_panel; hide menu_panel");
		menuhidden = true;
	}
	else if (int_uistate & INT_UISTATE_LOGIN)
	{
		corei.GUI_Exec("show login_panel; hide create_user_panel; hide cdkey_panel; hide menu_panel");
		menuhidden = true;
	}
	else if (int_uistate & INT_UISTATE_CREATEACCOUNT)
	{
		corei.GUI_Exec("show create_user_panel; hide login_panel; hide cdkey_panel; hide menu_panel");
		menuhidden = true;
	}
#endif //SAVAGE_DEMO
	else if (menuhidden)
	{
		menuhidden = false;

		if (irc_window)
		{
			corei.IRC_OutputCallback(INT_IrcOutput);
		}
		if (buddy_msgwindow)
		{
			corei.IRC_OutputMsgCallback(INT_IrcMsgOutput);
		}
	}

	if (!int_skipOutOfBand.integer)
	{
		INT_UpdateGameList();
		INT_UpdateUserSearchList();
		INT_UpdateUserInfo();
		INT_UpdateIRCUserList();
		INT_UpdateBuddyList();
	}

	if (corei.Input_GetMouseMode() != MOUSE_FREE)
	{
		int_uistate = INT_UISTATE_NORMAL;
		return;
	}
		
	//update the mouse pos
	corei.Input_GetMousePos(&mousepos);
	corei.Cvar_SetVarValue(&int_mousepos_x, mousepos.x);
	corei.Cvar_SetVarValue(&int_mousepos_y, mousepos.y);

	if (!int_skipGUI.integer)
	{
		//let the gui figure its stuff out
		if (corei.GUI_CheckMouseAgainstUI(mousepos.x, mousepos.y))
		{
			INT_SetCursorShader(arrowShader, 0, 0);
		}
	}
}




/*==========================

  INT_Restart

  called when we disconnect from a game,
  when we connect to a new game,
  and on engine startup

 ==========================*/

void	INT_Restart()
{	
	bool initializing = corei.Cvar_GetValue("host_initializing");

	if (!initializing)
		INT_StopIntro();

	if (int_playIntro.integer && initializing)
		corei.GUI_Exec("exec /intro.cfg");
	else
		corei.GUI_Exec(fmt("exec %s/ui_status_main_menu.cfg", corei.Cvar_GetString("gui_basepath")));
	corei.Input_SetMouseMode(MOUSE_FREE);

	//get pointers to the objects defined in the script
	INT_InitWidgetPointers();
}



/*==========================

  INT_SetCursorShader

 ==========================*/

void	INT_SetCursorShader(residx_t shader, float hotspotx, float hotspoty)
{
	cursorShader = shader;
	cursorHotspotX = hotspotx;
	cursorHotspotY = hotspoty;
}



/*==========================

  INT_DrawCursor

 ==========================*/

void	INT_DrawCursor()
{
	if (corei.Input_GetMouseMode() == MOUSE_RECENTER || corei.Input_GetMouseMode() == MOUSE_FREE_INPUT)
		return;

	if (int_playIntro.integer)
		return;

    corei.Draw_SetColor(white);

	corei.Draw_Quad2d(int_mousepos_x.value - cursorHotspotX * int_cursorsize.value, int_mousepos_y.value - cursorHotspotY * int_cursorsize.value, int_cursorsize.integer, int_cursorsize.integer, 0, 0, 1, 1, cursorShader);
}

/*==========================

  Int_DrawCrosshair

  separated out into its own function in case we want to draw it in menus

 ==========================*/

void	INT_DrawCrosshair(int x, int y, int w, int h, float focus, bool melee, bool gui)
{
	int bgw,bgh;
	int hw = w/2;
	int hh = h/2;
	int multw = 5;
	int multh = 5;
	int scale = melee ? 1.0 : focus;
	int wadd = w*(1-focus)*multw;
	int hadd = h*(1-focus)*multw;

	corei.GUI_ScaleToScreen(&multw,&multh);

	//bg

	//if (focus < 1.0 || melee)
	{
		bgw = w + wadd;
		bgh = h + hadd;

		corei.Draw_SetColor(vec4(int_crosshairR2.value, int_crosshairG2.value, int_crosshairB2.value, int_crosshairA2.value));
		if (gui)
			corei.GUI_Quad2d_S(x-bgw/2 + hw, y-bgh/2 + hh, bgw, bgh, corei.Res_LoadShader(int_crosshairShader.string));
		else
			corei.Draw_Quad2d(x-bgw/2 + hw, y-bgh/2 + hh, bgw, bgh, 0, 0, 1, 1, corei.Res_LoadShader(int_crosshairShader.string));

		corei.Draw_SetColor(vec4(int_crosshairR.value, int_crosshairG.value, int_crosshairB.value, int_crosshairA.value));
	}

	if (!melee)
	{
		x += 6;
		y += 6;
		hw -= 6;
		hh -= 6;

		if (gui)
		{
			//top
			corei.GUI_Quad2d_S(x + hw, y - hadd/2, 1, hh-2, corei.GetWhiteShader());
			//bottom
			corei.GUI_Quad2d_S(x + hw, y + (hh+2) + hadd/2, 1, hh-1, corei.GetWhiteShader());
			//left
			corei.GUI_Quad2d_S(x - wadd/2, y + (hh), hw-1, 1, corei.GetWhiteShader());
			//right
			corei.GUI_Quad2d_S(x + (hw+2) + wadd/2, y + (hh), hw-1, 1, corei.GetWhiteShader());
		}
		else
		{
			//top
			corei.Draw_Quad2d(x + hw, y - hadd/2, 1, hh-2, 0, 0, 1, 1, corei.GetWhiteShader());
			//bottom
			corei.Draw_Quad2d(x + hw, y + (hh+2) + hadd/2, 1, hh-1, 0, 0, 1, 1, corei.GetWhiteShader());
			//left
			corei.Draw_Quad2d(x - wadd/2, y + (hh), hw-1, 1, 0, 0, 1, 1, corei.GetWhiteShader());
			//right
			corei.Draw_Quad2d(x + (hw+2) + wadd/2, y + (hh), hw-1, 1, 0, 0, 1, 1, corei.GetWhiteShader());
		}
	}
}

void	INT_FindBuddies(char *name)
{
	memset(buddysearch, 0, sizeof(user_info_t) * MAX_BUDDYSEARCH_USERS);
	//get the list of users
	num_matchingusers = corei.MasterServer_FindUsers(name);
}

void	INT_FindBuddies_Cmd(int argc, char *argv[])
{
	if (argc && argv[0][0])
	{
		INT_FindBuddies(argv[0]);
	}
	else
	{
		corei.Console_Errorf("You must specify a name to search for\n");
	}
}

void	INT_GetGameList()
{

	memset(servers, 0, sizeof(server_info_t) * MAX_SERVERS);
	//get the gamelist
	num_servers = corei.MasterServer_GetGameList();

	corei.Input_SetMouseMode(MOUSE_FREE);
}

void	INT_GetGameList_Cmd(int argc, char *argv[])
{
	INT_GetGameList();
}

void	INT_IsBuddy_Cmd(int argc, char *argv[])
{
	char **users = corei.IRC_GetBuddyList();	
	char *name;
	int i = 0;

	if (argc < 2)
		return;

	name = argv[0];
	if (name[0] == '@' || name[0] == '+')
		name++;
	
	while (users[i])
	{
		if (strcmp(name, users[i]) == 0)
		{
			corei.Cvar_SetValue(argv[1], 1);
			return;
		}
		i++;
	}	

	corei.Cvar_SetValue(argv[1], 0);
}

void	INT_ErrorDialog(const char *text)
{
	corei.Cmd_Exec(fmt("textbuffer clear error_explanation; show error_panel; textbuffer print error_explanation \"%s\"", text));
}


void	*INT_GetWidget(char *itemname, char *classname)
{
	void *ret = corei.GUI_GetClass(itemname, classname);

	if (!ret)
		core.Game_Error(fmt("Main GUI Error: Couldn't find %s %s\n", classname, itemname));

	return ret;
}

void	INT_InitWidgetPointers()
{
	gamelist_scrollbuffer_widget = INT_GetWidget("connect_panel:connectmenu", "o_scrollbuffer");
#ifndef SAVAGE_DEMO
	buddysearch_scrollbuffer_widget = INT_GetWidget("addbuddy_panel:buddysearch_results", "scrollbuffer");
	irc_window = INT_GetWidget("irc_window", "scrollbuffer");
	buddy_msgwindow = INT_GetWidget("buddy_messages", "scrollbuffer");
	buddy_joingame = INT_GetWidget("buddy_joingame", "button");
	irc_userlist = INT_GetWidget("irc_userlist", "scrollbuffer");
	buddylist = INT_GetWidget("buddylist", "scrollbuffer");
#endif
}

void	INT_UIState_Cmd(int argc, char *argv[])
{
	bool locked;
	if (!argc)
	{
		corei.Console_DPrintf("You must specifiy a UI state (login, cdkey, createuser, normal)\n");
		return;
	}
	locked = int_uistate & INT_UISTATE_LOCKED;

#ifndef SAVAGE_DEMO
	if (strcmp(argv[0], "login") == 0)
		int_uistate = INT_UISTATE_LOGIN;
	else if (strcmp(argv[0], "cdkey") == 0)
		int_uistate = INT_UISTATE_CDKEY;
	else if (strcmp(argv[0], "createuser") == 0)
		int_uistate = INT_UISTATE_CREATEACCOUNT;
	else
#endif //SAVAGE_DEMO
	if (strcmp(argv[0], "normal") == 0)
		int_uistate = INT_UISTATE_NORMAL;

	if (locked)
		int_uistate |= INT_UISTATE_LOCKED;
}

/*	Called by the core engine during engine startup.  Do any
	initial setting of variables here, but remember this
	function is only called ONCE.  For resetting variables
	at the start of a game, use INT_Restart()
*/
void	INT_Init()
{
	bool autologin = corei.Cvar_GetValue("autologin");
	char *cdkey = corei.Cvar_GetString("cdkey");
	char *username = corei.Cvar_GetString("username");
	char *password = corei.Cvar_GetString("password");
	char *user_id = corei.Cvar_GetString("user_id");

	int_uistate = INT_UISTATE_NORMAL;	
		
#ifndef SAVAGE_DEMO
	if (!cdkey[0]) // || strcmp(cdkey->string, "00000000000000000000") == 0)
	{		
		corei.GUI_Exec("textbox activate cdkey_panel:cdkey_box");
		int_uistate = INT_UISTATE_CDKEY;
	}
	/*else if (!username[0] || !password[0])
	{
		corei.GUI_Exec("textbox activate login_panel:username_box");
		int_uistate |= INT_UISTATE_LOGIN;
	}*/
	else if (!authenticated)
	{
		if (!autologin)
			int_uistate |= INT_UISTATE_LOGIN;
		else
			corei.Cmd_Exec(fmt("irc_nick %s", username));
	}
#endif //SAVAGE_DEMO


	//initialize the user interface widgets
	GUI_InitClasses();

	corei.Cvar_Register(&int_mousepos_x);
	corei.Cvar_Register(&int_mousepos_y);
	corei.Cvar_Register(&int_cursorsize);
	corei.Cvar_Register(&int_skipOutOfBand);
	corei.Cvar_Register(&int_skip);
	corei.Cvar_Register(&int_skipGUI);
	corei.Cvar_Register(&int_playIntro);
	corei.Cvar_Register(&int_crosshairsize);
	corei.Cvar_Register(&int_crosshairR);
	corei.Cvar_Register(&int_crosshairG);
	corei.Cvar_Register(&int_crosshairB);
	corei.Cvar_Register(&int_crosshairA);
	corei.Cvar_Register(&int_crosshairR2);
	corei.Cvar_Register(&int_crosshairG2);
	corei.Cvar_Register(&int_crosshairB2);
	corei.Cvar_Register(&int_crosshairA2);
	corei.Cvar_Register(&int_crosshairShader);
	corei.Cvar_Register(&buddy_name);
	corei.Cvar_Register(&buddy_displayname);
	corei.Cvar_Register(&buddy_gameip);
	corei.Cvar_Register(&buddy_gameport);
	corei.Cvar_Register(&gamelist_empty);
	corei.Cvar_Register(&gamelist_full);
	corei.Cvar_Register(&gamelist_players);
	corei.Cvar_Register(&gamelist_needscommander);
	corei.Cvar_Register(&gamelist_forcemap);
	corei.Cvar_Register(&gamelist_forceraces);
	corei.Cvar_Register(&gamelist_forcetime);
	corei.Cvar_Register(&gamelist_forcegametype);
	corei.Cvar_Register(&gamelist_race);
	corei.Cvar_Register(&gamelist_gametype);
	corei.Cvar_Register(&gamelist_maxping);
	corei.Cvar_Register(&gamelist_passworded);
	corei.Cvar_Register(&gamelist_teamdamage);
	corei.Cvar_Register(&gamelist_pspeed);
	corei.Cvar_Register(&gamelist_pure);
	corei.Cvar_Register(&gamelist_demo);
	
	corei.Cmd_Register("gamelist", INT_GetGameList_Cmd);
	corei.Cmd_Register("findbuddies", INT_FindBuddies_Cmd);
	corei.Cmd_Register("isBuddy", INT_IsBuddy_Cmd);
	corei.Cmd_Register("uiState", INT_UIState_Cmd);
	corei.Input_SetMouseMode(MOUSE_FREE);

	//run the UI startup script
	corei.Cmd_Exec("exec /ui_main.cfg");


	//set the default interface cursor
	arrowShader = corei.Res_LoadShader("textures/cursors/arrow.tga");
	INT_SetCursorShader(arrowShader, 0, 0);
}

void	INT_Authenticated_CDKey(bool status)
{
	file_t *cdkey_file;

	if (status == true)
	{
		//corei.GUI_Exec("textbox activate create_user_panel:username_box");
		//int_uistate = INT_UISTATE_LOGIN;
		// XXX
		corei.GUI_Exec("hide create_user_panel; hide cdkey_panel; show menu_panel");
		int_uistate = INT_UISTATE_NORMAL;

	#ifdef unix
		cdkey_file = core.File_OpenAbsolute(fmt("%scdkey", core.Cvar_GetString("homedir")), "w");
	#else
		cdkey_file = core.File_Open("/cdkey", "w");
	#endif
		if (!cdkey_file)
		{		
			corei.Console_Printf("warning: can't open cdkey.cfg file!\n");
			return;
		}
		core.File_Printf(cdkey_file, fmt("// This file was generated by Savage.  DO NOT GIVE THIS FILE TO ANYONE!  S2 Games will not ask you to send this file to them\n"));
		core.File_Printf(cdkey_file, fmt("set username %s\n", corei.Cvar_GetString("username")));
		core.File_Printf(cdkey_file, fmt("set password %s\n", corei.Cvar_GetString("password")));
		core.File_Printf(cdkey_file, fmt("set cdkey %s\n", corei.Cvar_GetString("cdkey")));
		core.File_Printf(cdkey_file, fmt("set autologin %s\n", corei.Cvar_GetString("autologin")));
		if (corei.Cvar_GetInteger("user_id") > 0)
			core.File_Printf(cdkey_file, fmt("set user_id %s\n", corei.Cvar_GetString("user_id")));
		core.File_Close(cdkey_file);
	}
	//else
	//	INT_ErrorDialog("Invalid CD-Key.  Please look for any typos and try again.\n");
}

void	INT_Authenticated_AccountCreated(bool status)
{
	if (status == true)
	{
		INT_ErrorDialog("Account Created Successfully.\n");
		corei.GUI_Exec("hide create_user_panel; show menu_panel");
		int_uistate = INT_UISTATE_NORMAL;
	}
	else
	{
		corei.Console_DPrintf("Failure creating account\n");
		int_uistate = INT_UISTATE_CREATEACCOUNT;
	}
}

void	INT_Authenticated(bool status)
{
	file_t *cdkey_file;
	
	authenticated = status;

	if (!status)
	{
		//authentication failed
		if (!(int_uistate & INT_UISTATE_CDKEY))
			int_uistate |= INT_UISTATE_LOGIN;
		return;
	}
	
	//else it was successful
	//if (!(int_uistate & INT_UISTATE_LOGIN))
	//	return;

	authenticated = true;

	if (int_uistate & INT_UISTATE_LOGIN)
	{
		//the user has authenticated successfully, remove the login box
		int_uistate &= ~INT_UISTATE_LOGIN;
		corei.GUI_Exec("hide login_panel");

	#ifdef unix
		cdkey_file = core.File_OpenAbsolute(fmt("%scdkey", core.Cvar_GetString("homedir")), "w");
	#else
		cdkey_file = core.File_Open("/cdkey", "w");
	#endif
		if (!cdkey_file)
		{		
			corei.Console_Printf("warning: can't open cdkey.cfg file!\n");
			return;
		}
		core.File_Printf(cdkey_file, fmt("// This file was generated by Savage.  DO NOT GIVE THIS FILE TO ANYONE!  S2 Games will not ask you to send this file to them\n"));
		core.File_Printf(cdkey_file, fmt("set username %s\n", corei.Cvar_GetString("username")));
		core.File_Printf(cdkey_file, fmt("set password %s\n", corei.Cvar_GetString("password")));
		core.File_Printf(cdkey_file, fmt("set cdkey %s\n", corei.Cvar_GetString("cdkey")));
		core.File_Printf(cdkey_file, fmt("set autologin %s\n", corei.Cvar_GetString("autologin")));
		if (corei.Cvar_GetInteger("user_id") > 0)
			core.File_Printf(cdkey_file, fmt("set user_id %s\n", corei.Cvar_GetString("user_id")));
		core.File_Close(cdkey_file);

		INT_Restart();
	}
	
	corei.Cmd_Exec(fmt("irc_nick %s", corei.Cvar_GetString("username")));
}

void	INT_InitAPIs(coreAPI_interface_t *core_int_api, interfaceAPI_t *game_interface_api)
{
	memcpy(&corei, core_int_api, sizeof(coreAPI_interface_t));

	game_interface_api->Init = INT_Init;
	game_interface_api->Frame = INT_Frame;
	game_interface_api->Restart = INT_Restart;
	game_interface_api->DrawForeground = INT_DrawCursor;
	game_interface_api->InputEvent = INT_InputEvent;
	game_interface_api->Authenticated = INT_Authenticated;
	game_interface_api->Authenticated_CDKey = INT_Authenticated_CDKey;
	game_interface_api->Authenticated_AccountCreated = INT_Authenticated_AccountCreated;
	game_interface_api->ErrorDialog = INT_ErrorDialog;
	game_interface_api->LoadingFrame = INT_LoadingFrame;
	game_interface_api->LoadingStart = INT_LoadingStart;
	game_interface_api->ShowProgressMeter = INT_ShowProgressMeter;
}

