// (C) 2001 S2 Games

// world_objects.c

// provides an interface between the game code and core engine for dealing with ingame objects
// these are the objects put into the level editor and are deciphered by the game code when the game begins
// the LE will actual use this module in a dynamic fashion to organize objects as they are placed into the world
// the game code will only read the info at the beginning of a game and handle the objects itself from there

#include "core.h"
#include "savage_types.h"
#include "navrep.h"
#include "zip.h"

#define	MAX_OBJECT_DEFS		256

#define	MAX_GROUNDPLANE_VERTS 512
#define MAX_GROUNDPLANE_FACES 128

#define MAX_REFERENCE_OBJECTS	256

cvarContainer_t			objdefs[MAX_OBJECT_DEFS];
int						objdef_num = 0;
worldObject_t			objects[MAX_WORLDOBJECTS];
int						worldobj_base_index = 0;
int						useobjdef_index = 0;
referenceObject_t		reference_objects[MAX_REFERENCE_OBJECTS];

bool					server_loading_world = false;

//.objdef variables
cvar_t		obj_model = { "obj_model", "", CVAR_PATH_TO_FILE };
cvar_t		obj_skin = { "obj_skin", "" };
cvar_t		obj_name = { "obj_name", "NO_NAME" };
cvar_t		obj_sound = { "obj_sound", "" };
cvar_t		obj_soundVolume = { "obj_soundVolume", "1.0" };
cvar_t		obj_soundMinFalloff = { "obj_soundMinFalloff", "10" };
cvar_t		obj_editorScaleRangeLo = { "obj_editorScaleRangeLo", "1" };
cvar_t		obj_editorScaleRangeHi = { "obj_editorScaleRangeHi", "1" };
cvar_t		obj_editorCategory = { "obj_editorCategory", "General" };
cvar_t		obj_reference = { "obj_reference", "0" };
cvar_t		obj_navFloodFillPoint = { "obj_navFloodFillPoint", "0" };
cvar_t		obj_navBridge = { "obj_navBridge", "0" };
cvar_t		obj_navBridgeWide = { "obj_navBridgeWide", "0" };
cvar_t		obj_invisibleToGame = { "obj_invisibleToGame", "0" };

cvar_t		wo_translucent = { "wo_translucent", "0" /*CVAR_READONLY*/ };
cvar_t		wo_translucentAmount = { "wo_translucentAmount", "0.2" /*CVAR_READONLY*/ };
cvar_t		wo_brightnessFactor = { "wo_brightnessFactor", "1" };
cvar_t		wo_fixObjectRotation = { "wo_fixObjectRotation", "0", CVAR_CHEAT };
cvar_t		wo_animate = { "wo_animate", "0", CVAR_SAVECONFIG };

void	WO_MakeObjdef(const char *objclass, const char *objdefName)
{
	cvarContainer_t *objcfg;

	objcfg = CvarContainer_Find(objdefName, objdefs, objdef_num);

	if (objcfg)
	{
		Console_DPrintf("WO_MakeObjdef: objdef %s already exists\n", objdefName);
		return;
	}
	
	if (objdef_num >= MAX_OBJECT_DEFS)
	{
		Console_DPrintf("WO_MakeObjdef: exceeded MAX_OBJECT_DEFS (%i)\n", MAX_OBJECT_DEFS);
		return;
	}

	if (!CvarContainer_Alloc(objclass, objdefName, &objdefs[objdef_num]))
		return;

	objdef_num++;

	return;
}

bool	WO_UseObjdef(const char *objdefName)
{
	cvarContainer_t *objcfg;

	objcfg = CvarContainer_Find(objdefName, objdefs, objdef_num);

	if (!objcfg)
	{
		Console_DPrintf("WO_UseObjdef: object def %s does not exist\n", objdefName);
		return false;
	}

	CvarContainer_SetCvars(objcfg);

	useobjdef_index = objcfg - objdefs;
	if (useobjdef_index > objdef_num || useobjdef_index < 0)
		useobjdef_index = -1;

	return true;
}

void	WO_UseObjdef_Cmd(int argc, char *argv[])
{
	if (!argc)
		return;

	WO_UseObjdef(argv[0]);
}



int	WO_GetNumObjdefs()
{
	return objdef_num;
}

char	*WO_GetObjdefName(int n)
{
	if (n < 0 || n >= objdef_num)
		return "";

	return objdefs[n].name;
}


int	WO_GetObjdefId(const char *name)
{
	int n;

	for (n=0; n<objdef_num; n++)
	{
		if (strcmp(objdefs[n].name, name)==0)
			return n;
	}

	return -1;
}

//register a variable for use in the object config
int	WO_RegisterObjectVar(const char *objclass, const char *varname)
{
	return CvarContainer_RegisterVar(objclass, varname);
}



