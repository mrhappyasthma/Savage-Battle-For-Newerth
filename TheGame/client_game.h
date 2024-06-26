// (C) 2001 S2 Games

// client_game.h

// definitions for client game code
// included in all client game modules


#include "game.h"
#include "../toplayer/tl_shared.h"
#include "interface.h"

#define UISTATE_NORMAL	0x0000
#define	UISTATE_LOCKED	0x0001

#define MAX_CLIENT_QUEUED_RESEARCH 4

typedef enum
{
	IEVENT_TRIGGER,			//manually requested update
	IEVENT_INVENTORY,		//contents of the players inventory changed
	IEVENT_UNITTYPE,		//players unit type changed
	IEVENT_MONEY,			//players gained/lost money
	IEVENT_DEATH,			//player died
	IEVENT_SPAWN,			//player spawned from a command center/outpost
	IEVENT_SELECTION,		//commander selection changed
	IEVENT_RESOURCES,		//number of resources player/commander has changed
	IEVENT_HEALTH,			//player gained or lost health
	IEVENT_AMMO,			//players ammo total changed
	IEVENT_RESEARCH,		//new item was finished researching
	IEVENT_STATES,			//player gained or lost a state
	IEVENT_REQUEST,			//a request was sent, received, denied, ignored or approved
	IEVENT_DEPLOYMENT,		//received an update to the number of deployed items
	IEVENT_OFFICERS,		//officer was promoted or demoted
	IEVENT_COMMANDER,		//commander resigned, or team has a new commander
	IEVENT_VOTE,			//vote updatated
	
	NUM_IEVENTS
}
interfaceEvents_enum;

char ieventScriptTable[NUM_IEVENTS][64];

extern int	clientObjectTable[MAX_OBJECT_TYPES];


typedef enum
{
	NOTICE_BUILDING_COMPLETE,
	NOTICE_UNDER_ATTACK,
	NOTICE_CHAT,

	NUM_COMMANDER_NOTICES
}
commanderNotice_e;


/*==========================

  clientObject_t

  contains the current baseObject representation sent from the server, 
  as well as several variables which allow us to keep track of all 
  client-side effects.

 ==========================*/

typedef struct clientObject_s
{
	baseObject_t	base;				//this is where updates for the network are stored each frame
	baseObject_t	oldbase;			//the previous network update
	baseObject_t	visual;				//lerped between oldbase and base for smooth visuals

	skeleton_t		skeleton;						//skeleton structure for setting model animations and deforming
	//skeleton_t		armor;
	skeleton_t		stateSkeleton[MAX_STATE_SLOTS];	//for special effects
	int				stateAnimTime[MAX_STATE_SLOTS];

	float			brightness;
	float			lastBrightness;
	float			cloakAmount;
	int				cloakLastAdjust;

	float			animStateTime;			//timer for animstate animation
	float			animState2Time;			//timer for animstate2 animation
	bool			newAnimState;			//the movestate has changed or is being retriggered
	bool			newAnimState2;			//the wpstate has changed or is being retriggered

	int				lastTrailEffect;
	int				lastDamageEffect;
	int				lastDelayEffect;
	int				lastActiveEffect;

	int				lastContinuousEffect;	//stored using cl.systime

	int				trailEffectID;

	bool			hasMuzzle;
	vec3_t			muzzlePos;
	vec3_t			muzzleAxis[3];
	bool			snapToMuzzle;			//gets set in CL_ObjectUpdated and cleared in Rend_DoSpecialProjectileRendering

	int				lastStateEffect[MAX_STATE_SLOTS];

//	bool			linked;
	bool			worldobj_active;

	vec3_t			lastPos;

	vec3_t			pos;
	float			lastPosUpdateTime;

	bool			blink;	
	int				blinkStartTime;

	int				damagePulseTime;
	int				damagePulseDuration;

	residx_t		playingLoopSound;

	int				overrideAnim;
	float			overrideAnimTime;
	int				overrideAnimEnd;

	int				itemConstruction;

	int				researchQueue[MAX_CLIENT_QUEUED_RESEARCH];
	int				numQueuedResearch;

	int				drawFinalTrail;
	bool			triggeredFlyBy;

	sceneobj_t		cachedSC;
	unsigned int	lastSCFrame;

	struct clientObject_s	*nextSpawnPoint;	

	bool			inFogOfWar;

	bool			outsidePVS;

	bool			linked;	
} clientObject_t;



