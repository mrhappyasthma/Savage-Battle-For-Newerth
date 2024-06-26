// (C) 2003 S2 Games

// gl_sdl.c

// sdl specific OpenGL code...

/* handles:
		
		- Initalizing the GL window, getting a rendering context
		- Shutting down the GL window
		- Changing current resolution

*/

#include "core.h"
#include "resource.h"
#include "SDL.h"

typedef struct
{
	unsigned int width;
	unsigned int height;
	unsigned int bpp;
	bool fullscreen;
	char name[20];
} gl_mode_t;

gl_mode_t   gl_modes[MAX_VID_MODES];
int		current_mode;
int		mode_num;

bool	gl_initialized = false;
int		gl_mode_num = 0;

static bool isFullScreen = false;

extern cvar_t vid_multisample;
extern cvar_t vid_fullscreen;

#define DEFAULT_OVERBRIGHT 1

void	GL_SDL_GetGLModes()
{
	const SDL_VideoInfo *info;
	SDL_Rect **modes;
	int i;

	//always keep mode 0 open?
	gl_mode_num = 1;
	
	info = SDL_GetVideoInfo();
	if (info)
	{
		Console_DPrintf("Current display: %d bits-per-pixel\n",info->vfmt->BitsPerPixel);
		if ( info->vfmt->palette == NULL ) {
			Console_DPrintf("	Red Mask = 0x%.8x\n", info->vfmt->Rmask);
			Console_DPrintf("	Green Mask = 0x%.8x\n", info->vfmt->Gmask);
			Console_DPrintf("	Blue Mask = 0x%.8x\n", info->vfmt->Bmask);
		}
	}
	/* Print available fullscreen video modes */
	modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
	if ( modes == (SDL_Rect **)0 ) {
		Console_DPrintf("No available fullscreen video modes\n");
	} else
	if ( modes == (SDL_Rect **)-1 ) {
		Console_DPrintf("No special fullscreen video modes\n");
	} else {
		Console_DPrintf("Fullscreen video modes:\n");
		for ( i=0; modes[i]; ++i ) {
			gl_modes[gl_mode_num].width = modes[i]->w;
			gl_modes[gl_mode_num].height = modes[i]->h;
			//fixme: figure out how to put the right value in...
			gl_modes[gl_mode_num].bpp = 16;
			BPrintf(gl_modes[gl_mode_num].name, 20, "%dx%d", modes[i]->w, modes[i]->h);
			gl_modes[gl_mode_num].fullscreen = true;
			Console_DPrintf("\t%dx%d\n", modes[i]->w, modes[i]->h);
			gl_mode_num++;
		}
	}
	if (info)
	{
		if ( info->wm_available ) {
			Console_DPrintf("A window manager is available\n");
		}
		if ( info->hw_available ) {
			Console_DPrintf("Hardware surfaces are available (%dK video memory)\n",
				info->video_mem);
		}
		if ( info->blit_hw ) {
			Console_DPrintf("Copy blits between hardware surfaces are accelerated\n");
		}
		if ( info->blit_hw_CC ) {
			Console_DPrintf("Colorkey blits between hardware surfaces are accelerated\n");
		}
		if ( info->blit_hw_A ) {
			Console_DPrintf("Alpha blits between hardware surfaces are accelerated\n");
		}
		if ( info->blit_sw ) {
			Console_DPrintf("Copy blits from software surfaces to hardware surfaces are accelerated\n");
		}
		if ( info->blit_sw_CC ) {
			Console_DPrintf("Colorkey blits from software surfaces to hardware surfaces are accelerated\n");
		}
		if ( info->blit_sw_A ) {
			Console_DPrintf("Alpha blits from software surfaces to hardware surfaces are accelerated\n");
		}
		if ( info->blit_fill ) {
			Console_DPrintf("Color fills on hardware surfaces are accelerated\n");
		}
	}
}