void	WO_CreateObjectClass(const char *objclass)
{
	if (!CvarContainer_CreateClass(objclass))
	{
		Console_DPrintf("WO_CreateObjectClass: class %s already exists\n");
		return;
	}

	//if anything here is changed you MUST reflect the change in objectVarIDs_enum
	WO_RegisterObjectVar(objclass, "obj_model");
	WO_RegisterObjectVar(objclass, "obj_skin");
	WO_RegisterObjectVar(objclass, "obj_name");
	WO_RegisterObjectVar(objclass, "obj_sound");
	WO_RegisterObjectVar(objclass, "obj_soundVolume");
	WO_RegisterObjectVar(objclass, "obj_soundMinFalloff");
	WO_RegisterObjectVar(objclass, "obj_editorScaleRangeLo");
	WO_RegisterObjectVar(objclass, "obj_editorScaleRangeHi");
	WO_RegisterObjectVar(objclass, "obj_editorCategory");
	WO_RegisterObjectVar(objclass, "obj_reference");
	WO_RegisterObjectVar(objclass, "obj_navFloodFillPoint");
	WO_RegisterObjectVar(objclass, "obj_navBridge");
	WO_RegisterObjectVar(objclass, "obj_navBridgeWide");
	WO_RegisterObjectVar(objclass, "obj_invisibleToGame");
}


void	WO_CreateClasses()
{
	WO_CreateObjectClass("blocker");
}




/*==========================

  WO_WriteObjposFile

  writes out a config file with object information

  occluders go here too

 ==========================*/

void	WO_WriteObjposFile(void *zipfile, const char *filename)
{
	int i, n, ret;
	char line[1024];
	int method = Z_DEFLATED;
	int level = Z_DEFAULT_COMPRESSION;

	if ((ZIPW_AddFileInZip(zipfile, filename, NULL, NULL, 0, NULL, 0, NULL, method, level)) != ZIP_OK)
    {
		Console_DPrintf("Failed to open %s in zip\n", filename);
		return;
	}

	for (n=0; n<MAX_WORLDOBJECTS; n++)
	{
		if (!objects[n].active)
			continue;

		BPrintf(line, sizeof(line), "%s %s %.1f %.1f %.1f %.1f %.3f %.1f %.1f\n",
				objects[n].reference ? "createReference" : "createObject",
				objects[n].name,
				objects[n].objpos.position[X] * (1/world_scale.value),
				objects[n].objpos.position[Y] * (1/world_scale.value),
				objects[n].objpos.position[Z] * (1/world_heightscale.value),
				objects[n].objpos.rotation[Z],
				objects[n].objpos.scale,
				objects[n].objpos.rotation[X],
				objects[n].objpos.rotation[Y]);

		if ((ret = ZIPW_WriteFileInZip(zipfile, line, strlen(line))) < 0)
			Console_DPrintf("Error %i writing to zip file\n", ret);
	}

	for (n=0; n<world.numOccluders; n++)
	{
		BPrintf(line, sizeof(line), "createOccluder %i", world.occluders[n].numpoints);

		for (i=0; i<world.occluders[n].numpoints; i++)	
		{
			strcat(line, fmt(" %f %f %f",
							world.occluders[n].points[i][0],
							world.occluders[n].points[i][1],
							world.occluders[n].points[i][2]
							));
		}

		strcat(line, "\n");

		if ((ret = ZIPW_WriteFileInZip(zipfile, line, strlen(line))) < 0)
			Console_DPrintf("Error %i writing to zip file\n", ret);
	}

	ret = ZIPW_CloseFileInZip(zipfile);
	if (ret!=ZIP_OK)
		Console_DPrintf("error in closing %s in the zipfile\n",filename);
}


bool	beginObjDef = false;
char	beginObjDefClass[256];

void	WO_Begin_ObjDef_Cmd(int argc, char *argv[])
{
	//reset all object config vars to their default value
	
	if (!argc)
	{
		Console_DPrintf("syntax: beginObjdef [object_class] [objectdef_name]\n");
		return;
	}

	beginObjDef = true;

	strncpy(beginObjDefClass, argv[0], 255);

	CvarContainer_ResetCvars(argv[0]);
}

void	WO_End_ObjDef_Cmd(int argc, char *argv[])
{
	if (!beginObjDef)
	{
		Console_DPrintf("endObjdef must be preceeded by beginObjdef\n");
		return;
	}
	if (!argc)
	{
		Console_DPrintf("syntax: endObjdef [objectdef_name]\n");
		return;
	}

	WO_MakeObjdef(beginObjDefClass, argv[0]);

	beginObjDef = false;
}

