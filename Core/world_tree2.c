// (C) 2002 S2 Games

// world_tree2.c

// Collision detection, spacial partitioning

#include "core.h"

#define CONTACT_EPSILON 0.03125

//debugging variables
int	numSurfs = 0;
int sweepBoxTests = 0;
int nodesChecked = 0;
int triIntersections = 0;
int maxSurfsInNode = 0;
int	nodesPassed = 0;
static int wt_tracecount_server;
static int wt_tracecount_client;

cvar_t wt_debug = { "wt_debug", "0" };
cvar_t wt_maxSurfsInNode = { "wt_maxSurfsInNode", "0", CVAR_READONLY };
cvar_t wt_intersectMeshes = { "wt_intersectMeshes", "1", CVAR_CHEAT };
cvar_t wt_showTraceCount = { "wt_showTraceCount", "0" };
cvar_t wt_boxTraceTerrain = { "wt_boxTraceTerrain", "0", CVAR_CHEAT };
cvar_t wt_bevelExpand = { "wt_bevelExpand", "0" };


extern server_t localServer;
extern localClientInfo_t clientLocal;


//fixme: should probably dynamically alloc these
#define	MAX_STATIC_SURFACES	16384

linkedSurface_t	staticSurfaces[MAX_STATIC_SURFACES];
linkedObject_t	clientObjects[MAX_OBJECTS];
linkedObject_t	serverObjects[MAX_OBJECTS];

workingTraceVars_t tv;

#define	MAX_QUADSTACK_SIZE	16384

typedef struct
{
	int			startpos;
	int			endpos;

	quadnode_t *nodes[MAX_QUADSTACK_SIZE];
} quadstack_t;

static quadstack_t	stack;


void	WT_PushNode(quadnode_t *node)
{
	stack.nodes[stack.endpos] = node;

	stack.endpos = (stack.endpos + 1) % MAX_QUADSTACK_SIZE;
	if (stack.endpos == stack.startpos)
	{
		stack.endpos = (stack.endpos - 1) % MAX_QUADSTACK_SIZE;
		Console_DPrintf("WT_PushNode: out of stack space\n");
	}
}

quadnode_t *WT_PopNode()
{
	quadnode_t *ret;

	if (stack.startpos == stack.endpos)
		return NULL;

	ret = stack.nodes[stack.startpos];
	stack.startpos = (stack.startpos + 1) % MAX_QUADSTACK_SIZE;

	return ret;
}

void	WT_DecreaseThingCount(quadnode_t *node)
{
	quadnode_t *p = node;

	while (p)
	{
		p->thingCount--;
		if (p->thingCount < 0)
		{
			p->thingCount = 0;
			Game_Error("thingcount < 0");
		}

		p = p->parent;
	}
}

void	WT_IncreaseThingCount(quadnode_t *node)
{
	quadnode_t *p = node;

	while (p)
	{
		p->thingCount++;

		p = p->parent;
	}
}

void	WT_UpdateNodeHeight(quadnode_t *node, float minheight, float maxheight)
{
	bool recurse = false;

	if (maxheight > node->bmax[2])
	{
		node->bmax[2] = maxheight;
		recurse = true;
	}

	if (minheight < node->bmin[2])
	{
		node->bmin[2] = minheight;
		recurse = true;
	}

	if (!recurse)
		return;
	
	node = node->parent;
	while (node)
	{
		if (maxheight > node->bmax[2])
		{
			node->bmax[2] = maxheight;
		}
		if (minheight < node->bmin[2])
		{
			node->bmin[2] = minheight;
		}

		node = node->parent;
	}	
}

quadnode_t	*WT_GetNodeAtXY(int xpos, int ypos, int level)
{
	int index, y, x;
	FloatToInt(&y, (ypos / world.tree_boxdims[level]) * world.tree_levelwidths[level]);
	FloatToInt(&x, (xpos / world.tree_boxdims[level]));
	index = y + x;
	
	return &world.tree[level][index];
}

void	WT_UpdateTerrainHeight(int gridx, int gridy, float altitude)
{
	quadnode_t *node;
	int wx = GRID_TO_WORLD(gridx);
	int wy = GRID_TO_WORLD(gridy);

	WT_ClearStack();
	WT_PushNode(world.tree[0]);

	while((node = WT_PopNode()))
	{
		if ((wx >= node->bmin[0] && wx <= node->bmax[0]) &&
			(wy >= node->bmin[1] && wy <= node->bmax[1]))
		{

			if (node->children[0])
			{
				WT_PushNode(node->children[0]);
				WT_PushNode(node->children[1]);
				WT_PushNode(node->children[2]);
				WT_PushNode(node->children[3]);
			}
			else
			{
				WT_UpdateNodeHeight(node, altitude, altitude);
			}
		}
	}
}


void	WT_ClearStack()
{
	stack.startpos = stack.endpos = 0;
}
/*
//OPTIMIZATION: preprocess all poly surfaces and turn them into mini bsp trees for intersection.  would get rid of the need to intersect every triangle
bool	WT_IntersectBoxWithPolySurface(linkedSurface_t *polysurf)
{
	int tri;

	for (tri = 0; tri < polysurf->numTris; tri++)
	{
		WT_IntersectBoxWithTri(
	}
}

bool	WT_IntersectBoxWithBox

void	_WT_LinkInSurface(linkedSurface_t *surf)
{

}
*/

//polygonal surfaces (used for static objects)
linkedSurface_t *WT_AllocSurface(int numPlanes)
{
	int n;

	for (n=0; n<MAX_STATIC_SURFACES; n++)
	{
		if (!staticSurfaces[n].active)
		{
			memset(&staticSurfaces[n], 0, sizeof(staticSurfaces[n]));
			staticSurfaces[n].active = true;
			staticSurfaces[n].index = n;
			staticSurfaces[n].planes = Tag_Malloc(numPlanes * sizeof(plane_t), MEM_COLLISION);
			return &staticSurfaces[n];
		}
	}

	return NULL;
}

void	WT_FreeSurface(linkedSurface_t *surf)
{
	if (!surf->active)
		return;

	surf->active = false;
	Tag_Free(surf->planes);
}



enum
{
	CLIENTOBJECTLIST,
	SERVEROBJECTLIST
};

bool	_WT_IsLinked(baseObject_t *obj, linkedObject_t *objectArray)
{
	return objectArray[obj->index].linked;
}

void	_WT_UnlinkObject(object_grid_t *grid, baseObject_t *obj, linkedObject_t *objectArray)
{
	linkedObject_t *linkobj;
	
	if (!obj)
		return;

	linkobj = &objectArray[obj->index];

	if (!linkobj->linked)
		return;
	if (!linkobj->node)
		return;

	WT_DecreaseThingCount(linkobj->node);

	if (grid)
		WOG_Remove(grid, obj->index);
	LIST_REMOVE(linkobj);
	linkobj->node = NULL;
	linkobj->linked = false;
}


