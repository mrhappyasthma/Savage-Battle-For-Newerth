// (C) 2001 S2 Games

// world_tree2.h


#define		QUAD_TL	0
#define		QUAD_TR	1
#define		QUAD_BL	2
#define		QUAD_BR	3

#define		MAX_TREE_LEVELS	8

typedef struct
{
	vec3_t v[3];
	vec3_t normal;
} triangle_t;



#define MAX_TRIMESH_TRIS	128

//this is the struct we pass into world_tree when adding a surface to the world
//the mesh contained in this structure must be convex
typedef struct
{
	int		index;		//if surface belongs to a static worldObject, this is set to the index of the object

	vec3_t	pos;		//the position of the surface, in world space
	vec3_t	angles;		//the orientation of the surface

	int		numTris;	//the number of tris which compose the surface
	triangle_t tris[MAX_TRIMESH_TRIS];		//the tri data in object space

	int		flags;		//surface flags which can be used by the game code to respond in different ways to a collision
} convexTriMesh_t;


/*
typedef struct
{
	vec3_t	pos;
	vec3_t	angles;
	
	int		numTris;
	triangle_t
} convexMesh_t;
*/
//this is the struct we pass into world_tree when adding a static box to the world
typedef struct
{
	vec3_t	pos;
	vec3_t	angles;
	vec3_t	bmin;
	vec3_t	bmax;
	int		flags;
} boxSurface_t;


//linkedSurface_t describes the resulting collisionSurface_t after it has been linked into the world
typedef struct linkedSurface_s
{
	bool	active;
	int		index;

	residx_t model;	

	vec3_t worldPos;	
	vec3_t worldAxis[3];
	float invScale;
	
	int		numPlanes;			//the number of planes which compose the surface
	plane_t	*planes;		//the surface planes in world space
	int		flags;				//surface flags;

	//the AABB (axis-aligned bounding box) encloses the polygonal surface
	//calculated automatically when the surface is placed in the world
	vec3_t	bmin_w;
	vec3_t	bmax_w;

	struct	quadnode_s *node;

	struct linkedSurface_s *next;
	struct linkedSurface_s *prev;
} linkedSurface_t;

/*
typedef struct linkedModel_s
{
	bool	active;
	int		index;

	residx_t model;

	vec3_t worldPos;	
	vec3_t worldAxis[3];
	float invScale;

	int		flags;

	vec3_t	bmin_w;
	vec3_t	bmax_w;

	struct	quadnode_s *node;
	struct linkedModel_s *next;
	struct linkedModel_s *prev;
} linkedModel_t;
*/

//linkedObject_t is an axis-aligned bounding box representing a game object from cl.objects or sl.objects
//we store extra private data here that we don't need in baseObject, but we can consider this an extension to baseObject
typedef struct linkedObject_s
{
	bool	linked;
	int		index;
	int		surfaceFlags;

	//bounding box world coords, calculated when the object is linked via WT_LinkObject
	//this is going to differ slightly from the base object as we floor these values
	//(for consistency with client side prediction, since coordinates get sent as ints)
	vec3_t	bmin_w;
	vec3_t	bmax_w;

	struct	quadnode_s	*node;

	struct	linkedObject_s *next;
	struct	linkedObject_s *prev;
} linkedObject_t;


typedef struct quadnode_s
{
	//min / max form:
	vec3_t				bmin;		//bounding box min, world space
	vec3_t				bmax;		//bounding box max, world space

	struct quadnode_s	*children[4];	//(we preallocate nodes)
	struct quadnode_s	*parent;

	int					level;

	linkedSurface_t		surfaceList;
	//linkedModel_t		modelList;

	linkedObject_t		clientObjectList;		//for client-side collision detection (used in client-side prediction)
	linkedObject_t		serverObjectList;		//for server-side collision detection
	baseObject_t		*visList;

	int					thingCount;					//this node or its children contain objects/surfaces
} quadnode_t;



void			WT_Init();

void			WT_PushNode(quadnode_t *node);
quadnode_t		*WT_PopNode();
void			WT_ClearStack();

void			WT_UpdateHeight(int xpos, int ypos, heightmap_t altitude);
void			WT_RecalcTree();
void			WT_BuildTree(int levels);
void			WT_DestroyTree();

quadnode_t		*WT_ChildNodeReference(int parent_level, int parent_x, int parent_y, int quadpos);
quadnode_t		*WT_GetNodeAtXY(int x, int y, int level);
quadnode_t		*WT_FindBestFitNode(vec2_t bmin, vec2_t bmax);

bool			WT_TraceBox_Server(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface);
bool			WT_TraceBox_Client(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface);
bool			WT_TraceBoxEx_Server(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface, int ignoreIndex);
bool			WT_TraceBoxEx_Client(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface, int ignoreIndex);

void			WT_UpdateTerrainHeight(int gridx, int gridy, float altitude);

bool    		WT_IsLinked_Server(baseObject_t *obj);
void			WT_LinkObject_Client(baseObject_t *obj);
void			WT_LinkObject_Server(baseObject_t *obj);
void			WT_UnlinkObject_Client(baseObject_t *obj);
void			WT_UnlinkObject_Server(baseObject_t *obj);

linkedSurface_t	*WT_LinkPolyhedron(convexPolyhedron_t *poly, int index, residx_t model, float scale, const vec3_t position, const vec3_t axis[3]);
void			WT_UnlinkPolyhedron(linkedSurface_t *surf);

void			WT_ScalePolyhedron(convexPolyhedron_t *in, float scale, convexPolyhedron_t *out);
void			WT_TransformPolyhedron(convexPolyhedron_t *in, const vec3_t pos, const vec3_t axis[3], convexPolyhedron_t *out);

void			WT_FitPolyToTerrain(scenefacevert_t *verts, int num_verts, residx_t shader, int flags, void (*clipCallback)(int num_verts, scenefacevert_t *verts, residx_t shader, int flags));
void		WT_ResetTraceCount();

void	WT_ResetDynamic(bool client);