char	*WO_GetObjdefVar(int objdef_id, const char *varname)
{
	if (objdef_id < 0 || objdef_id >= objdef_num)
		return "";

	return CvarContainer_GetString(&objdefs[objdef_id], CvarContainer_GetVarID(&objdefs[objdef_id], varname));
}

void	WO_SetObjdefVar(int objdef_id, const char *varname, const char *string)
{
	if (objdef_id < 0 || objdef_id >= objdef_num)
		return;

	CvarContainer_Set(&objdefs[objdef_id], CvarContainer_GetVarID(&objdefs[objdef_id], varname), string);
}

float	WO_GetObjdefVarValue(int objdef_id, const char *varname)
{
	if (objdef_id < 0 || objdef_id >= objdef_num)
		return 0;

	return CvarContainer_GetFloat(&objdefs[objdef_id], CvarContainer_GetVarID(&objdefs[objdef_id], varname));
}

char	*WO_GetObjdefVarString(int objdef_id, const char *varname)
{
	if (objdef_id < 0 || objdef_id >= objdef_num)
		return 0;

	return CvarContainer_GetString(&objdefs[objdef_id], CvarContainer_GetVarID(&objdefs[objdef_id], varname));
}

//macro uglies (-:

#define GET_OBJECT(id) objects[id].active ? &objects[id] : NULL;
				       
#define OBJECT_FUNC_BOOL \
	worldObject_t *obj; \
	if (id < 0 || id >= MAX_WORLDOBJECTS) return false; \
	obj = GET_OBJECT(id); \
	if (!obj) \
		return false;

#define OBJECT_FUNC_VOID \
	worldObject_t *obj; \
	if (id < 0 || id >= MAX_WORLDOBJECTS) return; \
	obj = GET_OBJECT(id); \
	if (!obj) \
		return;

#define OBJECT_FUNC_INT \
	worldObject_t *obj; \
	if (id < 0 || id >= MAX_WORLDOBJECTS) return 0; \
	obj = GET_OBJECT(id); \
	if (!obj) \
		return 0;





bool	WO_GetObjectBounds(int id, vec3_t bmin, vec3_t bmax)
{
	OBJECT_FUNC_BOOL

	M_CopyVec3(obj->bmin, bmin);
	M_CopyVec3(obj->bmax, bmax);

	return true;
}


bool	WO_GetObjectPos(int id, objectPosition_t *objpos)
{
	OBJECT_FUNC_BOOL

	Mem_Copy(objpos, &obj->objpos, sizeof(objectPosition_t));

	return true;
}


#if 0

//this groundplane code is not particularly elegant...

int	fitpoly_objid = 0;
int fitpoly_numverts = 0;
int fitpoly_numfaces = 0;
int fitpoly_allocatedverts = 0;
int fitpoly_allocatedfaces = 0;
bool fitpoly_ignore = false;

void	WO_FitPolyCallback(int nverts, scenefacevert_t verts[], residx_t shader, int flags)
{
	worldObject_t *obj = &objects[fitpoly_objid];

	if (fitpoly_ignore)
		return;

	fitpoly_numverts += nverts;
	if (fitpoly_numverts > fitpoly_allocatedverts)
	{
		//alloc 64 more verts
		int numnew = 64;

		if (nverts > 64)
			numnew = nverts;

		if (fitpoly_allocatedverts + numnew > MAX_GROUNDPLANE_VERTS)
		{
			Console_DPrintf("Warning: exceeded MAX_GROUNDPLANE_VERTS for object %i\n", fitpoly_objid);
			fitpoly_ignore = true;
			return;
		}

		obj->groundplaneVerts = Mem_ReAlloc(obj->groundplaneVerts, sizeof(scenefacevert_t) * (fitpoly_allocatedverts + numnew));
		fitpoly_allocatedverts += numnew;
	}

	fitpoly_numfaces++;
	if (fitpoly_numfaces > fitpoly_allocatedfaces)
	{
		//alloc 16 more faces
		int numnew = 16;

		if (fitpoly_allocatedfaces + numnew > MAX_GROUNDPLANE_FACES)
		{
			Console_DPrintf("Warning: exceeded MAX_GROUNDPLANE_FACES for object %i\n", fitpoly_objid);
			fitpoly_ignore = true;
			return;
		}

		obj->groundplaneFaces = Mem_ReAlloc(obj->groundplaneFaces, sizeof(projectedFaceInfo_t) * (fitpoly_allocatedfaces + numnew));
		fitpoly_allocatedfaces += numnew;
	}

	memcpy(&obj->groundplaneVerts[fitpoly_numverts-nverts], verts, sizeof(scenefacevert_t) * nverts);
	obj->groundplaneFaces[fitpoly_numfaces-1].start = fitpoly_numverts-nverts;
	obj->groundplaneFaces[fitpoly_numfaces-1].numverts = nverts;
	obj->num_groundplaneFaces = fitpoly_numfaces;
}

