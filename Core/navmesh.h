#ifndef NAVMESH_H
#define NAVMESH_H

#include "quadtree.h"
#include "navpoly.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

struct navpoly_s;
struct navpolyedge_s;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct navmesh_s
{
	float					offset;
	int						frame_poly;
	int						frame_cnxn;
	int						partition;
	int						partitionNext;
	quadtree_t*				quadtree;
	navpolyrefs_t			navpolyrefs[MAX_OBJECTS];
	navpoly_t*				bridges[MAX_WORLDOBJECTS-MAX_OBJECTS];
	int						timeNextPartition;
}
navmesh_t;

//----------------------------------------------------------------------------

void NavMesh_Init(navmesh_t* navMesh, float offset);
void NavMesh_Term(navmesh_t* navMesh);

void NavMesh_CSGSubtractStatic(navmesh_t* navMesh, int n);
void NavMesh_CSGSubtract(navmesh_t* navMesh, const baseObject_t* obj);
void NavMesh_CSGAdd(navmesh_t* navMesh, const baseObject_t* obj);

bool NavMesh_CanPlaceBuilding(navmesh_t* navMesh, const vec3_t bminw, const vec3_t bmaxw);

bool NavMesh_FindNearestPointAroundObject(navmesh_t* navMesh, const baseObject_t* obj, const vec2_t point, int partition, struct navpoly_s** navpoly_nearest, vec2_t pointNearest);

bool NavMesh_Trace(navmesh_t* navMesh, const vec2_t src, const vec2_t dest);

void NavMesh_Partition(navmesh_t* navMesh);

struct navpoly_s* NavMesh_FindPolyContainingPoint(navmesh_t* navMesh, const vec2_t point, int partition);
void NavMesh_FindPolyNearestPoint(navmesh_t* navMesh, const vec2_t point, int partition, struct navpoly_s** navPolyNearest, vec2_t pointNearest);

void NavMesh_Process_Bridges(navmesh_t* navMesh);
void NavMesh_Process_Shrink(navmesh_t* navMesh);
void NavMesh_Process_Cleanup(navmesh_t* navMesh);
void NavMesh_Process_Optimize(navmesh_t* navMesh, bool disconnect);
void NavMesh_Process_Heightmap(navmesh_t* navMesh);
void NavMesh_Process_Finalize(navmesh_t* navMesh);
void NavMesh_Process_Clear(navmesh_t* navMesh);

void NavMesh_Startup();

#endif
