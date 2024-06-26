// (C) 2003 S2 Games

// geom.c

// .model loading, .clip loading. .anim loading functions
// bone deformation and vertex blending functions

//#define	NV_STRIPIFY

#include "core.h"

#ifdef NV_STRIPIFY
//#include "/nvtristrip/nvtristrip.h"
#endif



#define MODEL_CALC_BOUNDS
//#define MODEL_CALC_NORMALS

cvar_t					geom_defaultBoxScale = { "geom_defaultBoxScale", "1", CVAR_CHEAT };
cvar_t					geom_skelhack = { "geom_skelhack", "1", CVAR_CHEAT };
cvar_t					geom_simpleSurfaces = { "geom_simpleSurfaces", "0", CVAR_CHEAT };
cvar_t					geom_modelScale = { "geom_modelScale", "0.4", CVAR_CHEAT };
cvar_t					geom_skipSkel = { "geom_skipSkel", "0" };
cvar_t					geom_skipPose = { "geom_skipPose", "0" };
cvar_t					geom_skipEvents = { "geom_skipEvents", "0" };
cvar_t					geom_blendAnims = { "geom_blendAnim", "1" };
cvar_t					geom_blendFrames = { "geom_blendFrames", "1" };
cvar_t					geom_fatalError = { "geom_fatalError", "0" };
cvar_t					geom_debug = { "geom_debug", "0" };


//module level vars
static char				geom_error[256];
static model_t			*outModel;
static modelClip_t		*outClip;
static model_t			*animatedModel = NULL;
static model_t			*skinModel = NULL;
static char				*skinDir = "";
static char				skinName[SKIN_NAME_LENGTH] = "default";

static matrix43_t		yaw180;



void	Geom_Error(const char *error)
{
	strncpySafe(geom_error, error, sizeof(geom_error));

	if (geom_fatalError.integer)
		Game_Error(geom_error);
	else
		if (geom_debug.integer)
			Console_Printf("%s\n", error);
}


/****************************************************

  MODEL LOADING

 ****************************************************/


void	Geom_FixModelCoord(vec3_t coord)
{
	M_TransformPoint(coord, zero_vec, (const vec3_t *)yaw180.axis, coord);
	M_MultVec3(coord, geom_modelScale.value, coord);
} 



static matrix43_t flipmat =
{
	{
		{ -1,0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 },
	},
	{ 0, 0, 0 }
};

void	Geom_FixModelMatrix(matrix43_t *mat)
{
	matrix43_t tmp = *mat;

	M_MultiplyMatrix(&tmp, &yaw180, mat);
	M_MultVec3(mat->pos, geom_modelScale.value, mat->pos);

}


bool	Geom_ParseHeader(block_t *block)
{
	int n;
	int ver;
	modelHeader_t *in = (modelHeader_t *)block->data;

	if (block->length < sizeof(modelHeader_t))
	{
		Geom_Error("Bad header");
		return false;
	}

	ver = LittleInt(in->version);
	if (ver != 1 && ver != 2)
	{
		Geom_Error("Bad version");
		return false;
	}

	outModel->version = ver;
	
	outModel->num_meshes = LittleInt(in->num_meshes);
	if (outModel->num_meshes > 50)
		Console_DPrintf("HUGE num_meshes (%i)\n", outModel->num_meshes);
	if (outModel->num_meshes)
		outModel->meshes = Tag_Malloc(outModel->num_meshes * sizeof(mesh_t), MEM_MODEL);
	outModel->num_sprites = LittleInt(in->num_sprites);
	if (outModel->num_sprites)
		outModel->sprites = Tag_Malloc(outModel->num_sprites * sizeof(mdlsprite_t), MEM_MODEL);
			
	outModel->num_bones = LittleInt(in->num_bones);
	if (outModel->num_bones)
		outModel->bones = Tag_Malloc(outModel->num_bones * sizeof(bone_t), MEM_MODEL);
	for (n=0; n<3; n++)
	{
		outModel->bmin[n] = LittleFloat(in->bmin[n]);
		outModel->bmax[n] = LittleFloat(in->bmax[n]);
	}

	Geom_FixModelCoord(outModel->bmin);
	Geom_FixModelCoord(outModel->bmax);

	//allocate memory for collision surfaces, plus one extra surface which is used for storing the model bounds
	outModel->num_surfs = LittleInt(in->num_surfs);

	if (!outModel->num_surfs)
	{
		outModel->num_surfs = 2;		//one for the default surface, another for the model bounds		
	}
	else
	{
		outModel->num_surfs++;
	}

	if (geom_simpleSurfaces.integer)
		outModel->num_surfs = 2;

	outModel->surfs = Tag_Malloc((outModel->num_surfs) * sizeof(convexPolyhedron_t), MEM_MODEL);

	return true;
}


bool	Geom_ParseBoneBlock(block_t *block)
{
	vec3_t cross;
	int n,i;
	boneBlock_t *in = (boneBlock_t *)block->data;	

	if (block->length < sizeof(boneBlock_t) * outModel->num_bones)
	{
		Geom_Error("Bad bone block");
		return false;
	}

	for (n=0; n<outModel->num_bones; n++)
	{
		int parentIndex = LittleInt(in->parentIndex);
		bone_t *bone = &outModel->bones[n];

		bone->index = n;

		if (parentIndex == -1)
			bone->parent = NULL;
		else if (parentIndex >= 0 && parentIndex < outModel->num_bones)
		{
			bone_t *parent;
			parent = bone->parent = &outModel->bones[parentIndex];
			
			if (parent->numChildren >= MAX_BONES)
			{
				Geom_Error(fmt("Too many children for bone %i", n));
				return false;
			}
			
			parent->children[parent->numChildren] = &outModel->bones[n];
			parent->numChildren++;
		}
		else
		{
			Geom_Error(fmt("Invalid parent index for bone %i", n));
			return false;
		}
		strncpySafe(bone->name, in->name, BONE_NAME_LENGTH);

		for (i=0; i<3; i++)
		{
			bone->invBase.pos[i] = LittleFloat(in->invBase.t.pos[i]);
			bone->invBase.axis[0][i] = LittleFloat(in->invBase.t.axis[0][i]);
			bone->invBase.axis[1][i] = LittleFloat(in->invBase.t.axis[1][i]);
			bone->invBase.axis[2][i] = LittleFloat(in->invBase.t.axis[2][i]);

		}

		Geom_FixModelMatrix(&bone->invBase);
		
		for (i=0; i<3; i++)
		{
			bone->localBase.pos[i] = LittleFloat(in->base.t.pos[i]);
			bone->localBase.axis[0][i] = LittleFloat(in->base.t.axis[0][i]);
			bone->localBase.axis[1][i] = LittleFloat(in->base.t.axis[1][i]);
			bone->localBase.axis[2][i] = LittleFloat(in->base.t.axis[2][i]);						
		}		

		Geom_FixModelMatrix(&bone->localBase);

		//sometimes matrices are given to us in the wrong 
		//coordinate system, so we correct for it here
		M_CrossProduct(bone->localBase.axis[0], bone->localBase.axis[1], cross);
		if (M_DotProduct(cross, bone->localBase.axis[2]) < 0)
		{
			/*
			matrix43_t tmp;		
					
			M_MultiplyMatrix(mat, &flipmat, &tmp);
			*mat = tmp;	*/
		
			bone->flippedParity = true;
		}
		else
		{
			bone->flippedParity = false;
		}
		
		in++;
	}
	
	return true;
}


bool	Geom_ParseMeshBlock(block_t *block)
{
	meshBlock_t *in = (meshBlock_t *)block->data;
	int idx;
	int n;

	if (block->length < sizeof(meshBlock_t))
	{
		Geom_Error("Bad mesh block");
		return false;
	}

	idx = LittleInt(in->mesh_index);
	if (idx >= outModel->num_meshes || idx < 0)
	{
		Geom_Error("Bad mesh index");
		return false;
	}

	strncpySafe(outModel->meshes[idx].name, in->name, MESH_NAME_LENGTH);

	//
	outModel->meshes[idx].flags = 0;
	if (strncmp(outModel->meshes[idx].name, "_foliage", 8) == 0)
		outModel->meshes[idx].flags |= SURF_FOLIAGE;
	if (strncmp(outModel->meshes[idx].name, "_nohit", 6) == 0)
		outModel->meshes[idx].flags |= SURF_NOT_SOLID;
	if (strncmp(outModel->meshes[idx].name, "_ex0", 4) == 0)
		outModel->meshes[idx].flags |= SURF_EX0;
	if (strncmp(outModel->meshes[idx].name, "_ex1", 4) == 0)
		outModel->meshes[idx].flags |= SURF_EX1;
	if (strncmp(outModel->meshes[idx].name, "_ex2", 4) == 0)
		outModel->meshes[idx].flags |= SURF_EX2;
	if (strncmp(outModel->meshes[idx].name, "_ex3", 4) == 0)
		outModel->meshes[idx].flags |= SURF_EX3;
	if (strncmp(outModel->meshes[idx].name, "_ex4", 4) == 0)
		outModel->meshes[idx].flags |= SURF_EX4;
	if (strncmp(outModel->meshes[idx].name, "_ex5", 4) == 0)
		outModel->meshes[idx].flags |= SURF_EX5;
	if (strncmp(outModel->meshes[idx].name, "_ex6", 4) == 0)
		outModel->meshes[idx].flags |= SURF_EX6;
	if (strncmp(outModel->meshes[idx].name, "_ex7", 4) == 0)
		outModel->meshes[idx].flags |= SURF_EX7;
	//

	strncpySafe(outModel->meshes[idx].defaultShader, in->defaultShader, SKIN_SHADERNAME_LENGTH);
	outModel->meshes[idx].mode = LittleInt(in->mode);
	outModel->meshes[idx].num_verts = LittleInt(in->num_verts);	

	for (n=0; n<3; n++)
	{
		outModel->meshes[idx].bmin[n] = LittleFloat(in->bmin[n]);// * MODEL_SCALE_FACTOR;
		outModel->meshes[idx].bmax[n] = LittleFloat(in->bmax[n]);// * MODEL_SCALE_FACTOR;
	}

	Geom_FixModelCoord(outModel->meshes[idx].bmin);
	Geom_FixModelCoord(outModel->meshes[idx].bmax);

	outModel->meshes[idx].bonelink = LittleInt(in->bonelink);

	outModel->meshes[idx].model = outModel;

	return true;
}



