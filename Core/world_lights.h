// (C) 2001 S2 Games

// world_lights.h


#define MAX_WORLD_LIGHTS	4

typedef struct
{
	cvar_t	type;
	cvar_t	x;
	cvar_t	y;
	cvar_t	z;
	cvar_t	r;
	cvar_t	g;
	cvar_t	b;
} worldLight_t;


extern worldLight_t	objLights[MAX_WORLD_LIGHTS];
extern worldLight_t	terLights[MAX_WORLD_LIGHTS];

void	World_InitLighting();

extern cvar_t obj_ambient_r;
extern cvar_t obj_ambient_g;
extern cvar_t obj_ambient_b;
extern cvar_t ter_ambient_r;
extern cvar_t ter_ambient_g;
extern cvar_t ter_ambient_b;
extern cvar_t wr_sun_x;
extern cvar_t wr_sun_y;
extern cvar_t wr_sun_z;