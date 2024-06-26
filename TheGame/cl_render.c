// cl_render.c

#include "client_game.h"
#include <float.h>

#define WEAPON_SOUND_SOURCE_OFFSET 2048

cvar_t	cl_drawPlayerName	= { "cl_drawPlayerName",	"1", CVAR_SAVECONFIG };
cvar_t	cl_showWeapon		= { "cl_showWeapon",		"1", CVAR_SAVECONFIG };
cvar_t	cl_handoffset_x		= { "cl_handoffset_x",		"0" };
cvar_t	cl_handoffset_y		= { "cl_handoffset_y",		"0" };
cvar_t	cl_handoffset_z		= { "cl_handoffset_z",		"0" };
cvar_t	cl_handoffset_fov	= { "cl_handoffset_fov",	"45" };
cvar_t	cl_handJitterScale	= { "cl_handJitterScale",	"0.25" };
cvar_t	cl_waypoint_width	= { "cl_waypoint_width",	"10"};
cvar_t	cl_waypoint_height	= { "cl_waypoint_height",	"10000"};

cvar_t	cl_showfps		= { "cl_showfps",		"0", CVAR_SAVECONFIG };
cvar_t	cl_drawmockup	= { "cl_drawmockup",	"0" };

cvar_t	cl_interfaceBrightness = { "cl_interfaceBrightness", "0.7" };

cvar_t	cl_drawEnemyHealth =	{ "cl_drawEnemyHealth", "0", CVAR_CHEAT };

cvar_t	cl_shadowType	= { "cl_shadowType",	"1", CVAR_SAVECONFIG };
cvar_t	cl_shadowr		= { "cl_shadowr",		"0" };
cvar_t	cl_shadowg		= { "cl_shadowg",		"0" };
cvar_t	cl_shadowb		= { "cl_shadowb",		"0" };
cvar_t	cl_shadowalpha	= { "cl_shadowalpha",	"0.7" };
cvar_t	cl_shadowscalex	= { "cl_shadowscalex",	"3" };
cvar_t	cl_shadowscaley	= { "cl_shadowscaley",	"3" };
cvar_t	cl_shadowrangelo = { "cl_shadowrangelo", "400" };
cvar_t	cl_shadowrangehi = { "cl_shadowrangehi", "500" };

cvar_t	cl_charBrightnessFactor = { "cl_charBrightnessFactor", "2" };

cvar_t	cl_firstPersonIndicatorScale = { "cl_firstPersonIndicatorScale", "2" };

//throb rate for structure effects
cvar_t		cl_throbRate = { "cl_throbRate", "4" };

cvar_t	cl_seigeAlertDistance = { "cl_seigeAlertDistance", "400", CVAR_CHEAT };

//health meter gui tweaks
cvar_t	cl_healthMeterWidthScale	= { "cl_healthMeterWidthScale",		"1", CVAR_CHEAT };
cvar_t	cl_healthMeterHeight		= { "cl_healthMeterHeightScale",	"5", CVAR_CHEAT };
cvar_t	cl_healthMeterOffset		= { "cl_healthMeterOffset",			"0", CVAR_CHEAT };

cvar_t	cl_friendlyIconWidth	= { "cl_friendlyIconWidth",		"12", CVAR_CHEAT };
cvar_t	cl_friendlyIconHeight	= { "cl_friendlyIconHeight",	"12", CVAR_CHEAT };
cvar_t	cl_friendlyIconOffset	= { "cl_friendlyIconOffset",	"4", CVAR_CHEAT };

cvar_t	cl_voiceIconOffset =	{ "cl_voiceIconOffset", "14", CVAR_CHEAT };

cvar_t	cl_moneyXPos =	{ "cl_moneyXPos",	"900" };
cvar_t	cl_moneyYPos =	{ "cl_moneyYPos",	"10" };
cvar_t	cl_healthXPos =	{ "cl_healthXPos",	"10" };
cvar_t	cl_healthYPos =	{ "cl_healthYPos",	"700" };
cvar_t	cl_sprintXPos =	{ "cl_sprintXPos",	"900" };
cvar_t	cl_sprintYPos =	{ "cl_sprintYPos",	"700" };

cvar_t	cl_waypointFadeDist = { "cl_waypointFadeDist", "40000" };
cvar_t	cl_waypointMinFade = { "cl_waypointMinFade", "0.1" };
cvar_t	cl_electricEyeBeamFade = { "cl_electricEyeBeamFade", "0.1" };

cvar_t	cl_fov = { "cl_fov", "90" };

cvar_t	cl_weapbobamount =	{ "cl_weapbobamount",	"0.2" };	//scales the walk bob effect
cvar_t	cl_weapbobspeed =	{ "cl_weapbobspeed",	"15" };

//cvar_t	cl_fogOfWarDistance =	{ "cl_fogOfWarDistance",	"900" };
cvar_t	cl_fowFalloff =			{ "cl_fowFalloff",		".9" };
cvar_t	cl_fowUnexplored_r =	{ "cl_fowUnexplored_r",		"0" };
cvar_t	cl_fowUnexplored_g =	{ "cl_fowUnexplored_g",		"0" };
cvar_t	cl_fowUnexplored_b =	{ "cl_fowUnexplored_b",		"0" };
cvar_t	cl_fowUnexplored_a =	{ "cl_fowUnexplored_a",		"255" };
cvar_t	cl_fowExplored_r =		{ "cl_fowExplored_r",			"70" };
cvar_t	cl_fowExplored_g =		{ "cl_fowExplored_g",			"70" };
cvar_t	cl_fowExplored_b =		{ "cl_fowExplored_b",			"70" };
cvar_t	cl_fowExplored_a =		{ "cl_fowExplored_a",			"255" };
cvar_t	cl_notvis_alpha =		{ "cl_notvis_alpha",		"0.6" };

cvar_t	cl_namefontsize =		{ "cl_namefontsize",		"18" };
cvar_t	cl_nameiconsize =		{ "cl_nameiconsize",		"18" };
cvar_t	cl_namefontadjust =		{ "cl_namefontadjust",		"-40" };
cvar_t	cl_nameFadeRate =		{ "cl_nameFadeRate",		".1" };
cvar_t	cl_nameColors =			{ "cl_nameColors",			"1" };
cvar_t	cl_nameHealthHeight =	{ "cl_nameHealthHeight",	"8" };
cvar_t	cl_nameHealthWidth =	{ "cl_nameHealthWidth",		"60" };
float	namefont_strength = 0;

cvar_t	cl_showObjectInfo =		{ "cl_showObjectInfo",	"0", CVAR_CHEAT };

cvar_t	cl_cmdr_resourcelistx =			{ "cl_cmdr_resourcelistx",		"0" };
cvar_t	cl_cmdr_resourcelisty =			{ "cl_cmdr_resourcelisty",		"0" };
cvar_t	cl_cmdr_resourceNameHeight =	{ "cl_cmdr_resourceNameHeight",	"10" };
cvar_t	cl_cmdr_fog_near =				{ "cl_cmdr_fog_near", "1500" };
cvar_t	cl_cmdr_fog_far =				{ "cl_cmdr_fog_far", "3000" };

cvar_t	cl_proxMessageTime =	{ "cl_proxMessageTime",	"5000" };

cvar_t	cl_showLevelArmor =	{ "cl_showLevelArmor",	"1", CVAR_SAVECONFIG };
cvar_t	cl_rendUseGameTime = { "cl_rendUseGameTime", "0" };

cvar_t	cl_targetedObject = { "cl_targetedObject", "-1" };
cvar_t	cl_targetedTerrainX = { "cl_targetedTerrainX", "0" };
cvar_t	cl_targetedTerrainY = { "cl_targetedTerrainY", "0" };
cvar_t	cl_targetedTerrainZ = { "cl_targetedTerrainZ", "0" };

cvar_t	cl_showCommanderSelection = { "cl_showCommanderSelection", "1" };

cvar_t	cl_cloackAdjustRate = { "cl_cloakAdjustRate", ".7", CVAR_CHEAT };

#ifdef SAVAGE_DEMO
cvar_t	cl_constructionShader = { "cl_constructionShader", "/textures/effects/1_nl_construction.tga", CVAR_CHEAT };
#else	//SAVAGE_DEMO
cvar_t	cl_constructionShader = { "cl_constructionShader", "beast/weapons/ranged/1_nl_stratamagic.tga", CVAR_CHEAT };
#endif	//SAVAGE_DEMO

cvar_t	cl_drawTrailSegmentOnDeath =	{ "cl_drawTrailSegmentOnDeath", "1" };
cvar_t	cl_debugFlyby = { "cl_debugFlyby", "0", CVAR_CHEAT };

cvar_t	cl_debugMuzzle = { "cl_debugMuzzle", "0", CVAR_CHEAT };
cvar_t	cl_muzzleSnap = { "cl_muzzleSnap", "1", CVAR_SAVECONFIG };

cvar_t	cl_healthStageMultiplier = { "cl_healthStageMultiplier", "1", CVAR_SAVECONFIG };

cvar_t	_proximityMessage = { "_proximityMessage", "" };

cvar_t	cl_glowFilter = { "cl_glowFilter", "0", CVAR_SAVECONFIG };
cvar_t	cl_commanderGlowShader = { "cl_commanderGlowShader", "gui/standard/1_nl_nfm_screenglow_com.tga" };
cvar_t	cl_playerGlowShader = { "cl_playerGlowShader", "gui/standard/1_nl_nfm_screenglow.tga" };

camera_t interfaceCam;

extern int numTeamUnits;
extern int allTeamUnits[];

extern cvar_t cl_mousepos_x;
extern cvar_t cl_mousepos_y;

extern cvar_t cl_cmdr_fogDistance;
extern cvar_t cl_oldCommanderLook;

extern cvar_t cl_walkbobspeed;
extern cvar_t cl_walkbobamount;

extern cvar_t player_task;

void	CopyBVec4(bvec4_t in, bvec4_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}


/*==========================

  Rend_AddFogOfWar

  Visually mark where objects should be visible to us on the terrain

  Actual object visibility for the commander is determined by the server

 ==========================*/

void	Rend_AddFogOfWar()
{
	int i, team, grid_x, max_grid_x, min_grid_x, min_grid_y, grid_y, max_grid_y;
	float dist, intensity, fogsq, radius;
	vec3_t tmppos;
	bvec4_t color, oldColor;

	if (cl_oldCommanderLook.integer)
		return;

	Rend_ResetFogOfWar();

	team = cl.info->team;

	//fogsq = cl_fogOfWarDistance.value * cl_fogOfWarDistance.value;

	//radius = cl_fogOfWarDistance.value;
	
	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (!cl.objects[i].visual.active
			|| team != cl.objects[i].visual.team
			|| cl.objects[i].visual.health <= 0
			|| cl.objects[i].visual.flags & BASEOBJ_UNDER_CONSTRUCTION
			|| CL_ObjectType(cl.objects[i].visual.type)->objclass == OBJCLASS_WEAPON)
			continue;

		radius = CL_ObjectType(cl.objects[i].visual.type)->viewDistance;
		if (radius < 1)
			radius = 1.0;
		fogsq = radius * radius;

		//if (cl_fogOfWarDistance.value < 9999)
		if (radius < 9999)
		{
			
			min_grid_x  = floor(corec.WorldToGrid(cl.objects[i].visual.pos[X] - radius));
			max_grid_x  = ceil(corec.WorldToGrid(cl.objects[i].visual.pos[X] + radius));
			min_grid_y  = floor(corec.WorldToGrid(cl.objects[i].visual.pos[Y] - radius));
			max_grid_y  = ceil(corec.WorldToGrid(cl.objects[i].visual.pos[Y] + radius));

			for (grid_x = min_grid_x; grid_x < max_grid_x; grid_x++)
			{
				for (grid_y = min_grid_y; grid_y < max_grid_y; grid_y++)
				{
					int n;
					tmppos[X] = corec.GridToWorld(grid_x);
					tmppos[Y] = corec.GridToWorld(grid_y);
					tmppos[Z] = cl.objects[i].visual.pos[2];
					dist = sqrt(M_GetDistanceSq(tmppos, cl.objects[i].visual.pos));
					//if (dist > cl_fogOfWarDistance.value)
					if (dist > radius)
						continue;
					//if (dist < cl_fogOfWarFalloff.value)
					if ((dist/radius) < cl_fowFalloff.value)
						intensity = 1;
					else			
					{
						//float factor = cl_fogOfWarDistance.value - cl_fogOfWarFalloff.value;
						//if (!factor)
						//	factor = 1;
						//if (factor)
						//	
						float factor = (dist/radius);
						intensity = 1 - (factor - cl_fowFalloff.value) * (1 / (1 - cl_fowFalloff.value));												
					}
					
					//}
					/*else
					{
						intensity = MIN(1, sqrt(dist - fogsq) / cl_fogOfWarFalloff.value);
					}*/
					corec.WR_GetDynamap(grid_x, grid_y, oldColor);
					color[0] = LERP(intensity, cl_fowUnexplored_r.value, 255.0);
					color[1] = LERP(intensity, cl_fowUnexplored_g.value, 255.0);
					color[2] = LERP(intensity, cl_fowUnexplored_b.value, 255.0);
					color[3] = 255;
					//if (color[0] < oldColor[0])
					for (n=0; n<3; n++)
					{
						//color[n] += oldColor[n];
						if (color[n] < oldColor[n])
							color[n] = oldColor[n];
					}
						
					corec.WR_SetDynamap(grid_x, grid_y, color);				
				}
			}
		}
	}
}

void	Rend_DrawGlowFilter()
{
	if (!cl_glowFilter.integer)
		return;

	corec.Draw_SetColor(white);

	if (!cl.isCommander)
	{
		corec.Draw_Quad2d(cl.camera.x, cl.camera.y, cl.camera.width, cl.camera.height,
			0,0,1,1,
			corec.Res_LoadShader(cl_playerGlowShader.string));
	}
	else
	{
		corec.Draw_Quad2d(cl.camera.x, cl.camera.y, cl.camera.width, cl.camera.height,
			0,0,1,1,
			corec.Res_LoadShader(cl_commanderGlowShader.string));
	}
}