bool	Geom_ParseBlendedLinksBlock(block_t *block)
{
	int idx, n, w;
	int pos = sizeof(blendedLinksBlock_t);
	mesh_t *mesh;
	byte *pool;
	int poolsize, poolpos;
	blendedLinksBlock_t *in = (blendedLinksBlock_t *)block->data;

	idx = LittleInt(in->mesh_index);
	if (idx >= outModel->num_meshes || idx < 0)
	{
		Geom_Error("Bad mesh index in blended link block");
		return false;
	}	

	mesh = &outModel->meshes[idx];	

	if (mesh->mode != MESH_SKINNED_BLENDED)
	{
		Geom_Error("Invalid blended link block");
		return false;
	}

	if (LittleInt(in->num_verts) != mesh->num_verts)
	{
		Geom_Error("Invalid blended link block (num_verts didn't match)");
		return false;
	}

	//alloc a pool we can use for keeping the weight data so memory doesn't get fragmented into little pieces
	poolsize = block->length - (4 * mesh->num_verts) - sizeof(blendedLinksBlock_t);
	mesh->linkPool = pool = Tag_Malloc(poolsize, MEM_MODEL);  //allocate enough memory to store weights and indexes
	poolpos = 0;

	mesh->blendedLinks = Tag_Malloc(sizeof(blendedLink_t) * mesh->num_verts, MEM_MODEL);
	//also allocated the single links block, as we can use this as an LOD fallback
	mesh->singleLinks = Tag_Malloc(sizeof(singleLink_t) * mesh->num_verts, MEM_MODEL);

	for (n=0; n<mesh->num_verts; n++)
	{
		float biggestInfluence = 0;
		int biggestIndex = 0;
		float weight;
		int num_weights;
		float weightsum = 0;

		num_weights = mesh->blendedLinks[n].num_weights = LittleInt(*(int *)&block->data[pos]);
		pos += 4;
		mesh->blendedLinks[n].weights = (float *)(&pool[poolpos]);
		for (w=0; w<num_weights; w++)
		{				
			weight = mesh->blendedLinks[n].weights[w] = LittleFloat(*(float *)&block->data[pos]);
			weightsum += weight;
			if (weight > biggestInfluence)
			{
				biggestInfluence = weight;
				biggestIndex = w;
			}			
			poolpos += 4;
			pos += 4;
		}
		if (!(weightsum < 1.05 && weightsum > 0.95))
		{
			Geom_Error(fmt("weightsum != 1.0 for %s (was %f)\n", outModel->name, weightsum));
			return false;
		}

		mesh->blendedLinks[n].indexes = (int *)(&pool[poolpos]);		
		for (w=0; w<num_weights; w++)
		{
			mesh->blendedLinks[n].indexes[w] = LittleInt(*(int *)&block->data[pos]);
			poolpos += 4;
			pos += 4;
		}

		mesh->singleLinks[n] = mesh->blendedLinks[n].indexes[biggestIndex];

		if (poolpos > poolsize)
		{
			System_Error("Geom_LoadModel(%s): overflowed link pool\n", outModel->name);
		}
	}

	if (poolpos != poolsize)
	{
		System_Error("Geom_LoadModel(%s): overflowed link pool\n", outModel->name);
	}

	return true;
}

bool	Geom_ParseSingleLinksBlock(block_t *block)
{
	int idx, n;
	mesh_t *mesh;
	singleLinksBlock_t *in = (singleLinksBlock_t *)block->data;
	singleLink_t *links;

	idx = LittleInt(in->mesh_index);
	if (idx >= outModel->num_meshes || idx < 0)
	{
		Geom_Error("Bad mesh index in single link block");
		return false;
	}	

	mesh = &outModel->meshes[idx];	

	if (mesh->mode != MESH_SKINNED_NONBLENDED)
	{
		Geom_Error("Invalid single links block");
		return false;
	}

	if (LittleInt(in->num_verts) != mesh->num_verts)
	{
		Geom_Error("Invalid single links block (num_verts didn't match)");
		return false;
	}
	
	links = (singleLink_t *)(in + 1);
	mesh->singleLinks = Tag_Malloc(sizeof(int) * mesh->num_verts, MEM_MODEL);	
	for (n=0; n<mesh->num_verts; n++)
	{
		mesh->singleLinks[n] = LittleInt(links[n]);		
	}

	return true;
}

bool	Geom_ParseTextureCoordBlock(block_t *block)
{
	int idx, n;
	mesh_t *mesh;
	textureCoordsBlock_t *in = (textureCoordsBlock_t *)block->data;
	vec2_t *tverts;

	idx = LittleInt(in->mesh_index);
	if (idx >= outModel->num_meshes || idx < 0)
	{
		Geom_Error("Bad mesh index in single link block");
		return false;
	}	

	mesh = &outModel->meshes[idx];

	tverts = (vec2_t *)(in + 1);
	mesh->tverts = Tag_Malloc(sizeof(vec2_t) * mesh->num_verts, MEM_MODEL);

	for (n=0; n<mesh->num_verts; n++)
	{
		mesh->tverts[n][0] = LittleFloat(tverts[n][0]);
		mesh->tverts[n][1] = LittleFloat(tverts[n][1]);		
	}

	return true;
}

bool	Geom_ParseVertexBlock(block_t *block)
{
	int idx, n;
	mesh_t *mesh;
	vertexBlock_t *in = (vertexBlock_t *)block->data;
	vec3_t *verts;

	idx = LittleInt(in->mesh_index);
	if (idx >= outModel->num_meshes || idx < 0)
	{
		Geom_Error("Bad mesh index in single link block");
		return false;
	}

	mesh = &outModel->meshes[idx];

	verts = (vec3_t *)(in + 1);
	mesh->verts = Tag_Malloc(sizeof(vec3_t) * mesh->num_verts, MEM_MODEL);

	for (n=0; n<mesh->num_verts; n++)
	{
		int i;

		for (i=0; i<3; i++)
		{
  			mesh->verts[n][i] = LittleFloat(verts[n][i]);
		}
		
		Geom_FixModelCoord(mesh->verts[n]);
	}

	return true;
}

bool	Geom_ParseColorBlock(block_t *block)
{
	bool ignore = true;
	int idx, n;
	mesh_t *mesh;
	colorBlock_t *in = (colorBlock_t *)block->data;
	bvec4_t *colors;

	idx = LittleInt(in->mesh_index);
	if (idx >= outModel->num_meshes || idx < 0)
	{
		Geom_Error("Bad mesh index in single link block");
		return false;
	}	

	mesh = &outModel->meshes[idx];

	colors = (bvec4_t *)(in + 1);
	mesh->colors = Tag_Malloc(sizeof(bvec4_t) * mesh->num_verts, MEM_MODEL);

	for (n=0; n<mesh->num_verts; n++)
	{
		int i;
		//don't need to do any endian conversion since these are byte arrays
		for (i=0; i<4; i++)
		{
			mesh->colors[n][i] = colors[n][i];
			if (colors[n][i] != 255)
				ignore = false;
		}
			
	}

	if (ignore)
	{
		//hack to ignore vertex colors if they're all white
		Tag_Free(mesh->colors);
		mesh->colors = NULL;
	}

	return true;
}

bool	Geom_ParseNormalBlock(block_t *block)
{
	int idx, n;
	mesh_t *mesh;
	normalBlock_t *in = (normalBlock_t *)block->data;
	vec3_t *normals;

	idx = LittleInt(in->mesh_index);
	if (idx >= outModel->num_meshes || idx < 0)
	{
		Geom_Error("Bad mesh index in single link block");
		return false;
	}	

	mesh = &outModel->meshes[idx];

	normals = (vec3_t *)(in + 1);
	mesh->normals = Tag_Malloc(sizeof(vec3_t) * mesh->num_verts, MEM_MODEL);

	if (strnicmp(mesh->name, "_foliage", 8)==0)
	{
		for (n=0; n<mesh->num_verts; n++)
		{
			//hack for nice lighting on trees
			mesh->normals[n][0] = 0;
			mesh->normals[n][1] = 0;
			mesh->normals[n][2] = 1;
		}
	}
	else
	{
		for (n=0; n<mesh->num_verts; n++)
		{
			mesh->normals[n][0] = LittleFloat(normals[n][0]);
			mesh->normals[n][1] = LittleFloat(normals[n][1]);
			mesh->normals[n][2] = LittleFloat(normals[n][2]);

			//version 1 normals are fixed up during Geom_FinishMesh
			if (outModel->version != 1)
				Geom_FixModelCoord(mesh->normals[n]);
		}
	}

	return true;
}

bool	Geom_ParseSurfBlock(block_t *block)
{
	int idx;
	int n;
	convexPolyhedron_t *surf;
	surfBlock_t *in = (surfBlock_t *)block->data;
	plane_t *planes;

	if (geom_simpleSurfaces.integer)
		return true;

	idx = LittleInt(in->surf_index);
	if (idx >= outModel->num_surfs || idx < 0)
	{
		Geom_Error("Bad surface index in surface block");
		return false;
	}

	surf = &outModel->surfs[idx];

	surf->numPlanes = LittleInt(in->num_planes);
	if (surf->numPlanes >= MAX_POLYHEDRON_PLANES || surf->numPlanes < 0)
	{
		Geom_Error("Bad numPlanes in surface block");
		return false;
	}
	//surf->planes = Tag_Malloc(sizeof(plane_t) * surf->numPlanes, MEM_MODEL);

	for (n=0; n<3; n++)
	{
		surf->bmin[n] = LittleFloat(in->bmin[n]);
		surf->bmax[n] = LittleFloat(in->bmax[n]);
	}

	planes = (plane_t *)(in + 1);

	for (n=0; n<surf->numPlanes; n++)
	{
		surf->planes[n].normal[0] = LittleFloat(planes[n].normal[0]);
		surf->planes[n].normal[1] = LittleFloat(planes[n].normal[1]);
		surf->planes[n].normal[2] = LittleFloat(planes[n].normal[2]);
		surf->planes[n].dist = LittleFloat(planes[n].dist);
	}

	return true;
}

bool	Geom_ParseSpriteBlock(block_t *block)
{
	return false;
}

bool	Geom_ParseFaceBlock(block_t *block)
{
	int n;
	int idx;
	faceBlock_t *in = (faceBlock_t *)block->data;
	mesh_t *mesh;
	uivec3_t *facelist;

	idx = LittleInt(in->mesh_index);
	if (idx >= outModel->num_meshes || idx < 0)
	{
		Geom_Error("Bad mesh index in face block");
		return false;
	}

	mesh = &outModel->meshes[idx];

	mesh->num_faces = LittleInt(in->num_faces);
	mesh->facelist = Tag_Malloc(mesh->num_faces * sizeof(uivec3_t), MEM_MODEL);

	facelist = (uivec3_t *)(in + 1);

	for (n=0; n<mesh->num_faces; n++)
	{
		mesh->facelist[n][0] = LittleInt(facelist[n][0]);
		mesh->facelist[n][1] = LittleInt(facelist[n][1]);
		mesh->facelist[n][2] = LittleInt(facelist[n][2]);
	}

	return true;
}

bool	Geom_ReadBlocks(blockList_t *blocklist)
{
	int b;

	if (strcmp(blocklist->blocks[0].name, "head")!=0)
	{
		Geom_Error("No header");
		return false;
	}
	Geom_ParseHeader(&blocklist->blocks[0]);

	for (b=1; b<blocklist->num_blocks; b++)
	{
		block_t *block = &blocklist->blocks[b];
		if (strcmp(block->name, "bone")==0)
		{
			if (!Geom_ParseBoneBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "mesh")==0)
		{
			if (!Geom_ParseMeshBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "vrts")==0)
		{
			if (!Geom_ParseVertexBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "lnk1")==0)
		{
			if (!Geom_ParseBlendedLinksBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "lnk2")==0)
		{
			if (!Geom_ParseSingleLinksBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "texc")==0)
		{
			if (!Geom_ParseTextureCoordBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "colr")==0)
		{
			if (!Geom_ParseColorBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "nrml")==0)
		{
			if (!Geom_ParseNormalBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "face")==0)
		{
			if (!Geom_ParseFaceBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "surf")==0)
		{
			if (!Geom_ParseSurfBlock(&blocklist->blocks[b]))
				return false;
		}
		else if (strcmp(block->name, "sprt")==0)
		{
			if (!Geom_ParseSpriteBlock(&blocklist->blocks[b]))
				return false;
		}
	}

	return true;
}

