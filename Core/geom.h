// (C) 2003 S2 Games

// geom.h

// geometry stuff


#define			FACE_TERRAIN
#define			FACE_MODEL
#define			FACE_SPRITE

typedef char nml_t[3];
typedef word keyVert_t[3];

#define	MAX_POLYHEDRON_PLANES	128

#define MAX_BONES			128
#define MAX_SKINS_PER_MODEL	8

#define BONE_NAME_LENGTH	32
#define MESH_NAME_LENGTH	32
#define SKIN_NAME_LENGTH	32
#define MAX_RENDER_DATA		8
#define SURF_LIMIT			256

#define	SKIN_SHADERNAME_LENGTH	64

typedef enum
{
	RD_VERTS,
	RD_TVERTS,
	RD_COLORS,
	RD_NORMALS,
} renderData_enum;


typedef struct
{
	int	num_verts;
	int	*verts;
} polyface_t;

typedef struct
{
	int		numPlanes;
	plane_t planes[MAX_POLYHEDRON_PLANES + 6];	//the 6 extra planes are for the beveling step

	vec3_t	bmin;		//must be computed beforehand
	vec3_t	bmax;		//must be computed beforehand

	int		flags;

	//trimesh representation (for drawing and debugging)
	int	num_verts;
	vec3_t *verts;
	int num_faces;
	uivec3_t *facelist;	
} convexPolyhedron_t;


//a mesh is like a "sub object" of a model
//meshes may use any of the 3 following "modes":
//
//MESH_SKINNED_BLENDED
//	each vertex on the mesh is linked to one or more bones
//  if any one vertex in the mesh is linked to more than one bone, this is the mode that gets set
//MESH_SKINNED_NONBLENDED
//  one bone link per vertex
//  this mode gets set for all other meshes, including meshes that did not have physique applied
//  (a bone will be generated for this mesh that all vertices are linked to)
//  even meshes which use keyframe data will have their geometry set (though it may not get used)

typedef enum
{
	MESH_SKINNED_BLENDED = 1,
	MESH_SKINNED_NONBLENDED	
} meshModes_enum;



typedef struct
{
	int				*indexes;		//num_weights links will be allocated

	float			*weights;		//num_weights will be allocated

	int				num_weights;	//number of weights this vertex is affected by (from 1 to model_t::num_bones, or any maximum that we decide to set)
									//if num_weights is 1, use the union member "index" to get the bone attachment
} blendedLink_t;

typedef int singleLink_t;

typedef struct
{
	int				bone;			//bone we are linked to
} nonblendedVert_t;


typedef struct
{
	//a keyframe vertex is transformed to mesh space via:
	//(key_verts[v] / 255.0) * bext + bpos
	vec3_t			bpos;				//bounding box center
	vec3_t			bext;				//bounding box extents
	keyVert_t		*key_verts;
} keyframe_t;

typedef struct mesh_s
{
	struct model_s *model;		//model this mesh belongs to

	char			name[MESH_NAME_LENGTH];		//mesh name for skin lookup
	char			defaultShader[SKIN_SHADERNAME_LENGTH];

	int				mode;		//see MESH_* defines above

	int				flags;

	vec3_t			bmin;		//bounding box (in MESH coordinates)
	vec3_t			bmax;		//bounding box (in MESH coordinates)
	
	int				num_faces;
	uivec3_t		*facelist;

	int				num_verts;	//number of vertices
	
	vec2_t			*tverts;	//texture coords
	bvec4_t			*colors;	//vertex colors
	vec3_t			*normals;	//vertex normals
	vec3_t			*verts;		//vertex coords (always in MODEL space)
	vec3_t			*boneSpaceVerts;	//vertex coords (in BONE space, to optimize calculations)

	/*  
		mode == MESH_SKINNED_BLENDED

		The verts array will store coordinates in MODEL space
		(the world coords of the 3dsmax scene).  Verts are
		then blended via:

		for (v=0; v<mesh->num_verts; v++)
		{
			blendedVert_t *blend = &mesh->blended_verts[v];
			M_ClearVec3(outVerts[v]);
			for (link=0; link<mesh->blended_verts[v].num_links; link++)
			{
				vec3_t point;
				bone_t *bone = &model->bones[blend->links[link]];

				M_TransformPoint(mesh->verts[v], bone->invBase, point);		//get the point into the initial bone space
				M_TransformPoint(point, bone->current_transform, point);	//transform the point into the current bone space
				M_ScaleVec3(point, blend->weights[link], point);
				M_AddVec3(outVerts[v], point, outVerts[v]);
			}
		}

	*/
	byte			*linkPool;		//blendedLinks sets pointers here, rather than allocating its own memory
	blendedLink_t	*blendedLinks;	//vertBlend only allocated if mode == MESH_SKINNED_BLENDED

	/* 
		mode == MESH_SKINNED_NONBLENDED  (or LOD fallback for MESH_SKINNED_BLENDED)

		The verts array will store coordinates in BONE space, 
		so the calculation is simple:

		for (v=0; v<mesh->num_verts; v++)
		{
			M_TransformPoint(mesh->verts[v], bones[mesh->singleLinks[v]]->current_transform, outVerts[v]);
		}

		We could do this calculation in hardware if we split up the mesh according to link
	*/
	singleLink_t	*singleLinks;	//allocated for nonblended meshes.  can also be used as an LOD fallback for blended meshes.

	int				bonelink;		//if > -1, ALL vertices of this mesh are linked to this bone only, in which case both
									//singleLinks and blendedLinks will be NULL.


	bool			hasRenderData;	
	void			*renderData[MAX_RENDER_DATA];	//this may be used by the renderer to cache static geometry
	int				renderDataRefCount;				//refcount is used to make sure we don't reference old data
} mesh_t;