/*==========================

  clientInfo_t

  info about ourself and other players

 ==========================*/

typedef struct
{
	sharedClientInfo_t info;
	playerScore_t		score;
	
	waypoint_t		waypoint;		//commander needs waypoint info for everyone
	int				lastScoreTime;	//last time we got score info for this client
	int				voiceHandle;	//sound handle for voicechat
	int				voiceIconTime;	//gametime voice chat icon will disappear

	skeleton_t		weaponSkel;	//for animating 3rd person weapons
} clientInfo_t;

typedef enum
{
	//wounded sounds increase in degree as their number increases
	SOUND_WOUNDED1,
	SOUND_WOUNDED2,
	SOUND_ATTACK1,
	SOUND_ATTACK2,
	SOUND_FOOTSTEPS_RUN,
	NUM_CHAR_SOUNDS
} charSounds_enum;

typedef enum
{
	WSOUND_SWING1,
	WSOUND_SWING2,
	NUM_WEAPON_SOUNDS
} weaponSounds_enum;


#define	MAX_SKINS	2
//damage states
//#define SKIN_100	0
//#define SKIN_75		1
//#define SKIN_50		2
//#define SKIN_25		3
#define SKIN_TEAM1 0
#define SKIN_TEAM2 1

#define MAX_VISIBLE_STATES 6 //max number of active states shown for the selected unit


typedef struct
{
	bool		loaded;

	residx_t	model;
	residx_t	skin[MAX_SKINS];

	residx_t	handModel;
	residx_t	handSkin;	

	residx_t	projectileModel;
	residx_t	projectileSkin;

	residx_t	*sounds;
} objRes_t;

#define	CAMFX_THROW_ANGLES	0x01
#define	CAMFX_JITTER		0x02
#define CAMFX_TWANG			0x04

#define	MESSAGE_SCROLL_STRING_LENGTH	256

typedef struct
{
	int				cameraEffect;
	vec3_t			cameraAngleParam;
	float			cameraTimeParam;
	float			cameraTime;
	float			cameraNormalize;

	float			cameraNoiseInitialTime;
	float			cameraNoiseTime;
	float			cameraNoiseStrength;

	float			cameraJitterTime;
	float			cameraJitterDuration;
	float			cameraJitterFrequency;
	vec3_t			cameraJitter;

	float			cameraTwangTime;
	float			cameraTwangDuration;
	vec3_t			cameraTwang;
	int				cameraTwangPingPong;

	int				overlayEffect;
	vec4_t			overlayColor;
	vec4_t			overlayColorParam;
	float			overlayTime;
	float			overlayTimeParam;

	//walk bob effect
	float			walkbobTime;
	vec3_t			walkbobPos;
	vec3_t			walkbobAngles;

	//store the result of the camera effect here
	vec3_t			angleOffset;
	vec3_t			posOffset;

	vec2_t			messageScrollVelocity;
	vec2_t			messageScrollPos;
	int				messageScrollEffect;
	residx_t		messageScrollShader;
	char			messageScrollString[MESSAGE_SCROLL_STRING_LENGTH];

} clientEffects_t;

#define	PARTICLE_FLAG_PARENT_POS			0x0001
#define	PARTICLE_FLAG_PARENT_VEL			0x0002
#define PARTICLE_FLAG_PARAM_LOCATION		0x0004
#define PARTICLE_FLAG_PARAM_SCALE			0x0008
#define PARTICLE_FLAG_PARAM_COUNT			0x0010
#define	PARTICLE_FLAG_PARAM_WORLDZ			0x0020
#define	PARTICLE_FLAG_PARAM_SINEWAVE		0x0040
#define PARTICLE_FLAG_PARENT_RADIUS			0x0080
#define PARTICLE_FLAG_BILLBOARD_ORIENTED 	0x0100

