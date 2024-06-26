// (C) 2001 S2 Games

// game.h

//============================================================================
#include "savage.h"

#define S2_INTERNAL_DEV_TESTING

#ifdef SAVAGE_DEMO
//this is precisely what is needed to load all the human objects, the mines, and the chiprel/oschore/monkit
#define MAX_OBJECT_TYPES	117
#define	FIRST_OBJECT_TYPE	1
#define	LAST_OBJECT_TYPE	116

#else	//SAVAGE_DEMO

#define MAX_OBJECT_TYPES	256
#define	FIRST_OBJECT_TYPE	1
#define	LAST_OBJECT_TYPE	255
#endif	//SAVAGE_DEMO

#define B_ATTACK		BUTTON1
#define B_BLOCK			BUTTON2
#define	B_USE			BUTTON3
#define B_SPRINT		BUTTON4
#define	B_WALK			BUTTON5

#define STRUCTURE_SCALE	structureScale.value

#define MAX_SLIDE_COLLISIONS	8

#define	DEFAULT_GRAVITY	20

#define	MELEE_CHARGE_TIME	2000

//global scaling value for all physics units
#define PHYSICS_SCALE 20.0

#define	MAX_WORLD_OBJECTS_DELETED 100
#define	MAX_NPC_SPAWNPOINTS 60

//gui stuff
#define MAX_CONTEXTMENU_OPTIONS 5

#define MAX_TECHTREE_ITEMS (MAX_OBJECT_TYPES)

//this is for nameable configs, like effects
#define	MAX_NAME_LEN	64

//for configs and such only, not globally
#define MAX_FILENAME_LENGTH 64

#define MAX_OFFICERS			3
#define	MIN_CLIENTS_PER_OFFICER	5


typedef enum
{
	REQUEST_MONEY = 1,	//player is just asking for gold
	REQUEST_OBJECT,		//player wants a particular object (unit/weapon/item)
	REQUEST_POWERUP,	//player is asking for a commander issued power
	REQUEST_PROMOTE,	//player wants to be an officer
	REQUEST_STRUCTURE,	//player is asking commander to build something

	NUM_REQUEST_TYPES
}
requestTypes_enum;

// shared game cvars
extern cvar_t g_allUnitsAvailable;
extern cvar_t g_allWeaponsAvailable;


//============================================================================
// intrange, floatrange
//
// data types used to store a min/max value
// macros can be used to easily set these values, as well as retrieve a 
// random number within the range
//============================================================================
typedef struct
{
	int min, max;
}
intrange;

typedef struct
{
	float min, max;
}
floatrange;

#define	SETRANGE(r,n,x) {(r).min = (n); (r).max = (x);}
#define	LOCKRANGE(r,n)  {(r).min = (r).max = (n);}
#define	GETRANGE(r)		((r.min < r.max) ? M_Randnum(r.min, r.max) : r.max)
//============================================================================

//=============================================================================
// configVar_t
// structure that can describe the variables of another structure, allowing them
// to be referenced through a generic set of console commands
//=============================================================================
#define	NO_LIMIT	9999999

enum
{
	OBJCLASS_NULL,
	OBJCLASS_WEAPON,
	OBJCLASS_BUILDING,
	OBJCLASS_UNIT,
	OBJCLASS_ITEM,
	OBJCLASS_UPGRADE,
	OBJCLASS_MELEE,
	
	NUM_OBJCLASSES
};

#define	NUL	0x01
#define WPN 0x02
#define BLD 0x04
#define	UNT 0x08
#define ITM 0x10
#define UPG 0x20
#define MLE	0x40

#define ALL 0xff

enum
{
	T_INT,				//integer
	T_INTRANGE,			//integer range (min, max)
	T_FLOAT,			//float
	T_FLOATRANGE,		//float range (min, max)
	T_STRING,			//fixed length string
	T_FLAGS,			//integer, uses a string array to set bit fields
	T_DEFAULT,			//sets the default value for a field, then is ignored.
						//this is good for setting a default value for a field within a T_VARLIST
	T_DYNSTRING,
	T_MODEL,			//a string, but it also loads a skin and anim data
	T_ARRAY,			//takes two args, first arg is index into array of ints
};

typedef struct
{
	char	name[MAX_NAME_LEN];	//what the var is reffered to as in the console
	int		min, max;			//min and max values.  For T_STRING, max is max length.  For T_FLAGS, max is number of flags
	char	def[_MAX_PATH];		//default value
	char	**nameList;			//array of names to index value, or names of flags
	int		type;				//defines how var is handled
	int		offset;				//use offsetof() macro
	int		types;				//which objclasses should this be displayed for?
}
configVar_t;

#define offsetof(s,m) (int)&(((s *)0)->m)

#define	NUM_VARS(dat)	(sizeof(dat)/sizeof(configVar_t))

//get a pointer
#define	INTP_OFFSET(dat,var,i)			((int*)((char*)(dat) + var[i].offset))
#define	FLOATP_OFFSET(dat,var,i)		((float*)((char*)(dat) + var[i].offset))
#define	INTRANGEP_OFFSET(dat,var,i)		((intrange*)((char*)(dat) + var[i].offset))
#define	FLOATRANGEP_OFFSET(dat,var,i)	((floatrange*)((char*)(dat) + var[i].offset))
#define	STRINGP_OFFSET(dat,var,i)		((char*)((char*)(dat) + var[i].offset))
#define	VARLISTP_OFFSET(dat,var,i)		((configVar_t*)((char*)(dat) + var[i].offset))

//get the value
#define	INT_OFFSET(dat,var,i)			(*(int*)((char*)(dat) + var[i].offset))
#define	FLOAT_OFFSET(dat,var,i)			(*(float*)((char*)(dat) + var[i].offset))
#define	INTRANGE_OFFSET(dat,var,i)		(*(intrange*)((char*)(dat) + var[i].offset))
#define	FLOATRANGE_OFFSET(dat,var,i)	(*(floatrange*)((char*)(dat) + var[i].offset))
#define	STRING_OFFSET(dat,var,i)		(*(char*)((char*)(dat) + var[i].offset))
#define	VARLIST_OFFSET(dat,var,i)		(*(configVar_t*)((char*)(dat) + var[i].offset))

#define	POP_ARG		{argc--; argv = &argv[1];}
//=============================================================================

//=============================================================================
//
// GAME DATA STRUCTURES
//
//=============================================================================

//=============================================================================
// Resource Data
//=============================================================================
#ifdef SAVAGE_DEMO
#define	MAX_RESOURCE_TYPES	7
#else	//SAVAGE_DEMO
#define	MAX_RESOURCE_TYPES	16
#endif	//SAVAGE_DEMO

typedef struct
{
	char	name[MAX_NAME_LEN];
	char	*description;
	char	*icon;
	float	red, green, blue;
}
resourceData_t;

extern resourceData_t	resourceData[MAX_RESOURCE_TYPES];
extern char				*resourceNames[MAX_RESOURCE_TYPES];
//=============================================================================

//=============================================================================
// Race Data
//=============================================================================
#ifdef SAVAGE_DEMO

#define FIRST_RACE	1
#define LAST_RACE	1
#define MAX_RACES	2

#else	//SAVAGE_DEMO

#define FIRST_RACE	1
#define LAST_RACE	2
#define MAX_RACES	LAST_RACE+1
#if (LAST_RACE < FIRST_RACE)
#error LAST_RACE must be >= FIRST_RACE
#endif

#endif	//SAVAGE_DEMO

typedef struct 
{
	bool	active;				//true if a config has been loaded for this race

	int		index;

	char	name[MAX_NAME_LEN];	//internal reference
	char	*description;		//displayd name
	char	*baseBuilding;		//name of the main building for this race
	int		resources;			//bit flags indicating which resources are used
	int		currency;			//resource number that players use to purchase items
	int		officerState;		//state to apply whe na player is promtoed to officer status
	char	*ammoDrop;
}
raceData_t;

extern configVar_t	raceData_desc[];
extern raceData_t	raceData[MAX_RACES];
extern char			*raceNames[MAX_RACES];

raceData_t	*GetRaceDataByName(const char *name);

//=============================================================================

//=============================================================================
// State Data
//=============================================================================
#define	MAX_STATES	255

#define	MODIFIER(a)	float	a##Add; float a##Mult;