/*
void	CL_CalcCastedShadow(const vec3_t origin, const vec3_t dir, float zvalue, float width, float length, scenefacevert_t v[4])
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

	M_AddVec3(o, perp, v[0].vtx);
	SET_VEC2(v[0].tex, 1, 1);
	M_SubVec3(o, perp, v[1].vtx);
	SET_VEC2(v[1].tex, 0, 1);
	M_AddVec3(v[1].vtx, mag, v[2].vtx);
	SET_VEC2(v[2].tex, 0, 0);
	M_AddVec3(v[0].vtx, mag, v[3].vtx);
	SET_VEC2(v[3].tex, 1, 0);

	v[0].vtx[2] = v[1].vtx[2] = v[2].vtx[2] = v[3].vtx[2] = zvalue;

	SET_VEC4(v[0].col, 0, 0, 0, 128);
	CopyBVec4(v[0].col, v[1].col);
	CopyBVec4(v[0].col, v[2].col);
	CopyBVec4(v[0].col, v[3].col);
}

void CL_AddShadow(sceneobj_t *obj)
{
	pointinfo_t pi;
	vec3_t sv;
	vec3_t bmin, bmax;
	scenefacevert_t v[4];
	float width;

	corec.World_SampleGround(obj->pos[0], obj->pos[1], &pi);
	corec.Res_GetModelVisualBounds(obj->model, bmin, bmax);

	corec.WR_GetSunVector(sv);
	sv[2]=0;
	
		width = (bmax[0]-bmin[0]) / 2;
	
	SET_VEC3(v[0].vtx, bmin[0] + obj->pos[0]-width, bmin[1] + obj->pos[1]-width, pi.z);
	SET_VEC3(v[1].vtx, bmax[0] + obj->pos[0]+width, bmin[1] + obj->pos[1]-width, pi.z);
	SET_VEC3(v[2].vtx, bmax[0] + obj->pos[0]+width, bmax[1] + obj->pos[1]+width, pi.z);
	SET_VEC3(v[3].vtx, bmin[0] + obj->pos[0]-width, bmax[1] + obj->pos[1]+width, pi.z);
	SET_VEC4(v[0].col, 0, 0, 0, 160);
	CopyBVec4(v[0].col, v[1].col);
	CopyBVec4(v[0].col, v[2].col);
	CopyBVec4(v[0].col, v[3].col);
	SET_VEC2(v[0].tex, 0, 0);
	SET_VEC2(v[1].tex, 1, 0);
	SET_VEC2(v[2].tex, 1, 1);
	SET_VEC2(v[3].tex, 0, 1);
	
	corec.World_FitPolyToTerrain(v, 4, res.spotshadowShader, 0, corec.Scene_AddPoly);

//	CL_CalcCastedShadow(obj->pos, sv, pi.z, (bmax[0]-bmin[0])*0.7, M_GetVec3Length(sv) + (bmax[0]-bmin[0]), v);			
//	corec.World_FitPolyToTerrain(v, 4, res.longshadowShader, 0, corec.Scene_AddPoly);
//	M_MultVec3(sv, -1, sv);
	CL_CalcCastedShadow(obj->pos, sv, pi.z, (bmax[0]-bmin[0])*1.8, M_GetVec3Length(sv)*(bmax[2]-bmin[2])*2, v);

 	corec.World_FitPolyToTerrain(v, 4, res.shadowShader, 0, corec.Scene_AddPoly);			
}
*/



/*==========================

  CL_AddLoopingSound

 ==========================*/

void	CL_AddLoopingSound(objectData_t *def, int animstate, int objidx, int priority)
{
	if (!def->loopingSounds[animstate])
		return;
	if (!def->loopingSounds[animstate][0])
		return;
	
	corec.Sound_AddLoopingSound(
		corec.Res_LoadSound(def->loopingSounds[animstate]),
		objidx,
		priority);
}

/*==========================

  Rend_DrawNameHealth

 ==========================*/

void	Rend_DrawNameHealth(clientObject_t *obj)
{
	float fullhealth;
	float objheight, barWidth;
	vec3_t bmin,bmax,pos;
	vec2_t screenPos;
	char *name;
	int stringWidth, width, height;

	if (obj->visual.health <= 0 /*|| obj->base.moveState == AS_DEAD*/)
		return;
	if (obj->base.index == cl.clientnum)
		return;
	if (obj->base.index < MAX_CLIENTS)
		name = cl.clients[obj->base.index].info.name;
	else if (obj->base.nameIdx)
    {
        //don't draw the worker names for the commander
        name = "";
		//name = fmt("Worker %s", peonNames[obj->base.nameIdx]);
    }
	else
	{
		name = CL_ObjectType(obj->base.type)->description;
	}

	corec.Res_GetModelVisualBounds(CL_Model(obj->visual.type), bmin, bmax);
	objheight = (bmax[2]);
	if (cl.isCommander && IsUnitType(obj->visual.type))
		objheight *= CL_ObjectType(obj->base.type)->cmdrScale;		//account for larger characters in commander view
	else if (cl.isCommander && IsBuildingType(obj->visual.type))
	{/*
		objheight -= 200;		//fudge
		if (objheight < 50)
			objheight = 50;*/
	}
	
	M_CopyVec3(obj->visual.pos, pos);
	height = cl_friendlyIconHeight.value;
	pos[Z] += cl_friendlyIconOffset.value + height + objheight;
	corec.Vid_ProjectVertex(&cl.camera, pos, screenPos);
	stringWidth = corec.GUI_StringWidth(name, height, height, 1, 1024, corec.GetNicefontShader());

    /*
     * draw their name over their heads
    */
    if (name[0])
    {
	    corec.GUI_SetRGBA(1, 1, 1, 1); 
	    corec.GUI_DrawString_S(screenPos[X] - stringWidth/2, screenPos[Y], name, height, height, 1, 255, corec.GetNicefontShader());
    }
    
	screenPos[Y] += height + cl_healthMeterOffset.value;

	width = stringWidth;
	corec.GUI_ScaleToScreen(&width, &height);  

	fullhealth = obj->visual.fullhealth;
	if (fullhealth)
		barWidth = (cl_healthMeterWidthScale.value * width * (float)obj->base.health / fullhealth);

	//draw the background red graphic
	width *= cl_healthMeterWidthScale.value;
	corec.GUI_SetRGBA(1.0, 0.0, 0.0, 1.0); 
	corec.Draw_Quad2d(screenPos[X] - stringWidth/2, screenPos[Y], width, cl_healthMeterHeight.value, 0, 0, 1, 1, corec.GetWhiteShader());

	//now draw the foreground blue graphic
	corec.GUI_SetRGBA(0.0, 0.0, 1.0, 1.0); 
	corec.Draw_Quad2d(screenPos[X] - stringWidth/2, screenPos[Y], barWidth, cl_healthMeterHeight.value, 0, 0, 1, 1, corec.GetWhiteShader());

	//if it's a building under construction, draw a construction meter
	if (obj->base.flags & BASEOBJ_UNDER_CONSTRUCTION)
	{
		barWidth = (cl_healthMeterWidthScale.value * width * ((100 - (float)obj->base.percentToComplete)) / 100);

		//draw black background
		corec.GUI_SetRGBA(0.0, 0.0, 0.0, 1.0);
		corec.Draw_Quad2d(screenPos[X] - stringWidth/2, screenPos[Y] + cl_healthMeterHeight.value + 2, width, cl_healthMeterHeight.value, 0, 0, 1, 1, corec.GetWhiteShader());

		//draw green foreground
		corec.GUI_SetRGBA(0.3, 1.0, 0.3, 1.0);
		corec.Draw_Quad2d(screenPos[X] - stringWidth/2, screenPos[Y] + cl_healthMeterHeight.value + 2, barWidth, cl_healthMeterHeight.value, 0, 0, 1, 1, corec.GetWhiteShader());
	}
}


/*==========================

  Rend_DrawFriendlyIcon

 ==========================*/

void	Rend_DrawFriendlyIcon(clientObject_t *obj, bool reviveme)
{
	float objheight;
	vec3_t bmin,bmax;
	sceneobj_t icon;
	float scale;

	if (cl.gametype == GAMETYPE_DEATHMATCH)  //nobody is friendly in deathmatch mode!
		return;

	if (cl.isCommander)
		return;

	if (CL_ObjectType(obj->visual.type)->objclass == OBJCLASS_WEAPON)
		return;
	if (obj->visual.health <= 0 && CL_ObjectType(obj->visual.type)->fullHealth > 0 && !reviveme)
		return;
	if (obj->base.index == cl.clientnum)
		return;

	CLEAR_SCENEOBJ(icon);

	icon.objtype = OBJTYPE_BILLBOARD;
	icon.flags = SCENEOBJ_BILLBOARD_ALL_AXES;
	icon.shader = (reviveme) ? res.reviveIcon : res.friendlyIcon;
	icon.alpha = 1;

	corec.Res_GetModelVisualBounds(CL_Model(obj->visual.type), bmin, bmax);
	objheight = (bmax[2]);
	scale = (cl.isCommander) ? CL_ObjectType(obj->visual.type)->cmdrScale : CL_ObjectType(obj->visual.type)->scale;
	objheight *= scale;
	
	M_CopyVec3(obj->visual.pos, icon.pos);
	icon.pos[Z] +=	cl_friendlyIconOffset.value + objheight;
	icon.width = cl_friendlyIconWidth.value;
	icon.height = cl_friendlyIconHeight.value;
	SET_VEC4(icon.color, 1, 1, 1, 1);
	corec.Scene_AddObject(&icon);
}


/*==========================

  Rend_DrawVoiceIcon

 ==========================*/

void	Rend_DrawVoiceIcon(clientObject_t *obj)
{
	float objheight;
	vec3_t bmin,bmax;
	sceneobj_t icon;
	float scale;

	CLEAR_SCENEOBJ(icon);

	icon.objtype = OBJTYPE_BILLBOARD;
	icon.flags = SCENEOBJ_BILLBOARD_ALL_AXES;
	icon.shader = res.voiceIcon;
	icon.alpha = obj->base.index == cl.clientnum ? 0.4 : 1;

	corec.Res_GetModelVisualBounds(CL_Model(obj->visual.type), bmin, bmax);
	objheight = (bmax[2]/*-bmin[2]*/);
	scale = CL_ObjectType(obj->visual.type)->cmdrScale;
	if (cl.isCommander && IsUnitType(obj->visual.type))
		objheight *= scale;		//account for larger characters in commander view
	
	M_CopyVec3(obj->visual.pos, icon.pos);
	icon.pos[Z] +=	cl_voiceIconOffset.value + objheight;
	icon.width = 10;
	icon.height = 10;
	if (cl.isCommander)
	{
		icon.width *= scale;
		icon.height *= scale;
	}

	if (obj->base.index < MAX_CLIENTS && obj->base.index >= 0)		//should always be the case
	{
		if (cl.clients[obj->base.index].info.isOfficer)
			SET_VEC4(icon.color, 0.4, 0.4, 1, icon.alpha);
		else
			SET_VEC4(icon.color, 1, 1, 1, icon.alpha);
	}
	else
		SET_VEC4(icon.color, 1, 1, 1, icon.alpha);

	corec.Scene_AddObject(&icon);
}


/*==========================

  Rend_SurfaceDecal

  put a decal on the TERRAIN (objects are not affected)

 ==========================*/

void	Rend_SurfaceDecal(const vec3_t pos, float width, float height, residx_t shader, vec4_t color)
{
	pointinfo_t pi;
	scenefacevert_t v[4];

	corec.World_SampleGround(pos[0], pos[1], &pi);

	SET_VEC3(v[0].vtx, pos[0]-width, pos[1]-height, pi.z);
	SET_VEC3(v[1].vtx, pos[0]+width, pos[1]-height, pi.z);
	SET_VEC3(v[2].vtx, pos[0]+width, pos[1]+height, pi.z);
	SET_VEC3(v[3].vtx, pos[0]-width, pos[1]+height, pi.z);
	SET_VEC4(v[0].col, color[0]*255, color[1]*255, color[2]*255, color[3]*255);
	CopyBVec4(v[0].col, v[1].col);
	CopyBVec4(v[0].col, v[2].col);
	CopyBVec4(v[0].col, v[3].col);
	SET_VEC2(v[0].tex, 0, 0);
	SET_VEC2(v[1].tex, 1, 0);
	SET_VEC2(v[2].tex, 1, 1);
	SET_VEC2(v[3].tex, 0, 1);
					
	corec.World_FitPolyToTerrain(v, 4, shader, 0, corec.Scene_AddPoly);
}


/*==========================

  Rend_DrawIndicator

  draw a selection indicator on a model

 ==========================*/

vec4_t	selectionTeamColor = { 0,1,0,1 };
vec4_t	selectionEnemyColor = { 1,0,0,1 };
vec4_t	selectionNPCColor = { 0,0,1,1 };


void	Rend_DrawIndicator(int objidx, float alpha)
{
	vec3_t bmin, bmax;
	float scale, scaleadjust;
	vec4_t color;
	baseObject_t *obj = &cl.objects[objidx].visual;
	residx_t shader;

	if (obj->health <= 0)
		return;

	corec.Res_GetModelVisualBounds(CL_Model(obj->type), bmin, bmax);

	//characters are scaled up in commander view
	if (IsUnitType(obj->type) && cl.isCommander)
		scaleadjust = CL_ObjectType(obj->type)->cmdrScale * 0.7;
	else
	{
		if (!cl.thirdperson && objidx == cl.clientnum)
			scaleadjust = cl_firstPersonIndicatorScale.value;
		else
			scaleadjust = 0.7;

		if (objidx == cl.clientnum)	//ignore the alpha param
			alpha = (sin(cl.systime/250.0)+1)/2;
	}

	scale = MAX(bmax[0] - bmin[0], bmax[1] - bmin[1]) * scaleadjust;

	if (IsBuildingType(obj->type))
		scale *= STRUCTURE_SCALE;

	
	if (obj->team == cl.info->team)
	{
		M_CopyVec3(selectionTeamColor, color);			
	}
	else if (obj->team == 0)
	{
		M_CopyVec3(selectionNPCColor, color);			
	}
	else
	{
		M_CopyVec3(selectionEnemyColor, color);			
	}

	color[3] = alpha;

	if (scale > 50)
		shader = res.selectionIndicatorLargeShader;
	else
		shader = res.selectionIndicatorSmallShader;
	
	Rend_SurfaceDecal(obj->pos, scale, scale, shader, color);
}

void	Rend_DrawSelectionIndicators()
{
	int i = 0;
	int j;
	//if we're still selecting units, show indicators half transparent
	bool alreadySelected;

	if (!cl.isCommander)
		return;

	while (i < cl.selection.numSelected)
	{		
		Rend_DrawIndicator(cl.selection.array[i], 1);		
		i++;
	}

	i = 0;
	while (i < cl.potentialSelection.numSelected)
	{
		j = 0;
		alreadySelected = false;
		while (j < cl.selection.numSelected)
		{
			if (cl.selection.array[j] == cl.potentialSelection.array[i])
			{
				alreadySelected = true;
				break;
			}
			j++;
		}
		if (!alreadySelected)
		{
			Rend_DrawIndicator(cl.potentialSelection.array[i], 0.3);
		}

		i++;
	}
}

void	CL_CalcCastedShadow(const vec3_t origin, const vec3_t dir, float zvalue, float width, float length, float alpha, scenefacevert_t v[4])
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
	SET_VEC2(v[0].tex, 1, 1);
	M_SubVec3(o, perp, v[1].vtx);
	SET_VEC2(v[1].tex, 0, 1);
	M_AddVec3(v[1].vtx, mag, v[2].vtx);
	SET_VEC2(v[2].tex, 0, 0);
	M_AddVec3(v[0].vtx, mag, v[3].vtx);
	SET_VEC2(v[3].tex, 1, 0);

	v[0].vtx[2] = v[1].vtx[2] = v[2].vtx[2] = v[3].vtx[2] = zvalue;

	SET_VEC4(v[0].col, cl_shadowr.value * 255, cl_shadowg.value * 255, cl_shadowb.value * 255, cl_shadowalpha.value * alpha * 255);
	CopyBVec4(v[0].col, v[1].col);
	CopyBVec4(v[0].col, v[2].col);
	CopyBVec4(v[0].col, v[3].col);
}

