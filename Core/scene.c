// (C) 2003 S2 Games

// scene.c

// Scene management functions (adding/removing scene objects)

/*
	here we manage a linked list of objects which are added to
	the scene from the client side game code.  When the client
	calls Scene_Render(), we render each object in the list

	All Draw_* functions pass over the scene list and go
	directly to rendering
*/

#define TIGHT_CLIP

#include "core.h"


cvar_t	gfx_sceneStats = { "gfx_sceneStats", "0" };
cvar_t	gfx_cullOBB = { "gfx_cullOBB", "1" };




#define	FR_TOP		0
#define FR_LEFT		1
#define	FR_BOTTOM	2
#define FR_RIGHT	3
#define FR_FAR		4
#define FR_NEAR		5


#define MAX_SKYOBJECTS 4

typedef struct
{
	int numplanes;
	plane_t planes[MAX_OCCLUDER_POINTS+1];
	float influence;
	occluder_t *occ;
} occluderVolume_t;

sceneStats_t sceneStats;

scenelist_t *scenelist;
scenelist_t *skylist;
scenelist_t *spritelist;
scenefacelist_t *scenefacelist;
scenelightlist_t *scenelightlist;
occluderlist_t *occluderlist;

plane_t	frustum[6];
extern		line_t	frustum_edges[6][3];
vec3_t	frustum_bmin;
vec3_t	frustum_bmax;

#define OCCLUDER_LIST_SIZE 32

static occluderVolume_t scene_occluders[OCCLUDER_LIST_SIZE];
static int scene_num_occluders;

float	selectionRect_x1, selectionRect_x2, selectionRect_y1, selectionRect_y2;
float		draw_shadertime;
camera_t	*scene_cam = NULL;

//the following cvars specify various parameters for the
//scene.  it's up to the graphics driver code (i.e. gl_scene.c)
//to deal with these variables
cvar_t	gfx_fog =				{ "gfx_fog",			"0",		CVAR_WORLDCONFIG | CVAR_CHEAT };  //enable/disable fog
cvar_t	gfx_fog_far =			{ "gfx_fog_far",		"3000",		CVAR_WORLDCONFIG | CVAR_CHEAT };
cvar_t	gfx_fog_near =			{ "gfx_fog_near",		"1500",		CVAR_WORLDCONFIG | CVAR_CHEAT };
cvar_t	gfx_fogr =				{ "gfx_fogr",			"1",		CVAR_WORLDCONFIG | CVAR_CHEAT };
cvar_t	gfx_fogg =				{ "gfx_fogg",			"1",		CVAR_WORLDCONFIG | CVAR_CHEAT };
cvar_t	gfx_fogb =				{ "gfx_fogb",			"1",		CVAR_WORLDCONFIG | CVAR_CHEAT };
cvar_t  gfx_nearclip =			{ "gfx_nearclip",		"2" };
cvar_t	gfx_farclip =			{ "gfx_farclip",		"10000",	CVAR_WORLDCONFIG | CVAR_CHEAT };
cvar_t	gfx_polycount =			{ "gfx_polycount",		"0" };
cvar_t	gfx_sky =				{ "gfx_sky",			"1" };
cvar_t	gfx_soundSpace =		{ "gfx_soundSpace",		"100",		CVAR_SAVECONFIG };
cvar_t	gfx_objects =			{ "gfx_objects",		"1" };
cvar_t	gfx_toon =				{ "gfx_toon",			"0" };
cvar_t	gfx_lockfrustum =		{ "gfx_lockfrustum",	"0" };
cvar_t gfx_dbgplanes =			{ "gfx_dbgplanes",		"6" };
cvar_t	gfx_selectionScale =	{ "gfx_selectionScale",	"0.7" };
cvar_t	gfx_nocull =			{ "gfx_nocull",			"0" };
cvar_t	gfx_dynamicLighting =	{ "gfx_dynamicLighting","1" };
cvar_t	gfx_maxDLights =		{ "gfx_maxDLights",		"6" };
cvar_t	gfx_occlusion =			{ "gfx_occlusion",		"1" };
cvar_t	gfx_maxSceneOccluders = { "gfx_maxSceneOccluders", "6" };
cvar_t	gfx_drawOccluders =		{ "gfx_drawOccluders", "0", CVAR_CHEAT };

//LOD ranges
cvar_t	gfx_lod1range = { "gfx_lod1range", "300"  };
cvar_t	gfx_lod2range = { "gfx_lod2range", "900" };
cvar_t	gfx_lod3range = { "gfx_lod3range", "1500" };


static char	*framepool;
static int	fp_marker = 0;


/*==========================

  Scene_FramePool_Alloc

 ==========================*/

void	*Scene_FramePool_Alloc(int size)
{
	void *newmem;

	if (fp_marker + size > SCENE_FRAMEPOOL_SIZE)
	{
		Game_Error("Scene_FramePool_Alloc: pool size of %i bytes exceeded (couldn't allocate %i bytes)\n", SCENE_FRAMEPOOL_SIZE, size);
		return NULL;
	}

	newmem = &framepool[fp_marker];

	fp_marker += (size + 63) & ~63;	//align to 64 byte intervals

	return newmem;
}


/*==========================

  Scene_FramePool_Clear

  

 ==========================*/

void	Scene_FramePool_Clear()
{
	fp_marker = 0;
}


/*==========================

  Scene_InitFramePool

  

 ==========================*/

void	Scene_InitFramePool()
{
	framepool = Tag_Malloc(SCENE_FRAMEPOOL_SIZE, MEM_SCENE);
}


/*==========================

  Scene_AddObject

  add an object to be rendered

 ==========================*/

void	Scene_AddObject (sceneobj_t *scobj)
{
	scenelist_t		*newobj;

	newobj = Scene_FramePool_Alloc(sizeof(scenelist_t));
	if (!newobj)
		return;

	Mem_Copy(&newobj->obj, scobj, sizeof(sceneobj_t));
	
	//Vid_NormalizeColor(scobj->color, newobj->obj.color);	
	if (!(scobj->flags & SCENEOBJ_USE_AXIS))
	{
		//fill in axis
		M_GetAxis(scobj->angle[0], scobj->angle[1], scobj->angle[2], newobj->obj.axis);
	}

	if (newobj->obj.scale == 0)
		newobj->obj.scale = 1;		//fix objects that didn't fill in their scaling field		

	newobj->cull = false;

	switch(scobj->objtype)
	{
	case OBJTYPE_BILLBOARD:
	case OBJTYPE_BEAM:
		newobj->next = spritelist;
		spritelist = newobj;
		break;
	default:
		newobj->next = scenelist;
		scenelist = newobj;
		break;
	}

	sceneStats.sceneObjCount++;
}