typedef struct
{
	int		index;
	char	name[MAX_NAME_LEN];
	bool	active;
	char	*icon;			//this is displayed on the player's HUD to indicate the state applied to them
	int		slot;			//determines which slot to place this state in
							//if another state with the same slot preference is applied, it overwrites the old state
	int		priority;		//a state with lower priority will not overtake the slot of a state with higher priority

	MODIFIER(damage)		//
	MODIFIER(armor)			//
	MODIFIER(speed)			//
	MODIFIER(jump)			//
	MODIFIER(attackSpeed)	//
	MODIFIER(health)		//
	MODIFIER(regenRate)		//
	MODIFIER(regen)

	float	staminaRegenAdjust;

	int		damage;
	int		damageFrequency;
	int		damageFlags;

	float	cloak;			//exclude from other players view
	int		markEnemies;	//mark enemies
	bool	lockdownTech;	//prevents anything that has a tech associated with it from activating
	bool	splashProtect;	

	int		isVulnerable;	//this state can be removed by an attack that strips states

	int		radiusDamage;
	int		radiusDamageFreq;
	float	radius;			//affect others nearby
	int		radiusTargets;	//flags target types
	int		radiusState;	//state to apply to those within radius

	//this model is renedered over the players model while the state is active
	char	*model[5];				//***************************************
	char	*skin[5];				//***************************************
	char	*bone[5];
	bool	useCharAnim[5];
	char	*singleShader;

	int			effect;
	intrange	effectPeriod;
}
stateData_t;

extern configVar_t	stateData_desc[];
extern stateData_t	stateData[MAX_STATES];
extern char			*stateNames[MAX_STATES];
//=============================================================================

//=============================================================================
// Melee Attack Data
//=============================================================================
typedef struct
{
	//all times specified in milliseconds

	int			time;			//the amount of time it takes for the attack to complete
	intrange	impact;			//the time at which we should test for an impact
	int			lunge;			//time to start a lunge
	int			flurry;			//opening of window of oportunity for a flurry
	int			lungeTime;		//duration of lunge
	int			flurryTime;		//duration of flurry opportunity

	//these are only used for AI now
	int		damage;
	int		damageFlags;

	int		xLunge;			//a 'rightward' momentum to give the player when he performs the attack (or 'leftward', if negative)
	int		yLunge;			//a forward momentum to give the player when he performs the attack (or backward, if negative)
	int		range;			//distance ahead of player to trace
	float	horzKick;		//indicate direction of attack (1.0 = up, -1.0 = down)
	float	vertKick;		//indicate direction of attack (1.0 = right, -1.0 = left)
	float	speed;			//scale the unit's speed
	bool	lockAngles;		//lock viewing angles during this attack

	// npc only fields

	intrange restTime;	//npc only: amount of time to wait in between attacks
}
attackData_t;
//=============================================================================

//=============================================================================
// Experience Data
//=============================================================================
#define MAX_EXP_LEVEL	99

typedef struct
{
	int		points;
	char	*rewardtext;
	int		buildrate;
	int		repairrate;
	int		maxcarry;
	char	*bodymodel;
	int		health;
	char	*rmeleemodel;
	char	*lmeleemodel;
	int		damage;
	int		range;
	int		stamina;
	float	blockpower;
	float	bldpierce;
	float	unitpierce;
	float	siegepierce;
	int		warcry;
}
experienceData_t;

extern experienceData_t experienceTable[MAX_RACES][MAX_EXP_LEVEL+1];

typedef enum
{
	EXPERIENCE_NPC_KILL,
	EXPERIENCE_PLAYER_KILL,
	EXPERIENCE_WORKER_KILL,
	EXPERIENCE_ITEM_KILL,
	EXPERIENCE_SIEGE_KILL,

	EXPERIENCE_PLAYER_REVIVE,
	EXPERIENCE_PLAYER_HEAL,

	EXPERIENCE_STRUCTURE_DAMAGE,
	EXPERIENCE_STRUCTURE_RAZE,

	EXPERIENCE_MINE,
	EXPERIENCE_REPAIR,
	EXPERIENCE_BUILD,

	EXPERIENCE_BONUS,
	EXPERIENCE_SURVIVAL,

	EXPERIENCE_COMMANDER_ORDER_GIVEN,
	EXPERIENCE_COMMANDER_ORDER_FOLLOWED,
	EXPERIENCE_COMMANDER_POWERUP,
	EXPERIENCE_COMMANDER_STRUCTURE,
	EXPERIENCE_COMMANDER_RESEARCH,
	EXPERIENCE_COMMANDER_RAZE,
	EXPERIENCE_COMMANDER_GATHER,
	EXPERIENCE_COMMANDER_DEMOLISH,
	EXPERIENCE_COMMANDER_IGNORE_REQUEST,

	EXPERIENCE_REVIVE_PLAYER,
	EXPERIENCE_HEAL_PLAYER,

	NUM_EXPERIENCE_REWARDS
}
experienceRewards_enum;
//=============================================================================

//=============================================================================
// State String IDs
//=============================================================================

//state strings allow the server to communicate general game specific data
//to all clients without specialized network routines
//
//only clients really need to read this data.  the server should never have
//to read the data directly, because the strings are generated from server side
//data.
//
//up to MAX_STATE_STRINGS ids may be used

//slot 0-2 are reserved by the core engine

#define ST_GAMETYPE (ST_NUM_RESERVED)

#define ST_OBJECT_TYPES (ST_GAMETYPE+1)

#define ST_VOTES_YES (ST_OBJECT_TYPES+1)
#define ST_VOTES_NO (ST_VOTES_YES+1)
#define	ST_VOTES_MIN (ST_VOTES_NO+1)
#define ST_VOTES_NEED (ST_VOTES_MIN+1)
#define ST_VOTE_DESCRIPTION (ST_VOTES_NEED+1)
#define ST_VOTE_TEAM (ST_VOTE_DESCRIPTION+1)
#define ST_VOTE_END_TIME (ST_VOTE_TEAM+1)

#define ST_GAME_STATUS	(ST_VOTE_END_TIME+1)

#define ST_TIME_OF_DAY	(ST_GAME_STATUS+1)

//use ST_CLIENT_INFO + clientnum to get individual client data
//used for client information that doesn't change much (name, team, clan)
//the sharedClientInfo_t structure gets updated whenever one of these is modified
#define ST_CLIENT_INFO (ST_TIME_OF_DAY+1)

//use ST_CLIENT_SCORE + clientnum to get individual client score data
//used for kills, deaths, loyalty, experience, money, level
//these strings are set to REQUEST ONLY, which means the client doesn't
//get them unless they request it
#define ST_CLIENT_SCORE (ST_CLIENT_INFO + MAX_CLIENTS)

//use ST_CLIENT_FINAL_STATS + clientnum to get end of game statistics about each client
#define ST_CLIENT_FINAL_STATS (ST_CLIENT_SCORE + MAX_CLIENTS)

//ST_AWARDS + awardnum holds information about end of round awards
#define ST_AWARDS (ST_CLIENT_FINAL_STATS + MAX_CLIENTS)
#define MAX_AWARDS 10

#define ST_TEAM_INFO (ST_AWARDS + MAX_AWARDS)

#define ST_URGENCY (ST_TEAM_INFO + MAX_TEAMS)
#define ST_CLAN_INFO (ST_URGENCY + 1)

#define ST_MOTD (ST_CLAN_INFO + 1)

#if ((ST_MOTD + 1) >= MAX_STATE_STRINGS)
#error Too many state strings defined
#endif




//=============================================================================
// Client Info
//
// the following structs are portioned up to be bandwidth efficient
// clientScore_t only gets sent across when someone requests
// sharedClientInfo_t gets broadcast whenever it changes
// clientStats_t gets sent only at the end of the game
//=============================================================================


//any changes the server makes to a client's info are sent to each client using this structure
//these are reflected in the ST_CLIENT_INFO state strings
typedef struct
{
	bool			active;

	char			name[CLIENT_NAME_LENGTH];		//the client's name	
	int				team;							//team the client is on
	unsigned int	clan_id;						//their clan_id, or 0 if none
	int				connectTime;
	bool			isOfficer;
	bool			isReferee;
	bool			ready;
} sharedClientInfo_t;



typedef struct
{
	//we can work out accuracy and number of misses from these two stats
	int				shotsFired;
	int				shotsHit;
} weaponUsage_t;

typedef struct
{
	int				killsSinceRespawn;		//number of enemies killed since we last spawned
	int				buildingDamage;			//total damage we have done to enemy buildings
	int				buildingKills;
	int				playerDamage;			//total damage we have done to enemies
	int				npcDamage;				//total damage we have done to NPCs
	int				npcKills;
	int				aiUnitDamage;			//total damage we have done to enemy AI units
	int				aiUnitKills;
	int				moneyGained;			//total money we have accumulated
	int				moneySpent;				//total money we have spent
	int				loyaltyEarned;
	int				ordersGiven;
	int				impeached;
	int				kicked;
	int				revives;				//number of players we have revived
	int				xpTypes[NUM_EXPERIENCE_REWARDS];
	weaponUsage_t	weaponUsage;
} clientStats_t;


//=============================================================================
//
// Voice Chat
//
//=============================================================================


#define MAX_VOICECHAT_MENUS	8
#define MAX_VOICECHAT_ITEMS	10
#define MAX_VOICECHAT_CATEGORIES 10

//chat flags (used for specifying team chat or global chat)
#define	VOICE_GLOBAL	0x01