void CL_AddShadow(sceneobj_t *obj, int objindex)
{
	pointinfo_t pi;
	vec3_t sv;
	vec3_t bmin, bmax;
	scenefacevert_t v[4];
	float dist, height;
	float alpha = 1.0;
	vec3_t diff;
	vec3_t	p, vec;
	float length, width;

	if (!cl_shadowType.integer)
		return;

	M_SubVec3(obj->pos, cl.camera.origin, diff);
	dist = M_GetVec3Length(diff);

	if (!cl.isCommander)		//display all shadows in commander view for now
	{
		if (dist > cl_shadowrangehi.value)
			return;
		else if (dist > cl_shadowrangelo.value)	
		{
			float factor = cl_shadowrangehi.value - cl_shadowrangelo.value;
			if (!factor)
				factor = 1;
			alpha = 1 - (dist - cl_shadowrangelo.value) / factor;		
			if (alpha <= 0)
				return;
		}
	}

	corec.World_SampleGround(obj->pos[0], obj->pos[1], &pi);
	corec.Res_GetModelVisualBounds(obj->model, bmin, bmax);

	height = obj->pos[Z] - pi.z;

	corec.WR_GetSunVector(sv);
	sv[2]=0;
	
	length = (bmax[2] - bmin[2]) * M_GetVec3Length(sv) * cl_shadowscaley.value;
	width = ((bmax[0] - bmin[0]) / 2) * cl_shadowscalex.value;
	if (length < width)
		length = width;

	if (objindex == -1)
	{
		M_CopyVec3(obj->pos, p);
	}
	else
	{
		float maxdist = 30.0;
		float alphamult;

		if (objindex == 0)
			objindex = 0;
		M_MultVec3(sv, height, vec);
		M_AddVec3(obj->pos, vec, p);
		alphamult = (1.0 - (MIN(maxdist, height) / maxdist));
		alpha *= alphamult;
	}

	CL_CalcCastedShadow(p, sv, pi.z, width, length, alpha, v);
	corec.World_FitPolyToTerrain(v, 4, res.longshadowShader, 0, corec.Scene_AddPoly);
	M_MultVec3(sv, -1, sv);
	CL_CalcCastedShadow(p, sv, pi.z, width, width, alpha, v);
	corec.World_FitPolyToTerrain(v, 4, res.shadowShader, 0, corec.Scene_AddPoly);
}


void	Rend_SetMouseCursor(cursor_t *cursor)
{
	INT_SetCursorShader(cursor->shader, cursor->hotspotx, cursor->hotspoty);	
}



void	Rend_AttachToSkeleton(sceneobj_t *sc, char *boneName, residx_t model, bool inheritScale, sceneobj_t *out)
{	
	if (sc->axis[0][0] == 0.0 && sc->axis[0][1] == 0.0 && sc->axis[0][2] == 0.0)
	{
		//invalid axis.  fill it in
		M_GetAxis(sc->angle[PITCH], sc->angle[ROLL], sc->angle[YAW], sc->axis);
		sc->flags |= SCENEOBJ_USE_AXIS;
	}	

	CL_GetBoneWorldTransform(boneName, sc->pos, sc->axis, sc->scale, sc->skeleton, out->pos, out->axis);

	out->model = model;
	out->flags |= SCENEOBJ_USE_AXIS;
	if (inheritScale)
		out->scale = sc->scale;
	//out->flags |= SCENEOBJ_SKELETON_ATTACHMENT;
}


/*==========================

  Rend_AddPlayerAttachments

  this shouldn't get called if obj->visual.index >= MAX_CLIENTS

 ==========================*/

void	Rend_AddPlayerAttachments(clientObject_t *obj, sceneobj_t *sc, int cloakshader)
{
	residx_t	wpmodel;
	sceneobj_t	attachment;

	obj->hasMuzzle = false;

	if (obj->visual.index >= MAX_CLIENTS)
		return;

	if (!IsCharacterType(obj->visual.type))
		return;

	CLEAR_SCENEOBJ(attachment);

	if (cloakshader != -1)
	{
		attachment.shader = cloakshader;
		attachment.flags |= SCENEOBJ_SINGLE_SHADER;
	}

	attachment.color[0] = obj->brightness * cl_charBrightnessFactor.value;
	attachment.color[1] = obj->brightness * cl_charBrightnessFactor.value;
	attachment.color[2] = obj->brightness * cl_charBrightnessFactor.value;
	attachment.color[3] = 1;
	attachment.flags |= SCENEOBJ_SOLID_COLOR;

	//ranged (this animates)
	if (IsWeaponType(obj->visual.weapon) || IsItemType(obj->visual.weapon))
	{
		objectData_t *wp = CL_ObjectType(obj->visual.weapon);
		wpmodel = corec.Res_LoadModel(wp->model);

		if (wpmodel)
		{
			int wpanim;

			attachment.skeleton = &cl.clients[obj->visual.index].weaponSkel;

			//work out a wpState animation based on the player's animState2

			switch(obj->visual.animState2)
			{							
				case AS_WEAPON_CHARGE_1:
				case AS_WEAPON_CHARGE_2:
				case AS_WEAPON_CHARGE_3:
				case AS_WEAPON_CHARGE_4:					
					wpanim = AS_WPSTATE_CHARGE;
					break;
				case AS_WEAPON_FIRE_1:
				case AS_WEAPON_FIRE_2:
				case AS_WEAPON_FIRE_3:
				case AS_WEAPON_FIRE_4:				
					wpanim = AS_WPSTATE_FIRE;
					break;
				default:
					wpanim = AS_IDLE;
					break;
			}

			//add looping sound
			CL_AddLoopingSound(wp, wpanim, obj->visual.index + WEAPON_SOUND_SOURCE_OFFSET, obj->visual.index == cl.clientnum ? PRIORITY_HIGH : PRIORITY_LOW);			

			corec.Geom_BeginPose(attachment.skeleton, wpmodel);			
			corec.Geom_SetBoneAnim("", GetAnimName(wpanim), obj->animState2Time, cl.gametime, 100, 0);
			corec.Geom_EndPose();			

			//fire off any anim events for the 3rd person weapon
			if (attachment.skeleton->gotEvent)
				CL_AnimEvent(obj, attachment.skeleton->eventString);

			Rend_AttachToSkeleton(sc, CL_ObjectType(obj->visual.weapon)->attachBone, wpmodel, true, &attachment);

			corec.Scene_AddObject(&attachment);

			if (corec.Geom_GetBoneIndex(attachment.skeleton, "_boneMuzzle") > 0)
			{
				CL_GetBoneWorldTransform("_boneMuzzle", attachment.pos, attachment.axis, attachment.scale, attachment.skeleton, obj->muzzlePos, obj->muzzleAxis);
				obj->hasMuzzle = true;
			}
			else
			{
				obj->hasMuzzle = false;
				M_SetVec3(obj->muzzlePos,
						  obj->visual.pos[0],
						  obj->visual.pos[1],
						  obj->visual.pos[2] + CL_ObjectType(obj->visual.type)->viewheight);
				M_SetVec3(obj->muzzleAxis[0], 1, 0, 0);
				M_SetVec3(obj->muzzleAxis[1], 0, 1, 0);
				M_SetVec3(obj->muzzleAxis[2], 0, 0, 1);
			}

			if(CL_ObjectType(obj->visual.weapon)->effect)
				CL_DoEffect(obj, NULL, NULL, CL_ObjectType(obj->visual.weapon)->effect, 0, 0);
		}
	}
	//melee (don't animate it)
	else if (IsMeleeType(obj->visual.weapon))
	{
		int				index;
		objectData_t	*weapon, *unit;

		weapon = CL_ObjectType(obj->visual.weapon);
		unit = CL_ObjectType(obj->visual.type);

		////
		//left hand
		////

		//get the base model
		if (weapon->useDefaultMeleeModel)
			wpmodel = corec.Res_LoadModel(unit->leftMeleeModel);
		else
			wpmodel = corec.Res_LoadModel(weapon->leftMeleeModel);
		
		//check for replacements based on level
		if (obj->visual.index < MAX_CLIENTS && weapon->useDefaultMeleeModel)
		{
			int newmodel;
			int race = cl.teams[cl.clients[obj->visual.index].info.team].race;

			for (index = 1; index <= cl.clients[obj->visual.index].score.level; index++)
			{
				if (strlen(experienceTable[race][index].lmeleemodel) > 0)
				{
					newmodel = corec.Res_LoadModel(fmt("%s_%s.model", experienceTable[race][index].lmeleemodel, unit->name));
					if (newmodel)
						wpmodel = newmodel;
				}
			}
		}
		
		if (wpmodel)
		{
			Rend_AttachToSkeleton(sc, "_boneMeleeL", wpmodel, true, &attachment);
			corec.Scene_AddObject(&attachment);
		}

		////
		//right hand
		////

		//base model
		if (weapon->useDefaultMeleeModel)
			wpmodel = corec.Res_LoadModel(unit->rightMeleeModel);
		else
			wpmodel = corec.Res_LoadModel(weapon->rightMeleeModel);

		//check for replacements based on level
		if (obj->visual.index < MAX_CLIENTS)
		{
			int newmodel;
			int race = cl.teams[cl.clients[obj->visual.index].info.team].race;

			for (index = 1; index <= cl.clients[obj->visual.index].score.level; index++)
			{
				if (strlen(experienceTable[race][index].rmeleemodel) > 0)
				{
					newmodel = corec.Res_LoadModel(fmt("%s_%s.model", experienceTable[race][index].rmeleemodel, unit->name));
					if (newmodel)
						wpmodel = newmodel;
				}
			}
		}
		
		if (wpmodel)
		{
			Rend_AttachToSkeleton(sc, "_boneMeleeR", wpmodel, true, &attachment);
			corec.Scene_AddObject(&attachment);
		}
	}	
}





/*==========================

  Rend_ClientObjectToSceneObject

  convert a clientObject_t to a sceneobj_t for rendering

 ==========================*/

void	Rend_ClientObjectToSceneObject(clientObject_t *obj, sceneobj_t *sc, int flags)
{
	CLEAR_SCENEOBJ((*sc));

 	sc->model = CL_Model(obj->visual.type);

	sc->skin = CL_Skin(obj->visual.type, obj->visual.team);

	M_CopyVec3(obj->visual.pos, sc->pos);

	M_CopyVec3(obj->visual.angle, sc->angle);
	if (IsCharacterType(obj->visual.type))
		sc->angle[0] = 0;

	M_GetAxis(sc->angle[0], sc->angle[1], sc->angle[2], sc->axis);
	sc->flags |= SCENEOBJ_USE_AXIS;

	sc->skeleton = &obj->skeleton;

	if (obj->base.index == cl.clientnum)
		sc->flags |= SCENEOBJ_NEVER_CULL;
}

/*
void    Rend_DrawCursor ()
{
	//corec.Console_DPrintf("mouse is at (%i, %i)\n", mousepos.x, mousepos.y);
	if (corec.GUI_CheckMouseAgainstUI(cl_mousepos_x.value, cl_mousepos_y.value))
	{
		corec.Draw_Quad2d(cl_mousepos_x.value, cl_mousepos_y.value, cl_cursorsize.integer, cl_cursorsize.integer, 0, 0, 1, 1, res.mainCursor);
	}
	else
	{
		corec.Draw_Quad2d(cl_mousepos_x.value, cl_mousepos_y.value, cl_cursorsize.integer, cl_cursorsize.integer, 0, 0, 1, 1, cursorShader);
	}
}
*/

void	Rend_DrawObjectPreview(residx_t model, residx_t skin, vec3_t pos, vec3_t angle, 
							   float alpha, float scale, int flags, float frame, bool shadow)
{
	sceneobj_t obj;
	CLEAR_SCENEOBJ(obj);

	obj.model = model;
	obj.skin = skin;
	obj.loframe = 0;
	obj.hiframe = 0;
	obj.lerp_amt = 0;
	obj.objtype = OBJTYPE_MODEL;
	obj.flags = flags;
	obj.scale = scale;
	if (alpha < 1)
	{
		obj.alpha = alpha;
		obj.flags |= SCENEOBJ_USE_ALPHA;
	}

	M_CopyVec3(angle, obj.angle);
	M_CopyVec3(pos, obj.pos);

	corec.Scene_AddObject(&obj);

	if (shadow)
		CL_AddShadow(&obj, -1);
}



#define BLINK_SPEED	250
#define BLINK_DURATION 1500


void	Rend_DrawEffects(clientObject_t *obj)
{
	if (obj->blink)
	{
		if (cl.gametime - obj->blinkStartTime > BLINK_DURATION)
		{
			obj->blink = false;
		}
		else if ((cl.gametime - obj->blinkStartTime) % (BLINK_SPEED*2) < (BLINK_SPEED))
			Rend_DrawIndicator(obj->base.index, 1);
	}
}


/*==========================

  Rend_DoSpecialStructureRendering

  renders construction and damage effects for structures

 ==========================*/

void	Rend_DoSpecialStructureRendering(clientObject_t *obj, sceneobj_t *sc)
{
	int	healthstage, healthpercent;
	float fullhealth;
	objectData_t	*building = CL_ObjectType(obj->base.type);

	//no effects if it's dead
	if (!obj->base.health)
		return;

	//apply special shader if it's being built
	if (obj->visual.flags & BASEOBJ_UNDER_CONSTRUCTION)
	{
//		if (obj->base.percentToComplete > 0)
		{			
			sc->shader = corec.Res_LoadShader(cl_constructionShader.string);
			sc->flags |= SCENEOBJ_SINGLE_SHADER;
			//sc->color[0] = sc->color[1] = sc->color[2] = 0.5;
			//sc->color[3] = 0.75 + 0.25*
		//			cos(cl_throbRate.value*corec.Seconds());
						
			//sc->flags |= SCENEOBJ_SOLID_COLOR;
		}

		//add under construction looping sound
		if (building->constructionSound[0])
			corec.Sound_AddLoopingSound(corec.Res_LoadSound(building->constructionSound), obj->base.index, PRIORITY_LOW);
	}
	
	//determine current health stage to use
	fullhealth = obj->visual.fullhealth;
	if (fullhealth)
		healthpercent = (obj->visual.health / fullhealth) * 100;
	healthstage = 0;
	while (healthpercent < building->healthStages[healthstage])
	{
		healthstage++;

		if (healthstage >= BUILDING_HEALTH_STAGES)
			break;
	}
	healthstage--;

	//apply the effect
	if (healthstage >= 0 && !(obj->visual.flags & BASEOBJ_UNDER_CONSTRUCTION) && cl_healthStageMultiplier.value > 0)
	{
		if (!obj->lastDamageEffect)
			obj->lastDamageEffect = cl.gametime;

		if (cl.gametime - obj->lastDamageEffect > 2000)
			obj->lastDamageEffect = cl.gametime - 2000;

		while (true)
		{
			int increment = GETRANGE(building->healthStagePeriods[healthstage]) * (1 / cl_healthStageMultiplier.value);
			if (increment < 1)
				increment = 1;

			if (obj->lastDamageEffect + increment > cl.gametime)
				break;

			CL_DoEffect(obj, NULL, NULL, building->healthStageEffects[healthstage], 0, 0);
			obj->lastDamageEffect += increment;
		}
	}
	else
	{
		obj->lastDamageEffect = 0;
	}

	//structure damage effect	
	if (obj->base.health - obj->oldbase.health < 0 && !CL_ObjectType(obj->base.type)->isMine)		//structure took damage
	{
		obj->damagePulseTime = MAX(obj->damagePulseTime, (obj->oldbase.health - obj->base.health) * 4);
		if (obj->damagePulseTime > 2000)
			obj->damagePulseTime = 2000;

		obj->damagePulseDuration = obj->damagePulseTime;
	}

	if (obj->damagePulseTime && obj->damagePulseDuration)
	{
		sc->color[1] = sc->color[2] = 1 - ((float)obj->damagePulseTime / (float)obj->damagePulseDuration);
		sc->color[0] = sc->color[3] = 1;		
		sc->flags |= SCENEOBJ_SOLID_COLOR;
	}

	if (obj->damagePulseTime > 0)
	{
		obj->damagePulseTime -= cl.frametime * 1000.0;
		if (obj->damagePulseTime < 0)
			obj->damagePulseTime = obj->damagePulseDuration = 0;
	}
}


/*==========================

  Rend_DoSpecialProjectileRendering

  adds trails and adjusts the orientation to match the trajectory

 ==========================*/

