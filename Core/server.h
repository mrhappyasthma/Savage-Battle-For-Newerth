// (C) 2003 S2 Games

// server.h

#define		LOBBYDATA_MAXSIZE 				512
#define		MAX_CLIENT_GAMECMDS 			256
#define		CLIENT_TIMEOUT_TIME				1200


//OR'd in with the object index during an object update
#define OBJECT_HAS_EVENTS	0x800			//object has 1 or more events
#define OBJECT_FREE			0x1000			//object should be freed
#define OBJECT_END_UPDATE	0x2000			//update finished
#define OBJECT_CONTINUED	0x4000			//update is continued in next message
#define OBJECT_NOT_VISIBLE	0x8000			//object left network visibility

#define STATESTRING_REQUEST_ONLY	0x0001

typedef struct
{
	char	*string;
	int		modifyCount;
	int		length;			//so we don't need to keep recomputing it during sending
	int		memsize;		//memory size allocated
	int		lastModifiedFrame;
	int		flags;
} stateString_t;

typedef enum
{
	SNET_MESSAGE = 0x01,				//messages from the server admin or from other clients (chat, game code specific commands, etc)
	/*
		char	message[];
	*/
	SNET_DISCONNECTING,				//server is disconnecting
	/*
	*/
	SNET_UPDATE_OBJECTS,				//server is sending us a delta update of one or more objects
	SNET_UPDATE_PLAYERSTATE,		//server is sending us a delta update of our player state
	SNET_FRAME,
	SNET_FRAME_CONTINUED,
	/*
		int			timestamp;
	*/
	SNET_FREEOBJECT,
	/*
		short		index;
	*/
	SNET_CVAR,
	/*
	    char		*name;
		char		*value;
	*/
	SNET_KICK,
	/*
		char	*reason;
	*/
	SNET_SMALL_STRING,
	/*
		short	id;
		char	*string;
	*/
	SNET_BIG_STRING_START,
	/*
		short	id;
		int		size;		
	*/
	SNET_BIG_STRING_CONTINUE,
	/*
		short	id;
		int		offset;
		char	*data;  //NOT NULL TERMINATED
	*/	
	SNET_STARTING_STATE_STRINGS,
	SNET_FINISHED_STATE_STRINGS,
//	SNET_FINISHED_HASHCOMPARE,
	SNET_GOOD_TO_GO,
	/*
	    byte    clientnum;
	*/
	SNET_FRAME_FULL
} snetCommands_enum;


#define NUM_PING_COUNTS 8
#define MAX_SV_UPDATE_HISTORY	64		//this is how many networkUpdates we'll store for delta comparison

//this is the structure used for sending any changes in object positions, angles, etc, over the network
typedef struct
{
	unsigned int		framenum;
	int					time;			//server timestamp of this update

	unsigned int		start_object;	//non-modded index into client->net_objects circular buffer
	int					num_objects;	//number of objects in this update

	playerState_t		playerstate;	//playerstate structure (no need for a circular buffer, as there's only 1 per frame)
} networkUpdate_t;

#define MAX_CLIENT_INPUTSTATES	20		//max allowed inputstates per frame

#define SERVER_MAX_FPS	60

#define MAX_UPDATE_BUF 8192
#define NUM_SV_NET_OBJECTS (MAX_OBJECTS * 2)

typedef struct
{
	netconnection_t net;
	int				clientid;
	
	int				curInputTime;
	inputState_t	curInputState;
	int				inputStateNum;	//number of inputstates we have received this frame
	
	playerState_t	playerState;	//updated by gamecode with Server_UpdatePlayerState
	int				cstate;
	char			message[MAX_MESSAGE_LENGTH];
	int				num_gamecmds;
	bool			active;
	char			requestedName[CLIENT_NAME_LENGTH];
	unsigned int	clan_id;
	unsigned int	user_id;
	unsigned int 	guid;
	char			netSettings[1024];
	bool			restarting;

	int				pingCounts[NUM_PING_COUNTS];	

	unsigned int	lastMsgFrame;		//for computing timeouts

	int				maxPacketSize;		//the max packet size we can send per server frame to the client
	int				bps;				//max bytes per second the client wants to receive
	int				netfps;

	//each client stores a history of network updates that are used for delta compression
	networkUpdate_t	updates[MAX_SV_UPDATE_HISTORY];		//circular buffer, for comparing objects between frames
	int				num_updates_sent;
	int				num_objects_sent;			//total number of objects written to the client, and the current index in the circular buffer	
	int				cur_framenum;
	byte			*exclusionList;
	int				nextUpdateTime;

	//if an update doesn't finish sending in one frame, the remainder is delayed until the next server frame
	//these three variables store the info needed between frames to facilitate that mechanism
	networkUpdate_t	*update_old;
	networkUpdate_t	*update_new;
	int				update_old_marker;				//if not 0, need to continue the previous frame's update
	int				update_new_marker;				//if not 0, need to continue the previous frame's update
	int				continuedCount;
	unsigned int	old_diff_frame;
	baseObject_t	*net_objects;					//circular buffer used to save baseobjects for delta updates
	int				num_net_objects;
	unsigned int	cur_obj;						//current index into net_objects

	
	bool			just_connected;

	bool			resetVisibility;

	//if stateStringQueueSize > 1, we need to communicate state strings to the client
	int				stateStringQueue[MAX_STATE_STRINGS];
	int				stateStringQueueSize;
	char			*bigstring;		//used for large strings that won't fit in one packet
	char			*bigstringOffset;
	int				bigstringIndex;
	int				bigstringLength;

	bool			reliableHack;

	bool			demoPlayer;

	int				gameConnectTime;

	int				xorbits;

	int				lastSendTime;

	//bandwidth estimation
	int				sendSize[SERVER_MAX_FPS];
	int				bandResetTime;
} clientConnectionInfo_t;