void	_WT_LinkObject(baseObject_t *obj, linkedObject_t *objectArray, int serverOrClient)
{
	int n;
	quadnode_t *node;
	linkedObject_t *linkobj;
	
	if (!obj)
		return;
	
	linkobj = &objectArray[obj->index];
	
	//should we be doing this?
	if (obj->flags & BASEOBJ_NO_LINK)
		return;
	
	if (linkobj->linked)
	{
//		Console_DPrintf("WT_LinkObject(%s): warning: object was already linked, unlinking...\n", serverOrClient == CLIENTOBJECTLIST ? "cl" : "sv");
		_WT_UnlinkObject(serverOrClient == CLIENTOBJECTLIST ? NULL : &localServer.grid, 
						 obj, objectArray);
	}

	//work out bounding box
	//snap position to integers for network consistency
	for (n=0; n<3; n++)
	{
		linkobj->bmin_w[n] = obj->bmin[n] + obj->pos[n];
		linkobj->bmax_w[n] = obj->bmax[n] + obj->pos[n];
	}

	node = WT_FindBestFitNode(linkobj->bmin_w, linkobj->bmax_w);
	if (!node)
	{
		if (wt_debug.integer)
		{
			Console_DPrintf("WT_LinkObject(%s): warning: object doesn't fit in world - spans (%f, %f) to (%f %f)\n", 
						serverOrClient == CLIENTOBJECTLIST ? "cl" : "sv", 
						linkobj->bmin_w[0], 
						linkobj->bmin_w[1], 
						linkobj->bmax_w[0], 
						linkobj->bmax_w[1]);
		}
		return;
	}

	if (serverOrClient == CLIENTOBJECTLIST)
	{
		LIST_INSERT(&node->clientObjectList, linkobj);
		//WOG_Add(&localClient.grid, obj->index);
	}
	else
	{
		LIST_INSERT(&node->serverObjectList, linkobj);
		WOG_Add(&localServer.grid, obj->index);
	}


	WT_UpdateNodeHeight(node, linkobj->bmin_w[2]-1, linkobj->bmax_w[2]+1);
	
	linkobj->linked = true;
	linkobj->index = obj->index;
	linkobj->node = node;
	linkobj->surfaceFlags = obj->surfaceFlags;

	WT_IncreaseThingCount(linkobj->node);
}

bool	WT_IsLinked_Server(baseObject_t *obj)
{
	return _WT_IsLinked(obj, serverObjects);
}

void	WT_LinkObject_Client(baseObject_t *obj)
{
	_WT_LinkObject(obj, clientObjects, CLIENTOBJECTLIST);
}

void	WT_LinkObject_Server(baseObject_t *obj)
{
	_WT_LinkObject(obj, serverObjects, SERVEROBJECTLIST);
}

void	WT_UnlinkObject_Client(baseObject_t *obj)
{
	_WT_UnlinkObject(NULL, obj, clientObjects);
 }

void	WT_UnlinkObject_Server(baseObject_t *obj)
{
	_WT_UnlinkObject(&localServer.grid, obj, serverObjects);
}

//based on 2d bounds, find the node that will be the best container
quadnode_t	*WT_FindBestFitNode(vec2_t bmin, vec2_t bmax)
{
	quadnode_t *node;
	quadnode_t *bestfitnode = NULL;
	int deepestLevel = -1;

	WT_ClearStack();
	WT_PushNode(world.tree[0]);

	while ((node = WT_PopNode()))
	{		
		if (!M_RectInRect(bmin, bmax, node->bmin, node->bmax))
			continue;

		if (node->level > deepestLevel)
		{
			bestfitnode = node;
			deepestLevel = node->level;
		}

		if (node->children[0])
		{
			WT_PushNode(node->children[0]);
			WT_PushNode(node->children[1]);
			WT_PushNode(node->children[2]);
			WT_PushNode(node->children[3]);
		}
	}

	return bestfitnode;
}


void	WT_BoxPolyhedron(convexPolyhedron_t *poly, vec3_t bmin_w, vec3_t bmax_w)
{ 
	plane_t boxplanes[6] = 
	{
		{ {-1, 0, 0 }, 0 },
		{ { 1, 0, 0 }, 0 },
		{ { 0,-1, 0 }, 0 },
		{ { 0, 1, 0 }, 0 },
		{ { 0, 0,-1 }, 0 },
		{ { 0, 0, 1 }, 0 }
	};

	boxplanes[0].dist = -bmin_w[0];
	boxplanes[1].dist = bmax_w[0];
	boxplanes[2].dist = -bmin_w[1];
	boxplanes[3].dist = bmax_w[1];
	boxplanes[4].dist = -bmin_w[2];
	boxplanes[5].dist = bmax_w[2];

	poly->numPlanes = 6;
	memcpy(poly->planes, boxplanes, sizeof(boxplanes));
}



void	WT_AddPlaneToPolyhedron(convexPolyhedron_t *poly, plane_t *plane)
{
	int n;

	if (poly->numPlanes >= MAX_POLYHEDRON_PLANES+6)
		return;

	for (n=0; n<poly->numPlanes; n++)
	{
		//don't add duplicate planes (or planes that are very similar)
		if (M_CompareVec3(plane->normal, poly->planes[n].normal))
			return;
	}

	poly->planes[poly->numPlanes++] = *plane;
}


void	WT_BevelPolyhedron(convexPolyhedron_t *poly, convexPolyhedron_t *out)
{
	//we'll simply expand the polyhedron out a bit by BEVEL_EXPAND units and bevel it using the original bounding box
	vec3_t expand_max = { 1,1,1 };
	vec3_t expand_min = { -1,-1,-1 };
	int n;
	convexPolyhedron_t box;

	//expand the polyhedron by pushing out the planes by BEVEL_EXPAND units
	for (n=0; n<poly->numPlanes; n++)
	{
		vec3_t point;
		plane_t *plane = &poly->planes[n];

		M_MultVec3(plane->normal, plane->dist, point);
		M_PointOnLine(point, plane->normal, wt_bevelExpand.value, point);

		out->planes[n] = poly->planes[n];
		out->planes[n].dist = M_DotProduct(plane->normal, point);
	}

	out->numPlanes = poly->numPlanes;

	//bevel it using the planes of the original bounding box
	WT_BoxPolyhedron(&box, poly->bmin, poly->bmax);

	for (n=0; n<6; n++)
	{
		WT_AddPlaneToPolyhedron(out, &box.planes[n]);
	}

	//expand the bounds slightly to account for floating point error
	M_AddVec3(poly->bmax, expand_max, out->bmax);
	M_AddVec3(poly->bmin, expand_min, out->bmin);

	out->flags = poly->flags;
}

linkedSurface_t	*_WT_LinkPolyhedron(convexPolyhedron_t *poly, int index, residx_t model, float scale, const vec3_t position, const vec3_t axis[3])
{
	linkedSurface_t *surf;
	int level;
	quadnode_t *node;
	convexPolyhedron_t bevel;
	convexPolyhedron_t tempPoly;

//	if (poly->numPlanes < 4)
	//	return;

	//scale, transform and bevel
	WT_ScalePolyhedron(poly, scale, &tempPoly);
	WT_TransformPolyhedron(&tempPoly, position, axis, &tempPoly);
	if (!(poly->flags & SURF_MODELBOUNDS))
		WT_BevelPolyhedron(&tempPoly, &bevel);
	else
		bevel = tempPoly;

	surf = WT_AllocSurface(bevel.numPlanes);
	if (!surf)
	{
		//return NULL;
		Game_Error("Ran out of static surfaces\n");
	}
	
	M_CopyVec3(bevel.bmin, surf->bmin_w);
	M_CopyVec3(bevel.bmax, surf->bmax_w);

	surf->numPlanes = bevel.numPlanes;
	surf->flags = poly->flags;	
	surf->index = index;
	surf->model = model;

	//store transformation info to get from world space to model space during mesh intersection:
	M_CopyVec3(position, surf->worldPos);
	memcpy(surf->worldAxis, axis, sizeof(vec3_t) * 3);
	surf->invScale = 1 / scale;
	
	memcpy(surf->planes, bevel.planes, surf->numPlanes * sizeof(plane_t));
	
	//now link it into a node.  if it can't be linked, we'll free it and return
	node = WT_FindBestFitNode(surf->bmin_w, surf->bmax_w);
	if (!node)
	{
		Console_DPrintf("WT_LinkPolyhedron: surface doesn't fit in world\n");
		WT_FreeSurface(surf);
		return NULL;
	}

	//add the surface to the list of surfaces in this node
	LIST_INSERT(&node->surfaceList, surf);
	surf->node = node;
	//debug max surface
	{
		int numsurf = 0;
		linkedSurface_t *ls = node->surfaceList.next;
		while (ls != &node->surfaceList)
		{
			numsurf++;
			if (numsurf > wt_maxSurfsInNode.integer)
				Cvar_SetVarValue(&wt_maxSurfsInNode, numsurf);
			ls = ls->next;
		}
	}
	//expand the node's Z bounds if necessary
	WT_UpdateNodeHeight(node, surf->bmin_w[2], surf->bmax_w[2]);

	level = 0;
	while (node->parent)
	{
		level++;
		node=node->parent;
	}

	surf->flags |= SURF_COMPLEX;
	if (index < MAX_OBJECTS && index >= 0)
	{
		//WOG_Add(&localClient.grid, index);
		if (localServer.active)
			WOG_Add(&localServer.grid, index);
		surf->flags |= SURF_GAMEOBJECT;// & ~SURF_STATIC;
		surf->flags &= ~SURF_STATIC;
	}
	else
	{
		surf->flags |= SURF_STATIC;// & ~SURF_DYNAMIC;
		surf->flags &= ~SURF_GAMEOBJECT;
	}
	
	if (wt_debug.integer)
		Console_DPrintf("surface linked at level %i\n", level);

	WT_IncreaseThingCount(surf->node);



	return surf;
}

