// (C) 2003 S2 Games

// res.h

// Resource loading / management


// All the Res_Load* functions use a cacheing system.  If the 
// resource is already loaded into memory, its index is found and
// returned rather than re-loading the file.

// The file to be loaded is first searched through in whatever
// resource files have been registered (via zlib).  If not found
// there, the file is looked for on the disk

#define		MAX_SHADER_BUCKETS	MAX_SHADERS

#define		MAX_SHADERS			2048
#define		MAX_MODELS			2048
#define		MAX_SOUNDS			256		//not a strict maximum
#define		MAX_CLIPS			256		//not a strict maximum
#define		MAX_STRINGTABLES	64		//not a strict maximum
#define		MAX_FONTS			8


#define		SHADER_RESIDX_OFFSET	0
#define		MODEL_RESIDX_OFFSET		0
#define		SKIN_RESIDX_OFFSET		0
#define		SOUND_RESIDX_OFFSET		0
#define		FONT_RESIDX_OFFSET		0


#define MEDIA_NAME_LENGTH	1024

struct media_s;

typedef struct
{
	struct media_s *self;
} mediatag_t;

typedef struct media_s
{
//	bool	loaded;
	char	name[MEDIA_NAME_LENGTH];
	
	union
	{
		shader_t		*shader;		
		fontData_t		*font;
		model_t			*model;
		modelClip_t		*clip;
		modelSkin_t		*skin;
		stringTable_t	*stringtable;
		sample_t		*sample;

		mediatag_t		*tag;

		byte			*data;
	} d;
} media_t;



typedef residx_t	modelLods_t[4];

residx_t	Res_LoadModel(const char *name);
residx_t	Res_LoadShader(const char *name);
residx_t	Res_LoadShaderEx(const char *name, int flags);
residx_t    Res_LoadRawTextureFromMemoryEx(int shader_to_use, bitmap_t *bmp, int flags);

residx_t	Res_LoadTerrainShader(const char *name);
int			Res_LoadSkin(residx_t model, const char *name);
residx_t	Res_LoadClip(const char *name);
residx_t	Res_LoadStringTable(const char *name);

int			Res_GetNumTextureFrames(residx_t shader);
int			Res_GetAnimatedTextureFPS(residx_t shader);

void		Res_GetModelVisualBounds(residx_t model, vec3_t bmin, vec3_t bmax);
bool		Res_GetModelSurfaceBounds(residx_t model, vec3_t bmin, vec3_t bmax);

char		*Res_IsShaderFile(char *name);

bool		Res_UnloadShader(residx_t residx);
void		Res_UnloadSound(residx_t residx);

residx_t	Res_LoadFont(const char *filename, int small_size, int medium_size, int big_size);
bool        Res_UnloadFont(residx_t idx);
bool        Res_AddCharToFont(residx_t idx, int charnum);

//sound

residx_t	Res_LoadSound(const char *name);

// Registers a ZIP file which contains resource files which are to 
// be loaded using Res_Load*

bool		Res_RegisterZip(const char *name);

//non-api functions

shader_t	*Res_GetShader(residx_t idx);
model_t		*Res_GetModel(residx_t idx);
model_t		*Res_GetModelLOD(residx_t idx, int lod);
fontData_t	*Res_GetFont(residx_t idx);
modelClip_t		*Res_GetClip(residx_t idx);
stringTable_t	*Res_GetStringTable(residx_t idx);
sample_t	*Res_GetSample(residx_t idx);
bool        Res_RefreshFont(residx_t idx);

void		Res_Init();

void		Res_SetLoadingStatusCallback(void(*callback)(const char *resourcename));
void		Res_UpdateLoadingStatus(const char *name);

//pass in 0 on the first call, and the returned residx_t on subsequent calls
residx_t    Res_GetDynamapShader(residx_t shader);
int			Res_GetMoviePlayCount(residx_t shadernum);
void		Res_BinkStop(residx_t shadernum);
void		Res_BinkContinue(residx_t shadernum);
void		Res_BinkRestart(residx_t shadernum);
void		Res_BinkUnload(residx_t shadernum);
void		Res_TheoraStop(residx_t shadernum);
void		Res_TheoraContinue(residx_t shadernum);
void		Res_TheoraRestart(residx_t shadernum);
void		Res_TheoraUnload(residx_t shadernum);