void	Geom_ComputeModelBounds()
{
	int n,v;

	M_ClearBounds(outModel->bmin, outModel->bmax);

	for (n=0; n<outModel->num_meshes; n++)
	{
		mesh_t *mesh = &outModel->meshes[n];

		M_ClearBounds(mesh->bmin, mesh->bmax);
		for (v=0; v<mesh->num_verts; v++)
		{
			M_AddPointToBounds(mesh->verts[v], mesh->bmin, mesh->bmax);
		}

		M_AddPointToBounds(mesh->bmin, outModel->bmin, outModel->bmax);
		M_AddPointToBounds(mesh->bmax, outModel->bmin, outModel->bmax);
	}	
}

void	Geom_CreateBoundingSurface()
{
 	//create a default box shaped surface
 	vec3_t bmin = { outModel->bmin[0], outModel->bmin[1], outModel->bmin[2] };
 	vec3_t bmax = { outModel->bmax[0], outModel->bmax[1], outModel->bmax[2] };

 	plane_t boxplanes[6] = 
 	{
 		{ {-1, 0, 0 }, 0 },
 		{ { 1, 0, 0 }, 0 },
 		{ { 0,-1, 0 }, 0 },
 		{ { 0, 1, 0 }, 0 },
 		{ { 0, 0,-1 }, 0 },
 		{ { 0, 0, 1 }, 0 }
 	}; 	

 	boxplanes[0].dist = -bmin[0];
 	boxplanes[1].dist = bmax[0];
 	boxplanes[2].dist = -bmin[1];
 	boxplanes[3].dist = bmax[1];
 	boxplanes[4].dist = -bmin[2];
 	boxplanes[5].dist = bmax[2];

 	Mem_Copy(outModel->surfs[outModel->num_surfs-1].planes, boxplanes, sizeof(boxplanes));
 	outModel->surfs[outModel->num_surfs-1].numPlanes = 6;
 	outModel->surfs[outModel->num_surfs-1].flags = SURF_MODELBOUNDS;
 	M_CopyVec3(bmin, outModel->surfs[outModel->num_surfs-1].bmin);
 	M_CopyVec3(bmax, outModel->surfs[outModel->num_surfs-1].bmax);
}

void	Geom_CreateDefaultCollisionSurface()
{
 	//create a default box shaped surface
 	vec3_t bmin = { outModel->bmin[0] * geom_defaultBoxScale.value, outModel->bmin[1] * geom_defaultBoxScale.value, outModel->bmin[2] };
 	vec3_t bmax = { outModel->bmax[0] * geom_defaultBoxScale.value, outModel->bmax[1] * geom_defaultBoxScale.value, outModel->bmax[2] };

 	plane_t boxplanes[6] = 
 	{
 		{ {-1, 0, 0 }, 0 },
 		{ { 1, 0, 0 }, 0 },
 		{ { 0,-1, 0 }, 0 },
 		{ { 0, 1, 0 }, 0 },
 		{ { 0, 0,-1 }, 0 },
 		{ { 0, 0, 1 }, 0 }
 	}; 	

 	boxplanes[0].dist = -bmin[0];
 	boxplanes[1].dist = bmax[0];
 	boxplanes[2].dist = -bmin[1];
 	boxplanes[3].dist = bmax[1];
 	boxplanes[4].dist = -bmin[2];
 	boxplanes[5].dist = bmax[2];

 	Mem_Copy(outModel->surfs[0].planes, boxplanes, sizeof(boxplanes));
 	outModel->surfs[0].numPlanes = 6;
 	outModel->surfs[0].flags = 0;
 	M_CopyVec3(bmin, outModel->surfs[0].bmin);
 	M_CopyVec3(bmax, outModel->surfs[0].bmax);
}


#ifdef NV_STRIPIFY
bool	Stripify(model_t *model);
#endif

bool	Geom_FinishModel()
{
	int n, v, i;

	Geom_ComputeModelBounds();

	Geom_CreateBoundingSurface();			//used for per-poly hit detection

	if (geom_simpleSurfaces.integer)
		Geom_CreateDefaultCollisionSurface();	//used for character movement collision


	/*

		Create bone space vertices and fix up normals

    */

	for (n=0; n<outModel->num_meshes; n++)
	{
		mesh_t *mesh = &outModel->meshes[n];

		mesh->boneSpaceVerts = Tag_Malloc(mesh->num_verts * sizeof(vec3_t), MEM_MODEL);

		if (mesh->bonelink > -1)
		{		
			matrix43_t *matrix = &outModel->bones[mesh->bonelink].invBase;
			matrix43_t *basemat = &outModel->bones[mesh->bonelink].localBase;

			//all verts are linked to the same bone
			for (v=0; v<mesh->num_verts; v++)
			{				
				M_TransformPoint(mesh->verts[v], matrix->pos, (const vec3_t *)matrix->axis, mesh->boneSpaceVerts[v]);				
				if (outModel->version == 1)
				{					
					//version 1 models had bad normals on export.  this code corrects it.
					M_TransformPoint(mesh->normals[v], zero_vec, basemat->axis, mesh->normals[v]);
				}
			}
		}
		else
		{
			for (v=0; v<mesh->num_verts; v++)
			{
				matrix43_t *matrix = &outModel->bones[mesh->singleLinks[v]].invBase;
				M_TransformPoint(mesh->verts[v], matrix->pos, (const vec3_t *)matrix->axis, mesh->boneSpaceVerts[v]);
			}
		}
	}

	for (n=0; n<outModel->num_bones; n++)
	{
		matrix43_t mat;
		bone_t *bone = &outModel->bones[n];

		mat = bone->localBase;

		while (bone->parent)
		{
			matrix43_t tmp;
			M_MultiplyMatrix(&bone->parent->localBase, &mat, &tmp);
			mat = tmp;

			bone = bone->parent;
		}

		bone->base = mat;
	}

	if (outModel->base_lod)
	{		
		//this model is an LOD for another model.
		//skeletal posing is always done on the base model,
		//so we need a way to map our bones to the base
		//model's bones.

		int i,j;
		model_t *baseModel = Res_GetModel(outModel->base_lod);

		outModel->bone_mapping = Tag_Malloc(sizeof(int) * outModel->num_bones, MEM_MODEL);

		for (i=0; i<outModel->num_bones; i++)
		{
			outModel->bone_mapping[i] = -1;

			for (j=0; j<baseModel->num_bones; j++)
			{
				if (stricmp(outModel->bones[i].name, baseModel->bones[j].name)==0)
				{
					outModel->bone_mapping[i] = j;
					break;
				}					
			}
/*
			if (outModel->bone_mapping[i] == -1)
			{
				if (!geom_allowMismatchedLODs.integer)
				{					
					return false;
				}
			}
*/
		}
	}
	else
	{
		outModel->bone_mapping = Tag_Malloc(sizeof(int) * outModel->num_bones, MEM_MODEL);

		for (i=0; i<outModel->num_bones; i++)
		{
			outModel->bone_mapping[i] = i;
		}
	}

	/*

		Rotation and scale hack:
		
		Rotate all collision surfaces by 180 degrees around the Z and scale by
		MODEL_SCALE_FACTOR so they match up with the rendered model.
		(this also happens to the model itself during load)

	*/

	for (n=0; n<outModel->num_surfs; n++)
	{
		vec3_t axis[3];
		convexPolyhedron_t *surf = &outModel->surfs[n];
		convexPolyhedron_t tempsurf;

		if ( !(surf->flags & SURF_MODELBOUNDS) )		//modelbounds surface was generated from transformed/scaled verts, so ignore
		{
			WT_ScalePolyhedron(surf, geom_modelScale.value, &tempsurf);
			M_GetAxis(0, 0, 180, axis);
			WT_TransformPolyhedron(&tempsurf, zero_vec, (const vec3_t *)axis, surf);
		}
	}

#ifdef NV_STRIPIFY
	if (!Stripify(outModel))
	{
		Game_Error("strip failed\n");
	}
#endif

	return true;
}




bool	Geom_LoadModel(const char *filename, model_t *model, int lod, residx_t baselod)
{
	byte *buf;
	int buflen;
	bool ret = false;
	blockList_t *blocklist;
	char basefile[1024];

	buf = File_LoadIntoBuffer(filename, &buflen, 0);
	if (!buf && !lod)
	{
		//only set an error when loading a base model, never on LODs, since they're optional
		Geom_Error(fmt("%s: Invalid model file or file not found", filename));
		return false;
	}

	//check header
	if (buflen < 4)
	{
		File_FreeBuffer(buf);
		return false;
	}
	if (buf[0] != 'S' || buf[1] != 'M' || buf[2] != 'D' || buf[3] != 'L')
	{
		File_FreeBuffer(buf);
		return false;
	}

	memset(model, 0, sizeof(model_t));

	outModel = model;
	
 	blocklist = File_AllocBlockList(buf+4, buflen-4, filename);
	if (blocklist)
	{
		outModel->name = Tag_Strdup(filename, MEM_MODEL);
		ret = Geom_ReadBlocks(blocklist);
		File_FreeBlockList(blocklist);
	}

	File_FreeBuffer(buf);

	if (ret)
	{		
		int n;
		//bool makeDefault = true;	//generate a default skin
		modelSkin_t *skin;

		if (lod)
			model->base_lod = baselod;

		ret = Geom_FinishModel();

		if (ret)
		{			
			if (!model->num_surfs)
			{
				//create a default collision surface
				
			}
			
			if (dedicated_server.integer)
				return true;				//dedicated doesn't require animations or skins

			Filename_StripExtension(filename, basefile);
			if (lod)
				basefile[strlen(basefile)-2] = 0;
			
			//generate a default skin
					
			skin = &model->skins[0];

			strcpy(skin->name, "default");
			skin->shaderNames = Tag_Malloc(sizeof(shadername_t) * model->num_meshes, MEM_MODEL);
			skin->shaders = Tag_Malloc(sizeof(residx_t) * model->num_meshes, MEM_MODEL);
			skin->loaded = false;
			skin->baseDir = Tag_Strdup(Filename_GetDir(filename), MEM_MODEL);
			for (n=0; n<model->num_meshes; n++)
			{
				strncpySafe(skin->shaderNames[n], Filename_GetFilename(model->meshes[n].defaultShader), SKIN_SHADERNAME_LENGTH);
			}
			model->num_skins = 1;
			
			//load the skin file if it exists
			if (File_Exists(fmt("%s.skin", basefile)))
			{
				skinModel = model;
				skinDir = skin->baseDir;
				strcpy(skinName, "default");
				Cmd_ReadConfigFile(fmt("%s.skin", basefile), true);			
				skinModel = NULL;
				skinDir = "";
			}

			//create an anim out of the default clip if it exists
			if (File_Exists(fmt("%s.clip", basefile)))
			{
				animatedModel = model;
				Cmd_Exec(fmt("makeanim DEFAULT %s.clip", basefile));
				animatedModel = NULL;
			}
			
			//load the anim file if it exists
			if (File_Exists(fmt("%s.anim", basefile)))
			{				
				animatedModel = model;
				Cmd_ReadConfigFile(fmt("%s.anim", basefile), true);				
				//load the anim fixups file
				Cmd_ReadConfigFile("/animation/anim_fixups.cfg", true);
				animatedModel = NULL;
			}

			//register the model with the vid driver
			Vid_RegisterModel(model);
			
			return true;
		}
	}
	
	/*
	//the model is invalid, free all the memory we allocated
	Tag_FreeAll(MEM_MODEL);
	memset(model, 0, sizeof(model_t));
	*/

	Geom_Error(fmt("Geom_LoadModel: %s failed to load", filename));
	
	return false;
}