/*==========================

  Scene_AddSkyObj

  sky objects are always rendered before any other object, and in the order
  they are passed

 ==========================*/

void	Scene_AddSkyObj (sceneobj_t *skyobj)
{
	scenelist_t		*newobj;

	newobj = Scene_FramePool_Alloc(sizeof(scenelist_t));
	if (!newobj)
		return;

	Mem_Copy(newobj, skyobj, sizeof(sceneobj_t));

	newobj->next = skylist;
	newobj->obj.flags |= SCENEOBJ_SKYBOX;
	Vid_NormalizeColor(skyobj->color, newobj->obj.color);
	if (!(skyobj->flags & SCENEOBJ_USE_AXIS))
	{
		//fill in axis
		M_GetAxis(skyobj->angle[0], skyobj->angle[1], skyobj->angle[2], newobj->obj.axis);
	}

	skylist = newobj;
}




/*==========================

  Scene_AddPoly

 ==========================*/

void	Scene_AddPoly (int nverts, scenefacevert_t *verts, residx_t shader, int flags)
{
	scenefacelist_t	*newface;

	newface = Scene_FramePool_Alloc(sizeof(scenefacelist_t));
	if (!newface)
		return;

	newface->verts = Scene_FramePool_Alloc(nverts * sizeof(scenefacevert_t));
	if (!newface->verts)
		return;

	Mem_Copy(newface->verts, verts, nverts * sizeof(scenefacevert_t));
	newface->nverts = nverts;
	newface->shader = shader;
	newface->flags = flags;

	newface->next = scenefacelist;
	scenefacelist = newface;
}


/*==========================

  Scene_AddLight

 ==========================*/

void	Scene_AddLight (scenelight_t *light)
{
	scenelightlist_t *newlight;

	newlight = Scene_FramePool_Alloc(sizeof(scenelightlist_t));
	if (!newlight)
		return;

	Mem_Copy(newlight, light, sizeof(scenelight_t));

	newlight->next = scenelightlist;
	newlight->cull = true;
	scenelightlist = newlight;
}


/*==========================

  Scene_AddOccluder

 ==========================*/

void	Scene_AddOccluder (occluder_t *occluder)
{
	occluderlist_t *newoccluder;

	newoccluder = Scene_FramePool_Alloc(sizeof(occluder_t));
	if (!newoccluder)
		return;

	Mem_Copy(newoccluder, occluder, sizeof(occluder_t));

	newoccluder->next = occluderlist;
	newoccluder->cull = false;
	occluderlist = newoccluder;
}


/*==========================

  Scene_AddWorldOccluders

  called every frame to add world occluders

 ==========================*/

void	Scene_AddWorldOccluders()
{
	int n;

	if (!world.loaded)
		return;

	for (n=0; n<world.numOccluders; n++)
	{
		Scene_AddOccluder(&world.occluders[n]);
	}
}


/*==========================

  Scene_Raytrace_Cmd

  simple raytracer so we can debug collision surfaces

 ==========================*/

void	Scene_Raytrace_Cmd(int argc, char *argv[])
{
#ifndef _S2_DONT_INCLUDE_GL
	int x, y;
	int width, height;
	float ratio_x, ratio_y;
	bitmap_t bmp;
	float msec;


	if (!scene_cam)
		return;

	if (argc < 2)
	{
		width = 640;
		height = 480;
	}
	else
	{
		width = atoi(argv[0]);
		height = atoi(argv[1]);
	}

	ratio_x = scene_cam->width / width;
	ratio_y = scene_cam->height / height;

	Bitmap_Alloc(&bmp, width, height, BITMAP_RGBA);

	msec = System_Milliseconds();

	for (y=0; y<height; y++)
	{
		float msec;
		msec = System_Milliseconds();

		for (x=0; x<width; x++)
		{

			vec3_t dir;
			vec3_t end;
			float light;
			traceinfo_t trace;

			Cam_ConstructRay(scene_cam, (float)x*ratio_x, (float)y*ratio_y, dir);
			M_PointOnLine(scene_cam->origin, dir, gfx_farclip.value, end);
			WT_TraceBox_Client(&trace, scene_cam->origin, end, zero_vec, zero_vec, 0);//SURF_TERRAIN);//SURF_TERRAIN);
			if (trace.fraction < 1)
			{
				if (argc == 3)
					light = trace.fraction * 255.0;
				else
					light = ABS(M_DotProduct(trace.normal, cam->viewaxis[FORWARD])) * 255.0;
			}
			else
			{
				light = 0;
			}
			Bitmap_SetPixel4b(&bmp, x, y, (byte)light, (byte)light, (byte)(trace.flags & (SURF_TERRAIN ? 255 : 0)), 255);
		}
	
		msec = System_Milliseconds() - msec;
	}

	Console_Printf("raytrace took %f msec\n", System_Milliseconds() - msec);

	Bitmap_WritePNG("raytrace.png", &bmp);

	Bitmap_Free(&bmp);
#endif
}



/*==========================

  Scene_Clear

  clear all objects from memory

 ==========================*/

void	Scene_Clear ()
{
	Scene_FramePool_Clear();

	scenelist = NULL;
	scenefacelist = NULL;	
	skylist = NULL;
	spritelist = NULL;
	scenelightlist = NULL;
	occluderlist = NULL;	
}


/*==========================

  Scene_Init

 ==========================*/

