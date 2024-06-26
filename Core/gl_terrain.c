// (C) 2001 S2 Games

// gl_terrain.c

// Terrain Renderer


//define this to render terrain as triangle lists instead of strips
//#define TERRAIN_TRIANGLE_LIST

#define	NUM_TERRAIN_LAYERS			3
#define	MAX_CHUNKS_X				32
#define MAX_CHUNKS_Y				32
#define MAX_CHUNK_SIZE				255
#define MAX_CHUNK_SIZE				255

#define	TERRAIN_SHARE_VERTICES		0x0001
#define TERRAIN_SHARE_COLORS		0x0002
#define	TERRAIN_SHARE_TEXCOORDS		0x0004
#define	TERRAIN_SHARE_NORMALS		0x0008

#define TERRAIN_SHARE_ALL			(0x0001 | 0x0002 | 0x0004 | 0x0008)


#include "core.h"

#ifdef _WIN32
#include "glh_genext.h"
#endif


extern cvar_t gfx_radeon;

cvar_t	terrain_mem_usage = { "terrain_mem_usage", "0", CVAR_READONLY };
cvar_t	terrain_shownormals = { "terrain_shownormals", "0", CVAR_CHEAT };
cvar_t	terrain_layer1_scale = { "terrain_layer1_scale", "1", CVAR_WORLDCONFIG | CVAR_TERRAIN_FX };
cvar_t	terrain_layer2_scale = { "terrain_layer2_scale", "1", CVAR_WORLDCONFIG | CVAR_TERRAIN_FX };
cvar_t	terrain_layer1 = { "terrain_layer1", "1", CVAR_WORLDCONFIG | CVAR_CHEAT };
cvar_t	terrain_layer2 = { "terrain_layer2", "1", CVAR_WORLDCONFIG | CVAR_CHEAT };
cvar_t	terrain_dynamicLight = { "terrain_dynamicLight", "1", CVAR_WORLDCONFIG };
cvar_t	terrain_minslope = { "terrain_minslope", "0.7", CVAR_SAVECONFIG };
cvar_t	terrain_nocull = { "terrain_nocull", "0" };
cvar_t	terrain_wireframe = { "terrain_wireframe", "0", CVAR_CHEAT };

vec3_t	minitree_middle_bmin[MAX_CHUNKS_Y/2][MAX_CHUNKS_X/2];
vec3_t	minitree_middle_bmax[MAX_CHUNKS_Y/2][MAX_CHUNKS_X/2];
vec3_t	minitree_top_bmin[MAX_CHUNKS_Y/4][MAX_CHUNKS_X/4];
vec3_t	minitree_top_bmax[MAX_CHUNKS_Y/4][MAX_CHUNKS_X/4];

extern cvar_t gl_ext_vertex_buffer_object;

extern cvar_t gfx_nvidia;

typedef struct
{
	residx_t		shader;

	unsigned int	*elemlist;
	unsigned int	num_elems;

	unsigned int	last_tx;	//for keeping track of when a strip ends
	unsigned int	last_ty;	//for keeping track of when a strip ends
	unsigned int	last_vert;
	unsigned int	num_strips;
} gl_terrain_arrays_t;

typedef struct
{
	int			num_arrays;

	gl_terrain_arrays_t *arrays[256];

	int			validityFlags;
	bool		visible;

	vec3_t		*verts;
	vec2_t		*tverts;
	vec3_t		*normals;
	bvec4_t		*colors;
	bvec4_t		*paddedColors;
	
	//vertex buffer
	GLuint			buffer;

	vec3_t		bmin;
	vec3_t		bmax;
} gl_terrain_chunk_t;


typedef struct
{
	gl_terrain_chunk_t	chunks[MAX_CHUNKS_Y][MAX_CHUNKS_X];

	void				(*StartChunkRebuilding)(int layer, int chunkx, int chunky, int flags);
	residx_t 			(*GetShader)(int layer, int tx, int ty);

	void				(*RenderTerrainChunk)(int layer, int chunkx, int chunky);

	int					sharingFlags;
} gl_terrain_layer_t;


typedef struct
{
	unsigned int		normalsOffset;
	unsigned int		normalsSize;
	unsigned int		vertsOffset;
	unsigned int		vertsSize;
	unsigned int		colorsOffset;
	unsigned int		colorsSize;
	unsigned int		tvertsOffset;
	unsigned int		tvertsSize;
	unsigned int		totalSize;
	unsigned int		paddedColorsOffset;		//radeon hack	

	int					chunkSize;
	int					chunks_x;
	int					chunks_y;

	gl_terrain_layer_t	layers[NUM_TERRAIN_LAYERS];
} gl_terrain_t;

gl_terrain_t terrain;

GLuint	terrainStream;

#define	VERTS_PER_CHUNK	((terrain.chunkSize+1) * (terrain.chunkSize+1))
#define	MAX_ELEMS_PER_CHUNK (terrain.chunkSize * terrain.chunkSize * 6)



void	*GL_AllocTerrainArray()
{
	gl_terrain_arrays_t *newarray;

	newarray = Tag_Malloc(sizeof(gl_terrain_arrays_t), MEM_VIDDRIVER);
	newarray->num_elems = 0;
	newarray->elemlist = Tag_Malloc(MAX_ELEMS_PER_CHUNK * sizeof(unsigned int), MEM_VIDDRIVER);

	return newarray;
}


