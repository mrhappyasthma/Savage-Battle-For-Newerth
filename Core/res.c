// (C) 2003 S2 Games

// res.c

// Resource loading and management


#include "core.h"

/*
typedef enum
{
	RES_SHADER,
	RES_MODEL,
	RES_SOUND,
} restype_t;

struct
{
	restype_t		restype;
	int				id;			//internal index to the resource							
} resource_t;
*/

//todo:

//Res_IsShaderCached()
//Res_CacheShader()
//Res_IsModelCached()
//Res_CacheModel()
//Res_FlushShaders()
//Res_FlushModels()

//#define		MODEL_SCALE_FACTOR		0.02
//#define MODEL_SCALE_FACTOR 1

cvar_t	mat_shininess = { "mat_shininess", "" };
cvar_t	mat_ambient = { "mat_ambient", "" };
cvar_t	mat_specular = { "mat_specular", "" };
cvar_t	mat_emission = { "mat_emission", "" };
cvar_t	mat_uFunc = { "mat_uFunc", "NONE" };
cvar_t	mat_vFunc = { "mat_vFunc", "NONE" };
cvar_t	mat_wFunc = { "mat_wFunc", "NONE" };
cvar_t	mat_uSpeed = { "mat_uSpeed", "0" };
cvar_t	mat_vSpeed = { "mat_vSpeed", "0" };
cvar_t	mat_wSpeed = { "mat_wSpeed", "0" };
cvar_t	mat_uPhase = { "mat_uPhase", "0" };
cvar_t	mat_vPhase = { "mat_vPhase", "0" };
cvar_t	mat_wPhase = { "mat_wPhase", "0" };
cvar_t	mat_uAmplitude = { "mat_uAmplitude", "1" };
cvar_t	mat_vAmplitude = { "mat_vAmplitude", "1" };
cvar_t	mat_wAmplitude = { "mat_wAmplitude", "1" };
cvar_t	mat_doubleSided = { "mat_doubleSided", "0" };
cvar_t	res_hashBadFiles = { "res_hashBadFiles", "1" };

static int			shader_count = 0;  //0 is reserved for an error
static int			font_count = 1;

//model_t		null_model;


typedef enum
{
	MEDIA_SHADER,
	MEDIA_FONT,
	MEDIA_MODEL,
	MEDIA_CLIP,
	MEDIA_SAMPLE,
	MEDIA_STRINGTABLE,
	NUM_MEDIA_TYPES
} mediaTypes_enum;

char *mediaLibNames[] =
{
	"MEDIA_SHADER",
	"MEDIA_FONT",
	"MEDIA_MODEL",
	"MEDIA_CLIP",
	"MEDIA_SAMPLE",
	"MEDIA_STRINGTABLE",
	"",
};

#define MAX_BAD_MEDIA				1024


typedef struct
{
	int type;						//see MEDIA_* enum above
	bool strict;					//if num_media exceeds max_media, a system error will occur
	bool canReferenceNULL;			//if true, a media get can be NULL, and all relevant functions should check this
									//otherwise a media get will always return a valid pointer
									//models for instance always return a valid model (the "question mark"
									//model if the model is not found), while samples can be NULL
	GHashTable *hash;				//hash table for name lookups
	GHashTable *badhash;			//hash table which stores file names that could not be loaded, so we don't thrash the disk on multiple Res_Load* calls to a bad resource

	int num_media;
	int max_media;
	media_t **media;
	
	unsigned int badcounter;
	char *badfilenames[MAX_BAD_MEDIA];
} mediaLibrary_t;

static mediaLibrary_t libs[NUM_MEDIA_TYPES];

shader_t	*shaders[MAX_SHADERS];
fontData_t	*fonts[MAX_FACES];

GHashTable *shader_hash;

extern	cvar_t gfx;
extern 	bool	sound_initialized;


guint Res_Hash (gconstpointer key)
{
  const char *p = key;
  guint h = *p;

  if (h)
    for (p += 1; *p != '\0'; p++)
      h = (h << 5) - h + (tolower(*p));

  return h;
}

int Res_Str_Equal(gconstpointer v, gconstpointer v2)
{
	return (stricmp(v, v2) == 0);
}

void		Res_ShaderList_Cmd(int argc, char *argv[])
{
	int n;

	Console_Printf("Listing %i shaders:\n\n", shader_count);

	for (n=0; n<shader_count; n++)
	{
		if (shaders[n])
		{
			Console_Printf("%i: %s (order %i)\n", n, shaders[n]->name, shaders[n]->sortedidx);
		}
		else
		{
			Console_Printf("%i: INACTIVE\n", n);
		}
	}
}


void		Res_ReloadAnim_Cmd(int argc, char *argv[])
{
	int n;

	if (!argc)
		return;

	if (strcmp(argv[0], "all")==0)
	{
		for (n=0; n<libs[MEDIA_MODEL].num_media; n++)
		{
			if (libs[MEDIA_MODEL].media[n]->d.data)
				Geom_ReloadAnim(n);
		}
	}
	else
		Geom_ReloadAnim(atoi(argv[0]));
}

void		Res_SoundList_Cmd(int argc, char *argv[])
{
	int n;

	Console_Printf("Listing %i sounds:\n\n", libs[MEDIA_SAMPLE].num_media);

	for (n=0; n<libs[MEDIA_SAMPLE].num_media; n++)
	{
		Console_Printf("%i: %s", n, libs[MEDIA_SAMPLE].media[n]->name);
	
		if (!libs[MEDIA_SAMPLE].media[n]->d.data)
		{
			Console_Printf(" (INACTIVE)\n", n);
		}
		else
		{
			Console_Printf("\n");
		}
	}
}

void		Res_ModelList_Cmd(int argc, char *argv[])
{
	int n;

	Console_Printf("Listing %i models:\n\n", libs[MEDIA_MODEL].num_media);

	for (n=0; n<libs[MEDIA_MODEL].num_media; n++)
	{
		Console_Printf("%i: %s", n, libs[MEDIA_MODEL].media[n]->name);

		if (!libs[MEDIA_MODEL].media[n]->d.data)
		{
			Console_Printf(" (INACTIVE)\n", n);
		}
		else
		{
			Console_Printf("\n");
		}
	}
}