/****************************************************

  SKIN FUNCTIONS / CONFIG COMMANDS

 ****************************************************/


//sets the current skin name
void	Geom_SkinName_Cmd(int argc, char *argv[])
{
	int n;
	modelSkin_t *skin;

	if (argc < 1)
		return;

	if (!skinModel)
		return;

	strncpySafe(skinName, argv[0], SKIN_NAME_LENGTH);	

	//make sure this skin doesn't already exist
	for (n=0; n<skinModel->num_skins; n++)
	{
		if (stricmp(argv[0], skinModel->skins[n].name)==0)
			return;
	}

	if (skinModel->num_skins >= MAX_SKINS_PER_MODEL)
	{
		Console_DPrintf("Exceeded max skins for model\n");
		return;
	}

	skin = &skinModel->skins[skinModel->num_skins];

	//allocate the base skin info
	skin->baseDir = Tag_Strdup(skinDir, MEM_MODEL);	
	strcpy(skin->name, skinName);
	skin->shaderNames = Tag_Malloc(sizeof(shadername_t) * skinModel->num_meshes, MEM_MODEL);
	skin->shaders = Tag_Malloc(sizeof(residx_t) * skinModel->num_meshes, MEM_MODEL);

	//copy over the old skin to the new one, so we don't have to respecify all textures
	for (n=0; n<skinModel->num_meshes; n++)
	{
		Mem_Copy(skin->shaderNames[n], skinModel->meshes[n].defaultShader, sizeof(shadername_t));
	}

	skin->loaded = false;

	skinModel->num_skins++;
}

void	Geom_LoadSkinShaders(model_t *model, int skinidx)
{
	int i;
	modelSkin_t *skin;

	if (skinidx < 0 || skinidx >= model->num_skins)
	{
		//Console_DPrintf("Invalid skin index\n");
		skinidx = 0;
	}

	skin = &model->skins[skinidx];

	if (!skin->shaders)
	{
		Console_DPrintf("Trying to call Geom_LoadSkinShaders but skin->shaders is NULL!  Aborting!\n");
		return;
	}
	
	if (skin->loaded)
		return;

	for (i=0; i<model->num_meshes; i++)
	{
		//touch all the shaders the skin uses
		skin->shaders[i] = Res_LoadShader(fmt("%s", skin->shaderNames[i]));
		if (!skin->shaders[i])
			skin->shaders[i] = Res_LoadShader(fmt("%s%s", skin->baseDir, skin->shaderNames[i]));
	}

	skin->loaded = true;
}



//skinref command.  adds a shader reference to a mesh
//syntax: skinref meshname shadername
void	Geom_SkinRef_Cmd(int argc, char *argv[])
{
	int n;
	modelSkin_t *skin = NULL;

	if (argc < 2)
		return;
	if (!skinModel)
		return;
	
	for (n=0; n<skinModel->num_skins; n++)
	{
		if (strcmp(skinName, skinModel->skins[n].name)==0)
		{
			skin = &skinModel->skins[n];
			break;
		}
	}

	if (!skin)
		return;

	for (n=0; n<skinModel->num_meshes; n++)
	{	
		if (stricmp(argv[0], skinModel->meshes[n].name)==0)
		{
			strncpySafe(skin->shaderNames[n], argv[1], SKIN_SHADERNAME_LENGTH);
			return;
		}
	}

	Console_DPrintf("Unknown mesh name %s", argv[0]);
}


/****************************************************

  ANIMATION LOADING

 ****************************************************/



void	Geom_ForceLoop_Cmd(int argc, char *argv[])
{
	int n;

	if (!argc)
		return;

	if (!animatedModel)
		return;

	for (n=0; n<animatedModel->num_anims; n++)
	{
		modelAnim_t *anim = &animatedModel->anims[n];

		if (stricmp(argv[0], animatedModel->anims[n].name)==0)
		{
			anim->looping = true;
			if ((anim->loopbackframe = -1))
				anim->loopbackframe = anim->start;

			anim->numloopframes = MAX((anim->numframes - (anim->loopbackframe - anim->start)), 1);
		}
	}
}


/*==========================

  Geom_ReloadAnim

  

 ==========================*/

void	Geom_ReloadAnim(residx_t modelidx)
{
	char basefile[1024];
	int n;

	model_t *model = Res_GetModel(modelidx);
	if (!model)
		return;

	Filename_StripExtension(model->name, basefile);
	if (File_Exists(fmt("%s.anim", basefile)))
	{
		//free all anim data
		for (n=0; n<model->num_anims; n++)
		{
			Tag_Free(model->anims[n].events);
			Tag_Free(model->anims[n].motions);
		}
		Tag_Free(model->anims);
		model->anims = NULL;
		model->num_anims = 0;
	
		animatedModel = model;

		Cmd_ReadConfigFile(fmt("%s.anim", basefile), true);				
		//load the anim fixups file
		Cmd_ReadConfigFile("/animation/anim_fixups.cfg", true);

		animatedModel = NULL;
	}
}

/*==========================

  Geom_MakeAnim_Cmd

  this is how animations are created from external .anim files
 
  syntax: makeanim animName clipFile [startframe] [numframes] [fps] [loop] [loopbackframe]

 ==========================*/

void	Geom_MakeAnim_Cmd(int argc, char *argv[])
{
	int n,i;
	residx_t clipidx;
	modelClip_t *clip;
	modelAnim_t *anim;

	if (argc < 2)
	{		
		return;
	}

	if (!animatedModel)
		return;
	if (animatedModel->num_anims >= MAX_MODEL_ANIMS)
	{
		Console_Printf("Exceeded MAX_MODEL_ANIMS for %s\n", animatedModel->name);
		return;
	}

	clipidx = Res_LoadClip(argv[1]);
	if (!clipidx)
		return;
	clip = Res_GetClip(clipidx);

	if (!animatedModel->num_anims)
	{
		//allocate anims
		//when we first create an animation we will allocate the max allowed animations for the model.
		//no more allocation is needed after that
		animatedModel->anims = Tag_Malloc(sizeof(modelAnim_t) * MAX_MODEL_ANIMS, MEM_MODEL);
	}

	anim = &animatedModel->anims[animatedModel->num_anims];

	//setup initial params
	strncpySafe(anim->name, argv[0], ANIM_NAME_LENGTH);
	anim->clip = clipidx;	

	//set up pointers to bone motions
	anim->motions = Tag_Malloc(sizeof(boneMotion_t *) * animatedModel->num_bones, MEM_MODEL);
	for (n=0; n<animatedModel->num_bones; n++)
	{
		anim->motions[n] = NULL;

		for (i=0; i<clip->num_motions; i++)
		{
			if (stricmp(animatedModel->bones[n].name, clip->motions[i].boneName)==0)
			{
				//assign the bone to this motion
				anim->motions[n] = &clip->motions[i];
				break;
			}
		}
	}

	anim->start = 0;
	anim->fps = 30;
	anim->looping = false;
	anim->loopbackframe = -1;
	anim->numframes = 0;

	if (argc > 3)
	{
		anim->start = atoi(argv[2]);
		anim->numframes = atoi(argv[3]);
	}
	if (argc > 4)
	{
		anim->fps = atoi(argv[4]);
	}
	if (argc > 5)
	{
		anim->looping = atoi(argv[5]);
	}
	if (argc > 6)
	{
		anim->loopbackframe = atoi(argv[6]);
	}

	//fix up the frames
	if (anim->start == 0 && anim->numframes == 0)
	{
		anim->start = 0;
		anim->numframes = clip->num_frames;
	}
	else if (anim->numframes <= 0)
	{
		anim->numframes = 1;
	}
	else if (anim->start + anim->numframes > clip->num_frames)
	{
		anim->numframes = clip->num_frames - anim->start;
	}

	if (anim->looping)
	{
		if (anim->loopbackframe == -1)
			anim->loopbackframe = anim->start;

		anim->numloopframes = MAX((anim->numframes - (anim->loopbackframe - anim->start)), 1);
	}

	animatedModel->num_anims++;
}


/*==========================

  Geom_AnimEvent_Cmd

  events can be attached to specific frames of an animation with this command

  syntax: animEvent animName animFrame event_command

 ==========================*/

void	Geom_AnimEvent_Cmd(int argc, char *argv[])
{
	int n;
	modelAnim_t *anim = NULL;
	int frame = 0;
	static int id_counter = 1;

	if (argc < 3)
		return;

	if (!animatedModel)
		return;

	for (n=0; n<animatedModel->num_anims; n++)
	{
		if (stricmp(animatedModel->anims[n].name, argv[0])==0)
		{
			anim = &animatedModel->anims[n];
			break;
		}
	}

	if (!anim)
	{
		Console_Printf("animevent: anim name %s not found\n", argv[0]);
		return;
	}

	frame = atoi(argv[1]);
	if (frame < 0)
		return;

	if (anim->numEvents)
	{
		anim->events = Tag_Realloc(anim->events, (anim->numEvents + 1) * sizeof(animEvent_t), MEM_MODEL);
	}
	else
	{
		anim->events = Tag_Malloc(sizeof(animEvent_t), MEM_MODEL);
	}

	anim->events[anim->numEvents].frame = frame;
	anim->events[anim->numEvents].command = Tag_Strdup(argv[2], MEM_MODEL);
	anim->events[anim->numEvents].unique_id = id_counter;
	
	anim->numEvents++;
	id_counter++;
}



bool	Geom_ParseClipHeader(block_t *block)
{
	clipHeader_t *in = (clipHeader_t *)block->data;

	if (block->length < sizeof(clipHeader_t))
	{
		Geom_Error("Bad clip header");
		return false;
	}

	if (LittleInt(in->version) > 1)
	{
		Geom_Error("Bad clip version");
		return false;
	}
	
	outClip->num_motions = LittleInt(in->num_bones);
	outClip->num_frames = LittleInt(in->num_frames);
	outClip->motions = Tag_Malloc(outClip->num_motions * sizeof(boneMotion_t), MEM_MODEL);		

	return true;
}

void	Geom_ReadAngleKeys(float *data, int num_keys, floatKeys_t *keys, float tweak)
{
	int n;
	
	keys->keys = Tag_Malloc(sizeof(float) * num_keys, MEM_MODEL);

	for (n=0; n<num_keys; n++)
	{
		keys->keys[n] = LittleFloat(data[n]) + tweak;
		if (keys->keys[n] > 360)
			keys->keys[n] -= 360;
		if (keys->keys[n] < 0)
			keys->keys[n] += 360;
	}

	keys->num_keys = num_keys;
}

