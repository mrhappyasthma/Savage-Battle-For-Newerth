// (C) 2001 S2 Games

// cl_effects.c


#include "client_game.h"


/*=============================================================================
=============================================================================*/

#define	MAX_EFFECT_OBJECTS	2048
clientEffectObject_t	effectObjects[MAX_EFFECT_OBJECTS];

/*==========================

  CL_StartEffect

  Creates an effect object for the client.  This effect will be active until CL_StopEffect is
  called on it, or the effect expires due to it's lifetime or parent expiring.
  Returns index to new effec object

 ==========================*/

int	CL_StartEffect(clientObject_t *parent, const char *bone, vec3_t origin, int effect, int param1, int param2)
{
	int index, oldest = cl.gametime, replaceme;

	if (effect < 1 || effect >= MAX_EXEFFECTS)
		return 0;

	if (!origin && !parent)
	{
		corec.Console_DPrintf("CL_StartEffect: Effect #%i with no parent or possition!\n", effect);
		return -1;
	}

	//find an inactive effectObject (or oldest active)
	for (index = 1; index < MAX_EFFECT_OBJECTS; index++)
	{
		if (!effectObjects[index].active)
		{
			replaceme = index;
			break;
		}
		else if (effectObjects[index].timestamp < oldest)
		{
			oldest = effectObjects[index].timestamp;
			replaceme = index;
		}
	}

	effectObjects[replaceme].active			= true;
	effectObjects[replaceme].index			= replaceme;
	effectObjects[replaceme].effect			= effect;
	effectObjects[replaceme].parent			= parent;
	
	if (parent)
	{
		if (parent->visual.flags & BASEOBJ_USE_TRAJECTORY)
		{
			effectObjects[replaceme].timestamp	= parent->visual.traj.startTime;
			M_CopyVec3(parent->visual.traj.origin, effectObjects[replaceme].pos);
		}
		else
		{
			effectObjects[replaceme].timestamp	= cl.gametime;
			M_CopyVec3(parent->visual.pos, effectObjects[replaceme].pos);
		}
	}
	else
	{
		M_CopyVec3(origin, effectObjects[replaceme].pos);
	}
	
	for (index = 0; index < MAX_SUB_EFFECTS; index++)
	{
		if (!exEffectData[effect].subEffect[index].type)
		{
			effectObjects[replaceme].subactive[index]	= false;
			continue;
		}

		effectObjects[replaceme].subactive[index]	= true;
		effectObjects[replaceme].starttime[index]	= cl.gametime + GETRANGE(exEffectData[effect].subEffect[index].delay);
		effectObjects[replaceme].last[index]		= cl.gametime;
		effectObjects[replaceme].expiretime[index]	= effectObjects[replaceme].starttime[index] + GETRANGE(exEffectData[effect].subEffect[index].duration);

		if (exEffectData[effect].subEffect[index].replays.min == 0 &&
			exEffectData[effect].subEffect[index].replays.max == 0)
			effectObjects[replaceme].plays[index] = -1;
		else
			effectObjects[replaceme].plays[index] = GETRANGE(exEffectData[effect].subEffect[index].replays);
	}

	M_CopyVec3(effectObjects[replaceme].pos, effectObjects[replaceme].lastpos);

	return replaceme;
}



/*==========================

  CL_DoEffectSounds

  Plays sounds for an effect

 ==========================*/

