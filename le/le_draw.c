// (C) 2003 S2 Games

// le_draw.c

// client side game drawing functions

#include "../le/le.h"

vec4_t activebuttonleft = {0,1,0,1};
vec4_t activebuttonright = {0,0,1,1};
vec4_t buttoncolor = {0.1,0.1,0.8,0.3};
vec4_t buttoncolor2 = {1,1,1,1};

extern cvar_t tl_camera_x;
extern cvar_t tl_camera_y;

extern cvar_t tl_camera_pos_x;
extern cvar_t tl_camera_pos_y;
extern cvar_t tl_camera_pos_z;

extern cvar_t tl_camera_angle_x;
extern cvar_t tl_camera_angle_y;
extern cvar_t tl_camera_angle_z;

extern cvar_t tl_camera_fovx;
extern cvar_t tl_camera_zoom;

extern cvar_t le_scriptedcamera;

extern cvar_t le_particle_pos_x;
extern cvar_t le_particle_pos_y;
extern cvar_t le_particle_pos_z;

extern cvar_t le_testscale;
extern cvar_t le_testangle;
extern cvar_t le_testfixpos;
extern cvar_t le_testanim;

extern cvar_t le_drawElevation;

static residx_t cursorShader;
static int hotspotx, hotspoty;

float modelframe;

void	LE_SetCursor(residx_t shader)
{
	cursorShader = shader;
}

void	LE_SetHotspot(int x, int y)
{
	hotspotx = x;
	hotspoty = y;
}

void	LE_Draw_Cursor ()
{
	corec.Draw_SetColor(white);
	LE_Quad2d_S(le.mousepos.x-hotspotx, le.mousepos.y-hotspoty, 32, 32, cursorShader);	
}

void	CopyBVec4(bvec4_t in, bvec4_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}


void	LE_CalcCastedShadow(const vec3_t origin, const vec3_t dir, float zvalue, float width, float length, scenefacevert_t v[4])
{
	vec3_t perp, d, o, mag;

	M_CopyVec3(dir, d);
	d[2] = 0;
	M_NormalizeVec2(d);
	M_MultVec3(d, length, mag);

	//perpendicular vector
	perp[0] = -d[1];
	perp[1] = d[0];
	perp[2] = 0;
	M_MultVec3(perp, width / 2, perp);

	M_CopyVec3(origin, o);
	//M_SubVec3(o, d, o);

	//fixme: tex coords are modified here to account for strange 'edge' artifact on shadow quads
	M_AddVec3(o, perp, v[0].vtx);
	SET_VEC2(v[0].tex, 0.99, 0.99);
	M_SubVec3(o, perp, v[1].vtx);
	SET_VEC2(v[1].tex, 0.01, 0.99);
	M_AddVec3(v[1].vtx, mag, v[2].vtx);
	SET_VEC2(v[2].tex, 0.01, 0.01);
	M_AddVec3(v[0].vtx, mag, v[3].vtx);
	SET_VEC2(v[3].tex, 0.99, 0.01);

	v[0].vtx[2] = v[1].vtx[2] = v[2].vtx[2] = v[3].vtx[2] = zvalue;

	SET_VEC4(v[0].col, le_shadowr.value * 255, le_shadowg.value * 255, le_shadowb.value * 255, le_shadowalpha.value * 255);
	CopyBVec4(v[0].col, v[1].col);
	CopyBVec4(v[0].col, v[2].col);
	CopyBVec4(v[0].col, v[3].col);
}