void	Geom_ReadScaleKeys(float *data, int num_keys, floatKeys_t *keys)
{
	int n;
	
	keys->keys = Tag_Malloc(sizeof(float) * num_keys, MEM_MODEL);

	for (n=0; n<num_keys; n++)
	{
		keys->keys[n] = LittleFloat(data[n]);		
	}

	keys->num_keys = num_keys;
}

void	Geom_ReadPositionKeys(float *data, int num_keys, floatKeys_t *keys)
{
	int n;
	
	keys->keys = Tag_Malloc(sizeof(float) * num_keys, MEM_MODEL);

	for (n=0; n<num_keys; n++)
	{
		keys->keys[n] = LittleFloat(data[n]) * geom_modelScale.value;
	}

	keys->num_keys = num_keys;
}

void	Geom_ReadByteKeys(byte *data, int num_keys, byteKeys_t *keys)
{
	int n;

	keys->keys = Tag_Malloc(num_keys, MEM_MODEL);

	for (n=0; n<num_keys; n++)
	{
		keys->keys[n] = data[n];
	}

	keys->num_keys = num_keys;
}

void	Geom_FixPositionKeys(boneMotion_t *motion)
{
	int n;

	for (n=0; n<outClip->num_frames; n++)
	{
		vec3_t pos = 
		{
			motion->keys_x.keys[n % motion->keys_x.num_keys],
			motion->keys_y.keys[n % motion->keys_y.num_keys],
			motion->keys_z.keys[n % motion->keys_z.num_keys]
		};

		M_TransformPoint(pos, zero_vec, (const vec3_t *)yaw180.axis, pos);

		motion->keys_x.keys[n % motion->keys_x.num_keys] = pos[0];
		motion->keys_y.keys[n % motion->keys_y.num_keys] = pos[1];
		motion->keys_z.keys[n % motion->keys_z.num_keys] = pos[2];
	}
}

void	Geom_FixClip()
{
	//rotate the scene root 180 degrees

	outClip->motions[0].keys_yaw.keys[0] = 180;
//	for (n=0; n<outClip->num_bones; n++)
//	{
//		Geom_FixPositionKeys(&outClip->motions[n]);
//	}
}

bool	Geom_ParseBoneMotionBlock(block_t *block)
{	
	int idx;
	int num_keys;
	keyBlock_t *in = (keyBlock_t *)block->data;
	boneMotion_t *motion;
	void *keydata;
	
	if (block->length < sizeof(keyBlock_t))
	{
		Geom_Error("Bad bone motion block");
		return false;
	}

	idx = LittleInt(in->boneIndex);
	if (idx < 0 || idx >= outClip->num_motions)
	{
		Geom_Error("Invalid bone motion index");
		return false;
	}

	num_keys = LittleInt(in->num_keys);

	motion =  &outClip->motions[idx];
	strncpySafe(motion->boneName, in->boneName, BONE_NAME_LENGTH);	

	keydata = in+1;

	switch(LittleInt(in->key_type))
	{
		case MKEY_X:
			Geom_ReadPositionKeys(keydata, num_keys, &motion->keys_x);
			break;
		case MKEY_Y:
			Geom_ReadPositionKeys(keydata, num_keys, &motion->keys_y);
			break;
		case MKEY_Z:
			Geom_ReadPositionKeys(keydata, num_keys, &motion->keys_z);
			break;
		case MKEY_PITCH:
			Geom_ReadAngleKeys(keydata, num_keys, &motion->keys_pitch, 0);
			break;
		case MKEY_ROLL:
			Geom_ReadAngleKeys(keydata, num_keys, &motion->keys_roll, 0);
			break;
		case MKEY_YAW:
			Geom_ReadAngleKeys(keydata, num_keys, &motion->keys_yaw, 0);
			break;
		case MKEY_SCALE:
			Geom_ReadScaleKeys(keydata, num_keys, &motion->keys_scale);
			break;
		case MKEY_VISIBILITY:
			Geom_ReadByteKeys(keydata, num_keys, &motion->keys_visibility);
			break;
		default:
			return false;			
	}

	


//	Geom_ScaleMotion(&motion, MODEL_SCALE_FACTOR);

	/*
	for (n=0; n<16; n++)
		motion->invBase.matrix[n] = LittleFloat(in->invBase.matrix[n]);
	
	motion->vertexAnimation = LittleInt(in->vertexAnimation);

	//read frames
	for (n=0; n<outClip->num_frames; n++)
	{
		for (i=0; i<4; i++)
		{
			for (j=0; j<4; j++)
			{
				motion->transforms[n].axis[i][j] = LittleFloat(framedata->t.axis[i][j]);
			}
			motion->transforms[n].pos[i] = LittleFloat(framedata->t.pos[i]);
		}

		framedata++;
	}
	*/
	return true;
}

bool	Geom_ReadClipBlocks(blockList_t *blocklist)
{
	int b;

	if (strcmp(blocklist->blocks[0].name, "head")!=0)
	{
		Geom_Error("No header for clip file");
		return false;
	}
	Geom_ParseClipHeader(&blocklist->blocks[0]);

	for (b=1; b<blocklist->num_blocks; b++)
	{
		block_t *block = &blocklist->blocks[b];
		if (strcmp(block->name, "bmtn")==0)
		{
			if (!Geom_ParseBoneMotionBlock(&blocklist->blocks[b]))
				return false;
		}
		
	}

	Geom_FixClip();

	return true;
}


bool Geom_LoadClip(const char *filename, modelClip_t *clip)
{
	bool ret = false;
	byte *buf;
	int buflen;
	blockList_t *blocklist;		

	buf = File_LoadIntoBuffer(filename, &buflen, 0);
	if (!buf)
	{
		Geom_Error(fmt("%s: Invalid clip file or file not found", filename));
		return false;
	}
	
	//check header
	if (buflen < 4)
	{
		File_FreeBuffer(buf);
		return false;
	}
	if (buf[0] != 'C' || buf[1] != 'L' || buf[2] != 'I' || buf[3] != 'P')
	{
		File_FreeBuffer(buf);
		return false;
	}

	//Console_DPrintf("Loaded clip file %s\n", filename);
	
	blocklist = File_AllocBlockList(buf+4, buflen-4, filename);
	if (blocklist)
	{
		outClip = clip;
		ret = Geom_ReadClipBlocks(blocklist);
		File_FreeBlockList(blocklist);
	}
	
	File_FreeBuffer(buf);

	return ret;
}

void	Geom_ScaleModel(model_t *model, float scale)
{
}



/****************************************************

  SKELETAL ANIMATION SYSTEM

 ****************************************************/




#define MAX_BONESTACK_DEPTH 64
#define MAX_TRANSFORMSTACK_DEPTH 64


typedef struct
{
	int			startpos;
	int			endpos;

	int			nodes[MAX_BONESTACK_DEPTH];
} boneStack_t;

typedef struct
{
	int			startpos;
	int			endpos;

	matrix43_t	nodes[MAX_TRANSFORMSTACK_DEPTH];
} transformStack_t;

static matrix43_t *currentTransform;
static matrix43_t identity;				//identity matrix

static boneStack_t stack;
static transformStack_t tmstack;


void	Bone_Push(int node)
{
	stack.nodes[stack.endpos] = node;

	stack.endpos = (stack.endpos + 1) % MAX_BONESTACK_DEPTH;
	if (stack.endpos == stack.startpos)
	{
		stack.endpos = (stack.endpos - 1) % MAX_BONESTACK_DEPTH;
		Console_DPrintf("Bone_Push: out of stack space\n");
	}
}

int	Bone_Pop()
{
	int ret;

	if (stack.startpos == stack.endpos)
		return -1;

	ret = stack.nodes[stack.startpos];
	stack.startpos = (stack.startpos + 1) % MAX_BONESTACK_DEPTH;

	return ret;
}

void Bone_ClearStack()
{
	stack.startpos = stack.endpos = 0;
}

void	TM_PushCurrent(matrix43_t *tm)
{
	tmstack.nodes[tmstack.endpos] = *tm;
	currentTransform = &tmstack.nodes[tmstack.endpos];

	tmstack.endpos = (tmstack.endpos + 1) % MAX_TRANSFORMSTACK_DEPTH;

	if (tmstack.endpos == tmstack.startpos)
	{
		tmstack.endpos = (tmstack.endpos - 1) % MAX_TRANSFORMSTACK_DEPTH;
		Console_DPrintf("TM_Push: out of stack space\n");
	}
}

void	TM_PopCurrent()
{	
	if (tmstack.startpos == tmstack.endpos)
		return;

	currentTransform = &tmstack.nodes[tmstack.startpos];
	tmstack.startpos = (tmstack.startpos + 1) % MAX_TRANSFORMSTACK_DEPTH;	
}

void	TM_ClearStack()
{
	tmstack.startpos = tmstack.endpos = 0;	
	currentTransform = &identity;	
}

int		SecondsToFrame(modelAnim_t *anim, float time)
{
	return (int)(time * anim->fps);
}


matrix43_t dest_tms[MAX_BONES];
bool		isIdentity[MAX_BONES];




/**** QuatTypes.h - Basic type declarations            ****/
/**** by Ken Shoemake, shoemake@graphics.cis.upenn.edu ****/
/**** in "Graphics Gems IV", Academic Press, 1994      ****/


/*** Definitions ***/

typedef struct {float x, y, z, w;} Quat; /* Quaternion */
//enum QuatPart {X, Y, Z, W};
typedef float HMatrix[4][4]; /* Right-handed, for column vectors */
typedef Quat EulerAngles;    /* (x,y,z)=ang 1,2,3, w=order code  */





//--------------------------------------------------------------------------------

//EulerAngles.h

/**** EulerAngles.h - Support for 24 angle schemes      ****/
/**** by Ken Shoemake, shoemake@graphics.cis.upenn.edu  ****/
/**** in "Graphics Gems IV", Academic Press, 1994       ****/


/*** Order type constants, constructors, extractors ***/

    /* There are 24 possible conventions, designated by:    */
    /*	  o EulAxI = axis used initially		    */
    /*	  o EulPar = parity of axis permutation		    */
    /*	  o EulRep = repetition of initial axis as last	    */
    /*	  o EulFrm = frame from which axes are taken	    */
    /* Axes I,J,K will be a permutation of X,Y,Z.	    */
    /* Axis H will be either I or K, depending on EulRep.   */
    /* Frame S takes axes from initial static frame.	    */
    /* If ord = (AxI=X, Par=Even, Rep=No, Frm=S), then	    */
    /* {a,b,c,ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v	    */
    /* rotates v around Z by c radians.			    */

#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
    /* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;\
    n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
    /* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
    /* Static axes */
#define EulOrdXYZs    EulOrd(X,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(X,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(X,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(X,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(Y,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(Y,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(Y,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(Y,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(Z,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(Z,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(Z,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(Z,EulParOdd,EulRepYes,EulFrmS)
    /* Rotating axes */
#define EulOrdZYXr    EulOrd(X,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(X,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(X,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(X,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(Y,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(Y,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(Y,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(Y,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(Z,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(Z,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(Z,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(Z,EulParOdd,EulRepYes,EulFrmR)

