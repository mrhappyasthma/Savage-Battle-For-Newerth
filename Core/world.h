// (C) 2003 S2 Games

// world.h

// world management

#include "world_tree2.h"
#include "world_objects.h"
#include "world_lights.h"

#define WITHIN_WORLD(xpos, ypos) ((xpos) >= 0 && (xpos) < (world.gridwidth) && (ypos) >= 0 && (ypos) < (world.gridheight))

/********* Video driver notification messages ********/

typedef enum
{
	VID_NOTIFY_NEW_WORLD = 1,
	//a new world has been loaded
	VID_NOTIFY_TERRAIN_COLOR_MODIFIED,
	//the terrain colormap or dynamap has been modified
	//param1: xpos   param2: ypos   
	//param3: bool indicating if ALL terrain should be rebuilt (if so, first 2 params are ignored)
	VID_NOTIFY_TERRAIN_VERTEX_MODIFIED,
	//world.grid has been modified
	//param1: xpos   param2: ypos
	//param3: bool indicating if ALL terrain should be rebuilt (if so, first 2 params are ignored)
	VID_NOTIFY_TERRAIN_SHADER_MODIFIED,
	//a shader on the terrain has been changed
	//param1: xpos   param2: ypos
	//param3: bool indicating if ALL terrain should be rebuilt (if so, first 2 params are ignored)
	VID_NOTIFY_TERRAIN_TEXCOORD_MODIFIED,
	//unused
	VID_NOTIFY_TERRAIN_NORMAL_MODIFIED,
	//a normal on the terrain has been changed
	//param1: xpos   param2: ypos
	//param3: bool indicating if ALL terrain should be rebuilt (if so, first 2 params are ignored)
	VID_NOTIFY_WORLD_DESTROYED,
	//the world has been destroyed
} vidNotify_enum;

/*****************************************************/


//idea: two different types of terrain deforms?
//one can be render only (so the terrain can be deformed
//without affecting units...i.e. if we want water to come up
//above a unit's feet)..the other would affect the unit


typedef struct
{	
	//for all the following 'map' structures, the dimensions
	//must correspond EXACTLY to world.gridwidth and world.gridheight.
	//we also assume that all pointers have been allocated 
	//(handled in World_Load)
	
	colormap_t		*colormap;
	colormap_t		*dynamap;		//for dynamic lighting etc
	shadermap_t		*shadermap[2];	
	float			*foliageChance;

	residx_t		shaders[MAX_SHADERS];

	bool			ignoreColormap;
} worldrender_t;


//fixme: move back into world_tree2.c
typedef struct
{
	vec3_t	start;
	vec3_t	end;
	vec3_t	dir;
	vec3_t	bmin;
	vec3_t	bmax;
	vec3_t	bmin_w;		//start + bmin
	vec3_t	bmax_w;		//start + bmax
	vec3_t	bmin_w_end;	//end + bmin
	vec3_t	bmax_w_end;	//end + bmax
	vec3_t	bpos;
	vec3_t	bext;
	vec3_t	velocity;
	int		ignoreSurface;
	int		ignoreIndex;
	float	fraction;
	int		clientOrServer;
	vec3_t	normal;
	float	dist;
	int		collisionFlags;
	float	length;
	bool	embedded;
	bool	startedInSurface;
	bool	lineTrace;
	bool	startEqualsEnd;
	float	epsilonFrac;
} workingTraceVars_t;

#define	MAX_WORLD_OCCLUDERS 128

//this is the main structure which holds our collision info
//rendering info is not stored here, though the quadtree is used for culling during rendering

#define WORLD_NAME_LENGTH	256
typedef struct
{
	//these fields must remain in this order!
	int				gridwidth;
	int				gridheight;
	heightmap_t		*grid;
	float			max_z;
	float			min_z;

	bool			loaded;		//shared resources loaded
	bool			cl_loaded;	//client side resources loaded
	heightmap_t		*maxalt;
	heightmap_t		*minalt;
	vec3_t			*normal[2];

	float			farclip;	//value of farclip in config file

	float			savescale;

	char			name[WORLD_NAME_LENGTH];

	quadnode_t		*tree[MAX_TREE_LEVELS+1];	
	int				tree_levelwidths[MAX_TREE_LEVELS+1];
	float			tree_boxdims[MAX_TREE_LEVELS+1];
	int				tree_levels;

	//CRC hashes for heightmap and .objpos file
	//fixme: these should get replaced with a CRC for the entire ZIP file for a world
	unsigned char	s2z_hash[MAX_HASH_SIZE];
	int				s2z_hashlen;

	workingTraceVars_t	lastTrace;

	archive_t		*archive;

	int				numOccluders;
	occluder_t		occluders[MAX_WORLD_OCCLUDERS];
} world_t;

