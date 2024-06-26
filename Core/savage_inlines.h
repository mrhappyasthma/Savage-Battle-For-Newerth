// (C) 2003 S2 Games

// savage_inlines.h


INLINE void	M_Identity(matrix43_t *t)
{
	t->axis[0][0] = 1;
	t->axis[0][1] = 0;
	t->axis[0][2] = 0;	
	t->axis[1][0] = 0;
	t->axis[1][1] = 1;
	t->axis[1][2] = 0;	
	t->axis[2][0] = 0;
	t->axis[2][1] = 0;
	t->axis[2][2] = 1;	
	t->pos[0] = 0;
	t->pos[1] = 0;
	t->pos[2] = 0;	
}

INLINE void M_MultiplyMatrix(matrix43_t *a, matrix43_t *b, matrix43_t *out)
{
   out->axis[0][0] = a->axis[0][0] * b->axis[0][0] + a->axis[1][0] * b->axis[0][1] + a->axis[2][0] * b->axis[0][2];
   out->axis[0][1] = a->axis[0][1] * b->axis[0][0] + a->axis[1][1] * b->axis[0][1] + a->axis[2][1] * b->axis[0][2];
   out->axis[0][2] = a->axis[0][2] * b->axis[0][0] + a->axis[1][2] * b->axis[0][1] + a->axis[2][2] * b->axis[0][2];
   
   out->axis[1][0] = a->axis[0][0] * b->axis[1][0] + a->axis[1][0] * b->axis[1][1] + a->axis[2][0] * b->axis[1][2];
   out->axis[1][1] = a->axis[0][1] * b->axis[1][0] + a->axis[1][1] * b->axis[1][1] + a->axis[2][1] * b->axis[1][2];
   out->axis[1][2] = a->axis[0][2] * b->axis[1][0] + a->axis[1][2] * b->axis[1][1] + a->axis[2][2] * b->axis[1][2];
   
   out->axis[2][0] = a->axis[0][0] * b->axis[2][0] + a->axis[1][0] * b->axis[2][1] + a->axis[2][0] * b->axis[2][2];
   out->axis[2][1] = a->axis[0][1] * b->axis[2][0] + a->axis[1][1] * b->axis[2][1] + a->axis[2][1] * b->axis[2][2];
   out->axis[2][2] = a->axis[0][2] * b->axis[2][0] + a->axis[1][2] * b->axis[2][1] + a->axis[2][2] * b->axis[2][2];
   
   out->pos[0] = a->axis[0][0] * b->pos[0] + a->axis[1][0] * b->pos[1] + a->axis[2][0] * b->pos[2] + a->pos[0];
   out->pos[1] = a->axis[0][1] * b->pos[0] + a->axis[1][1] * b->pos[1] + a->axis[2][1] * b->pos[2] + a->pos[1];
   out->pos[2] = a->axis[0][2] * b->pos[0] + a->axis[1][2] * b->pos[1] + a->axis[2][2] * b->pos[2] + a->pos[2];
   
}

//does the same thing as M_TransformPoint
INLINE void	M_VectorMatrixMult(const vec3_t point, const matrix43_t *mat, vec3_t out)
{
	vec3_t tmp;

	tmp[0] = point[0] * mat->axis[0][0] + point[1] * mat->axis[1][0] + point[2] * mat->axis[2][0];  //M_DotProduct(point, mat->axis[0]);
	tmp[1] = point[0] * mat->axis[0][1] + point[1] * mat->axis[1][1] + point[2] * mat->axis[2][1];  //M_DotProduct(point, mat->axis[1]);
	tmp[2] = point[0] * mat->axis[0][2] + point[1] * mat->axis[1][2] + point[2] * mat->axis[2][2];  //M_DotProduct(point, mat->axis[2]);

	out[0] = tmp[0] + mat->pos[0];
	out[1] = tmp[1] + mat->pos[1];
	out[2] = tmp[2] + mat->pos[2];
}

//this does the same thing as multiplying a 4x3 matrix by a vector
INLINE void	M_TransformPoint(const vec3_t point, const vec3_t pos, const vec3_t axis[3], vec3_t out)
{
	vec3_t tmp;

	tmp[0] = point[0] * axis[0][0] + point[1] * axis[1][0] + point[2] * axis[2][0];  //M_DotProduct(point, axis[0]);
	tmp[1] = point[0] * axis[0][1] + point[1] * axis[1][1] + point[2] * axis[2][1];  //M_DotProduct(point, axis[1]);
	tmp[2] = point[0] * axis[0][2] + point[1] * axis[1][2] + point[2] * axis[2][2];  //M_DotProduct(point, axis[2]);

	out[0] = tmp[0] + pos[0];
	out[1] = tmp[1] + pos[1];
	out[2] = tmp[2] + pos[2];
}



