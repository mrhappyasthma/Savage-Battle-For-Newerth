// (C) 2003 S2 Games

// vid.h


#define MAX_VID_DRIVERS 5
#define MAX_VID_MODES	50


typedef struct
{
	int width;
	int height;
	int bpp;
	bool fullscreen;
	char name[20];

} vid_mode_t;

//this structure holds pointers to all fundamental graphics functions
typedef struct
{
	char	*drivername;
	
	void			(*RegisterVars)();
	void			(*PrintWarnings)();
	int				(*Init)();
	void			(*StartGL)();
	int				(*SetMode)(int mode);
	bool			(*GetMode)(int mode, vid_mode_t *vidmode);
	bool			(*IsFullScreen)(void);
	void			(*ShutDown)(void);

	//rendering
	void			(*BeginFrame)(void);
	void			(*EndFrame)(void);
	void			(*InitScene)(void);
	void			(*RenderScene)(camera_t *camera, vec3_t userpos);	
	void			(*DrawQuad2d)(float x, float y, float w, float h, float s1, float t1, float s2, float t2, shader_t *shader);
	void			(*DrawPoly2d)(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, shader_t *shader);
	void			(*SetDrawRegion)(int x, int y, int w, int h);
	void			(*SetColor)(vec4_t color);
//	void			(*RenderHeightmapSection)(vec3_t bmin, vec3_t bmax);
	void			(*Notify)(int message, int param1, int param2, int param3);
	bool    		(*UnregisterShader)(shader_t *shader);
	bool			(*RegisterShader)(shader_t *shader);
	bool			(*RegisterShaderImage)(shader_t *shader, bitmap_t *bmp);
	void			(*GetFrameBuffer)(bitmap_t *screenshot);
	void			(*ProjectVertex)(camera_t *cam, vec3_t vertex, vec2_t result);
	void			(*ReadZBuffer)(int x, int y, int w, int h, float *zpixels);
	bool			(*RegisterModel)(model_t *model);
	bool			(*SetGamma)(float gamma);
//	void *			(*AllocMem)(int size);
} _vid_driver_t;

//right now we're just using OpenGL, but this is here for the
//possibility of adding something like a Direct3d driver later on
extern _vid_driver_t	_vid_drivers[MAX_VID_DRIVERS];
extern int				_vid_driver_num;




//sets the initial driver or changes the current driver
//the default driver used will be _vid_drivers[0]
void	Vid_SetDriver (char *drivername);

void	Vid_RegisterVars ();
void	Vid_PrintWarnings ();

//initializes a video window using mode 0, unless otherwise
//indicated in the config file
//should only be called once at startup
void	Vid_Init ();

//changes the current video mode
void	Vid_SetMode (int mode);

//return a string in the format:
//[width]x[height]x[depth] [width]x[height]x[depth] ...etc
//
//i.e. "640x480x16 800x600x16 1024x768x16"
char	*Vid_GetModeString();

//returns a pointer to a vid_mode_t struct
vid_mode_t	*Vid_GetModePtr(int mode);

bool	Vid_RegisterModel(model_t *model);

//shuts down the current video driver 
void	Vid_ShutDown ();

void	Vid_InitScene ();

//all drawing goes through this function, so we call our
//specific driver drawing functions through here, as opposed to
//putting them in _vid_driver_t or having a function header for them
//(we don't want any external interface to them; they should only be
//called by the RenderScene function)
void	Vid_RenderScene (camera_t *camera, vec3_t userpos);

void	Vid_DrawQuad2d (float x, float y, float w, float h, float s1, float t1, float s2, float t2, shader_t *shader);
void	Vid_DrawPoly2d(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, shader_t *shader);


void	Vid_SetDrawRegion(int x, int y, int w, int h);

void	Vid_NormalizeColor(const vec4_t color, vec4_t out);
void	Vid_SetColor (vec4_t color);

//void	Vid_RenderHeightmapSection (vec3_t bmin, vec3_t bmax);

//sometimes system functions have to notify the video
//system that they have been called, such as loading
//of a new map, modification of terrain data, etc
void	Vid_Notify(int message, int param1, int param2, int param3);

//called before the client frame to notify the system we may be drawing
void	Vid_BeginFrame ();
//called at the end of the client frame to swap buffers (and any other cleanup)
void	Vid_EndFrame ();


//the following 3 functions will not work unless Vid_SetMode() was
//called:

//returns the screen width for the current mode
int		Vid_GetScreenW ();
//returns the screen height for the current mode
int		Vid_GetScreenH ();
//returns the number of the current mode
int		Vid_CurrentMode ();

#ifdef _WIN32
void	Vid_GetClientOffset(int *x, int *y);
#endif //_WIN32

bool    Vid_UnregisterShader(shader_t *shader);
bool	Vid_RegisterShader (shader_t *shader);
bool	Vid_RegisterShaderImage(shader_t *shader, bitmap_t *bmp);

void	Vid_GetFrameBuffer (bitmap_t *bmp);

void	Vid_ProjectVertex(camera_t *cam, vec3_t vertex, vec2_t result);

void	Vid_ReadZBuffer(int x, int y, int w, int h, float *zpixels);

bool	Vid_SetGamma(float gamma);

bool	Vid_IsFullScreen();

//void	*Vid_AllocMem (int size);

extern	cvar_t	vid_mode;
extern	cvar_t	vid_blockmegs;

extern	bool	vid_initialized;
extern	cvar_t	vid_useSystemMem;
