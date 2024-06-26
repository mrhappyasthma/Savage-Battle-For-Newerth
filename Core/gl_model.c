// (C) 2003 S2 Games

// gl_model.c

// Model Drawing

//#define MODEL_PARANOIA

#include "core.h"
#ifdef _WIN32
#include "glh_genext.h"
#endif

typedef struct
{
	//offsets into the buffer where data is stored
	//set to -1 if the data does not exist in the buffer
	int		vertOffset;
	int		tvertOffset;
	int		normalOffset;
	int		colorOffset;
} gl_vbo_data_t;

#define MESHDATA(mesh) ((gl_vbo_data_t *)(mesh->renderData[0]))

cvar_t	gfx_useLOD = { "gfx_useLOD", "1", CVAR_SAVECONFIG };
cvar_t	gfx_forceLOD = { "gfx_forceLOD", "0", CVAR_SAVECONFIG };
cvar_t	gfx_useVertexColors = { "gfx_useVertexColors", "1", CVAR_SAVECONFIG };
cvar_t	gfx_drawModelBounds = { "gfx_drawModelBounds", "0", CVAR_CHEAT };
cvar_t	gfx_drawMeshBounds = { "gfx_drawMeshBounds", "0", CVAR_CHEAT };
cvar_t	gfx_drawBones = { "gfx_drawBones", "0", CVAR_CHEAT };
cvar_t	gfx_boneAlpha = { "gfx_boneAlpha", "0.5" };
cvar_t	gfx_drawJoints = { "gfx_drawJoints", "1" };
cvar_t	gfx_drawBoneNames = { "gfx_drawBoneNames", "0" };
cvar_t	gfx_drawLinks = { "gfx_drawLinks", "1" };
cvar_t	gfx_drawMeshes = { "gfx_drawMeshes", "1", CVAR_CHEAT };
cvar_t	gfx_skipDeform = { "gfx_skipDeform", "0", CVAR_CHEAT };
cvar_t	gfx_forceNonBlendedDeform = { "gfx_forceNonBlendedDeform", "1", CVAR_SAVECONFIG };
cvar_t	gfx_showWeights = { "gfx_showWeights", "0", CVAR_CHEAT };
cvar_t	gfx_drawNormals = { "gfx_drawNormals", "0", CVAR_CHEAT };
cvar_t	gfx_normalLength = { "gfx_normalLength", "10" };
cvar_t	gfx_normalWidth = { "gfx_normalWidth", "1" };
cvar_t	gfx_drawWire = { "gfx_drawWire", "0", CVAR_CHEAT };
cvar_t	gfx_wireAlpha = { "gfx_wireAlpha", "0.2", CVAR_CHEAT };
cvar_t	gfx_allowMismatchedBones = { "gfx_allowMismatchedBones", "1" };
cvar_t	gfx_alwaysStatic = { "gfx_alwaysStatic", "0", CVAR_CHEAT };


//static matrix43_t worldTMs[MAX_BONES];
static matrix43_t identity;

//streaming vertex buffer
GLuint	vertexStream = 1;




/*==========================

  GL_MeshShouldDeform

  is the mesh going to dynamically deform?

 ==========================*/

bool	GL_MeshShouldDeform(sceneobj_t *obj, mesh_t *mesh)
{
	if (obj->skeleton && mesh->bonelink == -1)
		return true;

	return false;
}


/*==========================

  GL_BufferData

  initializes static vertex buffers for the given mesh
  the data will be sub data in the larger model buffer

  if sizecount is true, count the amount of data that should be copied, but don't copy it

 ==========================*/

unsigned int	GL_BufferSubData(int offset, mesh_t *mesh, bool sizecount)
{
	unsigned int size = 0;	

#ifdef USE_VBO

	if (!sizecount)
	{
		mesh->renderData[0] = Tag_Malloc(sizeof(gl_vbo_data_t), MEM_VIDDRIVER);
		mesh->hasRenderData = true;
		MESHDATA(mesh)->vertOffset = -1;
		MESHDATA(mesh)->normalOffset = -1;
		MESHDATA(mesh)->colorOffset = -1;
		MESHDATA(mesh)->tvertOffset = -1;
	}
	
//	if (!mesh->model->num_anims)
	{
		//if the parent model has no animations, it's likely that this mesh will not deform, so give it a static draw buffer		
		int datasize = mesh->num_verts * sizeof(vec3_t);

		if (!sizecount)
		{
			glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset + size, datasize, mesh->verts);
			MESHDATA(mesh)->vertOffset = offset + size;
		}

		size += (datasize + 31) & ~31;
	}
	if (mesh->normals)
	{
		int datasize = mesh->num_verts * sizeof(vec3_t);

		if (!sizecount)
		{
			glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset + size, datasize, mesh->normals);
			MESHDATA(mesh)->normalOffset = offset + size;
		}

		size += (datasize + 31) & ~31;
	}
	if (mesh->colors)
	{
		int datasize = mesh->num_verts * sizeof(bvec4_t);

		if (!sizecount)
		{
			glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset + size, datasize, mesh->colors);
			MESHDATA(mesh)->colorOffset = offset + size;
		}

		size += (datasize + 31) & ~31;
	}
	if (mesh->tverts)
	{
		int datasize = mesh->num_verts * sizeof(vec2_t);

		if (!sizecount)
		{
			glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset + size, datasize, mesh->tverts);
			MESHDATA(mesh)->tvertOffset = offset + size;
		}

		size += (datasize + 31) & ~31;
	}
	