void	Rend_DoSpecialProjectileRendering(clientObject_t *obj, sceneobj_t *sc)
{
	vec3_t	forward;
	objectData_t *wp = CL_ObjectType(obj->visual.type);
	bool isContinuous = wp->continuous;

	//add looping sound
	if (wp->projectileSound[0])
		corec.Sound_AddLoopingSound(corec.Res_LoadSound(wp->projectileSound), obj->base.index, obj->base.owner == cl.clientnum ? PRIORITY_MEDIUM : PRIORITY_LOW);

	//rotate based on velocity
	Traj_GetVelocity(&obj->base.traj, cl.gametime, forward);
	M_GetAxisFromForwardVec(forward, sc->axis);
	sc->flags |= SCENEOBJ_USE_AXIS;	

	if (wp->flybyEffect && !obj->triggeredFlyBy)
	{		
		if (obj->base.owner != cl.clientnum || cl_debugFlyby.integer)
		{
			float c1,c2,b;
			vec3_t flybypos;
			vec3_t v,w;		

			M_SubVec3(obj->base.pos, obj->oldbase.pos, v);
			M_SubVec3(cl.camera.origin, obj->oldbase.pos, w);

			c1 = M_DotProduct(w,v);
			c2 = M_DotProduct(v,v);
			if (c1 > 0 && c2 > c1)
			{
				b = c1/c2;
				M_PointOnLine(obj->oldbase.pos, v, b, flybypos);

				if (M_GetDistanceSq(cl.camera.origin, flybypos) < 1200)
				{
					//projectile flew by the camera this frame
					//hack around the old effect system for now by copying a new position into the clientobject
					vec3_t oldpos;
					if (cl_debugFlyby.integer)
						corec.Console_Printf("flyby: %f\n", M_GetDistance(cl.camera.origin,flybypos));
					M_CopyVec3(obj->visual.pos, oldpos);
					M_CopyVec3(flybypos, obj->visual.pos);
					CL_DoEffect(obj, NULL, flybypos, wp->flybyEffect, 0, 0);
					M_CopyVec3(oldpos, obj->visual.pos);
					obj->triggeredFlyBy = true;
				}
			}
		}
	}

	if (obj->snapToMuzzle && (cl_muzzleSnap.integer || isContinuous))		//will get set for all projectiles
	{
		//always snap the trajectory origin to the muzzle in third person views
		//in first person, we snap it only if the error is below a certain threshold
		//error will be higher when player movement is faster, ping is higher, or
		//other network latency issues
		clientObject_t *owner = &cl.objects[obj->base.owner];
		if (!cl.thirdperson && owner == cl.player)
		{
		//	if (CL_ObjectType(obj->base.type)->fuseTime.max < 500)
			{
				float error;
				vec3_t muzzle;
				
				//set the muzzle position for the FPS weapon				
				M_SetVec3(muzzle, cl.predictedState.pos[0], cl.predictedState.pos[1], cl.predictedState.pos[2] + CL_ObjectType(cl.predictedState.unittype)->viewheight);
				M_AddVec3(muzzle, cl.effects.posOffset, muzzle);
				M_PointOnLine(muzzle, cl.camera.viewaxis[0], wp->muzzleOffset[0], muzzle);
				M_PointOnLine(muzzle, cl.camera.viewaxis[1], wp->muzzleOffset[1], muzzle);
				M_PointOnLine(muzzle, cl.camera.viewaxis[2], wp->muzzleOffset[2], muzzle);

				error = M_GetDistanceSq(muzzle, obj->base.traj.origin);
				if (error < 500 || isContinuous)
				{
					M_CopyVec3(muzzle, obj->base.traj.origin);
				}
				if (cl_debugMuzzle.integer)
				{
					corec.Console_Printf("Muzzle error: %.2f\n", error);
				}
			}
		}
		else if (owner->hasMuzzle)		//we've computed the muzzle position
		{
			M_SetVec3(obj->base.traj.origin, owner->muzzlePos[0], owner->muzzlePos[1], owner->muzzlePos[2]);
		}
		if (isContinuous)
		{
			obj->lastTrailEffect = cl.gametime;
		}
		else
		{
			obj->snapToMuzzle = false;		//don't update trajectory again
		}

		M_CopyVec3(obj->base.traj.origin, obj->visual.traj.origin);
		M_CopyVec3(obj->base.traj.origin, obj->oldbase.traj.origin);
	}
	
	if (obj->drawFinalTrail==1)
	{
		obj->drawFinalTrail++;		//won't touch this object again
		if (CL_ObjectType(obj->base.type)->trailEffect > MAX_EXEFFECTS)
		{
			if (CL_ObjectType(obj->base.type)->objclass == OBJCLASS_WEAPON && CL_ObjectType(obj->base.type)->trailEffect && cl_drawTrailSegmentOnDeath.integer)
			{
				if (!obj->lastTrailEffect)
					obj->lastTrailEffect = obj->base.traj.startTime;

				//M_CopyVec3(obj->oldbase.pos, obj->visual.pos);
				Traj_GetPos(&obj->oldbase.traj, obj->lastTrailEffect, obj->oldbase.pos);

				CL_DoEffect(obj, NULL, NULL, CL_ObjectType(obj->base.type)->trailEffect, 0, 0);
				//obj->lastTrailEffect = cl.gametime + 30000;	//this will prevent a duplicate trail being drawn in doSpecialProjectileRendering
			}
		}
		else
		{
			CL_StopEffect(obj->trailEffectID);
		}

		return;
	}

	//draw the trail
	if (CL_ObjectType(obj->base.type)->trailEffect > MAX_EXEFFECTS)
	{
		vec3_t	realVisualPos, realOldBasePos;

		//save the object real position
		M_CopyVec3(obj->visual.pos, realVisualPos);
		M_CopyVec3(obj->oldbase.pos, realOldBasePos);
		

		if (obj->lastTrailEffect < obj->visual.traj.startTime)
			obj->lastTrailEffect = obj->visual.traj.startTime;

		if (cl.trajectoryTime - obj->lastTrailEffect > 250)
		{
			//core.Console_DPrintf("WARNING: lastTrailTime for %i was > 250\n", obj->base.index);
			obj->lastTrailEffect = cl.trajectoryTime - 250;
		}

		while (true)
		{
			int increment = GETRANGE(CL_ObjectType(obj->base.type)->trailPeriod);
			if (increment < 1)
				increment = 1;

			//if the next increment is beyond the current time, we're done
			if ((obj->lastTrailEffect + increment) > cl.trajectoryTime)
			{
				break;
			}
			//otherwise, simulate the objects position at the time increment and do an effect there
			else
			{
				if (obj->base.flags & BASEOBJ_USE_TRAJECTORY)
					Traj_GetPos(&obj->visual.traj, obj->lastTrailEffect, obj->oldbase.pos);
				obj->lastTrailEffect += increment;
				if (obj->base.flags & BASEOBJ_USE_TRAJECTORY)
					Traj_GetPos(&obj->visual.traj, obj->lastTrailEffect, obj->visual.pos);
				CL_DoEffect(obj, NULL, NULL, CL_ObjectType(obj->visual.type)->trailEffect, 0, 0);
			}
		}

		//restore real position
		M_CopyVec3(realVisualPos, obj->visual.pos);
		M_CopyVec3(realOldBasePos, obj->oldbase.pos);
	}
	else if (CL_ObjectType(obj->visual.type)->trailEffect)
	{
		if (!obj->trailEffectID)
			obj->trailEffectID = CL_StartEffect(obj, NULL, NULL, CL_ObjectType(obj->visual.type)->trailEffect, 0, 0);
	}
}


void	Rend_DrawContinuousBeam(clientObject_t *obj)
{
	clientObject_t *efobj;
	sceneobj_t sc;
	traceinfo_t trace;
	vec3_t muzzle;
	vec3_t dir;
	vec3_t target;		
	float len;
	float range;
	int n,b;
	int numsegs;
	vec3_t pos_a;		
	vec3_t veclastoffset;
	float offset;

	if (obj->base.index == cl.clientnum && !cl.thirdperson)
	{
		vec3_t offset;
		
		M_TransformPoint(CL_ObjectType(obj->visual.weapon)->muzzleOffset, zero_vec,
						 cl.camera.viewaxis, offset);

		//when in first person, the muzzle position is our predicted position plus the muzzle offset
		muzzle[0] = cl.predictedState.pos[0] + offset[0] + cl.effects.posOffset[0];
		muzzle[1] = cl.predictedState.pos[1] + offset[1] + cl.effects.posOffset[1];
		muzzle[2] = cl.predictedState.pos[2] + offset[2] + CL_ObjectType(cl.predictedState.unittype)->viewheight + cl.effects.posOffset[2];

		HeadingFromAngles(cl.predictedState.angle[PITCH], cl.predictedState.angle[YAW], dir, NULL);			
	}
	else
	{
		muzzle[0] = obj->muzzlePos[0];
		muzzle[1] = obj->muzzlePos[1];
		muzzle[2] = obj->muzzlePos[2];

		HeadingFromAngles(obj->visual.angle[PITCH], obj->visual.angle[YAW], dir, NULL);
	}

	range = GETRANGE(CL_ObjectType(obj->visual.weapon)->velocity);

	M_PointOnLine(muzzle, dir, range/*fixme: weapon range*/, target);

	/*
	if (_isnan(muzzle[0]))
	{
		corec.Console_DPrintf("nan\n");
	}
	*/
	
	corec.World_TraceBox(&trace, muzzle, target, zero_vec, zero_vec, TRACE_PROJECTILE);

	len = range * trace.fraction;
	
	numsegs = len/10+1;

	M_CopyVec3(muzzle, pos_a);

	offset = 0;
	M_ClearVec3(veclastoffset);

	for (b=0; b<3; b++)
	{
		M_ClearVec3(veclastoffset);
		for (n=0; n<numsegs; n++)
		{			
			float offset = /*((cl.systime % 6) + */((rand() % 11)-5) /* * sin(cl.systime/100.0+n)*/ * 0.15;//rand();//M_Randnum(-1, 1);			
			vec3_t vecoffset={0,0,0};

			
			

			//offset = sin(cl.systime * 0.03 * (n+1))*0.5;//M_Randnum(-2,2);
		
			sc.objtype = OBJTYPE_BEAM;
			sc.scale = 1;
			sc.flags = SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME | SCENEOBJ_SOLID_COLOR; 
			
			sc.shader = corec.Res_LoadShader(CL_ObjectType(obj->visual.weapon)->continuousBeamShader);

			sc.loframe = cl.systime;//LERP(lerp, 0.0, (float)corec.Res_GetNumTextureFrames(sc.shader));
			SET_VEC4(sc.color, 1,1,1,1);						

			M_PointOnLine(muzzle, dir, 10*n, sc.pos);
			M_AddVec3(sc.pos, veclastoffset, sc.pos);

			//set target position for this segment
			if (n == numsegs-1)
				M_CopyVec3(trace.endpos, sc.beamTargetPos);
			else
				M_PointOnLine(muzzle, dir, 10*(n+1), sc.beamTargetPos);

			//offset it
			M_PointOnLine(vecoffset, cl.camera.viewaxis[0], offset * (b==0 ? 1 : -1), vecoffset);			
			M_PointOnLine(vecoffset, cl.camera.viewaxis[1], offset * (b==1 ? 1 : -1), vecoffset);
			M_PointOnLine(vecoffset, cl.camera.viewaxis[2], offset * (b==2 ? 1 : -1), vecoffset);			
			
			M_AddVec3(sc.beamTargetPos, vecoffset, sc.beamTargetPos);

			sc.scale = 0.3;
			sc.height = 1;

			M_CopyVec3(vecoffset, veclastoffset);			

			//M_CopyVec3(trace.endpos, sc.beamTargetPos);


			//M_SubVec3(sc.pos, sc.beamTargetPos, vec);
			//length = M_Normalize(vec);
			//if (beams[n].tilelength)
			//	sc.height = length / beams[n].tilelength;
			//else


			corec.Scene_AddObject(&sc);
		}
	}

	if (cl.systime > obj->lastContinuousEffect + 70 && trace.fraction < 1)		//do impact effects 70ms apart
	{
		//HACK: generate the impact effect by using an array of clientobjects separate from the main array
		efobj = &cl.effectObjs[cl.effectObjCount % (sizeof(cl.effectObjs) / sizeof(clientObject_t))];
		cl.effectObjCount++;
		M_CopyVec3(trace.endpos, efobj->visual.pos);
		M_CopyVec3(trace.endpos, efobj->base.pos);
		
		CL_DoEffect(efobj, NULL, trace.endpos, CL_ObjectType(obj->visual.weapon)->effects[EVENT_DEATH], 0, 0);

		obj->lastContinuousEffect = cl.systime;
	}
}



/*==========================

  Rend_DoSpecialCharacterRendering

  Adds armor for player levels, state effects and other miscellaneous effects

 ==========================*/

