// (C) 2002 S2 Games

// cl_simpleparticles.c

// this is to get simple gameplay related particle effects in here
// we'll use this for blood effects and other minor effects that don't require the overhead of the internal particle engine
// more elaborate effects like fire from buildings, some magic effects, the 'electrified weapons' effects, etc, will use the internal
// particle engine.  when the internal particle engine is more feature rich we may move all of this over to it, though

#include "client_game.h"

#define MAX_ACTIVE_PARTICLES	4096

simpleParticle_t	particles[MAX_ACTIVE_PARTICLES];
complexParticle_t	exParticles[MAX_ACTIVE_PARTICLES];

static int lowestFreeParticle = 0;

extern cvar_t cl_cameraDistLerp;

#define	MAX_ACTIVE_BEAMS	4096
beam_t	beams[MAX_ACTIVE_BEAMS];

cvar_t	cl_maxActiveParticles = {"cl_maxActiveParticles",	"4096",	CVAR_VALUERANGE,	0,	MAX_ACTIVE_PARTICLES};
cvar_t	cl_maxActiveBeams =		{"cl_maxActiveBeams",		"1024",	CVAR_VALUERANGE,	0,	MAX_ACTIVE_BEAMS};
cvar_t	cl_maxParticleAllocsPerFrame = { "cl_maxParticleAllocsPerFrame", "2048", CVAR_VALUERANGE, 0, MAX_ACTIVE_PARTICLES};
cvar_t	cl_disableParticles = 	{"cl_disableParticles", 	"0"};
cvar_t	cl_cullParticles =		{"cl_cullParticles",		"0"};

int numAllocsThisFrame = 0;

void	CL_ResetParticleAllocCount()
{
	numAllocsThisFrame = 0;
}

simpleParticle_t	*CL_AllocParticle()
{
	int n;
	int	reuse = 0;
	int max = cl_maxActiveParticles.integer;

	if (numAllocsThisFrame > cl_maxParticleAllocsPerFrame.integer)
		return NULL;

	numAllocsThisFrame++;
	
	for (n = lowestFreeParticle; n < max; n++)
	{
		if (particles[n].active)
		{
			/*if (particles[n].spawntime < spawntime)		//we re-use the oldest particles
			{
				reuse = n;
				spawntime = particles[n].spawntime;
			}*/
			continue;
		}

		memset(&particles[n], 0, sizeof(simpleParticle_t));
		particles[n].active = true;
		particles[n].index = n;
		particles[n].minZ = -FAR_AWAY;
		particles[n].maxZ = FAR_AWAY;

		lowestFreeParticle = n+1;

		return &particles[n];
	}

	//use a particle that's already active
	memset(&particles[reuse], 0, sizeof(simpleParticle_t));
	particles[reuse].active = true;
	particles[reuse].index = reuse;
	particles[reuse].minZ = -FAR_AWAY;
	particles[reuse].maxZ = FAR_AWAY;
	return &particles[reuse];
}


/*==========================

  CL_AllocParticleEx

  Finds a suitable particle to use, marks it as active and returns a pointer to it

 ==========================*/

complexParticle_t	*CL_AllocParticleEx()
{
	int n;
	int max;
	int	oldest = cl.gametime, reuse = 0;

	if (numAllocsThisFrame > cl_maxParticleAllocsPerFrame.integer)
		return NULL;

	numAllocsThisFrame++;

	max = cl_maxActiveParticles.integer;
	for (n = 0; n < max; n++)
	{
		if (exParticles[n].active)
		{
			if (exParticles[n].birthtime < oldest)
			{
				reuse = n;
				oldest = exParticles[n].birthtime;
			}
			continue;
		}

		reuse = n;
		break;
	}

	//use a particle that's already active
	memset(&exParticles[reuse], 0, sizeof(complexParticle_t));
	exParticles[reuse].active = true;
	exParticles[reuse].birthtime = cl.gametime;
	return &exParticles[reuse];
}

void	CL_AddParticle(simpleParticle_t *particle)
{
	particle->active = true;
	particle->spawntime = cl.gametime;
}

void	CL_DeleteParticle(simpleParticle_t *particle)
{
	particle->active = false;
	if (lowestFreeParticle > particle->index)
		lowestFreeParticle = particle->index;
}

