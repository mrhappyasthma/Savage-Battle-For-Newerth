// (C) 2003 S2 Games

// host.c

// the main game loop is defined here

#include "core.h"
#include <setjmp.h>		//error handling mechanism
#include "allocator.h"
#include "navrep.h"
#include "bans.h"
//#include "libintl.h"

typedef struct
{
	char *string;
	int fontheight;
} engineInfoString_t;

engineInfoString_t engineInfoStrings[] = 
{
	{ "(video res goes here)", 12 },
	{ "Silverback build date: " __DATE__, 12 },
	{ "Net Protocol: " NET_PROTOCOL_VERSION_STRING, 12 },
	//{ "Gettext supported", 12 },
	{ GAME_VERSION, 24 },
	{ "" }
};

int host_msec=1;
int frame_msec;
hostmedia_t	hostmedia;
bool host_runLoadingFrame = false;
bool world_loading = false;
clientAPI_t cl_api;
serverAPI_t sv_api;
interfaceAPI_t int_api;
int DLLTYPE = DLLTYPE_NOTLOADED;
jmp_buf safeStack;		//state of the stack environment at the beginning of the frame

file_t *filenamelist_file = NULL;

static float startup_string_brightness=1;

bool	authenticated = false;
bool	authenticating = false;
bool	authenticating_cdkey = false;
bool	authenticating_createaccount = false;

bool	host_developer_mode = false;

cvar_t	server_address = { "server_address", "", CVAR_READONLY };
cvar_t	server_port = {"server_port", "11235", CVAR_READONLY };

cvar_t	cfg_autosave = { "cfg_autosave", "1", CVAR_SAVECONFIG };
cvar_t	mod = { "mod", "game" };

cvar_t	showPerf = { "showPerf", "0" };
cvar_t	speak = { "speak", "0" };
cvar_t	voice = { "voice", "" };

#ifdef unix
cvar_t 	host_os = { "host_os", "linux", CVAR_READONLY};
cvar_t  sys_useUnicode = { "sys_useUnicode", "1" };
cvar_t  sys_showKeycodes = { "sys_showKeycodes", "0" };
#else
cvar_t 	host_os = { "host_os", "windows", CVAR_READONLY};
#endif

cvar_t	homedir = { "homedir", "" };

cvar_t  sys_enumdir = { "sys_enumdir", "", CVAR_READONLY };
cvar_t	sys_sleep = { "sys_sleep", "0", };
cvar_t  sys_focus = { "sys_focus", "1", CVAR_READONLY };

cvar_t	restartDedicatedOnError = { "restartDedicatedOnError", "1", CVAR_SAVECONFIG };

cvar_t	username = { "username", "" };
cvar_t	password = { "password", "" };
cvar_t	password_confirm = { "password_confirm", "" };
cvar_t	user_id = { "user_id", "" };
cvar_t	autologin = { "autologin", "1" };

cvar_t	cdkey = { "cdkey", "" };

cvar_t	archive_precedence = { "archive_precedence", "1", CVAR_READONLY };

#ifdef __APPLE__
cvar_t	inverty = { "inverty", "1", CVAR_SAVECONFIG };
#else
cvar_t	inverty = { "inverty", "0", CVAR_SAVECONFIG};
#endif

cvar_t	language = { "language", "US" };

cvar_t	clan_icon_url = { "clan_icon_url", "http://clans.s2games.com/clanicon.php?file=" };

cvar_t gfx = { "gfx", "1" };
cvar_t gfx_driver = { "gfx_driver", "OpenGL" };

cvar_t host_maxfps = { "host_maxfps", "130", CVAR_SAVECONFIG | CVAR_VALUERANGE, 10, 1000 };

cvar_t host_initializing = { "host_initializing", "1", CVAR_READONLY, 0, 1, 1, 1 };		//initialize all fields up to "integer" since this may get checked before Cvar_Init

//cvar_t	authentication = {"authentication", "1" };

#ifdef DEDICATED_SERVER
cvar_t	dedicated_server = { "dedicated_server", "1", CVAR_READONLY };
#else
cvar_t	dedicated_server = { "dedicated_server", "0" };
#endif

cvar_t	font_adjustSize = { "font_adjustSize", "0", CVAR_SAVECONFIG };
cvar_t	font_useHinting = { "font_useHinting", "0", CVAR_SAVECONFIG };

cvar_t	default_world = { "default_world", "eden2", CVAR_SAVECONFIG };

cvar_t	writefilelist = { "writefilelist", "0" };

cvar_t autoexec = { "autoexec", "" };

cvar_t	demo_makeMovie = { "demo_makeMovie", "0" };
cvar_t	demo_movieFPS = { "demo_movieFPS", "30", CVAR_SAVECONFIG };
cvar_t	demo_movieDir = { "demo_movieDir", "movie_raw", CVAR_SAVECONFIG };
cvar_t	demo_movieWidth = { "demo_movieWidth", "320", CVAR_SAVECONFIG };
cvar_t	demo_movieHeight = { "demo_movieHeight", "240", CVAR_SAVECONFIG };
cvar_t	demo_speedTest = { "demo_speedTest", "0" };
cvar_t	demo_fixedTime = { "demo_fixedTime", "0" };

cvar_t	nextSessionCommand = { "nextSessionCommand", "" };

cvar_t	log_append =	{ "log_append",	"0", CVAR_SAVECONFIG };

cvar_t	timescale =	{ "timescale", "1.0", CVAR_TRANSMIT };

cvar_t host_date =	{ "host_date",	"", CVAR_READONLY };
cvar_t host_time =	{ "host_time",	"",	CVAR_READONLY };

cvar_t sys_restartProcess = { "sys_restartProcess", "0" };

cvar_t ignoreOldServers = { "ignoreOldServers", "1" };
cvar_t ignoreNewServers = { "ignoreNewServers", "1" };

cvar_t host_numGameErrors = { "numGameErrors", "0", CVAR_READONLY };

extern cvar_t	snd_disable;
extern cvar_t	vid_numModes;
extern cvar_t	vid_fullscreen;
extern cvar_t	vid_gamma;
extern cvar_t	gfx_gammaMult;
extern cvar_t	net_forceIP;

char keyserverCookie[COOKIE_SIZE+1];

void	Host_LoadWorld_Cmd(int argc, char *argv[]);
void	Host_DevLoadWorld_Cmd(int argc, char *argv[]);

int	numFontsNeedingRefreshing = 0;
residx_t	fontsNeedingRefreshing[MAX_FONTS] = { 0 };

void	Host_RefreshFont(residx_t idx)
{
	int i;
	for (i = 0; i < numFontsNeedingRefreshing; i++)
		if (fontsNeedingRefreshing[i] == idx)  //we already know about this one
			return;
	
	fontsNeedingRefreshing[numFontsNeedingRefreshing] = idx;
	numFontsNeedingRefreshing++;
}

void	Host_ActuallyRefreshFonts()
{
	int i;
	for (i = 0; i < numFontsNeedingRefreshing; i++)
	{
		Res_RefreshFont(fontsNeedingRefreshing[i]);
	}
	numFontsNeedingRefreshing = 0;
}