void	Rend_DoSpecialCharacterRendering(clientObject_t *obj, sceneobj_t *sc)
{
	sceneobj_t	armor;
	int level, n, index;
	bool iscloaked = false;
	bool adjustedCloak = false;
	int cloakshader = -1;
	float brightnesstarget;	

	{
		PERF_BEGIN;
	
		brightnesstarget = CL_SampleBrightness(obj->visual.pos);

		sc->flags |= SCENEOBJ_ALWAYS_BLEND;

		if (obj->oldbase.active)	
		{
			obj->brightness = M_ClampLerp(cl.frametime * 4, obj->brightness, brightnesstarget);
		}
		else
		{
			obj->brightness = brightnesstarget;
		}
			
		sc->color[0] = obj->brightness * cl_charBrightnessFactor.value;
		sc->color[1] = obj->brightness * cl_charBrightnessFactor.value;
		sc->color[2] = obj->brightness * cl_charBrightnessFactor.value;	
		sc->color[3] = 1;
			
		sc->flags |= SCENEOBJ_SOLID_COLOR;

		PERF_END(PERF_SCR_SAMPLEBRIGHTNESS);
	}

	{
		PERF_BEGIN;

		//pre-check for cloaking
		for (n = 0; n < MAX_STATE_SLOTS; n++)
		{
			stateData_t	*state = &stateData[obj->visual.states[n]];

			if (!obj->visual.states[n])
				continue;

			//check for cloaking
			if (state->cloak > 0.0)
			{
				iscloaked = true;

				if (obj->cloakAmount < state->cloak && !adjustedCloak)
				{
					int msecs;

					adjustedCloak = true;
					if (!obj->cloakLastAdjust)
						obj->cloakLastAdjust = cl.gametime;
					msecs = cl.gametime - obj->cloakLastAdjust;
					obj->cloakAmount += cl_cloackAdjustRate.value * (msecs / 1000.0);
					obj->cloakAmount = MIN(1.0, obj->cloakAmount);
					obj->cloakLastAdjust = cl.gametime;
				}

				if (obj->cloakAmount < 0.85)
					sc->color[3] = 1.0 - obj->cloakAmount;
				else
					sc->color[3] = 1.0;
				if (state->singleShader[0] && obj->cloakAmount >= 0.85)
				{

					sc->shader = corec.Res_LoadShader(state->singleShader);
					cloakshader = sc->shader;
					sc->flags |= SCENEOBJ_SINGLE_SHADER | SCENEOBJ_ALWAYS_BLEND;
				}
			}
		}

		//state effects
		for (n = 0; n < MAX_STATE_SLOTS; n++)
		{
			stateData_t	*state;
			int m;

			if (!obj->visual.states[n])
			{
				obj->lastStateEffect[n] = 0;
				obj->stateAnimTime[n] = 0;
				continue;
			}

			state = &stateData[obj->visual.states[n]];

			//apply the effect
			if (!obj->lastStateEffect[n])
				obj->lastStateEffect[n] = cl.gametime;

			if (cl.gametime - obj->lastStateEffect[n] > 1000)
				obj->lastStateEffect[n] = cl.gametime - 1000;

			while (true)
			{
				int increment = GETRANGE(state->effectPeriod);
				if (increment < 1)
					increment = 1;

				if (obj->lastStateEffect[n] + increment > cl.gametime)
					break;

				CL_DoEffect(obj, NULL, NULL, state->effect, 0, 0);
				obj->lastStateEffect[n] += increment;
			}

			for (m = 0; m < 5; m++)
			{
				int model = 0, skin = 0, param = -1;
				sceneobj_t efmodel;

				//set the param for the officer state
				if (state->index == raceData[cl.race].officerState)
				{
					for (index = 0; index < MAX_OFFICERS; index++)
					{
						if (obj->base.index == cl.officers[index])
							param = index;
					}
				}

				//show special models
				if (state->model[m][0])
				{
					//try to load the model + unit_type
					if (!model)
						model = corec.Res_LoadModel(fmt("%s_%s.model", state->model[m], CL_ObjectType(obj->visual.type)->name));

					//try to load just the model
					if (!model)
						model = corec.Res_LoadModel(fmt("%s.model", state->model[m]));
				}

				//now look for a skin
				if (state->skin[m][0] && model)
				{
					//try to load the skin + unit_type
					if (!skin)
						skin = corec.Res_LoadSkin(model, fmt("%s_%s.skin", state->skin[m], CL_ObjectType(obj->visual.type)->name));

					//try to load just the skin
					if (!skin)
						skin = corec.Res_LoadSkin(model, fmt("%s.skin", state->skin[m]));
				}

				if (model && (cl.clientnum != obj->base.index || cl.thirdperson))
				{
					if (state->bone[m][0])
					{
						sceneobj_t attach;
						CLEAR_SCENEOBJ(attach);
						if (cloakshader > -1)
						{
							attach.shader = cloakshader;
							attach.flags |= SCENEOBJ_SINGLE_SHADER | SCENEOBJ_ALWAYS_BLEND;
						}
						Rend_AttachToSkeleton(sc, state->bone[m], model, true, &attach);
						corec.Scene_AddObject(&attach);
					}
					else
					{
						if (state->useCharAnim[m])
						{
							memcpy(&efmodel, sc, sizeof(sceneobj_t));
							efmodel.model = model;
							efmodel.skin = skin;
							corec.Scene_AddObject(&efmodel);
						}
						else
						{
							memset(&efmodel, 0, sizeof(sceneobj_t));
							M_CopyVec3(sc->angle, efmodel.angle);
							M_CopyVec3(sc->axis[0], efmodel.axis[0]);
							M_CopyVec3(sc->axis[1], efmodel.axis[1]);
							M_CopyVec3(sc->axis[2], efmodel.axis[2]);
							efmodel.flags = SCENEOBJ_USE_AXIS | SCENEOBJ_NEVER_CULL | SCENEOBJ_ALWAYS_BLEND;
							efmodel.index = sc->index;
							efmodel.model = model;
							efmodel.skin = skin;
							M_CopyVec3(sc->pos, efmodel.pos);
							efmodel.scale = 1.0;
							efmodel.skeleton = &obj->stateSkeleton[n];

							corec.Geom_BeginPose(efmodel.skeleton, model);
							corec.Geom_SetBoneAnim("", "item_active", obj->stateAnimTime[n], cl.systime, 200, 0);
							corec.Geom_EndPose();
							corec.Scene_AddObject(&efmodel);
						}
					}
				}
			}
		}

		//fade out cloaking if it is no longer being applied
		if (!iscloaked)
		{
			if (obj->cloakAmount > 0.0)
			{
				int msecs;

				msecs = cl.gametime - obj->cloakLastAdjust;
				obj->cloakAmount -= cl_cloackAdjustRate.value * (msecs / 1000.0);
				obj->cloakAmount = MAX(0.0, obj->cloakAmount);
				obj->cloakLastAdjust = cl.gametime;
			}
			else
			{
				obj->cloakLastAdjust = 0;
			}

			sc->color[3] = 1.0 - obj->cloakAmount;
		}

		PERF_END(PERF_SCR_STATES);
	}

	{
		PERF_BEGIN;
	
		if (obj->base.flags & BASEOBJ_COMMANDER_SELECTED && cl_showCommanderSelection.integer && obj->base.team == cl.info->team)
			Rend_DrawIndicator(obj->base.index, 1);

		if (!iscloaked && (obj->base.animState < AS_DEATH_GENERIC || obj->base.animState > AS_DEATH_BACK))
		{
			CL_AddShadow(sc, obj->base.index);
		}

		PERF_END(PERF_SCR_GROUNDMARKS);
	}

	//don't do armor or attachments for the client's own player if they are in first person
	if (obj->base.index != cl.clientnum || cl.thirdperson)
	{

		//determine level to use for experience awards
		if (obj->base.index < MAX_CLIENTS)
			level = cl.clients[obj->base.index].score.level;
		else
		{
			if (obj->base.flags & BASEOBJ_NAMEIDX_SPECIFIES_LEVEL)
				level = obj->base.nameIdx;
			else
				level = 0;
		}
		
		if (cl_showLevelArmor.integer)
		{
			PERF_BEGIN;

			//draw most recently awarded armor
			memcpy(&armor, sc, sizeof(sceneobj_t));

			armor.model = 0;
			armor.skin = 0;
			for (index = 1; index <= level; index++)
			{
				int newmodel=0;
				int race = cl.teams[obj->visual.team].race;

				if (!experienceTable[race][index].bodymodel[0])
					continue;

				newmodel = corec.Res_LoadModel(fmt("%s_%s.model", experienceTable[race][index].bodymodel, CL_ObjectType(obj->visual.type)->name));

				if (newmodel)
					armor.model = newmodel;
			}

			if (armor.model)
			{
				//map the armor to the character skeleton so it deforms with the character
				//armor.skeleton = obj->skeleton;

				corec.Scene_AddObject(&armor);
			}

			PERF_END(PERF_SCR_ARMOR);
		}

		//client only stuff beyond here
		if (obj->base.index >= MAX_CLIENTS)
			return;

		//draw attachments
		{
			PERF_BEGIN;
			Rend_AddPlayerAttachments(obj, sc, cloakshader);
			PERF_END(PERF_SCR_ATTACHMENTS);
		}
	}
	
	//draw continuous weapon beams here
	if (obj->visual.exflags & BASEOBJEX_FIRING_CONTINUOUS)
	{
		PERF_BEGIN;
		Rend_DrawContinuousBeam(obj);
		PERF_END(PERF_SCR_CONTINUOUSBEAM);
	}

}


/*==========================

  Rend_DoSpecialItemRendering

  Handles periodic effects of items in their varius states (idling/activating)

 ==========================*/

void	Rend_DoSpecialItemRendering(clientObject_t *obj, sceneobj_t *sc)
{
	objectData_t	*itemData = CL_ObjectType(obj->visual.type);

	if (obj->visual.flags & BASEOBJ_ATTACHED_TO_OWNER && itemData->attachBone[0])
	{
		Rend_AttachToSkeleton(&cl.objects[obj->visual.owner].cachedSC, itemData->attachBone, sc->model, true, sc);
		M_CopyVec3(cl.objects[obj->visual.owner].cachedSC.angle, sc->angle);
		M_CopyVec3(sc->pos, obj->visual.pos);
	}

	switch (obj->visual.animState)
	{
	case AS_IDLE:
		if (itemData->delayEffect && itemData->delayPeriod)
		{
			int increment;

			if (!obj->lastDelayEffect)
				obj->lastDelayEffect = cl.gametime - itemData->delayPeriod;

			increment = itemData->delayPeriod;
			if (increment < 1)
				increment = 1;

			while (1)
			{
				if (obj->lastDelayEffect + increment >= cl.gametime)
					break;

				CL_DoEffect(obj, NULL, NULL, itemData->delayEffect, 0, 0);
				obj->lastDelayEffect += increment;
			}
		}
		break;

	case AS_ITEM_ACTIVE:
		if (itemData->activeEffect && itemData->activeEffectPeriod)
		{
			int increment;

			if (!obj->lastActiveEffect)
				obj->lastActiveEffect = cl.gametime;

			increment = itemData->activeEffectPeriod;
			if (increment < 1)
				increment = 1;

			while (1)
			{
				if (obj->lastActiveEffect + increment >= cl.gametime)
					break;

				CL_DoEffect(obj, NULL, NULL, itemData->activeEffect, 0, 0);
				obj->lastActiveEffect += increment;
			}
		}
		break;
	default:
		break;
	}
}

void	Rend_MapBlip(int x, int y, int w, int h, residx_t shader, int msec)
{
	simpleParticle_t *part = CL_AllocParticle();
	if (!part)
		return;
	part->ortho = true;
	part->pos[0] = x;
	part->pos[1] = y;
	part->width = w;
	part->height = h;
	part->shader = shader;
	CL_PlayAnimOnce(part, 10, shader);
}


/*==========================

  Rend_AddObjects

  this tells the renderer everything it needs to know about what objects and
  object related effects to draw

 ==========================*/

void	Rend_AddObjects()
{
	bool addIt = true;
	int i;
	//float dist;
	sceneobj_t sc;
	scenelight_t light;

	PERF_BEGIN;

	if (!cl.numSvObjs)
	{
		//we haven't received a server frame yet
		return;
	}

	SET_VEC3(light.color, 1, 1, 1);
	light.intensity = 100;
	M_CopyVec3(cl.camera.origin, light.pos);
	//corec.Scene_AddLight(&light);

	corec.Scene_SetFrustum(&cl.camera);

	for (i=0; i<cl.numSvObjs; i++)
	{
		clientObject_t *obj = cl.svObjs[i];
		byte type;
		objectData_t *def;
		
		if (!obj->visual.active || !obj->base.active)
			continue;
		if (!obj->oldbase.active)		//must have a lerp target
			continue;
		if (obj->visual.flags & BASEOBJ_NO_RENDER)
			continue;
		if (obj->visual.flags & BASEOBJ_MARKED_FOR_DEATH)
			continue;
		if (obj->drawFinalTrail==2)
			continue;

		def = CL_ObjectType(obj->visual.type);
		type = obj->visual.type;		
		obj->lastSCFrame = cl.frame;

		if (obj->inFogOfWar)
		{
			if (IsBuildingType(obj->visual.type))
			{
				sc.flags |= SCENEOBJ_USE_ALPHA;
				sc.alpha = cl_notvis_alpha.value;
			}
			else
				continue;
		}

		if (!IsWeaponType(type) && !(obj->visual.flags & BASEOBJ_UNDER_CONSTRUCTION))
		{
			//add looping sound
			CL_AddLoopingSound(def, obj->visual.animState, obj->visual.index, obj->visual.index == cl.clientnum ? PRIORITY_HIGH : PRIORITY_LOW);			
		}

		if (obj->visual.index == cl.clientnum)
		{
			if (cl.predictedState.flags & PS_NOT_IN_WORLD)
				continue;

			if (!cl.thirdperson)
			{
				PERF_BEGIN;
				Rend_DoSpecialCharacterRendering(cl.player, &sc);
				PERF_END(PERF_SPECIALCHARACTERRENDERING);
				continue;
			}
		}

		if (obj->visual.flags & BASEOBJ_EXPIRING)
		{
			//blink it on and off
			//add in a little offset based on the object index so objects blink out of sync
			if ((cl.gametime + obj->base.index*100) % 500 < 250)
				continue;
		}
		
		//this calls CLEAR_SCENEOBJ, so we don't have to
		Rend_ClientObjectToSceneObject(obj, &sc, 0);

		//work out scale
		//if (IsUnitType(type))
		{
			if (cl.isCommander)
				sc.scale = def->cmdrScale;		//hack to scale up characters in commander view
		}

		sc.scale *= def->scale;

		//do skeletal posing and anim events
		if (corec.Scene_ObjectInFrustum(&sc))
		{
			addIt = true;
			CL_PoseModel(obj, false);
		}
		else
		{
			addIt = false;
			CL_PoseModel(obj, true);
		}

		//add a selection flag to this object so that we can select it via Scene_SelectObjects()
		if (IsMobileType(obj->visual.type) || IsBuildingType(obj->visual.type))
		{
			if (obj->visual.health > 0)
			{
				sc.flags |= SCENEOBJ_SELECTABLE;
				sc.selection_id = obj->visual.index;
			}
		}

		if (IsCharacterType(type))
		{
			PERF_BEGIN;
			Rend_DoSpecialCharacterRendering(obj, &sc);
			PERF_END(PERF_SPECIALCHARACTERRENDERING);
		}
		else if (IsBuildingType(type))
		{
			PERF_BEGIN;
			sc.scale = STRUCTURE_SCALE;		//hack to scale down structures
			Rend_DoSpecialStructureRendering(obj, &sc);
			PERF_END(PERF_SPECIALSTRUCTURERENDERING);
		}
		else if (CL_ObjectType(type)->objclass == OBJCLASS_WEAPON)
		{
			//if (cl.objects[n].visual.traj.startTime > cl.gametime)
			//	continue;
			PERF_BEGIN;

			Rend_DoSpecialProjectileRendering(obj, &sc);

			PERF_END(PERF_SPECIALPROJECTILERENDERING);

			if (!def->projectileModel[0])
				continue;
		}
		else if (IsItemType(type))
		{
			PERF_BEGIN;

			Rend_DoSpecialItemRendering(obj, &sc);

			PERF_END(PERF_SPECIALITEMRENDERING);

			if (!def->model[0])
				continue;
		}

		if (def->drawTeamIcon &&
			cl.info->team &&
			obj->visual.team == cl.info->team)
		{
			if ((obj->visual.surfaceFlags & SURF_REVIVABLE) &&
				CL_ObjectType(cl.predictedState.unittype)->isHealer &&
				obj->visual.health <= 0)
				Rend_DrawFriendlyIcon(obj, true);
			else
				Rend_DrawFriendlyIcon(obj, false);
		}

		if (obj->visual.index < MAX_CLIENTS)
		{
			clientInfo_t *client = &cl.clients[obj->visual.index];
			if (client->voiceIconTime && cl.gametime < client->voiceIconTime)
			{
				Rend_DrawVoiceIcon(obj);
			}
		}

		//if (cl.isCommander)
		{
			if (IsUnitType(type))
			{
				//if (obj->visual.team == cl.info->team)
				{
					sc.flags |= SCENEOBJ_RTS_SILHOUETTE;		//allows us to see our units even when occluded
				}
			}
		}

		sc.flags |= SCENEOBJ_ALWAYS_BLEND;			//ignore gfx_noblending for all dynamic objects

		if (def->alwaysWaypoint &&
			obj->visual.health > 0 &&
			!(obj->visual.flags & BASEOBJ_UNDER_CONSTRUCTION))
		{			
			sceneobj_t beam;
			float dist, mod, modb;
			
			CLEAR_SCENEOBJ(beam);

			beam.objtype = OBJTYPE_BILLBOARD;
			beam.width = 100;
			beam.height = 10000;
			beam.shader = res.lightbeamShader;
			M_CopyVec3(obj->visual.pos, beam.pos);

			dist = M_GetDistanceSq(cl.objects[cl.clientnum].visual.pos, beam.pos);
			if (dist < 400000)
				beam.alpha = 0.01 + (0.7 - 0.01) * (sqrt(dist) / sqrt(400000));
			else
				beam.alpha = 0.7;

			mod = ((cl.gametime % 2500) / 2500.0);
			mod = (mod < .5) ? mod * 2.0 : 1.0 - ((mod - 0.5) * 2.0);

			modb = ((cl.gametime % 1250) / 1250.0);
			modb = (modb < .5) ? modb * 2.0 : 1.0 - ((modb - 0.5) * 2.0);

			SET_VEC4(beam.color, modb * beam.alpha, beam.alpha, mod * beam.alpha, beam.alpha);

			{
				PERF_BEGIN;
				corec.Scene_AddObject(&beam);
				PERF_END(PERF_ADDTOSCENE);
			}
		}

		Rend_DrawEffects(obj);

		/*}*/
		if (addIt)
		{
			PERF_BEGIN;
			corec.Scene_AddObject(&sc);
			PERF_END(PERF_ADDTOSCENE);
		}
		
		obj->cachedSC = sc;

		if (def->effect)
			CL_DoEffect(obj, NULL, NULL, def->effect, 0, 0);

		if (IsWeaponType(obj->visual.type))
			if (def->projectileEffect)
			CL_DoEffect(obj, NULL, NULL, def->projectileEffect, 0, 0);

	}
	if (!(cl.camera.flags & CAM_NO_WORLDOBJECTS))
	{
		PERF_BEGIN;
		corec.WO_RenderObjects(cl.camera.origin);
		PERF_END(PERF_ADDWORLDPROPS);
	}

	PERF_END(PERF_RENDADDOBJECTS);
}
/*
void	Rend_RenderPlayerCharacter()
{
	sceneobj_t sc;

	CLEAR_SCENEOBJ(sc);

	Rend_ClientObjectToSceneObject(&cl.player, &sc, 0);

	corec.Scene_AddObject(&sc);
}
*/



