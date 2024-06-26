// (C) 2003 S2 Games


// savage_mathlib.c

// general 2d/3d math functions

#include "savage_common.h"
#include <float.h>

//#define FAST_SQRT


//from "Texturing and Modeling, a Procedural Approach", chapter 6

#define B 0x100
#define BM 0xff

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

static int p[B + B + 2];
static float g3[B + B + 2][3];
static float g2[B + B + 2][2];
static float g1[B + B + 2];

#define s_curve(t) ( t * t * (3. - 2. * t) )

#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.;


int sin_tmp;

float sinData[MAX_SINE_SAMPLES] = { 0.0 };

void    M_InitSinData()
{
	int i;
	//float f;

	for (i = 0; i < MAX_SINE_SAMPLES; i++)
	{
		sinData[i] = sin((float)i/MAX_SINE_SAMPLES * (M_PI * 2));
		//cosData[i] = cos((float)i/MAX_SINE_SAMPLES * (M_PI * 2));
	}

/*
	for (i = 0; i < 360 * 5; i++)
	{
		if (DEGSIN(i) - sin(DEG2RAD(i)) > 0.01)
		{
            printf("DEGSIN(%i) is %f, sin(DEG2RAD(%i)) is %f\n", i, DEGSIN(i), i, sin(DEG2RAD(i)));
        }
    }

    for (f = 0; f < 2 * M_PI; f += 0.000001)
    {
    	if (POSSIN(f) - sin(f) > 0.02)
    	{
    		printf("POSSIN(%f) is %f, sin(%f) is %f\n", f, POSSIN(f), f, sin(f));
		}
	}
*/
}


static void noise_init(void)
{
	int i, j, k;
	srand(0);

	for (i = 0 ; i < B ; i++) {
		p[i] = i;

		g1[i] = (float)((rand() % (B + B)) - B) / B;

		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (float)((rand() % (B + B)) - B) / B;
		M_NormalizeVec2(g2[i]);

		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (float)((rand() % (B + B)) - B) / B;
		M_Normalize(g3[i]);
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = rand() % B];
		p[j] = k;
	}

	for (i = 0 ; i < B + 2 ; i++) {
		p[B + i] = p[i];
		g1[B + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[B + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[B + i][j] = g3[i][j];
	}
}


float M_Noise2(float x, float y)
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register int i, j;
	vec2_t vec = { x,y };

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = g2[ b00 ] ; u = at2(rx0,ry0);
	q = g2[ b10 ] ; v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = g2[ b01 ] ; u = at2(rx0,ry1);
	q = g2[ b11 ] ; v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}

float lattice[128][128];

void    M_InitSimpleNoise2()
{
	int x,y;

	for (y=0; y<128; y++)
	{
		for (x=0; x<128; x++)
		{
			lattice[y][x] = M_Randnum(-1,1);
		}
	}
}

float   M_SimpleNoise2(unsigned int x, unsigned int y)
{   
	return lattice[y%128][x%128];
}

//float to int is currently defined in savage_common.h
/*
#ifdef _WIN32
__forceinline void FloatToInt(int *int_pointer, float f)
{
	__asm  fld  f
	__asm  mov  edx,int_pointer
	__asm  FRNDINT
	__asm  fistp dword ptr [edx];
}
#else
*/
//INLINE void FloatToInt(int *int_pointer, float f)
//{
//	*int_pointer = (int)f;
/*
	asm("fld  f
		 mov  edx,int_pointer
		 FRNDINT
		 fistp dword ptr %%edx");
*/
//}
//#endif

//fast sqrt() from nvidia's fastmath.cpp

#define FP_BITS(fp) (*(int *)&(fp))
#define FP_ABS_BITS(fp) (FP_BITS(fp)&0x7FFFFFFF)
#define FP_SIGN_BIT(fp) (FP_BITS(fp)&0x80000000)
#define FP_ONE_BITS 0x3F800000

static unsigned int fast_sqrt_table[0x10000];  // declare table of square roots 

typedef union FastSqrtUnion
{
  float f;
  unsigned int i;
} FastSqrtUnion;

void  build_sqrt_table()
{
  unsigned int i;
  FastSqrtUnion s;
  
  for (i = 0; i <= 0x7FFF; i++)
  {
    
    // Build a float with the bit pattern i as mantissa
    //  and an exponent of 0, stored as 127
    
    s.i = (i << 8) | (0x7F << 23);
    s.f = (float)sqrt(s.f);
    
    // Take the square root then strip the first 7 bits of
    //  the mantissa into the table
    
    fast_sqrt_table[i + 0x8000] = (s.i & 0x7FFFFF);
    
    // Repeat the process, this time with an exponent of 1, 
    //  stored as 128
    
    s.i = (i << 8) | (0x80 << 23);
    s.f = (float)sqrt(s.f);
    
    fast_sqrt_table[i] = (s.i & 0x7FFFFF);
  }
}