#define NUM_FRAMES_BETWEEN_DIST_CHECK 5
static int batch = 0;

void	CL_AdvanceParticle(simpleParticle_t *particle)
{
	vec3_t accel, gravity, vel;
	vec2_t vector;
	float dist;

	if (particle->birthDelay)
	{
		if (cl.gametime - particle->spawntime <= particle->birthDelay)
			return;
		else
		{
			particle->spawntime = cl.gametime;
			particle->deathtime += particle->birthDelay;
			particle->birthDelay = 0;
		}
	}

	if (cl.gametime >= particle->deathtime
		|| particle->pos[Z] < particle->minZ 
		|| particle->pos[Z] > particle->maxZ)
	{
		CL_DeleteParticle(particle);
		return;
	}

	if (particle->flags & PARTICLE_FLAG_PARENT_RADIUS
		&& particle->index % NUM_FRAMES_BETWEEN_DIST_CHECK == batch)
	{
		dist = M_GetDistanceSqVec2(cl.objects[particle->parent].visual.pos, particle->pos);
		if (dist > particle->param2)
		{
			//move it to the other side of the circle
			M_SubVec2(cl.camera.origin, particle->pos, vector);
			M_NormalizeVec2(vector);
			M_MultVec2(vector, particle->param2, vector);
			M_AddVec2(vector, cl.objects[particle->parent].visual.pos, particle->pos);
		}
	}

	//update velocity
	M_SetVec3(gravity, 0.0, 0.0, -cl.frametime * particle->gravity);
	M_CopyVec3(particle->velocity, accel);
	M_Normalize(accel);
	M_MultVec3(accel, cl.frametime * particle->accel, accel);
	M_AddVec3(accel, gravity, accel);
	M_AddVec3(particle->velocity, accel, particle->velocity);

	//update position
	M_MultVec3(particle->velocity, cl.frametime, vel);
	M_AddVec3(particle->pos, vel, particle->pos);

	particle->visible = true;
	if (cl_cullParticles.integer)
	{
		M_SubVec2(cl.camera.origin, particle->pos, vel);
		if (M_DotProduct(vel, cl.camera.viewaxis[FORWARD]) >= 0)
			particle->visible = false;
	}
}


/*==========================

  CL_AdvanceParticleEx

  Adjusts the properties for an active particle over a frame

 ==========================*/

void	CL_AdvanceParticleEx(complexParticle_t *particle)
{
	vec3_t	vec;

	if (particle->deathtime <= cl.gametime)
	{
		particle->active = false;
		return;
	}

	//velocity movement
	M_MultVec3(particle->velocity, cl.frametime, vec);
	M_AddVec3(particle->position, vec, particle->position);
}

void	CL_ParticleFrame()
{
	//run all particle logic
	int n;

	PERF_BEGIN(PERF_PARTICLES);
	
	if (cl_disableParticles.integer)
		return;
	
	batch = (batch + 1) % NUM_FRAMES_BETWEEN_DIST_CHECK;

	for (n = 0; n < cl_maxActiveParticles.integer; n++)
	{
		if (particles[n].active)
			CL_AdvanceParticle(&particles[n]);

		if (exParticles[n].active)
			CL_AdvanceParticleEx(&exParticles[n]);
	}

	PERF_END(PERF_PARTICLES);
}