typedef struct
{
	bool				active;					//if false, we are not hosting the game
	bool				dedicated;
	char				name[256];	
	int					netdriver;
	int					port;

	unsigned int		framenum;

	int					nextHeartbeatTime;

	clientConnectionInfo_t	clients[MAX_CLIENTS];
	int					numClients;				//number of clients currently connected
	int					maxClients;				//maximum allowed to connect

	int					just_connected;			//clientnum of a client who just connected to the server
		
	//worldData_t		worldData;

	serverStatus_enum	status;					//see serverStatus_t
//	serverState_t	state;

	int					num_gameobjs;
	baseObject_t		*gameobjs[MAX_OBJECTS];		//pointers to the game code maintained array of objects

	baseObject_t		*net_object_pool;
	unsigned int		pool_size;
	//int				num_net_objects;

	byte				*clientExclusionLists[MAX_CLIENTS];

	object_grid_t 		grid;	

	stateString_t		stateStrings[MAX_STATE_STRINGS];
} server_t;

extern server_t	localServer;

extern cvar_t	sv_port;


void	Server_Init();

void    Server_RegisterVars();

void	Server_Reset();

void	Server_Start(const char *name, bool softrestart);

void	Server_Disconnect();

bool	Server_IsClientConnected(netaddr_t *clientAddr, int clientid, int *clientnum);

bool	Server_IsActive();

void	Server_Frame();

void	Server_SetStatus(int status);

int		Server_GetStatus();

int		Server_GetNumClients();

void	Server_BroadcastMessage(int sender, char *msg);

void    Server_SendMessage(int sender, int client, char *msg);

void    Server_SendUnreliableMessage(int sender, int client, char *msg);

void    Server_BroadcastCvar(cvar_t *var);

char	*Server_GetClientName(int clientnum);

void	Server_SpawnObject(void *objdata, int index);

void	Server_FreeObject(void *objdata);

void	Server_GameObjectPointer(void *base, int stride, int num_objects);

void	Server_ClientExclusionListPointer(void *base, int stride, int num_clients);

void	Server_UpdatePlayerState(int clientnum, playerState_t *ps);

void	Server_GetInputState(int clientnum, int inputnum, inputState_t *is);

int		Server_GetNumInputStates(int clientnum);

bool	Server_IsClientActive(int clientnum);

void    Server_SendInfo(netconnection_t *nc, bool extended);

char 	*Server_GetClientCookie(int client_id);
unsigned int Server_GetClientGUID(int client_id);
unsigned int Server_GetClientUID(int client_id);

void	Server_SendShutdown();

bool    Server_LoadWorld(char *worldName);

char    *Server_CheckForVIP(char *cookie);

short	Server_GetPort();

void	Server_SetStateString(int id, const char *string);
char	*Server_GetStateString(int id, char *buf, int size);
void	Server_SetRequestOnly(int id);
void	Server_SendRequestString(int clientnum, int id);

void	Server_SendHeartbeat();

bool    Server_StartStats(char *server_stats);
bool    Server_AddClientStats(char *cookie, char *user_stats);
bool    Server_SendStats();

bool    Server_WriteStatsHeader(packet_t *pkt);

void    Server_NotFirewalled();

int		Server_GetClientPing(int clientnum);
const char*	Server_GetClientIP(int clientnum);

int	Server_AllocVirtualClientSlot();

#ifdef SAVAGE_DEMO
bool	Server_IsDemoPlayer(int clientnum);
#endif

void	Server_SendOutputToClient(char *s);

//indirection to allow non-core server code to do WOG calls
int     WOG_FindClosestObjects_Server(int *objectTypes, int num_objectTypes, float min_dist, float max_dist, vec3_t pos, int team, float *closest_dist);
int     WOG_FindObjectsInRadius_Server(int *objectTypes, int num_objectTypes, float radius, vec3_t pos, int *objects, int numObjects);