void		Res_ModelInfo_Cmd(int argc, char *argv[])
{
	model_t *model;

	if (!argc)
		return;

	model = Res_GetModel(atoi(argv[0]));

	Console_Printf("Model info (type boneinfo for bone specific stuff):\n\n");

	Console_Printf("name: %s\n", model->name);
	Console_Printf("num_bones: %i\n", model->num_bones);
	Console_Printf("num_meshes: %i\n", model->num_meshes);
	Console_Printf("num_surfs: %i\n", model->num_surfs);
	Console_Printf("num_sprites: %i\n", model->num_sprites);
	Console_Printf("num_skins: %i\n", model->num_skins);
	Console_Printf("num_clips: %i\n", model->num_clips);
	Console_Printf("num_anims: %i\n", model->num_anims);

	Console_Printf("\ntype \"boneInfo %i\" and \"meshInfo %i\" for additional information\n", atoi(argv[0]), atoi(argv[0]));
}

void		Res_BoneInfo_Cmd(int argc, char *argv[])
{
	int n;
	model_t *model;

	if (!argc)
		return;

	model = Res_GetModel(atoi(argv[0]));

	Console_Printf("Bone info:\n\n");

	for (n=0; n<model->num_bones; n++)
	{		
		Console_Printf("bone[%i]: %s", n, model->bones[n].name);
		if (model->bones[n].parent)
		{
			Console_Printf("   (parent of: %s)\n", model->bones[n].parent->name);
		}
		else
		{
			Console_Printf("\n");
		}
	}
}

void		Res_MeshInfo_Cmd(int argc, char *argv[])
{
	int n;
	int totalverts = 0;
	int totalfaces = 0;
	model_t *model;

	if (!argc)
		return;

	model = Res_GetModel(atoi(argv[0]));

	Console_Printf("Mesh info:\n\n");

	for (n=0; n<model->num_meshes; n++)
	{
		mesh_t *mesh = &model->meshes[n];

		Console_Printf("mesh[%i]\n---------\n", n);
		Console_Printf("Name: %s\n", mesh->name);

		if (mesh->bonelink > -1)
		{
			Console_Printf("Mode: single link (linked to bone %i (%s))\n", mesh->bonelink, model->bones[mesh->bonelink].name);
		}
		else
		{
			switch(model->meshes[n].mode)
			{
				case MESH_SKINNED_NONBLENDED:
					Console_Printf("Mode: skinned (non blended)\n");
					break;
				case MESH_SKINNED_BLENDED:
					Console_Printf("Mode: skinned (blended)\n");
					break;
				default:
					Console_Printf("Mode: INVALID!\n");
					break;
			}
		}

		Console_Printf("Default texture: %s\n", mesh->defaultShader);
		Console_Printf("Bounds Min: (%.2f, %.2f, %.2f)\n", mesh->bmin[0], mesh->bmin[1], mesh->bmin[2]);
		Console_Printf("Bounds Max: (%.2f, %.2f, %.2f)\n", mesh->bmax[0], mesh->bmax[1], mesh->bmax[2]);
		Console_Printf("# Faces: %i\n", mesh->num_faces);
		Console_Printf("# Verts: %i\n\n", mesh->num_verts);
		
		totalfaces += mesh->num_faces;
		totalverts += mesh->num_verts;
		
	}

	Console_Printf("TOTAL FACES: %i\n", totalfaces);
	Console_Printf("TOTAL VERTS: %i\n", totalverts);
}



void	Res_FreeShader(residx_t idx)
{	
	Tag_Free(shaders[idx]->name);
	Tag_Free(shaders[idx]);
	shaders[idx] = NULL;
}




void Res_InitMediaLib(mediaLibrary_t *lib, int mediaType, int maxMedia, bool strict, bool canReferenceNULL)
{
	memset(lib, 0, sizeof(*lib));

	lib->type = mediaType;
	lib->strict = strict;
	lib->max_media = maxMedia;
	lib->hash = g_hash_table_new(Res_Hash, Res_Str_Equal);	
	lib->media = Tag_Malloc(4 * (lib->max_media), MEM_MEDIA);
	lib->num_media = 0;
	lib->badhash = g_hash_table_new(Res_Hash, Res_Str_Equal);	
	lib->badcounter = 0;
	lib->canReferenceNULL = canReferenceNULL;
}

int Res_MediaTypeSize(int type)
{
	switch (type)
	{
		case MEDIA_SAMPLE:
			return sizeof(sample_t);
		case MEDIA_SHADER:
			return sizeof(shader_t);
		case MEDIA_FONT:
			return sizeof(fontData_t);
		case MEDIA_MODEL:
			return sizeof(model_t);
		case MEDIA_CLIP:
			return sizeof(modelClip_t);
		case MEDIA_STRINGTABLE:
			return sizeof(stringTable_t);
		default:
			return 0;
	}

}

void Res_SetEmptyFirstSlot(mediaLibrary_t *lib)
{
	lib->media[0] = Tag_Malloc(sizeof(media_t), MEM_MEDIA);
	lib->num_media = 1;
}





/*==========================

  Res_IsMediaLoaded

  determines if an entry exists for the specified resource

  if so, the resource may still not be currently in memory,
  in which case we reload it and return true if reload==true,
  or return false otherwise

 ==========================*/

#define MEDIA_USE_HASH


static bool badMedia = false;

bool	Res_WasBadMedia()
{
	return badMedia;
}

