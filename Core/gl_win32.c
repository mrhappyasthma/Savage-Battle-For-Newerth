// (C) 2003 S2 Games

// gl_win32.c

// win32 specific OpenGL code...ptui!

/* handles:
		
		- Initalizing the GL window, getting a rendering context
		- Shutting down the GL window
		- Changing current resolution

*/


//don't even attempt to compile if this is not win32
#ifdef _WIN32

#include "core.h"
#include "resource.h"

static bool isFullscreen = false;

typedef struct
{
	WNDCLASS	wc;
	HWND	hwnd;
	HDC		hdc;
	HGLRC	hglrc;
} windowinfo_t;

typedef struct
{
	unsigned int width;
	unsigned int height;
	unsigned int bpp;
	bool fullscreen;
	char name[20];

} gl_mode_t;

gl_mode_t	gl_modes[MAX_VID_MODES];
windowinfo_t	win;

int		gl_current_mode;
int		gl_mode_num;
int		gl_initialized;

#define DEFAULT_OVERBRIGHT 1

HWND	System_Win32_GetHWnd()
{
	return win.hwnd;
}

HDC	System_Win32_GetHDC()
{
	return win.hdc;
}

extern cvar_t vid_fullscreen;

void	GL_Win32_GetGLModes()
{
	//FIXME: this will test Window's 2d resolutions, but not
	//actual GL resolutions.  Is there a way to get resolutions
	//we know will be supported by OpenGL?

	int success, modenum;
	DEVMODE devmode;
	bool modeexists;
	int n;

	gl_mode_num = 1;  //always keep mode 0 open
//	gl_mode_num = 0;
	modenum = 0;

	do
	{
		//devmode.dmSize = sizeof(DEVMODE);
		modeexists = 0;
		success = EnumDisplaySettings(NULL, modenum, &devmode);
		if (!success)
		{
			if (modenum == 0)
				System_Error("EnumDisplaySettings failed on mode 0");
			break;
		}

		if (devmode.dmBitsPerPel >= 15 && devmode.dmPelsWidth >= 640 && devmode.dmPelsHeight >= 480 && gl_mode_num < MAX_VID_MODES)
		{			
			//see if the mode is valid
			devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&devmode, CDS_TEST | CDS_FULLSCREEN)==DISP_CHANGE_SUCCESSFUL)
			{
				//make sure the mode doesn't already exist
				for (n=0; n<gl_mode_num; n++)
				{
					if (   gl_modes[n].width == devmode.dmPelsWidth
						&& gl_modes[n].height == devmode.dmPelsHeight
						&& gl_modes[n].bpp == devmode.dmBitsPerPel
						)
						modeexists = 1;
				}
				if (!modeexists)
				{
					//the mode is valid, so add it to gl_modes
					gl_modes[gl_mode_num].width = devmode.dmPelsWidth;
					gl_modes[gl_mode_num].height = devmode.dmPelsHeight;
					gl_modes[gl_mode_num].bpp = devmode.dmBitsPerPel;
					gl_modes[gl_mode_num].fullscreen = true;
					sprintf(gl_modes[gl_mode_num].name, "%ix%ix%i", 
								devmode.dmPelsWidth,
								devmode.dmPelsHeight,
								devmode.dmBitsPerPel);
					Console_DPrintf("GL mode %i:\n", gl_mode_num);
					Console_DPrintf("Width: %i, Height: %i, Bpp: %i\n", devmode.dmPelsWidth, devmode.dmPelsHeight, devmode.dmBitsPerPel);
					gl_mode_num++;
				}
			}

		}
		modenum++;
	} while (success);
}

