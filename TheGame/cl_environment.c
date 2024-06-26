/*
 * (C) 2002 S2 Games
 * cl_environment.c - environmental effects that are client-side only
 */

#include "client_game.h"

#define	MAX_CLIENT_ENVIRONMENT_OBJECTS 100

#define BIRD_LANDING 		0x01
#define BIRD_TAKINGOFF 		0x02
#define BIRD_CHASING 		0x04

#define	BIRD_CRUISINGALTITUDE	350
#define BIRD_SLOWDOWNHEIGHT	12
#define BIRD_CHASE_DIST		80
#define BIRD_DECISIONTIME	800

#define TAKEOFF_VELOCITY	1.7

#define	BUTTERFLY_CRUISINGALTITUDE 10
#define BUTTERFLY_DECISIONTIME 600

#define MAX_ENV_BLOCKS 6

typedef enum
{
	ENV_BIRD = 1,
	ENV_BUTTERFLY,
	NUM_CLIENT_ENVIRONMENT_OBJECTS
} CLIENT_ENVIRONMENT_OBJECTS;

typedef struct
{
	bool 	active;
	int 	type;
	vec3_t 	pos;
	vec3_t  direction;
	float 	speed;

	int		moveState;
	float 	animTime;

	float	nextDecision;

	int 	chasingBird;
	int	flags;
	skeleton_t	skeleton;
} env_object_t;

env_object_t env_objects[MAX_CLIENT_ENVIRONMENT_OBJECTS];

cvar_t	cl_showEnvEffects = { "cl_showEnvEffects", "1", CVAR_SAVECONFIG };
cvar_t	cl_environmentFalloff = { "cl_environmentFalloff", "400" };

//cvar_t	cl_birdModel = { "cl_birdModel", "/props/animals/bird01/bird.model" };
//cvar_t	cl_birdSkin = { "cl_birdSkin", "default" };
cvar_t	cl_butterflyModel = { "cl_butterflyModel", "/props/animals/butterfly/butterfly.model" };
cvar_t	cl_butterflySkin = { "cl_butterflySkin", "default" };
cvar_t	cl_env_timescale = { "cl_env_timescale", "10" };
cvar_t	cl_maxButterflies = { "cl_maxButterflies", "50" };

//cvar_t	*weather;
//residx_t	birdModel;
//residx_t	birdSkin;
residx_t	butterflyModel;
residx_t	butterflySkin;

#if 0
int			weatherEffect = 0;
int			_weatherEffectIdx[MAX_WEATHER_SUB_EFFECTS]; //into the effects array
#endif

typedef struct
{
	vec2_t pos;
	byte subEffect;
	unsigned int lastDelayEffect;
} effectTime_t;

effectTime_t lastDelayEffect[MAX_ENV_BLOCKS * MAX_ENV_BLOCKS * MAX_WEATHER_SUB_EFFECTS] = { { {0, 0}, 0}};

#if 0
void	CL_ReloadEnvironmentEffect(char *effect)
{
	int i;
	static int lastWeatherEffect = 0;
	weatherSubEffect_t *subEf = NULL;
	
	weatherEffect  = weLookup(effect);

	lastWeatherEffect = weatherEffect;
	
	if (weatherEffect >= 0)
	{
		for (i = 0; i < MAX_WEATHER_SUB_EFFECTS; i++)
		{
			subEf = &weatherEffectData[weatherEffect].effects[i];

			_weatherEffectIdx[i] = efLookup(subEf->effect);
	
			if (_weatherEffectIdx[i])
				CL_LoadEffectResources(_weatherEffectIdx[i]);
		}
	}
}

void	CL_ClearLocalParticles()
{
	int i;

	for (i = 0; i < MAX_ENV_BLOCKS * MAX_ENV_BLOCKS; i++)
		lastDelayEffect[i].lastDelayEffect = 0;
}

int		_getFreeBlock()
{
	int i;
	int oldestIdx = 0;
	unsigned int oldest = 2000000000;
	
	for (i = 0; i < MAX_ENV_BLOCKS * MAX_ENV_BLOCKS; i++)
	{
		if (lastDelayEffect[i].lastDelayEffect < oldest)
		{
			oldestIdx = i;
			oldest = lastDelayEffect[i].lastDelayEffect;
		}
	}
	return oldestIdx;
}