void	Host_ReloadFont()
{
	static int lastAdjustSize = 0;
	static int lastUseHinting = 0;
	int dummy, smallfont = 14, medfont = 16, bigfont = 20;

	if (hostmedia.nicefontShader)
	{
		//did .modified get set on the cvars but they values didn't change?
		if (lastAdjustSize == font_adjustSize.integer
			&& lastUseHinting == font_useHinting.integer)
			return;
		
		Res_UnloadFont(hostmedia.nicefontShader);
	}
	
	lastAdjustSize = font_adjustSize.integer;
	lastUseHinting = font_useHinting.integer;
	
	GUI_ScaleToScreen(&dummy, &smallfont);
	GUI_ScaleToScreen(&dummy, &medfont);
	GUI_ScaleToScreen(&dummy, &bigfont);
	
	hostmedia.nicefontShader = Res_LoadFont(fmt("%sgamefont.ttf", System_GetRootDir()), smallfont + font_adjustSize.integer, medfont + font_adjustSize.integer, bigfont + font_adjustSize.integer);
	if (!hostmedia.nicefontShader)
	{
		Cvar_SetVarValue(&font_adjustSize, 0);
		hostmedia.nicefontShader = Res_LoadFont(fmt("%sgamefont.ttf", System_GetRootDir()), smallfont + font_adjustSize.integer, medfont + font_adjustSize.integer, bigfont + font_adjustSize.integer);
		if (!hostmedia.nicefontShader)
			System_Error("Unrecoverable font loading error trying to load gamefont.ttf!\n");
	}

}

void Host_LoadResources()
{
	//load "slot 0" resources
	Res_LoadShader("textures/core/Null.tga");
	Res_LoadModel("models/null.model");
	Geom_LoadSkinShaders(Res_GetModel(0), 0);		//load up the null model texture right away before we change the current directory
	
	//load standard resources
	hostmedia.whiteShader = Res_LoadShader("textures/core/white.tga");
	hostmedia.consoleShader = Res_LoadShaderEx("textures/loading/background.tga", SHD_FULL_QUALITY);
	hostmedia.sysfontShader = Res_LoadShaderEx("textures/core/sysfont.tga", SHD_FULL_QUALITY | SHD_NO_COMPRESS | SHD_NO_MIPMAPS);
	hostmedia.alphaGradShader = Res_LoadShaderEx("textures/core/alphagrad.tga", SHD_FULL_QUALITY);
	hostmedia.nicefontShader = 0;
	Host_ReloadFont();

	hostmedia.savagelogoShader = hostmedia.consoleShader;
	hostmedia.loadingProgressShader = Res_LoadShader("textures/loading/load0000.tga");
	hostmedia.loadingScreenShader = hostmedia.consoleShader;

	if (!hostmedia.consoleShader || !hostmedia.sysfontShader || !hostmedia.savagelogoShader)
	{
	//	System_Error("Couldn't load basic resource files\n");
	}
}

bool	Host_GetCookie(char *cookie, int cookie_length)
{
	strncpy(cookie, keyserverCookie, cookie_length);
	return true;
}

bool	Host_CreateAccount(char *username, char *password, char *cdkey)
{
	authenticating_createaccount = true;
	authenticating = Keyserver_Authenticate(NULL, username, password, cdkey, true);
	return authenticating_createaccount;
}

bool	Host_CheckCDKey(char *cdkey)
{
	authenticating_cdkey = true;
	authenticating = Keyserver_Authenticate(NULL, NULL, NULL, cdkey, false);
	return authenticating_cdkey;
}

bool	Host_Authenticate(char *address)
{
	char *tmp;

	if (localClient.cstate > CCS_AUTHENTICATING && authenticated)
		return true;

	localClient.cstate = CCS_AUTHENTICATING;
	if (!server_address.string[0] || strcmp(server_address.string, address) != 0)
	{
		tmp = Tag_Strdup(address, MEM_HOST);
		Cvar_SetVar(&server_address, tmp);
	}
	//if (!authentication.integer)
	//	return true;

	authenticating = Keyserver_Authenticate(server_address.string, username.string, password.string, cdkey.string, false);
	return authenticating;
}

void	Host_AuthenticateFrame()
{
	int res;	

	if (DLLTYPE == DLLTYPE_EDITOR)
		return;

	/*
	//if (!authentication.integer)
	{
		authenticated = true;
		authenticating = false;
		//authenticating_cdkey = false;
		authenticating_createaccount = false;
		//tell the interface that we have successfully authenticated the user
		int_api.Authenticated(true);
		return true;
	}
	/**/
	
	if (!authenticating)
		return;
	
	res = Keyserver_AuthenticateFrame(keyserverCookie, COOKIE_SIZE);
	if (res == -1)
	{
		authenticated = false;
		return;
	}
	else
	{
		authenticating = false;
		if (authenticating_cdkey)
		{
			int_api.Authenticated_CDKey(res);
		}
		/*else if (authenticating_createaccount)
		{
			int_api.Authenticated_AccountCreated(res);
			authenticated = res;
			if (authenticated)
			{
				int_api.Authenticated(true);
			}
		}*/
		else
		{
			authenticated = res;
			if (!authenticated)
			{
				//if (localClient.cstate == CCS_AUTHENTICATING)
				//	Host_Disconnect();
				//Console_Errorf("Login failed!  You must use a valid username and password.\n");
				int_api.Authenticated(false);
			}
			else
			{
				//tell the interface that we have successfully authenticated the user
				int_api.Authenticated(true);
	
				return;
			}
		}
		authenticating_cdkey = false;
		authenticating_createaccount = false;
	}
	return;
}

void	Host_Authenticate_CreateAccount_Cmd(int argc, char *argv[])
{
	if (username.string[0] && password.string[0])
	{
		if (strcmp(password.string, password_confirm.string) == 0)
		{
			Host_CreateAccount(username.string, password.string, cdkey.string);
		}
		else
		{
			Console_Errorf("Your passwords don't match\n");
		}
	}
	else
	{
		Console_Errorf("You must enter a username and a password\n");
	}
}

void	Host_AuthenticateCDKey_Cmd(int argc, char *argv[])
{
	Host_CheckCDKey(cdkey.string);
}

void	Host_Authenticate_Cmd(int argc, char *argv[])
{
	bool res;
	res = Host_Authenticate("0.0.0.0");

	if (!res)
		Client_Disconnect();

	if (DLLTYPE == DLLTYPE_GAME)
		int_api.Authenticated(res);
}

//connect to a server
void	Host_Connect(char *address)
{
	char *ip = NULL;
	
	if (dedicated_server.integer)
		return;

	//if we're not connecting to a server started locally, stop the server
	if  (strcmp(net_forceIP.string, "0.0.0.0") == 0)
		ip = "127.0.0.1";
	else
		ip = net_forceIP.string;

	if (strcmp(address, fmt("%s:%i", ip, localServer.port)) != 0 && localServer.active)
		Server_Disconnect();

	//disconnect client
	Client_Disconnect();
		
	localClient.cstate = CCS_AUTHENTICATING;
	if (!Host_Authenticate(address))
		localClient.cstate = CCS_DISCONNECTED;
}

void	Host_ConnectFrame()
{
	if (authenticating)
		return;
	
	if (!authenticated)
		return;
	
	if (strcmp(server_address.string, "0.0.0.0") == 0)
	{
		Client_Disconnect();
		//localClient.cstate = CCS_DISCONNECTED;
		return;
	}

	Client_Connect(server_address.string);
}

void	Host_Connect_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("syntax: connect <address>\n");
		return;
	}

	Host_Connect(argv[0]);
}

void	Host_Reconnect_Cmd(int argc, char *argv[])
{
	if (server_address.string[0])
	{
		Host_Disconnect();
		Host_Connect(fmt("%s:%i", server_address.string, server_port.integer));
	}
	else
		Console_Printf("Connect to a server first\n");
}