typedef struct
{
	bool		active;

	bool		visible;

	int			index;

	vec3_t		velocity;
	float		accel;
	float		gravity;

	vec3_t		pos;			//current position
	residx_t	shader;
	residx_t	model;
	residx_t	skin;
	int			spawntime;		//time created
	float		alpha;
	float		red, green, blue;
	float		width;
	float		height;
	float		angle;
	float		scale;			//used for models instead of wdith/height
	float		deathscale;		//
	int			deathtime;
	float		deathalpha;
	float		deathred, deathgreen, deathblue;
	float		deathwidth;
	float		deathheight;
	float		deathAngle;
	int			birthDelay;
	int			lifespan;
	float		fps;			//specifies amount to increment animated textures
	float		frame;
	bool		ortho;

	float		param;			//optional parameter (used for sine wave multiplier)
	float		param2;			//optional parameter (used for nearparent)

	//for both of these, FAR_AWAY makes it ignored
	float		minZ;			//the minimum Z value for the object (kill it if it goes below this)
	float		maxZ;			//the maximum Z value for the object (kill it if it goes above this)
	float		direction;		//which direction it's facing (if used, just the Z component)

	int			parent;		//follow this object
	int			flags;

	skeleton_t	skeleton;
} simpleParticle_t;

typedef struct
{
	bool	active;
	int		index;

	int		effect;
	
	int		timestamp;

	int		starttime[MAX_SUB_EFFECTS];
	int		last[MAX_SUB_EFFECTS];
	int		expiretime[MAX_SUB_EFFECTS];
	int		plays[MAX_SUB_EFFECTS];
	bool	subactive[MAX_SUB_EFFECTS];

	vec3_t	pos;
	vec3_t	lastpos;

	clientObject_t	*parent;
}
clientEffectObject_t;

typedef struct
{
	bool		active;

	clientEffectObject_t	*efobj;

	int			birthtime;
	int			deathtime;

	residx_t	shader;
	residx_t	model;
	residx_t	skin;

	vec3_t		position;
	vec3_t		velocity;
	float		gravity;
	float		inertia;

	float		scale[4];
	float		scalebias[3];
	int			scaleKeys;

	float		red[4];
	float		redbias[3];
	int			redKeys;
	float		green[4];
	float		greenbias[3];
	int			greenKeys;
	float		blue[4];
	float		bluebias[3];
	int			blueKeys;
	float		alpha[4];
	float		alphabias[3];
	int			alphaKeys;

	float		angle[4];
	float		anglebias[3];
	int			angleKeys;
}
complexParticle_t;

typedef struct
{
	bool	active;

	vec3_t		start, end;
	residx_t	shader;
	int			spawntime, deathtime;
	float		alpha;
	float		red, green, blue;
	float		deathalpha;
	float		deathred, deathgreen, deathblue;
	float		size;
	float		deathsize;
	int			flags;
	float		tilelength;
}
beam_t;

//#include "gui_textbuffer.h"
//#include "gui_graphic.h"
//#include "gui_label.h"
//#include "gui_button.h"

typedef enum
{
	DEFEAT = 1,
	VICTORY
} winStatus_enum;

typedef enum
{
	CMDR_PLACING_OBJECT = 1,			//creating a building
	CMDR_PLACING_LINK,					//placing the second part of a building
	CMDR_PICKING_LOCATION,				//choosing a location (can be a point on the map or another unit)
	CMDR_PICKING_UNIT,					//choosing a unit only as a location
	CMDR_SELECTING_UNITS,
	CMDR_ACTIVATING_TECH,
} commanderModes_enum;

#define	MAX_CMDR_NOTICES	5

typedef struct
{
	int		noticeType;
	int		objnum;
	int		expireTime;
}
commanderNotice_t;

extern char *requestNames[];

typedef struct
{
	requestTypes_enum	type;
	int					money;
	int					object;
	int					timestamp;
	bool				active;
}
cmdrRequest_t;

#define MAX_SHOP_ITEMS		12
#define MAX_SHOP_WEAPONS	12

//this is what every player stores about every team
typedef struct
{
	int race;
	int commander;
	int command_center;
}
cl_teamInfo_t;

#define				MAX_NOTIFICATION_BUFFERS	4
#define				MAX_CHATBOXES				16

#define CMDR_MOUSEDOWN1	0x01
#define CMDR_MOUSEUP1	0x02
#define CMDR_MOUSEDOWN2	0x04
#define CMDR_MOUSEUP2	0x08


