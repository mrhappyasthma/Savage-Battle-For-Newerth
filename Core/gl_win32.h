// (C) 2003 S2 Games

// gl_win32.h



int		GL_Win32_SetMode( int mode );
int		GL_Win32_Init ();
void	GL_Win32_StartGL();
void	GL_Win32_ShutDown(void);
bool	GL_Win32_GetMode(int mode, vid_mode_t *vidmode);
void	GL_Win32_BeginFrame(void);
void	GL_Win32_EndFrame(void);
bool	GL_Win32_IsFullScreen(void);

bool    GL_Win32_SetGammaValue(float gamma);