void 	Host_Disconnect()
{
	HTTP_CancelAllRequests();

	if (localServer.active)
		Server_Disconnect();
	if (localClient.cstate)
		Client_Disconnect();
}

void Host_Disconnect_Cmd(int argc, char *argv[])
{
	Host_Disconnect();
}

extern bool launched_url;

void Host_Quit_Cmd(int argc, char *argv[])
{
	if (localServer.active)
		Server_Disconnect();
	if (localClient.cstate)
		Client_Disconnect();

	if (cfg_autosave.integer)
		Cvar_SaveConfigFile("/startup.cfg", 0, NULL, CVAR_SAVECONFIG, true);

#ifdef SAVAGE_DEMO
	if (!argc && !dedicated_server.integer && !launched_url)
	{
		Cmd_Exec("launchurl http://www.s2games.com/savagedemo_motd");
	}
#endif

	if (argc)
		Cvar_SetVarValue(&sys_restartProcess, 1);

	System_Quit();
}

void	MiscCmds_Register();

#ifndef LEVEL_EDITOR
void	Host_Server_Cmd(int argc, char *argv[]);
#endif

file_t *DEBUG_LOG_FILE = NULL;


void Host_LoadingScreen(const char *resourcename)
{
	static float shadertime = 0;

	System_ProcessEvents();		//so they can hit ESC if they want to
	Cmd_ExecBuf();

	Draw_SetShaderTime(shadertime);

	int_api.LoadingFrame(resourcename);

	shadertime += 0.05;
}


#define RIGHT_JUSTIFY(x, y, string, ch, font) (x) - DU_StringWidth(string, (ch), (ch), 1, 256, (font)), (y), string, ch, ch, 1, 256, (font)

void Host_DrawStartupStrings(float r, float g, float b, float alpha)
{
	int sw = Vid_GetScreenW();
	int sh = Vid_GetScreenH();
	int yy = sh-2;
	int numstrings = 0;
	int n;

	while(engineInfoStrings[numstrings].string[0])
	{		
		yy -= engineInfoStrings[numstrings].fontheight + 2;
		numstrings++;
	};
	
/*
	DU_DrawString(RIGHT_JUSTIFY(sw-2,sh-64,
					"Silverback initializing..." ,
					16, hostmedia.nicefontShader), false);
*/		
	
	for (n=0; n<numstrings; n++)
	{
		if (n == numstrings-1)
		{
			Draw_SetColor(vec4(0.8,0.8,0.8,alpha*0.8));
			Draw_Quad2d(Vid_GetScreenW()/2,yy,Vid_GetScreenW(),engineInfoStrings[n].fontheight+4,0,0,1,1,hostmedia.alphaGradShader);
		}

		Draw_SetColor(vec4(0,0,0,alpha/1.2));
		DU_DrawString(RIGHT_JUSTIFY(sw,yy+2,
			engineInfoStrings[n].string,
			engineInfoStrings[n].fontheight, 
			hostmedia.nicefontShader), 
			false);

		Draw_SetColor(vec4(r,g,b,alpha));
		DU_DrawString(RIGHT_JUSTIFY(sw-2,yy,
					engineInfoStrings[n].string,
					engineInfoStrings[n].fontheight, 
					hostmedia.nicefontShader), 
					false);


		yy += engineInfoStrings[n].fontheight + 2;
	}
}


void Host_LoadingStatusCallback(const char *resourcename)
{
	/* 
	static int xpos = 0;
	static int ypos = 0;
	static int velocity = 30;
	*/

	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		Draw_SetColor(black);
		Draw_Quad2d(0,0,Vid_GetScreenW(),Vid_GetScreenH(),0,0,1,1,hostmedia.whiteShader);

		Console_Format();
		Console_Draw();
	}
	else
	{
		Res_SetLoadingStatusCallback(NULL);		

		Host_LoadingScreen(resourcename);


#if 0
			Draw_SetColor(white);
			Draw_Quad2d(0,0,Vid_GetScreenW(),Vid_GetScreenH(),0,0,1,1,hostmedia.loadingScreenShader);
			Draw_Quad2d(Vid_GetScreenW() * 0.42, Vid_GetScreenH() * 0.82, (Vid_GetScreenW() * 0.58)-(Vid_GetScreenW() * 0.42), Vid_GetScreenH() - (Vid_GetScreenH() * 0.82), 0,1,1,0,hostmedia.loadingProgressShader);

			Console_Format();
			Console_Draw();
#endif

		Res_SetLoadingStatusCallback(Host_LoadingStatusCallback);
	}

	
/*	DU_DrawString(xpos, Vid_GetScreenH() - ypos, "Loading (-:", 32, 32, 1, 11, hostmedia.sysfontShader);

	xpos+=5;
	ypos+=velocity;
	velocity--;
	if (ypos<=0)
		velocity = 30;
	if (xpos>Vid_GetScreenW())
		xpos=0;*/
}

void    Host_GetInfo(char *hostname, int port, bool extended)
{
	int oldport;
	char oldAddress[64] = {0};

	//save the old values
	if (ncLocalBrowser.sendAddrName && ncLocalBrowser.sendAddrName[0])
		strncpySafe(oldAddress, ncLocalBrowser.sendAddrName, 64);
	oldport = ncLocalBrowser.sendAddr.sin_port;
	
	Net_SetSendAddr(&ncLocalBrowser, hostname, port);
	Pkt_Clear(&ncLocalBrowser.send);
	if (extended)
		Pkt_WriteCmd(&ncLocalBrowser.send, CPROTO_EXT_INFO_REQUEST);
	else
		Pkt_WriteCmd(&ncLocalBrowser.send, CPROTO_INFO_REQUEST);
	Pkt_WriteInt(&ncLocalBrowser.send, System_Milliseconds());
	Net_SendPacket(&ncLocalBrowser); 

	Console_DPrintf("Requesting game info from %s:%i\n", hostname, port);

	//restore old values
	if (oldAddress[0])
		Net_SetSendAddr(&ncLocalBrowser, oldAddress, oldport);
	ncLocalBrowser.sendAddr.sin_port = oldport;
}

void    Host_GetExtInfo_Cmd(int argc, char *argv[])
{
	int oldport;
	if (argc)
	{
		Host_GetInfo(argv[0], DEFAULT_SERVER_PORT, true);
	}
	else if (server_address.string[0])
	{
		oldport = ncLocalBrowser.sendAddr.sin_port;
		Host_GetInfo(server_address.string, DEFAULT_SERVER_PORT, true);
		ncLocalBrowser.sendAddr.sin_port = oldport;
	}
	else
		Console_Printf("error - you must specify an address to get info from when not connected to a server\n");
}

void    Host_GetInfo_Cmd(int argc, char *argv[])
{
	int oldport;
	if (argc)
	{
		Host_GetInfo(argv[0], DEFAULT_SERVER_PORT, false);
	}
	else if (server_address.string[0])
	{
		oldport = ncLocalBrowser.sendAddr.sin_port;
		Host_GetInfo(server_address.string, DEFAULT_SERVER_PORT, false);
		ncLocalBrowser.sendAddr.sin_port = oldport;
	}
	else
		Console_Printf("error - you must specify an address to get info from when not connected to a server\n");
}