//inversely transforms the point
INLINE void	M_TransformPointInverse(const vec3_t point, const vec3_t pos, const vec3_t axis[3], vec3_t out)
{
	vec3_t tmp;

	tmp[0] = point[0] - pos[0];
	tmp[1] = point[1] - pos[1];
	tmp[2] = point[2] - pos[2];

	out[0] = M_DotProduct(tmp, axis[0]);
	out[1] = M_DotProduct(tmp, axis[1]);
	out[2] = M_DotProduct(tmp, axis[2]);
}

INLINE bool M_PointInBounds(const vec3_t point, const vec3_t bmin, const vec3_t bmax)
{
	int i;

	for (i=0; i<3; i++)
		if ((point[i] < bmin[i]) || (point[i] > bmax[i]))
			return false;

	return true;
}

INLINE void M_ClearBounds(vec3_t bmin, vec3_t bmax)
{
	bmin[0] = 999999;
	bmin[1] = 999999;
	bmin[2] = 999999;
	bmax[0] = -999999;
	bmax[1] = -999999;
	bmax[2] = -999999;
}

INLINE void M_ClearRect(vec3_t bmin, vec3_t bmax)
{
	bmin[0] = 999999;
	bmin[1] = 999999;	
	bmax[0] = -999999;
	bmax[1] = -999999;	
}

INLINE void	M_ClearRectI(ivec2_t bmin, ivec2_t bmax)
{
	bmin[0] = 999999;
	bmin[1] = 999999;	
	bmax[0] = -999999;
	bmax[1] = -999999;	

}

INLINE void M_AddPointToBounds(const vec3_t point, vec3_t bmin, vec3_t bmax)
{
	if (point[0] < bmin[0])
		bmin[0] = point[0];
	if (point[1] < bmin[1])
		bmin[1] = point[1];
	if (point[2] < bmin[2])
		bmin[2] = point[2];

	if (point[0] > bmax[0])
		bmax[0] = point[0];
	if (point[1] > bmax[1])
		bmax[1] = point[1];
	if (point[2] > bmax[2])
		bmax[2] = point[2];
}

INLINE void M_AddPointToRect(const vec2_t point, vec2_t bmin, vec2_t bmax)
{
	if (point[0] < bmin[0])
		bmin[0] = point[0];
	if (point[1] < bmin[1])
		bmin[1] = point[1];

	if (point[0] > bmax[0])
		bmax[0] = point[0];
	if (point[1] > bmax[1])
		bmax[1] = point[1];
}

INLINE void	M_AddRectToRectI(const ivec2_t bmin1, const ivec2_t bmax1, ivec2_t bmin2, ivec2_t bmax2)
{
	if (bmin1[0] < bmin2[0])
		bmin2[0] = bmin1[0];
	if (bmax1[0] > bmax2[0])
		bmax2[0] = bmax1[0];
	if (bmin1[1] < bmin2[1])
		bmin2[1] = bmin1[1];
	if (bmax1[1] > bmax2[1])
		bmax2[1] = bmax1[1];

}

INLINE bool M_RectInRect(const vec2_t bmin1, const vec2_t bmax1, const vec2_t bmin2, const vec2_t bmax2)
{
	if (bmin1[0] >= bmin2[0] && bmax1[0] <= bmax2[0])
		if (bmin1[1] >= bmin2[1] && bmax1[1] <= bmax2[1])
			return true;

	return false;
}

INLINE void M_CalcBoxExtents(const vec3_t bmin, const vec3_t bmax, vec3_t pos, vec3_t ext)
{	
	ext[0] = ((bmax[0] - bmin[0]) * 0.5);
	ext[1] = ((bmax[1] - bmin[1]) * 0.5);
	ext[2] = ((bmax[2] - bmin[2]) * 0.5);

	pos[0] = bmin[0] + ext[0];
	pos[1] = bmin[1] + ext[1];
	pos[2] = bmin[2] + ext[2];

}

INLINE void	M_CalcBoxMinMax(const vec3_t bpos, const vec3_t bext, vec3_t bmin, vec3_t bmax)
{
	bmin[0] = bpos[0] - bext[0] / 2;
	bmin[1] = bpos[1] - bext[1] / 2;
	bmin[2] = bpos[2] - bext[2] / 2;
	bmax[0] = bpos[0] + bext[0] / 2;
	bmax[1] = bpos[1] + bext[1] / 2;
	bmax[2] = bpos[2] + bext[2] / 2;
}

INLINE void M_SetVec3(vec3_t out, float x, float y, float z)
{
	out[0] = x;
	out[1] = y;
	out[2] = z;
}


INLINE void M_SetVec2(vec2_t out, float x, float y)
{
	out[0] = x;
	out[1] = y;
}

INLINE void M_FlipVec3(const vec3_t in, vec3_t out)
{
	out[0] = -in[0];
	out[1] = -in[1];
	out[2] = -in[2];
}