#endif
	return size;
}


/*==========================

  GL_RegisterModel

  called when a new model has just been loaded into memory

 ==========================*/

extern cvar_t gl_ext_vertex_buffer_object;

bool	GL_RegisterModel(model_t *model)
{
#ifdef USE_VBO
	int size = 0;
	int n;

	if (!gl_ext_vertex_buffer_object.integer)			//only needed for VBO
		return true;

	//figure out how big a buffer to give the model
	for (n=0; n<model->num_meshes; n++)
	{
		size += GL_BufferSubData(0, &model->meshes[n], true);
	}

	//initialize a buffer to hold all the static vertex data
	glGenBuffersARB(1, &model->buffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->buffer);

	//reset GL error flags
	while (glGetError() != GL_NO_ERROR);

	//alloc buffer
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_STATIC_DRAW_ARB);

	//make sure buffer was initialized successfully
	if (glGetError() != GL_NO_ERROR)
	{		
		System_Error("GL: Error allocating vertex buffer (most likely out of memory)\n");
	}

	//add the data
	size = 0;
	for (n=0; n<model->num_meshes; n++)
	{
		size += GL_BufferSubData(size, &model->meshes[n], false);
	}
#endif

	return true;
}


/*==========================

  GL_SetupMeshArrays

  tell GL where to find the geometry data
  
  stuff non animating models into static memory if not already done

 ==========================*/

void	GL_SetupMeshArrays(sceneobj_t *obj, mesh_t *mesh, gl_arrays_t *arrays)
{
	OVERHEAD_INIT;

#ifdef USE_VBO
	if (gl_ext_vertex_buffer_object.integer)
	{
		//bool useNormals = false;
		//bool useColors = false;
		//bool useTverts = false;

		/****************************************

		VBO PATH

		*****************************************/

		//bind the model's buffer
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->model->buffer);

		//normals, colors and tverts are always static
		//(really normals should blend too, but this is not supported yet due to speed concerns)
		//if (mesh->hasRenderData)
		//{
		if (MESHDATA(mesh)->normalOffset != -1)
		{	
			GL_EnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, (char *)NULL + MESHDATA(mesh)->normalOffset);			
		}
		else
		{
			GL_DisableClientState(GL_NORMAL_ARRAY);
		}
		if (MESHDATA(mesh)->colorOffset != -1 && gfx_useVertexColors.integer)
		{
			GL_EnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, (char *)NULL + MESHDATA(mesh)->colorOffset);				
		}
		else
		{
			GL_DisableClientState(GL_COLOR_ARRAY);
		}
		if (MESHDATA(mesh)->tvertOffset != -1)
		{			
			GL_EnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, (char *)NULL + MESHDATA(mesh)->tvertOffset);
		}
		else
		{
			GL_DisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		//}

		if (GL_MeshShouldDeform(obj, mesh) && !gfx_alwaysStatic.integer /* || !mesh->hasRenderData*/)
		{
			//set up the streaming vertex buffer			
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexStream);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, mesh->num_verts * sizeof(vec3_t), NULL, GL_STREAM_DRAW_ARB);
			//(GL_DrawSkinnedMesh() will handle mapping and writing to the buffer)
			glVertexPointer(3, GL_FLOAT, 0, NULL);
		}
		else
		{
			glVertexPointer(3, GL_FLOAT, 0, (char *)NULL + MESHDATA(mesh)->vertOffset);
			//(char *)arrays->verts = (char *)NULL + MESHDATA(mesh)->vertOffset;
		}

		OVERHEAD_COUNT(OVERHEAD_MODEL_SETUPMESHARRAYS);
	}
	else
