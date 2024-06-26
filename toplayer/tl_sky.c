// (C) 2003 S2 Games

// tl_sky.c

// skybox code

#include "tl_shared.h"

#define	RES_ENVIRONMENT_PATH	"/props/enviro/"


cvar_t	tl_skyskin = { "tl_skyskin", "peach", CVAR_WORLDCONFIG };
cvar_t	tl_windspeed = { "tl_windspeed", "1.8", CVAR_WORLDCONFIG };
cvar_t	tl_skyoffset = { "tl_skyoffset", "10", CVAR_WORLDCONFIG };
cvar_t	tl_windspeed2 = { "tl_windspeed2", "0.9", CVAR_WORLDCONFIG };
cvar_t	tl_skytopoffset = { "tl_skytopoffset", "10", CVAR_WORLDCONFIG };
cvar_t	tl_drawsky = { "tl_drawsky", "1", CVAR_SAVECONFIG };
cvar_t	tl_skyr = { "tl_skyr", "1", CVAR_WORLDCONFIG };
cvar_t	tl_skyg = { "tl_skyg", "1", CVAR_WORLDCONFIG };
cvar_t	tl_skyb = { "tl_skyb", "1", CVAR_WORLDCONFIG };
cvar_t	tl_skya = { "tl_skya", "1", CVAR_WORLDCONFIG };

cvar_t	tl_backgroundr = { "tl_backgroundr", "0.4", CVAR_WORLDCONFIG };
cvar_t	tl_backgroundg = { "tl_backgroundg", "0.4", CVAR_WORLDCONFIG };
cvar_t	tl_backgroundb = { "tl_backgroundb", "1", CVAR_WORLDCONFIG };

cvar_t	tl_sunskin = { "tl_sunskin", "default", CVAR_WORLDCONFIG };
cvar_t	tl_sunmodel = { "tl_sunmodel", "e_sun", CVAR_WORLDCONFIG };
cvar_t	tl_sunr = { "tl_sunr", "1", CVAR_WORLDCONFIG };
cvar_t	tl_sung = { "tl_sung", "1", CVAR_WORLDCONFIG };
cvar_t	tl_sunb = { "tl_sunb", "1", CVAR_WORLDCONFIG };
cvar_t	tl_suna = { "tl_suna", "1", CVAR_WORLDCONFIG };
cvar_t	tl_sunscale = { "tl_sunscale", "2000", CVAR_WORLDCONFIG };
cvar_t	tl_drawsun = { "tl_drawsun", "1", CVAR_WORLDCONFIG };
cvar_t	tl_drawSunOption = { "tl_drawSunOption", "1", CVAR_SAVECONFIG };
cvar_t	tl_drawsunrays = { "tl_drawsunrays", "1", CVAR_WORLDCONFIG };
cvar_t	tl_sunaxis = {"tl_sunaxis", "1", CVAR_WORLDCONFIG};
cvar_t	tl_sunwidth = {"tl_sunwidth", "1150", CVAR_WORLDCONFIG};
cvar_t	tl_sunheight = {"tl_sunheight", "1150", CVAR_WORLDCONFIG};


cvar_t	tl_moonskin = { "tl_moonskin", "default", CVAR_WORLDCONFIG};
cvar_t	tl_moonmodel = { "tl_moonmodel", "e_moon", CVAR_WORLDCONFIG };
cvar_t	tl_moonr = { "tl_moonr", "1", CVAR_WORLDCONFIG };
cvar_t	tl_moong = { "tl_moong", "1", CVAR_WORLDCONFIG };
cvar_t	tl_moonb = { "tl_moonb", "1", CVAR_WORLDCONFIG };
cvar_t	tl_moona = { "tl_moona", "1", CVAR_WORLDCONFIG };
cvar_t	tl_moonscale = { "tl_moonscale", "50", CVAR_WORLDCONFIG };
cvar_t	tl_drawmoon = { "tl_drawmoon", "0" };
cvar_t	tl_drawflares = { "tl_drawflares", "1" };

