// (C) 2003 S2 Games

// gl_main.c
 
// main OpenGL functions


//#define USE_BUMP_MAPS		//undefine this to turn off bump mapping

#define GLH_EXT_SINGLE_FILE

#include "core.h"

#ifndef _S2_DONT_INCLUDE_GL

#ifdef __APPLE__
#include "SDL.h"
#endif


vec3_t bump_map_scale = {2.0, 2.0, 2.0};

#endif //_S2_DONT_INCLUDE_GL

extern cvar_t vid_overbright;
extern cvar_t vid_compressTextures;
extern cvar_t vid_writeCompressedTextures;
extern cvar_t vid_maxtexturesize;

cvar_t	gl_fence_status = { "gl_fence_status", "0"};
cvar_t  gl_use_texture_as_bumpmap = { "gl_use_texture_as_bumpmap", "0"};
cvar_t	ogl_noclamp = { "ogl_noclamp", "0" };
cvar_t	gfx_renderTranslucent = { "gfx_renderTranslucent", "1", CVAR_CHEAT };
cvar_t	gfx_renderOpaque = { "gfx_renderOpaque", "1", CVAR_CHEAT };
cvar_t	gl_minMemPerFence = { "gl_minMemPerFence", "16384" };
cvar_t	gfx_nvidia = { "gfx_nvidia", "0", CVAR_READONLY };
cvar_t	gfx_radeon = { "gfx_radeon", "0", CVAR_READONLY };
cvar_t	gfx_useVBO = { "gfx_useVBO", "1", CVAR_SAVECONFIG };
cvar_t	gfx_useVAR = { "gfx_useVAR", "1", CVAR_SAVECONFIG };
cvar_t	gfx_gammaMult = { "gfx_gammaMult", "1", CVAR_SAVECONFIG };
cvar_t	gfx_flush = { "gfx_flush", "0", CVAR_SAVECONFIG };

//int		gl_translucent_shader_index = 0;
cvar_t	gl_textures_loaded = { "gl_textures_loaded", "0"};

#ifndef _S2_DONT_INCLUDE_GL

gl_info_t gl;

/* ARB_texture_compression */
/* ARB_vertex_buffer_object */
#ifdef _WIN32
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC   glGetCompressedTexImageARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC    glCompressedTexImage2DARB;

PFNGLBINDBUFFERARBPROC				glBindBufferARB;
PFNGLDELETEBUFFERSARBPROC			glDeleteBuffersARB;
PFNGLGENBUFFERSARBPROC				glGenBuffersARB;
PFNGLISBUFFERARBPROC				glIsBufferARB;
PFNGLBUFFERDATAARBPROC				glBufferDataARB;
PFNGLBUFFERSUBDATAARBPROC			glBufferSubDataARB;
PFNGLGETBUFFERSUBDATAARBPROC		glGetBufferSubDataARB;
PFNGLMAPBUFFERARBPROC				glMapBufferARB;
PFNGLUNMAPBUFFERARBPROC				glUnmapBufferARB;
PFNGLGETBUFFERPARAMETERIVARBPROC	glGetBufferParameterivARB;
PFNGLGETBUFFERPOINTERVARBPROC		glGetBufferPointervARB;
#endif

extern cvar_t gfx;
extern cvar_t readCompressedTextures;
extern cvar_t vid_multisample;

shader_t	nullshader;
shader_t	*current_shader = &nullshader;

int blendmodeMapping[] = 
{
	GL_ONE,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_SRC_ALPHA
};


typedef struct
{
	bool	register_combiners_nv;
	bool	depth_test;
	bool	fog;
//	bool	texture_2d;
	bool	blend;
	bool	cull_face;
	bool	alpha_test;
	bool	polygon_smooth;
	bool	polygon_offset_fill;
//	bool	color_array;
//	bool	texture_coord_array;
//	bool	vertex_array;
	bool	lighting;
//	bool	normal_array;
	bool	vertex_array_range_nv;
	bool	light0;
	bool	light1;
	bool	light2;
	bool	light3;
	bool	light4;
	bool	light5;
	bool	light6;
	bool	light7;
	bool	multisample;

	int		texunit;		//current texture unit
} glStates_t;

glStates_t gls;

void		_GL_NullRegisterShaderCallback(shader_t *shader);

void		(*registerShaderCallback)(shader_t *shader) = _GL_NullRegisterShaderCallback;

int GL_RegisterShaderImages(shader_t *shader, int numLevels, int *levels, bitmap_t *images);










































/****************************  EXTENSIONS  ***************************

	  The following code handles all initialization of the OpenGL
		extensions we need.

 *********************************************************************/

//extension cvars
cvar_t	gl_ext_multitexture = { "gl_ext_multitexture", "0", CVAR_READONLY };
cvar_t	gl_ext_compiled_vertex_array = { "gl_ext_compiled_vertex_array", "0", CVAR_READONLY };
cvar_t	gl_ext_vertex_array_range = { "gl_ext_vertex_array_range", "0", CVAR_READONLY };
cvar_t	gl_ext_fence = { "gl_ext_fence", "0", CVAR_READONLY };
cvar_t	gl_ext_texenv_combine = { "gl_ext_texenv_combine", "0", CVAR_READONLY };
cvar_t	gl_ext_secondary_color = { "gl_ext_secondary_color", "0", CVAR_READONLY };
cvar_t	gl_ext_register_combiners = { "gl_ext_register_combiners", "0", CVAR_READONLY };
cvar_t	gl_ext_texture_env_dot3 = { "gl_ext_texture_env_dot3", "0", CVAR_READONLY };
cvar_t	gl_bump_mapping = { "gl_bump_mapping", "0", CVAR_READONLY };
cvar_t	gl_ext_separate_specular_color = { "gl_ext_separate_specular_color", "0", CVAR_READONLY };
cvar_t	gl_ext_texture_compression = { "gl_ext_texture_compression", "0", CVAR_READONLY };
cvar_t	gl_ext_vertex_buffer_object = { "gl_ext_vertex_buffer_object", "0", CVAR_READONLY };


#ifndef __APPLE__ //why does anyone require this?
#ifdef USE_NVIDIA_EXTENSIONS
	// NV_fence -- glext.h
	#ifndef GL_NV_fence
		#define GL_NV_fence
		#define GL_ALL_COMPLETED_NV               0x84F2
		#define GL_FENCE_STATUS_NV                0x84F3
		#define GL_FENCE_CONDITION_NV             0x84F4
		typedef void (APIENTRY * PFNGLDELETEFENCESNVPROC) (GLsizei n, const GLuint *fences);
		typedef void (APIENTRY * PFNGLGENFENCESNVPROC) (GLsizei n, GLuint *fences);
		typedef GLboolean (APIENTRY * PFNGLISFENCENVPROC) (GLuint fence);
		typedef GLboolean (APIENTRY * PFNGLTESTFENCENVPROC) (GLuint fence);
		typedef void (APIENTRY * PFNGLGETFENCEIVNVPROC) (GLuint fence, GLenum pname, GLint *params);
		typedef void (APIENTRY * PFNGLFINISHFENCENVPROC) (GLuint fence);
		typedef void (APIENTRY * PFNGLSETFENCENVPROC) (GLuint fence, GLenum condition);
	#endif //GL_NV_fence
#endif //USE_NVIDIA_EXTENSIONS
#endif //__APPLE__

//these are mostly for OS X
#ifndef GL_COMBINE_EXT
	#ifdef GL_COMBINE_ARB
		#define GL_COMBINE_EXT GL_COMBINE_ARB
	#endif
#endif
#ifndef GL_COMBINE_RGB_EXT
	#ifdef GL_COMBINE_RGB_ARB
		#define GL_COMBINE_RGB_EXT GL_COMBINE_RGB_ARB
	#endif
#endif
#ifndef GL_COMBINE_ALPHA_EXT
	#ifdef GL_COMBINE_ALPHA_ARB
		#define GL_COMBINE_ALPHA_EXT GL_COMBINE_ALPHA_ARB
	#endif
#endif
#ifndef GL_COMBINE_SCALE_EXT
	#ifdef GL_COMBINE_SCALE_ARB
		#define GL_COMBINE_SCALE_EXT GL_COMBINE_SCALE_ARB
	#endif
#endif
#ifndef GL_RGB_SCALE_EXT
	#ifdef GL_RGB_SCALE_ARB
		#define GL_RGB_SCALE_EXT GL_RGB_SCALE_ARB
	#endif
#endif


#ifndef GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_compression_s3tc
#endif

//#define GL_ARB_texture_compression

#ifdef _WIN32
#define GetProcAddressARB wglGetProcAddress
#elif __APPLE__
#define	GetProcAddressARB SDL_GL_GetProcAddress
#elif unix
#define	GetProcAddressARB glXGetProcAddressARB
#endif

#ifdef _WIN32
#	define GLH_EXT_SINGLE_FILE
#	include "glh_extensions.h"
#	include "glh_genext.h"
#else //unix
//glh_* is broken and dumb

//void (*glVertexArrayRangeNV)(GLsizei size, const GLvoid *pointer);

int glh_init_extension(char *ext)
{
	if (gl.extensions && ext && strstr(gl.extensions, ext))
		return true;
		
	return false;
}
#define glh_init_extentions glh_init_extention
#endif

bool GL_InitExt(const char *extname, cvar_t *var)
{
	Cvar_Register(var);

    if (!extname || !gl.extensions)
    {
        Cvar_SetVarValue(var, 0);
        Console_Printf("Extension %s not available\n", extname);
        return false;
    }

	if (glh_init_extension((char *)extname))
	{
		Cvar_SetVarValue(var, 1);
		Console_Printf("Enabling GL extension %s\n", extname);
		return true;
	}

	Cvar_SetVarValue(var, 0);
	Console_Printf("Extension %s not available\n", extname);
	return false;
}


void	GL_InitTextureCompression()
{
	bool ret = GL_InitExt("GL_ARB_texture_compression", &gl_ext_texture_compression);
	if (!ret || !gl_ext_texture_compression.integer)
		ret = GL_InitExt("GL_EXT_texture_compression_s3tc", &gl_ext_texture_compression);

#ifdef _WIN32

	//grab the function pointers

	if (ret)
	{
		glGetCompressedTexImageARB   =
			(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) 
			GetProcAddressARB("glGetCompressedTexImageARB");
		glCompressedTexImage2DARB    = 
			(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)    
			wglGetProcAddress("glCompressedTexImage2DARB");

		if (!glGetCompressedTexImageARB || !glCompressedTexImage2DARB)
		{
			System_Error("Couldn't get texture compression functions\n");
		}
	}
#else //unix code here
	if (ret)
	{
		if (!GetProcAddressARB("glGetCompressedTexImageARB") 
			|| !GetProcAddressARB("glCompressedTexImage2DARB"))
		{
			ret = false; //functions are null pointers
		}
	}

#endif

	if (!ret || !gl_ext_texture_compression.integer)
	{
		Cvar_SetVarValue(&vid_writeCompressedTextures, 0);
		Cvar_SetVarValue(&vid_compressTextures, 0);		
		Console_Printf("OpenGL: Extension GL_ARB_texture_compression not supported\n");
	}
	else
	{
		Console_Printf("OpenGL: s3tc seems to be supported\n");
	}
}


bool GL_InitVertexBufferObject()
{	
	if (GL_InitExt("GL_ARB_vertex_buffer_object", &gl_ext_vertex_buffer_object))
	{		
#ifdef _WIN32
		glBindBufferARB = 
			(PFNGLBINDBUFFERARBPROC)
			GetProcAddressARB("glBindBufferARB");
		glDeleteBuffersARB =
			(PFNGLDELETEBUFFERSARBPROC)
			GetProcAddressARB("glDeleteBuffersARB");
		glGenBuffersARB =
			(PFNGLGENBUFFERSARBPROC)
			GetProcAddressARB("glGenBuffersARB");
		glIsBufferARB =
			(PFNGLISBUFFERARBPROC)
			GetProcAddressARB("glIsBufferARB");
		glBufferDataARB =
			(PFNGLBUFFERDATAARBPROC)
			GetProcAddressARB("glBufferDataARB");
		glBufferSubDataARB =
			(PFNGLBUFFERSUBDATAARBPROC)
			GetProcAddressARB("glBufferSubDataARB");
		glGetBufferSubDataARB =
			(PFNGLGETBUFFERSUBDATAARBPROC)
			GetProcAddressARB("glGetBufferSubDataARB");
		glMapBufferARB =
			(PFNGLMAPBUFFERARBPROC)
			GetProcAddressARB("glMapBufferARB");
		glUnmapBufferARB =
			(PFNGLUNMAPBUFFERARBPROC)
			GetProcAddressARB("glUnmapBufferARB");
		glGetBufferParameterivARB =
			(PFNGLGETBUFFERPARAMETERIVARBPROC)
			GetProcAddressARB("glGetBufferParameterivARB");
		glGetBufferPointervARB =
			(PFNGLGETBUFFERPOINTERVARBPROC)
			GetProcAddressARB("glGetBufferPointervARB");
#endif
		return true;
	}

	return false;
}