void	CL_DoEffectSounds(clientEffectObject_t	*efobj, subEffectData_t *sfx)
{
#if 0
	int max;

	for (max = 0; max < MAX_SOUND_VARIATIONS; max++)
	{
		if (strlen(sfx->sound[max]) == 0)
			break;
	}

	if (max)
	{
		int n;
		residx_t	snd;

		n = rand() % max;
		snd = corec.Res_LoadSound(sfx->sound[n]);
		if (snd)
		{
			/*if (sfx->loop && efobj->parent)
			{
				if (efobj->parent->playingLoopSound)
				{
					corec.Console_DPrintf("Too many looping sounds for object %i\n", efobj->parent->visual.index);
					return;
				}

				efobj->parent->playingLoopSound = snd;
				corec.SC_PlayObjectSound(efobj->parent->visual.index, snd, 1, LOOP_CHANNEL, true);
				corec.Sound_SetMinDistance(snd, sfx->radius);
			}
			else*/
			{
				//CL_PlaySound3d(snd, efobj->pos, sfx->volume);
				corec.SC_PlayObjectSound(efobj->parent->visual.index, snd, 1, 3/*LOOP_CHANNEL*/, true);
				corec.Sound_SetMinDistance(snd, sfx->radius);
			}
		}
	}
#endif
}


/*==========================

  CL_DoEmitter

  Handles emitter sub effects spawning particles

 ==========================*/

void	CL_DoEmitter(clientEffectObject_t *efobj, subEffectData_t *sfx, int currenttime)
{
	int		index, n;
	vec3_t	vel;
	complexParticle_t	*part;

	for (index = 0; index < GETRANGE(sfx->count); index++)
	{
		part = CL_AllocParticleEx();
		if (!part)
			break;

		part->efobj = efobj;

		M_CopyVec3(efobj->pos, part->position);

		part->deathtime = cl.gametime + GETRANGE(sfx->lifetime);

		part->shader = corec.Res_LoadShader(sfx->shader);
		part->model = corec.Res_LoadModel(sfx->model);
		if (part->model)
			part->skin = corec.Res_LoadSkin(part->model, sfx->skin);

		M_CopyVec3(efobj->parent->visual.traj.velocity, vel);
		M_MultVec3(vel, GETRANGE(sfx->inertia), part->velocity);
		part->inertia = GETRANGE(sfx->inertia);

		//scale
		for (n = 0; n < 4; n++)
			part->scale[n] = GETRANGE(sfx->size[n]);
		for (n = 0; n < 3; n++)
			part->scalebias[n] = GETRANGE(sfx->sizeBias[n]);
		part->scaleKeys = sfx->sizeNumKeys;

		//angle
		for (n = 0; n < 4; n++)
			part->angle[n] = GETRANGE(sfx->angle[n]);
		for (n = 0; n < 3; n++)
			part->anglebias[n] = GETRANGE(sfx->angleBias[n]);
		part->angleKeys = sfx->angleNumKeys;

		//red
		for (n = 0; n < 4; n++)
			part->red[n] = GETRANGE(sfx->red[n]);
		for (n = 0; n < 3; n++)
			part->redbias[n] = GETRANGE(sfx->redBias[n]);
		part->redKeys = sfx->redNumKeys;

		//green
		for (n = 0; n < 4; n++)
			part->green[n] = GETRANGE(sfx->green[n]);
		for (n = 0; n < 3; n++)
			part->greenbias[n] = GETRANGE(sfx->greenBias[n]);
		part->greenKeys = sfx->greenNumKeys;

		//blue
		for (n = 0; n < 4; n++)
			part->blue[n] = GETRANGE(sfx->blue[n]);
		for (n = 0; n < 3; n++)
			part->bluebias[n] = GETRANGE(sfx->blueBias[n]);
		part->blueKeys = sfx->blueNumKeys;

		//alpha
		for (n = 0; n < 4; n++)
			part->alpha[n] = GETRANGE(sfx->alpha[n]);
		for (n = 0; n < 3; n++)
			part->alphabias[n] = GETRANGE(sfx->alphaBias[n]);
		part->alphaKeys = sfx->alphaNumKeys;
	}
}


/*==========================

  CL_StopEffect

  Just manually sets an effect to inactive

 ==========================*/

void	CL_StopEffect(int index)
{
	effectObjects[index].active = false;
	
	/*if (effectObjects[index].parent)
	{
		if(!effectObjects[index].parent->playingLoopSound)
			return;
		effectObjects[index].parent->playingLoopSound = 0;
		corec.SC_StopObjectChannel(effectObjects[index].parent->visual.index, LOOP_CHANNEL);
	}*/
}