residx_t Res_IsMediaLoaded(const char *filename, mediaLibrary_t *lib, bool reload)
{
	int n;

#ifndef MEDIA_USE_HASH

	char *fullpath = File_GetFullPath(filename);

	for (n=0; n<lib->num_media; n++)
	{
		if (lib->media[n]->d.data)
		{
			if (strcmp(fullpath, lib->media[n]->name)==0)
				return n;
		}
	}

	return 0;

#else
	gpointer table_entry;
	char *fullpath = File_GetFullPath(filename);

	badMedia = false;

	if (res_hashBadFiles.integer)
	{
		if (g_hash_table_lookup(lib->badhash, fullpath))
		{
			badMedia = true;
			return 0;
		}
	}	

	if ( (table_entry = g_hash_table_lookup(lib->hash, fullpath)) )
	{
		n = GPOINTER_TO_UINT(table_entry);

		if (n)
		{
			if (lib->media[n]->d.data)
				return n;
		}
		else
		{
			return 0;

			/*
			//TODO
			if (reload)
			{
				switch(lib->type)
				{
					default:
						System_Error("Don't know how to reload media type %i", lib->type);
						break;
				}
			}
			*/
		}
	}
#endif

	return 0;
}

char	*Res_MediaTypeString(int type)
{
	switch (type)
	{
		case MEDIA_SAMPLE:
			return "MEDIA_SAMPLE";
		case MEDIA_SHADER:
			return "MEDIA_SHADER";
		case MEDIA_FONT:
			return "MEDIA_FONT";
		case MEDIA_MODEL:
			return "MEDIA_MODEL";
		case MEDIA_CLIP:
			return "MEDIA_CLIP";
		case MEDIA_STRINGTABLE:
			return "MEDIA_STRINGTABLE";
		default:
			return "UNKNOWN";
	}
}

//stores an entry for the media type in the library and returns a valid resource index
//if datasize is 0, the "datachunk" pointer is stored directly instead of copying it
residx_t Res_StoreMedia(int type, const char *filename, void *datachunk, int datasize, int memtag, mediaLibrary_t *lib)
{
	residx_t idx;
	media_t *media;
	char *fullpath;

	idx = Res_IsMediaLoaded(filename, lib, true);
	if (idx)
		return idx;	

	fullpath = File_GetFullPath(filename);		//fullpath will only be valid until the next File_GetFullPath() call

	if (strlen(fullpath) >= MEDIA_NAME_LENGTH)
	{
		//fixme: handle this gracefully
		System_Error("Media name length too long: %s\n", fullpath);
	}
	
	if (lib->max_media && lib->num_media >= lib->max_media)
	{
		if (lib->strict)
		{
			//exceeded absolute max allowed, so error out
			System_Error("Exceeded strict maximum for media type %s", Res_MediaTypeString(lib->type));
		}
		//exceeded all allocated pointers, so allocate a few more
		lib->media = Tag_Realloc(lib->media, 4 * (lib->max_media + 64), memtag);
		lib->max_media += 64;
	}

	media = lib->media[lib->num_media] = Tag_Malloc(sizeof(media_t), memtag);	
	strncpySafe(media->name, fullpath, MEDIA_NAME_LENGTH);	
	g_hash_table_insert(lib->hash, media->name, GUINT_TO_POINTER(lib->num_media));

	if (datasize)
	{
		media->d.data = Tag_Malloc(datasize, memtag);	
		memcpy(media->d.data, datachunk, datasize);
	}
	else
	{
		media->d.data = datachunk;
	}

	//the first field in a media type always points to the media tag
	media->d.tag->self = media;

	lib->num_media++;

	return lib->num_media - 1;
}

void	Res_MediaInfo_Cmd(int argc, char *argv[])
{
	int n;
	int num = -1;

	if (argc)
		num = atoi(argv[0]);

	for (n=0; n<NUM_MEDIA_TYPES; n++)
	{
		mediaLibrary_t *lib = &libs[n];

		if (num != n && num != -1)
			continue;

		Console_Printf("%s\n----------\n\n", mediaLibNames[n]);
		Console_Printf("Num loaded:   %i\n", lib->num_media);
		Console_Printf("Num alloc'd:  %i\n", lib->max_media);
		Console_Printf("Strict max:   %i\n", lib->strict);
		Console_Printf("Bad loads:    %i\n", lib->badcounter);
		Console_Printf("\n");
	}
}

void	Res_StoreBadMedia(int type, const char *filename, mediaLibrary_t *lib)
{
	char *old;
	char *store;
	char *fullpath;
	
	if (!res_hashBadFiles.integer)
		return;

	fullpath = File_GetFullPath(filename);
	
	old = lib->badfilenames[lib->badcounter % MAX_BAD_MEDIA];
	store = Tag_Strdup(fullpath, MEM_MEDIA);

	if (old)
	{
		//we have an old entry, cycle it out
		g_hash_table_remove(lib->badhash, old);
		Tag_Free(old);
	}

	g_hash_table_insert(lib->badhash, (gpointer)store, GUINT_TO_POINTER(lib->badcounter));
	lib->badfilenames[lib->badcounter % MAX_BAD_MEDIA] = store;

	lib->badcounter++;
}


/*==========================

  Res_LoadClip

 ==========================*/

residx_t	Res_LoadClip(const char *name)
{
	modelClip_t clip;
	residx_t idx;	

	idx = Res_IsMediaLoaded(name, &libs[MEDIA_CLIP], true);
	if (idx)
		return idx;

	if (Res_WasBadMedia())
		return 0;

	if (!Geom_LoadClip(name, &clip))
	{
		Res_StoreBadMedia(MEDIA_CLIP, name, &libs[MEDIA_CLIP]);
		return 0;
	}

	//store an entry for the clip
	idx = Res_StoreMedia(MEDIA_CLIP, name, &clip, sizeof(modelClip_t), MEM_MODEL, &libs[MEDIA_CLIP]);
	
	Res_UpdateLoadingStatus(name);

	return idx;
}


/*==========================

  Res_LoadSound

 ==========================*/

residx_t	Res_LoadSound(const char *name)
{
	sample_t sample;
	residx_t idx;

	idx = Res_IsMediaLoaded(name, &libs[MEDIA_SAMPLE], true);
	if (idx)
		return idx;

	if (Res_WasBadMedia())
		return 0;

	if (!Sound_LoadSample(name, &sample))
	{
		Res_StoreBadMedia(MEDIA_SAMPLE, name, &libs[MEDIA_SAMPLE]);
		return 0;
	}

	//store an entry for the sample
	idx = Res_StoreMedia(MEDIA_SAMPLE, name, &sample, sizeof(sample_t), MEM_SOUND, &libs[MEDIA_SAMPLE]);

	Res_UpdateLoadingStatus(name);

	return idx;
}