void	CL_RenderParticles()
{
	int n, livetime;
	sceneobj_t sc;
	simpleParticle_t *particle;
	float lerp;
	float red, green, blue, alpha;

	for (n = 0; n < cl_maxActiveParticles.integer; n++)
	{
		particle = &particles[n];

		CLEAR_SCENEOBJ(sc);

		if (!particle->active
			|| !particle->visible
		   	|| particle->deathtime - cl.gametime <= 0 || particle->deathtime - particle->spawntime <= 0
		   	|| cl.gametime - particle->spawntime < particle->birthDelay)
			continue;

		livetime = (cl.gametime - particle->spawntime);
		lerp = (float)(livetime) / (float)(particle->deathtime - particle->spawntime);

		red = LERP(lerp, particle->red, particle->deathred);
		green = LERP(lerp, particle->green, particle->deathgreen);
		blue = LERP(lerp, particle->blue, particle->deathblue);
		alpha = LERP(lerp, particle->alpha, particle->deathalpha);

		if (particle->model)
		{
			sc.objtype = OBJTYPE_MODEL;
			sc.scale = LERP(lerp, (float)particle->width, particle->deathwidth);
			sc.flags = SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME | SCENEOBJ_SOLID_COLOR;
			sc.model = particle->model;
			sc.skin = particle->skin;
			sc.loframe = LERP(lerp, 0.0, (float)corec.Res_GetNumTextureFrames(sc.shader));

			sc.skeleton = &particle->skeleton;
			corec.Geom_BeginPose(sc.skeleton, particle->model);
			corec.Geom_SetBoneAnim("", "item_active", cl.gametime - particle->spawntime, cl.systime, 200, 0);
			corec.Geom_EndPose();
		}
		else
		{
			sc.objtype = OBJTYPE_BILLBOARD;
			sc.width = LERP(lerp, particle->width, particle->deathwidth);
			sc.height = LERP(lerp, particle->height, particle->deathheight);
			sc.alpha = LERP(lerp, particle->alpha, particle->deathalpha);
			if (particle->flags & PARTICLE_FLAG_BILLBOARD_ORIENTED)
				sc.flags = SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME | SCENEOBJ_SOLID_COLOR | SCENEOBJ_BILLBOARD_ORIENTATION;
			else
				sc.flags = SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME | SCENEOBJ_SOLID_COLOR | SCENEOBJ_BILLBOARD_ALL_AXES;
			sc.shader = particle->shader;
			sc.loframe = LERP(lerp, 0.0, (float)corec.Res_GetNumTextureFrames(sc.shader));
			//sc.loframe = (cl.gametime - particles[n].spawntime) / (1000 / particles[n].fps);
			sc.angle[0] = LERP(lerp, particle->angle, particle->deathAngle);
		}

		SET_VEC4(sc.color, red, green, blue, alpha);
		M_CopyVec3(particle->pos, sc.pos);
		if (particle->flags & PARTICLE_FLAG_PARENT_POS)
			M_AddVec3(sc.pos, cl.objects[particle->parent].visual.pos, sc.pos);

		if (particle->flags & PARTICLE_FLAG_PARAM_SINEWAVE)
		{
			int index = (int)(((float)livetime/particle->fps)) % MAX_SINE_SAMPLES;
			float sinVal = sinData[index] * particle->param;

			sc.pos[0] += particle->direction * sinVal;
			sc.pos[1] += (1 - particle->direction) * sinVal;
		}

		corec.Scene_AddObject(&sc);
	}
}


int	CL_GetComplexParticleLerp(float *lerp, int numkeys, float bias[])
{
	int		basekey;
	float	exp;

	switch(numkeys)
	{
	case 1:
		basekey = 0;
		break;

	case 2:
		if ((*lerp) < 0.5)
		{
			(*lerp) *= 2.0;
			basekey = 0;
		}
		else
		{
			(*lerp) = ((*lerp) - 0.5) * 2.0;
			basekey = 1;
		}
		break;

	case 3:
		if ((*lerp) < 0.333)
		{
			(*lerp) *= 3.0;
			basekey = 0;
		}
		else if ((*lerp) < 0.667)	
		{
			(*lerp) = ((*lerp) - 0.333) * 3.0;
			basekey = 1;
		}
		else
		{
			(*lerp) = ((*lerp) - 0.667) * 3.0;
			basekey = 2;
		}
		break;
	default:
		basekey = 0;
	}

	exp = 2.0 * (1.0 - bias[basekey]);
	(*lerp) = pow((*lerp), exp);

	return basekey;
}

/*==========================

  CL_RenderExParticles

  Draw the complex particles

 ==========================*/