int		_getBlock(vec2_t pos, int subEffect)
{
	int i;
	for (i = 0; i < MAX_ENV_BLOCKS * MAX_ENV_BLOCKS; i++)
	{
		if (subEffect == lastDelayEffect[i].subEffect
			&& ABS(lastDelayEffect[i].pos[X] - pos[X]) < 0.001
			&& ABS(lastDelayEffect[i].pos[Y] - pos[Y]) < 0.001)
		{
			return i;
		}
	}
	i = _getFreeBlock();
	lastDelayEffect[i].pos[X] = pos[X];
	lastDelayEffect[i].pos[Y] = pos[Y];
	lastDelayEffect[i].subEffect = subEffect;
	lastDelayEffect[i].lastDelayEffect = 0;
	return i;
}

void 	CL_RenderLocalParticles(vec3_t pos, int subEffect)
{
	int objIdx = MAX_OBJECTS-1;
	clientObject_t *obj;
	vec3_t blockSize;
	weatherEffect_t *weatherData;
	int block;
	
	if (weatherEffect)
	{
		int increment;

		weatherData = &weatherEffectData[weatherEffect];
		
		while (cl.objects[objIdx].base.active && objIdx > MAX_CLIENTS)
			objIdx--;

		if (objIdx <= MAX_CLIENTS)
			return; //no empty object slots?

		obj = &cl.objects[objIdx];
		//shouldn't be necessary...
		//memset(obj, 0, sizeof(clientObject_t));
		obj->visual.index = objIdx;
		obj->base.index = objIdx;
		obj->oldbase.index = objIdx;
		M_CopyVec3(pos, obj->visual.pos);
		M_CopyVec3(pos, obj->oldbase.pos);
	
		if (weatherData->effects[subEffect].numBlocks == 0)
			block = subEffect;
		else
			block = _getBlock(pos, subEffect);
		
		if (cl.gametime - lastDelayEffect[block].lastDelayEffect > 200)
			lastDelayEffect[block].lastDelayEffect = cl.gametime - 200;

		M_SetVec3(blockSize, weatherData->effects[subEffect].blockSize, weatherData->effects[subEffect].blockSize, weatherData->effects[subEffect].blockHeight);
		
		M_MultVec3(blockSize, -0.5, obj->visual.bmin);
		M_MultVec3(blockSize, 0.5, obj->visual.bmax);

		increment = weatherData->effects[subEffect].interval;
		if (increment < 1)
			increment = 1;

		while (1)
		{
			if (lastDelayEffect[block].lastDelayEffect + increment >= cl.gametime)
				break;

			CL_DoEffect(obj, NULL, NULL, _weatherEffectIdx[subEffect], 0, 0);
			corec.Console_DPrintf("calling DoEffect for block %i at <%f, %f, %f>\n", block, obj->visual.pos[X], obj->visual.pos[Y], obj->visual.pos[Z]);
			lastDelayEffect[block].lastDelayEffect += increment;
		}
	}
}

#endif
	
