// (C) 2003 S2 Games

// host.h


/*
	Host_Frame

	The 'host' is the machine the game is running on.  A local server 
	may be active, in which case Serv_Frame() is called (located in
	the Server side game code).  Client_Frame() (located in the Client
	side game code) is always called by this function)
*/

typedef struct
{
	residx_t	whiteShader;
	residx_t	consoleShader;
	residx_t	sysfontShader; //monospaced
	residx_t	nicefontShader; //relative-spaced
	residx_t	savagelogoShader;
	residx_t	loadingScreenShader;
	residx_t	loadingProgressShader;
	residx_t	alphaGradShader;
} hostmedia_t;

void	Host_Init();
void	Host_ShutDown();
void	Host_Frame(int time);
int		Host_Milliseconds();
float	Host_FrameSeconds();
int		Host_FrameMilliseconds();
float	Host_GetLogicFPS();
void	Host_GetCoreAPI(coreAPI_shared_t *api_shared, coreAPI_client_t *api_c, coreAPI_server_t *api_s, coreAPI_interface_t *api_i);
void 	Host_TestGameAPIValidity(clientAPI_t *capi, serverAPI_t *sapi, interfaceAPI_t *iapi);
bool    Host_GetCookie(char *cookie, int cookie_length);
void	Host_Connect(char *address);
void 	Host_Disconnect();
bool    Host_Authenticate(char *address);
void	Host_CloseLogFiles();

void    Host_LoadWorld(char *world, bool devMode);

void	Game_Error(const char *errormsg, ...);

void    Host_GetInfo(char *hostname, int port, bool extended);
char 	*Host_ParseServerInfo(char *ip, int port, packet_t *pkt);
char 	*Host_ParseExtServerInfo(char *ip, int port, packet_t *pkt);


//call this to register a font that needs to get refreshed (the bitmap has been updated)
//you can call this many times per frame, it will only get refreshed once
void    Host_RefreshFont(residx_t idx); 

residx_t	Host_GetSavageLogoShader();
residx_t	Host_GetWhiteShader();
residx_t	Host_GetSysfontShader();
residx_t	Host_GetNicefontShader();
residx_t	Host_GetLoadingScreenShader();

extern	hostmedia_t	hostmedia;
extern	bool	host_runLoadingFrame;
extern	clientAPI_t cl_api;
extern	serverAPI_t sv_api;
extern	interfaceAPI_t int_api;
extern	cvar_t	mod;
extern	int	DLLTYPE;
extern cvar_t dedicated_server;


//performance stat stuff
typedef enum
{
	OVERHEAD_FRAME,
	OVERHEAD_DRAWFOREGROUND,
	OVERHEAD_SCENE_RENDER,
	OVERHEAD_SCENE_CULL,
	OVERHEAD_SCENE_SELECTOBJECTS,
	OVERHEAD_SCENE_SETUP,
	OVERHEAD_SCENE_DRAWFOLIAGE,
	OVERHEAD_SCENE_DRAWWATER,
	OVERHEAD_SCENE_DRAWTERRAIN,
	OVERHEAD_SCENE_DRAWOBJECTS,
	OVERHEAD_SCENE_DRAWSPRITES,	
	OVERHEAD_SCENE_DRAWSKY,
	OVERHEAD_SCENE_DRAWSCENEPOLYS,
	OVERHEAD_SCENE_CLEANUP,
	OVERHEAD_SCENE_BUCKETSETUP,
	OVERHEAD_SCENE_OCCLUDERSETUP,
	OVERHEAD_TERRAIN_RENDER,
	OVERHEAD_TERRAIN_CULL,
	OVERHEAD_TERRAIN_REBUILD,	
	OVERHEAD_MODEL_SETUPMESHARRAYS,
	OVERHEAD_MODEL_ADDTOLISTS,
	OVERHEAD_MODEL_DEFORM,
	OVERHEAD_MODEL_SETUPVERTEXPOINTERS,
	OVERHEAD_MODEL_DRAWMESH,
	OVERHEAD_MODEL_DRAWRIGIDMESH,
	OVERHEAD_MODEL_DRAWSKINNEDMESH,
	OVERHEAD_MODEL_DRAWSTATICMESH,
	OVERHEAD_MEMCOPY,
	OVERHEAD_DRAWTRIANGLES,	
	OVERHEAD_SKELETAL,
	OVERHEAD_TRACEBOX_SERVER,
	OVERHEAD_TRACEBOX_CLIENT,
	OVERHEAD_LINK_STATIC,
	OVERHEAD_LINK_DYNAMIC_SERVER,
	OVERHEAD_LINK_DYNAMIC_CLIENT,
	OVERHEAD_CONSOLE_FORMAT,
	OVERHEAD_CONSOLE_DRAW,
	OVERHEAD_IO,
	OVERHEAD_NETWORK,
	OVERHEAD_SOUND,
	OVERHEAD_TEXTURES,
	OVERHEAD_SERVER,
	OVERHEAD_COUNT,
	OVERHEAD_SERVER_FRAME,
	OVERHEAD_SERVER_GAMELOGIC,
	OVERHEAD_SERVER_SENDPACKETS,
	OVERHEAD_CLIENT_FRAME,
	OVERHEAD_GUI_DRAW,
	OVERHEAD_GUI_FRAME,
	OVERHEAD_DRAWOVERHEAD,
	OVERHEAD_SCRIPTS,
	OVERHEAD_SYSTEM_PROCESSEVENTS,
	OVERHEAD_VID_ENDFRAME,
	OVERHEAD_CMD_EXECBUF,
	OVERHEAD_IRC,
	OVERHEAD_AUTHENTICATEFRAME,

	OVERHEAD_NUMTYPES
} overhead_enum;



extern cvar_t showPerf;
#define OVERHEAD_INIT   double __time;  if (showPerf.integer) __time = System_GetPerfCounter(); else __time = 0
#define OVERHEAD_COUNT(type) if (showPerf.integer) Host_Overhead((type), System_GetPerfCounter() - __time)

#define Host_PrintOverhead Console_Printf

void    Host_Overhead(overhead_enum type, double time);
void    Host_ResetOverhead();
char	*Host_TranslateString(const char *s);