void	GL_FreeChunkElems(gl_terrain_chunk_t *chunk)
{
	int n;

	for (n=0; n<chunk->num_arrays; n++)
	{
		Tag_Free(chunk->arrays[n]->elemlist);
		Tag_Free(chunk->arrays[n]);
	}
	chunk->num_arrays = 0;
}

void	GL_FreeChunkVertexArrays(gl_terrain_chunk_t *chunk)
{
	if (gl_ext_vertex_buffer_object.integer)
		return;

	Tag_Free(chunk->verts);
	Tag_Free(chunk->tverts);
	Tag_Free(chunk->colors);
	Tag_Free(chunk->normals);	
}


void	_GL_RebuildTerrainShaders(int layer, int startx, int starty, gl_terrain_chunk_t *chunk)
{
	//start num_arrays at 0 
	//set current_shader to NULL
	//set shader_list to empty
	//
	//for (tx, ty) to (tx+chunksize, ty+chunksize)
	//
	//	look at the shader at (tx, ty)
	//  if this shader is different from current_shader
	//		if this shader exists in shader_list
	//			set the current chunk->array to the one referenced by shader_list for this shader
	//		else if we can't find the shader in shader_list
	//			allocate a new chunk->array 
	//			add the current shader to shader_list	
	//			set the current chunk->array to the one we just allocated
	//
	//	write out all vertex data for (tx,ty), (tx,ty+1), (tx+1,ty)...etc...
	//		...for the 6 vertices that make up this terrain tile...
	//		...to the current chunk->array at chunk->array->pos
	//	increment chunk->array->pos
	//
	//continue for
	//
	//notes about allocating a new chunk->array:
	//	we always allocate the max possible number of vertices it could contain, so (chunksize * chunksize)
	
	int					tx, ty;
	residx_t			current_shader;
	int					cur_elem;
	gl_terrain_arrays_t *current_array = NULL;		
	residx_t			shader_list[256];
	int					shader_list_size = 0;
	residx_t			last_shader = -1;
	int					vert_pos = 0;

	GL_FreeChunkElems(chunk);

	shader_list[0] = -1;

	for (ty = starty; ty < starty + terrain.chunkSize; ty++)
	{
		for (tx = startx; tx < startx + terrain.chunkSize; tx++)
		{			
			if (terrain.layers[layer].GetShader)
				current_shader = terrain.layers[layer].GetShader(layer, tx, ty);			
			else
				current_shader = 0;

			if (current_shader != last_shader)
			{
				int n;
				bool foundshader = false;

				for (n=0; n<shader_list_size; n++)
				{
					if (shader_list[n] == current_shader)
					{
						current_array = chunk->arrays[n];
						foundshader = true;
					}										
				}

				if (!foundshader)
				{
					//we've encountered a new shader
					chunk->arrays[chunk->num_arrays] = GL_AllocTerrainArray();
					chunk->arrays[chunk->num_arrays]->num_elems = 0;
					chunk->arrays[chunk->num_arrays]->shader = current_shader;

					shader_list[shader_list_size] = current_shader;

					current_array = chunk->arrays[chunk->num_arrays];

					chunk->num_arrays++;
					shader_list_size++;

#ifndef TERRAIN_TRIANGLE_LIST
/*					vert_pos = (ty-starty) * (terrain.chunkSize+1) + (tx-startx);

					current_array->elemlist[0] = vert_pos;
					current_array->elemlist[1] = vert_pos + (terrain.chunkSize+1);
					vert_pos+=2;
					current_array->num_elems = 2;*/
#endif
				}
			}

#ifdef TERRAIN_TRIANGLE_LIST
			vert_pos = (ty-starty) * (terrain.chunkSize+1) + (tx-startx);
			cur_elem = current_array->num_elems;			

			current_array->elemlist[cur_elem++] = vert_pos + (terrain.chunkSize+1);
			current_array->elemlist[cur_elem++] = vert_pos;
			current_array->elemlist[cur_elem++] = vert_pos + 1;
			current_array->elemlist[cur_elem++] = vert_pos + 1 + (terrain.chunkSize+1);
			current_array->elemlist[cur_elem++] = vert_pos + (terrain.chunkSize+1);
			current_array->elemlist[cur_elem++] = vert_pos + 1;

			current_array->num_elems = cur_elem;			

#else
			/*

				vert_pos+(chunksize+1)

			*/
			vert_pos = (ty-starty) * (terrain.chunkSize+1) + (tx-startx);
			cur_elem = current_array->num_elems;

			//if we're starting a new row or we ended an old tristrip...
			if (tx - current_array->last_tx != 1 || ty - current_array->last_ty != 0 || !current_array->last_vert)
			{
				if (current_array->last_vert)
				{
					//create a degenerate triangle to separate strips					
					current_array->elemlist[cur_elem++] = current_array->last_vert;
					current_array->elemlist[cur_elem++] = vert_pos;//+ (terrain.chunkSize+1);					
				}

				//start the new strip
				current_array->elemlist[cur_elem++] = vert_pos;
				current_array->elemlist[cur_elem++] = vert_pos + (terrain.chunkSize+1);
				current_array->num_strips++;
			}

			current_array->elemlist[cur_elem++] = vert_pos + 1;
			current_array->elemlist[cur_elem++] = vert_pos + 1 + (terrain.chunkSize+1);

			current_array->last_vert = vert_pos + 1 + (terrain.chunkSize+1);
			current_array->last_tx = tx;
			current_array->last_ty = ty;
			current_array->num_elems = cur_elem;			

#endif	
		}

#ifndef TERRAIN_TRIANGLE_LIST		
		//current_array->elemlist[cur_elem++] = current_array->elemlist[cur_elem-1];
#endif
	}
}



