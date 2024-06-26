// (C) 2003 S2 Games

// le.h

// level editor

#include "../toplayer/tl_shared.h"

#define PERF_BEGIN double __time; if (showGamePerf.integer) __time = core.System_GetPerfCounter(); else __time = 0;
#define PERF_END(perftype) if (showGamePerf.integer) Perf_Count((perftype), core.System_GetPerfCounter() - __time)

typedef enum
{
	PERF_TERRAINOP,
	PERF_DRAWOBJECTS,
	PERF_DRAWFRAME,
	PERF_TOD,
	PERF_CLEARFRAME,
	PERF_SETUP,
	PERF_DRAWSKY,
	PERF_OCCLUDERS,
	PERF_RENDERSCENE,	
	PERF_NUMTYPES
} gamePerfTypes_enum;


typedef struct
{
	residx_t	mainCursor;
	residx_t	errorCursor;
	residx_t	crosshairCursor;
	residx_t	white;
	residx_t	black;
	residx_t	brushShaders[32];
	residx_t	uparrow;
	residx_t	icon_model;
	residx_t	icon_paint;
	residx_t	icon_texture;
	residx_t	barb;
	residx_t	barbSkin;
	residx_t	tree;
	residx_t	treeSkin;
	residx_t	checkscale_skin;
	residx_t	checkscale_model;
	residx_t	sliderbgShader;
	residx_t	sliderhandleShader;
	residx_t	sliderfillShader;
	residx_t	hsliderbgShader;
	residx_t	hsliderhandleShader;
	residx_t	hsliderfillShader;
	residx_t	shadowShader;
	residx_t	longshadowShader;
	residx_t	spotshadowShader;
} leres_t;

#define			DRAG_ZOOM	1
#define			DRAG_ROTATE	2
#define			DRAG_TILT	3	

#define			BUTTON_LEFT		0x01
#define			BUTTON_RIGHT	0x02
#define			BUTTON_MIDDLE	0x04
#define			BUTTON_SHIFT	0x08
#define			BUTTON_CTRL		0x10
#define			BUTTON_ALT		0x20

//ui state
typedef enum
{
	UISTATE_GAMEWINDOW,
	UISTATE_PAINTING,
	UISTATE_CAMERACONTROL,
	UISTATE_MENUSELECT,
	UISTATE_LOCKED,
	UISTATE_OBJECT_OP
} uistate_enum;

typedef struct
{
	vec3_t		worldbounds;

	//mouse
	mousepos_t	mousepos; //cursor position
	residx_t	cursor;   //current cursor
	camera_t	camera;

	int			button;		//current active mouse button

	int			framecount;
	float		frame;

	int			screenw;
	int			screenh;
	float		screenscalex;
	float		screenscaley;

	int			dragging;
	int			drag_origin_x;
	int			drag_origin_y;
	
	int			uistate;  //see UISTATE #defines above
	
	bool		showmouse;

	bool		popup;

//	vec3_t		cam_pos;
	vec3_t		cam_angle;
//	vec3_t		cam_target;
	vec3_t		cam_tgpos;
	float		cam_zoom;

	int			modepanel_x;
	int			modepanel_y;
	
	vec3_t		user_target;
	bool		locked_user_target;

	bool		rumble;
	vec3_t		velocity;

	bool		showAxes;
	sceneobj_t	axesObj;

	sound_handle_t rumblesound;
} le_t;

le_t le;
leres_t res;

#define	GRID(x, y, gridwidth) ((y) * (gridwidth) + (x))

#include "le_drawutils.h"
#include "le_draw.h"
#include "le_camera.h"
#include "le_tools.h"
#include "le_main.h"
#include "le_ui.h"
#include "le_modelmode.h"
#include "le_texturemode.h"
#include "le_objects.h"
#include "le_pool.h"
#include "le_occluder.h"

// ***** interface functions into engine ******
void	CL_Init();						//called when client code is first loaded (at engine startup)
void	CL_Frame(int gametime);			//called as many times as possible per second
void	CL_Restart();					//called when a new world is loaded
void	CL_DrawForeground();			//called after initial rendering to draw foreground elements (i.e. the cursor)
// ********************************************

void	Perf_Count(int perftype, double amount);
void	Perf_Clear();
void	Perf_Print();


extern cvar_t showGamePerf;

extern cvar_t le_showfps;
extern cvar_t le_scrollaccel;
extern cvar_t le_scrollspeed;
extern cvar_t le_showboxtree;	//displays quadtree in action
extern cvar_t le_noterrain;
extern cvar_t le_wireframe;
extern cvar_t le_notexture;
extern cvar_t le_brushstrength;
extern cvar_t le_usertarget;
extern cvar_t le_checkscale;
extern cvar_t le_modelfps;
extern cvar_t le_testmodel;
extern cvar_t le_testskin;
extern cvar_t le_testparticles;
extern cvar_t le_brushr;
extern cvar_t le_brushg;
extern cvar_t le_brushb;
extern cvar_t le_camera_x;
extern cvar_t le_camera_y;
extern cvar_t le_camera_width;
extern cvar_t le_camera_height;
extern cvar_t le_brush;
extern cvar_t le_flattenheight;
extern cvar_t le_modelmenu_xoffset;
extern cvar_t le_modelmenu_yoffset;
extern cvar_t le_menu_textwidth;
extern cvar_t le_menu_textheight;
extern cvar_t le_bstrength_width;
extern cvar_t le_bstrength_height;
extern cvar_t le_bstrength_xoffset;
extern cvar_t le_bstrength_yoffset;
extern cvar_t le_showlayer;
extern cvar_t le_hotspotx;
extern cvar_t le_hotspoty;
extern cvar_t le_mode;
extern cvar_t le_shadowr;
extern cvar_t le_shadowg;
extern cvar_t le_shadowb;
extern cvar_t le_shadowtype;
extern cvar_t le_shadowalpha;
extern cvar_t le_fov;
extern cvar_t le_freefly;
extern cvar_t le_tod;
extern cvar_t le_selectionSwitch;

extern gui_element_t bstrength;

void	GUI_InitClasses();