typedef struct
{
	residx_t	cl_soundidx;	//the cached sound index, modified and used by the client code only

	char		sound[64];		//the .wav file that gets played
	char		text[64];		//the text that gets printed
} voiceSound_t;

typedef struct
{
	int		number;
	int		more;				//"sounds" and "text" get ignored if this is set > 0

	char	desc[32];

	voiceSound_t vs[4];
	int		numSounds;

	int		flags;
} voiceItem_t;

typedef struct
{
	bool			active;
	voiceItem_t		items[MAX_VOICECHAT_ITEMS];
	int				numItems;
} voiceCategory_t;

typedef struct
{
	char				name[32];
	char				directory[256];

	bool				commander;
	int					race;

	voiceCategory_t categories[MAX_VOICECHAT_CATEGORIES];
} voiceMenu_t;


extern voiceMenu_t	voiceMenus[MAX_VOICECHAT_MENUS];

voiceMenu_t *GetVoiceMenu(const char *name);


//=============================================================================
// VisEffect Data
//=============================================================================
enum
{
	VFCLASS_NULL,
	VFCLASS_PARTICLE,
	VFCLASS_BEAM,

	NUM_VFCLASSES
};

typedef enum
{
	PARTICLE_STYLE_POINT,
	PARTICLE_STYLE_TRAIL,
	PARTICLE_STYLE_DIRECTIONAL,

	NUM_PARTICLE_STYLES
}
particleStyle_enum;

typedef struct
{
	int	vfclass;

	int					flags;
	particleStyle_enum	style;
	char				*shader;
	char				*model;
	char				*skin;

	intrange			alphaStart, alphaKey1, alphaKey2, alphaEnd;
	floatrange			alphaKey1Bias, alphaKey2Bias;
	
	intrange			count;
	floatrange			velocity;
	floatrange			acceleration;
	floatrange			size;
	floatrange			sizeHeight;		//optional, if not present use size for height
	floatrange			growth;
	intrange			fps;
	intrange			delay;
	intrange			lifetime;
	floatrange			offset;
	floatrange			gravity;
	floatrange			alpha;
	floatrange			fade;
	floatrange			red, green, blue;
	floatrange			redfade, greenfade, bluefade;
	floatrange			white;
	floatrange			whitefade;
	floatrange			angle;
	floatrange			spin;
	floatrange			altOffset;
	float				tilelength;
	float				minZ;
	float				maxZ;
	float				maxRange;
	float				direction;

	char				*bone;
}
viseffectData_t;
//=============================================================================

//=============================================================================
// Effect Data
//=============================================================================
#define MAX_EFFECTS	200
#define VISUALS_PER_EFFECT 4

typedef struct
{
	char		name[MAX_NAME_LEN];
	bool		touched;

	char		*sound[4];
	char		*loopSound;
	char		*broadcastSound;
	bool		stopLoopSound;
	floatrange		loopsoundVolume;
	floatrange		soundVolume;

	float		shakeScale;
	float		shakeDuration;
	float		shakeRadius;
	float		shakeFrequency;

	viseffectData_t	visuals[VISUALS_PER_EFFECT];
}
effectData_t;

extern effectData_t	effectData[MAX_EFFECTS];
//=============================================================================

//=============================================================================
// Weather Effect Data
//=============================================================================

#define STARTZ_USE_BASEPLAYERZ				0 //use the player's Z + startZ for all particles
#define STARTZ_USE_BASEGROUNDZ				1 //use the ground Z + startZ for all particles
#define STARTZ_USE_PERPARTICLEGROUNDZ		2 //calculate the ground Z + startZ value for every particle

#define MAX_WEATHER_EFFECTS 10
#define MAX_WEATHER_SUB_EFFECTS 4

typedef struct
{
	int 		startZType;
	float		startZ;
	int 		interval;
	float		blockHeight;
	float		blockSize;
	int 		numBlocks;
	char		loopingSound[MAX_NAME_LEN];	
	float		loopingSoundVolume;
	char		effect[MAX_NAME_LEN];
}
weatherSubEffect_t;

typedef struct
{
	char		name[MAX_NAME_LEN];

	weatherSubEffect_t effects[MAX_WEATHER_SUB_EFFECTS];
}
weatherEffect_t;

//=============================================================================

//=============================================================================
//ExEffect
//=============================================================================
#define MAX_EXEFFECTS			50
#define MAX_SUB_EFFECTS			8
#define	MAX_SOUND_VARIATIONS	6

typedef enum
{
	SUB_EFX_NULL,

	SUB_EFX_SOUND,
	SUB_EFX_EMITTER,
	SUB_EFX_ATTRACTOR,
	SUB_EFX_TRACER,

	NUM_SUB_EFX_TYPES
}
subEfx_enum;

typedef enum
{
	EMITTER_POINT,
	EMITTER_FORWARD,
	EMITTER_UP,
	EMITTER_RIGHT,
	EMITTER_LINE,

	NUM_EMITTER_STYLES
}
emitterStyle_enum;

#define DECLARE_KEYS(k)	floatrange	k[4], k##Bias[3]; int k##NumKeys;

typedef struct
{
	subEfx_enum	type;
	
	intrange	delay;
	intrange	frequency;
	intrange	replays;
	intrange	duration;

	//sound
	char	*sound[MAX_SOUND_VARIATIONS];
	bool	loop;
	bool	play2d;
	float	volume;
	float	radius;
	
	//particle emitter
	emitterStyle_enum	style;

	DECLARE_KEYS(velocity)
	DECLARE_KEYS(angle)
	
	floatrange	offset;
	floatrange	gravity;
	floatrange	inertia;

	intrange	count;

	bool		collide;
	floatrange	bounce;
	floatrange	sticky;

	intrange	lifetime;
	
	//particle attractor
	floatrange	pull;
	floatrange	area;

	//tracer
	floatrange	spacing;

	//common
	char	*shader;
	char	*model;
	char	*skin;

	DECLARE_KEYS(red)
	DECLARE_KEYS(green)
	DECLARE_KEYS(blue)
	DECLARE_KEYS(alpha)
	DECLARE_KEYS(size)
}
subEffectData_t;

typedef struct
{
	char	name[MAX_NAME_LEN];
	bool	touched;

	floatrange	bounce;
	floatrange	gravity;
	floatrange	vForward;
	floatrange	vVertical;
	floatrange	vHorizontal;
	floatrange	vRandom;

	char		*bone;

	subEffectData_t	subEffect[MAX_SUB_EFFECTS];
}
exEffectData_t;

extern exEffectData_t	exEffectData[MAX_EXEFFECTS];
extern char *exEffectNames[MAX_EFFECTS + MAX_EXEFFECTS];
//=============================================================================

typedef struct
{
	int	count;
	int storecount;
	int	expireTime;
}
researchData_t;


//physics
enum
{
	PHYSMODE_WALK,
	PHYSMODE_DRIVE,
	PHYSMODE_FREEFLY,
	PHYSMODE_DEAD,
};

enum
{
	SLIDE_OK,
	SLIDE_OBSTRUCTED,
	SLIDE_CLIPPED,
	SLIDE_STUCK,
	SLIDE_OVERTRACED
};


typedef enum
{
	RACE_UNDECIDED,
	RACE_HUMAN,
	RACE_BEAST
} race_enum;

typedef enum
{
	GAME_STATUS_EMPTY,		//server is waiting for clients to connect
	GAME_STATUS_SETUP,		//setting a game up in the lobby
	GAME_STATUS_WARMUP,		//warming up
	GAME_STATUS_NORMAL,		//normal play mode
	GAME_STATUS_ENDED,		//game finished and stats screen is shown
	GAME_STATUS_NEXTMAP,	//about to go to the next map	
	GAME_STATUS_PLAYERCHOSENMAP,	//about to go to a map that was voted in or that the referee chose
	GAME_STATUS_RESTARTING, //match is restarting (same map)

	NUM_GAME_STATUS
} gameStatus_enum;

typedef enum
{
	URGENCY_NONE,
	URGENCY_LOW,
	URGENCY_HIGH
} urgency_enum;

typedef enum
{
	STATUS_LOBBY,
	STATUS_UNIT_SELECT,
	STATUS_SPAWNPOINT_SELECT,
	STATUS_COMMANDER,
	STATUS_PLAYER,
	STATUS_SPECTATE,
	STATUS_ENDGAME,

	NUM_PLAYER_STATUS
} playerStatus_enum;

typedef enum
{
	TEAM_UNDECIDED,
	TEAM_ONE,
	TEAM_TWO,
	MAX_TEAMS
} teams_enum;

typedef enum
{
	GAMETYPE_RTSS,			//the main game type
	GAMETYPE_DEATHMATCH,	//a free for all deathmatch
	GAMETYPE_LOBBY,
	NUM_GAMETYPES
} gametypes_enum;


//animation states

//animStates are used to trigger object and player animations

