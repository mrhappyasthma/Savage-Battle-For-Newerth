// (C) 2001 S2 Games

// world_lights.c

// global lighting setup for levels (directional lights only)

// lights that attenuate are stored as scene objects so they can be culled

/*
  variable meanings:

  in the following variable descriptions, 0 can be replaced with any number from 0 to 3
  * can be replaced with either "ter" (lighting setup for terrain) or "obj" (lighting setup for objects)

  *_light0			specifies the state of light 0 for objects.  Can be "off", "vector", "rim", or "forward"
					"vector" allows you to specify any vector to cast a light
					"sun" uses the sun vector (wr_sun_x, wr_sun_y, wr_sun_z) and the light x,y,z values are ignored					
					"rim" and "forward" are vectors that are linked to the camera position and offset with the light0_x and light0_y vars
					"off" specifies that the light is not active

  *_light0_x		if light type is "rim" or "forward", specifies the X offset in screen space of the light, with -1 being the leftmost part of the screen, 1 being the rightmost (however, you may increase the range beyond (-1,1))
					if light type is "vector", specifies the X component of the light vector
  *_light0_y		if light type is "rim" or "forward", specifies the Y offset in screen space of the light, with -1 being the topmost part of the screen, 1 being the bottommost (however, you may increase the range beyond (-1,1))
					if light type is "vector", specifies the Y component of the light vector
  *_light0_z		if light type is "vector", specifies the Z component of the light vector.  has no effect for "rim" or "forward" lights
  *_light0_r		specifies the red component of the light color (ranges from 0 to 2)
  *_light0_g		specifies the green component of the light color (ranges from 0 to 2)
  *_light0_b		specifies the blue component of the light color (ranges from 0 to 2)

  *_ambient_r		specifies the red component of the overall ambient color

*/

#include "core.h"

cvar_t	obj_ambient_r = { "obj_ambient_r", "0.1", CVAR_WORLDCONFIG };
cvar_t	obj_ambient_g = { "obj_ambient_g", "0.1", CVAR_WORLDCONFIG };
cvar_t	obj_ambient_b = { "obj_ambient_b", "0.1", CVAR_WORLDCONFIG };
cvar_t	ter_ambient_r = { "ter_ambient_r", "0.1", CVAR_WORLDCONFIG };
cvar_t	ter_ambient_g = { "ter_ambient_g", "0.1", CVAR_WORLDCONFIG };
cvar_t	ter_ambient_b = { "ter_ambient_b", "0.1", CVAR_WORLDCONFIG };

cvar_t	wr_sun_x = { "wr_sun_x", "0", CVAR_WORLDCONFIG };
cvar_t	wr_sun_y = { "wr_sun_y", "0", CVAR_WORLDCONFIG };
cvar_t	wr_sun_z = { "wr_sun_z", "-1", CVAR_WORLDCONFIG };


worldLight_t	objLights[MAX_WORLD_LIGHTS];
worldLight_t	terLights[MAX_WORLD_LIGHTS];
worldLight_t	chrLights[MAX_WORLD_LIGHTS];