#endif

//rebuilds the polygons underneath the object that make up the 'groundplane', if one was specified in the model file
void	WO_UpdateGroundPlane(int id)
{
#if 0
	model_t *model;
	mdlsprite_t *groundplane;
	float halfwidth;
	float halfheight;

	float left, right, top, bottom;
	scenefacevert_t verts[4];
	vec3_t quad[4];
//	OVERHEAD_INIT;

	OBJECT_FUNC_VOID

	if (!obj->skin || !obj->model)
		return;

	model = Res_GetModel(obj->model);

	if (model->groundplane < 0)
		return;

	groundplane = &model->sprites[model->groundplane];
	if (!groundplane || groundplane->type != S2SPRITE_GROUNDPLANE)
		return;

	//alloc verts...if we need more, we'll realloc in FitPolyCallback
	if (obj->groundplaneVerts)
		Tag_Free(obj->groundplaneVerts);

	obj->groundplaneVerts = Mem_Alloc(sizeof(scenefacevert_t) * 64);
	obj->groundplaneFaces = Mem_Alloc(sizeof(projectedFaceInfo_t) * 16);

	halfwidth = groundplane->width / 2.0;
	halfheight = groundplane->height / 2.0;

	left = (-halfwidth + groundplane->pos[0][0]) * obj->objpos.scale;
	right = (halfwidth + groundplane->pos[0][0]) * obj->objpos.scale;
	top = (-halfheight + groundplane->pos[0][1]) * obj->objpos.scale;
	bottom = (halfheight + groundplane->pos[0][1]) * obj->objpos.scale;
	SET_VEC3(quad[0], left, top, 0);
	SET_VEC3(quad[1], left, bottom, 0);
	SET_VEC3(quad[2], right, bottom, 0);
	SET_VEC3(quad[3], right, top, 0);

	M_TransformPoint(quad[0], obj->objpos.position, (const vec3_t *)obj->axis, verts[3].vtx);
	M_TransformPoint(quad[1], obj->objpos.position, (const vec3_t *)obj->axis, verts[2].vtx);
	M_TransformPoint(quad[2], obj->objpos.position, (const vec3_t *)obj->axis, verts[1].vtx);
	M_TransformPoint(quad[3], obj->objpos.position, (const vec3_t *)obj->axis, verts[0].vtx);

	SET_VEC4(verts[0].col, 255, 255, 255, 255);
	SET_VEC2(verts[0].tex, 1, 1);

	SET_VEC4(verts[1].col, 255, 255, 255, 255);
	SET_VEC2(verts[1].tex, 1, 0);

	SET_VEC4(verts[2].col, 255, 255, 255, 255);
	SET_VEC2(verts[2].tex, 0, 0);

	SET_VEC4(verts[3].col, 255, 255, 255, 255);
	SET_VEC2(verts[3].tex, 0, 1);

	//project the quad onto the terrain
	fitpoly_objid = id;
	fitpoly_numverts = 0;
	fitpoly_numfaces = 0;
	fitpoly_allocatedverts = 64;
	fitpoly_allocatedfaces = 16;
	fitpoly_ignore = false;
	obj->groundplaneShader = Res_GetShaderFromSkin(obj->skin, groundplane->name);
	WT_FitPolyToTerrain(verts, 4, obj->groundplaneShader, POLY_LIGHTING, WO_FitPolyCallback);
	
//	OVERHEAD_COUNT(OVERHEAD_GEOM);

#endif
}

int         WO_GetObjectObjdef(int id)
{
	OBJECT_FUNC_INT

	return obj->objdef_idx;
}

residx_t	WO_GetObjectModel(int id)
{
	OBJECT_FUNC_INT

	return obj->model;
}

void	WO_SetObjectModelAndSkin(int id, residx_t model, residx_t skin)
{
	OBJECT_FUNC_VOID

	if (obj->model != model || obj->skin != skin)
	{
		obj->model = model;
		obj->skin = skin;

		WO_UpdateGroundPlane(id);
	}
}

residx_t	WO_GetObjectSkin(int id)
{
	OBJECT_FUNC_INT

	return obj->skin;
}

bool		WO_IsObjectActive(int id)
{
	OBJECT_FUNC_BOOL

	return true;
}

bool		WO_IsObjectDoubleSided(int id)
{
	OBJECT_FUNC_BOOL

	return true;
}

bool		WO_IsObjectReference(int id)
{
	OBJECT_FUNC_BOOL

	return obj->reference;
}

residx_t	WO_GetObjectSound(int id)
{
	OBJECT_FUNC_INT

	return obj->sound;
}

