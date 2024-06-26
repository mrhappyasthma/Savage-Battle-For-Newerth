// (C) 2003 S2 Games

// scene.h

// Includes function definitions for all drawing functions



#define		SCENE_FRAMEPOOL_SIZE 1024 * 1024 * 4

//scenelist, scenefacelist, and scenelightlist are needed by gl_scene, so we expose them here
typedef struct scenelist_s
{
	sceneobj_t			obj;	//must be the first field in this struct!
	bool				cull;  //if true, we do not draw this object
	struct scenelist_s	*next;
} scenelist_t;

typedef struct scenefacelist_s
{	
	scenefacevert_t			*verts;
	int						nverts;
	residx_t				shader;
	bool					cull;
	int						flags;	//see POLY_* defines in savage_types.h
	struct scenefacelist_s	*next;
} scenefacelist_t;

typedef struct scenelightlist_s
{
	scenelight_t			light;
	bool					cull;
	struct scenelightlist_s *next;
} scenelightlist_t;


typedef struct occluderlist_s
{
	occluder_t				occluder;
	bool					cull;
	struct occluderlist_s	*next;
} occluderlist_t;

typedef struct
{
	int						polycount;
	int						translucentPolycount;
	int						drawTrianglesCount;
	int						sceneObjCount;
	int						culledSceneObjCount;
	int						renderedSceneObjCount;
	int						numOutsideFrustum;
	int						numOccluded;
	int						numSetBoneAnims;
	int						numPoseBones;
	int						numTextureSwitches;
	int						numSelectShaders;
	int						numRenderScene;
} sceneStats_t;


extern scenelist_t			*scenelist;
extern scenelist_t			*skylist;
extern scenelist_t			*spritelist;
extern scenefacelist_t		*scenefacelist;
extern scenelightlist_t		*scenelightlist;
extern occluderlist_t		*occluderlist;
extern sceneStats_t			sceneStats;

void		Scene_SetFrustum(camera_t *camera);
void		Scene_AddOccluder(occluder_t *occluder);
void		Scene_AddWorldOccluders();
void		Scene_AddObject(sceneobj_t *scobj);
void		Scene_AddLight (scenelight_t *light);
bool		Scene_ObjectInFrustum(sceneobj_t *obj);
void		*Scene_FramePool_Alloc(int size);
void		Scene_Clear();
void		Scene_Render(camera_t *camera, vec3_t userpos);
void		Scene_AddPoly(int nverts,  scenefacevert_t *verts, residx_t shader, int flags);
int			Scene_ClipPoly(scenefacevert_t *verts, int numVerts, plane_t *clipPlanes, int numClipPlanes, scenefacevert_t *outpoly, int maxVerts);
void		Scene_Init();
void		Scene_AddSkyObj(sceneobj_t *sky);
int			Scene_ClipBounds(const vec3_t bmin, const vec3_t bmax);
bool		Scene_BoxInView(const vec3_t bmin, const vec3_t bmax);
int			Scene_SelectObjects(camera_t *camera, int rx, int ry, int rw, int rh, int *buf, int bufsize);
void		Scene_ClampLightedColor(vec3_t color);
void		Scene_CullFaces();
void		Scene_AddLight(scenelight_t *light);
void		Scene_BuildObjectMesh(sceneobj_t *obj, mesh_t *mesh, vec4_t *vertarray, vec3_t *normalarray);
int			Scene_ClipPolyToPlane(scenefacevert_t *verts, int numVerts, plane_t *clipPlane, scenefacevert_t *outpoly, int maxVerts);

void		Scene_ClipPolyToTerrainTris(scenefacevert_t *verts, int numverts, 
								 ivec2_t min, ivec2_t max,
								 scenefacevert_t *clipped_verts,
								 scenefacevert_t *clipped_verts1, int *clipped_numverts1, 
								 scenefacevert_t *clipped_verts2, int *clipped_numverts2);
bool		Scene_AABBInFrustum(const vec3_t bmin, const vec3_t bmax, bool ignoreNearFar);
bool		Scene_AABBIsVisible(const vec3_t bmin, const vec3_t bmax);

//2d rendering

void		Draw_SetShaderTime(float time);
void		Draw_SetColor(vec4_t color);
float       Draw_GetCurrentAlpha();
void		Draw_Quad2d(float x, float y, float w, float h, float s1, float t1, float s2, float t2, residx_t shader);
void		Draw_Poly2d(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, residx_t shader);
void		Draw_SetRegion(int x, int y, int w, int h);
void		Scene_StatsFrame();

extern		plane_t	frustum[6];				//frustum planes
extern		vec3_t	frustum_bmin;			//frustum bounding box
extern		vec3_t	frustum_bmax;			//frustum bounding box
extern		vec3_t	frustum_bpos;			//frustum bounding box
extern		vec3_t	frustum_bext;			//frustum bounding box
extern		line_t	frustum_edges[6][3];	//silhouette edges in 3 orthographic views

//cvars

extern cvar_t		gfx_fog;
extern cvar_t		gfx_fog_near;
extern cvar_t		gfx_fog_far;
extern cvar_t		gfx_fogr;
extern cvar_t		gfx_fogg;
extern cvar_t		gfx_fogb;
extern cvar_t		gfx_nearclip;
extern cvar_t		gfx_farclip;
extern cvar_t		gfx_staticlod;
extern cvar_t		gfx_sky;