INLINE void M_DivVec3(const vec3_t a, float b, vec3_t out)
{
	out[0] = a[0] / b;
	out[1] = a[1] / b;
	out[2] = a[2] / b;
}


INLINE void M_MultVec3(const vec3_t a, float b, vec3_t out)
{
	out[0] = a[0] * b;
	out[1] = a[1] * b;
	out[2] = a[2] * b;
}

INLINE void M_MultVec2(const vec2_t a, float b, vec2_t out)
{
	out[0] = a[0] * b;
	out[1] = a[1] * b;
}

INLINE void	M_CopyVec2(const vec2_t in, vec2_t out)
{
	out[0] = in[0];
	out[1] = in[1];
}

INLINE void	M_CopyVec3(const vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

INLINE void	M_CopyVec4(const vec4_t in, vec4_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}


INLINE void	M_CrossProduct(const vec3_t a, const vec3_t b, vec3_t out)
{
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - a[0]*b[2];
	out[2] = a[0]*b[1] - a[1]*b[0];
}

INLINE float	M_GetVec3Length(vec3_t vec)
{
	return SQRT(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
}

INLINE float	M_SetVec3Length(vec3_t out, float length)
{
	float oldlength = M_Normalize(out);
	M_MultVec3(out, length, out);

	return oldlength;
}

INLINE float	M_GetVec2Length(vec2_t vec)
{
	return SQRT(vec[0]*vec[0] + vec[1]*vec[1]);
}

INLINE float	M_SetVec2Length(vec2_t out, float length)
{
	float oldlength = M_NormalizeVec2(out);
	M_MultVec2(out, length, out);

	return oldlength;
}

INLINE void	M_SurfaceNormal(const vec3_t a, const vec3_t b, const vec3_t c, vec3_t out)
{
	vec3_t u, v;

	M_SubVec3(b, a, u);
	M_SubVec3(c, a, v);
	
	M_CrossProduct(u, v, out);
	M_Normalize(out);
}

//returns a valid basis from euler angles
//uses YXZ euler angle ordering convention
//this corresponds to roll, then pitch, then yaw in the engine
//this is a RIGHT HANDED coordinate system
INLINE void	M_GetAxis(float pitch, float roll, float yaw, vec3_t axis[3])
{

#if 0
	//the slow but understandable version

	vec3_t rot_x[3];
	vec3_t rot_y[3];
	vec3_t rot_z[3];
	vec3_t tmpaxis[3];	
	float angle, s, c;

	angle = DEG2RAD(roll);
	s = sin(angle);
	c = cos(angle);

	MAKEMATRIX(rot_y,
		c, 0, -s,
		0, 1, 0,
		s, 0, c);

	angle = DEG2RAD(pitch);
	s = sin(angle);
	c = cos(angle);

	MAKEMATRIX(rot_x,
		1, 0, 0,
		0, c, s,
		0, -s, c);

	angle = DEG2RAD(yaw);
	s = sin(angle);
	c = cos(angle);

	MAKEMATRIX(rot_z,
		c, s, 0,
		-s, c, 0,
		0, 0, 1);

	M_MultiplyAxis(rot_y, rot_x, tmpaxis);
	M_MultiplyAxis(tmpaxis, rot_z, axis);
#endif

	float sr,cr, sp,cp, sy,cy;
	float sr_sp;
	float neg_cr_sp;
/*
	if (ABS(DEGSIN(roll) - sin(DEG2RAD(roll))) > 0.02)
		printf("DEGSIN error - DEGSIN(%f) is %f, sin(DEG2RAD(%f)) is %f\n", roll, DEGSIN(roll), roll, sin(DEG2RAD(roll)));
	
	if (ABS(DEGSIN(pitch) - sin(DEG2RAD(pitch))) > 0.02)
		printf("DEGSIN error - DEGSIN(%f) is %f, sin(DEG2RAD(%f)) is %f\n", pitch, DEGSIN(pitch), pitch, sin(DEG2RAD(pitch)));
	
	if (ABS(DEGSIN(yaw) - sin(DEG2RAD(yaw))) > 0.02)
		printf("DEGSIN error - DEGSIN(%f) is %f, sin(DEG2RAD(%f)) is %f\n", yaw, DEGSIN(yaw), yaw, sin(DEG2RAD(yaw)));
	
	if (ABS(DEGCOS(roll) - cos(DEG2RAD(roll))) > 0.02)
		printf("DEGCOS error - DEGCOS(%f) is %f, cos(DEG2RAD(%f)) is %f\n", roll, DEGCOS(roll), roll, cos(DEG2RAD(roll)));
	
	if (ABS(DEGCOS(pitch) - cos(DEG2RAD(pitch))) > 0.02)
		printf("DEGCOS error - DEGCOS(%f) is %f, cos(DEG2RAD(%f)) is %f\n", pitch, DEGCOS(pitch), pitch, cos(DEG2RAD(pitch)));
	
	if (ABS(DEGCOS(yaw) - cos(DEG2RAD(yaw))) > 0.02)
		printf("DEGCOS error - DEGCOS(%f) is %f, cos(DEG2RAD(%f)) is %f\n", yaw, DEGCOS(yaw), yaw, cos(DEG2RAD(yaw)));
	*/
	//roll = DEG2RAD(roll);
	//pitch = DEG2RAD(pitch);
	//yaw = DEG2RAD(yaw);
	
	sr = DEGSIN(roll);
	cr = DEGCOS(roll);
	sp = DEGSIN(pitch);
	cp = DEGCOS(pitch);
	sy = DEGSIN(yaw);
	cy = DEGCOS(yaw);

	/*
		this is the concatanated version of the matrix multiples (generated with the matrix.c util):

		axis[0][0] = ((cr * 1 + 0 * 0 + -sr * 0) * cy + (cr * 0 + 0 * cp + -sr * -sp) *
						-sy + (cr * 0 + 0 * sp + -sr * cp) * 0)
		axis[0][1] = ((cr * 1 + 0 * 0 + -sr * 0) * sy + (cr * 0 + 0 * cp + -sr * -sp) *
						cy + (cr * 0 + 0 * sp + -sr * cp) * 0)
		axis[0][2] = ((cr * 1 + 0 * 0 + -sr * 0) * 0 + (cr * 0 + 0 * cp + -sr * -sp) * 0
						+ (cr * 0 + 0 * sp + -sr * cp) * 1)

		axis[1][0] = ((0 * 1 + 1 * 0 + 0 * 0) * cy + (0 * 0 + 1 * cp + 0 * -sp) * -sy +
						(0 * 0 + 1 * sp + 0 * cp) * 0)
		axis[1][1] = ((0 * 1 + 1 * 0 + 0 * 0) * sy + (0 * 0 + 1 * cp + 0 * -sp) * cy + (
						0 * 0 + 1 * sp + 0 * cp) * 0)
		axis[1][2] = ((0 * 1 + 1 * 0 + 0 * 0) * 0 + (0 * 0 + 1 * cp + 0 * -sp) * 0 + (0
						* 0 + 1 * sp + 0 * cp) * 1)

		axis[2][0] = ((sr * 1 + 0 * 0 + cr * 0) * cy + (sr * 0 + 0 * cp + cr * -sp) * -s
						y + (sr * 0 + 0 * sp + cr * cp) * 0)
		axis[2][1] = ((sr * 1 + 0 * 0 + cr * 0) * sy + (sr * 0 + 0 * cp + cr * -sp) * cy
					 + (sr * 0 + 0 * sp + cr * cp) * 0)
		axis[2][2] = ((sr * 1 + 0 * 0 + cr * 0) * 0 + (sr * 0 + 0 * cp + cr * -sp) * 0 +
					(sr * 0 + 0 * sp + cr * cp) * 1)
	*/

	sr_sp = sr * sp;
	neg_cr_sp = -cr * sp;

	axis[0][0] = cr * cy + sr_sp * -sy;
	axis[0][1] = cr * sy + sr_sp * cy;
	axis[0][2] = -sr * cp;
	
	axis[1][0] = cp * -sy;
	axis[1][1] = cp * cy;
	axis[1][2] = sp;

	axis[2][0] = sr * cy + neg_cr_sp * -sy;
	axis[2][1] = sr * sy + neg_cr_sp * cy;
	axis[2][2] = cr * cp;
}

INLINE void	M_ClearVec3(vec3_t out)
{
	out[0] = out[1] = out[2] = 0;
}

INLINE void	M_ClearVec2(vec2_t out)
{
	out[0] = out[1] = 0;
}

INLINE void	M_RotateVec2(float ang, vec2_t out)
{
	//float angle = ang;
	vec2_t tmp = { 0.0, 0.0 };
	vec2_t in = { out[0], out[1] };

	//angle = angle * (M_PI / 180);
	tmp[0] = in[0]*DEGCOS(ang) + in[1]*DEGSIN(ang);
	tmp[1] = in[0]*-DEGSIN(ang) + in[1]*DEGCOS(ang);
	out[0]=tmp[0];out[1]=tmp[1];
}

INLINE void	M_CalcPlane(const vec3_t a, const vec3_t b, const vec3_t c, plane_t *out)
{
	vec3_t v1, v2;

	M_SubVec3(b, a, v1);
	M_SubVec3(c, a, v2);
	M_CrossProduct(v1, v2, out->normal);
	
	M_Normalize(out->normal);

    out->dist = M_DotProduct(out->normal, a);

//	M_GetPlaneType(out);
}