float fastsqrt(float n)
{
  if (FP_BITS(n) == 0)
    return 0.0;                 // check for square root of 0
  
  FP_BITS(n) = fast_sqrt_table[(FP_BITS(n) >> 8) & 0xFFFF] | ((((FP_BITS(n) - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);
  
  return n;
}





float	M_Normalize(vec3_t out)
{
	int n;
	float length, invlength;

	length = SQRT(out[0]*out[0] + out[1]*out[1] + out[2]*out[2]);
	if (length == 0)
	{
		out[0] = 0;
		out[1] = 0;
		out[2] = 0;
		return 0;
	}
	
	invlength = 1.0 / length;
	out[0] = out[0]*invlength;
	out[1] = out[1]*invlength;
	out[2] = out[2]*invlength;

	//clamp components between -1 and 1 (necessary because of precision issues with SQRT)
	for (n=0; n<3; n++)
	{
		if (out[n] > 1)
			out[n] = 1;
		else if (out[n] < -1)
			out[n] = -1;
	}

	return length;
}

float M_NormalizeVec2(vec2_t out)
{
	float length, invlength;
	int n;

	length = SQRT(out[0]*out[0] + out[1]*out[1]);
	if (length == 0)
	{
		out[0] = 0;
		out[1] = 0;		
		return 0;
	}
	
	invlength = 1.0 / length;
	out[0] = out[0]*invlength;
	out[1] = out[1]*invlength;	

	//clamp components between -1 and 1 (necessary because of precision issues with SQRT)
	for (n=0; n<2; n++)
	{
		if (out[n] > 1)
			out[n] = 1;
		else if (out[n] < -1)
			out[n] = -1;
	}

	return length;
}



void	M_MultiplyAxis(const vec3_t a[3], const vec3_t b[3], vec3_t out[3])
{
	out[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0];
	out[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1];
	out[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2];
	out[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0];
	out[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1];
	out[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2];
	out[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0];
	out[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1];
	out[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2];
}

#define MAKEMATRIX(m, m00, m01, m02, m10, m11, m12, m20, m21, m22) \
	m[0][0] = m00; \
	m[0][1] = m01; \
	m[0][2] = m02; \
	m[1][0] = m10; \
	m[1][1] = m11; \
	m[1][2] = m12; \
	m[2][0] = m20; \
	m[2][1] = m21; \
	m[2][2] = m22;




void	M_GetAxis4x3(float pitch, float roll, float yaw, vec4_t axis[3])
{
	int i,j;
	vec3_t axis3x3[3];

	M_GetAxis(pitch, roll, yaw, axis3x3);

	for (i=0; i<3; i++)
	{
		for (j=0; j<3; j++)
		{
			axis[i][j] = axis3x3[i][j];
		}
		axis[i][3] = 0.0;
	}	
}


void	M_InvertAxis(const vec3_t axis[3], vec3_t out[3])
{
	vec3_t tmp[3];

	memcpy(tmp, axis, sizeof(tmp));

	out[0][0] = tmp[0][0];
	out[0][1] = tmp[1][0];
	out[0][2] = tmp[2][0];
	out[1][0] = tmp[0][1];
	out[1][1] = tmp[1][1];
	out[1][2] = tmp[2][1];
	out[2][0] = tmp[0][2];
	out[2][1] = tmp[1][2];
	out[2][2] = tmp[2][2];
}

void	M_ClearAxis(vec3_t axis[3])
{
	axis[0][0] = 1;
	axis[0][1] = 0;
	axis[0][2] = 0;
	axis[1][0] = 0;
	axis[1][1] = 1;
	axis[1][2] = 0;
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
}

float	M_GetDistanceSq(const vec3_t pos1, const vec3_t pos2)
{
	vec3_t diff;

	M_SubVec3(pos1, pos2, diff);
	return M_DotProduct(diff, diff);
}

float	M_GetDistance(const vec3_t pos1, const vec3_t pos2)
{
	return sqrt(M_GetDistanceSq(pos1, pos2));
}

float	M_GetDistanceSqVec2(const vec2_t pos1, const vec2_t pos2)
{
	vec2_t diff;

	M_SubVec2(pos1, pos2, diff);
	return M_DotProduct2(diff, diff);
}

float	M_DistToLineSegment(const vec3_t p, const vec3_t seg_a, const vec3_t seg_b)
{
	float c1,c2,b;
	vec3_t v,w;
	vec3_t pb;

	M_SubVec3(seg_b, seg_a, v);
	M_SubVec3(p, seg_a, w);

	c1 = M_DotProduct(w,v);
	if (c1 <= 0)
	{
		return M_GetDistance(p, seg_a);
	}

	c2 = M_DotProduct(v,v);
	if (c2 <= c1)
	{
		return M_GetDistance(p, seg_b);
	}

	b = c1/c2;
	M_PointOnLine(seg_a, v, b, pb);

	return M_GetDistance(p, pb);
}

/*
void	M_GetAxis(float pitch, float roll, float yaw, vec3_t axis[3])
{
	float c1,c2,c3;
	float s1,s2,s3;

	pitch = DEG2RAD(pitch);
	roll = DEG2RAD(roll);
	yaw = DEG2RAD(yaw);

	c1 = cos(yaw);		
	s1 = sin(yaw);		
	c2 = cos(pitch);		
	s2 = sin(pitch);
	c3 = cos(roll);
	s3 = sin(roll);
	axis[0][0] = c1 * c2;
	axis[0][1] = s1 * c2;
	axis[0][2] = -s2;
	axis[1][0] = -(s1 * c3)+(c1 * s2 * s3);
	axis[1][1] = (c1*c3) + (s1 * s2 * s3);
	axis[1][2] = c2 * s3;
	axis[2][0] = (s1 * s3) + (c1 * s2 * c3);
	axis[2][1] = -(c1 * s3) + (s1 * s2 * c3);
	axis[2][2] = c2*c3;
}
*/




void	M_RotatePoint2d(const vec2_t point, const vec3_t axis[3], vec2_t out)
{
	out[0] = M_DotProduct2(point, axis[0]);
	out[1] = M_DotProduct2(point, axis[1]);
}
/*
void	M_RotatePointAroundLine(const vec3_t in, const vec3_t aa, const vec3_t bb, const float angle, vec3_t out)
{
	vec3_t	a, b;
	float	r1[9], r2[9], r3[9];
	float	c, s;
	float	len;
	vec2_t	v;

	M_CopyVec3(in, out);
	M_CopyVec3(aa, a);
	M_CopyVec3(bb, b);

	//translate line and point to origin
	M_SubVec3(a, b, a);
	M_SetVec3(b, 0, 0, 0);
	M_SubVec3(out, b, out);

	//rotate line and point around z axis to zy plane
	len = M_GetVec2Length(a);
	c = a[0] / len;
	s = a[1] / len;
	r1[0] = c;	r1[1] = s;	r1[2] = 0;
	r1[3] = -s;	r1[4] = c;	r1[5] = 0;
	r1[6] = 0;	r1[7] = 0;	r1[8] = 1;

	M_MultiplyVecByMatrix3x3(a, r1, a);
	M_MultiplyVecByMatrix3x3(out, r1, out);

	//rotate around x axis to xy plane (line is now y axis aligned)
	v[0] = a[0];
	v[1] = a[2];
	len = M_GetVec2Length(v);
	c = a[0] / len;
	s = a[2] / len;
	r2[0] = 1;	r2[1] = 0;	r2[2] = 0;
	r2[3] = 0;	r2[4] = c;	r2[5] = s;
	r2[6] = 0;	r2[7] = -s;	r2[8] = c;

	M_MultiplyVecByMatrix3x3(a, r2, a);
	M_MultiplyVecByMatrix3x3(out, r2, out);

	//rotate point around y axis
	c = cos(angle);
	s = sin(angle);
	r3[0] = c;	r3[1] = 0;	r3[2] = s;
	r3[3] = 0;	r3[4] = 1;	r3[5] = 0;
	r3[6] = -s;	r3[7] = 0;	r3[8] = c;
	M_MultiplyVecByMatrix3x3(out, r3, out);

	//undo x axis rotation
	r2[5] = -r2[5];
	r2[7] = -r2[7];
	M_MultiplyVecByMatrix3x3(out, r2, out);

	//undo z axis rotation
	r1[1] = -r1[1];
	r1[3] = -r1[3];
	M_MultiplyVecByMatrix3x3(out, r1, out);

	//untranslate
	M_AddVec3(out, b, out);
}
*/
void	M_TransformPlane(const plane_t *plane, const vec3_t pos, const vec3_t axis[3], plane_t *out)
{
	vec3_t point;

	M_MultVec3(plane->normal, plane->dist, point);						//get a point on the plane
	M_TransformPoint(point, pos, axis, point);							//transform it
	M_TransformPoint(plane->normal, zero_vec, axis, out->normal);		//rotate normal
	M_Normalize(out->normal);											//this step probably isn't necessary but it might help with numerical imprecision
	out->dist = M_DotProduct(point, out->normal);						//recalc plane distance using new normal and point
}


//000 corresponds to lower-left-back octant
//111 corresponds to upper-right-front octant
#define OCT_000		0
#define OCT_100		1
#define OCT_010		2
#define OCT_110		3
#define OCT_001		4
#define OCT_101		5
#define OCT_011		6
#define OCT_111		7

//returns octant of vector (for box on plane side test)
int M_GetOctant (const vec3_t nml)
{
	int result = 0;

	if (nml[0] < 0)
		result += 1;
	if (nml[1] < 0)
		result += 2;
	if (nml[2] < 0)
		result += 4;

	return result;
}


//fixme: precompute octant

//returns:
//  0 - outside plane
//  1 - inside plane
//  2 - intersecting plane
int M_BoxOnPlaneSide (const vec3_t bmin, const vec3_t bmax, plane_t *p)
{
	vec3_t	n_vtx, p_vtx; //negative and positive vertices

	
	if (p->normal[0] > 0)
	{
		p_vtx[0] = bmax[0];
		n_vtx[0] = bmin[0];
	}
	else
	{
		p_vtx[0] = bmin[0];
		n_vtx[0] = bmax[0];
	}

	if (p->normal[1] > 0)
	{
		p_vtx[1] = bmax[1];
		n_vtx[1] = bmin[1];
	}
	else
	{
		p_vtx[1] = bmin[1];
		n_vtx[1] = bmax[1];
	}

	if (p->normal[2] > 0)
	{
		p_vtx[2] = bmax[2];
		n_vtx[2] = bmin[2];
	}
	else
	{
		p_vtx[2] = bmin[2];
		n_vtx[2] = bmax[2];
	}

	if (M_DotProduct(p->normal, p_vtx) - p->dist < 0)   //outside plane?
		return 0;
	else if (M_DotProduct(p->normal, n_vtx) - p->dist > 0)    //inside plane?
		return 1;
	else return 2;   //intersects plane
}

int M_OBBOnPlaneSide (const vec3_t bmin, const vec3_t bmax,  const vec3_t pos, const vec3_t axis[3], plane_t *plane)
{
	vec3_t	n_vtx, p_vtx; //negative and positive vertices
	vec3_t normal;

	//transform the plane normal into the space of the box so we can work out the P and N vertices
	normal[0] = M_DotProduct(plane->normal, axis[0]);
	normal[1] = M_DotProduct(plane->normal, axis[1]);
	normal[2] = M_DotProduct(plane->normal, axis[2]);

	if (normal[0] > 0)
	{
		p_vtx[0] = bmax[0];
		n_vtx[0] = bmin[0];
	}
	else
	{
		p_vtx[0] = bmin[0];
		n_vtx[0] = bmax[0];
	}

	if (normal[1] > 0)
	{
		p_vtx[1] = bmax[1];
		n_vtx[1] = bmin[1];
	}
	else
	{
		p_vtx[1] = bmin[1];
		n_vtx[1] = bmax[1];
	}

	if (normal[2] > 0)
	{
		p_vtx[2] = bmax[2];
		n_vtx[2] = bmin[2];
	}
	else
	{
		p_vtx[2] = bmin[2];
		n_vtx[2] = bmax[2];
	}

	//we need the N and P vertices in world space
	M_TransformPoint(p_vtx, pos, axis, p_vtx);
	M_TransformPoint(n_vtx, pos, axis, n_vtx);

	if (M_DotProduct(plane->normal, p_vtx) - plane->dist < 0)   //outside plane?
		return 0;
	else if (M_DotProduct(plane->normal, n_vtx) - plane->dist > 0)    //inside plane?
		return 1;
	else return 2;   //intersects plane
}


void	M_SurfaceVec(const vec3_t a, const vec3_t b, const vec3_t c, vec3_t out)
{
	vec3_t u, v;

	M_SubVec3(b, a, u);
	M_SubVec3(c, a, v);

	M_CrossProduct(u, v, out);
}

/*
void	M_Translate(float m[16], float x, float y, float z)
{
	m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
	m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
	m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
	m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];	
}
*/




void	M_Identity44(matrix44_t *t)
{
	t->t.axis[0][0] = 1;
	t->t.axis[0][1] = 0;
	t->t.axis[0][2] = 0;
	t->t.axis[0][3] = 0;
	t->t.axis[1][0] = 0;
	t->t.axis[1][1] = 1;
	t->t.axis[1][2] = 0;
	t->t.axis[1][3] = 0;
	t->t.axis[2][0] = 0;
	t->t.axis[2][1] = 0;
	t->t.axis[2][2] = 1;
	t->t.axis[2][3] = 0;
	t->t.pos[0] = 0;
	t->t.pos[1] = 0;
	t->t.pos[2] = 0;
	t->t.pos[3] = 1;
}






void M_MultiplyMatrix44(matrix44_t *a, matrix44_t *b, matrix44_t *out)
{
   out->t.axis[0][0] = a->t.axis[0][0] * b->t.axis[0][0] + a->t.axis[1][0] * b->t.axis[0][1] + a->t.axis[2][0] * b->t.axis[0][2] + a->t.pos[0] * b->t.axis[0][3];
   out->t.axis[0][1] = a->t.axis[0][1] * b->t.axis[0][0] + a->t.axis[1][1] * b->t.axis[0][1] + a->t.axis[2][1] * b->t.axis[0][2] + a->t.pos[1] * b->t.axis[0][3];
   out->t.axis[0][2] = a->t.axis[0][2] * b->t.axis[0][0] + a->t.axis[1][2] * b->t.axis[0][1] + a->t.axis[2][2] * b->t.axis[0][2] + a->t.pos[2] * b->t.axis[0][3];
   out->t.axis[0][3] = a->t.axis[0][3] * b->t.axis[0][0] + a->t.axis[1][3] * b->t.axis[0][1] + a->t.axis[2][3] * b->t.axis[0][2] + a->t.pos[3] * b->t.axis[0][3];
   out->t.axis[1][0] = a->t.axis[0][0] * b->t.axis[1][0] + a->t.axis[1][0] * b->t.axis[1][1] + a->t.axis[2][0] * b->t.axis[1][2] + a->t.pos[0] * b->t.axis[1][3];
   out->t.axis[1][1] = a->t.axis[0][1] * b->t.axis[1][0] + a->t.axis[1][1] * b->t.axis[1][1] + a->t.axis[2][1] * b->t.axis[1][2] + a->t.pos[1] * b->t.axis[1][3];
   out->t.axis[1][2] = a->t.axis[0][2] * b->t.axis[1][0] + a->t.axis[1][2] * b->t.axis[1][1] + a->t.axis[2][2] * b->t.axis[1][2] + a->t.pos[2] * b->t.axis[1][3];
   out->t.axis[1][3] = a->t.axis[0][3] * b->t.axis[1][0] + a->t.axis[1][3] * b->t.axis[1][1] + a->t.axis[2][3] * b->t.axis[1][2] + a->t.pos[3] * b->t.axis[1][3];
   out->t.axis[2][0] = a->t.axis[0][0] * b->t.axis[2][0] + a->t.axis[1][0] * b->t.axis[2][1] + a->t.axis[2][0] * b->t.axis[2][2] + a->t.pos[0] * b->t.axis[2][3];
   out->t.axis[2][1] = a->t.axis[0][1] * b->t.axis[2][0] + a->t.axis[1][1] * b->t.axis[2][1] + a->t.axis[2][1] * b->t.axis[2][2] + a->t.pos[1] * b->t.axis[2][3];
   out->t.axis[2][2] = a->t.axis[0][2] * b->t.axis[2][0] + a->t.axis[1][2] * b->t.axis[2][1] + a->t.axis[2][2] * b->t.axis[2][2] + a->t.pos[2] * b->t.axis[2][3];
   out->t.axis[2][3] = a->t.axis[0][3] * b->t.axis[2][0] + a->t.axis[1][3] * b->t.axis[2][1] + a->t.axis[2][3] * b->t.axis[2][2] + a->t.pos[3] * b->t.axis[2][3];
   out->t.pos[0]	 = a->t.axis[0][0] * b->t.pos[0] + a->t.axis[1][0] * b->t.pos[1] + a->t.axis[2][0] * b->t.pos[2] + a->t.pos[0] * b->t.pos[3];
   out->t.pos[1]	 = a->t.axis[0][1] * b->t.pos[0] + a->t.axis[1][1] * b->t.pos[1] + a->t.axis[2][1] * b->t.pos[2] + a->t.pos[1] * b->t.pos[3];
   out->t.pos[2]	 = a->t.axis[0][2] * b->t.pos[0] + a->t.axis[1][2] * b->t.pos[1] + a->t.axis[2][2] * b->t.pos[2] + a->t.pos[2] * b->t.pos[3];
   out->t.pos[3]	 = a->t.axis[0][3] * b->t.pos[0] + a->t.axis[1][3] * b->t.pos[1] + a->t.axis[2][3] * b->t.pos[2] + a->t.pos[3] * b->t.pos[3];
}

/*
void	M_MultiplyTransforms(transform_t *a, transform_t *b, transform_t *out)
{
   out->matrix[0]  = a->matrix[0] * b->matrix[0] + a->matrix[3] * b->matrix[1] + a->matrix[6] * b->matrix[2];
   out->matrix[1]  = a->matrix[1] * b->matrix[0] + a->matrix[4] * b->matrix[1] + a->matrix[7] * b->matrix[2];
   out->matrix[2]  = a->matrix[2] * b->matrix[0] + a->matrix[5] * b->matrix[1] + a->matrix[8] * b->matrix[2];
   
   out->matrix[3]  = a->matrix[0] * b->matrix[3] + a->matrix[3] * b->matrix[4] + a->matrix[6] * b->matrix[5];
   out->matrix[4]  = a->matrix[1] * b->matrix[3] + a->matrix[4] * b->matrix[4] + a->matrix[7] * b->matrix[5];
   out->matrix[5]  = a->matrix[2] * b->matrix[3] + a->matrix[5] * b->matrix[4] + a->matrix[8] * b->matrix[5];

   out->matrix[6]  = a->matrix[0] * b->matrix[6] + a->matrix[3] * b->matrix[7] + a->matrix[6] * b->matrix[8];
   out->matrix[7]  = a->matrix[1] * b->matrix[6] + a->matrix[4] * b->matrix[7] + a->matrix[7] * b->matrix[8];
   out->matrix[8]  = a->matrix[2] * b->matrix[6] + a->matrix[5] * b->matrix[7] + a->matrix[8] * b->matrix[8];

   out->matrix[9]  = a->matrix[0] * b->matrix[9] + a->matrix[3] * b->matrix[10] + a->matrix[6] * b->matrix[11] + a->matrix[9];
   out->matrix[10] = a->matrix[1] * b->matrix[9] + a->matrix[4] * b->matrix[10] + a->matrix[7] * b->matrix[11] + a->matrix[10];
   out->matrix[11] = a->matrix[2] * b->matrix[9] + a->matrix[5] * b->matrix[10] + a->matrix[8] * b->matrix[11] + a->matrix[11];
   
}*/





//from NVIDIA mathlib
/*
    calculate the determinent of a 2x2 matrix in the from

    | a1 a2 |
    | b1 b2 |

*/
/*
float M_Det2x2(float a1, float a2, float b1, float b2)
{
    return a1 * b2 - b1 * a2;
}
*/
//from NVIDIA mathlib
/*
    calculate the determinent of a 3x3 matrix in the form

    | a1 a2 a3 |
    | b1 b2 b3 |
    | c1 c2 c3 |

*//*
float M_Det3x3(float a1, float a2, float a3, 
                         float b1, float b2, float b3, 
                         float c1, float c2, float c3)
{
    return a1 * M_Det2x2(b2, b3, c2, c3) - b1 * M_Det2x2(a2, a3, c2, c3) + c1 * M_Det2x2(a2, a3, b2, b3);
}
*/
//from NVIDIA mathlib
/*
void M_InvertTransform(transform_t *in, transform_t *out)
{
    float det,oodet;

    out->f.f00 =  M_Det3x3(in->f.f11, in->f.f21, in->f.f31, in->f.f12, in->f.f22, in->f.f32, in->f.f13, in->f.f23, in->f.f33);
    out->f.f10 = -M_Det3x3(in->f.f10, in->f.f20, in->f.f30, in->f.f12, in->f.f22, in->f.f32, in->f.f13, in->f.f23, in->f.f33);
    out->f.f20 =  M_Det3x3(in->f.f10, in->f.f20, in->f.f30, in->f.f11, in->f.f21, in->f.f31, in->f.f13, in->f.f23, in->f.f33);
    out->f.f30 = -M_Det3x3(in->f.f10, in->f.f20, in->f.f30, in->f.f11, in->f.f21, in->f.f31, in->f.f12, in->f.f22, in->f.f32);

    out->f.f01 = -M_Det3x3(in->f.f01, in->f.f21, in->f.f31, in->f.f02, in->f.f22, in->f.f32, in->f.f03, in->f.f23, in->f.f33);
    out->f.f11 =  M_Det3x3(in->f.f00, in->f.f20, in->f.f30, in->f.f02, in->f.f22, in->f.f32, in->f.f03, in->f.f23, in->f.f33);
    out->f.f21 = -M_Det3x3(in->f.f00, in->f.f20, in->f.f30, in->f.f01, in->f.f21, in->f.f31, in->f.f03, in->f.f23, in->f.f33);
    out->f.f31 =  M_Det3x3(in->f.f00, in->f.f20, in->f.f30, in->f.f01, in->f.f21, in->f.f31, in->f.f02, in->f.f22, in->f.f32);

    out->f.f02 =  M_Det3x3(in->f.f01, in->f.f11, in->f.f31, in->f.f02, in->f.f12, in->f.f32, in->f.f03, in->f.f13, in->f.f33);
    out->f.f12 = -M_Det3x3(in->f.f00, in->f.f10, in->f.f30, in->f.f02, in->f.f12, in->f.f32, in->f.f03, in->f.f13, in->f.f33);
    out->f.f22 =  M_Det3x3(in->f.f00, in->f.f10, in->f.f30, in->f.f01, in->f.f11, in->f.f31, in->f.f03, in->f.f13, in->f.f33);
    out->f.f32 = -M_Det3x3(in->f.f00, in->f.f10, in->f.f30, in->f.f01, in->f.f11, in->f.f31, in->f.f02, in->f.f12, in->f.f32);

    out->f.f03 = -M_Det3x3(in->f.f01, in->f.f11, in->f.f21, in->f.f02, in->f.f12, in->f.f22, in->f.f03, in->f.f13, in->f.f23);
    out->f.f13 =  M_Det3x3(in->f.f00, in->f.f10, in->f.f20, in->f.f02, in->f.f12, in->f.f22, in->f.f03, in->f.f13, in->f.f23);
    out->f.f23 = -M_Det3x3(in->f.f00, in->f.f10, in->f.f20, in->f.f01, in->f.f11, in->f.f21, in->f.f03, in->f.f13, in->f.f23);
    out->f.f33 =  M_Det3x3(in->f.f00, in->f.f10, in->f.f20, in->f.f01, in->f.f11, in->f.f21, in->f.f02, in->f.f12, in->f.f22);

    det = (in->f.f00 * out->f.f00) + (in->f.f01 * out->f.f10) + (in->f.f02 * out->f.f20) + (in->f.f03 * out->f.f30);

    oodet = 1 / det;

    out->f.f00 *= oodet;
    out->f.f10 *= oodet;
    out->f.f20 *= oodet;
    out->f.f30 *= oodet;

    out->f.f01 *= oodet;
    out->f.f11 *= oodet;
    out->f.f21 *= oodet;
    out->f.f31 *= oodet;

    out->f.f02 *= oodet;
    out->f.f12 *= oodet;
    out->f.f22 *= oodet;
    out->f.f32 *= oodet;

    out->f.f03 *= oodet;
    out->f.f13 *= oodet;
    out->f.f23 *= oodet;
    out->f.f33 *= oodet;
}*/
void M_QuatToAxis(const vec4_t quat, vec3_t m[3])
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = quat[X] + quat[X]; y2 = quat[Y] + quat[Y]; 
	z2 = quat[Z] + quat[Z];
	xx = quat[X] * x2;   xy = quat[X] * y2;   xz = quat[X] * z2;
	yy = quat[Y] * y2;   yz = quat[Y] * z2;   zz = quat[Z] * z2;
	wx = quat[W] * x2;   wy = quat[W] * y2;   wz = quat[W] * z2;

	m[0][0] = 1.0 - (yy + zz); 	m[1][0] = xy - wz;
	m[2][0] = xz + wy;		

	m[0][1] = xy + wz;		m[1][1] = 1.0 - (xx + zz);
	m[2][1] = yz - wx;		

	m[0][2] = xz - wy;		m[1][2] = yz + wx;
	m[2][2] = 1.0 - (xx + yy);
}
/*	float wx, wy, wz, xx, yy, yz, xy, xz, zz;

	xx      = quat[X] * quat[X];
    xy      = X * Y;
    xz      = X * Z;
    xw      = X * W;

    yy      = Y * Y;
    yz      = Y * Z;
    yw      = Y * W;

    zz      = Z * Z;
    zw      = Z * W;

    m[0]  = 1 - 2 * ( yy + zz );
    m[1]  =     2 * ( xy - zw );
    m[2]  =     2 * ( xz + yw );

    m[4]  =     2 * ( xy + zw );
    m[5]  = 1 - 2 * ( xx + zz );
    m[6]  =     2 * ( yz - xw );

    m[8]  =     2 * ( xz - yw );
    m[9]  =     2 * ( yz + xw );
    m[10] = 1 - 2 * ( xx + yy );

    m[3]  = m[7] = m[11 = m[12] = m[13] = m[14] = 0;
    m[15] = 1;*/

/*
void M_QuatToAxis(const vec4_t in, vec3_t out[3])
{
    float fTx  = 2.0f*in[X];
    float fTy  = 2.0f*in[Y];
    float fTz  = 2.0f*in[Z];
    float fTwx = fTx*in[W];
    float fTwy = fTy*in[W];
    float fTwz = fTz*in[W];
    float fTxx = fTx*in[X];
    float fTxy = fTy*in[X];
    float fTxz = fTz*in[X];
    float fTyy = fTy*in[Y];
    float fTyz = fTz*in[Y];
    float fTzz = fTz*in[Z];

    out[0][0] = 1.0f-(fTyy+fTzz);
    out[0][1] = fTxy-fTwz;
    out[0][2] = fTxz+fTwy;
	
    out[1][0] = fTxy+fTwz;
    out[1][1] = 1.0f-(fTxx+fTzz);
    out[1][2] = fTyz-fTwx;
	
    out[2][0] = fTxz-fTwy;
    out[2][1] = fTyz+fTwx;
    out[2][2] = 1.0f-(fTxx+fTyy);	
}*/
/*
//from NVIDIA mathlib
void M_QuatToMatrix(const vec4_t quat, transform_t *out)
{
	float wx,wy,wz,xy,yz,xz;

    float x2 = quat[X] * quat[X] * 2.0;
    float y2 = quat[Y] * quat[Y] * 2.0;
    float z2 = quat[Z] * quat[Z] * 2.0;

    out->f.f00 = 1.0 - y2 - z2;
    out->f.f11 = 1.0 - x2 - z2;
    out->f.f22 = 1.0 - x2 - y2;

    wz = quat[W]*quat[Z];
	xy = quat[X]*quat[Y];        
    out->f.f01 = 2.0 * (xy - wz);
    out->f.f10 = 2.0 * (xy + wz);

    wx = quat[W]*quat[X];
	yz = quat[Y]*quat[Z];
    out->f.f12 = 2.0 * (yz - wx);
    out->f.f21 = 2.0 * (yz + wx);

    wy = quat[W]*quat[Y];
	xz = quat[X]*quat[Z];
    out->f.f02 = 2.0 * (xz + wy);
    out->f.f20 = 2.0 * (xz - wy);    
}

*/
/*
//from NVIDIA mathlib
void M_AxisToQuat(const vec3_t in[3], vec4_t out)
{
    float tr = in[0][0] + in[1][1] + in[2][2];
      
    // check the diagonal
    if ( tr > 0.0) 
    {
        // Diagonal is positive.
        float s = sqrt(tr + 1.0);
        out[W] = s * 0.5;
        s = 0.5 / s;
        out[X] = (in[2][1] - in[1][2]) * s;
        out[Y] = (in[0][2] - in[2][0]) * s;
        out[Z] = (in[1][0] - in[0][1]) * s;
    } // have positive trace
    else
    {                
		int i,j,k,s;
        // Diagonal is negative.
        int nxt[3] = {1, 2, 0};
   
        // Find the largest diagonal.
        i = 0;
        if ( in[1][1] > in[0][0] )
            i = 1;
        if ( in[2][2] > in[i][i] )
            i = 2;
        j = nxt[i];
        k = nxt[j];
      
        s = sqrt( in[i][i] - ( in[j][j] + in[k][k] )) + 1.0;
      
        out[i] = s * 0.5;
        s = 0.5 / s;
      
        out[W]  = (in[k][j] - in[j][k]) * s;
        out[j] = (in[j][i] + in[i][j]) * s;
        out[k] = (in[k][i] + in[i][k]) * s;
    } // have negative or zero trace    
}
*/

void M_AxisToQuat(const vec3_t m[3], vec4_t quat)
{
  float  tr, s, q[4];
  int    i, j, k;


  int nxt[3] = {1, 2, 0};


  tr = m[0][0] + m[1][1] + m[2][2];


  // check the diagonal
  if (tr > 0.0) {
    s = SQRT (tr + 1.0);
    quat[W] = s / 2.0;
    s = 0.5 / s;
    quat[X] = (m[1][2] - m[2][1]) * s;
    quat[Y] = (m[2][0] - m[0][2]) * s;
    quat[Z] = (m[0][1] - m[1][0]) * s;
} else {		
	 // diagonal is negative
    	  i = 0;
          if (m[1][1] > m[0][0]) i = 1;
	     if (m[2][2] > m[i][i]) i = 2;
            j = nxt[i];
            k = nxt[j];


            s = SQRT ((m[i][i] - (m[j][j] + m[k][k])) + 1.0);
      
	     q[i] = s * 0.5;
            
            if (s != 0.0) s = 0.5 / s;


	    q[3] = (m[j][k] - m[k][j]) * s;
            q[j] = (m[i][j] + m[j][i]) * s;
            q[k] = (m[i][k] + m[k][i]) * s;


	  quat[X] = q[0];
	  quat[Y] = q[1];
	  quat[Z] = q[2];
	  quat[W] = q[3];
  }
}



bool M_BoundsIntersect(const vec3_t bmin_a, const vec3_t bmax_a, const vec3_t bmin_b, const vec3_t bmax_b)
{
	int n;

	for (n=0; n<3; n++)
	{
		if (bmin_a[n] > bmax_b[n] || bmin_b[n] > bmax_a[n])
			return false;
	}
	
	return true;
}

bool M_2dBoundsIntersect(const vec2_t bmin_a, const vec2_t bmax_a, const vec2_t bmin_b, const vec2_t bmax_b)
{
	int n;

	for (n=0; n<2; n++)
	{
		if (bmin_a[n] > bmax_b[n] || bmin_b[n] > bmax_a[n])
			return false;
	}
	
	return true;
}



//from graphics gems I, p736 
//(with a correction located at http://www.acm.org/tog/GraphicsGems/Errata.GraphicsGems)
bool M_RayBoxIntersect(const vec3_t origin, const vec3_t dir, const vec3_t bmin, const vec3_t bmax, vec3_t out)
{
	bool inside = true;
	char quadrant[3];
	int i;
	int whichplane;
	vec3_t max_t;
	vec3_t	candidateplane;
	vec3_t	candidatenml;
	vec3_t nml;

	for (i=0; i<3; i++)
		if (origin[i] < bmin[i])
		{
			quadrant[i] = 1;  //left
			candidateplane[i] = bmin[i];
			candidatenml[i] = -1;
			inside = false;
		}
		else if (origin[i] > bmax[i])
		{
			quadrant[i] = 0;
			candidateplane[i] = bmax[i];
			candidatenml[i] = 1;
			inside = false;
		}
		else
			quadrant[i] = 2;

	if (inside)
	{
		//point is inside box
		out[0] = origin[0];
		out[1] = origin[1];
		out[2] = origin[2];
		return true;
	}

	for (i=0; i<3; i++)
		if (quadrant[i] != 2 && dir[i] != 0)
			max_t[i] = (candidateplane[i]-origin[i]) / dir[i];
		else
			max_t[i] = -1;

	whichplane = 0;
	for (i=1; i<3; i++)
		if (max_t[whichplane] < max_t[i])
			whichplane = i;

	if (max_t[whichplane] < 0) return false;
	for (i=0; i<3; i++)
		if (whichplane != i)
		{
			out[i] = origin[i] + max_t[whichplane] * dir[i];
			nml[i] = 0;
			if (out[i] < bmin[i] || out[i] > bmax[i])
				return false;
		}
		else
		{
			out[i] = candidateplane[i];
			nml[i] = candidatenml[i];
		}
	
	return true;
}


//same function, expanded upon to return the normal at the intersection point
//returns: 0 - didn't intersect, 1 - intersected, 2 - origin inside box
int M_RayBoxIntersect2(const vec3_t origin, const vec3_t dir, const vec3_t bmin, const vec3_t bmax, vec3_t out, vec3_t nml)
{
	bool inside = true;
	char quadrant[3];
	int i;
	int whichplane;
	vec3_t max_t;
	vec3_t	candidateplane;
	vec3_t	candidatenml;


	for (i=0; i<3; i++)
		if (origin[i] < bmin[i])
		{
			quadrant[i] = 1;  //left
			candidateplane[i] = bmin[i];
			candidatenml[i] = -1;
			inside = false;
		}
		else if (origin[i] > bmax[i])
		{
			quadrant[i] = 0;
			candidateplane[i] = bmax[i];
			candidatenml[i] = 1;
			inside = false;
		}
		else
		{
			quadrant[i] = 2;
			candidatenml[i] = 0;
		}

	if (inside)
	{
		//point is inside box
		out[0] = origin[0];
		out[1] = origin[1];
		out[2] = origin[2];
		return 2;
	}

	for (i=0; i<3; i++)
		if (quadrant[i] != 2 && dir[i] != 0)
			max_t[i] = (candidateplane[i]-origin[i]) / dir[i];
		else
			max_t[i] = -1;

	whichplane = 0;
	for (i=1; i<3; i++)
		if (max_t[whichplane] < max_t[i])
			whichplane = i;

	if (max_t[whichplane] < 0) return 0;
	for (i=0; i<3; i++)
		if (whichplane != i)
		{
			out[i] = origin[i] + max_t[whichplane] * dir[i];
			nml[i] = 0;
			if (out[i] < bmin[i] || out[i] > bmax[i])
				return 0;
		}
		else
		{
			out[i] = candidateplane[i];
			nml[i] = candidatenml[i];
		}
	
	return 1;
}

void	M_Vec3toBVec3(const vec3_t in, bvec3_t out)
{
	out[0] = in[0] * 255;
	out[1] = in[1] * 255;
	out[2] = in[2] * 255;
}

void	M_CopyBVec4(bvec4_t in, bvec4_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

float	M_GetVec2Angle(vec2_t vec)
{
	vec2_t norm;
	float angle;

	M_CopyVec2(vec, norm);
	if (!M_NormalizeVec2(norm))
		return 0;
	
	if (vec[1] < 0)
		angle = RAD2DEG(asin(norm[0])) + 180;
	else
		angle = RAD2DEG(asin(-norm[0]));

	if (angle < 0)
		angle += 360;

	return angle;
}

/*
 * simple random number generator
 * randNum - Return a random floating point number such that
 *      (min <= return-value <= max)
 * 32,767 values are possible for any given range.
 */
float M_Randnum (float min, float max)
{
	int r;
    float	x;
    
	r = rand ();
    x = (float)(r & 0x7fff) / (float)0x7fff;
    return (x * (max - min) + min);
}

/** 
 * here is a nice pseudo-random # generator, aka sequence generator
 * linear congruential generator.  Generator x[n+1] = a * x[n] mod m 
 */

//seed it with the previous value in the sequence
unsigned int M_Sequence (unsigned int seed)
{
/* The following parameters are recommended settings based on research
   uncomment the one you want. */

/* static unsigned int a = 1588635695, m = 4294967291U, q = 2, r = 1117695901; */
/* static unsigned int a = 1223106847, m = 4294967291U, q = 3, r = 625646750;*/
/* static unsigned int a = 279470273, m = 4294967291U, q = 15, r = 102913196;*/
   static unsigned int a = 1583458089, m = 2147483647, q = 1, r = 564025558;
/* static unsigned int a = 784588716, m = 2147483647, q = 2, r = 578306215;  */
/* static unsigned int a = 16807, m = 2147483647, q = 127773, r = 2836;      */
/* static unsigned int a = 950706376, m = 2147483647, q = 2, r = 246070895;  */

   seed = a*(seed % q) - r*(seed / q);
   return (unsigned int)(((double)seed / (double)m) * MAX_UINT);
}

//rotates the bounding box points and creates a new axis-aligned bounding box out of it (new box may be larger than the original)
void	M_TransformBounds(const vec3_t bmin, const vec3_t bmax, const vec3_t pos, const vec3_t axis[3], vec3_t bmin_out, vec3_t bmax_out)
{
	vec3_t trans_bmin,trans_bmax;
	vec3_t p;

	M_ClearBounds(trans_bmin,trans_bmax);

	M_TransformPoint(vec3(bmin[0],bmin[1],bmin[2]), pos, axis, p);
	M_AddPointToBounds(p,trans_bmin,trans_bmax);
	M_TransformPoint(vec3(bmin[0],bmin[1],bmax[2]), pos, axis, p);
	M_AddPointToBounds(p,trans_bmin,trans_bmax);
	M_TransformPoint(vec3(bmin[0],bmax[1],bmin[2]), pos, axis, p);
	M_AddPointToBounds(p,trans_bmin,trans_bmax);
	M_TransformPoint(vec3(bmin[0],bmax[1],bmax[2]), pos, axis, p);
	M_AddPointToBounds(p,trans_bmin,trans_bmax);
	M_TransformPoint(vec3(bmax[0],bmin[1],bmin[2]), pos, axis, p);
	M_AddPointToBounds(p,trans_bmin,trans_bmax);
	M_TransformPoint(vec3(bmax[0],bmin[1],bmax[2]), pos, axis, p);
	M_AddPointToBounds(p,trans_bmin,trans_bmax);
	M_TransformPoint(vec3(bmax[0],bmax[1],bmin[2]), pos, axis, p);
	M_AddPointToBounds(p,trans_bmin,trans_bmax);
	M_TransformPoint(vec3(bmax[0],bmax[1],bmax[2]), pos, axis, p);
	M_AddPointToBounds(p,trans_bmin,trans_bmax);

	M_CopyVec3(trans_bmin,bmin_out);
	M_CopyVec3(trans_bmax,bmax_out);
//	M_TransformPoint(bmin, pos, axis, trans_bmin);
//	M_TransformPoint(bmax, pos, axis, trans_bmax);
	
//	M_ClearBounds(bmin_out, bmax_out);
//	M_AddPointToBounds(trans_bmin,bmin_out,bmax_out);
//	M_AddPointToBounds(trans_bmax,bmin_out,bmax_out);
}



void M_BarycentricToXYZ(float u, float v, const vec3_t tri[3], vec3_t out)
{
	vec3_t a, b, c;

	M_MultVec3(tri[0], 1-u-v, a);
	M_MultVec3(tri[1], u, b);
	M_MultVec3(tri[2], v, c);

	M_AddVec3(a, b, out);
	M_AddVec3(out, c, out);
}

// intersection test for ray with plane
float M_RayPlaneIntersect(const vec3_t origin, const vec3_t dir, const vec3_t normal, float dist, vec3_t ipoint)
{
	float t, denom;

	denom = M_DotProduct(normal, dir);
//	if (ABS(denom) < EPSILON)
//		return 0;

	t = (M_DotProduct(normal, origin) - dist) / denom;

	if (t < 0)
		return 0;

	M_MultVec3(dir, t, ipoint);
	M_AddVec3(origin, ipoint, ipoint);

	return t;
}

void	M_RayPoint(const vec3_t origin, const vec3_t dir, float t, vec3_t out)
{
	M_MultVec3(dir, t, out);
	M_AddVec3(origin, out, out);
}

byte M_RangeCompress(const float f)
{ 
	return (f+1)*127.5;
}

void M_RangeCompressVec3(const vec3_t in, bvec3_t out)
{ 
	out[0] = M_RangeCompress(in[0]);
	out[1] = M_RangeCompress(in[1]);
	out[2] = M_RangeCompress(in[2]);
}

void M_Modulate(vec3_t lhs, const vec3_t rhs)
{
	lhs[0] *= rhs[0]; 
	lhs[1] *= rhs[1]; 
	lhs[2] *= rhs[2]; 
}

//used for scaling collision surface polyhedrons
void M_ScalePlane(const plane_t *in, float scale, plane_t *out)
{
	vec3_t point;

	point[0] = in->dist * in->normal[0];
	point[1] = in->dist * in->normal[1];
	point[2] = in->dist * in->normal[2];

	M_MultVec3(point, scale, point);
	out->dist = M_DotProduct(point, in->normal);
	M_CopyVec3(in->normal, out->normal);
}

bool	M_CompareVec2(const vec2_t a, const vec2_t b)
{
	int i;

	for (i=0; i<2; i++)
		if (fabs(a[i] - b[i]) > 0.001)
			return false;

	return true;
}

bool	M_CompareVec3(const vec3_t a, const vec3_t b)
{
	int i;

	for (i=0; i<3; i++)
		if (fabs(a[i] - b[i]) > 0.001)
			return false;

	return true;
}

void	M_PointOnLine(const vec3_t origin, const vec3_t dir, float time, vec3_t out)
{
	out[0] = origin[0] + dir[0] * time;
	out[1] = origin[1] + dir[1] * time;
	out[2] = origin[2] + dir[2] * time;
}

void	M_PointOnProjectilePath(const vec3_t origin, const vec3_t init_velocity, float gravity, float time, vec3_t out)
{
	out[0] = origin[0] + init_velocity[0] * time;
	out[1] = origin[1] + init_velocity[1] * time;
	out[2] = (origin[2] + init_velocity[2] * time) - (0.5 * time * gravity * gravity);
}

int		M_NormalToByte(vec3_t normal)
{
	float phi,theta;
	int lo,hi;
	float s = SQRT(normal[0] * normal[0] + normal[1] * normal[1]);
	if (s==0)
		s = EPSILON;

	//convert cartesian coords to spherical coords and encode the result in the byte
	phi = acos(normal[2]);
	if (normal[0] >= 0)
		theta = asin(normal[1] / s);
	else
		theta = M_PI - asin(normal[1] / s);
	if (theta < 0)
		theta += 2*M_PI;

	//encode phi and theta into the high and low parts of the byte
	phi = (phi / M_PI) * 15.0;
	theta = (theta / (2*M_PI)) * 15.0;
	
	lo = iround(phi);
	hi = iround(theta);

	return lo + (hi << 4);
}

void	M_ByteToNormal(int normal, vec3_t out)
{
	float phi,theta;
	float sin_phi;

	phi = normal & ~0xF0;
	theta = normal >> 4;

	phi = (phi / 15.0) * M_PI;
	theta = (theta / 15.0) * (M_PI * 2);
	
	sin_phi = SIN(phi);

	out[0] = sin_phi * COS(theta);
	out[1] = sin_phi * SIN(theta);
	out[2] = COS(phi);
}

int		M_NormalToWord(vec3_t normal)
{
	float phi,theta;
	int lo,hi;
	float s = SQRT(normal[0] * normal[0] + normal[1] * normal[1]);
	if (s==0)
		s = EPSILON;

	//convert cartesian coords to spherical coords and encode the result in the byte
	phi = acos(normal[2]);
	if (normal[0] >= 0)
		theta = asin(normal[1] / s);
	else
		theta = M_PI - asin(normal[1] / s);
	if (theta < 0)
		theta += 2*M_PI;

	//encode phi and theta into the high and low parts of the word
	phi = (phi / M_PI) * 255.0;
	theta = (theta / (2*M_PI)) * 255.0;
	
	lo = iround(phi);
	hi = iround(theta);

	return lo + (hi << 8);
}

void	M_WordToNormal(int normal, vec3_t out)
{
	float phi,theta;
	float sin_phi;

	phi = normal & ~0xFF00;
	theta = normal >> 8;

	phi = (phi / 255.0) * M_PI;
	theta = (theta / 255.0) * (M_PI * 2);
	
	sin_phi = SIN(phi);

	out[0] = sin_phi * COS(theta);
	out[1] = sin_phi * SIN(theta);
	out[2] = COS(phi);
}

//projects a direction vector (must be normalized) onto a bounding box, as if the vector originated at the center of the box
void	M_ProjectDirOnBounds(const vec3_t dir, const vec3_t bmin, const vec3_t bmax, vec3_t out)
{
	vec3_t pos;
	vec3_t ext;
	float lowest_t = 999999;
	int axis = 0;
	int n;
	float t;

	M_CalcBoxExtents(bmin, bmax, pos, ext);

	//find the first plane intersection
	for (n=0; n<3; n++)
	{
		if (dir[n])
		{
			t = ABS(ext[n] / dir[n]);
		}
		else
		{
			t = 999999;
		}

		if (t < lowest_t)
		{
			lowest_t = t;
			axis = n;
		}
	}
	
	M_PointOnLine(pos, dir, lowest_t, out);
}

float	M_CosLerp(float t, float a, float b)
{
	float f = (1 - cos(t * M_PI)) * 0.5;
	return a * (1-f) + b*f;
}

void	M_LerpVec3(float amt, const vec3_t a, const vec3_t b, vec3_t out)
{
	out[0] = LERP(amt, a[0], b[0]);
	out[1] = LERP(amt, a[1], b[1]);
	out[2] = LERP(amt, a[2], b[2]);
}

void	M_LerpAngleVec3(float amt, const vec3_t a, const vec3_t b, vec3_t out)
{
	out[0] = M_LerpAngle(amt, a[0], b[0]);
	out[1] = M_LerpAngle(amt, a[1], b[1]);
	out[2] = M_LerpAngle(amt, a[2], b[2]);
}


void	M_LerpQuat(float amt, const vec4_t from, const vec4_t to, vec4_t res)
{
	vec4_t to1;
	double omega, cosom, sinom, scale0, scale1;

	// calc cosine
	cosom = from[X] * to[X] + from[Y] * to[Y] + from[Z] * to[Z] + from[W] * to[W];

	// adjust signs (if necessary)
	if(cosom < 0.0)
	{
		cosom = -cosom;
		to1[0] = - to[X];
		to1[1] = - to[Y];
		to1[2] = - to[Z];
		to1[3] = - to[W];
	}
	else
	{
		to1[0] = to[X];
		to1[1] = to[Y];
		to1[2] = to[Z];
		to1[3] = to[W];
	}

	// calculate coefficients
	if((1.0 - cosom) > EPSILON)
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = SIN(omega);
		scale0 = SIN((1.0 - amt) * omega) / sinom;
		scale1 = SIN(amt * omega) / sinom;
	}
	else
	{      
		// "from" and "to" quaternions are very close 
		//  ... so we can do a linear interpolation
		scale0 = 1.0 - amt;
		scale1 = amt;
	}

	// calculate final values
	res[X] = scale0 * from[X] + scale1 * to1[0];
	res[Y] = scale0 * from[Y] + scale1 * to1[1];
	res[Z] = scale0 * from[Z] + scale1 * to1[2];
	res[W] = scale0 * from[W] + scale1 * to1[3];
}
/*
//from NVIDIA mathlib
void	M_LerpQuat(float alpha, const vec4_t p, const vec4_t q, vec4_t out)
{
	int bflip;
	float beta, omega;
	float one_over_sin_omega;
	vec4_t r;

	float cos_omega = p[0] * q[0] + p[1] * q[1] + p[2] * q[2] + p[3] * q[3];
	// if B is on opposite hemisphere from A, use -B instead

	if ( ( bflip = (cos_omega < 0)) )
		cos_omega = -cos_omega;

	// complementary interpolation parameter
	beta = 1.0 - alpha;     

	if(cos_omega <= 1.0 - EPSILON)
		return;

	omega = acos(cos_omega);
	one_over_sin_omega = 1.0 / sin(omega);

	beta    = sin(omega*beta)  * one_over_sin_omega;
	alpha   = sin(omega*alpha) * one_over_sin_omega;

	if (bflip)
		alpha = -alpha;

	r[0] = beta * p[0]+ alpha * q[0];
	r[1] = beta * p[1]+ alpha * q[1];
	r[2] = beta * p[2]+ alpha * q[2];
	r[3] = beta * p[3]+ alpha * q[3];	
}

*/
bool	M_LineBoxIntersect3d(const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, float *fraction)
{
	vec3_t v;
	vec3_t t0 = { 0,0,0 };
	vec3_t t1 = { 1,1,1 };
	float enter, exit;
	int i;

	if (M_PointInBounds(start, bmin, bmax))
	{
		*fraction = 0;
		return true;
	}

	M_SubVec3(end, start, v);

	if (v[0] == 0 && v[1] == 0 && v[2] == 0)
	{
		//line is a point outside the box
		return false;
	}

	//get first times of overlapping axes	
	for (i=0; i<3; i++)
	{
		bool cont;
		
		cont = false;

		if (v[i] == 0)
		{
			if (start[i] > bmax[i] || start[i] < bmin[i])
			{
				return false;		//this axis is outside the box
			}
			else
				continue;
		}

		if (v[i] < 0)
		{
			t0[i] = (bmax[i]-start[i]) / v[i];
			t1[i] = (bmin[i]-start[i]) / v[i];
			cont = true;
		}
		else
		{
			t0[i] = (bmin[i]-start[i]) / v[i];
			t1[i] = (bmax[i]-start[i]) / v[i];
			cont = true;
		}
		
		if (!cont)
			return false;
	}

	enter = MAX(t0[X], MAX(t0[Y], t0[Z]));
	exit = MIN(t1[X], MIN(t1[Y], t1[Z]));

	if (enter <= exit)
	{
		if (enter < 1)
		{
			*fraction = enter;
			return true;
		}
		else
		{

		}
	}

	return false;
}

bool	M_PointInBounds2d(const vec2_t p, const vec2_t bmin, const vec2_t bmax)
{
	int i;

	for (i=0; i<2; i++)
	{
		if ((p[i] < bmin[i]) || (p[i] > bmax[i]))		
			return false;		
	}

	return true;
}

bool	M_LineRectIntersect(const vec2_t start, const vec2_t end, const vec2_t bmin, const vec2_t bmax, float *fraction)
{
	vec2_t v;
	vec2_t t0 = { 0,0 };
	vec2_t t1 = { 1,1 };
	float first_t, last_t;
	int i;

	if (M_PointInBounds2d(start, bmin, bmax))
	{
		*fraction = 0;
		return true;
	}
	
	M_SubVec2(end, start, v);

	if (v[0] == 0 && v[1] == 0)
	{
		//line is a point outside the box
		return false;
	}

	//get first times of overlapping axes	
	for (i=0; i<2; i++)
	{
		bool cont;
		
		cont = false;

		if (v[i] == 0)
		{
			if (start[i] > bmax[i] || start[i] < bmin[i])
			{
				return false;		//this axis is outside the box
			}
			else
				continue;
		}

		if (v[i] < 0)
		{
			t0[i] = (bmax[i]-start[i]) / v[i];
			t1[i] = (bmin[i]-start[i]) / v[i];
			cont = true;
		}
		else
		{
			t0[i] = (bmin[i]-start[i]) / v[i];
			t1[i] = (bmax[i]-start[i]) / v[i];
			cont = true;
		}
		
		if (!cont)
			return false;
	}

	first_t = MAX(t0[X], t0[Y]);
	last_t = MIN(t1[X], t1[Y]);

	if (first_t <= last_t)
	{
		if (first_t < 1 && first_t >= 0)
		{
			*fraction = first_t;
			return true;
		}
	}

	return false;
}


void	M_GetAxisFromForwardVec(const vec3_t forward, vec3_t axis[3])
{
	vec3_t up = { UP == 0 ? 1 : 0, UP == 1 ? 1 : 0, UP == 2 ? 1 : 0 };
	
	M_CopyVec3(forward, axis[FORWARD]);
	M_Normalize(axis[FORWARD]);
	
	M_CrossProduct(axis[FORWARD], up, axis[RIGHT]);	//right vector
	M_Normalize(axis[RIGHT]);
	
	//recompute object up
	M_CrossProduct(axis[RIGHT], axis[FORWARD], axis[UP]); //up vector
	M_Normalize(axis[UP]);
}

float	M_ClampLerp(float amount, float low, float high)
{
	amount = CLAMP(amount, 0, 1);
	return LERP(amount, low, high);
}

//lerp across the smallest arc
float M_LerpAngle (float a, float low, float high) 
{
	float ret;

//	low-=180;
//	high-=180;

	if (high - low > 180)
	{
		low += 360;
	}
	if (high - low < -180)
	{
		high += 360;
	}

	ret = LERP(a, low, high);
	if (ret < 0)
		ret += 360;
	if (ret > 360)
		ret -= 360;

	return ret;
}

#ifndef MACRO_MATH
float M_DotProduct (const vec3_t v1, const vec3_t v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void M_AddVec3(const vec3_t a, const vec3_t b, vec3_t out)
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}

void M_SubVec3(const vec3_t a, const vec3_t b, vec3_t out)
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}

void M_SubVec2(const vec2_t a, const vec2_t b, vec2_t out)
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
}