void	CL_RenderEnvironmentEffects()
{
	int i;
	sceneobj_t sc;
	float time;
	float dist, falloff;

#if 0
	vec3_t pos, basepos, blockSize;

	weatherEffect_t *weatherData;
	weatherSubEffect_t *subEffectData;
	pointinfo_t z;
	//vec3_t forward;
	//vec3_t axis[3];
#endif
	
	if (!cl_showEnvEffects.integer)
		return;

#if 0
	if (weather->modified)
	{
		CL_ReloadEnvironmentEffect(weather->string);
		weather->modified = false;
	}

	if (weatherEffect)
	{
		weatherData = &weatherEffectData[weatherEffect];

		for (subEffect = 0; subEffect < MAX_WEATHER_SUB_EFFECTS; subEffect++)
		{
			subEffectData = &weatherData->effects[subEffect];

			if (subEffectData->numBlocks * 2 >= MAX_ENV_BLOCKS-1)
				subEffectData->numBlocks = (MAX_ENV_BLOCKS/2) - 1;

			if (subEffectData->numBlocks == 0)
			{
				basepos[X] = cl.camera.origin[X]; //cl.objects[cl.clientnum].base.pos[X];
				basepos[Y] = cl.camera.origin[Y]; //cl.objects[cl.clientnum].base.pos[Y];
			}
			else
			{
				basepos[X] = ((int)((cl.camera.origin[X] + (subEffectData->blockSize/2)) / subEffectData->blockSize))*subEffectData->blockSize;
				basepos[Y] = ((int)((cl.camera.origin[Y] + (subEffectData->blockSize/2)) / subEffectData->blockSize))*subEffectData->blockSize;
			}


			if (subEffectData->startZType == STARTZ_USE_BASEPLAYERZ)
				basepos[Z] = cl.camera.origin[Z] + subEffectData->startZ; //cl.objects[cl.clientnum].base.pos[Z] + subEffectData->startZ;
			else if (subEffectData->startZType == STARTZ_USE_BASEGROUNDZ)
			{
				corec.World_SampleGround(basepos[X], basepos[Y], &z);
				basepos[Z] = z.z;
			}
			else if (subEffectData->startZType == STARTZ_USE_PERPARTICLEGROUNDZ)
				basepos[Z] = 0;
			
			M_SetVec3(blockSize, subEffectData->blockSize, subEffectData->blockSize, subEffectData->blockSize);

			pos[Z] = basepos[Z];
			/*
			corec.Console_DPrintf("player pos is (%f, %f, %f), basepos is (%f, %f, %f)\n", cl.objects[cl.clientnum].base.pos[X], cl.objects[cl.clientnum].base.pos[Y], cl.objects[cl.clientnum].base.pos[Z],
						basepos[X], basepos[Y], basepos[Z]);
			*/

			for (i = -subEffectData->numBlocks; i <= subEffectData->numBlocks; i++)
			{
				for (j = -subEffectData->numBlocks; j <= subEffectData->numBlocks; j++)
				{
					pos[X] = basepos[X] + subEffectData->blockSize * i;
					pos[Y] = basepos[Y] + subEffectData->blockSize * j;
					//corec.Console_DPrintf("Calling RenderLocalParticles for block (%i, %i)\n", i, j);
					CL_RenderLocalParticles(pos, subEffect); 
				}
			}
		}
	}

#endif

	falloff = cl_environmentFalloff.value * cl_environmentFalloff.value;
	
	time = ((float)corec.Milliseconds() / 1000.0) * 30;
	
	CLEAR_SCENEOBJ(sc);
	//sc.flags |= SCENEOBJ_USE_AXIS;
	sc.scale = 1;

	for (i = 0; i < MAX_CLIENT_ENVIRONMENT_OBJECTS; i++)
	{
		if (!env_objects[i].active)
			continue;

		dist = M_GetDistanceSq(env_objects[i].pos, cl.camera.origin);
		if (dist > falloff)
		{
			continue;
		}
		
		//M_GetAxisFromForwardVec(forward, sc.axis);
		sc.angle[0] = 0;
		sc.angle[1] = 0;
		sc.angle[2] = -((int)M_GetVec2Angle(env_objects[i].direction));
		
		M_CopyVec3(env_objects[i].pos, sc.pos);
		
		switch (env_objects[i].type)
		{
			/*case ENV_BIRD:
				sc.model = birdModel;
				sc.skin = birdSkin;
				if (env_objects[i].moveState == AS_WALK_FWD)
				{
				}
				else
				{
				}
				break;*/

			case ENV_BUTTERFLY:
				sc.model = butterflyModel;
				sc.skin = butterflySkin;
				corec.Geom_BeginPose(&env_objects[i].skeleton, sc.model);

				corec.Geom_SetBoneAnim("", GetAnimName(env_objects[i].moveState), env_objects[i].animTime, cl.systime, 200, 0);
				
				break;
		}

		sc.skeleton = &env_objects[i].skeleton;
		
		corec.Geom_EndPose();
		
		corec.Scene_AddObject(&sc);
	}
}