typedef enum
{
	AS_IDLE,				//should always be kept at the 0 slot

	//melee attacks should not move out of this spot,
	//as they're linked to the server's attack data
	AS_MELEE_1,
	AS_MELEE_2,
	AS_MELEE_3,
	AS_MELEE_4,
	AS_ALT_MELEE_1,
	AS_ALT_MELEE_2,
	AS_ALT_MELEE_3,
	AS_ALT_MELEE_4,
	AS_MELEE_CHARGE,		//charge up a power attack
	AS_MELEE_RELEASE,		//perform the power attack

	AS_MELEE_MOVE_1,
	AS_MELEE_MOVE_2,
	AS_MELEE_MOVE_3,
	AS_MELEE_MOVE_4,
	AS_MELEE_MOVE_CHARGE,
	AS_MELEE_MOVE_RELEASE,

	AS_BLOCK,

	AS_WALK_LEFT,
	AS_WALK_RIGHT,
	AS_WALK_FWD,
	AS_WALK_BACK,

	AS_RUN_LEFT,
	AS_RUN_RIGHT,
	AS_RUN_FWD,
	AS_RUN_BACK,

	AS_SPRINT_LEFT,
	AS_SPRINT_RIGHT,
	AS_SPRINT_FWD,
	AS_SPRINT_BACK,

	AS_JUMP_START_LEFT,
	AS_JUMP_START_RIGHT,
	AS_JUMP_START_FWD,
	AS_JUMP_START_BACK,

	AS_JUMP_MID_LEFT,
	AS_JUMP_MID_RIGHT,
	AS_JUMP_MID_FWD,
	AS_JUMP_MID_BACK,

	AS_JUMP_END_LEFT,
	AS_JUMP_END_RIGHT,
	AS_JUMP_END_FWD,
	AS_JUMP_END_BACK,

	AS_JUMP_UP_START,
	AS_JUMP_UP_MID,
	AS_JUMP_UP_END,

	AS_JUMP_LAND,

	AS_DODGE_START_LEFT,
	AS_DODGE_START_RIGHT,
	AS_DODGE_START_FWD,
	AS_DODGE_START_BACK,

	AS_DODGE_MID_LEFT,
	AS_DODGE_MID_RIGHT,
	AS_DODGE_MID_FWD,
	AS_DODGE_MID_BACK,

	AS_DODGE_END_LEFT,
	AS_DODGE_END_RIGHT,
	AS_DODGE_END_FWD,
	AS_DODGE_END_BACK,

	AS_CROUCH_IDLE,

	AS_CROUCH_LEFT,
	AS_CROUCH_RIGHT,
	AS_CROUCH_FWD,
	AS_CROUCH_BACK,

	AS_WALK_WITH_BAG,

	AS_MINE,
	AS_REPAIR,
	AS_CONSTRUCT,

	AS_WOUNDED_LEFT,
	AS_WOUNDED_RIGHT,
	AS_WOUNDED_FWD,
	AS_WOUNDED_BACK,

	AS_DEATH_GENERIC,
	AS_DEATH_LEFT,
	AS_DEATH_RIGHT,
	AS_DEATH_FWD,
	AS_DEATH_BACK,

	AS_RESURRECTED,

	AS_WEAPON_IDLE_1,
	AS_WEAPON_IDLE_2,
	AS_WEAPON_IDLE_3,
	AS_WEAPON_IDLE_4,
	AS_WEAPON_IDLE_5,
	AS_WEAPON_IDLE_6,

	AS_WEAPON_CHARGE_1,
	AS_WEAPON_CHARGE_2,
	AS_WEAPON_CHARGE_3,
	AS_WEAPON_CHARGE_4,
	AS_WEAPON_CHARGE_5,
	AS_WEAPON_CHARGE_6,

	AS_WEAPON_FIRE_1,
	AS_WEAPON_FIRE_2,
	AS_WEAPON_FIRE_3,
	AS_WEAPON_FIRE_4,
	AS_WEAPON_FIRE_5,
	AS_WEAPON_FIRE_6,

	AS_WEAPON_RELOAD_1,
	AS_WEAPON_RELOAD_2,
	AS_WEAPON_RELOAD_3,
	AS_WEAPON_RELOAD_4,
	AS_WEAPON_RELOAD_5,
	AS_WEAPON_RELOAD_6,

	AS_WEAPON_SWITCH,

	//FPS weapon specific animations (also used by combat code to track weapon state):
	AS_WPSTATE_IDLE,
	AS_WPSTATE_SWITCH,
	AS_WPSTATE_CHARGE,
	AS_WPSTATE_SPINUP,
	AS_WPSTATE_SPINDOWN,
	AS_WPSTATE_OVERHEAT,
	AS_WPSTATE_FIRE,
	AS_WPSTATE_BACKFIRE,
	
	AS_ITEM_SLEEP,
	AS_ITEM_ACTIVE,			//an item has transitioned into its active state (electric eye)

	AS_CONSTRUCT_1,
	AS_CONSTRUCT_2,
	AS_CONSTRUCT_3,
	AS_CONSTRUCT_FINAL,

	AS_SUICIDE,				// chiprel suicide anim

	NUM_ANIMSTATES
} animStates_enum;

#define NUM_MELEE_ATTACKS (AS_MELEE_RELEASE+1)
#define	IS_MELEE_ATTACK(animstate)	((animstate) > 0 && (animstate) < NUM_MELEE_ATTACKS ? true : false)
#define IS_WEAPON_STATE(animstate)	(((animstate) >= AS_WEAPON_SWITCH && (animstate) <= AS_WEAPON_OVERHEAT) ? true : false)





//non-core client/server messages

/****** CLIENT to server **********************************************/

#define		CLIENT_CHAT_MSG               		"chat"				  	//send a global chat message
#define     CLIENT_CHAT_TEAM_MSG           		"chat_team"			  	//send a team chat message
#define     CLIENT_CHAT_PRIVATE_MSG        		"chat_private"		  	//send a private chat message
#define     CLIENT_CHAT_SELECTED_MSG         	"chat_selected"		  	//send a chat message to teammates who are selected
#define		CLIENT_CHAT_COMMANDER_MSG			"chat_commander"		//send a chat message to your team's commander
#define		CLIENT_REQUEST_MSG					"crequest"				//client is asmking commander for something
#define		CLIENT_KILL_MSG						"kill"					//request to be killed
#define		CLIENT_EJECT_MSG					"eject"					//request to eject
#define     CLIENT_TEAM_REQUEST_MSG         	"team_request"			//request a new team be created
#define     CLIENT_TEAM_JOIN_MSG            	"team_join"				//request to join a team
#define		CLIENT_GIVE_MSG						"give"					//put an item in the inventory
#define		CLIENT_GIVEBACK_MSG					"giveback"				//give an item back to the shop
#define		CLIENT_ENTER_BUILDING_MSG			"enter_building"
#define		CLIENT_VOTE_MSG						"vote"
#define		CLIENT_CALLVOTE_MSG					"callvote"
#define		CLIENT_REFEREE_MSG					"ref"					//client wants to become referee
#define		CLIENT_REFEREE_COMMAND_MSG			"refc"					//referee sending a command
#define		CLIENT_READY_MSG					"ready"
#define		CLIENT_GETCOOKIE_MSG				"getcookie"				//request the cookie of a player
#define		CLIENT_SCORE_PLAYER_MSG				"sp"
#define		CLIENT_SCORE_TEAM_MSG				"st"
#define		CLIENT_SCORE_ALL_MSG				"sa"
#define		CLIENT_VOICECHAT_MSG				"vc"
#define		CLIENT_SHOW_LOADOUT_MSG				"loadout"				//asking to go to the loadout screen