void GL_InitExtensions()
{
	bool usingVBO = false;

	Cvar_Register(&gl_bump_mapping);

	//these need to get registered here because they may not get registered depending on the value of gfx_useVBO and gfx_useVAR
	Cvar_Register(&gl_ext_vertex_array_range);
	Cvar_Register(&gl_ext_vertex_buffer_object);

	if (gfx_useVBO.integer)
	{
		usingVBO = GL_InitVertexBufferObject();
	}

#ifdef USE_NVIDIA_EXTENSIONS
	if (gfx_useVAR.integer && !usingVBO)
	{
#ifdef __APPLE__
		if (GL_InitExt("GL_APPLE_vertex_array_range", &gl_ext_vertex_array_range))
		{
			if (!GL_InitExt("GL_APPLE_fence", &gl_ext_fence))
			{
				//this shouldn't happen, NV_fence should always be available as long as VAR is available
				Console_Printf("OpenGL: VAR without Fence!  Disabling VAR\n");
				Cvar_SetVarValue(&gl_ext_vertex_array_range, 0);
			}
		}
#else //not APPLE
		if (GL_InitExt("GL_NV_vertex_array_range", &gl_ext_vertex_array_range))
		{
			if (!GL_InitExt("GL_NV_fence", &gl_ext_fence))
			{
				//this shouldn't happen, NV_fence should always be available as long as VAR is available
				Console_Printf("OpenGL: VAR without Fence!  Disabling VAR\n");
				Cvar_SetVarValue(&gl_ext_vertex_array_range, 0);
			}
		}
#endif //APPLE
	}	
#endif

	GL_InitExt("GL_EXT_compiled_vertex_array", &gl_ext_compiled_vertex_array);
	GL_InitExt("GL_EXT_texture_env_combine", &gl_ext_texenv_combine);
	if (!gl_ext_texenv_combine.integer)
		GL_InitExt("GL_ARB_texture_env_combine", &gl_ext_texenv_combine);
	GL_InitExt("GL_ARB_multitexture", &gl_ext_multitexture);
	GL_InitExt("GL_EXT_secondary_color", &gl_ext_secondary_color);
	GL_InitExt("GL_EXT_separate_specular_color", &gl_ext_separate_specular_color);
	GL_InitExt("GL_NV_register_combiners", &gl_ext_register_combiners);
	GL_InitExt("GL_EXT_texture_env_dot3", &gl_ext_texture_env_dot3);

	GL_InitTextureCompression();
	
	if (gl_ext_multitexture.integer && gl_ext_secondary_color.integer && (gl_ext_register_combiners.integer || gl_ext_texture_env_dot3.integer))
		Cvar_SetVarValue(&gl_bump_mapping, 1);
}



















































/*********************        Memory Allocation       *********************/

/* 
  AGP allocation:

  GL_Malloc attempts to allocate the specified size from a partition.

  If the requested size is not available, fences are finished until the
  memory is free.

  These functions are a modified version of the NVidia AGP memory manager
  written by Sebastien Domine.  This version allows for multiple
  memory partitions, if desired.

*/


typedef struct gl_memlist_s
{
	byte					*ptr;
	unsigned int				fence_id;
	struct gl_memlist_s 	*next;
} gl_memlist_t;



typedef struct
{
	byte	*curPtr;

	byte	*endMem;

	byte	*startPartition;
	byte	*endPartition;

	gl_memlist_t	*listHead;
	gl_memlist_t	*listTail;
	gl_memlist_t	*listOriginator;

	int num_nodes;

	bool allowWraparound;
} gl_partition_t;

typedef struct
{
	byte	*memBlock;
	int		pos;
	int		size;
	int		lastPos;
} gl_staticPartition_t;

gl_partition_t	gl_dynamicPartition;
gl_staticPartition_t gl_staticPartition;


void	GL_InitPartition(gl_partition_t *part, byte *varMemory, int size, bool allowWraparound)
{
	part->curPtr = part->startPartition = varMemory;
	part->endMem = part->endPartition = varMemory + size;

	part->listOriginator = Tag_Malloc(sizeof(gl_memlist_t), MEM_VIDDRIVER);
	part->listOriginator->next = part->listOriginator;

#ifdef USE_NVIDIA_EXTENSIONS
	if (gl_ext_fence.integer)
		glGenFencesNV(1, &part->listOriginator->fence_id);	
#endif

	part->listOriginator->ptr = part->curPtr;

	part->listHead = part->listTail = part->listOriginator;
	
	part->num_nodes = 1;

	part->allowWraparound = allowWraparound;
}

void	GL_ShutdownPartition(gl_partition_t *part)
{
	int n;
	gl_memlist_t *list, *next;

	list = part->listOriginator;
	for (n=0; n < part->num_nodes; n++)
	{
		next = list->next;
#ifdef USE_NVIDIA_EXTENSIONS
		if (gl_ext_fence.integer)
			glDeleteFencesNV(1, &list->fence_id);
#endif
		Tag_Free(list);
		list = next;
	}

	part->listHead = part->listTail = part->listOriginator = NULL;
}


bool	_GL_SetFence(gl_partition_t *part)
{
	if (part->listTail->ptr > part->curPtr)
	{
		if (part->endMem - part->listTail->ptr + part->curPtr - part->startPartition < gl_minMemPerFence.integer)
			return false;
	}
	else
	{
		if (part->curPtr - part->listTail->ptr < gl_minMemPerFence.integer)
			return false;
	}

#ifdef USE_NVIDIA_EXTENSIONS
	//set the fence
	if (gl_ext_fence.integer)
#ifdef __APPLE__
		glSetFenceNV(part->listTail->fence_id);
#else //NOT APPLE
		glSetFenceNV(part->listTail->fence_id, GL_ALL_COMPLETED_NV);
#endif //APPLE
#endif //USE_NVIDIA_EXTENSIONS

	    // make sure we are not eating our own tail
   	if (part->listTail->next == part->listHead)
	{
		// need to insert another node
		gl_memlist_t *new_node = Tag_Malloc(sizeof(gl_memlist_t), MEM_VIDDRIVER);

#ifdef USE_NVIDIA_EXTENSIONS
		// initialize the fence object
		if (gl_ext_fence.integer)
			glGenFencesNV(1, &new_node->fence_id);
#endif

		//insert the node
		new_node->next = part->listTail->next;
		part->listTail->next = new_node;
		// increment the number of fences
		part->num_nodes++;
    }
	
	//set the next fence
	part->listTail = part->listTail->next;
	part->listTail->ptr = part->curPtr;

	return true;
}




void	*GL_Malloc(gl_partition_t *part, int size)
{
	byte		*curbuf;
	gl_memlist_t	*list;
	gl_memlist_t	*list_prev;

	if (gl_ext_vertex_buffer_object.integer)
	{
		System_Error("GL Error: GL_Malloc() was called when vertex_buffer_object is present\n");
	}

	if (size & 31)
		size += (32 - (size & 31));

	if (part->curPtr + size >= part->endMem)
	{
		//see if we can grow the memory
		if (part->curPtr + size < part->endPartition)
		{
			//grow
			part->endMem = part->curPtr + size;
		}
		else
		{
			//wrap
			if (!part->allowWraparound)
				return NULL;

			part->curPtr = part->startPartition;
		}
	}

	curbuf = part->curPtr;
	part->curPtr += size;

	//check if we overlap any memory
	list_prev = list = part->listHead;
	while (1)
	{
        	/* -- 1st case --
   	
        	  _dyn_head->ptr            _cur_dyn_var_ptr                                                _dyn_tail->ptr
         	      ||                         ||                                                             ||                
         	      \/                         \/                                                             \/
        	       [..........................................................................................]
        	      |----------------------------------------------------------------------------------------------| VAR buffer
        	*/
		if (((part->listTail->ptr > list->ptr  &&  part->curPtr >= list->ptr  &&  part->listTail->ptr > part->curPtr)
			|| (part->listTail->ptr > list->ptr  &&  curbuf >= list->ptr  &&  part->listTail->ptr > curbuf ))

      		/* -- 2nd case --                                              
  
        	_dyn_tail->ptr             _dyn_head->ptr            _cur_dyn_var_ptr
          	     ||                         ||                      ||
          	     \/                         \/                      \/
          	     .]                         [..................................................................
         	    |----------------------------------------------------------------------------------------------| VAR buffer
       		*/
		|| ((list->ptr > part->listTail->ptr  &&  part->curPtr >= list->ptr  &&  part->curPtr > part->listTail->ptr )
			|| (list->ptr > part->listTail->ptr  &&  curbuf >= list->ptr  &&  curbuf > part->listTail->ptr ))

      		/* -- 3rd case --                                                    
    
         	 _cur_dyn_var_ptr            _dyn_tail->ptr                                  _dyn_head->ptr      
          	     ||                         ||                                               ||
          	     \/                         \/                                               \/
          	     ............................]                                               [.................
          	    |----------------------------------------------------------------------------------------------| VAR buffer
       		*/
        	|| ((list->ptr > part->listTail->ptr  &&  part->listTail->ptr > part->curPtr && list->ptr >= part->curPtr)
			|| (list->ptr > part->listTail->ptr && part->listTail->ptr > curbuf && list->ptr >= curbuf)))
		
	        {
	            list_prev = list;
	            list = list->next;
	        }
		else
			break;
	}

	if (list != part->listHead)
	{
#ifdef USE_NVIDIA_EXTENSIONS
		if (gl_ext_fence.integer)
			glFinishFenceNV(list_prev->fence_id);
#endif
		part->listHead = list_prev->next;
	}

	return curbuf;
}



//memory init / shutdown


#ifdef _WIN32
#define NV_ALLOC wglAllocateMemoryNV
#define NV_FREE wglFreeMemoryNV
#elif __APPLE__
#define NV_ALLOC(a, b, c, d) malloc((a))
#define NV_FREE(a) free(a)
#else //unix
#define NV_ALLOC glXAllocateMemoryNV
#define NV_FREE glXFreeMemoryNV
#endif


static byte	*gl_memBlock = NULL;
static int gl_memBlockSize = 0;
static bool gl_usingSystemMem;

void	GL_InitMemory()
{	
	gl_memBlock = NULL;

	
	if (gl_ext_vertex_buffer_object.integer)
	{
		//using vertex buffer object extension, we don't need a memory manager
		return;
	}

#ifdef USE_NVIDIA_EXTENSIONS
	if (gl_ext_vertex_array_range.integer)
	{
		int n;

		//try to allocate AGP memory

		gl_memBlock = NULL;

#ifndef __APPLE__
		for (n = MAX(vid_blockmegs.integer, 4); n >= 4; n--)
		{
			gl_memBlock = NV_ALLOC(n * 1024 * 1024, 0, 0, 0.5);
			if (gl_memBlock)
			{
				gl_memBlockSize = n * 1024 * 1024;
				Console_Printf("Allocated %i bytes of AGP memory\n", gl_memBlockSize);
				gl_usingSystemMem = false;
				break;
			}
		}
#endif
		
		if (!gl_memBlock)
		{
			//default to system memory
#ifndef __APPLE__
			Console_Printf("PERFORMANCE WARNING: Defaulting to system memory after failing to allocate AGP memory!\n");
#endif
		}			

	}
#endif
	
	if (!gl_memBlock)
	{
		gl_memBlockSize = MAX(vid_blockmegs.integer * 1024 * 1024, 4 * 1024 * 1024);
		gl_memBlock = Tag_Malloc(vid_blockmegs.integer * 1024 * 1024, MEM_VIDDRIVER);
		gl_usingSystemMem = true;
		Console_Printf("Allocated %i bytes of system memory\n", gl_memBlockSize);
	}

#ifdef USE_NVIDIA_EXTENSIONS
	if (gl_ext_vertex_array_range.integer)
	{
		glVertexArrayRangeNV(gl_memBlockSize, gl_memBlock);
		GL_EnableClientState(GL_VERTEX_ARRAY_RANGE_NV);		
	}
#endif

	//initialize partitions for dynamic and static geometry
	GL_InitPartition(&gl_dynamicPartition, gl_memBlock, (gl_memBlockSize >> 1), true);
	gl_staticPartition.memBlock = gl_memBlock + (gl_memBlockSize >> 1);
	gl_staticPartition.pos = 0;
	gl_staticPartition.size = gl_memBlockSize >> 1;	
}