int	CL_AddEnvironmentEffect(int type, vec3_t pos, bool force)
{
	int i = 0;
	while (i < MAX_CLIENT_ENVIRONMENT_OBJECTS && i < cl_maxButterflies.integer && env_objects[i].active)
		i++;

	if (i >= MAX_CLIENT_ENVIRONMENT_OBJECTS || i >= cl_maxButterflies.integer)
	{
		if (!force)
			return -1;
		
		//get rid of the "oldest" env object
		memmove(env_objects, &env_objects[1], sizeof(env_object_t) * (MAX_CLIENT_ENVIRONMENT_OBJECTS - 1));
		i = MAX_CLIENT_ENVIRONMENT_OBJECTS-1;
	}

	memset(&env_objects[i], 0, sizeof(env_object_t));

	env_objects[i].active = true;
	env_objects[i].type = type;
	M_CopyVec3(pos, env_objects[i].pos);

	env_objects[i].moveState = AS_IDLE;

	env_objects[i].animTime = 0;

	//the M_Randnum is to stagger them so we don't get 100 decisions on one frame and then 0 for the next 50
	env_objects[i].nextDecision = cl.gametime + M_Randnum(1, (type == ENV_BUTTERFLY ? BUTTERFLY_DECISIONTIME : BIRD_DECISIONTIME));
	
	return i;
}

