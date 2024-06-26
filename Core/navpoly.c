// (C) 2003 S2 Games

#include "core.h"
#include "allocator.h"
#include "navpoly.h"
#include "navmesh.h"
#include "list.h"
#include "bsp.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

cvar_t	navmesh_process_cnxntesselate = { "navmesh_process_cnxntesselate", "500" };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define BREAK() {}
#define PATH_EPSILON 0.001f

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(navpoly_t);
IMPLEMENT_ALLOCATOR(navpolyjump_t);
IMPLEMENT_ALLOCATOR(navpolyedge_t);
IMPLEMENT_ALLOCATOR(navpolycnxn_t);
IMPLEMENT_ALLOCATOR(navpolygate_t);
IMPLEMENT_ALLOCATOR(navpolyref_t);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void NavPolyCnxn_Destroy(navpolycnxn_t* cnxn)
{
	navpolygate_t* gate = cnxn->gates;
	while ( gate )
	{
		navpolygate_t* gate_next = gate->next;
		DEALLOCATE(navpolygate_t, gate);
		gate = gate_next;
	}

	DEALLOCATE(navpolycnxn_t, cnxn);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

float NavPolyEdge_CalculateClosestPoint(const navpolyedge_t* edge, const vec2_t point, vec2_t point_closest)
{
	return M_ClosestPointToSegment2d(edge->src, edge->dest, point, point_closest);
}

//----------------------------------------------------------------------------

float NavPolyEdge_CalculateDistanceToPoint(navpolyedge_t* edge, const vec2_t point)
{
	float fDistance = M_DotProduct2(edge->perp, edge->src);
	return fDistance - M_DotProduct2(edge->perp, point);
}

//----------------------------------------------------------------------------

navpolyedge_t* NavPolyEdge_Create(vec2_t src, vec2_t dest)
{
	vec2_t dir;
	navpolyedge_t* edge = ALLOCATE(navpolyedge_t);
	
	M_CopyVec2(src, edge->src);
	M_CopyVec2(dest, edge->dest);

	M_SubVec2(edge->dest, edge->src, dir);
	M_NormalizeVec2(dir);
	edge->perp[0] = dir[1];
	edge->perp[1] = -dir[0];
	edge->cnxns.first = NULL;
	edge->cnxns.last = NULL;
	edge->next = NULL;

	return edge;
}

//----------------------------------------------------------------------------

void NavPolyEdge_Destroy(navpolyedge_t* edge)
{
	navpolycnxn_t* cnxn = edge->cnxns.first;
	while ( cnxn )
	{
		navpolycnxn_t* cnxn_next = cnxn->next;
		NavPolyCnxn_Destroy(cnxn);
		cnxn = cnxn_next;
	}

	DEALLOCATE(navpolyedge_t, edge);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

navpoly_t* NavPoly_CreateFromBSPPoly(const bsp_poly_t* bsp_poly)
{
	vec2_t vert_prev;
	bsp_vert_t* bsp_vert = bsp_poly->verts->last;
	navpoly_t* navpoly = ALLOCATE(navpoly_t);
	navpoly->edges = NULL;
	navpoly->min[0] = FAR_AWAY;
	navpoly->min[1] = FAR_AWAY;
	navpoly->max[0] = -FAR_AWAY;
	navpoly->max[1] = -FAR_AWAY;
	navpoly->frame = 0;
	navpoly->partition = -1;
	navpoly->bridge = false;
	navpoly->bridge2 = false;
	navpoly->active = true;

	M_CopyVec2(bsp_poly->verts->first->vert, vert_prev);

	while ( bsp_vert )
	{
		vec2_t dir;
		navpolyedge_t* edge = ALLOCATE(navpolyedge_t);
		
		M_CopyVec2(vert_prev, edge->dest);
		M_CopyVec2(bsp_vert->vert, edge->src);

		M_SubVec2(edge->dest, edge->src, dir);
		M_NormalizeVec2(dir);
		edge->perp[0] = dir[1];
		edge->perp[1] = -dir[0];
		edge->cnxns.first = NULL;
		edge->cnxns.last = NULL;

		edge->next = navpoly->edges;
		navpoly->edges = edge;

		navpoly->min[0] = MIN(bsp_vert->vert[0], navpoly->min[0]);
		navpoly->min[1] = MIN(bsp_vert->vert[1], navpoly->min[1]);
		navpoly->max[0] = MAX(bsp_vert->vert[0], navpoly->max[0]);
		navpoly->max[1] = MAX(bsp_vert->vert[1], navpoly->max[1]);

		M_CopyVec2(bsp_vert->vert, vert_prev);
		bsp_vert = bsp_vert->prev;
	}

	return navpoly;
}

//----------------------------------------------------------------------------

void NavPoly_Destroy(navpoly_t* navpoly)
{
	{
		navpolyedge_t* edge = navpoly->edges;
		while ( edge )
		{
			navpolyedge_t* edge_next = edge->next;
			NavPolyEdge_Destroy(edge);
			edge = edge_next;
		}
	}

	DEALLOCATE(navpoly_t, navpoly);
}

//----------------------------------------------------------------------------

void NavPoly_Activate(navpoly_t* navpoly)
{
	navpoly->active = true;
}

//----------------------------------------------------------------------------

void NavPoly_Deactivate(navpoly_t* navpoly)
{
	navpoly->active = false;
}

//----------------------------------------------------------------------------

bool NavPoly_IsActive(navpoly_t* navpoly)
{
	//ASSERT(navpoly->fragmentof == navpoly); // should never be called on fragments
	return navpoly->active;
}

//----------------------------------------------------------------------------

void NavPoly_CalculateCentroid(const navpoly_t* navpoly, vec2_t centroid)
{
	float verts = 0;
	navpolyedge_t* edge = navpoly->edges;

	M_SetVec2(centroid, 0, 0);

	while ( edge )
	{
		M_AddVec2(centroid, edge->src, centroid);
		verts++;
		edge = edge->next;
	}

	M_MultVec2(centroid, 1.0f/verts, centroid);
}

//----------------------------------------------------------------------------

bool NavPoly_ContainsPoint(navpoly_t* navpoly, const vec2_t point)
{
	if ( point[0] >= navpoly->min[0] && point[0] <= navpoly->max[0] &&
		 point[1] >= navpoly->min[1] && point[1] <= navpoly->max[1] )
	{
		navpolyedge_t* edge = navpoly->edges;
		while ( edge )
		{
			// check against edge from pathvert_prev -> pathvert
			if ( NavPolyEdge_CalculateDistanceToPoint(edge, point) < -PATH_EPSILON ) //10e-4f )
			{
				return false;
			}

			edge = edge->next;
		}

		return true;
	}
	else
	{
		return false;
	}
}

//----------------------------------------------------------------------------

bool NavPoly_IsConnectedTo(navpoly_t* navpoly, navpoly_t* navpoly_other)
{
	navpolyedge_t* navpolyedge = navpoly->edges;
	while ( navpolyedge )
	{
		navpolycnxn_t* navpolycnxn = navpolyedge->cnxns.first;
		while ( navpolycnxn )
		{
			if ( navpolycnxn->into == navpoly_other )
			{
				return true;
			}

			navpolycnxn = navpolycnxn->next;
		}

		navpolyedge = navpolyedge->next;
	}

	return false;
}

//----------------------------------------------------------------------------

void NavPoly_ConnectToPoly(navpoly_t* pathpoly, navpoly_t* pathpoly_other)
{
	// $todo: gotta be a smarter way to do this!
	// make sure we're not already connected, ugh!!!
	if ( pathpoly == pathpoly_other || 
		 NavPoly_IsConnectedTo(pathpoly, pathpoly_other) || 
		 NavPoly_IsConnectedTo(pathpoly_other, pathpoly) )
	{
		return;
	}

	// check for colinear edge spans
	{
		navpolyedge_t* edge = pathpoly->edges;

		while ( edge )
		{
			float ts[2];
			bsp_line_t line;
			float length;

			navpolyedge_t* edge_other = pathpoly_other->edges;

			BSP_Line_Set(&line, edge->src, edge->dest);
			length = BSP_Line_CalculateLength(&line);

			while ( edge_other )
			{
				// $todo: optimize this; we are doing repeated calcs on verts (dest/src)
				if ( BSP_Line_ParameterizePoint(&line, edge_other->src, &ts[0]) && 
					 BSP_Line_ParameterizePoint(&line, edge_other->dest, &ts[1]) )
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
						navpolycnxn_t* cnxn; 
						navpolycnxn_t* cnxn_other;

						vec2_t verts[2];

						ts[0] = MAX(ts[0], 0.0f);
						ts[1] = MIN(ts[1], 1.0f);

						M_MultVec2(line.dir, length*ts[0], verts[0]);
						M_AddVec2(verts[0], line.src, verts[0]);

						M_MultVec2(line.dir, length*ts[1], verts[1]);
						M_AddVec2(verts[1], line.src, verts[1]);

						cnxn = ALLOCATE(navpolycnxn_t);
						cnxn->gates = NULL;
						cnxn->next = NULL;
						cnxn->into = pathpoly_other;
						M_CopyVec2(verts[0], cnxn->src);
						M_CopyVec2(verts[1], cnxn->dest);
						List_PushBack(&edge->cnxns, cnxn);

						cnxn_other = ALLOCATE(navpolycnxn_t);
						cnxn_other->gates = NULL;
						cnxn_other->next = NULL;
						cnxn_other->into = pathpoly;
						M_CopyVec2(verts[0], cnxn_other->src);
						M_CopyVec2(verts[1], cnxn_other->dest);
						List_PushBack(&edge_other->cnxns, cnxn_other);

						{
							float cnxn_length = sqrt(M_GetDistanceSqVec2(verts[0], verts[1]));
//								if ( cnxn_length > thresh )
							{
								int gates = cnxn_length/navmesh_process_cnxntesselate.value + 1;
								float gate_length = cnxn_length/(float)(gates+1);
								int gate = 0;
								navpolygate_t* path_poly_gate;// = ALLOCATE(navpolygate_t); 

								for ( gate = 1 ; gate <= gates ; ++gate )
								{
									vec2_t vert;

									M_MultVec2(line.dir, ((float)gate)*gate_length, vert);
									M_AddVec2(vert, verts[0], vert);

									path_poly_gate = ALLOCATE(navpolygate_t);
									path_poly_gate->frame = 0;
									path_poly_gate->next = NULL;
									path_poly_gate->cnxn = cnxn;//into = pathpoly_other;
									M_CopyVec2(vert, path_poly_gate->position);
									path_poly_gate->next = cnxn->gates;
									cnxn->gates = path_poly_gate;

									path_poly_gate = ALLOCATE(navpolygate_t);
									path_poly_gate->frame = 0;
									path_poly_gate->cnxn = cnxn_other;//into = pathpoly;
									M_CopyVec2(vert, path_poly_gate->position);
									path_poly_gate->next = cnxn_other->gates;
									cnxn_other->gates = path_poly_gate;
								}
							}
						}
					}
				}

				edge_other = edge_other->next;
			}

			edge = edge->next;
		}
	}
}

