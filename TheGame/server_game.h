// (C) 2001 S2 Games

// server_game.h

// definitions for client game code
// included in all server game modules

#include "game.h"

#define CORPSE_LINGER_TIME			  20000

#define	ITEM_SPAWNPOINT_SEARCH_STRIDE 0.2

#define	MAX_RIDERS						10

//from the bug where they actually collide twice when they hit a peon
#define PLAYER_WAYPOINT_DISTANCE_FUDGE_FACTOR 2000.0

typedef enum
{
	TARGTYPE_NONE,
	TARGTYPE_OBJECT,
	TARGTYPE_LOCATION
} targetTypes_enum;


#define MAX_BUILD_QUEUES	4

#define MAX_GOAL_PARAMS 3

//GOAL_SPAWN_BUILDING
#define GOALPARM_BUILDINGTYPE	0
#define GOALPARM_BUILDINGANGLE	1
//GOAL_DROPOFF_RESOURCES
#define GOALPARM_MINEINDEX		0

#define	SV_GET_INV_ITEM(obj)	((byte)(sl.clients[obj->base.index].ps.inventory[sl.clients[obj->base.index].ps.item]))


#define ALLOCATE(x) core.Allocator_Allocate(&allocator_##x, sizeof(x))
#define DEALLOCATE(x, pv) core.Allocator_Deallocate(&allocator_##x, pv)


typedef enum
{
	VOTE_ABSTAIN,
	VOTE_NO,
	VOTE_YES
} voteStatus_enum;

typedef struct client_s
{
	bool				active;
	sharedClientInfo_t	info;		//broadcast to all clients
	int					index;

	int					physicsTime;
	int					physicsClearTime;

	vec3_t				forward;
	vec3_t				right;

	playerState_t		ps;			//player position in world
	inputState_t		input;			//mouse/button input from client
	struct serverObject_s *obj;

	inputState_t	lastInput;	//last processed inputstate gametime
	int				lastVoteCalled;
	int				lastChatMsg;
	int				chatFloodCount;
	int				floodMuteEndTime;
	int				canCommandTime;
	int				teamLockEndTime;
	bool			mute;

	int				nextChaseTime;	//time when the player can spectate the next team member
	int				nextSurvivalReward;
	int				nextOfficerCmdTime;
	int				nextPingTime;
	int				nextNagTime;	//for demo

	int				timeOfDeath;

	int				ressurectEndTime;

	//store info when a player dies that will be restored when they revive
	byte			corpseUnitType;
	int				corpseInventory[MAX_INVENTORY];
	int				corpseAmmo[MAX_INVENTORY];

	phys_out_t		phys_out;

	waypoint_t      waypoint;

	//misc stats...these are reset at the beginning of a game
	clientStats_t	stats;

	bool			alreadySpawned; //for when they are in a garrison to see if they just died
									// or if they reentered a garrison after spawning
	bool			requestedRespawn;

	voteStatus_enum	voteStatus;

	float	driveYaw;
	int		aimX, aimY;		//mouse coords for vehicle aiming
	float	aspect;

	exclusionList_t exclusionList;

	//virtual client / proxy code
	bool			isVirtual;
	struct client_s	*inputRedirect;		//inputs go from self->inputRedirect
	struct client_s *psRedirect;		//playerstates go from self->psRedirect
	struct client_s *inputHasProxy;		//another client is dictating our inputs
	struct client_s *psHasProxy;		//another client is dictating our playerstate
} client_t;

typedef struct
{
	int		maxCarryBonus;
	int		repairRate;
	int		buildRate;
	int		fullhealth;
	int		meleeDamageBonus;
	int		meleeRangeBonus;
	int		maxStamina;
	float	blockPower;
	float	unitPierce;
	float	bldPierce;
	float	siegePierce;
}
adjustedStats_t;