EulerAngles Eul_(float ai, float aj, float ah, int order);
Quat Eul_ToQuat(EulerAngles ea);
void Eul_ToHMatrix(EulerAngles ea, HMatrix M);
EulerAngles Eul_FromHMatrix(HMatrix M, int order);
EulerAngles Eul_FromQuat(Quat q, int order);




//--------------------------------------------------------------------------------

//EulerAngles.c

/**** EulerAngles.c - Convert Euler angles to/from matrix or quat ****/
/* Ken Shoemake, 1993 */

EulerAngles Eul_(float ai, float aj, float ah, int order)
{
    EulerAngles ea;
    ea.x = ai; ea.y = aj; ea.z = ah;
    ea.w = order;
    return (ea);
}
/* Construct quaternion from Euler angles (in radians). */
Quat Eul_ToQuat(EulerAngles ea)
{
    Quat qu;
    double a[3], ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd(ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) ea.y = -ea.y;
    ti = ea.x*0.5; tj = ea.y*0.5; th = ea.z*0.5;
    ci = cos(ti);  cj = cos(tj);  ch = cos(th);
    si = sin(ti);  sj = sin(tj);  sh = sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	a[i] = cj*(cs + sc);	/* Could speed up with */
	a[j] = sj*(cc + ss);	/* trig identities. */
	a[k] = sj*(cs - sc);
	qu.w = cj*(cc - ss);
    } else {
	a[i] = cj*sc - sj*cs;
	a[j] = cj*ss + sj*cc;
	a[k] = cj*cs - sj*sc;
	qu.w = cj*cc + sj*ss;
    }
    if (n==EulParOdd) a[j] = -a[j];
    qu.x = a[X]; qu.y = a[Y]; qu.z = a[Z];
    return (qu);
}

/* Construct matrix from Euler angles (in radians). */
void Eul_ToHMatrix(EulerAngles ea, HMatrix M)
{
    double ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd(ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = -ea.y; ea.z = -ea.z;}
    ti = ea.x;	  tj = ea.y;	th = ea.z;
    ci = cos(ti); cj = cos(tj); ch = cos(th);
    si = sin(ti); sj = sin(tj); sh = sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	M[i][i] = cj;	  M[i][j] =  sj*si;    M[i][k] =  sj*ci;
	M[j][i] = sj*sh;  M[j][j] = -cj*ss+cc; M[j][k] = -cj*cs-sc;
	M[k][i] = -sj*ch; M[k][j] =  cj*sc+cs; M[k][k] =  cj*cc-ss;
    } else {
	M[i][i] = cj*ch; M[i][j] = sj*sc-cs; M[i][k] = sj*cc+ss;
	M[j][i] = cj*sh; M[j][j] = sj*ss+cc; M[j][k] = sj*cs-sc;
	M[k][i] = -sj;	 M[k][j] = cj*si;    M[k][k] = cj*ci;
    }
    M[W][X]=M[W][Y]=M[W][Z]=M[X][W]=M[Y][W]=M[Z][W]=0.0; M[W][W]=1.0;
}

/* Convert matrix to Euler angles (in radians). */
EulerAngles Eul_FromHMatrix(HMatrix M, int order)
{
    EulerAngles ea;
    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==EulRepYes) {
	double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);
	if (sy > 16*FLT_EPSILON) {
	    ea.x = atan2(M[i][j], M[i][k]);
	    ea.y = atan2(sy, M[i][i]);
	    ea.z = atan2(M[j][i], -M[k][i]);
	} else {
	    ea.x = atan2(-M[j][k], M[j][j]);
	    ea.y = atan2(sy, M[i][i]);
	    ea.z = 0;
	}
    } else {
	double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);
	if (cy > 16*FLT_EPSILON) {
	    ea.x = atan2(M[k][j], M[k][k]);
	    ea.y = atan2(-M[k][i], cy);
	    ea.z = atan2(M[j][i], M[i][i]);
	} else {
	    ea.x = atan2(-M[j][k], M[j][j]);
	    ea.y = atan2(-M[k][i], cy);
	    ea.z = 0;
	}
    }
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = - ea.y; ea.z = -ea.z;}
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    ea.w = order;
    return (ea);
}

/* Convert quaternion to Euler angles (in radians). */
EulerAngles Eul_FromQuat(Quat q, int order)
{
    HMatrix M;
    double Nq = q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w;
    double s = (Nq > 0.0) ? (2.0 / Nq) : 0.0;
    double xs = q.x*s,	  ys = q.y*s,	 zs = q.z*s;
    double wx = q.w*xs,	  wy = q.w*ys,	 wz = q.w*zs;
    double xx = q.x*xs,	  xy = q.x*ys,	 xz = q.x*zs;
    double yy = q.y*ys,	  yz = q.y*zs,	 zz = q.z*zs;
    M[X][X] = 1.0 - (yy + zz); M[X][Y] = xy - wz; M[X][Z] = xz + wy;
    M[Y][X] = xy + wz; M[Y][Y] = 1.0 - (xx + zz); M[Y][Z] = yz - wx;
    M[Z][X] = xz - wy; M[Z][Y] = yz + wx; M[Z][Z] = 1.0 - (xx + yy);
    M[W][X]=M[W][Y]=M[W][Z]=M[X][W]=M[Y][W]=M[Z][W]=0.0; M[W][W]=1.0;
    return (Eul_FromHMatrix(M, order));
}




Geom_EulerToQuat(float roll, float pitch, float yaw, vec4_t quat)
{
	float cr, cp, cy, sr, sp, sy, cpcy, spsy;

	float halfyaw = DEG2RAD(yaw) * 0.5;
	float halfpitch = DEG2RAD(pitch) * 0.5;
	float halfroll = DEG2RAD(roll) * 0.5;

	// calculate trig identities
	cr = cos(halfroll);
	cp = cos(halfpitch);
	cy = cos(halfyaw);

	sr = sin(halfroll);
	sp = sin(halfpitch);
	sy = sin(halfyaw);
	
	cpcy = cp * cy;
	spsy = sp * sy;

	quat[0] = cr * cpcy + sr * spsy;
	quat[1] = sr * cpcy - cr * spsy;
	quat[2] = cr * sp * cy + sr * cp * sy;
	quat[3] = cr * cp * sy - sr * sp * cy;
}

void Geom_QuatToMatrix(const vec4_t quat, vec3_t out[3])
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2; 


	// calculate coefficients
	x2 = quat[1] + quat[1]; y2 = quat[2] + quat[2]; 
	z2 = quat[3] + quat[3];
	xx = quat[1] * x2; xy = quat[1] * y2; xz = quat[1] * z2;
	yy = quat[2] * y2; yz = quat[2] * z2; zz = quat[3] * z2;
	wx = quat[0] * x2; wy = quat[0] * y2; wz = quat[0] * z2;


	out[0][0] = 1.0 - (yy + zz); out[1][0] = xy - wz;
	out[2][0] = xz + wy;

	out[0][1] = xy + wz; out[1][1] = 1.0 - (xx + zz);
	out[2][1] = yz - wx;


	out[0][2] = xz - wy; out[1][2] = yz + wx;
	out[2][2] = 1.0 - (xx + yy);
} 


/*
void Geom_EulerToQuat(float yaw, float pitch, float roll, vec4_t out)
{
	float halfyaw = DEG2RAD(yaw) * 0.5;
	float halfpitch = DEG2RAD(pitch) * 0.5;
	float halfroll = DEG2RAD(roll) * 0.5;
	float c1 = cos(halfyaw);
	float s1 = sin(halfyaw);
	float c2 = cos(halfpitch);
    float s2 = sin(halfpitch);
	float c3 = cos(halfroll);
	float s3 = sin(halfroll);
    float c1c2 = c1*c2;
	float s1s2 = s1*s2;
	out[W] =c1c2*c3 + s1s2*s3;
  	out[X] =c1c2*s3 - s1s2*c3;
	out[Y] =c1*s2*c3 + s1*c2*s3;
	out[Z] =s1*c2*c3 - c1*s2*s3;
}
*//*
void Geom_QuatToMatrix(const vec4_t q, vec3_t out[3])
{    
	double sqw = q[0]*q[0];
	double sqx = q[1]*q[1];
	double sqy = q[2]*q[2];
	double sqz = q[3]*q[3];
	out[0][0] =  sqx - sqy - sqz + sqw; // since sqw + sqx + sqy + sqz =1
	out[1][1] = -sqx + sqy - sqz + sqw;   
	out[2][2] = -sqx - sqy + sqz + sqw;
	{
	float tmp1 = q[1]*q[2];
	float tmp2 = q[3]*q[0];
	out[1][0] = 2.0 * (tmp1 + tmp2);
	out[0][1] = 2.0 * (tmp1 - tmp2);
	tmp1 = q[1]*q[3];
	tmp2 = q[2]*q[0];
	out[2][0] = 2.0 * (tmp1 - tmp2);
	out[0][2] = 2.0 * (tmp1 + tmp2);
	tmp1 = q[2]*q[3];
	tmp2 = q[1]*q[0];
	out[2][1] = 2.0 * (tmp1 + tmp2);
	out[1][2] = 2.0 * (tmp1 - tmp2);
	}
}
*/