void	Scene_Init()
{
	Cvar_Register(&gfx_fog);
	Cvar_Register(&gfx_fog_near);
	Cvar_Register(&gfx_fog_far);
	Cvar_Register(&gfx_fogr);
	Cvar_Register(&gfx_fogg);
	Cvar_Register(&gfx_fogb);
	Cvar_Register(&gfx_farclip);
	Cvar_Register(&gfx_nearclip);
	Cvar_Register(&gfx_polycount);
	Cvar_Register(&gfx_sky);	
	Cvar_Register(&gfx_soundSpace);
	Cvar_Register(&gfx_objects);
	Cvar_Register(&gfx_toon);
	Cvar_Register(&gfx_lockfrustum);
	Cvar_Register(&gfx_dbgplanes);
	Cvar_Register(&gfx_selectionScale);
	Cvar_Register(&gfx_nocull);	
	Cvar_Register(&gfx_sceneStats);
	Cvar_Register(&gfx_dynamicLighting);
	Cvar_Register(&gfx_maxDLights);
	Cvar_Register(&gfx_occlusion);
	Cvar_Register(&gfx_maxSceneOccluders);
	Cvar_Register(&gfx_drawOccluders);
	Cvar_Register(&gfx_lod1range);
	Cvar_Register(&gfx_lod2range);
	Cvar_Register(&gfx_lod3range);
	
	Cmd_Register("raytrace", Scene_Raytrace_Cmd);

	Scene_Clear();

	if (!dedicated_server.integer)
	{
		Vid_InitScene();
		Scene_InitFramePool();
	}
}



/*==========================

  Scene_AABBInFrustum

  returns
    false - not in view
    true  - completely in view or intersects view

 ==========================*/

bool	Scene_AABBInFrustum(const vec3_t bmin, const vec3_t bmax, bool ignoreNearFar)
{
	int n;
	int numplanes = ignoreNearFar ? 4 : 6;

//	if (M_PointInBounds(frustum_bmin, bmin, bmax) && M_PointInBounds(frustum_bmax, bmin, bmax))
//		return 2;	//bounds contain frustum

	for (n=0; n<numplanes; n++)
	{
		if (!M_BoxOnPlaneSide(bmin, bmax, &frustum[n]))
			return false;		
	}

	return true;
}



/*==========================

  Scene_PointInFrustum

 ==========================*/

bool	Scene_PointInFrustum(vec3_t p, bool ignoreNearFar)
{
	int n;
	int numplanes = ignoreNearFar ? 4 : 6;

	for (n=0; n<numplanes; n++)
	{
		if (M_DotProduct(p, frustum[n].normal) - frustum[n].dist < 0)
			return false;
	}

	return true;
}




/*==========================

  Scene_BoxInSelectionRect

  test left, right, top, and bottom planes only

 ==========================*/

bool	Scene_BoxInSelectionRect(const vec3_t bmin, const vec3_t bmax)
{
	int n;

	for (n=0; n<4; n++)
	{
		if (!M_BoxOnPlaneSide(bmin, bmax, &frustum[n]))
			return false;		
	}

	return true;
}


/*==========================

  Scene_OBBInFrustum

 ==========================*/

bool	Scene_OBBInFrustum(const vec3_t bmin, const vec3_t bmax, const vec3_t pos, const vec3_t axis[3], bool ignoreNearFar)
{
	int n;
	int numplanes = ignoreNearFar ? 4 : 6;
//	numplanes=4;

	for (n=0; n<numplanes; n++)
	{
		if (!M_OBBOnPlaneSide(bmin, bmax, pos, axis, &frustum[n]))
			return false;
	}

	return true;
}


/*==========================

  Scene_OBBIsVisible

  determines if an obb is inside the camera view and not occluded

 ==========================*/

bool	Scene_OBBIsVisible(const vec3_t bmin, const vec3_t bmax, const vec3_t pos, const vec3_t axis[3])
{
	int n,i;

	if (!Scene_OBBInFrustum(bmin, bmax, pos, axis, false))
	{
		sceneStats.numOutsideFrustum++;
		return false;
	}

	//test occlusion
	for (n=0; n<scene_num_occluders; n++)
	{
		occluderVolume_t *occ = &scene_occluders[n];
		for (i=0; i<occ->numplanes; i++)
		{
			int planeside = M_OBBOnPlaneSide(bmin, bmax, pos, axis, &occ->planes[i]);
			if (planeside != 1)		//if the OBB isn't completely inside the plane
				break;
		}
		if (i==occ->numplanes)
		{
			//OBB is occluded
			sceneStats.numOccluded++;
			return false;
		}
	}

	return true;
}


/*==========================

  Scene_AABBIsVisible

  determines if an AABB is inside the camera view and not occluded

 ==========================*/

bool	Scene_AABBIsVisible(const vec3_t bmin, const vec3_t bmax)
{
	int n,i;

	if (!Scene_AABBInFrustum(bmin, bmax, false))
	{
		sceneStats.numOutsideFrustum++;
		return false;
	}

	//test occlusion
	for (n=0; n<scene_num_occluders; n++)
	{
		occluderVolume_t *occ = &scene_occluders[n];
		for (i=0; i<occ->numplanes; i++)
		{
			int planeside = M_BoxOnPlaneSide(bmin, bmax, &occ->planes[i]);
			if (planeside != 1)		//if the AABB isn't completely inside the plane
				break;
		}
		if (i==occ->numplanes)
		{
			//AABB is occluded
			sceneStats.numOccluded++;
			return false;
		}
	}

	return true;
}




/*==========================

  Scene_CullLights

 ==========================*/

void	Scene_CullLights()
{
	scenelightlist_t *list;
	int numlights = 0;
	int n;

	OVERHEAD_INIT;

	if (!gfx_dynamicLighting.integer)
		return;		//all lights are culled by default

	for (list = scenelightlist; list; list=list->next)
	{
		scenelight_t *light = &(list->light);		

	//	if (Scene_SphereInFrustum(light->pos, light->intensity, false))
		{
			numlights++;
			list->cull = false;
		}
	}

	if (numlights > gfx_maxDLights.integer)
	{
		//cull out some lights, quick and dirty
		for (n=0; n<numlights; n++)
		{
			list->cull = true;
		}
	}
	

	OVERHEAD_COUNT(OVERHEAD_SCENE_CULL);
}


/*==========================

  Scene_CullObjects

 ==========================*/