linkedSurface_t *WT_LinkPolyhedron(convexPolyhedron_t *poly, int index, residx_t model, float scale, const vec3_t position, const vec3_t axis[])
{
	linkedSurface_t *ret;

	OVERHEAD_INIT;
	ret = _WT_LinkPolyhedron(poly, index, model, scale, position, axis);
	OVERHEAD_COUNT(OVERHEAD_LINK_STATIC);

	return ret;
}


//unlink and free
void	WT_UnlinkPolyhedron(linkedSurface_t *surf)
{
	OVERHEAD_INIT;
	if (!surf)
		return;
	if (!surf->node)
		return;

	WT_DecreaseThingCount(surf->node);

	LIST_REMOVE(surf);
	WT_FreeSurface(surf);


	OVERHEAD_COUNT(OVERHEAD_LINK_STATIC);
}


void	WT_TransformPolyhedron(convexPolyhedron_t *in, const vec3_t pos, const vec3_t axis[3], convexPolyhedron_t *out)
{
	int n;

	M_TransformBounds(in->bmin, in->bmax, pos, axis, out->bmin, out->bmax);
	
	for (n=0; n<in->numPlanes; n++)
	{
		M_TransformPlane(&in->planes[n], pos, axis, &out->planes[n]);
	}

	out->flags = in->flags;
	out->numPlanes = in->numPlanes;	
}

void	WT_ScalePolyhedron(convexPolyhedron_t *in, float scale, convexPolyhedron_t *out)
{
	int n;

	for (n=0; n<in->numPlanes; n++)
	{
		M_ScalePlane(&in->planes[n], scale, &out->planes[n]);
	}
	
	out->flags = in->flags;
	out->numPlanes = in->numPlanes;

	M_MultVec3(in->bmin, scale, out->bmin);
	M_MultVec3(in->bmax, scale, out->bmax);
}

/*
//link a triangle mesh into the world.  mesh must be convex!
void	WT_LinkTriMesh(convexTriMesh_t *mesh)
{
	int n, m, i;
	convexPolyhedron_t poly;
	int planeIndex[MAX_POLYHEDRON_PLANES];
	OVERHEAD_INIT;
	
	if (mesh->numTris < 4)
		return;

	poly.numPlanes = 0;

	//check planes for similarity.  we might be able to discard unnecessary planes
	for (n=0; n<mesh->numTris; n++)
	{
		vec3_t nml_a;
		bool ignorePlane;

		ignorePlane = false;

		M_SurfaceNormal(mesh->tris[n].v[2], mesh->tris[n].v[1], mesh->tris[n].v[0], nml_a);

		for (m=n+1; m<mesh->numTris; m++)
		{
			vec3_t nml_b;

			M_SurfaceNormal(mesh->tris[m].v[2], mesh->tris[m].v[1], mesh->tris[m].v[0], nml_b);

			for (i=0; i<3; i++)
			{
				if (ABS(nml_b[i] - nml_a[i]) > EPSILON)
					break;
			}

			if (i == 3)
			{
				ignorePlane = true;
				break;
			}
		}

		if (!ignorePlane)
		{
			planeIndex[poly.numPlanes] = n;
			poly.numPlanes++;
			if (poly.numPlanes >= MAX_POLYHEDRON_PLANES)
			{
				Console_DPrintf("WT_LinkTriMesh: exceeded MAX_POLYHEDRON_PLANES\n");
				return;
			}
		}
	}

	M_ClearBounds(poly.bmin,poly.bmax);

	//transform triangle geometry and build planes
	for (n=0; n<mesh->numTris; n++)
	{
		triangle_t *tri = &mesh->tris[n];
		//rotate vertices
#if 0
		M_RotateVec3(col->tris[n].v[0], col->axis, v0);
		M_RotateVec3(col->tris[n].v[1], col->axis, v1);
		M_RotateVec3(col->tris[n].v[2], col->axis, v2);
#endif
		//translate vertices
		M_AddVec3(tri->v[0], mesh->pos, tri->v[0]);
		M_AddVec3(tri->v[1], mesh->pos, tri->v[1]);
		M_AddVec3(tri->v[2], mesh->pos, tri->v[2]);

		M_AddPointToBounds(tri->v[0], poly.bmin, poly.bmax);
		M_AddPointToBounds(tri->v[1], poly.bmin, poly.bmax);
		M_AddPointToBounds(tri->v[2], poly.bmin, poly.bmax);
	}

	for (n=0; n<poly.numPlanes; n++)
	{
		vec3_t edge1,edge2;
		triangle_t *tri = &mesh->tris[planeIndex[n]];

		//get the plane for this tri
		M_SubVec3(tri->v[1], tri->v[0], edge1);
		M_SubVec3(tri->v[2], tri->v[0], edge2);
		M_CrossProduct(tri->v[1], tri->v[2], poly.planes[n].normal);
		M_Normalize(poly.planes[n].normal);
		poly.planes[n].dist = M_DotProduct(poly.planes[n].normal, tri->v[0]);
	}

	poly.flags = mesh->flags;

	WT_LinkPolyhedron(&poly);
	OVERHEAD_COUNT(OVERHEAD_PHYSICS);
}
*/