//changes the display resolution
int		GL_SDL_SetMode( int mode )
{
#ifndef _S2_DONT_INCLUDE_GL
	int rgb_size[3];
	int w = 800;
	int h = 600;
	int bpp = 0;
	int depth_size = 32;
	Uint32 video_flags;
	int value, i;
	//const SDL_VideoInfo *video;

	/*
	SDL_Surface *screen;

	screen = SDL_GetVideoSurface();
	if (screen)
	if ( SDL_WM_ToggleFullScreen(screen) ) {
		Console_DPrintf("Toggled fullscreen mode - now %s\n",
			(screen->flags&SDL_FULLSCREEN) ? "fullscreen" : "windowed");
	} else {
		Console_DPrintf("Unable to toggle fullscreen mode\n");
	}
	*/
	
	w = gl_modes[mode].width;
	h = gl_modes[mode].height;
	
	/*
	// See if we should detect the display depth
	if ( bpp == 0 ) {
		video = SDL_GetVideoInfo();
		if ( !video || video->vfmt->BitsPerPixel <= 8 ) {
			bpp = 8;
		} else {
			bpp = video->vfmt->BitsPerPixel;
		}
	}

	// Initialize the display
	switch (bpp) {
	    case 8:
		rgb_size[0] = 2;
		rgb_size[1] = 3;
		rgb_size[2] = 3;
		break;
	    case 15:
	    case 16:
		rgb_size[0] = 5;
		rgb_size[1] = 6;
		rgb_size[2] = 5;
		break;
            default:
		rgb_size[0] = 8;
		rgb_size[1] = 8;
		rgb_size[2] = 8;
		break;
	}
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
	*/

	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depth_size );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
	/* Set the flags we want to use for setting the video mode */
	video_flags = SDL_OPENGL;

	if (vid_fullscreen.value)
	{
		video_flags |= SDL_FULLSCREEN;
		isFullScreen = true;
	}
	else
	{
		isFullScreen = false;
	}
	
	if (vid_multisample.value)
	{
#ifdef USE_SDL_MULTISAMPLE
		SDL_GL_SetAttribute( SDL_GL_SAMPLE_BUFFERS_SIZE, 1);
		SDL_GL_SetAttribute( SDL_GL_SAMPLES_SIZE, vid_multisample.integer);
#endif
	}
	
	i = 0;
	Console_Printf("attempting to set the following video mode: %i x %i x %i (%i bit depth buffer)\n", w, h, bpp, depth_size);
	while (SDL_SetVideoMode( w, h, bpp, video_flags ) == NULL )
	{
		switch (i)
		{
			case 0:
			case 1:
				if (depth_size <= 8)
				{
					Console_Printf("Couldn't get at least a 16-bit depth buffer: %s\n", SDL_GetError());
					SDL_Quit();
					exit(1);
				}
				//maybe we can't get a depth buffer this large...
				depth_size -= 8;
				SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depth_size );
				break;
			case 2:
				bpp = 16; //fall back to 16 bit
				break;
			case 3:
				SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 0 );
				break;
			default:
				Console_Printf("Couldn't set GL mode: %s\n", SDL_GetError());
				SDL_Quit();
				exit(1);
		}
		i++;
		Console_Printf("attempting to set the following video mode: %i x %i x %i (%i bit depth buffer)\n", w, h, bpp, depth_size);
	}

	Console_Printf("Screen Res: %d x %d\n", w, h);
	Console_Printf("Screen BPP: %d\n", SDL_GetVideoSurface()->format->BitsPerPixel);
	Console_Printf("Depth Buffer Size: %d\n", depth_size);
	Console_Printf("\n");
	Console_Printf( "Vendor     : %s\n", glGetString( GL_VENDOR ) );
	Console_Printf( "Renderer   : %s\n", glGetString( GL_RENDERER ) );
	Console_Printf( "Version    : %s\n", glGetString( GL_VERSION ) );
	Console_Printf( "Extensions : %s\n", glGetString( GL_EXTENSIONS ) );
	Console_Printf("\n");

	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &value );
	Console_Printf( "SDL_GL_RED_SIZE: requested %d, got %d\n", rgb_size[0],value);
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &value );
	Console_Printf( "SDL_GL_GREEN_SIZE: requested %d, got %d\n", rgb_size[1],value);
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &value );
	Console_Printf( "SDL_GL_BLUE_SIZE: requested %d, got %d\n", rgb_size[2],value);
	SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &value );
	Console_Printf( "SDL_GL_DEPTH_SIZE: requested %d, got %d\n", bpp, value );
	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &value );
	Console_Printf( "SDL_GL_DOUBLEBUFFER: requested 1, got %d\n", value );

