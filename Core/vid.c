// (C) 2003 S2 Games

// vid.c

// basically a wrapper for the actual video functions contained
// in _vid_drivers


#include "core.h"

cvar_t		vid_aspect = { "aspect", "0.75", CVAR_NETSETTING | CVAR_READONLY };
cvar_t		vid_mode = { "vid_mode", "-1", CVAR_SAVECONFIG }; //-1 means select a good one for us
cvar_t		vid_blockmegs = { "vid_blockmegs", "64" };

cvar_t		vid_useSystemMem = { "vid_useSystemMem", "0" };
bool		vid_initialized = false;
cvar_t		vid_overbright = { "vid_overbright", "1" };
cvar_t		vid_overbrightNormalize = { "vid_overbrightNormalize", "0.5" };
cvar_t  	vid_compressTextures = { "vid_compressTextures", "1" };
cvar_t  	vid_writeCompressedTextures = { "vid_writeCompressedTextures", "0", CVAR_SAVECONFIG };
cvar_t  	vid_maxtexturesize = { "vid_maxtexturesize", "2048", CVAR_SAVECONFIG };
cvar_t		vid_multisample = { "vid_multisample", "0" };
cvar_t		vid_currentMode = { "vid_currentMode", "" };
cvar_t		vid_fullscreen = { "vid_fullscreen", "1", CVAR_SAVECONFIG };
cvar_t		vid_numModes = { "vid_numModes", "0" };
cvar_t		vid_gamma = { "vid_gamma", "1", CVAR_SAVECONFIG };

static int	current_driver = 0;
static int	current_mode = 0;

extern cvar_t gfx_gammaMult;

static vid_mode_t vm;

void	Vid_PrintWarnings()
{
		_vid_drivers[current_driver].PrintWarnings();
}

void	Vid_NormalizeColor(const vec4_t color, vec4_t out)
{
	if (vid_overbright.integer)
	{
		out[0] = color[0] * vid_overbrightNormalize.value;
		out[1] = color[1] * vid_overbrightNormalize.value;
		out[2] = color[2] * vid_overbrightNormalize.value;
	}
	else
	{
		//just clamp values brighter than 1
		if (color[0] > 1)
			out[0] = 1;
		else
			out[0] = color[0];

		if (color[1] > 1)
			out[1] = 1;
		else
			out[1] = color[1];

		if (color[2] > 1)
			out[2] = 1;
		else
			out[2] = color[2];
	}

	out[3] = color[3];
}

void	Vid_RegisterVars ()
{
	int n;
	
	Cvar_Register(&vid_mode);
	Cvar_Register(&vid_blockmegs);	
	Cvar_Register(&vid_useSystemMem);
	Cvar_Register(&vid_overbright);
	Cvar_Register(&vid_overbrightNormalize);
	Cvar_Register(&vid_compressTextures);
	Cvar_Register(&vid_writeCompressedTextures);
	Cvar_Register(&vid_maxtexturesize);
	Cvar_Register(&vid_multisample);
	Cvar_Register(&vid_currentMode);
	Cvar_Register(&vid_fullscreen);
	Cvar_Register(&vid_gamma);
	Cvar_Register(&vid_numModes);
	Cvar_Register(&vid_aspect);

	for (n = 0; n < _vid_driver_num; n++)
		_vid_drivers[n].RegisterVars();
}

//changes current video driver.  NOT TESTED
void	Vid_SetDriver (char *drivername)
{
	int n;

	for (n=0; n<_vid_driver_num; n++)
	{
		if (strcmp(drivername, _vid_drivers[n].drivername)==0)
		{
			if (current_driver != n)
			{
				Vid_ShutDown();	//shutdown old driver
				current_driver = n;
				Vid_Init();		//start up new driver				
			}
			return;
		}
	}
}

void	Vid_SetMode (int mode)
{
	Vid_ShutDown();
	Vid_Init(mode);
	/*current_mode = _vid_drivers[current_driver].SetMode(mode);
	_vid_drivers[current_driver].GetMode(current_mode, &vm);*/
	Console_Printf("Vid_SetMode: width=%i, height=%i\n", vm.width, vm.height);
}

