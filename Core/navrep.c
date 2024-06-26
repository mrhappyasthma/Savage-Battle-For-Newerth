// (C) 2003 S2 Games

#include "core.h"
#include "list.h"
#include "heap.h"
#include "set.h"
#include "allocator.h"
#include "navrep.h"
#include "bsp.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define NAV_PRINTF if (navrep_debug.integer) Console_DPrintf
#define NAVMESH_PARTITION_INTERVAL 1000

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(navpath_t);
IMPLEMENT_ALLOCATOR(navpathwaypt_t);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

cvar_t	navrep_render = { "navrep_render", "-1" };
cvar_t	navrep_rendertess = { "navrep_rendertess", "1" };
cvar_t	navrep_debug = { "navrep_debug", "0" };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static navrep_t navrep;
//static navpaths_t navpaths = { 0 };

//----------------------------------------------------------------------------

const float navmeshsizes[num_navmeshsizes] = { 20.0f }; //, 30.0f };

//----------------------------------------------------------------------------

navpath_t* path_last = NULL;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define PATH_EPSILON 0.001f
#define path_sqrt(x) sqrt(x)

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bool Heap_Compare_PathPolyCnxn(void* pv0, void* pv1)
{
	return ((navpolygate_t*)pv0)->cost < ((navpolygate_t*)pv1)->cost;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

navpathwaypt_t* NavPathWaypt_Create(const vec2_t position)
{
	navpathwaypt_t* waypt = ALLOCATE(navpathwaypt_t);
	waypt->bridge = NULL;
	M_CopyVec2(position, waypt->position);
	return waypt;
}
//----------------------------------------------------------------------------

void NavPathWaypt_Destroy(navpathwaypt_t* waypt)
{
	DEALLOCATE(navpathwaypt_t, waypt);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void NavRep_Init()
{
	int navmesh;
	int timeNextRepartition = NAVMESH_PARTITION_INTERVAL;

	for ( navmesh = 0 ; navmesh < num_navmeshsizes ; ++navmesh )
	{
		NavMesh_Init(&navrep.navmeshes[navmesh], navmeshsizes[navmesh]);

		// stagger the partitions to run 1 second apart 
		navrep.navmeshes[navmesh].timeNextPartition = Host_Milliseconds() + timeNextRepartition;
		timeNextRepartition += NAVMESH_PARTITION_INTERVAL;
	}
}

//----------------------------------------------------------------------------

void NavRep_Term()
{
	int navmesh;
	for ( navmesh = 0 ; navmesh < num_navmeshsizes ; ++navmesh )
	{
		NavMesh_Term(&navrep.navmeshes[navmesh]);
	}
}
//----------------------------------------------------------------------------

void NavRep_Update()
{
	int navmesh;
	for ( navmesh = 0 ; navmesh < num_navmeshsizes ; ++navmesh )
	{
		if ( Host_Milliseconds() > navrep.navmeshes[navmesh].timeNextPartition )
		{
			NavMesh_Partition(&navrep.navmeshes[navmesh]);

			// partition again in n seconds where n = number of navmeshes
			navrep.navmeshes[navmesh].timeNextPartition = Host_Milliseconds() + NAVMESH_PARTITION_INTERVAL*num_navmeshsizes;
		}
	}
}
//----------------------------------------------------------------------------

void NavRep_CSGSubtract(const baseObject_t* obj)
{
	int navmesh;
	for ( navmesh = 0 ; navmesh < num_navmeshsizes ; ++navmesh )
	{
		NavMesh_CSGSubtract(&navrep.navmeshes[navmesh], obj);
	}
}

//----------------------------------------------------------------------------

void NavRep_CSGAdd(const baseObject_t* obj)
{
	int navmesh;
	for ( navmesh = 0 ; navmesh < num_navmeshsizes ; ++navmesh )
	{
		NavMesh_CSGAdd(&navrep.navmeshes[navmesh], obj);
	}
}

//----------------------------------------------------------------------------

bool NavRep_CanPlaceBuilding(const vec3_t bminw, const vec3_t bmaxw)
{
	int navmesh;
	for ( navmesh = 0 ; navmesh < num_navmeshsizes ; ++navmesh )
	{
		if ( !NavMesh_CanPlaceBuilding(&navrep.navmeshes[navmesh], bminw, bmaxw) )
		{
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------------

bool NavRep_Trace(navmeshsize_t navmeshsize, const vec2_t src, const vec2_t dest)
{
	navmesh_t* navmesh = &navrep.navmeshes[navmeshsize];
	return NavMesh_Trace(navmesh, src, dest);
}

//----------------------------------------------------------------------------

navpath_t* NavRep_PathFinalize(navmesh_t* navmesh, const vec2_t src, const vec2_t dest, navpolygate_t* gate)
{
	navpath_t* navpath;
	navpathwaypt_t* waypoint;

	navpath = ALLOCATE(navpath_t);
	navpath->waypoints.first = NULL;
	navpath->waypoints.last = NULL;

	M_CopyVec2(dest, navpath->pos);
	M_CopyVec2(dest, navpath->src);
	M_CopyVec2(src, navpath->dest);

	while ( gate )
	{
		waypoint = NavPathWaypt_Create(gate->position);
		if ( gate->cnxn->into->bridge2 )
		{
			waypoint->bridge = gate->cnxn->into;
		}
//		M_CopyVec2(gate->position, waypoint->position);
		List_PushBack(&navpath->waypoints, waypoint);
		gate = gate->from;
	}

	waypoint = NavPathWaypt_Create(src);
	List_PushBack(&navpath->waypoints, waypoint);

	navpath->waypoint = navpath->waypoints.first;

	return navpath;
}

//----------------------------------------------------------------------------

navpath_t* NavRep_PathCreate_Internal(navmesh_t* navmesh, const vec2_t src, navpoly_t* pathpoly_src, const vec2_t dest, navpoly_t* pathpoly_dest)
{
	int ms = System_Milliseconds();

	// increment the frame code (frame code is used in place of open/closed list)

	int frame = ++navmesh->frame_cnxn;

	if ( pathpoly_src == pathpoly_dest )
	{
		// if we're in the same pathpoly, just go there

		navpathwaypt_t* waypoint;

		path_last = ALLOCATE(navpath_t);
		path_last->waypoints.first = NULL;
		path_last->waypoints.last = NULL;
		M_CopyVec2(dest, path_last->pos);
		M_CopyVec2(dest, path_last->src);
		M_CopyVec2(src, path_last->dest);

		waypoint = NavPathWaypt_Create(src);
		waypoint->bridge = pathpoly_src->bridge2 ? pathpoly_src : NULL;
		List_PushFront(&path_last->waypoints, waypoint);

		path_last->waypoint = path_last->waypoints.first;
		
//		List_PushFront(&navpaths, path_last);

		return path_last;
	}

	if ( !pathpoly_dest->bridge2 && NavMesh_Trace(navmesh, src, dest) )
	{
		// if we can trace, just go there

		navpathwaypt_t* waypoint;

		path_last = ALLOCATE(navpath_t);
		path_last->waypoints.first = NULL;
		path_last->waypoints.last = NULL;
		M_CopyVec2(dest, path_last->pos);
		M_CopyVec2(dest, path_last->src);
		M_CopyVec2(src, path_last->dest);

		waypoint = NavPathWaypt_Create(src);
		List_PushFront(&path_last->waypoints, waypoint);

		path_last->waypoint = path_last->waypoints.first;

//		List_PushFront(&navpaths, path_last);

		return path_last;
	}

	{
		// create our heap

		heap_t heap;
		Heap_Init(&heap, 16384, Heap_Compare_PathPolyCnxn);

		// push all initial cnxns onto heap
		{
			navpolyedge_t* edge = pathpoly_src->edges;
			while ( edge )
			{
				navpolycnxn_t* cnxn = edge->cnxns.first;
				while ( cnxn )
				{
					navpolygate_t* gate = cnxn->gates;
					while ( gate )
					{
						// initialize costs

						gate->distance = path_sqrt(M_GetDistanceSqVec2(src, gate->position));
						gate->cost = gate->distance + path_sqrt(M_GetDistanceSqVec2(dest, gate->position));
						gate->from = NULL;
						gate->frame = frame;

						// add to heap

						Heap_Push(&heap, gate);

						gate = gate->next;
					}
				
					cnxn = cnxn->next;
				}

				edge = edge->next;
			}
		}

		while ( !Heap_IsEmpty(&heap) )
		{
			// extract min cost gate

			navpolygate_t* gate = Heap_Pop(&heap);
			if ( gate->cnxn->into == pathpoly_dest ) 
			{
				if ( fabs(gate->distance - gate->cost) < PATH_EPSILON )
				{
					// found a path, finalize it

					path_last = NavRep_PathFinalize(navmesh, src, dest, gate);

					NAV_PRINTF("NavRep_PathCreate succeeded, took %d milliseconds\n", System_Milliseconds() - ms);

					// term the heap

					Heap_Term(&heap);

//					List_PushFront(&navpaths, path_last);

					return path_last;
				}
				else
				{
					gate->distance += path_sqrt(M_GetDistanceSqVec2(gate->position, dest));
					gate->cost = gate->distance;

					Heap_Update(&heap, gate);
					
					continue;
				}
			}

			// relax all adjacent gates
			{
				navpoly_t* pathpoly = gate->cnxn->into;
				navpolyedge_t* pathpoly_edge = pathpoly->edges;
				while ( pathpoly_edge )
				{
					navpolycnxn_t* pathpoly_cnxn = pathpoly_edge->cnxns.first;
					while ( pathpoly_cnxn )
					{
						navpolygate_t* pathpoly_gate = pathpoly_cnxn->gates;
						while ( pathpoly_gate )
						{
							float distance = 1.0f + gate->distance + path_sqrt(M_GetDistanceSqVec2(pathpoly_gate->position, gate->position));
							float cost = distance + path_sqrt(M_GetDistanceSqVec2(pathpoly_gate->position, dest));

							if ( pathpoly_gate->frame != frame )
							{
								pathpoly_gate->cost = cost;
								pathpoly_gate->distance = distance;
								pathpoly_gate->frame = frame;
								pathpoly_gate->from = gate;

								Heap_Push(&heap, pathpoly_gate);
							}
							// $todo: why is it okay (faster + just as accurate) to compare distance here rather than cost????
							else if ( (pathpoly_gate->distance > distance) )
							{
								pathpoly_gate->cost = cost;
								pathpoly_gate->distance = distance;
								pathpoly_gate->from = gate;

								Heap_Update(&heap, pathpoly_gate);
							}

							pathpoly_gate = pathpoly_gate->next;
						}

						pathpoly_cnxn = pathpoly_cnxn->next;
					}

					pathpoly_edge = pathpoly_edge->next;
				}
			}
		}

		NAV_PRINTF("NavRep_PathCreate failed, took %d milliseconds\n", System_Milliseconds() - ms);

		// term the heap

		Heap_Term(&heap);
	}

	return NULL;
}

//----------------------------------------------------------------------------

navpath_t* NavRep_PathCreateToPosition(navmeshsize_t navmeshsize, const vec3_t src, navpoly_t* navPolySrc, const vec3_t dest)
{
	navmesh_t* navmesh = &navrep.navmeshes[navmeshsize];

	navpoly_t* pathpoly_src = NULL;
	navpoly_t* pathpoly_dest = NULL;
	vec2_t point_src;
	vec2_t point_dest;

	if ( navPolySrc )
	{
		pathpoly_src = navPolySrc;
		M_CopyVec2(src, point_src);
	}
	else
	{
		NavMesh_FindPolyNearestPoint(navmesh, src, -1, &pathpoly_src, point_src);
		if ( !pathpoly_src ) 
		{
			NAV_PRINTF("NavRep_PathCreateToPosition could not find nearest src poly");
			return NULL;
		}
	}
	
	// find nearest dest poly

	if ( pathpoly_src->partition == -1 )
	{
		int ms = System_Milliseconds();

		navmesh->partition = ++navmesh->partitionNext;
		NavPoly_Partition(navmesh, pathpoly_src);

		NAV_PRINTF("NavPoly_Partition took %d milliseconds\n", System_Milliseconds() - ms);
	}

	NavMesh_FindPolyNearestPoint(navmesh, dest, pathpoly_src->partition, &pathpoly_dest, point_dest);
	if ( !pathpoly_dest )
	{
		NAV_PRINTF("NavRep_PathCreateToPosition could not find nearest dest poly");
		return NULL;
	}

	return NavRep_PathCreate_Internal(navmesh, point_dest, pathpoly_dest, point_src, pathpoly_src);
}

//----------------------------------------------------------------------------

navpath_t* NavRep_PathCreateToObject(navmeshsize_t navmeshsize, const vec3_t src, navpoly_t* navPolySrc, const baseObject_t* obj, bool close)
{
	navmesh_t* navmesh = &navrep.navmeshes[navmeshsize];

	navpath_t* navpath; 

	navpoly_t* pathpoly_src = NULL;
	navpoly_t* pathpoly_dest = NULL;
	vec2_t point_src;
	vec2_t point_dest;
	bool approx = false;
	
	// find nearest src poly

	if ( navPolySrc )
	{
		pathpoly_src = navPolySrc;
		M_CopyVec2(src, point_src);
	}
	else
	{
		NavMesh_FindPolyNearestPoint(navmesh, src, -1, &pathpoly_src, point_src);
		if ( !pathpoly_src )
		{
			NAV_PRINTF("NavRep_PathCreateToObject could not find nearest src poly");
			return NULL;
		}
	}
	
	// find nearest dest poly

	if ( pathpoly_src->partition == -1 )
	{
		int ms = System_Milliseconds();

		navmesh->partition = ++navmesh->partitionNext;
		NavPoly_Partition(navmesh, pathpoly_src);

		NAV_PRINTF("NavPoly_Partition took %d milliseconds\n", System_Milliseconds() - ms);
	}

	NavMesh_FindNearestPointAroundObject(navmesh, obj, src, pathpoly_src->partition, &pathpoly_dest, point_dest);
	if ( !pathpoly_dest )
	{
		approx = true;

		NavMesh_FindPolyNearestPoint(navmesh, obj->pos, pathpoly_src->partition, &pathpoly_dest, point_dest);

		if ( !pathpoly_dest )
		{
			NAV_PRINTF("NavRep_PathCreateToObject could not find nearest dest poly");
			return NULL;
		}
	}

	// find the path
	
	navpath = NavRep_PathCreate_Internal(navmesh, point_dest, pathpoly_dest, point_src, pathpoly_src);
	if ( navpath )
	{
		if ( close )
		{
			navpathwaypt_t* waypoint;
			waypoint = NavPathWaypt_Create(obj->pos);
			List_PushBack(&navpath->waypoints, waypoint);
		}

		navpath->obj = obj;		
		navpath->approx = approx;
	}

	return navpath;
}

//----------------------------------------------------------------------------

void NavRep_PathDestroy(navmeshsize_t navmeshsize, navpath_t* navpath)
{
	navmesh_t* navmesh = &navrep.navmeshes[navmeshsize];

	navpathwaypt_t* waypoint = navpath->waypoints.first;
	while ( waypoint )
	{
		navpathwaypt_t* waypoint_next = waypoint->next;
		NavPathWaypt_Destroy(waypoint);
		waypoint = waypoint_next;
	}

//	List_Remove(&navpaths, navpath);

	DEALLOCATE(navpath_t, navpath);	

	if (path_last == navpath)
	{
		path_last = NULL;
	}
}

//----------------------------------------------------------------------------

bool NavRep_PathOptimize(navmeshsize_t navmeshsize, navpath_t* navpath)
{
	navmesh_t* navmesh = &navrep.navmeshes[navmeshsize];

	navpathwaypt_t* waypoint = navpath->waypoint;
	bool bOptimized = false;

	if ( !waypoint ) return bOptimized;

	while ( waypoint->next )
	{
		navpathwaypt_t* waypoint_next = waypoint->next;

		if ( waypoint->bridge )
		{
			break;
		}
		
		if ( NavMesh_Trace(navmesh, navpath->pos, waypoint_next->position) )
		{
			bOptimized = true;

			navpath->waypoint = waypoint_next;
			List_Remove(&navpath->waypoints, waypoint);
			NavPathWaypt_Destroy(waypoint);
		}
		else 
		{
			vec2_t pos_trace;
			M_ClosestPointToSegment2d(waypoint->position, waypoint_next->position, navpath->pos, pos_trace);

			if ( NavMesh_Trace(navmesh, navpath->pos, pos_trace) )
			{
				bOptimized = true;

				M_CopyVec2(pos_trace, navpath->waypoint->position);
			}

			break;
		}

		waypoint = waypoint_next;
	}

	return bOptimized;
}

//----------------------------------------------------------------------------

void NavRep_Process(int argc, char *argv[])
{
	int navmesh;

	NavRep_Term();
	NavRep_Init();

	for ( navmesh = 0 ; navmesh < num_navmeshsizes ; ++navmesh )
	{
		NavMesh_Process_Clear(&navrep.navmeshes[navmesh]);
		NavMesh_Process_Heightmap(&navrep.navmeshes[navmesh]);
		NavMesh_Process_Optimize(&navrep.navmeshes[navmesh], true);
		NavMesh_Process_Shrink(&navrep.navmeshes[navmesh]);

		if ( DLLTYPE == DLLTYPE_EDITOR )
		{
			int n;
			for ( n = 0 ; n < MAX_OBJECTS ; n++ )
			{
				if ( WO_IsObjectActive(n) )
				{
					NavMesh_CSGSubtractStatic(&navrep.navmeshes[navmesh], n);
				}
			}
		}
		else
		{
			int n;
			for ( n = MAX_OBJECTS ; n < MAX_WORLDOBJECTS ; n++ )
			{
				if ( WO_IsObjectActive(n) )
				{
					NavMesh_CSGSubtractStatic(&navrep.navmeshes[navmesh], n);
				}
			}
		}

		NavMesh_Process_Bridges(&navrep.navmeshes[navmesh]);
		NavMesh_Process_Finalize(&navrep.navmeshes[navmesh]);
	}
}

//----------------------------------------------------------------------------

void NavRep_RenderLine(vec2_t src, vec2_t dest, float height)
{
#ifndef _S2_DONT_INCLUDE_GL
	pointinfo_t pi;
	vec2_t dir;
	float x, y, mag, t;
	M_SubVec2(dest, src, dir);
	mag = M_NormalizeVec2(dir);

	for ( t = 0 ; t < navrep_rendertess.value ; ++t )
	{
		x = src[0] + dir[0]*mag*t/(float)navrep_rendertess.value;
		y = src[1] + dir[1]*mag*t/(float)navrep_rendertess.value;
		World_SampleGround(x, y, &pi);
		glVertex3f(x, y, pi.z+height);
		x = src[0] + dir[0]*mag*(t+1)/(float)navrep_rendertess.value;
		y = src[1] + dir[1]*mag*(t+1)/(float)navrep_rendertess.value;
		World_SampleGround(x, y, &pi);
		glVertex3f(x, y, pi.z+height);
	}
#endif
}

//----------------------------------------------------------------------------

bool Quadtree_Lambda_NavRep_Render(void* data0, void* data1)
{
#ifndef _S2_DONT_INCLUDE_GL
	navpoly_t* pathpoly = (navpoly_t*)data0;

	if ( pathpoly->bridge || pathpoly->bridge2 )
		glColor3f(1,0,0);
	else
		glColor3f(1,1,1);
	{
		navpolyedge_t* edge = pathpoly->edges;
		while ( edge )
		{
			NavRep_RenderLine(edge->src, edge->dest, 0);

			edge = edge->next;
		}
	}

	glColor3f(1,1,0);
	{
		navpolyedge_t* path_poly_edge;
		navpolycnxn_t* path_poly_cnxn;
		navpolygate_t* path_poly_gate;

		vec2_t centroid;
		NavPoly_CalculateCentroid(pathpoly, centroid);

		path_poly_edge = pathpoly->edges;
		while ( path_poly_edge )
		{
			path_poly_cnxn = path_poly_edge->cnxns.first;
			while ( path_poly_cnxn )
			{
				path_poly_gate = path_poly_cnxn->gates;
				while ( path_poly_gate )
				{
					NavRep_RenderLine(centroid, path_poly_gate->position, 0);

					path_poly_gate = path_poly_gate->next;
				}

				path_poly_cnxn = path_poly_cnxn->next;
			}

			path_poly_edge = path_poly_edge->next;
		}
	}
#endif

	return true;
}

//----------------------------------------------------------------------------

void NavRep_Render()
{
#ifndef _S2_DONT_INCLUDE_GL
	navmesh_t* navmesh;
	int n;

	if ( navrep_render.integer < 0 || navrep_render.integer >= num_navmeshsizes ) return;

	navmesh = &navrep.navmeshes[navrep_render.integer];
	Quadtree_Lambda(navmesh->quadtree, Quadtree_Lambda_NavRep_Render, NULL);

	for ( n = 0 ; n < MAX_WORLDOBJECTS-MAX_OBJECTS ; ++n )
	{
		if ( navmesh->bridges[n] )
			Quadtree_Lambda_NavRep_Render(navmesh->bridges[n], NULL);
	}

/*	glColor3f(0,1,0);
	{
		navpath_t* navpath = navpaths.first;
		while ( navpath )
		{
			navpathwaypt_t* navpathwaypt = navpath->waypoints.first;
			if ( navpathwaypt )
			{
				NavRep_RenderLine(navpathwaypt->position, navpath->src, 10.0f);
			}
			while ( navpathwaypt )
			{
				navpathwaypt_t* navpathwayptNext = navpathwaypt->next;
				if ( navpathwayptNext )
				{
					NavRep_RenderLine(navpathwaypt->position, navpathwayptNext->position, 10.0f);
				}
				navpathwaypt = navpathwayptNext;
			}
			navpath = navpath->next;
		}
	}
*/
#endif
}

//----------------------------------------------------------------------------

void NavRep_Allocs(int argc, char *argv[])
{
	Console_DPrintf("bsp_vert_t.ref = %d\n", allocator_bsp_vert_t.ref);
	Console_DPrintf("bsp_verts_t.ref = %d\n", allocator_bsp_verts_t.ref);
	Console_DPrintf("bsp_line_t.ref = %d\n", allocator_bsp_line_t.ref);
	Console_DPrintf("bsp_lines_t.ref = %d\n", allocator_bsp_lines_t.ref);
	Console_DPrintf("bsp_poly_t.ref = %d\n", allocator_bsp_poly_t.ref);
	Console_DPrintf("bsp_polys_t.ref = %d\n", allocator_bsp_polys_t.ref);
	Console_DPrintf("bsp_node_t.ref = %d\n", allocator_bsp_node_t.ref);
	Console_DPrintf("bsp_t.ref = %d\n", allocator_bsp_t.ref);
	Console_DPrintf("navpoly_t.ref = %d\n", allocator_navpoly_t.ref);
	Console_DPrintf("navpolyedge_t.ref = %d\n", allocator_navpolyedge_t.ref);
	Console_DPrintf("navpolycnxn_t.ref = %d\n", allocator_navpolycnxn_t.ref);
	Console_DPrintf("navpolygate_t.ref = %d\n", allocator_navpolygate_t.ref);
	Console_DPrintf("navpolyref_t.ref = %d\n", allocator_navpolyref_t.ref);
	Console_DPrintf("navpath_t.ref = %d\n", allocator_navpath_t.ref);
	Console_DPrintf("navpathwaypt_t.ref = %d\n", allocator_navpathwaypt_t.ref);
	Console_DPrintf("quadtree_node_element_t.ref = %d\n", allocator_quadtree_node_element_t.ref);
	Console_DPrintf("quadtree_node_t.ref = %d\n", allocator_quadtree_node_t.ref);
	Console_DPrintf("quadtree_t.ref = %d\n", allocator_quadtree_t.ref);
//	Console_DPrintf("navpathpolys.frame_poly = %d\n", navpathpolys.frame_poly);
//	Console_DPrintf("navpathpolys.framecnxn = %d\n", navpathpolys.framecnxn);
}

//----------------------------------------------------------------------------

void NavRep_Startup()
{
	Cmd_Register("navrep_process", NavRep_Process);
	Cmd_Register("navrep_allocs", NavRep_Allocs);
	
	Cvar_Register(&navrep_render);
	Cvar_Register(&navrep_rendertess);
	Cvar_Register(&navrep_debug);

	BSP_Startup();
	NavMesh_Startup();
	NavPoly_Startup();

	NavRep_Init();
}