float		WO_GetObjectSoundVolume(int id)
{
	OBJECT_FUNC_INT

	return obj->soundVolume;
}

sound_handle_t		WO_GetObjectSoundHandle(int id)
{
	OBJECT_FUNC_INT

	return obj->soundIdx;
}

bool		WO_SetObjectSoundHandle(int id, sound_handle_t handle)
{
	OBJECT_FUNC_INT

	obj->soundIdx = handle;
	return true;
}


void WO_UnlinkObjectFromCollisionSystem(int id)
{
	int n;

	OBJECT_FUNC_VOID

	for (n=0; n<obj->num_surfs; n++)
	{
		WT_UnlinkPolyhedron(obj->surfs[n]);
	}

	obj->num_surfs = 0;
}


bool WO_LinkObjectToCollisionSystem(int id)
{
	residx_t modelid;
	model_t *model;
	int n;
	worldObject_t *obj = &objects[id];

	modelid = objects[id].model;
	model = Res_GetModel(modelid);

	M_GetAxis(obj->objpos.rotation[X], obj->objpos.rotation[Y], obj->objpos.rotation[Z], obj->axis);

	if (!model->num_surfs)
		return true;	//fixme: we might want to add a simple bounding box surface instead of not adding anything
	if (model->num_surfs > SURF_LIMIT)
		return false;

	obj->num_surfs = 0;

	M_ClearBounds(obj->bmin, obj->bmax);	

	for (n=0; n<model->num_surfs; n++)
	{	
		linkedSurface_t *linkedsurf = WT_LinkPolyhedron(&model->surfs[n], id, modelid, obj->objpos.scale, obj->objpos.position, (const vec3_t *)obj->axis);
		if (!linkedsurf)
		{
			return false;
		}
		else
		{
			obj->surfs[obj->num_surfs] = linkedsurf;
			obj->num_surfs++;
			M_AddPointToBounds(linkedsurf->bmin_w, obj->bmin, obj->bmax);
			M_AddPointToBounds(linkedsurf->bmax_w, obj->bmin, obj->bmax);
		}
	}

	

	return true;
}

void	WO_DeleteObject_Server(int id)
{
	if ( id < MAX_OBJECTS )
	{
		NavRep_CSGAdd(localServer.gameobjs[id]);
	}

	WO_DeleteObject(id);
}

bool	WO_CreateObject_Server(int objdef, const char *optmodel, const char *optskin, objectPosition_t *pos, int id)
{
	float width, height;
	bool ret = WO_CreateObject(objdef, optmodel, optskin, pos, id);
	
	if (objects[id].active)
		objects[id].flags |= WO_SERVER_CREATED;

	width = objects[id].bmax[0] - objects[id].bmin[0];
	height = objects[id].bmax[1] - objects[id].bmin[1];

	if (ret)
	{
		if (localServer.gameobjs[id])		//should always be true
		{
			Res_GetModelSurfaceBounds(objects[id].model, localServer.gameobjs[id]->bmin, localServer.gameobjs[id]->bmax);
		}

		if ( id < MAX_OBJECTS )
		{
			NavRep_CSGSubtract(localServer.gameobjs[id]);
		}
	}


	return ret;
}

bool	WO_CreateObject_Client(int objdef, const char *optmodel, const char *optskin, objectPosition_t *pos, int id)
{
	bool ret = WO_CreateObject(objdef, optmodel, optskin, pos, id);

	if (objects[id].active)
	{
		objects[id].flags |= WO_CLIENT_CREATED;
	
		WO_UpdateGroundPlane(id);
	}

	return ret;
}