void	Vid_ChangeMode(int mode)
{
	current_mode = _vid_drivers[current_driver].SetMode(mode);
	_vid_drivers[current_driver].GetMode(current_mode, &vm);
	Console_Printf("Vid_SetMode: width=%i, height=%i\n", vm.width, vm.height);
	Cvar_SetVar(&vid_currentMode, fmt("%ix%ix%i", vm.width, vm.height, vm.bpp));

	//the following cvar is a net setting the server needs
	Cvar_SetValue("aspect", Vid_GetScreenH() / (float)Vid_GetScreenW());
}


void	Vid_ChangeVideo_Cmd (int argc, char *argv[])
{
	Vid_ChangeMode(vid_mode.integer);
}

//for whatever reason, initial_mode wasn't getting assigned correctly with optimizations enabled
#pragma optimize( "", off )
void	Vid_Init ()
{
	int best = -1, bestWidth = 0, bestHeight = 0, bestBPP = 0, i = 0, j = 1;
	bool idealBest = false;
	int initial_mode;
	vid_mode_t vm2;

	Console_Printf("---------------------------------------------------------\n");
	Console_Printf("Vid_Init():\n");
	initial_mode = vid_mode.integer;

	Cmd_Register("changevideo", Vid_ChangeVideo_Cmd);

	current_mode = _vid_drivers[current_driver].Init();
					
	while (_vid_drivers[current_driver].GetMode(i, &vm2))
	{
		if (vm2.width && vm2.height && vm2.bpp)
		{
			Console_Printf(" * Adding mode %i: %ix%ix%i\n", i, vm2.width, vm2.height, vm2.bpp);
				
			//create two cvars, one for each direction
			Cmd_Exec(fmt("createvar vid_mode%i %ix%ix%i", j, vm2.width, vm2.height, vm2.bpp));
			Cmd_Exec(fmt("createvar %ix%ix%i %i", vm2.width, vm2.height, vm2.bpp, i));

			//just get a fallback here, worst case for if they have no modes <= 1024x768
			if (best == -1)
			{
				Console_DPrintf("setting best to this mode, since best was -1\n");
				best = i;
			}
			
			if (vm2.width == 1024 && vm2.height == 768)  //this is our ideal starting video mode
			{
				if (!idealBest || bestBPP != 16 || bestBPP == 0)
				{
					Console_DPrintf("setting idealBest and best to this mode, since this is ideal (%ix%ix%i), old res was (%ix%ix%i)\n", vm2.width, vm2.height, vm2.bpp, bestWidth, bestHeight, bestBPP);
					best = i;
					idealBest = true;
					bestWidth = vm2.width;
					bestHeight = vm2.height;
					bestBPP = vm2.bpp;
				}
			}
			if (!idealBest)
			{
				//don't choose a mode bigger than 1024x768 for the sake of older cards
				if (vm2.width < 1024 
					&& vm2.height < 768
					&& bestWidth < vm2.width 
					&& bestHeight < vm2.height
					&& bestBPP <= vm2.bpp)
				{
					Console_DPrintf("setting best to this mode, since (%ix%ix%i) is better than (%ix%ix%i)\n", vm2.width, vm2.height, vm2.bpp, bestWidth, bestHeight, bestBPP);
					best = i;
					bestWidth = vm2.width;
					bestHeight = vm2.height;
					bestBPP = vm2.bpp;
				}
			}
			j++;
		}
		i++;
	}
	Cvar_SetVar(&vid_numModes, fmt("%i", j - 1));

	if (initial_mode == -1)
	{
		if (best)
			initial_mode = best;
		else
			initial_mode = 0;
	}
	
	current_mode = _vid_drivers[current_driver].SetMode(initial_mode);
	_vid_drivers[current_driver].GetMode(current_mode, &vm);
	Console_Printf("Using mode %i: %ix%ix%i\n", current_mode, vm.width, vm.height, vm.bpp);
	Cvar_SetVar(&vid_currentMode, fmt("%ix%ix%i", vm.width, vm.height, vm.bpp));

	_vid_drivers[current_driver].StartGL();

	vid_initialized = true;
}
#pragma optimize( "", on )


void	Vid_InitScene ()
{
	_vid_drivers[current_driver].InitScene();
}