typedef struct serverObject_s
{
	baseObject_t 	base;
//	solid_t			solid;

	client_t		*client;						//set for all objects with index < MAX_CLIENTS
	
	int             resources[MAX_RESOURCE_TYPES]; //how many resources this object is carrying

	int				flags;  //see the objectFlags enums

	byte			goal;
	int				goalParameters[MAX_GOAL_PARAMS];	//see GOALPARM_* defines above
	
	//melee strike info
	bool			hitObjects[MAX_OBJECTS];

	//movement vars
	int				nextDecisionTime;
	int				expireTime;
	bool			subtracted;			// have we csg subtracted oursevles from the pathpolys
	
	int				targetType;			//0 if no target. see TARGETTYPE defines above

	struct serverObject_s	*objectTarget; //object target
	vec3_t			posTarget;				//positional target

	vec3_t			velocity;
	vec3_t			forward;		//projectiles use this for soem of their actions
	vec3_t			oldpos;
	
	float			charge;		//for weapon effects caused by a charging wepon...
	int				chargeFlags;

	//the following two fields apply to buildings creating objects, or under construction
	int				itemConstruction;					//type of object being constructed.  if the building itself is under construction, itemConstruction == base.type
	int				itemConstructionAmountLeft;			//amount left to construct the item or the building, in milliseconds

	struct serverObject_s	*owner;
	struct serverObject_s	*enemy;
	struct serverObject_s	*twin;
	struct serverObject_s	*link;

	struct serverObject_s	*nextOutpost;

	struct ai_s*	ai;

	adjustedStats_t	adjustedStats;

	int				lastHeal;
	int				lastManaAdd;
	int				lastBuildTime;

	referenceObject_t	refObject;					//this field will get set if this object was spawned from a reference

	int				livesLeft;  					//how many more times should we respawn them?  
										//if 0, assume unlimited.  If they were at 1, we don't respawn them again
	int				stateExpireTimes[MAX_STATE_SLOTS];
	int				stateDamageTimes[MAX_STATE_SLOTS];
	int				stateInflictors[MAX_STATE_SLOTS];
	int				stateRadiusDamageTime[MAX_STATE_SLOTS];

	int				moneyCarried;
	int				lastClaimTime;
	int				lastNewEnemyTime;

	int				nextPickupTime;			//used for pickup items

	int				numRiders;
	int				riders[MAX_RIDERS];

	int				fogRevealTime;
	bool			hasShifted;

	int				nextStateTransition;
	int				nextState;

	bool			mineWarning;			//set for mines when they're getting low
	int				nextWarningTime;

	int				damageAccum;

	void            (*clearTarget)(struct serverObject_s *obj, int target_goal);
	void            (*targetPosition)(client_t *giver, struct serverObject_s *obj, float x, float y, int target_goal);
	void            (*targetObject)(client_t *giver, struct serverObject_s *obj, struct serverObject_s *target, int target_goal);
	void    		(*noticeObject)(struct serverObject_s *obj, struct serverObject_s *target);
	int	            (*damage)(struct serverObject_s *obj, struct serverObject_s *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags);
	void			(*kill)(struct serverObject_s *obj, struct serverObject_s *attacker, vec3_t pos, int weapon, int damageFlags);
	void            (*frame)(struct serverObject_s *obj);
} serverObject_t;

/*=============================================================================
  Game Script definitions
=============================================================================*/
typedef enum
{
	GS_ENTRY_NULL = 0,

	GS_ENTRY_PICKUP,
	GS_ENTRY_DROP,
	GS_ENTRY_TOSS,
	GS_ENTRY_ATTACH,

	GS_ENTRY_USE,
	GS_ENTRY_FIZZLE,
	GS_ENTRY_SPAWN,

	GS_ENTRY_IDLE,
	GS_ENTRY_IDLING,
	GS_ENTRY_ACTIVATE,
	GS_ENTRY_ACTIVE,
	GS_ENTRY_SLEEP,
	GS_ENTRY_SLEEPING,

	GS_ENTRY_IMPACT,
	GS_ENTRY_FUSE,
	GS_ENTRY_BACKFIRE,

	GS_ENTRY_WOUNDED,
	GS_ENTRY_DIE,

	NUM_GS_ENTRIES
}
gameScriptEntryPoint_t;

typedef enum
{
	GS_TARGET_SELF,
	GS_TARGET_TARGET,
	GS_TARGET_ENEMY,
	GS_TARGET_OWNER,
	GS_TARGET_LINK,

	GS_NUM_TARGETS
}
gameScriptTarget_t;