void    CL_DrawFPS ()
{
	static int lasttime = 0;
	static int time = 0;
	char s[256];
	static int fps = 0;
	static int count = 0;
	
	time = corec.Milliseconds();
	
	count++;

	if (time - lasttime >= 1000)
	{
		lasttime = time;
		fps = count;		
		count = 0;
	}
	
	corec.Draw_SetColor(white);
	
	BPrintf(s, 255, "FPS: %i", fps);
	s[255] = 0;
	
	if (cl.isCommander)
		corec.GUI_DrawStringMonospaced(10, 40, s, 16, 16, 1, 1024, corec.GetSysfontShader());	
	else
		corec.GUI_DrawStringMonospaced(1024 - (strlen(s)*16+4) -100, 768 - 120, s, 16, 16, 1, 1024, corec.GetSysfontShader());	

}



/*
// converts values given in 640x480 to screenwidth x screenheight
void	TL_640x480(int *x, int *y, int *w, int *h)
{	
	float fx,fy,fw,fh;

	fx = *x * le.screenscalex;
	fy = *y * le.screenscaley;
	fw = *w * le.screenscalex;
	fh = *h * le.screenscaley;

	//assumes coordinate pointers are valid
	*x = (int)fx;
	*y = (int)fy;
	*w = (int)fw;
	*h = (int)fh;
}

// accepts 640x480 coords
void	TL_Quad2d(int x, int y, int w, int h, residx_t shader)
{
	TL_640x480(&x, &y, &w, &h);
	corec.Draw_Quad2d(x, y, w, h, 0, 0, 1, 1, shader);
}
*/



extern cvar_t int_crosshairsize;

void	Rend_DrawPlayerCrosshair()
{
	float w,h;
	int width, height;	
	objectData_t *unitData;
	objectData_t *wp;

	w = corec.Vid_GetScreenW() / 2;
	h = corec.Vid_GetScreenH() / 2;

	width = int_crosshairsize.integer;
	height = int_crosshairsize.integer;
	corec.GUI_ScaleToScreen(&width, &height);
	
	corec.Draw_SetColor(white);
	unitData = CL_ObjectType(cl.objects[cl.clientnum].base.type);	
	if (unitData->isVehicle && cl.status == STATUS_PLAYER && !CL_Dead())
	{
		float minx, miny, maxx, maxy;

		minx = w*2 * unitData->minAimX;
		maxx = (w*2 * unitData->maxAimX) - minx;
		miny = h*2 * unitData->minAimY;
		maxy = (h*2 * unitData->maxAimY) - miny;
		//xhair = corec.Res_LoadShader(Tex("siege_crosshair"));
		//corec.Draw_Quad2d(cl_mousepos_x.value-(width/2), cl_mousepos_y.value-(height/2), width, height, 0, 0, 1, 1, xhair);
		TL_LineBox2d(minx, miny, maxx, maxy, 2);

		INT_DrawCrosshair(cl_mousepos_x.value - width/2, cl_mousepos_y.value-height/2, width, height, cl.predictedState.focus, false, false);
	}
	else
	{
		/*
		if (cl.thirdperson)
		{
			thirdpersonxhair = corec.Res_LoadShader(Tex("tp_crosshair"));
			corec.Draw_Quad2d(w-(width/2),h-(height/2),width, height ,0,0,1,1,thirdpersonxhair);
		}
		else
		{
			xhair = corec.Res_LoadShader(Tex("fp_crosshair"));
			corec.Draw_Quad2d(w-(width/2),h-(height/2),width, height ,0,0,1,1,xhair);
		}
		*/
		
		wp = CL_ObjectType(cl.predictedState.inventory[cl.predictedState.item]);
				
		INT_DrawCrosshair(w - width/2, h-height/2, width, height, cl.predictedState.focus, wp->objclass == OBJCLASS_MELEE, false);
	}
}


void	Rend_RenderFPSWeapon()
{
	sceneobj_t		sc;
	residx_t		wpModel, wpSkin;
	camera_t		cam;
	vec3_t			angles = {cl.predictedState.angle[PITCH] ,cl.predictedState.angle[ROLL],cl.predictedState.angle[YAW]};
	byte			item = cl.predictedState.inventory[cl.predictedState.item];	
	char			*animName;
	objectData_t	*wp;
	int				weapswitch;
	objectData_t *unit = CL_ObjectType(cl.predictedState.unittype);
	inputState_t	in;
	static vec3_t idlebob = {0,0,0};
	static vec3_t anglebob = {0,0,0};


	if (cl.thirdperson)
		return;

	if (!IsWeaponType(item) && !IsItemType(item))
		return;

	wp = CL_ObjectType(item);

	//add looping sound
	CL_AddLoopingSound(wp, cl.predictedState.weaponState, cl.predictedState.clientnum + WEAPON_SOUND_SOURCE_OFFSET, PRIORITY_HIGH);
	
	if (!cl_showWeapon.integer)
		return;

	idlebob[0] = sin(cl.systime * 0.00073) * 5;
	idlebob[1] = sin(cl.systime * 0.0007) * 3;

	if (cl.predictedState.animState >= AS_JUMP_START_LEFT && cl.predictedState.animState <= AS_JUMP_UP_END)
		idlebob[2] = M_ClampLerp(cl.frametime * 10, idlebob[2], -20);
	else
		idlebob[2] = M_ClampLerp(cl.frametime * 10, idlebob[2], sin(cl.systime * 0.00055) * 2);

	//anglebob[YAW] = sin(cl.systime * 0.0003) * 1;
	//anglebob[PITCH] = sin(cl.systime * 0.0002) * 5;

	//begin a new scene to overlay the hand model
	corec.Scene_Clear();

	Cam_DefaultCamera(&cam, corec.Vid_GetScreenW(), corec.Vid_GetScreenH());
	cam.time = Rend_SceneTime();

	CLEAR_SCENEOBJ(sc);

	M_SetVec3(cam.origin, cl.predictedState.pos[0],cl.predictedState.pos[1],cl.predictedState.pos[2]+unit->viewheight);	
	M_CopyVec3(cam.origin, sc.pos);

	//get the hands model associated with our unit type
	wpModel = CL_HandModel(item);
	wpSkin = CL_HandSkin(item, cl.info->team);

	if (!wpModel)
	{
		corec.Console_DPrintf("Couldn't find model for weapon %i\n", item);
		return;
	}

	corec.Client_GetInputState(corec.Client_GetCurrentInputStateNum(), &in);

	animName = GetAnimName(cl.predictedState.weaponState);

	sc.model = wpModel;
	sc.skin = wpSkin;	
	sc.skeleton = &cl.wpSkeleton;		//we keep a persistent skeleton between frames so that we can do blending on our weapon

	corec.Geom_BeginPose(sc.skeleton, sc.model);
	//inherit the player's weaponstate animation
	corec.Geom_SetBoneAnim("", animName, in.gametime - cl.predictedState.wpAnimStartTime, in.gametime, 100, 0);
	corec.Geom_EndPose();
	//fire off any anim events for the fps weapon
	if (sc.skeleton->gotEvent)
		CL_AnimEvent(cl.player, sc.skeleton->eventString);
	
	weapswitch = cl.gametime - cl.weaponSwitchTime;
	if (weapswitch > 200)
		weapswitch = 200;
	weapswitch = 200-weapswitch;

	sc.angle[0] = cl.predictedState.angle[PITCH] + anglebob[PITCH];
	sc.angle[2] = cl.predictedState.angle[YAW] + anglebob[YAW];

	Cam_SetAngles(&cam, angles);

	sc.pos[0] += cl_handoffset_x.value /*+ cl.effects.posOffset[0] */ + (cl.effects.walkbobPos[0] + idlebob[0]) * 0.05;
	sc.pos[1] += cl_handoffset_y.value /*+ cl.effects.posOffset[1] */ + (cl.effects.walkbobPos[1] + idlebob[1]) * 0.05;
	//hack to get walk bobbing in there..could do this better	
	sc.pos[2] += (cl.effects.walkbobPos[2] + idlebob[2]) * 0.05;	
	
	sc.pos[2] -= weapswitch/20 ;//+ cl.effects.posOffset[2] * cl_handJitterScale.value;
	sc.angle[1] += weapswitch/50;//180;

	M_GetAxis(sc.angle[0],sc.angle[1],sc.angle[2],sc.axis);

	cam.fovx = cl_handoffset_fov.value * (cl.camera.fovx / 90);
	Cam_CalcFovy(&cam);

	sc.color[0] = CLAMP(cl.player->brightness * cl_charBrightnessFactor.value, 0, 1);
	sc.color[1] = CLAMP(cl.player->brightness * cl_charBrightnessFactor.value, 0, 1);
	sc.color[2] = CLAMP(cl.player->brightness * cl_charBrightnessFactor.value, 0, 1);
	sc.color[3] = 1;

	sc.flags |= SCENEOBJ_NEVER_CULL | SCENEOBJ_USE_AXIS | SCENEOBJ_ALWAYS_BLEND | SCENEOBJ_SOLID_COLOR; // | SCENEOBJ_TRANSLATE_ROTATE;// | SCENEOBJ_USE_AXIS;

	cam.flags = CAM_NO_WORLD;

	corec.Scene_AddObject(&sc);

	corec.Scene_Render(&cam, zero_vec);
}