/*
int	CL_FindCloseBirds(vec3_t pos, float maxDistSq, float *dist)
{
	int i = 0, closest = -1;
	float tmpDist;

	*dist = FAR_AWAY;
	while (i < MAX_CLIENT_ENVIRONMENT_OBJECTS)
	{
		if (!env_objects[i].active || env_objects[i].type != ENV_BIRD)
		{
			i++;
			continue;
		}

		tmpDist = M_GetDistanceSq(pos, env_objects[i].pos);
		if (tmpDist < maxDistSq
		    && tmpDist < *dist)
		{
			*dist = tmpDist;
			closest = i;
		}
		i++;
	}
	return closest;
}

void	CL_UpdateBird(env_object_t *obj)
{
	vec3_t dir, velocity;
	float highest, tmpDist;

	//player = CL_FindClosestObjects(unitlist, NUM_UNIT_TYPES, obj->pos, BIRD_CHASE_DIST, &dist);
	//if (player >= 0)
	
	if (obj->moveState == AS_IDLE
		&& M_GetDistanceSq(cl.objects[cl.clientnum].base.pos, obj->pos) < 80*80)
	{
		//evade!	
	  	obj->moveState = AS_WALK_FWD;
	  	obj->flags |= BIRD_TAKINGOFF;
	  	obj->speed = M_Randnum(1, 1.8);
	  	obj->direction[X] = M_Randnum(0, 1);
	  	obj->direction[Y] = M_Randnum(0, 1);
	  	obj->direction[Z] = TAKEOFF_VELOCITY;
	}
	else if (obj->nextDecision < cl.gametime)
	{
		obj->nextDecision = cl.gametime + BIRD_DECISIONTIME;
	  	switch (obj->moveState)
	  	{
	  		case AS_IDLE:
	  				if (M_Randnum(0, 1) < 0.004)
	  				{
	  					//corec.Console_DPrintf("Taking off!\n");
	  					obj->moveState = AS_WALK_FWD;
	  					obj->flags |= BIRD_TAKINGOFF;
	  					obj->speed = M_Randnum(1, 1.8);
	  					obj->direction[X] = M_Randnum(0, 1);
	  					obj->direction[Y] = M_Randnum(0, 1);
	  					obj->direction[Z] = TAKEOFF_VELOCITY;
	  				}
	  				break;
	  		case AS_WALK_FWD:	  		
	  				obj->direction[X] += M_Randnum(-0.5, 0.5);
	  				obj->direction[Y] += M_Randnum(-0.5, 0.5);
	 
	 					if (obj->flags & BIRD_CHASING
	 					    && obj->chasingBird >= 0)
	 					{
	 						tmpDist = M_GetDistanceSq(obj->pos, env_objects[obj->chasingBird].pos);
	 						//is the bird we're chasing still alive and flying around?
	 						if (!env_objects[obj->chasingBird].active
	 						    || env_objects[obj->chasingBird].moveState != AS_WALK_FWD
	 						    || tmpDist > BIRD_CHASE_DIST*BIRD_CHASE_DIST*1.5)
	 						{
	 							//corec.Console_DPrintf("No longer chasing a bird!\n");
	 							obj->flags &= ~BIRD_CHASING;
	 						}
	 						else
	 						{
	 							//follow the other bird here
	 							M_SubVec3(env_objects[obj->chasingBird].pos, obj->pos, dir);
	 							M_Normalize(dir);
	 							obj->direction[0] = LERP(0.5, obj->direction[0], dir[0]);
	 							obj->direction[1] = LERP(0.5, obj->direction[1], dir[1]);
	 							obj->direction[2] = LERP(0.5, obj->direction[2], dir[2]);
	 						}
	 					}
	 					
	 					if (obj->flags & BIRD_LANDING)
	 					{
	 						//have we hit the ground?
	 						highest = CL_GetHighestSolidPoint(obj->pos[X], obj->pos[Y]);
	 						if (obj->pos[Z] <= highest)
	 						{
	 							//corec.Console_DPrintf("We've landed!\n");
	 							obj->pos[Z] = highest + 0.01;
	 							obj->moveState = AS_IDLE;
	 							obj->flags &= ~BIRD_LANDING;
	 							obj->flags &= ~BIRD_CHASING;
	 							obj->speed = 0;
	 						}
	 						else if (obj->pos[Z] - BIRD_SLOWDOWNHEIGHT <= highest)
	 						{
	 							//corec.Console_DPrintf("We're close to the ground, slowing down! obj->direction[Z] is %f\n", obj->direction[Z]);
	 							//we're close to the ground, slow down
	 							obj->speed = MAX(0.5, obj->speed - 0.1);
	 							obj->direction[Z] = MIN(-2.1, obj->direction[Z] + 3);
	 						}
	 						else
	 						{
	 							//we're not close to the ground, let's drop and slow down
	 							obj->direction[Z] = MAX(-5, obj->direction[Z] - 0.2);
	 						}
	 					}
	 					else if (obj->flags & BIRD_TAKINGOFF)
	  					{
	  						//are we really high up?
	  						highest = CL_GetHighestSolidPoint(obj->pos[X], obj->pos[Y]);
	  						if (obj->pos[Z] - BIRD_CRUISINGALTITUDE >= highest)
	  						{
	  							//corec.Console_DPrintf("We're at cruising altitude!\n");
	  							obj->direction[Z] = M_Randnum(-1,1);
	  							obj->flags &= ~BIRD_TAKINGOFF;
	  						}
	  						else if (obj->pos[Z] <= highest)
	  						{
	  							//corec.Console_DPrintf("We've landed!\n");
	  							obj->pos[Z] = highest + 0.01;
	  							obj->moveState = AS_IDLE;
	  							obj->flags &= ~BIRD_LANDING;
	  							obj->flags &= ~BIRD_CHASING;
	  							obj->speed = 0;
	  						}
	  
	  					}
	  					else if (M_Randnum(0, 1) < 0.05)
	  					{
	  						//corec.Console_DPrintf("I'm tired of flying, let's land!\n");
	  						obj->flags |= BIRD_LANDING;
	  						obj->direction[Z] -= 0.8;
	  					}
	  					else
	  					{
	  						//otherBird = CL_FindCloseBirds(obj->pos, 100, &dist);
	  						//if (otherBird != obj->index
	  						//    && otherBird >= 0
	  						//    && M_Randnum(0, 1) > 0.6)
	  						//{
	  						//	//corec.Console_DPrintf("Ooh, let's chase that bird!\n");
	  						//	obj->flags |= BIRD_CHASING;
	  						//	obj->chasingBird = otherBird;
	  						//}
	  						//else
	  						//{
	  							//just fly around
	  							obj->direction[Z] += M_Randnum(-0.5, 0.5);
	  							obj->speed += M_Randnum(-0.04, 0.04);
	  						//}
	  					}
	  					break;
	  	}
	}
	//we want them to occasionally turn a bit while they're on the ground
	if (obj->moveState == AS_IDLE)
	{
		obj->direction[Z] += M_Randnum(-0.1, 0.1);
	}
	else
	{
		if (obj->speed)
		{
			M_MultVec3(obj->direction, obj->speed, velocity);
			M_MultVec3(velocity, corec.FrameSeconds()*cl_env_timescale.value, velocity);
			M_AddVec3(obj->pos, velocity, obj->pos);
		}
	}

	if (obj->pos[X] > cl.worldbounds[X]
		|| obj->pos[Y] > cl.worldbounds[Y])
		obj->active = false;
}
*/
				