//intersect a swept box with a convex surface
bool	WT_IntersectBoxWithSurface(linkedSurface_t *surf, int *index, int *meshFlags)
{
	int n, i;
	plane_t *plane;
	float distOffset;
	int idx;
	float t0,t1;
	float enter,leave;
	bool startout, getout;
	vec3_t contact;

	if (!surf->numPlanes)
		return false;

	enter = -1;
	leave = 1;
	startout = false;
	getout = false;
	idx = -1;

	numSurfs++;

//	if (!I_MovingBoundsIntersect(tv.start, tv.end, tv.bmin_w, tv.bmax_w, surf->bmin_w, surf->bmax_w, &blah))
//		return false;

	for (n=0; n<surf->numPlanes; n++)
	{
		float t;

		plane = &surf->planes[n];

		//shift the plane based on bmin/bmax
		if (tv.lineTrace)
		{
			distOffset = plane->dist;
		}
		else
		{
			//determine the point of contact on the AABB that we
			//should use for offsetting the plane equation
			for (i=0; i<3; i++)
			{
				if (plane->normal[i] >= 0)
					contact[i] = tv.bmin[i];
				else
					contact[i] = tv.bmax[i];
			}

			distOffset = plane->dist - M_DotProduct(plane->normal, contact);
		}
		
		//find distance from plane at start point
		t0 = M_DotProduct(tv.start, plane->normal) - distOffset;
		//find distance from plane at end point
		t1 = M_DotProduct(tv.end, plane->normal) - distOffset;

		if (t0 > 0)
			startout = true;
		if (t1 > 0)
			getout = true;

		if (t0 > 0 && t1 >= t0)	//no intersection
		{
			return false;
		}

		if (t0 <= 0 && t1 <= 0)	//inside plane
			continue;

		//t0 and t1 have different signs



		//if (!plane->invis)
		//{
			if (t0 > t1)		//t0 is positive, going towards the surface
			{
				t = (t0 - CONTACT_EPSILON) / (t0-t1);		//back off by an epsilon
				if (t > enter)
				{
					enter = t;
					//if (!plane->invis)
					//	idx = -1;
					//else
						idx = n;
				}
			}
			else				//t1 is positive, going away from the surface
			{
				t = (t0 + CONTACT_EPSILON) / (t0-t1);	
				if (t < leave)
					leave = t;
			}
	//	}
	}

	if (!startout)
	{
		tv.startedInSurface = true;
		if (!getout)
		{
			tv.embedded = true;
			tv.fraction = 0;
		}
		*meshFlags |= surf->flags;
		return true;
	}

	if (enter < leave)
	{
		if (enter > -1 && enter < tv.fraction && idx != -1)
		{
			if (enter < 0)
				enter = 0;

/*
			if (enter == tv.fraction && tv.fraction < 1)
			{				
				if (!(M_DotProduct(tv.velocity, surf->planes[idx].normal) < M_DotProduct(tv.velocity, tv.normal)))
					return false;
			}
*/
			tv.fraction = enter;
			*index = idx;
			tv.normal[0] = surf->planes[idx].normal[0];
			tv.normal[1] = surf->planes[idx].normal[1];
			tv.normal[2] = surf->planes[idx].normal[2];
			tv.dist = surf->planes[idx].dist;

			*meshFlags |= surf->flags;
			return true;
		}
	}

	return false;
}

bool	WT_IntersectLineWithModel(linkedSurface_t *surf, int *index, int *meshFlags)
{
	vec3_t mStart;		//start in model space
	vec3_t mEnd;		//end in model space
	int n;
	float frac;
	mesh_t *meshhit;
	model_t *model;
	int facehit;

	//if (!I_MovingBoundsIntersect(tv.start, tv.end, tv.bmin_w, tv.bmax_w, surf->bmin_w, surf->bmax_w, &blah))
	//	return false;
	//if (blah >= tv.fraction)
	//	return false;

	//get the line into model space
	M_TransformPointInverse(tv.start, surf->worldPos, (const vec3_t *)surf->worldAxis, mStart);
	M_TransformPointInverse(tv.end, surf->worldPos, (const vec3_t *)surf->worldAxis, mEnd);	

	M_MultVec3(mStart, surf->invScale, mStart);
	M_MultVec3(mEnd, surf->invScale, mEnd);

	model = Res_GetModel(surf->model);			
	frac = tv.fraction;
	facehit = -1;
	meshhit = NULL;
	
	//test intersection with all meshes
	for (n = 0; n<model->num_meshes; n++)
	{		
		float tmp_frac;
		int tmp_face;
		mesh_t *mesh = &model->meshes[n];

		if (!M_LineBoxIntersect3d(mStart, mEnd, mesh->bmin, mesh->bmax, &tmp_frac))
			continue;
		if (tmp_frac >= frac)
			continue;
		if (mesh->flags & tv.ignoreSurface)
			continue;

		if (I_LineMeshIntersect(mStart, mEnd, mesh, &tmp_frac, &tmp_face))
		//if (!M_LineBoxIntersect3d(mStart, mEnd, mesh->bmin, mesh->bmax, &tmp_frac))
		{
			if (tmp_frac < frac)
			{
				frac = tmp_frac;
				meshhit = mesh;
				facehit = tmp_face;
				if (frac <= 0)
				{
					frac = 0;
					break;
				}
			}
		}

		triIntersections += model->meshes[n].num_faces;
	}

	if (meshhit)
	{
		//back off by an epsilon
		tv.fraction -= tv.epsilonFrac;
		if (tv.fraction < 0)
			tv.fraction = 0;

		if (frac < tv.fraction)
		{
			plane_t contactPlane;
			unsigned int *face;
			float *p1, *p2, *p3;

			tv.fraction = frac;
			*index = surf->index; 

			//work out face plane
			face = meshhit->facelist[facehit];
			p1 = meshhit->verts[face[0]];
			p2 = meshhit->verts[face[1]];
			p3 = meshhit->verts[face[2]];

			M_CalcPlane(p1, p2, p3, &contactPlane);
			//transform it back to world space
			M_ScalePlane(&contactPlane, 1 / surf->invScale, &contactPlane);
			M_TransformPlane(&contactPlane, surf->worldPos, (const vec3_t *)surf->worldAxis, &contactPlane);

			//if we hit a triangle's back side, flip the normal
			if (M_DotProduct(contactPlane.normal, tv.dir) > 0)
			{
				M_FlipVec3(contactPlane.normal, contactPlane.normal);
			}

			M_CopyVec3(contactPlane.normal, tv.normal);
			tv.dist = contactPlane.dist;

			*meshFlags = meshhit->flags;
			return true;
		}
	}

	return false;
}


/*
bool	WT_IntersectBoxWithSurface(linkedSurface_t *surf, int *index)
{
	int n;
	float blah,t;
	float lowest_t = tv.fraction;
	vec3_t zero_vec = {0,0,0};
	bool hit=false;

	//if (!M_BoundsIntersect(tv.tracebounds_min, tv.tracebounds_max, surf->bmin_w, surf->bmax_w))
	//	return false;

	sweepBoxTests++;
	if (!I_MovingBoundsIntersect(tv.start, tv.end, tv.bmin_w, tv.bmax_w, surf->bmin_w, surf->bmax_w, &blah))
		return false;

	for (n=0; n<surf->numTris; n++)
	{
		if (I_BoxTriIntersect(surf->tris[n].v, tv.bpos, tv.bext, tv.velocity, 1, &t, &blah))
		{
			if (M_DotProduct(tv.dir, surf->tris[n].normal) > 0)
				continue;		 //surface normal points away from us.  fixme: double-sided surfaces

			Console_DPrintf("dot: %f\n",M_DotProduct(tv.dir,surf->tris[n].normal));

			if (t < lowest_t && t >= 0)
			{
				lowest_t = t;
				*index = n;
				hit = true;
				tv.fraction = lowest_t;
				tv.normal[0] = surf->tris[n].normal[0];
				tv.normal[1] = surf->tris[n].normal[1];
				tv.normal[2] = surf->tris[n].normal[2];

				//triIntersections++;
			}
			else if (t == lowest_t)
			{	
				//we must have hit an edge				
				//tv.collisionFlags |= COL_EDGE;
				if (M_DotProduct(tv.velocity, surf->tris[n].normal) < M_DotProduct(tv.velocity, tv.normal))
				{
					Console_DPrintf("new edge\n");
					//more 'pressure' on this surface
					*index = n;
					hit = true;
					tv.fraction = lowest_t;
					tv.normal[0] = surf->tris[n].normal[0];
					tv.normal[1] = surf->tris[n].normal[1];
					tv.normal[2] = surf->tris[n].normal[2];
				}
			}
			triIntersections++;
		}
		triTests++;
	}

	if (!hit)
		return false;

	return true;
}
*/

