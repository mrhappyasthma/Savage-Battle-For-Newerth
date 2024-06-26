// (C) 2003 S2 Games

// le_occluder.c

// code for manipulating and displaying occluders

#include "../le/le.h"

typedef struct
{
	int		index;
	vec3_t	endpos;
	float	fraction;
	int		vertex;
	vec3_t	offset;
	occluder_t occluder;
} occtrace_t;

typedef enum
{
	OCC_NOOP,
	OCC_MOVE,
	OCC_ROTATE,
	OCC_SCALE
} occOp_enum;


cvar_t	le_occluderAlpha = { "le_occluderAlpha", "0.5", CVAR_SAVECONFIG  };
cvar_t	le_occluderDepthTest = { "le_occluderDepthTest", "1", CVAR_SAVECONFIG };
cvar_t	le_drawOccluders = { "le_drawOccluders", "0", CVAR_SAVECONFIG };
cvar_t	le_occluderFade = { "le_occluderFade", "1", CVAR_SAVECONFIG };

static int mouseoverIndex = -1;
static int mouseoverVertex = -1;
static int cur_op = 0;
static occtrace_t occtrace;

/*
void	buildquad(vec3_t v1, vec3_t v2, vec3_t v3, vec3_t v4, scenefacevert_t out[4])
{
	int n;

	for (n=0; n<4; n++)
	{
		SET_VEC4(out[n].col, 255, 255, 255, 30);
		SET_VEC2(out[n].tex, 0, 0);		
	}

	M_CopyVec3(v1, out[0].vtx);
	SET_VEC2(out[0].tex, 0, 0);
	M_CopyVec3(v2, out[1].vtx);
	SET_VEC2(out[1].tex, 0, 1);
	M_CopyVec3(v3, out[2].vtx);
	SET_VEC2(out[2].tex, 1, 1);
	M_CopyVec3(v4, out[3].vtx);
	SET_VEC2(out[3].tex, 1, 0);
}*/



/*==========================

  LE_DrawPoint

 ==========================*/

void	LE_DrawPoint(vec3_t point, vec4_t color, float size)
{
	int n;
	vec3_t tmp;
	scenefacevert_t poly[4];
	bvec4_t col = { color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255 };

	for (n=0; n<4; n++)
	{
		M_CopyBVec4(col, poly[n].col);
		SET_VEC2(poly[n].tex, 0, 0);
	}

	M_PointOnLine(point, le.camera.viewaxis[RIGHT], size/2, tmp);
	M_PointOnLine(tmp, le.camera.viewaxis[UP], size/2, poly[0].vtx);
	M_PointOnLine(poly[0].vtx, le.camera.viewaxis[RIGHT], -size, poly[1].vtx);
	M_PointOnLine(poly[1].vtx, le.camera.viewaxis[UP], -size, poly[2].vtx);
	M_PointOnLine(poly[2].vtx, le.camera.viewaxis[RIGHT], size, poly[3].vtx);

	corec.Scene_AddPoly(4, poly, corec.GetWhiteShader(), POLY_DOUBLESIDED | POLY_NO_DEPTH_TEST);
}



/*==========================

  LE_QuickQuad

 ==========================*/

void	LE_QuickQuad(vec4_t col, vec3_t p1, vec3_t p2, vec3_t p3, vec3_t p4, int flags)
{
	scenefacevert_t poly[4];
	int n;

	for (n=0; n<4; n++)
	{
		SET_VEC4(poly[n].col, col[0] * 255, col[1] * 255, col[2] * 255, col[3] * 255);
		SET_VEC2(poly[n].tex, 0, 0);		
	}

	M_CopyVec3(p1, poly[0].vtx);
	M_CopyVec3(p2, poly[1].vtx);
	M_CopyVec3(p3, poly[2].vtx);
	M_CopyVec3(p4, poly[3].vtx);

	corec.Scene_AddPoly(4, poly, corec.GetWhiteShader(), POLY_DOUBLESIDED | flags);
}


/*==========================

  LE_DrawOccluderPoly

 ==========================*/

