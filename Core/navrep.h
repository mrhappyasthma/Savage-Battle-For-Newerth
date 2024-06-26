#ifndef NAVREP_H
#define NAVREP_H

#include "navmesh.h"

//----------------------------------------------------------------------------

extern const float navmeshsizes[num_navmeshsizes];

//----------------------------------------------------------------------------

typedef struct navrep_s
{
	navmesh_t	navmeshes[num_navmeshsizes];
}
navrep_t;

//----------------------------------------------------------------------------

void NavRep_Startup();

void NavRep_Init();
void NavRep_Term();

void NavRep_Update();

void NavRep_Render();

void NavRep_CSGSubtract(const baseObject_t* obj);
void NavRep_CSGAdd(const baseObject_t* obj);
bool NavRep_CanPlaceBuilding(const vec3_t bminw, const vec3_t bmaxw);

bool NavRep_Trace(navmeshsize_t navmeshsize, const vec3_t src, const vec3_t dest);

navpath_t* NavRep_PathCreateToPosition(navmeshsize_t navmeshsize, const vec3_t src, navpoly_t* navPolySrc, const vec3_t dest);
navpath_t* NavRep_PathCreateToObject(navmeshsize_t navmeshsize, const vec3_t src, navpoly_t* navPolySrc, const baseObject_t* obj, bool close);
void NavRep_PathDestroy(navmeshsize_t navmeshsize, navpath_t* navpath);
bool NavRep_PathOptimize(navmeshsize_t navmeshsize, navpath_t* navpath);

#endif