void	_GL_RebuildTerrainVertices(int layer, int startx, int starty, vec3_t *vertarray, gl_terrain_chunk_t *chunk);
void	_GL_RebuildTerrainTexCoords(int layer, int startx, int starty, vec2_t *texarray);
void	_GL_RebuildTerrainColors(int layer, int startx, int starty, bool padAlpha, bvec4_t *colorarray);
void	_GL_RebuildTerrainNormals(int layer, int startx, int starty, vec3_t *nmlarray);



//rebuild a square block of the terrain at the given layer
//assumes space has already been allocated for holding the vertex data
//this isn't meant to be particularly fast, but should be fast enough to keep an interactive framerate in the editor when modifying terrain
void	_GL_RebuildTerrainChunk(int layer, int chunkx, int chunky)
{
	int					startx, starty;
	
	gl_terrain_chunk_t *chunk = &terrain.layers[layer].chunks[chunky][chunkx];
	int					flags = chunk->validityFlags;
	int					sharingFlags = terrain.layers[layer].sharingFlags;
	void				*vertarray;
	void				*colorarray;
	void				*tvertarray;
	void				*nmlarray;
	void				*paddedcolorarray;

	if (sharingFlags & TERRAIN_SHARE_ALL)
		if (!(flags & TERRAIN_REBUILD_SHADERS))
			return;  //we share all elements with the base chunk and we're not rebuilding shaders, so return

	startx = chunkx * terrain.chunkSize;		//grid top-left tile coord
	starty = chunky * terrain.chunkSize;		//grid top-left tile coord

	

	if (!flags)
	{
		return;	
	}

#ifdef USE_VBO
	if (gl_ext_vertex_buffer_object.integer)
	{
		char *buf;

		if (!chunk->buffer)
		{
			glGenBuffersARB(1, &chunk->buffer);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->buffer);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, terrain.totalSize, NULL, GL_STATIC_DRAW_ARB);		//fixme: DYNAMIC_DRAW_ARB?
		}
		else
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->buffer);		
		}

		buf = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
		if (!buf)
		{
			Console_DPrintf("GL_RebuildTerrainChunk: GL out of memory\n");
			return;
		}

		vertarray = buf + terrain.vertsOffset;
		tvertarray = buf + terrain.tvertsOffset;
		nmlarray = buf + terrain.normalsOffset;
		colorarray = buf + terrain.colorsOffset;

		if (gfx_radeon.integer)
		{
			paddedcolorarray = buf + terrain.paddedColorsOffset;
		}
	}
	else
#endif //USE_VBO
	{
		vertarray = chunk->verts;
		colorarray = chunk->colors;
		tvertarray = chunk->tverts;
		nmlarray = chunk->normals;
	}

	if (terrain.layers[layer].StartChunkRebuilding)
		terrain.layers[layer].StartChunkRebuilding(layer, chunkx, chunky, flags);

	if (flags & TERRAIN_REBUILD_SHADERS)
		_GL_RebuildTerrainShaders(layer, startx, starty, chunk);

	if (flags & TERRAIN_REBUILD_VERTICES && vertarray)
		_GL_RebuildTerrainVertices(layer, startx, starty, vertarray, chunk);

	if (flags & TERRAIN_REBUILD_TEXCOORDS && tvertarray)
		_GL_RebuildTerrainTexCoords(layer, startx, starty, tvertarray);

	if (flags & TERRAIN_REBUILD_NORMALS && nmlarray)
		_GL_RebuildTerrainNormals(layer, startx, starty, nmlarray);

	if (flags & TERRAIN_REBUILD_COLORS && colorarray)
	{
		_GL_RebuildTerrainColors(layer, startx, starty, false, colorarray);

		if (gfx_radeon.integer && gl_ext_vertex_buffer_object.integer)
		{
			//radeon needs a duplicate color array with padded alpha
			_GL_RebuildTerrainColors(layer, startx, starty, true, paddedcolorarray);
		}
	}

#ifdef USE_VBO
	if (gl_ext_vertex_buffer_object.integer)
	{
		if (!glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
		{
			//shouldn't ever happen
			Console_DPrintf("GL_RebuildTerrainChunk: buffer corrupted\n");
		}
	}
#endif

	chunk->validityFlags = 0;
}

void	GL_RebuildTerrainChunk(int layer, int chunkx, int chunky)
{
	OVERHEAD_INIT;
	_GL_RebuildTerrainChunk(layer, chunkx, chunky);
	OVERHEAD_COUNT(OVERHEAD_TERRAIN_REBUILD);
}



//static tmu = 0;

