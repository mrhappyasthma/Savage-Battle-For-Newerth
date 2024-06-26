// (C) 2003 S2 Games

// gl_scene.c

// OpenGL scene rendering functions


#include "core.h"

#ifdef _WIN32
#include "glh_genext.h"
#endif

extern cvar_t wt_debug;
extern float draw_shadertime;
extern cvar_t gfx_noBlending;
extern cvar_t gfx_alphaTestRef;

#define MAX_GROUNDPLANE_LISTS	2048
GLuint	groundPlaneListBase;

//mostly for Mac OS X
#ifndef GL_LIGHT_MODEL_COLOR_CONTROL_EXT
	#ifdef GL_LIGHT_MODEL_COLOR_CONTROL
		#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT GL_LIGHT_MODEL_COLOR_CONTROL
	#endif
#endif
#ifndef GL_SEPARATE_SPECULAR_COLOR_EXT
	#ifdef GL_SEPARATE_SPECULAR_COLOR
		#define GL_SEPARATE_SPECULAR_COLOR_EXT GL_SEPARATE_SPECULAR_COLOR
	#endif
#endif
#ifndef GL_SINGLE_COLOR_EXT
	#ifdef GL_SINGLE_COLOR
		#define GL_SINGLE_COLOR_EXT GL_SINGLE_COLOR
	#endif
#endif


/*==========================

  CVARS

 ==========================*/