//should always return a valid pointer
void	*GL_DynamicMalloc(int size)
{
	return GL_Malloc(&gl_dynamicPartition, size);
}

void	GL_SetFence()
{
	if (!gl_ext_vertex_array_range.integer)
		return;

	_GL_SetFence(&gl_dynamicPartition);
}


//this will return NULL if there's no more room in the static partition, so be sure to check the return value!
//'cache' will hold information about the vertex data being allocated, and will be updated if the data
//falls out of the cache
void	*GL_StaticMalloc(int size)
{
	void *newmem;

	if (gl_staticPartition.pos + size > gl_staticPartition.size)
	{
		return NULL;
	}

	newmem = &gl_staticPartition.memBlock[gl_staticPartition.pos];

	gl_staticPartition.pos += (size + 31) & ~31;	//align to 32 byte intervals

	gl_staticPartition.lastPos = gl_staticPartition.pos;

	return newmem;	
}

void	GL_StaticMallocHold()
{
	gl_staticPartition.lastPos = gl_staticPartition.pos;
}

void	GL_StaticMallocFetch()
{
	gl_staticPartition.pos = gl_staticPartition.lastPos;
}

void	GL_ClearStatic()
{
	gl_staticPartition.pos = 0;
	gl_staticPartition.lastPos = 0;
}


//copies arrays from system memory to AGP memory
void	GL_CopySystemArrays(gl_arrays_t *from, gl_arrays_t *to, int numVerts)
{
	if (from->verts)
	{
		to->verts = GL_DynamicMalloc(numVerts * sizeof(vec3_t));
		Mem_Copy(to->verts, from->verts, numVerts * sizeof(vec3_t));
	}
	if (from->tverts)
	{
		to->tverts = GL_DynamicMalloc(numVerts * sizeof(vec2_t));
		Mem_Copy(to->tverts, from->tverts, numVerts * sizeof(vec2_t));
	}
	if (from->colors)
	{
		to->colors = GL_DynamicMalloc(numVerts * sizeof(bvec4_t));
		Mem_Copy(to->colors, from->colors, numVerts * sizeof(bvec4_t));
	}
	
	if (from->normals)
	{
		to->normals = GL_DynamicMalloc(numVerts * sizeof(vec3_t));
		Mem_Copy(to->normals, from->normals, numVerts * sizeof(vec3_t));
	}
}

void	GL_ShowMemInfo_Cmd(int argc, char *argv[])
{
	Console_Printf("Static: %i\n", gl_staticPartition.pos);
}


void	GL_SetVertexPointers(gl_arrays_t *arrays, bool ignoreAlpha)
{
	glVertexPointer(3, GL_FLOAT, 0, arrays->verts);
	glNormalPointer(GL_FLOAT, 0, arrays->normals);
	glTexCoordPointer(2, GL_FLOAT, 0, arrays->tverts);
	if (ignoreAlpha)
		glColorPointer(3, GL_UNSIGNED_BYTE, 4, arrays->colors);
	else
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, arrays->colors);
}



void	GL_ShutDownMemory()
{
	if (gl_ext_vertex_buffer_object.integer)
		return;

	if (!gl_memBlock)
		return;

	GL_ShutdownPartition(&gl_dynamicPartition);
	GL_ClearStatic();

	if (gl_usingSystemMem)
		Tag_Free(gl_memBlock);
#ifdef USE_NVIDIA_EXTENSIONS
	else
		NV_FREE(gl_memBlock);
#endif

	gl_memBlock = NULL;
	gl_memBlockSize = 0;
}









































































/***************************   SHADERS   ************************



 ****************************************************************/




void	GL_SetRegisterShaderCallback(void(*callback)(shader_t *shader))
{
	registerShaderCallback = callback;
}

void	_GL_NullRegisterShaderCallback(shader_t *shader)
{
}


/*==========================

  GL_TexEnvMode

 ==========================*/

void	GL_TexEnvMode(int mode)
{
	static int last_mode = -1;

	if (last_mode != mode || gls.texunit != GL_TEXTURE0_ARB)
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
		if (gls.texunit == GL_TEXTURE0_ARB)
			last_mode = mode;
	}
}


/*==========================

  GL_TexModulate

  modify the texture environment to modulate a texture by the specified amount
  this is how we're doing overbright, rather than changing the gamma ramp..
  only 1.0, 2.0, and 4.0 are acceptable values

 ==========================*/

void	GL_TexModulate(float amt)
{
	static float last_amt = 0;

	if (gl_ext_texenv_combine.integer)
	{
		GL_TexEnvMode(GL_COMBINE_EXT);

		if (amt != last_amt)
		{
			last_amt = amt;
		
			glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
			glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
			glTexEnvf (GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, amt);
		}
	}
	else
	{
		GL_TexEnvMode(GL_MODULATE);			
	}
}


/*
//when a shader is registered, this is called to determine how the shader should be sorted when rendering
//this just assigns a default index.  res.c does a better job of resorting the shaders
void	GL_AssignSortedIndex(shader_t *shader)
{
	shader->driveridx = gl_shader_index;
	gl_shader_index++;
	if (gl_sorted_index >= MAX_SHADER_BUCKETS)
	{
		//System_Error("Exceeded MAX_SHADERS in GL_AssignDriverIndex\n");
		Console_Printf("WARNING: sorted index wraparound\n");
		gl_shader_index = 0;
	}
}*/

void	GL_SwitchTexUnit(int texunit)
{
	if (!gl_ext_multitexture.integer)
		return;

	glActiveTextureARB(texunit);
	glClientActiveTextureARB(texunit);

	gls.texunit = texunit;
}

void	GL_SelectShader(shader_t *shader, float time)
{
	bool first = true;
	int n = 0;
	static int last_texunit = 0;
	static int last_texmap_used = -1;
	static int last_srcblend = -1;
	static int last_dstblend = -1;
	static int last_time = -1;
	static bool tex_identity = true;
	//static int last_specular = -1, last_emission = -1, last_shininess = -1;

	int frame;

	sceneStats.numSelectShaders++;

	if (current_shader == shader && time == last_time && last_texunit == gls.texunit)	
		return;	

	switch (shader->maptype)
	{
	case SHADER_ANIMATED_TEXTURE:
	case SHADER_ANIMATED_TEXTURE_EX:
		frame = SHD_CALCULATEFRAME(time, shader->fps, shader->texmap_num);
		break;

	case SHADER_SINGLE_TEXTURE:
	default:
		frame = 0;
		break;
	}

	//simple check to avoid redundant bindTexture calls (doesn't handle multitexture very well, but won't cause any problems)
	if (last_texmap_used != shader->texmaps[frame] || last_texunit != gls.texunit)	
	{
		glBindTexture(GL_TEXTURE_2D, shader->texmaps[frame]);		

		last_texunit = gls.texunit;
		last_texmap_used = shader->texmaps[frame];

		sceneStats.numTextureSwitches++;
	}

	if (shader->maptype == SHADER_BINKVIDEO)
		Bink_GetFrame(shader);
	else if (shader->maptype == SHADER_THEORAVIDEO)
		Theora_GetFrame(shader);

	//only change the blend function specified for the base texture unit
	if (gls.texunit == GL_TEXTURE0_ARB)
	{
		if (last_srcblend != blendmodeMapping[shader->srcblend] || last_dstblend != blendmodeMapping[shader->dstblend])
		{
			glBlendFunc(blendmodeMapping[shader->srcblend], blendmodeMapping[shader->dstblend]);
			last_srcblend = blendmodeMapping[shader->srcblend];
			last_dstblend = blendmodeMapping[shader->dstblend];		
		}
	}

	if (shader->animateTexCoords)
	{
		//set UVW function
		//todo: cool idea...use the console function evaluation as a parameter for UV animation!	
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		
		for (n=0; n<3; n++)
		{
			switch(shader->uv_func[n])
			{
				case UV_LINEAR_PAN:
					glTranslatef(n==0 ? (shader->uv_speed[n] * time + shader->uv_phase[n]) * shader->uv_amplitude[n] : 0,
								 n==1 ? (shader->uv_speed[n] * time + shader->uv_phase[n]) * shader->uv_amplitude[n] : 0,
								 n==2 ? (shader->uv_speed[n] * time + shader->uv_phase[n]) * shader->uv_amplitude[n] : 0
								 );
					break;
				case UV_SIN_PAN:
					glTranslatef(n==0 ? SIN(time * shader->uv_speed[n] + DEG2RAD(shader->uv_phase[n])) * shader->uv_amplitude[n] : 0,
								 n==1 ? SIN(time * shader->uv_speed[n] + DEG2RAD(shader->uv_phase[n])) * shader->uv_amplitude[n] : 0,
								 n==2 ? SIN(time * shader->uv_speed[n] + DEG2RAD(shader->uv_phase[n])) * shader->uv_amplitude[n] : 0
								 );
					break;
				case UV_LINEAR_SCALE:
					glScalef(n==0 ? (shader->uv_speed[n] * time + shader->uv_phase[n]) * shader->uv_amplitude[n] : 1,
								 n==1 ? (shader->uv_speed[n] * time + shader->uv_phase[n]) * shader->uv_amplitude[n] : 1,
								 n==2 ? (shader->uv_speed[n] * time + shader->uv_phase[n]) * shader->uv_amplitude[n] : 1
								 );				
					break;
				case UV_SIN_SCALE:
					glScalef(n==0 ? 1 + SIN(time * shader->uv_speed[n] + DEG2RAD(shader->uv_phase[n])) * shader->uv_amplitude[n] : 1,
								 n==1 ? 1 + SIN(time * shader->uv_speed[n] + DEG2RAD(shader->uv_phase[n])) * shader->uv_amplitude[n] : 1,
								 n==2 ? 1 + SIN(time * shader->uv_speed[n] + DEG2RAD(shader->uv_phase[n])) * shader->uv_amplitude[n] : 1
								 );
					break;
				default:
					break;
			}
		}

		glMatrixMode(GL_MODELVIEW);

		tex_identity = false;
	}
	else
	{
		if (tex_identity == false)
		{
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
		}
	}

	current_shader = shader;
	last_time = time;
}



/* fixme: base mipmap code stolen from mesa 

   this is just so i can try out terrain mipmaps
*/

/*
 * Find the value nearest to n which is also a power of two.
 */
GLint round2(GLint n)
{
   GLint m;

   for (m = 1; m < n; m *= 2);

   /* m>=n */
   if (m - n <= n - m / 2) {
      return m;
   }
   else {
      return m / 2;
   }
}

/*
 * Given an pixel format and datatype, return the number of bytes to
 * store one pixel.
 */
static GLint
bytes_per_pixel(GLenum format, GLenum type)
{
   GLint n, m;

   switch (format) {
   case GL_COLOR_INDEX:
   case GL_STENCIL_INDEX:
   case GL_DEPTH_COMPONENT:
   case GL_RED:
   case GL_GREEN:
   case GL_BLUE:
   case GL_ALPHA:
   case GL_LUMINANCE:
      n = 1;
      break;
   case GL_LUMINANCE_ALPHA:
      n = 2;
      break;
   case GL_RGB:
      n = 3;
      break;
   case GL_RGBA:
#ifdef GL_EXT_abgr
   case GL_ABGR_EXT:
#endif
      n = 4;
      break;
   default:
      n = 0;
   }

   switch (type) {
   case GL_UNSIGNED_BYTE:
      m = sizeof(GLubyte);
      break;
   case GL_BYTE:
      m = sizeof(GLbyte);
      break;
   case GL_BITMAP:
      m = 1;
      break;
   case GL_UNSIGNED_SHORT:
      m = sizeof(GLushort);
      break;
   case GL_SHORT:
      m = sizeof(GLshort);
      break;
   case GL_UNSIGNED_INT:
      m = sizeof(GLuint);
      break;
   case GL_INT:
      m = sizeof(GLint);
      break;
   case GL_FLOAT:
      m = sizeof(GLfloat);
      break;
   default:
      m = 0;
   }

   return n * m;
}


