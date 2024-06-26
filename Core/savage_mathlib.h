// (C) 2003 S2 Games

// savage_mathlib.h

#ifndef SAVAGE_MATHLIB_H
#define SAVAGE_MATHLIB_H

#define MACRO_MATH		//enable for fast macro versions of functions

#include <math.h>

#define MAX_INT			2147483647 //assume 32-bit here, because assuming otherwise will break things currently

#define	PLANETYPE_X		0 //plane is parallel to the x axis
#define PLANETYPE_Y		1 //plane is parallel to the y axis
#define PLANETYPE_Z		2 //plane is parallel to the z axis
#define PLANETYPE_ANY	3 //all other planes

//euler rotation
#define PITCH			0
#define ROLL			1
#define YAW				2

#ifdef FAST_SQRT
#define SQRT fastsqrt
#else
#define SQRT sqrt
#endif

extern float sinData[];

#define MAX_SINE_SAMPLES 4096

#ifdef SIN_LOOKUP_TABLE

//extern int sin_tmp;

//the following handles negative values for SIN(), but at the expense of a branch operation
//#define SIN(a) (sin_tmp = (int)(MAX_SINE_SAMPLES*((float)(a)/(2*M_PI))) sinData[(sin_tmp < 0 ? sin_tmp + MAX_SINE_SAMPLES * (1 + -sin_tmp/MAX_SINE_SAMPLES) : sin_tmp) % MAX_SINE_SAMPLES])
//#define COS(a) (sin_tmp = (int)(MAX_SINE_SAMPLES*((float)((a) + M_PI)/(2*M_PI))); sinData[(sin_tmp < 0 ? sin_tmp + MAX_SINE_SAMPLES * (1 + -sin_tmp/MAX_SINE_SAMPLES) : sin_tmp) % MAX_SINE_SAMPLES])


#define POSSIN(a) (sinData[((int)(0.5 + MAX_SINE_SAMPLES*((float)(a)/(2*M_PI)))) % MAX_SINE_SAMPLES])
#define POSCOS(a) (sinData[((int)(0.5 + MAX_SINE_SAMPLES*((float)((a) + M_PI/2)/(2*M_PI)))) % MAX_SINE_SAMPLES])

#define POSDEGSIN(a) (sinData[((int)(0.5 + MAX_SINE_SAMPLES*((float)(a)/360))) % MAX_SINE_SAMPLES])
#define POSDEGCOS(a) (sinData[((int)(0.5 + MAX_SINE_SAMPLES*((float)((a) + 90)/360))) % MAX_SINE_SAMPLES])

//hack, allows up to -10 * 2pi values (hopefully won't cause too much skew in the floats)
#define SIN(a) (ABS(POSSIN(a + 5*2*M_PI) - sin(a)) > 0.02 ? printf("SIN(%f) = %f, sin(%f) = %f", (float)a, POSSIN(a + 5*2*M_PI), (float)a, sin(a)) : POSSIN(a + 5*2*M_PI))
#define COS(a) (ABS(POSCOS(a + 5*2*M_PI) - cos(a)) > 0.02 ? printf("COS(%f) = %f, cos(%f) = %f", (float)a, POSCOS(a + 5*2*M_PI), (float)a, cos(a)) : POSCOS(a + 5*2*M_PI))

/*
#define SIN(a) (sin(a))
#define COS(a) (cos(a))
*/

#define DEGSIN(a) (POSDEGSIN(a + 5*360))
#define DEGCOS(a) (POSDEGCOS(a + 5*360))

/*
#define DEGSIN(a) (ABS(POSDEGSIN(a + 5*360) - sin(DEG2RAD(a))) > 0.02 ? printf("DEGSIN(%f) = %f, sin(deg2rad(%f)) = %f", (float)a, POSDEGSIN(a + 5*360), (float)a, sin(DEG2RAD(a))) : POSDEGSIN(a + 5*360))
#define DEGCOS(a) (ABS(POSDEGCOS(a + 5*360) - cos(DEG2RAD(a))) > 0.02 ? printf("DEGCOS(%f) = %f, cos(deg2rad(%f)) = %f", (float)a, POSDEGCOS(a + 5*360), (float)a, cos(DEG2RAD(a))) : POSDEGCOS(a + 5*360))
*/

#else

#define SIN(a) (sin(a))
#define COS(a) (cos(a))

#define DEGSIN(a) (sin(DEG2RAD(a)))
#define DEGCOS(a) (cos(DEG2RAD(a)))

#endif

