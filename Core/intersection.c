// (C) 2002 S2 Games

// intersection.c

// geometry intersection routines
// repurposed from david eberly's source code (www.magic-software.com)
// all this code has to be waaaaay optimized for our specific engine case (i.e. AABBs only)


#include "core.h"

#define	M_CROSS(a,b,out) M_CrossProduct(b,a,out)

//----------------------------------------------------------------------------
static void I_ProjectTriangle (const vec3_t rkD, const vec3_t apkTri[3],
    float *rfMin, float *rfMax)
{
	float fDot;

    *rfMin = M_DotProduct(rkD, apkTri[0]);
    *rfMax = *rfMin;

    fDot = M_DotProduct(rkD, apkTri[1]);
    if ( fDot < *rfMin )
        *rfMin = fDot;
    else if ( fDot > *rfMax )
        *rfMax = fDot;

    fDot = M_DotProduct(rkD, apkTri[2]);
    if ( fDot < *rfMin )
        *rfMin = fDot;
    else if ( fDot > *rfMax )
        *rfMax = fDot;
}

//----------------------------------------------------------------------------
static void I_ProjectBox (const vec3_t rkD, const obb_t *rkBox, float *rfMin,
    float *rfMax)
{
    float fDdC = M_DotProduct(rkD, rkBox->center);
    float fR =
        rkBox->extents[0]*ABS(M_DotProduct(rkD,rkBox->axis[0])) +
		rkBox->extents[1]*ABS(M_DotProduct(rkD,rkBox->axis[1])) +
		rkBox->extents[2]*ABS(M_DotProduct(rkD,rkBox->axis[2]));
        
    *rfMin = fDdC - fR;
    *rfMax = fDdC + fR;
}
/*
//----------------------------------------------------------------------------
bool Mgc::TestIntersection (const Vector3* apkTri[3], const Box3& rkBox)
{
    Real fMin0, fMax0, fMin1, fMax1;
    Vector3 kD, akE[3];

    // test direction of triangle normal
    akE[0] = (*apkTri[1]) - (*apkTri[0]);
    akE[1] = (*apkTri[2]) - (*apkTri[0]);
    kD = akE[0].Cross(akE[1]);
    fMin0 = kD.Dot(*apkTri[0]);
    fMax0 = fMin0;
    ProjectBox(kD,rkBox,fMin1,fMax1);
    if ( fMax1 < fMin0 || fMax0 < fMin1 )
        return false;

    // test direction of box faces
    for (int i = 0; i < 3; i++)
    {
        kD = rkBox.Axis(i);
        ProjectTriangle(kD,apkTri,fMin0,fMax0);
        Real fDdC = kD.Dot(rkBox.Center());
        fMin1 = fDdC - rkBox.Extent(i);
        fMax1 = fDdC + rkBox.Extent(i);
        if ( fMax1 < fMin0 || fMax0 < fMin1 )
            return false;
    }

    // test direction of triangle-box edge cross products
    akE[2] = akE[1] - akE[0];
    for (int i0 = 0; i0 < 3; i0++)
    {
        for (int i1 = 0; i1 < 3; i1++)
        {
            kD = akE[i0].Cross(rkBox.Axis(i1));
            ProjectTriangle(kD,apkTri,fMin0,fMax0);
            ProjectBox(kD,rkBox,fMin1,fMax1);
            if ( fMax1 < fMin0 || fMax0 < fMin1 )
                return false;
        }
    }

    return true;
}
*/