cvar_t	tl_flare1_type = { "tl_flare1_type", "1", CVAR_WORLDCONFIG};
cvar_t	tl_flare1_scale = { "tl_flare1_scale", "0.5", CVAR_WORLDCONFIG};
cvar_t	tl_flare1_pos = { "tl_flare1_pos", "0.5", CVAR_WORLDCONFIG};
cvar_t	tl_flare1_active = { "tl_flare1_active", "1", CVAR_WORLDCONFIG};
cvar_t	tl_flare2_type = { "tl_flare2_type", "0", CVAR_WORLDCONFIG};
cvar_t	tl_flare2_scale = { "tl_flare2_scale", "0.25", CVAR_WORLDCONFIG};
cvar_t	tl_flare2_pos = { "tl_flare2_pos", "0.33", CVAR_WORLDCONFIG};
cvar_t	tl_flare2_active = { "tl_flare2_active", "1", CVAR_WORLDCONFIG};
cvar_t	tl_flare3_type = { "tl_flare3_type", "1", CVAR_WORLDCONFIG};
cvar_t	tl_flare3_scale = { "tl_flare3_scale", "1.0", CVAR_WORLDCONFIG};
cvar_t	tl_flare3_pos = { "tl_flare3_pos", "0.125", CVAR_WORLDCONFIG};
cvar_t	tl_flare3_active = { "tl_flare3_active", "1", CVAR_WORLDCONFIG};
cvar_t	tl_flare4_type = { "tl_flare4_type", "0", CVAR_WORLDCONFIG};
cvar_t	tl_flare4_scale = { "tl_flare4_scale", "0.5", CVAR_WORLDCONFIG};
cvar_t	tl_flare4_pos = { "tl_flare4_pos", "-0.5", CVAR_WORLDCONFIG};
cvar_t	tl_flare4_active = { "tl_flare4_active", "1", CVAR_WORLDCONFIG};
cvar_t	tl_flare5_type = { "tl_flare5_type", "1", CVAR_WORLDCONFIG};
cvar_t	tl_flare5_scale = { "tl_flare5_scale", "0.25", CVAR_WORLDCONFIG};
cvar_t	tl_flare5_pos = { "tl_flare5_pos", "-0.25", CVAR_WORLDCONFIG};
cvar_t	tl_flare5_active = { "tl_flare5_active", "1", CVAR_WORLDCONFIG};
cvar_t	tl_flare6_type = { "tl_flare6_type", "0", CVAR_WORLDCONFIG};
cvar_t	tl_flare6_scale = { "tl_flare6_scale", "0.25", CVAR_WORLDCONFIG};
cvar_t	tl_flare6_pos = { "tl_flare6_pos", "-0.18", CVAR_WORLDCONFIG};
cvar_t	tl_flare6_active = { "tl_flare6_active", "1", CVAR_WORLDCONFIG };

sceneobj_t sun;

#define NUM_SUN_FLARES 2

residx_t sun_glow_shader;
residx_t sun_star_shader;
residx_t sun_streak_shader;
residx_t sun_flare_shaders[NUM_SUN_FLARES];

residx_t sky_model;
residx_t sky_top_model;



void	TL_DrawSun(camera_t *cam, sceneobj_t *sun)
{
	vec4_t suncolor;

	if (!tl_drawsky.integer)
		return;
	if (!tl_drawsun.integer)
		return;
	if (!tl_drawSunOption.integer)
		return;

	suncolor[0] = tl_sunr.value;
	suncolor[1] = tl_sung.value;
	suncolor[2] = tl_sunb.value;
	suncolor[3] = tl_suna.value;

	sun->shader = corec.Res_LoadShader("/textures/enviro/sun/1_nl_sun.tga");	
	sun->loframe = 0;
	sun->lerp_amt = 0;
	sun->objtype = OBJTYPE_BILLBOARD;
	sun->width = 1000;
	sun->height = 1000;
	sun->scale = 1;
	sun->flags = SCENEOBJ_BILLBOARD_ALL_AXES;		//always rotate towards the viewer
	M_CopyVec4(suncolor, sun->color);

	SET_VEC3(sun->angle, 0, 0, 0);
	sun->pos[0] = cam->origin[0] - tl_sunscale.value * corec.Cvar_GetValue("wr_sun_x");
	sun->pos[1] = cam->origin[1] - tl_sunscale.value * corec.Cvar_GetValue("wr_sun_y");
	sun->pos[2] = cam->origin[2] - tl_sunscale.value * corec.Cvar_GetValue("wr_sun_z");

	corec.Scene_AddSkyObj(sun);
}