void	LE_DrawOccluderPoly(occluder_t *occluder, bool selected)
{
	int n;
	
	scenefacevert_t poly[64];

	memset(poly, 0, sizeof(poly));

	for (n=0; n<occluder->numpoints; n++)
	{
		M_CopyVec3(occluder->points[n], poly[n].vtx);

		if (selected)
			SET_VEC4(poly[n].col, 255, 255, 0, 255);
		else
		{
			float dist;
			int brightness;
			
			if (le_occluderFade.integer)
			{
				dist = M_GetDistance(occluder->points[n], le.camera.origin);
				brightness = (1 - (dist / corec.Cvar_GetValue("gfx_farclip"))) * 255;

				if (brightness < 0)
					brightness = 0;
				else if (brightness > 255)
					brightness = 255;
			}
			else
			{
				brightness = 255;
			}

			SET_VEC4(poly[n].col, 255,255,255, brightness);
		}
	}

	if (selected && occtrace.vertex > -1)
	{		
		//show selected vertex
		LE_DrawPoint(occluder->points[occtrace.vertex], vec4(1,1,1,1), 15);
	}

	corec.Scene_AddPoly(occluder->numpoints, poly, corec.GetWhiteShader(), POLY_DOUBLESIDED | POLY_WIREFRAME | POLY_NO_DEPTH_TEST);

	for (n=0; n<occluder->numpoints; n++)
	{
		SET_VEC4(poly[n].col, 255, 255, 255, 255 * le_occluderAlpha.value);		
	}

	corec.Scene_AddPoly(occluder->numpoints, poly, corec.Res_LoadShader("/textures/core/occluder.tga"), POLY_DOUBLESIDED);
}




/*==========================

  RayTriIntersect

  from http://www.acm.org/jgt/papers/MollerTrumbore97/

 ==========================*/