#ifdef USE_SDL_MULTISAMPLE
	SDL_GL_GetAttribute( SDL_GL_SAMPLE_BUFFERS_SIZE, &value);
	Console_Printf( "GL_SAMPLE_BUFFERS_ARB: requested %i, got %d\n", vid_multisample.integer, value);
	if (value)
	{
		SDL_GL_GetAttribute( SDL_GL_SAMPLES_SIZE, &value);
		Console_Printf( "Using anti-aliasing with %i samples\n", value);
	}
	else
#endif
	{
		Console_Printf( "Anti-aliasing not enabled\n");
	}

	/* Set the window manager title bar */
	SDL_WM_SetCaption( "Silverback Engine", "Savage" );

	return mode;
#endif
}

void GL_SDL_SetupPixelFormat()
{
  //no way to change pixel format in Linux.  If we move win32 to SDL, we need to add the call here
}

void	GL_SDL_SetGamma(float gamma, float overbright)
{  
	Uint16 ramp[256*3]; 
	float brightness=0; 
	int i;


	for (i=0 ; i<256 ; i++){ 
		float f = (float) (255 * pow ( (float)i/255.0 , 1/gamma )); 
		f = (f+brightness) * overbright; 
		if (f < 0) f = 0; 
		if (f > 255) f = 255; 
		ramp[i+0] = ramp[i+256] = ramp[i+512] = (unsigned short) (iround(f)<<8); 
	} 
	

	if (!SDL_SetGammaRamp(ramp, ramp, ramp))
	{
		Console_Printf("GL_SDL_SetGamma() failed\n");
	} 
} 

bool	GL_SDL_GetMode(int mode, vid_mode_t *vidmode)
{
	if (!vidmode || mode>gl_mode_num) return false;

	vidmode->width = gl_modes[mode].width;
	vidmode->height = gl_modes[mode].height;
	vidmode->bpp = gl_modes[mode].bpp;
	vidmode->fullscreen = true;
	strcpy(vidmode->name, gl_modes[mode].name);
	return true;
}

//initializes an OpenGL rendering context and window
int		GL_SDL_Init ()
{	
	if ( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 )
	{
		Console_Printf("Warning, couldn't initialize video, I hope this is a dedicated server\n");
	}
	
	if (!gl_mode_num)
		GL_SDL_GetGLModes();

	return 1;
}

//initializes an OpenGL rendering context and window
void	GL_SDL_StartGL ()
{
	SDL_GrabMode mode;

	//grab our input here
	mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);
	//fixme: re-enable grabbing of input
    //if ( mode != SDL_GRAB_ON ) {
    if ( mode != SDL_GRAB_OFF ) {
	    mode = SDL_WM_GrabInput(!mode);
	}

	SDL_ShowCursor(false);
		
	gl_initialized = true;
	
	GL_Init();
}

void	GL_SDL_ShutDown(void)
{
	if (!gl_initialized) return;

	GL_ShutDown();
	
	SDL_ShowCursor(true);
}

bool	GL_SDL_SetGammaValue(float gamma)
{
	if (gamma < 0.1)
	{
		Console_Printf("Invalid gamma value of %f\n", gamma);
		return false;
	}
	return (SDL_SetGamma(gamma, gamma, gamma) == 0); //0 == success
}

void	GL_SDL_BeginFrame(void)
{
}

extern cvar_t gfx_flush;

void	GL_SDL_EndFrame(void)
{
#ifndef _S2_DONT_INCLUDE_GL
	if (gfx_flush.integer)
		glFlush();
#endif
	SDL_GL_SwapBuffers( );
}

bool	GL_SDL_IsFullScreen(void)
{
	return isFullScreen;
}