void	Rend_DrawPlayerName()
{
	static int	lastName = 0;
    vec3_t		dir, endpos, mins, maxs;
	traceinfo_t	trace;
	int			width, height;
	char		name[MAX_NAME_LEN], teamstr[10];
	int			level = 0;
	int			health, fullhealth;
	int			x, y;
	float		tracesize = 0;
	float		r, g, b;
	float		tracedist;

	if (!cl_drawPlayerName.integer)
		return;

	if (cl.info->team)
		tracedist = corec.Cvar_GetValue("gfx_farclip") * 0.5;
	else
		tracedist = corec.Cvar_GetValue("gfx_farclip");
	//trace ahead
	Cam_ConstructRay(&cl.camera, cl_mousepos_x.integer, cl_mousepos_y.integer, dir);
	M_PointOnLine(cl.camera.origin, dir, tracedist, endpos);
	M_SetVec3(mins, -tracesize, -tracesize, -tracesize);
	M_SetVec3(maxs, tracesize, tracesize, tracesize);
	corec.World_TraceBox(&trace, cl.camera.origin, endpos, mins, maxs, 0);

	//if we hit something, save the index and fade in the name, otherwise fade it out
	if (trace.index >= 0 && trace.index < MAX_OBJECTS && 
		(trace.index < MAX_CLIENTS || 
		IsUnitType(cl.objects[trace.index].visual.type) || 
		IsBuildingType(cl.objects[trace.index].visual.type || 
		CL_ObjectType(cl.objects[trace.index].visual.type)->objclass == OBJCLASS_ITEM)))
	{
		lastName = trace.index;
		namefont_strength = MIN(1.5, namefont_strength + 0.5);
	}
	else if (namefont_strength > 0)
	{
		namefont_strength -= cl_nameFadeRate.value;
	}

	corec.Cvar_SetVarValue(&cl_targetedObject, trace.index);
	corec.Cvar_SetVarValue(&cl_targetedTerrainX, trace.endpos[X]);
	corec.Cvar_SetVarValue(&cl_targetedTerrainY, trace.endpos[Y]);
	corec.Cvar_SetVarValue(&cl_targetedTerrainZ, trace.endpos[Z]);
	
	if (cl_showObjectInfo.integer)
	{
		if (trace.index < MAX_OBJECTS && trace.index > -1)
		{
#define PRINT(str) corec.GUI_DrawShadowedString(x, y, str, 12, 12, 1, 1024, corec.GetNicefontShader(), 1, 1, 1, 1); y+=14;
			int x = 150, y = 300;

			clientObject_t *obj = &cl.objects[trace.index];
			
			PRINT(fmt("Index: %i", trace.index));
			PRINT(fmt("Base idx: %i  Visual idx: %i", obj->base.index, obj->visual.index));
			PRINT(fmt("Type: %s (%i)", cl.objNames[obj->visual.type], obj->visual.type));
			PRINT(fmt("Vis. Pos: (%f, %f, %f)", obj->visual.pos[0], obj->visual.pos[1], obj->visual.pos[2]));
			PRINT(fmt("Base Pos: (%f, %f, %f)", obj->base.pos[0], obj->base.pos[1], obj->base.pos[2]));
			PRINT(fmt("Old  Pos: (%f, %f, %f)", obj->oldbase.pos[0], obj->oldbase.pos[1], obj->oldbase.pos[2]));
			PRINT(fmt("AnimState:  %s at %.0f msec", GetAnimName(obj->visual.animState), obj->animStateTime));
			PRINT(fmt("AnimState2: %s at %.0f msec", GetAnimName(obj->visual.animState2), obj->animState2Time));
			PRINT(fmt("Health: %i", obj->visual.health));
		}
		else if (trace.index > MAX_OBJECTS)
		{
			int x = 150, y = 300;

			PRINT(fmt("Index: %i", trace.index));
			PRINT("WORLD OBJECT");
			PRINT(fmt("Objdef: %s", corec.WO_GetObjdefName(corec.WO_GetObjectObjdef(trace.index))));
#undef PRINT
		}
	}

	if (lastName < 0 || lastName >= MAX_OBJECTS)
		lastName = 0;

	//get the name and level of what we are identifying
	if (lastName < MAX_CLIENTS)
	{
		strcpy(name, cl.clients[lastName].info.name);
		level = cl.clients[lastName].score.level;
	}
	else if (IsUnitType(cl.objects[lastName].visual.type))
	{
		level = CL_ObjectType(cl.objects[lastName].visual.type)->level;
		if (cl.objects[lastName].visual.nameIdx)
			strcpy(name, fmt("Worker")); // %s", peonNames[cl.objects[lastName].visual.nameIdx]));
		else
			strcpy(name, CL_ObjectType(cl.objects[lastName].visual.type)->description);
	}
	else if (IsBuildingType(cl.objects[lastName].visual.type) || 
			CL_ObjectType(cl.objects[lastName].visual.type)->objclass == OBJCLASS_ITEM)
	{
		strcpy(name, CL_ObjectType(cl.objects[lastName].visual.type)->description);
		level = 0;
	}

	//draw the strings
	if (namefont_strength > 0 && cl.objects[lastName].visual.active && cl.objects[lastName].visual.health)
	{
		bool drawhealth = false;
		objectData_t *def = CL_ObjectType(cl.objects[lastName].visual.type);

		if (def->drawName)
		{
			height = cl_namefontsize.integer;
			y = ((cl_mousepos_y.value/(float)corec.Vid_GetScreenH())*768) - (height*2) + cl_namefontadjust.integer - (cl_nameHealthHeight.integer/2);

			//show friendly/
			if (cl.info->team)
			{
				if (cl.info->team == cl.objects[lastName].visual.team)
					strcpy(teamstr, "Ally");
				else if (cl.objects[lastName].visual.team != 0)
					strcpy(teamstr, "Enemy");
			}
			else
			{
				strcpy(teamstr, fmt("Team %i", cl.objects[lastName].visual.team));			
			}

			if (cl.objects[lastName].visual.team != 0)
			{
				width = corec.GUI_StringWidth(teamstr, cl_namefontsize.integer, cl_nameiconsize.integer, 1, 1024, corec.GetNicefontShader());
				x = ((cl_mousepos_x.value/(float)corec.Vid_GetScreenW())*1024) - width/2;
				
				corec.GUI_DrawShadowedString(x, y, teamstr, height, cl_nameiconsize.integer, 1, 1024, corec.GetNicefontShader(), 1, 1, 1, namefont_strength);
			}


			y += height;
			width = corec.GUI_StringWidth(name, cl_namefontsize.integer, cl_nameiconsize.integer, 1, 1024, corec.GetNicefontShader());
			x = ((cl_mousepos_x.value/(float)corec.Vid_GetScreenW())*1024) - width/2;
			
			//name drop shadow
			
			if (cl_nameColors.integer && level)
			{
				int diff = level - cl.clients[cl.clientnum].score.level;

				if (diff <= 0)	//blue -> green
				{
					r = 0;
					g = (MAX(diff, -4)/-4.0);
					b = 1 - (MAX(diff, -4)/-4.0);
				}
				else	//yellow -> red
				{
					r = 1;
					g = 1 - (MIN(diff-1, 3)/3.0);
					b = 0;
				}
			}
			else
			{
				//corec.GUI_SetRGBA(1, 1, 1, namefont_strength);
				r = g = b = 1;
			}
			corec.GUI_DrawShadowedString(x, y, name, height, cl_nameiconsize.integer, 1, 1024, corec.GetNicefontShader(), r, g, b, namefont_strength);
		}
		
		//work out if we should draw the health bar
		if (def->drawHealth)
		{
			if (cl.objects[lastName].visual.team == cl.info->team ||
				def->objclass == OBJCLASS_BUILDING || cl_drawEnemyHealth.integer ||
				def->isVehicle || !cl.info->team)		
				drawhealth = true;
		}

		if (drawhealth)
		{
			//draw the health meter
			health = cl.objects[lastName].visual.health;
			fullhealth= cl.objects[lastName].visual.fullhealth;
			y += height;
			height = cl_nameHealthHeight.integer;
			width = cl_nameHealthWidth.integer;
			x = ((cl_mousepos_x.value/(float)corec.Vid_GetScreenW())*1024) - width/2;
			corec.GUI_SetRGBA(1, 0, 0, namefont_strength);
			corec.GUI_Quad2d_S(x, y, width, height, corec.GetWhiteShader());
			corec.GUI_SetRGBA(0, 0, 1, namefont_strength);
			corec.GUI_Quad2d_S(x, y, width * (health/(float)fullhealth), height, corec.GetWhiteShader());
		}

		//if it's a building under construction, draw a construction meter
		if (cl.objects[lastName].visual.flags & BASEOBJ_UNDER_CONSTRUCTION)
		{
			int barWidth = (width * ((100 - (float)cl.objects[lastName].visual.percentToComplete)) / 100);
	
			//draw black background
			corec.GUI_SetRGBA(0.0, 0.0, 0.0, namefont_strength * 0.5);
			corec.GUI_Quad2d_S(x, y + height + 2, width, cl_nameHealthHeight.value, corec.GetWhiteShader());
	
			//draw green foreground
			corec.GUI_SetRGBA(0.3, 1.0, 0.3, namefont_strength);
			corec.GUI_Quad2d_S(x, y + height + 2, barWidth, cl_nameHealthHeight.value, corec.GetWhiteShader());
		}
		
		//if the names aren't color coded, draw a string of the level
		if (!cl_nameColors.integer && level)
		{
			char lvl[10];
			
			if (level == 10)
				strcpy(lvl, "Level: 10");
			else
			{
				strcpy(lvl, "Level:   ");
				lvl[8] = level + '0';
			}
			
			y += height;
			height = cl_namefontsize.integer;
			width = corec.GUI_StringWidth(lvl, cl_namefontsize.integer, cl_nameiconsize.integer, 1, 1024, corec.GetNicefontShader());
			x = ((cl_mousepos_x.value/(float)corec.Vid_GetScreenW())*1024) - width/2;
			corec.GUI_DrawShadowedString(x, y, lvl, cl_namefontsize.integer, cl_nameiconsize.integer, 1, 1024, corec.GetNicefontShader(), 1, 1, 1, namefont_strength);
		}
	}
}


/*==========================

  Rend_DrawNames

  draw object names in commander view

  ==========================*/

void	Rend_DrawNames()
{
	int i;

	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (cl.objects[i].visual.active
			&& cl.objects[i].visual.team == cl.info->team
			&& !(cl.objects[i].visual.flags & BASEOBJ_NO_RENDER)
			&& (IsMobileType(cl.objects[i].visual.type) || IsBuildingType(cl.objects[i].visual.type)))
		{
			Rend_DrawNameHealth(&cl.objects[i]);
		}
	}
}


/*==========================

  Rend_DrawSelectionRect

  draw the selection rectangle for selecting units in commander view

 ==========================*/

void	Rend_DrawSelectionRect()
{
	vec4_t selcolor = { 1,1,0,0.8 };

	if (!cl.showSelectionRect)
		return;
	if (abs(cl.selectionRect_dim[X]) < 2 && abs(cl.selectionRect_dim[Y]) < 2)
		return;
	
	corec.Draw_SetColor(selcolor);
	TL_LineBox2d(cl.selectionRect_tl[X], cl.selectionRect_tl[Y],
					  cl.selectionRect_dim[X], cl.selectionRect_dim[Y],
					  2);
}

void	Rend_DrawSeigeIndicators()
{
	waypoint_t *waypoint = &cl.clients[cl.clientnum].waypoint;
	int i;
	float range, dist;

	if (waypoint->active)
	{
		i = waypoint->object_index;
		if (waypoint->object
			&& waypoint->fake_waypoint
			&& (!cl.objects[i].visual.active
				|| cl.objects[i].visual.flags & BASEOBJ_NO_RENDER				
				|| !CL_ObjectType(cl.objects[i].visual.type)->isVehicle))
		{
			waypoint->active = false;
		}
		return;
	}

	if (!cl.objects[cl.clientnum].visual.active
		|| cl.objects[cl.clientnum].visual.health <= 0
		|| CL_ObjectType(cl.objects[cl.clientnum].visual.type)->isVehicle)
		return;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		//eventually we'll need a field to check to see if this is an object that is "worth defending" 
		//but for now, we'll just use isVehicle
		if (!cl.objects[i].visual.active
			|| cl.objects[i].visual.team != cl.info->team
			|| cl.objects[i].visual.flags & BASEOBJ_NO_RENDER
			|| !CL_ObjectType(cl.objects[i].visual.type)->isVehicle)
			continue;
	
		range = cl_seigeAlertDistance.value * cl_seigeAlertDistance.value;
		dist = M_GetDistanceSq(cl.objects[i].visual.pos, cl.objects[cl.clientnum].visual.pos);
		if (dist < range && i != cl.clientnum)
		{
			waypoint->active = true;
			waypoint->object = true;
			waypoint->object_index = i;
			waypoint->object_type = cl.objects[i].visual.type;
			waypoint->fake_waypoint = true;

       		corec.GUI_Exec("fadein player_task_panel 500");
	        cl.ui_playerTaskPanel->pos[1] = -100;

			corec.Cvar_SetVar(&player_task, "Protect the siege weapon!");
			CL_PlayGoalSound(cl.teams[cl.info->team].commander, GOAL_DEFEND, waypoint->object_type);     
			
			return;
		}
	}
}

void	Rend_DrawElectricEyeBeams()
{
	sceneobj_t sc;
	int i, j;
	float range;

	CLEAR_SCENEOBJ(sc);

	sc.objtype = OBJTYPE_BILLBOARD;
	sc.width = 8;
	sc.height = 10000;
	sc.shader = res.lightbeamRedShader;
	sc.alpha = cl_electricEyeBeamFade.value;
	SET_VEC4(sc.color,	sc.alpha, sc.alpha, sc.alpha, sc.alpha);
	
	//check for objects applying the effect
	for (i = MAX_CLIENTS; i < MAX_OBJECTS; i++)
	{
		if (!cl.objects[i].visual.active
			|| cl.objects[i].visual.team != cl.info->team
			|| !CL_ObjectType(cl.objects[i].visual.type)->revealHidden)
			continue;
			
		range = CL_ObjectType(cl.objects[i].visual.type)->viewDistance;
		range = range * range;
		
		for (j = 0; j < MAX_CLIENTS; j++)
		{
			if (!cl.objects[j].visual.active
				|| cl.objects[j].visual.team == cl.info->team)
				continue;

			if (M_GetDistanceSq(cl.objects[j].visual.pos, cl.objects[i].visual.pos) < range)
			{
				M_CopyVec3(cl.objects[j].visual.pos, sc.pos);

				corec.Scene_AddObject(&sc);	
			}
		}
	}

	//check for player having the effect personally
	for (i = 0; i < MAX_STATE_SLOTS; i++)
	{
		int statenum = cl.objects[cl.clientnum].visual.states[i];
		if (!statenum)
			continue;

		if (!stateData[statenum].markEnemies)
			continue;

		range = CL_ObjectType(cl.objects[cl.clientnum].visual.type)->viewDistance;
		range = range * range;
		
		for (j = 0; j < MAX_CLIENTS; j++)
		{
			if (!cl.objects[j].visual.active
				|| cl.objects[j].visual.team == cl.info->team)
				continue;

			if (M_GetDistanceSq(cl.objects[j].visual.pos, cl.objects[cl.clientnum].visual.pos) < range)
			{
				M_CopyVec3(cl.objects[j].visual.pos, sc.pos);
				corec.Scene_AddObject(&sc);	
			}
		}
		break;	//only need to do it once
	}
}

void	Rend_DrawPlayerWaypointBeam()
{
	sceneobj_t sc;
	float dist;
	waypoint_t *waypoint = &cl.clients[cl.clientnum].waypoint;
	
	if (!waypoint->active)
		return;

	//draw lightbeam

	CLEAR_SCENEOBJ(sc);

	sc.objtype = OBJTYPE_BILLBOARD;
	sc.width = 50;
	sc.height = 10000;
	if (waypoint->goal == GOAL_ATTACK_OBJECT)
		sc.shader = res.lightbeamRedShader;
	else
		sc.shader = res.lightbeamShader;

	if (waypoint->object && cl.objects[waypoint->object_index].visual.pos)
	{
		M_CopyVec3(cl.objects[waypoint->object_index].visual.pos, sc.pos);
	}
	else if (waypoint->pos)
		M_CopyVec3(waypoint->pos, sc.pos);

	dist = M_GetDistanceSq(cl.objects[cl.clientnum].visual.pos, sc.pos);
	if (dist < cl_waypointFadeDist.value)
		sc.alpha = cl_waypointMinFade.value + (1 - cl_waypointMinFade.value) * (sqrt(dist) / sqrt(cl_waypointFadeDist.value));
	else
		sc.alpha = 1;

	SET_VEC4(sc.color,	sc.alpha, sc.alpha, sc.alpha, sc.alpha);

	corec.Scene_AddObject(&sc);	
}

void	Rend_DrawPlayerWaypointOverlay()
{
	float angle1,angle2,anglediff;
	vec2_t screenpos;
	int x,y;
	vec3_t waypos, diff;
	float dist;
	char distreading[10];
	int width;
	waypoint_t *waypoint = &cl.clients[cl.clientnum].waypoint;

	if (!waypoint->active)
		return;

	cl.ui_playerTaskPanel->pos[1] += MAX(cl.frametime * 100, 1);
	if (cl.ui_playerTaskPanel->pos[1] > 0)
		cl.ui_playerTaskPanel->pos[1] = 0;

	if (waypoint->object)
		M_CopyVec3(cl.objects[waypoint->object_index].visual.pos, waypos);
	else
		M_CopyVec3(waypoint->pos, waypos);

	M_SubVec3(waypos, cl.predictedState.pos, diff);

 	//compare XY component of view angle with XY component of the 
 	//vector going towards the waypoint to see if we should draw any 'helper' arrows
 	angle1 = M_GetVec2Angle(diff);	
 	angle2 = M_GetVec2Angle(cl.heading);
 	corec.GUI_Hide(cl.ui_rightarrow->element);
 	corec.GUI_Hide(cl.ui_leftarrow->element);
	anglediff = angle1 - angle2;
	if (anglediff < -180)
		anglediff += 360;
	else if (anglediff > 180)
		anglediff -= 360;
 	if (anglediff < -30)
 		corec.GUI_Show(cl.ui_rightarrow->element);
 	else if (anglediff > 30)
 		corec.GUI_Show(cl.ui_leftarrow->element);	
 
	//don't draw it if it's behind us
 	if (M_DotProduct(cl.camera.viewaxis[FORWARD], diff) < 0)
 		return;

	corec.Vid_ProjectVertex(&cl.camera, waypos, screenpos);
	
	x = screenpos[0];
	y = screenpos[1];
	corec.GUI_Coord(&x, &y);

	dist = M_GetVec3Length(diff) / 10;	//convert to 'feet'

	corec.Draw_SetColor(white);

	if (waypoint->goal == GOAL_ATTACK_OBJECT)	
		corec.GUI_Quad2d_S(x - 24, y - 24, 48, 48, res.waypointAttackIcon);
	else
		corec.GUI_Quad2d_S(x - 24, y - 24, 48, 48, res.waypointMoveIcon);

	corec.Draw_SetColor(black);

	strncpy(distreading, fmt("%.0f'", dist), 9);
	width = corec.GUI_StringWidth(distreading, 12, 12, 1, 9, corec.GetNicefontShader());
	corec.GUI_DrawString(x - width / 2 + 2, y-3, distreading, 12, 12, 1, 200, corec.GetNicefontShader());
	if (waypoint->goal == GOAL_ATTACK_OBJECT)
		corec.Draw_SetColor(red);
	else
		corec.Draw_SetColor(green);
	corec.GUI_DrawString(x - width / 2, y-5, distreading, 12, 12, 1, 200, corec.GetNicefontShader());
}