//changes the display resolution
int		GL_Win32_SetMode( int mode )
{
	DEVMODE devmode;
	RECT	winsize;

	if (mode > gl_mode_num || mode < 0)
	{
		Console_Errorf("GL_Win32_SetMode: %i is an invalid mode.  Defaulting to mode 0", mode);
		gl_current_mode = 0;
	} else
		gl_current_mode = mode;		

	memset(&devmode, 0, sizeof(devmode));
	devmode.dmPelsWidth = gl_modes[gl_current_mode].width;
	devmode.dmPelsHeight = gl_modes[gl_current_mode].height;
	devmode.dmBitsPerPel = gl_modes[gl_current_mode].bpp;
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	devmode.dmSize = sizeof(devmode);

	if (vid_fullscreen.integer)
	{
		if (ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			if (mode==0)
			{			
				System_Error("GL_Win32_SetMode: Default mode is invalid!");			
			}
			else
			{
				GL_Win32_SetMode(0);			
			}
		}

		isFullscreen = true;
	}
	else
	{
		isFullscreen = false;
	}

	if (win.hwnd)
	{
		if (!vid_fullscreen.integer)
		{
			if (vid_fullscreen.modified)
			{
				ChangeDisplaySettings(NULL, 0);
				vid_fullscreen.modified = false;
			}
			if(!SetWindowLong(win.hwnd, GWL_STYLE, WS_CAPTION|WS_MINIMIZEBOX|WS_VISIBLE|WS_SYSMENU))
			{
				LPVOID lpMsgBuf;

				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
				);
				System_Error("SetWindowLong() - %s", (char*)lpMsgBuf);
			}
			winsize.top = 0;
			winsize.bottom = devmode.dmPelsHeight;
			winsize.left = 0;
			winsize.right = devmode.dmPelsWidth;
			AdjustWindowRect(&winsize, WS_CAPTION|WS_MINIMIZEBOX|WS_VISIBLE, false);
			if (!SetWindowPos(win.hwnd, HWND_TOP, 0, 0, winsize.right - winsize.left, winsize.bottom - winsize.top, SWP_SHOWWINDOW))
			{
				LPVOID lpMsgBuf;

				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
				);
				System_Error("SetWindowPos() - %s", (char*)lpMsgBuf);
			}
		}
		else
		{
			if (!SetWindowLong(win.hwnd, GWL_STYLE, WS_POPUP | WS_MAXIMIZE))
			{
				LPVOID lpMsgBuf;

				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
				);
				System_Error("SetWindowLong() - %s", (char*)lpMsgBuf);
			}
			if (!SetWindowPos(win.hwnd, HWND_TOP, 0, 0, devmode.dmPelsWidth, devmode.dmPelsHeight, SWP_SHOWWINDOW))
			{
				LPVOID lpMsgBuf;

				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
				);
				System_Error("SetWindowPos() - %s", (char*)lpMsgBuf);
			}
		}
	}

	return gl_current_mode;
}

void GL_Win32_SetupPixelFormat()
{
	int pixelformat;
    static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
	1,				// version number
	PFD_DRAW_TO_WINDOW 		// support window
	|  PFD_SUPPORT_OPENGL 	// support OpenGL
	|  PFD_DOUBLEBUFFER ,	// double buffered
	PFD_TYPE_RGBA,			// RGBA type
	32,				// 32-bit color depth
	0, 0, 0, 0, 0, 0,		// color bits ignored
	0,				// no alpha buffer
	0,				// shift bit ignored
	0,				// no accumulation buffer
	0, 0, 0, 0, 			// accum bits ignored
	24,				// 32-bit z-buffer	
	0,				// no stencil buffer
	0,				// no auxiliary buffer
	PFD_MAIN_PLANE,			// main layer
	0,				// reserved
	0, 0, 0				// layer masks ignored
    };
    
    if (!(pixelformat = ChoosePixelFormat(win.hdc, &pfd)))    
        System_Error("GL_Win32_SetupPixelFormat: ChoosePixelFormat failed");            

    if (!SetPixelFormat(win.hdc, pixelformat, &pfd))
        System_Error("SetPixelFormat failed");                
}

void	GL_Win32_SetGamma(float gamma, float overbright)
{  
	WORD ramp[256*3]; 


	float brightness=0; 
	int i;


	for (i=0 ; i<256 ; i++){ 
		float f = (float) (255 * pow ( (float)i/255.0 , 1/gamma )); 
		f = (f+brightness) * overbright; 
		if (f < 0) f = 0; 
		if (f > 255) f = 255; 
		ramp[i+0] = ramp[i+256] = ramp[i+512] = (WORD) (iround(f)<<8); 
	} 
	
	if (!SetDeviceGammaRamp(win.hdc, ramp))
	{
		Console_Printf("GL_Win32_SetGamma() failed\n");
	} 
}; 

bool	GL_Win32_GetMode(int mode, vid_mode_t *vidmode)
{
	if (!vidmode || mode > gl_mode_num) return false;
	
	vidmode->width = gl_modes[mode].width;
	vidmode->height = gl_modes[mode].height;
	vidmode->bpp = gl_modes[mode].bpp;
	vidmode->fullscreen = true;
	strcpy(vidmode->name, gl_modes[mode].name);
	return true;
}

//sys_win32.h
LONG WINAPI GL_Win32_MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HINSTANCE	System_Win32_GetHInstance();

