// (C) 2003 S2 Games

// tl_shared.h

// "top layer" shared code between level editor / client game code

#include "savage.h"


void	TL_Register();

//draw utils

typedef enum
{
	SCREEN_COORDS,
	GUI_COORDS,	
	CUSTOM_COORDS
} drawingCoordSystems_enum;

void	TL_SetCoordSystem(int coordSystem);
void	TL_SetCustomCoords(float width, float height);
void	TL_DrawChar(int x, int y, int w, int h, int c, residx_t fontshader);
void	TL_DrawString(int x, int y, const char *string, int charHeight, int rows, int maxWidth, residx_t fontshader);
void	TL_Printf(int x, int y, const char *format, ...);
void	TL_LineBox2d(int x, int y, int w, int h, int thickness);
void	TL_Quad2d(int x, int y, int w, int h, residx_t shader);

//allocation pool

void	TL_Pool_Init();
char	*TL_StrDup(const char *s);
void	*TL_Alloc(int size);

// camera utils

void	TL_Camera_Init();
void	TL_SetCameraCvars(camera_t *cam, vec3_t angles);
void	TL_CopyPosition(camera_t *cam);

// sky

void	TL_InitSky();
void	TL_DrawSky(camera_t *cam);
void	TL_DrawSun(camera_t *cam, sceneobj_t *sun);
void	TL_DrawSunRays(camera_t *cam);
void	TL_ClearBackground();
void	TL_SetTimeOfDay(float minute);

// object vars

void	TL_InitObjectVars();


extern cvar_t		obj_skin; //{ "obj_skin", "" };
extern cvar_t		obj_editorModel; //{ "obj_editorModel", "" };
extern cvar_t		obj_editorSkin; //{ "obj_editorSkin", "" };
extern cvar_t		obj_editorScaleRangeLo; //{ "obj_editorScaleRangeLo", "1" };
extern cvar_t		obj_editorScaleRangeHi; //{ "obj_editorScaleRangeHi", "1" };
extern cvar_t		obj_editorCategory; //{ "obj_editorCategory", "General" };
extern cvar_t		obj_bmin_x; //{ "obj_bmin_x", "-20" };
extern cvar_t		obj_bmin_y; //{ "obj_bmin_y", "-20" };
extern cvar_t		obj_bmin_z; //{ "obj_bmin_z", "0" };
extern cvar_t		obj_bmax_x; //{ "obj_bmax_x", "20" };
extern cvar_t		obj_bmax_y; //{ "obj_bmax_y", "20" };
extern cvar_t		obj_bmax_z; //{ "obj_bmax_z", "0" };
extern cvar_t		obj_name; //{ "obj_name", "NO_NAME" };

//npc
extern cvar_t		npc_hitpoints; //{ "npc_hitpoints", "100" };
extern cvar_t		npc_speed; //{ "npc_speed", "10" };
extern cvar_t		npc_strength; //{ "npc_strength", "10" };
extern cvar_t		npc_respawntime; //{ "npc_respawntime", "10" };

extern coreAPI_client_t corec;
extern coreAPI_shared_t core;
