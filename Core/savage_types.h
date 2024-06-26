// (C) 2003 S2 Games

// savage_types.h

// structs and defines shared by both the core engine and game code

#ifndef SAVAGE_TYPES_H
#define SAVAGE_TYPES_H

#include "../masterserver/sharedcommon.h"
#include "savage_common.h"

//version string
//=============================================================================
#ifdef SAVAGE_DEMO
#define DEMO_STRING "DEMO "
#else
#define DEMO_STRING ""
#endif

#define VERSION_STRING "2.00e"

#define DEBUG_STRING ""
#ifdef DEBUG
#define DEBUG_STRING " DEV"
#endif

#ifdef INTERNATIONAL
#define INTL_STRING ""
#else
#define INTL_STRING ""
#endif

#define GAME_VERSION "Savage " DEMO_STRING VERSION_STRING DEBUG_STRING INTL_STRING
//=============================================================================

#define 	CLIENT_NAME_LENGTH	32

//navpaths

#define					GAME_PATH	"/../game/"


#define		MAX_HASH_SIZE		(16+20)

typedef struct file_s
{
	void *file;
	void *data; //data for the codec to store
	bool buffered;

	//compressed files can't seek, so we should eliminate all fseek calls if we can
	int (*seek) (struct file_s *file, long offset, int whence);
	long	(*tell) (struct file_s *file);
	size_t (*read) (void *ptr, size_t size, size_t nmemb, struct file_s *file);
	size_t (*write) (const void *ptr, size_t size, size_t nmemb, struct file_s *stream);
	int (*close) (struct file_s *file);
	int (*eof) (struct file_s *file);
	void (*flush) (struct file_s *file);
} file_t;

typedef enum
{
	ARCHIVE_TYPE_PATH, //not really an archive, but search this path
	ARCHIVE_TYPE_ZIP,
	NUM_ARCHIVE_TYPES
} archiveEnum;

#define ARCHIVE_NORMAL					0x00
#define ARCHIVE_OFFICIAL 				0x01
#define ARCHIVE_THIS_CONNECTION_ONLY 	0x02

typedef struct
{
    bool active;
	char *data; //to store the path or other data for other archive types
	archiveEnum type;
	char hash[MAX_HASH_SIZE];
	int hashlen;
	int flags;

	char *filename;

	void (*System_Dir)(void *archive, char *directory, char *wildcard, bool recurse,
	void(*dirCallback)(const char *dir, void *userdata),
	void(*fileCallback)(const char *filename, void *userdata),
	void *userdata);
} archive_t;

//console command system
#define	CMD_MAX_ARGS	64
#define CMD_MAX_LENGTH	2048
#define SCRIPT_BUFFER_SIZE 20

typedef struct
{
	char	scriptName[_MAX_PATH];
	char	*buffer;
	int		length;
	int		lastRun;
	bool	locked;
}
scriptBufferEntry_t;

//argc contains number of arguments
//argv[0] contains the first argument, etc...
typedef void (*cmdfunc_t)(int argc, char *argv[]);



/*
 *  console variables
 */



#define					CVAR_SAVECONFIG		0x0001
//#define					CVAR_LOBBYDATA		0x02	//changes in variable only communicated in lobby
#define					CVAR_READONLY		0x0004
#define					CVAR_TERRAIN_FX		0x0040	//a color value for the terrain
#define					CVAR_USER_DEFINED	0x0080	//made with the createvar command
#define					CVAR_PATH_TO_FILE	0x0100	//the full path to the file is automatically prefixed when the cvar is set.  if the file does not exist, the cvar is set as normal
#define					CVAR_WORLDCONFIG	0x0200	//the cvar is saved out with the world on a world save
#define					CVAR_CHEAT			0x0400	//the cvar is disabled except when in dev mode
#define					CVAR_TRANSMIT		0x0800	//the server will transmit this variable to the client when it is changed or on a connect
#define					CVAR_NETSETTING		0X1000	//send this cvar value in a message to the server when connecting or when the value changes
#define					CVAR_VALUERANGE		0x2000	//use the lorange and hirange fields of the cvar struct to constrain the variable
#define					CVAR_SERVERINFO		0x4000	//the server will change its ST_SERVER_INFO string whenenver this cvar changes
#define					CVAR_DONTSAVE		0x8000 //don't save a var out when saving vars that match the name

typedef struct cvar_s
{
	char				*name;			//name of variable
	char				*string;		//what the variable is set to	
	int					flags;			//see CVAR_* defines above
	float				lorange;		//used when CVAR_VALUERANGE set
	float				hirange;		//used when CVAR_VALUERANGE set
	
	float				value;			//floating point representation of string
	int					integer;		//integer representation of value
	char				*default_string;
	float				default_value;
	bool				modified;		//set to true if the value has been modified since this was last set to false
	int					modifiedCount;	//incremented every time the var is modified

	//linkage
	struct cvar_s		*nextNetSetting;
	struct cvar_s		*prevNetSetting;
	struct cvar_s		*nextTransmit;
	struct cvar_s		*prevTransmit;
	struct cvar_s		*nextServerInfo;
	struct cvar_s		*prevServerInfo;
	struct cvar_s		*next;
	struct cvar_s		*prev;
	void				*hashkey;
} cvar_t;

//world management

typedef float	heightmap_t;
typedef	vec2_t	texcoordmap_t;
typedef	bvec4_t	colormap_t;
typedef bvec_t	shadermap_t;