void	GL_AllocateTerrainChunk(int layer, int x, int y)
{	

	gl_terrain_chunk_t *chunk = &terrain.layers[layer].chunks[y][x];

	memset(chunk, 0, sizeof(gl_terrain_chunk_t));	
	
	//allocate vertex array
	if (gl_ext_vertex_buffer_object.integer)
	{
		chunk->verts = NULL;
		chunk->tverts = NULL;
		chunk->colors = NULL;
		chunk->normals = NULL;
	}
	else
	{
		chunk->verts = Tag_Malloc(VERTS_PER_CHUNK * sizeof(vec3_t), MEM_VIDDRIVER);
		chunk->tverts = Tag_Malloc(VERTS_PER_CHUNK * sizeof(vec2_t), MEM_VIDDRIVER);
		chunk->colors = Tag_Malloc(VERTS_PER_CHUNK * sizeof(bvec4_t), MEM_VIDDRIVER);
		chunk->normals = Tag_Malloc(VERTS_PER_CHUNK * sizeof(vec3_t), MEM_VIDDRIVER);
	}

	chunk->validityFlags =	TERRAIN_REBUILD_VERTICES | 
							TERRAIN_REBUILD_TEXCOORDS |
							TERRAIN_REBUILD_COLORS |
							TERRAIN_REBUILD_NORMALS |
							TERRAIN_REBUILD_SHADERS;

	GL_RebuildTerrainChunk(layer, x, y);

//	Cvar_SetValue("terrain_mem_usage", tmu);
}

void	GL_DestroyTerrainChunk(int layer, int x, int y)
{
	gl_terrain_chunk_t *chunk = &terrain.layers[layer].chunks[y][x];

	GL_FreeChunkElems(chunk);	
	GL_FreeChunkVertexArrays(chunk);

#ifdef USE_VBO
	if (gl_ext_vertex_buffer_object.integer && chunk->buffer)
	{
		glDeleteBuffersARB(1, &chunk->buffer);
	}
#endif

	memset(chunk, 0, sizeof(gl_terrain_chunk_t));
}





void	GL_DestroyTerrain()
{
	int x, y, l;

	for (y=0; y<terrain.chunks_y; y++)
	{
		for (x=0; x<terrain.chunks_x; x++)
		{
			for (l=0; l<NUM_TERRAIN_LAYERS; l++)
				GL_DestroyTerrainChunk(l, x, y);			
		}
	}

//	tmu = 0;
}



void	GL_RebuildTerrain(int chunkSize)
{
	int x, y, l;
	unsigned int pos = 0;
	//unsigned int size = 0;

	if (chunkSize > world.gridwidth - 1)
		chunkSize = world.gridwidth - 1;

	GL_DestroyTerrain();

	if (world.gridheight % (chunkSize+1) != 0 ||
		world.gridwidth % (chunkSize+1) != 0)
	{
		System_Error("GL_RebuildTerrain: world dimensions were not a multiple of chunkSize+1\n");
		return;
	}

	terrain.chunks_x = world.gridwidth / (chunkSize+1);
	terrain.chunks_y = world.gridheight / (chunkSize+1);
	terrain.chunkSize = chunkSize;

#ifdef USE_VBO
	if (gl_ext_vertex_buffer_object.integer)
	{
		//initialize the buffer to the correct size
		terrain.vertsOffset = pos;
		terrain.vertsSize = VERTS_PER_CHUNK * sizeof(vec3_t);
		pos += (terrain.vertsSize + 31) & ~31;
		terrain.normalsOffset = pos;
		terrain.normalsSize = VERTS_PER_CHUNK * sizeof(vec3_t);
		pos += (terrain.normalsSize + 31) & ~31;
		terrain.colorsOffset = pos;
		terrain.colorsSize = VERTS_PER_CHUNK * sizeof(bvec4_t);
		pos += (terrain.colorsSize + 31) & ~31;
		terrain.tvertsOffset = pos;
		terrain.tvertsSize = VERTS_PER_CHUNK * sizeof(vec2_t);
		pos += (terrain.tvertsSize + 31) & ~31;
		if (gfx_radeon.integer)
		{
			//hack to get around colorpointer slowdown in radeon drivers
			terrain.paddedColorsOffset = pos;
			pos += (terrain.colorsSize + 31) & ~31;
		}
		terrain.totalSize = pos;
	}
#endif

	for (y=0; y<terrain.chunks_y; y++)
	{
		for (x=0; x<terrain.chunks_x; x++)
		{
			for (l=0; l<NUM_TERRAIN_LAYERS; l++)
			{
				GL_AllocateTerrainChunk(l, x, y);
			}
		}
	}
}


void	GL_SetTerrainLayerProperties(
	int layer,
	void				(*StartChunkRebuilding)(int layer, int chunkx, int chunky, int flags),
	residx_t			(*GetShader)(int layer, int tx, int ty),
	void				(*RenderTerrainChunk)(int layer, int chunkx, int chunky),
	int					sharingFlags
)
{
	if (layer < NUM_TERRAIN_LAYERS && layer >= 0)
	{
		terrain.layers[layer].StartChunkRebuilding = StartChunkRebuilding;
		//terrain.layers[layer].GetTileVerts = GetTileVerts;
		terrain.layers[layer].GetShader = GetShader;
		terrain.layers[layer].RenderTerrainChunk = RenderTerrainChunk;
		terrain.layers[layer].sharingFlags = sharingFlags;
	}
}



