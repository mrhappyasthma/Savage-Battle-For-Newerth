// (C) 2003 S2 Games

// gl_main.h

// OpenGL variables, function headers


// array dimensions
#define	VERTEX_ARRAY_SIZE		3
#define	VERTEX_ARRAY_DATATYPE	GL_FLOAT
#define	VERTEX_ARRAY_STRIDE		16

#define NORMAL_ARRAY_DATATYPE	GL_FLOAT
#define	NORMAL_ARRAY_STRIDE		0

#define	RGBA_COLOR_ARRAY_SIZE		4
#define	RGBA_COLOR_ARRAY_DATATYPE	GL_UNSIGNED_BYTE
#define	RGBA_COLOR_ARRAY_STRIDE		0

#define	RGB_COLOR_ARRAY_SIZE		3
#define	RGB_COLOR_ARRAY_DATATYPE	GL_UNSIGNED_BYTE
#define	RGB_COLOR_ARRAY_STRIDE		4

#define	TEXCOORD_ARRAY_SIZE		2
#define	TEXCOORD_ARRAY_DATATYPE	GL_FLOAT
#define TEXCOORD_ARRAY_STRIDE	0


//Vid_ functions
void	GL_RenderScene (camera_t *camera, vec3_t userpos);
void	GL_Quad2d (float x, float y, float w, float h, float s1, float t1, float s2, float t2, shader_t *shader);
void	GL_Poly2d(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, shader_t *shader);
void	GL_SetColor (vec4_t color);
void	GL_Notify (int message, int param1, int param2, int param3);
bool    GL_UnregisterShader(shader_t *shader);
bool	GL_RegisterShader (shader_t *shader);
bool	GL_RegisterShaderImageTextureMap(shader_t *shader, char *filename);
bool	GL_RegisterShaderImageBink(shader_t *shader, char *filename);
bool	GL_RegisterShaderImageTheora(shader_t *shader, char *filename);
bool	GL_RegisterShaderImageBumpMap(shader_t *shader, char *filename);
bool 	GL_RegisterShaderImageFromMemory(shader_t *shader, bitmap_t *image);
bool	GL_RegisterShaderImage(shader_t *shader, char *filename, bitmap_t *bitmap);
void	GL_ReadZBuffer(int x, int y, int w, int h, float *pixels);
void    GL_PrintWarnings();
bool	GL_RegisterModel(model_t *model);

/*
typedef struct
{
	bool multitexture;			//mmm... multitexturing....
	bool cva;
	bool nv_var;				//nvidia vertex array range extension
	bool nv_fence;				//nvidia fence extension
	bool texenv_combine;		//GL_EXT_texture_env_combine
	bool secondary_color;		//GL_EXT_secondary_color (for lighting / bump mapping)
	bool nv_register_combiners; //nvidia register combiners extension
	bool texture_env_dot3;		//ability to do plain dot3 bump-mapping
	bool bump_mapping;			//for convenience we set this if they can do bump mapping
								// since bump mapping relies of 3 of 4 extensions, we
								// don't want to check those every time
	bool separate_specular_color;
} gl_caps_t;
*/
typedef struct
{
	char		*vendor;
	char		*renderer;
	char		*version;
	char		*extensions;	

	float		cur_objtm[9];	//hack for lighting
} gl_info_t;

//used by image loading functions
typedef struct
{
	int				width;
	int				height;
	unsigned char	*data;
	int				mode;
	int				components;
} gl_bitmap_t;


//a utility structure for passing in array data to GL_DrawTriangles(), which calls glDrawElements


typedef struct
{
	vec3_t	*verts;
	vec2_t	*tverts;
	bvec4_t	*colors;
	vec3_t	*normals;
} gl_arrays_t;


//we call this from the driver init code.  This is init code that is non
//platform specific
void GL_RegisterVars();
void	GL_Init ();
void	GL_ShutDown ();

//sets a 2d drawing matrix 
void	GL_2dMode ();
void	GL_SetDrawRegion(int x, int y, int w, int h);


void	GL_Enable(int mode);
void	GL_Disable(int mode);
void	GL_DepthMask(int state);
void	GL_EnableClientState(int mode);
void	GL_DisableClientState(int mode);
void	GL_DepthFunc(int func);
void	GL_CullFace(int mode);
void	GL_DisableTextures();

void	GL_SelectShader(shader_t *shader, float time);
void	GL_SwitchTexUnit(int texunit);

void	GL_SetRegisterShaderCallback(void(*callback)(shader_t *shader));

void	*GL_DynamicMalloc(int size);
void	*GL_StaticMalloc(int size);
void	GL_StaticMallocHold();
void	GL_StaticMallocFetch();
void	GL_SetFence();

void	GL_CopySystemArrays(gl_arrays_t *from, gl_arrays_t *to, int numVerts);
void	GL_SetVertexPointers(gl_arrays_t *arrays, bool ignoreAlpha);
//should always use this instead of glDrawElements()
void	GL_DrawTriangles(int primtype, int numElems, void *elemList);

//glu functions
void _gluPerspective (GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);

//framebuffer grab functions
void GL_GetFrameBuffer(bitmap_t *screenshot);

void GL_ProjectVertex(camera_t *cam, vec3_t vertex, vec2_t result);
void GL_ProjectVertexInternal(vec3_t vertex, vec3_t result);

GLint round2(GLint n);

int		GL_GetTexEnvModeFromString(const char *string);
void	GL_AxisToMatrix(vec3_t axis[3], float tm[16]);
void	GL_TransformToMatrix(matrix43_t *transform, float tm[16]);

//GL extensions

extern cvar_t	gl_ext_multitexture;
extern cvar_t	gl_ext_compiled_vertex_array;
extern cvar_t	gl_ext_vertex_array_range;
extern cvar_t	gl_ext_fence;
extern cvar_t	gl_ext_texenv_combine;
extern cvar_t	gl_ext_secondary_color;
extern cvar_t	gl_ext_register_combiners;
extern cvar_t	gl_ext_texture_env_dot3;
extern cvar_t	gl_bump_mapping;
extern cvar_t	gl_ext_separate_specular_color;

//vertex buffer object support
#ifndef unix
extern PFNGLBINDBUFFERARBPROC				glBindBufferARB;
extern PFNGLDELETEBUFFERSARBPROC			glDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC				glGenBuffersARB;
extern PFNGLISBUFFERARBPROC				glIsBufferARB;
extern PFNGLBUFFERDATAARBPROC				glBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC			glBufferSubDataARB;
extern PFNGLGETBUFFERSUBDATAARBPROC		glGetBufferSubDataARB;
extern PFNGLMAPBUFFERARBPROC				glMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC				glUnmapBufferARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC	glGetBufferParameterivARB;
extern PFNGLGETBUFFERPOINTERVARBPROC		glGetBufferPointervARB;
#endif