//called from the editor / game when placing objects
bool		WO_CreateObject(int objdef, const char *optmodel, const char *optskin, objectPosition_t *pos, int id)
{
	if (world.loaded)
	{
		//don't allow creation of objects outside the index range of the game objects array
		if ((id < 0 || id >= MAX_OBJECTS) && DLLTYPE != DLLTYPE_EDITOR)
			return false;
		if (objdef > -1 && objdef < objdef_num)
		{
			//use the original objdef to create the object instead of the current cvar values
			WO_UseObjdef(objdefs[objdef].name);
		}
		//else
		//	objdef = useobjdef_index;
	}
	else
	{
		//don't allow creation of objects from the .objpos file to intefere with the game objects array
		if ((id < MAX_OBJECTS || id >= MAX_WORLDOBJECTS) && DLLTYPE != DLLTYPE_EDITOR)
			return false;
	}

	if (!pos)
		return false;

	if (objects[id].active)
		return false;
	
	if (!world.loaded)
	{
		objects[id].flags |= WO_CLIENT_CREATED;		//if we're creating the object during a level load, set the client creation flag so the client will render the object
	}	

	//CvarContainer_Alloc(objdefs[objdef].classname, objdefs[objdef].name, &objects[id].config);
	Mem_Copy(&objects[id].objpos, pos, sizeof(objectPosition_t));


	if (objdef < objdef_num && objdef >= 0)
	{ 
		objects[id].model = Res_LoadModel(CvarContainer_GetString(&objdefs[objdef], OBJVAR_MODEL));	
		objects[id].skin = Res_LoadSkin(objects[id].model, CvarContainer_GetString(&objdefs[objdef], OBJVAR_SKIN));	
		objects[id].sound = Res_LoadSound(CvarContainer_GetString(&objdefs[objdef], OBJVAR_SOUND));	
		objects[id].soundVolume = CvarContainer_GetFloat(&objdefs[objdef], OBJVAR_SOUNDVOLUME);
		objects[id].invisibleToGame = CvarContainer_GetInteger(&objdefs[objdef], OBJVAR_INVISIBLETOGAME);
	}
	else
	{
		objects[id].model = Res_LoadModel(optmodel);
		objects[id].skin = Res_LoadSkin(objects[id].model, optskin);
	}

//	if (!objects[id].model)
	//	return false;
	
	objects[id].objdef_idx = objdef;
	objects[id].createTime = (int)(rand() % 1000);
	if (objdef > -1 && objdef < objdef_num)
	{
		objects[id].reference = CvarContainer_GetInteger(&objdefs[objdef], CvarContainer_GetVarID(&objdefs[objdef], "obj_reference"));
		strcpy(objects[id].name, objdefs[objdef].name);
	}

	if (!WO_LinkObjectToCollisionSystem(id))
		return false;

	objects[id].shouldRender = true;

	objects[id].active = true;

	return true;
}


void	WO_DeleteObject(int id)
{
	OBJECT_FUNC_VOID

	if (obj->num_surfs)
	{
		WO_UnlinkObjectFromCollisionSystem(id);
	}
	//free any groundplane geometry
	Tag_Free(obj->groundplaneFaces);
	Tag_Free(obj->groundplaneVerts);
		
	memset(obj, 0, sizeof(worldObject_t));
}

void	WO_DeleteObject_Client(int id)
{
	OBJECT_FUNC_VOID

	if (localServer.active)
		return;

	WO_DeleteObject(id);
}

void	WO_ClearObjects()
{
	int n;

	for (n=0; n<MAX_WORLDOBJECTS; n++)
	{
		if (!objects[n].active)
			continue;

		WO_DeleteObject(n);
	}

	memset(reference_objects, 0, sizeof(reference_objects));
}

bool	WO_ArgsToObjectPosition(int argc, char *argv[], objectPosition_t *pos)
{
	if (argc < 3)
		return false;

	//casting position to int here to fix network coordinate problems
	pos->position[X] = (int)atof(argv[0]);
	pos->position[Y] = (int)atof(argv[1]);
	pos->position[Z] = (int)atof(argv[2]);

	M_MultVec3(pos->position, world_scale.value, pos->position);

	if (*argv[3])
	{
		M_SetVec3(pos->rotation, 0, 0, 0);
			
		if (wo_fixObjectRotation.integer)		//fix to convert old worlds to new coord system
			pos->rotation[Z] = -atof(argv[3])+180;
		else
			pos->rotation[Z] = atof(argv[3]);

		if (argc > 5)
		{
			pos->rotation[X] = atof(argv[5]);
			if (argc > 6)
				pos->rotation[Y] = atof(argv[6]);

			//normalize rotation
			while (pos->rotation[X] > 360)
				pos->rotation[X] -= 360;
			while (pos->rotation[X] < 0)
				pos->rotation[X] += 360;

			//normalize rotation
			while (pos->rotation[Y] > 360)
				pos->rotation[Y] -= 360;
			while (pos->rotation[Y] < 0)
				pos->rotation[Y] += 360;
		}
		
		//normalize rotation
		while (pos->rotation[Z] > 360)
			pos->rotation[Z] -= 360;
		while (pos->rotation[Z] < 0)
			pos->rotation[Z] += 360;
	}
	else
		M_SetVec3(pos->rotation, 0, 0, 0);

	if (argc > 4 && *argv[4])
		pos->scale = atof(argv[4]);
	else
		pos->scale = 1;

	return true;
}




/*==========================

  WO_CreateOccluder_Cmd

  this is called from the .objpos file when loading a world

 ==========================*/

void	WO_CreateOccluder_Cmd(int argc, char *argv[])
{
	int n,i;
	int arg = 0;
	occluder_t occluder;

	if (argc < 4)
		return;

	occluder.numpoints= atoi(argv[0]);

	if (occluder.numpoints >= MAX_OCCLUDER_POINTS || occluder.numpoints < 0)
		return;

	arg = 1;
	for (n=0; n<occluder.numpoints; n++)
	{
		for (i=0; i<3; i++)
		{
			occluder.points[n][i] = atof(argv[arg++]);
		}
	}

	World_AddOccluder(&occluder);
}

