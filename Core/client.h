// (C) 2003 S2 Games

// client.h

// local client

#define	CLIENT_MESSAGEBUF_SIZE	16384
#define	MAX_GAMECMDS_PER_FRAME	32

//client-side network protocol
#define	CNET_PROTOCOL_VERSION		2

#define localClient lc

typedef enum
{
	CNET_KEEPALIVE = 0x01,
	/*
	*/
	CNET_MESSAGE,
	/*
		char message[];
	*/	
	CNET_INPUTSTATE,
	/*
		short	angles[3];
		byte	movement;
		byte	buttons;
	*/

	CNET_DISCONNECT,
	/*
	*/
	CNET_CVAR,
	/*
		char *name;
		char *value;
	*/
	CNET_REQUEST_STATE_STRINGS,
//	CNET_HASHCOMPARE,
	CNET_OK_TO_START,
	/*
	*/
	CNET_NETSETTINGS,
//	CNET_CLEAR_UPDATE
} cnetCommands_enum;

//typedef 

	//

#define MAX_CL_UPDATE_HISTORY 64
#define NUM_CL_NET_OBJECTS	(MAX_OBJECTS * 12)

typedef struct
{
	int					conn_start;

	int					netdriver;

	int					cstate;		//see CCS_* defines in savage_types.h

	int					clientnum;	

	char				serverAddr[256];
	char				address[256];

	unsigned int		lastValidFrame;
	unsigned int		lastServerFrame;
	int					lastServerPacketTime;
	networkUpdate_t		frames[MAX_CL_UPDATE_HISTORY];
	baseObject_t		net_objects[NUM_CL_NET_OBJECTS];
	unsigned int		cur_net_object;		//current unmodded index into circular net_objects array
	bool				validFrame;			//if false, curFrame won't get copied to the frame history at the end of the packet read
	bool				finishedFrame;
	int					continuedCount;
	networkUpdate_t		curFrame;			//temporary data structure used to read packet data
	networkUpdate_t		*fromFrame;
	int					invalidFrameCount;


	int					lastSendTime;

	int					gametime;		//last server timestamp + elapsed MILLISECONDS since

	float				objectUpdateTime;
	bool				objectUpdateFlag;

	playerState_t		playerState;

	baseObject_t		*gameobjs[MAX_OBJECTS];
	int					num_gameobjs;

	inputState_t		inputStates[INPUTSTATE_BUFFER_SIZE];
	unsigned int		inputStateNum;
	unsigned int		firstInputNum;

	stateString_t		stateStrings[MAX_STATE_STRINGS];
	int					bigstringIndex;
	int					bigstringLength;
	char				*bigstringBuffer;
	int					bigstringBufferSize;

	bool				softRestart;			//set to true if we're in the middle of a soft restart

	bool				connectionProblems;
	int					connectionProblemsStartTime;

	int					server_timestamp;
	int					last_timestamp;

	int					totaldelta;

	int					xorbits;
	bool				worldPending;
	bool				gettingFile;

	int					lastKeepalive;

	int					framenum;

	packet_t			inputCommand;

	bool				initialConnect;
} localClientInfo_t;

typedef enum
{
	DEMOCMD_PACKET,
	DEMOCMD_EOF = 255
} democmds_enum;

typedef struct
{
	file_t *file;
	bool recording;
	int waiting;		//don't write to the file until this is true
	bool playing;		//demo playing back
	int startTime;
	int time;
	int framecount;
	bool paused;
	float speed;
} clientDemoInfo_t;


extern localClientInfo_t localClient;
extern clientDemoInfo_t demo;

void	Client_Init();
void	Client_Reset();
void	Client_Frame();
void	Client_Disconnect();
void	Client_Connect(char *addr);

//used to draw graphics that should always be displayed
//in front of everything else (like the cursor)
//called after Client_Frame()
void	Client_DrawForeground();
void	Client_HandleMouseInput();

//the following functions are used by the game code to retrieve client information
int		Client_GetConnectionState();
bool	Client_GetServerMessage(char *buf);

//network functions called from game code
void	Client_SendMessageToServer(char *buf);
void	Client_RequestStateString(int id, int num);
void    Client_SendCvarPtr(cvar_t *var);

void	Client_GameObjectPointer(void *base, int stride, int num_objects);
void	Client_GetInputState(unsigned int num, inputState_t *is);
unsigned int Client_GetCurrentInputStateNum();
void	Client_ModifyInputStateItem(byte item);		//right now 'item' is the only thing we need to modify
void	Client_ModifyInputStateAngles(short pitch, short yaw);
void	Client_GetRealPlayerState(playerState_t *ps);

int		Client_GetNumTeams();
int		Client_CountTeamMembers(int team);
void	Client_SendTeamRequest(int type);
void	Client_SendTeamJoin(int type);
int		Client_GetOwnClientNum();
void	Client_GetStateString(int id, char *buf, int size);

int		Client_GetConnectionState();
bool    Client_ConnectionProblems();

bool	Client_IsPlayingDemo();

//this function is located in parsestats.c
bool    Client_ProcessStats(packet_t *pkt);

int     WOG_GetNextObjectInGridSquare_Client(int *objectTypes, int num_objectTypes, int x, int y, int *offset);
int     WOG_FindClosestObjects_Client(int *objectTypes, int num_objectTypes, float min_dist, float max_dist, vec3_t pos, int team, float *closest_dist);
bool    WOG_GetGridSquareCoords_Client(int grid_x, int grid_y, vec2_t topleft, vec2_t bottomright);
bool    WOG_GetGridSquareForPos_Client(vec3_t pos, ivec2_t gridpos);
