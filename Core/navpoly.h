#ifndef NAVPOLY_H
#define NAVPOLY_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

struct navmesh_s;
struct bsp_poly_s;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

EXTERN_ALLOCATOR(navpoly_t);
EXTERN_ALLOCATOR(navpolyedge_t);
EXTERN_ALLOCATOR(navpolycnxn_t);
EXTERN_ALLOCATOR(navpolygate_t);
EXTERN_ALLOCATOR(navpolyref_t);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct navpolygate_s
{
	struct navpolycnxn_s*	cnxn;
	vec2_t					position;
	float					cost;
	float					distance;

	// pathfinding state

	struct navpolygate_s*	from;
	int						frame;

	// list

	struct navpolygate_s*	next;
}
navpolygate_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct navpolycnxn_s
{
	vec2_t					src;
	vec2_t					dest;
	struct navpoly_s*		into;
	navpolygate_t*			gates;

	// list

	struct navpolycnxn_s*	prev;
	struct navpolycnxn_s*	next;
}
navpolycnxn_t;

//----------------------------------------------------------------------------

void NavPolyCnxn_Destroy(navpolycnxn_t* cnxn);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct navpolycnxns_s
{
	navpolycnxn_t*		first;
	navpolycnxn_t*		last;
}
navpolycnxns_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct navpolyedge_s
{
	vec2_t					src;
	vec2_t					dest;
	vec2_t					perp;
	navpolycnxns_t			cnxns;

	// list

	struct navpolyedge_s*	next;
}
navpolyedge_t;

//----------------------------------------------------------------------------

navpolyedge_t* NavPolyEdge_Create(vec2_t src, vec2_t dest);
void NavPolyEdge_Destroy(navpolyedge_t* edge);
float NavPolyEdge_CalculateDistanceToPoint(navpolyedge_t* edge, const vec2_t point);
float NavPolyEdge_CalculateClosestPoint(const navpolyedge_t* edge, const vec2_t point, vec2_t point_closest);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct navpoly_s
{
	navpolyedge_t*		edges;
	vec2_t				min;
	vec2_t				max;
	struct navpoly_s*	fragmentof;
	bool				active;			// $todo: combine bools into flags
	bool				bridge;
	bool				bridge2;
	int					partition;

	// pathfinding state

	int					frame;
}
navpoly_t;

//----------------------------------------------------------------------------

navpoly_t* NavPoly_CreateFromBSPPoly(const struct bsp_poly_s* bsp_poly);
void NavPoly_Destroy(navpoly_t* navpoly);
void NavPoly_Activate(navpoly_t* navpoly);
void NavPoly_Deactivate(navpoly_t* navpoly);
bool NavPoly_IsActive(navpoly_t* navpoly);
void NavPoly_CalculateCentroid(const navpoly_t* navpoly, vec2_t centroid);
bool NavPoly_ContainsPoint(navpoly_t* navpoly, const vec2_t point);
bool NavPoly_IsConnectedTo(navpoly_t* navpoly, navpoly_t* navpoly_other);
void NavPoly_ConnectToPoly(navpoly_t* navpoly, navpoly_t* navpoly_other);
void NavPoly_Connect(struct navmesh_s* navmesh, navpoly_t* navpoly);
void NavPoly_Disconnect(struct navmesh_s* navmesh, navpoly_t* navpoly);
void NavPoly_Partition(struct navmesh_s* navMesh, navpoly_t* navPoly);
void NavPoly_Startup();

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct navpolyref_s
{
	navpoly_t*				navpoly;

	// list

	struct navpolyref_s*	prev;
	struct navpolyref_s*	next;
}
navpolyref_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct navpolyrefs_s
{
	navpolyref_t*			first;
	navpolyref_t*			last;
	const baseObject_t*		obj;
}
navpolyrefs_t;

#endif