void	Scene_CullObjects(bool selectionMode)
{
	scenelist_t		*list;
	model_t			*model;
	int				inview = true;
	sceneobj_t		*obj;
	vec3_t			bmin, bmax;	

	OVERHEAD_INIT;

	for (list = scenelist; list; list=list->next)
	{		
		obj = &(list->obj);
		
		if (selectionMode)
		{
			if (!(obj->flags & SCENEOBJ_SELECTABLE))
			{
				list->cull = true;
				continue;
			}
		}
		else
		{
			if (gfx_nocull.integer)
			{
				list->cull = false;
				continue;
			}
			if (obj->objtype == OBJTYPE_BILLBOARD)
			{
				list->cull = false;
				continue;
			}

			if (obj->objtype == OBJTYPE_BEAM)
			{
				list->cull = false;
				continue;
			}

			if (obj->flags & SCENEOBJ_NEVER_CULL)
			{
				list->cull = false;
				continue;
			}
		}
/*
		if (obj->pos[0] > world.width-1 || obj->pos[1] > world.height-1 ||
			obj->pos[0] < 0 || obj->pos[1] < 0)
		{
			list->cull = true;
			continue;
		}		
*/
		model = Res_GetModel(obj->model);
		if (!model) 
		{
			list->cull = true;
			continue;
		}
//#ifdef TIGHT_CLIP
	/*	
		if (selectionMode)
		{
			M_CopyVec3(model->bmin, bmin);
			M_CopyVec3(model->bmax, bmax);

			//if (obj->scale != 1.0)
			//{
				M_MultVec3(bmin, obj->scale * MODEL_SCALE_FACTOR, bmin);
				M_MultVec3(bmax, obj->scale * MODEL_SCALE_FACTOR, bmax);
			//}
	
			inview = Scene_OBBInFrustum(bmin, bmax, obj->pos, (const vec3_t *)obj->axis, selectionMode);			
			//Res_GetModelVisualBounds(obj->model, bmin, bmax);
		}
		else
		{	*/		
			M_CopyVec3(model->bmin, bmin);
			M_CopyVec3(model->bmax, bmax);
		
			if (obj->scale != 1.0)
			{
				M_MultVec3(bmin, obj->scale, bmin);
				M_MultVec3(bmax, obj->scale, bmax);
			}			

			//M_AddVec3(obj->pos, bmin, bmin);
			//M_AddVec3(obj->pos, bmax, bmax);

			if (selectionMode)
				inview = Scene_OBBInFrustum(bmin, bmax, obj->pos, (const vec3_t *)obj->axis, true);
			else
				inview = Scene_OBBIsVisible(bmin, bmax, obj->pos, (const vec3_t *)obj->axis);
	
			//inview = Scene_AABBInFrustum(bmin, bmax, selectionMode);
	//	}

//#endif

		if (inview) 
		{
			list->cull = false;
			if (!selectionMode)
			{
				sceneStats.renderedSceneObjCount++;
			}
		}
		else
		{
			list->cull = true;
			if (!selectionMode)
			{
				sceneStats.culledSceneObjCount++;
			}
		}
	}

	OVERHEAD_COUNT(OVERHEAD_SCENE_CULL);
}


/*==========================

  Scene_ObjectInFrustum

  a way to short circuit adding an object from game code (use in conjunction with Scene_SetFrustum)

  ==========================*/

bool	Scene_ObjectInFrustum(sceneobj_t *obj)
{
	vec3_t bmin,bmax;
	model_t *model = Res_GetModel(obj->model);

	M_CopyVec3(model->bmin, bmin);
	M_CopyVec3(model->bmax, bmax);

	if (obj->scale != 1.0)
	{
		M_MultVec3(bmin, obj->scale, bmin);
		M_MultVec3(bmax, obj->scale, bmax);
	}			

	return Scene_OBBInFrustum(bmin, bmax, obj->pos, (const vec3_t *)obj->axis, false);
}


/*==========================

  Scene_CullFaces

 ==========================*/

void	Scene_CullFaces()
{
	scenefacelist_t		*list;
	int					n;

	OVERHEAD_INIT;

	for (list = scenefacelist; list; list=list->next)
	{
		list->cull = true;

		for (n=0; n<list->nverts; n++)
		{
			if (M_PointInBounds(list->verts[n].vtx, frustum_bmin, frustum_bmax))
			{
				list->cull = false;
				break;
			}
		}
	}

	OVERHEAD_COUNT(OVERHEAD_SCENE_CULL);
}



/*==========================

  Scene_ClipBounds

  returns:
    0 - not in view
    1 - completely in view
    2 - intersects view

 ==========================*/

int		Scene_ClipBounds(const vec3_t bmin, const vec3_t bmax)
{/*
	int p;
	int planeside;
	int intersects = 0;

	if (!M_BoxExtentsIntersect(bpos, bext, frustum_bpos, frustum_bext))
		return 0;

	if (M_PointInBounds(frustum_bmin, bmin, bmax) && M_PointInBounds(frustum_bmax, bmin, bmax))
		return 1;	//bounds contain frustum

	for (p=0; p<5; p++)
	{
		planeside = M_BoxOnPlaneSide(bmin, bmax, &frustum[p]);

		if (!planeside)
			return 0;
		
		if (planeside==1)
			intersects++;		
	}

	if (intersects==5)
		return 2;
		*/

	int n, planeside;
	bool intersects;

//	if (!M_BoundsIntersect(bmin, bmax, frustum_bmin, frustum_bmax))
//		return 0;

//	if (M_PointInBounds(frustum_bmin, bmin, bmax) && M_PointInBounds(frustum_bmax, bmin, bmax))
//		return 2;	//bounds contain frustum

	if (gfx_nocull.integer)
	{
		return 1;
	}

	intersects = false;

	for (n=0; n<gfx_dbgplanes.integer; n++)
	{
		planeside = M_BoxOnPlaneSide(bmin, bmax, &frustum[n]);

		if (!planeside) return 0;
		if (planeside == 2) intersects = true;
	}
	
	if (intersects) return 2;

	return 1;
}

#if 0

bool Scene_AddEdge(int axis, int count, int side1, side2)
{
	int n;
	vec2_t a, b;

	if (OPP_SIGNS(frustum[side1].normal[axis], frustum[side2].normal[axis]))
	{
		for (n=0; n<3; n++)
		{
			if (n==axis) continue;

			a[n] = 
		}

		return true;
	}
	
	return false;
}

