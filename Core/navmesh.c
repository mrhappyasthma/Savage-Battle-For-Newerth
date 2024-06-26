// (C) 2003 S2 Games

#include "core.h"
#include "allocator.h"
#include "navmesh.h"
#include "bsp.h"
#include "set.h"
#include "list.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define WORLD_TO_STEP(x) ((x)/navmesh_process_stepsize.value)
#define STEP_TO_WORLD(x) ((x)*navmesh_process_stepsize.value)
#define NAV_PRINTF if (navrep_debug.integer) Console_DPrintf

#define PATH_EPSILON 0.001f

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

char* pbyData = NULL;
bsp_polys_t* polys_anti = NULL;

//----------------------------------------------------------------------------

extern worldObject_t objects[];
extern cvar_t navrep_debug;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

cvar_t	navmesh_process_minslope = { "navmesh_process_minslope", "0.75" };
cvar_t	navmesh_process_stepsize = { "navmesh_process_stepsize", "100" };
cvar_t	navmesh_process_hullthreshhold = { "navmesh_process_hullthreshhold", "20" };
cvar_t	navmesh_process_subdivision = { "navmesh_process_subdivision", "200.0" };
cvar_t	navmesh_process_bridgeoffset = { "navmesh_process_bridgeoffset", "5.0" };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bool Quadtree_Lambda_DestroyNavPoly(void* data0, void* data1)
{
	navpoly_t* navpoly = (navpoly_t*)data0;
	NavPoly_Destroy(navpoly);
	return true;
}

//----------------------------------------------------------------------------

bool Quadtree_Lambda_NavPoly_ConnectAll(void* data0, void* data1)
{
	navpoly_t* pathpoly = (navpoly_t*)data0;
	navmesh_t* navMesh = (navmesh_t*)data1;
	
	NavPoly_Connect(navMesh, pathpoly);

	return true;
}

//----------------------------------------------------------------------------

bool Quadtree_Lambda_NavPoly_DisconnectAll(void* data0, void* data1)
{
	navpoly_t* pathpoly = (navpoly_t*)data0;
	navmesh_t* navMesh = (navmesh_t*)data1;
	
	NavPoly_Disconnect(navMesh, pathpoly);

	return true;
}

//----------------------------------------------------------------------------

