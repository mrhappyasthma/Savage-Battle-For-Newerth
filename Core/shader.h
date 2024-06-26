// (C) 2003 S2 Games

// shader.h

// shader implementation


#define MAX_SHADER_MAPS	128


//the following constants specify how textures are handled:
typedef enum
{
	SHADER_SINGLE_TEXTURE,			//a single texture
	SHADER_ANIMATED_TEXTURE,
	SHADER_ANIMATED_TEXTURE_EX,
	SHADER_BINKVIDEO,
	SHADER_THEORAVIDEO,
	SHADER_MULTI_TEXTURE			//not supported yet
} shaderModes_enum;


typedef enum
{
	BLEND_ONE,
	BLEND_ONE_MINUS_SRC_ALPHA,
	BLEND_SRC_ALPHA
} blendmodes_enum;

typedef enum
{
	TEX_MODULATE,
	TEX_REPLACE,
	TEX_DECAL,
	TEX_BLEND,
	TEX_ADD,
} textureModes_enum;

typedef enum
{
	UV_NONE,
	UV_LINEAR_SCALE,
	UV_SIN_SCALE,
	UV_LINEAR_PAN,
	UV_SIN_PAN
} uvModes_enum;

typedef enum
{
	ZSORT_NONE,
	ZSORT_BACK_TO_FRONT
} zsortModes_enum;

typedef struct shader_s
{			
	char	*name;
	int		sortedidx;			//(used by gl_scene_builder.c)

	int		movieid;			//for bink/theora movie files
	int		numplays;			//for bink/theora movie files

	int		texmaps[MAX_SHADER_MAPS];			//internal indexes of images
	int		textureMode[MAX_SHADER_MAPS];		// texture blend operation
	int		texmap_num;			//number of texmaps.  in the case of SHADER_ANIMATED_TEXTURE, 
								//a list of textures in the animated shader.  in the case
								//of SHADER_MULTI_TEXTURE, the textures being
								//blended together (not supported yet).

	int		bumpmap;			// bump map image, 0 if none

	int		fps;				// fps of animated shader
	
	char	maptype;			//fixme: rename to shadermode
	int		srcblend;			// framebuffer blend operation
	int		dstblend;			// framebuffer blend operation

	//vec4_t	ambient;
	vec4_t	specular;
	vec4_t	emission;
	float	shininess;

	int		flags;

	bool	translucent;	//contains translucent texels
	int		zsortType;			//see ZSORT_* enum above

	//UV texture coord animation
	bool	animateTexCoords;
	int		uv_func[3];
	float	uv_speed[3];
	float	uv_phase[3];
	float	uv_amplitude[3];
	
	//if this texture is being used on the terrain, its shadermap reference is stored here
//	int		shdref;

	int		doubleSided;


	bool		active;
} shader_t;

#define	SHD_CALCULATEFRAME(time, fps, num_frames) ((int)((ABS(time)) * (fps)) % (num_frames))
#define SHD_CALCULATETIME(frame, fps) ((float)(frame) / (fps))


//uv functions

float	uvSin(shader_t *me, int paramidx, float time, float tc);
float	uvSquare(shader_t *me, int paramidx, float time, float tc);
float	uvTriangle(shader_t *me, int paramidx, float time, float tc);
float	uvSawtooth(shader_t *me, int paramidx, float time, float tc);
float	uvRandom(shader_t *me, int paramidx, float time, float tc);
float	uvNull(shader_t *me, int paramidx, float time, float tc);