//commander client->server messages
#define	CLIENT_COMMANDER_CLEAR_CLIENT_TARGETS_MSG	"clear_targets"		//clear all the current waypoints for a particular client
#define	CLIENT_COMMANDER_PURCHASE_MSG				"purchase"			//purchase an item in the techtree
#define	CLIENT_COMMANDER_ACTIVATE_MSG				"activate"			//activate a non-passive upgrade that was previsouly researched
#define	CLIENT_COMMANDER_RESIGN_MSG					"request_player"	//for a commander to resign and become a player
#define	CLIENT_COMMANDER_REQUEST_MSG				"request_commander"	//request to become the new commander
#define	CLIENT_COMMANDER_SELECTION_MSG				"selection"			//send the current selection set to the server
#define	CLIENT_COMMANDER_SPAWN_BUILDING_MSG			"spawn_bld"			//tell a peon to spawn a building
#define	CLIENT_COMMANDER_SPAWN_BUILDING_LINK_MSG	"spawn_bld_link"	//tell a peon to spawn a building
#define	CLIENT_COMMANDER_CAN_SPAWN_BUILDING_MSG		"can_spawn_bld"		//ask if we can spawn a building
#define CLIENT_COMMANDER_CANCEL_LINK_MSG			"cancel_link"		//let the server know we skipped out on the second half
#define	CLIENT_COMMANDER_CONSTRUCT_MSG				"construct"			//tell a peon to construct a building
#define	CLIENT_COMMANDER_LOCATION_GOAL_MSG			"loc_goal"			//location-related task
#define	CLIENT_COMMANDER_OBJECT_GOAL_MSG			"obj_goal"			//object-related task
#define	CLIENT_COMMANDER_GIVEMONEY_MSG				"givemoney"			//gives a player money from the team stash
#define	CLIENT_COMMANDER_GIVETHING_MSG				"givething"			//gives a player an object with his funds augmented by the team
#define	CLIENT_COMMANDER_PROMOTE_MSG				"promote_officer"	//promote a player to officer slot
#define	CLIENT_COMMANDER_DEMOTE_MSG					"demote_officer"	//remove officer status from a player
#define	CLIENT_COMMANDER_DEMOLISH_MSG				"demolish"			//destroy a building
#define	CLIENT_COMMANDER_SUICIDE_MSG				"suicide"			//destroy a worker
#define	CLIENT_COMMANDER_STOP_RESEARCH_MSG			"stopresearch"		//stop whatever a building is making
#define	CLIENT_COMMANDER_DECLINE_MSG				"decline"			//commander denied a request
#define	CLIENT_COMMANDER_APPROVE_MSG				"approve"			//commander approved a request
#define	CLIENT_COMMANDER_EXPIRE_MSG					"expire"			//commander ignored a request
#define	CLIENT_COMMANDER_CANCEL_MSG					"cancel_request"	//client changed their mind
#define CLIENT_OFFICER_OBJECT_GOAL_MSG				"officer_objorder"	//object goal order from an officer
#define CLIENT_OFFICER_LOCATION_GOAL_MSG			"officer_locorder"	//location goal order from an officer

#define	CLIENT_START_GAME_MSG	"start_game"

//admin client->server messages
#define		CLIENT_ADMIN_POSTPONE_GAME_MSG		"admin_postpone"
#define		CLIENT_ADMIN_CHANGE_SETTING_MSG		"admin_setting"
#define		CLIENT_ADMIN_RACE_MSG				"admin_race"

//these are sent to the core for the server-end
#define		CLIENT_ADMIN_MAP_MSG				"admin_map"
#define		CLIENT_ADMIN_END_SETUP_MSG			"admin_startgame"

/******** SERVER to client ********************************************/

#define     SERVER_CHAT_MSG        				"chat"					//a global chat message
#define		SERVER_VOICECHAT_MSG				"vc"
#define     SERVER_CHAT_TEAM_MSG        		"chat_team"				//a team chat message
#define     SERVER_CHAT_PRIVATE_MSG        		"chat_msg"				//a /msg-type chat message
#define     SERVER_CLIENT_TEAM_JOIN_MSG     	"join"			//announcement that a client has joined a team
#define		SERVER_CLIENTCOOKIE_MSG				"cookie"
#define		SERVER_ITEM_DEPLOYMENT_MSG			"deployment"

#define     SERVER_WORLD_OBJECT_DESTROY_MSG		"wo_delete"				//announcement that a world object has been deleted

#define     SERVER_COMMANDER_TEAM_RESOURCES_MSG	"tres" 		//tell a team their current resources
#define		SERVER_COMMANDER_UNITINFO_MSG		"uinfo"		//tell a commander the unit info
#define     SERVER_NOTICE_MSG       			"notice" 				//tell a team commander various notices and errors

#define		SERVER_OBJECT_TABLE_MSG				"object_table"			//client needs to index the object data

#define		SERVER_COMMANDER_RESEARCH_UPDATE_MSG		"ru"		//update the client with a newly researched or constructed item
#define		SERVER_COMMANDER_ALL_RESEARCH_MSG			"all_research"				//send the client the entire research upgrades array
#define		SERVER_COMMANDER_OFFICER_LIST_MSG			"officer_list"
#define		SERVER_COMMANDER_PLACE_LINK_MSG				"place_link"

#define		SERVER_ADMIN_NOTIFY_MSG				"admin"

#define		SERVER_TEAM_INFO_MSG				"team_info" //tell a client about a particular team
#define		SERVER_TEAM_JOIN_MSG				"joined" //tell a client what team they're on

#define		SERVER_COMMANDER_CAN_SPAWN_BUILDING_MSG		"can_spawn_bld" //response to can-spawn request

#define		SERVER_GENERAL_MSG						"msg"



//waypoint messages
#define		SERVER_WAYPOINT_POSITION		"waypos" 		//tell a client to target a position
#define		SERVER_WAYPOINT_OBJECT			"wayobj"  //tell a client to target an object
#define		SERVER_WAYPOINT_COMPLETED		"waydone" //reached the target / completed goal
#define		SERVER_WAYPOINT_CANCEL			"waycancel"	//cancelled order

/**** serverObject_t.flag values **************************************/

#define		OBJECT_FLAG_ATTACK_FINISHED		0x01		//like the ps flag, but used for npcs
#define		OBJECT_FLAG_NOT_A_TARGET		0x02		//things should not attack this

typedef enum
{
	ERROR_NOTCOMMANDER,
	ERROR_NEEDRESOURCE,
	ERROR_NOTAVAILABLE,
	ERROR_PLAYERNOTFOUND,

	/* non-errors, just messages in general */
	NOTICE_BEGIN_RESEARCH,
	NOTICE_QUEUED_RESEARCH,
	NOTICE_UPGRADECOMPLETE,
	NOTICE_CANCEL_RESEARCH,

	NOTICE_GENERAL,

	NOTICE_3_KILLS,
	NOTICE_5_KILLS,
	NOTICE_HERO,
	NOTICE_LEGEND,
	NOTICE_SKILLFUL,

	NOTICE_BASE_UNDER_ATTACK,

	NOTICE_VICTORY,
	NOTICE_DEFEAT,
	NOTICE_SUDDEN_DEATH,
	NOTICE_OVERTIME,

	NOTICE_OBITUARY,

	NOTICE_BUILDING_CLAIMED,
	NOTICE_BUILDING_STOLEN,

	NOTICE_PROMOTE,
	NOTICE_DEMOTE,
	
	NOTICE_ALERT_STRING,

	NUM_SERVERMESSAGES
} serverMessages;


//the following events are used for playerState_t::event and netObject_t::event
//they are mostly used for signalling sound effects and graphical effects generated
//by an object/player, as well as telling a player when he has been wounded



#define IS_ATTACK_ANIM(anim) (anim >= ATTACK_JAB && anim < NUM_ATTACK_TYPES ? true : false)

#define	MAX_REQUIREMENTS	2
#define MAX_OBJECT_TOOLTIP_LENGTH	256

/*=============================================================================
=============================================================================*/
typedef enum
{
	UPGRADE_PASSIVE,	//constantly operating upgrade, e.g. improved armor
	UPGRADE_ACTIVE,		//upgrade that allows the commander to perform a special action

	NUM_UPGRADE_TYPES
}
upgradeType_enum;

#define TARGET_ENEMY		0x0001
#define	TARGET_ALLY			0x0002
#define	TARGET_NEUTRAL		0x0004
#define TARGET_TEAMS		(TARGET_ENEMY|TARGET_ALLY|TARGET_NEUTRAL)

#define	TARGET_UNIT			0x0008
#define	TARGET_BUILDING		0x0010
#define TARGET_ITEM			0x0020
//these are not really active right now
#define	TARGET_WORLD		0x0040	//hit a spot on the world
#define	TARGET_TEAM			0x0080	//apply to an entire team
#define TARGET_TYPES		(TARGET_UNIT|TARGET_BUILDING|TARGET_ITEM|TARGET_WORLD)

#define TARGET_SAME_TYPE	0x0100
/*=============================================================================
=============================================================================*/

typedef enum
{
	GOAL_NONE,
	GOAL_MINE,
	GOAL_DROPOFF_RESOURCES,
	GOAL_ATTACK_OBJECT,
	GOAL_DEFEND,
	GOAL_FOLLOW,
	GOAL_REACH_WAYPOINT,		//travel to an object or locational target
//	GOAL_SPAWN_BUILDING,		//create a building in an empty spot on the map
	GOAL_CONSTRUCT_BUILDING,	//aid the construction of a building that is already spawned	
	GOAL_REPAIR,				//repair a damaged building	
	GOAL_FLEE,
	GOAL_ENTER_BUILDING,
	GOAL_ENTER_TRANSPORT,
	GOAL_COMPLETED,				//currently used client side only to play a goal completed sound

	NUM_GOALS
} objectGoals_enum;

/*=============================================================================
Weapons
=============================================================================*/
//damage flags
#define	DAMAGE_NO_KNOCKBACK			0x00000001
#define	DAMAGE_NO_AIR_TARGETS		0x00000002
#define DAMAGE_NO_GROUND_TARGETS	0x00000004
#define DAMAGE_UNBLOCKABLE			0x00000008