void LE_AddShadow(sceneobj_t *obj)
{
	pointinfo_t pi;
	vec3_t sv;
	vec3_t bmin, bmax;
	scenefacevert_t v[4];
	float width;

	corec.World_SampleGround(obj->pos[0], obj->pos[1], &pi);
	corec.Res_GetModelVisualBounds(obj->model, bmin, bmax);

	if (!le_shadowtype.integer)
		return;

	switch(le_shadowtype.integer)
	{
		case 2:
			corec.WR_GetSunVector(sv);
			sv[2]=0;
				
			LE_CalcCastedShadow(obj->pos, sv, pi.z, (bmax[0]-bmin[0]) * 1.8, M_GetVec3Length(sv) + (bmax[0]-bmin[0]), v);			
			corec.World_FitPolyToTerrain(v, 4, res.longshadowShader, 0, corec.Scene_AddPoly);
			M_MultVec3(sv, -1, sv);
			LE_CalcCastedShadow(obj->pos, sv, pi.z, (bmax[0]-bmin[0]) * 1.8, (bmax[0]-bmin[0]) * 0.5, v);
			corec.World_FitPolyToTerrain(v, 4, res.shadowShader, 0, corec.Scene_AddPoly);			
			return;
	}

	width = (bmax[0]-bmin[0]) / 2;
	
	SET_VEC3(v[0].vtx, bmin[0] + obj->pos[0]-width, bmin[1] + obj->pos[1]-width, pi.z);
	SET_VEC3(v[1].vtx, bmax[0] + obj->pos[0]+width, bmin[1] + obj->pos[1]-width, pi.z);
	SET_VEC3(v[2].vtx, bmax[0] + obj->pos[0]+width, bmax[1] + obj->pos[1]+width, pi.z);
	SET_VEC3(v[3].vtx, bmin[0] + obj->pos[0]-width, bmax[1] + obj->pos[1]+width, pi.z);
	SET_VEC4(v[0].col, le_shadowr.value * 255, le_shadowg.value * 255, le_shadowb.value * 255, le_shadowalpha.value);
	CopyBVec4(v[0].col, v[1].col);
	CopyBVec4(v[0].col, v[2].col);
	CopyBVec4(v[0].col, v[3].col);
	SET_VEC2(v[0].tex, 0, 0);
	SET_VEC2(v[1].tex, 1, 0);
	SET_VEC2(v[2].tex, 1, 1);
	SET_VEC2(v[3].tex, 0, 1);
					
	corec.World_FitPolyToTerrain(v, 4, res.spotshadowShader, 0, corec.Scene_AddPoly);
	
}

void LE_SetTOD()
{
	PERF_BEGIN;

	TL_SetTimeOfDay(le_tod.integer);

	PERF_END(PERF_TOD);
//	corec.SetTimeOfDay(le_tod.integer);
}

pointinfo_t pi; //hack: made this global until I have the mark fragment routine done

void LE_DrawTestModel()
{
	sceneobj_t sc;
	traceinfo_t trace;
	static skeleton_t skel = { 0 };

	if (le_testmodel.string[0] == 0)
		return;

	CLEAR_SCENEOBJ(sc);

	if (!le_testfixpos.integer)
	{
		LE_CursorTrace(&trace, 0);
		M_CopyVec3(trace.endpos, sc.pos);
	}
	
	sc.scale = le_testscale.value;
	sc.angle[2] = le_testangle.value;
	sc.model = corec.Res_LoadModel(le_testmodel.string);
	sc.skin = corec.Res_LoadSkin(sc.model, le_testskin.string);
	sc.objtype = OBJTYPE_MODEL;	
	
	corec.Geom_BeginPose(&skel, sc.model);
	corec.Geom_SetBoneAnim("", le_testanim.string, corec.Milliseconds(), corec.Milliseconds(), 0, 0);
	corec.Geom_EndPose();
	sc.skeleton = &skel;

	corec.Scene_AddObject(&sc);	
}


void	LE_NormalizeCvars(cvar_t *x, cvar_t *y, cvar_t *z)
{
	vec3_t xyz = { x->value, y->value, z->value };

	M_Normalize(xyz);

	corec.Cvar_SetValue(x->name, xyz[0]);
	corec.Cvar_SetValue(y->name, xyz[1]);
	corec.Cvar_SetValue(z->name, xyz[2]);
}
/*
void   LE_DrawCheckParticles()
{
	sceneobj_t test_obj2;
	int index;

	memset(&test_obj2, 0, sizeof(sceneobj_t));

	if ( corec.Particles_GetSystemCount() <= 0 )
		return;

	LE_NormalizeCvars(&le_particle_normal_x, &le_particle_normal_y, &le_particle_normal_z);

	// update particle config
	index = corec.Particles_MakeConfig("testconfig");
	corec.Particles_UpdateConfig(corec.Particles_GetSystemCount()-1, index);

	if ( le_testparticles.integer )
	{
		// set up test_obj
		test_obj2.particle_id = corec.Particles_GetSystemCount()-1;  //no longer -1, now we just pass in the system we allocated during init
//		test_obj2.shader = corec.Res_LoadShader("../../source_models/effects/xplode/awesome_sun.png");
		test_obj2.shader = corec.Res_LoadShader(le_part_shader.string);
		test_obj2.objtype = OBJTYPE_PARTICLES;

		test_obj2.pos[0] = le_particle_pos_x.value;
		test_obj2.pos[1] = le_particle_pos_y.value;
		test_obj2.pos[2] = le_particle_pos_z.value;

		test_obj2.angle[0] = le_particle_normal_x.value;
		test_obj2.angle[1] = le_particle_normal_y.value;
		test_obj2.angle[2] = le_particle_normal_z.value;

		test_obj2.model = LE_GetActiveModel();

		test_obj2.flags = SCENEOBJ_NEVER_CULL;

		test_obj2.scale = le_particle_modelscale.value;

		test_obj2.loframe = le.frame / (30.0 / corec.Res_GetModelKPS(test_obj2.model));

		// add test_obj to scene object list
		corec.Scene_AddObject(&test_obj2);
	}
}
*/