//surface flags
#define		SURF_TERRAIN				0x00000001		//RESERVED: surface is part of the terrain heightmap
#define		SURF_STATIC					0x00000002		//RESERVED: surface is a static worldObject
#define		SURF_DYNAMIC				0x00000004		//RESERVED: surface is a dynamic baseObject
#define		SURF_ENTRANCE				0x00000008		//RESERVED: this plane is marked as the entrance to a building
#define		SURF_COMPLEX				0x00000010		//RESERVED: surface is a linked polyhedron as opposed to simple AABB
#define		SURF_GAMEOBJECT				0x00000020		//RESERVED
#define		SURF_NOT_SOLID				0x00000040		//RESERVED
#define		SURF_FOLIAGE				0x00000080		//RESERVED
#define		SURF_MODELBOUNDS			0x80000000		//RESERVED: used internally

#define		SURF_SOLID_ITEM				0x00001000
#define		SURF_TRANSPARENT_ITEM		0x00002000
#define		SURF_CORPSE					0x00004000
#define		SURF_PUSH_ZONE				0x00008000
#define		SURF_REVIVABLE				0x00010000

#define		SURF_EX0					0x00000100
#define		SURF_EX1					0x00000200
#define		SURF_EX2					0x00000400
#define		SURF_EX3					0x00000800
#define		SURF_EX4					0x00020000
#define		SURF_EX5					0x00040000
#define		SURF_EX6					0x00080000
#define		SURF_EX7					0x00100000

//these TRACE_* defines tell the trace function which surfaces to ignore
#define		TRACE_PLAYER_MOVEMENT		(SURF_TRANSPARENT_ITEM | SURF_SOLID_ITEM | SURF_FOLIAGE | SURF_CORPSE | SURF_NOT_SOLID | SURF_PUSH_ZONE)
#define		TRACE_MELEE					(SURF_TERRAIN | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_STATIC)
#define		TRACE_PROJECTILE			(SURF_TRANSPARENT_ITEM | SURF_NOT_SOLID | SURF_FOLIAGE)
#define		TRACE_ALL_SURFACES			0

enum
{
	STATIC_OBJECT = 1,
	GAME_OBJECT
};

typedef struct
{
	float	fraction;		//amount box/ray traveled before hitting something
	vec3_t	endpos;			//point where the box/ray stopped
	int		gridx, gridy;	//grid square where the box/ray stopped

	int		flags;			//surface flags
	int		objectType;		//STATIC_OBJECT or GAME_OBJECT
	int		index;			//if objectType==STATIC_OBJECT, the index to the internal worldObjects array.  if objectType==GAME_OBJECT, the index into the sl/cl.objects array

	vec3_t	normal;			//normal to the surface
	float	dist;			//surface plane dist
//	vec3_t	normal_b;		//normal to the other surface if we hit an edge

	int		collisionType;	//see COL_ defines above

	bool	startedInside;	//the trace started inside the surface
	bool	embedded;		//the trace is entirely inside the surface

	int		dbg_numsurfs;
} traceinfo_t;

typedef struct
{
	float	z;				//height of terrain at this point
	vec3_t	nml;			//surface normal
} pointinfo_t;


//game object management


#define MAX_OBJECTS			1024		//limit for dynamic game objects
#define	MAX_WORLDOBJECTS	(MAX_OBJECTS + 1024) //limit for static world objects

//constants for quick reference of object class variables

//common
typedef enum
{
	OBJVAR_MODEL = 1,
	OBJVAR_SKIN,
	OBJVAR_NAME,
	OBJVAR_SOUND,
	OBJVAR_SOUNDVOLUME,
	OBJVAR_SOUNDMINFALLOFF,
	OBJVAR_EDITORSCALERANGELO,
	OBJVAR_EDITORSCALERANGEHI,
	OBJVAR_EDITORCATEGORY,
	OBJVAR_REFERENCE,
	OBJVAR_NAVFLOODFILLPOINT,
	OBJVAR_NAVBRIDGE,
	OBJVAR_NAVBRIDGEWIDE,
	OBJVAR_INVISIBLETOGAME,
	LAST_OBJVAR_PLUS_ONE
} objectVarIDs_enum;

typedef struct
{
	vec3_t position;
	vec3_t rotation;
	float scale;
} objectPosition_t;


// input
typedef bool (*inputcallback_t)(int key, unsigned char rawchar, bool down, void *userdata);

typedef struct
{
	int x;
	int y;

	int deltax;
	int deltay;

	float realpitch;
	float realyaw;
} mousepos_t;

//special key codes
#define		KEY_TAB			9
#define		KEY_ENTER		13
#define		KEY_ESCAPE		27
#define		KEY_SPACE		32
#define		KEY_BACKSPACE	127
#define		KEY_UP			128
#define		KEY_LEFT		129
#define		KEY_RIGHT		130
#define		KEY_DOWN		131
#define		KEY_ALT			132
#define		KEY_CTRL		133
#define		KEY_SHIFT		134
#define		KEY_F1			135
#define		KEY_F2			136
#define		KEY_F3			137
#define		KEY_F4			138
#define		KEY_F5			139
#define		KEY_F6			140
#define		KEY_F7			141
#define		KEY_F8			142
#define		KEY_F9			143
#define		KEY_F10			144
#define		KEY_F11			145
#define		KEY_F12			146
#define		KEY_INS			147
#define		KEY_DEL			148
#define		KEY_PGDN		149
#define		KEY_PGUP		150
#define		KEY_HOME		151
#define		KEY_END			152
#define		KEY_PAUSE		153

#define		KEY_LBUTTON		200
#define		KEY_RBUTTON		201
#define		KEY_MBUTTON		202
#define		KEY_MWHEELUP	203
#define		KEY_MWHEELDOWN	204