static bool load_model_lods = true;
static residx_t base_lod = 0;

void	Res_LoadModelLODs(residx_t basemodel, const char *name);

residx_t	Res_LoadModel(const char *name)
{
	model_t *model;
	residx_t idx;

	if ( !strcmp(name, "none") )
	{
		return 0;
	}

	idx = Res_IsMediaLoaded(name, &libs[MEDIA_MODEL], true);
	if (idx)
		return idx;

	if (Res_WasBadMedia())
		return 0;

	model = Tag_Malloc(sizeof(model_t), MEM_MODEL);

	if (!Geom_LoadModel(name, model, !load_model_lods, base_lod))
	{
		Tag_Free(model);
		Res_StoreBadMedia(MEDIA_MODEL, name, &libs[MEDIA_MODEL]);
		return 0;
	}

	//store an entry for the model
	idx = Res_StoreMedia(MEDIA_MODEL, name, model, 0, MEM_MODEL, &libs[MEDIA_MODEL]);

	Res_UpdateLoadingStatus(name);

	if (load_model_lods)
		Res_LoadModelLODs(idx, name);

	return idx;
}


void	Res_LoadModelLODs(residx_t basemodel, const char *name)
{
	//LODs are denoted with a _1, _2, or _3 attached to their filename

	residx_t modelidx;
	int n;
	char base[1024];
	model_t *bmodel = Res_GetModel(basemodel);

	strncpySafe(base, name, sizeof(base));
	Filename_StripExtension(base, base);

	bmodel->num_lods = 0;

	for (n=1; n<=MAX_MODEL_LODS; n++)
	{
		char mdlname[1024];
		
		strncpySafe(mdlname, fmt("%s_%i.model", base, n), sizeof(mdlname));

		load_model_lods = false;
		base_lod = basemodel;
		modelidx = Res_LoadModel(mdlname);
		load_model_lods = true;
		base_lod = 0;

		if (modelidx)
		{
			bmodel->lods[bmodel->num_lods].modelidx = modelidx;
			bmodel->lods[bmodel->num_lods].distance = Cvar_GetValue(fmt("gfx_lod%irange", n));
			if (bmodel->lods[bmodel->num_lods].distance <= 0)
			{
				Console_Printf("WARNING: model LOD %s was loaded with bad distance range!\n", name);
			}
			bmodel->num_lods++;
		}
	}
}



/*==========================

  Res_UnloadSound

 ==========================*/

void	Res_UnloadSound(residx_t idx)
{

}


/*==========================

  Res_LoadStringTable

 ==========================*/

residx_t	Res_LoadStringTable(const char *name)
{
	stringTable_t stringtable;
	int idx;

	residx_t ret = Res_IsMediaLoaded(name, &libs[MEDIA_STRINGTABLE], true);
	if (ret)
		return ret;

	if (Res_WasBadMedia())
		return 0;

	if (!Str_LoadStringTable(name, &stringtable))
	{
		Res_StoreBadMedia(MEDIA_STRINGTABLE, name, &libs[MEDIA_STRINGTABLE]);
		return 0;
	}

	//store an entry for the stringtable
	idx = Res_StoreMedia(MEDIA_STRINGTABLE, name, &stringtable, sizeof(stringTable_t), MEM_STRINGTABLE, &libs[MEDIA_STRINGTABLE]);

	Res_UpdateLoadingStatus(name);

	return idx;
}

void	*Res_LibRef(int mediatype, int index)
{
	if (index < 0 || index >= libs[mediatype].num_media)
	{
		if (libs[mediatype].canReferenceNULL)
			return NULL;
		else
			return libs[mediatype].media[0]->d.data;
	}
	else
		return libs[mediatype].media[index]->d.data;
}

modelClip_t		*Res_GetClip(residx_t idx)
{
	return Res_LibRef(MEDIA_CLIP, idx);
}

stringTable_t	*Res_GetStringTable(residx_t idx)
{
	return Res_LibRef(MEDIA_STRINGTABLE, idx);
}

sample_t		*Res_GetSample(residx_t idx)
{
	return Res_LibRef(MEDIA_SAMPLE, idx);
}

model_t			*Res_GetModel(residx_t idx)
{
	return Res_LibRef(MEDIA_MODEL, idx);
}

bool		Res_UnloadShader(residx_t idx)
{
	if (idx < 0 || idx >= MAX_SHADERS)
		return false;

	if (shaders[idx])
	{
		if (!g_hash_table_remove(shader_hash, shaders[idx]->name))
			Console_DPrintf("Warning: Couldn't find %s in the hash table\n");
		Vid_UnregisterShader(shaders[idx]);
		Res_FreeShader(idx);
	}

	return true;
}


void		Res_UnloadAllSounds()
{
	int n;

	Console_DPrintf("Res_UnloadAllSounds()\n");

	for (n=0; n<libs[MEDIA_SAMPLE].num_media; n++)
	{
		if (libs[MEDIA_SAMPLE].media[n]->d.data)
		{
			Res_UnloadSound(n);
		}
	}
}

char		*Res_IsShaderFile(char *name)
{
	const char *s;
	char ext[2048];

	s = Filename_GetExtension(name);
	if (!s) return NULL;

	strncpySafe(ext, s, 2048);  //so we can lowercase it
	strlwr(ext);

	if (strcmp(ext, "tga")==0 || 
		strcmp(ext, "png")==0 || 
		strcmp(ext, "jpg")==0 || 
		strcmp(ext, "thumb") == 0 ||
		strcmp(ext, "bik") == 0 ||
		strcmp(ext, "ogg") == 0)
		return (char *)s;

	return NULL;

	//todo: add other file types when supported
}

