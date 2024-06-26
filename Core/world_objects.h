// (C) S2 Games

// world_objects.h

// provides an interface between the game code and core engine for dealing with ingame objects


//provides an index into an array of verts for groundplanes (fixme: move into savage_types so we can use this for other things)
typedef struct
{
	int start;
	int numverts;
} projectedFaceInfo_t;

#define MAX_REFERENCE_OBJECTS	256
extern referenceObject_t reference_objects[];

#define WO_CLIENT_CREATED	0x01
#define WO_SERVER_CREATED	0x02

#define	PATHNODE_LIMIT		7 // 4 corners in 2d, plus 3 nodes on top for bridges

#define	OBJECT_NODE_SPACER_DIST 35  //how far out from the sides do we put the nodes?

typedef struct
{
	bool				active;

//	cvarContainer_t		*def;
	objectPosition_t	objpos;
	vec3_t				axis[3];		//axis for rotation, derived from objpos
	int					objdef_idx;

	char				name[MAX_OBJECT_NAME_LENGTH];

	bool				reference;

	bool				shouldRender;

	bool				invisibleToGame;

	vec3_t				bmin;			//object bounds (world space)
	vec3_t				bmax;			//object bounds (world space)

	residx_t			model;
	residx_t			skin;

	residx_t			sound;
	float				soundVolume;

	sound_handle_t		soundIdx;
	
	float				createTime;

	bool				validGeomCache;	//for invalidating ground planes if the object changes position, etc.  set to false by world_objects, set to true by the rendering subsystem

	scenefacevert_t		*groundplaneVerts;
	residx_t			groundplaneShader;
	projectedFaceInfo_t	*groundplaneFaces;
	int					num_groundplaneFaces;

	int					game_index;		//index into the game objects array (used by structures we build in the game)

	linkedSurface_t		*surfs[SURF_LIMIT];
	int					num_surfs;

	int					flags;	//see WO_* defines above

	skeleton_t			skeleton;
} worldObject_t;

void		WO_Init();
int			WO_GetNumObjdefs();
char		*WO_GetObjdefName(int n);
int			WO_RegisterObjectVar(const char *objclass, const char *varname);
char		*WO_GetObjdefVar(int objdef_id, const char *varname);
void		WO_SetObjdefVar(int objdef_id, const char *varname, const char *string);
float		WO_GetObjdefVarValue(int objdef_id, const char *varname);
char 		*WO_GetObjdefVarString(int objdef_id, const char *varname);
void		WO_WriteObjposFile(void *zipfile, const char *filename);
void		WO_ClearObjects();
bool		WO_UseObjdef(const char *objdefName);
bool		WO_GetObjectPos(int id, objectPosition_t *objpos);
bool		WO_GetObjectBounds(int id, vec3_t bmin, vec3_t bmax);
void		WO_CreateObjectClass(const char *objclass);
void		WO_DeleteObject(int id);
void		WO_DeleteObject_Client(int id);
residx_t	WO_GetObjectModel(int id);
residx_t	WO_GetObjectSound(int id);
float		WO_GetObjectSoundVolume(int id);
sound_handle_t      WO_GetObjectSoundHandle(int id);
bool		WO_SetObjectSoundHandle(int id, sound_handle_t handle);
bool		WO_CreateObject(int objdef, const char *optmodel, const char *optskin, objectPosition_t *pos, int id);
residx_t	WO_GetObjectSkin(int id);
int			WO_GetObjectObjdef(int id);
void		WO_RenderObjects(vec3_t cameraPos);
int			WO_GetObjdefId(const char *name);
bool		WO_IsObjectDoubleSided(int id);
bool		WO_IsObjectReference(int id);
bool		WO_IsObjectActive(int id);
void		WO_SetObjectModelAndSkin(int id, residx_t model, residx_t skin);
int			WO_FindReferenceObject(int startindex, const char *refname, referenceObject_t *refobj);

bool	WO_CreateObject_Client(int objdef, const char *optmodel, const char *optskin, objectPosition_t *pos, int id);
bool	WO_CreateObject_Server(int objdef, const char *optmodel, const char *optskin, objectPosition_t *pos, int id);
void	WO_DeleteObject_Server(int id);

