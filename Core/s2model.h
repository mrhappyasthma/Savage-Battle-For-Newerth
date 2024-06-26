// (C) 2003 S2 Games

// .model file format


#define	MESH_LIMIT	32
#define	BBOX_LIMIT	8
#define	SPRITE_LIMIT 32
#define SURF_LIMIT 32

typedef struct
{
	char	id[8];					//always "S2_MODEL"
	int		version;				//currently 3
	char	name[64];				//name of the model (ex: "h_barbarian", "tree")
	int		num_meshes;				//number of meshes in file
	int		num_keyframes;			//number of keyframes in model
	char	author[64];
	byte	vertex_precision;
	int		kps;
	int		normals;				//model contains normal info
	int		num_bboxes;				//DEFUNCT
	vec3_t	bmins[8];				//DEFUNCT
	vec3_t	bmaxs[8];				//DEFUNCT
	int		num_sprites;			//number of sprite effects
	int		revision;				//specifies revision number (version number changes only if the file structure changes)
	int		num_surfs;				//number of collision meshes
	char	reserved[808];
} s2model_header_t;

typedef struct
{
	char	name[64];				//name of the mesh
	int		mesh_end;				//end pos of mesh data
	int		num_faces;				//number of faces	
	int		num_verts;				//number of vertices		
	char	shadername[256];		//shader (texture) name;
	int		face_start;				//start of face data in file
	int		frameinfo_start;		//start of frame info in file
	int		vtx_start;				//start of vertex data in file	
} s2model_mesh_header_t;

typedef struct
{
	int		numPlanes;

	vec3_t	bmin;		//bounding box of the polyhedra
	vec3_t	bmax;
} s2model_surface_header_t;

typedef struct
{
	plane_t	plane;
	int		flags;
} s2surfPlane_t;

typedef struct
{
	vec3_t	bmin;					//geometry bounds
	vec3_t	bmax;
} s2model_frameinfo_t;

#define			S2SPRITE_CORONA			0x0001
#define			S2SPRITE_BILLBOARD		0x0002
#define			S2SPRITE_GROUNDPLANE	0x0003

typedef struct
{
	char	name[64];				//name of sprite
	int		type;					//type of sprite effect
	char	shadername[256];
	float	width;
	float	height;
	char	reserved[244];
} s2model_sprite_header_t;

typedef struct
{
	vec3_t	pos;
	char	reserved[244];
} s2model_sprite_frameinfo_t;