/*==========================

  WO_CreateObject_Cmd

  this is called from the .objpos file when loading a world  

 ==========================*/

void	WO_CreateObject_Cmd(int argc, char *argv[])
{
	//syntax: createobject objdef_name x y z [rot] [scale]

	int id;
	objectPosition_t pos;
	int objdef;

	if (argc < 4)
		return;

	if (world.loaded)
	{
 		Console_DPrintf("Object creation not allowed outside of .objpos\n");
		return;
	}

	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		for (id=0; id<MAX_OBJECTS; id++)
		{
			if (!objects[id].active)
				break;
		}

		if (id == MAX_OBJECTS)
		{
			Console_Errorf("Too many objects!\n");
			return;
		}
	}
	else
	{
		for (id=MAX_OBJECTS; id<MAX_WORLDOBJECTS; id++)
		{
			if (!objects[id].active)
				break;
		}

		if (id == MAX_WORLDOBJECTS)
		{
			Game_Error("Exceeded MAX_WORLDOBJECTS\n");
			return;
		}
	}

	//find the object definition
	objdef = WO_GetObjdefId(argv[0]);
	if (objdef == -1)
		return;
	
	WO_ArgsToObjectPosition(argc-1, &argv[1], &pos);

	if (server_loading_world)
		WO_CreateObject_Server(objdef, NULL, NULL, &pos, id);
	else
		WO_CreateObject(objdef, NULL, NULL, &pos, id);
}


referenceObject_t *WO_CreateReference(const char *refname, objectPosition_t *pos)
{
	int n;

	for (n=0; n<MAX_REFERENCE_OBJECTS; n++)
	{
		if (!reference_objects[n].active)
			break;
	}

	if (n == MAX_REFERENCE_OBJECTS)
	{
		Console_DPrintf("Exceeded max reference objects\n");
		return NULL;
	}

	memset(&reference_objects[n], 0, sizeof(referenceObject_t));

	reference_objects[n].pos = *pos;
	reference_objects[n].active = true;
	strncpySafe(reference_objects[n].refname, refname, sizeof(reference_objects[n].refname));	

	return &reference_objects[n];
}

static referenceObject_t *last_refobj = NULL;

void	WO_CreateReference_Cmd(int argc, char *argv[])
{
	objectPosition_t pos;

	//editor loads reference objects as real objects
	if (DLLTYPE == DLLTYPE_EDITOR)
	{
		WO_CreateObject_Cmd(argc, argv);
		return;
	}

	if (!WO_ArgsToObjectPosition(argc-1, &argv[1], &pos))
	{
		Console_DPrintf("createReference: not enough arguments\n");
		return;
	}

	last_refobj = WO_CreateReference(argv[0], &pos);
}

int WO_FindReferenceObject(int startindex, const char *refname, referenceObject_t *refobj)
{
	int n;

	if (startindex < 0 || startindex >= MAX_REFERENCE_OBJECTS)
		return -1;

	for (n=startindex; n<MAX_REFERENCE_OBJECTS; n++)
	{
		if (reference_objects[n].active)
		{
			if (strcmp(refname, reference_objects[n].refname)==0)
			{
				*refobj = reference_objects[n];
				return n;
			}
		}
	}

	return -1;
}

void	WO_RefSet_Cmd(int argc, char *argv[])
{
	int n;
	char catargs[1024] = "";

	if (!last_refobj)
		return;
	if (!last_refobj->active)
		return;

	if (argc < 2)
		return;

	strcpy(catargs, argv[1]);

	for (n=2; n<argc; n++)
	{
		strcat(catargs, " ");
		strcat(catargs, argv[n]);
	}

	ST_SetState(last_refobj->info, argv[0], catargs, sizeof(last_refobj->info));
}

extern cvar_t wr_marks;

void	WO_DrawGroundPlane(worldObject_t *obj)
{
	int n;

	if (!wr_marks.integer)
		return;

	for (n=0; n<obj->num_groundplaneFaces; n++)
	{
		Scene_AddPoly(obj->groundplaneFaces[n].numverts, &obj->groundplaneVerts[obj->groundplaneFaces[n].start], obj->groundplaneShader, POLY_LIGHTING);
	}
}