//mouse modes
#define		MOUSE_FREE		0			//default 2d mouse mode.  does not modify yaw/pitch in inputState_t
#define		MOUSE_RECENTER	1			//3d mouse mode.  allows for player 'turning'
#define     MOUSE_FREE_INPUT 2			//similar to MOUSE_FREE, but modifies yaw/pitch in inputState_t (used for siege vehicles)

/*
 Skeletal Animation Interface

  skeleton_t stores transformation information about all the bones in a model.

  Each client object stores its own unique skeleton_t structure to keep track
  of joint movement, which the core engine uses to deform a model.

*/

#define ANIM_NAME_LENGTH	64
#define MAX_BONES 128
//#define MAX_SIMULTANEOUS_ANIM_EVENTS 4
#define ANIM_EVENT_STRING_LENGTH 256
#define MAX_EVENT_CHANNELS	4

typedef enum
{
	SKEL_INVALID,
	SKEL_NORMAL,
	SKEL_ANIM_NOT_FOUND,
	SKEL_ANIM_FINISHED,
	SKEL_BONE_NOT_FOUND,
	SKEL_INVALID_CLIP
} setBoneAnimReturnValues_enum;

#define POSE_MASKED	0x0001

typedef struct
{
	matrix43_t	tm;
	matrix43_t	tm_world;	//computed at Geom_EndPose()
	int			poseState;
	byte		visibility;
} boneState_t;


//the only fields here the game code needs to deal with directly are the 'events' fields
//all other fields are managed internally via the Geom_SetBoneAnim() call

typedef struct
{
	//numEvents and events are filled in on every call to Geom_SetBoneAnim
	//if the animation encountered events since the last time SetBoneAnim was called
	//then numEvents will be > 0 and the appropriate 'events' strings will be filled
	//in, otherwise numEvents will be 0
	bool		gotEvent;
	char		eventString[ANIM_EVENT_STRING_LENGTH];

	/////////////////////////
	//INTERNAL, DO NOT TOUCH!

	int			eventNumber[MAX_EVENT_CHANNELS];	

	char		animName[ANIM_NAME_LENGTH];		//name of the animation currently being played
	int			time;			//time in milliseconds
	int			blendTime;		//time left in milliseconds for blending to occur
	float		animTime;		//time in milliseconds into the animation

	residx_t	model;		//model this skeleton references for its bone data
		
	//current state of all the bones in the skeleton
	int			numBones;
	boneState_t *bones;		//core engine allocates this automatically
//	byte		*visibility;

	bool		invalid;	//set to true if this skeleton is not valid for this frame
	/////////////////////////
} skeleton_t;




//scene management

enum
{
	OBJTYPE_MODEL,
	OBJTYPE_BILLBOARD,
	OBJTYPE_BEAM,
	OBJTYPE_BOX
} objTypes_t;

#define		SCENEOBJ_NO_SHADOW				0x00000001
//#define		SCENEOBJ_NO_TEXTURE				0x00000002
#define		SCENEOBJ_SOLID_COLOR			0x00000004
#define		SCENEOBJ_SHOW_BOUNDS			0x00000008
#define		SCENEOBJ_MESH_BOUNDS			0x00000010
//#define		SCENEOBJ_NO_LIGHTING			0x00000020
#define		SCENEOBJ_NEVER_CULL				0x00000040
//#define		SCENEOBJ_NO_FOG					0x00000080
#define		SCENEOBJ_NO_ZWRITE				0x00000100
#define		SCENEOBJ_NO_ZTEST				0x00000200
#define		SCENEOBJ_SELECTABLE				0x00000400
#define		SCENEOBJ_USE_AXIS				0x00000800
#define		SCENEOBJ_USE_ALPHA				0x00001000
#define		SCENEOBJ_BILLBOARD_ORIENTATION  0x00002000
#define		SCENEOBJ_WIREFRAME				0x00004000
//#define		SCENEOBJ_SPECIFY_SHADER			0x00008000
//#define		SCENEOBJ_DOUBLE_SIDED			0x00010000
//#define		SCENEOBJ_CULL_FRONT				0x00020000		//cull away front facing polygons (unrelated to SCENEOBJ_NEVER_CULL, which works on the object level)
#define		SCENEOBJ_TRANSLATE_ROTATE		0x00040000		//perform rotation first, then translate
#define		SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME	0x00080000		//loframe specifies the frame of the animated texture
#define		SCENEOBJ_BILLBOARD_ALL_AXES		0x00100000
#define		SCENEOBJ_ORIGINAL_ROTATION		0x00200000		//don't internally rotate the model 180 degrees around Z
#define		SCENEOBJ_SKELETON_ATTACHMENT	0x00400000
#define		SCENEOBJ_SHOW_WIRE				0x00800000
#define		SCENEOBJ_RTS_SILHOUETTE			0x01000000		//draw the silhouette if occluded by something.  uses the sceneobject color to determine the color of the silhouette
#define		SCENEOBJ_SINGLE_SHADER			0x02000000		//ignore the skin and use a single shader for meshes
#define		SCENEOBJ_ALWAYS_BLEND			0x04000000

#define		SCENEOBJ_SKYBOX (SCENEOBJ_SOLID_COLOR | SCENEOBJ_ALWAYS_BLEND)

//#define		SCENEOBJ_SKYBOX	(SCENEOBJ_NO_LIGHTING | SCENEOBJ_NO_FOG | SCENEOBJ_NO_ZWRITE /*| SCENEOBJ_NO_ZTEST */| SCENEOBJ_SOLID_COLOR)

