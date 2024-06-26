// (C) 2003 S2 Games

// le_main.c

// level editor

#include "../le/le.h"

coreAPI_client_t corec;
coreAPI_shared_t core;

cvar_t	showGamePerf = { "showGamePerf", "0" };

cvar_t	le_showfps = { "le_showfps", "1", CVAR_SAVECONFIG };
cvar_t	le_scrollaccel = { "le_scrollaccel", "25", CVAR_SAVECONFIG };
cvar_t  le_scrollspeed = { "le_scrollspeed", "10", CVAR_SAVECONFIG };
cvar_t	le_noterrain = { "le_noterrain", "0" };
cvar_t	le_wireframe = { "le_wireframe", "0" };
cvar_t	le_showboxtree = { "le_showboxtree", "0" };
cvar_t	le_notexture = { "le_notexture", "0" };
cvar_t	le_brushstrength = { "le_brushstrength", "10", 0 };
cvar_t	le_brushr = { "le_brushr", "1", CVAR_SAVECONFIG };
cvar_t	le_brushg = { "le_brushg", "0", CVAR_SAVECONFIG };
cvar_t	le_brushb = { "le_brushb", "0", CVAR_SAVECONFIG };
cvar_t	le_usertarget = { "le_usertarget", "0", CVAR_SAVECONFIG };
cvar_t	le_showgridpos = { "le_showgridpos", "0", CVAR_SAVECONFIG };
cvar_t	le_modelfps = { "le_modelfps", "30", CVAR_SAVECONFIG };
cvar_t	le_testmodel = { "le_testmodel", "" };
cvar_t	le_testskin = { "le_testskin", "default" };
cvar_t	le_testfixpos = { "le_testfixpos", "0", CVAR_SAVECONFIG };
cvar_t	le_mode = { "le_mode", "model" };  //can also be "color", "texture", or "object"
cvar_t	le_brush = { "le_brush", "0", true };
cvar_t	le_flattenheight = { "le_flattenheight", "0" };
cvar_t	le_brushstrength_multiply = { "le_brushstrength_multiply", "0.35", true };
cvar_t	le_showlayer = { "le_showlayer", "0" };
cvar_t	le_shadowr = { "le_shadowr", "0", CVAR_SAVECONFIG };
cvar_t	le_shadowg = { "le_shadowg", "0", CVAR_SAVECONFIG };
cvar_t	le_shadowb = { "le_shadowb", "0", CVAR_SAVECONFIG };
cvar_t	le_shadowalpha = { "le_shadowalpha", "0.7", CVAR_SAVECONFIG };
cvar_t	le_shadowtype = { "le_shadowtype", "2", CVAR_SAVECONFIG };
cvar_t	le_tod = { "le_tod", "650" };
cvar_t	le_freefly = { "le_freefly", "0" };
cvar_t	le_testscale = { "le_testscale", "1" };
cvar_t	le_testangle = { "le_testangle", "0" };
cvar_t	le_testanim = { "le_testanim", "idle", CVAR_SAVECONFIG };

cvar_t	le_sliderbg =		{ "le_sliderbg",		"sliderbg.tga",			CVAR_SAVECONFIG };
cvar_t	le_sliderhandle =	{ "le_sliderhandle",	"sliderhandle.tga",		CVAR_SAVECONFIG };
cvar_t	le_sliderfill =		{ "le_sliderfill",		"sliderfill.tga",		CVAR_SAVECONFIG };
cvar_t	le_hsliderbg =		{ "le_hsliderbg",		"hsliderbg.tga",		CVAR_SAVECONFIG };
cvar_t	le_hsliderhandle =	{ "le_hsliderhandle",	"hsliderhandle.tga",	CVAR_SAVECONFIG };
cvar_t	le_hsliderfill =	{ "le_hsliderfill",		"hsliderfill.tga",		CVAR_SAVECONFIG };

//goofy effects
cvar_t	le_cam_tilt = {"le_cam_tilt", "0", CVAR_SAVECONFIG };
cvar_t	le_rumble = {"le_rumble", "1", CVAR_SAVECONFIG };

//texturing stuff
cvar_t	le_tmatmode = {"le_tmatmode", "0"};

cvar_t le_drawElevation = {"le_drawElevation",	"0", CVAR_SAVECONFIG };

cvar_t le_selectionSwitch = { "le_selectionSwitch", "0" };

cvar_t	le_lightSamples = { "le_lightSamples", "2", CVAR_SAVECONFIG  };
cvar_t	le_lightSampleArea = { "le_lightSampleArea", "100", CVAR_SAVECONFIG };

cvar_t	le_storeUndo = { "le_storeUndo", "1", CVAR_SAVECONFIG };