//----------------------------------------------------------------------------

void NavPoly_Connect(navmesh_t* navmesh, navpoly_t* pathpoly)
{
	float find_width = 10;
	float find_height = 10;
	int num_pathpolys_found = 0;
	navpoly_t* pathpolys_found[4096];
	int i;

	num_pathpolys_found = Quadtree_Find(navmesh->quadtree, (void **)pathpolys_found, 4096, vec2(pathpoly->min[0]-find_width, pathpoly->min[1]-find_height), vec2(pathpoly->max[0]+find_width, pathpoly->max[1]+find_height));

	for ( i = 0 ; i < num_pathpolys_found ; ++i )
	{
		navpoly_t* pathpoly_other = pathpolys_found[i];
		NavPoly_ConnectToPoly(pathpoly, pathpoly_other);
	}	
}

//----------------------------------------------------------------------------

void NavPoly_Disconnect(navmesh_t* navmesh, navpoly_t* pathpoly)
{
	navpolyedge_t* path_poly_edge = pathpoly->edges;
	while ( path_poly_edge )
	{
		navpolycnxn_t* path_poly_cnxn = path_poly_edge->cnxns.first;
		
		while ( path_poly_cnxn )
		{
			navpolycnxn_t* path_poly_cnxn_next = path_poly_cnxn->next;
			navpoly_t* pathpoly_other = path_poly_cnxn->into;

			navpolyedge_t* path_poly_edge_other = pathpoly_other->edges;

			while ( path_poly_edge_other )
			{
				navpolycnxn_t* path_poly_cnxn_other = path_poly_edge_other->cnxns.first;

				while ( path_poly_cnxn_other )
				{
					navpolycnxn_t* path_poly_cnxn_other_next = path_poly_cnxn_other->next;

					if ( path_poly_cnxn_other->into == pathpoly )
					{
						List_Remove(&path_poly_edge_other->cnxns, path_poly_cnxn_other);
						NavPolyCnxn_Destroy(path_poly_cnxn_other);

						break;
					}

					path_poly_cnxn_other = path_poly_cnxn_other_next;
				}

				path_poly_edge_other = path_poly_edge_other->next;
			}

			List_Remove(&path_poly_edge->cnxns, path_poly_cnxn);
			NavPolyCnxn_Destroy(path_poly_cnxn);

			path_poly_cnxn = path_poly_cnxn_next;
		}

		path_poly_edge->cnxns.first = NULL;
		path_poly_edge->cnxns.last = NULL;

		path_poly_edge = path_poly_edge->next;
	}
}

//----------------------------------------------------------------------------

void NavPoly_Startup()
{
	Cvar_Register(&navmesh_process_cnxntesselate);
}