void	Vid_RenderScene (camera_t *camera, vec3_t userpos)
{
	_vid_drivers[current_driver].RenderScene(camera, userpos);
}

void	Vid_DrawQuad2d(float x, float y, float w, float h, float s1, float t1, float s2, float t2, shader_t *shader)
{
	_vid_drivers[current_driver].DrawQuad2d(x, y, w, h, s1, t1, s2, t2, shader);
}

void	Vid_DrawPoly2d(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, shader_t *shader)
{
	_vid_drivers[current_driver].DrawPoly2d(v1, v2, v3, v4, s1, t1, s2, t2, shader);
}

void	Vid_SetDrawRegion(int x, int y, int w, int h)
{
	_vid_drivers[current_driver].SetDrawRegion(x, y, w, h);
}

void	Vid_SetColor (vec4_t color)
{
	_vid_drivers[current_driver].SetColor(color);
}

void	Vid_ShutDown ()
{
	_vid_drivers[current_driver].ShutDown();
}

void	Vid_BeginFrame ()
{
	_vid_drivers[current_driver].BeginFrame();
}

void	Vid_EndFrame ()
{
	OVERHEAD_INIT;
	_vid_drivers[current_driver].EndFrame();
	OVERHEAD_COUNT(OVERHEAD_VID_ENDFRAME);
}

bool	Vid_IsFullScreen ()
{
	return _vid_drivers[current_driver].IsFullScreen();
}

extern cvar_t	demo_makeMovie;
extern cvar_t	demo_movieWidth;
extern cvar_t	demo_movieHeight;

int		Vid_GetScreenW ()
{
//	return 1024;
	if (demo.playing && demo_makeMovie.integer)
		return demo_movieWidth.integer;
	return vm.width;
}

int		Vid_GetScreenH ()
{
//	return 768;
	if (demo.playing && demo_makeMovie.integer)
		return demo_movieHeight.integer;
	return vm.height;
}


int		Vid_CurrentMode ()
{
	return current_mode;
}

/*void	Vid_RenderHeightmapSection(vec3_t bmin, vec3_t bmax)
{
	_vid_drivers[current_driver].RenderHeightmapSection(bmin, bmax);
}*/


void	Vid_Notify(int message, int param1, int param2, int param3)
{
	_vid_drivers[current_driver].Notify(message, param1, param2, param3);
}

bool	Vid_UnregisterShader(shader_t *shader)
{
	bool ret;
	ret = _vid_drivers[current_driver].UnregisterShader(shader);
	return ret;
}

bool	Vid_RegisterShader(shader_t *shader)
{	
	bool ret;
	OVERHEAD_INIT;
	ret = _vid_drivers[current_driver].RegisterShader(shader);
	OVERHEAD_COUNT(OVERHEAD_TEXTURES);
	return ret;
}

bool	Vid_RegisterShaderImage(shader_t *shader, bitmap_t *bmp)
{
	bool ret;
	OVERHEAD_INIT;
	ret = _vid_drivers[current_driver].RegisterShaderImage(shader, bmp);
	OVERHEAD_COUNT(OVERHEAD_TEXTURES);
	return ret;
}

bool	Vid_RegisterModel(model_t *model)
{
	return _vid_drivers[current_driver].RegisterModel(model);
}

void	Vid_GetFrameBuffer(bitmap_t *bmp)
{
	//OVERHEAD_INIT;
	_vid_drivers[current_driver].GetFrameBuffer(bmp);
//	OVERHEAD_COUNT(OVERHEAD_RENDER);
}

void	Vid_ProjectVertex(camera_t *cam, vec3_t vertex, vec2_t result)
{
	_vid_drivers[current_driver].ProjectVertex(cam, vertex, result);
}

void	Vid_ReadZBuffer(int x, int y, int w, int h, float *zpixels)
{
	_vid_drivers[current_driver].ReadZBuffer(x, y, w, h, zpixels);
}

bool	Vid_SetGamma(float gamma)
{
	return _vid_drivers[current_driver].SetGamma(gfx_gammaMult.value * gamma);
}



/*void	*Vid_AllocMem(int size)
{
	return _vid_drivers[current_driver].AllocMem(size);
}*/