#define GS_PARAM_FLAG_PERCENT	0x0001
#define GS_PARAM_FLAG_EQUALS	0x0002
#define	GS_PARAM_FLAG_MULTIPLY	0x0004

typedef struct gameScriptParam_s
{
	int		flags;
	int		integer;
	float	value;
	char	*string;

	struct gameScriptParam_s *next;
}
gameScriptParam_t;

typedef struct gameScript_s
{
	int	functionID;
	gameScriptTarget_t	target;
	gameScriptParam_t	*param;

	struct gameScript_s	*next;
}
gameScript_t;

typedef enum
{
	GS_RESULT_PASS,
	GS_RESULT_FAIL,
	GS_RESULT_BREAK,
	GS_RESULT_ERROR,
	GS_RESULT_CHECK,

	GS_NUM_RESULTS
}
gameScriptResult_t;

typedef struct
{
	char	*name;
	gameScriptResult_t	(*function)(serverObject_t *self, int type, serverObject_t *target, gameScriptParam_t *params);
}
gameScriptDefinition_t;
/*=============================================================================
=============================================================================*/

typedef struct
{
	int					builderIndex; //index of the object to build the next item
	int					objectType; //what to build
} build_queue_t;


typedef struct
{
	int					index;									//index this struct is stored in the sl.teams[] array
	int					race;								//nomads, beasts, or none
	char				*name;								//optional name for the team
	int					commander;							//client number (sl.clients[n]) of the commander
	int					officers[MAX_OFFICERS];
	int					oldOfficers[MAX_OFFICERS];
	unitSelection_t		selection;							//current unit selection
	int					command_center;						//object index of this team's command center
	int					num_players;						//number of players on team
	int					lastResourceAdd;
	int					techLevel;			// total tech points team has accumulated
											//team is awarded <techPointVale> once per building type
	int					baseLevel;			//same as tech points, but this is used to track base upgrades

	int					resources[MAX_RESOURCE_TYPES];			//resources (stone, metal, supplies) currently available
	int					oldResources[MAX_RESOURCE_TYPES];		//resources last frame
	researchData_t		research[MAX_OBJECT_TYPES];				//number of each object/upgrade that the taem posses

	int					nextUnderAttackMessageTime;
	int					nextDemolishTime;

	serverObject_t		*outpostList;

	bool				orderGiven, orderFollowed;
	int					nextOrderReward;

	build_queue_t		buildQueue[MAX_BUILD_QUEUES];
	int					deployedItems[MAX_OBJECT_TYPES];
	int					oldDeployedItems[MAX_OBJECT_TYPES];
	
	int					nextRespawnWindow;			//gametime when players are able to respawn
} teamInfo_t;


#define MAX_SPAWNPOINTS		64		//spawnpoints for DM game


#define VOTABLE_ALL 0
#define VOTABLE_TEAM 1

typedef struct
{
	char	*description;
	char	*command;
	bool	(*voteStartFunc)(int initiatingClient, int argc, char *argv[]);
	void	(*votePassedFunc)();
	int		votetype;
	float	minPercent;
	cvar_t	*cvar;
} votable_t;

typedef struct
{	
	votable_t	*votable;
	client_t	*affectedClient;		//this will get set when the vote is called to prevent name changing to avoid a kick, etc
	teamInfo_t	*team;
	char	command[1024];
} voteInfo_t;


//just for fun... 
#define	EGG_FOURTH_OF_JULY	0x0001
#define	EGG_EASTER			0x0002
#define	EGG_CHRISTMAS		0x0004
#define	EGG_HALLOWEEN		0x0008
#define	EGG_CINCO_DE_MAYO	0x0010

typedef struct
{
	serverObject_t	objects[MAX_OBJECTS];
	int				lastActiveObject;

	client_t		clients[MAX_CLIENTS];
	int				numClients;

	teamInfo_t		teams[MAX_TEAMS];
//	int             num_teams;

	bool			voteInProgress;
	int				voteEndTime;
	voteInfo_t		voteInfo;
	int				votesYes;
	int				votesNo;

	int				gametype;

	int				gametime;
	int				frame_msec;
	float			frame_sec;
	int				framenum;

	bool			gameStarted;
	int				winningTeam;
	vec3_t			globalPos;

	vec3_t			worldBounds;

	bool			emptyServer;		//true if the server has no clients connected

	vec3_t			spawnPoints[MAX_SPAWNPOINTS];		//DM only
	int				numSpawnPoints;						//DM only

	int				status;
	char			statusString[256];
	int				endTime;			//if not 0, the time when we will transition into the next server status mode

	int				activeEasterEggs;

	client_t		*referee;
	bool			guestReferee;

	int				urgencyResetTime;

	client_t		*hero;
} serverLocal_t;