typedef struct
{
	int			index;			//provide this index to allow the renderer to cache certain elements

	vec3_t		pos;
	vec3_t		angle;
	vec3_t		beamTargetPos;
	vec3_t		axis[3];		//used only if SCENEOBJ_USE_AXIS is set, otherwise 'angle' is used
	float		scale;

	float		creation_time;	//in seconds, just like the cam->time
								//currently used only for animated shaders
	
	float		alpha;
	
	residx_t	model;
	residx_t	shader;			//for single polys, or used instead of the skin if SCENEOBJ_SINGLE_SHADER is set
	int			skin;			//skin index for models (fixme: we should allow a string to be given here, too)

	float		s1;				//used only if it's a billboard
	float		t1;
	float		s2;
	float		t2;
	
	vec4_t		color;			//used if SCENEOBJ_SOLID_COLOR is set
	
	//the following 3 variables are only used if skeleton == NULL
	int			loframe;		//frame to lerp from (in the default animation)
	int			hiframe;		//frame to lerp to (in the default animation)
	float		lerp_amt;		//amount to lerp between loframe and hiframe

	skeleton_t	*skeleton;		//information about bone positions, for deforming characters
	
	float		width, height;	//for billboards

	int			objtype;

	int			selection_id;	//for mouse->object selection
	int			particle_id;	// for particle system selection

	int			flags;			//see SCENEOBJ_* defines above

	int			*custom_mapping;	//used internally
} sceneobj_t;


typedef enum
{
	LIGHT_OMNI
} lightTypes_enum;


typedef struct
{
	int			lighttype;		//see LIGHT_* defines above

	vec3_t		pos;								
	vec3_t		color;		
	
	float		intensity;		//affects omni lights only
} scenelight_t;

#define MAX_OCCLUDER_POINTS 16

typedef struct
{
	int		numpoints;
	vec3_t	points[MAX_OCCLUDER_POINTS];
} occluder_t;


#define CLEAR_SCENEOBJ(sc) memset(&sc, 0, sizeof(sceneobj_t)); sc.scale = 1; sc.objtype = OBJTYPE_MODEL;

#define		POLY_LIGHTING			0x0001
#define		POLY_DOUBLESIDED		0x0002
#define		POLY_NO_DEPTH_TEST		0x0004
#define		POLY_WIREFRAME			0x0008


typedef struct
{
	vec3_t		vtx;
	vec2_t		tex;
	bvec4_t		col;
} scenefacevert_t;

typedef struct
{
	int			index;
	int			numpoints;
} projectionFragment_t;

#define		CAM_WIREFRAME_TERRAIN	0x00000001
#define		CAM_NO_TERRAIN			0x00000002
#define		CAM_NO_COLORMAP			0x00000004
#define		CAM_NO_SHADERMAP		0x00000008
#define		CAM_NO_SHADERMAP2		0x00000010
#define		CAM_SHOW_BBOXES			0x00000020
#define		CAM_NO_TEXTURE			0x00000040
#define		CAM_NO_WORLDOBJECTS		0x00000080
#define		CAM_NO_SKY				0x00000100
//#define		CAM_NO_LIGHTING			0x0200
#define		CAM_NO_OCCLUDERS		0x00000200
#define		CAM_NO_WORLD	(CAM_NO_TERRAIN | CAM_NO_WORLDOBJECTS | CAM_NO_SKY)

typedef struct
{
	int x, y, width, height;  //viewport
	
	//4x4 transformation matrix (use functions in camutils to manipulate this)	
	vec3_t	viewaxis[3];
	vec3_t	origin;	

	float	fovx, fovy;   //field of view

	float	time;			//for shader animation effects (specified in seconds)	//fixme: specify in milliseconds?

	//flags allow control over terrain rendering, among other things
	int		flags;

	int		mode;

	//if not 0, uses custom near/far settings for fog
	float	fog_near;
	float	fog_far;
} camera_t;


//bitmap
#define	BITMAP_RGB 3
#define	BITMAP_RGBA	4
#define BITMAP_GRAYSCALE 1
#define BITMAP_S3TC_DXT3 GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define BITMAP_S3TC_DXT5 GL_COMPRESSED_RGBA_S3TC_DXT5_EXT

#define S2GRAPHIC_HEADER "S2Graphic"
#define S2GRAPHIC_HEADERLEN 9

typedef struct
{
	unsigned char	*data;

	int				width;
	int				height;
	int				mode;  //8, 16, 24, or 32 (bits)
	int				bmptype;
	bool			translucent;
	int				size; //for compressed textures
} bitmap_t;


typedef enum
{
	CCS_VIRTUAL = -1,
	CCS_DISCONNECTED,
	CCS_AUTHENTICATING,
	CCS_CONNECTING,	
	CCS_AWAITING_STATE_STRINGS,
//	CCS_HASHCOMPARE,
	CCS_LOADING,
	CCS_AWAITING_FIRST_FRAME,
	CCS_IN_GAME
} clientConnectionStates_enum;

typedef enum
{
	SVS_DISCONNECTED,
//	SVS_SETUP,
	SVS_STARTED,
//	SVS_AWAITING_CLIENTS,
	SVS_PLAYING
} serverStatus_enum;


//reserved state strings
typedef enum
{
	ST_SERVER_INFO = 0,			//the string sent to out of band "info" requests
	ST_CVAR_SETTINGS,			//the values of all cvars flagged with the CVAR_TRANSMIT setting
	ST_NUM_RESERVED
} reservedStateStrings_enum;

//shader flags