void	Scene_FindSilhouetteEdges(int axis)
{
	//from graphics gems p77:
	/* "Finding a convex polyhedron's silhouette edges is particularly straightforward 
	   in an orthographic view.  In a view down an axis, call it the u-axis, if the u 
	   components off the outward-pointing normals of two adjacent faces have opposite 
	   signs, their common edge is a silhouette edge."
	*/

	//adjacent faces (12 edges):
	//  top (0), left (1)
	//  top (0), right (3)
	//  top (0), near (5)
	//  top (0), far (4)
	//  left (1), near (5)
	//  left (1), bottom (2)
	//  left (1), far (4)
	//  right (3), near (5)
	//  right (3), far (4)
	//  right (3), bottom (2)
	//  bottom (2), near (5)
	//  bottom (2), far (4)

	int count = 0;

#define OPP_SIGNS(a, b) ( (SGN((x)) != SGN((y))) ? 1 : 0 )
#define ADD_EDGE { frustum_edges[axis][count]

	
	Scene_AddEdge(axis, count, FR_TOP, FR_LEFT);
	
		Scene_AddEdge(axis, FR_TOP, FR_LEFT);

#undef OPP_SIGNS
}
#endif





/*==========================

  Scene_SetFrustum
  
  see www.cs.unc.edu/~hoff/research

 ==========================*/

void Scene_SetFrustum(camera_t *camera)
{
	vec3_t eye, viewdir, viewup, viewright;
	float xfov, yfov;
	float wr, wl, wt, wb;
	float tanx, tany;
	vec3_t spts[4];
	vec3_t offset;
	//vec3_t farpoint, nearpoint;
	//vec3_t bmin, bmax;
	//vec3_t raydir, bpoint;
	//vec3_t inv_viewdir;
	//vec3_t toward s;
	vec3_t p1,p2;
	int n;
	vec3_t invViewDir;

	if (gfx_lockfrustum.integer)
		return;

	M_CopyVec3(camera->viewaxis[RIGHT], viewright);
	M_CopyVec3(camera->viewaxis[FORWARD], viewdir);
	M_CopyVec3(camera->viewaxis[UP], viewup);
	M_CopyVec3(camera->origin, eye);
	M_FlipVec3(viewdir, invViewDir);

	yfov = DEG2RAD(camera->fovy);
	xfov = DEG2RAD(camera->fovx);
		
	tanx=tan(xfov*0.5);

	tany=tan(yfov*0.5);

//	wl = wr * ((((selectionRect_x1) / (float)camera->width) - 0.5) * 2);	
//	wb = wt * ((((camera->height - selectionRect_y2) / (float)camera->height) - 0.5) * 2);	
//	wt *= ((((camera->height - selectionRect_y1) / (float)camera->height) - 0.5) * 2);
//	wr *= (((selectionRect_x2) / (float)camera->width) - 0.5) * 2;	

	wl = tanx * (((selectionRect_x1 / (float)camera->width) - 0.5) * 2);
	wr = tanx * (((selectionRect_x2 / (float)camera->width) - 0.5) * 2);
	wb = tany * (((selectionRect_y1 / (float)camera->height) - 0.5) * 2);
	wt = tany * (((selectionRect_y2 / (float)camera->height) - 0.5) * 2);
  
	M_AddVec3(camera->origin, viewdir, offset);
	for (n=0; n<3; n++)
	{
		spts[0][n] = viewright[n]*wr + viewup[n]*wt + offset[n];	//top-right point
		spts[1][n] = viewright[n]*wl + viewup[n]*wt + offset[n];	//top-left point
		spts[2][n] = viewright[n]*wl + viewup[n]*wb + offset[n];	//bottom-left point
		spts[3][n] = viewright[n]*wr + viewup[n]*wb + offset[n];	//bottom-right point
	}

	M_CalcPlane(spts[1], spts[0], eye, &frustum[0]);   //top plane
	M_CalcPlane(spts[2], spts[1], eye, &frustum[1]);   //left plane
	M_CalcPlane(spts[3], spts[2], eye, &frustum[2]);   //bottom plane
	M_CalcPlane(spts[0], spts[3], eye, &frustum[3]);   //right plane

	//far plane
	M_PointOnLine(camera->origin, viewdir, gfx_farclip.value, p1);	
	M_CopyVec3(invViewDir, frustum[4].normal);
	frustum[4].dist = M_DotProduct(invViewDir, p1);	

	//near plane
	M_PointOnLine(camera->origin, viewdir, gfx_nearclip.value, p1);
	M_CopyVec3(viewdir, frustum[5].normal);
	frustum[5].dist = M_DotProduct(viewdir, p1);
	

	//compute frustum bounding box (for Scene_ClipBounds)
	//this is not a very efficient way to do this

	M_ClearBounds(frustum_bmin, frustum_bmax);

	M_AddPointToBounds(spts[0], frustum_bmin, frustum_bmax);
	M_AddPointToBounds(spts[1], frustum_bmin, frustum_bmax);
	M_AddPointToBounds(spts[2], frustum_bmin, frustum_bmax);
	M_AddPointToBounds(spts[3], frustum_bmin, frustum_bmax);


	M_SubVec3(spts[0], camera->origin, p1);
	M_Normalize(p1);
	M_PointOnLine(camera->origin, p1, gfx_farclip.value, p2);
	M_AddPointToBounds(p2, frustum_bmin, frustum_bmax);

	M_SubVec3(spts[1], camera->origin, p1);
	M_Normalize(p1);
	M_PointOnLine(camera->origin, p1, gfx_farclip.value, p2);
	M_AddPointToBounds(p2, frustum_bmin, frustum_bmax);

	M_SubVec3(spts[2], camera->origin, p1);
	M_Normalize(p1);
	M_PointOnLine(camera->origin, p1, gfx_farclip.value, p2);
	M_AddPointToBounds(p2, frustum_bmin, frustum_bmax);

	M_SubVec3(spts[3], camera->origin, p1);
	M_Normalize(p1);
	M_PointOnLine(camera->origin, p1, gfx_farclip.value, p2);
	M_AddPointToBounds(p2, frustum_bmin, frustum_bmax);


/*
	M_ClearBounds(bmin, bmax);
	
	Cam_ConstructRay(camera, 0, 0, raydir);
	M_MultVec3(raydir, gfx_farclip.value, raydir);
	M_AddVec3(eye, raydir, bpoint);
	M_AddPointToBounds(bpoint, bmin, bmax);

	Cam_ConstructRay(camera, camera->width, 0, raydir);
	M_MultVec3(raydir, gfx_farclip.value, raydir);
	M_AddVec3(eye, raydir, bpoint);
	M_AddPointToBounds(bpoint, bmin, bmax);

	Cam_ConstructRay(camera, 0, camera->height, raydir);
	M_MultVec3(raydir, gfx_farclip.value, raydir);
	M_AddVec3(eye, raydir, bpoint);
	M_AddPointToBounds(bpoint, bmin, bmax);

	Cam_ConstructRay(camera, camera->width, camera->height, raydir);
	M_MultVec3(raydir, gfx_farclip.value, raydir);
	M_AddVec3(eye, raydir, bpoint);
	M_AddPointToBounds(bpoint, bmin, bmax);

	M_AddPointToBounds(nearpoint, bmin, bmax);

	M_CopyVec3(bmin, frustum_bmin);
	M_CopyVec3(bmax, frustum_bmax);
	*/
}