void	CL_UpdateButterfly(env_object_t *obj)
{
	vec3_t velocity;
	float highest;

	if (obj->moveState == AS_IDLE
		&& M_GetDistanceSq(cl.objects[cl.clientnum].base.pos, obj->pos) < 80*80)
	{
		//evade!	
		obj->moveState = AS_WALK_FWD;
		obj->flags |= BIRD_TAKINGOFF;
		obj->speed = M_Randnum(2, 4);
		obj->direction[X] = M_Randnum(0, 1);
		obj->direction[Y] = M_Randnum(0, 1);
		obj->direction[Z] = TAKEOFF_VELOCITY;
		obj->animTime = 0;
	}
	else if (obj->nextDecision < cl.gametime)
	{
		obj->nextDecision = cl.gametime + BUTTERFLY_DECISIONTIME;

		switch (obj->moveState)
		{
			case AS_IDLE:
					if (M_Randnum(0, 1) < 0.004)
					{
						//corec.Console_DPrintf("Taking off!\n");
						obj->moveState = AS_WALK_FWD;
						obj->flags |= BIRD_TAKINGOFF;
						obj->speed = M_Randnum(2, 4);
						obj->direction[X] = M_Randnum(0, 1);
						obj->direction[Y] = M_Randnum(0, 1);;
						obj->direction[Z] = TAKEOFF_VELOCITY;
						obj->animTime = 0;
					}
					break;
			case AS_WALK_FWD:			
					obj->direction[Y] += M_Randnum(-0.2, 0.2);
					obj->direction[X] += M_Randnum(-0.2, 0.2);

					if (obj->flags & BIRD_LANDING)
					{
						//have we hit the ground?
						highest = CL_GetHighestSolidPoint(obj->pos[X], obj->pos[Y]);
						if (obj->pos[Z] <= highest)
						{
							//corec.Console_DPrintf("We've landed!\n");
							obj->pos[Z] = highest + 0.01;
							obj->moveState = AS_IDLE;
							obj->flags &= ~BIRD_LANDING;
							obj->flags &= ~BIRD_CHASING;
							obj->speed = 0;
							obj->animTime = 0;
						}
						else
						{
							//we're not close to the ground, let's drop and slow down
							obj->direction[Z] -= 0.7;
						}
					}
					else if (obj->flags & BIRD_TAKINGOFF)
					{
						//are we really high up?
						highest = CL_GetHighestSolidPoint(obj->pos[X], obj->pos[Y]);
						if (obj->pos[Z] - BUTTERFLY_CRUISINGALTITUDE >= highest)
						{
							//corec.Console_DPrintf("We're at cruising altitude!\n");
							obj->direction[Z] = M_Randnum(-0.1,0.1);
							obj->flags &= ~BIRD_TAKINGOFF;
						}
						else if (obj->pos[Z] <= highest)
						{
							//corec.Console_DPrintf("We've landed!\n");
							obj->pos[Z] = highest + 0.01;
							obj->moveState = AS_IDLE;
							obj->flags &= ~BIRD_LANDING;
							obj->flags &= ~BIRD_CHASING;
							obj->speed = 0;
							obj->animTime = 0;
						}

					}
					else if (M_Randnum(0, 1) < 0.05)
					{
						//corec.Console_DPrintf("I'm tired of flying, let's land!\n");
						obj->flags |= BIRD_LANDING;
						obj->direction[Z] -= 2;
					}
					else
					{
						//just fly around
						obj->direction[Z] += M_Randnum(-0.3, 0.3);
						obj->speed = MIN(1.2, obj->speed + M_Randnum(-0.08, 0.08));
					}
					break;
		

		}
	}
	//we want them to occasionally turn a bit while they're on the ground
	if (obj->moveState == AS_IDLE)
	{
		obj->direction[Z] += M_Randnum(-1, 1);
	}
	else
	{
		M_Normalize(obj->direction);
		M_MultVec3(obj->direction, obj->speed, velocity);
		M_MultVec3(velocity, corec.FrameSeconds()*cl_env_timescale.value, velocity);
		M_AddVec3(obj->pos, velocity, obj->pos);
	}

	if (obj->pos[X] > cl.worldbounds[X]
		|| obj->pos[Y] > cl.worldbounds[Y])
		obj->active = false;
}