#define	SHD_TEXCOORD_DEFORM		0x0001
#define	SHD_DISPLACEMENT		0x0002
#define SHD_TERRAIN_MIPMAPS		0x0004  //special mipmap mode for terrain textures
#define	SHD_NO_MIPMAPS			0x0008
#define SHD_NO_LIGHTING			0x0010
#define	SHD_REPEAT				0x0020  //texture is a repeated (tiled) texture
#define	SHD_FULL_QUALITY		0x0040
#define SHD_BUMP_MAP			0x0080  //use a bump map
#define SHD_NO_FOLIAGE			0x0100  //don't draw foliage here
#define SHD_NO_COMPRESS			0x0200  //don't use texture compression on this texture
#define SHD_ALREADY_COMPRESSED	0x0400	//the texture is already compressed
#define SHD_MOVIE_PRELOAD_ALL	0x0800	//precache rather than streaming
#define SHD_MOVIE_LOOP			0x1000	//if not set, movie plays once only

//sound defs

typedef int sound_handle_t;
typedef int sound_id_t;
typedef int sound_buffer_t;

typedef struct
{
	byte			channels;
	byte			bits_per_sample;
	int				sample_rate;
	unsigned int	length; //the length of the music, in samples
} sound_information_t;

typedef enum
{
	SOUND_EVENT_PLAY,			//begin playing the sound
	SOUND_EVENT_PLAY_LOOPING,	//begin playing the sound
	SOUND_EVENT_STOP			//stop playing the sound (sound will stop automatically if it's non-looping)
} object_sound_event_enum;

typedef struct
{
    residx_t					sound;
	object_sound_event_enum		event_type;
	int							frame;
	float						volume;
} obj_sound_t;

typedef struct
{
	char			*name;
	int				num_refs;
	unsigned int	buffer_num;
} sound_t;

// sound management

typedef enum
{
	SC_AUTOCHANNEL = -1,
	SC_CHANNEL1,
	SC_CHANNEL2,
	SC_CHANNEL3,
	SC_CHANNEL4
} objectSoundChannels_enum;

#define SC_LOOP		true
#define SC_NORMAL	false

typedef enum
{
	CHANNEL_AUTO = -1,
	CHANNEL_MUSIC,
	CHANNEL_NOTIFICATION,
	CHANNEL_NOTIFICATION_2,
	CHANNEL_NOTIFICATION_3,
	CHANNEL_GOAL,
	CHANNEL_GUI,
	CHANNEL_CMDR_BUILDING_SELECT,
	CHANNEL_CMDR_UNIT_SELECT,
	CHANNEL_VOICE_CHAT,
	CHANNEL_WEAPON,					//our own weapon only...everyone else uses CHANNEL_AUTO
	CHANNEL_FIRST_AUTO				//= 10
} soundChannels_enum;

//sound priorities
#define PRIORITY_LOWEST -1
#define PRIORITY_LOW 1
#define PRIORITY_MEDIUM 2
#define PRIORITY_HIGH 3

// GUI stuff

struct gui_element_s;

typedef struct
{
	char *name;
	void (*selected)(int item);
} gui_menuitem_t;

#define GUI_ELEMENT_NAME_LENGTH			128
#define GUI_ELEMENT_CLASS_NAME_LENGTH	128
#define GUI_ALT_TEXT_DELAY_TIME			100

#define		UI_PREFIX ""

#define GUI_GETWIDGET(obj) corec.GUI_GetUserdata(class_name, obj);
	
typedef enum
{
	FADE_NONE,
	FADE_IN,
	FADE_OUT
} fadeType_t;

typedef struct gui_element_s
{
	char	name[GUI_ELEMENT_NAME_LENGTH];
	char	class_name[GUI_ELEMENT_CLASS_NAME_LENGTH];

	ivec2_t	bmin;
	ivec2_t	bmax;
	int		width;
	int		height;

	bool	noclip;
	fadeType_t	fadeType;
	float		alpha;			//current alpha

	float		alphaSetting;
	float		fadeTime;		//how long to take to fade it in/out
	float		fadeStart;		//when did we start fading it?

	int		char_height;
	vec3_t	textcolor;

	
	//optional callback to clean up things allocated other than the class struct itself (i.e. textblock allocating an internal buffer, etc.)
	void	(*destroy)(struct gui_element_s *obj);
	
	void	(*move)(struct gui_element_s *obj, int x, int w);
	void	(*draw)(struct gui_element_s *obj, int w, int h);
	void	(*drawalttext)(struct gui_element_s *obj, int w, int h);
	void	(*mousedown)(struct gui_element_s *obj, mouse_button_enum button, int x, int y);
	void	(*mouseup)(struct gui_element_s *obj, mouse_button_enum button, int x, int y);
	void	(*mouseover)(struct gui_element_s *obj, int x, int y);
	void	(*mouseenter)(struct gui_element_s *obj);
	void	(*mouseout)(struct gui_element_s *obj);
	void	(*idle)(struct gui_element_s *obj);
	void	(*show)(struct gui_element_s *obj);
	void	(*hide)(struct gui_element_s *obj);
	void    (*focused)(struct gui_element_s *obj);
	void    (*unfocused)(struct gui_element_s *obj);
	void	(*loseInput)(struct gui_element_s *obj);

	void	(*notify)(struct gui_element_s *obj, int argc, char *argv[]);
	void	(*param)(struct gui_element_s *obj, int argc, char *argv[]);

	char *	(*getvalue)(struct gui_element_s *obj);

	residx_t	mouseover_sound;
	residx_t 	mouseout_sound;
	residx_t	mousedown_sound;
	residx_t 	mouseup_sound;
	
	char	*onMove_cmd;
	char	*onMouseDown_cmd;
	char	*onMouseUp_cmd;
	char	*onMouseOver_cmd;
	char	*onMouseOut_cmd;
	char	*onIdle_cmd;
	char	*onShow_cmd;
	char	*onHide_cmd;

	bool	visible;
	bool	focus;
	bool	interactive;
	bool    loseFocusOnMouseout;
	bool	wantsMouseDownClicks;
	bool	wantsMouseUpClicks;
	bool	wantsFunctionKeys;

	int		flags;

	/*  this shouldn't be here
	gui_menuitem_t	*menuitems;		//if not NULL, element is a menu
	struct  gui_element_s	*menupanel;
	int		selection;
	int		numitems;
	*/

	//optimization variable
	void	*userdata;

	struct	gui_panel_s	 *panel;

	struct  gui_element_s *parent;

	struct  gui_element_s *next, *prev;
} gui_element_t;