#endif //USE_VBO
	{

		/****************************************

		VAR AND FALLBACK PATH

		*****************************************/

		if (mesh->renderDataRefCount != gfx_renderDataRefCount.integer)
		{
			//if this mesh has no anims, we can allocate static geometry to make the GPU happier
			//this is done once per level load, so it's a "first come first served" scheme
			//for allocating static data for meshes.  once the static partition runs out,
			//we default to dynamic allocation
			//
			//note that just because a mesh has no model animations, does not mean the mesh
			//will never animate.  it may still use a skeleton_t structure to deform
			//for most models, however, this will be true, so we can optimize this case

			mesh->renderDataRefCount = gfx_renderDataRefCount.integer;
			if (!mesh->model->num_anims)
			{
				//allocate static memory

				//these may return NULL, in which case we'll just allocate dynamically

				GL_StaticMallocHold();		//save the static block state in case we can't allocate enough

				mesh->renderData[RD_VERTS] = GL_StaticMalloc(mesh->num_verts * sizeof(vec3_t));
				mesh->renderData[RD_NORMALS] = mesh->normals ? GL_StaticMalloc(mesh->num_verts * sizeof(vec3_t)) : NULL;			
				mesh->renderData[RD_COLORS] = mesh->colors ? GL_StaticMalloc(mesh->num_verts * sizeof(bvec4_t)) : NULL;			
				mesh->renderData[RD_TVERTS] = mesh->colors ? GL_StaticMalloc(mesh->num_verts * sizeof(vec2_t)) : NULL;
				
				if (!mesh->verts 
					|| (mesh->normals && !mesh->renderData[RD_NORMALS]) 
					|| (mesh->colors && !mesh->renderData[RD_COLORS]) 
					|| (mesh->tverts && !mesh->renderData[RD_TVERTS])
					)
				{
					mesh->renderData[RD_VERTS] = 
						mesh->renderData[RD_NORMALS] = 
						mesh->renderData[RD_COLORS] = 
						mesh->renderData[RD_TVERTS] = 
						NULL;

					GL_StaticMallocFetch();		//regain the static memory we're not going to use
					mesh->hasRenderData = false;				
				}
				else
				{
					Mem_Copy(mesh->renderData[RD_VERTS], mesh->verts, mesh->num_verts * sizeof(vec3_t));
					if (mesh->renderData[RD_NORMALS])
						Mem_Copy(mesh->renderData[RD_NORMALS], mesh->normals, mesh->num_verts * sizeof(vec3_t));
					if (mesh->renderData[RD_COLORS])
						Mem_Copy(mesh->renderData[RD_COLORS], mesh->colors, mesh->num_verts * sizeof(bvec4_t));
					if (mesh->renderData[RD_TVERTS])
						Mem_Copy(mesh->renderData[RD_TVERTS], mesh->tverts, mesh->num_verts * sizeof(vec2_t));

					mesh->hasRenderData = true;
				}
			}
		}

		if (GL_MeshShouldDeform(obj, mesh) || !mesh->hasRenderData)
		{
			arrays->verts = GL_DynamicMalloc(mesh->num_verts * sizeof(vec3_t));
			arrays->normals = GL_DynamicMalloc(mesh->num_verts * sizeof(vec3_t));
			if (mesh->colors)
				arrays->colors = GL_DynamicMalloc(mesh->num_verts * sizeof(bvec4_t));
			else
				arrays->colors = NULL;
			arrays->tverts = GL_DynamicMalloc(mesh->num_verts * sizeof(vec2_t));

			if (!GL_MeshShouldDeform(obj, mesh))
			{
				//we haven't cached this in the static memory partition,
				//so copy the geometry over from system memory
				Mem_Copy(arrays->verts, mesh->verts, sizeof(vec3_t) * mesh->num_verts);
				Mem_Copy(arrays->normals, mesh->normals, sizeof(vec3_t) * mesh->num_verts);
				Mem_Copy(arrays->colors, mesh->colors, sizeof(bvec4_t) * mesh->num_verts);
				Mem_Copy(arrays->tverts, mesh->tverts, sizeof(vec2_t) * mesh->num_verts);		
			}
		}
		else
		{
			//mesh has cached geometry
			arrays->verts = mesh->renderData[RD_VERTS];
			arrays->normals = mesh->renderData[RD_NORMALS];
			arrays->colors = mesh->renderData[RD_COLORS];
			arrays->tverts = mesh->renderData[RD_TVERTS];
		}

		OVERHEAD_COUNT(OVERHEAD_MODEL_SETUPMESHARRAYS);

		{
			OVERHEAD_INIT;		
				
			//enable the appropriate vertex arrays
			if (arrays->normals)
				GL_EnableClientState(GL_NORMAL_ARRAY);
			else
				GL_DisableClientState(GL_NORMAL_ARRAY);
			
			if (arrays->colors && gfx_useVertexColors.integer)
				GL_EnableClientState(GL_COLOR_ARRAY);
			else
				GL_DisableClientState(GL_COLOR_ARRAY);

			if (arrays->tverts)
				GL_EnableClientState(GL_TEXTURE_COORD_ARRAY);
			else
				GL_DisableClientState(GL_TEXTURE_COORD_ARRAY);

			GL_SetVertexPointers(arrays, false);

			OVERHEAD_COUNT(OVERHEAD_MODEL_SETUPVERTEXPOINTERS);
		}
	}
}