/*==========================

  CL_EffectFrame

  Generates the sound and visuals for an effect

 ==========================*/

void	CL_EffectFrame(clientEffectObject_t *efobj)
{
	exEffectData_t	*efData;
	int				index, basetime;
	vec3_t			vec, pos;

	////
	//update positional information
	////

	//save lastpos
	M_CopyVec3(efobj->pos, efobj->lastpos);
		
	//determine new pos from parent, or trajectory
	if (efobj->parent)
	{
		if (efobj->parent->visual.flags & BASEOBJ_USE_TRAJECTORY)
			Traj_GetPos(&efobj->parent->visual.traj, cl.trajectoryTime, efobj->pos);
		else
			M_CopyVec3(efobj->parent->visual.pos, efobj->pos);
	}
	else
	{
		//use the effect objects velocities
	}
		
	efData = &exEffectData[efobj->effect];
	for (index = 0; index < MAX_SUB_EFFECTS; index++)
	{
		if (!efobj->subactive[index])
			continue;

		if (efobj->starttime[index] > cl.gametime)
			continue;

		if ((efData->subEffect[index].duration.max > 0 && efobj->expiretime[index] <= cl.gametime) 
			|| efobj->plays[index] == 0)
		{
			efobj->subactive[index] = false;
			continue;
		}

		basetime = efobj->last[index];
		M_CopyVec3(efobj->pos, pos);

		while (efobj->last[index] < cl.gametime)
		{
			M_SubVec3(efobj->pos, efobj->lastpos, vec);
			M_MultVec3(vec, (efobj->last[index] - basetime) / (float)(cl.gametime - basetime), vec);
			M_AddVec3(efobj->lastpos, vec, efobj->pos);

			efobj->last[index] += GETRANGE(efData->subEffect[index].frequency);
			
			if (efobj->plays[index] == 0)
				break;

			if (efobj->plays[index] > 0)
				efobj->plays[index]--;

			switch (efData->subEffect[index].type)
			{
			case SUB_EFX_SOUND:
				CL_DoEffectSounds(efobj, &efData->subEffect[index]);
				break;

			case SUB_EFX_EMITTER:
				CL_DoEmitter(efobj, &efData->subEffect[index], efobj->last[index]);
				break;

			case SUB_EFX_ATTRACTOR:
				break;

			case SUB_EFX_TRACER:
				break;

			default:
				break;
			}

			M_CopyVec3(pos, efobj->pos);
		}
	}

	//determine new pos from parent, or trajectory
	if (efobj->parent)
		M_CopyVec3(efobj->parent->visual.pos, efobj->pos);
		
	//if the parent object has become inactive, kill the effect
	if (efobj->parent)
	{
		if (!efobj->parent->visual.active)
			CL_StopEffect(efobj->index);
	}

	//if no sub effects are active, stop the effect
	for (index = 0; index < MAX_SUB_EFFECTS; index++)
	{
		if (efobj->subactive[index])
			return;
	}

	if (efobj->parent)
	{
		if (!efobj->parent->playingLoopSound)
		{
			CL_StopEffect(efobj->index);
		}
	}
	else
	{
		CL_StopEffect(efobj->index);
	}
}


/*==========================

  CL_ProcessEffects

  Steps through and updates all effect objects

 ==========================*/

void	CL_ProcessEffects()
{
	int	index;

	for (index = 1; index < MAX_EFFECT_OBJECTS; index++)
	{
		if (!effectObjects[index].active)
			continue;

		CL_EffectFrame(&effectObjects[index]);
	}
}


/*==========================

  CL_SpawnEffect_Cmd

  Debugging command to create an arbitrary effect

 ==========================*/