#if 0
bool	WT_IntersectBoxWithObject(linkedObject_t *obj)
{
	linkedSurface_t surf;

	if (!I_MovingBoundsIntersectEx(tv.start, tv.end, tv.bmin_w, tv.bmax_w, obj->bmin_w, obj->bmax_w, &t, normal))
		return false;


	if (t < tv.fraction)
	{
		float	tracelength;
		float	oldfrac;

		tracelength = t * tv.length;		//get the non-normalized time
		tracelength -= CONTACT_EPSILON;				//back off a fraction of a unit so we don't get stuck on a plane
		oldfrac = t;
		t = tracelength / tv.length;		//fraction now shortened to push back by an epsilon distance
		if (t < 0)
			t = 0;
	
		tv.fraction = t;
		tv.normal[0] = normal[0];
		tv.normal[1] = normal[1];
		tv.normal[2] = normal[2];
		return true;
	}

	return false;*/

	WT_TempBoxSurface(&surf, obj->bmin_w, obj->bmax_w);
}
#endif

#define DIAG	0.70710678118654752440084436210485

//x and y in grid coords
void	WT_TempTerrainSurface(linkedSurface_t *surf, int x, int y, int tri)
{
	vec3_t pt;
	int index;
	static plane_t tplanes_tri0[4] = 
	{
		{ {0,0,1}, 0 },
		{ {-1,0,0}, 0 },
		{ {0,-1,0}, 0 },
		{ {DIAG,DIAG,0}, 0 }
	};
	static plane_t tplanes_tri1[4] =
	{
		{ {0,0,1}, 0 },
		{ {1,0,0}, 0 },
		{ {0,1,0}, 0 },
		{ {-DIAG,-DIAG,0}, 0 }
	};

	//shared point we can use for plane equations
	SET_VEC3(pt, GRID_TO_WORLD(x), GRID_TO_WORLD(y+1), WORLD_GRIDREF(x,y+1));
	index = GRIDREF(x,y);

	if (tri==0)
	{
		SET_VEC3(tplanes_tri0[0].normal, world.normal[0][index][0], world.normal[0][index][1], world.normal[0][index][2]);
		tplanes_tri0[0].dist = M_DotProduct(tplanes_tri0[0].normal, pt);
		tplanes_tri0[1].dist = -GRID_TO_WORLD(x);
		tplanes_tri0[2].dist = -GRID_TO_WORLD(y);
		tplanes_tri0[3].dist = M_DotProduct(tplanes_tri0[3].normal, pt);

		surf->planes = tplanes_tri0;
		surf->numPlanes = 4;
	}
	else
	{
		SET_VEC3(tplanes_tri1[0].normal, world.normal[1][index][0], world.normal[1][index][1], world.normal[1][index][2]);
		tplanes_tri1[0].dist = M_DotProduct(tplanes_tri1[0].normal, pt);
		tplanes_tri1[1].dist = GRID_TO_WORLD(x+1);
		tplanes_tri1[2].dist = GRID_TO_WORLD(y+1);
		tplanes_tri1[3].dist = M_DotProduct(tplanes_tri1[3].normal, pt);

		surf->planes = tplanes_tri1;
		surf->numPlanes = 4;
	}
}

void	WT_TempBoxSurface(linkedSurface_t *surf, vec3_t bmin_w, vec3_t bmax_w)
{
	static plane_t boxplanes[6] = 
	{
		{ {-1, 0, 0 }, 0 },
		{ { 1, 0, 0 }, 0 },
		{ { 0,-1, 0 }, 0 },
		{ { 0, 1, 0 }, 0 },
		{ { 0, 0,-1 }, 0 },
		{ { 0, 0, 1 }, 0 }
	};

	boxplanes[0].dist = -bmin_w[0];
	boxplanes[1].dist = bmax_w[0];
	boxplanes[2].dist = -bmin_w[1];
	boxplanes[3].dist = bmax_w[1];
	boxplanes[4].dist = -bmin_w[2];
	boxplanes[5].dist = bmax_w[2];

	surf->numPlanes = 6;
	surf->planes = boxplanes;
}

enum
{
	HIT_TERRAIN = 1,
	HIT_OBJECT,
	HIT_SURFACE
};