void	World_InitLighting()
{
	int n;

	Cvar_Register(&obj_ambient_r);
	Cvar_Register(&obj_ambient_g);
	Cvar_Register(&obj_ambient_b);

	Cvar_Register(&ter_ambient_r);
	Cvar_Register(&ter_ambient_g);
	Cvar_Register(&ter_ambient_b);

	Cvar_Register(&wr_sun_x);
	Cvar_Register(&wr_sun_y);
	Cvar_Register(&wr_sun_z);

	for (n=0; n<MAX_WORLD_LIGHTS; n++)
	{
		objLights[n].type.name = fmt("obj_light%i", n);
		objLights[n].type.string = "off";
		objLights[n].type.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&objLights[n].type);
		objLights[n].x.name = fmt("obj_light%i_x", n);
		objLights[n].x.string = "";
		objLights[n].x.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&objLights[n].x);
		objLights[n].y.name = fmt("obj_light%i_y", n);
		objLights[n].y.string = "";
		objLights[n].y.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&objLights[n].y);
		objLights[n].z.name = fmt("obj_light%i_z", n);
		objLights[n].z.string = "";
		objLights[n].z.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&objLights[n].z);
		objLights[n].r.name = fmt("obj_light%i_r", n);
		objLights[n].r.string = "";
		objLights[n].r.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&objLights[n].r);
		objLights[n].g.name = fmt("obj_light%i_g", n);
		objLights[n].g.string = "";
		objLights[n].g.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&objLights[n].g);
		objLights[n].b.name = fmt("obj_light%i_b", n);
		objLights[n].b.string = "";
		objLights[n].b.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&objLights[n].b);

		terLights[n].type.name = fmt("ter_light%i", n);
		terLights[n].type.string = "off";
		terLights[n].type.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&terLights[n].type);
		terLights[n].x.name = fmt("ter_light%i_x", n);
		terLights[n].x.string = "";
		terLights[n].x.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&terLights[n].x);
		terLights[n].y.name = fmt("ter_light%i_y", n);
		terLights[n].y.string = "";
		terLights[n].y.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&terLights[n].y);
		terLights[n].z.name = fmt("ter_light%i_z", n);
		terLights[n].z.string = "";
		terLights[n].z.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&terLights[n].z);
		terLights[n].r.name = fmt("ter_light%i_r", n);
		terLights[n].r.string = "";
		terLights[n].r.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&terLights[n].r);
		terLights[n].g.name = fmt("ter_light%i_g", n);
		terLights[n].g.string = "";
		terLights[n].g.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&terLights[n].g);
		terLights[n].b.name = fmt("ter_light%i_b", n);
		terLights[n].b.string = "";
		terLights[n].b.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&terLights[n].b);

		chrLights[n].type.name = fmt("chr_light%i", n);
		chrLights[n].type.string = "off";
		chrLights[n].type.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&chrLights[n].type);
		chrLights[n].x.name = fmt("chr_light%i_x", n);
		chrLights[n].x.string = "";
		chrLights[n].x.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&chrLights[n].x);
		chrLights[n].y.name = fmt("chr_light%i_y", n);
		chrLights[n].y.string = "";
		chrLights[n].y.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&chrLights[n].y);
		chrLights[n].z.name = fmt("chr_light%i_z", n);
		chrLights[n].z.string = "";
		chrLights[n].z.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&chrLights[n].z);
		chrLights[n].r.name = fmt("chr_light%i_r", n);
		chrLights[n].r.string = "";
		chrLights[n].r.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&chrLights[n].r);
		chrLights[n].g.name = fmt("chr_light%i_g", n);
		chrLights[n].g.string = "";
		chrLights[n].g.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&chrLights[n].g);
		chrLights[n].b.name = fmt("chr_light%i_b", n);
		chrLights[n].b.string = "";
		chrLights[n].b.flags = CVAR_WORLDCONFIG;
		Cvar_Register(&chrLights[n].b);
	}
}



#if 0

cvar_t	obj_light0 = { "obj_light0", "sun" };						//light type
cvar_t	obj_light0_x = { "obj_light0_x", "" };						//direction
cvar_t	obj_light0_y = { "obj_light0_y", "" };						//direction
cvar_t	obj_light0_z = { "obj_light0_z", "" };						//direction
cvar_t	obj_light0_r = { "obj_light0_r", "1" };						//diffuse component
cvar_t	obj_light0_g = { "obj_light0_g", "1" };						//diffuse component
cvar_t	obj_light0_b = { "obj_light0_b", "1" };						//diffuse component
cvar_t	obj_light0_ambient_r = { "obj_light0_ambient_r", "0" };		//ambient component
cvar_t	obj_light0_ambient_g = { "obj_light0_ambient_g", "0" };		//ambient component
cvar_t	obj_light0_ambient_b = { "obj_light0_ambient_b", "0" };		//ambient component
cvar_t	obj_light1 = { "obj_light1", "rim" };
cvar_t	obj_light1_x = { "obj_light1_x", "" };
cvar_t	obj_light1_y = { "obj_light1_y", "" };
cvar_t	obj_light1_z = { "obj_light1_z", "" };
cvar_t	obj_light1_r = { "obj_light1_r", "1" };
cvar_t	obj_light1_g = { "obj_light1_g", "1" };
cvar_t	obj_light1_b = { "obj_light1_b", "1" };
cvar_t	obj_light1_ambient_r = { "obj_light0_ambient_r", "0" };		//ambient component
cvar_t	obj_light1_ambient_g = { "obj_light0_ambient_g", "0" };		//ambient component
cvar_t	obj_light1_ambient_b = { "obj_light0_ambient_b", "0" };		//ambient component
cvar_t	obj_light2 = { "obj_light2", "off" };
cvar_t	obj_light2_x = { "obj_light2_x", "" };
cvar_t	obj_light2_y = { "obj_light2_y", "" };
cvar_t	obj_light2_z = { "obj_light2_z", "" };
cvar_t	obj_light2_r = { "obj_light2_r", "1" };
cvar_t	obj_light2_g = { "obj_light2_g", "1" };
cvar_t	obj_light2_b = { "obj_light2_b", "1" };
cvar_t	obj_light2_ambient_r = { "obj_light0_ambient_r", "0" };		//ambient component
cvar_t	obj_light2_ambient_g = { "obj_light0_ambient_g", "0" };		//ambient component
cvar_t	obj_light2_ambient_b = { "obj_light0_ambient_b", "0" };		//ambient component
cvar_t	obj_light3 = { "obj_light3", "off" };
cvar_t	obj_light3_x = { "obj_light3_x", "" };
cvar_t	obj_light3_y = { "obj_light3_y", "" };
cvar_t	obj_light3_z = { "obj_light3_z", "" };
cvar_t	obj_light3_r = { "obj_light3_r", "1" };
cvar_t	obj_light3_g = { "obj_light3_g", "1" };
cvar_t	obj_light3_b = { "obj_light3_b", "1" };
cvar_t	obj_light3_ambient_r = { "obj_light0_ambient_r", "0" };		//ambient component
cvar_t	obj_light3_ambient_g = { "obj_light0_ambient_g", "0" };		//ambient component
cvar_t	obj_light3_ambient_b = { "obj_light0_ambient_b", "0" };		//ambient component