#define	DAMAGE_SELF_NONE			0x00000010
#define	DAMAGE_SELF_HALF			0x00000020
#define	DAMAGE_NO_FALLOFF			0x00000040
#define	DAMAGE_NO_STRUCTURES		0x00000080

#define	DAMAGE_QUAKE_EVENT			0x00000100
#define DAMAGE_NO_REACTION			0x00000200
#define	DAMAGE_STRIP_STATES			0x00000400

#define	DAMAGE_EXPLOSIVE			0x00000800
#define	DAMAGE_BURNING				0x00001000
#define	DAMAGE_ELECTRICAL			0x00002000

#define	DAMAGE_SPLASH				0x00004000


#define	MOD_VELOCITY	0x00000001
#define MOD_DAMAGE		0x00000002	//fixme: projectile needs to store this value
#define	MOD_RANGE		0x00000004	//fixme: remove this?
#define	MOD_ACCURACY	0x00000008

#define	MOD_FUSETIME	0x00000010	//fixme: projectile needs to store this value
#define	MOD_BLASTRADIUS	0x00000020	//fixme: projectile needs to store this value
#define	MOD_BLASTDAMAGE	0x00000040	//fixme: remove this?
#define	MOD_COUNT		0x00000080

#define MOD_INVERSE		0x00000100	//fixme: make a different flag type that can be +/-/0

#define	CHARGEMODTEST(wd,m)	((((wd)->chargeFlags) & (m)) && ((wd)->chargeTime > 0))
#define CHARGEMODIFY(wd,v,c)	((v) *= (((wd)->chargeFlags) & MOD_INVERSE) ? (1.0 - (c)) : (c))
#define CHARGEMOD(wd,m,v,c)	if(CHARGEMODTEST(wd,m)) CHARGEMODIFY(wd,v,c)

#define	HEATMODTEST(wd,m)	((((wd)->heatFlags) & (m)) && ((wd)->overheatTime > 0))
#define HEATMODIFY(wd,v,c)	((v) *= (((wd)->heatFlags) & MOD_INVERSE) ? (1.0 - (c)) : (c))
#define HEATMOD(wd,m,v,c)	if(HEATMODTEST(wd,m)) HEATMODIFY(wd,v,c)

typedef enum
{
	PROJ_DIE,			//damage target, expire, send death event
	PROJ_DIE_NO_EFFECT,	//damage target, expire, no death event
	PROJ_STOP,			//jsut stop. anyhting <= PROJ_STOP halts trajectory processing
	PROJ_IGNORE,		//no interaction
	PROJ_PIERCE,		//damage target, continue on trajectory
	PROJ_BOUNCE,		//change trajectory, send bounce event
	PROJ_BOUNCE_DAMAGE,	//bounce and hurt teh target

	NUM_PROJ_REACTIONS
}
projReaction_t;

#define	ACTION_DAMAGE_RADIUS	0x0001	//activate the projectiles radius damage
#define	ACTION_FIND_TARGET		0x0002	//search the radius for an enemy and save it
#define	ACTION_ATTACK			0x0004	//fire the default weapon forward, or at the projectiles enemy
#define	ACTION_ATTACK_RANDOM	0x0008	//fire the default weapon in a random direction

typedef enum
{		
	EVENT_WOUNDED = 1,
	EVENT_DEFLECTED,
	EVENT_DAZED,
	EVENT_FALLING_DAMAGE,

	EVENT_JUMP,
	EVENT_JUMP_LAND,

	EVENT_BOUNCE,
	EVENT_STOP,

	EVENT_MINE,
	EVENT_DROPOFF,
	EVENT_RESOURCE_FULL,

	EVENT_LEVEL_UP,

	EVENT_SPAWN,
	EVENT_DEATH,
	EVENT_DEATH_QUIET,

	EVENT_ANIM_RETRIGGER,		//retrigger the animState
	EVENT_ANIM2_RETRIGGER,		//retrigger the animState2
	
	EVENT_TASK_FINISHED,
	
	EVENT_USE_ITEM,
	EVENT_ITEM_SLEEP,
	EVENT_ITEM_IDLE,
	EVENT_ITEM_ACTIVATE,
	EVENT_FIZZLE,
	EVENT_BACKFIRE,

	EVENT_PICKUP_WEAPON,
	
	EVENT_QUAKE,
	EVENT_WEAPON_FIRE,

	EVENT_WEAPON_HIT,

	EVENT_GOODIE_PICKUP,
	EVENT_SPLODEY_DEATH,

	EVENT_COMMANDER_SELECTED,

	EVENT_ATTACK_POUND,
	EVENT_ATTACK_SUICIDE,

	EVENT_POWERUP,

	EVENT_RESURRECTED,

	NUM_EVENTS	//events higher than 63 will send an unnessecary byte, since the high bit is used to flag the presence of a parameter
} netEvents_t;

#define BUILDING_HEALTH_STAGES 3