void	CL_RenderExParticles()
{
	int n;
	sceneobj_t sc;
	float truelerp, lerp;
	int	key;
	float red, green, blue, alpha;

	CLEAR_SCENEOBJ(sc);

	for (n = 0; n < cl_maxActiveParticles.integer; n++)
	{
		complexParticle_t	*particle = &exParticles[n];

		if (!particle->active)
			continue;

		truelerp = lerp = (float)(cl.gametime - particle->birthtime) / (float)(particle->deathtime - particle->birthtime);

		key = CL_GetComplexParticleLerp(&lerp, particle->scaleKeys, particle->scalebias);
		sc.width = LERP(lerp, particle->scale[key], particle->scale[key+1]);
		sc.height = LERP(lerp, particle->scale[key], particle->scale[key+1]);
		lerp = truelerp;

		key = CL_GetComplexParticleLerp(&lerp, particle->angleKeys, particle->anglebias);
		sc.angle[0] = LERP(lerp, particle->angle[key], particle->angle[key] + particle->angle[key+1]);
		lerp = truelerp;

		key = CL_GetComplexParticleLerp(&lerp, particle->redKeys, particle->redbias);
		red = LERP(lerp, particle->red[key], particle->red[key+1]);
		lerp = truelerp;

		key = CL_GetComplexParticleLerp(&lerp, particle->greenKeys, particle->greenbias);
		green = LERP(lerp, particle->green[key], particle->green[key+1]);
		lerp = truelerp;

		key = CL_GetComplexParticleLerp(&lerp, particle->blueKeys, particle->bluebias);
		blue = LERP(lerp, particle->blue[key], particle->blue[key+1]);
		lerp = truelerp;

		key = CL_GetComplexParticleLerp(&lerp, particle->alphaKeys, particle->alphabias);
		alpha = LERP(lerp, particle->alpha[key], particle->alpha[key+1]);
		lerp = truelerp;

		/*if (particles[n].model)
		{
			sc.objtype = OBJTYPE_MODEL;
			sc.scale = 1.0;//LERP(lerp, (float)particles[n].scale, particles[n].deathscale);
			sc.flags = SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME | SCENEOBJ_SOLID_COLOR;
			sc.model = particles[n].model;
			sc.skin = particles[n].skin;
			sc.loframe = LERP(lerp, 0.0, (float)corec.Res_GetNumTextureFrames(sc.shader));
		}
		else*/
		sc.objtype = OBJTYPE_BILLBOARD;
		sc.alpha = 1;//LERP(lerp, particles[n].alpha, particles[n].deathalpha);
		sc.flags = SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME | SCENEOBJ_SOLID_COLOR | SCENEOBJ_BILLBOARD_ALL_AXES;
		sc.shader = particle->shader;
		//sc.loframe = LERP(lerp, 0, (float)corec.Res_GetNumTextureFrames(sc.shader));
		sc.loframe = (cl.gametime - particles[n].spawntime) / (1000 / particles[n].fps);

		SET_VEC4(sc.color, red, green, blue, alpha);
		M_CopyVec3(exParticles[n].position, sc.pos);
		//if (exParticles[n].flags & PARTICLE_FLAG_PARENT_POS)
		//	M_AddVec3(sc.pos, cl.objects[particles[n].parent].visual.pos, sc.pos);

		corec.Scene_AddObject(&sc);
	}
}

void	CL_Draw2dParticles()
{
	int n;

	for (n=0; n < cl_maxActiveParticles.integer; n++)
	{
		float x,y,w,h,lerp;

		if (!particles[n].active)
			continue;
		if (particles[n].deathtime - cl.gametime <= 0 || particles[n].deathtime - particles[n].spawntime <= 0)
			continue;
		if (cl.gametime - particles[n].spawntime <= particles[n].birthDelay)
			continue;
		if (!particles[n].ortho)
			continue;			

		w = particles[n].width * particles[n].scale;
		h = particles[n].height * particles[n].scale;
		x = particles[n].pos[0] - (w/2);
		y = particles[n].pos[1] - (h/2);
		w = particles[n].width;
		h = particles[n].height;

		lerp = (float)(cl.gametime - particles[n].spawntime) / (float)(particles[n].deathtime - particles[n].spawntime);	
		corec.Draw_SetShaderTime(LERP(lerp, 0, (float)corec.Res_GetNumTextureFrames(particles[n].shader)));
		//corec.Draw_SetShaderTime((cl.gametime - particles[n].spawntime) / (1000 / particles[n].fps));

		corec.Draw_SetColor(vec4(1,1,1,1));
		corec.Draw_Quad2d(particles[n].pos[0], particles[n].pos[1], particles[n].width, particles[n].height, 0, 0, 1, 1, particles[n].shader);
	}
}