/*==========================

  Scene_LerpFaceVert

  ==========================*/

void	Scene_LerpFaceVert(float amt, scenefacevert_t *v1, scenefacevert_t *v2, scenefacevert_t *out)
{	
	out->vtx[0] = LERP(amt, v1->vtx[0], v2->vtx[0]);
	out->vtx[1] = LERP(amt, v1->vtx[1], v2->vtx[1]);
	out->vtx[2] = LERP(amt, v1->vtx[2], v2->vtx[2]);	
	out->tex[0] = LERP(amt, v1->tex[0], v2->tex[0]);
	out->tex[1] = LERP(amt, v1->tex[1], v2->tex[1]);
	out->col[0] = LERP(amt, (float)v1->col[0], (float)v2->col[0]);
	out->col[1] = LERP(amt, (float)v1->col[1], (float)v2->col[1]);
	out->col[2] = LERP(amt, (float)v1->col[2], (float)v2->col[2]);
	out->col[3] = LERP(amt, (float)v1->col[3], (float)v2->col[3]);
}

bool	vtxState(plane_t *pl, vec3_t vtx, float *dotout)
{
	*dotout = M_DotProduct(pl->normal, vtx) - pl->dist;
	
	return (*dotout <= 0 ? true : false);
}


/*==========================

  Scene_ClipPolyToPlane

 ==========================*/

int		Scene_ClipPolyToPlane(scenefacevert_t *verts, int numVerts, plane_t *clipPlane, scenefacevert_t *outpoly, int maxVerts)
{
	int v;
	int vcount = 0;

	for (v=0; v<numVerts; v++)
	{
		scenefacevert_t nextvert;
		float dot, nextdot;
		bool state;
		
		state = vtxState(clipPlane, verts[v].vtx, &dot);

		if (state)
		{			
			if (vcount >= maxVerts)
			{
				return 0;
			}

			Mem_Copy(&outpoly[vcount++], &verts[v], sizeof(scenefacevert_t));				
		}

		Mem_Copy(&nextvert, &verts[(v+1) % numVerts], sizeof(scenefacevert_t));

		if (state != vtxState(clipPlane, nextvert.vtx, &nextdot))
		{
			if (vcount >= maxVerts)
			{
				return 0;
			}

			Scene_LerpFaceVert(dot / (dot-nextdot), &verts[v], &verts[(v+1) % numVerts], &outpoly[vcount++]);
		}
	}
	
	return vcount;
}


/*==========================

  Scene_ClipPoly

  standard sutherland-hodgeman clipping, used for marks on terrain etc
  fixme: optimize me!

 ==========================*/

int		Scene_ClipPoly(scenefacevert_t *verts, int numVerts, plane_t *clipPlanes, int numClipPlanes, scenefacevert_t *outpoly, int maxVerts)
{
	int plane;
	int n = 0;
	scenefacevert_t workingVerts[32];
	int numWorkingVerts = numVerts;

	OVERHEAD_INIT;

	Mem_Copy(workingVerts, verts, sizeof(scenefacevert_t) * numVerts);


	for (plane = 0; plane<numClipPlanes; plane++)
	{
		n = Scene_ClipPolyToPlane(workingVerts, numWorkingVerts, &clipPlanes[plane], outpoly, maxVerts);
		
		numWorkingVerts = n;
		Mem_Copy(workingVerts, outpoly, sizeof(scenefacevert_t) * n);
	}

	Mem_Copy(outpoly, workingVerts, sizeof(scenefacevert_t) * n);

	return n;
}


/*==========================

  Scene_StatsFrame

 ==========================*/

void	Scene_StatsFrame()
{
	static int mtris_count = 0;
	static float mtris_sec = 0;
	static float mtris_lasttime = 0;

	if (!gfx_sceneStats.integer)
	{
		return;
	}

	mtris_count += sceneStats.polycount;
	if (System_Milliseconds() - mtris_lasttime > 1000)
	{
		mtris_sec = mtris_count / 1000000.0;
		mtris_count = 0;
		mtris_lasttime = System_Milliseconds();
	}	

	Host_PrintOverhead("============================\n");
	Host_PrintOverhead("Scene stats\n\n");

	Host_PrintOverhead("CULL\n");
	Host_PrintOverhead("Objects drawn:        %i (%i added, %i culled)\n", sceneStats.renderedSceneObjCount, sceneStats.sceneObjCount, sceneStats.culledSceneObjCount);
	Host_PrintOverhead("Objects outside FOV:  %i\n", sceneStats.numOutsideFrustum);
	Host_PrintOverhead("Objects occluded:     %i\n", sceneStats.numOccluded);
	Host_PrintOverhead("Occluders used:		  %i\n\n", scene_num_occluders);

	Host_PrintOverhead("RENDER\n");
	Host_PrintOverhead("Render scene calls:   %i\n", sceneStats.numRenderScene);	
	Host_PrintOverhead("Total tris drawn:     %i\n", sceneStats.polycount);
	Host_PrintOverhead("Translucent tris:     %i\n", sceneStats.translucentPolycount);
	Host_PrintOverhead("MTris / sec:          %.2f\n", mtris_sec);
	Host_PrintOverhead("DrawTriangles calls:  %i\n", sceneStats.drawTrianglesCount);
	Host_PrintOverhead("Texture switches:     %i\n", sceneStats.numTextureSwitches);
	Host_PrintOverhead("SelectShader calls:   %i\n\n", sceneStats.numSelectShaders);	
	
	
	Host_PrintOverhead("SKEL\n");
	Host_PrintOverhead("SetBoneAnim calls:    %i\n", sceneStats.numSetBoneAnims);
	Host_PrintOverhead("PoseBones calls:      %i\n\n", sceneStats.numPoseBones);

	//Host_PrintOverhead("MEM\n");
	//Host_PrintOverhead("Bytes copied:         %i\n", Mem_GetBytesCopied());

	memset(&sceneStats, 0, sizeof(sceneStats));
	Mem_ResetCopyCount();
}