extern cvar_t tl_sunwidth;
extern cvar_t tl_sunheight;

void	TL_DrawSunFlare(int flare, vec2_t center, vec2_t sun_vector, float length, float scale, float pos)
{
	corec.Draw_Quad2d(
		center[0] + sun_vector[0] * length*pos - (tl_sunwidth.value*scale/2), 
		center[1] + sun_vector[1] * length*pos - (tl_sunheight.value*scale/2), 
		tl_sunwidth.value*scale, tl_sunheight.value*scale, 
		0, 0, 1, 1, sun_flare_shaders[flare]);

}

void	TL_DrawSunFlares(camera_t *cam, vec2_t sun, vec2_t center, vec2_t sun_vector, float length)
{
	float flare_color[4];


	if (!tl_drawsky.integer)
		return;
	if (!tl_drawsun.integer)
		return;
	if (!tl_drawSunOption.integer)
		return;
	if (!tl_drawflares.value)
		return;

	flare_color[0] = flare_color[1] = flare_color[2] = 0.5;
	//Console_Printf("length=%f\n",length);
    //flare_color[3] = 1/length;

	corec.Draw_SetColor(flare_color);


	//TL_DrawSunFlare(0, center, sun_vector, length, 1.0,  1.0);
	if (tl_flare1_active.integer)
		TL_DrawSunFlare(tl_flare1_type.integer, center, sun_vector, length, tl_flare1_scale.value,  tl_flare1_pos.value);
	if (tl_flare2_active.integer)
		TL_DrawSunFlare(tl_flare2_type.integer, center, sun_vector, length, tl_flare2_scale.value,  tl_flare2_pos.value);
	if (tl_flare3_active.integer)
		TL_DrawSunFlare(tl_flare3_type.integer, center, sun_vector, length, tl_flare3_scale.value,  tl_flare3_pos.value);
	if (tl_flare4_active.integer)
		TL_DrawSunFlare(tl_flare4_type.integer, center, sun_vector, length, tl_flare4_scale.value,  tl_flare4_pos.value);
	if (tl_flare5_active.integer)
		TL_DrawSunFlare(tl_flare5_type.integer, center, sun_vector, length, tl_flare5_scale.value,  tl_flare5_pos.value);
	if (tl_flare6_active.integer)
		TL_DrawSunFlare(tl_flare6_type.integer, center, sun_vector, length, tl_flare6_scale.value,  tl_flare6_pos.value);
}