void    Host_Ping(char *hostname, int port)
{
	int oldport;
	char *sendAddrName = NULL;

	//save the old values
	if (ncLocalBrowser.sendAddrName && ncLocalBrowser.sendAddrName[0])
		sendAddrName = Tag_Strdup(ncLocalBrowser.sendAddrName, MEM_HOST);
	oldport = ncLocalBrowser.sendAddr.sin_port;
	
	//now send the ping
	Net_SetSendAddr(&ncLocalBrowser, hostname, port);
	Pkt_Clear(&ncLocalBrowser.send);
	Pkt_WriteCmd(&ncLocalBrowser.send, CPROTO_PING);
	Pkt_WriteInt(&ncLocalBrowser.send, System_Milliseconds());
	Net_SendPacket(&ncLocalBrowser); 
	Pkt_Clear(&ncLocalBrowser.send);

	//restore the old values
	if (sendAddrName)
		Net_SetSendAddr(&ncLocalBrowser, sendAddrName, oldport);
	ncLocalBrowser.sendAddr.sin_port = oldport;
	
	if (sendAddrName)
		Tag_Free(sendAddrName);
}

void    Host_Ping_Cmd(int argc, char *argv[])
{
	int oldport;
	if (argc)
	{
		Host_Ping(argv[0], DEFAULT_SERVER_PORT);
	}
	else if (server_address.string[0])
	{
		oldport = ncLocalBrowser.sendAddr.sin_port;
		Host_Ping(server_address.string, DEFAULT_SERVER_PORT);
		ncLocalBrowser.sendAddr.sin_port = oldport;
	}
	else
		Console_Printf("error - you must specify an address to ping when not connected to a server\n");
}

char *Host_ParseServerInfo_StateStrings1(char *ip, int port, packet_t *pkt)
{
	char coreinfo[512];
	char gameinfo[512];
	int pingtime;

	pingtime = System_Milliseconds() - Pkt_ReadInt(pkt);
	Pkt_ReadString(pkt, coreinfo, 512);
	Pkt_ReadString(pkt, gameinfo, 512);
	
	MasterServer_SetGameInfo(ip, port, pingtime, coreinfo, gameinfo);
	
	return fmt("%s\n%s\n", coreinfo, gameinfo);

}

char *Host_ParseExtServerInfo(char *ip, int port, packet_t *pkt)
{
	int version, ping;
	char *ret = "";
	char *coreInfo, *gameInfo;

	version = Pkt_ReadByte(pkt);

	if (version < NET_PROTOCOL_VERSION && ignoreOldServers.integer)
	{
		MasterServer_EraseGameInfo(ip, port);
		Console_Printf("Old server info packet, ignored\n");
	}
	else if (version > NET_PROTOCOL_VERSION && ignoreNewServers.integer)
	{
		Console_Printf("Unknown INFO packet version %i\n", version);
		MasterServer_EraseGameInfo(ip, port);
	}
	else
	{
		ret = Host_ParseServerInfo_StateStrings1(ip, port, pkt);
		if (MasterServer_GetGameInfo(ip, port, &ping, &coreInfo, &gameInfo))
		{
			cl_api.ExtendedServerInfo(ip, port, ping, coreInfo, gameInfo);
		}
	}

	Pkt_Clear(pkt);

	return ret;
}

char *Host_ParseServerInfo(char *ip, int port, packet_t *pkt)
{
	int version;	
	char *ret = "";

	version = Pkt_ReadByte(pkt);

	if (version < NET_PROTOCOL_VERSION && ignoreOldServers.integer)
	{
		MasterServer_EraseGameInfo(ip, port);
		Console_Printf("Old server info packet, ignored\n");
	}
	else if (version > NET_PROTOCOL_VERSION && ignoreNewServers.integer)
	{
		Console_Printf("Unknown INFO packet version %i\n", version);
		MasterServer_EraseGameInfo(ip, port);
	}
	else
	{
		ret = Host_ParseServerInfo_StateStrings1(ip, port, pkt);
	}

	Pkt_Clear(pkt);

	return ret;
}

extern cvar_t	mem_freeOnExit;


void	Host_Version_Cmd(int argc, char *argv[])
{
	int n=0;
	static int toggle = 1;

	Console_Printf("VERSION INFO\n");
	Console_Printf("=====================================\n");

	while(engineInfoStrings[n].string[0])
	{
		Console_Printf(fmt("%s\n",engineInfoStrings[n].string));
		n++;
	};	

	toggle ^= 1;

	if (toggle)
	{
		Console_Printf("\nVersion stamp is now ON (type \"version\" again to disable)\n");
		startup_string_brightness = 999999999;
	}
	else
	{
		Console_Printf("\nVersion stamp is now OFF\n");
		startup_string_brightness = 1;
	}
}

void	Host_HashTest_Cmd(int argc, char *argv[])
{
	char orig[21];
	char test[21];
	char *hash;
	int i, len, xor = M_Randnum(-MAX_INT, MAX_INT-1);

	for (i = 0; i < 20; i++)
		orig[i] = M_Randnum(0,255);

	memcpy(test, orig, 20);

	hash = BinaryToHexWithXor(test, 20, xor);
	len = HexToBinary(hash, test, 21);
	hash = BinaryToHexWithXor(test, 20, xor);
	len = HexToBinary(hash, test, 21);
	Console_Printf("After conversion, len is %i, and the values are:", len);
	for (i = 0; i < 20; i++)
		if (orig[i] != test[i])
			Console_Printf("Different!");
	Console_Printf("\n");
}

void	Host_SetDateTime()
{
	time_t		t;
	struct tm	*datetime;

	time(&t);
	datetime = localtime(&t);
	Cvar_SetVar(&host_date, fmt("%04i/%02i/%02i", datetime->tm_year + 1900, datetime->tm_mon + 1, datetime->tm_mday));
	Cvar_SetVar(&host_time, fmt("%02i:%02i:%02i", datetime->tm_hour, datetime->tm_min, datetime->tm_sec));
}

