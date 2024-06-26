#ifndef BSP_H
#define BSP_H

//----------------------------------------------------------------------------


EXTERN_ALLOCATOR(bsp_vert_t);
EXTERN_ALLOCATOR(bsp_verts_t);
EXTERN_ALLOCATOR(bsp_line_t);
EXTERN_ALLOCATOR(bsp_lines_t);
EXTERN_ALLOCATOR(bsp_poly_t);
EXTERN_ALLOCATOR(bsp_polys_t);
EXTERN_ALLOCATOR(bsp_node_t);
EXTERN_ALLOCATOR(bsp_t);

//----------------------------------------------------------------------------

typedef struct bsp_vert_s
{
	vec2_t				vert;
	struct bsp_vert_s*	prev;
	struct bsp_vert_s*	next;
}
bsp_vert_t;

bsp_vert_t* BSP_Vert_Create();
void BSP_Vert_Destroy(bsp_vert_t* vert);

//----------------------------------------------------------------------------

typedef struct bsp_verts_s
{
	bsp_vert_t*			first;
	bsp_vert_t*			last;
	int					ref;
}
bsp_verts_t;

bsp_verts_t* BSP_Verts_Create();
void BSP_Verts_Destroy(bsp_verts_t* verts);

void BSP_Verts_Clear(bsp_verts_t* verts);
void BSP_Verts_PushFront(bsp_verts_t* verts, const bsp_vert_t* vert);
void BSP_Verts_PushBack(bsp_verts_t* verts, const bsp_vert_t* vert);

//----------------------------------------------------------------------------

typedef struct bsp_poly_s
{
	bsp_verts_t*		verts;
	struct bsp_poly_s*	prev;
	struct bsp_poly_s*	next;
}
bsp_poly_t;

bsp_poly_t* BSP_Poly_Create(bool bCreateVerts);
bsp_poly_t* BSP_Poly_CreateFromWorld();
bsp_poly_t* BSP_Poly_CreateFromPathPoly(const navpoly_t* navpoly);
void BSP_Poly_Destroy(bsp_poly_t* poly);
float BSP_Poly_CalculateArea(bsp_poly_t* bspPoly);
bsp_verts_t* BSP_Poly_CloneVerts(const bsp_poly_t* poly);

//----------------------------------------------------------------------------

typedef struct bsp_polys_s
{
	bsp_poly_t*			first;
	bsp_poly_t*			last;
}
bsp_polys_t;

bsp_polys_t* BSP_Polys_Create();
void BSP_Polys_Destroy(bsp_polys_t* polys);

void BSP_Polys_Clear(bsp_polys_t* polys);
void BSP_Polys_PushFront(bsp_polys_t* polys, const bsp_poly_t* poly);
void BSP_Polys_PushBack(bsp_polys_t* polys, const bsp_poly_t* poly);

//----------------------------------------------------------------------------

typedef struct bsp_line_s
{
	vec2_t				src;
	vec2_t				dest;
	vec2_t				dir;
	vec2_t				perp;
	struct bsp_line_s*	prev;
	struct bsp_line_s*	next;
}
bsp_line_t;

bsp_line_t* BSP_Line_Create();
void BSP_Line_Destroy(bsp_line_t* line);

bool BSP_Line_SharesVert(bsp_line_t* line, bsp_line_t* line_other);
void BSP_Line_Set(bsp_line_t* line, const vec2_t src, const vec2_t dest);
void BSP_Line_Offset(bsp_line_t* line, float offset);
float BSP_Line_CalculateLength(const bsp_line_t* line);
bool BSP_Line_ParameterizePoint(const bsp_line_t* line, const vec2_t point, float* t);

//----------------------------------------------------------------------------

typedef struct bsp_lines_s
{
	bsp_line_t*			first;
	bsp_line_t*			last;
}
bsp_lines_t;

bsp_lines_t* BSP_Lines_Create();
void BSP_Lines_Destroy(bsp_lines_t* lines);

void BSP_Lines_Clear(bsp_lines_t* lines);
void BSP_Lines_PushFront(bsp_lines_t* lines, const bsp_line_t* line);
void BSP_Lines_PushBack(bsp_lines_t* lines, const bsp_line_t* line);

//----------------------------------------------------------------------------

#define BSP_PLANE_POINT_FRONT	0x01
#define BSP_PLANE_POINT_BACK	0x02
#define BSP_PLANE_POINT_ON		0x03

typedef struct
{
	vec2_t				normal;
	float				distance;
}
bsp_plane_t;

void BSP_Plane_Init(bsp_plane_t* self, const bsp_line_t* line);
int BSP_Plane_ClassifyPoint(const bsp_plane_t* self, const vec2_t point);
bool BSP_Plane_IntersectLine(const bsp_plane_t* self, const bsp_line_t* line, bsp_line_t* lineFront, bsp_line_t* lineBack);
bool BSP_Plane_IntersectSegment(const bsp_plane_t* self, const bsp_line_t* line, vec2_t point);

//----------------------------------------------------------------------------

typedef struct bsp_node_s
{
	bsp_plane_t			plane;
	bsp_line_t			line;
	struct bsp_node_s*	front;
	struct bsp_node_s*	back;
}
bsp_node_t;

bsp_node_t* BSP_Node_Create(const bsp_line_t* line);
void BSP_Node_Destroy(bsp_node_t* node);

//----------------------------------------------------------------------------

typedef struct bsp_s
{
	bsp_node_t*			root;
	vec2_t				min;
	vec2_t				max;
	struct bsp_s*		next;
}
bsp_t;

bsp_t* BSP_Create();
bsp_t* BSP_CreateFromObject(const baseObject_t* obj, float offset);
bsp_t* BSP_CreateFromBounds(const vec3_t min, const vec3_t max, float offset);
bsp_t* BSP_CreateFromObjectSurface(const worldObject_t* obj, const linkedSurface_t* surf, const vec3_t poly_bmin, const vec3_t poly_bmax, float offset);
void BSP_Destroy(bsp_t* bsp);
void BSP_Clear(bsp_t* self, bsp_node_t* node);
void BSP_Add(bsp_t* self, const bsp_line_t* line, bsp_node_t* node);
void BSP_ClipPoly(const bsp_t* self, const bsp_poly_t* poly, bsp_polys_t* polys_in, bsp_polys_t* polys_out, const bsp_node_t* node);
void BSP_ClipLine(const bsp_t* self, const bsp_line_t* line, bsp_lines_t* lines_in, bsp_lines_t* lines_out, const bsp_node_t* node);
bool BSP_Subtract(bsp_t* self, bsp_poly_t* poly, bsp_polys_t* polys);

void BSP_Startup();

#endif