void	Geom_GetBoneTransform(boneMotion_t *motion, int loframe, int hiframe, float lerp, matrix43_t *out)
{
	vec3_t angles;
	
	if (!motion->keys_pitch.num_keys || !motion->keys_x.num_keys)
	{		
		M_Identity(out);

		return;
	}

	if (!geom_blendFrames.integer)
	{
		angles[PITCH] = motion->keys_pitch.keys[loframe % motion->keys_pitch.num_keys];
		angles[ROLL] = motion->keys_roll.keys[loframe % motion->keys_roll.num_keys];
		angles[YAW] = motion->keys_yaw.keys[loframe % motion->keys_yaw.num_keys];

		out->pos[X] = motion->keys_x.keys[loframe % motion->keys_x.num_keys] * geom_skelhack.value;
		out->pos[Y] = motion->keys_y.keys[loframe % motion->keys_y.num_keys] * geom_skelhack.value;
		out->pos[Z] = motion->keys_z.keys[loframe % motion->keys_z.num_keys] * geom_skelhack.value;

		M_GetAxis(angles[PITCH], angles[ROLL], angles[YAW], out->axis);
	}
	else
	{
		//vec4_t fromquat, toquat, lerpquat;
		//EulerAngles angs;
		//Quat blah;
		//int n;

		//no quat interpolation here.  if we start seeing artifacts i'll change it, but this seems to do the job pretty well
		//wraparound of euler angles is not a problem, because we go through M_LerpAngle

		/*Geom_EulerToQuat(motion->keys_yaw.keys[loframe % motion->keys_yaw.num_keys],
						 motion->keys_pitch.keys[loframe % motion->keys_pitch.num_keys],
						 motion->keys_roll.keys[loframe % motion->keys_roll.num_keys],
						 fromquat);*/
/*
		angs.x = DEG2RAD(motion->keys_pitch.keys[hiframe % motion->keys_pitch.num_keys]);
		angs.y = DEG2RAD(motion->keys_roll.keys[hiframe % motion->keys_roll.num_keys]);
		angs.z = DEG2RAD(motion->keys_yaw.keys[hiframe % motion->keys_yaw.num_keys]);
		angs.w = EulOrdYXZs;
		blah = Eul_ToQuat(angs);
		/*Geom_EulerToQuat(motion->keys_pitch.keys[hiframe % motion->keys_pitch.num_keys],
						 motion->keys_roll.keys[hiframe % motion->keys_roll.num_keys],
						 motion->keys_yaw.keys[hiframe % motion->keys_yaw.num_keys],
						 toquat);*/

		
		angles[PITCH] = M_LerpAngle(lerp, motion->keys_pitch.keys[loframe % motion->keys_pitch.num_keys], motion->keys_pitch.keys[hiframe % motion->keys_pitch.num_keys]);
		angles[ROLL] = M_LerpAngle(lerp, motion->keys_roll.keys[loframe % motion->keys_roll.num_keys], motion->keys_roll.keys[hiframe % motion->keys_roll.num_keys]);
		angles[YAW] = M_LerpAngle(lerp, motion->keys_yaw.keys[loframe % motion->keys_yaw.num_keys], motion->keys_yaw.keys[hiframe % motion->keys_yaw.num_keys]);
		

		//M_CopyVec4(toquat, lerpquat);
		//M_LerpQuat(lerp, fromquat, toquat, lerpquat);
/*
		lerpquat[0] = blah.w;
		lerpquat[1] = blah.x;
		lerpquat[2] = blah .y;
		lerpquat[3] = blah.z;
		Geom_QuatToMatrix(lerpquat, out->axis);*/

		out->pos[X] = LERP(lerp, motion->keys_x.keys[loframe % motion->keys_x.num_keys], motion->keys_x.keys[hiframe % motion->keys_x.num_keys]);
		out->pos[Y] = LERP(lerp, motion->keys_y.keys[loframe % motion->keys_y.num_keys], motion->keys_y.keys[hiframe % motion->keys_y.num_keys]);
		out->pos[Z] = LERP(lerp, motion->keys_z.keys[loframe % motion->keys_z.num_keys], motion->keys_z.keys[hiframe % motion->keys_z.num_keys]);

		M_GetAxis(angles[PITCH], angles[ROLL], angles[YAW], out->axis);
		//M_MultiplyAxis(out->axis, yaw180.axis, tmpaxis);		//fixme: sucks that i have to do this.  the proper fix is to refactor the euler angles
		//memcpy(out->axis, tmpaxis, sizeof(vec3_t)*3);
	}
}

byte	Geom_GetBoneVisibility(boneMotion_t *motion, int loframe, int hiframe, float lerp)
{	
	if (!motion->keys_visibility.num_keys)
		return 255;

	return motion->keys_visibility.keys[loframe % motion->keys_visibility.num_keys];		
}


bool	newPose;
skeleton_t *skel;
int loframe, hiframe;
float lerp;


//todo:

//do a begin/end type thing:

//Geom_BeginPose(&skeleton, sc.model);
//Geom_SetBoneAnim("", "walk", ...);		//sets only relative transforms!
//Geom_SetBoneRotation("head", 0, 0, cl.yaw);	//sets transform for this bone only!  (does not recurse)
//Geom_EndPose();							//transforms relative transforms into world transforms! (recurses heirarchy)

//mix walk and run
//
//Geom_BeginPose(&skeleton, sc.model)
//Geom_SetBoneAnim("", "walk", ...);
//Geom_SetBoneAnim(



/*==========================

  Geom_BeginPose

  this must be called by the game code before SetBoneAnim, RotateBone, etc, can be called

 ==========================*/

void	Geom_BeginPose(skeleton_t *skeleton, residx_t refmodel)
{
	int n;
	model_t *model = Res_GetModel(refmodel);

	skel = skeleton;

	if (!refmodel || geom_skipSkel.integer || !model->num_anims)
	{
		//free old skeleton data, if any
		Geom_FreeSkeleton(skeleton);

		skel = NULL;
		return;
	}

	//the skeleton is using a new model, so clear all the data
	if (refmodel != skel->model)
	{		
		Geom_FreeSkeleton(skel);

		skel->model = refmodel;
		skel->numBones = model->num_bones;
		
		skel->bones = Tag_Malloc(sizeof(boneState_t) * skel->numBones, MEM_SKELETON);
	}

	//reset masks, etc
	for (n=0; n<skel->numBones; n++)
	{
		skel->bones[n].poseState = 0;
		skel->bones[n].visibility = 255;
	}

	/*
	for (n=0; n<skel->num_bones; n++)
	{
		skel->bones[n].tm_world = identity;
	}
	*/
}

void	Geom_FreeSkeleton(skeleton_t *skeleton)
{
	if (!skeleton)
		return;

	if (skeleton->numBones && skeleton->bones)
	{
		Tag_Free(skeleton->bones);
	}

	memset(skeleton, 0, sizeof(skeleton_t));
}

/*
void	Geom_MapSkeleton(const skeleton_t *skel, residx_t model, skeleton_t *out)
{
	int n;
	boneState_t *bonesptr;

	if (out->numBones != skel->numBones || !out->bones)
	{
		Geom_FreeSkeleton(out);

		out->bones = Tag_Malloc(sizeof(boneState_t) * skel->numBones, MEM_SKELETON);
	}

	bonesptr = out->bones;
	*out = *skel;
	out->bones = bonesptr;
}
*/

void	Geom_SetBoneState(const char *boneName, int posestate)
{
	int boneidx;

	if (!skel)
		return;

	boneidx = Geom_GetBoneIndex(skel, boneName);
	if (boneidx == -1)
		return;

	skel->bones[boneidx].poseState = posestate;
}

int		Geom_GetBoneState(const char *boneName)
{
	int idx;

	if (!skel)
		return 0;

	idx = Geom_GetBoneIndex(skel, boneName);
	if (idx == -1)
		return 0;
	
	return skel->bones[idx].poseState;
}

#define BONE_POSTMULTIPLY	0x0001			//multiply the current transform
#define BONE_REPLACE		0x0002			//replace the current transform
#define BONE_ABSOLUTE		0x0004			//calculate a transform that will put the bone into the world space specified

void	Geom_SetBonePosition(const char *boneName, const vec3_t position, int mode)
{

}

void	Geom_GetBonePosition(const char *boneName, vec3_t position, int mode)
{
}

void	Geom_SetBoneScale(const char *boneName, const vec3_t scale, int mode)
{
}

void	Geom_GetBoneScale(const char *boneName, vec3_t scale, int mode)
{
}

void	Geom_GetBoneWorldTransform(skeleton_t *skel, const vec3_t objectPosition, const vec3_t objectAxis[3], const char *boneName, vec3_t position, vec3_t axis[3])
{
	int boneidx;

	if (!skel)
		return;

	boneidx = Geom_GetBoneIndex(skel, boneName);

	
}

//void	Geom_

//static matrix43_t out_tms[MAX_BONES];

void	Geom_RelativeToWorldRecurse(bone_t *bone)
{
	int n;	
	bone_t *parent = bone->parent;

	if (skel->bones[bone->index].poseState & POSE_MASKED)
		return;

	if (parent)
	{
		matrix43_t temp;

		M_MultiplyMatrix(&skel->bones[parent->index].tm_world, &skel->bones[bone->index].tm, &temp);		
		skel->bones[bone->index].tm_world = temp;
	}
	else
	{
		skel->bones[bone->index].tm_world = skel->bones[bone->index].tm;
	}
	
	for (n=0; n<bone->numChildren; n++)
	{
		Geom_RelativeToWorldRecurse(bone->children[n]);
	}
}




void	Geom_CalcWorldTransforms()
{
	model_t *model;
	bone_t *root;

	if (!skel)
		return;

	model = Res_GetModel(skel->model);
	
	root = &model->bones[0];

	Geom_RelativeToWorldRecurse(root);
}



/*==========================

  Geom_EndPose

  finalize the skeleton by computing the world 
  transform for all bones

 ==========================*/

void	Geom_EndPose()
{
	if (!skel)
		return;

	if (geom_skipPose.integer)
	{
		skel = NULL;
		return;
	}

	Geom_CalcWorldTransforms();

	skel = NULL;
	newPose = true;
}



/*
void	Geom_EndPose()
{
	int n;
	model_t *model;

	if (!skel)
		return;

	model = Res_GetModel(skel->model);

	for (n=0; n<model->num_bones; n++)
	{
		skel->bones[n].tm_world = dest_tms[n];
	}
}
*/

/*==========================

  Geom_PoseBoneAnim

  recurse through all children and set their relative transforms for the animation

 ==========================*/

void	Geom_PoseBoneAnim(model_t *model, modelAnim_t *anim, int boneidx)
{
	int n;
	matrix43_t lerpedTransform;	
	matrix43_t temp;

	if (skel->numBones <= boneidx || boneidx < 0)
	{
		Console_DPrintf("PoseBoneAnim Error: boneidx is %i, numBones is %i!\n", boneidx, skel->numBones);
		return;
	}
	
	if (skel->bones[boneidx].poseState & POSE_MASKED)
	{
		return;
	}


	sceneStats.numPoseBones++;


/*
	for (n=0; n<model->num_bones; n++)
	{
		bone_t *parent = model->bones[n].parent;
		boneMotion_t *motion = anim->motions[n];

		if (!motion || !parent)
			continue;
		
		Geom_GetBoneTransform(anim->motions[n], loframe, hiframe, lerp, &lerpedTransform);
		skel->bones[boneidx].visibility = Geom_GetBoneVisibility(anim->motions[boneidx], loframe, hiframe, lerp);
		
		dest_tms[n] = lerpedTransform;
		M_MultiplyMatrix(&parent->base, &lerpedTransform, &dest_tms[n]);

		//Geom_FixModelMatrix(&dest_tms[n]);
		
	}
*/

	if (anim->motions[boneidx])
	{
		Geom_GetBoneTransform(anim->motions[boneidx], loframe, hiframe, lerp, &lerpedTransform);
		skel->bones[boneidx].visibility = Geom_GetBoneVisibility(anim->motions[boneidx], loframe, hiframe, lerp);

		if (!isIdentity[boneidx])		//optimize away the multiply when the matrix is the identity
		{
			M_MultiplyMatrix(&dest_tms[boneidx], &lerpedTransform, &temp);
			dest_tms[boneidx] = temp;
		}
		else
		{
			dest_tms[boneidx] = lerpedTransform;
			isIdentity[boneidx] = false;
		}
	}

	for (n=0; n<model->bones[boneidx].numChildren; n++)
	{
		int childidx = model->bones[boneidx].children[n]->index;

		Geom_PoseBoneAnim(model, anim, childidx);
	}

}


/*==========================

  Geom_RotateBone

  set euler angles for bone.  'multiply' specifies whether or not 
  to multiply by the current transform or to use an absolute
  orientation

 ==========================*/

void	Geom_RotateBone(skeleton_t *skeleton, const char *boneName, float pitch_offset, float roll_offset, float yaw_offset, bool multiply)
{
	matrix43_t rotation, temp;
	int boneidx = 0;
	model_t *model;

	if (!skeleton)
		return;

	model = Res_GetModel(skeleton->model);

	boneidx = Geom_GetBoneIndex(skeleton, boneName);
	if (boneidx == -1)
		return;

	rotation = identity;

	//postmultiply the rotation
	M_GetAxis(pitch_offset, roll_offset, yaw_offset, rotation.axis);

	if (multiply)
	{
		M_MultiplyMatrix(&skeleton->bones[boneidx].tm, &rotation, &temp);
		skeleton->bones[boneidx].tm = temp;
	}
	else
	{
		Mem_Copy(skeleton->bones[boneidx].tm.axis, rotation.axis, sizeof(rotation.axis));
	}
}