typedef struct
{
	clientObject_t	objects[MAX_OBJECTS];

	clientObject_t	effectObjs[256];		//temporary hack
	unsigned int	effectObjCount;

	clientInfo_t	clients[MAX_CLIENTS];
	int				numActiveClients;

	sharedClientInfo_t	*info;				//info about ourself (cl.clients[cl.clientnum].info)
	bool			gotOurInfo;
	clientObject_t	*player;				//points to cl.objects[cl.clientnum]
	int				clientnum;				//cl.predictedState.clientnum

	int				realClientnum;

	int				race;
	int				isCommander;

	int				gametype;

	clientEffects_t	effects;

	float			screenscalex;
	float			screenscaley;

	clientObject_t	*spawnList;
	
	camera_t		camera;

	inputState_t	inputstate, lastInputstate;
	int			wishitem;		//the item we want to select in our inventory
	int				lastwishitem;
	int				nextInventoryCycle;

	vec3_t			heading;
	vec3_t			rightvec;

	playerState_t	playerstate;	//the latest playerstate sent to us from the server
	playerState_t	lastRealPS;		//the previous playerstate sent to us from the server
	playerState_t	predictedState; //the predicted playerstate
	playerState_t	predictionCompareState;
	playerState_t	oldPlayerState; //last frame's predicted playerstate

	skeleton_t		wpSkeleton;		//skeleton used for first person weapon

	int				status;
	int				lastStatus;

	float			bobCycle;		//between -1 and 1
	float			bobHalfCycle;	//between -1 and 1.  half the frequency of bobCycle
	bool			pendingFootstep;
	bool			playedFireEvent;
	int				fireEffectId;

	vec3_t			tpAngles;
	vec3_t			tpPos;
	float			tpNewDistance;
	int				tpLastFrame;

	int				gametime;	//in milliseconds, not guaranteed to be smooth (based off server time)
	int				systime;	//in milliseconds, guaranteed to be smooth (use this for smooth animation)
	float			frametime;	//in seconds
	float			trajectoryTime;	//in milliseconds, the time at which we should evaluate trajectories

	float			todStart;	//time of day start time
	float			todSpeed;	//time of day speed

	int				officers[MAX_OFFICERS];
	int				lastOfficerCmdTime;
	int				lastOfficerSelected;

	int				nextUpdateTime;
	int				lastUpdateTime;
	int				serverFrameTime;
	unsigned int	svFrameNum;
	int				svTime;
	//bool			pendingEvents;

	float			objLerp;

	int				lastInputTime;

	int				weaponSwitchTime;

	int				frame;

	vec3_t			worldbounds;

	mousepos_t		mousepos;

	int				winStatus;
	int				winObject;
	vec3_t			winPos;

	residx_t		blipShader;
	int				blipStartTime;
	int				blipEndTime;

	//common UI elements
	gui_textbuffer_t	*ui_goldgain;
	gui_button_t		*ui_spawnbutton;
	gui_scrollbuffer_t	*ui_notifications[MAX_NOTIFICATION_BUFFERS];
	int					ui_numNotifications;
	gui_textbox_t		*ui_chatBoxes[MAX_CHATBOXES];
	int					ui_numChatBoxes;
	gui_graphic_t		*ui_money_icon;

	gui_panel_t			*ui_playerTaskPanel;
	gui_graphic_t		*ui_leftarrow;
	gui_graphic_t		*ui_rightarrow;

	//commander specific
	vec3_t			cmdrLookAtPoint;		//camera target
	int				cmdrLastYaw;			//for camera "grab" mode
	bool			cmdrJustGrabbedCamera;
	float			cmdrZoom;				//current zoom
	vec3_t			cmdrAngles;				//camera angles
	int				cmdrMode;				//current action (picking a location / creating a building)
	byte			cmdrPlaceObjectType;
	byte			cmdrPlaceObjectTwin;
	vec3_t			cmdrPlaceObjectPos;
	vec3_t			cmdrPlaceObjectAngle;
	float			cmdrPlaceObjectTime;
	bool			cmdrPlaceObjectDirty;
	bool			cmdrPlaceObjectValid;
	int				cmdrPlaceObjectReqId;
	int				cmdrPendingInput;
	int				cmdrGoal;
	int				cmdrLocationType;
	int				cmdrActiveTech;
	bool			fogCleared;
	int				cmdrNextSelectBldSoundTime;
	int				cmdrNextSelectUnitSoundTime;
	
	float			player_nearclip;
	float			player_farclip;

	float			oldAmbient_r;
	float 			oldAmbient_g;
	float			oldAmbient_b;

	bool			guimode;
	bool			draw_world;
	int				uistate;  //used to track if the uistate is LOCKED or not, currently
	
	commanderNotice_t	noticeQueue[MAX_CMDR_NOTICES];
	cmdrRequest_t	cmdrRequests[MAX_CLIENTS];	//requests received as commander
	cmdrRequest_t	pendingRequest;				//info about a pending request sent as a playyer

	int				noticeIndex;
	int				noticeCount;

	unitSelection_t	grouplist[10];
	int				lastGroupCmdTime;
	int				selectedGroup;
	
	unitSelection_t selection;
	unitSelection_t	potentialSelection;
	bool			showSelectionRect;
	ivec2_t			selectionRect_dim;
	ivec2_t			selectionRect_tl;
	
	int				resources[MAX_RESOURCE_TYPES];

	researchData_t	research[MAX_OBJECT_TYPES];
	bool			techModified;		//set to true when we get a techtree update

	bool			thirdperson;

	cl_teamInfo_t	teams[NUM_TEAMS];

	int				serverStatus;
	int				endTime;	

	int				voteEndTime;

	bool			ignoring[MAX_CLIENTS];

	int				statsPopupTime;

	int				lastScoreRequestTime;
	int				lastWaypointUpdate;

	int				lastMinimapRefresh;

	sound_handle_t	hitConfirmSound;	

	bool			voiceActive;	
	voiceMenu_t		*voiceMenu;
	voiceCategory_t *voiceCategory;

	int				deployment[MAX_OBJECT_TYPES];

	int				urgencyLevel;

	int				lastLeftMouseClick;

	char			nextMusic[1024];
	int				nextMusicTime;
	float			musicVolume;

	unsigned int	svframecount;

	clientObject_t	*svObjs[MAX_OBJECTS];
	int				numSvObjs;

	objectData_t	*objData[MAX_OBJECT_TYPES];
	char			*objNames[MAX_OBJECT_TYPES];
	bool			gotObjectTypes;

#ifdef SAVAGE_DEMO
	bool			showingNag;
#endif
} clientLocal_t;