/*==========================

  GL_DrawStaticMesh

  draw a normal, non animating, non deforming mesh

 ==========================*/

bool	GL_DrawStaticMesh(mesh_t *mesh)
{
	if (gfx_drawMeshes.integer)
		GL_DrawTriangles(GL_TRIANGLES, mesh->num_faces * 3, mesh->facelist);

	if (gfx_drawNormals.integer && mesh->normals)
	{
		int n;

		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glDisable(GL_LIGHTING);
		GL_DisableTextures();	

		glColor4f(1,1,1,1);

		glLineWidth(gfx_normalWidth.value);

		for (n=0; n<mesh->num_verts; n++)
		{
			vec3_t end;

			M_PointOnLine(mesh->verts[n], mesh->normals[n], gfx_normalLength.value, end);

			glBegin(GL_LINES);
			glVertex3fv(mesh->verts[n]);
			glVertex3fv(end);
			glEnd();

			glPointSize(8);
			glBegin(GL_POINTS);
			glVertex3fv(mesh->verts[n]);
			glEnd();
		}

		glPopAttrib();
	}

	return true;
}


static int *mapping = NULL;


/*==========================

  GL_DrawRigidMesh

  draw a mesh that animates its position/orientation but does not deform

 ==========================*/

bool	GL_DrawRigidMesh(sceneobj_t *obj, mesh_t *mesh)
{
	matrix43_t *tm;
	float gl_tm[16];
	int mdlbone = mesh->bonelink;
	int skelbone = mapping[mdlbone];

	OVERHEAD_INIT;

	//we expect an obj->skeleton structure here
	if (!obj->skeleton)
	{
		Console_DPrintf("GL_DrawMesh: mesh %s in model %s has no skeleton\n", mesh->name, mesh->model->name);
		return false;
	}
	if (skelbone == -1 || skelbone >= obj->skeleton->numBones)
	{
		Console_DPrintf("GL_DrawMesh: no bone found in skeleton for mesh %s in model %s\n", mesh->name, mesh->model->name);
		return false;
	}

	//mesh is invisible on this frame
	if (obj->skeleton->bones[skelbone].visibility == 0)
		return false;

	glPushMatrix();

	//transform the bone to model space
	tm = &obj->skeleton->bones[skelbone].tm_world;
		
	GL_TransformToMatrix(tm, gl_tm);
	glMultMatrixf(gl_tm);

	//vertices are in model coordinates, so we have to multiply them
	//by the inverse of the base bone transform
	GL_TransformToMatrix(&mesh->model->bones[mdlbone].invBase, gl_tm);
	glMultMatrixf(gl_tm);

	if (mesh->model->bones[mdlbone].flippedParity)
		glFrontFace(GL_CW);

	GL_DrawStaticMesh(mesh);

	if (mesh->model->bones[mdlbone].flippedParity)
		glFrontFace(GL_CCW);

	glPopMatrix();

	OVERHEAD_COUNT(OVERHEAD_MODEL_DRAWRIGIDMESH);

	return true;
}




/*==========================

  GL_DrawWireTriangles

 ==========================*/

vec4_t influenceColors[8] = 
{
	{ 1,	1,		1,		1 },
	{ 1,	0.8,	0.8,	1 },
	{ 1,	0.6,	0.6,	1 },
	{ 1,	0.4,	0.4,	1 },
	{ 1,	0.2,	0.2,	1 },
	{ 1,	0,		0,		1 },
	{ 1,	0,		0,		1 },
	{ 1,	0,		0,		1 }
};


void	GL_DrawWireTriangles(int primtype, int numelements, void *elemlist)
{	
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glDisable(GL_LIGHTING);
	GL_DisableTextures();	

	glColor4f(1,1,1,gfx_wireAlpha.value);
	glLineWidth(2);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	GL_DisableClientState(GL_COLOR_ARRAY);
	GL_DrawTriangles(primtype, numelements, elemlist);	

	glPopAttrib();
}