//plane
typedef struct
{
	vec3_t	normal;   //A,B,C
	float	dist;  //D
	//byte	type;
//	int		octant;
} plane_t;

typedef struct
{
	vec2_t	normal;
	float	dist;	
} line_t;

//reference as axis and position
typedef struct
{
	vec4_t			axis[3];
	vec4_t			pos;
} _matrix44_t;

typedef struct
{
	vec3_t			axis[3];
	vec3_t			pos;
} matrix43_t;

//reference as list of floats
typedef struct
{
	float f00;
	float f01;
	float f02;
	float f03;
	float f10;
	float f11;
	float f12;
	float f13;
	float f20;
	float f21;
	float f22;
	float f23;
	float f30;
	float f31;
	float f32;
	float f33;
} __matrix44_t;

//4x4 matrix, can be referred to either by 'axis' and 'pos' or through the 'matrix' array
typedef union
{
	_matrix44_t t;
	float		matrix[16];		//reference as array
	__matrix44_t f;
} matrix44_t;


#define	iround(x)	((int)(x+0.5))

#ifndef ABS
#define	ABS(a)	(((a)<0) ? -(a) : (a))
#endif

#define SGN(a)	(((a)<0) ? -1 : 1)
#define	LERP(a,l,h) ((l)+(((h)-(l))*(a)))

#ifndef CLAMP
#define CLAMP(v,l,h) ((v)<(l) ? (l) : (v) > (h) ? (h) : (v))
#endif

#ifndef MIN
#define MIN(x,y) ((x)>(y)?(y):(x))
#endif

#ifndef MAX
#define MAX(x,y) ((x)<(y)?(y):(x))
#endif

#define	EPSILON		0.000001f

#define M_VEC2LENSQR(a) ((a[0]*a[0])+(a[1])*(a[1]))
#define M_VEC3LENSQR(a) ((a[0]*a[0])+(a[1])*(a[1])+(a[2])*(a[2]))

/*
#ifdef _WIN32
__forceinline void FloatToInt(int *int_pointer, float f);
#else*/
//INLINE void FloatToInt(int *int_pointer, float f);
//#endif

#ifdef MACRO_MATH
#define M_AddVec3(a, b, out) ((out[0]) = (a[0]) + (b[0]), (out[1]) = (a[1]) + (b[1]), (out[2]) = (a[2]) + (b[2]))
#define M_AddVec2(a, b, out) ((out[0]) = (a[0]) + (b[0]), (out[1]) = (a[1]) + (b[1]))
#define M_SubVec3(a, b, out) ((out[0]) = (a[0]) - (b[0]), (out[1]) = (a[1]) - (b[1]), (out[2]) = (a[2]) - (b[2]))
#define M_SubVec2(a, b, out) ((out[0]) = (a[0]) - (b[0]), (out[1]) = (a[1]) - (b[1]))
#define M_DotProduct(a,b) ((a[0])*(b[0]) + (a[1])*(b[1]) + (a[2])*(b[2]))
#define M_DotProduct2(a,b) ((a[0])*(b[0]) + (a[1])*(b[1]))

extern vec3_t __BEIvec__;
#define M_BoxExtentsIntersect(pos_a, ext_a, pos_b, ext_b) \
( \
	( M_SUBVEC3(pos_b, pos_a, __BEIvec__), \
	  (ABS(__BEIvec__[0]) <= ((ext_a[0]) + (ext_b[0])) && \
	  ABS(__BEIvec__[1]) <= ((ext_a[1]) + (ext_b[1])) && \
	  ABS(__BEIvec__[2]) <= ((ext_a[2]) + (ext_b[2]))) ? \
	  true : false \
	 ) \
)

#else
bool	M_BoxExtentsIntersect(const vec3_t pos_a, const vec3_t ext_a, const vec3_t pos_b, const vec3_t ext_b);
float	M_DotProduct(const vec3_t v1, const vec3_t v2);
void	M_AddVec3(const vec3_t a, const vec3_t b, vec3_t out);
void	M_AddVec2(const vec2_t a, const vec2_t b, vec2_t out);
void	M_SubVec3(const vec3_t a, const vec3_t b, vec3_t out);
void	M_SubVec2(const vec2_t a, const vec2_t b, vec2_t out);
#endif