/*==========================

  Scene_BuildVolume

  create a polyhedron (uncapped) represented as planes from a point and a polygon

  assumes numpoints is >= 3

  planes[] must have capacity for numpoints+1 entries

  the poly array must be coplanar, convex, and specified in counterclockwise order
  
 ==========================*/

void Scene_BuildVolume(const vec3_t point, const vec3_t poly[], int numpoints, plane_t planes[])
{
	int n;
	vec3_t forward;		
	
	M_CalcPlane(poly[0], poly[1], poly[2], &planes[numpoints]);
	M_SubVec3(poly[0], point, forward);

	//determine corner winding
	if (M_DotProduct(planes[numpoints].normal, forward) > 0)
	{		
		for (n=0; n<numpoints; n++)
		{
			M_CalcPlane(point, poly[n], poly[(n+1) % numpoints], &planes[n]);
		}
	}
	else
	{
		M_FlipVec3(planes[numpoints].normal, planes[numpoints].normal);
		planes[numpoints].dist = -planes[numpoints].dist;
		for (n=0; n<numpoints; n++)
		{
			M_CalcPlane(poly[(n+1) % numpoints], poly[n], point, &planes[n]);
		}
	}
}


int	Scene_OccluderSortFunc(const occluderVolume_t *occ1, const occluderVolume_t *occ2)
{
	if (occ1->influence > occ2->influence)
		return -1;

	return 1;
}

/*==========================

  Scene_BuildOccluderList

 ==========================*/

void	Scene_BuildOccluderList()
{	
	int n;
	occluderlist_t *list;

	OVERHEAD_INIT;

	scene_num_occluders = 0;

	if (scene_cam->flags & CAM_NO_OCCLUDERS || !gfx_occlusion.integer)
		return;


	for (list = occluderlist; list; list=list->next)
	{
		int i,j;
		occluder_t *occ = &list->occluder;

		if (occ->numpoints < 3)
			continue;

		//determine if the occluder polygon is inside the camera frustum
		for (n=0; n<occ->numpoints; n++)
		{
			float *a = occ->points[n%occ->numpoints];
			float *b = occ->points[(n+1)%occ->numpoints];

			for (i=0; i<6; i++)
			{
				float dist = 0;
				float dot_a = M_DotProduct(a, frustum[i].normal) - frustum[i].dist;
				float dot_b = M_DotProduct(b, frustum[i].normal) - frustum[i].dist;

				if (dot_a < 0 && dot_b < 0)
					continue;

				//polygon intersects camera frustum
							
				//compute a distance approximation to decide if we should use this occluder
				for (j=0; j<occ->numpoints; j++)
				{
					dist += M_GetDistanceSq(scene_cam->origin, occ->points[j]);
				}
				dist /= occ->numpoints;
				scene_occluders[scene_num_occluders].influence = 1 / dist;

				Scene_BuildVolume(scene_cam->origin, occ->points, occ->numpoints, scene_occluders[scene_num_occluders].planes);
				scene_occluders[scene_num_occluders].numplanes = occ->numpoints+1;
				scene_occluders[scene_num_occluders].occ = occ;
				scene_num_occluders++;
				if (scene_num_occluders >= OCCLUDER_LIST_SIZE)
					goto sortOccluders;
				break;				
			}

			if (i < 6)
				break;
		}
	}	

sortOccluders:

	qsort(scene_occluders, scene_num_occluders, sizeof(occluderVolume_t), Scene_OccluderSortFunc);
	if (scene_num_occluders > gfx_maxSceneOccluders.integer)
		scene_num_occluders = gfx_maxSceneOccluders.integer;

	if (gfx_drawOccluders.integer)
	{
		int i;

		for (i=0; i<scene_num_occluders; i++)
		{	
			occluder_t *occluder = scene_occluders[i].occ;
			scenefacevert_t poly[64];

			memset(poly, 0, sizeof(poly));

			for (n=0; n<occluder->numpoints; n++)
			{
				M_CopyVec3(occluder->points[n], poly[n].vtx);

				{
					int brightness;
					
					/*
					{
						dist = M_GetDistance(occluder->points[n], le.camera.origin);
						brightness = (1 - (dist / corec.Cvar_GetValue("gfx_farclip"))) * 255;

						if (brightness < 0)
							brightness = 0;
						else if (brightness > 255)
							brightness = 255;
					}
					else
					*/
					{
						brightness = 255;
					}

					SET_VEC4(poly[n].col, 255,255,255, brightness);
				}
			}
			Scene_AddPoly(occluder->numpoints, poly, Host_GetWhiteShader(), POLY_DOUBLESIDED | POLY_WIREFRAME | POLY_NO_DEPTH_TEST);

			/*
			for (n=0; n<occluder->numpoints; n++)
			{
				SET_VEC4(poly[n].col, 255, 255, 255, 255 * le_occluderAlpha.value);		
			}

			corec.Scene_AddPoly(occluder->numpoints, poly, Res_LoadShader("/textures/core/occluder.tga"), POLY_DOUBLESIDED);
			*/
		}
	}

	OVERHEAD_COUNT(OVERHEAD_SCENE_OCCLUDERSETUP);
}


/*==========================

  Scene_Render

 ==========================*/

void	Scene_Render (camera_t *camera, vec3_t userpos)
{	
	vec3_t tmp;
	OVERHEAD_INIT;

	if (!camera)
		return;

	scene_cam = camera;

	if (!camera->width || !camera->height)
	{
		camera->x = 0;
		camera->y = 0;
		camera->width = Vid_GetScreenW();
		camera->height = Vid_GetScreenH();
	}

	//reset the selection rectangle
	selectionRect_x1 = 0;
	selectionRect_y1 = 0;
	selectionRect_x2 = camera->width-1;
	selectionRect_y2 = camera->height-1;

	/*
	if (gfx_objects.integer && !(camera->flags & CAM_NO_WORLDOBJECTS))
	{
		WO_RenderObjects();
	}
	*/
	
	Scene_SetFrustum(camera);
	Scene_BuildOccluderList();

	if (scenelist)
		Scene_CullObjects(false);		
	if (scenelightlist)
		Scene_CullLights();
	//fixme: i'm taking CullFaces out of this function since polys can
	//still be added during Vid_RenderScene, so gl_scene.c will call
	//it for now...i don't like this much
	//Scene_CullFaces();

	M_CopyVec3(camera->origin, tmp);
	//M_DivVec3(tmp, gfx_soundSpace.value, listener_origin); 
	//Sound_SetListenerPosition(listener_origin, camera->viewaxis[1], camera->viewaxis[2]);
	//Sound_SetListenerPosition(camera->origin, camera->viewaxis[2]);
	//Console_Printf("cam: (%f, %f, %f)\n", listener_origin[0], listener_origin[1], listener_origin[2]);

	Vid_RenderScene(camera, userpos);	

	sceneStats.numRenderScene++;

	OVERHEAD_COUNT(OVERHEAD_SCENE_RENDER);
}