//held data is data that is persistent over server restarts
//this structure is written to right before the server restarts,
//and restored via various initialization functions

typedef struct
{
	int					team;
	bool				commander;
	bool				officer;
	int					officerslot;
	int					nextNagTime;
} heldClientData_t;

typedef struct
{
	bool				active;					//set to true if we should be looking at this data during the restart

	int					gametype;

	int					referee;
	bool				guestRef;

	heldClientData_t	clients[MAX_CLIENTS];
} heldServerData_t;


extern serverLocal_t sl;
extern heldServerData_t held;

extern coreAPI_server_t cores;

extern int npcs[];
extern int num_npcs;

extern cvar_t sv_fastTech;
extern cvar_t sv_goodieBags;

//sv_world.c
void 	SV_LocateAllTeamObjects();
void    SV_SpawnReferenceObjects();
float   SV_GetHighestSolidPoint(float x, float y);
float   SV_GetDistanceSq(int obj1, int obj2);
float   SV_GetDistanceSqVec3(vec3_t pos1, vec3_t pos2);
float   SV_GetDistanceSqVec2(vec2_t pos1, vec2_t pos2);
void    SV_SendDeletedWorldObjects(int clientnum);
void    SV_GetNPCSpawnpoints();

//sv_clients.c
void	SV_RefreshClientInfo(int clientnum);
void	SV_FreeClientObject(int clientnum);
void	SV_UpdateClientObject(int clientnum);
void	SV_InitClients();
void	SV_ClientVoiceChat(int clientnum, int category, int item);
void	SV_SetInitialPlayerState(int clientnum);
void	SV_ProcessClientInput(int clientnum, inputState_t inputState);
void	SV_FilterPredictedEvents(playerState_t *ps);
void    SV_UpdateObject(int obj_num);
void 	SV_SelectPlayerUnit(int clientnum, const char *unitname);
bool	SV_TraceBox(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface);
void	SV_PlayerStateToBaseObject(int clientnum, baseObject_t *obj);
void	SV_SetClientStatus(int clientnum, byte status);
void	SV_PutClientIntoGame(client_t *client);
void	SV_GiveMoney(int client, int money, bool tithe);
bool	SV_SpendMoney(int clientnum, int amount);
void	SV_SpawnPlayerFrom(client_t *client, int objidx);
void	SV_SpawnClient(int clientnum, vec3_t pos, vec3_t angle);
bool	SV_IsOfficer(int objnum);
int		SV_GetTeam(int objnum);
int		SV_GetRace(int objnum);
void	SV_GiveClient(client_t *client, int item);
void	SV_UpdateClientWaypoint(client_t *client);
void	SV_AdvanceClients();
bool	SV_IsValidSpawnObject(int team, int objindex, int clientnum);
void	SV_PlayerTargetObject(client_t *giver, serverObject_t *player, serverObject_t *object, int goal);
void	SV_PlayerTargetPosition(client_t *giver, serverObject_t *player, float x, float y, int goal);
void	SV_UseButton(client_t *client);
void	SV_SendScores(int sendTo, int teamfilter, int clientfilter);
bool	SV_SetDefaultUnit(client_t *client);
bool	SV_PlayerUnitRequest(client_t *client, int unittype);
bool	SV_ReviveCorpse(int corpsenum, float percent);
void	SV_InitClientData(int clientnum, const char *netsettings, unsigned int clan_id, bool restarting, bool virtualClient);
bool	SV_Eject(int clientnum);
void	SV_ResetInventory(client_t *client);
void	SV_RelinquishLife(client_t *client);
bool	SV_WaitingToBeRevived(client_t *client);