cvar_t bl = { "bl", "0" };
bool	_WT_TraceBox(traceinfo_t *result)
{
	int idx = -1;
	quadnode_t *node;
	linkedSurface_t *surf_hit = NULL;
	int meshFlags = 0;
	linkedObject_t *obj_hit = NULL;
	int numsurfs = 0;
	int hittype = 0;
	linkedSurface_t tsurf;
	vec3_t v;

	//don't operate on NAN numbers, or temporary freeze ups can occur
	if (_isnan(tv.start[0]) || _isnan(tv.start[1]) || _isnan(tv.start[2]) ||
		_isnan(tv.end[0]) || _isnan(tv.end[1]) || _isnan(tv.end[2]))
	{
		goto noresult;
	}

	M_SubVec3(tv.end,tv.start,v);
	if (wt_debug.integer)
	{
		if (M_GetVec3Length(v) <= CONTACT_EPSILON)
		{
			Console_DPrintf("trace len: %f\n", M_GetVec3Length(v));
		}
	}

	WT_ClearStack();
	WT_PushNode(world.tree[0]);

	while((node = WT_PopNode()))
	{
		float blah;
		linkedSurface_t *surf;

		nodesChecked++;

		//if the swept box covered by the trace doesn't intersect with a quadtree node, we can safely skip the node
		if (!I_MovingBoundsIntersect(tv.start, tv.end, tv.bmin_w, tv.bmax_w, node->bmin, node->bmax, &blah))
			continue;

		if (blah > tv.fraction)
			continue;
		//if (!M_RayBoxIntersect(tv.start, tv.dir, node->bmin, node->bmax, blah))
		//	continue;

		nodesPassed++;

		numsurfs = 0;

		if (tv.lineTrace && wt_intersectMeshes.integer)
		{
			//step through each model linked to this node (if any) and test the line against them
		
			surf = node->surfaceList.next;

			while (surf != &node->surfaceList)
			{
				if (surf->flags & tv.ignoreSurface || surf->index == tv.ignoreIndex || !(surf->flags & SURF_MODELBOUNDS))
				{
					surf = surf->next;
					continue;
				}

				if (WT_IntersectLineWithModel(surf, &idx, &meshFlags))
				{
					surf_hit = surf;
					hittype = HIT_SURFACE;
				}

				surf = surf->next;

				numsurfs++;
			}
		}
		else
		{

			//step through each polyhedron linked to this node (if any) and test the swept box against them
		
			surf = node->surfaceList.next;

			while (surf != &node->surfaceList)
			{
				if (surf->flags & tv.ignoreSurface || surf->index == tv.ignoreIndex || surf->flags & SURF_MODELBOUNDS)
				{
					surf = surf->next;
					continue;
				}

				if (WT_IntersectBoxWithSurface(surf, &idx, &meshFlags))
				{
					surf_hit = surf;
					hittype = HIT_SURFACE;
				}

				surf = surf->next;

				numsurfs++;
			}
		}

		if (!(tv.ignoreSurface & SURF_DYNAMIC))
		{
			//step through each dynamic baseObject linked to this node
			if (tv.clientOrServer == CLIENTOBJECTLIST)
			{
				linkedObject_t *obj;

				obj = node->clientObjectList.next;

				while (obj != &node->clientObjectList)
				{
					linkedSurface_t box;

					if (obj->index == tv.ignoreIndex || (obj->surfaceFlags & tv.ignoreSurface))
					{
						obj = obj->next;
						continue;
					}

					WT_TempBoxSurface(&box, obj->bmin_w, obj->bmax_w);

					if (WT_IntersectBoxWithSurface(&box, &idx, &meshFlags))		
					{
						obj_hit = obj;
						hittype = HIT_OBJECT;						
					}
					
					obj = obj->next;
				}
			}
			else if (tv.clientOrServer == SERVEROBJECTLIST)
			{
				linkedObject_t *obj;

				obj = node->serverObjectList.next;

				while (obj != &node->serverObjectList)
				{
					linkedSurface_t box;

					if (obj->index == tv.ignoreIndex || (obj->surfaceFlags & tv.ignoreSurface))
					{
						obj = obj->next;
						continue;
					}

					WT_TempBoxSurface(&box, obj->bmin_w, obj->bmax_w);

					if (WT_IntersectBoxWithSurface(&box, &idx, &meshFlags))		
					{
						obj_hit = obj;
						hittype = HIT_OBJECT;						
					}
					
					obj = obj->next;
				}
			}
		}

		if (numsurfs > maxSurfsInNode)
			maxSurfsInNode = numsurfs;

		if (node->children[0])
		{
			int child;

			for (child=0; child<4; child++)
			{
				if (node->children[child]->thingCount || !(tv.ignoreSurface & SURF_TERRAIN))
					WT_PushNode(node->children[child]);
			}
		}
		else
		{
			//trace terrain
			if (!(tv.ignoreSurface & SURF_TERRAIN))
			{
				int gx,gy;
				int gwx;
				int gwy;
				vec3_t old_bmin,old_bmax;

				gwx = WORLD_TO_GRID(node->bmax[0]);
				gwy = WORLD_TO_GRID(node->bmax[1]);
				if (!wt_boxTraceTerrain.integer)
				{				
					//test terrain with ray only (inaccuracy problems crop up with boxes)

					M_CopyVec3(tv.bmin, old_bmin);
					M_CopyVec3(tv.bmax, old_bmax);
					M_SetVec3(tv.bmin,0,0,0);
					M_SetVec3(tv.bmax,0,0,0);				
				}
				for (gy = WORLD_TO_GRID(node->bmin[1]); gy <= gwy; gy++)
				{
					for (gx = WORLD_TO_GRID(node->bmin[0]); gx <= gwx; gx++)
					{
						if (gy >= world.gridheight || gy < 0 ||
							gx >= world.gridwidth || gx < 0)
							continue;

						WT_TempTerrainSurface(&tsurf, gx, gy, 0);

						if (WT_IntersectBoxWithSurface(&tsurf, &idx, &meshFlags))
						{
							hittype = HIT_TERRAIN;
						}

						WT_TempTerrainSurface(&tsurf, gx, gy, 1);

						if (WT_IntersectBoxWithSurface(&tsurf, &idx, &meshFlags))
						{
							hittype = HIT_TERRAIN;
						}
					}
				}
				if (!wt_boxTraceTerrain.integer)
				{				
					M_CopyVec3(old_bmin,tv.bmin);
					M_CopyVec3(old_bmax,tv.bmax);
				}
			}
		}
	}



	if (hittype)
	{
		if (tv.fraction < 0)
		{
			if (wt_debug.integer)
				Console_DPrintf("tv.fraction < 0 at surf %p, tri %i\n", surf_hit, idx);
			tv.fraction = 0;			
		}

		result->fraction = tv.fraction;

		result->normal[0] = tv.normal[0];
		result->normal[1] = tv.normal[1];
		result->normal[2] = tv.normal[2];
		result->dist = tv.dist;

		result->startedInside = tv.startedInSurface;
		result->embedded = tv.embedded;
		
		switch (hittype)
		{
			case HIT_OBJECT:
				result->flags = obj_hit->surfaceFlags;
				result->objectType = GAME_OBJECT;
				result->index = obj_hit->index;
				break;
			case HIT_SURFACE:
				result->flags = surf_hit->flags | meshFlags;
				result->objectType = STATIC_OBJECT;
				result->index = surf_hit->index;
				break;			
			case HIT_TERRAIN:
				result->flags = SURF_TERRAIN;
				result->objectType = STATIC_OBJECT;
				result->index = -1;
				break;
		}
		
		if (tv.fraction > 0)
		{
			result->endpos[0] = (tv.start[0] + (tv.velocity[0] * tv.fraction));
			result->endpos[1] = (tv.start[1] + (tv.velocity[1] * tv.fraction));
			result->endpos[2] = (tv.start[2] + (tv.velocity[2] * tv.fraction));
		}
		else
		{
			M_CopyVec3(tv.start, result->endpos);
		}

		result->gridx = WORLD_TO_GRID(result->endpos[0]);
		result->gridy = WORLD_TO_GRID(result->endpos[1]);

		result->collisionType = tv.collisionFlags;

		return true;
	}

noresult:
	result->fraction = 1;
	result->index = -1;
	result->objectType = 0;
	M_CopyVec3(tv.end, result->endpos);
	result->gridx = WORLD_TO_GRID(tv.end[0]);
	result->gridx = WORLD_TO_GRID(tv.end[1]);
	result->flags = 0;
	result->collisionType = 0;
	result->startedInside = false;
	result->embedded = false;

	return false;
}

void	WT_ResetWorkingTraceVars(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax)
{
	memset(result, 0, sizeof(traceinfo_t));
	//set some working variables so we don't have to keep passing them around
	M_CopyVec3(start, tv.start);
	M_CopyVec3(end, tv.end);
	M_CopyVec3(bmax, tv.bmax);
	M_CopyVec3(bmin, tv.bmin);
	//work out the axis-aligned area covered by the trace (we can use this to reject initial quadtree nodes quickly)
	M_SubVec3(end, start, tv.velocity);
	M_AddVec3(start, bmin, tv.bmin_w);
	M_AddVec3(start, bmax, tv.bmax_w);
	M_AddVec3(end, bmin, tv.bmin_w_end);
	M_AddVec3(end, bmax, tv.bmax_w_end);	
	M_CalcBoxExtents(tv.bmin_w,tv.bmax_w,tv.bpos,tv.bext);
	M_CopyVec3(tv.velocity, tv.dir);
	tv.length = M_Normalize(tv.dir);
	tv.epsilonFrac = 1 - ((tv.length - CONTACT_EPSILON) / tv.length);
	tv.fraction = 1;
	tv.collisionFlags = 0;
	tv.embedded = false;
	tv.startedInSurface = false;

	if (M_CompareVec3(bmin, zero_vec) && M_CompareVec3(bmax, zero_vec))
	{
		tv.lineTrace = true;
	}
	else
	{
		tv.lineTrace = false;
	}

	if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2])
	{
		tv.startEqualsEnd = true;
	}
	else
	{
		tv.startEqualsEnd = false;
	}

	//debugging variables	
	sweepBoxTests = 0;
	nodesChecked = 0;
	triIntersections = 0;
	maxSurfsInNode = 0;
	nodesPassed = 0;
	numSurfs = 0;
}

//sweep an axis-aligned box through the world and return the first intersection
bool	WT_TraceBox_Server(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface)
{
	bool ret;

	OVERHEAD_INIT;

	WT_ResetWorkingTraceVars(result,start,end,bmin,bmax);

	tv.clientOrServer = SERVEROBJECTLIST;
	tv.ignoreIndex = -1;
	tv.ignoreSurface = ignoreSurface;

	ret = _WT_TraceBox(result);

	if (wt_debug.integer)
		Console_DPrintf("numSurfs: %i  sweepBoxTests: %i\nnodesChecked: %i  triInt:%i\nmaxSurfsInNode: %i  nodesPassed: %i\n", numSurfs, sweepBoxTests, nodesChecked, triIntersections, maxSurfsInNode, nodesPassed);

	result->dbg_numsurfs = numSurfs;

	wt_tracecount_server++;

	OVERHEAD_COUNT(OVERHEAD_TRACEBOX_SERVER);

	return ret;
}