//mdlsprite_t is used for billboard type effects

#define			S2SPRITE_CORONA			0x0001
#define			S2SPRITE_BILLBOARD		0x0002
#define			S2SPRITE_GROUNDPLANE	0x0003


typedef struct
{
	struct model_s *parent;

	char		*name;
	
	int			type;		//see S2SPRITE_* defines in s2model.h

	float		width;
	float		height;

	int			bone;		//bone the sprite is linked to
} mdlsprite_t;


typedef enum
{
	BONETYPE_MESH = 1,
	BONETYPE_SPRITE,
	BONETYPE_REFERENCE
} boneTypes_enum;


typedef struct bone_s
{
	int				index;

#ifdef _S2_EXPORTER
		int				parentIndex;			//this is for exporter use only
#else
		struct bone_s	*parent;
#endif	

	int				numChildren;
	struct bone_s	*children[MAX_BONES];

	bool			type;						//see BONETYPE_* defines above
#ifdef _S2_EXPORTER
	matrix44_t		invBase;					//INVERSE of the base transform for the bone (to transform verts to bone space)
	matrix44_t		base;						//base transform for the bone (for static meshes)
#else
	matrix43_t		invBase;					//INVERSE of the base transform for the bone (to transform verts to bone space)
	matrix43_t		localBase;					//base transform for the bone (for static meshes)
	matrix43_t		base;
#endif
	char			name[BONE_NAME_LENGTH];

	bool			flippedParity;
} bone_t;

typedef struct
{
	float			*keys;				//will allocate num_keys keys
	int				num_keys;
} floatKeys_t;

typedef struct
{
	byte			*keys;
	int				num_keys;
} byteKeys_t;

typedef struct
{	
//	int				bone;				//index to the bone this motion belongs to
	char			boneName[BONE_NAME_LENGTH];
	
	//for now we just allocate num_frames keys if the track is animated at all,
	//otherwise we just allocate 1 key.

	//rotation keys (converted to a quaternion for interpolation)
	floatKeys_t		keys_pitch;
	floatKeys_t		keys_roll;
	floatKeys_t		keys_yaw;	
	
	//position keys
	floatKeys_t		keys_x;
	floatKeys_t		keys_y;
	floatKeys_t		keys_z;

	//scale keys
	floatKeys_t		keys_scale;

	//visibility keys
	byteKeys_t		keys_visibility;
} boneMotion_t;

typedef struct
{
	int				unique_id;
	int				frame;
	char			*command;			//e.g. "footstep"
} animEvent_t;

typedef struct
{
	struct media_s	*media;

	int				num_motions;		
	int				num_frames;
	
	boneMotion_t	*motions;			//will allocate num_motions motions
} modelClip_t;


typedef struct
{
	residx_t		clip;				//clip this animation references
	boneMotion_t	**motions;			//maps model bones to clip animation data
	char			name[ANIM_NAME_LENGTH];

	int				start;		//start frame
	int				numframes;
	int				loopbackframe;
	bool			looping;
	int				fps;
	int				numloopframes;

	//frame triggered events, like footstep sounds and particles
	int				numEvents;
	animEvent_t		*events;

} modelAnim_t;


typedef char shadername_t[SKIN_SHADERNAME_LENGTH];

typedef struct
{
	bool			loaded;

	char			name[SKIN_NAME_LENGTH];
	char			*baseDir;
	shadername_t	*shaderNames;			//stores shader filenames
	residx_t		*shaders;				//only gets allocated when the skin needs to be used
} modelSkin_t;



#define MAX_MODEL_CLIPS	128
#define MAX_MODEL_ANIMS	128
#define MAX_MODEL_LODS 3

typedef struct
{
	residx_t		modelidx;
	float			distance;
} modelLOD_t;

typedef struct model_s
{
	struct media_s	*media;

	char			*name;				//file name (FIXME..use header instead)

	modelLOD_t		lods[MAX_MODEL_LODS];
	int				num_lods;
	residx_t		base_lod;			//id of the base LOD.  if this is set, num_lods should be 0

	int				version;

	int				memtag;

	int				num_meshes;
	int				num_sprites;
	int				num_surfs;
	int				num_bones;
	int				num_anims;
	int				num_clips;
	int				num_skins;
	
	bone_t			*bones;				//will allocate num_bones bones
	int				*bone_mapping;		//aligns LOD bones with the base model bones
	modelSkin_t		skins[MAX_SKINS_PER_MODEL];
	mesh_t			*meshes;			//will allocate num_meshes meshes
	mdlsprite_t		*sprites;			//will allocate num_sprites sprites
	modelAnim_t		*anims;				//will allocate num_anims anims
	
//	int				*mapping;			//bone remapping

	int				groundplane;		//index into sprite array of groundplane

	convexPolyhedron_t *surfs;			//surfaces used for collision detection

	vec3_t			bmin;				//bounding box of the base model (frame 0)
	vec3_t			bmax;				//bounding box of the base model (frame 0)

	unsigned int 	buffer;				//buffer index used by the opengl driver
} model_t;