//sv_teams.c
void    SV_SendTeamInfo(int clientnum);
void    SV_SendOfficerList(int team);
bool    SV_ClientIsCommander(int clientnum);
bool    SV_SetCommander(int team, client_t *client, bool silent);
void    SV_SetExclusionList(int clientnum);
void    SV_CommanderTargetPosition(int clientnum, int object, float targetx, float targety, int goal);
void    SV_CommanderTargetObject(int clientnum, int object, int target, int goal);
void    SV_CommanderClearTargets(int clientnum, int object);
bool	SV_GetSpawnPointFromBuilding(serverObject_t *obj, vec3_t point, int objectType);
int     SV_CountTeamPlayers(int team);
void	SV_CreateTeam(int type, int commander);
void    SV_DestroyTeam(int team);
void	SV_ClientJoinTeam(client_t *client, int team, bool silent, bool force);
void    SV_PrintTeamResources();
void    SV_CommanderElect(client_t *client, teamInfo_t *team);
void    SV_CommanderResign(int clientnum, bool silent);
bool	SV_CanPurchaseObject(int team, int objtype, int buildertype, bool ignorecost);
bool    SV_PurchaseObject(int team, int type, serverObject_t *builder, bool ignorecost);
void	SV_SendCommanderTeamResources(int team, int clientnum);
void	SV_SendNotice(int clientnum, int message, int param, char *explanation);
void	SV_GiveUpgradeToTeam(int team, int upgrade);
bool	SV_IsItemResearched(int team, int item);
void	SV_ResetTeams();
void	SV_EndGame(int winningTeam);
void	SV_BroadcastNotice(int message, int param, char *explanation);
teamInfo_t 	*SV_GetTeamInfo(int clientnum);
void	SV_GroupMove(teamInfo_t *team, vec3_t position, int goal);
void	SV_GroupMoveToObject(teamInfo_t *team, int objidx, int goal);
void	SV_SendNoticeToTeam(int team, int message, int param, char *explanation);
void	SV_SendTeamMessage(teamInfo_t *teamptr, int sender, char *msg, bool sendToCommander, bool reliable);
void    SV_SendUnreliableTeamMessage(teamInfo_t *teamptr, int sender, char *msg, bool sendToCommander);
void	SV_MessageSelected(teamInfo_t *teamptr, int sender, char *msg);
void    SV_SendPrivateMessage(int sender, int receiver, char *msg);
bool	SV_ApplyStateToObject(int inflictor, int objindex, int state, int time);
void	SV_ApplyStateToRadius(int objindex, float radius, int state, int duration);
void	SV_RemoveStateFromObject(int objindex, int state);
void	SV_TeamFrame();
void    SV_InitTeams();
bool 	SV_GiveResourceToTeam(int team, int type, int amount);
void	SV_GiveResource_Cmd(int argc, char *argv[]);
void	SV_PromoteClient(client_t *client, int slot);
void	SV_DemoteOfficer(int teamnum, int slot);
void	SV_DemoteClient(client_t *client);
bool    SV_AddQueuedRequest(teamInfo_t *team, int builder_index, byte type);
bool    SV_RemoveQueuedRequest(int builderIndex, byte type);
int     SV_GetQueuedRequest(int builderIndex);
void    SV_UpdateTeamResearch(int t);
void	SV_RefreshTeamInfo(int teamnum);
void	SV_Announcer(int team, char *string);

//sv_objects.c
serverObject_t *SV_AllocObject(byte object_type, int team);
void    SV_FreeObject(int obj_num);
void	SV_FreeObjectFrame(serverObject_t *obj);
serverObject_t *SV_SpawnObject(int team, int object_type, vec3_t pos, vec3_t angle);
bool	SV_LinkToWorldObject(serverObject_t *obj);
void	SV_UnlinkFromWorldObject(serverObject_t *obj);
void    SV_AdvanceObjects();
int    SV_ObjectDamaged(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags);
void    SV_AdvanceMovingObject(serverObject_t *obj);
bool	SV_ObjectAtTarget(serverObject_t *obj);
void	SV_CalcObjectDistances();
float	SV_GetDistance(int obj1, int obj2);
serverObject_t *SV_CommanderPlaceBuilding(client_t *client, int type, vec3_t pos, vec3_t angle, bool ignorecost);
bool	SV_GetSpawnPointAroundObject(byte structureType, vec3_t structurePos, vec3_t structureAngle, int itemType, vec3_t spawnPoint);
bool    SV_GetRandomPointAroundObject(byte structureType, vec3_t structurePos, vec3_t structureAngle, byte objectTypeAroundStructure, vec3_t spawnPoint);
bool	SV_UnderConstruction(serverObject_t *structure);