void		WO_RenderObjects(vec3_t cameraPos)
{
	int n;
	sceneobj_t sc;
	memset(&sc, 0, sizeof(sceneobj_t));

	//add all world occluders to the scene.  this must be done every frame.
	Scene_AddWorldOccluders();

	for (n=MAX_OBJECTS; n<MAX_WORLDOBJECTS; n++)
	{
		int ft;
 		float time;
		bvec4_t dyncol;
		worldObject_t *obj = &objects[n];

		if (!obj->shouldRender)
			continue;
		if (!(obj->flags & WO_CLIENT_CREATED))	//don't render this object until we call CreateObject from the client game code
			continue;		
		
		time = ((float)Host_Milliseconds() / 1000.0) * 30 + obj->createTime;
		ft = (int)time;

		//do the object sound before we cull the object out based on distance or fog of war
		//this is mainly so things will sound good in commander mode
		if (obj->sound)
			Sound_AddLoopingSoundEx(obj->sound, n, obj->soundVolume, PRIORITY_LOWEST);		

		Sound_MoveSource(n, obj->objpos.position, NULL);

		if (obj->invisibleToGame && DLLTYPE == DLLTYPE_GAME)
			continue;

		if (M_GetDistanceSq(cameraPos, obj->objpos.position) > (gfx_farclip.value + 500) * (gfx_farclip.value + 500))
			continue;

		//aggregious fog of war hack
		WR_GetDynamap((int)WORLD_TO_GRID(obj->objpos.position[0]), (int)WORLD_TO_GRID(obj->objpos.position[1]), dyncol);
		if (dyncol[0] < 30 && dyncol[1] < 30 && dyncol[2] < 30)
			continue;

		//if (n>=MAX_OBJECTS)							//we let the client render out dynamic objects
		//{
			memset(&sc, 0, sizeof(sceneobj_t));

			sc.scale = obj->objpos.scale;
			
			memcpy(sc.axis, obj->axis, sizeof(vec3_t)*3);
			//M_GetAxis(0,0,obj->objpos.rotation[Z],sc.axis);
			//sc.angle[2] = obj->objpos.rotation[Z];
			sc.model = obj->model;
			sc.skin = obj->skin;
			sc.objtype = OBJTYPE_MODEL;
			//sc.animTime = 
			//sc.loframe = ft;
			//sc.hiframe = (ft+1);
			//sc.lerp_amt = time - ft;
			if (wo_animate.integer)
			{
				Geom_BeginPose(&obj->skeleton, sc.model);
				Geom_SetBoneAnim("", "idle", Host_Milliseconds() + (n*200), Host_Milliseconds(), 0, 0);
				Geom_EndPose();
				sc.skeleton = &obj->skeleton;
			}

			sc.flags |= SCENEOBJ_USE_AXIS;// | SCENEOBJ_USE_ALPHA;

			M_CopyVec3(obj->objpos.position, sc.pos);

			if (wo_translucent.integer)
			{
				sc.alpha = wo_translucentAmount.value;
				if (sc.alpha < 1.0)
					sc.flags |= SCENEOBJ_USE_ALPHA;
			}			

			Scene_AddObject(&sc);
			//sc.flags &= ~SCENEOBJ_NO_ZWRITE;
			//Scene_AddObject(&sc);
		//}

		if (obj->num_groundplaneFaces)
			WO_DrawGroundPlane(&objects[n]);
	}
}

void	WO_Init()
{
	memset(objdefs, 0, sizeof(objdefs));
	objdef_num = 0;

	Cmd_Register("useObjdef", WO_UseObjdef_Cmd);
	Cmd_Register("createObject", WO_CreateObject_Cmd);
	Cmd_Register("createReference", WO_CreateReference_Cmd);
	Cmd_Register("createOccluder", WO_CreateOccluder_Cmd);
	Cmd_Register("refSet", WO_RefSet_Cmd);

	//these commands are used for defining objdefs in the .objdef files
	Cmd_Register("beginObjdef", WO_Begin_ObjDef_Cmd);
	Cmd_Register("endObjdef", WO_End_ObjDef_Cmd);

	Cvar_Register(&obj_model);
	Cvar_Register(&obj_skin);
	Cvar_Register(&obj_name);
	Cvar_Register(&obj_sound);
	Cvar_Register(&obj_soundVolume);
	Cvar_Register(&obj_soundMinFalloff);
	Cvar_Register(&obj_editorScaleRangeLo);
	Cvar_Register(&obj_editorScaleRangeHi);
	Cvar_Register(&obj_editorCategory);
	Cvar_Register(&obj_reference);
	Cvar_Register(&obj_navFloodFillPoint);
	Cvar_Register(&obj_navBridge);
	Cvar_Register(&obj_navBridgeWide);
	Cvar_Register(&obj_invisibleToGame);
	Cvar_Register(&wo_translucent);
	Cvar_Register(&wo_translucentAmount);
	Cvar_Register(&wo_brightnessFactor);
	Cvar_Register(&wo_fixObjectRotation);
	Cvar_Register(&wo_animate);

	WO_CreateClasses();
}