void	Res_GetShaderInfoFromFilename(const char *name, shader_t *shader)
{
	const char *fname = Filename_GetFilename((char *)name);
	const char *ext = Filename_GetExtension((char*)name);

	if (strncmp(fname, "1_", 2)==0 || strstr(fname, "_1_"))
	{
		shader->srcblend = BLEND_ONE;
		shader->dstblend = BLEND_ONE;
	}
	else
	{
		shader->srcblend = BLEND_SRC_ALPHA;
		shader->dstblend = BLEND_ONE_MINUS_SRC_ALPHA;
	}

	if (strncmp(fname, "nl_", 3)==0 || strstr(fname, "_nl_"))
	{
		shader->flags |= SHD_NO_LIGHTING;
	}

	if (strncmp(fname, "ng_", 3)==0 || strstr(fname, "_ng_"))
	{
		shader->flags |= SHD_NO_FOLIAGE;
	}

	if (strncmp(ext, "bik", 3) == 0)
	{
		shader->maptype = SHADER_BINKVIDEO;
	}
	else if (strncmp(ext, "ogg", 3) == 0)
	{
		shader->maptype = SHADER_THEORAVIDEO;
	}
	else if (strstr(fname, "000."))
	{
		shader->maptype = SHADER_ANIMATED_TEXTURE_EX;
	}
	else if (strstr(fname, "00."))
	{
		shader->maptype = SHADER_ANIMATED_TEXTURE;
	}

	if (strncmp(fname, "t_", 2)==0 || strstr(fname, "_t_"))
	{
		shader->flags |= SHD_REPEAT;
	}

	if (strncmp(fname, "m_", 2)==0 || strstr(fname, "_m_"))
	{
		shader->flags |= SHD_TERRAIN_MIPMAPS;
	}

	if (strncmp(fname, "nm_", 3)==0 || strstr(fname, "_nm_"))
	{
		shader->flags &= ~SHD_TERRAIN_MIPMAPS;
	}
	if (strncmp(fname, "nfm_", 3)==0 || strstr(fname, "_nfm_"))
	{
		shader->flags |= SHD_NO_MIPMAPS;
	}

	if (strncmp(fname, "pre_", 4) == 0 || strstr(fname, "_pre_"))
	{
		shader->flags |= SHD_MOVIE_PRELOAD_ALL;
	}
}

int		Res_GetNumTextureFrames(residx_t shader)
{
	shader_t *s = Res_GetShader(shader);

	return s->texmap_num;
}

int		Res_GetAnimatedTextureFPS(residx_t shader)
{
	shader_t *s = Res_GetShader(shader);

	if (s->fps)
		return s->fps;
	else
		return 30;
}


int		Res_GetMoviePlayCount(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (!shader->movieid)
		return -1;
		
	return shader->numplays;
}

void	Res_TheoraStop(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (shader->movieid)
		Theora_Stop(shader);
}

void	Res_TheoraContinue(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (shader->movieid)
		Theora_Continue(shader);
}

void	Res_TheoraRestart(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (shader->movieid)
		Theora_Restart(shader);
}

void	Res_TheoraUnload(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (shader->movieid)
		Theora_Unload(shader);
	shader->active = false;
	shader->movieid = 0;
}

void	Res_BinkStop(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (shader->movieid)
		Bink_Stop(shader);
}

void	Res_BinkContinue(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (shader->movieid)
		Bink_Continue(shader);
}

void	Res_BinkRestart(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (shader->movieid)
		Bink_Restart(shader);
}

void	Res_BinkUnload(residx_t shadernum)
{
	shader_t *shader = Res_GetShader(shadernum);

	if (shader->movieid)
		Bink_Unload(shader);
	shader->active = false;
	shader->movieid = 0;
}

residx_t	Res_LoadRawTextureEx(const char *name, int flags)
{
	Console_DPrintf("Res_LoadRawTextureEx(%s): ", name);

	shaders[shader_count] = Tag_Malloc(sizeof(shader_t), MEM_NEEDS_TO_USE_NEW_LIB_SYSTEM);
	memset(shaders[shader_count], 0, sizeof(shader_t));
	shaders[shader_count]->name = Tag_Strdup(name, MEM_NEEDS_TO_USE_NEW_LIB_SYSTEM);
	shaders[shader_count]->maptype = SHADER_SINGLE_TEXTURE;
	shaders[shader_count]->flags = flags;
	shaders[shader_count]->fps = 15;
	
	Res_GetShaderInfoFromFilename(name, shaders[shader_count]);

	if (!Vid_RegisterShader(shaders[shader_count]))
	{
		Console_DPrintf("Vid_RegisterShader() failed\n");
		Res_FreeShader(shader_count);		
		return -1;
	}

	//add this to the shader hash table
	g_hash_table_insert(shader_hash, (gpointer)shaders[shader_count]->name, GUINT_TO_POINTER(shader_count));
	//Console_DPrintf("hash: shader %s hashes to %u\n", name, g_str_hash(name));

	shader_count++;

	Console_DPrintf("ok\n", name);	

	return shader_count-1;
}

residx_t	Res_LoadRawTexture(const char *name)
{
	return Res_LoadRawTextureEx(name, 0);
}

residx_t	Res_LoadRawTextureFromMemoryEx(int shader_to_use, bitmap_t *bmp, int flags)
{
	if (shader_to_use <= 0)
	{
		
		if (shader_count > MAX_SHADERS) shader_count = MAX_SHADERS;
	
		//Console_DPrintf("Res_LoadRawTextureFromMemoryEx()");

		if (shader_count >= MAX_SHADERS)
		{
			Console_DPrintf("couldn't allocate shader\n");
			return 0;
		} else {
			//Console_DPrintf("\n");
		}
		shader_to_use = shader_count;
	}
	else
	{		
		Res_UnloadShader(shader_to_use);
	}

	shaders[shader_to_use] = Tag_Malloc(sizeof(shader_t), MEM_NEEDS_TO_USE_NEW_LIB_SYSTEM);
	memset(shaders[shader_to_use], 0, sizeof(shader_t));
	shaders[shader_to_use]->name = Tag_Strdup(fmt("-no-filename-%i", shader_to_use), MEM_NEEDS_TO_USE_NEW_LIB_SYSTEM);
	shaders[shader_to_use]->maptype = SHADER_SINGLE_TEXTURE;
	shaders[shader_to_use]->flags = flags;
	shaders[shader_to_use]->fps = 15;	
	
	shaders[shader_to_use]->srcblend = BLEND_SRC_ALPHA;
	shaders[shader_to_use]->dstblend = BLEND_ONE_MINUS_SRC_ALPHA;
	//Res_GetShaderInfoFromFilename(name, shaders[shader_to_use]);

	if (!Vid_RegisterShaderImage(shaders[shader_to_use], bmp))
	{
		Console_DPrintf("Vid_RegisterShaderImage() failed\n");
		Res_FreeShader(shader_to_use);		
		return hostmedia.whiteShader;		
	}

	//add this to the shader hash table
	g_hash_table_insert(shader_hash, (gpointer)shaders[shader_to_use]->name, GUINT_TO_POINTER(shader_to_use));

	if (shader_to_use == shader_count)
		shader_count++;	

	return shader_to_use;
}