serverObject_t  *SV_SpawnReferenceObject(referenceObject_t *obj, int type);
int     SV_FindClosestObject(int objectType, int object, int team, float max_dist, float *actual_dist);
int     SV_FindClosestObjects(int *objectTypes, int num_objectTypes, int object, int team, float max_dist, float *actual_dist);
int     SV_FindClosestObjectToPos(int objectType, vec3_t pos, int team, float max_dist, float *actual_dist);
int     SV_FindClosestObjectsToPos(int *objectTypes, int num_objectTypes, vec3_t pos, int team, float max_dist, float *actual_dist);
int     SV_FindObjectsInRadius(int *objectTypes, int num_objectTypes, float radius, vec3_t pos, int *objects, int numObjects);
void	SV_FillInBaseStats(serverObject_t *obj);

//sv_main.c
void	SV_SetUrgencyLevel(int level);
void	SV_BroadcastMessage(int sender, const char *msg);
void	SV_BroadcastTeamMessage(int sender, int team, const char *msg);
void	SV_SetReferee(client_t *client, bool guest);
void	SV_ClientEcho(int clientnum, const char *msg);
void	SV_SetGameStatus(int status, int endTime, const char *optParams);
void	SV_GiveBackFromClient(client_t *client, int slot);
bool	SV_GetPosition(int objnum, vec3_t outpos);
int	SV_GetOriginater(int objindex);
void	SV_ExtendTime(int msec);
int		SV_FindPlayer(char *name);
int		SV_FindPlayerSubString(char *name, int *numfound);
void	SV_SetControlProxy(client_t *client, int othernum);
void	SV_SetViewProxy(client_t *client, int othernum);
void	SV_SetFullProxy(client_t *client, int othernum);
void	SV_ReleaseProxy(client_t *client);

//sv_player.c
void	SV_PlayerCorpseFrame(serverObject_t *obj);
void	SV_PlayerClearTarget(serverObject_t *player, int goal);
void	SV_ActivateUpgrade(int source, int target, int tech);

//sv_events.c
byte	SV_EncodePositionOnObject(serverObject_t *obj, const vec3_t pos);
void	SV_DoOncePerFrameEvents(client_t *client);
void	SV_DoOncePerInputStateEvents(client_t *client);
void	SV_InitEvents();
void	SV_KillClient(client_t *client, serverObject_t *killer, vec3_t damageSource, int weapon, int attackDamage, int damageFlags);
void    SV_PushObjectAround(serverObject_t *target, vec3_t origin, int velocity, bool changeZ);
void    SV_DamageTarget(serverObject_t *attacker, serverObject_t *target, vec3_t pos, int weapon, int damage, int damageFlags);
void	SV_DamageRadius(serverObject_t *attacker, vec3_t origin, int weapon, float radius, int attackDamage, int damageFlags);
void	SV_DeathEvent(serverObject_t *obj, byte event, byte param, byte param2, int msec, bool render);
void	SV_FreeAfterEvent_Frame(serverObject_t *obj);
bool	SV_IsCloaked(serverObject_t *looker, serverObject_t *target);
serverObject_t	*SV_GenerateCorpse(client_t *client, int animState);
void	SV_UnitHeal(int objindex);

//sv_experience.c
void	SV_AdjustStatsForCurrentLevel(serverObject_t *obj, int level);
void	SV_AddExperience(client_t *client, experienceRewards_enum rewardType, int param, float multiplier);
void	SV_SubtractExperience(client_t *client, int amount);
void	SV_InitExperience();

//sv_phys_object.c
bool    SV_Phys_ObjectOnGround(serverObject_t *obj, traceinfo_t *trace);

//sv_phys_object.c
void    SV_Phys_AddEvent(serverObject_t *obj, byte event, byte param, byte param2);