//----------------------------------------------------------------------------
static bool I_NoBoxTriIntersect (float fTMax, float fSpeed, float fMin0, float fMax0,
    float fMin1, float fMax1, float *rfTFirst, float *rfTLast)
{
    float fInvSpeed, fT;

    if ( fMax1 < fMin0 )  // C1 initially on left of C0
    {
        if ( fSpeed <= 0.0 )
        {
            // intervals moving apart
            return true;
        }

        fInvSpeed = 1.0/fSpeed;

        fT = (fMin0 - fMax1)*fInvSpeed;
        if ( fT > *rfTFirst )
            *rfTFirst = fT;
        if ( *rfTFirst > fTMax )
            return true;

        fT = (fMax0 - fMin1)*fInvSpeed;
        if ( fT < *rfTLast )
            *rfTLast = fT;
        if ( *rfTFirst > *rfTLast )
            return true;
    }
    else if ( fMax0 < fMin1 )  // C1 initially on right of C0
    {
        if ( fSpeed >= 0.0f )
        {
            // intervals moving apart
            return true;
        }

        fInvSpeed = 1.0f/fSpeed;

        fT = (fMax0 - fMin1)*fInvSpeed;
        if ( fT > *rfTFirst )
            *rfTFirst = fT;
        if ( *rfTFirst > fTMax )
            return true;

        fT = (fMin0 - fMax1)*fInvSpeed;
        if ( fT < *rfTLast )
            *rfTLast = fT;
        if ( *rfTFirst > *rfTLast )
            return true;
    }
    else  // C0 and C1 overlap
    {
        if ( fSpeed > 0.0f )
        {
            fT = (fMax0 - fMin1)/fSpeed;
            if ( fT < *rfTLast )
                *rfTLast = fT;
            if ( *rfTFirst > *rfTLast )
                return true;
        }
        else if ( fSpeed < 0.0f )
        {
            fT = (fMin0 - fMax1)/fSpeed;
            if ( fT < *rfTLast )
                *rfTLast = fT;
            if ( *rfTFirst > *rfTLast )
                return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
bool I_BoxTriIntersect (const vec3_t apkTri[3],
    const vec3_t bpos, const vec3_t bext, const vec3_t rkBoxVel, float fTMax, float *rfTFirst,
    float *rfTLast)
{
	int i, i0, i1;
    float fMin0, fMax0, fMin1, fMax1, fSpeed;
    vec3_t kD, akE[3];
	vec3_t kW;
	obb_t rkBox;
	
	SET_VEC3(rkBox.axis[0], 1,0,0);
	SET_VEC3(rkBox.axis[1], 0,1,0);
	SET_VEC3(rkBox.axis[2], 0,0,1);
	M_CopyVec3(bpos, rkBox.center);
	M_CopyVec3(bext, rkBox.extents);

	M_CopyVec3(rkBoxVel, kW);
    
    *rfTFirst = 0.0f;
    *rfTLast = FAR_AWAY;

    // test direction of triangle normal
	M_SubVec3(apkTri[1], apkTri[0], akE[0]);
	M_SubVec3(apkTri[2], apkTri[0], akE[1]);
    
	M_CROSS(akE[0], akE[1], kD);
    fMin0 = M_DotProduct(kD, apkTri[0]);
    fMax0 = fMin0;
    
    I_ProjectBox(kD,&rkBox,&fMin1,&fMax1);
	fSpeed = M_DotProduct(kD, kW);
    
    if ( I_NoBoxTriIntersect(fTMax,fSpeed,fMin0,fMax0,fMin1,fMax1,rfTFirst,rfTLast) )
        return false;

    // test direction of box faces
    for (i = 0; i < 3; i++)
    {
		float fDdC;

		M_CopyVec3(rkBox.axis[i], kD);
        
        I_ProjectTriangle(kD,apkTri,&fMin0,&fMax0);
		fDdC = M_DotProduct(kD, rkBox.center);

        fMin1 = fDdC - rkBox.extents[i];
        fMax1 = fDdC + rkBox.extents[i];
		fSpeed = M_DotProduct(kD, kW);
        
        if ( I_NoBoxTriIntersect(fTMax,fSpeed,fMin0,fMax0,fMin1,fMax1,rfTFirst,
             rfTLast) )
        {
            return false;
        }
    }

    // test direction of triangle-box edge cross products
	M_SubVec3(akE[1], akE[0], akE[2]);
    
	for (i0 = 0; i0 < 3; i0++)
	{
		for (i1 = 0; i1 < 3; i1++)
		{
			M_CROSS(akE[i0], rkBox.axis[i1], kD);
			I_ProjectTriangle(kD, apkTri, &fMin0, &fMax0);
			I_ProjectBox(kD, &rkBox, &fMin1, &fMax1);
			fSpeed = M_DotProduct(kD, kW);
			if (I_NoBoxTriIntersect(fTMax, fSpeed, fMin0, fMax0, fMin1, fMax1, rfTFirst, rfTLast))
			{
				return false;
			}
		}
	}
    
    return true;
}
/*
//reduction of the moving bounds intersection algo for a line and a box
bool	I_LineBoundsIntersect(const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, float *fraction)
{
	vec3_t v;
	vec3_t t0 = { 0,0,0 };
	vec3_t t1 = { 1,1,1 };
	float first_t,last_t;
	int i;

	if (M_PointInBounds(start, bmin, bmax))
	{
		*fraction = 0;
		return true;
	}

	M_SubVec3(end, start, v);

	if (v[0] == 0 && v[1] == 0 && v[2] == 0)
	{
		//line is a point
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

		if (end[i] < bmax[i] && v[i] < 0)
		{
			t0[i] = (bmax[i] - start[i]) / v[i];
			cont = true;
		}
		else if (end[i] > bmin[i] && v[i] > 0)
		{
			t0[i] = (bmin[i] - start[i]) / v[i];
			cont = true;
		}
		
		if (!cont)
			return false;
	}

	first_t = MAX(t0[X], t0[Y], t0[Z]));
	last_t = MIN(t1[X], MIN(t1[Y], t1[Z]));

	if (first_t <= last_t)
	{
		*fraction = first_t;
		return true;
	}

	return false;
}
*/
bool	I_MovingBoundsIntersect(vec3_t start, vec3_t end, vec3_t bmin_a, vec3_t bmax_a, vec3_t bmin_b, vec3_t bmax_b, float *fraction)
{
	vec3_t v;
	vec3_t t0 = { 0,0,0 };
	vec3_t t1 = { 1,1,1 };
	float first_t,last_t;
	int i;

	if (M_BoundsIntersect(bmin_a, bmax_a, bmin_b, bmax_b))
	{
		*fraction = 0;
		return true;
	}

	M_SubVec3(start, end, v);

	if (v[0] == 0 && v[1] == 0 && v[2] == 0)
	{
		//velocity is zero and we already tested the bounds
		return false;
	}

	//get first times of overlapping axes
	//the boxes only intersect when all extents overlap
	for (i=0; i<3; i++)
	{
		bool cont;
		
		cont = false;

		if (v[i] == 0)
		{
			if (bmin_a[i] > bmax_b[i] || bmin_b[i] > bmax_a[i])
			{
				return false;
			}
			else
				continue;
		}

		if (bmax_a[i] < bmin_b[i] && v[i] < 0)
		{
			t0[i] = (bmax_a[i] - bmin_b[i]) / v[i];
			cont = true;
		}
		else if (bmax_b[i] < bmin_a[i] && v[i] > 0)
		{
			t0[i] = (bmin_a[i] - bmax_b[i]) / v[i];
			cont = true;
		}
		if (bmax_b[i] > bmin_a[i] && v[i] < 0)
		{
			t1[i] = (bmin_a[i] - bmax_b[i]) / v[i];
			cont = true;
		}
		else if (bmax_a[i] > bmin_b[i] && v[i] > 0)
		{
			t1[i] = (bmax_a[i] - bmin_b[i]) / v[i];
			cont = true;
		}
		
		if (!cont)
			return false;
	}

	first_t = MAX(t0[X], MAX(t0[Y], t0[Z]));
	last_t = MIN(t1[X], MIN(t1[Y], t1[Z]));

	if (first_t <= last_t)
	{
		*fraction = first_t;
		return true;
	}

	return false;
}


bool I_RayTriIntersect(const vec3_t orig, const vec3_t dir,
                   vec3_t vert0, vec3_t vert1, vec3_t vert2,
                   float *fraction)
{
   float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   float det,inv_det;
   float u,v;

   /* find vectors for two edges sharing vert0 */
   M_SubVec3(vert1, vert0, edge1);
   M_SubVec3(vert2, vert0, edge2);

   /* begin calculating determinant - also used to calculate U parameter */
   M_CrossProduct(dir, edge2, pvec);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = M_DotProduct(edge1, pvec);

#if 0
   if (det < EPSILON)
      return 0;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec);
   if (*u < 0.0 || *u > det)
      return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec);
   if (*v < 0.0 || *u + *v > det)
      return 0;

   /* calculate t, scale parameters, ray intersects triangle */
   *t = DOT(edge2, qvec);
   inv_det = 1.0 / det;
   *t *= inv_det;
   *u *= inv_det;
   *v *= inv_det;
#endif
   
   /* the non-culling branch */
   if (det > -EPSILON && det < EPSILON)
     return false;
   inv_det = 1.0 / det;

   /* calculate distance from vert0 to ray origin */
   M_SubVec3(orig, vert0, tvec);

   /* calculate U parameter and test bounds */
   u = M_DotProduct(tvec, pvec) * inv_det;
   if (u < 0.0 || u > 1.0)
     return false;

   /* prepare to test V parameter */
   M_CrossProduct(tvec, edge1, qvec);

   /* calculate V parameter and test bounds */
   v = M_DotProduct(dir, qvec) * inv_det;
   if (v < 0.0 || u + v > 1.0)
     return false;

   /* calculate t, ray intersects triangle */
   *fraction = M_DotProduct(edge2, qvec) * inv_det;

   if (*fraction < 0)
	   return false;

   return true;
}



bool	I_LineMeshIntersect(const vec3_t start, const vec3_t end, const struct mesh_s *mesh, float *fraction, int *face_hit)
{
	//vec3_t newstart;
	int tri;
	float frac = 1.0;
	int facehit = -1;
	vec3_t dir;
	float len;
	float *p1, *p2, *p3;
	unsigned int *face;

	M_SubVec3(end, start, dir);	
	len = M_Normalize(dir);

//	M_CopyVec3(start, newstart);

	for (tri=0; tri<mesh->num_faces; tri++)
	{
		float tmp;
		
		face = mesh->facelist[tri];
		p1 = mesh->verts[face[0]];
		p2 = mesh->verts[face[1]];
		p3 = mesh->verts[face[2]];

		if (I_RayTriIntersect(start, dir, p1, p2, p3, &tmp))
		{
			tmp /= len;
			if (tmp < frac)
			{
				facehit = tri;
				frac = tmp;
				//M_PointOnLine(start, dir, frac * len, newstart);

				if (frac <= 0)
					break;
			}
		}
	}

	if (facehit != -1)
	{
		*face_hit = facehit;
		*fraction = frac;
		return true;
	}

	return false;
}



bool	I_MovingBoundsIntersectEx(vec3_t start, vec3_t end, vec3_t bmin_a, vec3_t bmax_a, vec3_t bmin_b, vec3_t bmax_b, float *fraction, vec3_t normal)
{
	vec3_t v;
	vec3_t t0 = { 0,0,0 };
	vec3_t t1 = { 1,1,1 };
	float first_t,last_t;
	int i;

	if (M_BoundsIntersect(bmin_a, bmax_a, bmin_b, bmax_b))
	{
		*fraction = 0;
		return true;
	}

	M_SubVec3(start, end, v);

	if (v[0] == 0 && v[1] == 0 && v[2] == 0)
	{
		//velocity is zero and we already tested the bounds
		return false;
	}

	//get first times of overlapping axes
	//the boxes only intersect when all extents overlap
	for (i=0; i<3; i++)
	{
		bool cont;
		
		cont = false;

		if (v[i] == 0)
		{
			if (bmin_a[i] > bmax_b[i] || bmin_b[i] > bmax_a[i])
			{
				return false;
			}
			else
				continue;
		}

		if (bmax_a[i] < bmin_b[i] && v[i] < 0)
		{
			t0[i] = (bmax_a[i] - bmin_b[i]) / v[i];
			cont = true;
		}
		else if (bmax_b[i] < bmin_a[i] && v[i] > 0)
		{
			t0[i] = (bmin_a[i] - bmax_b[i]) / v[i];
			cont = true;
		}
		if (bmax_b[i] > bmin_a[i] && v[i] < 0)
		{
			t1[i] = (bmin_a[i] - bmax_b[i]) / v[i];
			cont = true;
		}
		else if (bmax_a[i] > bmin_b[i] && v[i] > 0)
		{
			t1[i] = (bmax_a[i] - bmin_b[i]) / v[i];
			cont = true;
		}
		
		if (!cont)
		{
			return false;
		}
	}

	first_t = MAX(t0[X], MAX(t0[Y], t0[Z]));
	last_t = MIN(t1[X], MIN(t1[Y], t1[Z]));

	if (first_t <= last_t)
	{
		*fraction = first_t;

		if (first_t == t0[X])
		{
			if (v[X] > 0)
			{
				normal[X] = 1;
				normal[Y] = 0;
				normal[Z] = 0;
			}
			else
			{
				normal[X] = -1;
				normal[Y] = 0;
				normal[Z] = 0;
			}
		}
		else if (first_t == t0[Y])
		{
			if (v[Y] > 0)
			{
				normal[X] = 0;
				normal[Y] = 1;
				normal[Z] = 0;
			}
			else
			{
				normal[X] = 0;
				normal[Y] = -1;
				normal[Z] = 0;
			}
		}
		else
		{
			if (v[Z] > 0)
			{
				normal[X] = 0;
				normal[Y] = 0;
				normal[Z] = 1;
			}
			else
			{
				normal[X] = 0;
				normal[Y] = 0;
				normal[Z] = -1;
			}
		}

		return true;
	}
	
	return false;
}