void    Rend_DrawCommanderWaypoints()
{
	sceneobj_t sc;
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (cl.clients[i].info.team != cl.info->team)
			continue;

		if (cl.clients[i].waypoint.active)
		{
			int goal = cl.clients[i].waypoint.goal;

			CLEAR_SCENEOBJ(sc);
			sc.objtype = OBJTYPE_BILLBOARD;
		
			sc.width = 10;
			sc.height = 20000;
						
			if (goal == GOAL_ATTACK_OBJECT)
				sc.shader = res.lightbeamRedShader;
			else
				sc.shader = res.lightbeamShader;

			sc.alpha = 1;

			SET_VEC4(sc.color, 1, 1, 1, 1);
			/*if (cl.selection.numSelected && cl.selection.array[0] == i)			
				SET_VEC4(sc.color, 1, 1, 1, 1);			
			else
				SET_VEC4(sc.color, 0.4, 0.4, 0.4, 0.4);*/

			if (cl.clients[i].waypoint.object)
			{
				M_CopyVec3(cl.objects[cl.clients[i].waypoint.object_index].visual.pos, sc.pos);
			}
			else
			{
				M_CopyVec3(cl.clients[i].waypoint.pos, sc.pos);
			}

			//corec.Console_DPrintf("Drawing waypoint at (%f, %f, %f)\n", sc.pos[X], sc.pos[Y], sc.pos[Z]);
//			corec.Scene_AddObject(&sc);

			sc.objtype = OBJTYPE_MODEL;

			sc.model = corec.Res_LoadModel("/models/waypointflag/waypointflag.model");

			switch(goal)
			{
				case GOAL_ATTACK_OBJECT:
					sc.skin = corec.Res_LoadSkin(sc.model, "enemy");
					break;
				default:
					sc.skin = corec.Res_LoadSkin(sc.model, "neutral");
					break;
			}			

			Rend_SurfaceDecal(sc.pos, 24, 24, corec.Res_LoadShader("/textures/icons/1_nl_waypoint_ground.tga"), goal == GOAL_ATTACK_OBJECT ? red : green);

			corec.Scene_AddObject(&sc);
		}
	}
}

float	Rend_SceneTime()
{
	if (cl_rendUseGameTime.integer)
		return (float)cl.gametime / 1000.0;
	else
		return (float)corec.Milliseconds() / 1000.0;
}

void	Rend_RenderCommanderView()
{
	int noblend;

	//clear black background for commander
	corec.Draw_SetColor(vec4(0,0,0,1));
	corec.Draw_Quad2d(0,0,cl.camera.width,cl.camera.height,0,0,1,1,corec.GetWhiteShader());

	Rend_DrawCommanderWaypoints();
	Rend_DrawSelectionIndicators();

	CL_RenderParticles();
	CL_RenderExParticles();
	CL_RenderBeams();

	//CL_RenderEnvironmentEffects();
	
	cl.camera.fog_near = cl_cmdr_fog_near.value;
	cl.camera.fog_far = cl_cmdr_fog_far.value;

	//always do alpha blending in commander mode, or world objects will either disappear or obstruct vision
	noblend = corec.Cvar_GetInteger("gfx_noBlending");
	if (noblend)
		corec.Cvar_SetValue("gfx_noBlending", 0);

	//render the scene
	corec.Scene_Render(&cl.camera, zero_vec);

	Rend_DrawGlowFilter();

	//restore noblending var
	if (noblend)
		corec.Cvar_SetValue("gfx_noBlending", noblend);

	Rend_DrawNames();

	Rend_DrawSelectionRect();
}


/*==========================

  Rend_ProximityMessages

  Checks for nearby objects to deliver a proximity message

 ==========================*/

void	Rend_ProximityMessages()
{
	static int lastframeobject = MAX_OBJECTS;
	static int lastmessagetime = 0;
	static int lastmessageobject = MAX_OBJECTS;
	int prev;
	objectData_t *obj;
	traceinfo_t	trace;

	prev = lastframeobject;
	lastframeobject = MAX_OBJECTS;

	//this should be the same on client and server (see sv_clients.c)
	corec.World_TraceBoxEx(&trace, cl.predictedState.pos, cl.predictedState.pos, vec3(-40,-40,-40), vec3(40,40,40),  SURF_DYNAMIC|SURF_TERRAIN|SURF_STATIC, cl.clientnum);

	if (trace.fraction == 1.0)
		return;
	if (trace.index < 0 || trace.index >= MAX_OBJECTS)
		return;
	else
		obj = CL_ObjectType(cl.objects[trace.index].visual.type);

	if (cl.objects[trace.index].visual.team != 0 
		&& cl.objects[trace.index].visual.team != cl.info->team)
		return;

	if (prev != trace.index)
	{
		if (lastmessageobject != trace.index || cl.gametime - lastmessagetime >= cl_proxMessageTime.integer)
		{
			corec.Cmd_Exec(fmt("set _proximityMessage \"%s\"", obj->proximityMessage));
			if (!stricmp(obj->proximitySound, "default"))
				CL_NotifyMessage(_proximityMessage.string, NULL);
			else
				CL_NotifyMessage(_proximityMessage.string, obj->proximitySound);
			lastmessagetime = cl.gametime;
			lastmessageobject = trace.index;
		}
		lastframeobject = trace.index;
	}
	else
	{
		lastframeobject = trace.index;
	}

}

extern cvar_t cl_freeMouse;

void	Rend_RenderPlayerView()
{
	TL_ClearBackground();
	TL_DrawSky(&cl.camera);

	Rend_DrawSeigeIndicators();
	Rend_DrawPlayerWaypointBeam();
	Rend_DrawElectricEyeBeams();

	CL_RenderParticles();
	CL_RenderExParticles();
	CL_RenderBeams();

	CL_RenderEnvironmentEffects();
	
	cl.camera.fog_near = cl.camera.fog_far = 0;

	//draw the 3d view	
	corec.Scene_Render(&cl.camera, cl.objects[cl.clientnum].visual.pos);

	//draw stuff on top of the rendered view
	Rend_DrawPlayerWaypointOverlay();
	
	TL_DrawSunRays(&cl.camera);

	Rend_RenderFPSWeapon();

	Rend_DrawGlowFilter();

	Rend_DrawPlayerCrosshair();

	if (cl.effects.overlayEffect)
	{
		corec.Draw_SetColor(cl.effects.overlayColor);
		corec.Draw_Quad2d(0,0, cl.camera.width, cl.camera.height, 0, 0, 1, 1, corec.GetWhiteShader());
	}

#ifdef SAVAGE_DEMO
	if ((cl.predictedState.respawnTime > cl.gametime + 30000))
	{
		corec.Cvar_SetVarValue(&cl_freeMouse, 1);
		corec.GUI_Exec("show demo_respawn_nag\n");
		if (!cl.showingNag)
		{
			corec.GUI_Exec("fadein demo_respawn_nag\n");
			cl.showingNag = true;
		}
	}
	else if (cl.showingNag)
	{		
		corec.Cvar_SetVarValue(&cl_freeMouse, 0);
		corec.GUI_Exec("fadeout demo_respawn_nag\n");
		cl.showingNag = false;
	}
#endif

	Rend_DrawPlayerName();
	Rend_ProximityMessages();
}


void	Rend_Render()
{
	PERF_BEGIN;

	if (cl.isCommander && !cl.winStatus)
		Rend_RenderCommanderView();
	else
		Rend_RenderPlayerView();	
	
	if (cl_showfps.integer)
		CL_DrawFPS();
	
	PERF_END(PERF_RENDER);
}

//draws the rotating model of the current selection in commnander view
void	Rend_Draw3dUnitWindow(byte objecttype, int objectIndex, ivec2_t pos, ivec2_t size)
{
/*	sceneobj_t sc;
	objectData_t *obj = CL_ObjectType(objecttype);	
	camera_t cam;
	float nearclip;	
	vec3_t bmin,bmax;

	corec.Scene_Clear();
	CLEAR_SCENEOBJ(sc);

	sc.model = CL_Model(objecttype);
	sc.skin = CL_Skin(objecttype, cl.info->team);
	sc.scale = obj->scale;
	//sc.flags = SCENEOBJ_SOLID_COLOR;
	//M_CopyVec4(vec4(1,0.9,0.7,1), sc.color);

	sc.skeleton = &cl.objects[objectIndex].skeleton;		//put the model in the same pose as the actual object

	corec.Res_GetModelVisualBounds(sc.model, bmin, bmax);
	//dist += MAX(MAX(MAX(bmax[X], bmax[Y]), -bmin[X]), -bmin[Y]);
	//sc.pos[Z] = -(bmax[Z] - bmin[Z])/4;	
	//sc.angle[2] = cl.gametime / 50 % 360;

	//M_CopyVec3(obj->selectionPos, sc.pos);
	sc.pos[Z] -= (bmax[2] - bmin[2]) / 2;		//center it
	//M_CopyVec3(od->selectionAngle, sc.angle);

	corec.Scene_AddObject(&sc);
			
	Cam_DefaultCamera(&cam, size[X], size[Y]);
	cam.x = pos[X];
	cam.y = pos[Y];
	cam.flags = CAM_NO_WORLD;// | CAM_NO_LIGHTING;
	cam.fovx = 30;
	//Cam_SetAngles(&cam, vec3(obj->selectionAngle[0], obj->selectionAngle[1], obj->selectionAngle[2] + 180));	
	Cam_SetDistance(&cam, obj->selectionDist);
	Cam_CalcFovy(&cam);

	nearclip = corec.Cvar_GetValue("gfx_nearclip");
	corec.Cvar_SetValue("gfx_nearclip", 0.1);

	corec.Scene_Render(&cam, zero_vec);

	corec.Cvar_SetValue("gfx_nearclip", nearclip);*/
}

void	Rend_ResetFogOfWar()
{	
	

	if (cl.fogCleared)
	{
		bvec4_t col;

		col[0] = cl_fowExplored_r.integer;
		col[1] = cl_fowExplored_g.integer;
		col[2] = cl_fowExplored_b.integer;
		col[3] = cl_fowExplored_a.integer;
	
		//clear to this color only if the color at that point is brighter
		corec.WR_ClearDynamapToColorEx(col, col);
	}
	else
	{
		bvec4_t fogColor = { cl_fowUnexplored_r.integer, cl_fowUnexplored_g.integer, cl_fowUnexplored_b.integer, cl_fowUnexplored_a.integer };
		corec.WR_ClearDynamapToColor(fogColor);
		cl.fogCleared = true;
	}
}

void	Rend_Blip_Cmd(int argc, char *argv[])
{
	if (argc<5)
		return;
	Rend_MapBlip(atoi(argv[0]),atoi(argv[1]),atoi(argv[2]),atoi(argv[3]),corec.Res_LoadShaderEx(argv[4], SHD_FULL_QUALITY | SHD_NO_MIPMAPS), 1500);
}

void	Rend_Init()
{
	corec.Cvar_Register(&cl_shadowType);
	corec.Cvar_Register(&cl_showWeapon);
	corec.Cvar_Register(&cl_handoffset_x);
	corec.Cvar_Register(&cl_handoffset_y);
	corec.Cvar_Register(&cl_handoffset_z);
	corec.Cvar_Register(&cl_handoffset_fov);
	corec.Cvar_Register(&cl_showfps);
	corec.Cvar_Register(&cl_waypoint_width);
	corec.Cvar_Register(&cl_waypoint_height);
	corec.Cvar_Register(&cl_drawmockup);
	corec.Cvar_Register(&cl_interfaceBrightness);
	corec.Cvar_Register(&cl_shadowr);
	corec.Cvar_Register(&cl_shadowg);
	corec.Cvar_Register(&cl_shadowb);
	corec.Cvar_Register(&cl_shadowalpha);
	corec.Cvar_Register(&cl_shadowscalex);
	corec.Cvar_Register(&cl_shadowscaley);
	corec.Cvar_Register(&cl_throbRate);
	corec.Cvar_Register(&cl_healthMeterWidthScale);
	corec.Cvar_Register(&cl_healthMeterHeight);
	corec.Cvar_Register(&cl_healthMeterOffset);
	corec.Cvar_Register(&cl_friendlyIconOffset);
	corec.Cvar_Register(&cl_friendlyIconWidth);
	corec.Cvar_Register(&cl_friendlyIconHeight);
	corec.Cvar_Register(&cl_voiceIconOffset);
	corec.Cvar_Register(&cl_fov);
	corec.Cvar_Register(&cl_healthXPos);
	corec.Cvar_Register(&cl_healthYPos);
	corec.Cvar_Register(&cl_moneyXPos);
	corec.Cvar_Register(&cl_moneyYPos);
	corec.Cvar_Register(&cl_namefontsize);
	corec.Cvar_Register(&cl_nameiconsize);
	corec.Cvar_Register(&cl_namefontadjust);
	corec.Cvar_Register(&cl_sprintXPos);
	corec.Cvar_Register(&cl_sprintYPos);
	corec.Cvar_Register(&cl_weapbobamount);
	corec.Cvar_Register(&cl_weapbobspeed);
	corec.Cvar_Register(&cl_charBrightnessFactor);
	corec.Cvar_Register(&cl_handJitterScale);
	corec.Cvar_Register(&cl_proxMessageTime);
	
	corec.Cvar_Register(&cl_fowFalloff);
	corec.Cvar_Register(&cl_fowUnexplored_r);
	corec.Cvar_Register(&cl_fowUnexplored_g);
	corec.Cvar_Register(&cl_fowUnexplored_b);
	corec.Cvar_Register(&cl_fowUnexplored_a);
	corec.Cvar_Register(&cl_fowExplored_r);
	corec.Cvar_Register(&cl_fowExplored_g);
	corec.Cvar_Register(&cl_fowExplored_b);
	corec.Cvar_Register(&cl_fowExplored_a);
	corec.Cvar_Register(&cl_notvis_alpha);

	corec.Cvar_Register(&cl_nameFadeRate);
	corec.Cvar_Register(&cl_nameColors);
	corec.Cvar_Register(&cl_nameHealthHeight);

	corec.Cvar_Register(&cl_shadowrangelo);
	corec.Cvar_Register(&cl_shadowrangehi);

	corec.Cvar_Register(&cl_cmdr_resourcelistx);
	corec.Cvar_Register(&cl_cmdr_resourcelisty);
	corec.Cvar_Register(&cl_cmdr_resourceNameHeight);
	corec.Cvar_Register(&cl_cmdr_fog_near);
	corec.Cvar_Register(&cl_cmdr_fog_far);
	corec.Cvar_Register(&cl_nameHealthWidth);

	corec.Cvar_Register(&cl_showLevelArmor);

	corec.Cvar_Register(&cl_rendUseGameTime);	

	corec.Cvar_Register(&cl_showObjectInfo);
	corec.Cvar_Register(&cl_targetedObject);
	corec.Cvar_Register(&cl_targetedTerrainX);
	corec.Cvar_Register(&cl_targetedTerrainY);
	corec.Cvar_Register(&cl_targetedTerrainZ);

	corec.Cvar_Register(&cl_drawPlayerName);

	corec.Cvar_Register(&cl_waypointFadeDist);
	corec.Cvar_Register(&cl_waypointMinFade);
	corec.Cvar_Register(&cl_electricEyeBeamFade);
 
	corec.Cvar_Register(&cl_seigeAlertDistance);
	
	corec.Cvar_Register(&cl_showCommanderSelection);
	corec.Cvar_Register(&cl_cloackAdjustRate);

	corec.Cvar_Register(&cl_firstPersonIndicatorScale);

	corec.Cvar_Register(&cl_drawEnemyHealth);

	corec.Cvar_Register(&cl_constructionShader);
	corec.Cvar_Register(&_proximityMessage);

	corec.Cvar_Register(&cl_drawTrailSegmentOnDeath);

	corec.Cvar_Register(&cl_debugMuzzle);
	corec.Cvar_Register(&cl_muzzleSnap);

	corec.Cvar_Register(&cl_healthStageMultiplier);

	corec.Cvar_Register(&cl_debugFlyby);

	corec.Cvar_Register(&cl_glowFilter);
	corec.Cvar_Register(&cl_commanderGlowShader);
	corec.Cvar_Register(&cl_playerGlowShader);

	corec.Cmd_Register("blip", Rend_Blip_Cmd);
}
