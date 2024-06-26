// (C) 2003 S2 Games

// vid_win32.c

// Graphics management


//don't even attempt to compile if this is not win32
#ifdef _WIN32

#include "core.h"
#include "gl_win32.h"

HWND	System_Win32_GetHWnd();

_vid_driver_t _vid_drivers[MAX_VID_DRIVERS] = 
{
	{
		"OpenGL",

		GL_RegisterVars,
		GL_PrintWarnings,
		GL_Win32_Init,
		GL_Win32_StartGL,
		GL_Win32_SetMode,
		GL_Win32_GetMode,
		GL_Win32_IsFullScreen,
		GL_Win32_ShutDown,

		GL_Win32_BeginFrame,
		GL_Win32_EndFrame,
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
		GL_Win32_SetGammaValue
	//	GL_AllocMem
	}
	/*,
	{
		"Direct3d",

		D3D_Init,
		D3D_SetMode,
		D3d_GetModes,
		D3D_Shutdown,

		D3D_BeginFrame,
		D3D_EndFrame,
		Scene_D3D_Render
	}*/
};

extern cvar_t vid_fullscreen;

int _vid_driver_num = 1;

void	Vid_GetClientOffset(int *x, int *y)
{
	RECT adj;
	int style;

	if (Vid_IsFullScreen())
		style = WS_POPUP | WS_MAXIMIZE;
	else
		style = WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE;

	GetClientRect(System_Win32_GetHWnd(), &adj);
	AdjustWindowRect(&adj, style, false);

	*x = -adj.left;
	*y = -adj.top;
}

#endif //_WIN32