//////////////////////////////////////////
/*        Model File I/O structures     */
//////////////////////////////////////////

#pragma pack(1)		//make sure there's no member padding.  these structs must match up with the file read in

typedef struct
{
	int version;
	int num_meshes;
	int num_sprites;
	int num_surfs;
	int num_bones;
	vec3_t bmin;
	vec3_t bmax;
} modelHeader_t;

typedef struct
{
	int	 parentIndex;
	char name[BONE_NAME_LENGTH];
	matrix44_t invBase;				//invbase is stored as a 4x4 matrix in the model file
	matrix44_t base;				//base is stored as a 4x4 matrix in the model file
} boneBlock_t;

typedef struct
{
	int	mesh_index;
	char name[MESH_NAME_LENGTH];
	char defaultShader[SKIN_SHADERNAME_LENGTH];
	int mode;
	int num_verts;
	vec3_t bmin;
	vec3_t bmax;
	int bonelink;
} meshBlock_t;

typedef struct
{
	int mesh_index;
	int num_verts;
} blendedLinksBlock_t;

typedef struct
{
	int mesh_index;
	int num_verts;
} singleLinksBlock_t;

typedef struct
{
	int mesh_index;
	int num_faces;
} faceBlock_t;

typedef struct
{
	int mesh_index;
} textureCoordsBlock_t;

typedef struct
{
	int mesh_index;
} colorBlock_t;

typedef struct
{
	int mesh_index;
} normalBlock_t;

typedef struct
{
	int surf_index;
	int num_planes;
	vec3_t bmin;
	vec3_t bmax;
	int flags;
} surfBlock_t;

typedef struct
{
	int version;
	int num_bones;
	int num_frames;	
} clipHeader_t;

typedef enum
{
	MKEY_X,
	MKEY_Y,
	MKEY_Z,
	MKEY_PITCH,
	MKEY_ROLL,
	MKEY_YAW,
	MKEY_VISIBILITY,
	MKEY_SCALE,
	NUM_MKEY_TYPES
} keyTypes_enum;

typedef struct
{
	char		boneName[BONE_NAME_LENGTH];
	int			boneIndex;
	int			key_type;		//see MKEY_* defines above
	int			num_keys;
} keyBlock_t;

typedef struct
{
	int mesh_index;
} vertexBlock_t;


#pragma pack()

//////////////////////////////////////////
//////////////////////////////////////////



typedef struct
{
	vec3_t	v1;
	vec3_t	v2;
	vec3_t	v3;
} tri_t;

#define			MESHREF(_mesh, _framenum, _index) ((_mesh->num_verts * _framenum) + _index)

void	Geom_Init();
bool	Geom_LoadModel(const char *filename, model_t *model, int lod, residx_t baselod);
void	Geom_FreeModel(model_t *model);
void	Geom_ScaleModel(model_t *model, float scale);
void	Geom_BuildMesh(mesh_t *mesh, int loframe, int hiframe, float lerp_amt, float scale, vec4_t *vertarray, vec3_t *normalarray);
void	Geom_SetModelStatsMode(bool on);
void	Geom_CalcSpritePos(mdlsprite_t *sprite, float frame);
int		Geom_GetNumKeyframes(residx_t model);
void	Geom_DecompressMeshVertex(mesh_t *mesh, int frame, unsigned int index, vec3_t out);
bool	Geom_LoadClip(const char *filename, modelClip_t *clip);
void	Geom_LoadSkinShaders(model_t *model, int skinidx);
void	Geom_InitSkeleton(skeleton_t *skel, residx_t model);
void	Geom_BeginPose(skeleton_t *skeleton, residx_t refmodel);
//void	Geom_MapSkeleton(const skeleton_t *skel, residx_t model, skeleton_t *out);
int		Geom_SetBoneAnim(const char *parentBone, const char *animName, float animTime, int currentTime, int blendTime, int eventChannel);
void	Geom_RotateBone(skeleton_t *skeleton, const char *boneName, float pitch_offset, float roll_offset, float yaw_offset, bool multiply);
void	Geom_EndPose();
int		Geom_GetBoneIndex(skeleton_t *skeleton, const char *boneName);
void	Geom_SetBoneState(const char *boneName, int posestate);
void	Geom_FreeSkeleton(skeleton_t *skeleton);
void	Geom_SetBoneAnimNoPose(const char *animName, float animTime, int currentTime, int eventChannel);
void    Geom_ReloadAnim(residx_t modelidx);