/*==========================

  GL_DrawSkinnedMesh

  draw a skeletally deformed mesh, non blended (one vertex per bone),
  or blended (multiple vertices per bone), depending on the mesh
  data itself and the value of gfx_forceNonBlendedDeform  

 ==========================*/


bool	GL_DrawSkinnedMesh(sceneobj_t *obj, mesh_t *mesh, gl_arrays_t *gpu_arrays)
{
	OVERHEAD_INIT;

	if (!obj->skeleton)
	{
		Console_DPrintf("GL_DrawSkinnedMesh: mesh %s in model %s has no skeleton\n", mesh->name, mesh->model->name);
		return false;
	}

	if (mesh->mode != MESH_SKINNED_BLENDED && mesh->mode != MESH_SKINNED_NONBLENDED)
	{
		Console_DPrintf("GL_DrawSkinnedMesh: mesh %s in model %s has an invalid mode (mode==%i)\n", mesh->name, mesh->model->name, mesh->mode);
		return false;
	}

#ifdef USE_VBO
	if (gl_ext_vertex_buffer_object.integer)
	{
		//using VBO, map the vertex stream that has been set up in GL_SetupMeshArrays()
		gpu_arrays->verts = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
		if (!gpu_arrays->verts)
		{
			Console_DPrintf("GL_DrawSkinnedMesh: GL out of memory error\n");
			return false;
		}
	}
#endif //USE_VBO

	if (gfx_skipDeform.integer)
	{
		Mem_Copy(gpu_arrays->verts, mesh->verts, sizeof(vec3_t) * mesh->num_verts);		
	}
	else if (mesh->mode == MESH_SKINNED_BLENDED && !gfx_forceNonBlendedDeform.integer)
	{		
		//blended deformation

		int v, w;
		boneState_t *skel_tms;
		model_t *model = mesh->model;		

		OVERHEAD_INIT;

		skel_tms = obj->skeleton->bones;		

		for (v=0; v<mesh->num_verts; v++)
		{
			vec3_t final;
			vec3_t p1,p2,p3;
			int index;
			bone_t *bone;
			matrix43_t *invbase, *tm_world;

			blendedLink_t *link = &mesh->blendedLinks[v];

			M_ClearVec3(final);			

			for (w=0; w<link->num_weights; w++)
			{	
				index = link->indexes[w];

				if (mapping[index] == -1)
				{
					M_CopyVec3(mesh->verts[v], final);
					break;
				}

#ifdef MODEL_PARANOIA
				if (index >= model->num_bones)
				{
					Console_DPrintf("model->num_bones is %i but we're trying to access index %i\n", model->num_bones, index);
					continue;
				}	
#endif
				
				invbase = &model->bones[index].invBase;
				tm_world = &skel_tms[mapping[index]].tm_world;

				bone = &model->bones[index];
				
				M_TransformPoint(mesh->verts[v], invbase->pos, (const vec3_t *)invbase->axis, p1);
				M_TransformPoint(p1, tm_world->pos, (const vec3_t *)tm_world->axis, p2);
				M_ScaleVec3(p2, link->weights[w], p3);
				M_AddVec3(final, p3, final);
			}

			M_CopyVec3(final, gpu_arrays->verts[v]);			
		}

		OVERHEAD_COUNT(OVERHEAD_MODEL_DEFORM);
	}	
	else
	{
		//non blended deformation

		int v;
		boneState_t *skel_tms = obj->skeleton->bones;		

		OVERHEAD_INIT;
		
		for (v=0; v<mesh->num_verts; v++)
		{	
			int boneidx;
			matrix43_t *tm_world;			

#ifdef MODEL_PARANOIA
			if (mesh->singleLinks[v] >= mesh->model->num_bones)
			{
				Console_DPrintf("Error: model->num_bones is %i but we're trying to access index %i\n", mesh->model->num_bones, mesh->singleLinks[v]);
				continue;
			}			
#endif
			
			boneidx = mapping[mesh->singleLinks[v]];
			if (boneidx == -1)
			{				
				M_CopyVec3(mesh->verts[v], gpu_arrays->verts[v]);
				continue;
			}
			
			tm_world = &skel_tms[boneidx].tm_world;
			//use the bone space vertices to skip the vert * invbase matrix multiply
			M_TransformPoint(mesh->boneSpaceVerts[v], tm_world->pos, (const vec3_t *)tm_world->axis, gpu_arrays->verts[v]);			
		}	
			
		OVERHEAD_COUNT(OVERHEAD_MODEL_DEFORM);
	}		

	if (!gl_ext_vertex_buffer_object.integer)
	{
		//copy the static data when we're not using VBO
		Mem_Copy(gpu_arrays->normals, mesh->normals, sizeof(vec3_t) * mesh->num_verts);
		Mem_Copy(gpu_arrays->colors, mesh->colors, sizeof(bvec4_t) * mesh->num_verts);
		Mem_Copy(gpu_arrays->tverts, mesh->tverts, sizeof(vec2_t) * mesh->num_verts);
	}
#ifdef USE_VBO
	else
	{
		//unmap the vertex buffer
		if (!glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
		{
			Console_DPrintf("GL_DrawSkinnedMesh: corrupt vertex stream\n");
			return false;
		}
	}
#endif //USE_VBO

	if (gfx_drawMeshes.integer)
	{
		GL_DrawTriangles(GL_TRIANGLES, mesh->num_faces * 3, mesh->facelist);
	}

	if (mesh->mode == MESH_SKINNED_BLENDED && gfx_showWeights.integer && !gfx_forceNonBlendedDeform.integer)
	{
		int v, w;
		boneState_t *skel_tms;
		model_t *model = mesh->model;		

		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glPointSize(8);		
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		GL_SwitchTexUnit(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		GL_SwitchTexUnit(GL_TEXTURE0_ARB);
		glDisable(GL_LIGHTING);

		glBegin(GL_POINTS);
		
		skel_tms = obj->skeleton->bones;

		for (v=0; v<mesh->num_verts; v++)
		{
			vec3_t final;
			vec3_t p1,p2,p3;
			int index;
			bone_t *bone;

			blendedLink_t *link = &mesh->blendedLinks[v];

			M_ClearVec3(final);

			for (w=0; w<link->num_weights; w++)
			{				
				index = link->indexes[w];
				if (mapping[index] == -1)
				{
					M_CopyVec3(mesh->verts[v], final);
					break;
				}

				bone = &model->bones[index];
				
				M_VectorMatrixMult(mesh->verts[v], &model->bones[index].invBase, p1);
				M_VectorMatrixMult(p1, &skel_tms[mapping[index]].tm_world, p2);
				M_ScaleVec3(p2, link->weights[w], p3);
				M_AddVec3(final, p3, final);			
			}

			M_CopyVec3(final, gpu_arrays->verts[v]);

			if (link->num_weights > 8)
				GL_SetColor(influenceColors[7]);
			else
				GL_SetColor(influenceColors[link->num_weights-1]);

			glVertex3fv(final);			
		}

		glEnd();

		glPopAttrib();
	}

	OVERHEAD_COUNT(OVERHEAD_MODEL_DRAWSKINNEDMESH);

	return true;
}


sceneobj_t *last_obj = NULL;
mesh_t	*last_mesh = NULL;

/*==========================

  GL_DrawMesh

  the general mesh drawing function, which selects the appropriate function above

  ==========================*/

void	GL_DrawMesh(sceneobj_t *obj, mesh_t *mesh)
{
	bool ret;
	gl_arrays_t gpu_arrays;

	OVERHEAD_INIT;	

	glPushMatrix();

	GL_SetupMeshArrays(obj, mesh, &gpu_arrays);	

	if (gfx_drawModelBounds.integer || obj->flags & SCENEOBJ_SHOW_BOUNDS)
	{
		//yeah, this will get drawn for as many meshes are in the model...so what
		GL_DrawBox(vec3(mesh->model->bmin[0],mesh->model->bmin[1],mesh->model->bmin[2]), vec3(mesh->model->bmax[0],mesh->model->bmax[1],mesh->model->bmax[2]));			
	}
	if (gfx_drawMeshBounds.integer)
	{
		GL_DrawBox(vec3(mesh->bmin[0],mesh->bmin[1],mesh->bmin[2]), vec3(mesh->bmax[0],mesh->bmax[1],mesh->bmax[2]));			
	}

	if (!obj->skeleton || gfx_alwaysStatic.integer)		//does not transform at all
	{
		OVERHEAD_INIT;
		ret = GL_DrawStaticMesh(mesh);
		OVERHEAD_COUNT(OVERHEAD_MODEL_DRAWSTATICMESH);
	}
	else
	{
		if (obj->custom_mapping)
			mapping = obj->custom_mapping;
		else
			mapping = mesh->model->bone_mapping;

		if (GL_MeshShouldDeform(obj, mesh))
		{			
			//a deforming (skinned) mesh
			ret = GL_DrawSkinnedMesh(obj, mesh, &gpu_arrays);	
		}
		else
		{
			ret = GL_DrawRigidMesh(obj, mesh);
		}
	}

	if (gfx_drawWire.integer || obj->flags & SCENEOBJ_SHOW_WIRE)
	{
		GL_DrawWireTriangles(GL_TRIANGLES, mesh->num_faces * 3, mesh->facelist);
	}

	glPopMatrix();

	if (ret)
	{
		last_obj = obj;
		last_mesh = mesh;
	}
	else
	{
		last_obj = NULL;
		last_mesh = NULL;
	}

	OVERHEAD_COUNT(OVERHEAD_MODEL_DRAWMESH);
}


/*==========================

  GL_RedrawMesh

  draw the same mesh again.  must be preceeded by a GL_DrawMesh() call

  used by the scene builder to do the translucent polygon depth trick

 ==========================*/

void	GL_RedrawMesh()
{
	if (!last_obj)
		return;
	if (!last_mesh)
		return;
	if (!gfx_drawMeshes.integer)
		return;

	if (GL_MeshShouldDeform(last_obj, last_mesh) || !last_obj->skeleton)
	{
		GL_DrawTriangles(GL_TRIANGLES, last_mesh->num_faces * 3, last_mesh->facelist);
	}
	else
	{
		GL_DrawRigidMesh(last_obj, last_mesh);
	}
}



/*==========================

  GL_DrawBoneChildren

  recursively draw a skeleton starting at the given root bone

 ==========================*/

void	GL_DrawBoneChildren(int boneidx, skeleton_t *skel, model_t *model)
{
	int n;
	
	if (gfx_drawBoneNames.integer)
	{
		vec3_t textpos;		

		GL_SetColor(white);

		GL_ProjectVertexInternal(skel->bones[boneidx].tm_world.pos, textpos);

		//get into an ortho projection to draw the string
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, Vid_GetScreenW(), Vid_GetScreenH(), 0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

//		glPointSize(50);
/*		glBegin(GL_QUADS);
		glVertex2f(textpos[0], textpos[1]+20);
		glVertex2f(textpos[0]+20, textpos[1]+20);
		glVertex2f(textpos[0]+20, textpos[1]);
		glVertex2f(textpos[0], textpos[1]+20);
		glEnd();
*/
		glEnable(GL_TEXTURE_2D);
		DU_DrawString(textpos[0], textpos[1], model->bones[boneidx].name, 16, 16, 1, 9999, Host_GetNicefontShader(), false);
		glDisable(GL_TEXTURE_2D);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	for (n=0; n<model->bones[boneidx].numChildren; n++)
	{
		int childidx = model->bones[boneidx].children[n]->index;
		GL_SetColor(vec4(1,1,1,gfx_boneAlpha.value));
		
		if (gfx_drawLinks.integer)
		{
			glBegin(GL_LINES);
			glVertex3fv(skel->bones[boneidx].tm_world.pos);
			glVertex3fv(skel->bones[childidx].tm_world.pos);
			glEnd();
		}
		
		GL_SetColor(vec4(0,0,1,gfx_boneAlpha.value));
		if (gfx_drawJoints.integer)
		{
			glBegin(GL_POINTS);
			glVertex3fv(skel->bones[boneidx].tm_world.pos);
			glVertex3fv(skel->bones[childidx].tm_world.pos);
			glEnd();
		}

		GL_DrawBoneChildren(childidx, skel, model);
	}
}


/*==========================

  GL_DrawSkeleton

  pretty slow, but only for debug use anyway

 ==========================*/

void	GL_DrawSkeleton(int boneidx, skeleton_t *skel, model_t *model)
{
	if (!skel->numBones)
		return;
	if (!skel)
		return;

	glPushAttrib(GL_ALL_ATTRIB_BITS);		

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glLineWidth(4);
	glPointSize(10);

	GL_DrawBoneChildren(boneidx, skel, model);

	glPopAttrib();
}





/*==========================

  GL_AddModelToLists

  tell gl_scene_builder that we want to queue a model for rendering
  
  also select which LOD to use

 ==========================*/

bool	GL_AddModelToLists(sceneobj_t *obj)
{
	vec3_t bmin;
	vec3_t bmax;
	//vec3_t p;
	model_t *model;
	int meshnum;	
	float dist;
	int n;
	//float z[4];

	OVERHEAD_INIT;
	

	if (!obj)
		return false;

	model = Res_GetModel(obj->model);

	if (gfx_useLOD.integer && model->num_lods)
	{
		vec3_t distvec;
		int lod = -1;

		M_SubVec3(cam->origin, obj->pos, distvec);
		dist = M_GetVec3Length(distvec);

		for (n=model->num_lods-1; n>=0; n--)
		{
			if (dist >= model->lods[n].distance)
			{			
				lod = n;				
				break;
			}						
		}

		if (gfx_forceLOD.integer)
		{
			lod += gfx_forceLOD.integer;
			if (lod > model->num_lods-1)
				lod = model->num_lods-1;
		}

		if (lod >= 0)
			model = Res_GetModel(model->lods[lod].modelidx);
	}
	else
	{
		model = Res_GetModel(obj->model);
	}

	if (!model)
		return false;

	M_AddVec3(model->bmin, obj->pos, bmin);
	M_AddVec3(model->bmax, obj->pos, bmax);


	//make sure skin index is valid
	if (obj->skin >= model->num_skins || obj->skin < 0)
		obj->skin = 0;

	//make sure the shaders we'll be using for the model are loaded
	Geom_LoadSkinShaders(model, obj->skin);

	if (obj->skeleton)
	{
		if (!obj->skeleton->numBones || obj->skeleton->invalid)
		{
			obj->skeleton = NULL;
		}
		else
		{			
			model_t *skelmodel = Res_GetModel(obj->skeleton->model);

			if (obj->skeleton->model != obj->model)
			{
				int i,j;
				model_t *objmodel = Res_GetModel(obj->model);
				int *custom_mapping = Scene_FramePool_Alloc(sizeof(int) * objmodel->num_bones);

				//we're using a skeleton from a different model
				//some sceneobjs do this (character armor for instance)

				//for the model to be deformed correctly, we'll need
				//to create a custom bone mapping.  this gets slow if
				//lots of models are doing this
				for (i=0; i<objmodel->num_bones; i++)
				{
					custom_mapping[i] = -1;

					for (j=0; j<skelmodel->num_bones; j++)
					{
						if (strcmp(objmodel->bones[i].name, skelmodel->bones[j].name)==0)
						{
							custom_mapping[i] = j;
							break;
						}					
					}
				}

				obj->custom_mapping = custom_mapping;
			}
			else
			{
				//make sure custom mapping is NULL
				obj->custom_mapping = NULL;
			}

			if (obj->skeleton->numBones != skelmodel->num_bones)
			{
				Console_DPrintf("GL_AddModelToLists(%p): invalid skeleton (skeleton->model != obj->model)\n");
				return false;
			}
		}
	}

	for (meshnum=0; meshnum<model->num_meshes; meshnum++)
	{
		mesh_t *mesh = &model->meshes[meshnum];
		residx_t shader;
		
		if (obj->flags & SCENEOBJ_SINGLE_SHADER)
			shader = obj->shader;
		else if (obj->skin < model->num_skins)
			shader = model->skins[obj->skin].shaders[meshnum];
		else
		{
			Console_DPrintf("Error!  Trying to load skin %i but there are only %i skins\n", obj->skin, model->num_skins);
			continue;
		}
		
		GL_AddMesh(shader, mesh, obj);
	}
/*
	for (spritenum=0; spritenum<model->num_sprites; spritenum++)
	{
		mdlsprite_t *sprite = &model->sprites[spritenum];
		shader_t *shader = Res_GetShader(Res_GetShaderFromSkin(obj->skin, sprite->name));

		GL_AddSprite(shader, sprite, obj);
	}
*/	

	OVERHEAD_COUNT(OVERHEAD_MODEL_ADDTOLISTS);

	return true;
}




/*==========================

  GL_InitModel

 ==========================*/

void	GL_InitModel()
{
	Cvar_Register(&gfx_useLOD);
	Cvar_Register(&gfx_forceLOD);
	Cvar_Register(&gfx_useVertexColors);
	Cvar_Register(&gfx_drawModelBounds);
	Cvar_Register(&gfx_drawMeshBounds);
	Cvar_Register(&gfx_drawBones);
	Cvar_Register(&gfx_drawJoints);
	Cvar_Register(&gfx_drawLinks);
	Cvar_Register(&gfx_drawMeshes);
	Cvar_Register(&gfx_skipDeform);
	Cvar_Register(&gfx_forceNonBlendedDeform);
	Cvar_Register(&gfx_showWeights);		
	Cvar_Register(&gfx_drawNormals);
	Cvar_Register(&gfx_drawWire);
	Cvar_Register(&gfx_normalLength);
	Cvar_Register(&gfx_normalWidth);
	Cvar_Register(&gfx_boneAlpha);
	Cvar_Register(&gfx_drawBoneNames);
	Cvar_Register(&gfx_wireAlpha);
	Cvar_Register(&gfx_allowMismatchedBones);
	Cvar_Register(&gfx_alwaysStatic);

	M_Identity(&identity);

#ifdef USE_VBO
	if (gl_ext_vertex_buffer_object.integer)
	{
		//initialize a streaming vertex buffer
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexStream);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, 65536, NULL, GL_STREAM_DRAW_ARB);
	}
#endif
}