void	TL_DrawSunRays(camera_t *cam)
{
	vec2_t sun_screen_coords;
	vec3_t ray;
	int width, height;
	vec2_t sun_vector;
	vec2_t center;
	vec4_t white;
	vec4_t relative_sun;
	float inv_length, length, dotprod;

	if (!tl_drawsky.integer)
		return;
	if (!tl_drawsun.integer)
		return;
	if (!tl_drawSunOption.integer)
		return;
	if (!tl_drawsunrays.integer)
		return;

	white[0] = white[1] = white[2] = white[3] = 0.5;

	corec.Vid_ProjectVertex(cam, sun.pos, sun_screen_coords);

	M_SubVec3(sun.pos, cam->origin, relative_sun);	
	dotprod = M_DotProduct(relative_sun, cam->viewaxis[FORWARD]);

	if (
		dotprod > 0
		&& sun_screen_coords[0] > 0 - tl_sunwidth.value*0.2 
		&& sun_screen_coords[0] < corec.Vid_GetScreenW() + tl_sunwidth.value*0.2 
		&& sun_screen_coords[1] > 0 - tl_sunheight.value*0.2 
		&& sun_screen_coords[1] < corec.Vid_GetScreenH() + tl_sunheight.value*0.2)
	{
		float occluded;

		M_SubVec3(sun.pos, cam->origin, ray);

		length = M_GetVec3Length(ray);

		corec.Vid_ReadZBuffer(sun_screen_coords[0], sun_screen_coords[1], 1, 1, &occluded);
		occluded = 1.0 - occluded;

		//-- setup for the flares

		center[0] = corec.Vid_GetScreenW()*0.5;
		center[1] = corec.Vid_GetScreenH()*0.5;

		sun_vector[0] = center[0] - sun_screen_coords[0];
		sun_vector[1] = center[1] - sun_screen_coords[1];
		length = sqrt(sun_vector[0]*sun_vector[0] + sun_vector[1]*sun_vector[1]);
		inv_length = 1.0 / length;

		sun_vector[0] *= inv_length;
		sun_vector[1] *= inv_length;

		width = tl_sunwidth.value - length*2;
		height = tl_sunheight.value - length*2;

		if (!occluded)
		{
			corec.Draw_SetColor(white);

			corec.Draw_Quad2d(
				sun_screen_coords[0] - (width/2), 
				sun_screen_coords[1] - (height/2), 
				width, height,
				0, 0, 1, 1, sun_glow_shader);

			corec.Draw_Quad2d(
				sun_screen_coords[0] - (width/2), 
				sun_screen_coords[1] - (height/2), 
				width, height,
				0, 0, 1, 1, sun_star_shader);

			/*corec.Draw_Quad2d(
				sun_screen_coords[0] - (width/2), 
				sun_screen_coords[1] - (height/2), 
				width, height,
				0, 0, 1, 1, sun_streak_shader);*/

			TL_DrawSunFlares(cam, sun_screen_coords, center, sun_vector, length);
		}

		/*for (f = 0; f < 180; f += 60)
		{
			corec.Draw_RotatedQuad2d(
				sun_screen_coords[0] - (width/2), 
				sun_screen_coords[1] - (height/2), 
				width, height,
				f,
				0, 0, 1, 1, sun_streak_shader);

			corec.Draw_RotatedQuad2d(
				sun_screen_coords[0] - (width/2), 
				sun_screen_coords[1] - (height/2), 
				width, height,
				-f,
				0, 0, 1, 1, sun_streak_shader);
		}*/		
	}
}

void	TL_DrawMoon(camera_t *cam)
{
	char moonmodel[256];
	char moonskin[256];
	vec4_t mooncolor;
	sceneobj_t moon;

	if (!tl_drawmoon.value)
		return;

	BPrintf(moonmodel, 255, RES_ENVIRONMENT_PATH "moon/%s.model", tl_moonmodel.string);
	moonmodel[255] = 0;
	BPrintf(moonskin, 255, RES_ENVIRONMENT_PATH "moon/%s.skin", tl_moonskin.string);
	moonskin[255] = 0;

	mooncolor[0] = tl_moonr.value;
	mooncolor[1] = tl_moong.value;
	mooncolor[2] = tl_moonb.value;
	mooncolor[3] = tl_moona.value;

	moon.model = corec.Res_LoadModel(moonmodel);
	moon.skin = corec.Res_LoadSkin(moon.model, moonskin);
	moon.loframe = 0;
	moon.lerp_amt = 0;
	moon.objtype = OBJTYPE_MODEL;
	moon.scale = 1;
	moon.flags = 0;
	M_CopyVec4(mooncolor, moon.color);

	SET_VEC3(moon.angle, 0, 0, 0);
	moon.pos[0] = cam->origin[0] - (1/tl_moonscale.value) * corec.Cvar_GetValue("wr_sun_x");
	moon.pos[1] = cam->origin[1] - (1/tl_moonscale.value) * corec.Cvar_GetValue("wr_sun_y");
	moon.pos[2] = cam->origin[2] - (1/tl_moonscale.value) * corec.Cvar_GetValue("wr_sun_z");

	corec.Scene_AddSkyObj(&moon);
}