objectData_t *_CL_GetObjectByType(int type);
#define CL_ObjectType(type) ((type) < MAX_OBJECT_TYPES && (type) > 0 ? cl.objData[(type)] : cl.objData[0])
#define IsCharacterType(type) (CL_ObjectType(type)->objclass == OBJCLASS_UNIT && !CL_ObjectType(type)->isVehicle)
#define IsUnitType(type) (CL_ObjectType(type)->objclass == OBJCLASS_UNIT)
#define IsBuildingType(type) (CL_ObjectType(type)->objclass == OBJCLASS_BUILDING)
#define IsWorkerType(type) (CL_ObjectType(type)->isWorker && IsUnitType(type))
#define IsMobileType(type) IsUnitType(type)
#define IsItemType(type) (CL_ObjectType(type)->objclass == OBJCLASS_ITEM)
#define IsWeaponType(type) (CL_ObjectType(type)->objclass == OBJCLASS_WEAPON)
#define IsMeleeType(type) (CL_ObjectType(type)->objclass == OBJCLASS_MELEE)
#define IsUpgradeType(type) (CL_ObjectType(type)->objclass == OBJCLASS_UPGRADE)


typedef struct
{
	float	 hotspotx;		//between 0 and 1
	float	 hotspoty;		//between 0 and 1
	residx_t shader;
} cursor_t;