GLint _gluRegister2DMipmaps(GLenum target, int nummipmaps, int *levels, bitmap_t *mipmaps)
{
   GLint w, h, maxsize;
   GLint level;
   GLboolean done;
   GLint unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
   GLint packrowlength, packalignment, packskiprows, packskippixels;

   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);

   w = round2(mipmaps[0].width);
   if (w > maxsize) {
      w = maxsize;
   }
   h = round2(mipmaps[0].height);
   if (h > maxsize) {
      h = maxsize;
   }

   /* Get current glPixelStore values */
   glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unpackrowlength);
   glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackalignment);
   glGetIntegerv(GL_UNPACK_SKIP_ROWS, &unpackskiprows);
   glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &unpackskippixels);
   glGetIntegerv(GL_PACK_ROW_LENGTH, &packrowlength);
   glGetIntegerv(GL_PACK_ALIGNMENT, &packalignment);
   glGetIntegerv(GL_PACK_SKIP_ROWS, &packskiprows);
   glGetIntegerv(GL_PACK_SKIP_PIXELS, &packskippixels);

   /* set pixel packing */
   glPixelStorei(GL_PACK_ROW_LENGTH, 0);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_SKIP_ROWS, 0);
   glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

   done = GL_FALSE;

   level = 0;
   while (!done) {
		   //I don't know why we're doing this - Jon
      if (level != 0) {
	 /* set pixel unpacking */
	 glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	 glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	 glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	 glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      }

	  w = mipmaps[level].width;
	  h = mipmaps[level].height;
	  //Console_DPrintf("mipmap level %i (%i by %i) is %i bytes\n", levels[level], w, h, mipmaps[level].size);
	 glCompressedTexImage2DARB(GL_TEXTURE_2D, levels[level], mipmaps[level].bmptype, w, h, 0, mipmaps[level].size, mipmaps[level].data);

	  Cvar_SetVarValue(&gl_textures_loaded, gl_textures_loaded.value + mipmaps[level].size);

      level++;
	  if (level == nummipmaps)
		done = true;
   }

   /* Restore original glPixelStore state */
   glPixelStorei(GL_UNPACK_ROW_LENGTH, unpackrowlength);
   glPixelStorei(GL_UNPACK_ALIGNMENT, unpackalignment);
   glPixelStorei(GL_UNPACK_SKIP_ROWS, unpackskiprows);
   glPixelStorei(GL_UNPACK_SKIP_PIXELS, unpackskippixels);
   glPixelStorei(GL_PACK_ROW_LENGTH, packrowlength);
   glPixelStorei(GL_PACK_ALIGNMENT, packalignment);
   glPixelStorei(GL_PACK_SKIP_ROWS, packskiprows);
   glPixelStorei(GL_PACK_SKIP_PIXELS, packskippixels);

   return true;
}

GLint _gluBuild2DMipmaps(GLenum target, GLint components,
		  GLsizei width, GLsizei height, GLenum format,
		  GLenum type, const void *data)
{
   GLint w, h, maxsize;
   void *image, *newimage;
   GLint neww, newh, level, bpp;
   int error;
   GLboolean done;
   GLint retval = 0;
   GLint unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
   GLint packrowlength, packalignment, packskiprows, packskippixels;

   if (width < 1 || height < 1)
      return GLU_INVALID_VALUE;

   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);

   w = round2(width);
   if (w > maxsize) {
      w = maxsize;
   }
   h = round2(height);
   if (h > maxsize) {
      h = maxsize;
   }

   bpp = bytes_per_pixel(format, type);
   if (bpp == 0) {
      /* probably a bad format or type enum */
      return GLU_INVALID_ENUM;
   }

   /* Get current glPixelStore values */
   glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unpackrowlength);
   glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackalignment);
   glGetIntegerv(GL_UNPACK_SKIP_ROWS, &unpackskiprows);
   glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &unpackskippixels);
   glGetIntegerv(GL_PACK_ROW_LENGTH, &packrowlength);
   glGetIntegerv(GL_PACK_ALIGNMENT, &packalignment);
   glGetIntegerv(GL_PACK_SKIP_ROWS, &packskiprows);
   glGetIntegerv(GL_PACK_SKIP_PIXELS, &packskippixels);

   /* set pixel packing */
   glPixelStorei(GL_PACK_ROW_LENGTH, 0);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_SKIP_ROWS, 0);
   glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

   done = GL_FALSE;

   if (w != width || h != height) {
      /* must rescale image to get "top" mipmap texture image */
      image = malloc((w + 4) * h * bpp);
      if (!image) {
	 return GLU_OUT_OF_MEMORY;
      }
      error = gluScaleImage(format, width, height, type, data,
			    w, h, type, image);
      if (error) {
	 	  retval = error;
	 	  done = GL_TRUE;
      }
   }
   else {
      image = (void *) data;
   }

   level = 0;
   while (!done) {
      if (image != data) {
	 /* set pixel unpacking */
	 glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	 glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	 glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	 glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      }

	  Console_DPrintf(" level %i is %i by %i\n", level, w, h);
      glTexImage2D(target, level, components, w, h, 0, format, type, image);
	  Cvar_SetVarValue(&gl_textures_loaded, gl_textures_loaded.value + (w * h * bpp));

      if (w == 1 && h == 1)
	 break;

      neww = (w < 2) ? 1 : w / 2;
      newh = (h < 2) ? 1 : h / 2;
      newimage = malloc((neww + 4) * newh * bpp);
      if (!newimage) {
	 return GLU_OUT_OF_MEMORY;
      }

      error = gluScaleImage(format, w, h, type, image,
			    neww, newh, type, newimage);
      if (error) {
	 retval = error;
	 done = GL_TRUE;
      }

      if (image != data) {
	 free(image);
      }
      image = newimage;

      w = neww;
      h = newh;

      level++;
   }

   if (image != data) {
      free(image);
   }

   /* Restore original glPixelStore state */
   glPixelStorei(GL_UNPACK_ROW_LENGTH, unpackrowlength);
   glPixelStorei(GL_UNPACK_ALIGNMENT, unpackalignment);
   glPixelStorei(GL_UNPACK_SKIP_ROWS, unpackskiprows);
   glPixelStorei(GL_UNPACK_SKIP_PIXELS, unpackskippixels);
   glPixelStorei(GL_PACK_ROW_LENGTH, packrowlength);
   glPixelStorei(GL_PACK_ALIGNMENT, packalignment);
   glPixelStorei(GL_PACK_SKIP_ROWS, packskiprows);
   glPixelStorei(GL_PACK_SKIP_PIXELS, packskippixels);


   return retval;
}

#define CEILING( A, B )  ( (A) % (B) == 0 ? (A)/(B) : (A)/(B)+1 )

/* To work around optimizer bug in MSVC4.1 */
#if defined(__WIN32__) && !defined(OPENSTEP)
void
dummy(GLuint j, GLuint k)
{
}
#else
#define dummy(J, K)
#endif


GLint GL_ScaleTerrainImage(GLenum format,
	      GLsizei widthin, GLsizei heightin,
		  float *dstcolor, float dstblend,
	      GLenum typein, const void *datain,
	      GLsizei widthout, GLsizei heightout,
	      GLenum typeout, void *dataout		  )
{
   GLint components, i, j, k;
   GLfloat *tempin, *tempout;
   GLfloat sx, sy;
   GLint unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
   GLint packrowlength, packalignment, packskiprows, packskippixels;
   GLint sizein, sizeout;
   GLint rowstride, rowlen;


   /* Determine number of components per pixel */
   switch (format) {
   case GL_COLOR_INDEX:
   case GL_STENCIL_INDEX:
   case GL_DEPTH_COMPONENT:
   case GL_RED:
   case GL_GREEN:
   case GL_BLUE:
   case GL_ALPHA:
   case GL_LUMINANCE:
      components = 1;
      break;
   case GL_LUMINANCE_ALPHA:
      components = 2;
      break;
   case GL_RGB:
      components = 3;
      break;
   case GL_RGBA:
#ifdef GL_EXT_abgr
   case GL_ABGR_EXT:
#endif
      components = 4;
      break;
   default:
      return GLU_INVALID_ENUM;
   }

   /* Determine bytes per input datum */
   switch (typein) {
   case GL_UNSIGNED_BYTE:
      sizein = sizeof(GLubyte);
      break;
   case GL_BYTE:
      sizein = sizeof(GLbyte);
      break;
   case GL_UNSIGNED_SHORT:
      sizein = sizeof(GLushort);
      break;
   case GL_SHORT:
      sizein = sizeof(GLshort);
      break;
   case GL_UNSIGNED_INT:
      sizein = sizeof(GLuint);
      break;
   case GL_INT:
      sizein = sizeof(GLint);
      break;
   case GL_FLOAT:
      sizein = sizeof(GLfloat);
      break;
   case GL_BITMAP:
      /* not implemented yet */
   default:
      return GL_INVALID_ENUM;
   }

   /* Determine bytes per output datum */
   switch (typeout) {
   case GL_UNSIGNED_BYTE:
      sizeout = sizeof(GLubyte);
      break;
   case GL_BYTE:
      sizeout = sizeof(GLbyte);
      break;
   case GL_UNSIGNED_SHORT:
      sizeout = sizeof(GLushort);
      break;
   case GL_SHORT:
      sizeout = sizeof(GLshort);
      break;
   case GL_UNSIGNED_INT:
      sizeout = sizeof(GLuint);
      break;
   case GL_INT:
      sizeout = sizeof(GLint);
      break;
   case GL_FLOAT:
      sizeout = sizeof(GLfloat);
      break;
   case GL_BITMAP:
      /* not implemented yet */
   default:
      return GL_INVALID_ENUM;
   }

   /* Get glPixelStore state */
   glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unpackrowlength);
   glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackalignment);
   glGetIntegerv(GL_UNPACK_SKIP_ROWS, &unpackskiprows);
   glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &unpackskippixels);
   glGetIntegerv(GL_PACK_ROW_LENGTH, &packrowlength);
   glGetIntegerv(GL_PACK_ALIGNMENT, &packalignment);
   glGetIntegerv(GL_PACK_SKIP_ROWS, &packskiprows);
   glGetIntegerv(GL_PACK_SKIP_PIXELS, &packskippixels);

   /* Allocate storage for intermediate images */
   tempin = (GLfloat *) malloc(widthin * heightin
			       * components * sizeof(GLfloat));
   if (!tempin) {
      return GLU_OUT_OF_MEMORY;
   }
   tempout = (GLfloat *) malloc(widthout * heightout
				* components * sizeof(GLfloat));
   if (!tempout) {
      free(tempin);
      return GLU_OUT_OF_MEMORY;
   }


   /*
    * Unpack the pixel data and convert to floating point
    */

   if (unpackrowlength > 0) {
      rowlen = unpackrowlength;
   }
   else {
      rowlen = widthin;
   }
   if (sizein >= unpackalignment) {
      rowstride = components * rowlen;
   }
   else {
      rowstride = unpackalignment / sizein
	 * CEILING(components * rowlen * sizein, unpackalignment);
   }

   switch (typein) {
   case GL_UNSIGNED_BYTE:
      k = 0;
      for (i = 0; i < heightin; i++) {
	 GLubyte *ubptr = (GLubyte *) datain
	    + i * rowstride
	    + unpackskiprows * rowstride + unpackskippixels * components;
	 for (j = 0; j < widthin * components; j++) {
	    dummy(j, k);
	    tempin[k++] = (GLfloat) * ubptr++;
	 }
      }
      break;
   case GL_BYTE:
      k = 0;
      for (i = 0; i < heightin; i++) {
	 GLbyte *bptr = (GLbyte *) datain
	    + i * rowstride
	    + unpackskiprows * rowstride + unpackskippixels * components;
	 for (j = 0; j < widthin * components; j++) {
	    dummy(j, k);
	    tempin[k++] = (GLfloat) * bptr++;
	 }
      }
      break;
   case GL_UNSIGNED_SHORT:
      k = 0;
      for (i = 0; i < heightin; i++) {
	 GLushort *usptr = (GLushort *) datain
	    + i * rowstride
	    + unpackskiprows * rowstride + unpackskippixels * components;
	 for (j = 0; j < widthin * components; j++) {
	    dummy(j, k);
	    tempin[k++] = (GLfloat) * usptr++;
	 }
      }
      break;
   case GL_SHORT:
      k = 0;
      for (i = 0; i < heightin; i++) {
	 GLshort *sptr = (GLshort *) datain
	    + i * rowstride
	    + unpackskiprows * rowstride + unpackskippixels * components;
	 for (j = 0; j < widthin * components; j++) {
	    dummy(j, k);
	    tempin[k++] = (GLfloat) * sptr++;
	 }
      }
      break;
   case GL_UNSIGNED_INT:
      k = 0;
      for (i = 0; i < heightin; i++) {
	 GLuint *uiptr = (GLuint *) datain
	    + i * rowstride
	    + unpackskiprows * rowstride + unpackskippixels * components;
	 for (j = 0; j < widthin * components; j++) {
	    dummy(j, k);
	    tempin[k++] = (GLfloat) * uiptr++;
	 }
      }
      break;
   case GL_INT:
      k = 0;
      for (i = 0; i < heightin; i++) {
	 GLint *iptr = (GLint *) datain
	    + i * rowstride
	    + unpackskiprows * rowstride + unpackskippixels * components;
	 for (j = 0; j < widthin * components; j++) {
	    dummy(j, k);
	    tempin[k++] = (GLfloat) * iptr++;
	 }
      }
      break;
   case GL_FLOAT:
      k = 0;
      for (i = 0; i < heightin; i++) {
	 GLfloat *fptr = (GLfloat *) datain
	    + i * rowstride
	    + unpackskiprows * rowstride + unpackskippixels * components;
	 for (j = 0; j < widthin * components; j++) {
	    dummy(j, k);
	    tempin[k++] = *fptr++;
	 }
      }
      break;
   default:
      return GLU_INVALID_ENUM;
   }


   /*
    * Scale the image!
    */

   if (widthout > 1)
      sx = (GLfloat) (widthin - 1) / (GLfloat) (widthout - 1);
   else
      sx = (GLfloat) (widthin - 1);
   if (heightout > 1)
      sy = (GLfloat) (heightin - 1) / (GLfloat) (heightout - 1);
   else
      sy = (GLfloat) (heightin - 1);