typedef struct gui_objectlist_s
{
	struct gui_objectlist_s			*next, *prev;
	gui_element_t					*object;
} gui_objectlist_t;

typedef struct gui_panel_s
{
	struct gui_panel_s	*next, *prev;

	ivec2_t				pos;
	ivec2_t				pivot;
	ivec2_t				bmin;
	ivec2_t				bmax;

	char				*name;
	bool				visible;
	bool				valid;
	bool				staticdepth;

	int					num_objects;
	gui_objectlist_t	objects;

	int					num_children;
	struct gui_panel_s	*parent;
	struct gui_panel_s	**children;
} gui_panel_t;


#define DLLTYPE_NOTLOADED 	0
#define DLLTYPE_GAME		1
#define DLLTYPE_EDITOR  	2



#define	MOVE_FORWARD	0x01
#define	MOVE_BACKWARD	0x02
#define	MOVE_LEFT		0x04
#define	MOVE_RIGHT		0x08
#define	MOVE_UP			0x10		//probably jump
#define	MOVE_DOWN		0x20		//probably crouch
#define MOVE_ZIG		0x40
#define MOVE_ZAG		0x80		//(-:

#define BUTTON1 0x01
#define	BUTTON2	0x02
#define	BUTTON3	0x04
#define	BUTTON4	0x08
#define	BUTTON5	0x10
#define	BUTTON6	0x20
#define	BUTTON7	0x40
#define	BUTTON8	0x80

//inputState_t defines our basic structure for sending the player's input to the server
//the server will use this struct to advance the playerstate, which is sent back to the
//client every server frame.

#define INPUTSTATE_BUFFER_SIZE	64	//this is the size of the buffer that's used in the core for storing the current and previous inputstates on the client for prediction

typedef struct
{	
	unsigned int	sequence;			//used for acknowledgement during prediction
	int				gametime;			//current server game time
	byte			delta_msec;			//time elapsed since last input
	short			pitch;				//rotation about the X (right) axis
	short			roll;				//here for future use, but not currently used
	short			yaw;				//rotation about the Z (up) axis
	byte			movement;			//see MOVE defines above
	byte			buttons;			//see BUTTON defines above
	byte			item;
} inputState_t;

typedef struct
{
	unsigned int    game_id;
	unsigned int    user_id;
	byte			client_id;
	char            cookie[COOKIE_SIZE+1];
	char			name[CLIENT_NAME_LENGTH];
	int             enemy_kills;
	int             enemy_damage;
	int             deaths;
	int             building_damage;
	int             building_kills;
	int             npc_damage;
	int             npc_kills;
	int             ai_damage;
	int             ai_kills;
	int             gold_earned;
	int             gold_spent;
	int				orders_followed;
	int				orders_given;
	bool            won;
	bool            lost;
	bool            commander;
} user_game_stats_t;

struct baseObject_s;

typedef struct
{
	void (*Init)();
	char *(*GetBuild)();
	void (*Frame)(int gametime);
	void (*BeginServerFrame)(int updateTime, unsigned int serverFrame);
	void (*ObjectUpdated)(const struct baseObject_s *serverObj, bool changed);
	void (*ObjectFreed)(int index);
	void (*ObjectNotVisible)(int index);
	void (*EndServerFrame)();	
	void (*DrawForeground)();
	void (*Restart)();
	void (*Shutdown)();
	bool (*ServerMessage)(int clientnum, char *msg);
	bool (*InputEvent)(int key, char rawchar, bool down);
	void (*StateStringModified)(int id);
	void (*ExtendedServerInfo)(const char *ip, int port, int ping, const char *coreInfo, const char *gameInfo);
	void (*PrecacheResources)();
} clientAPI_t;

typedef struct
{
	void (*Init)();
	void (*Shutdown)();
	void (*Reset)(bool first);
	void (*ClientConnect)(int clientnum, const char *netsettings, unsigned int clan_id, bool restarting);
	void (*ClientDisconnect)(int clientnum, const char *reason);
	bool (*ClientMessage)(int clientnum, char *msg);
	void (*NewNetSettings)(int clientnum, const char *settings);
	void (*ProcessClientInput)(int clientnum, inputState_t input);
	void (*KeyserverString)(char *str);
	void (*Frame)(int gametime);
	void (*NewPatchAvailable)();
	void (*NotFirewalled)();
	void (*BuildInfoString)(char *buf, int size);
	void (*BuildExtendedInfoString)(char *buf, int size);
} serverAPI_t;

typedef struct
{
	void (*Init)();
	void (*Frame)();
	bool (*InputEvent)(int key, char rawchar, bool down);
	void (*DrawForeground)();
	void (*Restart)();
	void (*Authenticated)(bool success);
	void (*Authenticated_CDKey)(bool success);
	void (*Authenticated_AccountCreated)(bool success);
	void (*ErrorDialog)(const char *text);
	void (*MessageDialog)(const char *text);
	void (*LoadingStart)();
	void (*LoadingFrame)(const char *currentResource);
	void (*ShowProgressMeter)(const char *currentResource, float progress, int totalSize, int currentAmount, int totalTime);
} interfaceAPI_t;