//sweep an axis-aligned box through the world and return the first intersection
bool	WT_TraceBox_Client(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface)
{
	bool ret;

	OVERHEAD_INIT;

	WT_ResetWorkingTraceVars(result,start,end,bmin,bmax);

	tv.clientOrServer = CLIENTOBJECTLIST;
	tv.ignoreIndex = -1;
	tv.ignoreSurface = ignoreSurface;

	ret = _WT_TraceBox(result);

	if (wt_debug.integer)
		Console_DPrintf("numSurfs: %i  sweepBoxTests: %i\nnodesChecked: %i  triInt:%i\nmaxSurfsInNode: %i  nodesPassed: %i\n", numSurfs, sweepBoxTests, nodesChecked, triIntersections, maxSurfsInNode, nodesPassed);

	result->dbg_numsurfs = numSurfs;

	wt_tracecount_client++;

	OVERHEAD_COUNT(OVERHEAD_TRACEBOX_CLIENT);
	
	return ret;
}

bool	WT_TraceBoxEx_Server(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface, int ignoreIndex)
{
	bool ret;
	
	OVERHEAD_INIT;

	WT_ResetWorkingTraceVars(result,start,end,bmin,bmax);

	tv.clientOrServer = SERVEROBJECTLIST;
	tv.ignoreIndex = ignoreIndex;
	tv.ignoreSurface = ignoreSurface;

	ret = _WT_TraceBox(result);

	if (wt_debug.integer)
		Console_DPrintf("numSurfs: %i  sweepBoxTests: %i\nnodesChecked: %i  triInt:%i\nmaxSurfsInNode: %i  nodesPassed: %i\n", numSurfs, sweepBoxTests, nodesChecked, triIntersections, maxSurfsInNode, nodesPassed);

	result->dbg_numsurfs = numSurfs;

	wt_tracecount_server++;

	OVERHEAD_COUNT(OVERHEAD_TRACEBOX_SERVER);

	return ret;
}


bool	WT_TraceBoxEx_Client(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface, int ignoreIndex)
{
	bool ret;

	OVERHEAD_INIT;

	WT_ResetWorkingTraceVars(result,start,end,bmin,bmax);

	tv.clientOrServer = CLIENTOBJECTLIST;
	tv.ignoreIndex = ignoreIndex;
	tv.ignoreSurface = ignoreSurface;

	ret = _WT_TraceBox(result);

	if (wt_debug.integer)
		Console_DPrintf("numSurfs: %i  sweepBoxTests: %i\nnodesChecked: %i  triInt:%i\nmaxSurfsInNode: %i  nodesPassed: %i\n", numSurfs, sweepBoxTests, nodesChecked, triIntersections, maxSurfsInNode, nodesPassed);

	result->dbg_numsurfs = numSurfs;

	wt_tracecount_client++;

	OVERHEAD_COUNT(OVERHEAD_TRACEBOX_CLIENT);
	
	return ret;
}

void	WT_ResetTraceCount()
{
	if (wt_showTraceCount.integer)
	{
		Console_DPrintf("client traces: %i   server trace: %i\n", wt_tracecount_client, wt_tracecount_server);
	}
	wt_tracecount_client = 0;
	wt_tracecount_server = 0;
}

void	WT_DestroyTree()
{
	int n;
	int l;

	for (n=0; n<world.tree_levels; n++)
	{
		Tag_Free(world.tree[n]);
	}
	
	for (l=0; l<MAX_TREE_LEVELS; l++)
	{
		world.tree[l] = NULL;		
	}

	//delete surfaces that were previously linked in
	for (n=0; n<MAX_STATIC_SURFACES; n++)
	{
		WT_FreeSurface(&staticSurfaces[n]);
	}

	memset(staticSurfaces, 0, sizeof(staticSurfaces));

	//clear dynamic objects
	memset(clientObjects, 0, sizeof(clientObjects));
	memset(serverObjects, 0, sizeof(serverObjects));
}

//get a child node pointer based on a level and the x/y position of the parent
quadnode_t	*WT_ChildNodeReference(int parent_level, int parent_x, int parent_y, int quadpos)
{
	int node_x = 0, node_y = 0;
	int level = parent_level+1;
	int level_width = world.tree_levelwidths[parent_level+1];

	if (level > world.tree_levels-1)
		return NULL;

	switch(quadpos)
	{
		
		case QUAD_TL:
			node_x = parent_x * 2;
			node_y = parent_y * 2;
			break;
		case QUAD_TR:
			node_x = parent_x * 2 + 1;
			node_y = parent_y * 2;
			break;
		case QUAD_BL:
			node_x = parent_x * 2;
			node_y = parent_y * 2 + 1;
			break;
		case QUAD_BR:
			node_x = parent_x * 2 + 1;
			node_y = parent_y * 2 + 1;
			break;
	}

	return &world.tree[parent_level+1][((node_y * level_width) + node_x)];
}

void	WT_LockTrace_Cmd(int argc, char *argv[])
{
	memcpy(&world.lastTrace, &tv, sizeof(tv));
}

void	WT_ResetDynamic(bool client)
{
	int i,j, l;
	linkedObject_t *objects;

	if (client)
		objects = clientObjects;
	else
		objects = serverObjects;

	for (l=0; l<world.tree_levels; l++)
	{
		j = world.tree_levelwidths[l] * world.tree_levelwidths[l];

		for (i = 0; i < j; i++)
		{
			quadnode_t *node = &world.tree[l][i];

			if (client)
				LIST_CLEAR(&node->clientObjectList);
			else
				LIST_CLEAR(&node->serverObjectList);
		}
	}

	if (client)
		memset(clientObjects, 0, sizeof(clientObjects));
	else
		memset(serverObjects, 0, sizeof(serverObjects));
}

void	WT_BuildTree(int levels)
{
	int l, n;
	int num_nodes;	
	int x, y;

	memset(staticSurfaces, 0, sizeof(staticSurfaces));
	memset(clientObjects, 0, sizeof(clientObjects));
	memset(serverObjects, 0, sizeof(serverObjects));

	if (levels > MAX_TREE_LEVELS)
		levels = MAX_TREE_LEVELS;

	if (levels < 2)
		levels = 2;

	world.tree_levels = levels;
	num_nodes = (int)((pow(4, levels+1) - 1) / 3);	

	//allocate space for the new tree
	for (l=0; l<levels; l++)
	{
		world.tree[l] = Tag_Malloc((int)pow(4, l) * sizeof(quadnode_t), MEM_COLLISION);

		world.tree_boxdims[l] = GRID_TO_WORLD(world.gridwidth) / ((float)pow(2, l));
		world.tree_levelwidths[l] = pow(2, l);
	}

	//compute the lookup table for our box widths / heights

	//instead of recursively building the tree, we'll build it
	//"mipmap" style, one level at a time.  this makes referencing
	//any node at any level very easy, and opens things up for
	//some clever optimizations later on

	world.tree[0]->parent = NULL;

	for (l=0; l<levels; l++)
	{		
		n=0;		

		for (y=0; y<world.tree_levelwidths[l]; y++)
		{
			for (x=0; x<world.tree_levelwidths[l]; x++)
			{	
				quadnode_t *node;

				node = &world.tree[l][n];

				node->children[QUAD_TL] = WT_ChildNodeReference(l, x, y, QUAD_TL);
				node->children[QUAD_TR] = WT_ChildNodeReference(l, x, y, QUAD_TR);
				node->children[QUAD_BL] = WT_ChildNodeReference(l, x, y, QUAD_BL);
				node->children[QUAD_BR] = WT_ChildNodeReference(l, x, y, QUAD_BR);				

				//set parents
				
				if (node->children[0])
				{
					node->children[QUAD_TL]->parent = node;
					node->children[QUAD_TR]->parent = node;
					node->children[QUAD_BL]->parent = node;
					node->children[QUAD_BR]->parent = node;
				}

				LIST_CLEAR(&node->surfaceList);
				LIST_CLEAR(&node->clientObjectList);
				LIST_CLEAR(&node->serverObjectList);
			
				//compute the 2d coordinates of the box (we'll leave Z for later)
				node->bmin[0] = (world.tree_boxdims[l] * x)-1;
				node->bmin[1] = (world.tree_boxdims[l] * y)-1;
				node->bmax[0] = (world.tree_boxdims[l] * (x+1))+1;
				node->bmax[1] = (world.tree_boxdims[l] * (y+1))+1;

				//clear node height
				node->bmin[2] = 999999;
				node->bmax[2] = -999999;

				node->level = l;

				node->thingCount = 0;

				n++;
			}
		}
	}

	{
		int x,y;

		for (y=0; y<world.gridheight; y++)
			for (x=0; x<world.gridwidth; x++)
				WT_UpdateTerrainHeight(x, y, WORLD_GRIDREF(x,y));
	}
}

