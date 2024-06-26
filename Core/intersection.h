// (C) 2002 S2 Games

// intersection.h

// geometry intersection routines
// repurposed from david eberly's source code (www.magic-software.com)


typedef struct
{
	vec3_t	center;
	vec3_t	axis[3];
	float	extents[3];
} obb_t;

bool I_BoxTriIntersect (const vec3_t apkTri[3],
    const vec3_t bpos, const vec3_t bext, const vec3_t rkBoxVel, float fTMax, float *rfTFirst,
    float *rfTLast);
bool	I_MovingBoundsIntersect(vec3_t start, vec3_t end, vec3_t bmin_a, vec3_t bmax_a, vec3_t bmin_b, vec3_t bmax_b, float *fraction);
bool	I_MovingBoundsIntersectEx(vec3_t start, vec3_t end, vec3_t bmin_a, vec3_t bmax_a, vec3_t bmin_b, vec3_t bmax_b, float *fraction, vec3_t normal);
bool	I_LineMeshIntersect(const vec3_t start, const vec3_t end, const struct mesh_s *mesh, float *fraction, int *face_hit);