typedef struct
{
	bool	touched;

	//common
	int		objclass;							//***this must be the first field*** determines what fields are used and how
	char	name[MAX_NAME_LEN];					//internal reference name
	char	*description;						//name as it will appear in game
	int		race;								//race that can build/use this
	bool	drawTeamIcon;
	byte	type;
	bool	drawName;
	bool	drawHealth;

	//visual representation of the unit/building
	char	*model;								//pathname of model file	***************************************
	char	*skin;								//pathname of skin file		***************************************
	char	*shader;							//pathname of shader file (used to render object as a billboard if model/skin not specified
	int		effect;
	float	scale;								//tweak the size, 1.0 == normal

	floatrange	yaw;
	floatrange	pitch;
	floatrange	roll;
	
	//model to display in first person view
	char	*handModel;							//pathname of model file	***************************************
	char	*handSkin;							//pathname of skin file		***************************************
	int		handEffect;

	//visual data for projectiles (or anything else that this object 'spawns')
	char		*projectileModel;					//pathname of model file	***************************************
	char		*projectileSkin;					//pathname of skin file		***************************************
	char		*projectileShader;					//pathname of shader file (used to render object as a billboard if model/skin not specified
	int			projectileEffect;
	float		projectileScale;					//tweak the size, 1.0 * normal
	float		projectileRadius;					// collision radius, 0 = point mass, >0 = aabb
	int			trailEffect;
	intrange	trailPeriod;						//msecs between trail effects
	int			flybyEffect;

	//interface information
	char	*icon;								//path name to icon image
	char	*selectionIcon;						//path name for alterante icon
	char	*mapIcon;							//path name for mini-map icon
	char	*tooltip;							//text description for commander
	char	*selectionSound;					//pathname to sound to play on selection
	char	*gridmenu;							//path name to the grid menu cfg file this unit should use
	char	*proximityMessage;
	char	*proximitySound;
	float	proximity;

	//techtree placement
	char	*builder[2];						//name of structure or unit that creates this object (builde1 inherits from parent buildings)
	char	*requirements[MAX_REQUIREMENTS];	//names of other objects that must be built/researched 
	int		needTechPoints;						//number of "tech points" needed for this
	int		needBasePoints;						//number of "base points" needed for this
	int		techPointValue;						//number of "tech points" that building this structure is worth
	int		basePointValue;						//number of "tech points" that building this structure is worth
	bool	alwaysAvailable;					//this item does not need to be researched
	int		techType;

	//voicechat
	char	*voiceMenu;

	//weapons	
	char		*ammoName;						//name of ammo
	floatrange	velocity;						//initial speed of a projectile, or range of a trace
	floatrange	vertVelocity;					//adds a vertical component to projectiles
	intrange	count;							//number of projectiles/traces to perform
	float		accuracy;						//range of [0,1] where 1 is perfectly straight, 0 is totally random
	int			chargeFlags;					//determines which fields are affected by the chargeTime
	int			heatFlags;						//determines which fields are affected by the overheatTime
	int			chargeTime;						//time in msecs from activation time to full charge
	int			spinupTime;						//delay in msecs before activation
	int			spindownTime;					//delay in msecs after activation before a new activation cycle can begin
	int			cooldownTime;					//time in msecs after an overheat before object can be triggered
	int			overheatTime;					//time in msecs from activation 
	int			refreshTime;					//time in msecs after an activation before it reactivates, 0 means trigger must be released
	int			fireDelay;						//time in msecs after entering the weaponFire state before the action is taken
	int			ammoMax;						//maximum uses
	int			ammoStart;						//amount of ammo to recieve with purchase of the weapon
	int			ammoGroup;						//amount of ammo to get per ammo purchase
	int			ammoCost;						//cost of 1 ammo
	float		twangTime;
	float		twangAmount;
	int			twangPingPong;
	float		minVelocity;					//always apply this much velocity, regarless of modifiers such as charge
	float		maxVelocity;					//never exceed this velocity, regarless of modifiers such as acceleration
	float		gravity;						//amount of gravity to apply, 1.0 * standard
	float		bounce;							//amount of velocity to retain after an impact, 0 * no bounce
	float		accelerate;						//velocty to add per second
	intrange	fuseTime;						//time in msecs before object is destroyed
	float		scanRange;						//distance at wich another object will activate this object
	int			damage;							//damage to apply to target
	floatrange	radius;							//radius to affect on an impact, for range 0 radius around source
	float		unitPierce;						//multiplier for damage against units
	float		bldPierce;						//multiplier for damage against buildings
	float		siegePierce;					//multiplier for damage against siege weapons
	bool		repeat;							//if false, trigger must be released before next shot will fire
	bool		continuous;
	projReaction_t	hitBuilding;
	projReaction_t	hitUnit;
	projReaction_t	hitWorld;
	float		focusPenalty;
	float		focusDegradeRate;
	float		focusRecoverRate;
	char		*attachBone;
	int			damageFlags;
	float		transferHealth;
	float		staminaDrain;
	int			inheritVelocity;
	vec3_t		muzzleOffset;
	bool		useAltMelee;
	bool		useDefaultMeleeModel;
	bool		useWeaponFire;
	int			weapPointValue;
	bool		startFuseAtCharge;
	int			backfireTime;
	bool		targetCorpse;					//just for resurrection, really

	bool		useMana;
	int			manaCost;
	float		speedPenalty;

	float		minfov;							//the max zoom (minimum FOV) for an weapon

	int			animGroup;						//animation group for third person views

	char		*continuousBeamShader;					//for continuous weapon beams
	int			continuousBeamType;

	//events
	int			effects[NUM_EVENTS + NUM_ANIMSTATES];

	char		*loopingSounds[NUM_ANIMSTATES];
	char		*projectileSound;
	char		*constructionSound;

	int		cost[MAX_RESOURCE_TYPES];			//cost to commander to build this
	int		researchTime;						//number of msecs to build/research
	int		playerCost;							//cost to player (in currency defined in race config) to purchase this for their inventory
	float	armor;
	float	turnRate;
	
	//buildings
	int			fullHealth;
	bool		isMine;						//resources can be mined from this structure
	bool		isClaimable;				//the mine can be claimed by another team
	int			mineType;					//the type of resource mined
	int			mineAmount;					//amount of material that mine holds
	bool		dropoff;					//resources can be dropped off here
	bool		commandCenter;				//if 1, this is a team's command center
	bool		spawnFrom;
	bool		canEnter;
	int			generate;
	int			store[MAX_RESOURCE_TYPES];
	int			healthStages[BUILDING_HEALTH_STAGES];
	int			healthStageEffects[BUILDING_HEALTH_STAGES];
	intrange	healthStagePeriods[BUILDING_HEALTH_STAGES];
	int			radiusState;
	bool		targetProjectiles;
	bool		linked;
	bool		selfBuild;
	bool		canBeRepaired;
	int			lifeSpan;
	float		anchorRange;
	bool		needsAnchor;			//only buildings with this set count for maxNeighborDistance
	float		shiftProjAccel;
	float		shiftProjGrav;

	//items
	int		maxHold;					//maximum number that one player can put in their inventory
	int		maxDeployment;				//maximum number that a team can own and have active
	int		delayEffect;				//play this effect while delaying
	int		delayPeriod;				//frequency to activate dealyEffect (in msecs)
	int		activeEffect;				//play this effect while active
	int		activeEffectPeriod;			//frequency to activate activeEffect (in msecs)
	int		isActivated;				//flags whether an item jsut sits in the inventory or needs to be used
	bool	isVulnerable;				//can receive normal damage
	bool	isVolatile;					//dies immediately if destabilized
	bool	meleeOnlyVulnerable;		//rejects damage from non-melee source
	bool	revealHidden;				//objects within the viewdistance of this object will be marked, and have their cloak benefits nullified
	bool	canPickup;
	char	pickupGive[MAX_NAME_LEN];
	int		pickupRespawn;
	bool	giveAmmo;
	int		giveMana;
	int		manaRegenAdd;
	float	manaRateMult;
	bool	isSolid;
	int		deathLinger;
	bool	isSelectable;
	bool	linkToOwner;

	//units
	bool			e3noblend;
	bool			npcTarget;
	float			cmdrScale;
	char			*footstepType;
	float			bobSpeedRun;
	float			bobSpeedSprint;
	float			bobAmount;
	bool			isWorker;
	bool			isVehicle;
	bool			canRide;
	bool			canEject;
	char			*ejectUnit;	
	int				maxRiders;
	float			fixedPitch;
	attackData_t	attacks[NUM_MELEE_ATTACKS];
	float			blockPower;
	float			blockArc;
	float			speed;
	float			jumpheight;
	float			stepheight;
	float			aircontrol;
	float			friction;
	vec3_t			bmin;
	vec3_t			bmax;
	float			viewheight;
	int				distOffset;
	int				range;
	int				killGoldReward;
	int				killResourceReward[MAX_RESOURCE_TYPES];
	float			maxChaseRange;
	float			attackProbability;
	float			respawnTime;
	int				maxResources[MAX_RESOURCE_TYPES];
	int				spawnAtStartNum;
	char			*forceInventory[MAX_INVENTORY];
	int				allowInventory[MAX_INVENTORY];
	int				flurryCount;
	int				maxPopulation;
	int				mineRate;
	int				repairRate;
	int				buildRate;		//advance this many msecs towards completion each second
	bool			isScared;
	bool			isPassive;
	float			minAimX, maxAimX;	//constraints for siege weapon aiming
	float			minAimY, maxAimY;	//0 is left/bottom, 1 is right/top
	float			viewDistance;
	char			*leftMeleeModel;
	char			*rightMeleeModel;
	int				healAmount;
	int				healRate;
	bool			canPurchase;
	bool			allowFirstPerson;	//if not set, this unit must always be played in third person
	bool			canBlock;
	bool			canDodge;
	bool			isSiegeWeapon;		//siege units receive special treatment in some cases
	int				maxMana;
	int				manaRegenRate;
	int				manaRegenAmount;
	bool			canSprint;
	int				maxStamina;
	float			staminaRegenRate;
	float			expMult;
	int				thorndamage;
	bool			alwaysWaypoint;
	float			mass;
	int				maxWeapPoints;
	int				blockStunTime;
	int				revivable;
	bool			isHealer;

	//npc only
	float			attackMeleeChance;
	float			attackMissileChance;
	float			attackSuicideChance;
	floatrange		attackSuicideDelay;
	floatrange		attackSuicideRange;
	float			attackSuicideRadius;
	float			attackSuicideDamage;
	float			attackPoundChance;
	float			fleeChance;
	float			fleeThreshhold;
	float			dodgeChance;
	intrange		dodgeRestTime;
	floatrange		dodgeDistance;
	float			aggressionChance;
	floatrange		aggressionRange;	
	floatrange		aggressionInterval;
	float			packRange;
	intrange		packSize;
	int				level;				//for non-clients, sets a static level
	int				totalLives;			//max times this unit should respawn

	//upgrades
	char				*recipients;	//name of object/group that will recieve this
	int					duration;							//how long before this runs out
	int					targetFlags;
	int		useTarget;
	char	*cursor;
	int		activateCost[MAX_RESOURCE_TYPES];
}
objectData_t;

/*
 * The data we store about each waypoint
 */
typedef struct
{
	bool		active;
	
	//when this goal was created
	int			time_assigned;
	
	bool		commander_order;			//if false, an officer assigned the order
	int			clientnum;					//client who issued the order
	bool		fake_waypoint;
	
	//is this an object waypoint or a position waypoint?
	bool		object;
	//if (object), the index of the object that is our waypoint
	int			object_index;
	//object type (can't guarantee the client will know about the object)
	byte		object_type;
	//else, the position that serves as our waypoint
	vec3_t		pos;
	
	residx_t	shader;
	int			goal;
} waypoint_t;


#if (NUM_OBJECT_TYPES > 255)
	#error Too many object types in netObjectTypes_enum
#endif

typedef struct
{
//	solid_t *playersolid;

	//the following fields must be set before calling any physics functions
	bool (*tracefunc)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface);
	objectData_t *(*objectTypeFunc)(int type);
	
	bool	smartSliding;

	float frametime;

	float speed;
	float jumpheight;
	float friction;
	float aircontrol;
	float gravity;
	float stepheight;

	//bounding box for the object we're moving
	vec3_t	bmin;
	vec3_t	bmax;

	float	*pos;
	float	*velocity;

	//the following vars are used as scratchpad variables by the physics code, no need to set them
	vec3_t forward;
	vec3_t right;
	traceinfo_t ground;

	bool server;		//true if calling from server
} physicsParams_t;