//sets the deathtime 
void	CL_PlayAnimOnce(simpleParticle_t *particle, int fps, residx_t shader)
{
	int numframes = corec.Res_GetNumTextureFrames(shader);

	//default to avoid div by zero
	if (fps == 0)
		fps = 30;

	particle->deathtime = cl.gametime + (int)(((float)numframes / fps) * 1000.0);
	particle->fps = fps;
	particle->frame = 0;
	particle->shader = shader;
}

void	CL_ResetBeams()
{
	int n;

	for (n = 0; n < MAX_ACTIVE_BEAMS; n++)
		beams[n].active = false;
}

beam_t	*CL_AddBeam(vec3_t start, vec3_t end, residx_t shader)
{
	int	n, deadest = 0, deathtime = cl.gametime + 30000;

	for (n = 0; n < cl_maxActiveBeams.integer; n++)
	{
		if (!beams[n].active)
		{
			deadest = n;
			break;
		}

		if (deathtime > beams[n].deathtime)
		{
			deadest = n;
			deathtime = beams[n].deathtime;
		}
	}

	beams[deadest].active = true;
	beams[deadest].spawntime = cl.gametime;

	M_CopyVec3(start, beams[deadest].start);
	M_CopyVec3(end, beams[deadest].end);
	beams[deadest].shader = shader;
	return &beams[deadest];
}

#pragma optimize ("", off)
void	CL_RenderBeams()
{
	int			n;
	sceneobj_t	sc;
	float		lerp;
	float		red, green, blue, alpha;
	vec3_t		vec;
	float		length;

	for (n = 0; n < cl_maxActiveBeams.integer; n++)
	{
		CLEAR_SCENEOBJ(sc);

		if (!beams[n].active)
			continue;
		if (beams[n].deathtime - beams[n].spawntime <= 0)
			continue;

		if (cl.gametime >= beams[n].deathtime)
		{
			beams[n].active = false;
			continue;
		}

		lerp = (float)(cl.gametime - beams[n].spawntime) / (float)(beams[n].deathtime - beams[n].spawntime);

		//calculate alpha
		red = LERP(lerp, beams[n].red, beams[n].deathred);
		green = LERP(lerp, beams[n].green, beams[n].deathgreen);
		blue = LERP(lerp, beams[n].blue, beams[n].deathblue);
		alpha = LERP(lerp, beams[n].alpha, beams[n].deathalpha);

		sc.objtype = OBJTYPE_BEAM;
		sc.scale = LERP(lerp, (float)beams[n].size, beams[n].deathsize);
		sc.flags = SCENEOBJ_LOFRAME_SPECIFIES_TEXTUREFRAME | SCENEOBJ_SOLID_COLOR; 
		sc.shader = beams[n].shader;
		sc.loframe = LERP(lerp, 0.0, (float)corec.Res_GetNumTextureFrames(sc.shader));
		SET_VEC4(sc.color, red, green, blue, alpha);
		M_CopyVec3(beams[n].start, sc.pos);
		M_CopyVec3(beams[n].end, sc.beamTargetPos);
		M_SubVec3(sc.pos, sc.beamTargetPos, vec);
		length = M_Normalize(vec);
		if (beams[n].tilelength)
			sc.height = length / beams[n].tilelength;
		else
			sc.height = 1.0;
		
		corec.Scene_AddObject(&sc);

	}
}
#pragma optimize ("", on)

void	CL_ResetParticles()
{
	int i;

	for (i = 0; i < MAX_ACTIVE_PARTICLES; i++)
	{
		particles[i].active = false;
		exParticles[i].active = false;
	}
}

void	CL_InitParticles()
{
	CL_ResetBeams();
	CL_ResetParticles();

	corec.Cvar_Register(&cl_maxActiveParticles);
	corec.Cvar_Register(&cl_maxActiveBeams);
	corec.Cvar_Register(&cl_maxParticleAllocsPerFrame);
	corec.Cvar_Register(&cl_disableParticles);
	corec.Cvar_Register(&cl_cullParticles);
}