//network

#define MAX_STATE_STRINGS	256
#define	MAX_CLIENTS	64
#define NUM_TEAMS	3
#define	MAX_MESSAGE_LENGTH	1024

#define	NOTIFY_MESSAGE	0

//the clientnum to use when sending a message as the server itself
#define     SERVER_CLIENTNUM                -1

enum
{
	MESSAGE_PUBLIC,
	MESSAGE_TEAM,
	MESSAGE_PRIVATE
} messageType_t;
  
#define		MAX_WORLDNAME_LENGTH	32

typedef struct
{
	char	worldName[MAX_WORLDNAME_LENGTH];

} worldData_t;



//
// reference objects are used by the server to spawn game-specific objects at the start of a game that were placed down in the editor
//
#define	MAX_OBJECT_NAME_LENGTH	64

typedef struct
{
	bool				active;

	char				refname[MAX_OBJECT_NAME_LENGTH];

	char				info[512];

	objectPosition_t	pos;
} referenceObject_t;



#define MAX_OBJECT_EVENTS 8

#define EVENT_SENT_PARAM1	0x80
#define EVENT_SENT_PARAM2	0x40

typedef struct
{
	byte type;
	byte param;
	byte param2;
} objEvent_t;

//playerstate flags
#define PS_JUMP_LOCKED			0x0001
#define PS_BLOCK_LOCKED			0x0002
#define PS_ATTACK_LOCKED		0x0004
#define PS_ON_GROUND			0x0010
#define PS_CHASECAM				0x0020
#define PS_DAMAGE_PUSH			0x0040	//remove ground info for one frame
#define PS_USE_LOCKED			0x0080
#define PS_INSIDE_TRANSPORT		0x0100
#define PS_NOT_IN_WORLD			0x0200
#define	PS_EXTERNAL_ANGLES		0x0400
#define PS_COMMANDER_SELECTED	0x4000	//this matches the BASEOBJ_ flag in case of a typo

//playerstate attackFlags
#define PS_ATK_FINISHED_IMPACT	0x01
#define PS_ATK_STARTED_LUNGE	0x02
#define PS_ATK_QUEUE_FLURRY		0x04
#define PS_ATK_CLEAR_MELEE_HITS	0x08

#define STUN_MOVEMENT	0x01		//player cannot move
#define STUN_COMBAT		0x02		//player cannot fight

#define	MAX_INVENTORY	16

#define MAX_STATE_SLOTS 32

#define STATUSMSG_WAITING_TO_RESPAWN	0x01
#define STATUSMSG_ATTACK_TO_RESPAWN		0x02
#define STATUSMSG_SPECTATING_PLAYER		0x04
#define STATUSMSG_NAG					0x80

typedef struct
{	
	int				kills;
	int				deaths;
	float			experience;
	int				level;
	int				money;
	int				loyalty;
	int				ping;	
} playerScore_t;

//player state is the player's position in the world, his current heading, statistics, etc
typedef struct
{
	byte	phys_mode;

	short	flags;

	vec3_t	pos;
	vec3_t	angle;
	vec3_t	velocity;
	vec3_t	intent;

	short	attacker;
	
	//movestate and wpstate are used to determine which animation to play, as well as
	//to keep track of the current weapon and movement state in the phys_player* code
	byte	animState;	//animation to play, (may be interrupted by an event)
						//characters may mix event animations and movestate animations into upper/lower body
	byte	animState2;	//upper body animation to play if not set to AS_IDLE

	byte	weaponState;	//state of the weapon we're holding, or melee attack

	int		health;
	int		fullhealth;
	byte	item;

	int		stamina;
	int		maxstamina;

	byte	unittype;	//set to one of the enums in baseObjectTypes_enum (i.e. NOMAD_LIGHT, NOMAD_COMMANDER, etc)

	objEvent_t	events[MAX_OBJECT_EVENTS];		//a single event which happens on one frame and is then cleared
	byte	numEvents;

	float	focus;

	int		wpStateStartTime;	//gametime when the weapon state changed
	int		wpAnimStartTime;
	int		attackFlags;
	int		overheatCounter;
	float	charge;
	int		respawnTime;	//gametime when we will respawn
	int		landTime;		//jump land time
	int		walkTime;		//timestamp when the chracter last stopped sprinting

	bool	checkedImpact;

	byte	states[MAX_STATE_SLOTS];
	int		stateExpireTimes[MAX_STATE_SLOTS];

	byte	stunFlags;		//see STUN_* defines above
	int		stunTime;		//time during which the player can't control his movement (only in effect if the appropriate STUN flag is set (see defines above))

	byte	inventory[MAX_INVENTORY];
	short	ammo[MAX_INVENTORY];
	short	clip[MAX_INVENTORY];
	short	mana;

	vec3_t	chasePos;
	short		chaseIndex;

	int		invincibleTime;

	byte	airMovementState;

	int		insideSurface;

	playerScore_t	score;

	byte	status;

	byte	statusMessage;

	byte	clientnum;

	int		inputSequence;
} playerState_t;

typedef struct
{
	vec3_t	pos;
	vec3_t	velocity;
	vec3_t	intent;
} physicsState_t;

typedef struct
{
	vec3_t	origin;
	vec3_t	velocity;
	float	gravity;		
	float	acceleration;
	int		startTime;
}
trajectory_t;