int Geom_GetBoneIndex(skeleton_t *skeleton, const char *boneName)
{
	int n;
	model_t *model;

	if (!skeleton)
		return -1;
	
	if (!boneName[0])
		return 0;

	model = Res_GetModel(skeleton->model);

	for (n=0; n<skeleton->numBones; n++)
	{
		if (stricmp(boneName, model->bones[n].name)==0)
			return n;
	}

	return -1;
}



/*==========================

  Geom_BlendBones

  blend between dest_tms and the old skeleton tms

 ==========================*/

void Geom_BlendBones(bone_t *bone, float lerp)
{
	int n;

	if (skel->bones[bone->index].poseState & POSE_MASKED)
		return;

	if (lerp == 1)
	{
		skel->bones[bone->index].tm = dest_tms[bone->index];
	}
	else
	{

		vec4_t from, to, result;
		M_AxisToQuat(skel->bones[bone->index].tm.axis, from);
		M_AxisToQuat(dest_tms[bone->index].axis, to);
		
		//M_CopyVec4(to, result);
		//result[0] = LERP(lerp, from[0], to[0]);
		//result[1] = LERP(lerp, from[1], to[1]);
		//result[2] = LERP(lerp, from[2], to[2]);
		//result[3] = LERP(lerp, from[3], to[3]);
		M_LerpQuat(lerp, from, to, result);

		M_QuatToAxis(result, skel->bones[bone->index].tm.axis);

		//M_CopyVec3(dest_tms[bone->index].pos, skel->bones[bone->index].tm.pos);

		
		//lerp position
		M_LerpVec3(lerp,
					skel->bones[bone->index].tm.pos,
					dest_tms[bone->index].pos,
					skel->bones[bone->index].tm.pos);
		
		//temporary until quat code is working:
/*
		M_LerpVec3(lerp, skel->bones[bone->index].tm.axis[0], dest_tms[bone->index].axis[0], skel->bones[bone->index].tm.axis[0]);
		//M_Normalize(skel->bones[bone->index].tm.axis[0]);
		M_LerpVec3(lerp, skel->bones[bone->index].tm.axis[1], dest_tms[bone->index].axis[1], skel->bones[bone->index].tm.axis[1]);
		//M_Normalize(skel->bones[bone->index].tm.axis[1]);
		M_LerpVec3(lerp, skel->bones[bone->index].tm.axis[2], dest_tms[bone->index].axis[2], skel->bones[bone->index].tm.axis[2]);
		//M_Normalize(skel->bones[bone->index].tm.axis[2]);

		//lerp position
		M_LerpVec3(lerp, 
					skel->bones[bone->index].tm.pos,
					dest_tms[bone->index].pos,
					skel->bones[bone->index].tm.pos);*/

	}

	for (n=0; n<bone->numChildren; n++)
	{			
		Geom_BlendBones(bone->children[n], lerp);
	}	
}



/*==========================

  CL_ComputeAnimFrame

  computes loframe, hiframe, and lerp amounts based on a modelAnim_t structure and a given time
  non looping animations will freeze on their last frame
  looping animations will loop back to the loopbackframe specified in the anim struct
  time is specified in seconds

 ==========================*/

void CL_ComputeAnimFrame(modelAnim_t *anim, float time, int *loframe, int *hiframe, float *lerp_amt)
{
	float atime = time * anim->fps;
	int ft = (int)(atime);
	
	if (ft >= anim->numframes-1)
	{
		if (!anim->looping)
		{
			*loframe = anim->start + anim->numframes - 1;
			*hiframe = anim->start + anim->numframes - 1;
			*lerp_amt = 0;
			return;
		}
		else
		{
			*loframe = (ft % anim->numloopframes) + anim->loopbackframe;
			*hiframe = ((ft+1) % anim->numloopframes) + anim->loopbackframe;
			*lerp_amt = atime - ft;
			return;
		}
	}

	*loframe = (ft % anim->numframes) + anim->start;
	*hiframe = ((ft+1) % anim->numframes) + anim->start;
	*lerp_amt = atime - ft;

	return;
}



/*==========================

  Geom_MixBoneAnim		(TODO)

  Mix in a secondary bone animation after a call to SetBoneAnim was made

  blendAmount - a value between 0 and 1 specifying how much to mix the animation into the current skeleton

 ==========================*/

void Geom_MixBoneAnim(const char *parentBone, const char *animName, float animTime, float blendAmount)
{
	if (!skel)
		return;
}


static int events_only = false;

/*==========================

  Geom_SetBoneAnim

  position a skeleton based on the animation specified.
  if animation events are encountered, they will be
  placed into the skeleton's events fields.

  currentTime - counter keeping track of the current game time or system time (milliseconds)
  blendTime - if the animation changes, specify how many milliseconds to transition completely to the new animation

 ==========================*/

int		Geom_SetBoneAnim(const char *parentBone, const char *animName, float animTime, int currentTime, int blendTime, int eventChannel)
{
	int n;
	model_t *model;
	modelClip_t *clip = NULL;		//clip this animation references
	int parentidx;
	bool newanim = false;
	float animSecs;	
	modelAnim_t *anim = NULL;
	//bool noAnim = false;
	float atime;
	int frame;

	if (!skel)
		return SKEL_INVALID;

	sceneStats.numSetBoneAnims++;

	if (eventChannel < 0 || eventChannel >= MAX_EVENT_CHANNELS)
		eventChannel = 0;

	model = Res_GetModel(skel->model);

	for (n=0; n<model->num_anims; n++)
	{
		if (stricmp(animName, model->anims[n].name)==0)
		{
			anim = &model->anims[n];
			break;
		}
	}

	if (!anim)
	{
		//we're not referencing a valid anim, but pose the model anyway		
		anim = &model->anims[0];
		animTime = 0;
		blendTime = 0;
		currentTime = 0;

		skel->invalid = true;

		return SKEL_ANIM_NOT_FOUND;
		//noAnim = true;
	}

	skel->numBones = model->num_bones;

	if (newPose)
	{
		if (skel->animName)
		{
			if (stricmp(animName, skel->animName)!=0)
			{
				//set up the blend time here
				skel->blendTime = blendTime;		
				newanim = true;
				strncpySafe(skel->animName, anim->name, ANIM_NAME_LENGTH);
			}
		}	
		newPose = false;
	}

	/*
	//set all working TMs to the default pose
	for (n=0; n<skel->numBones; n++)
	{
		dest_tms[n] = model->bones[n].base;
		isIdentity[n] = false;
	}
	*/

	//work out the frames we'll be interpolating between
	animSecs = animTime / 1000.0;	//convert animTime to seconds

	CL_ComputeAnimFrame(anim, animSecs, &loframe, &hiframe, &lerp);

	atime = animSecs * anim->fps;
	lerp = atime - (int)atime;

	/************************
	 * Event handling
	 ************************/

	
	//frame = (int)(animSecs * anim->fps) % anim->numframes;

	if (anim->numEvents && !geom_skipEvents.integer)
	{
		frame = loframe - anim->start;
		for (n=0; n<anim->numEvents; n++)
		{
			animEvent_t *event = &anim->events[n];		
			if (event->frame >= frame && event->frame < frame+2) //give the event a small window to account for crappy FPS
			{
				if (skel->eventNumber[eventChannel] == event->unique_id)
				{
					//we already got this event on the last call
					skel->gotEvent = false;				
					break;
				}

				skel->gotEvent = true;
				skel->eventNumber[eventChannel] = event->unique_id;
				strncpySafe(skel->eventString, event->command, ANIM_EVENT_STRING_LENGTH);

				break;
			}
		}

		if (n == anim->numEvents)
		{
			//no events were encountered
			skel->gotEvent = false;

			//reset the event number to allow the same event to happen again
			skel->eventNumber[eventChannel] = 0;
		}
	}
	else
	{
		skel->gotEvent = false;
		//keep the event number so that animation mixing doesn't cause duplicate events over subsequent frames
	}


	if (events_only || geom_skipPose.integer)
	{
		skel->time = currentTime;

		return SKEL_NORMAL;
	}
	
	/***************************
	 * Bone calculation
	 ***************************/

	//set all working TMs to the identity
	for (n=0; n<skel->numBones; n++)
	{
		dest_tms[n] = identity;
		isIdentity[n] = true;
	}

	parentidx = Geom_GetBoneIndex(skel, parentBone);
	if (parentidx == -1)
		return SKEL_BONE_NOT_FOUND;
	

	clip = Res_GetClip(anim->clip);
	if (!clip)
		return SKEL_INVALID_CLIP;

	//make sure we don't get an access violation trying to access bad frames
	loframe = (int)loframe % clip->num_frames;
	hiframe = (int)hiframe % clip->num_frames;

	//loframe and hiframe should be valid array indexes at this point

	TM_ClearStack();
	Geom_PoseBoneAnim(model, anim, parentidx);

	/************************
	 * Animation blending
	 ************************/

	if (skel->blendTime > 0 && geom_blendAnims.integer)
	{
		float blendAmt;

		skel->blendTime -= (currentTime - skel->time);
		if (skel->blendTime < 0)
			skel->blendTime = 0;

		blendAmt = 1.0 - (float)skel->blendTime / (float)blendTime;

		Geom_BlendBones(&model->bones[parentidx], blendAmt);
	}
	else
	{
		Geom_BlendBones(&model->bones[parentidx], 1.0);
	}

	skel->time = currentTime;
	skel->invalid = false;

	return SKEL_NORMAL;
}

void Geom_SetBoneAnimNoPose(const char *animName, float animTime, int currentTime, int eventChannel)
{
	events_only = true;
	Geom_SetBoneAnim("", animName, animTime, currentTime, 0, eventChannel);
	events_only = false;
}



void	Geom_Init()
{
	Cvar_Register(&geom_defaultBoxScale);
	Cvar_Register(&geom_skelhack);
	Cvar_Register(&geom_simpleSurfaces);
	Cvar_Register(&geom_modelScale);
	Cvar_Register(&geom_skipSkel);
	Cvar_Register(&geom_blendAnims);
	Cvar_Register(&geom_blendFrames);
	Cvar_Register(&geom_fatalError);
	Cvar_Register(&geom_skipPose);
	Cvar_Register(&geom_skipEvents);
	Cvar_Register(&geom_debug);

	Cmd_Register("makeanim", Geom_MakeAnim_Cmd);
	Cmd_Register("animEvent", Geom_AnimEvent_Cmd);
	Cmd_Register("skinName", Geom_SkinName_Cmd);
	Cmd_Register("skinRef", Geom_SkinRef_Cmd);
	Cmd_Register("forceLoop", Geom_ForceLoop_Cmd);

	M_Identity(&identity);
	M_GetAxis(0,0, 180, yaw180.axis);
	yaw180.pos[0] = 0;
	yaw180.pos[1] = 0;
	yaw180.pos[2] = 0;
}