typedef struct
{
	objRes_t		objres[MAX_OBJECT_TYPES];

	cursor_t		mainCursor;
	cursor_t		errorCursor;
	cursor_t		unknownCursor;
	cursor_t		crosshairCursor;

	residx_t		whiteShader;
	residx_t		blackShader;
	residx_t		digits[10];
	residx_t		longshadowShader;
	residx_t		shadowShader;
	residx_t		spotshadowShader;
	residx_t		lightbeamShader;
	residx_t		lightbeamRedShader;
		
	residx_t		selectionIndicatorSmallShader;
	residx_t		selectionIndicatorLargeShader;

	residx_t		friendlyIcon;
	residx_t		reviveIcon;
	residx_t		voiceIcon;

	residx_t		connectionProblemShader;

	residx_t		waypointMoveIcon;
	residx_t		waypointAttackIcon;

	residx_t		minimap;
} clientResources_t;

extern clientLocal_t cl;
extern clientResources_t res;

/*=============================================================================
Weapons
=============================================================================*/


typedef struct
{
	char		name[MAX_NAME_LEN];

	char		shader[_MAX_PATH];
	floatrange	alpha;
	floatrange	fade;
	intrange	lifetime;
	floatrange	size;
	floatrange	growth;
}
beamData_t;

/*=============================================================================
=============================================================================*/

//cl_render.c
void	Rend_Render();
float	Rend_SceneTime();
void	Rend_Init();
void	Rend_DrawCursor();
void    Rend_SetMouseCursor(cursor_t *cursor);
void    Rend_Draw3dUnitWindow(byte objecttype, int objectIndex, ivec2_t pos, ivec2_t size);
void    Rend_DrawObjectPreview(residx_t model, residx_t skin, vec3_t pos, vec3_t angle,
							   float alpha, float scale, int flags, float frame, bool shadow);
void    Rend_ClientObjectToSceneObject(clientObject_t *obj, sceneobj_t *sc, int flags);
void    Rend_DrawPlayerName();
void	Rend_AddObjects();
void	Rend_AddFogOfWar();
void	Rend_ResetFogOfWar();
void    Rend_DrawWinStatus();

//cl_animation.c

void	CL_InitAnimation();
void	CL_PoseModel(clientObject_t *obj, bool eventsOnly);
void	CL_PoseGenericModel(clientObject_t *owner, residx_t model);
void	CL_PosePlayerModel(clientObject_t *owner, residx_t model);
void	CL_ImpulseAnim(clientObject_t *obj, const char *parentBone, int animstate, float animTime);
void	CL_SetBoneAnim(clientObject_t *obj, const char *parentBone, const char *animName, float animTime, int currentTime, int blendTime);
void	CL_GetBoneWorldTransform(const char *boneName, const vec3_t objectPos, const vec3_t objectAxis[3], float scale, skeleton_t *skel, vec3_t pos, vec3_t axis[3]);


//cl_camera.c
void	Cam_DetermineViewpoint();

//cl_resources.c
void			CL_InitResources();
void			CL_LoadInterfaceResources();
void			CL_PrecacheResources();						//API CALLBACK
int				CL_Skin(byte objtype, int team);
residx_t		CL_Model(byte objnum);
residx_t		CL_WeaponModel(byte objnum);
residx_t		CL_WeaponSkin(byte objnum);
int				CL_HandSkin(byte objtype, int team);
residx_t		CL_HandModel(byte objnum);
residx_t		CL_StateModel(byte objtype, int modelnum, int statenum, int param);
int				CL_StateSkin(byte objtype, int statenum, int param);
void			CL_GetAnimationInfo(clientObject_t *obj, sceneobj_t *sc, int flags);
void			CL_PlayEventAnimation(clientObject_t *obj, int animNum);
char			*CL_GetAnimFromEvent(int event, byte objtype);
void    		CL_LoadEffectResources(int index);

void			CL_ClearUnitSelection(unitSelection_t *selection);
void			CL_GetUnitSelection(unitSelection_t *selection);
void			CL_FinalizeUnitSelection(unitSelection_t *selection);

void			CL_SendTeamRequest(int type);
void			CL_SendTeamJoin(int team);
int				CL_GetNumTeams();
bool			CL_GetTeamCommander(int team);
bool			CL_AddTeam(char *str);
void			CL_SendObjectRequest(int obj_type, vec3_t pos, vec3_t angle);