//game code will call this function
//example: sceneobj.skin = corec.Res_LoadSkin(sceneobj.model, "default");
int		Res_LoadSkin(residx_t model, const char *name)
{
	int n;
	model_t *m;

	if (dedicated_server.integer)		//dedicated server doesn't care about skins
		return 0;

	m = Res_GetModel(model);

	for (n=0; n<m->num_skins; n++)
	{
		if (stricmp(m->skins[n].name, name)==0)
		{						
			Geom_LoadSkinShaders(m, n);
			return n;
		}
	}

	Geom_LoadSkinShaders(m, 0);

	return 0;
}



void		Res_GetModelVisualBounds(residx_t model, vec3_t bmin, vec3_t bmax)
{
	model_t *mdl = Res_GetModel(model);

	if (mdl)
	{
		M_CopyVec3(mdl->bmin, bmin);
		M_CopyVec3(mdl->bmax, bmax);
	}
}

bool	Res_GetModelSurfaceBounds(residx_t model, vec3_t bmin, vec3_t bmax)
{
	int surfnum;
	model_t *mdl = Res_GetModel(model);

	if (!mdl)
		return false;

	M_ClearBounds(bmin, bmax);

	for (surfnum=0; surfnum<mdl->num_surfs; surfnum++)
	{
		convexPolyhedron_t *surf;
		surf = &mdl->surfs[surfnum];
		if (surf->flags & SURF_MODELBOUNDS)
			continue;
		M_AddPointToBounds(surf->bmin, bmin, bmax);
		M_AddPointToBounds(surf->bmax, bmin, bmax);
	}

	if (mdl->num_surfs)
		return true;
	return false;
}




void	Res_SortShadersByType()
{
	int n;
	int d = 0;

	//opaque, normal textures render first
	for (n=0; n<shader_count; n++)
	{
		if (shaders[n])
		{
			if (shaders[n]->srcblend == BLEND_SRC_ALPHA && shaders[n]->dstblend == BLEND_ONE_MINUS_SRC_ALPHA && !shaders[n]->translucent)
				shaders[n]->sortedidx = d++;				
		}

		if (d > MAX_SHADER_BUCKETS)
			d = 0;
	}

	//translucent textures render next
	for (n=0; n<shader_count; n++)
	{
		if (shaders[n])
		{
			if (shaders[n]->srcblend == BLEND_SRC_ALPHA && shaders[n]->dstblend == BLEND_ONE_MINUS_SRC_ALPHA && shaders[n]->translucent)
				shaders[n]->sortedidx= d++;			
		}

		if (d > MAX_SHADER_BUCKETS)
			d = 0;
	}
	//additive texture render next
	for (n=0; n<shader_count; n++)
	{
		if (shaders[n])
		{
			if (shaders[n]->srcblend == BLEND_ONE && shaders[n]->dstblend == BLEND_ONE)
				shaders[n]->sortedidx = d++;			
		}

		if (d > MAX_SHADER_BUCKETS)
			d = 0;
	}
}

residx_t	Res_LoadShader(const char *name)
{	
	return Res_LoadShaderEx(name, 0);
}

#define MAX_BAD_SHADERS 256

int	Res_StringToUVFunc(const char *string)
{
	if (stricmp(string, "SIN_SCALE")==0)
		return UV_SIN_SCALE;
	else if (stricmp(string, "LINEAR_SCALE")==0)
		return UV_LINEAR_SCALE;
	else if (stricmp(string, "SIN_PAN")==0)
		return UV_SIN_PAN;
	else if (stricmp(string, "LINEAR_PAN")==0)
		return UV_LINEAR_PAN;
	
	return UV_NONE;
}

void	Res_ResetShaderVars()
{
	Cvar_SetVar(&mat_shininess, "0");
//	Cvar_SetVar(&mat_ambient, "0.5 0.5 0.5 1.0");
	Cvar_SetVar(&mat_specular, "0.0 0.0 0.0 1.0");
	Cvar_SetVar(&mat_emission, "0.0 0.0 0.0 1.0");
	Cvar_SetVar(&mat_uFunc, "NONE");
	Cvar_SetVar(&mat_vFunc, "NONE");
	Cvar_SetVar(&mat_wFunc, "NONE");
	Cvar_SetVar(&mat_uSpeed, "0");
	Cvar_SetVar(&mat_vSpeed, "0");
	Cvar_SetVar(&mat_wSpeed, "0");
	Cvar_SetVar(&mat_uPhase, "0");
	Cvar_SetVar(&mat_vPhase, "0");
	Cvar_SetVar(&mat_wPhase, "0");
	Cvar_SetVar(&mat_uAmplitude, "1");
	Cvar_SetVar(&mat_vAmplitude, "1");
	Cvar_SetVar(&mat_wAmplitude, "1");
	Cvar_SetVar(&mat_doubleSided, "0");
}