void	WT_TestTrace_Cmd(int argc, char *argv[])
{

}


int		WT_ClipPolyToSurface(linkedSurface_t *surf, scenefacevert_t *verts, int num_verts, scenefacevert_t *out, int ignorePlane)
{
	int plane;
	int n = 0;
	scenefacevert_t workingVerts[32];
	int numWorkingVerts = num_verts;

	if (num_verts < 3)
		return num_verts;

	memcpy(workingVerts, verts, sizeof(scenefacevert_t) * num_verts);

	for (plane=0; plane<surf->numPlanes; plane++)
	{
		if (plane == ignorePlane)
			continue;

		n = Scene_ClipPolyToPlane(workingVerts, numWorkingVerts, &surf->planes[plane], out, 32);
		numWorkingVerts = n;
		memcpy(workingVerts, out, sizeof(scenefacevert_t) * n);
	}

	memcpy(out, workingVerts, sizeof(scenefacevert_t) * n);

	return n;
}


extern cvar_t wr_marks;

#define FITPOLY_PUSHOUT	1


//this is an expensive function to call a lot of times per frame, so we provide a cacheing mechanism
void	WT_FitPolyToTerrain(scenefacevert_t *verts, int num_verts, residx_t shader, int flags, void (*clipCallback)(int num_verts, scenefacevert_t *verts, residx_t shader, int flags))
{
	vec2_t gridmin,gridmax;
	int	igridmin[2], igridmax[2];
	int n;
	int x,y,v;
	scenefacevert_t out[32];
	int facecount = 0;
	bool flatplane = true;
	float first_z;

	if (!wr_marks.integer)
		return;

	//get the gridspace we will be working in

	if (num_verts < 3)
		return;
	if (num_verts >= 32)
		return;

	M_ClearRect(gridmin, gridmax);

	for (n=0; n<num_verts; n++)
	{
		vec2_t vert;

		vert[0] = WORLD_TO_GRID(verts[n].vtx[0]);
		vert[1] = WORLD_TO_GRID(verts[n].vtx[1]);

		M_AddPointToRect(vert, gridmin, gridmax);
	}

	igridmin[0] = (int)(floor(gridmin[0]) + 0.5f);
	igridmin[1] = (int)(floor(gridmin[1]) + 0.5f);
	igridmax[0] = (int)(ceil(gridmax[0]) + 0.5f);
	igridmax[1] = (int)(ceil(gridmax[1]) + 0.5f);

	if (igridmax[0] - igridmin[0] > 9)
	{
		Console_DPrintf("warning, clipping poly from (%f, %f) to (%f, %f) grid squares\n", gridmin[0], gridmin[1], gridmax[0], gridmax[1]);
	}

	if (igridmin[0] < 0) igridmin[0] = 0;
	if (igridmin[1] < 0) igridmin[1] = 0;
	if (igridmin[0] > world.gridwidth-1) igridmin[0] = world.gridwidth-1;
	if (igridmin[1] > world.gridheight-1) igridmin[1] = world.gridheight-1;

	if (igridmax[0] < 0) igridmax[0] = 0;
	if (igridmax[1] < 0) igridmax[1] = 0;
	if (igridmax[0] > world.gridwidth-1) igridmax[0] = world.gridwidth-1;
	if (igridmax[1] > world.gridheight-1) igridmax[1] = world.gridheight-1;

	first_z = WORLD_GRIDREF(igridmin[0],igridmin[1]);

	for (y=igridmin[1]; y<=igridmax[1]; y++)
	{
		for (x=igridmin[0]; x<=igridmax[0]; x++)
		{
			if (WORLD_GRIDREF(x,y) != first_z)
				goto out;
		}
	}

	if (flatplane)
	{
		//easy flat case
		memcpy(out, verts, sizeof(scenefacevert_t) * num_verts);
		for (n=0; n<num_verts; n++)
		{
			out[n].vtx[2] = first_z + FITPOLY_PUSHOUT;
		}
		clipCallback(num_verts, out, shader, flags);
		return;
	}
out:

	for (y=igridmin[1]; y<=igridmax[1]; y++)
	{
		for (x=igridmin[0]; x<=igridmax[0]; x++)
		{
			linkedSurface_t tmp;

			WT_TempTerrainSurface(&tmp, x, y, 0);
			//align the poly with the surface plane
			for (v=0; v<num_verts; v++)
			{
				plane_t *pl;
				pl = &tmp.planes[0];
				verts[v].vtx[2] = ((pl->dist - (verts[v].vtx[0] * pl->normal[0] + verts[v].vtx[1] * pl->normal[1])) / pl->normal[2]) + FITPOLY_PUSHOUT;
			}
			
			n = WT_ClipPolyToSurface(&tmp, verts, num_verts, out, 0);
			if (n>2)
			{
				clipCallback(n, out, shader, flags);

				facecount++;
			}

			WT_TempTerrainSurface(&tmp, x, y, 1);
			//align the poly with the surface plane
			for (v=0; v<num_verts; v++)
			{
				plane_t *pl;
				pl = &tmp.planes[0];
				verts[v].vtx[2] = ((pl->dist - (verts[v].vtx[0] * pl->normal[0] + verts[v].vtx[1] * pl->normal[1])) / pl->normal[2]) + FITPOLY_PUSHOUT;
			}

			n = WT_ClipPolyToSurface(&tmp, verts, num_verts, out, 0);
			if (n>2)
			{
				clipCallback(n, out, shader, flags);

				facecount++;
			}
		}
	}
}




void	WT_Init()
{
	int l;
	
	for (l=0; l<MAX_TREE_LEVELS; l++)
	{
		world.tree[l] = NULL;		
	}

	world.tree_levels = 0;
	
	Cmd_Register("locktrace", WT_LockTrace_Cmd);
	Cmd_Register("testtrace", WT_TestTrace_Cmd);

	Cvar_Register(&wt_debug);	
	Cvar_Register(&wt_maxSurfsInNode);
	Cvar_Register(&wt_intersectMeshes);
	Cvar_Register(&wt_showTraceCount);
	Cvar_Register(&wt_boxTraceTerrain);
	Cvar_Register(&wt_bevelExpand);
}

	                  