bool Quadtree_LambdaFind_NavPoly_Unflooded(void* data0, void* data1)
{
	navpoly_t* pathpoly = (navpoly_t*)data0;

	// check and reset frame count for flooded poly

	bool result = pathpoly->frame == 0;
	pathpoly->frame = 0;
	
	return result;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void NavMesh_Init(navmesh_t* navMesh, float offset)
{
	vec3_t wmin, wmax;
	World_GetBounds(wmin,wmax);

	navMesh->offset = offset;
	navMesh->frame_cnxn = 0;
	navMesh->frame_poly = 0;
	navMesh->partition = 0;
	navMesh->partitionNext = 0;
	navMesh->quadtree = Quadtree_Create(wmin, wmax, 6);
	navMesh->timeNextPartition = 0;

	memset(navMesh->navpolyrefs, 0, sizeof(navpolyref_t)*MAX_OBJECTS);
	memset(navMesh->bridges, 0, sizeof(navpoly_t*)*(MAX_WORLDOBJECTS-MAX_OBJECTS));
}

//----------------------------------------------------------------------------

void NavMesh_Term(navmesh_t* navMesh)
{
	navpoly_t* navpolys = NULL;

	int n;
	for ( n = 0 ; n < MAX_OBJECTS ; ++n )
	{
		navpolyref_t* navpolyref = navMesh->navpolyrefs[n].first;
		while ( navpolyref )
		{
			navpolyref_t* navpolyref_next = navpolyref->next;
			navpoly_t* navpoly = navpolyref->navpoly;

			if ( !NavPoly_IsActive(navpoly) )
			{
				NavPoly_Activate(navpoly);
				
				// hack, build up inactive navpolys into a list using fragmentof member as "next"
				if ( navpolys )
					navpoly->fragmentof = navpolys;
				else
					navpoly->fragmentof = NULL;
				navpolys = navpoly;

//				Quadtree_Add(navMesh->quadtree, navpoly, navpoly->min, navpoly->max);
			}

			DEALLOCATE(navpolyref_t, navpolyref);

			navpolyref = navpolyref_next;
		}

		navMesh->navpolyrefs[n].first = NULL;
		navMesh->navpolyrefs[n].last = NULL;
		navMesh->navpolyrefs[n].obj = NULL;
	}

	if ( navpolys )
	{
		navpoly_t* navPoly = navpolys;
		while ( navPoly )
		{
			navpoly_t* navPolyNext = navPoly->fragmentof;
			NavPoly_Destroy(navPoly);
			navPoly = navPolyNext;
		}
	}

	for ( n = 0 ; n < (MAX_WORLDOBJECTS-MAX_OBJECTS) ; ++n )
	{
		navpoly_t* navPoly = navMesh->bridges[n];
		if ( navPoly )
		{
			NavPoly_Destroy(navPoly);
			navMesh->bridges[n] = NULL;
		}
	}

	if ( navMesh->quadtree )
	{
		Quadtree_Lambda(navMesh->quadtree, Quadtree_Lambda_DestroyNavPoly, NULL);
		Quadtree_Destroy(navMesh->quadtree);
		navMesh->quadtree = NULL;
	}
}

//----------------------------------------------------------------------------

void NavMesh_CSGSubtractStaticBSP(navmesh_t* navMesh, bsp_t* bsp)
{
	float find_width = 10;
	float find_height = 10;
	int num_pathpolys_found = 0;
	navpoly_t* pathpolys_found[4096];
	int i;

	// find all polies that may be clipped

	num_pathpolys_found = Quadtree_Find(navMesh->quadtree, (void **)pathpolys_found, 4096, vec2(bsp->min[0]-find_width, bsp->min[1]-find_height), vec2(bsp->max[0]+find_width, bsp->max[1]+find_height));

	for ( i = 0 ; i < num_pathpolys_found ; ++i )
	{
		navpoly_t* pathpoly = pathpolys_found[i];

		// csg-subtract the structure from this poly

		bsp_polys_t polys = { 0 };
		bsp_poly_t* poly = BSP_Poly_CreateFromPathPoly(pathpoly);
		
		BSP_Subtract(bsp, poly, &polys);

		BSP_Poly_Destroy(poly);

		// take the poly out of the active list

		Quadtree_Remove(navMesh->quadtree, pathpoly, pathpoly->min, pathpoly->max);

		// destroy the poly

		NavPoly_Destroy(pathpoly);

		// add the fragments into the world

		poly = polys.first;
		while ( poly )
		{
			navpoly_t* pathpoly_new = NavPoly_CreateFromBSPPoly(poly);
			
			// we're a root fragment
			
			pathpoly_new->fragmentof = pathpoly_new;

			// add this new pathpoly
			
			Quadtree_Add(navMesh->quadtree, pathpoly_new, pathpoly_new->min, pathpoly_new->max);

			//
			
			poly = poly->next;
		}

		BSP_Polys_Clear(&polys);
	}
}

//----------------------------------------------------------------------------

void NavMesh_CSGSubtractStatic(navmesh_t* navMesh, int n)
{
	int ms = System_Milliseconds();

	worldObject_t* object = &objects[n];
	model_t* model = Res_GetModel(object->model);
	int surf;

	if ( !WO_IsObjectActive(n) ) return;

	for ( surf = 0 ; surf < object->num_surfs ; ++surf )
	{
		int numSubdivsX = (object->surfs[surf]->bmax_w[0]-object->surfs[surf]->bmin_w[0])/navmesh_process_subdivision.value;
		int numSubdivsY = (object->surfs[surf]->bmax_w[1]-object->surfs[surf]->bmin_w[1])/navmesh_process_subdivision.value;
		int subdivsX, subdivsY;

		if ( object->surfs[surf]->flags & SURF_MODELBOUNDS ) continue;

		numSubdivsX = MAX(1, numSubdivsX);
		numSubdivsY = MAX(1, numSubdivsY);

		for ( subdivsY = 0 ; subdivsY < numSubdivsY ; ++subdivsY )
		{
			for ( subdivsX = 0 ; subdivsX < numSubdivsX ; ++subdivsX )
			{
				bsp_t* bsp_object;
				float z;
				vec3_t bmin, bmax;
				vec3_t bmin_w, bmax_w;

				bmin_w[0] = object->surfs[surf]->bmin_w[0] + subdivsX*((object->surfs[surf]->bmax_w[0] - object->surfs[surf]->bmin_w[0])/(float)numSubdivsX);
				bmax_w[0] = object->surfs[surf]->bmin_w[0] + (subdivsX+1)*((object->surfs[surf]->bmax_w[0] - object->surfs[surf]->bmin_w[0])/(float)numSubdivsX);
				bmin_w[1] = object->surfs[surf]->bmin_w[1] + subdivsY*((object->surfs[surf]->bmax_w[1] - object->surfs[surf]->bmin_w[1])/(float)numSubdivsY);
				bmax_w[1] = object->surfs[surf]->bmin_w[1] + (subdivsY+1)*((object->surfs[surf]->bmax_w[1] - object->surfs[surf]->bmin_w[1])/(float)numSubdivsY);

				z = World_CalcMaxZUnaligned(bmin_w[0], bmin_w[1], bmax_w[0], bmax_w[1]);

				if ( z < (object->surfs[surf]->bmin_w[2] - navmesh_process_hullthreshhold.value) || 
					 z > (object->surfs[surf]->bmax_w[2] + navmesh_process_hullthreshhold.value) ) continue;

				bmin[0] = model->surfs[surf].bmin[0] + subdivsX*((model->surfs[surf].bmax[0] - model->surfs[surf].bmin[0])/(float)numSubdivsX);
				bmax[0] = model->surfs[surf].bmin[0] + (subdivsX+1)*((model->surfs[surf].bmax[0] - model->surfs[surf].bmin[0])/(float)numSubdivsX);
				bmin[1] = model->surfs[surf].bmin[1] + subdivsY*((model->surfs[surf].bmax[1] - model->surfs[surf].bmin[1])/(float)numSubdivsY);
				bmax[1] = model->surfs[surf].bmin[1] + (subdivsY+1)*((model->surfs[surf].bmax[1] - model->surfs[surf].bmin[1])/(float)numSubdivsY);

				bsp_object = BSP_CreateFromObjectSurface(object, object->surfs[surf], bmin, bmax, navMesh->offset);
				if ( !bsp_object ) continue;

				NavMesh_CSGSubtractStaticBSP(navMesh, bsp_object);
				
				BSP_Destroy(bsp_object);
			}
		}
	}

	NAV_PRINTF("NavMesh_CSGSubtractStatic took %d ms\n", System_Milliseconds() - ms);
}

//----------------------------------------------------------------------------

void NavMesh_CSGSubtract_Internal(navmesh_t* navMesh, const baseObject_t* obj, navpoly_t* pathpoly_filter)
{
	int ms = System_Milliseconds();

	int fragment;
	set_t set_roots;
	bsp_t* bsp;

	bsp = BSP_CreateFromObject(obj, navMesh->offset);
	if ( !bsp ) return;	

	Set_Init(&set_roots, 0, NULL);

	{
		float find_width = 10;
		float find_height = 10;
		int num_pathpolys_found = 0;
		navpoly_t* pathpolys_found[4096];
		int i;

		num_pathpolys_found = Quadtree_Find(navMesh->quadtree, (void **)pathpolys_found, 4096, vec2(bsp->min[0]-find_width, bsp->min[1]-find_height), vec2(bsp->max[0]+find_width, bsp->max[1]+find_height));

		for ( i = 0 ; i < num_pathpolys_found ; ++i )
		{
			navpoly_t* pathpoly = pathpolys_found[i];
			navpoly_t* pathpoly_fragmentof = pathpoly->fragmentof;

			// we don't CSG bridge polies

			if ( pathpoly->bridge || pathpoly->bridge2 )
			{
				continue;
			}

			// if we're filtering, make sure this pathpoly is a fragment of the filter

			if ( pathpoly_filter && pathpoly_fragmentof != pathpoly_filter )
			{
				continue;
			}

			// csg-subtract the structure from this poly
			{
				bsp_polys_t polys = { 0 };
				bsp_poly_t* poly = BSP_Poly_CreateFromPathPoly(pathpoly);
				
				BSP_Subtract(bsp, poly, &polys);

				BSP_Poly_Destroy(poly);

				// take the poly out of the active list

				Quadtree_Remove(navMesh->quadtree, pathpoly, pathpoly->min, pathpoly->max);

				// disconnect it from all polies
				
				NavPoly_Disconnect(navMesh, pathpoly);

				// if it's a root fragment, deactivate it, otherwise delete it 

				if ( pathpoly == pathpoly_fragmentof )
				{
					NavPoly_Deactivate(pathpoly);
				}
				else
				{
					NavPoly_Destroy(pathpoly);
				}

				// track the root fragment in our set

				Set_Insert(&set_roots, pathpoly_fragmentof);

				// add the fragments into the world

				poly = polys.first;
				while ( poly )
				{
					navpoly_t* pathpoly_new = NavPoly_CreateFromBSPPoly(poly);
					
					// record what poly we are a fragment of
					
					pathpoly_new->fragmentof = pathpoly_fragmentof;

					// add this new pathpoly
	
					Quadtree_Add(navMesh->quadtree, pathpoly_new, pathpoly_new->min, pathpoly_new->max);

					// establish connectivity

					NavPoly_Connect(navMesh, pathpoly_new);

					//
					
					poly = poly->next;
				}

				BSP_Polys_Clear(&polys);
			}
		}
	}

	for ( fragment = 0 ; fragment < Set_GetSize(&set_roots) ; ++fragment )
	{
		navpolyref_t* navpolyref = ALLOCATE(navpolyref_t);
		navpolyref->navpoly = (navpoly_t*)set_roots.elements[fragment];
		List_PushFront(&navMesh->navpolyrefs[obj->index], navpolyref);
	}

	navMesh->navpolyrefs[obj->index].obj = obj;

	NAV_PRINTF("NavMesh_CSGSubtract_Internal added %d root fragments to structure %d\n", fragment, obj->index);

	BSP_Destroy(bsp);

	Set_Term(&set_roots);

	NAV_PRINTF("NavMesh_CSGSubtract_Internal took %d ms\n", System_Milliseconds() - ms);
}

//----------------------------------------------------------------------------

void NavMesh_CSGSubtract(navmesh_t* navMesh, const baseObject_t* obj)
{
	NavMesh_CSGSubtract_Internal(navMesh, obj, NULL);
}

//----------------------------------------------------------------------------

void NavMesh_CSGAdd_Internal(navmesh_t* navMesh, const baseObject_t* obj)
{
	int ms = System_Milliseconds();

	navpolyref_t* navpolyref;
	
	// kill all polys that are a fragment of the saved poly_ref

	navpolyref = navMesh->navpolyrefs[obj->index].first;
	while ( navpolyref )
	{
		// $todo: should we skip the ref if the poly is active?

		float find_width = 10;
		float find_height = 10;
		int num_pathpolys_found = 0;
		navpoly_t* pathpolys_found[4096];
		int i;

		// all our fragments must fall within the dims of the original pathpoly,
		// so we can speed up this lookup with the quadtree

		num_pathpolys_found = Quadtree_Find(navMesh->quadtree, (void **)pathpolys_found, 4096, vec2(navpolyref->navpoly->min[0]-find_width, navpolyref->navpoly->min[1]-find_height), vec2(navpolyref->navpoly->max[0]+find_width, navpolyref->navpoly->max[1]+find_height));

		for ( i = 0 ; i < num_pathpolys_found ; ++i )
		{
			navpoly_t* pathpoly = pathpolys_found[i];

			if ( navpolyref->navpoly == pathpoly->fragmentof )
			{
				Quadtree_Remove(navMesh->quadtree, pathpoly, pathpoly->min, pathpoly->max);
				NavPoly_Disconnect(navMesh, pathpoly);

				// if the poly is not a root fragment, destroy it, otherwise deactivate it

				if ( pathpoly != pathpoly->fragmentof )
				{
					NavPoly_Destroy(pathpoly);
				}
				else
				{
					// $todo: does the code EVER go in here?
					NavPoly_Deactivate(pathpoly);
				}
			}
		}

		navpolyref = navpolyref->next;
	}

	// re-add all original polygons stored in poly_refs 
	
	navpolyref = navMesh->navpolyrefs[obj->index].first;
	while ( navpolyref )
	{
		navpolyref_t* navpolyref_next = navpolyref->next;

		if ( !NavPoly_IsActive(navpolyref->navpoly) )
		{
			NavPoly_Activate(navpolyref->navpoly);
			Quadtree_Add(navMesh->quadtree, navpolyref->navpoly, navpolyref->navpoly->min, navpolyref->navpoly->max);
			NavPoly_Connect(navMesh, navpolyref->navpoly);

			// clear out partition id
			navpolyref->navpoly->partition = -1;

			// re-subtract structures from this poly
			{
				int i;
				set_t set_structures;

				Set_Init(&set_structures, 0, NULL);

				for ( i = 0 ; i < MAX_OBJECTS ; ++i )
				{
					navpolyref_t* navpolyref_other;

					if ( i == obj->index || !navMesh->navpolyrefs[i].obj ) continue;

					navpolyref_other = navMesh->navpolyrefs[i].first;
					while ( navpolyref_other )
					{
						navpolyref_t* navpolyref_other_next = navpolyref_other->next;

						if ( navpolyref_other->navpoly == navpolyref->navpoly )
						{
							List_Remove(&navMesh->navpolyrefs[i], navpolyref_other);
							DEALLOCATE(navpolyref_t, navpolyref_other);
							Set_Insert(&set_structures, (void*)i);
						}

						navpolyref_other = navpolyref_other_next;
					}
				}

				for ( i = 0 ; i < Set_GetSize(&set_structures) ; ++i )
				{
					NavMesh_CSGSubtract_Internal(navMesh, navMesh->navpolyrefs[(int)set_structures.elements[i]].obj, navpolyref->navpoly);
				}

				Set_Term(&set_structures);
			}
		}

		DEALLOCATE(navpolyref_t, navpolyref);

		navpolyref = navpolyref_next;
	}

	navMesh->navpolyrefs[obj->index].obj = NULL;
	navMesh->navpolyrefs[obj->index].first = NULL;
	navMesh->navpolyrefs[obj->index].last = NULL;

	NAV_PRINTF("NavMesh_CSGAdd_Internal took %d ms\n", System_Milliseconds() - ms);
}

//----------------------------------------------------------------------------

void NavMesh_CSGAdd(navmesh_t* navMesh, const baseObject_t* obj)
{
	NavMesh_CSGAdd_Internal(navMesh, obj);
}

//----------------------------------------------------------------------------

void NavMesh_FindPolyNearestPoint(navmesh_t* navMesh, const vec2_t point, int partition, navpoly_t** navPolyNearest, vec2_t pointNearest)
{
	float find_radius = 100;
	int num_navpolys_found = 0;
	navpoly_t* navpolys_found[4096];
	int i;

	float distance_nearest = FAR_AWAY;

	while ( !num_navpolys_found )
	{
		num_navpolys_found = Quadtree_Find(navMesh->quadtree, (void **)navpolys_found, 4096, vec2(point[0]-find_radius, point[1]-find_radius), vec2(point[0]+find_radius, point[1]+find_radius));

		if ( partition != -1 )
		{
			int i;
			for ( i = 0 ; i < num_navpolys_found ; )
			{
				navpoly_t* pathpoly = navpolys_found[i];
				// if we specified a partition, only look for navpolies in that partition
				if ( (pathpoly->partition != partition) )
				{
					// partition is not the same, throw out this pathpoly
					navpolys_found[i] = navpolys_found[--num_navpolys_found];
				}
				else
				{
					++i;
				}
			}
		}

		find_radius += 100;

		if ( find_radius > 16384 ) 
		{
			*navPolyNearest = NULL;
			return;
		}
	}

	for ( i = 0 ; i < num_navpolys_found ; ++i )
	{
		navpoly_t* pathpoly = navpolys_found[i];
		bool contains = true;
		navpolyedge_t* edge = pathpoly->edges;

		while ( edge )
		{
			if ( NavPolyEdge_CalculateDistanceToPoint(edge, point) < -PATH_EPSILON )
			{
				vec2_t point_closest;
				float distance;
				
				distance = NavPolyEdge_CalculateClosestPoint(edge, point, point_closest);
				if ( distance < distance_nearest )
				{
					if ( navPolyNearest )
					{
						*navPolyNearest = pathpoly;
					}

					M_CopyVec2(point_closest, pointNearest);
					distance_nearest = distance;
				}

				contains = false;
			}

			edge = edge->next;
		}

		if ( contains )
		{
			if ( navPolyNearest )
			{
				*navPolyNearest = pathpoly;
			}

			M_CopyVec2(point, pointNearest);

			return;
		}
	}

	// $TODO: handle this
	// search ALL

//	__asm { int 0x03 }

//	return true;
}

//----------------------------------------------------------------------------

navpoly_t* NavMesh_FindPolyContainingPoint(navmesh_t* navMesh, const vec2_t point, int partition)
{
	int num_navpolys_found = 0;
	navpoly_t* navpolys_found[4096];
	int pathpoly;

	num_navpolys_found = Quadtree_Find(navMesh->quadtree, (void **)navpolys_found, 4096, point, point);

	for ( pathpoly = 0 ; pathpoly < num_navpolys_found ; ++pathpoly )
	{
		// if we specified a partition, only look for navpolies in that partition
		if ( (partition != -1) && (navpolys_found[pathpoly]->partition != partition) ) continue;

		if ( NavPoly_ContainsPoint(navpolys_found[pathpoly], point) )
		{
			return navpolys_found[pathpoly];
		}
	}

	return NULL;
}

//----------------------------------------------------------------------------

bool NavMesh_Trace_Internal(navmesh_t* navMesh, navpoly_t* pathpoly, const vec2_t src, const vec2_t dest, const vec2_t dir)
{
	navpolyedge_t* edge;
	navpolycnxn_t* cnxn;
	bool dest_inside = true;

	pathpoly->frame = navMesh->frame_poly;

	edge = pathpoly->edges;
	while ( edge )
	{
		float distance_src = NavPolyEdge_CalculateDistanceToPoint(edge, src);
		float distance_dest = NavPolyEdge_CalculateDistanceToPoint(edge, dest);

		if ( distance_dest < -PATH_EPSILON )
		{
			dest_inside = false;
		}

		if ( (distance_src < -PATH_EPSILON && distance_dest > PATH_EPSILON) ||
			 (distance_src > PATH_EPSILON && distance_dest < -PATH_EPSILON) )
		{
			// we cross this (infinite) edge

			cnxn = edge->cnxns.first;
			while ( cnxn )
			{
				// $todo: somebody crashed here?
				if ( M_RayIntersectsLineSeg2d(src, dir, cnxn->src, cnxn->dest, PATH_EPSILON) )
				{
					if ( cnxn->into->frame < navMesh->frame_poly )
					{
						if ( NavMesh_Trace_Internal(navMesh, cnxn->into, src, dest, dir) )
						{
							return true;
						}
					}
				}
				
				cnxn = cnxn->next;
			}
		}

		edge = edge->next;
	}

	return dest_inside;
}

//----------------------------------------------------------------------------

bool NavMesh_Trace(navmesh_t* navMesh, const vec2_t src, const vec2_t dest)
{
	vec2_t dir;
	navpoly_t* pathpoly;

	M_SubVec2(dest, src, dir);
	M_NormalizeVec2(dir);

	pathpoly = NavMesh_FindPolyContainingPoint(navMesh, src, -1);
	if ( !pathpoly ) return false;

	navMesh->frame_poly++;

	return NavMesh_Trace_Internal(navMesh, pathpoly, src, dest, dir);
}

//----------------------------------------------------------------------------

bool BSP_Subtract_KeepInsideFragments(bsp_t* self, bsp_poly_t* poly, bsp_polys_t* polys)
{
	bsp_polys_t polys_out = { NULL, NULL };
	
	BSP_ClipPoly(self, poly, polys, &polys_out, self->root);	

	if ( !polys_out.first )
	{
		// Not clipped, just put "poly" in "polys"
		
		BSP_Polys_Clear(polys);
		BSP_Polys_PushBack(polys, poly);

		return false;
	}
	else
	{
		// Clipped, fragments are in "polys"
		
		BSP_Polys_Clear(&polys_out);

		return true;
	}
}

//----------------------------------------------------------------------------

bool NavMesh_CanPlaceBuilding(navmesh_t* navMesh, const vec3_t bminw, const vec3_t bmaxw)
{
	int num_navpolys_found = 0;
	navpoly_t* navpolys_found[4096];
	int navpoly;
	bsp_t* bsp;
	bsp_polys_t* bspPolysIn;
	bsp_poly_t* bspPolyIn;
	float area = 0;

	num_navpolys_found = Quadtree_Find(navMesh->quadtree, navpolys_found, 4096, bminw, bmaxw);
	if ( !num_navpolys_found ) return false;

	//

	bsp = BSP_CreateFromBounds(bminw, bmaxw, 0);//navMesh->offset);
	bspPolysIn = BSP_Polys_Create();

	//

	for ( navpoly = 0 ; navpoly < num_navpolys_found ; ++navpoly )
	{
		bsp_poly_t* bspPoly = BSP_Poly_CreateFromPathPoly(navpolys_found[navpoly]);
		BSP_Subtract_KeepInsideFragments(bsp, bspPoly, bspPolysIn);	
		BSP_Poly_Destroy(bspPoly);
	}

	//

	bspPolyIn = bspPolysIn->first;
	while ( bspPolyIn )
	{
		area += BSP_Poly_CalculateArea(bspPolyIn);
		bspPolyIn = bspPolyIn->next;
	}

	//

	BSP_Polys_Destroy(bspPolysIn);
	BSP_Destroy(bsp);

	// compare area of inside fragments vs. area of extents

	return fabs(area - (bmaxw[0] - bminw[0])*(bmaxw[1] - bminw[1])) < 1.0f;
}

//----------------------------------------------------------------------------

void NavMesh_FindNearestPointAroundObject_SetupEdges(bsp_line_t edges[4], const baseObject_t* obj, float offset)
{
	vec3_t bmin, bmax;
	vec3_t axis[3];
	vec3_t points[4];
	objectPosition_t objpos;

	WO_GetObjectPos(obj->index, &objpos);
	Res_GetModelSurfaceBounds(WO_GetObjectModel(obj->index), bmin, bmax);

	// this is a building. offset the dims, and transform to obb.

	bmin[0] *= objpos.scale;
	bmin[0] -= offset;
	
	bmin[1] *= objpos.scale;
	bmin[1] -= offset;

	bmax[0] *= objpos.scale;
	bmax[0] += offset;
	
	bmax[1] *= objpos.scale;
	bmax[1] += offset;

	M_GetAxis(obj->angle[0], obj->angle[1], obj->angle[2], axis);
	M_TransformPoint(vec3(bmin[0], bmin[1], 0), objpos.position, (const vec3_t *)axis, points[0]);
	M_TransformPoint(vec3(bmin[0], bmax[1], 0), objpos.position, (const vec3_t *)axis, points[1]);
	M_TransformPoint(vec3(bmax[0], bmax[1], 0), objpos.position, (const vec3_t *)axis, points[2]);
	M_TransformPoint(vec3(bmax[0], bmin[1], 0), objpos.position, (const vec3_t *)axis, points[3]);

	BSP_Line_Set(&edges[0], points[1], points[0]);
	BSP_Line_Set(&edges[1], points[2], points[1]);
	BSP_Line_Set(&edges[2], points[3], points[2]);
	BSP_Line_Set(&edges[3], points[0], points[3]);
}

//----------------------------------------------------------------------------

bool NavMesh_FindNearestPointAroundObject(navmesh_t* navMesh, const baseObject_t* obj, const vec2_t point, int partition, struct navpoly_s** navPolyNearest, vec2_t pointNearest)
{
	int i;
	int num_navpolys_found = 0;
	navpoly_t* navpolys_found[4096];
	int pathpoly;
	vec3_t bmin, bmax;
	bsp_line_t edges[4];
	vec2_t point_closest;
	float closestSqr = FAR_AWAY;

	// we don't do this for npcs and whatnot, only structures
	if ( !WO_IsObjectActive(obj->index) )
	{
		return false;
	}

	*navPolyNearest = NULL;

	NavMesh_FindNearestPointAroundObject_SetupEdges(edges, obj, navMesh->offset);

	bmin[0] = MIN(edges[0].src[0], MIN(edges[1].src[0], MIN(edges[2].src[0], edges[3].src[0])));
	bmin[1] = MIN(edges[0].src[1], MIN(edges[1].src[1], MIN(edges[2].src[1], edges[3].src[1])));
	bmax[0] = MAX(edges[0].src[0], MAX(edges[1].src[0], MAX(edges[2].src[0], edges[3].src[0])));
	bmax[1] = MAX(edges[0].src[1], MAX(edges[1].src[1], MAX(edges[2].src[1], edges[3].src[1])));

//	M_AddVec3(obj->pos, obj->bmin, bmin);
//	M_AddVec3(obj->pos, obj->bmax, bmax);

	num_navpolys_found = Quadtree_Find(navMesh->quadtree, navpolys_found, 4096, bmin, bmax);

	for ( i = 0 ; i < 4 ; ++i )
	{
		float length = sqrt(M_GetDistanceSqVec2(edges[i].src, edges[i].dest));

		for ( pathpoly = 0 ; pathpoly < num_navpolys_found ; ++pathpoly )
		{
			navpoly_t* poly = navpolys_found[pathpoly];
			navpolyedge_t* edge = poly->edges;

			// if we specified a partition, only look for navpolies in that partition
			if ( (partition != -1) && (poly->partition != partition) ) continue;

			while ( edge )
			{
				float ts[2];

				if ( BSP_Line_ParameterizePoint(&edges[i], edge->src, &ts[0]) && 
					 BSP_Line_ParameterizePoint(&edges[i], edge->dest, &ts[1]) )
				{
					// sort the two t's
					{
						float temp;
						if ( ts[0] > ts[1] )
						{
							temp = ts[1];
							ts[1] = ts[0];
							ts[0] = temp;
						}
					}

					if ( (ts[0] < 0.0f && ts[1] > 0.0f) ||
						 (ts[0] > 0.0f && ts[0] < 1.0f) ||
						 (ts[1] > 0.0f && ts[1] < 1.0f) ||
						 (ts[0] < 0.0f && ts[1] > 1.0f) ||
						 (ts[1] == 1.0f && ts[0] < 1.0f) ||
						 (ts[0] == 0.0f && ts[1] > 0.0f) )
					{
						vec2_t verts[2];
						float distanceSqr;

						ts[0] = MAX(ts[0], 0.0f);
						ts[1] = MIN(ts[1], 1.0f);

						M_MultVec2(edges[i].dir, length*ts[0], verts[0]);
						M_AddVec2(verts[0], edges[i].src, verts[0]);

						M_MultVec2(edges[i].dir, length*ts[1], verts[1]);
						M_AddVec2(verts[1], edges[i].src, verts[1]);

						M_ClosestPointToSegment2d(verts[0], verts[1], point, point_closest);
						distanceSqr = M_GetDistanceSqVec2(point, point_closest);

						if ( distanceSqr < closestSqr )
						{
							M_CopyVec2(point_closest, pointNearest);
							closestSqr = distanceSqr;
							*navPolyNearest = poly;
						}
					}
				}

				edge = edge->next;
			}
		}
	}

	return *navPolyNearest != NULL;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void NavMesh_Process_Vectorize_Cell(navmesh_t* navMesh, int cx, int cy, int x, int y, bsp_t* bsp)
{
	int sx0 = STEP_TO_WORLD(x);
	int sx1 = STEP_TO_WORLD(x+1);
	int sy0 = STEP_TO_WORLD(y);
	int sy1 = STEP_TO_WORLD(y+1);
	bsp_line_t line;

	if ( pbyData[y*cx + x] & 0x80 )
	{
		// Check left edge
		{
			if ( (x == 0) || !(pbyData[y*cx + x-1] & 0x08) )
			{
				BSP_Line_Set(&line, vec2(sx0, sy1), vec2(sx0, sy0));
				BSP_Add(bsp, &line, NULL);
			}
		}
		// Check bottom edge
		{
			if ( (y == 0) || !(pbyData[(y-1)*cx + x] & 0x08) )
			{
				BSP_Line_Set(&line, vec2(sx0, sy0), vec2(sx1, sy0));
				BSP_Add(bsp, &line, NULL);
			}
		}
		// Check diagonal edge
		{
			if ( !(pbyData[y*cx + x] & 0x08) )
			{
				BSP_Line_Set(&line, vec2(sx1, sy0), vec2(sx0, sy1));
				BSP_Add(bsp, &line, NULL);
			}
		}
	}
	if ( pbyData[y*cx + x] & 0x08 )
	{
		// Check right edge
		{
			if ( (x == (cx-1)) || !(pbyData[y*cx + x+1] & 0x80) )
			{
				BSP_Line_Set(&line, vec2(sx1, sy0), vec2(sx1, sy1));
				BSP_Add(bsp, &line, NULL);
			}
		}
		// Check top edge
		{
			if ( (y == (cy-1)) || !(pbyData[(y+1)*cx + x] & 0x80) )
			{
				BSP_Line_Set(&line, vec2(sx1, sy1), vec2(sx0, sy1));
				BSP_Add(bsp, &line, NULL);
			}
		}
		// Check diagonal edge
		{
			if ( !(pbyData[y*cx + x] & 0x80) )
			{
				BSP_Line_Set(&line, vec2(sx0, sy1), vec2(sx1, sy0));
				BSP_Add(bsp, &line, NULL);
			}
		}
	}
}

//----------------------------------------------------------------------------

navpoly_t* NavPoly_CreateFrom4Points(vec3_t point0, vec3_t point1, vec3_t point2, vec3_t point3)
{
	navpolyedge_t* navEdge;
	navpoly_t* navPoly = ALLOCATE(navpoly_t);
	navPoly->edges = NULL;
	navPoly->min[0] = MIN(point0[0], MIN(point1[0], MIN(point2[0], point3[0])));
	navPoly->min[1] = MIN(point0[1], MIN(point1[1], MIN(point2[1], point3[1])));
	navPoly->max[0] = MAX(point0[0], MAX(point1[0], MAX(point2[0], point3[0])));
	navPoly->max[1] = MAX(point0[1], MAX(point1[1], MAX(point2[1], point3[1])));
	navPoly->frame = 0;
	navPoly->partition = -1;
	navPoly->fragmentof = navPoly;
	navPoly->bridge = false;
	navPoly->bridge2 = false;
	navPoly->active = true;

	navEdge = NavPolyEdge_Create(point0, point1);
	navEdge->next = navPoly->edges;
	navPoly->edges = navEdge;

	navEdge = NavPolyEdge_Create(point1, point2);
	navEdge->next = navPoly->edges;
	navPoly->edges = navEdge;
	
	navEdge = NavPolyEdge_Create(point2, point3);
	navEdge->next = navPoly->edges;
	navPoly->edges = navEdge;
	
	navEdge = NavPolyEdge_Create(point3, point0);
	navEdge->next = navPoly->edges;
	navPoly->edges = navEdge;

	return navPoly;
}

//----------------------------------------------------------------------------

void NavPoly_Partition(navmesh_t* navMesh, navpoly_t* navPoly)
{
	navpolyedge_t* edge;

	// if this navPoly's partition id equals the current partition id,
	// then we've already visited it, so we can just return

	if ( navPoly->partition >= navMesh->partition ) return;

	// set our partition id to the current partition id

	navPoly->partition = navMesh->partitionNext;

	// recursively flood this partition id to all our connected navpolies

	edge = navPoly->edges;
	while ( edge )
	{
		navpolycnxn_t* cnxn = edge->cnxns.first;
		while ( cnxn )
		{
			NavPoly_Partition(navMesh, cnxn->into);

			cnxn = cnxn->next;
		}

		edge = edge->next;
	}
}

bool Quadtree_Lambda_NavPoly_Partition(void* data0, void* data1)
{
	navpoly_t* navPoly = (navpoly_t*)data0;
	navmesh_t* navMesh = (navmesh_t*)data1;

	// if we've already been flooded, return

	if ( navPoly->partition >= navMesh->partition ) return true;

	// flood through all the connected navPolys with the partition id.

	NavPoly_Partition(navMesh, navPoly);

	// after the flood, a set has been partitioned. increment the partition id.

	navMesh->partitionNext++;

	return true;
}

void NavMesh_Partition(navmesh_t* navMesh)
{
	navMesh->partition = ++navMesh->partitionNext;

	Quadtree_Lambda(navMesh->quadtree, Quadtree_Lambda_NavPoly_Partition, navMesh);	
}

//----------------------------------------------------------------------------

void NavMesh_Process_Bridges(navmesh_t* navMesh)
{
	int ms = System_Milliseconds();

	int n;
	for ( n = MAX_OBJECTS ; n < MAX_WORLDOBJECTS ; n++ )
	{
		if ( WO_IsObjectActive(n) && !WO_IsObjectReference(n) )
		{
			int objdef = WO_GetObjectObjdef(n);
			if ( objdef != 0 && WO_GetObjdefVarValue(objdef, "obj_navBridge") )
			{
				vec3_t bmin, bmax;
				worldObject_t* obj = &objects[n];
				int axis;
				navpoly_t* navPoly = NULL, * navPolyMin = NULL, * navPolyMax = NULL;
				vec3_t pointMin, pointMax, pointsBridge[4];
				float offsetMin = navMesh->offset + navmesh_process_bridgeoffset.value;
				float offsetMax = navMesh->offset + navmesh_process_bridgeoffset.value;

				Res_GetModelSurfaceBounds(obj->model, bmin, bmax);
				axis = bmax[0] - bmin[0] > bmax[1] - bmin[1] ? 0 : 1;
				if ( WO_GetObjdefVarValue(objdef, "obj_navBridgeWide") )
				{
					axis ^= 1;
				}

				while ( (!navPolyMin || !navPolyMax) && (offsetMin < 300 && offsetMax < 300) )
				{
					vec3_t pointsMin[2], pointsMax[2];

					pointsMin[0][axis] = bmin[axis] - offsetMin;
					pointsMin[0][axis^1] = bmin[axis^1];
					pointsMin[0][2] = 0;

					pointsMin[1][axis] = bmin[axis] - offsetMin;
					pointsMin[1][axis^1] = bmax[axis^1];
					pointsMin[1][2] = 0;

					pointsMax[0][axis] = bmax[axis] + offsetMax;
					pointsMax[0][axis^1] = bmin[axis^1];
					pointsMax[0][2] = 0;

					pointsMax[1][axis] = bmax[axis] + offsetMax;
					pointsMax[1][axis^1] = bmax[axis^1];
					pointsMax[1][2] = 0;

					M_AddVec3(pointsMin[1], pointsMin[0], pointMin);
					M_MultVec3(pointMin, 0.3f, pointMin);
					M_AddVec3(pointsMax[1], pointsMax[0], pointMax);
					M_MultVec3(pointMax, 0.3f, pointMax);

					M_MultVec3(pointMin, obj->objpos.scale, pointMin);
					M_TransformPoint(pointMin, obj->objpos.position, (const vec3_t *)obj->axis, pointMin);					

					M_MultVec3(pointMax, obj->objpos.scale, pointMax);
					M_TransformPoint(pointMax, obj->objpos.position, (const vec3_t *)obj->axis, pointMax);

					navPolyMin = NavMesh_FindPolyContainingPoint(navMesh, pointMin, -1);
					if ( !navPolyMin ) offsetMin += 10.0f;
					navPolyMax = NavMesh_FindPolyContainingPoint(navMesh, pointMax, -1);
					if ( !navPolyMax ) offsetMax += 10.0f;
				}

				if ( !navPolyMin || !navPolyMax ) continue;

				navPolyMin = NavPoly_CreateFrom4Points(vec2(pointMin[0]-5, pointMin[1]-5), vec2(pointMin[0]+5, pointMin[1]-5), vec2(pointMin[0]+5, pointMin[1]+5), vec2(pointMin[0]-5, pointMin[1]+5));
				navPolyMin->bridge = true;
				{
					bsp_t* bsp_object = BSP_CreateFromBounds(vec2(pointMin[0]-5,pointMin[1]-5), vec2(pointMin[0]+5,pointMin[1]+5), 0.0f);
					if ( !bsp_object ) continue;

					NavMesh_CSGSubtractStaticBSP(navMesh, bsp_object);
					
					BSP_Destroy(bsp_object);
				}
				Quadtree_Add(navMesh->quadtree, navPolyMin, navPolyMin->min, navPolyMin->max);

				navPolyMax = NavPoly_CreateFrom4Points(vec2(pointMax[0]-5, pointMax[1]-5), vec2(pointMax[0]+5, pointMax[1]-5), vec2(pointMax[0]+5, pointMax[1]+5), vec2(pointMax[0]-5, pointMax[1]+5));
				navPolyMax->bridge = true;
				{
					bsp_t* bsp_object = BSP_CreateFromBounds(vec2(pointMax[0]-5,pointMax[1]-5), vec2(pointMax[0]+5,pointMax[1]+5), 0.0f);
					if ( !bsp_object ) continue;

					NavMesh_CSGSubtractStaticBSP(navMesh, bsp_object);
					
					BSP_Destroy(bsp_object);
				}
				Quadtree_Add(navMesh->quadtree, navPolyMax, navPolyMax->min, navPolyMax->max);

				if (navPolyMin->edges)
				{
					M_CopyVec2(navPolyMin->edges->src, pointsBridge[0]);
					M_CopyVec2(navPolyMin->edges->dest, pointsBridge[1]);
					M_CopyVec2(navPolyMax->edges->src, pointsBridge[2]);
					M_CopyVec2(navPolyMax->edges->dest, pointsBridge[3]);

					navPoly = NavPoly_CreateFrom4Points(pointsBridge[0], pointsBridge[1], pointsBridge[2], pointsBridge[3]);
					navPoly->bridge2 = true;
					navMesh->bridges[n-MAX_OBJECTS] = navPoly;

					NavPoly_ConnectToPoly(navPoly, navPolyMin);
					NavPoly_ConnectToPoly(navPoly, navPolyMax);
				}
			}
		}
	}

	NAV_PRINTF("NavMesh_Process_Bridges() took %d ms\n", System_Milliseconds() - ms);
}

//----------------------------------------------------------------------------

void NavMesh_Process_ConnectAll(navmesh_t* navMesh)
{
	int ms = System_Milliseconds();

	Quadtree_Lambda(navMesh->quadtree, Quadtree_Lambda_NavPoly_ConnectAll, navMesh);

	NAV_PRINTF("NavMesh_Process_ConnectAll() took %d ms\n", System_Milliseconds() - ms);
}

//----------------------------------------------------------------------------

void NavMesh_Process_DisconnectAll(navmesh_t* navMesh)
{
	int ms = System_Milliseconds();

	Quadtree_Lambda(navMesh->quadtree, Quadtree_Lambda_NavPoly_DisconnectAll, navMesh);

	NAV_PRINTF("NavMesh_Process_DisconnectAll() took %d ms\n", System_Milliseconds() - ms);
}

//----------------------------------------------------------------------------

void NavMesh_Process_Vectorize(navmesh_t* navMesh)
{
	bsp_t bsp_terrain =  { 0 };
	bsp_polys_t* polys_terrain = BSP_Polys_Create();

	// make bsp of unshared edges in terrain
	{
		vec3_t wmin, wmax;
		int t, x, y, cx, cy;
		
		World_GetBounds(wmin,wmax);

		cx = WORLD_TO_STEP(wmax[0]);
		cy = WORLD_TO_STEP(wmax[1]);

		for ( t = 0 ; t < cx ; ++t )
		{
			for ( x = 0 ; x <= t ; ++x )
			{
				NavMesh_Process_Vectorize_Cell(navMesh, cx, cy, x, t, &bsp_terrain);
			}

			for ( y = 0 ; y <= t ; ++y )
			{
				NavMesh_Process_Vectorize_Cell(navMesh, cx, cy, t, y, &bsp_terrain);
			}
		}
	}

	// drop world quad through terrain bsp to walkable poly fragments (in "polys_terrain")
	{
		bsp_poly_t* poly_world = NULL;
		poly_world = BSP_Poly_CreateFromWorld();
		polys_anti = BSP_Polys_Create();

		BSP_ClipPoly(&bsp_terrain, poly_world, polys_terrain, polys_anti, bsp_terrain.root);
		
		BSP_Poly_Destroy(poly_world);
	}

	// convert fragments into pathpolys
	{
		int numpolys = 0;

		const bsp_poly_t* poly = polys_terrain->first;
		while ( poly )
		{
			navpoly_t* pathpoly = NavPoly_CreateFromBSPPoly(poly);

			pathpoly->fragmentof = pathpoly;

			numpolys++;

			Quadtree_Add(navMesh->quadtree, pathpoly, pathpoly->min, pathpoly->max);

			poly = poly->next;
		}

		BSP_Polys_Destroy(polys_terrain);
		BSP_Clear(&bsp_terrain, bsp_terrain.root);
	}

	Tag_Free(pbyData);
	pbyData = NULL;
}

//----------------------------------------------------------------------------

void NavMesh_Process_Shrink(navmesh_t* navMesh)
{
	bsp_t* bsps_anti = NULL;

	// convert anti polies into expanded anti bsps
	{
		const bsp_poly_t* poly = polys_anti->first;
		while ( poly )
		{
			bsp_t* bsp_anti = BSP_Create();
			bsp_lines_t lines_anti = { 0 };

			bsp_anti->min[0] = bsp_anti->min[1] = FAR_AWAY;
			bsp_anti->max[0] = bsp_anti->max[1] = -FAR_AWAY;

			// convert bsp_poly into bsp_lines
			{
				bsp_vert_t* vert = poly->verts->first;
				bsp_vert_t* vert_next = vert->next;

				do
				{
					bsp_line_t line_anti;
					BSP_Line_Set(&line_anti, vert->vert, vert_next->vert);
					BSP_Lines_PushBack(&lines_anti, &line_anti);

					vert = vert_next;
					vert_next = vert->next;

					if ( vert_next == NULL )
					{
						vert_next = poly->verts->first;
					}
				}
				while ( vert != poly->verts->first );
			}

			// expand lines
			{
				bsp_line_t* line = lines_anti.first;
				bsp_line_t* line_next = line->next;

				do
				{
					// expand line out

					bsp_line_t line_offset;
					memcpy(&line_offset, line, sizeof(bsp_line_t));
					line_offset.src[0] += line_offset.perp[0] * navMesh->offset;
					line_offset.src[1] += line_offset.perp[1] * navMesh->offset;
					line_offset.dest[0] += line_offset.perp[0] * navMesh->offset;
					line_offset.dest[1] += line_offset.perp[1] * navMesh->offset;
					BSP_Add(bsp_anti, &line_offset, NULL);

					bsp_anti->min[0] = MIN(bsp_anti->min[0], MIN(line_offset.src[0], line_offset.dest[0]));
					bsp_anti->min[1] = MIN(bsp_anti->min[1], MIN(line_offset.src[1], line_offset.dest[1]));
					bsp_anti->max[0] = MAX(bsp_anti->max[0], MAX(line_offset.src[0], line_offset.dest[0]));
					bsp_anti->max[1] = MAX(bsp_anti->max[1], MAX(line_offset.src[1], line_offset.dest[1]));

					if ( !M_CompareVec2(line->dir, line_next->dir) )
					{
						bsp_line_t line_next_offset;
						memcpy(&line_next_offset, line_next, sizeof(bsp_line_t));
						line_next_offset.src[0] += line_next_offset.perp[0] * navMesh->offset;
						line_next_offset.src[1] += line_next_offset.perp[1] * navMesh->offset;
						line_next_offset.dest[0] += line_next_offset.perp[0] * navMesh->offset;
						line_next_offset.dest[1] += line_next_offset.perp[1] * navMesh->offset;
						BSP_Add(bsp_anti, &line_next_offset, NULL);

						bsp_anti->min[0] = MIN(bsp_anti->min[0], MIN(line_next_offset.src[0], line_next_offset.dest[0]));
						bsp_anti->min[1] = MIN(bsp_anti->min[1], MIN(line_next_offset.src[1], line_next_offset.dest[1]));
						bsp_anti->max[0] = MAX(bsp_anti->max[0], MAX(line_next_offset.src[0], line_next_offset.dest[0]));
						bsp_anti->max[1] = MAX(bsp_anti->max[1], MAX(line_next_offset.src[1], line_next_offset.dest[1]));

						{
							bsp_line_t line_bevel;
							BSP_Line_Set(&line_bevel, line_offset.dest, line_next_offset.src);
							BSP_Add(bsp_anti, &line_bevel, NULL);

							bsp_anti->min[0] = MIN(bsp_anti->min[0], MIN(line_bevel.src[0], line_bevel.dest[0]));
							bsp_anti->min[1] = MIN(bsp_anti->min[1], MIN(line_bevel.src[1], line_bevel.dest[1]));
							bsp_anti->max[0] = MAX(bsp_anti->max[0], MAX(line_bevel.src[0], line_bevel.dest[0]));
							bsp_anti->max[1] = MAX(bsp_anti->max[1], MAX(line_bevel.src[1], line_bevel.dest[1]));
						}
					}

					line = line_next;
					line_next = line->next;

					if ( line_next == NULL )
					{
						line_next = lines_anti.first;
					}
				}
				while ( line != lines_anti.first );
			}

			BSP_Lines_Clear(&lines_anti);

			bsp_anti->next = bsps_anti;
			bsps_anti = bsp_anti;

			poly = poly->next;
		}
	}

	// clip path polies by anti bsps
	if ( bsps_anti )
	{
		bsp_t* bsp = bsps_anti;
		while ( bsp )
		{
			float find_width = 10;
			float find_height = 10;
			int num_pathpolys_found = 0;
			navpoly_t* pathpolys_found[4096];
			int i;

			num_pathpolys_found = Quadtree_Find(navMesh->quadtree, (void **)pathpolys_found, 4096, vec2(bsp->min[0]-find_width, bsp->min[1]-find_height), vec2(bsp->max[0]+find_width, bsp->max[1]+find_height));

			for ( i = 0 ; i < num_pathpolys_found ; ++i )
			{
				navpoly_t* pathpoly = pathpolys_found[i];

				// create a bsp poly from the path poly

				bsp_poly_t* poly = BSP_Poly_CreateFromPathPoly(pathpoly);

				// clip the pathpoly against the anti bsp
				
				bsp_polys_t polys = { 0 };
				bool subtracted = BSP_Subtract(bsp, poly, &polys);

				// destroy bsp poly

				BSP_Poly_Destroy(poly);

				// if we have real fragments, push them onto the end of the list and destroy the pathpoly

				if ( subtracted )
				{
					poly = polys.first;
					while ( poly )
					{
						navpoly_t* pathpoly_fragment = NavPoly_CreateFromBSPPoly(poly);
						pathpoly_fragment->fragmentof = pathpoly_fragment;
						Quadtree_Add(navMesh->quadtree, pathpoly_fragment, pathpoly_fragment->min, pathpoly_fragment->max);

						poly = poly->next;
					}
				
					// remove this pathpoly from the list

					Quadtree_Remove(navMesh->quadtree, pathpoly, pathpoly->min, pathpoly->max);

					// destroy the path poly

					NavPoly_Destroy(pathpoly);

					// clear out the fragment list

					BSP_Polys_Clear(&polys);

					// break

					//$TODO: WHY did we have this??? break;
				}
				else
				{
					// clear out the fragment list

					BSP_Polys_Clear(&polys);
				}
			}
			
			bsp = bsp->next;
		}
	}

	// clean out anti bsps
	{
		bsp_t* bsp = bsps_anti;
		while ( bsp )
		{
			bsp_t* bsp_next = bsp->next;
			BSP_Destroy(bsp);
			bsp = bsp_next;
		}

		bsps_anti = NULL;
	}

	BSP_Polys_Destroy(polys_anti);
	polys_anti = NULL;
}

//----------------------------------------------------------------------------

void NavMesh_Process_Cleanup(navmesh_t* navMesh)
{
	// merge polies into larger convex if possible
}

//----------------------------------------------------------------------------

void NavMesh_Process_Flood(navmesh_t* navMesh, navpoly_t* path_poly)
{
	navpolyedge_t* edge;
	
	path_poly->frame = -1;

	edge = path_poly->edges;
	while ( edge )
	{
		navpolycnxn_t* cnxn = edge->cnxns.first;
		while ( cnxn )
		{
			if ( cnxn->into->frame == 0 )
			{
				NavMesh_Process_Flood(navMesh, cnxn->into);
			}

			cnxn = cnxn->next;
		}

		edge = edge->next;
	}
}

//----------------------------------------------------------------------------

void NavMesh_Process_Rasterize(navmesh_t* navMesh)
{
	vec3_t wmin, wmax;
	int x, y, cx, cy;
	//int halfstep = navmesh_process_stepsize.value/2;
	int quarterstep = navmesh_process_stepsize.value/4;
	pointinfo_t pi;
	
	World_GetBounds(wmin,wmax);

	cx = WORLD_TO_STEP(wmax[0]);
	cy = WORLD_TO_STEP(wmax[1]);

	pbyData = Tag_Malloc(cx*cy, MEM_PATH);

	for ( y = 0 ; y < cy ; ++y )
	{
		for ( x = 0 ; x < cx ; ++x )
		{
			int wx0, wy0, wx1, wy1;
			wx0 = STEP_TO_WORLD(x);
			wy0 = STEP_TO_WORLD(y);
			wx1 = STEP_TO_WORLD(x+1);
			wy1 = STEP_TO_WORLD(y+1);

			pbyData[y*cx + x] = 0;

			{
				World_SampleGround(wx0+quarterstep, wy0+quarterstep, &pi);

				if ( pi.nml[2] > navmesh_process_minslope.value )
				{
					pbyData[y*cx + x] |= 0x80;
				}
				
				World_SampleGround(wx1-quarterstep, wy1-quarterstep, &pi);

				if ( pi.nml[2] > navmesh_process_minslope.value )
				{
					pbyData[y*cx + x] |= 0x08;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------

void NavMesh_Process_Optimize(navmesh_t* navMesh, bool disconnect)
{
	int n;

	NavMesh_Process_ConnectAll(navMesh);

	if ( DLLTYPE == DLLTYPE_EDITOR )
	{
		for ( n = 0 ; n < MAX_OBJECTS ; n++ )
		{
			int objdef = WO_GetObjectObjdef(n);
			if ( objdef != 0 && WO_GetObjdefVarValue(objdef, "obj_navFloodFillPoint") )
			{
				objectPosition_t objpos;
				navpoly_t* path_poly_floodstart;

				WO_GetObjectPos(n, &objpos);

				path_poly_floodstart = NavMesh_FindPolyContainingPoint(navMesh, objpos.position, -1);
				if ( path_poly_floodstart )
				{
					NavMesh_Process_Flood(navMesh, path_poly_floodstart);
				}
			}
		}
	}
	else
	{
		for ( n = 0 ; n < MAX_REFERENCE_OBJECTS ; n++ )
		{
			if ( reference_objects[n].active )
			{
				int objdef = WO_GetObjdefId(reference_objects[n].refname);
				if ( objdef != -1 && WO_GetObjdefVarValue(objdef, "obj_navFloodFillPoint") )
				{
					navpoly_t* path_poly_floodstart = NavMesh_FindPolyContainingPoint(navMesh, reference_objects[n].pos.position, -1);
					if ( path_poly_floodstart )
					{
						NavMesh_Process_Flood(navMesh, path_poly_floodstart);
					}
				}
			}
		}
	}

	// throw out unflooded polys
	{
		navpoly_t* pathpolys_found[4096];
		int num_pathpolys_found = Quadtree_LambdaFind(navMesh->quadtree, Quadtree_LambdaFind_NavPoly_Unflooded, NULL, (void **)pathpolys_found, 4096);

		int pathpoly;
		for ( pathpoly = 0 ; pathpoly < num_pathpolys_found ; ++pathpoly )
		{
			navpoly_t* path_poly = pathpolys_found[pathpoly];
			if ( path_poly->bridge || path_poly->bridge2 ) continue;
			Quadtree_Remove(navMesh->quadtree, path_poly, path_poly->min, path_poly->max);
			NavPoly_Disconnect(navMesh, path_poly);
			NavPoly_Destroy(path_poly);			
		}
	}

	// disonnect all polies
	if ( disconnect )
	{
		NavMesh_Process_DisconnectAll(navMesh);
	}
}

//----------------------------------------------------------------------------

void NavMesh_Process_Heightmap(navmesh_t* navMesh)
{
	NavMesh_Process_Rasterize(navMesh);
	NavMesh_Process_Vectorize(navMesh);
}

//----------------------------------------------------------------------------

void NavMesh_Process_Finalize(navmesh_t* navMesh)
{
	NavMesh_Process_Cleanup(navMesh);

	if ( DLLTYPE == DLLTYPE_EDITOR )
	{
		NavMesh_Process_ConnectAll(navMesh);
	}
	else
	{
		NavMesh_Process_Optimize(navMesh, false);
	}
}

//----------------------------------------------------------------------------

void NavMesh_Process_Clear(navmesh_t* navMesh)
{
	vec3_t wmin, wmax;
	World_GetBounds(wmin,wmax);

	if ( navMesh->quadtree )
	{
		Quadtree_Lambda(navMesh->quadtree, Quadtree_Lambda_DestroyNavPoly, NULL);
		Quadtree_Destroy(navMesh->quadtree);
	}

	navMesh->quadtree = Quadtree_Create(wmin, wmax, 6);

	{
		int n;
		for ( n = 0 ; n < MAX_OBJECTS ; ++n )
		{
			navpolyref_t* navpolyref = navMesh->navpolyrefs[n].first;
			while ( navpolyref )
			{
				navpolyref_t* navpolyref_next = navpolyref->next;
				DEALLOCATE(navpolyref_t, navpolyref);

				navpolyref = navpolyref_next;
			}

			navMesh->navpolyrefs[n].first = NULL;
			navMesh->navpolyrefs[n].last = NULL;
			navMesh->navpolyrefs[n].obj = NULL;
		}
	}
}

//----------------------------------------------------------------------------

void NavMesh_Startup()
{
	Cvar_Register(&navmesh_process_minslope);
	Cvar_Register(&navmesh_process_stepsize);
	Cvar_Register(&navmesh_process_subdivision);
	Cvar_Register(&navmesh_process_hullthreshhold);
	Cvar_Register(&navmesh_process_bridgeoffset);
}
