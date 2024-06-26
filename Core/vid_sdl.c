// (C) 2003 S2 Games

// vid_sdl.c

// Graphics management

#include "core.h"
#include "gl_sdl.h"
#include "gl_console.h"
#include <SDL.h>

_vid_driver_t _vid_drivers[MAX_VID_DRIVERS] = 
{
#ifndef _S2_DONT_INCLUDE_GL
	{
		"OpenGL",
		
		GL_RegisterVars,
		GL_PrintWarnings,
		GL_SDL_Init,
		GL_SDL_StartGL,
		GL_SDL_SetMode,
		GL_SDL_GetMode,
		GL_SDL_IsFullScreen,
		GL_SDL_ShutDown,

		GL_SDL_BeginFrame,
		GL_SDL_EndFrame,
		GL_InitScene,
		GL_RenderScene,
		GL_Quad2d,
		GL_Poly2d,
		GL_SetDrawRegion,
		GL_SetColor,
//		GL_RenderHeightmapSection,
		GL_Notify,
		GL_UnregisterShader,
		GL_RegisterShader,
		GL_RegisterShaderImageFromMemory,
		GL_GetFrameBuffer,
		GL_ProjectVertex,
		GL_ReadZBuffer,
		GL_RegisterModel,
		GL_SDL_SetGammaValue
	//	GL_AllocMem
	},
#endif
	{
		"GL_Console",

		GL_Console_RegisterVars,
		GL_Console_PrintWarnings,
		GL_Console_Init,
		GL_Console_StartGL,
		GL_Console_SetMode,
		GL_Console_GetMode,
		GL_Console_IsFullScreen,
		GL_Console_ShutDown,

		GL_Console_BeginFrame,
		GL_Console_EndFrame,
		GL_Console_InitScene,
		GL_Console_RenderScene,
		GL_Console_Quad2d,
		GL_Console_Poly2d,
		GL_Console_SetDrawRegion,
		GL_Console_SetColor,
//		GL_Console_RenderHeightmapSection,
		GL_Console_Notify,
		GL_Console_UnregisterShader,
		GL_Console_RegisterShader,
		GL_Console_RegisterShaderImageFromMemory,
		GL_Console_GetFrameBuffer,
		GL_Console_ProjectVertex,
		GL_Console_ReadZBuffer,
		GL_Console_RegisterModel,
		GL_Console_SetGamma
	//	GL_AllocMem
	}
};

#ifndef _S2_DONT_INCLUDE_GL
int _vid_driver_num = 2;
#else
int _vid_driver_num = 1;
#endif