//initializes all game systems
void Host_Init()
{	
#ifdef unix
	FILE *festivalCheck;
	struct stat fstat;
	file_t *f;
	char line[128];
#endif	
	/*if (setjmp(safeStack))		//save the stack environment
	{
		System_Error("Game_Error() was called during initialization.  Check debug.log");
		return;					//there was an error during the init
	}*/


	M_Init();
	Cmd_Init();	
	Mem_RegisterCmds();
	Cvar_Init();

	srand(System_GetTicks());

	if (!dedicated_server.integer)
		Input_Init();
	
	Cvar_Register(&host_numGameErrors);
	Cvar_Register(&homedir);
	Cvar_Register(&archive_precedence);

	Cvar_SetVar(&homedir, fmt("%s/.savage/", getenv("HOME")));

#ifdef unix
	if (stat(homedir.string, &fstat) != 0)
	{
		if (mkdir(homedir.string, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH)!=0)
		{
			fprintf(stderr, "Unable to create .savage directory in your home directory location, bailing out.\n");
			exit(1);
		}
	}
	else
	{
		if (!S_ISDIR(fstat.st_mode))
		{
			fprintf(stderr, "There is a file called .savage in your home directory.  Please move it out of the way and run Savage again.\n");
			exit(1);
		}
	}
	
	if (stat(fmt("%sstartup.cfg", homedir.string), &fstat) != 0)
	{		
		fprintf(stderr, "no startup.cfg found, generating one for you...\n");

		f = File_OpenAbsolute(fmt("%sstartup.cfg", homedir.string), "w");
		if (!f)
		{
			fprintf(stderr, "Couldn't open %sstartup.cfg for writing, bailing out.\n", homedir.string);
			exit(1);
		}
		Cvar_WriteConfigToFile(f, 0, "", CVAR_SAVECONFIG, true);		
		File_Close(f);
	}
#endif

	//load up default.cfg first to set up initial values
	Cmd_ReadConfigFile(fmt("/%s/settings/default.cfg", mod.string), false);

//#ifdef unix
//	Cmd_ReadConfigFileAbsolute(fmt("%sstartup.cfg", homedir.string), false);
//#else	
	Cmd_ReadConfigFile(fmt("/%s/startup.cfg", mod.string), false);	
//#endif


	File_ClanIconInit();

	Vid_RegisterVars();

	Cvar_Register(&sys_sleep);
	Cvar_Register(&sys_focus);
	Cvar_Register(&sys_restartProcess);

#ifdef unix
	Cvar_Register(&sys_useUnicode);
	Cvar_Register(&sys_showKeycodes);
#endif

	Cvar_Register(&host_maxfps);
	Cvar_Register(&host_initializing);

	Cvar_Register(&sys_enumdir);
	Cvar_Register(&mod);

	Cvar_Register(&ignoreOldServers);
	Cvar_Register(&ignoreNewServers);

	Cvar_Register(&gfx);
	Cvar_Register(&gfx_driver);
	Cvar_Register(&dedicated_server);
	Cvar_Register(&default_world);
	//Cvar_Register(&authentication);

	Cvar_Register(&username);
	Cvar_Register(&password);
	Cvar_Register(&password_confirm);
	Cvar_Register(&user_id);
	Cvar_Register(&cdkey);
	Cvar_Register(&autologin);
	Cvar_Register(&clan_icon_url);
	Cvar_Register(&inverty);
	Cvar_Register(&language);

	Cvar_Register(&showPerf);
	Cvar_Register(&speak);
	Cvar_Register(&voice);

	Cvar_Register(&mem_freeOnExit);

	Cvar_Register(&server_address);
	Cvar_Register(&server_port);

	Cvar_Register(&restartDedicatedOnError);
	Cvar_Register(&host_os);
	Cvar_Register(&host_date);
	Cvar_Register(&host_time);
	Host_SetDateTime();

	Cvar_Register(&autoexec);
	Cvar_Register(&font_adjustSize);
	Cvar_Register(&font_useHinting);
 
	Cvar_Register(&demo_speedTest);
	Cvar_Register(&demo_makeMovie);
	Cvar_Register(&demo_movieFPS);
	Cvar_Register(&demo_movieDir);
	Cvar_Register(&demo_movieWidth);
	Cvar_Register(&demo_movieHeight); 

	Cvar_Register(&nextSessionCommand);

	Cvar_Register(&timescale);

	Cvar_Register(&log_append);

	//execute this once now right away, so it can affect initialization
	Cmd_Exec(System_GetCommandLine());

	if (log_append.integer)
	{
#ifdef _WIN32
	DEBUG_LOG_FILE = File_Open("debug.log", "a+");
#else
	DEBUG_LOG_FILE = File_OpenAbsolute(fmt("%sdebug.log", homedir.string), "a+");
#endif
	}
	else
	{
#ifdef _WIN32
	DEBUG_LOG_FILE = File_Open("debug.log", "w");
#else
	DEBUG_LOG_FILE = File_OpenAbsolute(fmt("%sdebug.log", homedir.string), "w");
#endif
	}

	if (!DEBUG_LOG_FILE)
		System_Error("Couldn't open debug.log for writing\n");

	Archive_Init();

	Console_Init();

	// i hate this:
	init();

//#ifdef INTERNATIONAL
#if 0
#pragma message ( "Compiling gettext() support" )

	setlocale (LC_CTYPE, "");
	setlocale (LC_COLLATE, "");
	setlocale (LC_MESSAGES, "");

	Console_Printf("Locale: %s\n", setlocale(LC_ALL, NULL));
	Console_Printf("Root dir: %s\n", System_GetRootDir());

	textdomain ("savage");
	bindtextdomain("savage", fmt("%s/locale", System_GetRootDir()));
#endif

	Console_Printf("************************************************************\n");
	Console_Printf("Silverback Engine starting up...\n");
	Console_Printf("[%s]\n[%s]\n", host_date.string, host_time.string);
	Host_Version_Cmd(0, NULL);
	startup_string_brightness = 2;


	Console_Printf("running with glib version %i.%i.%i\n", GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);
	
	Vid_RegisterVars();
	Sound_RegisterVars();
	Input_RegisterVars();

	Hash_Init();
	MasterServer_Init();
	Server_RegisterVars();

	Geom_Init();
	Bitmap_Init();
	Font_Init();

	TCP_Init();
	Net_Server_Resolve();
	TCP_SetSignalHandlers();

	HTTP_Init();
	IRC_Init();

	

//	if (DLLTYPE == DLLTYPE_GAME)
		Net_RegisterVars();

	if (writefilelist.integer)
		filenamelist_file = File_Open("/filelist.txt", "w");
	
	Cvar_Register(&cfg_autosave);
	//Cvar_SetVarValue(&vid_mode, -1);

	if (dedicated_server.integer && gfx.integer)
	{
		Cvar_SetVar(&gfx_driver, "GL_Console");
		Cvar_SetVarValue(&gfx, 0);
	}
	else
	{
		Vid_SetDriver(gfx_driver.string);
		
		Vid_Init();

		engineInfoStrings[0].string = Tag_Strdup(fmt(_("Video resolution: %i x %i"), Vid_GetScreenW(), Vid_GetScreenH()), MEM_HOST);
	}

	Res_Init();

#ifdef unix
	/*
	if (!dedicated_server.integer)
	{
		festivalCheck = fopen("/usr/bin/festival", "r");
		if (!festivalCheck)
			festivalCheck = fopen("/usr/local/bin/festival", "r");
		if (festivalCheck)
		{
			fclose(festivalCheck);
			Cvar_SetVarValue(&speak, 1);
		}
	}
	*/
#endif

	if (!dedicated_server.integer)
		Sound_Init();

	System_ChangeRootDir(fmt("/%s", mod.string));
	
	Buddies_Init();
	Bans_Init();

	/*
#ifndef SAVAGE_DEMO
	//must do this in this order, misc zips first, then official archives
	//Archive_RegisterArchivesInDir("/");
#endif
	*/

	Archive_RegisterOfficialArchives();

	//init the GUI even if we're running a dedicated server, since portions of cmd.c call gui functions
	//we also need this to be called before Host_LoadResources()
	GUI_Init();	

	if (!dedicated_server.integer)
	{
		char *s = "Initializing...";
		int len;		

		Host_LoadResources();

		Draw_SetColor(white);
		len = DU_StringWidth(s, 16, 16, 1, 256, hostmedia.nicefontShader);
		DU_DrawString(Vid_GetScreenW()/2 - len/2, Vid_GetScreenH()/2-8, s, 16, 16, 1, 256, hostmedia.nicefontShader, false);
		Host_DrawStartupStrings(1,1,1,1);
		Vid_EndFrame();		
	}
	host_msec = 1;

	NavRep_Startup();

	//the dedicated server needs to call Scene_Init to fill in gfx_farclip
	Scene_Init();
	
	World_Init();
	
	Cmd_Register("quit", Host_Quit_Cmd);
	Cmd_Register("exit", Host_Quit_Cmd);
	Cmd_Register("disconnect", Host_Disconnect_Cmd);
	Cmd_Register("version", Host_Version_Cmd);
	
	if (!dedicated_server.integer)
	{
		Cmd_Register("connect", Host_Connect_Cmd);
		Cmd_Register("reconnect", Host_Reconnect_Cmd);
	}

	Cmd_Register("ping", Host_Ping_Cmd);
	Cmd_Register("info", Host_GetInfo_Cmd);
	Cmd_Register("extinfo", Host_GetExtInfo_Cmd);
	
	Cmd_Register("world",		Host_LoadWorld_Cmd);
#ifndef SAVAGE_DEMO
	Cmd_Register("devworld",	Host_DevLoadWorld_Cmd);
#endif

	Cmd_Register("authenticate", Host_Authenticate_Cmd);
	Cmd_Register("authenticate-cdkey", Host_AuthenticateCDKey_Cmd);
	Cmd_Register("create-account", Host_Authenticate_CreateAccount_Cmd);

	Cmd_Register("hashtest", Host_HashTest_Cmd);

	host_runLoadingFrame = true;

	if (!dedicated_server.integer)
	{
		Res_SetLoadingStatusCallback(Host_LoadingStatusCallback);
	}

	System_InitGameDLL();

	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		Cvar_SetValue("cmd_useScriptCache", 0);
	}

	if (DLLTYPE == DLLTYPE_GAME)
	{
		Net_Init();
		Cvar_Set("world_save_path", "/");
	}

	Server_Init();
	TOD_Init();

	if (!dedicated_server.integer)
	{
#ifdef unix
		Cmd_ReadConfigFileAbsolute(fmt("%scdkey", homedir.string), false);
#else
		Cmd_ReadConfigFile("/../game/cdkey", false);
#endif
		Client_Init();
		//initialize and restart the main interface
		int_api.Init();
		int_api.Restart();		
		Input_SetMouseMode(MOUSE_FREE);
	}
	
	//Cmd_Register("server", Host_Server_Cmd);	

	MiscCmds_Register(); //register some extra console commands

	Vid_PrintWarnings();

	Cvar_SetVarValue(&host_initializing, 0);

	Cmd_ReadConfigFile("/autoexec.cfg", false);

	//do autoexec cvar
	if (autoexec.string[0])
	{		
		Cmd_BufPrintf(autoexec.string);
	}

	if (dedicated_server.integer)
		Cmd_BufPrintf("world %s", default_world.string);
}