cvar_t	ter_ambient_r = { "ter_ambient_r", "0.1" };
cvar_t	ter_ambient_g = { "ter_ambient_g", "0.1" };
cvar_t	ter_ambient_b = { "ter_ambient_b", "0.1" };
cvar_t	ter_light0 = { "ter_light0", "sun" };						//light type
cvar_t	ter_light0_x = { "ter_light0_x", "" };						//direction
cvar_t	ter_light0_y = { "ter_light0_y", "" };						//direction
cvar_t	ter_light0_z = { "ter_light0_z", "" };						//direction
cvar_t	ter_light0_r = { "ter_light0_r", "1" };						//diffuse component
cvar_t	ter_light0_g = { "ter_light0_g", "1" };						//diffuse component
cvar_t	ter_light0_b = { "ter_light0_b", "1" };						//diffuse component
cvar_t	ter_light0_ambient_r = { "ter_light0_ambient_r", "0" };		//ambient component
cvar_t	ter_light0_ambient_g = { "ter_light0_ambient_g", "0" };		//ambient component
cvar_t	ter_light0_ambient_b = { "ter_light0_ambient_b", "0" };		//ambient component
cvar_t	ter_light1 = { "ter_light1", "rim" };
cvar_t	ter_light1_x = { "ter_light1_x", "" };
cvar_t	ter_light1_y = { "ter_light1_y", "" };
cvar_t	ter_light1_z = { "ter_light1_z", "" };
cvar_t	ter_light1_r = { "ter_light1_r", "1" };
cvar_t	ter_light1_g = { "ter_light1_g", "1" };
cvar_t	ter_light1_b = { "ter_light1_b", "1" };
cvar_t	ter_light1_ambient_r = { "ter_light0_ambient_r", "0" };		//ambient component
cvar_t	ter_light1_ambient_g = { "ter_light0_ambient_g", "0" };		//ambient component
cvar_t	ter_light1_ambient_b = { "ter_light0_ambient_b", "0" };		//ambient component
cvar_t	ter_light2 = { "ter_light2", "off" };
cvar_t	ter_light2_x = { "ter_light2_x", "" };
cvar_t	ter_light2_y = { "ter_light2_y", "" };
cvar_t	ter_light2_z = { "ter_light2_z", "" };
cvar_t	ter_light2_r = { "ter_light2_r", "1" };
cvar_t	ter_light2_g = { "ter_light2_g", "1" };
cvar_t	ter_light2_b = { "ter_light2_b", "1" };
cvar_t	ter_light2_ambient_r = { "ter_light0_ambient_r", "0" };		//ambient component
cvar_t	ter_light2_ambient_g = { "ter_light0_ambient_g", "0" };		//ambient component
cvar_t	ter_light2_ambient_b = { "ter_light0_ambient_b", "0" };		//ambient component
cvar_t	ter_light3 = { "ter_light3", "off" };
cvar_t	ter_light3_x = { "ter_light3_x", "" };
cvar_t	ter_light3_y = { "ter_light3_y", "" };
cvar_t	ter_light3_z = { "ter_light3_z", "" };
cvar_t	ter_light3_r = { "ter_light3_r", "1" };
cvar_t	ter_light3_g = { "ter_light3_g", "1" };
cvar_t	ter_light3_b = { "ter_light3_b", "1" };
cvar_t	ter_light3_ambient_r = { "ter_light0_ambient_r", "0" };		//ambient component
cvar_t	ter_light3_ambient_g = { "ter_light0_ambient_g", "0" };		//ambient component
cvar_t	ter_light3_ambient_b = { "ter_light0_ambient_b", "0" };		//ambient component

#endif