void	GL_InvalidateTerrainVertex(int layer, int tx, int ty, int flags)
{
	int chunkx, chunky;

	if (!terrain.chunkSize)
		return;

	chunkx = (tx) / terrain.chunkSize;
	chunky = (ty) / terrain.chunkSize;

	//check if we're right on a chunk border, 
	//in which case we'll have to invalidate more than one chunk

	if (tx % terrain.chunkSize != 0  &&  chunkx != 0)
		terrain.layers[layer].chunks[chunky-1][chunkx].validityFlags |= flags;

	if (ty % terrain.chunkSize != 0  &&  chunky != 0)
		terrain.layers[layer].chunks[chunky][chunkx-1].validityFlags |= flags;
	
	terrain.layers[layer].chunks[chunky][chunkx].validityFlags |= flags;
}

void	GL_InvalidateTerrainLayer(int layer, int flags)
{
	int x, y;

	for (y=0; y<terrain.chunks_y; y++)
	{
		for (x=0; x<terrain.chunks_x; x++)
		{
			terrain.layers[layer].chunks[y][x].validityFlags |= flags;
		}
	}

}

void	GL_FlagTerrainChunkForRendering(int chunkx, int chunky)
{
	terrain.layers[0].chunks[chunky][chunkx].visible = true;	
}



void	GL_FlagVisibleTerrainChunks()
{
	int x,y;

	for (x=0; x<terrain.chunks_x; x++)
	{
		for (y=0; y<terrain.chunks_y; y++)
		{
			gl_terrain_chunk_t *chunk = &terrain.layers[0].chunks[y][x];

			if (Scene_AABBIsVisible(chunk->bmin, chunk->bmax))
				GL_FlagTerrainChunkForRendering(x, y);
		}
	}
}

void	GL_RenderTerrainNormals();
void	GL_RenderPath();

extern cvar_t gl_ext_vertex_buffer_object;

void	GL_RenderTerrain()
{
	int chunkx, chunky, l;
	static int startx = 0;
	static int starty = 0;

	OVERHEAD_INIT;

	GL_Enable(GL_CULL_FACE);
	//change face winding for terrain
	glFrontFace(GL_CW);

	GL_FlagVisibleTerrainChunks();

	GL_EnableClientState(GL_VERTEX_ARRAY);
	GL_EnableClientState(GL_NORMAL_ARRAY);
	GL_EnableClientState(GL_COLOR_ARRAY);
	GL_EnableClientState(GL_TEXTURE_COORD_ARRAY);	

	for (chunky=starty; chunky<terrain.chunks_y; chunky++)
	{
		for (chunkx=startx; chunkx<terrain.chunks_x; chunkx++)
		{
			if (terrain.layers[0].chunks[chunky][chunkx].visible || terrain_nocull.integer)
			{
				for (l=0; l<NUM_TERRAIN_LAYERS; l++)
				{
					gl_terrain_chunk_t *chunk;
					
					chunk = &terrain.layers[l].chunks[chunky][chunkx];
					
					if (chunk->validityFlags)
					{
						GL_RebuildTerrainChunk(l, chunkx, chunky);						
					}						
				}
				
				for (l=0; l<NUM_TERRAIN_LAYERS; l++)
				{
					if (terrain.layers[l].RenderTerrainChunk)
						terrain.layers[l].RenderTerrainChunk(l, chunkx, chunky);					
				}
				
				terrain.layers[0].chunks[chunky][chunkx].visible = false;
			}
		}
	}

	if (cam->flags & CAM_WIREFRAME_TERRAIN)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (terrain_shownormals.value)
		GL_RenderTerrainNormals();

	GL_RenderPath();

	GL_Disable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	OVERHEAD_COUNT(OVERHEAD_TERRAIN_RENDER);
}





/*******************		VERTEX DATA CALLBACKS		*********************

  these are the only functions which interface to the core engine itself.
  the rest of the terrain code may operate independently of the core.
  fixme: move these to another module (i.e. gl_scene)

****************************************************************************/


int		startx, starty;
bvec4_t	lighting[MAX_CHUNK_SIZE+1][MAX_CHUNK_SIZE+1];
float	vertices[MAX_CHUNK_SIZE+1][MAX_CHUNK_SIZE+1];


void	_GL_RebuildTerrainVertices(int layer, int startx, int starty, vec3_t *vertarray, gl_terrain_chunk_t *chunk)
{
	int tx, ty;
	int vertpos = 0;
	int arrayidx0;
	float minz = 999999;
	float maxz = -999999;

	for (ty = starty; ty <= starty + terrain.chunkSize; ty++)
	{
		for (tx = startx; tx <= startx + terrain.chunkSize; tx++)
		{	
			int worldx, worldy;
			float z;

			arrayidx0 = GRIDREF(tx, ty);			

			worldx = GRID_TO_WORLD(tx);
			worldy = GRID_TO_WORLD(ty);

			z = world.grid[arrayidx0] - 0.1;
			
			SET_VEC3(vertarray[vertpos],     worldx,			worldy,				z);
			
			if (z < minz)
				minz = z;
			if (z > maxz)
				maxz = z;

			vertpos ++;
		}
	}

	chunk->bmin[2] = minz;
	chunk->bmax[2] = maxz;
	
	chunk->bmin[0] = GRID_TO_WORLD(startx);
	chunk->bmin[1] = GRID_TO_WORLD(starty);	
	chunk->bmax[0] = GRID_TO_WORLD(startx + terrain.chunkSize + 1);
	chunk->bmax[1] = GRID_TO_WORLD(starty + terrain.chunkSize + 1);	
}