residx_t	Host_GetSavageLogoShader()
{
	return hostmedia.savagelogoShader;
}

residx_t	Host_GetWhiteShader()
{
	return hostmedia.whiteShader;
}

residx_t	Host_GetLoadingScreenShader()
{
	return hostmedia.loadingScreenShader;
}

residx_t	Host_GetConsoleShader()
{
	return hostmedia.consoleShader;
}

residx_t	Host_GetSysfontShader()
{
	return hostmedia.sysfontShader;
}

residx_t	Host_GetNicefontShader()
{
	return hostmedia.nicefontShader;
}

int		Host_Milliseconds()
{
	return host_msec;
}

float	Host_FrameSeconds()
{
	return (float)frame_msec / 1000.0;
}

int		Host_FrameMilliseconds()
{
	return frame_msec;
}

extern file_t *SCRIPT_LOG_FILE;
extern int total_script_executions;
extern int total_script_execution_time;
extern int total_script_loads;
extern int total_script_load_failures;
extern int total_script_load_time;
extern int total_script_cache_hits;
extern int total_script_cache_read_time;
extern int total_script_preprocess_passes;
extern int total_script_preprocess_time;


//the crashrpt exception callback will call this function so that we can get complete log files
void	Host_CloseLogFiles()
{
	if (DEBUG_LOG_FILE)
	{
		File_Close(DEBUG_LOG_FILE);
		DEBUG_LOG_FILE=NULL;
	}
	if (filenamelist_file)
	{
		File_Close(filenamelist_file);
		filenamelist_file=NULL;
	}

	if (SCRIPT_LOG_FILE)
	{
		File_Printf(SCRIPT_LOG_FILE, "         *** Totals ***\n");
		File_Printf(SCRIPT_LOG_FILE, "              instances     msecs\n");
		File_Printf(SCRIPT_LOG_FILE, "executions:      %6i  %8i\n", total_script_executions, total_script_execution_time);
		File_Printf(SCRIPT_LOG_FILE, "disk access:     %6i  %8i\n", total_script_loads, total_script_load_time);
		File_Printf(SCRIPT_LOG_FILE, "cache hits:      %6i  %8i\n", total_script_cache_hits, total_script_cache_read_time);
		File_Printf(SCRIPT_LOG_FILE, "preprocessor:    %6i  %8i\n", total_script_preprocess_passes, total_script_preprocess_time);
		File_Printf(SCRIPT_LOG_FILE, "\n");
		File_Printf(SCRIPT_LOG_FILE, "load failures: %i", total_script_load_failures);
		File_Close(SCRIPT_LOG_FILE);
		SCRIPT_LOG_FILE=NULL;
	}
}

void Host_ShutDown ()
{
	if (localServer.active)
		Server_SendShutdown();
		
	HTTP_Shutdown();
	
	Net_ShutDown();
	Buddies_Shutdown();
	Bans_Shutdown();
	IRC_Shutdown();

	Vid_ShutDown();
	Input_Shutdown();
	Sound_Shutdown();

	Completion_Shutdown();

	Archive_Shutdown();

	Host_CloseLogFiles();
	File_Shutdown();

	Mem_ShutDown();
}



vec4_t	clearcol = {0,0,0,0};

void	Host_LoadWorld(char *world, bool devMode)
{
	//make sure the world exists first.  if it doesn't, don't do anything
	if (!File_Exists(fmt("/world/%s.s2z", world)))
		return;	

	if (devMode || DLLTYPE == DLLTYPE_EDITOR)
	{
		host_developer_mode = true;
		Cvar_AllowCheats();		
	}
	else
	{
		host_developer_mode = false;
		Cvar_BlockCheats();
		Cvar_ResetVars(CVAR_CHEAT);
	}

	//only disconnect if we're not a server changing maps
	if (!localServer.active)
		Host_Disconnect();

	console.active = false;

	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		static bool firstLoad = true;

		if (firstLoad)
		{
			GUI_Reset();
			cl_api.Init();
			Cvar_AllowCheats();
			firstLoad = false;
			Cvar_BlockCheats();
		}

//		Sound_StopAllSounds();
		
		if (World_Load(world, true))
		{						
			cl_api.Restart();		
		}
		return;
	}

	//if (localClient.cstate)
		//Client_Disconnect();  //disconnect our local client

	world_loading = true;
	Server_Start(world, false);
	world_loading = false;
}

void	Host_LoadWorld_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("syntax: world <worldname> (don't include any file extension)\n");
		return;
	}

	Host_LoadWorld(argv[0], false);
}

#ifndef SAVAGE_BETA
void	Host_DevLoadWorld_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("syntax: devworld <worldname> (don't include any file extension)\n");
		return;
	}

	Host_LoadWorld(argv[0], true);
}
#endif	//SAVAGE_BETA

float Host_GetLogicFPS()
{
	return 20;
}