void	M_RotatePointAroundVector(const vec3_t dir, const vec3_t point, float degrees, vec3_t out);
void	M_ProjectPointOnPlane(const vec3_t p, const vec3_t normal, vec3_t out);
float	M_DistToLineSegment(const vec3_t p, const vec3_t seg_a, const vec3_t seg_b);
float   M_GetDistance(const vec3_t pos1, const vec3_t pos2);
float   M_GetDistanceSq(const vec3_t pos1, const vec3_t pos2);
float   M_GetDistanceSqVec2(const vec2_t pos1, const vec2_t pos2);
void	M_SetVec3(vec3_t out, float x, float y, float z);
void	M_SetVec2(vec3_t out, float x, float y);
void	M_ClearVec3(vec3_t out);
void	M_ClearVec2(vec2_t out);
void	M_RotateVec2(float angle, vec2_t out);
bool	M_CompareVec2(const vec2_t a, const vec2_t b);
bool	M_CompareVec3(const vec3_t a, const vec3_t b);

INLINE void	M_MultVec3(const vec3_t a, float b, vec3_t out);
#define M_ScaleVec3 M_MultVec3
INLINE void M_FlipVec3(const vec3_t in, vec3_t out);
INLINE void M_MultVec2(const vec2_t a, float b, vec2_t out);
float	M_Normalize(vec3_t out);
float	M_NormalizeVec2(vec2_t out);
INLINE void	M_AddPointToBounds(const vec3_t point, vec3_t bmin, vec3_t bmax);
INLINE void	M_ClearBounds(vec3_t bmin, vec3_t bmax);

void	M_DivVec3(const vec3_t a, float b, vec3_t out);
void	M_GetNormal(vec3_t nml, int x, int y);
INLINE void	M_SurfaceNormal(const vec3_t a, const vec3_t b, const vec3_t c, vec3_t out);
void	M_SurfaceVec(const vec3_t a, const vec3_t b, const vec3_t c, vec3_t out);
void	M_CrossProduct(const vec3_t a, const vec3_t b, vec3_t out);
void	M_CopyVec2(const vec2_t in, vec2_t out);
void	M_CopyVec3(const vec3_t in, vec3_t out);
void	M_CopyVec4(const vec4_t in, vec4_t out);
void	M_GetRightVector(const vec3_t fwd, vec3_t out);
int		M_BoxOnPlaneSide(const vec3_t bmin, const vec3_t bmax, plane_t *p);
int		M_OBBOnPlaneSide (const vec3_t bmin, const vec3_t bmax,  const vec3_t pos, const vec3_t axis[3], plane_t *plane);
int		M_GetOctant(const vec3_t nml);
INLINE float	M_GetVec3Length(vec3_t vec);
INLINE float	M_SetVec3Length(vec3_t out, float length);
INLINE float	M_GetVec2Length(vec2_t vec);
INLINE float	M_SetVec2Length(vec2_t out, float length);
INLINE void	M_CalcPlane(const vec3_t a, const vec3_t b, const vec3_t c, plane_t *out);
void	M_AnglesToAxis(const vec3_t angles, vec3_t axis[3]);
bool	M_PointInBounds(const vec3_t point, const vec3_t bmin, const vec3_t bmax);
bool	M_RayBoxIntersect(const vec3_t origin, const vec3_t dir, const vec3_t bmin, const vec3_t bmax, vec3_t out);
int		M_RayBoxIntersect2(const vec3_t origin, const vec3_t dir, const vec3_t bmin, const vec3_t bmax, vec3_t out, vec3_t nml);
void	M_AddPointToRect(const vec2_t point, vec2_t bmin, vec2_t bmax);
void	M_AddRectToRectI(const ivec2_t bmin1, const ivec2_t bmax1, ivec2_t bmin2, ivec2_t bmax2);
void	M_ClearRectI(ivec2_t bmin, ivec2_t bmax);
void	M_ClearRect(vec2_t bmin, vec2_t bmax);
bool	M_RectInRect(const vec2_t bmin1, const vec2_t bmax1, const vec2_t bmin2, const vec2_t bmax2);
void	M_CalcBoxExtents(const vec3_t bmin, const vec3_t bmax, vec3_t pos, vec3_t ext);
void	M_CalcBoxMinMax(const vec3_t bpos, const vec3_t bext, vec3_t bmin, vec3_t bmax);
float	M_RayPlaneIntersect(const vec3_t origin, const vec3_t dir, const vec3_t normal, float dist, vec3_t ipoint);
void	M_PointOnLine(const vec3_t origin, const vec3_t dir, float fraction, vec3_t out);
void	M_BarycentricToXYZ(float u, float v, const vec3_t tri[3], vec3_t out);
void	 M_ScalePlane(const plane_t *in, float scale, plane_t *out);
INLINE void	M_GetAxis(float anglex, float angley, float anglez, vec3_t axis[3]);
void	M_GetAxis4x3(float pitch, float roll, float yaw, vec4_t axis[3]);
void	M_MultiplyAxis(const vec3_t a[3], const vec3_t b[3], vec3_t out[3]);
void	M_InvertAxis(const vec3_t axis[3], vec3_t out[3]);
void    M_RotatePoint2d(const vec2_t point, const vec3_t axis[3], vec2_t out);
void	M_RotatePointAroundLine(const vec3_t in, const vec3_t aa, const vec3_t bb, const float angle, vec3_t out);
INLINE void	M_TransformPoint(const vec3_t point, const vec3_t pos, const vec3_t axis[3], vec3_t out);
INLINE void	M_TransformPointInverse(const vec3_t point, const vec3_t pos, const vec3_t axis[3], vec3_t out);
//rotates the bounding box points and creates a new axis-aligned bounding box out of it (new box may be larger than the original)
void	M_TransformBounds(const vec3_t bmin, const vec3_t bmax, const vec3_t pos, const vec3_t axis[3], vec3_t bmin_out, vec3_t bmax_out);
void	M_TransformPlane(const plane_t *plane, const vec3_t pos, const vec3_t axis[3], plane_t *out);
void    M_GetAxisFromForwardVec(const vec3_t forward, vec3_t axis[3]);
void	M_ClearAxis(vec3_t axis[3]);