cvar_t	gfx_antialias = { "gfx_antialias", "0", CVAR_SAVECONFIG };
cvar_t	gfx_water_hack = { "gfx_waterhack", "0", CVAR_WORLDCONFIG };
cvar_t	gfx_waterr = { "gfx_waterr", "0", CVAR_WORLDCONFIG };
cvar_t	gfx_waterg = { "gfx_waterg", "0", CVAR_WORLDCONFIG };
cvar_t	gfx_waterb = { "gfx_waterb", "1", CVAR_WORLDCONFIG };
cvar_t	gfx_wateralpha = { "gfx_wateralpha", "0.5", CVAR_WORLDCONFIG };
cvar_t	gfx_waterlevel = { "gfx_waterlevel", "1", CVAR_WORLDCONFIG };
cvar_t	gfx_watershader = { "gfx_watershader", "/textures/core/white.tga", CVAR_WORLDCONFIG };
cvar_t	gfx_waterscale = { "gfx_waterscale", "1", CVAR_WORLDCONFIG };
cvar_t	gfx_separate_specular_color = { "gfx_separate_specular_color", "1" };
cvar_t	gfx_ambient_and_diffuse = { "gfx_ambient_and_diffuse", "1" };
cvar_t	gfx_foliageDensity = { "gfx_foliageDensity", "8", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageThinDistance = { "gfx_foliageThinDistance", "1", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageAlphaDistance = { "gfx_foliageAlphaDistance", "0.1", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageAlphaScale = { "gfx_foliageAlphaScale", "0.1", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageSmooth = { "gfx_foliageSmooth", "5", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageMaxSlope = { "gfx_foliageMaxSlope", "0.5", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageCurve = { "gfx_foliageCurve", "4", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageWidth = { "gfx_foliageWidth", "12", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageHeight = { "gfx_foliageHeight", "6", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageRandomSize = { "gfx_foliageRandomSize", "6", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageDrawPoints = { "gfx_foliageDrawPoints", "0" };
cvar_t	gfx_foliageAnimateStrength = { "gfx_foliageAnimateStrength", "4", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageAnimateSpeed = { "gfx_foliageAnimateSpeed", "0.2", CVAR_WORLDCONFIG };
cvar_t	gfx_foliagePhaseMultiplier = { "gfx_foliagePhaseMultiplier", "50", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageShader = { "gfx_foliageShader", "/textures/foliage/foliage_grass.tga", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageNoiseThreshold = { "gfx_foliageNoiseThreshold", "0.5", CVAR_WORLDCONFIG };
cvar_t	gfx_foliageSmoothThreshold = { "gfx_foliageSmoothThreshold", "0.20" };
cvar_t	gfx_foliageFalloff = { "gfx_foliageFalloff", "300", CVAR_SAVECONFIG};
cvar_t	gfx_foliageAngleFudge = { "gfx_foliageAngleFudge", "235" };
cvar_t	gfx_foliageFudge = { "gfx_foliageFudge", "17" };
cvar_t	gfx_foliageTrampleDist = { "gfx_foliageTrampleDist", "0" };
cvar_t	gfx_foliage = { "gfx_foliage", "1", CVAR_SAVECONFIG };
cvar_t	gfx_foliageAlphaRef = { "gfx_foliageAlphaRef", "0.3" };
cvar_t	gfx_foliageMethod = {"gfx_foliageMethod", "0" };
cvar_t	gfx_cloudShader = { "gfx_cloudShader", "/textures/projectors/t_cloudlayer1.tga", CVAR_WORLDCONFIG };
cvar_t	gfx_clouds = { "gfx_clouds", "1", CVAR_SAVECONFIG };
cvar_t	gfx_cloudScale = { "gfx_cloudScale", "50", CVAR_WORLDCONFIG };
cvar_t	gfx_cloudSpeedX = { "gfx_cloudSpeedX", "0.5", CVAR_WORLDCONFIG };
cvar_t	gfx_cloudSpeedY = { "gfx_cloudSpeedY", "0.8", CVAR_WORLDCONFIG };
cvar_t	gfx_cloudBlendMode = { "gfx_cloudBlendMode", "GL_MODULATE", CVAR_WORLDCONFIG };
cvar_t	gfx_render = { "gfx_render", "1", CVAR_CHEAT };		//turn this off to skip scene rendering altogether
cvar_t	gfx_renderDataRefCount = { "gfx_renderDataRefCount", "1", CVAR_READONLY };
cvar_t	gfx_drawSprites = { "gfx_drawSprites", "1" };
cvar_t	gfx_drawSilhouettes = { "gfx_drawSilhouettes", "0", CVAR_CHEAT };
cvar_t	gfx_silhouetteR = { "gfx_silhouetteR", "0" };
cvar_t	gfx_silhouetteG = { "gfx_silhouetteG", "1" };
cvar_t	gfx_silhouetteB = { "gfx_silhouetteB", "0" };
cvar_t	gfx_whiteHud = { "gfx_whiteHud", "0", CVAR_CHEAT };

cvar_t	terrain_chunksize = { "terrain_chunksize", "31" };


int screenw, screenh;

camera_t				*cam;  
float					tm[16];  //current transformation matrix	
float					rot_tm[9];	//rotation part
int						lod1range, lod2range, lod3range;
float					objectRadiusMin;
float					objectRadiusMax;


/*==========================

  GL_PushCamera

  setup the modelview and projection matrices in one call

 ==========================*/

void	GL_PushCamera(camera_t *camera)
{
	float	aspect;
	vec3_t tr;

	glViewport(camera->x, Vid_GetScreenH() - camera->y - camera->height, camera->width, camera->height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	aspect = (float)camera->width / (float)camera->height;
	gluPerspective(camera->fovy, aspect, gfx_nearclip.value, gfx_farclip.value);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	//set up the inverse of the camera's matrix (axis and position)

	M_MultVec3(camera->origin, -1, tr);
	//construct the transformation matrix
	tm[0] = camera->viewaxis[0][0];
	tm[1] = camera->viewaxis[1][0];
	tm[2] = camera->viewaxis[2][0];
	tm[3] = 0.0;
	tm[4] = camera->viewaxis[0][1];
	tm[5] = camera->viewaxis[1][1];
	tm[6] = camera->viewaxis[2][1];
	tm[7] = 0.0;
	tm[8] = camera->viewaxis[0][2];
	tm[9] = camera->viewaxis[1][2];
	tm[10] = camera->viewaxis[2][2];
	tm[11] = 0.0;
	tm[12] = 0;//M_DotProduct(camera->viewaxis[0], tr);
	tm[13] = 0;//M_DotProduct(camera->viewaxis[1], tr);
	tm[14] = 0;//M_DotProduct(camera->viewaxis[2], tr);
	tm[15] = 1.0;
	
	//glLoadMatrixf(tm);
	glLoadIdentity();
	glRotatef(-90, 1, 0, 0);		//rotate around the x axis so viewaxis[Z] points UP from our viewpoint
	glMultMatrixf(tm);
	glTranslatef(tr[0],tr[1],tr[2]);
	

	glGetFloatv(GL_MODELVIEW_MATRIX, tm);	


	rot_tm[0] = tm[0];
	rot_tm[1] = tm[1];
	rot_tm[2] = tm[2];
	rot_tm[3] = tm[4];
	rot_tm[4] = tm[5];
	rot_tm[5] = tm[6];
	rot_tm[6] = tm[8];
	rot_tm[7] = tm[9];
	rot_tm[8] = tm[10];	
}


/*==========================

  GL_PopCamera

 ==========================*/

void	GL_PopCamera()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}




/*==========================

  GL_SetColor

 ==========================*/

void	GL_SetColor(vec4_t color)
{
	glColor4fv(color);
}



/*==========================

  GL_DrawBox

 ==========================*/

void	GL_DrawBox(vec3_t bmin, vec3_t bmax)
{
	GL_Disable(GL_TEXTURE_2D);

	glColor4f(1,1,1,1);
	glBegin(GL_LINE_LOOP);
		glVertex3f(bmin[0], bmin[1], bmin[2]);
		glVertex3f(bmax[0], bmin[1], bmin[2]);
		glVertex3f(bmax[0], bmax[1], bmin[2]);
		glVertex3f(bmin[0], bmax[1], bmin[2]);		
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex3f(bmin[0], bmin[1], bmax[2]);
		glVertex3f(bmax[0], bmin[1], bmax[2]);
		glVertex3f(bmax[0], bmax[1], bmax[2]);	
		glVertex3f(bmin[0], bmax[1], bmax[2]);		
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(bmin[0], bmin[1], bmin[2]);
		glVertex3f(bmin[0], bmin[1], bmax[2]);
		glVertex3f(bmin[0], bmax[1], bmin[2]);
		glVertex3f(bmin[0], bmax[1], bmax[2]);
		glVertex3f(bmax[0], bmin[1], bmin[2]);
		glVertex3f(bmax[0], bmin[1], bmax[2]);
		glVertex3f(bmax[0], bmax[1], bmin[2]);
		glVertex3f(bmax[0], bmax[1], bmax[2]);
	glEnd();

	GL_Enable(GL_TEXTURE_2D);
}


/*==========================

  GL_Quad2d

 ==========================*/

void	GL_Quad2d(float x, float y, float w, float h, float s1, float t1, float s2, float t2, shader_t *shader)
{
	float time = draw_shadertime;
	
	if (gfx_whiteHud.integer)
		GL_SelectShader(Res_GetShader(Host_GetWhiteShader()), 0);
	else
		GL_SelectShader(shader, time);	

	glBegin(GL_QUADS);
	glTexCoord2f(s1, t1);
	glVertex2f(x, y);
	glTexCoord2f(s1, t2);
	glVertex2f(x, y+h);
	glTexCoord2f(s2, t2);
	glVertex2f(x+w, y+h);
	glTexCoord2f(s2, t1);
	glVertex2f (x+w, y);
	glEnd();

	sceneStats.polycount += 2;
	if (shader->translucent)
		sceneStats.translucentPolycount += 2;
}


/*==========================

  GL_Poly2d

 ==========================*/

void	GL_Poly2d(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, shader_t *shader)
{
	float time = draw_shadertime;
	  
	GL_SelectShader(shader, time);

	glBegin(GL_QUADS);
	glTexCoord2f(s1, t1);
	glVertex2f(v1[0], v1[1]);
	glTexCoord2f(s1, t2);
	glVertex2f(v2[0], v2[1]);
	glTexCoord2f(s2, t2);
	glVertex2f(v3[0], v3[1]);
	glTexCoord2f(s2, t1);
	glVertex2f(v4[0], v4[1]);
	glEnd();	
}




/*==========================

  GL_DrawBillboard

 ==========================*/

void	GL_DrawBillboard(vec3_t pos, float width, float height, float angle, float s1, float t1, float s2, float t2, bool all_axes)
{
	float w, h, r;
	vec3_t p0,p1,p2,p3;
	float *up;

	if (all_axes)
		up = cam->viewaxis[UP];
	else
		up = vec3(0,0,1);
	
	w = width/2;
	h = height/2;

	//temporary fix until we get rotating billboards to be non-square
	if (!angle)
	{
		w = width/2;
		h = height/2;

		M_PointOnLine(pos, cam->viewaxis[RIGHT], w, p0);

		M_PointOnLine(p0, up, -h, p0);
	
		M_PointOnLine(p0, up, height, p1);

		M_PointOnLine(p1, cam->viewaxis[RIGHT], -width, p2);

		M_PointOnLine(p2, up, -height, p3);
	}	
	else
	{
		angle += 45;	//since we're multiplying by r (billboards radius), the
					//distance to a corner, the orientation is off by 45
	
		r = sqrt(w * w + h * h);
		h = SIN(DEG2RAD(angle)) * r;
		w = COS(DEG2RAD(angle)) * r;
		M_PointOnLine(pos, cam->viewaxis[RIGHT], w, p0);

		M_PointOnLine(p0, up, -h, p0);
		M_PointOnLine(pos, cam->viewaxis[RIGHT], h, p1);
		M_PointOnLine(p1, up, w, p1);
		M_PointOnLine(pos, cam->viewaxis[RIGHT], -w, p2);
		M_PointOnLine(p2, up, h, p2);
		M_PointOnLine(pos, cam->viewaxis[RIGHT], -h, p3);
		M_PointOnLine(p3, up, -w, p3);
	}
	
	if (!s1 && !t1 && !s2 && !t2)
	{
		s2 = 1;
		t2 = 1;
	}

	glBegin(GL_QUADS);

	glTexCoord2f(s2, t2);
	glVertex3fv(p0);
	glTexCoord2f(s2, t1);
	glVertex3fv(p1);
	glTexCoord2f(s1, t1);
	glVertex3fv(p2);
	glTexCoord2f(s1, t2);
	glVertex3fv(p3);

	glEnd();

	sceneStats.polycount += 2;
}

/*==========================

  GL_DrawBeam

 ==========================*/

void	GL_DrawBeam(vec3_t start, vec3_t end, float size, float tile)
{
	vec3_t	forward, right, up, p;

	up[0] = 0;
	up[1] = 0;
	up[2] = 1;

	GL_Disable(GL_CULL_FACE);

	//get base values
	M_SubVec3(end, start, forward);
	M_Normalize(forward);
	M_CrossProduct(forward, up, right);
	M_Normalize(right);
	M_CrossProduct(right, forward, up);
	M_Normalize(up);
	M_MultVec3(right, size, right);
	M_MultVec3(up, size, up);

	glBegin(GL_QUADS);

	//first cross quad
	M_CopyVec3(start, p);
	M_AddVec3(p, right, p);
	M_AddVec3(p, up, p);
	glTexCoord2f(0.0, 0.0);
	glVertex3fv(p);

	M_CopyVec3(end, p);
	M_AddVec3(p, right, p);
	M_AddVec3(p, up, p);
	glTexCoord2f(tile, 0.0);
	glVertex3fv(p);

	M_CopyVec3(end, p);
	M_SubVec3(p, right, p);
	M_SubVec3(p, up, p);
	glTexCoord2f(tile, 1.0);
	glVertex3fv(p);
	
	M_CopyVec3(start, p);
	M_SubVec3(p, right, p);
	M_SubVec3(p, up, p);
	glTexCoord2f(0.0, 1.0);
	glVertex3fv(p);

	//second cross quad
	M_CopyVec3(start, p);
	M_AddVec3(p, right, p);
	M_SubVec3(p, up, p);
	glTexCoord2f(0.0, 0.0);
	glVertex3fv(p);

	M_CopyVec3(end, p);
	M_AddVec3(p, right, p);
	M_SubVec3(p, up, p);
	glTexCoord2f(tile, 0.0);
	glVertex3fv(p);

	M_CopyVec3(end, p);
	M_SubVec3(p, right, p);
	M_AddVec3(p, up, p);
	glTexCoord2f(tile, 1.0);
	glVertex3fv(p);
	
	M_CopyVec3(start, p);
	M_SubVec3(p, right, p);
	M_AddVec3(p, up, p);
	glTexCoord2f(0.0, 1.0);
	glVertex3fv(p);

	glEnd();

	sceneStats.polycount += 4;

	GL_Enable(GL_CULL_FACE);
}



/*==========================

  FOLIAGE DRAWING CODE

 ==========================*/



/*==========================

  GL_DoFoliageVerts

 ==========================*/

void	GL_DoFoliageVerts(vec3_t pos, float width, float height, float topshift, vec3_t trampleAngle)
{
	float w, h;
	vec3_t p0,p1,p2,p3; 
	static vec3_t up = { 0,0,1 };
	
	w = width/2;
	h = height/2;
	
	M_PointOnLine(pos, cam->viewaxis[RIGHT], w, p0);
	M_PointOnLine(p0, up, -h, p0);
	
	M_PointOnLine(p0, up, height, p1);
	
	M_PointOnLine(p1, cam->viewaxis[RIGHT], -width, p2);
	
	M_PointOnLine(p2, up, -height, p3);
	
	glTexCoord2f(1, 1);
	glVertex3fv(p0);
	glTexCoord2f(1, 0);
	glVertex3f(p1[0]+topshift,p1[1],p1[2]);
	glTexCoord2f(0, 0);
	glVertex3f(p2[0]-topshift,p2[1],p2[2]);
	glTexCoord2f(0, 1);
	glVertex3fv(p3);
	
	//glPopMatrix();
	
	sceneStats.polycount += 2;
}



void	_affectScore(int tile_x, int tile_y, float *chance)
{
	shader_t *shader;
	colormap_t *colormap;
	
	if (!WITHIN_WORLD(tile_x, tile_y))
		return;

	colormap = &WR_COLREF(tile_x, tile_y);
	if (*colormap[3] < 128)
		shader = Res_GetShader(wr.shaders[WR_SHADERREF2(tile_x, tile_y)]);
	else
		shader = Res_GetShader(wr.shaders[WR_SHADERREF(tile_x, tile_y)]);
	if (shader->flags & SHD_NO_FOLIAGE)
	{
		*chance -= *chance/gfx_foliageSmooth.value;
	}
}

/*
void	GL_DrawFoliageOnTile(int tile_x, int tile_y, vec3_t campos, float maxlen, int xinc, int yinc)
{
	float x, y;
	float secs;
	float *viewx, *viewy;
	float height;
	float val;
	float minalpha = 999999, avgalpha = 0;
	float thindistsq, alphadistsq, alphasize;
	int startx,starty,endx,endy;
	int numalpha = 0;
	int phase;
	int gridref;
	float alpha, angle;
	float len;
	float noise, absnoise;
	float chance = 0;
	vec3_t p;
	vec3_t q;

	angle = ((int)(M_GetVec2Angle(cam->viewaxis[FORWARD]) + gfx_foliageAngleFudge.integer)) % 360;
	//Console_DPrintf("angle is %f\n", angle);
	
	if (angle < 90)
	{
		//Console_DPrintf("using case 0 for foliage\n");
		viewy = &y;
		viewx = &x;
		yinc = 1;
		xinc = 1;
	}
	else if (angle < 180)
	{
		//Console_DPrintf("using case 1 for foliage\n");
		viewx = &y;
		viewy = &x;
		xinc = 1;
		yinc = -1;
	}
	else if (angle < 270)
	{
		//Console_DPrintf("using case 2 for foliage\n");
		viewy = &y;
		viewx = &x;
		yinc = -1;
		xinc = -1;
	}
	else
	{
		//Console_DPrintf("using case 3 for foliage\n");
		viewx = &y;
		viewy = &x;
		xinc = -1;
		yinc = 1;
	}
	
	starty = (yinc > 0 ? GRID_TO_WORLD(tile_y) : GRID_TO_WORLD(tile_y+1));
	endy = (yinc < 0 ? GRID_TO_WORLD(tile_y) : GRID_TO_WORLD(tile_y+1));
	startx = (xinc > 0 ? GRID_TO_WORLD(tile_x) : GRID_TO_WORLD(tile_x+1));
	endx = (xinc < 0 ? GRID_TO_WORLD(tile_x) : GRID_TO_WORLD(tile_x+1));

	secs = Host_Milliseconds() / 1000.0;

	thindistsq = maxlen * gfx_foliageThinDistance.value;
	if (thindistsq >= maxlen)
		thindistsq = 0;
	alphadistsq = maxlen * gfx_foliageAlphaDistance.value;
	alphasize = maxlen - alphadistsq;

	xinc *= gfx_foliageDensity.integer;
	yinc *= gfx_foliageDensity.integer;

	*viewy = starty;
	
	for (; yinc > 0 ? *viewy<=endy : *viewy>=starty; (*viewy)+=yinc)
	{
		*viewx = startx;
	
		for (; xinc > 0 ? *viewx<=endx : *viewx>=startx; (*viewx)+=xinc)
		{
			p[0] = x;
			p[1] = y;
			p[2] = 0;
			M_SubVec3(p, campos, q);

			len = M_DotProduct(q,q);
			if (len > maxlen)
				continue;

			noise = M_SimpleNoise2(p[0], p[1]);
			absnoise = ABS(noise);

			absnoise *= chance * chance;

			if (thindistsq && len > thindistsq
				&& gfx_foliageNoiseThreshold.value > absnoise)
				continue;

			if (!alphadistsq)
			{
				alpha = 1 - (len - alphadistsq) / alphasize;
			}
			else
			{
				if (len > alphadistsq)
				{
					if (thindistsq && gfx_foliageNoiseThreshold.value > absnoise)
						alpha = 1 - (len - alphadistsq) / (thindistsq - alphadistsq);
					else
					{
						alpha = 1 - (len - alphadistsq) / alphasize;
						alpha -= (1 - alpha) * gfx_foliageAlphaScale.value;
					}
				}
				else
					alpha = 1;
			}

			if (alpha < 0.01)
				continue;

			if (alpha < minalpha)
				minalpha = alpha;

			avgalpha += alpha;
			numalpha++;

			//{
				p[0] += ABS(xinc) * noise;
				p[1] += ABS(yinc) * noise;					
				p[2] = World_GetTerrainHeight(p[0], p[1]);

				if (gfx_foliageDrawPoints.integer)
				{
					glVertex3fv(p);
				}
				else
				{
					height = gfx_foliageHeight.value + absnoise * gfx_foliageRandomSize.value;
						
					p[2] += height/2;
					phase = noise * gfx_foliagePhaseMultiplier.value;
	
					gridref = GRIDREF(tile_x, tile_y);
					
					glNormal3fv(world.normal[0][gridref]);
					glColor4f(wr.colormap[gridref][0]*BYTE_TO_FLOAT, wr.colormap[gridref][1]*BYTE_TO_FLOAT, wr.colormap[gridref][2]*BYTE_TO_FLOAT, alpha);
	
					val = sin((secs+phase)*gfx_foliageAnimateSpeed.value)*gfx_foliageAnimateStrength.value;
	
					GL_DoFoliageVerts(p, gfx_foliageWidth.value, height, val);
				}
			//}
		}
	}

}

//==========================
//
//  GL_DrawFoliage
//
//  the main foliage drawing function
//
//==========================


void	GL_DrawFoliage(vec3_t campos)
{
	int x,y;
	residx_t grass;
	ivec2_t ip;
	vec2_t pos;
	float maxlen;
	int xinc,yinc;
	int startx,starty,endx,endy;
	int i, j;
	int min_tilex, max_tilex, min_tiley, max_tiley;
	float angle;
	int *viewx, *viewy;
	vec3_t p, q;
	OVERHEAD_INIT;

	if (!gfx_foliage.integer)
		return;
	if (cam->flags & CAM_NO_TERRAIN)
		return;
	if (!gfx_foliageFalloff.integer)
		return;

	grass = Res_LoadShader(gfx_foliageShader.string);
	GL_SelectShader(Res_GetShader(grass), cam->time);

	ip[0] = campos[0];
	ip[1] = campos[1];

	ip[0] &= ~31;
	ip[1] &= ~31;

	pos[0] = ip[0];
	pos[1] = ip[1];

	GL_DepthMask(GL_FALSE);
	GL_Enable(GL_DEPTH_TEST);

	if (gfx_foliageDrawPoints.integer)
	{
		GL_Disable(GL_TEXTURE_2D);
		GL_Disable(GL_LIGHTING);
		glPointSize(5);
		glBegin(GL_POINTS);
	}
	else
	{
		GL_Enable(GL_TEXTURE_2D);
		GL_Enable(GL_LIGHTING);
		glBegin(GL_QUADS);
	}
	if (gfx_fog.integer)
	{
		GL_Enable(GL_FOG);
	}
	else
	{
		GL_Disable(GL_FOG);
	}

	//M_MultVec3(cam->viewaxis[2],-1,towards);
	//glNormal3fv(towards);

	maxlen = (gfx_foliageFalloff.value * gfx_foliageFalloff.value);
	//determine rendering order for the grid

	angle = ((int)(M_GetVec2Angle(cam->viewaxis[FORWARD]) + gfx_foliageAngleFudge.integer)) % 360;
	//Console_DPrintf("angle is %f\n", angle);
	
	if (angle < 90)
	{
		//Console_DPrintf("using case 0 for foliage\n");
		viewy = &y;
		viewx = &x;
		yinc = 1;
		xinc = 1;
	}
	else if (angle < 180)
	{
		//Console_DPrintf("using case 1 for foliage\n");
		viewx = &y;
		viewy = &x;
		xinc = 1;
		yinc = -1;
	}
	else if (angle < 270)
	{
		//Console_DPrintf("using case 2 for foliage\n");
		viewy = &y;
		viewx = &x;
		yinc = -1;
		xinc = -1;
	}
	else
	{
		//Console_DPrintf("using case 3 for foliage\n");
		viewx = &y;
		viewy = &x;
		xinc = -1;
		yinc = 1;
	}
	
	starty = (yinc < 0 ? 1 : -1) * (gfx_foliageFalloff.integer + gfx_foliageFudge.value);
	endy = (yinc > 0 ? 1 : -1) * (gfx_foliageFalloff.integer + gfx_foliageFudge.value);
	startx = (xinc < 0 ? 1 : -1) * (gfx_foliageFalloff.integer + gfx_foliageFudge.value);
	endx = (xinc > 0 ? 1 : -1) * (gfx_foliageFalloff.integer + gfx_foliageFudge.value);

	min_tilex = WORLD_TO_GRID(startx + pos[0]);
	max_tilex = WORLD_TO_GRID(endx + pos[0]);
	min_tiley = WORLD_TO_GRID(starty + pos[1]);
	max_tiley = WORLD_TO_GRID(endy + pos[1]);

	*viewy = min_tiley;
	
	for (; yinc > 0 ? *viewy <= max_tiley : *viewy >= max_tiley; (*viewy) += yinc)
	{
		*viewx = min_tilex;
	
		for (; xinc > 0 ? *viewx <= max_tilex : *viewx >= max_tilex; (*viewx) += xinc)
		{

			if (wr.foliageChance[GRIDREF(x, y)] <= 0)
				goto done;

			for (i = 0; i < 2; i++)
			{
				for (j = 0; j < 2; j++)
				{
					p[0] = GRID_TO_WORLD(x+i);
					p[1] = GRID_TO_WORLD(y+j);
					p[2] = 0;
			
					M_SubVec3(p, campos, q);
					if (M_DotProduct(q, cam->viewaxis[FORWARD]) >= 0)
					{
						GL_DrawFoliageOnTile(x, y, campos, maxlen, xinc, yinc);
						goto done;
					}
				}
			}
			
done:
			//hi
		}
	}

	glEnd();
	
	//Console_Printf("num loops: %i, num foliage: %i, lowest alpha %f, avg alpha %f\n", dbg_numloop, numalpha, minalpha, (avgalpha / numalpha));

	GL_DepthMask(GL_TRUE);

	OVERHEAD_COUNT(OVERHEAD_FOLIAGE);

//	Console_Printf("numbil: %i numloop: %i\n", dbg_numbillboards, dbg_numloop);
}
*/

void	GL_TexModulate(float amt);

void	_GL_DrawFoliage(vec3_t campos, vec3_t userpos)
{
	int x,y;
	residx_t grass;
	ivec2_t ip;
	vec2_t pos;
	float maxlen;
	float thindistsq, alphadistsq, alphasize;
	int *viewx, *viewy;
	int xinc,yinc;
	int gridref;
	int startx,starty,endx,endy;
	float r,g,b;
	float secs, randsize;
	float height;
	float val;
	int phase;
	float alpha, angle;
	int tile_x, tile_y;
	float len;
	float noise, absnoise;
	float trampledist;
	vec3_t trampleAngle = { 0,0,1 };
	float chance = 0;

	if (!gfx_foliage.integer)
		return;
	if (cam->flags & CAM_NO_TERRAIN)
		return;
	if (!gfx_foliageFalloff.integer)
		return;

	GL_Enable(GL_BLEND);
	GL_DepthMask(GL_FALSE);
/*
	if (gfx_noBlending.integer)
	{
		GL_Disable(GL_BLEND);
		GL_Enable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, gfx_foliageAlphaRef.value);
		GL_DepthMask(GL_TRUE);
	}
	else
	{
		GL_DepthMask(GL_FALSE);
	}
*/

	grass = Res_LoadShader(gfx_foliageShader.string);
	GL_SelectShader(Res_GetShader(grass), cam->time);

	ip[0] = campos[0];
	ip[1] = campos[1];

	ip[0] &= ~31;
	ip[1] &= ~31;

	pos[0] = ip[0];
	pos[1] = ip[1];

	GL_DepthMask(GL_FALSE);
	GL_Enable(GL_DEPTH_TEST);
	GL_TexModulate(2.0);

	if (gfx_foliageDrawPoints.integer)
	{
		GL_Disable(GL_TEXTURE_2D);
		GL_Disable(GL_LIGHTING);
		glPointSize(5);
		glBegin(GL_POINTS);
		r = g = b = 1;
	}
	else
	{
		GL_Enable(GL_TEXTURE_2D);
		GL_Enable(GL_LIGHTING);
		glBegin(GL_QUADS);
		//r = Cvar_GetValue("obj_light0_r");
		//g = Cvar_GetValue("obj_light0_g");
		//b = Cvar_GetValue("obj_light0_b");
		r = g = b = 1;
	}
	if (gfx_fog.integer)
	{
		GL_Enable(GL_FOG);
	}
	else
	{
		GL_Disable(GL_FOG);
	}

	secs = Host_Milliseconds() / 1000.0;
	
	maxlen = (gfx_foliageFalloff.value * gfx_foliageFalloff.value) + 10;
	thindistsq = maxlen * gfx_foliageThinDistance.value;
	if (thindistsq >= maxlen)
		thindistsq = 0;
	alphadistsq = maxlen * gfx_foliageAlphaDistance.value;
	alphasize = maxlen - alphadistsq;

	trampledist = gfx_foliageTrampleDist.value * gfx_foliageTrampleDist.value;

	//determine rendering order for the grid

	angle = ((int)(M_GetVec2Angle(cam->viewaxis[FORWARD]) + gfx_foliageAngleFudge.integer)) % 360;
//	Console_DPrintf("angle is %f\n", angle);
	
	if (angle < 90)
	{
		//Console_DPrintf("using case 0 for foliage\n");
		viewy = &y;
		viewx = &x;
		yinc = 1;
		xinc = 1;
	}
	else if (angle < 180)
	{
		//Console_DPrintf("using case 1 for foliage\n");
		viewx = &y;
		viewy = &x;
		xinc = 1;
		yinc = -1;
	}
	else if (angle < 270)
	{
		//Console_DPrintf("using case 2 for foliage\n");
		viewy = &y;
		viewx = &x;
		yinc = -1;
		xinc = -1;
	}
	else
	{
		//Console_DPrintf("using case 3 for foliage\n");
		viewx = &y;
		viewy = &x;

		xinc = -1;
		yinc = 1;
	}
	
	starty = (yinc < 0 ? 1 : -1) * (gfx_foliageFalloff.integer + gfx_foliageFudge.value);
	endy = (yinc > 0 ? 1 : -1) * (gfx_foliageFalloff.integer + gfx_foliageFudge.value);
	startx = (xinc < 0 ? 1 : -1) * (gfx_foliageFalloff.integer + gfx_foliageFudge.value);
	endx = (xinc > 0 ? 1 : -1) * (gfx_foliageFalloff.integer + gfx_foliageFudge.value);

//	startx /= gfx_foliageDensity.integer;
//	starty /= gfx_foliageDensity.integer;
//	endx /= gfx_foliageDensity.integer;
//	endy /= gfx_foliageDensity.integer;
	xinc *= gfx_foliageDensity.integer;
	yinc *= gfx_foliageDensity.integer;

	//if (gfx_noBlending.integer)
	//	alphadistsq = 999999999;  //force alpha to always be 1 without an extra branch
			
	if (yinc > 0)
		*viewy = starty;
	else
		*viewy = starty - (starty % yinc)*2;
	
	for (; yinc > 0 ? *viewy<=endy : *viewy>=endy; (*viewy)+=yinc)
	{
		if (xinc > 0)
			*viewx = startx;
		else
			*viewx = startx - (startx % xinc)*2;
	
		for (; xinc > 0 ? *viewx<=endx : *viewx>=endx; (*viewx)+=xinc)
		{
			vec3_t p = { pos[0]+x, pos[1]+y, 0 };
			vec3_t q;

			tile_x = WORLD_TO_GRID(p[0]);
			tile_y = WORLD_TO_GRID(p[1]);

			gridref = GRIDREF(tile_x, tile_y);

			if (tile_x < 0 || tile_x > world.gridwidth
			    || tile_y < 0 || tile_y > world.gridheight)
				continue;
			
			//if (gridref != last_gridref)
			//{
			//	last_gridref = gridref;
				chance = wr.foliageChance[gridref]; //WR_FOLIAGECHANCE(tile_x, tile_y);
			//}
			M_SubVec3(p, campos, q);
			len = M_DotProduct(q,q);
			if (chance <= 0 || len > maxlen || M_DotProduct(q, cam->viewaxis[FORWARD]) < 0)
				continue;

			noise = M_SimpleNoise2(p[0],p[1]);
			absnoise = (noise + 1)/2;

			absnoise *= chance * chance;

			if ((thindistsq && len > thindistsq)
				|| gfx_foliageSmoothThreshold.value > absnoise)
				continue;

			/*
			if (!alphadistsq)
			{
				alpha = 1 - (len - alphadistsq) / alphasize;

				if (alpha < 0.01)
					continue;
			}
			else
			{
			*/
				if (len > alphadistsq)
				{
					if (thindistsq && gfx_foliageNoiseThreshold.value > absnoise)
						alpha = 1 - (len - alphadistsq) / (thindistsq - alphadistsq);
					else
					{
						alpha = 1 - (len - alphadistsq) / alphasize;
						alpha -= (1 - alpha) * gfx_foliageAlphaScale.value;
					}

					if (alpha < 0.01)
						continue;
				}
				else
					alpha = 1;
			//}

			p[0] += ABS(xinc) * noise;
			p[1] += ABS(yinc) * noise;					
			p[2] = World_GetTerrainHeight(p[0],p[1]);

// KJS
// not used per JPS
//			if (trampledist)
//			{
//				M_SubVec3(p, userpos, q);
//				len = M_DotProduct(q,q);
//				if (len < trampledist)
//				{
//					M_SetVec3(trampleAngle, q[0], q[1], 1 - len/trampledist);
//					M_Normalize(trampleAngle);
//				}
//				else
//				{
//					M_SetVec3(trampleAngle, 0, 0, 1);
//				}
//			}

			//if (gfx_foliageDrawPoints.integer)
			//{
			//	glVertex3fv(p);
			//}
			//else
			//{
				randsize = noise * gfx_foliageRandomSize.value;
				height = gfx_foliageHeight.value + randsize;
					
				p[2] += height/2;
				phase = noise * gfx_foliagePhaseMultiplier.value;

				glNormal3fv(world.normal[0][gridref]);

				glColor4f(wr.colormap[gridref][0]*BYTE_TO_FLOAT, wr.colormap[gridref][1]*BYTE_TO_FLOAT, wr.colormap[gridref][2]*BYTE_TO_FLOAT, alpha);

				val = sin((secs+phase)*gfx_foliageAnimateSpeed.value)*gfx_foliageAnimateStrength.value;

				GL_DoFoliageVerts(p, gfx_foliageWidth.value + randsize, height, val, trampleAngle);
			//}
		}
	}
	glEnd();
	
/*
	//restore states
	if (gfx_noBlending.integer)
	{
		//re-enable blending and disable alpha test
		GL_Enable(GL_BLEND);
		GL_Disable(GL_ALPHA_TEST);
		
	}
*/
	
	GL_DepthMask(GL_TRUE);
//	Console_Printf("numbil: %i numloop: %i\n", dbg_numbillboards, dbg_numloop);
}


void	GL_DrawFoliage(vec3_t campos, vec3_t userpos)
{
	OVERHEAD_INIT;
	_GL_DrawFoliage(campos, userpos);
	OVERHEAD_COUNT(OVERHEAD_SCENE_DRAWFOLIAGE);
}


/*==========================

  GL_Notify

 ==========================*/

void GL_Notify(int message, int param1, int param2, int param3)
{
	switch(message)
	{
		case VID_NOTIFY_NEW_WORLD:
			//precache the cloud layer shader
			if (gfx_clouds.integer)
				Res_LoadShader(gfx_cloudShader.string);
			//precache the foliage shader
			if (gfx_foliage.integer)
				Res_LoadShader(gfx_foliageShader.string);
			GL_RebuildTerrain(terrain_chunksize.integer);
			//increase the ref count so we can expire old static geometry
			Cvar_SetVarValue(&gfx_renderDataRefCount, gfx_renderDataRefCount.integer + 1);
			break;
		case VID_NOTIFY_WORLD_DESTROYED:
			GL_DestroyTerrain();
			break;
		case VID_NOTIFY_TERRAIN_COLOR_MODIFIED:
			if (param3)
				GL_InvalidateTerrainLayer(0, TERRAIN_REBUILD_COLORS | TERRAIN_REBUILD_TEXCOORDS);
			else
				GL_InvalidateTerrainVertex(0, param1, param2, TERRAIN_REBUILD_COLORS);
			break;
		case VID_NOTIFY_TERRAIN_NORMAL_MODIFIED:
			if (param3)
				GL_InvalidateTerrainLayer(0, TERRAIN_REBUILD_NORMALS | TERRAIN_REBUILD_TEXCOORDS);
			else
				GL_InvalidateTerrainVertex(0, param1, param2, TERRAIN_REBUILD_NORMALS);
			break;
		case VID_NOTIFY_TERRAIN_VERTEX_MODIFIED:
			if (param3)
				GL_InvalidateTerrainLayer(0, TERRAIN_REBUILD_VERTICES);
			else
				GL_InvalidateTerrainVertex(0, param1, param2, TERRAIN_REBUILD_VERTICES);
			break;
		case VID_NOTIFY_TERRAIN_SHADER_MODIFIED:
			if (param3)
			{
				GL_InvalidateTerrainLayer(0, TERRAIN_REBUILD_SHADERS);
				GL_InvalidateTerrainLayer(1, TERRAIN_REBUILD_SHADERS);
			}
			else
			{
				GL_InvalidateTerrainVertex(0, param1, param2, TERRAIN_REBUILD_SHADERS);
				GL_InvalidateTerrainVertex(1, param1, param2, TERRAIN_REBUILD_SHADERS);
			}
			break;
		case VID_NOTIFY_TERRAIN_TEXCOORD_MODIFIED:
			if (param3)
				GL_InvalidateTerrainLayer(0, TERRAIN_REBUILD_TEXCOORDS);
			else
				GL_InvalidateTerrainVertex(0, param1, param2, TERRAIN_REBUILD_TEXCOORDS);
			break;
	}
}





/*==========================

  GL_SetupLighting

  setup opengl lighting based on a worldLight array


 ==========================*/


extern cvar_t gfx_maxDLights;
extern cvar_t gfx_dynamicLighting;

int glLight[] =
{
	GL_LIGHT0,
	GL_LIGHT1,
	GL_LIGHT2,
	GL_LIGHT3,
	GL_LIGHT4,
	GL_LIGHT5,
	GL_LIGHT6,
	GL_LIGHT7, 
	0
};

void	GL_SetupLighting(worldLight_t wl[])
{
	int n;	
	int freelights[8];	
	int numfree = 0;

	OVERHEAD_INIT;

	if (wl)
	{
		for (n=0; n<MAX_WORLD_LIGHTS; n++)
		{
			vec4_t ambient, diffuse, position;

			if (strcmp(wl[n].type.string, "off")==0)
			{
				//GL_Disable(glLight[n]);
				freelights[numfree++] = n;
				continue;
			}
			else if (strcmp(wl[n].type.string, "sun")==0)
			{
				SET_VEC4(ambient, 0, 0, 0, 1);
				SET_VEC4(diffuse, wl[n].r.value*0.5,wl[n].g.value*0.5,wl[n].b.value*0.5, 0.0);
				SET_VEC4(position, -wr_sun_x.value, -wr_sun_y.value, -wr_sun_z.value, 0.0);

			}
			else if (strcmp(wl[n].type.string, "vector")==0)
			{
				SET_VEC4(ambient, 0,0,0,1);
				SET_VEC4(diffuse, wl[n].r.value*0.5,wl[n].g.value*0.5,wl[n].b.value*0.5,0 );
				SET_VEC4(position, -wl[n].x.value, -wl[n].y.value, -wl[n].z.value, 0.0 );
			}
			else if (strcmp(wl[n].type.string, "rim")==0)
			{
				SET_VEC4(ambient, 0,0,0,1);
				SET_VEC4(diffuse, wl[n].r.value*0.5,wl[n].g.value*0.5,wl[n].b.value*0.5,0 );
				Cam_ConstructRay(cam, cam->width*wl[n].x.value, cam->height*wl[n].y.value, position);		
				//M_MultVec3(position,-1,position);
				position[3] = 0;
			}
			else if (strcmp(wl[n].type.string, "forward")==0)
			{
				SET_VEC4(ambient, 0,0,0,1);
				SET_VEC4(diffuse, wl[n].r.value*0.5,wl[n].g.value*0.5,wl[n].b.value*0.5,0 );
				Cam_ConstructRay(cam, cam->width*wl[n].x.value, cam->height*wl[n].y.value, position);		
				M_MultVec3(position,-1,position);
				position[3] = 0;
			}
			else
			{
				//GL_Disable(glLight[n]);
				freelights[numfree++] = n;
				continue;
			}

			glLightfv(glLight[n], GL_AMBIENT, ambient);
			glLightfv(glLight[n], GL_DIFFUSE, diffuse);
			glLightfv(glLight[n], GL_POSITION, position);

			GL_Enable(glLight[n]);
		}
	}

	for (n=MAX_WORLD_LIGHTS; n<8; n++)
		freelights[numfree++] = n;

	if (gfx_dynamicLighting.integer)
	{
		scenelightlist_t *list = scenelightlist;

		//dynamic lights
		for (n=0; n<gfx_maxDLights.integer; n++)
		{
			int lightnum = glLight[freelights[n]];

			if (!list)
				break;

			glLightfv(lightnum, GL_AMBIENT, vec4(0,0,0,1));
			glLightfv(lightnum, GL_DIFFUSE, list->light.color);
			glLightfv(lightnum, GL_POSITION, vec4(list->light.pos[0], list->light.pos[1], list->light.pos[2], 1));
			glLightf(lightnum,	GL_CONSTANT_ATTENUATION, 0);
			glLightf(lightnum, GL_LINEAR_ATTENUATION, 1/list->light.intensity);
			glLightf(lightnum, GL_QUADRATIC_ATTENUATION, 0);

			GL_Enable(lightnum);

			freelights[n] = -1;

			list = list->next;
		}
	}

	for (n=0; n<numfree; n++)
	{
		if (freelights[n] != -1)
			GL_Disable(glLight[freelights[n]]);
	}

	OVERHEAD_COUNT(OVERHEAD_SCENE_SETUP);

	//glSecondaryColor3fEXT(1,1,1);
	//glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, vec4(0.5,0.5,0.5,1));
}



void	GL_SelectObjectShader(sceneobj_t *obj, shader_t *shader)
{
	if (obj->flags & SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME)
	{
		GL_SelectShader(shader, SHD_CALCULATETIME(obj->loframe, shader->fps));
	}
	else
	{
		GL_SelectShader(shader, cam->time - obj->creation_time);
	}
}

//a simple hack for water until we get 'real' water in the game
void	GL_DrawWaterHack()
{
	shader_t *shader;


	if (!gfx_water_hack.integer)
		return;

	GL_Enable(GL_TEXTURE_2D);
	GL_Disable(GL_CULL_FACE);

	shader = Res_GetShader(Res_LoadShader(gfx_watershader.string));
	GL_SelectShader(shader, cam->time);

	glColor4f(gfx_waterr.value, gfx_waterg.value, gfx_waterb.value, gfx_wateralpha.value);

	glBegin(GL_QUADS);

		glNormal3f(0,0,1);
		glTexCoord2f(gfx_waterscale.value,0);
		glVertex3f(world.tree[0]->bmax[0], world.tree[0]->bmin[1], gfx_waterlevel.value);
		glTexCoord2f(gfx_waterscale.value,gfx_waterscale.value);
		glVertex3f(world.tree[0]->bmax[0], world.tree[0]->bmax[1], gfx_waterlevel.value);
		glTexCoord2f(0,gfx_waterscale.value);
		glVertex3f(world.tree[0]->bmin[0], world.tree[0]->bmax[1], gfx_waterlevel.value);
		glTexCoord2f(0,0);
		glVertex3f(world.tree[0]->bmin[0], world.tree[0]->bmin[1], gfx_waterlevel.value);

	glEnd();
}

void	GL_DrawWater()
{
	OVERHEAD_INIT;

	if (cam->flags & CAM_NO_TERRAIN)
		return;

	GL_DrawWaterHack();

	OVERHEAD_COUNT(OVERHEAD_SCENE_DRAWWATER);
}






/*==========================

  GL_DrawSky

  sky rendering just goes through the list and renders each object one by one
  no depth writing or testing is done
  we could improve performance by doing this last, and performing depth testing

 ==========================*/

void	GL_DrawSky()
{
	int oldblend = gfx_noBlending.integer;
	scenelist_t *list;

	OVERHEAD_INIT;

	if (cam->flags & CAM_NO_SKY || !gfx_sky.integer)
		return;

	
	GL_Enable(GL_TEXTURE_2D);
	GL_Disable(GL_LIGHTING);
	GL_Disable(GL_FOG);
	GL_DepthMask(GL_FALSE);
	GL_Disable(GL_DEPTH_TEST);
/*
	if (oldblend)
		Cvar_SetVarValue(&gfx_noBlending, 0);
*/
	for (list=skylist; list; list=list->next)
	{
		switch (list->obj.objtype)
		{
			case OBJTYPE_MODEL:				
				if (GL_AddModelToLists(&list->obj))
				{
					//render the lists right away to maintain drawing order
					GL_RenderLists();
					GL_ClearLists();
				}
				break;
			case OBJTYPE_BILLBOARD:
				glColor4fv(list->obj.color);
				GL_DepthMask(GL_FALSE);
				GL_SelectShader(Res_GetShader(list->obj.shader), Host_Milliseconds() / 1000.0);
				GL_DrawBillboard(list->obj.pos, list->obj.width, list->obj.height, list->obj.angle[0], list->obj.s1, list->obj.t1, list->obj.s2, list->obj.t2, list->obj.flags & SCENEOBJ_BILLBOARD_ALL_AXES);
				break;
		}
	}
/*
	if (oldblend)
		Cvar_SetVarValue(&gfx_noBlending, oldblend);
*/
	glColor4fv(white);

	OVERHEAD_COUNT(OVERHEAD_SCENE_DRAWSKY);
}



/*==========================

  GL_BeginCloudLayer

  uses the specified texunit to render projection of cloud shadows over everything

 ==========================*/

void	GL_BeginCloudLayer(int texunit)
{
	residx_t cloudshader = Res_LoadShader(gfx_cloudShader.string);

	if (cloudshader && gfx_clouds.integer)
	{				
		float cloudX, cloudY;

		GL_SwitchTexUnit(GL_TEXTURE1_ARB);

		GL_Enable(GL_TEXTURE_2D);
		GL_SelectShader(Res_GetShader(cloudshader), cam->time);
		
		GL_Enable(GL_TEXTURE_GEN_S);
		GL_Enable(GL_TEXTURE_GEN_T);
		//automatically generate texture coordinates
		//we'll generate them linearly based on the width of a terrain tile		

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGenfv(GL_S, GL_EYE_PLANE, vec4(0.01,0,0,0));
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGenfv(GL_T, GL_EYE_PLANE, vec4(0,-0.01,0,0));
		
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_GetTexEnvModeFromString(gfx_cloudBlendMode.string));
			
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();					

		cloudX = gfx_cloudSpeedX.value * (Host_Milliseconds() / 1000.0);
		cloudY = gfx_cloudSpeedY.value * (Host_Milliseconds() / 1000.0);

		glScalef(1 / gfx_cloudScale.value, 1 / gfx_cloudScale.value, 1);
		glTranslatef(cloudX, cloudY, 0);		
		
		glColor4f(1,1,1,1);

		glMatrixMode(GL_MODELVIEW);

		GL_SwitchTexUnit(GL_TEXTURE0_ARB);		
	}
}


/*==========================

  GL_EndCloudLayer

  disable cloud layer on the specified texture unit

 ==========================*/

void	GL_EndCloudLayer(int texunit)
{
	if (gfx_clouds.integer)
	{
		GL_SwitchTexUnit(GL_TEXTURE1_ARB);
				
		GL_Disable(GL_TEXTURE_2D);
		GL_Disable(GL_TEXTURE_GEN_S);
		GL_Disable(GL_TEXTURE_GEN_T);

		glMatrixMode(GL_TEXTURE);
		//glLoadIdentity();
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		GL_SwitchTexUnit(GL_TEXTURE0_ARB);		
	}
}



/*==========================

  GL_SetupScene

  set up some initial variables before we draw the scene

 ==========================*/

void	GL_SetupScene(camera_t *camera)
{
	vec4_t fog_color = { gfx_fogr.value, gfx_fogg.value, gfx_fogb.value, 0 };

	OVERHEAD_INIT;

	//set up a few global vars
	screenw = Vid_GetScreenW();
	screenh = Vid_GetScreenH();
	cam = camera;

	//fog

	if (gfx_fog.value) 
	{
		GL_Enable(GL_FOG);
		glFogf(GL_FOG_MODE, GL_LINEAR);
		if (cam->fog_near > 0 && cam->fog_far >= cam->fog_near)
		{
			glFogf(GL_FOG_START, cam->fog_near);
			glFogf(GL_FOG_END, cam->fog_far);			
		}
		else
		{
			glFogf(GL_FOG_START, gfx_fog_near.value);
			glFogf(GL_FOG_END, gfx_fog_far.value);
		}
		glFogfv(GL_FOG_COLOR, fog_color);
	}
	else
		GL_Disable(GL_FOG);

	//enable depth testing and alpha blending

	GL_Enable(GL_DEPTH_TEST);
	GL_Enable(GL_BLEND);

	//enable antialiasing if it's turned on

	if (gfx_antialias.value)
		GL_Enable(GL_POLYGON_SMOOTH);
	else
		GL_Disable(GL_POLYGON_SMOOTH);

	//setup specular setting

	if (gfx_separate_specular_color.integer)
	{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);	
	}
	else
	{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SINGLE_COLOR_EXT);
	}

	//tie ambient and diffuse material colors together?
	
	if (gfx_ambient_and_diffuse.integer)
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	else
		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	//disable alpha test

	GL_Disable(GL_ALPHA_TEST);

	//set up the camera

	GL_PushCamera(camera);

	//clear the depth buffer

	glClear(GL_DEPTH_BUFFER_BIT);

	GL_ClearLists();

	GL_Enable(GL_CULL_FACE);

	OVERHEAD_COUNT(OVERHEAD_SCENE_SETUP);
}


/*==========================

  GL_UseTerrainLighting

 ==========================*/

void	GL_UseTerrainLighting()
{
	vec4_t ter_ambient = { ter_ambient_r.value * 0.5, ter_ambient_g.value * 0.5, ter_ambient_b.value * 0.5, 1 };

	//set up terrain lights
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ter_ambient);
	GL_SetupLighting(terLights);
}

/*==========================

  GL_UseObjectLighting

 ==========================*/

void	GL_UseObjectLighting()
{
	vec4_t obj_ambient = { obj_ambient_r.value * 0.5, obj_ambient_g.value * 0.5, obj_ambient_b.value * 0.5, 1 };

	//set up terrain lights
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, obj_ambient);
	GL_SetupLighting(objLights);
}


/*==========================

  GL_DrawTerrain

 ==========================*/

void	GL_DrawTerrain()
{
	OVERHEAD_INIT;

	if (cam->flags & CAM_NO_TERRAIN)
		return;
	
	GL_DepthMask(GL_TRUE);
	GL_Enable(GL_LIGHTING);

	if (world.cl_loaded && !(cam->flags & CAM_NO_TERRAIN)) 
	{
		GL_RenderTerrain();			
	}

	OVERHEAD_COUNT(OVERHEAD_SCENE_DRAWTERRAIN);
}


/*==========================

  GL_DrawObjects

  draw all models

 ==========================*/

void	GL_DrawObjects()
{
	scenelist_t *list;
	OVERHEAD_INIT;
	
	GL_Enable(GL_TEXTURE_2D);
	GL_Enable(GL_LIGHTING);
	if (gfx_fog.integer)
		GL_Enable(GL_FOG);
	else
		GL_Disable(GL_FOG);
	GL_DepthMask(GL_TRUE);
	GL_Enable(GL_DEPTH_TEST);

	for (list = scenelist; list; list = list->next)
	{
		if (list->cull)
			continue;

		switch (list->obj.objtype)
		{
			case OBJTYPE_MODEL:
				GL_AddModelToLists(&list->obj);
				break;
			default:
				break;
		}
	}

	GL_RenderLists();
	GL_ClearLists();

	OVERHEAD_COUNT(OVERHEAD_SCENE_DRAWOBJECTS);
}


/*==========================

  GL_DrawSprites

  draw all particles and beams

 ==========================*/

void	GL_DrawSprites()
{
	shader_t *shader;
	scenelist_t *list;
	GLboolean fog = glIsEnabled(GL_FOG);

	OVERHEAD_INIT;

	if (!gfx_drawSprites.integer)
		return;

	GL_DepthMask(GL_FALSE);
	GL_Disable(GL_LIGHTING);
	GL_TexModulate(1.0);

	for (list = spritelist; list; list = list->next)
	{
		if (list->cull)
			continue;

		shader = Res_GetShader(list->obj.shader);
		if (shader->srcblend == BLEND_ONE && shader->dstblend == BLEND_ONE)
		{
			//disable fog for additive textures
			if (fog)
				GL_Disable(GL_FOG);
		}
		else
		{
			if (fog)
				GL_Enable(GL_FOG);
		}

		switch (list->obj.objtype)
		{
			case OBJTYPE_BILLBOARD:
				glColor4fv(list->obj.color);					
				GL_SelectObjectShader(&list->obj, Res_GetShader(list->obj.shader));
				//if (

				GL_DrawBillboard(
					list->obj.pos, 
					list->obj.width, 
					list->obj.height, 
					list->obj.angle[0], 
					list->obj.s1, 
					list->obj.t1, 
					list->obj.s2,
					list->obj.t2,
					list->obj.flags & SCENEOBJ_BILLBOARD_ALL_AXES);

				break;
			case OBJTYPE_BEAM:
				glColor4fv(list->obj.color);					
				GL_SelectObjectShader(&list->obj, Res_GetShader(list->obj.shader));

				GL_DrawBeam(
					list->obj.pos,
					list->obj.beamTargetPos,
					list->obj.scale,
					list->obj.height
					);

				break;
		}
	}

	GL_DepthMask(GL_TRUE);

	OVERHEAD_COUNT(OVERHEAD_SCENE_DRAWSPRITES);
}



/*==========================

  GL_DrawScenePolys

  draw all polygons added with Scene_AddPoly

 ==========================*/

void	GL_DrawScenePolys()
{
	scenefacelist_t *list;

	OVERHEAD_INIT;

	GL_DepthMask(GL_FALSE);
	GL_Disable(GL_LIGHTING);
	GL_TexModulate(1.0);
	GL_Enable(GL_BLEND);

	for (list = scenefacelist; list; list = list->next)
	{
		int n;
		scenefacevert_t *verts = list->verts;

		GL_SelectShader(Res_GetShader(list->shader), Host_Milliseconds() / 1000.0);

		if (list->flags & POLY_NO_DEPTH_TEST)
			GL_Disable(GL_DEPTH_TEST);
		else
			GL_Enable(GL_DEPTH_TEST);

		if (list->flags & POLY_WIREFRAME)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBegin(GL_POLYGON);
		for (n=0; n<list->nverts; n++)
		{
			glColor4ubv(verts[n].col);
			glTexCoord2fv(verts[n].tex);
			glVertex3f(verts[n].vtx[0], verts[n].vtx[1], verts[n].vtx[2]);
			glNormal3f(0,0,1);	//fixme: hack for groundplane lighting			
		}
		glEnd();

		if (list->flags & POLY_WIREFRAME)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		sceneStats.polycount += list->nverts - 2;
	}

	GL_DepthMask(GL_TRUE);

	OVERHEAD_COUNT(OVERHEAD_SCENE_DRAWSCENEPOLYS);
}

/*==========================

  GL_DrawSkeletons


 ==========================*/

extern cvar_t gfx_drawBones;

void	GL_DrawSkeletons()
{
	scenelist_t *list;

	if (!gfx_drawBones.integer)
		return;

	for (list = scenelist; list; list = list->next)
	{
		if (list->cull)
			continue;

		switch (list->obj.objtype)
		{
			case OBJTYPE_MODEL:
			{
				if (list->obj.skeleton)
				{
					glPushMatrix();
					GL_LoadObjectMatrix(&list->obj);
					GL_DrawSkeleton(0, list->obj.skeleton, Res_GetModel(list->obj.model));
					glPopMatrix();
				}
				break;
			}
			default:
				break;
		}
	}
}





/*==========================

  GL_DrawRTSSilhouettes

  this doesn't work due to an apparent bug in the opengl driver

 ==========================*/

void	GL_DrawRTSSilhouettes()
{
	scenelist_t *list;

	if (!gfx_drawSilhouettes.integer)
		return;

	GL_Disable(GL_LIGHTING);
	GL_Disable(GL_TEXTURE_2D);	
	GL_DepthMask(GL_FALSE);	
	glDepthFunc(GL_GREATER);

	for (list = scenelist; list; list = list->next)
	{
		if (list->cull)
			continue;

		switch(list->obj.objtype)
		{
			case OBJTYPE_MODEL:
				if (list->obj.flags & SCENEOBJ_RTS_SILHOUETTE)
				{
					model_t *model = Res_GetModel(list->obj.model);
					int n;

					glPushMatrix();		
					GL_LoadObjectMatrix(&list->obj);

					for (n=0; n<model->num_meshes; n++)
					{
						GL_SetColor(vec4(gfx_silhouetteR.value, gfx_silhouetteG.value, gfx_silhouetteB.value, 1));					
						GL_DrawMesh(&list->obj, &model->meshes[n]);
					}

					glPopMatrix();
				}
				break;
			default:
				break;
		}
	}

	glDepthFunc(GL_LESS);
	glDepthRange(0,1);
}


/*==========================

  GL_Cleanup

 ==========================*/

void	GL_Cleanup()
{
	OVERHEAD_INIT;

	GL_Enable(GL_TEXTURE_2D);
	GL_Disable(GL_LIGHTING);
	GL_DepthMask(GL_TRUE);

	GL_PopCamera();
	GL_2dMode();

	OVERHEAD_COUNT(OVERHEAD_SCENE_CLEANUP);
}


/*==========================

  GL_RenderScene

  The main drawing function

 ==========================*/

void	GL_RenderScene(camera_t *camera, vec3_t userpos)
{
	if (!world.cl_loaded || !gfx_render.integer)
	{
		GL_2dMode();
		return;
	}

	//initialize the scene: setup the camera, clear the depth buffer, set some global vars
	GL_SetupScene(camera);

	//sky, sun, moon, whatever was added with Scene_AddSkyObject
	GL_DrawSky();

	//everything we render from now on will be affected by clouds
	GL_BeginCloudLayer(GL_TEXTURE1_ARB);

		//render terrain using object lighting
		GL_UseTerrainLighting();

		//draw the base terrain surface
		GL_DrawTerrain();

		GL_UseObjectLighting();
		
		//draw characters and object models
		GL_DrawObjects();

		//go back to terrain lighting for water and grass
		GL_UseTerrainLighting();

		//draw water
		GL_DrawWater();

		//draw awesome grass
		GL_DrawFoliage(cam->origin, userpos);

	//no more clouds
	GL_EndCloudLayer(GL_TEXTURE1_ARB);

	//draw polygons added with Scene_AddPoly()
	//these are generally decal type things, so 
	//it's a good idea to draw them before sprites
	GL_DrawScenePolys();

	//particle effects and other billboarded effects
	GL_DrawSprites();

	//special rendering for the RTS viewpoint
	GL_DrawRTSSilhouettes();

	//skeleton drawing, for debugging
	GL_DrawSkeletons();

	GL_Cleanup();
}







/*==========================

  GL_InitScene

 ==========================*/

void	GL_InitScene(void)
{
	Cvar_Register(&gfx_antialias);
	Cvar_Register(&gfx_waterr);
	Cvar_Register(&gfx_waterg);
	Cvar_Register(&gfx_waterb);
	Cvar_Register(&gfx_wateralpha);
	Cvar_Register(&gfx_water_hack);
	Cvar_Register(&gfx_waterlevel);
	Cvar_Register(&gfx_waterscale);
	Cvar_Register(&gfx_watershader);
	Cvar_Register(&gfx_separate_specular_color);
	Cvar_Register(&gfx_ambient_and_diffuse);	
	Cvar_Register(&terrain_chunksize);	
	Cvar_Register(&gfx_foliage);
	Cvar_Register(&gfx_foliageDensity);
	Cvar_Register(&gfx_foliageFalloff);
	Cvar_Register(&gfx_foliageThinDistance);
	Cvar_Register(&gfx_foliageAlphaDistance);
	Cvar_Register(&gfx_foliageAlphaScale);
	Cvar_Register(&gfx_foliageSmooth);
	Cvar_Register(&gfx_foliageSmoothThreshold);
	Cvar_Register(&gfx_foliageMaxSlope);
	Cvar_Register(&gfx_foliageCurve);
	Cvar_Register(&gfx_foliageWidth);
	Cvar_Register(&gfx_foliageHeight);
	Cvar_Register(&gfx_foliageRandomSize);
	Cvar_Register(&gfx_foliageDrawPoints);
	Cvar_Register(&gfx_foliageAnimateStrength);
	Cvar_Register(&gfx_foliageAnimateSpeed);
	Cvar_Register(&gfx_foliageShader);
	Cvar_Register(&gfx_foliageNoiseThreshold);
	Cvar_Register(&gfx_foliageAngleFudge);
	Cvar_Register(&gfx_foliageFudge);
	Cvar_Register(&gfx_foliageTrampleDist);
	Cvar_Register(&gfx_render);
	Cvar_Register(&gfx_foliagePhaseMultiplier);
	Cvar_Register(&gfx_clouds);
	Cvar_Register(&gfx_cloudShader);
	Cvar_Register(&gfx_cloudScale);
	Cvar_Register(&gfx_cloudSpeedX);
	Cvar_Register(&gfx_cloudSpeedY);
	Cvar_Register(&gfx_cloudBlendMode);
	Cvar_Register(&gfx_renderDataRefCount);
	Cvar_Register(&gfx_drawSprites);
	Cvar_Register(&gfx_drawSilhouettes);
	Cvar_Register(&gfx_silhouetteR);
	Cvar_Register(&gfx_silhouetteG);
	Cvar_Register(&gfx_silhouetteB);
	Cvar_Register(&gfx_foliageAlphaRef);
	Cvar_Register(&gfx_foliageMethod);
	Cvar_Register(&gfx_whiteHud);


	GL_InitSceneBuilder();
	GL_InitTerrain();
	GL_InitModel();

	if (gl_ext_texenv_combine.integer)  //we'll be using 2x modulation
	{
		Cvar_Set("vid_overbrightNormalize", "0.5");
	}
}