//#define POINT_SAMPLE
#ifdef POINT_SAMPLE
   for (i = 0; i < heightout; i++) {
      GLint ii = i * sy;
      for (j = 0; j < widthout; j++) {
	 GLint jj = j * sx;

	 GLfloat *src = tempin + (ii * widthin + jj) * components;
	 GLfloat *dst = tempout + (i * widthout + j) * components;

	 for (k = 0; k < components; k++) {
	    *dst++ = *src++;
	 }
      }
   }
#else
   if (sx < 1.0 && sy < 1.0) {
      /* magnify both width and height:  use weighted sample of 4 pixels */
      GLint i0, i1, j0, j1;
      GLfloat alpha, beta;
      GLfloat *src00, *src01, *src10, *src11;
      GLfloat s1, s2;
      GLfloat *dst;

      for (i = 0; i < heightout; i++) {
	 i0 = i * sy;
	 i1 = i0 + 1;
	 if (i1 >= heightin)
	    i1 = heightin - 1;
/*	 i1 = (i+1) * sy - EPSILON;*/
	 alpha = i * sy - i0;
	 for (j = 0; j < widthout; j++) {
	    j0 = j * sx;
	    j1 = j0 + 1;
	    if (j1 >= widthin)
	       j1 = widthin - 1;
/*	    j1 = (j+1) * sx - EPSILON; */
	    beta = j * sx - j0;

	    /* compute weighted average of pixels in rect (i0,j0)-(i1,j1) */
	    src00 = tempin + (i0 * widthin + j0) * components;
	    src01 = tempin + (i0 * widthin + j1) * components;
	    src10 = tempin + (i1 * widthin + j0) * components;
	    src11 = tempin + (i1 * widthin + j1) * components;

	    dst = tempout + (i * widthout + j) * components;

	    for (k = 0; k < components; k++) {
	       s1 = *src00++ * (1.0 - beta) + *src01++ * beta;
	       s2 = *src10++ * (1.0 - beta) + *src11++ * beta;
	       *dst = s1 * (1.0 - alpha) + s2 * alpha;	
		   *dst = LERP(dstblend, *dst, dstcolor[k]);
		   //*dst += (rand() % 20) - 10;
		   if (*dst > 255) *dst = 255;
		   if (*dst < 0) *dst = 0;
		   dst++;
	    }
	 }
      }
   }
   else {
      /* shrink width and/or height:  use an unweighted box filter */
      GLint i0, i1;
      GLint j0, j1;
      GLint ii, jj;
      GLfloat sum, *dst;

      for (i = 0; i < heightout; i++) {
	 i0 = i * sy;
	 i1 = i0 + 1;
	 if (i1 >= heightin)
	    i1 = heightin - 1;
/*	 i1 = (i+1) * sy - EPSILON; */
	 for (j = 0; j < widthout; j++) {
	    j0 = j * sx;
	    j1 = j0 + 1;
	    if (j1 >= widthin)
	       j1 = widthin - 1;
/*	    j1 = (j+1) * sx - EPSILON; */

	    dst = tempout + (i * widthout + j) * components;

	    /* compute average of pixels in the rectangle (i0,j0)-(i1,j1) */
	    for (k = 0; k < components; k++) {
	       sum = 0.0;
	       for (ii = i0; ii <= i1; ii++) {
		  for (jj = j0; jj <= j1; jj++) {
		     sum += *(tempin + (ii * widthin + jj) * components + k);
		  }
	       }
	       sum /= (j1 - j0 + 1) * (i1 - i0 + 1);
	       *dst = sum;		   
		   *dst = LERP(dstblend, *dst, dstcolor[k]);
		   //*dst += (rand() % 20) - 10;
		   if (*dst > 255) *dst = 255;
		   if (*dst < 0) *dst = 0;
		   dst++;
	    }
	 }
      }
   }
#endif


   /*
    * Return output image
    */

   if (packrowlength > 0) {
      rowlen = packrowlength;
   }
   else {
      rowlen = widthout;
   }
   if (sizeout >= packalignment) {
      rowstride = components * rowlen;
   }
   else {
      rowstride = packalignment / sizeout
	 * CEILING(components * rowlen * sizeout, packalignment);
   }

   switch (typeout) {
   case GL_UNSIGNED_BYTE:
      k = 0;
      for (i = 0; i < heightout; i++) {
	 GLubyte *ubptr = (GLubyte *) dataout
	    + i * rowstride
	    + packskiprows * rowstride + packskippixels * components;
	 for (j = 0; j < widthout * components; j++) {
	    dummy(j, k + i);
	    *ubptr++ = (GLubyte) tempout[k++];
	 }
      }
      break;
   case GL_BYTE:
      k = 0;
      for (i = 0; i < heightout; i++) {
	 GLbyte *bptr = (GLbyte *) dataout
	    + i * rowstride
	    + packskiprows * rowstride + packskippixels * components;
	 for (j = 0; j < widthout * components; j++) {
	    dummy(j, k + i);
	    *bptr++ = (GLbyte) tempout[k++];
	 }
      }
      break;
   case GL_UNSIGNED_SHORT:
      k = 0;
      for (i = 0; i < heightout; i++) {
	 GLushort *usptr = (GLushort *) dataout
	    + i * rowstride
	    + packskiprows * rowstride + packskippixels * components;
	 for (j = 0; j < widthout * components; j++) {
	    dummy(j, k + i);
	    *usptr++ = (GLushort) tempout[k++];
	 }
      }
      break;
   case GL_SHORT:
      k = 0;
      for (i = 0; i < heightout; i++) {
	 GLshort *sptr = (GLshort *) dataout
	    + i * rowstride
	    + packskiprows * rowstride + packskippixels * components;
	 for (j = 0; j < widthout * components; j++) {
	    dummy(j, k + i);
	    *sptr++ = (GLshort) tempout[k++];
	 }
      }
      break;
   case GL_UNSIGNED_INT:
      k = 0;
      for (i = 0; i < heightout; i++) {
	 GLuint *uiptr = (GLuint *) dataout
	    + i * rowstride
	    + packskiprows * rowstride + packskippixels * components;
	 for (j = 0; j < widthout * components; j++) {
	    dummy(j, k + i);
	    *uiptr++ = (GLuint) tempout[k++];
	 }
      }
      break;
   case GL_INT:
      k = 0;
      for (i = 0; i < heightout; i++) {
	 GLint *iptr = (GLint *) dataout
	    + i * rowstride
	    + packskiprows * rowstride + packskippixels * components;
	 for (j = 0; j < widthout * components; j++) {
	    dummy(j, k + i);
	    *iptr++ = (GLint) tempout[k++];
	 }
      }
      break;
   case GL_FLOAT:
      k = 0;
      for (i = 0; i < heightout; i++) {
	 GLfloat *fptr = (GLfloat *) dataout
	    + i * rowstride
	    + packskiprows * rowstride + packskippixels * components;
	 for (j = 0; j < widthout * components; j++) {
	    dummy(j, k + i);
	    *fptr++ = tempout[k++];
	 }
      }
      break;
   default:
      return GLU_INVALID_ENUM;
   }


   /* free temporary image storage */
   free(tempin);
   free(tempout);

   return 0;
}

GLint GL_BuildTerrainMipmaps(GLenum target, GLint components,
		  GLsizei width, GLsizei height, GLenum format,
		  GLenum type, const void *data, bitmap_t *bitmap)
{
   GLint w, h, maxsize;
   void *image, *newimage;
   GLint neww, newh, level, bpp;
   int error;
   GLboolean done;
   GLint retval = 0;
   GLint unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
   GLint packrowlength, packalignment, packskiprows, packskippixels;
   bvec4_t avgcol_b;
   vec4_t avgcol;
   int maxlevels;
   float ww = width;
   

   if (width < 1 || height < 1)
      return GLU_INVALID_VALUE;

   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);

   Bitmap_GetAverageColor(bitmap, avgcol_b);
   avgcol[0] = (float)avgcol_b[0];// / 255.0;
   avgcol[1] = (float)avgcol_b[1];// / 255.0;
   avgcol[2] = (float)avgcol_b[2];// / 255.0;
   avgcol[3] = 255;

   w = round2(width);
   if (w > maxsize) {
      w = maxsize;
   }
   h = round2(height);
   if (h > maxsize) {
      h = maxsize;
   }

   bpp = bytes_per_pixel(format, type);
   if (bpp == 0) {
      /* probably a bad format or type enum */
      return GLU_INVALID_ENUM;
   }

   /* Get current glPixelStore values */
   glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unpackrowlength);
   glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackalignment);
   glGetIntegerv(GL_UNPACK_SKIP_ROWS, &unpackskiprows);
   glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &unpackskippixels);
   glGetIntegerv(GL_PACK_ROW_LENGTH, &packrowlength);
   glGetIntegerv(GL_PACK_ALIGNMENT, &packalignment);
   glGetIntegerv(GL_PACK_SKIP_ROWS, &packskiprows);
   glGetIntegerv(GL_PACK_SKIP_PIXELS, &packskippixels);

   /* set pixel packing */
   glPixelStorei(GL_PACK_ROW_LENGTH, 0);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_SKIP_ROWS, 0);
   glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

   done = GL_FALSE;

   if (w != width || h != height) {
      /* must rescale image to get "top" mipmap texture image */
      image = malloc((w + 4) * h * bpp);
      if (!image) {
	 return GLU_OUT_OF_MEMORY;
      }
      error = GL_ScaleTerrainImage(format, width, height, avgcol, 0, type, data,
			    w, h, type, image);
      if (error) {
	 retval = error;
	 done = GL_TRUE;
      }
   }
   else {
      image = (void *) data;
   }

   level = 0;
   maxlevels = 1;   
   while(1)
   {	   
	   ww /= 2;
 	   if (ww < 2)
		   break;

	   maxlevels++;
   }

   while (!done) {
      if (image != data) {
	 /* set pixel unpacking */
	 glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	 glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	 glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	 glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      }

	  Console_DPrintf(" level %i is %i by %i\n", level, w, h);
      glTexImage2D(target, level, components, w, h, 0, format, type, image);
	  Cvar_SetVarValue(&gl_textures_loaded, gl_textures_loaded.value + (w * h * bpp));

      if (w == 1 && h == 1)
	 break;

      neww = (w < 2) ? 1 : w / 2;
      newh = (h < 2) ? 1 : h / 2;
      newimage = malloc((neww + 4) * newh * bpp);
      if (!newimage) {
	 return GLU_OUT_OF_MEMORY;
      }

	  
	  error = GL_ScaleTerrainImage(format, w, h, avgcol, ((float)level/maxlevels) * 2.0, type, image,
			    neww, newh, type, newimage);

      if (error) {
	 retval = error;
	 done = GL_TRUE;
      }

      if (image != data) {
	 free(image);
      }
      image = newimage;

      w = neww;
      h = newh;

      level++;
   }

   if (image != data) {
      free(image);
   }

   /* Restore original glPixelStore state */
   glPixelStorei(GL_UNPACK_ROW_LENGTH, unpackrowlength);
   glPixelStorei(GL_UNPACK_ALIGNMENT, unpackalignment);
   glPixelStorei(GL_UNPACK_SKIP_ROWS, unpackskiprows);
   glPixelStorei(GL_UNPACK_SKIP_PIXELS, unpackskippixels);
   glPixelStorei(GL_PACK_ROW_LENGTH, packrowlength);
   glPixelStorei(GL_PACK_ALIGNMENT, packalignment);
   glPixelStorei(GL_PACK_SKIP_ROWS, packskiprows);
   glPixelStorei(GL_PACK_SKIP_PIXELS, packskippixels);

   return retval;
}