void	_GL_RebuildTerrainTexCoords(int layer, int startx, int starty, vec2_t *texarray)
{
	int tx, ty;
	int vertpos = 0;
	float cycle_x = terrain.chunkSize;
	float cycle_y = terrain.chunkSize;	

	for (ty = 0; ty <= terrain.chunkSize; ty++)
	{
		cycle_y += 1;
//		cycle_y -= (int)cycle_y;

		cycle_x = 0;
		for (tx = terrain.chunkSize; tx >= 0; tx--)
		{	
			cycle_x -= 1;
//			cycle_x -= (int)cycle_x;

			SET_VEC2(texarray[vertpos],     cycle_x,	cycle_y);
			
			vertpos ++;
		}
		
	}
}

void	_GL_RebuildTerrainNormals(int layer, int startx, int starty, vec3_t *nmlarray)
{
	int tx, ty;
	int vertpos = 0;
	int gridx, gridy;
	int arrayidx0;

	for (ty = starty; ty <= starty + terrain.chunkSize; ty++)
	{
		gridy = ty - starty;

		for (tx = startx; tx <= startx + terrain.chunkSize; tx++)
		{
			arrayidx0 = GRIDREF(tx, ty);

			gridx = tx - startx;

			M_CopyVec3(world.normal[0][arrayidx0],		nmlarray[vertpos]);
			
			vertpos ++;
		}
	}
}

void	_GL_RebuildTerrainColors(int layer, int startx, int starty, bool padAlpha, bvec4_t *colorarray)
{
	int tx, ty;
	int vertpos = 0;
	int gridx, gridy;

	for (ty = starty; ty <= starty + terrain.chunkSize; ty++)
	{		
		gridy = ty - starty;

		for (tx = startx; tx <= startx + terrain.chunkSize; tx++)
		{	
			gridx = tx - startx;

			SET_VEC4(colorarray[vertpos],		lighting[gridy][gridx][0], 
												lighting[gridy][gridx][1], 
												lighting[gridy][gridx][2],												
												padAlpha ? 255 : lighting[gridy][gridx][3]);



			vertpos ++;
		}
	}
}

residx_t	GL_GetTerrainShaderLayer1(int layer, int tx, int ty)
{
	return WR_GetShaderAt(0, tx, ty);
}

residx_t	GL_GetTerrainShaderLayer2(int layer, int tx, int ty)
{
	return WR_GetShaderAt(1, tx, ty);
}

//we use this function to optimize retrieving vertex data through the callbacks
//fixme: don't use the gridref macros, use an 'arrayidx' method
void	GL_StartChunkRebuilding(int layer, int chunkx, int chunky, int flags)
{
	int x, y;		

	if (!(flags & TERRAIN_REBUILD_COLORS) &&
		!(flags & TERRAIN_REBUILD_VERTICES))
		return;

	startx = chunkx * terrain.chunkSize;
	starty = chunky * terrain.chunkSize;	
	
	for (y=0; y<terrain.chunkSize+1; y++)
	{
		for (x=0; x<terrain.chunkSize+1; x++)
		{
			if (flags & TERRAIN_REBUILD_COLORS)
			{
				vec3_t col;			

				WR_CalcColor(startx + x, starty + y, col);
				//M_MultVec3(col, vid_dynamicMultiplier.value, col);
				//Scene_ClampLightedColor(col);
		
				lighting[y][x][0] = col[0] * 255.0;
				lighting[y][x][1] = col[1] * 255.0;
				lighting[y][x][2] = col[2] * 255.0;
				lighting[y][x][3] = wr.colormap[GRIDREF(startx+x,starty+y)][3];				
			}
		}
	}
}

extern cvar_t navrep_render;
//extern cvar_t path_process_render;

extern void NavRep_Render();
//extern void Path_Render();

void	GL_RenderPath()
{
	if ( navrep_render.integer != -1 )
	{
		vec4_t white = {1,1,1,1};

		GL_Disable(GL_TEXTURE_2D);
		GL_Disable(GL_LIGHTING);
		GL_SetColor(white);

		glBegin(GL_LINES);
			NavRep_Render();
		glEnd();
	}
/*
	if ( path_process_render.integer )
	{
		vec4_t white = {1,1,1,1};

		GL_Disable(GL_TEXTURE_2D);
		GL_Disable(GL_LIGHTING);
		GL_SetColor(white);

		glBegin(GL_LINES);
			Path_Process_Render();
		glEnd();
	}
*/
}

void	GL_RenderTerrainNormals()
{
	int tx, ty;
	vec4_t white = {1,1,1,1};
	vec4_t toosteep = { 1,0,1,1 };

	GL_Disable(GL_TEXTURE_2D);
	GL_Disable(GL_LIGHTING);
	GL_SetColor(white);

	glBegin(GL_LINES);

	for (ty=0; ty<world.gridheight-1; ty++)
	{
		for (tx=0; tx<world.gridwidth-1; tx++)
		{
			vec3_t nml;
			pointinfo_t pi;
			int wx, wy;
			
			wx = GRID_TO_WORLD(tx);
			wy = GRID_TO_WORLD(ty);

			World_SampleGround(wx+25, wy+25, &pi);

			WR_GetNormal(tx, ty, nml, 1);
			if (nml[2] < terrain_minslope.value)
				GL_SetColor(toosteep);
			else
				GL_SetColor(white);
			glVertex3f(wx+25, wy+25, pi.z);
			glVertex3f(wx+25+nml[0]*40, wy+25+nml[1]*40, pi.z+nml[2]*40);

			World_SampleGround(wx+75, wy+75, &pi);
			WR_GetNormal(tx, ty, nml, 0);
			if (nml[2] < terrain_minslope.value)
				GL_SetColor(toosteep);
			else
				GL_SetColor(white);
			glVertex3f(wx+75, wy+75, pi.z);
			glVertex3f(wx+75+nml[0]*40, wy+75+nml[1]*40, pi.z+nml[2]*40);

		}
	}

	glEnd();
}



