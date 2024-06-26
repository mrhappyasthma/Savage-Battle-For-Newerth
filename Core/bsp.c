#include "core.h"
#include "allocator.h"
#include "navpoly.h"
#include "bsp.h"
#include "list.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define NAVBREAK(x) {};
cvar_t	navrep_bsp_epsilon = { "navrep_bsp_epsilon", ".003" };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(bsp_vert_t);
IMPLEMENT_ALLOCATOR(bsp_verts_t);
IMPLEMENT_ALLOCATOR(bsp_line_t);
IMPLEMENT_ALLOCATOR(bsp_lines_t);
IMPLEMENT_ALLOCATOR(bsp_poly_t);
IMPLEMENT_ALLOCATOR(bsp_polys_t);
IMPLEMENT_ALLOCATOR(bsp_node_t);
IMPLEMENT_ALLOCATOR(bsp_t);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bsp_vert_t* BSP_Vert_Create()
{
	bsp_vert_t* vert = ALLOCATE(bsp_vert_t);
	return vert;
}

//----------------------------------------------------------------------------

void BSP_Vert_Destroy(bsp_vert_t* vert)
{
	DEALLOCATE(bsp_vert_t, vert);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bsp_verts_t* BSP_Verts_Create()
{
	bsp_verts_t* verts = ALLOCATE(bsp_verts_t);
	verts->first = NULL;
	verts->last = NULL;
	return verts;
}

//----------------------------------------------------------------------------

void BSP_Verts_Destroy(bsp_verts_t* verts)
{
	BSP_Verts_Clear(verts);
	DEALLOCATE(bsp_verts_t, verts);
}

//----------------------------------------------------------------------------

void BSP_Verts_Clear(bsp_verts_t* verts)
{
	bsp_vert_t* vert = verts->first;

	while ( vert )
	{
		bsp_vert_t* vertnext = vert->next;
		BSP_Vert_Destroy(vert);
		vert = vertnext;
	}

	verts->first = NULL;
	verts->last = NULL;
}

//----------------------------------------------------------------------------

void BSP_Verts_PushFront(bsp_verts_t* verts, const bsp_vert_t* vert)
{
	bsp_vert_t* vertnew = BSP_Vert_Create();
	
	M_CopyVec2(vert->vert, vertnew->vert);

	List_PushFront(verts, vertnew);
}

//----------------------------------------------------------------------------

void BSP_Verts_PushBack(bsp_verts_t* verts, const bsp_vert_t* vert)
{
	bsp_vert_t* vertnew = BSP_Vert_Create();
	
	M_CopyVec2(vert->vert, vertnew->vert);

	List_PushBack(verts, vertnew);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bsp_poly_t* BSP_Poly_Create(bool bCreateVerts)
{
	bsp_poly_t* poly = ALLOCATE(bsp_poly_t);
	if ( bCreateVerts )
	{
		poly->verts = BSP_Verts_Create();
		poly->verts->first = NULL;
		poly->verts->last = NULL;
		poly->verts->ref = 1;
	}
	else
	{
		poly->verts = NULL;
	}
	return poly;
}

//----------------------------------------------------------------------------

bsp_poly_t* BSP_Poly_CreateFromWorld()
{
	vec3_t wmin, wmax;
	bsp_vert_t vert;
	bsp_poly_t* poly = BSP_Poly_Create(true);

	World_GetBounds(wmin,wmax);

	M_SetVec2(vert.vert, 0, 0);
	BSP_Verts_PushBack(poly->verts, &vert);
	M_SetVec2(vert.vert, wmax[0], 0);
	BSP_Verts_PushBack(poly->verts, &vert);
	M_SetVec2(vert.vert, wmax[0], wmax[1]);
	BSP_Verts_PushBack(poly->verts, &vert);
	M_SetVec2(vert.vert, 0, wmax[1]);
	BSP_Verts_PushBack(poly->verts, &vert);

	return poly;
}

//----------------------------------------------------------------------------

bsp_poly_t* BSP_Poly_CreateFromPathPoly(const navpoly_t* navpoly)
{
	bsp_poly_t* poly = BSP_Poly_Create(true);

	navpolyedge_t* edge = navpoly->edges;
	while ( edge )
	{
		bsp_vert_t vert;
		
		M_CopyVec2(edge->src, vert.vert);
		BSP_Verts_PushBack(poly->verts, &vert);

		edge = edge->next;
	}

	return poly;
}

//----------------------------------------------------------------------------

void BSP_Poly_Destroy(bsp_poly_t* poly)
{
	poly->verts->ref--;

	if ( 0 == poly->verts->ref )
	{
		// free up verts
		BSP_Verts_Destroy(poly->verts);
	}

	DEALLOCATE(bsp_poly_t, poly);
}

//----------------------------------------------------------------------------
// from http://astronomy.swin.edu.au/~pbourke/geometry/polyarea/

float BSP_Poly_CalculateArea(bsp_poly_t* bspPoly)
{
	float area = 0;

	vec2_t verts[128];
	int numVerts = 0;
	int vert;

	bsp_vert_t* bspVert = bspPoly->verts->first;

	while ( bspVert )
	{
		M_CopyVec2(bspVert->vert, verts[numVerts++]);
		bspVert = bspVert->next;
	}

	M_CopyVec2(bspPoly->verts->first->vert, verts[numVerts]);

	for ( vert = 0 ; vert < numVerts ; ++vert )
	{
		area += (verts[vert][0]*verts[vert+1][1] - verts[vert+1][0]*verts[vert][1]);
	}

	area /= 2.0f;

	return area;
}

//----------------------------------------------------------------------------

bsp_verts_t* BSP_Poly_CloneVerts(const bsp_poly_t* poly)
{
	((bsp_poly_t*)poly)->verts->ref++;
	return poly->verts;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bsp_polys_t* BSP_Polys_Create()
{
	bsp_polys_t* polys = ALLOCATE(bsp_polys_t);
	polys->first = NULL;
	polys->last = NULL;
	return polys;
}

//----------------------------------------------------------------------------

void BSP_Polys_Destroy(bsp_polys_t* polys)
{
	BSP_Polys_Clear(polys);
	DEALLOCATE(bsp_polys_t, polys);
}

//----------------------------------------------------------------------------

void BSP_Polys_Clear(bsp_polys_t* polys)
{
	bsp_poly_t* poly = polys->first;

	while ( poly )
	{
		bsp_poly_t* polynext = poly->next;
		BSP_Poly_Destroy(poly);
		poly = polynext;
	}

	polys->first = NULL;
	polys->last = NULL;
}

//----------------------------------------------------------------------------

void BSP_Polys_PushFront(bsp_polys_t* polys, const bsp_poly_t* poly)
{
	bsp_poly_t* polynew = BSP_Poly_Create(false);
	
	polynew->verts = BSP_Poly_CloneVerts(poly);

	List_PushFront(polys, polynew);
}

//----------------------------------------------------------------------------

void BSP_Polys_PushBack(bsp_polys_t* polys, const bsp_poly_t* poly)
{
	bsp_poly_t* polynew = BSP_Poly_Create(false);
	
	polynew->verts = BSP_Poly_CloneVerts(poly);

	List_PushBack(polys, polynew);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bsp_line_t* BSP_Line_Create()
{
	bsp_line_t* line = ALLOCATE(bsp_line_t);
	return line;
}

//----------------------------------------------------------------------------

void BSP_Line_Destroy(bsp_line_t* line)
{
	DEALLOCATE(bsp_line_t, line);
}

//----------------------------------------------------------------------------

void BSP_Line_Set(bsp_line_t* line, const vec2_t src, const vec2_t dest)
{
	M_CopyVec2(src, line->src);
	M_CopyVec2(dest, line->dest);
	M_SubVec2(dest, src, line->dir);
	M_NormalizeVec2(line->dir);
	line->perp[0] = line->dir[1];
	line->perp[1] = -line->dir[0];
}

//----------------------------------------------------------------------------

void BSP_Line_Offset(bsp_line_t* line, float offset)
{
	vec2_t temp;
	M_MultVec2(line->perp, -offset, temp);
	M_AddVec2(line->src, temp, line->src);
	M_AddVec2(line->dest, temp, line->dest);
}

//----------------------------------------------------------------------------

float BSP_Line_CalculateLength(const bsp_line_t* line)
{
	return sqrt(M_GetDistanceSqVec2(line->dest, line->src));
}

//----------------------------------------------------------------------------

bool BSP_Line_ParameterizePoint(const bsp_line_t* line, const vec2_t point, float* t)
{
	vec2_t edge;
	vec2_t offset;

	M_SubVec2(line->dest, line->src, edge);
	M_SubVec2(point, line->src, offset);

	*t = M_DotProduct2(edge, offset)/M_DotProduct2(edge, edge);

	{
		vec2_t diff;
		float d;

		M_MultVec2(edge, *t, diff);
		M_AddVec2(diff, line->src, diff);
		M_SubVec2(point, diff, diff);

		d = M_GetVec2Length(diff);

		if ( d < 10e-4f )
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bsp_lines_t* BSP_Lines_Create()
{
	bsp_lines_t* lines = ALLOCATE(bsp_lines_t);
	lines->first = NULL;
	lines->last = NULL;
	return lines;
}

//----------------------------------------------------------------------------

void BSP_Lines_Destroy(bsp_lines_t* lines)
{
	BSP_Lines_Clear(lines);
	DEALLOCATE(bsp_lines_t, lines);
}

//----------------------------------------------------------------------------

void BSP_Lines_Clear(bsp_lines_t* lines)
{
	bsp_line_t* line = lines->first;

	while ( line )
	{
		bsp_line_t* linenext = line->next;
		BSP_Line_Destroy(line);
		line = linenext;
	}

	lines->first = NULL;
	lines->last = NULL;
}

//----------------------------------------------------------------------------

void BSP_Lines_PushFront(bsp_lines_t* lines, const bsp_line_t* line)
{
	bsp_line_t* linenew = BSP_Line_Create();
	
	M_CopyVec2(line->src, linenew->src);
	M_CopyVec2(line->dest, linenew->dest);
	M_CopyVec2(line->dir, linenew->dir);
	M_CopyVec2(line->perp, linenew->perp);

	List_PushFront(lines, linenew);
}

//----------------------------------------------------------------------------

void BSP_Lines_PushBack(bsp_lines_t* lines, const bsp_line_t* line)
{
	bsp_line_t* linenew = BSP_Line_Create();
	
	M_CopyVec2(line->src, linenew->src);
	M_CopyVec2(line->dest, linenew->dest);
	M_CopyVec2(line->dir, linenew->dir);
	M_CopyVec2(line->perp, linenew->perp);

	List_PushBack(lines, linenew);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void BSP_Plane_Init(bsp_plane_t* self, const bsp_line_t* line)
{
	M_SetVec2(self->normal, -line->perp[0], -line->perp[1]);
	self->distance = -M_DotProduct2(self->normal, line->src);
}

//----------------------------------------------------------------------------

int BSP_Plane_ClassifyPoint(const bsp_plane_t* self, const vec2_t point)
{
	float f = M_DotProduct2(self->normal, point) + self->distance;

	if ( fabs(f) < .01 && fabs(f) > navrep_bsp_epsilon.value )
	{
		int a = 1;
	}

	if ( f > navrep_bsp_epsilon.value ) //.01 )
	{
		return BSP_PLANE_POINT_FRONT;
	}
	else if ( f < -navrep_bsp_epsilon.value ) //-.01 )
	{
		return BSP_PLANE_POINT_BACK;
	}
	else
	{
		return BSP_PLANE_POINT_ON;
	}
}

//----------------------------------------------------------------------------

bool BSP_Plane_IntersectLine(const bsp_plane_t* self, const bsp_line_t* line, bsp_line_t* line_front, bsp_line_t* line_back)
{
	float num, den, t;
	vec2_t intersection;
	
	den = M_DotProduct2(line->dir, self->normal);
	// $eps should just be 0?
	if ( fabs(den) < navrep_bsp_epsilon.value ) return false;

	num = -self->distance - M_DotProduct2(self->normal, line->src);

	t = num/den;

	M_CopyVec2(line->dir, intersection);
	M_MultVec2(intersection, t, intersection);
	M_AddVec2(intersection, line->src, intersection);

	if ( BSP_Plane_ClassifyPoint(self, line->dest) == BSP_PLANE_POINT_FRONT )
	{
		BSP_Line_Set(line_front, intersection, line->dest);
		BSP_Line_Set(line_back, line->src, intersection);
	}
	else if ( BSP_Plane_ClassifyPoint(self, line->dest) == BSP_PLANE_POINT_BACK )
	{
		BSP_Line_Set(line_front, line->src, intersection);
		BSP_Line_Set(line_back, intersection, line->dest);
	}
	else
	{
		NAVBREAK(false);
	}
	
	return true;
}

//----------------------------------------------------------------------------

bool BSP_Plane_IntersectSegment(const bsp_plane_t* self, const bsp_line_t* line, vec2_t point)
{
	float den, num, t;
	
	den = M_DotProduct2(line->dir, self->normal);
	// $eps should just be 0?
	if ( fabs(den) < navrep_bsp_epsilon.value ) return false;

	num = -self->distance - M_DotProduct2(self->normal, line->src);

	t = num/den;

	if ( t < .01f || t > (BSP_Line_CalculateLength(line) - .01f) )//(line->dest-line->src).Mag()-.01f )
	{
		return false;
	}

	M_CopyVec2(line->dir, point);
	M_MultVec2(point, t, point);
	M_AddVec2(point, line->src, point);

	return true;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bsp_node_t* BSP_Node_Create(const bsp_line_t* line) 
{ 
	bsp_node_t* node = ALLOCATE(bsp_node_t);
	BSP_Plane_Init(&node->plane, line);
	node->line = *line;
	node->front = NULL;
	node->back = NULL;
	return node; 
}

void BSP_Node_Destroy(bsp_node_t* node)
{
	if ( node->front ) 
	{
		BSP_Node_Destroy(node->front);
	}

	if ( node->back )
	{
		BSP_Node_Destroy(node->back);
	}

	DEALLOCATE(bsp_node_t, node);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bsp_t* BSP_Create()
{
	bsp_t* bsp = ALLOCATE(bsp_t);
	bsp->root = NULL;
	bsp->next = NULL;
	return bsp;
}

//----------------------------------------------------------------------------

bsp_t* BSP_CreateFromObject(const baseObject_t* obj, float offset)
{
	vec3_t bmin, bmax;

	if ( !WO_IsObjectActive(obj->index) )
	{
		// this is a dynamic object, like an npc. just send the dims in.

		M_CopyVec2(obj->bmin, bmin);
		M_CopyVec2(obj->bmax, bmax);

		//offset = MAX((bmax[0] - bmin[0]), (bmax[1] - bmin[1]))+5.0f;

		M_AddVec2(obj->pos, bmin, bmin);
		M_AddVec2(obj->pos, bmax, bmax);

		return BSP_CreateFromBounds(bmin, bmax, offset);
	}
	else
	{
		vec3_t axis[3];
		vec3_t points[4];
		objectPosition_t objpos;

		// this is a building. offset the dims, and transform to obb.

		WO_GetObjectPos(obj->index, &objpos);
		Res_GetModelSurfaceBounds(WO_GetObjectModel(obj->index), bmin, bmax);

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

		{
			bsp_line_t line;
			bsp_t* bsp = NULL;

			bsp = BSP_Create();

			BSP_Line_Set(&line, points[1], points[0]);
			BSP_Add(bsp, &line, NULL);

			BSP_Line_Set(&line, points[2], points[1]);
			BSP_Add(bsp, &line, NULL);

			BSP_Line_Set(&line, points[3], points[2]);
			BSP_Add(bsp, &line, NULL);

			BSP_Line_Set(&line, points[0], points[3]);
			BSP_Add(bsp, &line, NULL);

			M_ClearRect(bsp->min, bsp->max);
			M_AddPointToRect(points[0], bsp->min, bsp->max);
			M_AddPointToRect(points[1], bsp->min, bsp->max);
			M_AddPointToRect(points[2], bsp->min, bsp->max);
			M_AddPointToRect(points[3], bsp->min, bsp->max);

			return bsp;
		}
	}
}


//----------------------------------------------------------------------------

bsp_t* BSP_CreateFromObjectSurface(const worldObject_t* obj, const linkedSurface_t* surf, const vec3_t poly_bmin, const vec3_t poly_bmax, float offset)
{
	vec3_t bmin, bmax;
	vec3_t axis[3];
	vec3_t points[4];
//	objectPosition_t obj->objpos;

	// this is a building. offset the dims, and transform to obb.

//	WO_GetObjectPos(obj->index, &obj->objpos);
	bmin[0] = poly_bmin[0];
	bmin[0] *= obj->objpos.scale;
	bmin[0] -= offset;
	
	bmin[1] = poly_bmin[1];
	bmin[1] *= obj->objpos.scale;
	bmin[1] -= offset;

	bmax[0] = poly_bmax[0];
	bmax[0] *= obj->objpos.scale;
	bmax[0] += offset;
	
	bmax[1] = poly_bmax[1];
	bmax[1] *= obj->objpos.scale;
	bmax[1] += offset;

	memcpy(axis, surf->worldAxis, sizeof(axis));
//	M_GetAxis(obj->angle[0], obj->angle[1], obj->angle[2], axis);
	M_TransformPoint(vec3(bmin[0], bmin[1], 0), surf->worldPos/*obj->objpos.position*/, (const vec3_t *)axis, points[0]);
	M_TransformPoint(vec3(bmin[0], bmax[1], 0), surf->worldPos/*obj->objpos.position*/, (const vec3_t *)axis, points[1]);
	M_TransformPoint(vec3(bmax[0], bmax[1], 0), surf->worldPos/*obj->objpos.position*/, (const vec3_t *)axis, points[2]);
	M_TransformPoint(vec3(bmax[0], bmin[1], 0), surf->worldPos/*obj->objpos.position*/, (const vec3_t *)axis, points[3]);

	{
		bsp_line_t line;
		bsp_t* bsp = NULL;

		bsp = BSP_Create();

		BSP_Line_Set(&line, points[1], points[0]);
		BSP_Add(bsp, &line, NULL);

		BSP_Line_Set(&line, points[2], points[1]);
		BSP_Add(bsp, &line, NULL);

		BSP_Line_Set(&line, points[3], points[2]);
		BSP_Add(bsp, &line, NULL);

		BSP_Line_Set(&line, points[0], points[3]);
		BSP_Add(bsp, &line, NULL);

		M_ClearRect(bsp->min, bsp->max);
		M_AddPointToRect(points[0], bsp->min, bsp->max);
		M_AddPointToRect(points[1], bsp->min, bsp->max);
		M_AddPointToRect(points[2], bsp->min, bsp->max);
		M_AddPointToRect(points[3], bsp->min, bsp->max);

		return bsp;
	}
}

//----------------------------------------------------------------------------

bsp_t* BSP_CreateFromBounds(const vec3_t min, const vec3_t max, float offset)
{
	bsp_line_t line;
	bsp_t* bsp = NULL;

	bsp = BSP_Create();

	// set bsp dims, and fudge a little just to err on the side of caution

	M_SetVec2(bsp->min, min[0]-offset-1, min[1]-offset-1);
	M_SetVec2(bsp->max, max[0]+offset+1, max[1]+offset+1);

	// construct the bsp, offsetting by the specified amount. todo: bevel?

	BSP_Line_Set(&line, vec2(-offset+min[0], -offset+min[1]), vec2(+offset+max[0], -offset+min[1]));
	BSP_Add(bsp, &line, NULL);

	BSP_Line_Set(&line, vec2(+offset+max[0], -offset+min[1]), vec2(+offset+max[0], +offset+max[1]));
	BSP_Add(bsp, &line, NULL);

	BSP_Line_Set(&line, vec2(+offset+max[0], +offset+max[1]), vec2(-offset+min[0], +offset+max[1]));
	BSP_Add(bsp, &line, NULL);

	BSP_Line_Set(&line, vec2(-offset+min[0], +offset+max[1]), vec2(-offset+min[0], -offset+min[1]));
	BSP_Add(bsp, &line, NULL);

	return bsp;
}

//----------------------------------------------------------------------------

void BSP_Destroy(bsp_t* bsp)
{
	if ( bsp->root )
	{
		BSP_Clear(bsp, bsp->root);
		bsp->root = NULL;
	}

	DEALLOCATE(bsp_t, bsp);
}

//----------------------------------------------------------------------------

void BSP_Clear(bsp_t* self, bsp_node_t* node)
{
	if ( !node ) return;

	if ( node->front ) 
	{
		BSP_Clear(self, node->front);
		node->front = NULL;
	}

	if ( node->back ) 
	{
		BSP_Clear(self, node->back);
		node->back = NULL;
	}

	BSP_Node_Destroy(node);
}

//----------------------------------------------------------------------------

void BSP_Add(bsp_t* self, const bsp_line_t* line, bsp_node_t* node)
{
	int pcSrc, pcDest;

	if ( node == NULL && self->root == NULL )
	{
		NAVBREAK(line.CalculateLength() > 0);
		self->root = BSP_Node_Create(line);
		return;
	}

	if ( node == NULL )
	{
		node = self->root;
	}

	pcSrc = BSP_Plane_ClassifyPoint(&node->plane, line->src);
	pcDest = BSP_Plane_ClassifyPoint(&node->plane, line->dest);

	if ( pcSrc == BSP_PLANE_POINT_ON )
	{
		pcSrc = pcDest;
	}
	else if ( pcDest == BSP_PLANE_POINT_ON )
	{
		pcDest = pcSrc;
	}

	if ( pcSrc != pcDest )
	{
		bsp_line_t line_front, line_back;

		if ( BSP_Plane_IntersectLine(&node->plane, line, &line_front, &line_back) )
		{
			NAVBREAK(M_DotProduct2(line_front.dir, line->dir) > 0);
			NAVBREAK(M_DotProduct2(line_back.dir, line->dir) > 0);

			if ( node->front )
			{
				BSP_Add(self, &line_front, node->front);
			}
			else
			{
				NAVBREAK(line_front.CalculateLength() > 0);
				node->front = BSP_Node_Create(&line_front);
			}

			if ( node->back )
			{
				BSP_Add(self, &line_back, node->back);
			}
			else
			{
				NAVBREAK(line_back.CalculateLength() > 0);
				node->back = BSP_Node_Create(&line_back);
			}
		}
		else
		{
			NAVBREAK(false);
		}
	}
	else if ( pcSrc == BSP_PLANE_POINT_ON )
	{
	}
	else
	{
		if ( pcSrc == BSP_PLANE_POINT_FRONT )
		{
			if ( node->front )
			{
				BSP_Add(self, line, node->front);
			}
			else
			{
				node->front = BSP_Node_Create(line);
			}
		}
		else if ( pcSrc == BSP_PLANE_POINT_BACK )
		{
			if ( node->back )
			{
				BSP_Add(self, line, node->back);
			}
			else
			{
				node->back = BSP_Node_Create(line);
			}
		}
	}
}

//----------------------------------------------------------------------------

void BSP_ClipPoly(const bsp_t* self, const bsp_poly_t* poly, bsp_polys_t* polys_in, bsp_polys_t* polys_out, const bsp_node_t* node)
{	
	bool destroy_poly_front = true;
	bool destroy_poly_back = true;
	bsp_poly_t* poly_front = BSP_Poly_Create(true);
	bsp_poly_t* poly_back = BSP_Poly_Create(true);
	bsp_vert_t* vert_prev;
	bsp_vert_t* vert;
	int pcVertPrev;
	int pcVert;

	if ( NULL == node )
	{
		node = self->root;
		if ( !node ) return;
	}

	vert_prev = poly->verts->last;
	vert = poly->verts->first;
	pcVertPrev = BSP_Plane_ClassifyPoint(&node->plane, vert_prev->vert);

	while ( vert )
	{
		pcVert = BSP_Plane_ClassifyPoint(&node->plane, vert->vert);

		if ( pcVertPrev == BSP_PLANE_POINT_FRONT )
		{
			if ( pcVert == BSP_PLANE_POINT_FRONT )
			{
				BSP_Verts_PushBack(poly_front->verts, vert_prev);
			}
			else if ( pcVert == BSP_PLANE_POINT_BACK )
			{
				bsp_vert_t intersection;
				
				bsp_line_t line;
				BSP_Line_Set(&line, vert_prev->vert, vert->vert);

				if ( BSP_Plane_IntersectSegment(&node->plane, &line, intersection.vert) )
				{
					BSP_Verts_PushBack(poly_front->verts, vert_prev);
					BSP_Verts_PushBack(poly_front->verts, &intersection);

					BSP_Verts_PushBack(poly_back->verts, &intersection);
				}
				else
				{
					NAVBREAK(false);
				}
			}
			else // if ( pcVertNext == BSP_PLANE_POINT_ON )
			{
				BSP_Verts_PushBack(poly_front->verts, vert_prev);
			}
		}
		else if ( pcVertPrev == BSP_PLANE_POINT_BACK )
		{
			if ( pcVert == BSP_PLANE_POINT_BACK )
			{
				BSP_Verts_PushBack(poly_back->verts, vert_prev);
			}
			else if ( pcVert == BSP_PLANE_POINT_FRONT )
			{
				bsp_vert_t intersection;
				
				bsp_line_t line;
				BSP_Line_Set(&line, vert_prev->vert, vert->vert);

				if ( BSP_Plane_IntersectSegment(&node->plane, &line, intersection.vert) )
				{
					BSP_Verts_PushBack(poly_back->verts, vert_prev);
					BSP_Verts_PushBack(poly_back->verts, &intersection);

					BSP_Verts_PushBack(poly_front->verts, &intersection);
				}
				else
				{
					NAVBREAK(false);
				}
			}
			else // if ( pcVertNext == BSP_PLANE_POINT_ON )
			{
				BSP_Verts_PushBack(poly_back->verts, vert_prev);
			}
		}
		else // if ( pcVertPrev == BSP_PLANE_POINT_ON )
		{
			BSP_Verts_PushBack(poly_front->verts, vert_prev);
			BSP_Verts_PushBack(poly_back->verts, vert_prev);
		}

		vert_prev = vert;
		vert = vert->next;
		pcVertPrev = pcVert;
	}

	if ( poly_front->verts->first && poly_front->verts->first->next && poly_front->verts->first->next->next )
	{
		if ( node->front )
		{
			BSP_ClipPoly(self, poly_front, polys_in, polys_out, node->front);
		}
		else if ( polys_in )
		{
			List_PushBack(polys_in, poly_front);
			destroy_poly_front = false;
//			BSP_Polys_PushBack(polys_in, poly_front);
		}
	}

	if ( poly_back->verts->first && poly_back->verts->first->next && poly_back->verts->first->next->next )
	{
		if ( node->back )
		{
			BSP_ClipPoly(self, poly_back, polys_in, polys_out, node->back);
		}
		else if ( polys_out )
		{
			List_PushBack(polys_out, poly_back);
			destroy_poly_back = false;
//			BSP_Polys_PushBack(polys_out, poly_back);
		}
	}

	if ( destroy_poly_front )
		BSP_Poly_Destroy(poly_front);

	if ( destroy_poly_back )
		BSP_Poly_Destroy(poly_back);
}

//----------------------------------------------------------------------------

void BSP_ClipLine(const bsp_t* self, const bsp_line_t* line, bsp_lines_t* lines_in, bsp_lines_t* lines_out, const bsp_node_t* node)
{	
	int pcSrc, pcDest;

	if ( NULL == node )
	{
		node = self->root;

		if ( !node ) return;
	}

	pcSrc = BSP_Plane_ClassifyPoint(&node->plane, line->src);
	pcDest = BSP_Plane_ClassifyPoint(&node->plane, line->dest);

	if ( pcSrc == BSP_PLANE_POINT_ON )
	{
		pcSrc = pcDest;
	}
	else if ( pcDest == BSP_PLANE_POINT_ON )
	{
		pcDest = pcSrc;
	}

	if ( pcSrc != pcDest )
	{
		bsp_line_t line_front, line_back;

		if ( BSP_Plane_IntersectLine(&node->plane, line, &line_front, &line_back) )
		{
			NAVBREAK(line_front->dir.Dot(line->dir) > 0);
			NAVBREAK(line_back->dir.Dot(line->dir) > 0);

			if ( node->front )
			{
				BSP_ClipLine(self, &line_front, lines_in, lines_out, node->front);
			}
			else if ( lines_in )
			{
				BSP_Lines_PushBack(lines_in, &line_front);
			}

			if ( node->back )
			{
				BSP_ClipLine(self, &line_back, lines_in, lines_out, node->back);
			}
			else if ( lines_out )
			{
				BSP_Lines_PushBack(lines_out, &line_back);
			}
		}
		else
		{
			NAVBREAK(false);
		}
	}
	else if ( pcSrc == BSP_PLANE_POINT_ON )
	{
		if ( M_DotProduct2(line->perp, node->line.perp) < 0 )
		{
			// opposite winding order - line is inside (front)

			if ( node->front )
			{
				BSP_ClipLine(self, line, lines_in, lines_out, node->front);
			}
			else if ( lines_in )
			{
				BSP_Lines_PushBack(lines_out, line);
			}
		}
		else
		{
			// same winding order - line is outside (back)

			if ( node->back )
			{
				BSP_ClipLine(self, line, lines_in, lines_out, node->back);
			}
			else if ( lines_out )
			{
				BSP_Lines_PushBack(lines_out, line);
			}
		}
	}
	else
	{
		if ( pcSrc == BSP_PLANE_POINT_FRONT )
		{
			if ( node->front )
			{
				BSP_ClipLine(self, line, lines_in, lines_out, node->front);
			}
			else if ( lines_in )
			{
				BSP_Lines_PushBack(lines_in, line);
			}
		}
		else if ( pcSrc == BSP_PLANE_POINT_BACK )
		{
			if ( node->back )
			{
				BSP_ClipLine(self, line, lines_in, lines_out, node->back);
			}
			else if ( lines_out )
			{
				BSP_Lines_PushBack(lines_out, line);
			}
		}
	}
}

//----------------------------------------------------------------------------

bool BSP_Subtract(bsp_t* self, bsp_poly_t* poly, bsp_polys_t* polys)
{
	bsp_polys_t polys_in = { NULL, NULL };
	
	BSP_ClipPoly(self, poly, &polys_in, polys, self->root);	

	if ( !polys_in.first )
	{
		// Not clipped, just put "poly" in "polys"
		
		BSP_Polys_Clear(polys);
		BSP_Polys_PushBack(polys, poly);

		return false;
	}
	else
	{
		// Clipped, fragments are in "polys"
		
		BSP_Polys_Clear(&polys_in);

		return true;
	}
}

//----------------------------------------------------------------------------

void BSP_Startup()
{
	Cvar_Register(&navrep_bsp_epsilon);
}