/*==========================

  Scene_SelectObjects

  left, right, bottom, and top should be specified in real screen coordinates
  (i.e. from 0 to camera->width-1 or 0 to camera->height)

 ==========================*/

int		Scene_SelectObjects(camera_t *camera, int rx, int ry, int rw, int rh, int *buf, int bufsize)
{
	scenelist_t *list;
	int marker = 0;	
	OVERHEAD_INIT;

	if (!camera)
		return 0;

	if (rw < 0)
	{	
		rw = -rw;		
		rx -= rw;
	}
	if (rh < 0)
	{
		rh = -rh;
		ry -= rh;
	}

//	rw = abs(rw);
//	rh = abs(rh);
	if (!rw) rw=1;
	if (!rh) rh=1;

	//rx = camera->width - rx;
	ry = camera->height - ry;
	selectionRect_x1 = rx;
	selectionRect_y2 = ry;
	selectionRect_x2 = rx + rw;
	selectionRect_y1 = ry - rh;
	
	Scene_SetFrustum(camera);
	Scene_CullObjects(true);
	
	buf[0] = -1;

	for (list=scenelist; list; list=list->next)
	{
		if (!list->cull && list->obj.flags & SCENEOBJ_SELECTABLE)
		{
			buf[marker] = list->obj.selection_id;
			marker++;
			if (marker>=bufsize-1)
				break;
		}
	}

	if (bufsize > 1)
		buf[marker] = -1;  //marks the end of the selection list

	OVERHEAD_COUNT(OVERHEAD_SCENE_SELECTOBJECTS);

	return marker;
}


/*==========================

  Draw_SetShaderTime

 ==========================*/

void		Draw_SetShaderTime(float time)
{
	draw_shadertime = time;
}


/*==========================

  Draw_Quad2d

 ==========================*/

void		Draw_Quad2d(float x, float y, float w, float h, float s1, float t1, float s2, float t2, residx_t shader)
{
	shader_t *shd;

	shd = Res_GetShader(shader);
	Vid_DrawQuad2d(x, y, w, h, s1, t1, s2, t2, shd);
}


/*==========================

  Draw_Poly2d

 ==========================*/

void		Draw_Poly2d(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, residx_t shader)
{
	shader_t *shd;

	shd = Res_GetShader(shader);
	Vid_DrawPoly2d(v1, v2, v3, v4, s1, t1, s2, t2, shd);
}

vec4_t current_color;


/*==========================

  Draw_SetColor

 ==========================*/

void		Draw_SetColor(vec4_t color)
{
	Vid_SetColor(color);
	M_CopyVec4(color, current_color);
	//Vid_SetColor(color);
}


/*==========================

  Draw_GetCurrentAlpha

 ==========================*/

float		Draw_GetCurrentAlpha()
{
	return current_color[3];
}


/*==========================

  Draw_SetRegion

 ==========================*/

void		Draw_SetRegion(int x, int y, int w, int h)
{
	Vid_SetDrawRegion(x, y, w, h);
}

#define	COLOR_SCALE_DONT_CLAMP


/*==========================

  Scene_ClampLightedColor

 ==========================*/

INLINE void		Scene_ClampLightedColor(vec3_t color)
{
#ifdef COLOR_SCALE_DONT_CLAMP
	//divide color components by the largest component

	float temp, scale;
	
	temp = color[0];
	if (color[1] > color[0]) temp = color[1];
	if (color[2] > temp) temp = color[2];

	if (temp > 1)
	{
		scale = 1.0f / temp;

		color[0] *= scale;
		color[1] *= scale;
		color[2] *= scale;
	}

	if (color[0] < ter_ambient_r.value /** vid_dynamicMultiplier.value*/) color[0] = ter_ambient_r.value /** vid_dynamicMultiplier.value*/;
	if (color[1] < ter_ambient_g.value /** vid_dynamicMultiplier.value*/) color[1] = ter_ambient_g.value /** vid_dynamicMultiplier.value*/;
	if (color[2] < ter_ambient_b.value /** vid_dynamicMultiplier.value*/) color[2] = ter_ambient_b.value /** vid_dynamicMultiplier.value*/;
#else
	if (color[0] > 1) color[0] = 1;
	if (color[1] > 1) color[1] = 1;
	if (color[2] > 1) color[2] = 1;
	if (color[0] < ter_ambientr.value) color[0] = ter_ambientr.value;
	if (color[1] < ter_ambientg.value) color[1] = ter_ambientg.value;
	if (color[2] < ter_ambientb.value) color[2] = ter_ambientb.value;
#endif
}


/*==========================

  Scene_LerpFaceVert2d

 ==========================*/

void	Scene_LerpFaceVert2d(float amt, scenefacevert_t *v1, scenefacevert_t *v2, scenefacevert_t *out)
{	
	out->vtx[0] = LERP(amt, v1->vtx[0], v2->vtx[0]);
	out->vtx[1] = LERP(amt, v1->vtx[1], v2->vtx[1]);
	out->tex[0] = LERP(amt, v1->tex[0], v2->tex[0]);
	out->tex[1] = LERP(amt, v1->tex[1], v2->tex[1]);
	out->col[0] = LERP(amt, (float)v1->col[0], (float)v2->col[0]);
	out->col[1] = LERP(amt, (float)v1->col[1], (float)v2->col[1]);
	out->col[2] = LERP(amt, (float)v1->col[2], (float)v2->col[2]);
	out->col[3] = LERP(amt, (float)v1->col[3], (float)v2->col[3]);
}