//initializes an OpenGL rendering context and window
int		GL_Win32_Init ()
{	
	gl_modes[0].width = 640;
	gl_modes[0].height = 480;
	gl_modes[0].bpp = 0;

	//before we start, enumerate all available video modes
	gl_initialized = 0;

	GL_Win32_GetGLModes();
	return 1;
}

void	GL_Win32_StartGL()
{
	static int class_registered = 0;	
	MSG msg;

	if (!class_registered)
	{
		memset(&win.wc, 0, sizeof(WNDCLASS));		
		win.wc.lpfnWndProc = GL_Win32_MainWndProc;
		win.wc.hInstance = System_Win32_GetHInstance();
		win.wc.lpszClassName = "Silverback_OpenGL";
		win.wc.hIcon = LoadIcon(win.wc.hInstance, MAKEINTRESOURCE(IDI_ICON1)); 
		
		if (!RegisterClass(&win.wc))
			System_Error("GL_Win32_Init: RegisterClass() failed");

		class_registered = 1;
	}

	//create the window
#ifdef SAVAGE_DEMO
	win.hwnd = CreateWindow("Silverback_OpenGL",
						"Savage Demo",
						WS_MAXIMIZE | WS_POPUP,
						0, 
						0, 
						gl_modes[gl_current_mode].width,
						gl_modes[gl_current_mode].height,
						NULL,
						NULL,
						System_Win32_GetHInstance(),
						NULL);
#else
	win.hwnd = CreateWindow("Silverback_OpenGL",
						"Savage",
						WS_MAXIMIZE | WS_POPUP,
						0, 
						0, 
						gl_modes[gl_current_mode].width,
						gl_modes[gl_current_mode].height,
						NULL,
						NULL,
						System_Win32_GetHInstance(),
						NULL);
#endif

	if (!win.hwnd)
		System_Error("GL_Win32_Init: CreateWindow() failed");

	GL_Win32_SetMode(gl_current_mode);

	ShowWindow(win.hwnd, SW_SHOW);
	UpdateWindow(win.hwnd);
//	SetForegroundWindow(win.hwnd);

	//create the opengl context
	win.hdc = GetDC(win.hwnd);

	GL_Win32_SetupPixelFormat(win.hdc);
	if (!(win.hglrc = wglCreateContext(win.hdc)))
		System_Error("GL_Win32_Init: wglCreateContext() failed");
	if (!wglMakeCurrent(win.hdc, win.hglrc))
		System_Error("GL_Win32_Init: wglMakeCurrent() failed");

	GL_Win32_SetGamma(1,DEFAULT_OVERBRIGHT);

	gl_initialized = 1;
	
	ShowCursor(FALSE);
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	SetForegroundWindow(win.hwnd);

	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	{
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}

	Sleep (100);

	SetWindowPos (win.hwnd, HWND_TOP, 0, 0, 0, 0,
				  SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
				  SWP_NOCOPYBITS);

	SetForegroundWindow(win.hwnd);	
	SetFocus(win.hwnd);	

	PatBlt(win.hdc, 0, 0, gl_modes[gl_current_mode].width, gl_modes[gl_current_mode].height, BLACKNESS);

	//initialize GL related stuff
	GL_Init();
//	glViewport(0, 0, gl_modes[ret].width, gl_modes[ret].height);
}





void	GL_Win32_ShutDown(void)
{
	if (!gl_initialized) return;

	GL_ShutDown();
	
	//update our dc's incase something happened to them
	win.hglrc = wglGetCurrentContext();
    win.hdc = wglGetCurrentDC();

    wglMakeCurrent(NULL, NULL);

	GL_Win32_SetGamma(1, 1);

    if (win.hglrc)
        wglDeleteContext(win.hglrc);
	if (win.hdc)
		ReleaseDC(win.hwnd, win.hdc);

	DestroyWindow(win.hwnd);

	ChangeDisplaySettings(NULL, 0);

	ShowCursor(TRUE);
}


void	GL_Win32_BeginFrame(void)
{
}

extern cvar_t gfx_flush;

void	GL_Win32_EndFrame(void)
{
	if (gfx_flush.integer)
		glFlush();
	SwapBuffers(win.hdc);
}

bool    GL_Win32_SetGammaValue(float gamma)
{
	if (gamma < 0.1)
	{
		Console_Printf("Invalid gamma value of %f\n", gamma);
		return false;
	}
	GL_Win32_SetGamma(gamma, DEFAULT_OVERBRIGHT);
	return true;
}

bool	GL_Win32_IsFullScreen(void)
{
	return isFullscreen;
}

#endif //_WIN32