void	CL_SpawnEffect_Cmd(int argc, char *argv[])
{
	int		effect, param, param2, id;
	vec3_t	pos;

	if (argc < 1)
		return;

	SetInt(&effect, 1, MAX_EXEFFECTS - 1, exEffectNames, argv[0]);

	if (effect < 0 || effect >= MAX_EXEFFECTS)
	{
		corec.Console_DPrintf("Invalid effect\n");
		return;
	}

	if (argc > 1)
		param = atoi(argv[1]);
	if (argc > 2)
		param2 = atoi(argv[2]);

	M_CopyVec3(cl.player->visual.pos, pos);
	id = CL_StartEffect(NULL, NULL, pos, effect, (byte)param, (byte)param2);

	corec.Console_DPrintf("Spawned effect #%i\n", id);
}


/*==========================

  CL_KillEffect_Cmd

  Manually kill an effect object from the console

 ==========================*/

void	CL_KillEffect_Cmd(int argc, char *argv[])
{
	int effect;

	if (argc < 1)
		return;

	effect = atoi(argv[0]);

	if (effect >= 0 && effect < MAX_EFFECT_OBJECTS)
		CL_StopEffect(effect);
}

void	CL_DoEffect(clientObject_t *obj, const char *bone, vec3_t pos, int effect, int param, int param2)
{
	PERF_BEGIN;

	if (effect >= MAX_EXEFFECTS)
	{
		effect -= MAX_EXEFFECTS;
		CL_OldEffect(obj, bone, effect, (byte)param, (byte)param2);
	}
	else
	{
		CL_StartEffect(obj, bone, pos, effect, param, param2);
	}

	PERF_END(PERF_DOEFFECT);
}

extern cvar_t cl_showEvents;