#define	MAX_UPDATE_OBJECTS	256


#define BASEOBJ_NO_RENDER				0x0001			//don't render
#define BASEOBJ_NO_LINK					0x0002			//don't link into collision system
#define BASEOBJ_WORLDOBJ_REPRESENTS		0x0004			//a worldObject represents the baseObject
#define BASEOBJ_UNDER_CONSTRUCTION		0x0008			//to say that it's currently being built
#define BASEOBJ_NO_ANIMATE				0x0010			//object freezes on last frame of animation (so corpses don't play death aniamtion)
//#define BASEOBJ_NOT_VISIBLE				0x0020			//excluded by fog of war
//#define BASEOBJ_USE_SCALE			0x0020			//scale the object using the 'scale' field
//#define	BASEOBJ_NOT_SOLID		0x0020			//player can interact with this, but it shouldn't block them
#define BASEOBJ_ATTACHED_TO_OWNER		0x0020
#define	BASEOBJ_MARKED_FOR_DEATH		0x0040			//set by SV_DeathEvent, means the object is just hanging out to broadcast it's last event
#define BASEOBJ_USE_TRAJECTORY			0x0080
#define BASEOBJ_ASSIGNED_TO_CLIENT		0x0100
#define BASEOBJ_EXPIRING				0x0200
#define BASEOBJ_HAS_OWNER				0x0400
#define BASEOBJ_INVINCIBLE				0x0800
#define BASEOBJ_NAMEIDX_SPECIFIES_LEVEL 0x1000
#define BASEOBJ_SNAP_TO_MUZZLE			0x2000
#define BASEOBJ_COMMANDER_SELECTED		0x4000			//== PS_COMMANDER_SELECTED
//!!! DO NOT ADD A BASEOBJ FLAG WITH THE VALUE OF 0X8000.  IT WON'T GET TRANSMITTED !!!

#define	BASEOBJEX_FIRING_CONTINUOUS		0x0001


/*==========================

 baseObject_t

 this is the main structure used for sending and receiving object updates over the network

 ==========================*/

typedef struct baseObject_s
{
	//not sent over the network:
	bool	active;

	int		index;

	//bounding box
	vec3_t	bmin; 
	vec3_t	bmax;

	//sent over the network:
	byte	team;
	byte	animState;
	byte	animState2;
	byte	weapon;					//weapon a character is holding	
	int		health;
	int		fullhealth;

	byte	percentToComplete;		//for structures, either the amount of time until building is done or research time for an upgrade	
	byte	states[MAX_STATE_SLOTS];

	byte	nameIdx;  //for peons to have names, or level if BASEOBJ_NAMEIDX_SPECIFIES_LEVEL is set

	byte	assignedToClient;
	
	byte	owner;
	
	objEvent_t	events[MAX_OBJECT_EVENTS];
	byte	numEvents;
	byte	type;		//the type of object to render
	short	flags;		//see BASEOBJ_* defines above
	short	exflags;	//extra flags, see BASEOBJEX_* defines above
	int		surfaceFlags;

	vec3_t	pos;
	vec3_t	angle;
	vec3_t	prVelocity;		//velocity field used for prediction

	trajectory_t	traj;	//for projectile movement
} baseObject_t;



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//this is what we'll pass back to an object looking for a navpath to follow

struct navpoly_s;

typedef struct navpathwaypt_s
{
	vec2_t					position;
	struct navpoly_s*		bridge;		// on a bridge while walking to this waypoint
	struct navpathwaypt_s*	next;
	struct navpathwaypt_s*	prev;
}
navpathwaypt_t;

//----------------------------------------------------------------------------

typedef struct navpathwaypts_s
{
	navpathwaypt_t*			first;
	navpathwaypt_t*			last;
}
navpathwaypts_t;

//----------------------------------------------------------------------------

typedef struct navpath_s
{
	navpathwaypts_t			waypoints;
	navpathwaypt_t*			waypoint;
	vec2_t					src;
	vec2_t					dest;
	const baseObject_t*		obj;
	vec2_t					pos;
	bool					approx;
	int						timeNextOptimize;
}
navpath_t;

//----------------------------------------------------------------------------

typedef enum
{
	navmesh_small	= 0,
//	navmesh_large	= 1,
	
	num_navmeshsizes
}
navmeshsize_t;

//----------------------------------------------------------------------------

//remote commands that can be sent to the tcp port of a server
#define REMOTE_SVCMD_CMD	"svcmd"

//----------------------------------------------------------------------------

#ifndef IGNORE_PACKET_T
#define	MAX_PACKET_SIZE (8192 - HEADER_SIZE)
#define HEADER_SIZE		5 //(sizeof(unsigned int) + sizeof(byte))

typedef struct
{
    unsigned char   __buf[MAX_PACKET_SIZE + HEADER_SIZE];
	//  byte    *buf;
	int     curpos;
	int     length;

	bool	overflowed;
} packet_t;
#endif

typedef struct
{
	char    ip_address[16];
	short   port;
	
	char	coreInfo[512];
	char	gameInfo[512];

	short	ping;
} server_info_t;

typedef struct
{
	int     user_id;
	char    username[40];

	char    server_ip[15];
	int     port;
} user_info_t;

typedef byte exclusionList_t[MAX_OBJECTS];

typedef struct allocator_s
{
	char*	name;
	void*	free;
	int		ref;
	int		peak;
	int		saves;
}
allocator_t;

#define IMPLEMENT_ALLOCATOR(x) allocator_t allocator_##x = { #x, 0, 0, 0, 0 }
#define EXTERN_ALLOCATOR(x)	extern allocator_t allocator_##x;

#endif
