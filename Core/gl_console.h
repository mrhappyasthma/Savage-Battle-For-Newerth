// (C) 2003 S2 Games

// gl_console.h

int		GL_Console_SetMode( int mode );
int		GL_Console_Init ();
void    GL_Console_StartGL();
void	GL_Console_ShutDown(void);
bool	GL_Console_GetMode(int mode, vid_mode_t *vidmode);
void	GL_Console_BeginFrame(void);
void	GL_Console_EndFrame(void);

bool	GL_Console_IsFullScreen();

void	GL_Console_InitScene(void);
void	GL_Console_RenderScene (camera_t *camera, vec3_t userpos);
void    GL_Console_Quad2d (float x, float y, float w, float h, float s1, float t1, float s2, float t2, shader_t *shader);
void    GL_Console_Poly2d(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, shader_t *shader);
void    GL_Console_SetColor (vec4_t color);
void    GL_Console_Notify (int message, int param1, int param2, int param3);
bool    GL_Console_UnregisterShader(shader_t *shader);
bool    GL_Console_RegisterShader (shader_t *shader);
bool    GL_Console_RegisterShaderImageTextureMap(shader_t *shader, char *filename);
bool    GL_Console_RegisterShaderImageBumpMap(shader_t *shader, char *filename);
bool    GL_Console_RegisterShaderImageFromMemory(shader_t *shader, bitmap_t *image);
bool    GL_Console_RegisterShaderImage(shader_t *shader, char *filename, bitmap_t *bitmap);
void    GL_Console_ReadZBuffer(int x, int y, int w, int h, float *pixels);

//framebuffer grab functions
void 	GL_Console_GetFrameBuffer(bitmap_t *screenshot);
void 	GL_Console_ProjectVertex(camera_t *cam, vec3_t vertex, vec2_t result);
void	GL_Console_SetDrawRegion(int x, int y, int w, int h);

bool	GL_Console_RegisterModel(model_t *model);
bool    GL_Console_SetGamma(float gamma);

void	GL_Console_RegisterVars();
void	GL_Console_PrintWarnings();