bool	M_BoundsIntersect(const vec3_t bmin_a, const vec3_t bmax_a, const vec3_t bmin_b, const vec3_t bmax_b);
bool	M_2dBoundsIntersect(const vec2_t bmin_a, const vec2_t bmax_a, const vec2_t bmin_b, const vec2_t bmax_b);


void	M_Vec3toBVec3(const vec3_t in, bvec3_t out);
void	M_CopyBVec4(bvec4_t in, bvec4_t out);
float	M_GetVec2Angle(vec2_t vec);

float	M_Randnum (float min, float max);
unsigned int M_Sequence (unsigned int seed);

byte 	M_RangeCompress(const float f);
void 	M_RangeCompressVec3(const vec3_t in, bvec3_t out);
void 	M_Modulate(vec3_t lhs, const vec3_t rhs);

int		M_NormalToByte(vec3_t normal);
void	M_ByteToNormal(int normal, vec3_t out);
int		M_NormalToWord(vec3_t normal);
void	M_WordToNormal(int normal, vec3_t out);
void	M_ProjectDirOnBounds(const vec3_t dir, const vec3_t bmin, const vec3_t bmax, vec3_t out);

float 	M_Noise2(float x, float y);
float	M_CosLerp(float t, float a, float b);
void	M_LerpVec3(float amt, const vec3_t a, const vec3_t b, vec3_t out);
void	M_LerpAngleVec3(float amt, const vec3_t a, const vec3_t b, vec3_t out);
float	M_ClampLerp(float amount, float low, float high);
float	M_LerpAngle (float a, float low, float high);

void    M_InitSimpleNoise2();
float   M_SimpleNoise2(unsigned int x, unsigned int y);

bool	M_LineBoxIntersect3d(const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, float *fraction);
bool	M_LineRectIntersect(const vec2_t start, const vec2_t end, const vec2_t bmin, const vec2_t bmax, float *fraction);

//matrix math (4x3, unless otherwise specified in the function name)
INLINE void	M_Identity(matrix43_t *t);
void	M_Identity44(matrix44_t *t);
INLINE void	M_MultiplyMatrix(matrix43_t *a, matrix43_t *b, matrix43_t *out);
void	M_MultiplyMatrix44(matrix44_t *a, matrix44_t *b, matrix44_t *out);
INLINE void	M_VectorMatrixMult(const vec3_t point, const matrix43_t *mat, vec3_t out);
//void	M_InvertTransform(transform_t *in, transform_t *out);
//quaternion routines for skeletal animation
void	M_QuatToAxis(const vec4_t quat, vec3_t out[3]);
void	M_AxisToQuat(const vec3_t in[3], vec4_t out);
void	M_LerpQuat(float alpha, const vec4_t p, const vec4_t q, vec4_t out);
float	M_ClosestPointToSegment2d(const vec2_t src, const vec2_t dest, const vec2_t point, vec2_t point_closest);
bool	M_RayIntersectsLineSeg2d(const vec2_t src, const vec2_t dir, const vec2_t line_src, const vec2_t line_dest, float epsilon);

void HeadingFromAngles(float pitch, float yaw, vec3_t heading, vec3_t perp);

void    M_Init();

#include "savage_inlines.h"

#endif
 