bool RayTriIntersect(const vec3_t orig, const vec3_t dir,
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



/*==========================

  LE_OccluderHitTest

  see if the mouse cursor is over an occluder

 ==========================*/

int		LE_OccluderHitTest(occtrace_t *trace)
{
	int n,i;
	int numoccluders;
	vec3_t dir;
	
	trace->fraction = 99999;
	trace->index = -1;
	trace->vertex = -1;

	Cam_ConstructRay(&le.camera, le.mousepos.x, le.mousepos.y, dir);

	numoccluders = corec.World_GetNumOccluders();

	for (n=0; n<numoccluders; n++)
	{
		float *v1, *v2, *v3;
		occluder_t occluder;

		corec.World_GetOccluder(n, &occluder);
		
		v1 = occluder.points[0];

		for (i=0; i<occluder.numpoints-2; i++)
		{
			float frac;

			v2 = occluder.points[i+1];
			v3 = occluder.points[i+2];
		
			if (RayTriIntersect(le.camera.origin, dir, v1, v2, v3, &frac))
			{
				if (frac < trace->fraction)
				{
					trace->fraction = frac;
					trace->index = n;					
				}
			}
		}
	}

	if (trace->index > -1)
	{
		M_PointOnLine(le.camera.origin, dir, trace->fraction, trace->endpos);		

		corec.World_GetOccluder(trace->index, &trace->occluder);

		for (i=0; i<trace->occluder.numpoints; i++)
		{
			if (M_GetDistanceSq(trace->occluder.points[i], trace->endpos) < 500)
			{
				trace->vertex = i;
				break;
			}
		}

		M_SubVec3(trace->endpos, trace->occluder.points[0], trace->offset);
	}

	return trace->index;
}


/*==========================

  LE_CorrectOccluderError

  make sure all occluder points are coplanar

 ==========================*/

void	LE_CorrectOccluderError(occluder_t *occ)
{
	//make sure all occluder points are coplanar

	int n;
	plane_t plane;

	if (occ->numpoints == 3)
		return;

	M_CalcPlane(occ->points[0], occ->points[1], occ->points[2], &plane);

	for (n=3; n<occ->numpoints; n++)
	{
		float error;

		error = M_DotProduct(plane.normal, occ->points[n]) - plane.dist;

		if (error > 0.001)
			M_PointOnLine(occ->points[n], plane.normal, -error, occ->points[n]);
	}
}

void	LE_OccluderOp()
{
	int n;
	vec3_t change = { 0,0,0 };

	switch(cur_op)
	{
		case OCC_MOVE:
		{
			if (le.button & BUTTON_SHIFT)
				change[Z] = -le.mousepos.deltay * le.screenscaley;
			else if (le.button & BUTTON_CTRL)
			{
				change[Y] = -le.mousepos.deltay * le.screenscaley;
			}
			else
			{
				M_MultVec3(le.camera.viewaxis[RIGHT], le.mousepos.deltax * le.screenscalex, change);
				M_PointOnLine(change, vec3(le.camera.viewaxis[FORWARD][0], le.camera.viewaxis[FORWARD][1], 0), -le.mousepos.deltay * le.screenscaley, change);
			}

			if (!M_CompareVec3(change, zero_vec))
			{
				if (occtrace.vertex == -1)
				{
					//move the entire occluder
					for (n=0; n<occtrace.occluder.numpoints; n++)
					{
						M_AddVec3(occtrace.occluder.points[n], change, occtrace.occluder.points[n]);
					}					
				}
				else
				{
					float a;
					plane_t plane;
					//move an individual vertex
					//manipulate it in the same plane as the occluder
					M_CalcPlane(occtrace.occluder.points[0], occtrace.occluder.points[1], occtrace.occluder.points[2], &plane);

					//project the change vector onto the plane
					a = M_DotProduct(change, plane.normal);
					for (n=0; n<3; n++)
						change[n] = change[n] - plane.normal[n] * a;
					
					M_AddVec3(occtrace.occluder.points[occtrace.vertex], change, occtrace.occluder.points[occtrace.vertex]);
				}

				LE_CorrectOccluderError(&occtrace.occluder);

				corec.World_UpdateOccluder(occtrace.index, &occtrace.occluder);
			}

			break;
		}
	
		case OCC_ROTATE:
		{
			int n;
			vec3_t axis[3];
			vec3_t change = { 0,0,0 };
			if (occtrace.vertex > -1)
				break;

			if (le.button & BUTTON_CTRL)
				change[PITCH] = -le.mousepos.deltay * le.screenscaley;
			else if (le.button & BUTTON_ALT)
				change[ROLL] = le.mousepos.deltax * le.screenscalex;
			else
				change[YAW] = le.mousepos.deltax * le.screenscalex;

			M_GetAxis(change[PITCH], change[ROLL], change[YAW], axis);

			for (n=0; n<occtrace.occluder.numpoints; n++)
			{
				M_SubVec3(occtrace.occluder.points[n], occtrace.endpos, occtrace.occluder.points[n]);
				M_TransformPoint(occtrace.occluder.points[n], zero_vec, axis, occtrace.occluder.points[n]);
				M_AddVec3(occtrace.occluder.points[n], occtrace.endpos, occtrace.occluder.points[n]);
			}

			LE_CorrectOccluderError(&occtrace.occluder);

			corec.World_UpdateOccluder(occtrace.index, &occtrace.occluder);

			break;
		}

		case OCC_SCALE:
		{
			SET_VEC3(change,	1,1,1);/*1 + -le.mousepos.deltay * le.screenscaley,
								1 + -le.mousepos.deltay * le.screenscaley,
								1 + -le.mousepos.deltay * le.screenscaley);*/

			for (n=0; n<occtrace.occluder.numpoints; n++)
			{
				occtrace.occluder.points[n][0] *= change[0];
				occtrace.occluder.points[n][1] *= change[1];
 				occtrace.occluder.points[n][2] *= change[2];
			}

			LE_CorrectOccluderError(&occtrace.occluder);

			corec.World_UpdateOccluder(occtrace.index, &occtrace.occluder);

			break;
		}
	}

	le.showAxes = true;
	//if (cur_op)
	{
		bool occaxis = false;

		//draw axis markers

		if (occtrace.vertex > -1 ||
			cur_op == OCC_ROTATE || 
			!cur_op)
			occaxis = true;

		if (occaxis)
		{			
			//show occluder axes

			plane_t plane;

			M_CalcPlane(occtrace.occluder.points[0], occtrace.occluder.points[1], occtrace.occluder.points[2], &plane);

			M_GetAxisFromForwardVec(plane.normal, le.axesObj.axis);			
		}
		else
		{
			//show world axes

			SET_VEC3(le.axesObj.axis[0], 1, 0, 0);
			SET_VEC3(le.axesObj.axis[1], 0, 1, 0);
			SET_VEC3(le.axesObj.axis[2], 0, 0, 1);
		}		

		if (occtrace.vertex > -1)
		{
			M_CopyVec3(occtrace.occluder.points[occtrace.vertex], le.axesObj.pos);
		}
		else
		{
			if (cur_op == OCC_ROTATE)
			{
				M_CopyVec3(occtrace.endpos, le.axesObj.pos);
			}
			else
			{
				//use the offset position so it moves with the occluder
				M_AddVec3(occtrace.occluder.points[0], occtrace.offset, le.axesObj.pos);				
			}
		}
		
		le.axesObj.flags |= SCENEOBJ_USE_AXIS;
		le.axesObj.scale = 1;
		if (cur_op)
			le.axesObj.scale += sin(corec.Milliseconds() * 0.01) * 0.1;
	}
}

/*==========================

  LE_OccludersFrame

  handles drawing and manipulation of occluders, and adding them to the scene

 ==========================*/

void	LE_OccludersFrame()
{
	int n;
	int numoccluders;

	if (strcmp(le_mode.string, "occluder")==0)
	{		
		if (!cur_op)
			mouseoverIndex = LE_OccluderHitTest(&occtrace);

		if (mouseoverIndex > -1)
		{
			LE_SetCursor(res.mainCursor);
			LE_SetHotspot(0,0);
			LE_OccluderOp();
		}
		else
		{
			LE_SetCursor(res.crosshairCursor);
			LE_SetHotspot(16,16);
		}		
	}

	numoccluders = corec.World_GetNumOccluders();

	for (n=0; n<numoccluders; n++)
	{
		occluder_t occluder;

		corec.World_GetOccluder(n, &occluder);
		corec.Scene_AddOccluder(&occluder);

		if (strcmp(le_mode.string, "occluder") != 0 && !le_drawOccluders.integer)
			continue;

		LE_DrawOccluderPoly(&occluder, mouseoverIndex == n);
	}

}


/*==========================

  LE_OccluderAddVertex

 ==========================*/

void	LE_OccluderAddVertex()
{
	occtrace_t trace;
	int bestdist=999999999;
	int edge=-1;
	int n;

	if (LE_OccluderHitTest(&trace) == -1)
		return;
	
	if (trace.vertex != -1)
		return;

	if (trace.occluder.numpoints >= MAX_OCCLUDER_POINTS)
		return;

	for (n=0; n<trace.occluder.numpoints; n++)
	{		
		float d = M_DistToLineSegment(trace.endpos, trace.occluder.points[n], trace.occluder.points[(n+1) % trace.occluder.numpoints]);
		if (d < bestdist)
		{
			bestdist = d;
			edge = n;
		}
	}

	if (edge > -1)
	{
		float c1,c2,b;
		vec3_t vtxpos;
		vec3_t v,w;
		float *occ_v1 = trace.occluder.points[edge];
		float *occ_v2 = trace.occluder.points[(edge+1) % trace.occluder.numpoints];

		M_SubVec3(occ_v2, occ_v1, v);
		M_SubVec3(trace.endpos, occ_v1, w);

		c1 = M_DotProduct(w,v);
		c2 = M_DotProduct(v,v);
		if (c1 > 0 && c2 > c1)
		{
			int idx1 = edge;
			int idx2 = (edge+1) % trace.occluder.numpoints;
			b = c1/c2;
			M_PointOnLine(occ_v1, v, b, vtxpos);

			//insert new vertex
			memmove(&trace.occluder.points[idx2+1],
					&trace.occluder.points[idx2],
					sizeof(vec3_t) * (trace.occluder.numpoints - idx2));			

			M_CopyVec3(vtxpos, occ_v2);

			trace.occluder.numpoints++;

			LE_CorrectOccluderError(&trace.occluder);

			corec.World_UpdateOccluder(trace.index, &trace.occluder);
		}
	}
}


/*==========================

  LE_OccluderRemoveVertex

 ==========================*/

void	LE_OccluderRemoveVertex()
{
	occtrace_t trace;

	if (LE_OccluderHitTest(&trace) == -1)
		return;

	if (trace.vertex == -1)
		return;

	if (trace.vertex < trace.occluder.numpoints-1)
		memmove(&trace.occluder.points[trace.vertex], 
				&trace.occluder.points[trace.vertex+1],
				sizeof(vec3_t) * (trace.occluder.numpoints - trace.vertex));

	trace.occluder.numpoints--;

	corec.World_UpdateOccluder(trace.index, &trace.occluder);
}


/*==========================

  LE_OccluderMouseDown

 ==========================*/

void	LE_OccluderMouseDown()
{
	if (mouseoverIndex == -1)
		return;

	if (corec.Input_IsKeyDown('v'))
	{
		LE_OccluderAddVertex();
		return;
	}

	cur_op = OCC_MOVE;
}


/*==========================

  LE_OccluderMouseUp

 ==========================*/

void	LE_OccluderMouseUp()
{
	cur_op = OCC_NOOP;
}


/*==========================

  LE_OccluderRightMouseDown

 ==========================*/

void	LE_OccluderRightMouseDown()
{
	if (mouseoverIndex == -1)
		return;

	if (corec.Input_IsKeyDown('v'))
	{		
		LE_OccluderRemoveVertex();
		return;
	}

	if (le.button & BUTTON_SHIFT)
		cur_op = OCC_SCALE;
	else
		cur_op = OCC_ROTATE;
}


/*==========================

  LE_OccluderRightMouseUp

 ==========================*/

void	LE_OccluderRightMouseUp()
{
	cur_op = OCC_NOOP;
}




/*==========================

  LE_CenterTrace

  trace through the middle of the screen

 ==========================*/

bool	LE_CenterTrace(traceinfo_t *result, int ignoreSurface)
{
	vec3_t dir,end;

	Cam_ConstructRay(&le.camera, le.screenw / 2, le.screenh / 2, dir);

	M_PointOnLine(le.camera.origin, dir, 99999, end);

	return corec.World_TraceBox(result, le.camera.origin, end, zero_vec, zero_vec, 0);
}

/*==========================

  LE_NewOccluder_Cmd

 ==========================*/

void	LE_NewOccluder_Cmd(int argc, char *argv[])
{
	occluder_t occ;
	traceinfo_t trace;

	if (strcmp(le_mode.string, "occluder")!=0)
		return;

	LE_CenterTrace(&trace, 0);

	if (trace.fraction == 1)
		return;

	occ.numpoints = 4;
	M_PointOnLine(trace.endpos, le.camera.viewaxis[RIGHT], -200, occ.points[0]);
	M_PointOnLine(occ.points[0], vec3(0,0,1), 400, occ.points[1]);
	M_PointOnLine(occ.points[1], le.camera.viewaxis[RIGHT], 400, occ.points[2]);
	M_PointOnLine(occ.points[2], vec3(0,0,1), -400, occ.points[3]);

	corec.World_AddOccluder(&occ);
}


/*==========================

  LE_DeleteOccluder_Cmd

 ==========================*/

void	LE_DeleteOccluder_Cmd(int argc, char *argv[])
{	
	occtrace_t trace;

	if (strcmp(le_mode.string, "occluder")!=0)
		return;

	if (LE_OccluderHitTest(&trace) == -1)
		return;

	if (trace.vertex > -1)
	{
		LE_OccluderRemoveVertex();

	}
	else
	{
		corec.World_RemoveOccluder(trace.index);
	}
}

void	LE_InitOccluder()
{
	corec.Cvar_Register(&le_occluderAlpha);
	corec.Cvar_Register(&le_occluderDepthTest);
	corec.Cvar_Register(&le_drawOccluders);
	corec.Cvar_Register(&le_occluderFade);

	corec.Cmd_Register("newOccluder", LE_NewOccluder_Cmd);
	corec.Cmd_Register("delOccluder", LE_DeleteOccluder_Cmd);
}