//sv_weapons.c
void	SV_Attack(serverObject_t *owner, int wpState);
void	SV_InitWeapons();
void	SV_Projectile_Frame(serverObject_t *proj);
void	SV_WeaponFire(serverObject_t *owner, vec3_t dir, int weapontype, vec3_t	target);

//sv_items.c
serverObject_t	*SV_SpawnPickupItem(byte objtype, int team, vec3_t pos, vec3_t angle);
void	SV_UseItem(int clientnum, int itemnum);
void	SV_InitItems();
void	SV_SpawnGoodieBag(serverObject_t *target, const vec3_t pos, client_t *attacker);
char	*GetGoodieBagName();
int 	SV_ItemDamage(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags);
void	SV_ItemKill(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int damageFlags);
void	SV_ItemIdle(serverObject_t *item, int duration, int nextstate);
void	SV_ItemActivate(serverObject_t *item, int duration, int nextstate);
void	SV_ItemSleep(serverObject_t *item, int duration, int nextstate);
void	SV_ItemSleepFrame(serverObject_t *item);

//sv_ai_unit.c
bool	SV_AISpawnBuilding(serverObject_t *obj, byte obj_type, vec3_t pos, vec3_t angle);
serverObject_t 	*SV_SpawnAIUnit(byte type, int team, vec3_t pos, vec3_t angle);
void    SV_AITargetObject(client_t *giver, serverObject_t *obj, serverObject_t *target, int goal);
void    SV_AITargetPosition(client_t *giver, serverObject_t *obj, float x, float y, int goal);
void    SV_AINoticeObject(serverObject_t *obj, serverObject_t *target);
void	SV_AIClearTarget(serverObject_t *obj, int goal);


//sv_buildings.c
void	SV_ClaimBuilding(serverObject_t *claimer, serverObject_t *building);
serverObject_t	*SV_SpawnBuilding(byte structure_type, int team, vec3_t pos, vec3_t angle);
bool	SV_MineResourcesFromBuilding(serverObject_t *miner, serverObject_t *building, int amount);
serverObject_t	*SV_FindNearestDropoffPoint(serverObject_t *obj);
void	SV_DestroyBuilding(serverObject_t *obj);
bool	SV_RepairBuilding(serverObject_t *worker, serverObject_t *building, int amount);
void	SV_SetupCommandCenter(serverObject_t *obj);
bool	SV_ConstructBuilding(serverObject_t *worker, serverObject_t *building, int amount);
void	SV_ConstructObject(serverObject_t *builder, int objType);
bool	SV_BuildingCanFit(int structure_type, vec3_t pos, vec3_t angle, int team, char* reason);
bool	SV_EnterBuilding(int clientnum);
void	SV_InitBuildings();
bool	SV_DropoffResources(serverObject_t *obj, serverObject_t *building);
void	SV_CancelConstruction(int objindex);

//sv_vote.c
void	SV_ClientCastVote(client_t *client, int vote);
void	SV_CallVote(int initiatingClient, int argc, char *argv[]);
void	SV_TallyVote();
void	SV_VoteFrame();
void	SV_InitVotes();

//sv_gamescript.c
void	SV_InitGameScript();
bool	SV_GameScriptExecute(serverObject_t *self, serverObject_t *other, int type, gameScriptEntryPoint_t entry);
bool	SV_GameScriptLoad(gameScript_t *script[], const char *filename);

//sv_vclient.c
void	SV_InitVirtualClients();

//object_config.c

//only the server externs the objData array, even though it exists in shared code
//this is because the client should always reference objectData via CL_ObjectType()
//same goes for the GetObjectByType and related functions

extern objectData_t	objData[MAX_OBJECT_TYPES];
extern char *objectNames[MAX_OBJECT_TYPES];	//this is generated from the objectData array on init
objectData_t	*GetObjectByName(const char *name);
objectData_t	*GetObjectByType(int index);
int				GetObjectTypeByName(const char *name);
bool			IsItemType(int type);
bool			IsWeaponType(int type);
bool			IsMeleeType(int type);
bool			IsUpgradeType(int type);
bool			IsUnitType(int type);
bool			IsCharacterType(int type);
bool			IsBuildingType(int type);
bool			IsMobileType(int type);
bool			IsWorkerType(int type);
int				CanPutInInventory(int unittype, int itemtype);