void	CL_OldEffect(clientObject_t *obj, const char *bone, int effect, byte param, byte param2)
{
	effectData_t	*ef = &effectData[effect];
	viseffectData_t	*visualinfo;
	residx_t		snd;	
	float			length = 0;
	vec3_t			line;

	int numparts;
	int n, index, max;

	if (effect > MAX_EFFECTS)
	{
		corec.Console_Errorf("unknown effect\n");
		return;
	}

	if (cl_showEvents.integer)
	{
		corec.Console_Printf("Oldeffect: %s\n", ef->name);
	}

	//camera shake
	//	
	if (ef->shakeScale > 0)
	{
		float proximity = sqrt(M_GetDistanceSq(obj->visual.pos, cl.camera.origin));
		float duration = ef->shakeDuration;
		float scale = ef->shakeScale;

		if (proximity < ef->shakeRadius)
		{
			if (proximity > ef->shakeRadius / 1.5)
				scale *= 0.25;
			else if (proximity > ef->shakeRadius / 2)
				scale *= 0.5;

			CL_CameraShake(duration, ef->shakeFrequency, scale, scale, scale);
		}
	}

	//sound (picks a random sound from any of the three that have an entry)
	//
	for (max = 0; max < 4; max++)
	{
		if (!ef->sound[max][0])
			break;
	}

	if (max)
	{		
		n = rand() % max;
		snd = corec.Res_LoadSound(ef->sound[n]);
		if (snd)
		{
			CL_PlaySound3d(snd, NULL, obj->visual.pos, GETRANGE(ef->soundVolume), CHANNEL_AUTO);
		}
	}

	//visuals
	//

	//don't do visuals if the sceneobject wasn't computed for a few frames (out of camera view)
//	if (cl.frame - obj->lastSCFrame > 4)
	//	return;

	for (index = 0; index < 4; index++)
	{
		visualinfo = &ef->visuals[index];

		//beam
		//
		if (ef->visuals[index].vfclass == VFCLASS_BEAM)
		{
			beam_t	*beam;
			vec3_t	start, end;

			M_CopyVec3(obj->oldbase.pos, start);
			M_CopyVec3(obj->visual.pos, end);

			beam = CL_AddBeam(start, end, corec.Res_LoadShader(visualinfo->shader));
			beam->alpha = GETRANGE(visualinfo->alpha);
			beam->alpha = MIN(1.0, beam->alpha);
			beam->alpha = MAX(0.0, beam->alpha);

			beam->deathalpha = beam->alpha + GETRANGE(visualinfo->fade);
			beam->deathalpha = MIN(1.0, beam->deathalpha);
			beam->deathalpha = MAX(0.0, beam->deathalpha);
			
			beam->size = GETRANGE(visualinfo->size);
			beam->size = MAX(0.0, beam->size);
			beam->deathsize = beam->size + GETRANGE(visualinfo->growth);
			beam->deathsize = MAX(0.0, beam->deathsize);

			beam->deathtime = cl.gametime + GETRANGE(visualinfo->lifetime);
			beam->tilelength = visualinfo->tilelength;
			if (visualinfo->red.max == 0 && visualinfo->green.max == 0 && visualinfo->blue.max == 0)
			{
				beam->red = beam->green = beam->blue = GETRANGE(visualinfo->white);
				beam->deathred = beam->deathgreen = beam->deathblue = beam->red + GETRANGE(visualinfo->whitefade);
			}
			else
			{
				beam->red = GETRANGE(visualinfo->red);
				beam->green = GETRANGE(visualinfo->green);
				beam->blue = GETRANGE(visualinfo->blue);
				beam->deathred = beam->red + GETRANGE(visualinfo->redfade);
				beam->deathgreen = beam->green + GETRANGE(visualinfo->greenfade);
				beam->deathblue = beam->blue + GETRANGE(visualinfo->bluefade);
			}
		}

		//particles
		//
		else if (ef->visuals[index].vfclass == VFCLASS_PARTICLE)
		{
			if (visualinfo->flags & PARTICLE_FLAG_PARAM_COUNT)
				numparts = GETRANGE(visualinfo->count) * (param2 / 255.0);
			else
				numparts = GETRANGE(visualinfo->count);

			if (visualinfo->style == PARTICLE_STYLE_TRAIL)
			{
				M_SubVec3(obj->visual.pos, obj->oldbase.pos, line);
				length = M_Normalize(line);

				if (visualinfo->tilelength > 0)
					numparts *= (length / visualinfo->tilelength);
			}

			for (n = 0; n < numparts; n++)
			{
				vec3_t				vel;//, accel;
				vec3_t				v;
				simpleParticle_t	*part;
				//pointinfo_t			pi;

				part = CL_AllocParticle();

				if (!part)
					break;
			
				//flags
				part->flags = visualinfo->flags;

				//parent
				part->parent = obj->base.index;

				//position
				switch (visualinfo->style)
				{
				case PARTICLE_STYLE_DIRECTIONAL:
					if (visualinfo->flags & PARTICLE_FLAG_PARENT_POS)
						M_SetVec3(part->pos, 0, 0, 0);
					else
						M_CopyVec3(obj->oldbase.pos, part->pos);

					part->pos[2] += (obj->visual.bmax[2] + obj->visual.bmin[2]) / 2;
					break;

				case PARTICLE_STYLE_POINT:
					M_SetVec3(v, M_Randnum(-1,1), M_Randnum(-1,1), M_Randnum(-1,1));
					M_Normalize(v);
					M_MultVec3(v, GETRANGE(visualinfo->offset), v);
					
					if (visualinfo->flags & PARTICLE_FLAG_PARENT_POS)
					{
						if (visualinfo->flags & PARTICLE_FLAG_PARAM_LOCATION)
						{
							CL_GetPositionOnObject(obj, param, v);
							M_SubVec3(v, obj->visual.pos, part->pos);
						}
						else
						{
							M_CopyVec3(v, part->pos);
							part->pos[2] += (obj->visual.bmax[2] + obj->visual.bmin[2]) / 2;
						}
					}
					else
					{
						if (visualinfo->flags & PARTICLE_FLAG_PARAM_LOCATION)
						{
							CL_GetPositionOnObject(obj, param, part->pos);
						}
						else
						{
							if (strlen(visualinfo->bone) > 0)
							{
								vec3_t	bonepos, boneaxis[3];

								CL_GetBoneWorldTransform(bone, obj->cachedSC.pos, obj->cachedSC.axis, obj->cachedSC.scale, obj->cachedSC.skeleton, bonepos, boneaxis);
								M_AddVec3(bonepos, v, part->pos);
								M_AddVec3(part->pos, obj->visual.pos, part->pos);
								//part->pos[2] += (obj->visual.bmax[2] + obj->visual.bmin[2]) / 2;
							}
							else
							{
								if (bone)
								{
									vec3_t	boneaxis[3];

									CL_GetBoneWorldTransform(bone, obj->cachedSC.pos, obj->cachedSC.axis, obj->cachedSC.scale, obj->cachedSC.skeleton, part->pos, boneaxis);
									
									M_AddVec3(part->pos, v, part->pos);									
								}
								else
								{
									M_CopyVec3(obj->visual.pos, part->pos);
									//part->pos[2] += (obj->visual.bmax[2] + obj->visual.bmin[2]) / 2;
								}
								M_AddVec3(part->pos, v, part->pos);
								
							}

							if (visualinfo->flags & PARTICLE_FLAG_PARAM_WORLDZ)
							{
								/*
								corec.World_SampleGround(part->pos[X], part->pos[Y], &pi);
								part->pos[Z] = pi.z;
								*/
								part->pos[Z] = CL_GetHighestSolidPoint(part->pos[X], part->pos[Y]);
							}

							if (visualinfo->flags & PARTICLE_FLAG_PARAM_SINEWAVE)
							{
								part->fps = GETRANGE(visualinfo->fps);
								part->param = GETRANGE(visualinfo->altOffset);
							}

							if (visualinfo->flags & PARTICLE_FLAG_PARENT_RADIUS)
							{
								part->param2 = visualinfo->maxRange * visualinfo->maxRange;
							}

							part->minZ = visualinfo->minZ + part->pos[Z] - v[Z];
							part->maxZ = visualinfo->maxZ + part->pos[Z] - v[Z];

							part->direction = M_Randnum(0,1);
						}
					}
					break;

				case PARTICLE_STYLE_TRAIL:
					if (visualinfo->tilelength == 0)
					{
						//set origin to a random point on the line
						M_MultVec3(line, length * M_Randnum(0.0, 1.0), v);
						M_AddVec3(obj->oldbase.pos, v, part->pos);
					}
					else
					{
						//even distribution
						float spacing;

						spacing = (length / (float)numparts);
						M_MultVec3(line, spacing * n, v);
						M_AddVec3(obj->oldbase.pos, v, part->pos);
					}

					//offset from line
					M_SetVec3(v, M_Randnum(-100,100), M_Randnum(-100,100), M_Randnum(-100,100));
					M_Normalize(v);
					M_MultVec3(v, GETRANGE(visualinfo->offset), v);
					M_AddVec3(part->pos, v, part->pos);
					break;

					break;

				default:
					corec.Console_DPrintf("particle type %i not handled!\n", visualinfo->style);
				}

				//velocity
				switch (visualinfo->style)
				{
				case PARTICLE_STYLE_POINT:
				case PARTICLE_STYLE_TRAIL:
					M_SetVec3(vel, M_Randnum(-100,100), M_Randnum(-100,100), M_Randnum(-100,100));
					M_Normalize(vel);
					M_MultVec3(vel, GETRANGE(visualinfo->velocity), part->velocity);
					break;

				case PARTICLE_STYLE_DIRECTIONAL:
					//M_SubVec3(obj->visual.pos, obj->oldbase.pos, vel);
					if (obj->visual.index == cl.clientnum)
						HeadingFromAngles(cl.predictedState.angle[PITCH], cl.predictedState.angle[YAW], vel, NULL);
					else
						HeadingFromAngles(obj->visual.angle[0], obj->visual.angle[2], vel, NULL);

					M_Normalize(vel);
					M_MultVec3(vel, GETRANGE(visualinfo->velocity), part->velocity);
					break;
				}

				//acceleration
				part->accel = GETRANGE(visualinfo->acceleration);

				//gravity
				part->gravity = DEFAULT_GRAVITY * PHYSICS_SCALE * GETRANGE(visualinfo->gravity);

				//size
				if (visualinfo->flags & PARTICLE_FLAG_PARAM_SCALE)
					part->width = GETRANGE(visualinfo->size) * (param2 / 255.0);
				else
					part->width = GETRANGE(visualinfo->size);

				if (visualinfo->sizeHeight.min == 0 && visualinfo->sizeHeight.max == 0)
					part->height = part->width;
				else
				{
					if (visualinfo->flags & PARTICLE_FLAG_PARAM_SCALE)
						part->height = GETRANGE(visualinfo->sizeHeight) * (param2 / 255.0);
					else
						part->height = GETRANGE(visualinfo->sizeHeight);
				}
				
				if (visualinfo->flags & PARTICLE_FLAG_PARAM_SCALE)
					part->deathwidth = part->width + GETRANGE(visualinfo->growth) * (param2 / 255.0);
				else
					part->deathwidth = part->width + GETRANGE(visualinfo->growth);

				if (visualinfo->sizeHeight.min == 0 && visualinfo->sizeHeight.max == 0)
					part->deathheight = part->deathwidth;
				else
				{
					if (visualinfo->flags & PARTICLE_FLAG_PARAM_SCALE)
						part->deathheight = part->height + GETRANGE(visualinfo->growth) * (param2 / 255.0);
					else
						part->deathheight = part->height + GETRANGE(visualinfo->growth);
				}

				//alpha
				part->alpha = GETRANGE(visualinfo->alpha);
				part->deathalpha = part->alpha + GETRANGE(visualinfo->fade);

				//colors
				if (visualinfo->red.max == 0 && visualinfo->green.max == 0 && visualinfo->blue.max == 0)
				{
					part->red = part->green = part->blue = GETRANGE(visualinfo->white);
					part->deathred = part->deathgreen = part->deathblue = part->red + GETRANGE(visualinfo->whitefade);
				}
				else
				{
					part->red = GETRANGE(visualinfo->red);
					part->green = GETRANGE(visualinfo->green);
					part->blue = GETRANGE(visualinfo->blue);
					part->deathred = part->red + GETRANGE(visualinfo->redfade);
					part->deathgreen = part->green + GETRANGE(visualinfo->greenfade);
					part->deathblue = part->blue + GETRANGE(visualinfo->bluefade);
				}

				part->angle = GETRANGE(visualinfo->angle);
				part->deathAngle = part->angle + GETRANGE(visualinfo->spin);
				
				//appearance
				part->birthDelay = GETRANGE(visualinfo->delay);
				part->deathtime = cl.gametime + GETRANGE(visualinfo->lifetime);
				if (visualinfo->shader[0])
					part->shader = corec.Res_LoadShader(visualinfo->shader);
				if (visualinfo->model[0])
				{
					part->model = corec.Res_LoadModel(visualinfo->model);
					if (visualinfo->skin[0])
						part->skin = corec.Res_LoadSkin(part->model, visualinfo->skin);
				}
				//CL_PlayAnimOnce(part, GETRANGE(partinfo->fps), part->shader);

				if (part->deathtime != cl.gametime)
					CL_AddParticle(part);
			}
		}
	}
}


void	CL_ResetEffects()
{
	int i;

	for (i = 0; i < MAX_EFFECT_OBJECTS; i++)
		CL_StopEffect(i);
}

/*==========================

  CL_InitEffects

  ==========================*/

void	CL_InitEffects()
{
	CL_ResetEffects();

	corec.Cmd_Register("killEffect", CL_KillEffect_Cmd);
	corec.Cmd_Register("spawnEffect", CL_SpawnEffect_Cmd);
}