bool	GL_UnregisterShader(shader_t *shader)
{
	glDeleteTextures(shader->texmap_num, shader->texmaps);
	return true;
}

bool	GL_RegisterShader(shader_t *shader)
{
	char *sprintf_filename, *loop_filename; //, *bumpmap_filename;
	int i;

	i = 0;

	shader->texmap_num = 0;

	if (shader->maptype == SHADER_ANIMATED_TEXTURE ||
		shader->maptype == SHADER_ANIMATED_TEXTURE_EX)
	{
		bool ret;
		//now all this mess is to turn: blahblahblah-blah-00.png
		//                        into: blahblahblah-blah-%02d.png
		// then we'll loop through with an int and build the filenames that way
		//  (if only we had strdup_printf() :)

		//this just has to hold the filenames
		loop_filename = Tag_Malloc(strlen(shader->name) + 1, MEM_VIDDRIVER);

		//allocate the right space
	    sprintf_filename = Tag_Malloc(strlen(shader->name) + 4, MEM_VIDDRIVER);
		//copy the filename over
		strcpy(sprintf_filename, shader->name);
		//replace the 00. with a \0 for the upcoming sprintf
		if (shader->maptype == SHADER_ANIMATED_TEXTURE_EX)
			strcpy(strstr(sprintf_filename, "000."), "");
		else
			strcpy(strstr(sprintf_filename, "00."), "");

		// add on the digit code and then the old extension
		if (shader->maptype == SHADER_ANIMATED_TEXTURE_EX)
			strcat(sprintf_filename, "%03d");
		else
			strcat(sprintf_filename, "%02d");
		strcat(sprintf_filename, ".");
		strcat(sprintf_filename, Filename_GetExtension(shader->name));

		sprintf(loop_filename, sprintf_filename, i);
		while (File_Exists(loop_filename))
		{
			ret = GL_RegisterShaderImageTextureMap(shader, loop_filename);
			i++;
			sprintf(loop_filename, sprintf_filename, i);
		}

		Tag_Free(loop_filename);
		Tag_Free(sprintf_filename);

		if (!i)
		{
			Console_DPrintf("GL_RegisterShader: no files found for animated texture\n");
			return false;
		}

		//no images, don't both to animate anything
		if (shader->texmap_num == 0)
			return false;	

		shader->fps = 15;
	} 
	else if (shader->maptype == SHADER_SINGLE_TEXTURE)
	{
		if (!GL_RegisterShaderImageTextureMap(shader, shader->name))
			return false;
	}
	else if (shader->maptype == SHADER_BINKVIDEO)
	{
		if (!GL_RegisterShaderImageBink(shader, shader->name))
			return false;
	}
	else if (shader->maptype == SHADER_THEORAVIDEO)
	{
		if (!GL_RegisterShaderImageTheora(shader, shader->name))
			return false;
	}
	else
	{
		Console_DPrintf("GL_RegisterShader: unknown shader type\n");
		return false;
	}

	//GL_AssignDriverIndex(shader);	

	registerShaderCallback(shader);

	shader->active = true;

	return true;
}

#define MAX_MIPMAP_LEVELS 11
bool GL_RegisterShaderImageTextureMap(shader_t *shader, char *filename)
{
	bitmap_t images[MAX_MIPMAP_LEVELS];
	int	texture;
	int levels[MAX_MIPMAP_LEVELS];
	byte numLevels;
	byte i;

	images[0].data = NULL;

	if (!Bitmap_CheckForS2G(filename, &numLevels, MAX_MIPMAP_LEVELS, levels, images))
	{
		if (!Bitmap_Load(filename, &images[0]))
		{
			Console_Printf("GL_RegisterShaderImageTextureMap: %s not found or bad shader", shader->name);
			return false;
		}
		numLevels = 1;
	}

	if (images[0].bmptype == BITMAP_S3TC_DXT3
		|| images[0].bmptype == BITMAP_S3TC_DXT5)
	{
		shader->flags |= SHD_NO_COMPRESS | SHD_ALREADY_COMPRESSED | SHD_NO_MIPMAPS;
	}
	
	if (shader->texmap_num >= MAX_SHADER_MAPS)
	{
		Console_Printf("GL_RegisterShaderImageTextureMap: exceeded %i maps in %s", MAX_SHADER_MAPS, shader->name);
		return false;
	}
	
	if (numLevels > 1)
	{
		texture = GL_RegisterShaderImages(shader, numLevels, levels, images);
	}
	else
	{
		texture = GL_RegisterShaderImage(shader, filename, &images[0]);
	}

	for (i = 0; i < numLevels; i++)
		Tag_Free(images[i].data);

	shader->texmaps[shader->texmap_num] = texture;
	shader->texmap_num++;

	if (images[0].translucent)
		shader->translucent = true;

	return true;
}


bool GL_RegisterShaderImageTheora(shader_t *shader, char *filename)
{
	GLuint id;
	bitmap_t image;
	int	movieid;

	movieid = Theora_Load(filename, &image, (shader->flags & SHD_MOVIE_PRELOAD_ALL));

	if (movieid == -1)
		return false;
	else
		shader->movieid = movieid;

	glGenTextures(1, &id);
	
	glBindTexture(GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	if (!(shader->flags & SHD_REPEAT))
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	#ifdef _TEXTURE_NEAREST
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	#else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	#endif

	glTexImage2D (GL_TEXTURE_2D, 0, image.bmptype, image.width, image.height , 0, 
			(image.mode == 32) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image.data);
	
	shader->texmaps[0] = id;
	shader->texmap_num = 1;

	if (image.translucent)
		shader->translucent = true;

	return true;
}


bool GL_RegisterShaderImageBink(shader_t *shader, char *filename)
{
	GLuint id;
	bitmap_t image;
	int	movieid;

	movieid = Bink_Load(filename, &image, (shader->flags & SHD_MOVIE_PRELOAD_ALL));

	if (movieid == -1)
		return false;
	else
		shader->movieid = movieid;

	glGenTextures(1, &id);
	
	glBindTexture(GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	if (!(shader->flags & SHD_REPEAT))
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	#ifdef _TEXTURE_NEAREST
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	#else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	#endif

	glTexImage2D (GL_TEXTURE_2D, 0, image.bmptype, image.width, image.height , 0, 
			(image.mode == 32) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image.data);
	
	shader->texmaps[0] = id;
	shader->texmap_num = 1;

	if (image.translucent)
		shader->translucent = true;

	return true;
}


bool GL_RegisterShaderImageFromMemory(shader_t *shader, bitmap_t *image)
{
	int	texture;

	texture = GL_RegisterShaderImage(shader, NULL,  image);

	shader->texmaps[shader->texmap_num] = texture;
	shader->texmap_num++;

	if (image->translucent)
		shader->translucent = true;

/*	if (shader->translucent)
	{
		shader->driveridx = MAX_SHADERS - gl_translucent_shader_index - 1;  //so translucent polys will always get drawn last
		if (MAX_SHADERS - gl_translucent_shader_index - 1 <= gl_shader_index)
		{
			Console_Printf("GL_RegisterShader: WARNING, array_object collision");
		}
		gl_translucent_shader_index++;
	}
	else
	{*/

	//GL_AssignDriverIndex(shader);
		
	//}

	registerShaderCallback(shader);

	shader->active = true;

	return true;
}

bool GL_RegisterShaderImageBumpMap(shader_t *shader, char *filename)
{
	bitmap_t image;
	bitmap_t bumpmap;
	
	image.data = NULL;

	if (!Bitmap_Load(filename, &image))
	{
		Console_Printf("GL_RegisterShaderImageBumpMap: %s not found or bad shader", shader->name);
		return false;
	}
	
	Bitmap_HeightMapToNormalMap(&image, &bumpmap, bump_map_scale); 
	shader->bumpmap = GL_RegisterShaderImage(shader, NULL, &bumpmap);
	
	//Bitmap_WritePNG("bump_test.png", &bumpmap);

	Tag_Free(image.data);
	Tag_Free(bumpmap.data);

	return true;
}

bool	GL_GetCompressedShaderImage(int level, bitmap_t *bmp)
{
	//clear the error list
	while (glGetError() != GL_NO_ERROR);
		
	/* Query for the compressed internal format */
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_INTERNAL_FORMAT, &bmp->bmptype);

	if (glGetError() != GL_NO_ERROR)
		return false;
	
	/* Query for the compressed internal format */
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &bmp->width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &bmp->height);

	if (!bmp->width || !bmp->height)
		return false;
	
	/* Query for the size of the compressed data texture buffer */
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &bmp->size);
	if (bmp->size == 0)
		return false;

	/* Allocate a buffer to host a copy of the compressed image data */
	bmp->data = (unsigned char *)Tag_Malloc(bmp->size * sizeof(unsigned char), MEM_VIDDRIVER);

	/* get the compressed data buffer */
	glGetCompressedTexImageARB(GL_TEXTURE_2D, level, bmp->data);

	return true;
}

void	GL_WriteCompressedShaderImages(shader_t *shader, char *filename, bool translucent)
{
	char s2gFilename[1024];
	int level, i;
	int levels[MAX_MIPMAP_LEVELS];
	bitmap_t bmps[MAX_MIPMAP_LEVELS];

	level = 0;
	while (GL_GetCompressedShaderImage(level, &bmps[level]))
	{
		bmps[level].translucent = translucent;
		levels[level] = level;
		level++;
	}
	strncpySafe(s2gFilename, filename ? filename : shader->name, 1024);
	if (s2gFilename[strlen(s2gFilename)-4] == '.')
	{
		strcpy(&s2gFilename[strlen(s2gFilename)-3], "s2g");
		Bitmap_WriteS2G(s2gFilename, (byte)level, levels, bmps);
	}

	for (i = 0; i < level; i++)
		Tag_Free(bmps[i].data);
}

//we assume numLevels is >1, else it would go to RegisterShaderImage
int GL_RegisterShaderImages(shader_t *shader, int numLevels, int *levels, bitmap_t *images)
{
	GLuint id;
	GLuint components;
	int power2width, power2height;
	int internalFormat;
	int startLevel;

	glGenTextures(1, &id);
	
	//fixme: read a shader script and change texture parameters accordingly
	glBindTexture (GL_TEXTURE_2D, id);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	if (!(shader->flags & SHD_REPEAT))
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

//	GL_Modulate(2.0);
	
	switch(images[0].bmptype)
	{
		case BITMAP_S3TC_DXT3:
		case BITMAP_S3TC_DXT5: 	components = GL_RGBA;  break;
		case BITMAP_RGBA: 		components = GL_RGBA;  break;
		case BITMAP_GRAYSCALE: 	components = GL_LUMINANCE; break;
		case BITMAP_RGB: 
		default: 				components = GL_RGB; break;
	}

	//given that we're registering a set of mipmaps and an image, this should ever happen
	if (vid_compressTextures.integer && !(shader->flags & SHD_NO_COMPRESS))
	{
		if (images[0].translucent)
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;  //4bit interpolated alpha
		else
			//4bit explicit alpha
			//we could use DXT1 for non-nvidia cards here, but the 16-bit mode is a killer
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	}
	else
	{
		internalFormat = images[0].bmptype;
	}

	power2width = round2(images[0].width);
	power2height = round2(images[0].height);

	if (power2width != images[0].width)
		Console_DPrintf("GL_RegisterShader: warning: bad texture width\n");
	
	if (power2height != images[0].height)
		Console_DPrintf("GL_RegisterShader: warning: bad texture height\n");

	Cvar_SetValue("vid_maxtexturesize", round2(vid_maxtexturesize.integer));
	if (vid_maxtexturesize.integer < 1)
		Cvar_SetValue("vid_maxtexturesize", 1);

	#ifdef _TEXTURE_NEAREST
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	#else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	#endif

	startLevel = 0;
	while (images[startLevel].width > vid_maxtexturesize.integer
			|| images[startLevel].height > vid_maxtexturesize.integer)
	{
		startLevel++;
	}
		
	_gluRegister2DMipmaps(id, numLevels - startLevel, &levels[startLevel], &images[startLevel]);

	return id;
}