void	LE_DrawSky ()
{
	TL_DrawSky(&le.camera);
}

void	LE_DrawBrushInfluence()
{
	int x,y;
	traceinfo_t trace;

	return;
	
	if (!le_usertarget.integer)
	{
		LE_CursorTrace(&trace, 0);
	}
	else
		return;

	if (strcmp(le_mode.string, "object")!=0)
	{
		int brush = LE_CurrentBrush();
		for (y=-16; y<16; y++)
			for (x=-16; x<16; x++)
			{
				
				//brushcolor[0] = le_brushr.value * le_brushes[brush][y+16][x+16];
				//brushcolor[1] = le_brushg.value * le_brushes[brush][y+16][x+16];
				//brushcolor[2] = le_brushb.value * le_brushes[brush][y+16][x+16];
				//brushcolor[3] = 0xff;
				//corec.WR_SetDynamap(trace.gridx + x, trace.gridy + y, white);
				if (le_brushes[brush][y+16][x+16])
				{
					pointinfo_t pi;

					sceneobj_t sc;

					CLEAR_SCENEOBJ(sc);

					sc.pos[0] = corec.GridToWorld(trace.gridx + x);
					sc.pos[1] = corec.GridToWorld(trace.gridy + y);
					corec.World_SampleGround(sc.pos[0], sc.pos[1], &pi);
					sc.pos[2] = pi.z;
					sc.model = 0;
					sc.scale = 10;
					
					corec.Scene_AddObject(&sc);
				}
			}
	}
}

void	LE_DrawFrame ()
{	
	traceinfo_t trace;

	PERF_BEGIN;

	{
		PERF_BEGIN;
	
		TL_ClearBackground();
		corec.Scene_Clear();
		corec.Sound_ClearLoopingSounds();

		PERF_END(PERF_CLEARFRAME);
	}

	{
		PERF_BEGIN;

		LE_SetupCamera();
		LE_SetTOD();

		PERF_END(PERF_SETUP);
	}

	{
		PERF_BEGIN;

		LE_DrawSky();

		PERF_END(PERF_DRAWSKY);
	}

	LE_DrawObjects();
	//corec.WO_RenderObjects();

	{
		PERF_BEGIN;

		LE_OccludersFrame();

		PERF_END(PERF_OCCLUDERS);
	}

	LE_DrawTestModel();

//	LE_DrawBrushInfluence();

	{
		PERF_BEGIN;

		le.camera.flags |= CAM_NO_WORLDOBJECTS;		//editor draws the world objects in a custom function
		le.camera.flags &= ~CAM_NO_WORLD;
		corec.Scene_Render(&le.camera, NULL);

		PERF_END(PERF_RENDERSCENE);
	}

	TL_DrawSunRays(&le.camera);

	/*sprintf(s, "%f %f %f\n", dir[0], dir[1], dir[2]);
	DU_DrawString(0, 50, s, 12, 12, strlen(s), Host_GetNicefontShader());*/

	corec.Draw_SetColor(white);	

	modelframe += le_modelfps.value / (1.0 / (corec.FrameSeconds()));

	if (le_drawElevation.integer)
	{
		LE_CursorTrace(&trace, 0);
		TL_DrawString(le.mousepos.x, le.mousepos.y, fmt("Elevation: %f", trace.endpos[2]), 16, 1, 255, corec.GetNicefontShader());
	}

	if (le.showAxes)
	{
		corec.Scene_Clear();
		corec.Scene_AddObject(&le.axesObj);
		le.camera.flags |= CAM_NO_WORLD;
		corec.Scene_Render(&le.camera, NULL);
	}

	PERF_END(PERF_DRAWFRAME);
}

void	LE_Draw_FPS ()
{	
	static float lasttime = 0;
	static float time = 0;		
	char s[256];
	static int fps = 0;	
	static float count = 0;

	time = corec.FrameSeconds();	
		
	count += time;
	if (count>0.5)
	{
		fps = (int)(1.0 / (time));
		count = 0;
	}	

	corec.Draw_SetColor(white);

	BPrintf(s, 255, "FPS: %i", fps);
	s[255] = 0;

	TL_DrawString(le.screenw - (strlen(s)*16+4), le.screenh - 20, s, 16, 1, 255, corec.GetNicefontShader());
	
	lasttime = time;
}

void	CL_DrawForeground()
{
	if (le.showmouse)
		LE_Draw_Cursor();
	if (le_showfps.value)
	{
		LE_Draw_FPS();
	}
}