/*
typedef struct
{
	
} gridcell_t;
*/
extern world_t world;
extern worldrender_t wr;


//	Returns information about the terrain at the given x/y value, including
//	terrain flags, height of the terrain at (x,y), and the surface normal.
void		World_SampleGround(float x, float y, pointinfo_t *result);

int			World_RayCellIntersect(int x, int y, const vec3_t origin, const vec3_t dir, vec3_t ipoint);

//float		World_GetHeight(float x, float y);

//	Deforms terrain at the given heightmap point (x,y)
void		World_DeformGround(int gridx, int gridy, heightmap_t altitude);

heightmap_t	World_GetGridHeight(int gridx, int gridy);

bool    	World_Exists(const char *name);
//initializes and loads a world
bool		World_Load(const char *name, bool loadClientSide);

void		World_Destroy();

void		World_Init();

void		World_Update();

void		World_GetBounds(vec3_t bmin, vec3_t bmax);

float		World_CalcMinZUnaligned(float x1, float y1, float x2, float y2);
float		World_CalcMaxZUnaligned(float x1, float y1, float x2, float y2);
float		World_CalcMinZ(float x1, float y1, float x2, float y2);
float		World_CalcMaxZ(float x1, float y1, float x2, float y2);

float World_WorldToGrid(float coord);
float World_GridToWorld(float gridcoord);



//world render (WR) functions

//builds a face list of faces to render based on the
//current viewing volume.  No sorting of the faces is
//done here.  That part is left up to the actual render 
//function (which will most likely sort faces in order
//of shader, as well as do tristrip finding, etc)
void		WR_RenderTerrain(camera_t *camera);

//returns a pointer to a shader based on the grid (x,y) position
residx_t	WR_GetShaderAt(int layer, int x, int y);

//returns the amount the two shaders should be blended at this location
float		WR_GetShaderBlend(int x, int y);
void		WR_GetNormal(int x, int y, vec3_t nml, int tri);

void		WR_ClearDynamap();
void		WR_SetDynamap(int x, int y, bvec4_t color);
void        WR_GetDynamap(int gridx, int gridy, bvec4_t color);
void		WR_ClearDynamapToColor(bvec4_t color);
void        WR_ClearDynamapToColorEx(bvec4_t color, bvec4_t conditional);
void        WR_DynamapToBitmap(bitmap_t *bmp);

void		WR_SetColormap(int x, int y, bvec4_t color);
void		WR_GetColormap(int x, int y, bvec4_t color);

void		WR_PaintShader(int x, int y, residx_t shader, bool layer2);
residx_t	WR_GetShadermap(int x, int y);

void		WR_CalcColor(int xpos, int ypos, vec3_t color);
void		WR_GetSunVector(vec3_t out);

float		WR_FarClip();

float		World_GetTerrainHeight(float xpos, float ypos);

//void		WR_FitQuad(scenefacevert_t quadverts[4], residx_t shader, int flags);

char		*World_GetName();

bool    	World_CheckHashes(int s2z_hashlen, unsigned char s2z_hash[MAX_HASH_SIZE]);
bool		World_CheckHashForDemo(int length, unsigned char hash[MAX_HASH_SIZE]);

bool        World_UseColormap(bool use);

void		World_InitLighting();

void		World_GetOccluder(int num, occluder_t *out);
bool		World_UpdateOccluder(int num, const occluder_t *occ);
int			World_GetNumOccluders();
void		World_ClearOccluders();
int			World_AddOccluder(occluder_t *out);
void		World_RemoveOccluder(int num);

/*
extern		cvar_t	wr_lighting;
extern		cvar_t	wr_lightheight;
extern		cvar_t	wr_lightr;
extern		cvar_t	wr_lightg;
extern		cvar_t	wr_lightb;
extern		cvar_t	wr_lightx;
extern		cvar_t	wr_lighty;
extern		cvar_t	wr_lightz;
extern		cvar_t	wr_lightactive;
extern		cvar_t	wr_light2r;
extern		cvar_t	wr_light2g;
extern		cvar_t	wr_light2b;
extern		cvar_t	wr_light2x;
extern		cvar_t	wr_light2y;
extern		cvar_t	wr_light2z;
extern		cvar_t	wr_light2active;
*/
extern		cvar_t	wr_showtree;
extern		cvar_t	world_scale;
extern		cvar_t	world_heightscale;

//hack
extern		vec3_t leray;
extern		vec3_t leorigin;
extern		vec3_t tri1[3];
extern		vec3_t tri2[3];