//cl_main.c
bool    		CL_ClientNameIsOfficer(char *name);
bool    		CL_ClientNameIsCommander(char *name);
bool			CL_ClientNameIsSpectator(char *name);
int				CL_TeamUnitToGlobalUnitNum(int unitnum);
int				CL_AddPositionWaypoint(int sender, int clientnum, vec3_t pos, int goal);
int				CL_AddObjectWaypoint(int sender, int clientnum, int object, int objecttype, int goal);
bool			CL_DestroyWaypoint(int clientnum);
void			CL_SendCommanderRequest();
void			CL_SendCommanderResignation();
void			CL_TechnologyUnavailable(int type);
//void			CL_SendTechnologyListRequest();
int				CL_FindLocalTechnology(int type);
void			CL_SendObjectSpawnRequest(int obj_type);
int     		CL_DetermineGoalForObject(int objidx);
void			CL_Play2d(const char *filename, float volume, int channel);
void			CL_InterfaceEvent(interfaceEvents_enum event);
void			CL_NotifyMessage(const char *msg, const char *soundfile);
char			*CL_RaceSnd(const char *id);
char			*CL_RaceMsg(const char *id);
void			CL_UpdatePendingRequestCvars();
void			CL_CycleInventory(int inc);

//interface.c
void	INT_UpdateBuddyList();

//cl_objects.c
int		CL_GetObjectTypeByName(const char *name);
char	*CL_GetGoalString(int goal);
void	CL_PlayGoalSound(int sender, int goal, int objecttype);
int		CL_GetGoalFromString(char *goaltext);

//cl_commander.c
void	CL_CommanderFrame();
void	CL_CommanderMoveCamera();
bool	CL_CommanderInitializeCamera();
void	CL_CommanderSetupCamera();
void	CL_CommanderInit();
void	CL_CommanderReset();
void	CL_InitEvents();
void	CL_ToggleUnitSelection(unitSelection_t *selection, int obj);
bool	CL_AddToUnitSelection(unitSelection_t *selection, int obj);
bool	CL_UnitIsSelected(unitSelection_t *selection, int obj);
void	CL_ProcessPotentialSelection();
void	CL_CommanderShowUnitMenu(int unit_type);
void	CL_CommanderEnableButtons();
void	CL_CommanderMouseDown1();
void	CL_CommanderMouseUp1();
void	CL_CommanderMouseDown2();
void	CL_CommanderMouseUp2();
bool	CL_CommanderEscapeKey();
void	CL_SendPurchaseRequest(int obj_type, int builder_index);
void	CL_MessageSelected(char *msg);
void	CL_SendLocationGoalToSelection(int goal, vec3_t pos);
void	CL_SendSelectionToServer();
int		CL_GetObjectUnderCursor();
void	CL_StartPlaceObject(int objectType);
void	CL_AddNotification(int index, int type, bool playSound);

//cl_cmdr_interface.c
void	CMDR_RefreshManageUsers();
void	CMDR_UpdateUnitInventory(int unitnum, int numItems, int items[MAX_INVENTORY]);
void	CMDR_InitInterface();
void	CMDR_InitGUIWidgets();
void	CMDR_RotatingModelFrame();
void	CMDR_RefreshSelectionIcons();
void	CMDR_RefreshSelectedView();

//cl_cmdr_requests.c
void	CMDR_AddRequestToQueue(int clientnum, int money, int item);
void	CMDR_InitRequests();
void	CMDR_UpdateRequestCvars();


//cl_cmdr_officer.c
void	CMDR_InitOfficers();
bool	CMDR_UnitIsOfficer(int unitnum);
void	CMDR_PromoteToOfficer(int slot, int client);
void	CMDR_DemoteOfficerUnit(int unitnum);
extern	cvar_t	cl_cmdr_officers[];

//cl_cmdr_gridmenu.c
void	CMDR_InitGridMenu();
void	CMDR_RefreshGrid();
//void	CMDR_SetCurrentGridMenu(gridLayout_t *layout);

//cl_cmdr_upgrades.c
void	CMDR_InitUpgrades();
bool	CMDR_ActivateTech();