void Host_TestGameAPIValidity(clientAPI_t *capi, serverAPI_t *sapi, interfaceAPI_t *iapi)
{
	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		if (!capi->Init)
			System_Error("No Init function was defined\n");
		if (!capi->Frame)
			System_Error("No Frame function was defined\n");
		if (!capi->Restart)
			System_Error("No Restart function was defined\n");
		if (!capi->DrawForeground)
			System_Error("No DrawForeground function was defined\n");
		if (!capi->InputEvent)
			System_Error("No InputEvent function was defined\n");
	}
	else
	{
		//client api
		if (!capi->Init)
			System_Error("No client side Init function was defined\n");
		if (!capi->GetBuild)
			System_Error("No client side GetBuild function was defined\n");
		if (!capi->Frame)
			System_Error("No client side Frame function was defined\n");
		if (!capi->InputEvent)
			System_Error("No client side InputEvent function was defined\n");
		if (!capi->Restart)
			System_Error("No Restart function was defined\n");
		if (!capi->DrawForeground)
			System_Error("No DrawForeground function was defined\n");
		if (!capi->BeginServerFrame)
			System_Error("No BeginServerFrame function was defined\n");
		if (!capi->EndServerFrame)
			System_Error("No EndServerFrame function was defined\n");
		if (!capi->PrecacheResources)
			System_Error("No PrecacheResources function was defined\n");

		//server api
		if (!sapi->ClientConnect)
			System_Error("No ClientConnect function was defined\n");
		if (!sapi->ClientDisconnect)
			System_Error("No ClientDisconnect function was defined\n");
		if (!sapi->Init)
			System_Error("No server side Init function was defined\n");
		if (!sapi->Frame)
			System_Error("No server side Frame function was defined\n");
		if (!sapi->Reset)
			System_Error("No Reset function was defined\n");
		if (!sapi->ClientMessage)
			System_Error("No ClientMessage function was defined\n");

		//interface api
		if (!iapi->Init)
			System_Error("No Init function was defined\n");
		if (!iapi->Frame)
			System_Error("No Frame function was defined\n");
		if (!iapi->InputEvent)
			System_Error("No InputEvent function was defined\n");
		if (!iapi->Restart)
			System_Error("No Restart function was defined\n");
		if (!iapi->DrawForeground)
			System_Error("No DrawForeground function was defined\n");
		if (!iapi->Authenticated)
			System_Error("No Authenticated function was defined\n");
		if (!iapi->LoadingFrame)
			System_Error("No interface LoadingFrame function was defined\n");
		//if (!iapi->ErrorDialog)							//not required
		//	System_Error("No ErrorDialog function was defined\n");
	}
}


extern cvar_t svr_world;

void	Game_Error(const char *errormsg, ...)
{	
	static int num_errors = 0;

	if (host_initializing.integer)
	{
		//if this is called during initialization then quit out
		System_Error(fmt("Game error during initialization:\n\n%s", errormsg));
	}

	//clear the command buffer
	Cmd_ClearBuf();

	//disconnect the server and/or client
	Host_Disconnect();

	//make sure we go back to the root directory if we had done any File_ChangeDir() calls
	File_ResetDir();

	Console_Printf("\n\n\n***********  GAME ERROR  ***********\n");
	Console_Printf(errormsg);
	Console_Printf("************************************\n\n\n\n");

	Console_Errorf(errormsg);

	if (nextSessionCommand.string[0])
	{
		Cmd_BufPrintf("%s\n", nextSessionCommand.string);
	}
	if (dedicated_server.integer && restartDedicatedOnError.integer)
	{
		Cmd_BufPrintf("world %s\n", svr_world.string);
	}

	num_errors++;
	Cvar_SetVarValue(&host_numGameErrors, num_errors);

	//non local goto back to the beginning of Host_Frame
	longjmp(safeStack,1);
}

double overhead_amounts[OVERHEAD_NUMTYPES];

void	Host_Overhead(overhead_enum type, double amount)
{
	if (type >= OVERHEAD_NUMTYPES || type < 0)
	{		
		return;
	}

	overhead_amounts[type] += amount*1000;
}

static double last_time = 0;

void	Host_ResetOverhead()
{
	memset(overhead_amounts, 0, sizeof(overhead_amounts));
	
	last_time = System_GetPerfCounter();

	WT_ResetTraceCount();
}






extern cvar_t net_showStats;

void	Host_DrawOverhead()
{
//	OVERHEAD_INIT;

	if (!showPerf.integer)
		return;	

	Host_PrintOverhead("============================\n");
	Host_PrintOverhead("Performance\n\n");
	Host_PrintOverhead("+Client Frame ms:      +%.2f\n", overhead_amounts[OVERHEAD_CLIENT_FRAME]);
	Host_PrintOverhead("  Tracebox ms:          %.2f\n\n", overhead_amounts[OVERHEAD_TRACEBOX_CLIENT]);

	Host_PrintOverhead("  Scene Render ms:      %.2f\n", overhead_amounts[OVERHEAD_SCENE_RENDER]);
	Host_PrintOverhead("    Cull ms:            %.2f\n", overhead_amounts[OVERHEAD_SCENE_CULL]);
	Host_PrintOverhead("    OccluderList ms     %.2f\n", overhead_amounts[OVERHEAD_SCENE_OCCLUDERSETUP]);
	Host_PrintOverhead("    Setup ms:           %.2f\n", overhead_amounts[OVERHEAD_SCENE_SETUP]);
	Host_PrintOverhead("    Terrain ms:         %.2f\n", overhead_amounts[OVERHEAD_SCENE_DRAWTERRAIN]);
	Host_PrintOverhead("      cull ms:          %.2f\n", overhead_amounts[OVERHEAD_TERRAIN_CULL]);
	Host_PrintOverhead("      rebuild ms:       %.2f\n", overhead_amounts[OVERHEAD_TERRAIN_REBUILD]);
	Host_PrintOverhead("    Foliage ms:         %.2f\n", overhead_amounts[OVERHEAD_SCENE_DRAWFOLIAGE]);
	Host_PrintOverhead("   /Sky ms:             %.2f\n", overhead_amounts[OVERHEAD_SCENE_DRAWSKY]);
	Host_PrintOverhead("   \\Object ms:          %.2f\n", overhead_amounts[OVERHEAD_SCENE_DRAWOBJECTS]);
	Host_PrintOverhead("      bucketsetup ms:   %.2f\n", overhead_amounts[OVERHEAD_SCENE_BUCKETSETUP]);
	Host_PrintOverhead("      addtolists ms:    %.2f\n", overhead_amounts[OVERHEAD_MODEL_ADDTOLISTS]);
	Host_PrintOverhead("      Drawmesh ms:      %.2f\n", overhead_amounts[OVERHEAD_MODEL_DRAWMESH]);
	Host_PrintOverhead("        setuparrays ms: %.2f\n", overhead_amounts[OVERHEAD_MODEL_SETUPMESHARRAYS]);
	Host_PrintOverhead("        vtxpointers ms: %.2f\n", overhead_amounts[OVERHEAD_MODEL_SETUPVERTEXPOINTERS]);
	Host_PrintOverhead("        staticmesh ms:  %.2f\n", overhead_amounts[OVERHEAD_MODEL_DRAWSTATICMESH]);
	Host_PrintOverhead("        rigid trans ms: %.2f\n", overhead_amounts[OVERHEAD_MODEL_DRAWRIGIDMESH]);
	Host_PrintOverhead("        skinnedmesh ms: %.2f\n", overhead_amounts[OVERHEAD_MODEL_DRAWSKINNEDMESH]);
	Host_PrintOverhead("    Sprites ms:         %.2f\n", overhead_amounts[OVERHEAD_SCENE_DRAWSPRITES]);
	Host_PrintOverhead("    Scene polys ms:     %.2f\n", overhead_amounts[OVERHEAD_SCENE_DRAWSCENEPOLYS]);
	
	Host_PrintOverhead("    Cleanup ms:         %.2f\n\n", overhead_amounts[OVERHEAD_SCENE_CLEANUP]);	
	
	Host_PrintOverhead("+Server Frame ms:      +%.2f\n", overhead_amounts[OVERHEAD_SERVER_FRAME]);
	Host_PrintOverhead("  Game Logic ms:        %.2f\n", overhead_amounts[OVERHEAD_SERVER_GAMELOGIC]);
	Host_PrintOverhead("  Send Packets ms:      %.2f\n", overhead_amounts[OVERHEAD_SERVER_SENDPACKETS]);
	Host_PrintOverhead("  Tracebox ms:          %.2f\n", overhead_amounts[OVERHEAD_TRACEBOX_SERVER]);
	Host_PrintOverhead("+Console ms:           +%.2f\n", overhead_amounts[OVERHEAD_CONSOLE_FORMAT] + overhead_amounts[OVERHEAD_CONSOLE_DRAW]);
	Host_PrintOverhead("  Draw ms:              %.2f\n", overhead_amounts[OVERHEAD_CONSOLE_DRAW]);
	Host_PrintOverhead("  Format ms:            %.2f\n\n", overhead_amounts[OVERHEAD_CONSOLE_FORMAT]);	
//	Host_PrintOverhead("+Authenticate ms:       %.2f\n", overhead_amounts[OVERHEAD_AUTHENTICATEFRAME]);
	Host_PrintOverhead("+GUI draw ms:          +%.2f\n", overhead_amounts[OVERHEAD_GUI_DRAW]);
	Host_PrintOverhead("+GUI frame ms:         +%.2f\n", overhead_amounts[OVERHEAD_GUI_FRAME]);
	Host_PrintOverhead("+Draw foreground ms:   +%.2f\n", overhead_amounts[OVERHEAD_DRAWFOREGROUND]);
	Host_PrintOverhead("+System Events ms:     +%.2f\n", overhead_amounts[OVERHEAD_SYSTEM_PROCESSEVENTS]);
	Host_PrintOverhead("+Exec Buf ms:          +%.2f\n", overhead_amounts[OVERHEAD_CMD_EXECBUF]);
	Host_PrintOverhead("+Sound ms:             +%.2f\n", overhead_amounts[OVERHEAD_SOUND]);
	Host_PrintOverhead("+Vid End Frame ms:     +%.2f\n", overhead_amounts[OVERHEAD_VID_ENDFRAME]);

	Host_PrintOverhead("=Frame Total ms:       =%.2f\n", overhead_amounts[OVERHEAD_FRAME]);
	Host_PrintOverhead("  Mem copy ms:          %.2f\n", overhead_amounts[OVERHEAD_MEMCOPY]);

//	OVERHEAD_COUNT(OVERHEAD_DRAWOVERHEAD);

//	Host_PrintOverhead("Perf draw ms:         %.2f\n\n", overhead_amounts[OVERHEAD_DRAWOVERHEAD]);
}