bool M_BoxExtentsIntersect(const vec3_t pos_a, const vec3_t ext_a, const vec3_t pos_b, const vec3_t ext_b)
{
	//perform a separating axis test
	
	vec3_t t;
	
	M_SubVec3(pos_b, pos_a, t);

	return	ABS(t[0]) <= (ext_a[0] + ext_b[0]) &&
			ABS(t[1]) <= (ext_a[1] + ext_b[1]) &&
			ABS(t[2]) <= (ext_a[2] + ext_b[2]);
}
#endif

float	M_ClosestPointToSegment2d(const vec2_t src, const vec2_t dest, const vec2_t point, vec2_t point_closest)
{
    vec2_t diff;
	float t;
	vec2_t dir;

	M_SubVec2(dest, src, dir);
	M_SubVec2(point, src, diff);
    t = M_DotProduct2(diff, dir);

    if ( t <= 0.0f )
    {
		M_CopyVec2(src, point_closest);
    }
    else
    {
		float length = M_VEC2LENSQR(dir);
        if ( t >= length )
        {
			M_CopyVec2(dest, point_closest);
        }
        else
        {
            t /= length;
			M_MultVec2(dir, t, dir);
            M_SubVec2(diff, dir, diff);
			M_AddVec2(src, dir, point_closest);
        }
    }

    return M_GetDistanceSqVec2(point, point_closest);
}

bool M_RayIntersectsLineSeg2d(const vec2_t src, const vec2_t dir, const vec2_t line_src, const vec2_t line_dest, float epsilon)
{
	const float distance = dir[0]*src[1] - dir[1]*src[0];

	const float distance_src = distance + dir[1]*line_src[0] - dir[0]*line_src[1];
	const float distance_dest   = distance + dir[1]*line_dest[0] - dir[0]*line_dest[1];

	return   ( distance_src > -epsilon && distance_dest <  epsilon )
		  || ( distance_src <  epsilon && distance_dest > -epsilon );

}

void HeadingFromAngles(float pitch, float yaw, vec3_t heading, vec3_t perp)
{
	vec3_t axis[3];
	M_GetAxis(pitch, 0, yaw, axis);
	if (heading)
		M_CopyVec3(axis[FORWARD], heading);
	if (perp)
		M_CopyVec3(axis[RIGHT], perp);
}

void    M_Init()
{
	M_InitSinData();
	noise_init();
	build_sqrt_table();
}