int	GL_RegisterShaderImage(shader_t *shader, char *filename, bitmap_t *image)
{
	GLuint id;
	GLuint components;
	int power2width, power2height;
	int compressed, internalFormat;

	compressed = 0;
	
	glGenTextures(1, &id);
	
	//fixme: read a shader script and change texture parameters accordingly
	glBindTexture (GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	if (!(shader->flags & SHD_REPEAT))
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

//	GL_Modulate(2.0);
	
	switch(image->bmptype)
	{
		case BITMAP_S3TC_DXT3:
		case BITMAP_S3TC_DXT5: 	components = GL_RGBA;  break;
		case BITMAP_RGBA: 		components = GL_RGBA;  break;
		case BITMAP_GRAYSCALE: 	components = GL_LUMINANCE; break;
		case BITMAP_RGB: 
		default: 				components = GL_RGB; break;
	}

	if (vid_compressTextures.integer && !(shader->flags & SHD_NO_COMPRESS))
	{
		if (image->translucent)
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;  //4bit interpolated alpha
		else
			//4bit explicit alpha
			//we could use DXT1 for non-nvidia cards here, but the 16-bit mode is a killer
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	}
	else
	{
		internalFormat = image->bmptype;
	}

	power2width = round2(image->width);
	power2height = round2(image->height);

	if (power2width != image->width)
		Console_DPrintf("GL_RegisterShader: warning: bad texture width\n");
	
	if (power2height != image->height)
		Console_DPrintf("GL_RegisterShader: warning: bad texture height\n");

	Cvar_SetVarValue(&vid_maxtexturesize, round2(vid_maxtexturesize.integer));
	if (vid_maxtexturesize.integer < 1)
		Cvar_SetVarValue(&vid_maxtexturesize, 1);



	if (!(shader->flags & SHD_FULL_QUALITY))
	{
		if (power2width > vid_maxtexturesize.integer)
		{
			int oldwidth = power2width;
	
			power2width = vid_maxtexturesize.integer;
			power2height *= (float)power2width / oldwidth;
		}

		if (power2height > vid_maxtexturesize.integer)
		{
			int oldheight = power2height;
	
			power2height = vid_maxtexturesize.integer;
			power2width *= (float)power2height / oldheight;
		}
	}

	if (power2width != image->width || power2height != image->height)
	{
		byte *scaledbmp = Tag_Malloc(power2width * power2height * image->bmptype, MEM_VIDDRIVER);

		gluScaleImage(	components, image->width, image->height, GL_UNSIGNED_BYTE, image->data,
						power2width, power2height, GL_UNSIGNED_BYTE, scaledbmp);

		Bitmap_Free(image);

		image->width = power2width;
		image->height = power2height;
		image->data = scaledbmp;
	}

	if (!(shader->flags & SHD_ALREADY_COMPRESSED))
		Bitmap_DetermineTranslucency(image);
	else
		Console_DPrintf("it appears that %s is already compressed\n", shader->name);
/*
	// set pixel packing
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	// set pixel unpacking
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
*/


	
// 	Cvar_SetVarValue(&gl_textures_loaded, gl_textures_loaded.value + (image->width * image->height * bpp));
	
	if (!(shader->flags & SHD_NO_MIPMAPS) && !(shader->flags & SHD_ALREADY_COMPRESSED))
	{		
		#ifdef _TEXTURE_NEAREST
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		#else
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		#endif
	    if (shader->flags & SHD_TERRAIN_MIPMAPS)
		{
			GL_BuildTerrainMipmaps(GL_TEXTURE_2D, internalFormat, image->width, image->height, components, GL_UNSIGNED_BYTE, image->data, image);

		}
		else
		{
			_gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, image->width, image->height, components, GL_UNSIGNED_BYTE, image->data);
		}

		if (vid_compressTextures.integer && !(shader->flags & SHD_NO_COMPRESS))
		{
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
			if (!compressed)
			{
				Console_Printf("attempt to compress texture failed for %s\n", shader->name);
			}
		}
	}
	else
	{
		#ifdef _TEXTURE_NEAREST
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		#else
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		#endif
		if (shader->flags & SHD_ALREADY_COMPRESSED)
			glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, internalFormat, image->width, image->height, 0, image->size, image->data);
		else
			glTexImage2D (GL_TEXTURE_2D, 0, internalFormat, image->width, image->height , 0, components, GL_UNSIGNED_BYTE, image->data);

		if (vid_compressTextures.integer && !(shader->flags & SHD_NO_COMPRESS || shader->flags & SHD_ALREADY_COMPRESSED))
		{
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
			if (!compressed)
			{
				Console_Printf("attempt to compress texture failed for %s\n", shader->name);
			}
		}
	}
	if (compressed
		&& vid_compressTextures.integer 
		&& !(shader->flags & SHD_NO_COMPRESS)
		&& !(shader->flags & SHD_ALREADY_COMPRESSED) 
		&& vid_writeCompressedTextures.integer)
	{
		GL_WriteCompressedShaderImages(shader, filename, image->translucent);
	}
	
	return id;
}





























































































/************************   Utility functions   ****************************


 ***************************************************************************/


int		GL_GetTexEnvModeFromString(const char *string)
{
	if (stricmp(string, "GL_BLEND")==0)
		return GL_BLEND;
	else if (stricmp(string, "GL_ADD")==0)
		return GL_ADD;
	else if (stricmp(string, "GL_MODULATE")==0)
		return GL_MODULATE;
	else if (stricmp(string, "GL_DECAL")==0)
		return GL_DECAL;
	else if (stricmp(string, "GL_COMBINE")==0)
		return GL_COMBINE_EXT;
	else
		return GL_MODULATE;
}

//so we don't have to worry about linking issues with glu
void _gluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
   GLdouble xmin, xmax, ymin, ymax;

   ymax = zNear * tan(fovy * M_PI / 360.0);
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void GL_2dMode()
{		
	glViewport(0, 0, Vid_GetScreenW(), Vid_GetScreenH());	
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(0, Vid_GetScreenW(), Vid_GetScreenH(), 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	GL_Disable(GL_DEPTH_TEST);
	GL_Disable(GL_CULL_FACE);
	GL_Enable(GL_BLEND);	
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_TexModulate(1.0);
}

void GL_SetDrawRegion(int x, int y, int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x, x+w, y+h, y, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(x, Vid_GetScreenH() - y - h, w, h);
}



void GL_GetFrameBuffer(bitmap_t *screenshot)
{ 
	screenshot->width = Vid_GetScreenW();
	screenshot->height = Vid_GetScreenH();
	screenshot->bmptype = BITMAP_RGB;
	screenshot->mode = 24;
	screenshot->data = Tag_Malloc(3 * screenshot->width * screenshot->height, MEM_VIDDRIVER);
	if (screenshot->data == NULL)
	{
		return;
	}

	glReadPixels(0, 0, screenshot->width, screenshot->height, GL_RGB, GL_UNSIGNED_BYTE, screenshot->data);
	Bitmap_Flip(screenshot);
}

void	GL_Identity(double m[16])
{
	m[0] = 1.0; m[4] = 0.0; m[8]  = 0.0; m[12] = 0.0;
	m[1] = 0.0; m[5] = 1.0; m[9]  = 0.0; m[13] = 0.0;
	m[2] = 0.0; m[6] = 0.0; m[10] = 1.0; m[14] = 0.0;
	m[3] = 0.0; m[7] = 0.0; m[11] = 0.0; m[15] = 1.0;
}

void	GL_PushCamera(camera_t *camera);
void	GL_PopCamera();

void	GL_ProjectVertexInternal(vec3_t vertex, vec3_t result)
{
	GLdouble res[3];
	GLdouble model_view[16];
	GLdouble projection[16];
	GLint viewport[4];

	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	gluProject(vertex[0], vertex[1], vertex[2],
				model_view, 
				projection,
				viewport,
				&res[0], &res[1], &res[2]);

	result[0] = res[0];
	result[1] = Vid_GetScreenH() - res[1];
	result[2] = res[2];
}

void	GL_ProjectVertex(camera_t *cam, vec3_t vertex, vec2_t result)
{
	GLdouble res[3];
	GLdouble projection[16];
	GLint viewport[4];
	GLdouble	model_view[16];

	GL_PushCamera(cam);
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	gluProject(vertex[0], vertex[1], vertex[2],
				model_view, 
				projection,
				viewport,
				&res[0], &res[1], &res[2]);

	GL_PopCamera();

	result[0] = res[0];
	result[1] = Vid_GetScreenH() - res[1];
}

void	GL_ProjectVertex3(camera_t *cam, vec3_t vertex, vec3_t result)
{
	GLdouble res[3];
	GLdouble projection[16];
	GLint viewport[4];
	GLdouble	model_view[16];

	GL_PushCamera(cam);
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	gluProject(vertex[0], vertex[1], vertex[2],
				model_view, 
				projection,
				viewport,
				&res[0], &res[1], &res[2]);
	
	GL_PopCamera();

	result[0] = res[0];
	result[1] = Vid_GetScreenH() - res[1];
	result[2] = res[2];
}



void	GL_ReadZBuffer(int x, int y, int w, int h, float *zpixels)
{
	glReadPixels(x, Vid_GetScreenH() - y, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, zpixels);
}


//put the transform data into a 4x4 matrix
void	GL_TransformToMatrix(matrix43_t *transform, float tm[16])
{
	//construct the transformation matrix
	tm[0] = transform->axis[0][0];
	tm[1] = transform->axis[0][1];
	tm[2] = transform->axis[0][2];
	tm[3] = 0.0;
	tm[4] = transform->axis[1][0];
	tm[5] = transform->axis[1][1];
 	tm[6] = transform->axis[1][2];
 	tm[7] = 0.0;
 	tm[8] = transform->axis[2][0];
 	tm[9] = transform->axis[2][1];
 	tm[10] = transform->axis[2][2];
 	tm[11] = 0.0;
 	tm[12] = transform->pos[0];
 	tm[13] = transform->pos[1];
 	tm[14] = transform->pos[2];
 	tm[15] = 1.0;
}



void	GL_AxisToMatrix(vec3_t axis[3], float tm[16])
{
	//construct the transformation matrix
	tm[0] = axis[0][0];
	tm[1] = axis[0][1];
	tm[2] = axis[0][2];
	tm[3] = 0.0;
	tm[4] = axis[1][0];
	tm[5] = axis[1][1];
 	tm[6] = axis[1][2];
 	tm[7] = 0.0;
 	tm[8] = axis[2][0];
 	tm[9] = axis[2][1];
 	tm[10] = axis[2][2];
 	tm[11] = 0.0;
 	tm[12] = 0.0;
 	tm[13] = 0.0;
 	tm[14] = 0.0;
 	tm[15] = 1.0;
}



/*==========================

  GL_DrawTriangles

  should always use this instead of glDrawElements() to keep track of the polycount

 ==========================*/

void	GL_DrawTriangles(int primtype, int numElems, void *elemList)
{
	int count = 0;

	OVERHEAD_INIT;

	if (current_shader->translucent)
	{
		if (!gfx_renderTranslucent.integer)
			return;
	}
	else
	{
		if (!gfx_renderOpaque.integer)
			return;
	}

	glDrawElements(primtype, numElems, GL_UNSIGNED_INT, elemList);

	switch(primtype)
	{
		case GL_TRIANGLES:
			count = numElems / 3;
			break;
		case GL_TRIANGLE_STRIP:
			count = numElems - 2;
			break;
	}
	
	sceneStats.polycount += count;
	if (current_shader->translucent)
		sceneStats.translucentPolycount += count;

	sceneStats.drawTrianglesCount++;

	OVERHEAD_COUNT(OVERHEAD_DRAWTRIANGLES);
}


























































































/******************************  State switching  *****************************

 ******************************************************************************/


void	GL_Enable(int mode)
{
#define ENABLE_STATE(m) if (!m) { glEnable(mode); m = true; }
	switch(mode)
	{
		case GL_REGISTER_COMBINERS_NV:
			ENABLE_STATE(gls.register_combiners_nv);
			break;
		case GL_DEPTH_TEST:
			ENABLE_STATE(gls.depth_test);
			break;
		case GL_FOG:
			ENABLE_STATE(gls.fog);
			break;
//		case GL_TEXTURE_2D:
//			ENABLE_STATE(gls.texture_2d);
//			break;
		case GL_BLEND:
			ENABLE_STATE(gls.blend);
			break;
		case GL_CULL_FACE:
			ENABLE_STATE(gls.cull_face);
			break;
		case GL_ALPHA_TEST:
			ENABLE_STATE(gls.alpha_test);
			break;
		case GL_POLYGON_SMOOTH:
			ENABLE_STATE(gls.polygon_smooth);
			break;
		case GL_POLYGON_OFFSET_FILL:
			ENABLE_STATE(gls.polygon_offset_fill);
			break;
		case GL_LIGHTING:
			ENABLE_STATE(gls.lighting);
			break;
		case GL_LIGHT0:
			ENABLE_STATE(gls.light0);
			break;
		case GL_LIGHT1:
			ENABLE_STATE(gls.light1);
			break;
		case GL_LIGHT2:
			ENABLE_STATE(gls.light2);
			break;
		case GL_LIGHT3:
			ENABLE_STATE(gls.light3);
			break;
		case GL_LIGHT4:
			ENABLE_STATE(gls.light4);
			break;
		case GL_LIGHT5:
			ENABLE_STATE(gls.light5);
			break;
		case GL_LIGHT6:
			ENABLE_STATE(gls.light6);
			break;
		case GL_LIGHT7:
			ENABLE_STATE(gls.light7);
			break;
		case GL_MULTISAMPLE_ARB:
			ENABLE_STATE(gls.multisample);
			break;
		default:
//			Console_Printf("GL: Unrecognized state change %i\n", mode);
			glEnable(mode);
			break;
	}
#undef ENABLE_STATE
}

void	GL_EnableClientState(int mode)
{
	glEnableClientState(mode);
	/*
#define ENABLE_STATE(m) if (!m) { glEnableClientState(mode); m = true; }
	switch(mode)
	{
		case GL_COLOR_ARRAY:
			ENABLE_STATE(gls.color_array);
			break;
		case GL_TEXTURE_COORD_ARRAY:
			ENABLE_STATE(gls.texture_coord_array);
			break;
		case GL_VERTEX_ARRAY:
			ENABLE_STATE(gls.vertex_array);
			break;
		case GL_NORMAL_ARRAY:
			ENABLE_STATE(gls.normal_array);
			break;
		case GL_VERTEX_ARRAY_RANGE_NV:
			ENABLE_STATE(gls.vertex_array_range_nv);
			break;
		default:
			glEnableClientState(mode);
			break;
	}
#undef ENABLE_STATE
	*/
}

void	GL_DepthMask(int state)
{
	static bool last_state = 123456789;
	if (state != last_state)
	{
		glDepthMask((GLboolean)state);
		last_state = state;
	}
}

void	GL_Disable(int mode)
{
#define DISABLE_STATE(m) if (m) { glDisable(mode); m = false; }
	switch(mode)
	{
		case GL_REGISTER_COMBINERS_NV:
			DISABLE_STATE(gls.register_combiners_nv);
			break;
		case GL_DEPTH_TEST:
			DISABLE_STATE(gls.depth_test);
			break;
		case GL_FOG:
			DISABLE_STATE(gls.fog);
			break;
//		case GL_TEXTURE_2D:
//			DISABLE_STATE(gls.texture_2d);
//			break;
		case GL_BLEND:
			DISABLE_STATE(gls.blend);
			break;
		case GL_CULL_FACE:
			DISABLE_STATE(gls.cull_face);
			break;
		case GL_ALPHA_TEST:
			DISABLE_STATE(gls.alpha_test);
			break;
		case GL_POLYGON_SMOOTH:
			DISABLE_STATE(gls.polygon_smooth);
			break;
		case GL_POLYGON_OFFSET_FILL:
			DISABLE_STATE(gls.polygon_offset_fill);
			break;
		case GL_LIGHTING:
			DISABLE_STATE(gls.lighting);
			break;
		case GL_LIGHT0:
			DISABLE_STATE(gls.light0);
			break;
		case GL_LIGHT1:
			DISABLE_STATE(gls.light1);
			break;
		case GL_LIGHT2:
			DISABLE_STATE(gls.light2);
			break;
		case GL_LIGHT3:
			DISABLE_STATE(gls.light3);
			break;
		case GL_LIGHT4:
			DISABLE_STATE(gls.light4);
			break;
		case GL_LIGHT5:
			DISABLE_STATE(gls.light5);
			break;
		case GL_LIGHT6:
			DISABLE_STATE(gls.light6);
			break;
		case GL_LIGHT7:
			DISABLE_STATE(gls.light7);
			break;
		case GL_MULTISAMPLE_ARB:
			DISABLE_STATE(gls.multisample);
			break;
		default:
			glDisable(mode);
			break;
	}
#undef DISABLE_STATE
}

void	GL_DisableClientState(int mode)
{
	glDisableClientState(mode);
	/*
#define DISABLE_STATE(m) if (m) { glDisableClientState(mode); m = false; }
	switch(mode)
	{
		case GL_COLOR_ARRAY:
			DISABLE_STATE(gls.color_array);
			break;
		case GL_TEXTURE_COORD_ARRAY:
			DISABLE_STATE(gls.texture_coord_array);
			break;
		case GL_VERTEX_ARRAY:
			DISABLE_STATE(gls.vertex_array);
			break;
		case GL_NORMAL_ARRAY:
			DISABLE_STATE(gls.normal_array);
			break;
		case GL_VERTEX_ARRAY_RANGE_NV:
			DISABLE_STATE(gls.vertex_array_range_nv);
			break;
		default:
			glDisableClientState(mode);
			break;
	}
#undef DISABLE_STATE
	*/
}

void	GL_DepthFunc(int func)
{
	static int last_depthfunc = GL_LESS;

	if (func != last_depthfunc)
	{
		glDepthFunc(func);
		last_depthfunc = func;
	}
}

void	GL_CullFace(int mode)
{
	static int last_mode = 0;

	if (mode != last_mode)
	{
		glCullFace(mode);
		last_mode = mode;
	}
}

static GLuint texUnits[] = 
{
	GL_TEXTURE0_ARB,
	GL_TEXTURE1_ARB,
	GL_TEXTURE2_ARB,
	GL_TEXTURE3_ARB,
	GL_TEXTURE4_ARB,
	GL_TEXTURE5_ARB,
	GL_TEXTURE6_ARB,
	GL_TEXTURE7_ARB,
	GL_TEXTURE8_ARB,
	GL_TEXTURE9_ARB,
	GL_TEXTURE10_ARB,
	GL_TEXTURE11_ARB,
	GL_TEXTURE12_ARB,
	GL_TEXTURE13_ARB,
	GL_TEXTURE14_ARB,
	GL_TEXTURE15_ARB,
	GL_TEXTURE16_ARB,
	GL_TEXTURE17_ARB,
	GL_TEXTURE18_ARB,
	GL_TEXTURE19_ARB,
	GL_TEXTURE20_ARB,
	GL_TEXTURE21_ARB,
	GL_TEXTURE22_ARB,
	GL_TEXTURE23_ARB,
	GL_TEXTURE24_ARB,
	GL_TEXTURE25_ARB,
	GL_TEXTURE26_ARB,
	GL_TEXTURE27_ARB,
	GL_TEXTURE28_ARB,
	GL_TEXTURE29_ARB,
	GL_TEXTURE30_ARB,
	GL_TEXTURE31_ARB,
};

void	GL_DisableTextures()
{
	int n;
	int maxtex;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxtex);

	for (n=0; n<maxtex; n++)
	{
		GL_SwitchTexUnit(texUnits[n]);
		glDisable(GL_TEXTURE_2D);
	}	
	GL_SwitchTexUnit(GL_TEXTURE0_ARB);
}



















































