void	TL_DrawSky (camera_t *cam)
{	
	sceneobj_t sky, skytop;
	static float sky_angle = 0;
	static float skytop_angle = 0;
	vec4_t skycolor;

	if (!tl_drawsky.value)		
		return;

	CLEAR_SCENEOBJ(sky);
	CLEAR_SCENEOBJ(skytop);
	
	skycolor[0] = tl_skyr.value;
	skycolor[1] = tl_skyg.value;
	skycolor[2] = tl_skyb.value;
	skycolor[3] = tl_skya.value;

	sky.model = sky_model;
	sky.skin = corec.Res_LoadSkin(sky.model, tl_skyskin.string);
	sky.loframe = 0; //modelframe / (30.0 / corec.Res_GetModelKPS(sky.model));
	sky.lerp_amt = 0;
	sky.objtype = OBJTYPE_MODEL;	
	sky.scale = 1;
	sky.flags = 0;
	M_CopyVec4(skycolor, sky.color);

	skytop.model = sky_top_model;
	skytop.skin = corec.Res_LoadSkin(skytop.model, tl_skyskin.string);
	skytop.loframe = 0; //modelframe / (30.0 / corec.Res_GetModelKPS(skytop.model));
	skytop.lerp_amt = 0;
	skytop.objtype = OBJTYPE_MODEL;	
	skytop.scale = 1;
	skytop.flags = 0;
	M_CopyVec4(skycolor, skytop.color);

	sky_angle += tl_windspeed.value / (1.0 / corec.FrameSeconds());
	skytop_angle += tl_windspeed2.value / (1.0 / corec.FrameSeconds());
	if (sky_angle > 360) sky_angle -= 360;
	if (skytop_angle > 360) skytop_angle -= 360;

	SET_VEC3(sky.angle, 0, 0, sky_angle);
	SET_VEC3(sky.pos, cam->origin[0], cam->origin[1], cam->origin[2] - tl_skyoffset.value);
	SET_VEC3(skytop.angle, 0, 0, skytop_angle);
	SET_VEC3(skytop.pos, cam->origin[0], cam->origin[1], cam->origin[2] - tl_skytopoffset.value);

	//must pass in front to back order
	TL_DrawSun(cam, &sun);
	TL_DrawMoon(cam);

	corec.Scene_AddSkyObj(&skytop);
	corec.Scene_AddSkyObj(&sky);
}

void TL_ClearBackground()
{
	vec4_t bgcolor;
	
	if (tl_drawsky.integer)
		SET_VEC4(bgcolor, tl_backgroundr.value,tl_backgroundg.value,tl_backgroundb.value,1);
	else
		SET_VEC4(bgcolor, tl_skyr.value,tl_skyg.value,tl_skyb.value,1);

	corec.Draw_SetColor(bgcolor);
	
	corec.Draw_Quad2d(0, 0, corec.Vid_GetScreenW(), corec.Vid_GetScreenH(), 0, 0, 1, 1, corec.GetWhiteShader());
}



// SetTimeOfDay(minute)
// This is the main function.  You pass in the minute, and it will 
// set up all the variables to match that time of day.
void TL_SetTimeOfDay(float minute)
{
	float phi, theta;
	float x, y, z;

	phi = minute * M_PI / 720;
	theta = tl_sunaxis.value;

	x = sin (phi) * cos (theta);
	y = sin (phi) * sin (theta);
	z = cos (phi);

	corec.Cvar_SetValue("wr_sun_x", x);
	corec.Cvar_SetValue("wr_sun_y", y);
	corec.Cvar_SetValue("wr_sun_z", z);

	corec.SetTimeOfDay(minute);
}