void	Res_CopyVarsToShader(shader_t *shader)
{
	shader->shininess = mat_shininess.value;
//	StringToVec4(mat_ambient.string, shader->ambient);
	StringToVec4(mat_specular.string, shader->specular);
	StringToVec4(mat_emission.string, shader->emission);
	shader->uv_func[0] = Res_StringToUVFunc(mat_uFunc.string);
	shader->uv_func[1] = Res_StringToUVFunc(mat_vFunc.string);
	shader->uv_func[2] = Res_StringToUVFunc(mat_wFunc.string);
	shader->uv_speed[0] = mat_uSpeed.value;
	shader->uv_speed[1] = mat_vSpeed.value;
	shader->uv_speed[2] = mat_wSpeed.value;
	shader->uv_phase[0] = mat_uPhase.value;
	shader->uv_phase[1] = mat_vPhase.value;
	shader->uv_phase[2] = mat_wPhase.value;
	shader->uv_amplitude[0] = mat_uAmplitude.value;
	shader->uv_amplitude[1] = mat_vAmplitude.value;
	shader->uv_amplitude[2] = mat_wAmplitude.value;
	shader->doubleSided = mat_doubleSided.integer;

	if (shader->uv_func[0] || shader->uv_func[1] || shader->uv_func[2])
		shader->animateTexCoords = true;
}

void	Res_LoadMatFile(const char *fname, shader_t *shader)
{	
	Res_ResetShaderVars();
	Cmd_ReadConfigFile(fname, false);
	Res_CopyVarsToShader(shader);
}

residx_t	Res_LoadShaderEx(const char *filename, int flags)
{
	static int badshader_count = 0;
	static char *badshaders[MAX_BAD_SHADERS];
	char *ext;
	residx_t ret = 0;
	int n;
	gpointer table_entry;
	
	if (!gfx.integer)
		return 0;
	
	if (shader_count > MAX_SHADERS) shader_count = MAX_SHADERS;

	if ((table_entry = g_hash_table_lookup(shader_hash, filename)))
		return GPOINTER_TO_UINT(table_entry);

	for (n=0; n<badshader_count; n++)
	{
		if (strcmp(filename, badshaders[n])==0)
		{
			return 0;
		}
	}

	if (shader_count >= MAX_SHADERS)
	{
		Console_DPrintf("couldn't allocate shader\n");
		return 0;
	}

	Res_UpdateLoadingStatus(filename);

	if (!File_Exists(filename))
	{
		Console_DPrintf("Shader file %s doesn't exist\n", filename);
		goto badshader;
	}

	ext = Res_IsShaderFile((char*)filename);
	if (!ext)
		return 0;
	
	ret = Res_LoadRawTextureEx(filename, flags);		
	if (ret == -1)
		goto badshader;

	if (File_Exists(fmt("%s.mat", filename)))
	{
		Res_LoadMatFile(fmt("%s.mat", filename), shaders[ret]);
		//Res_GetShaderInfoFromFilename(filename, shaders[ret]);	//filename flags should override .mat settings
	}
	else
	{
		Res_ResetShaderVars();
		Res_CopyVarsToShader(shaders[ret]);
		//Res_GetShaderInfoFromFilename(filename, shaders[ret]);	//filename flags should override .mat settings
	}

	Res_SortShadersByType();

	//todo: load .shader file
	return ret;

badshader:
	//hack - don't add clan icons to the bad shader list, since they could appear later
	if (strstr(filename, "clans/"))
		return 0;
	
	if (badshader_count < MAX_BAD_SHADERS)
	{
		badshaders[badshader_count] = Tag_Strdup(filename, MEM_NEEDS_TO_USE_NEW_LIB_SYSTEM);
		badshader_count++;
	}
	return 0;
}

residx_t	Res_LoadTerrainShader(const char *name)
{
	return Res_LoadShaderEx(name, SHD_TERRAIN_MIPMAPS | SHD_REPEAT);
}

residx_t	Res_LoadFont(const char *filename, int small_size, int medium_size, int big_size)
{
	int i;
	int idx = font_count;
	int sizes[MAX_FONT_SIZES];
	if (font_count > MAX_FONTS)
		return 0;

	for (idx = 0; idx < MAX_FONTS; idx++)
	{
		if (fonts[idx] && fonts[idx]->active)
			continue;

		break;
	}
	
	if (fonts[idx] && fonts[idx]->active)
	{
		Console_Errorf("Too many fonts loaded, loading failed for font %s\n", filename);
		return 0;
	}
	
	sizes[0] = small_size;
	sizes[1] = medium_size;
	sizes[2] = big_size;
	fonts[idx] = Font_LoadFace(filename, sizes);
	if (!fonts[idx])
	{
#ifndef _S2_EXCLUDE_FREETYPE
		System_Error("critical error: failed to load font file %s\n", filename);
#endif
		return 0;
	}
	for (i = 0; i < MAX_FONT_SIZES; i++)
		fonts[idx]->fontShaders[i] = Res_LoadRawTextureFromMemoryEx(0, fonts[idx]->bmps[i], SHD_FULL_QUALITY | SHD_NO_MIPMAPS | SHD_NO_COMPRESS); 

	font_count++;
	return idx + 1;
}

bool		Res_RefreshFont(residx_t idx)
{
	fontData_t *fontData;
	int i;

	fontData = Res_GetFont(idx);
	if (!fontData)
		return false;
	
	for (i = 0; i < MAX_FONT_SIZES; i++)
	{
		fontData->fontShaders[i] = Res_LoadRawTextureFromMemoryEx(fontData->fontShaders[i], fontData->bmps[i], SHD_FULL_QUALITY | SHD_NO_MIPMAPS | SHD_NO_COMPRESS); 
		//Bitmap_WritePNG(fmt("font%i.png", i), fontData->bmps[i]);
	}

	return true;
}

bool		Res_AddCharToFont(residx_t idx, int charnum)
{
	fontData_t *fontData;

	fontData = Res_GetFont(idx);
	if (!fontData)
		return false;
	
	if (!Font_AddChar(fontData, charnum))
		return false;
	
	Host_RefreshFont(idx);

	return true;
}

bool		Res_UnloadFont(residx_t idx)
{
	int i;
	int index = idx - 1;

	if (index < 0 || index > MAX_FONTS)
		return false;
	if (!fonts[index] || !fonts[index]->active)
		return false;
	
	for (i = 0; i < MAX_FONT_SIZES; i++)
		Res_UnloadShader(fonts[index]->fontShaders[i]);
	Font_Unload(fonts[index]);
	fonts[index] = NULL;
	font_count--;
	return true;
}