/*==========================

  Host_TranslateString

  used by game.dll

 ==========================*/

char *Host_TranslateString(const char *s)
{
	return _(s);
}

/*==========================

  Host_Frame

 ==========================*/

void Host_Frame (int time)  //time is the elapsed milliseconds since last Host_Frame()
{	
	static int curframe = 0;
	OVERHEAD_INIT;

	if (setjmp(safeStack))		//save the stack environment
	{
		frame_msec = 0;
		return;					//there was an error during the frame
	}

	Host_SetDateTime();

	//demo playback timing
	if (demo.playing)	//let it settle in for a couple of frames before advancing the time
	{
		if (demo.paused)
		{
			time = 0;
		}
		else
		{
			if (demo_fixedTime.integer)
			{
				time = demo_fixedTime.integer * demo.speed;
				if (time < 1)
					time = 1;
			}
			else if (demo_speedTest.integer)
			{
				time = 100;
			}
			else if (demo_makeMovie.integer)
			{
				time = 1000 / (demo_movieFPS.value > 0 ? demo_movieFPS.value : 30);
				if (time < 1)
					time = 1;
			}
			else
			{
				time *= demo.speed;
				if (time < 1)
					time = 1;
				if (time > 150)
					time = 150;		//don't let the timestep get too big, or demo playback can hitch badly
			}

			demo.time += time;
		}
	}
	
	frame_msec += time;

	if (frame_msec < 1000.0 / host_maxfps.value && 
		!((demo_speedTest.integer || demo_fixedTime.integer || demo.paused) && demo.playing))
		return;

	frame_msec *= timescale.value;
	//if (frame_msec < 1)
	//	frame_msec = 1;
	host_msec += frame_msec;

	if (localServer.active)
	{
		Server_Frame();
	}

	if (!dedicated_server.integer)
	{
		Host_AuthenticateFrame(server_address.string);

		if (localClient.cstate > CCS_DISCONNECTED
			&& localClient.cstate < CCS_CONNECTING)
		{
			Host_ConnectFrame();
		}
	}

	//process window events (like keyboard/mouse input)	
	System_ProcessEvents();

	//fprintf(stderr, "calling Cmd_ExecBuf()\n");
	//execute any commands we may have put into the command buffer
	Cmd_ExecBuf();

	if (demo.playing && !demo.startTime)
	{
		//return here so we can start at the proper time
		return;
	}

	if (!world_loading)
	{
		Vid_BeginFrame();
	}

	IRC_Frame();
	HTTP_Frame();

	Host_ActuallyRefreshFonts();
	
	if (!dedicated_server.integer)
	{		
		Client_Frame();		//run the client code
		Sound_Frame();
		GUI_Frame();

		if (gfx.integer)
		{
			//fixme
		/*	if (!world.loaded)
			{
				Draw_SetColor(black);
				Draw_Quad2d(0, 0, Vid_GetScreenW(), Vid_GetScreenH(), 0, 0, 1, 1, hostmedia.whiteShader);
			}*/
			if (font_adjustSize.modified 
				|| font_useHinting.modified 
				)
			{
				Host_ReloadFont();
				font_adjustSize.modified = false;
				font_useHinting.modified = false;
			}

			if (vid_gamma.modified || gfx_gammaMult.modified)
			{
				Vid_SetGamma(vid_gamma.value);
				vid_gamma.modified = false;
				gfx_gammaMult.modified = false;
			}
		
			if (localClient.cstate == CCS_DISCONNECTED || localClient.cstate == CCS_IN_GAME)
			{
				GUI_Draw();
				Client_DrawForeground();
			}
			else
			{
				Host_LoadingScreen("");
			}
				
			Host_DrawOverhead();
			
			Host_ResetOverhead();

			if (localClient.cstate == CCS_DISCONNECTED || localClient.cstate == CCS_IN_GAME)
			{
				Console_Format();
				Console_Draw();				
			}

			if (startup_string_brightness > 0)
			{				
				Host_DrawStartupStrings(1,1,1,startup_string_brightness > 1 ? 1 : startup_string_brightness);
				startup_string_brightness -= Host_FrameSeconds() * 0.5;
			}

			Vid_EndFrame();		//swap buffers

			if (demo_makeMovie.integer && demo.playing)
			{
				char filename[256];
				bitmap_t screenshot;

				System_CreateDir(demo_movieDir.string);

				File_GetNextFileIncrement(6, fmt("%s/frame", demo_movieDir.string), "png", filename, 255);

				Vid_GetFrameBuffer(&screenshot);
				
				Bitmap_WritePNG(filename, &screenshot);

				Bitmap_Free(&screenshot);
			}
		}		
	}
	else
	{
		console.active = true;
	}

	Net_StatsFrame();
	Scene_StatsFrame();

	curframe++;
	frame_msec = 0;		//reset frame time

	//note: construction of the scene is handled through the client,
	//which adds objects to the scene, sets a camera, and calls Scene_Render()

	OVERHEAD_COUNT(OVERHEAD_FRAME);
}