//cl_events.c
void	CL_ProcessObjects();			//process all object events
void	CL_DoObjectEvent(clientObject_t *obj, objEvent_t *event);
void	CL_AnimEvent(clientObject_t *obj, const char *eventString);
void	CL_BeginServerFrame(int serverTime, unsigned int serverFrame);		//API callback
void	CL_EndServerFrame();												//API callback
void	CL_ObjectFreed(int index);											//API callback
void	CL_ObjectNotVisible(int index);										//API callback
void	CL_ObjectUpdated(const baseObject_t *serverObj, bool changed);		//API callback
void	CL_AddEvent(int clientnum, byte event, byte param, byte param2);
void	CL_FreeClientObject(int n);


void	CL_ShowCommanderTask(int task, char *message);
void	CL_ReceiveCommanderWaypoint(int clientnum, int goal, vec3_t pos);
void	CL_ReceiveCommanderTask(int clientnum, int goal, int object, int objtype);
void	CL_CommanderTaskTimeout();
void	CL_PlaySound3d(residx_t sound, clientObject_t *obj, vec3_t pos, float volume, int channel);
int		CL_CommanderSound(int objidx, char *wav);
void	CL_FootstepSound(clientObject_t *obj);
void	CL_GetPositionOnObject(clientObject_t *obj, byte encodedPos, vec3_t out);


extern	cvar_t	cl_cmdrNoticePersistTime;
//cl_world.c
float			CL_GetHighestSolidPoint(float x, float y);
float			CL_SampleBrightness(vec3_t pos);

//cl_environment.c
void			CL_InitEnvironment();
void			CL_UpdateEnvironmentEffects();
void			CL_RenderEnvironmentEffects();
void    		CL_ClearLocalParticles();

//cl_playerstate.c
void			CL_ProcessPlayerState();
void			CL_PlayerStateToClientObject(playerState_t *ps, clientObject_t *obj);
void			CL_DamageEffect(int damage);

bool			CL_MouseDown1();
bool			CL_MouseUp1();
bool			CL_MouseDown2();
bool			CL_MouseUp2();

//cl_simpleparticles.c
void    			CL_ResetParticleAllocCount();
simpleParticle_t	*CL_AllocParticle();
complexParticle_t	*CL_AllocParticleEx();
void				CL_AddParticle(simpleParticle_t *particle);
void				CL_ParticleFrame();
void				CL_PlayAnimOnce(simpleParticle_t *particle, int fps, residx_t shader);
void				CL_RenderParticles();
void				CL_RenderExParticles();
void				CL_Draw2dParticles();

void				CL_ResetBeams();
void				CL_ResetParticles();
void				CL_RenderBeams();
beam_t				*CL_AddBeam(vec3_t start, vec3_t end, residx_t shader);
void				CL_InitParticles();

//cl_drawutils.c
void	CL_640x480(int *x, int *y, int *w, int *h);
void	CL_Quad2d(int x, int y, int w, int h, residx_t shader);
void	CL_Quad2d_S(int x, int y, int w, int h, residx_t shader);
void	CL_ShadowQuad2d_S(int x, int y, int w, int h, residx_t shader);

//defs from the gui code  == special gui hooks, essentially
void	GUI_HUD_Center(gui_element_t *obj, vec3_t center);
void	GUI_HUD_SetCells(gui_element_t *obj, int h_cells, int v_cells);

extern	cvar_t	cl_alwaysThirdPerson;
extern	cvar_t	cl_mousepos_x;
extern	cvar_t	cl_mousepos_y;
extern 	cvar_t	cl_showhands;

//cl_effects.c
void	CL_InitEffects();
void	CL_OldEffect(clientObject_t *obj, const char *bone, int effect, byte param, byte param2);
void	CL_DoEffect(clientObject_t *obj, const char *bone, vec3_t pos, int effect, int param, int param2);
void	CL_CameraShake(float duration, float frequency, float x, float y, float z);
void	CL_ProcessEffects();
int		CL_StartEffect(clientObject_t *parent, const char *bone, vec3_t origin, int effect, int param1, int param2);
void	CL_StopEffect(int index);
void	CL_ResetEffects();

//effects.c
int     efLookup(char *name);

//extern	int	cl_objEffectTable[NUM_OBJECT_TYPES][NUM_EVENTS];

//cl_objedit.c
void	CL_InitObjectEditor();

//cl_vote.c
void	CL_VoteInit();
void	CL_VoteUpdateCvars(int id);

//cl_ask.c
void	CL_InitAsk();

bool	CL_Dead();