void	GL_BeginLayer2()
{	
	if (gfx_fog.value)
		GL_Enable(GL_FOG);

	if (cam->flags & CAM_WIREFRAME_TERRAIN)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GL_Enable(GL_DEPTH_TEST);

	//if we're drawing layer1, don't bother doing a zwrite
	if (terrain_layer1.integer)
		GL_DepthMask(GL_FALSE);
	else
		GL_DepthMask(GL_TRUE);

	if (terrain_dynamicLight.integer)
		GL_Enable(GL_LIGHTING);
	else
		GL_Disable(GL_LIGHTING);

	//change the depth func for layer 2
	GL_DepthFunc(GL_LEQUAL);

	if (cam->flags & CAM_WIREFRAME_TERRAIN || terrain_wireframe.integer)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		GL_Disable(GL_TEXTURE_2D);
	}
	else
	{
		GL_Enable(GL_TEXTURE_2D);
	}
}

void	GL_EndLayer2()
{
	GL_DepthMask(GL_TRUE);

	GL_DepthFunc(GL_LESS);

	if (cam->flags & CAM_WIREFRAME_TERRAIN || terrain_wireframe.integer)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		GL_Enable(GL_TEXTURE_2D);
	}
}



void	GL_BeginLayer1()
{	
	if (gfx_fog.value)
		GL_Enable(GL_FOG);

	GL_Enable(GL_DEPTH_TEST);

	if (terrain_dynamicLight.integer)
		GL_Enable(GL_LIGHTING);
	else
		GL_Disable(GL_LIGHTING);

	GL_Enable(GL_BLEND);	

		if (cam->flags & CAM_WIREFRAME_TERRAIN || terrain_wireframe.integer)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		GL_Disable(GL_TEXTURE_2D);
	}
	else
	{
		GL_Enable(GL_TEXTURE_2D);
	}

}


void	GL_EndLayer1()
{
	if (cam->flags & CAM_WIREFRAME_TERRAIN || terrain_wireframe.integer)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		GL_Enable(GL_TEXTURE_2D);
	}
}






void	GL_RenderTerrainChunk(int layer, int chunkx, int chunky)
{
	int n;
	int numverts = VERTS_PER_CHUNK;
	gl_terrain_chunk_t *chunk = &terrain.layers[1].chunks[chunky][chunkx];
	gl_terrain_chunk_t *basechunk = &terrain.layers[0].chunks[chunky][chunkx];
	gl_arrays_t sys_arrays;
	gl_arrays_t gpu_arrays;	
	
	if (!gl_ext_vertex_buffer_object.integer)
	{
		sys_arrays.verts = basechunk->verts;
		sys_arrays.tverts = basechunk->tverts;
		sys_arrays.colors = basechunk->colors;
		sys_arrays.normals = basechunk->normals;

		if (gl_ext_vertex_array_range.integer)
		{
			//copy terrain arrays into the vertex array range if one is set
			GL_CopySystemArrays(&sys_arrays, &gpu_arrays, numverts);			
		}
		else
		{
			gpu_arrays = sys_arrays;
		}

		GL_SetVertexPointers(&gpu_arrays, true);
	}
#ifdef USE_VBO
	else
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, basechunk->buffer);

		(char *)gpu_arrays.verts = (char *)NULL + terrain.vertsOffset;
		(char *)gpu_arrays.normals = (char *)NULL + terrain.normalsOffset;
		(char *)gpu_arrays.tverts = (char *)NULL + terrain.tvertsOffset;
		if (gfx_radeon.integer)
		{
			(char *)gpu_arrays.colors = (char *)NULL + terrain.paddedColorsOffset;
			GL_SetVertexPointers(&gpu_arrays, false);
		}
		else
		{
			(char *)gpu_arrays.colors = (char *)NULL + terrain.colorsOffset;
			GL_SetVertexPointers(&gpu_arrays, true);
		}
	}