void	CL_UpdateEnvironmentEffects()
{
	int i;
	vec3_t pos;

	if (!cl_showEnvEffects.value)
		return;
	
	if (M_Randnum(0,1) > 0.94)
	{
		M_SetVec3(pos, 
				M_Randnum(0, cl.worldbounds[X]),
				M_Randnum(0, cl.worldbounds[Y]),
				0);
		pos[Z] = CL_GetHighestSolidPoint(pos[X], pos[Y]);
		
		/*if (M_Randnum(0,1) > 0.5)
			CL_AddEnvironmentEffect(ENV_BIRD, pos, false);
		else*/
			CL_AddEnvironmentEffect(ENV_BUTTERFLY, pos, false);
	}
	
	for (i = 0; i < MAX_CLIENT_ENVIRONMENT_OBJECTS; i++)
	{
		if (!env_objects[i].active)
			continue;

		switch (env_objects[i].type)
		{
			/*case ENV_BIRD:
				CL_UpdateBird(&env_objects[i]);
				break;*/
			case ENV_BUTTERFLY:
				CL_UpdateButterfly(&env_objects[i]);
				break;
		}
		env_objects[i].animTime += corec.FrameMilliseconds();
	}
}

void	CL_InitEnvironment()
{
	int i;
	vec3_t pos;
	
	memset(env_objects, 0, sizeof(env_objects));

	/*for (i = 0; i < MAX_CLIENT_ENVIRONMENT_OBJECTS/2; i++)
	{
		M_SetVec3(pos, 
				M_Randnum(0, cl.worldbounds[X]),
				M_Randnum(0, cl.worldbounds[Y]),
				0);
		pos[Z] = CL_GetHighestSolidPoint(pos[X], pos[Y]) + 0.1;
		
		CL_AddEnvironmentEffect(ENV_BIRD, pos, false);
	}*/
	
	//for (i=MAX_CLIENT_ENVIRONMENT_OBJECTS/2; i < MAX_CLIENT_ENVIRONMENT_OBJECTS; i++)
	for (i=0; i < MAX_CLIENT_ENVIRONMENT_OBJECTS/2; i++)
	{
		M_SetVec3(pos, 
				M_Randnum(0, cl.worldbounds[X]),
				M_Randnum(0, cl.worldbounds[Y]),
				0);
		pos[Z] = CL_GetHighestSolidPoint(pos[X], pos[Y]) + 0.1;
		
		CL_AddEnvironmentEffect(ENV_BUTTERFLY, pos, false);
	}

	//corec.Cvar_Register(&cl_birdModel);
	//corec.Cvar_Register(&cl_birdSkin);
	corec.Cvar_Register(&cl_butterflyModel);
	corec.Cvar_Register(&cl_butterflySkin);
	corec.Cvar_Register(&cl_env_timescale);
	corec.Cvar_Register(&cl_environmentFalloff);
	corec.Cvar_Register(&cl_showEnvEffects);
	corec.Cvar_Register(&cl_maxButterflies);

#if 0
	weather = corec.Cvar_Find("weather");
	
	CL_ReloadEnvironmentEffect(corec.Cvar_GetString("weather"));
#endif
	
	//birdModel = corec.Res_LoadModel(cl_birdModel.string);
	//birdSkin = corec.Res_LoadSkin(birdModel, cl_birdSkin.string);
	butterflyModel = corec.Res_LoadModel(cl_butterflyModel.string);
	butterflySkin = corec.Res_LoadSkin(butterflyModel, cl_butterflySkin.string);
}
