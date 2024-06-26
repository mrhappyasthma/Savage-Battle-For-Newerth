// (C) 2003 S2 Games

// gl_console.c

// console (null driver) specific OpenGL code...

/* handles:
		
		- Initalizing the GL window, getting a rendering context
		- Shutting down the GL window
		- Changing current resolution

*/

#include "core.h"
#include "resource.h"
#include <SDL.h>

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

bool	gl_initialized;
int		gl_mode_num;

void    GL_Console_RegisterVars()
{
}

void    GL_Console_PrintWarnings()
{
}

void	GL_Console_GetGLModes()
{
	//always keep mode 0 open?
	gl_mode_num = 1;
}

bool	GL_Console_IsFullScreen()
{
	return false;
}

void	GL_Console_StartGL()
{
}

//changes the display resolution
int		GL_Console_SetMode( int mode )
{
	System_Printf( "Renderer   : GL_Console\n");
	return mode;
}

void GL_Console_SetupPixelFormat()
{
  //no way to change pixel format in Linux.  If we move win32 to SDL, we need to add the call here
}

bool	GL_Console_GetMode(int mode, vid_mode_t *vidmode)
{
	if (!vidmode || mode>gl_mode_num) return false;

	vidmode->width = 80;
	vidmode->height = 25;
	vidmode->bpp = 8;
	vidmode->fullscreen = true;
	strcpy(vidmode->name, "GL_Console");
	return true;
}

//initializes an OpenGL rendering context and window
int		GL_Console_Init (int initial_mode, vid_mode_t *vm)
{	
	int ret;
	ret = GL_Console_SetMode(initial_mode);
	
	gl_initialized = true;
	
	GL_Console_GetMode(ret, vm);

#ifndef _S2_DONT_INCLUDE_GL
	GL_Init();
#endif

	return ret;
}



void	GL_Console_ShutDown(void)
{
	if (!gl_initialized) return;

	SDL_Quit();
}


void	GL_Console_BeginFrame(void)
{
	static struct timeval timeout;
	//char buf[256];
	//fd_set rmask;
	fd_set mask;

	FD_ZERO(&mask);
	FD_SET(fileno(stdin), &mask);
	
	while (true)
	{	
		timeout.tv_sec = 0;
		timeout.tv_usec = 1;
	
		/*
		rmask = mask;
		numfound = select(fileno(stdin)+1, &rmask, NULL, NULL, &timeout);
		if (numfound < 0)
		{
			if(errno == EINTR) 
			{
		  		printf ("Interrupted system call\n");
				continue;
			}
		}
		if (numfound == 0)
		{
			return;
		}
		n = read(fileno(stdin), buf, sizeof(buf));
		if (n == -1 && errno != EAGAIN)
		{
			Console_Printf("error %i on read from stdin!\n", errno);
		}
		for (i = 0; i < n; i++)
		{
			value = buf[i];
			//just send a down-up combo (as opposed to an up, down, up, down, left, right, left, right, b, a, select, start combo
			Console_Key(value);
		}
		*/
		return;
	}
}

void	GL_Console_EndFrame(void)
{
}

void    GL_Console_InitScene(void)
{
}

void    GL_Console_RenderScene (camera_t *camera, vec3_t userpos)
{
}

void    GL_Console_Quad2d (float x, float y, float w, float h, float s1, float t1, float s2, float t2, shader_t *shader)
{
}

void    GL_Console_Poly2d(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, shader_t *shader)
{
}

void    GL_Console_SetColor (vec4_t color)
{
}

void    GL_Console_Notify (int message, int param1, int param2, int param3)
{
}

bool    GL_Console_UnregisterShader(shader_t *shader)
{
	return true;
}

bool    GL_Console_RegisterShader (shader_t *shader)
{
	return true;
}

bool    GL_Console_RegisterShaderImageTextureMap(shader_t *shader, char *filename)
{
	return true;
}

bool    GL_Console_RegisterShaderImageBumpMap(shader_t *shader, char *filename)
{
	return true;
}

bool    GL_Console_RegisterShaderImageFromMemory(shader_t *shader, bitmap_t *image)
{
	return true;
}

bool    GL_Console_RegisterShaderImage(shader_t *shader, bitmap_t *bitmap)
{
	return true;
}

void    GL_Console_ReadZBuffer(int x, int y, int w, int h, float *pixels)
{
}

void GL_Console_GetFrameBuffer(bitmap_t *screenshot)
{
}

void GL_Console_ProjectVertex(camera_t *cam, vec3_t vertex, vec2_t result)
{
	result[0] = 0;
	result[1] = 0;
}

void	GL_Console_SetDrawRegion(int x, int y, int w, int h)
{
}

bool	GL_Console_RegisterModel(model_t *model)
{
	return true;
}

bool    GL_Console_SetGamma(float gamma)
{
	return false;
}