#endif

	glColor4f(1,1,1,1);	

	if (terrain_layer1.integer)
	{
		GL_BeginLayer1();


		for (n=0; n<basechunk->num_arrays; n++)
		{
		//	glBindTexture(GL_TEXTURE_2D, basechunk->arrays[n]->shader->texmaps[0]);
			GL_SelectShader(Res_GetShader(basechunk->arrays[n]->shader), cam->time);

#ifdef TERRAIN_TRIANGLE_LIST
			GL_DrawTriangles(GL_TRIANGLES, basechunk->arrays[n]->num_elems, basechunk->arrays[n]->elemlist);
#else
			GL_DrawTriangles(GL_TRIANGLE_STRIP, basechunk->arrays[n]->num_elems, basechunk->arrays[n]->elemlist);
#endif

		}			

		GL_EndLayer1();
	}
	
	if (terrain_layer2.integer)
	{
		GL_BeginLayer2();

#ifdef USE_VBO
		if (gfx_radeon.integer && gl_ext_vertex_buffer_object.integer)
		{
			(char *)gpu_arrays.colors = (char *)NULL + terrain.colorsOffset;			
		}
#endif //USE_VBO

		glColorPointer(4, GL_UNSIGNED_BYTE, 0, gpu_arrays.colors);		//use the alpha channel to blend the color		
		
		for (n=0; n<chunk->num_arrays; n++)
		{
			//glBindTexture(GL_TEXTURE_2D, chunk->arrays[n]->shader->texmaps[0]);

			GL_SelectShader(Res_GetShader(chunk->arrays[n]->shader), cam->time);

#ifdef TERRAIN_TRIANGLE_LIST
			GL_DrawTriangles(GL_TRIANGLES, chunk->arrays[n]->num_elems, chunk->arrays[n]->elemlist);
#else
			GL_DrawTriangles(GL_TRIANGLE_STRIP, chunk->arrays[n]->num_elems, chunk->arrays[n]->elemlist);
#endif

		}

		GL_EndLayer2();
	}

	GL_SetFence();
	

	/*

	if (!tris)
	{
		Console_DPrintf("GL_RenderTerrainChunk: tris fell out of cache\n");
		return;
	}

	if (cam->flags & CAM_NO_TERRAIN)
		return;

	if (!(cam->flags & CAM_NO_SHADERMAP) && terrain_layer1.integer)
	{
		for (n=0; n<basechunk->num_arrays; n++)
		{
			GL_RenderTris(	&basechunk->cacheIndex,
#ifdef TERRAIN_TRISTRIPS
							GL_TRIS_IGNORE_ALPHA | GL_TRIS_NO_TRANSFORM | GL_TRIS_STRIPS,
#elif defined TERRAIN_DEGENERATE_TRISTRIPS
							GL_TRIS_IGNORE_ALPHA | GL_TRIS_NO_TRANSFORM,
#endif
							basechunk->arrays[n]->elemlist,
							basechunk->arrays[n]->num_elems / 3,
							basechunk->arrays[n]->shader,
							cam->time,
							NULL,
							GL_ScaleLayer1Textures,
							GL_ScaleLayer1TexturesExit							
						);
		}
	}

	//now use alpha information to blend in the second layer

	if (!(cam->flags & CAM_NO_SHADERMAP2) && terrain_layer2.integer)
	{
		int renderFlags = 0;

		if (cam->flags & CAM_NO_SHADERMAP)
		{			
			renderFlags |= GL_TRIS_IGNORE_ALPHA;
		}
		else
		{
			renderFlags |= GL_TRIS_DEPTHFUNC_LEQUAL;
		}

		for (n=0; n<chunk->num_arrays; n++)    
		{
			GL_RenderTris(	&basechunk->cacheIndex, 
#ifdef TERRAIN_TRISTRIPS
							renderFlags | GL_TRIS_NO_TRANSFORM | GL_TRIS_STRIPS,
#elif defined TERRAIN_DEGENERATE_TRISTRIPS
							renderFlags | GL_TRIS_NO_TRANSFORM,
#endif
							chunk->arrays[n]->elemlist,
							chunk->arrays[n]->num_elems / 3,
							chunk->arrays[n]->shader,
							cam->time,
							NULL,
							GL_ScaleLayer2Textures,
							GL_ScaleLayer2TexturesExit
						);						
		}
	}
*/
}



void	GL_InitTerrain()
{	
	memset(&terrain, 0, sizeof(gl_terrain_t));
	Cvar_Register(&terrain_mem_usage);
	Cvar_Register(&terrain_shownormals);
	Cvar_Register(&terrain_layer1_scale); 
	Cvar_Register(&terrain_layer2_scale);
	Cvar_Register(&terrain_layer1);
	Cvar_Register(&terrain_layer2);
	Cvar_Register(&terrain_dynamicLight);
	Cvar_Register(&terrain_minslope);
	Cvar_Register(&terrain_nocull);
	Cvar_Register(&terrain_wireframe);

	//engine specific:
		
	GL_SetTerrainLayerProperties(
		0,
		GL_StartChunkRebuilding,
		//GL_GetTerrainTileVerts,
		GL_GetTerrainShaderLayer1,
		GL_RenderTerrainChunk,
		0
	);

	GL_SetTerrainLayerProperties(
		1,
		NULL,
		//NULL,
		GL_GetTerrainShaderLayer2,
		NULL,
		TERRAIN_SHARE_ALL
		//TERRAIN_SHARE_VERTICES | TERRAIN_SHARE_COLORS
	);

	GL_SetTerrainLayerProperties(
		2,
		NULL,
		NULL,
		NULL,
		TERRAIN_SHARE_ALL
	);
/*
	GL_SetTerrainLayerProperties(
		2,
		NULL,
		GL_GetTerrainWaterShader,
		NULL,

	);
*/

#ifdef USE_VBO
	if (gl_ext_vertex_buffer_object.integer)
	{
		//intiialize vertex buffer for streaming terrain data
		//glGenBuffersARB(1, &terrainStream);
		terrainStream = 1;
	}
#endif
}