typedef struct
{
	traceinfo_t collisions[MAX_SLIDE_COLLISIONS];
	int			num_collisions;
	int			doAttack;			//this is the frame we need to check for a melee impact or ranged weapon release..set to an AS_* define	
} phys_out_t;


#define	MAX_SELECTABLE_UNITS	10

typedef struct
{
	int array[MAX_SELECTABLE_UNITS];
	int numSelected;
	bool mobileUnitsSelected;
} unitSelection_t;


void	Phys_AdvancePlayerState(playerState_t *playerState, int team, inputState_t *inputState, inputState_t *oldInputState,
								bool (*tracefunc)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface),
								objectData_t *(*objectTypeFunc)(int type),
								phys_out_t *phys_out);
void		Phys_Init();
void		Phys_DoCombat();
void		Phys_AddEvent(playerState_t *ps, byte event, byte param, byte param2);
void		Phys_ClearEvents(playerState_t *ps);
void		Phys_SetupParams(byte unittype, int team, byte states[MAX_STATE_SLOTS], vec3_t pos, vec3_t velocity,
					 	bool (*tracefunc)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface),
						objectData_t *(*objectTypeFunc)(int type),
						float frametime,
						physicsParams_t *out);
void 		Phys_ParallelPlane(vec3_t in_vel, vec3_t normal, vec3_t out_vel);
int			Phys_Slide(physicsParams_t *in, phys_out_t *out);
int			Phys_SmartSlide(physicsParams_t *in, phys_out_t *out);

void		InitObjdefReferences();

//units.c
void		InitUnitData();
void		PostProcessUnits();
void		SetObjectBounds(baseObject_t *obj);
char		*Unit_GetAnimFromMoveState(int movestate);

//techtree.c

#define GRIDMENU_SIZE 12

#define MAX_RESEARCHED_ITEMS	64

#define MAX_ITEMS_TO_BUILD	12

typedef enum
{
	TECHSTATUS_UNAVAILABLE,
	TECHSTATUS_AVAILABLE_TO_PURCHASE,
	TECHSTATUS_RESEARCHING,
	TECHSTATUS_RESEARCHED
} techStatus_enum;

typedef struct
{
	int		objnum;
	
	int		builder[2];

	//requirements
	int		requirements[MAX_REQUIREMENTS];
	int		num_requirements;
	int		minBaseLevel;
	int		minTechLevel;
	int		maxPopulation;
	byte	itemsToBuild[MAX_ITEMS_TO_BUILD];
	int		numItemsToBuild;
} techEntry_t;

#define	GRID_NOFLAGS 					0x00
#define	GRID_VALID_FOR_OTHER_TEAM 		0x01
#define GRID_VALID_FOR_ONE_UNIT_ONLY	0x02

typedef struct
{
	int	identifier;
	char *description;
	char *command;
	char *tooltip;
	char *icon;
	int	flags;
} menuGridItem_t;

typedef struct
{
	byte	object_type;
	
	menuGridItem_t item[GRIDMENU_SIZE];		//4x3 grid
} menuGrid_t;

typedef struct
{
	char *name;

	int items[GRIDMENU_SIZE];
} gridLayout_t;

//both the server and the client use the following functions to determine availability of techtree items
techEntry_t	*Tech_GetEntry(int objtype);
bool		Tech_IsResearchable(int objtype, researchData_t research[]);
bool		Tech_IsResearched(int objtype, researchData_t research[]);
bool		Tech_IsResearchType(int objtype);
bool		Tech_IsAvailable(int objtype, researchData_t research[]);
bool		Tech_IsBuilder(int objtype, int buildertype);
bool		Tech_HasEntry(int objtype);
void		Tech_Generate(objectData_t *(*objectTypeFunc)(int type));

extern int game_objdefs[MAX_OBJECT_TYPES];
extern float frametime;
extern coreAPI_shared_t core;
extern techEntry_t techtree[];
extern char *peonNames[];

extern cvar_t	structureScale;
extern cvar_t	team1_race;
extern cvar_t	team1_racename;
extern cvar_t	team2_race;
extern cvar_t	team2_racename;

//weapons.c
extern	char *evNames[];
extern	char *dmgFlagNames[];
extern	char *chargeFlagNames[];
extern	char *projReactionNames[];
extern	char *projActionNames[];

//object_config.c


void	InitObjectDefinitions();
bool	xMakeString(configVar_t info[], int count, void *data, const char *field, char *sub, char *out, int maxlen);
extern	char *TargetFlagNames[];

//misc_shared.c
extern	residx_t	soundTable;
extern	residx_t	msgTable;
extern	residx_t	texTable;
char	*Snd(const char *id);
char	*GameMsg(const char *id);
char	*Tex(const char *id);
char	*Mdl(const char *id);
char	*UI(const char *id);

char	*GetAnimName(int animState);
char	*GetEventName(int event);
bool	IsMovementAnim(int animState);
void	InitMiscShared();
bool	IsForwardMovementAnim(int animState);
bool	IsSideMovementAnim(int animState);
bool	IsBackwardMovementAnim(int animState);
bool	IsLeftMovementAnim(int animState);
bool	IsRightMovementAnim(int animState);
bool	IsAttackAnim(int animState);
char	*GetGametypeName(int gametype);
int		StringToGametype(const char *string);


//dynstrings.c
char	*DynAllocString(const char *s);
void	DynFree(char *s);
bool	DynSetString(char **s, const char *a);
int		CountActiveObjects();

int		RegisterVarArray(cvar_t varArray[]);
void	InitObjectDefinitions();

bool	SetInt(int *v, int min, int max, char *nameList[], const char *string);
bool	SetIntEx(int *v, configVar_t *info, int argc, char *argv[]);
bool	SetIntRange(intrange *v, int minr, int maxr, int argc, char *argv[]);
bool	SetFloat(float *v, float minr, float maxr, int argc, char *argv[]);
bool	SetFloatRange(floatrange *v, float minr, float maxr, int argc, char *argv[]);
void	SetFlags(int *v, char *flagNames[], int argc, char *argv[]);
bool	SetString(char *v, int maxlen, int argc, char *argv[]);

int		countConfigVars(configVar_t *info);
void	xSave(file_t	*f, configVar_t info[], int count, void *data, const char *pre, int mask);
void	xInit(configVar_t info[], int count, void *data, int id, bool dynstringsonly);
void	xList(configVar_t info[], int count, void *data, int mask);
bool	xSet(configVar_t info[], int count, void *data, int argc, char *argv[]);

//items.c
void	InitItems();

extern configVar_t		objectData_desc[];

//effects.c
extern configVar_t	effectData_desc[];
extern configVar_t	viseffect_desc[];
int	GetEffectNumber(char *name);
void	InitEffects();

//weather.c
extern weatherEffect_t weatherEffectData[MAX_WEATHER_EFFECTS];
int     weLookup(char *name);
void	InitWeather();

//metaconfig.c
int		GetNumResources();
int		GetResourceByName(char *resourceName);
char	*ResourceName(int index);
void	InitMetaConfig();

//states.c
void	InitStates();

//trajectory.c
void	Traj_GetPos(trajectory_t *traj, int time, vec3_t out);
float	Traj_GetVelocity(trajectory_t *traj, int time, vec3_t out);

//exptable.c
void	Exp_Init();
float	Exp_GetPercentNextLevel(int race, int level, int exp);
int		Exp_GetTotalPointsForLevel(int race, int level);

//for mem debugging in linux
#ifdef DMALLOC
#include "dmalloc.h"
#endif

typedef enum
{
	PERF_POSEMODEL,
	PERF_PROCESSOBJECTS,
	PERF_PREDICTION,
	PERF_RENDADDOBJECTS,
	PERF_PARTICLES,
	PERF_RENDER,
	PERF_CLIENTFRAME,
	PERF_DOEFFECT,
	PERF_SPECIALCHARACTERRENDERING,
	PERF_SPECIALSTRUCTURERENDERING,
	PERF_SPECIALPROJECTILERENDERING,
	PERF_SPECIALITEMRENDERING,
	PERF_ADDTOSCENE,
	PERF_ADDWORLDPROPS,
	PERF_SCR_SAMPLEBRIGHTNESS,
	PERF_SCR_STATES,
	PERF_SCR_ATTACHMENTS,
	PERF_SCR_ARMOR,
	PERF_SCR_GROUNDMARKS,
	PERF_SCR_CONTINUOUSBEAM,

	PERF_NUMTYPES
} gamePerfTypes_enum;

extern cvar_t showGamePerf;

#define PERF_BEGIN double __time; if (showGamePerf.integer) __time = core.System_GetPerfCounter(); else __time = 0;
#define PERF_END(perftype) if (showGamePerf.integer) Perf_Count((perftype), core.System_GetPerfCounter() - __time)

void	Perf_Count(int perftype, double amount);
void	Perf_ClearServer();
void	Perf_ClearClient();
void	Perf_Print();

void	TodoList();