void	LE_LoadResources()
{
	char *gui_basepath = corec.Cvar_GetString("gui_basepath");

	res.mainCursor = corec.Res_LoadShaderEx("textures/cursors/arrow.tga", SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
	res.errorCursor = corec.Res_LoadShaderEx("textures/cursors/red_x.tga", SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
	res.crosshairCursor = corec.Res_LoadShaderEx("textures/cursors/crosshair.tga", SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
	res.white = corec.GetWhiteShader();
	res.black = corec.Res_LoadShader("textures/black.tga");
	res.sliderbgShader = corec.Res_LoadShader(fmt("%s/%s", gui_basepath, le_sliderbg.string));
	res.sliderhandleShader = corec.Res_LoadShader(fmt("%s/%s", gui_basepath, le_sliderhandle.string));
	res.sliderfillShader = corec.Res_LoadShader(fmt("%s/%s", gui_basepath, le_sliderfill.string));
	res.hsliderbgShader = corec.Res_LoadShader(fmt("%s/%s", gui_basepath, le_hsliderbg.string));
	res.hsliderhandleShader = corec.Res_LoadShader(fmt("%s/%s", gui_basepath, le_hsliderhandle.string));
	res.hsliderfillShader = corec.Res_LoadShader(fmt("%s/%s", gui_basepath, le_hsliderfill.string));
	res.shadowShader = corec.Res_LoadShader("textures/effects/shadow.tga");
	res.longshadowShader = corec.Res_LoadShader("textures/effects/longshadow.tga");
	res.spotshadowShader = corec.Res_LoadShader("textures/effects/spotshadow.tga");
}



void	LE_Center_Cmd(int argc, char *argv[])
{	
	le.camera.origin[0] = le.worldbounds[0] / 2;
	le.camera.origin[1] = le.worldbounds[1] / 2;
	le.camera.origin[2] = 1000;
	le.cam_angle[0] = -45;
	le.cam_angle[1] = 0;
	le.cam_angle[2] = 0;
}	

void	LE_MouseDown1_Cmd(int argc, char *argv[])
{
	gui_element_t *widget;
		
	le.button |= BUTTON_LEFT;
	
	if (le.uistate == UISTATE_LOCKED) return;
	if (le.uistate == UISTATE_CAMERACONTROL) return;

	if ((widget = corec.GUI_GetWidgetAtXY(le.mousepos.x, le.mousepos.y)))
	{
		corec.GUI_Focus(widget);
		le.uistate = UISTATE_LOCKED;
		corec.GUI_SendMouseDown(MOUSE_LBUTTON, le.mousepos.x, le.mousepos.y);  //send mouse down to the UI
	}
	else
	{
		le.uistate = UISTATE_GAMEWINDOW;
		if (le_storeUndo.integer)
			corec.Cmd_BufPrintf("save _undo_");

		if (strcmp(le_mode.string, "object")==0)
		{
			LE_ObjectMouseDown();  //for moving and rotating selected objects, etc
		}
		else if (strcmp(le_mode.string, "occluder")==0)
		{
			LE_OccluderMouseDown();
		}
	}
}

void	LE_AltDown_Cmd(int argc, char *argv[])
{
	le.button |= BUTTON_ALT;
}

void	LE_AltUp_Cmd(int argc, char *argv[])
{
	le.button = le.button & ~BUTTON_ALT;
}

void	LE_CtrlDown_Cmd(int argc, char *argv[])
{
	le.button |= BUTTON_CTRL;
}

void	LE_CtrlUp_Cmd(int argc, char *argv[])
{
	le.button = le.button & ~BUTTON_CTRL;
}

void	LE_ShiftDown_Cmd(int argc, char *argv[])
{
	le.button |= BUTTON_SHIFT;
}

void	LE_ShiftUp_Cmd(int argc, char *argv[])
{
	le.button = le.button & ~BUTTON_SHIFT;
}

void	LE_MouseUp1_Cmd(int argc, char *argv[])
{
	le.button = le.button & ~BUTTON_LEFT;

	if (le.uistate == UISTATE_CAMERACONTROL) return;

	if (le.uistate == UISTATE_LOCKED)
	{
		corec.GUI_SendMouseUp(MOUSE_LBUTTON, le.mousepos.x, le.mousepos.y);		
	}

	if (corec.GUI_CheckMouseAgainstUI(le.mousepos.x, le.mousepos.y))
		le.uistate = UISTATE_MENUSELECT;
	else
	{
		le.uistate = UISTATE_GAMEWINDOW;
		if (strcmp(le_mode.string, "object")==0)
		{
			LE_ObjectMouseUp();
		}
		else if (strcmp(le_mode.string, "occluder")==0)
		{
			LE_OccluderMouseUp();
		}
	}
}

void	LE_MouseDown2_Cmd(int argc, char *argv[])
{
	le.button |= BUTTON_RIGHT;

	if (le.uistate == UISTATE_CAMERACONTROL) return;

	if (corec.GUI_CheckMouseAgainstUI(le.mousepos.x, le.mousepos.y))
	{
		le.uistate = UISTATE_MENUSELECT;
		//don't send a mouse down, right click doesn't affect UI
	}
	else
	{
		le.uistate = UISTATE_PAINTING;		
		if (le_storeUndo.integer)
			corec.Cmd_BufPrintf("save _undo_");

		if (strcmp(le_mode.string, "object")==0)
		{
			LE_ObjectRightMouseDown();
		}
		else if (strcmp(le_mode.string, "occluder")==0)
		{
			LE_OccluderRightMouseDown();
		}
	}
}

void	LE_MouseUp2_Cmd(int argc, char *argv[])
{
	le.button = le.button & ~BUTTON_RIGHT;

//	if (le.uistate == UISTATE_PAINTING)
//	{
		le.uistate = UISTATE_GAMEWINDOW;

		if (strcmp(le_mode.string, "object")==0)
		{
			LE_ObjectRightMouseUp();
		}
		else if (strcmp(le_mode.string, "occluder")==0)
		{
			LE_OccluderRightMouseUp();
		}
//	}
}

sound_handle_t bgmusic;

/*
void	LE_TestSound_Cmd(int argc, char *argv[])
{
	residx_t sound;
	int handle;
	vec3_t pos, velocity;

	velocity[0] = velocity[1] = velocity[2] = 0; 

	pos[0] = 0;
	pos[1] = 0;
	pos[2] = 0;
	
	sound = corec.Res_LoadSound("test.wav");
	
	handle = corec.Sound_PlayLooped(sound, 1.0, zero_vec, true);
	corec.Sound_SetPosition(handle, pos);
	corec.Sound_SetVolume(handle, 0.9);
}
*/

void	LE_ResetParticles_Cmd(int argc, char *argv[])
{
//	corec.Particles_Reset(corec.Particles_GetSystemCount()-1, corec.Seconds());
}


int	InitGameDLL(coreAPI_shared_t *core_api_shared)
{
	memcpy(&core, core_api_shared, sizeof(coreAPI_shared_t));
	return DLLTYPE_EDITOR;
}

bool	CL_InputEvent(int key, char rawchar, bool down)
{
	return false;
}

void	CL_InitAPIs(coreAPI_client_t *core_api, clientAPI_t *le_api)
{
	corec = *core_api;

	le_api->Init = CL_Init;
	le_api->Frame = CL_Frame;
	le_api->Restart = CL_Restart;
	le_api->DrawForeground = CL_DrawForeground;
	le_api->InputEvent = CL_InputEvent;
}

void	LE_HeightShift_Cmd(int argc, char *argv[])
{
	int x, y;
	float	oldheight, shift;
	vec3_t	world_min, world_max;
	float	width, height;

	if (argc < 1)
		return;

	shift = atof(argv[0]);
	corec.World_GetBounds(world_min, world_max);
	width = world_max[0] - world_min[0];
	height = world_max[1] - world_min[1];

	//shift the terrain
	for (x = 0; x < width; x++)
	{
		for (y = 0; y < height; y++)
		{
			oldheight = corec.World_GetGridHeight(x, y);
			corec.World_DeformGround(x, y, oldheight + shift);
		}
	}
}

void	LE_LightMap_Cmd(int argc, char *argv[])
{
	vec3_t wmin, wmax;
	int gridmaxy, gridmaxx;
	int x,y,subx,suby;
	vec3_t sunvec;
	float substep = le_lightSampleArea.value / (le_lightSamples.integer+1);
	int shadow;
	int maxshadow = le_lightSamples.integer * le_lightSamples.integer;
	float shadowstep = 255.0 / maxshadow;
	
	corec.World_GetBounds(wmin,wmax);

	gridmaxx = corec.WorldToGrid(wmax[0]);
	gridmaxy = corec.WorldToGrid(wmax[1]);
	
	corec.WR_GetSunVector(sunvec);

	for (y=0; y<gridmaxy; y++)
	{
		for (x=0; x<gridmaxx; x++)
		{
			bvec4_t color;

			shadow = maxshadow;

			for (suby=0; suby<le_lightSamples.integer; suby++)
			{
				for (subx=0; subx<le_lightSamples.integer; subx++)
				{
					vec3_t start = { corec.GridToWorld(x)+subx*substep, corec.GridToWorld(y)+suby*substep, corec.World_GetGridHeight(x,y) };
					vec3_t end;
					traceinfo_t trace;

					M_PointOnLine(start, sunvec, -99999, end);

					corec.World_TraceBox(&trace, start, end, zero_vec, zero_vec, 0);

					if (trace.fraction == 1)
					{
						shadow--;
					}
				}
			}
			
			corec.WR_GetColormap(x, y, color);		//this is for the alpha info
			color[0] = color[1] = color[2] = 255 - (shadowstep * shadow);
			corec.WR_SetColormap(x, y, color);
		}
	}
}

void	LE_Undo_Cmd(int argc, char *argv[])
{	
	corec.Cmd_BufPrintf("world _undo_");
}

void	LE_LoadBrushes()
{
	int n;
	char s[256];

	//load modeling/painting brushes

	for (n=0; n<MAX_BRUSHES; n++)
	{
		BPrintf(s, 255, "/brushes/standard/brush%i.tga", n+1);
		s[255] = 0;
		LE_SelectBrush(n);
		res.brushShaders[n] = corec.Res_LoadShaderEx(s, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		LE_LoadBrushBmp(s);
	}
}

void	CL_Register()
{

}

void	CL_Init ()
{
	memset(&le, 0, sizeof(le_t));

	LE_LoadResources();

	TL_Register();
	
	corec.Cvar_Register(&showGamePerf);
	corec.Cvar_Register(&le_showfps);
	corec.Cvar_Register(&le_scrollaccel);
	corec.Cvar_Register(&le_scrollspeed);
//	Cvar_Register(&le_showboxtree);
	corec.Cvar_Register(&le_noterrain);
	corec.Cvar_Register(&le_wireframe);
	corec.Cvar_Register(&le_notexture);
	corec.Cvar_Register(&le_brushstrength);
	corec.Cvar_Register(&le_brushr);
	corec.Cvar_Register(&le_brushg);
	corec.Cvar_Register(&le_brushb);
	corec.Cvar_Register(&le_usertarget);
	corec.Cvar_Register(&le_showgridpos);
	corec.Cvar_Register(&le_modelfps);
	corec.Cvar_Register(&le_testmodel);
	corec.Cvar_Register(&le_testskin);
	corec.Cvar_Register(&le_brush);
	corec.Cvar_Register(&le_flattenheight);
	corec.Cvar_Register(&le_mode);
	corec.Cvar_Register(&le_brushstrength_multiply);
	corec.Cvar_Register(&le_showlayer);
	corec.Cvar_Register(&le_shadowr);
	corec.Cvar_Register(&le_shadowg);
	corec.Cvar_Register(&le_shadowb);
	corec.Cvar_Register(&le_shadowalpha);
	corec.Cvar_Register(&le_shadowtype);
	corec.Cvar_Register(&le_tod);
	corec.Cvar_Register(&le_sliderbg);
	corec.Cvar_Register(&le_sliderhandle);
	corec.Cvar_Register(&le_sliderfill);
	corec.Cvar_Register(&le_hsliderbg);
	corec.Cvar_Register(&le_hsliderhandle);
	corec.Cvar_Register(&le_hsliderfill);
	corec.Cvar_Register(&le_testscale);
	corec.Cvar_Register(&le_testangle);
	corec.Cvar_Register(&le_testfixpos);
	corec.Cvar_Register(&le_testanim);

	corec.Cvar_Register(&le_cam_tilt);
	corec.Cvar_Register(&le_rumble);

	corec.Cvar_Register(&le_fov);

	corec.Cvar_Register(&le_freefly);

	corec.Cvar_Register(&le_selectionSwitch);
	
	corec.Cvar_Register(&le_lightSamples);
	corec.Cvar_Register(&le_lightSampleArea);

	corec.Cvar_Register(&le_storeUndo);

	corec.Cmd_Register("undo", LE_Undo_Cmd);
	corec.Cmd_Register("center", LE_Center_Cmd);
	corec.Cmd_Register("mousedown1", LE_MouseDown1_Cmd);
	corec.Cmd_Register("mouseup1", LE_MouseUp1_Cmd);
	corec.Cmd_Register("mousedown2", LE_MouseDown2_Cmd);
	corec.Cmd_Register("mouseup2", LE_MouseUp2_Cmd);	
	corec.Cmd_Register("shiftdown", LE_ShiftDown_Cmd);
	corec.Cmd_Register("shiftup", LE_ShiftUp_Cmd);
	corec.Cmd_Register("ctrldown", LE_CtrlDown_Cmd);
	corec.Cmd_Register("ctrlup", LE_CtrlUp_Cmd);
	corec.Cmd_Register("altdown", LE_AltDown_Cmd);
	corec.Cmd_Register("altup", LE_AltUp_Cmd);
	corec.Cmd_Register("lightmap", LE_LightMap_Cmd);
	corec.Cmd_Register("heightshift",	LE_HeightShift_Cmd);

	corec.Cmd_Register("reset_particles", LE_ResetParticles_Cmd);
	
	corec.Cvar_Register(&le_drawElevation);

	TL_Pool_Init();
	LE_Pool_Init();

	le.screenw = corec.Vid_GetScreenW();
	le.screenh = corec.Vid_GetScreenH();
	le.screenscalex = (float)le.screenw / 640.0;
	le.screenscaley = (float)le.screenh / 480.0;

	LE_InitTools();
	GUI_InitClasses();

//	LE_InitBrushFuncs();

	le.modepanel_x = 0;
	le.modepanel_y = 448;

	LE_ModelMode_Init();
//	LE_TextureMode_Init();

	TL_InitSky();
	
	corec.Cmd_Exec("exec /ui.cfg");

	LE_Objects_Init();

	LE_InitCamera();

	corec.Input_CenterMouse();

	le.uistate = UISTATE_GAMEWINDOW;

	LE_LoadBrushes();

	LE_InitOccluder();
}


void	LE_ShutDown()
{
	corec.World_Destroy();
}



void	LE_HandleMouseInput()
{
	if (le.uistate == UISTATE_LOCKED) 
	{
		corec.GUI_CheckMouseAgainstUI(le.mousepos.x, le.mousepos.y); //for mouseover events
		return;
	}
	
	if (le_mouselook.integer || le_targetrotate.integer)
	{
		le.uistate = UISTATE_CAMERACONTROL;
		le.locked_user_target = true;

		if (!le_targetrotate.integer)
		{
			traceinfo_t trace;
			
			if (LE_CursorTrace(&trace, 0))
			{
				M_CopyVec3(trace.endpos, le.user_target);
			}
		}

		LE_StartCameraControl();

		return;
	}

	if (!le_mouselook.integer && !le_targetrotate.integer)
	{		
		if (le.uistate == UISTATE_CAMERACONTROL)
		{
			le.locked_user_target = false;
			le.uistate = UISTATE_GAMEWINDOW;
			LE_StopCameraControl();
		}
	}

	//see if the mouse cursor is over any UI elements	
	if (corec.GUI_CheckMouseAgainstUI(le.mousepos.x, le.mousepos.y))
	{
		le.uistate = UISTATE_MENUSELECT;	
		LE_SetCursor(res.mainCursor);
		LE_SetHotspot(0,0);
	}
	else
	{
		LE_SetCursor(res.crosshairCursor);
		LE_SetHotspot(16,16);
		le.uistate = UISTATE_GAMEWINDOW;
		LE_ObjectMouseOver();
	}
}

void	LE_ResetObjects();


void	CL_Restart ()
{
	static bool startup = true;

	vec3_t nul;

	le.screenw = corec.Vid_GetScreenW();
	le.screenh = corec.Vid_GetScreenH();
	le.screenscalex = (float)le.screenw / 640.0;
	le.screenscaley = (float)le.screenh / 480.0;

	le.camera.flags = 0;//CAM_WIREFRAME_TERRAIN;

	corec.World_GetBounds(nul, le.worldbounds);

	le.showmouse = true;
	corec.Input_SetMouseMode(MOUSE_FREE);

	le.locked_user_target = false;	

	LE_SetCursor(res.mainCursor);

	LE_ResetObjects();

	if (startup)
	{
		LE_Center_Cmd(0,NULL);

		le.camera.time = 0;

		startup = false;
	}

}

void	LE_RumbleOn()
{/*
	if (!le_rumble.integer)
		return;

	if (!le.rumble)
	{
		le.rumblesound = corec.Sound_Play(corec.Res_LoadSound(GAME_PATH "sfx/rumble2_mono.wav"), -1, NULL, 1.0, CHANNEL_GUI, 0);

		le.rumble = true;
	}
*/
}

void	LE_RumbleOff()
{/*
	if (le.rumble)
	{
		corec.Sound_Stop(le.rumblesound);
		le.rumble = false;
	}*/
}

bool	LE_CursorTrace(traceinfo_t *result, int ignoreSurface)
{
	vec3_t dir,end;

	Cam_ConstructRay(&le.camera, le.mousepos.x, le.mousepos.y, dir);

	M_PointOnLine(le.camera.origin, dir, 99999, end);

	return corec.World_TraceBox(result, le.camera.origin, end, zero_vec, zero_vec, 0);
}




void	_LE_DoTerrainOperation()
{
	static traceinfo_t trace;
	char	s[256];
	static int oldmousex, oldmousey;
	vec4_t white = {1,1,1,1};

	if (le.uistate != UISTATE_GAMEWINDOW) return;

	if (!le.locked_user_target)
	{
		if (LE_CursorTrace(&trace, 0))
		{
			M_CopyVec3(trace.endpos, le.user_target);	//might want to use this as a rotation target
			le.user_target[2] += 15;
		}
	}
	else
	{
		return;
	}
	
	oldmousex = le.mousepos.x;
	oldmousey = le.mousepos.y;

	LE_SetBrushStrength(le_brushstrength.value);

//	corec.WR_ClearDynamap();


	if (le_showgridpos.value)
	{
		BPrintf(s, 255, "Gridx: %i  Gridy: %i", trace.gridx, trace.gridy);
		s[255] = 0;
		corec.Draw_SetColor(white);
		TL_DrawString(0,200,s,12, 1024, 1, corec.GetNicefontShader());	
	}

	if (strcmp(le_mode.string, "model")==0)
	{
		if (trace.flags & SURF_TERRAIN)
		{

			if (le.button & BUTTON_LEFT)
			{
				LE_SetBrushStrength(le_brushstrength.value * le_brushstrength_multiply.value);
	//			LE_SetModelFunction(LE_GetSelectedModelFunction());
				LE_ModelTerrain(trace.gridx, trace.gridy);

				LE_RumbleOn();
			}
			else if (le.button & BUTTON_RIGHT)
			{
				LE_SetBrushStrength(-le_brushstrength.value * le_brushstrength_multiply.value);
	//			LE_SetModelFunction(LE_GetSelectedModelFunction());
				LE_ModelTerrain(trace.gridx, trace.gridy);	

				LE_RumbleOn();
			}
			else
			{
				LE_RumbleOff();
			}
		}
	}
	else
	{
		LE_RumbleOff();
	}

	if (strcmp(le_mode.string, "color")==0)
	{
		if (le.button & BUTTON_LEFT)
		{
			LE_ColorTerrain(trace.gridx, trace.gridy);
		}
		else if (le.button & BUTTON_RIGHT)
		{
			float oldr, oldg, oldb;

			oldr = le_brushr.value;
			oldg = le_brushg.value;
			oldb = le_brushb.value;
			le_brushr.value = 2;
			le_brushg.value = 2;
			le_brushb.value = 2;
			LE_ColorTerrain(trace.gridx, trace.gridy);
			le_brushr.value = oldr;
			le_brushg.value = oldg;
			le_brushb.value = oldb;
			//shouldn't really change cvars this way, but here it's ok since we are restoring the values after one call
		}
	}

	if (strcmp(le_mode.string, "texture")==0)
	{
		if (le_tmatmode.value)
		{
			tmat_t *tm;
			tm = LE_CurrentTMat();

			if (tm && tm->width && tm->height)
			{
				if (le.button & BUTTON_LEFT)
				{				
					LE_PaintTMat(trace.gridx - (trace.gridx % tm->width), trace.gridy - (trace.gridy % tm->height), 1.0);					
				}
				else if (le.button & BUTTON_RIGHT)
				{					
					LE_PaintTMat(trace.gridx - (trace.gridx % tm->width), trace.gridy - (trace.gridy % tm->height), 0.0);
				}
			}
		}
		else
		{
			if (le.button & BUTTON_LEFT)
			{
				if (le.button & BUTTON_SHIFT)
				{
					if (le_showlayer.value == 0)
						LE_PaintAlpha(trace.gridx, trace.gridy, 1.0);
				}
				else
				{
					LE_TextureTerrain(trace.gridx, trace.gridy);
					if (le_showlayer.value == 0)
						LE_PaintAlpha(trace.gridx, trace.gridy, 1.0);
				}
			}
			else if (le.button & BUTTON_RIGHT)
			{
				if (le_layer2.value)
					LE_PaintAlpha(trace.gridx, trace.gridy, 0.0);
			}
		}
	}
/*
	if (strcmp(le_mode.string, "object")==0)
	{
		if (le.button & BUTTON_LEFT)
			LE_PlaceObject(trace.endpos[0], trace.endpos[1]);
	}
	*/
}

void	LE_DoTerrainOperation()
{
	PERF_BEGIN;
	_LE_DoTerrainOperation();
	PERF_END(PERF_TERRAINOP);
}

void	LE_UIFrame()
{
	LE_SelectBrush(le_brush.value);
}

void	CL_Frame (int gametime)
{
	Perf_Clear();

	le.showAxes = false;
	CLEAR_SCENEOBJ(le.axesObj);
	le.axesObj.objtype = OBJTYPE_MODEL;
	le.axesObj.model = corec.Res_LoadModel("/models/axes.model");
	le.axesObj.shader = corec.GetWhiteShader();
	le.axesObj.flags = SCENEOBJ_SINGLE_SHADER;

	TL_SetCoordSystem(SCREEN_COORDS);
	corec.Draw_SetShaderTime((float)corec.Milliseconds() / 1000.0);

	corec.Input_GetMousePos(&le.mousepos);
	LE_HandleMouseInput();
		
	LE_UIFrame();

	LE_DoTerrainOperation();

	LE_DrawFrame();

	le.framecount++;

	le.frame += le_modelfps.value / (1.0 / (corec.FrameSeconds()));

	TL_SetCoordSystem(GUI_COORDS);

	Perf_Print();
}

extern void LE_ColorReachableInit();

void	INT_Init()
{	
	LE_ColorReachableInit();
}

void	INT_Frame()
{
}
void	INT_Restart()
{
}
void	INT_DrawForeground()
{
		CL_DrawForeground();
}

int		INT_InputEvent(int key, char rawchar, bool down)
{
	return 0;
}

void	ErrorDialog()
{
}

void	ShutdownGameDLL()
{
}

void	INT_InitAPIs(coreAPI_interface_t *core_int_api, interfaceAPI_t *game_interface_api)
{
	game_interface_api->Init = INT_Init;
	game_interface_api->Frame = INT_Frame;
	game_interface_api->Restart = INT_Restart;
	game_interface_api->DrawForeground = INT_DrawForeground;
	game_interface_api->InputEvent = INT_InputEvent;
}

// "COLOR REACHABLE" hack functionality

cvar_t	colorReachableStepSize = { "colorReachableStepSize", "100", CVAR_SAVECONFIG };
cvar_t	colorReachableMinSlope = { "colorReachableMinSlope", "0.7", CVAR_SAVECONFIG };

#define WORLD_TO_STEP(x) ((x)/colorReachableStepSize.value)
#define STEP_TO_WORLD(x) ((x)*colorReachableStepSize.value)

#define ALLOCATE(x) core.Allocator_Allocate(&allocator_##x, sizeof(x))
#define DEALLOCATE(x, pv) core.Allocator_Deallocate(&allocator_##x, pv)

typedef struct flood_node_s
{
	int						x;
	int						y;
	int						lr;
	struct flood_node_s*	next;
}
flood_node_t;
IMPLEMENT_ALLOCATOR(flood_node_t);

void LE_ColorReachable(int argc, char *argv[])
{
	vec3_t wmin, wmax;
	int x, y, cx, cy;
	int halfstep = colorReachableStepSize.value/2;
	int quarterstep = colorReachableStepSize.value/4;
	pointinfo_t pi;
	char* pbyData;
	int n;

	corec.World_GetBounds(wmin,wmax);

	cx = WORLD_TO_STEP(wmax[0]);
	cy = WORLD_TO_STEP(wmax[1]);

	pbyData = malloc(cx*cy);

	for ( y = 0 ; y < cy ; ++y )
	{
		for ( x = 0 ; x < cx ; ++x )
		{
			int wx0, wy0, wx1, wy1;
			wx0 = STEP_TO_WORLD(x);
			wy0 = STEP_TO_WORLD(y);
			wx1 = STEP_TO_WORLD(x+1);
			wy1 = STEP_TO_WORLD(y+1);

			pbyData[y*cx + x] = 0;

			{
				corec.World_SampleGround(wx0+quarterstep, wy0+quarterstep, &pi);

				if ( pi.nml[2] > colorReachableMinSlope.value )
				{
					pbyData[y*cx + x] |= 0x80;
				}
				
				corec.World_SampleGround(wx1-quarterstep, wy1-quarterstep, &pi);

				if ( pi.nml[2] > colorReachableMinSlope.value )
				{
					pbyData[y*cx + x] |= 0x08;
				}
			}
		}
	}

	// now flood out to get rid of unreachable polies
		
	for ( n = 0 ; n < MAX_OBJECTS ; n++ )
	{
		flood_node_t* floodnodes = NULL;
		flood_node_t* floodnode = NULL;

		objectPosition_t pos;
		int objdef = corec.WO_GetObjectObjdef(n);
		if ( objdef == 0 || !corec.WO_GetObjdefVarValue(objdef, "obj_navFloodFillPoint") )
		{
			continue;
		}

		corec.WO_GetObjectPos(n, &pos);

		floodnodes = ALLOCATE(flood_node_t);
		floodnodes->next = NULL;
		floodnodes->x = WORLD_TO_STEP(pos.position[0]);//colorReachablefloodx.value;
		floodnodes->y = WORLD_TO_STEP(pos.position[1]);//colorReachablefloody.value;
		floodnodes->lr = 0;
		floodnode = floodnodes;
		pbyData[floodnode->y*cx + floodnode->x] |= 0x40;

		while ( floodnode )
		{			
			int x = floodnode->x;
			int y = floodnode->y;
			int lr = floodnode->lr;

			floodnodes = floodnodes->next;

			if ( 0 == lr ) // left
			{
				// check left, bottom, and diag

				if ( (x > 0) && (pbyData[y*cx + (x-1)] & 0x08) && !(pbyData[y*cx + (x-1)] & 0x04) )
				{
					flood_node_t* floodnodenew = ALLOCATE(flood_node_t);
					floodnodenew->x = x-1;
					floodnodenew->y = y;
					floodnodenew->lr = 1;
					floodnodenew->next = floodnodes;
					floodnodes = floodnodenew;
					pbyData[floodnodenew->y*cx + floodnodenew->x] |= 0x04;
				}
				if ( (y < (cy-1)) && (pbyData[(y+1)*cx + x] & 0x08) && !(pbyData[(y+1)*cx + x] & 0x04) )
				{
					flood_node_t* floodnodenew = ALLOCATE(flood_node_t);
					floodnodenew->x = x;
					floodnodenew->y = y+1;
					floodnodenew->lr = 1;
					floodnodenew->next = floodnodes;
					floodnodes = floodnodenew;
					pbyData[floodnodenew->y*cx + floodnodenew->x] |= 0x04;
				}
				if ( (pbyData[y*cx + x] & 0x08) && !(pbyData[y*cx + x] & 0x04) )
				{
					flood_node_t* floodnodenew = ALLOCATE(flood_node_t);
					floodnodenew->x = x;
					floodnodenew->y = y;
					floodnodenew->lr = 1;
					floodnodenew->next = floodnodes;
					floodnodes = floodnodenew;
					pbyData[floodnodenew->y*cx + floodnodenew->x] |= 0x04;
				}
			}
			else // if ( 0 == lr )
			{
				// check right, top, and diag

				if ( (x < (cx-1)) && (pbyData[y*cx + (x+1)] & 0x80) && !(pbyData[y*cx + (x+1)] & 0x40) )
				{
					flood_node_t* floodnodenew = ALLOCATE(flood_node_t);
					floodnodenew->x = x+1;
					floodnodenew->y = y;
					floodnodenew->lr = 0;
					floodnodenew->next = floodnodes;
					floodnodes = floodnodenew;
					pbyData[floodnodenew->y*cx + floodnodenew->x] |= 0x40;
				}
				if ( (y > 0) && (pbyData[(y-1)*cx + x] & 0x80) && !(pbyData[(y-1)*cx + x] & 0x40) )
				{
					flood_node_t* floodnodenew = ALLOCATE(flood_node_t);
					floodnodenew->x = x;
					floodnodenew->y = y-1;
					floodnodenew->lr = 0;
					floodnodenew->next = floodnodes;
					floodnodes = floodnodenew;
					pbyData[floodnodenew->y*cx + floodnodenew->x] |= 0x40;
				}
				if ( (pbyData[y*cx + x] & 0x80) && !(pbyData[y*cx + x] & 0x40) )
				{
					flood_node_t* floodnodenew = ALLOCATE(flood_node_t);
					floodnodenew->x = x;
					floodnodenew->y = y;
					floodnodenew->lr = 0;
					floodnodenew->next = floodnodes;
					floodnodes = floodnodenew;
					pbyData[floodnodenew->y*cx + floodnodenew->x] |= 0x40;
				}
			}

			DEALLOCATE(flood_node_t, floodnode);
			floodnode = floodnodes;
		}
	}

	// Clear out tris with no flooded bit (0x40 or 0x04) set

	corec.Cvar_Set("gfx_fog", "0.0");
	corec.Cvar_Set("gfx_farclip", "10000.0");
	corec.Cvar_Set("ter_ambient_r", "2.0");
	corec.Cvar_Set("ter_ambient_g", "2.0");
	corec.Cvar_Set("ter_ambient_b", "2.0");

	for ( y = 0 ; y < cy ; ++y )
	{
		for ( x = 0 ; x < cx ; ++x )
		{
			bvec4_t red = { 255, 0, 0, 255 };
			bvec4_t green = { 0, 255, 0, 255 };

			if ( !(pbyData[y*cx + x] & 0x40) )
			{
				pbyData[y*cx + x] &= 0x0f;
			}

			if ( !(pbyData[y*cx + x] & 0x04) )
			{
				pbyData[y*cx + x] &= 0xf0;
			}
			
			corec.WR_SetColormap(x, y, pbyData[y*cx+x]&0x04 ? green : red);
			corec.WR_SetColormap(x+1, y, pbyData[y*cx+x]&0x04 ? green : red);
			corec.WR_SetColormap(x, y+1, pbyData[y*cx+x]&0x04 ? green : red);
			corec.WR_SetColormap(x+1, y+1, pbyData[y*cx+x]&0x40 ? green : red);
			corec.WR_SetColormap(x+1, y, pbyData[y*cx+x]&0x40 ? green : red);
			corec.WR_SetColormap(x, y+1, pbyData[y*cx+x]&0x40 ? green : red);
		}
	}

	free(pbyData);
}

void LE_ColorReachableInit()
{
	corec.Cvar_Register(&colorReachableMinSlope);
	corec.Cvar_Register(&colorReachableStepSize);

	corec.Cmd_Register("ColorReachable", LE_ColorReachable);
}