void	TL_Register()
{
	corec.Cvar_Register(&tl_skyskin);
	corec.Cvar_Register(&tl_windspeed);
	corec.Cvar_Register(&tl_windspeed2);
	corec.Cvar_Register(&tl_skyoffset);
	corec.Cvar_Register(&tl_skytopoffset);
	corec.Cvar_Register(&tl_drawsky);
	corec.Cvar_Register(&tl_skyr);
	corec.Cvar_Register(&tl_skyg);
	corec.Cvar_Register(&tl_skyb);
	corec.Cvar_Register(&tl_skya);

	corec.Cvar_Register(&tl_drawflares);
	corec.Cvar_Register(&tl_sunmodel);
	corec.Cvar_Register(&tl_drawsun);
	corec.Cvar_Register(&tl_drawsunrays);
	corec.Cvar_Register(&tl_sunr);
	corec.Cvar_Register(&tl_sung);
	corec.Cvar_Register(&tl_sunb);
	corec.Cvar_Register(&tl_suna);
	corec.Cvar_Register(&tl_sunscale);
	corec.Cvar_Register(&tl_sunaxis);
	corec.Cvar_Register(&tl_sunwidth);
	corec.Cvar_Register(&tl_sunheight);
	corec.Cvar_Register(&tl_drawSunOption);


	corec.Cvar_Register(&tl_moonmodel);
	corec.Cvar_Register(&tl_drawmoon);
	corec.Cvar_Register(&tl_moonr);
	corec.Cvar_Register(&tl_moong);
	corec.Cvar_Register(&tl_moonb);
	corec.Cvar_Register(&tl_moona);
	corec.Cvar_Register(&tl_moonscale);

	corec.Cvar_Register(&tl_backgroundr);
	corec.Cvar_Register(&tl_backgroundg);
	corec.Cvar_Register(&tl_backgroundb);

	corec.Cvar_Register(&tl_flare1_type);
	corec.Cvar_Register(&tl_flare1_scale);
	corec.Cvar_Register(&tl_flare1_pos);
	corec.Cvar_Register(&tl_flare1_active);
	corec.Cvar_Register(&tl_flare2_type);
	corec.Cvar_Register(&tl_flare2_scale);
	corec.Cvar_Register(&tl_flare2_pos);
	corec.Cvar_Register(&tl_flare2_active);
	corec.Cvar_Register(&tl_flare3_type);
	corec.Cvar_Register(&tl_flare3_scale);
	corec.Cvar_Register(&tl_flare3_pos);
	corec.Cvar_Register(&tl_flare3_active);
	corec.Cvar_Register(&tl_flare4_type);
	corec.Cvar_Register(&tl_flare4_scale);
	corec.Cvar_Register(&tl_flare4_pos);
	corec.Cvar_Register(&tl_flare4_active);
	corec.Cvar_Register(&tl_flare5_type );
	corec.Cvar_Register(&tl_flare5_scale );
	corec.Cvar_Register(&tl_flare5_pos);
	corec.Cvar_Register(&tl_flare5_active);
	corec.Cvar_Register(&tl_flare6_type);
	corec.Cvar_Register(&tl_flare6_scale);
	corec.Cvar_Register(&tl_flare6_pos);
	corec.Cvar_Register(&tl_flare6_active);
}

void	TL_InitSky()
{
	sun_glow_shader = corec.Res_LoadShader("/textures/enviro/sun/1_nl_glow.tga");
	sun_star_shader = corec.Res_LoadShader("/textures/enviro/sun/1_nl_star.tga");
	sun_streak_shader = corec.Res_LoadShader("/textures/enviro/sun/1_nl_streak.tga");
	sun_flare_shaders[0] = corec.Res_LoadShader("/textures/enviro/sun/1_nl_secondary_1.tga");
	sun_flare_shaders[1] = corec.Res_LoadShader("/textures/enviro/sun/1_nl_secondary_2.tga");

	sky_model = corec.Res_LoadModel(RES_ENVIRONMENT_PATH "sky/sky.model");
	sky_top_model = corec.Res_LoadModel(RES_ENVIRONMENT_PATH "sky/sky_top.model");

	TL_SetTimeOfDay(700);
}