/************************  Shutdown and Initialization  *************************



 ********************************************************************************/




void GL_ShutDown()
{
	GL_ShutDownMemory();
}

//sets up driver info, retrieves available extensions,
//sets up GL related cvars and sets GL states
void GL_Init()
{
	char *vendor;
	int major, minor;
	
	memset(&nullshader, 0, sizeof(shader_t));

	//now called fmom host.c so we can set gfx_useVBO and gfx_useVAR in startup.cfg
	//GL_InitVars();

	Cmd_Register("glMemInfo", GL_ShowMemInfo_Cmd);

	memset(&gls, 0, sizeof(glStates_t));
	memset(&gl, 0, sizeof(gl_info_t));

	if (gfx.integer)
	{
		gl.vendor = (char*)glGetString(GL_VENDOR);
		gl.renderer = (char*)glGetString(GL_RENDERER);
		gl.version = (char*)glGetString(GL_VERSION);
		gl.extensions = (char*)glGetString(GL_EXTENSIONS);
	
		Console_Printf("OpenGL: Vendor: %s\n", gl.vendor);
		Console_Printf("OpenGL: Renderer: %s\n", gl.renderer);
		Console_Printf("OpenGL: Version: %s\n", gl.version);
		Console_Printf("OpenGL: Extensions: %s\n", gl.extensions);

		if (sscanf(gl.version, "%i.%i", &major, &minor))
		{
			if (major == 1 && minor < 2)
			{
				System_Error("Your OpenGL driver (%s %s version %s) doesn't support at least OpenGL version 1.2.  Savage requires at least OpenGL version 1.2\n", gl.vendor, gl.renderer, gl.version);
			}
		}
		
		vendor = Tag_Strdup(gl.vendor, MEM_VIDDRIVER);
		strlwr(vendor);
		if (strstr(vendor, "nvidia"))
		{
			Cvar_SetVarValue(&gfx_nvidia, 1);
		}
		if (strstr(gl.renderer, "Radeon")
			|| strstr(gl.renderer, "RADEON")
			|| strstr(gl.renderer, "RV250"))
		{
			Cvar_SetVarValue(&gfx_radeon, 1);
		}


		Tag_Free(vendor);

		GL_InitExtensions();

		glClearColor(0, 0, 0, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glShadeModel(GL_SMOOTH);
		GL_CullFace(GL_BACK);
		GL_Enable(GL_CULL_FACE);
		if (vid_multisample.value)
			GL_Enable(GL_MULTISAMPLE_ARB);
					
		GL_Enable(GL_TEXTURE_2D);		

		GL_Enable(GL_DEPTH_TEST);
		GL_Enable(GL_BLEND);
	//	GL_Enable(GL_ALPHA_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//	glBlendFunc(GL_ONE, GL_ONE);
	/*	glHint(GL_FOG, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST); */
		glEnable(GL_NORMALIZE);
	
		//enable vertex arrays
		GL_EnableClientState(GL_VERTEX_ARRAY);
		GL_EnableClientState(GL_TEXTURE_COORD_ARRAY);
		GL_EnableClientState(GL_COLOR_ARRAY);
		GL_EnableClientState(GL_NORMAL_ARRAY);
	
		glLineWidth(4);

		glEnable(GL_COLOR_MATERIAL);

		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		GL_2dMode();

		GL_InitMemory();

		GL_SwitchTexUnit(GL_TEXTURE0_ARB);
	}
	M_InitSimpleNoise2();
}	

void	GL_PrintWarnings()
{
	if (!dedicated_server.integer)
	{
		if (gl.renderer && strstr(gl.renderer, "GDI Generic"))
		{
			Console_Errorf("You do not have OpenGL drivers with hardware acceleration installed.  You need to install new video card drivers - if you have a NVidia GeForce card, go to www.nvidia.com.  If you have an ATI Radeon, go to www.ati.com");
		}
		else if (gl.renderer && strstr(gl.renderer, "PCI") && !strstr(gl.renderer, "AGP"))
		{
#ifdef unix
			Console_Errorf("Your video card is not in AGP Mode!  This card probably won't be able to run Savage correctly.");
#else
			Console_Errorf("Your video card is in PCI mode.  Please click on the 'Important Performance Information' shortcut in your 'Savage - The Battle for Newerth' program group.");
#endif
		}
		else if (gl_usingSystemMem)
		{
			if (strstr(gl.renderer, "Radeon")
				|| strstr(gl.renderer, "RADEON")
				|| strstr(gl.renderer, "RV250"))
				Console_Printf("ATI Radeon detected\n");
			else
			{
#ifdef unix
				Console_Errorf("Couldn't allocate AGP Memory.  Make sure you have agpgart or some similar AGP driver support in your kernel.");
#else
				//Console_Errorf("Couldn't allocate AGP Memory.  Please click on the 'Important Performance Information' shortcut in your 'Savage - The Battle for Newerth' program group.");
#endif
			}

		}
	}
}

#endif //_S2_DONT_INCLUDE_GL

void GL_RegisterVars()
{
	Cvar_Register(&gl_fence_status);
	Cvar_Register(&gl_use_texture_as_bumpmap);
	Cvar_Register(&gl_minMemPerFence);
	Cvar_Register(&ogl_noclamp);
	Cvar_Register(&gl_textures_loaded);
	Cvar_Register(&gfx_renderTranslucent);
	Cvar_Register(&gfx_renderOpaque);
	Cvar_Register(&gfx_nvidia);
	Cvar_Register(&gfx_useVBO);
	Cvar_Register(&gfx_radeon);
	Cvar_Register(&gfx_useVAR);
	Cvar_Register(&gfx_gammaMult);
}

