// (C) 2003 S2 Games

// gl_sdl.h


//these first three should be called in this order
int		GL_SDL_Init ();
int		GL_SDL_SetMode( int mode );
void    GL_SDL_StartGL ();


void	GL_SDL_ShutDown(void);
bool	GL_SDL_GetMode(int mode, vid_mode_t *vidmode);
void	GL_SDL_BeginFrame(void);
void	GL_SDL_EndFrame(void);

bool	GL_SDL_IsFullScreen(void);

bool    GL_SDL_SetGammaValue(float gamma);