// zip stuff

bool		Res_RegisterZip(const char *name)
{
	return false;
}




//#endif

fontData_t	*Res_GetFont(residx_t idx)
{
	int index = idx -1;

	if (index >= 0 && index < MAX_FONTS)
		return fonts[index];
	else
	{
		Console_DPrintf("Res_GetFont: Invalid index spedified (%i)\n", idx);
		return NULL;
	}
}

shader_t	*Res_GetShader(residx_t idx)
{
	int index = idx;

	if (index>=0 && index<shader_count)
		if (shaders[index] && shaders[index]->active)
			return shaders[index];
	
	return shaders[0];
}


/*
model_t		*Res_GetModelLOD(residx_t idx, int lod)
{
	int index = idx - MODEL_RESIDX_OFFSET;	

	if (lod < 0 || lod > 3)
		return NULL;

	if (index>=0 && index<MAX_MODELS)
	{
		if (lod>=0 && lod<4)
		{
			return Res_GetModel(modelLods[index][lod]);
		}		
	}

	return NULL;
}
*/


//odd function just to get the dynamap as a shader
residx_t	Res_GetDynamapShader(residx_t shader)
{
	bitmap_t bmp;

	memset(&bmp, 0, sizeof(bitmap_t));
	
	WR_DynamapToBitmap(&bmp);
	Bitmap_DesaturateToAlpha(&bmp);
	shader = Res_LoadRawTextureFromMemoryEx(shader, &bmp, SHD_FULL_QUALITY | SHD_NO_MIPMAPS | SHD_NO_COMPRESS);
	Tag_Free(bmp.data);

	return shader;
}

void		(*loadingStatusCB)() = NULL;

void		Res_SetLoadingStatusCallback(void(*callback)(const char *resourcename))
{
	loadingStatusCB = callback;
}

void		Res_UpdateLoadingStatus(const char *name)
{
	if (!loadingStatusCB)
		return;
	if ((DLLTYPE == DLLTYPE_EDITOR && !world.cl_loaded) ||
		(DLLTYPE == DLLTYPE_GAME && (localClient.cstate < CCS_IN_GAME && localClient.cstate > CCS_DISCONNECTED) && host_runLoadingFrame && loadingStatusCB))	
	{
		System_ProcessEvents();
		Vid_BeginFrame();
		loadingStatusCB(name);
		Vid_EndFrame();
	}
}


void		Res_Init()
{
	Cmd_Register("shaderlist", Res_ShaderList_Cmd);
	Cmd_Register("modellist", Res_ModelList_Cmd);
	Cmd_Register("soundlist", Res_SoundList_Cmd);
	Cmd_Register("modelinfo", Res_ModelInfo_Cmd);
	Cmd_Register("boneinfo", Res_BoneInfo_Cmd);
	Cmd_Register("meshinfo", Res_MeshInfo_Cmd);
	Cmd_Register("mediainfo", Res_MediaInfo_Cmd);

	Cmd_Register("reloadanim", Res_ReloadAnim_Cmd);

	Cvar_Register(&mat_shininess);
	Cvar_Register(&mat_ambient);
	Cvar_Register(&mat_specular);
	Cvar_Register(&mat_emission);
	Cvar_Register(&mat_uPhase);
	Cvar_Register(&mat_vPhase);
	Cvar_Register(&mat_wPhase);
	Cvar_Register(&mat_uSpeed);
	Cvar_Register(&mat_vSpeed);
	Cvar_Register(&mat_wSpeed);
	Cvar_Register(&mat_uFunc);
	Cvar_Register(&mat_vFunc);
	Cvar_Register(&mat_wFunc);
	Cvar_Register(&mat_uAmplitude);
	Cvar_Register(&mat_vAmplitude);
	Cvar_Register(&mat_wAmplitude);
	Cvar_Register(&mat_doubleSided);

	Cvar_Register(&res_hashBadFiles);

	//
	// MEDIA LIBRARIES
	//

	//model library
	//never returns NULL
	//no strict maximum
	Res_InitMediaLib(&libs[MEDIA_MODEL], MEDIA_MODEL, MAX_MODELS, false, false);
	//shader library
	//never returns NULL
	//strict maximum
	Res_InitMediaLib(&libs[MEDIA_SHADER], MEDIA_SHADER, MAX_SHADERS, true, false);
	//sample library
	//can return NULL
	//no strict maximum
	Res_InitMediaLib(&libs[MEDIA_SAMPLE], MEDIA_SAMPLE, MAX_SOUNDS, false, true);
	//clip library
	//can return NULL
	//no strict maximum
	Res_InitMediaLib(&libs[MEDIA_CLIP], MEDIA_CLIP, MAX_CLIPS, false, true);

	Res_InitMediaLib(&libs[MEDIA_FONT], MEDIA_FONT, MAX_FACES, false, false);

	Res_InitMediaLib(&libs[MEDIA_STRINGTABLE], MEDIA_STRINGTABLE, MAX_STRINGTABLES, false, true);


	//fixme: get rid of this code and move over to the new medialib system
	memset(shaders, 0, sizeof(shaders));	
	memset(fonts, 0, sizeof(fonts));

	//create hash tables for the models, sounds, and textures (based on filename)
	shader_hash = g_hash_table_new(Res_Hash, Res_Str_Equal);

	//load null models / textures
	//Res_LoadModel("/game/testmodels/light.model");
	shader_count = 0;
	font_count = 0;
	//fonts[0]->name = "NULL_FONT";

	//set first slot to empty so a Load command never uses slot 0
	//MEDIA_MODEL and MEDIA_SHADER will not do this because they load
	//valid media into slot 0 (used for the "not found" resource)
	Res_SetEmptyFirstSlot(&libs[MEDIA_CLIP]);
	Res_SetEmptyFirstSlot(&libs[MEDIA_STRINGTABLE]);
	Res_SetEmptyFirstSlot(&libs[MEDIA_SAMPLE]);
	Res_SetEmptyFirstSlot(&libs[MEDIA_FONT]);
}
