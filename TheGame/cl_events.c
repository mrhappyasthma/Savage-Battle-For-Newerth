// (C) 2001 S2 Games

// cl_events.c


#include "client_game.h"

cvar_t	cl_showEvents =					{ "cl_showEvents",				"0" };
cvar_t	cl_showAnimEvents =				{ "cl_showAnimEvents",			"0" };
cvar_t	cl_cameraJitterScale =			{ "cl_cameraJitterScale",		"10.0" };
cvar_t	cl_cameraJitterDuration =		{ "cl_cameraJitterDuration",	"0.3" };

cvar_t	cl_lerpObjectAngles =			{ "cl_lerpObjectAngles",		"1" };
cvar_t	cl_useTrajectory =				{ "cl_useTrajectory",			"1", CVAR_CHEAT };
cvar_t	cl_linkHack =					{ "cl_linkHack",				"0", CVAR_CHEAT };
cvar_t	cl_bloodThreshhold =			{ "cl_bloodThreshhold",			"0" };
cvar_t	cl_lerpShift =					{ "cl_lerpShift",				"0.4", CVAR_VALUERANGE | CVAR_SAVECONFIG, 0, 1 };

cvar_t	cl_debugUpdates =				{ "cl_debugUpdates",			"0" };

extern cvar_t cl_lerpObjects;
extern cvar_t cl_defaultSoundVolume;




/*==========================

  CL_GetPositionOnObject

  take the encoded position and translate it to an actual coordinate

 ==========================*/

void	CL_GetPositionOnObject(clientObject_t *obj, byte encodedPos, vec3_t out)
{
	//get the normal
	M_ByteToNormal(encodedPos, out);
	//project it onto the bounding box
	M_ProjectDirOnBounds(out, obj->base.bmin, obj->base.bmax, out);
	//add the object position
	M_AddVec3(out, obj->visual.pos, out);
}

void	CL_PlaySound3d(residx_t sound, clientObject_t *obj, vec3_t pos, float volume, int channel)
{
	int priority;


	if (!obj)
	{
		corec.Sound_Play(sound, -1, pos, volume, channel, PRIORITY_LOW);
	}
	else
	{
		//sounds emitting from ourself are always high priority
		if (obj->base.index || cl.clientnum)
			priority = PRIORITY_HIGH;
		else
			priority = PRIORITY_LOW;

		corec.Sound_Play(sound, obj->base.index, NULL, volume, channel, priority);
	}
}



void	CL_FootstepSound(clientObject_t *obj)
{	
	objectData_t *def = CL_ObjectType(obj->base.type);
	char *surface = "grass";

	CL_PlaySound3d(corec.Res_LoadSound(Snd(fmt("footstep_%s_%s_%i", surface, def->footstepType, rand()%8+1))), obj, NULL, 1.0, CHANNEL_AUTO);

	if (obj->base.index < MAX_CLIENTS)
	{
		int level = cl.clients[obj->base.index].score.level;

		//play armor sounds for leveled up players		
		if (level > 2)
		{
			if (level > 8)
			{
				//tier 3
				CL_PlaySound3d(corec.Res_LoadSound(Snd(fmt("armor_heavy_%i", rand()%4+1))), obj, NULL, 1.0, CHANNEL_AUTO);
			}
			else
			{
				//tier 1 & 2			
				CL_PlaySound3d(corec.Res_LoadSound(Snd(fmt("armor_light_%i", rand()%4+1))), obj, NULL, 1.0, CHANNEL_AUTO);
			}
		}
	}
}


/*==========================

  CL_DoObjectEvent

  Respond to an object event we received over the network

  ==========================*/

void	CL_DoObjectEvent(clientObject_t *obj, objEvent_t *event)
{
	int effect = 0;
	byte	src;
	
	if (event->type != EVENT_DROPOFF &&
		event->type != EVENT_WOUNDED &&
		event->type != EVENT_DEATH)		//special cased dropoff event uses the two params differently
		src = (event->param2) ? event->param2 : obj->base.type;
	else
		src = obj->base.type;

	//debug
	if (cl_showEvents.integer)
		corec.Console_Printf("CL_EVENT: %s with param %i, %i for #%i (%s) [%i]\n", GetEventName(event->type), event->param, event->param2, obj->base.index, cl.objNames[src], cl.gametime);

	if (event->type == EVENT_WEAPON_FIRE && !CL_ObjectType(obj->base.weapon)->continuous)
	{
		//reset the anim counter explicitly
		if (obj->base.animState >= AS_WEAPON_FIRE_1 && obj->base.animState <= AS_WEAPON_FIRE_6)
		{
			obj->newAnimState = true;
			obj->animStateTime = 0;
		}
		if (obj->base.animState2 >= AS_WEAPON_FIRE_1 && obj->base.animState <= AS_WEAPON_FIRE_6)
		{
			obj->newAnimState2 = true;
			obj->animState2Time = 0;
		}
	}
	
	// This will handle all sounds and particles.  most events probably just need this definition

	effect = CL_ObjectType(src)->effects[event->type];
	CL_DoEffect(obj, NULL, NULL, effect, event->param, event->param2);

	//
	//Anything beyond this should be a special case that an effect defintion can't handle

	switch(event->type)
	{
	case EVENT_QUAKE:
		{
			float scale = (event->param/127.0) * cl_cameraJitterScale.value;
			CL_CameraShake(cl_cameraJitterDuration.value, 250, scale, scale, scale);
			break;
		}

	case EVENT_WOUNDED:
		{
			float fullhealth = cl.player->visual.fullhealth;
			if (fullhealth)
				CL_DamageEffect(((cl.oldPlayerState.health - cl.playerstate.health) * 100) / fullhealth);
//			CL_PlayEventAnimation(obj, CL_GetAnimFromEvent(event->type, obj->base.type));
			if (IsCharacterType(obj->visual.type) && !IsAttackAnim(obj->visual.animState2) && !IsAttackAnim(obj->visual.animState) && obj->visual.health > 0)
			{
				CL_ImpulseAnim(obj, "", AS_WOUNDED_BACK, 0);
			}

			if (IsCharacterType(obj->visual.type))
			{
				vec3_t	pos;

				CL_GetPositionOnObject(obj, event->param, pos);
				if (event->param2 > MIN(255, cl_bloodThreshhold.integer + 160))
					CL_DoEffect(obj, NULL, pos, GetEffectNumber("blood3"), event->param, event->param2);
				else if (event->param2 > MIN(255, cl_bloodThreshhold.integer + 80))
					CL_DoEffect(obj, NULL, pos, GetEffectNumber("blood2"), event->param, event->param2);
				else if (event->param2 > MIN(255, cl_bloodThreshhold.integer))
					CL_DoEffect(obj, NULL, pos, GetEffectNumber("blood"), event->param, event->param2);
			}
			break;
		}
	case EVENT_JUMP_LAND:
		break;

	case EVENT_RESURRECTED:
		break;

	case EVENT_DEATH:
	case EVENT_DEATH_QUIET:
		if (IsCharacterType(obj->visual.type))
		{
			vec3_t	pos;

			CL_GetPositionOnObject(obj, event->param, pos);
			if (event->param2 > MIN(255, cl_bloodThreshhold.integer + 160))
				CL_DoEffect(obj, NULL, pos, GetEffectNumber("blood3"), event->param, event->param2);
			else if (event->param2 > MIN(255, cl_bloodThreshhold.integer + 80))
				CL_DoEffect(obj, NULL, pos, GetEffectNumber("blood2"), event->param, event->param2);
			else if (event->param2 > MIN(255, cl_bloodThreshhold.integer))
				CL_DoEffect(obj, NULL, pos, GetEffectNumber("blood"), event->param, event->param2);
		}

		obj->drawFinalTrail = 1;		//perform any final trail drawing for projectiles

		break;

	case EVENT_SPAWN:
		break;

	case EVENT_DAZED:
		break;

	case EVENT_DEFLECTED:
		break;

	case EVENT_PICKUP_WEAPON:
		CL_PlaySound3d(corec.Res_LoadSound(Snd("pickup_weapon")), obj, NULL, 1.0, CHANNEL_AUTO);
		break;

	case EVENT_ATTACK_POUND:
		{
			// $todo: only do this if the client is on the ground and within range
			float scale = (event->param/127.0) * cl_cameraJitterScale.value;
			CL_CameraShake(cl_cameraJitterDuration.value, 250, scale, scale, scale);
			break;
		}

	default:
		break;
	}
}

/*==========================

  CL_AnimEvent


 ==========================*/

void	CL_AnimEvent(clientObject_t *obj, const char *eventString)
{
	char eventstringcopy[1024];
	char *argv[32];
	int argc;

	strncpySafe(eventstringcopy, eventString, 1024);

	argc = SplitArgs(eventstringcopy, argv, 32);

	if (cl_showAnimEvents.integer)
		corec.Console_Printf("animevent: %s\n", eventString);

	if (strcmp(argv[0], "footstepleft")==0)
	{
		if (obj->base.index == cl.clientnum && !cl.thirdperson)
			return;

		CL_FootstepSound(obj);
	}
	else if (strcmp(argv[0], "footstepright")==0)
	{
		if (obj->base.index == cl.clientnum && !cl.thirdperson)
			return;

		CL_FootstepSound(obj);
	}
	else if (strcmp(argv[0], "sound")==0)
	{		
		if (argc > 1)
		{
			float volume;
			
			if (argc > 2)
			{
				volume = atof(argv[2]);
			}
			else
			{
				volume = 1.0;
			}
			
			CL_PlaySound3d(corec.Res_LoadSound(argv[1]), obj, NULL, volume, CHANNEL_AUTO);
			
			if (cl_showAnimEvents.integer)
				corec.Console_Printf("Playing %s, volume %f", argv[1], volume);
		}
	}
	else if (strcmp(argv[0], "randsound")==0)
	{
		if (argc > 2)
		{
			float volume;
			char *soundfile;
			int numsounds;

			numsounds = atoi(argv[1]);
			soundfile = fmt("%s_%i.wav", argv[2], (rand() % numsounds) + 1);

			if (argc > 3)
			{
				volume = atof(argv[3]);
			}
			else
			{
				volume = 1.0;
			}
		
			CL_PlaySound3d(corec.Res_LoadSound(soundfile), obj, NULL, volume, CHANNEL_AUTO);
		}
	}
	else if (strcmp(argv[0], "effect")==0)
	{
		int n;
		char bonename[256] = "";
		if (argc > 2)
		{
			for (n=2; n<argc; n++)
				strcat(bonename, fmt("%s%s", argv[n], n==argc-1 ? "" : " "));
		}
		CL_DoEffect(obj, argc > 2 ? bonename : NULL, NULL, GetEffectNumber(argv[1]), 0 , 0);
	}
}



/*==========================

  CL_LerpObjectForRendering

  modifies fields in the "visual" baseobject to smooth
  between server frames

 ==========================*/

void	CL_LerpObjectForRendering(clientObject_t *obj)
{
	M_CopyVec3(obj->visual.pos, obj->lastPos);

	if (obj->base.index == cl.clientnum)
		return;

	if (obj->oldbase.flags & BASEOBJ_USE_TRAJECTORY)
	{
		//if we were just using a trajectory, don't lerp on this frame
		obj->visual = obj->base;
		obj->oldbase = obj->base;
	}

	if (cl.objLerp == 1)
	{
		obj->visual = obj->base;		
		return;
	}
	else if (cl.objLerp == 0)
	{
		obj->visual = obj->oldbase;
	}

	//if lerpshift is 1, this interpolates object positions between 1 and 2 instead of 0 and 1
	//the advantage is that the object will be much closer to its current "actual" position,
	//and moving targets will be easier to hit (won't have to compensate as much)
	//the disadvantage is that movement appears slightly choppy when the interpolated object
	//doesn't match up with what gets sent from the server
	obj->visual.pos[X] = LERP(cl.objLerp + cl_lerpShift.value, obj->oldbase.pos[X], obj->base.pos[X]);
	obj->visual.pos[Y] = LERP(cl.objLerp + cl_lerpShift.value, obj->oldbase.pos[Y], obj->base.pos[Y]);
	obj->visual.pos[Z] = LERP(cl.objLerp + cl_lerpShift.value, obj->oldbase.pos[Z], obj->base.pos[Z]);
	
	//lerp angles..must use LerpAngle here, otherwise we won't turn on the smallest arc
	if (cl_lerpObjectAngles.integer)
	{
		obj->visual.angle[X] = M_LerpAngle(cl.objLerp, obj->oldbase.angle[X], obj->base.angle[X]);
		obj->visual.angle[Y] = M_LerpAngle(cl.objLerp, obj->oldbase.angle[Y], obj->base.angle[Y]);
		obj->visual.angle[Z] = M_LerpAngle(cl.objLerp, obj->oldbase.angle[Z], obj->base.angle[Z]);	
	}
	else
	{
		obj->visual.angle[X] = obj->base.angle[X];
		obj->visual.angle[Y] = obj->base.angle[Y];
		obj->visual.angle[Z] = obj->base.angle[Z];
	}
}


/*==========================

  CL_HandleWorldObjectRepresentation

  handle a static object with a complex collision surface

 ==========================*/

void	CL_HandleWorldObjectRepresentation(int index)
{
	clientObject_t *obj = &cl.objects[index];
	//if the world object is not active, we must create it
	if (!obj->worldobj_active)
	{
		objectPosition_t objpos;

		M_CopyVec3(obj->base.pos, objpos.position);
		M_CopyVec3(obj->base.angle, objpos.rotation);
		objpos.scale = STRUCTURE_SCALE;					//hack to scale down structures
		corec.WO_CreateObject(-1, CL_ObjectType(obj->base.type)->model, CL_ObjectType(obj->base.type)->skin, &objpos, index);

		cl.objects[index].worldobj_active = true;
	}
}

void	CL_ClearSpawnList()
{
	cl.spawnList = NULL;
}


/*==========================

  CL_AddToSpawnList

  generate a linked list of points we can spawn from as a player

 ==========================*/

void	CL_AddToSpawnList(clientObject_t *obj)
{
	obj->nextSpawnPoint = cl.spawnList;
	cl.spawnList = obj;
}


/*==========================

  CL_UpdateClientInfoVars

  these need to get updated on every CL_BeginServerFrame

 ==========================*/

extern cvar_t player_race;
extern cvar_t player_racename;

void	CL_UpdateClientInfoVars()
{
	//don't change race info if we didn't receive our info yet
	//fixme: the server architecture should guarantee that we have cl.info before BeginServerFrame()!
	if (!cl.gotOurInfo)
		return;

	if (cl.info->team)
	{
		cl.race = cl.teams[cl.info->team].race;
		cl.isCommander = (cl.teams[cl.info->team].commander == cl.clientnum);
	}
	else
	{
		cl.race = 0;
		cl.isCommander = 0;
	}
			
	corec.Cvar_SetVarValue(&player_race, cl.race);
	corec.Cvar_SetVar(&player_racename, raceData[cl.race].name);
}

/*==========================

  CL_BeginServerFrame

  called when a new server frame has arrived

  gametime will always be less than servertime.
  server time represents the end of the object lerp for this server frame.

  immediately after this function, CL_ObjectFreed, CL_ObjectUpdated, and
  CL_ObjectNotVisible will be called for all objects in the server frame

 ==========================*/

void	CL_BeginServerFrame(int serverTime, unsigned int serverFrame)
{
	CL_ClearSpawnList();

	cl.lastUpdateTime = cl.nextUpdateTime;
	cl.serverFrameTime = serverTime - cl.lastUpdateTime;
	
	cl.nextUpdateTime = cl.svTime = serverTime;			//we store this time for working out the amount to interpolate between oldbase and base

	cl.svFrameNum = serverFrame;

	cl.lastRealPS = cl.playerstate;
	//incorporate playerstate fields that are sent from the server
	corec.Client_GetRealPlayerState(&cl.playerstate);

	cl.status = cl.playerstate.status;
	if (cl.status != 0)
		core.Console_Printf("Status: %i\n", cl.status);

	//set pointers in case ps.clientnum changed
	cl.clientnum = cl.playerstate.clientnum;
	if (cl.clientnum < 0 || cl.clientnum >= MAX_CLIENTS)
		core.Game_Error("Invalid cl.playerstate.clientnum\n");
	cl.player = &cl.objects[cl.clientnum];
	cl.info = &cl.clients[cl.clientnum].info;

	CL_UpdateClientInfoVars();

	//clear the object list and put our own player object at the beginning
	cl.numSvObjs = 1;
	cl.svObjs[0] = cl.player;
	
	//cl.pendingEvents = true;
}


/*==========================

  CL_FreeClientObject

 ==========================*/

void	CL_FreeClientObject(int index)
{
	int	n;

	clientObject_t *obj = &cl.objects[index];

	if (!obj->base.active && !obj->outsidePVS)
		return;		//already free

	//delete any worldobject representations
	if (obj->worldobj_active)
		corec.WO_DeleteObject(index);

	corec.World_UnlinkObject(&obj->base);

	//free skeleton data, or we'll have a mem leak
	corec.Geom_FreeSkeleton(&obj->skeleton);
	//corec.Geom_FreeSkeleton(&obj->armor);
	for (n = 0; n < MAX_STATE_SLOTS; n++)
		corec.Geom_FreeSkeleton(&obj->stateSkeleton[n]);

	memset(obj, 0, sizeof(clientObject_t));
	obj->base.index = index;		//retain index just to be safe
}

/*==========================

  CL_ObjectFreed

  called by the server when an object is deleted

 ==========================*/

void	CL_ObjectFreed(int index)		
{
	//corec.Console_DPrintf("CL_FreeObject(%i)\n",index);
	
	if (cl_debugUpdates.integer)
	{
		corec.Console_Printf("(update)Object Freed: %i\n", index);
	}

	CL_FreeClientObject(index);
}



/*==========================

  CL_ObjectNotVisible

  called by the server when an object leaves our PVS

 ==========================*/

void	CL_ObjectNotVisible(int index)
{
	clientObject_t *object = &cl.objects[index];

	if (cl_debugUpdates.integer)
	{
		corec.Console_Printf("(update)Outside Vis: %i\n", index);
	}

	object->outsidePVS = true;

	//for the commander, an object leaving visibility means that the object has entered the fog of war...
	
	if (cl.isCommander)
	{
		object->inFogOfWar = true;

		//...but buildings should still get rendered when in FOW
		if (IsBuildingType(object->base.type))
			return;		
	}

	//for all other cases, the object becomes inactive, but retains its data
	//this means the object won't get another update until it comes back into visibility.
	//this could cause problems with animation, as the timers won't reflect the "actual" animation time
	//when the object comes back into view.  however, i doubt this will ever be noticeable.
	//easily fixed if it becomes a problem.

	 object->base.active = false;
	 object->visual.active = false;
}

extern cvar_t cl_charBrightnessFactor;

/*==========================

  CL_ObjectUpdated

  the server has sent over information about an object

  this could be a new object, or an update of an existing object

  currently there is no distinction between the two as far as
  the server is concerned, so determining whether or not the object 
  is new is only a "best guess".

  currently, we consider an object to be new if:
    - the object was outside the PVS and re-entered it
	  (in this case it's a good idea to treat as new
	   anyway, because we don't want interpolation
	   artifacts)
	- the object type changed
	- obj->base.active was false

 ==========================*/

void	CL_ObjectUpdated(const baseObject_t *serverObj, bool changed)
{
	int e;
	clientObject_t *obj = &cl.objects[serverObj->index];
	bool wasActive;
	trajectory_t oldtraj;

	
	//add the object into the list for loop updates
	cl.svObjs[cl.numSvObjs++] = obj;	
	
	//this needs to be done every frame
	if ((IsBuildingType(obj->base.type) || (IsItemType(obj->base.type) && obj->base.owner == cl.clientnum)) &&
		obj->base.team == cl.info->team)
	{
		if (CL_ObjectType(obj->base.type)->spawnFrom || CL_ObjectType(obj->base.type)->commandCenter)
			CL_AddToSpawnList(obj);
	}

	//everything else only needs to be done for modified objects
	if (!changed)
	{
		//*serverObj == obj->base at this point, but copy obj->base to obj->oldbase for interpolation
		obj->oldbase = obj->base;
		return;
	}

	if (cl_debugUpdates.integer)
	{
		if (!obj->base.active)
		{
			corec.Console_Printf("(update)Entered Vis: %i\n", serverObj->index);
		}
		else
		{
			corec.Console_Printf("(update)Modified: %i\n", serverObj->index);
		}
	}

	wasActive = obj->base.active;
	oldtraj = obj->base.traj;

	if (obj->outsidePVS || 
		obj->base.type != serverObj->type || 
		!obj->base.active ||
		((obj->base.flags & BASEOBJ_WORLDOBJ_REPRESENTS) != (serverObj->flags & BASEOBJ_WORLDOBJ_REPRESENTS)))
	{
		//free the old one (if any)
		CL_FreeClientObject(obj->base.index);
		//set the 'from' lerp object (oldbase) to the same data as serverObj
		obj->oldbase = *serverObj;
	}
	else
	{
		//save the old baseobject state for interpolation
		obj->oldbase = obj->base;		
	}
	
	//copy over the serverObj data
	obj->base = *serverObj;
	//set bounding box based on object type.  we don't get this from the network
	SetObjectBounds(&obj->base);
	if (cl_linkHack.integer)
	{
		if (M_CompareVec3(obj->base.bmin, zero_vec) && M_CompareVec3(obj->base.bmax, zero_vec))
		{
			M_SetVec3(obj->base.bmin, -5,-5,-5);
			M_SetVec3(obj->base.bmax, 5,5,5);
		}
	}
	
	if (obj->base.flags & BASEOBJ_SNAP_TO_MUZZLE && obj->base.flags & BASEOBJ_HAS_OWNER)
	{
		if (!wasActive || (wasActive && (oldtraj.startTime != obj->base.traj.startTime)))
		{
			obj->snapToMuzzle = true;
			obj->oldbase.traj = obj->base.traj;
			M_CopyVec3(obj->base.traj.origin, obj->oldbase.pos);			
		}
		else
		{
			obj->base.traj = oldtraj;
		}
	}

	//snap the visual representation to the most recent data
	obj->visual = obj->base;

	if (IsCharacterType(obj->base.type))
	{
		//brightness field is only used for characters
		obj->brightness = CL_SampleBrightness(obj->base.pos);
	}

	if (obj->base.flags & BASEOBJ_WORLDOBJ_REPRESENTS)
	{
		CL_HandleWorldObjectRepresentation(obj->base.index);
	}
	
	//determine if the animation states have changed or been retriggered
	for (e = 0; e < obj->base.numEvents; e++)
	{	
		if (obj->base.events[e].type == EVENT_ANIM2_RETRIGGER)
		{
			obj->newAnimState2 = true;
			continue;
		}
		else if (obj->base.events[e].type == EVENT_ANIM_RETRIGGER)
		{
			obj->newAnimState = true;
			continue;
		}		
	}	
	
	if (obj->base.animState2 != obj->oldbase.animState2)
	{
		obj->newAnimState2 = true;
	}

	if (obj->base.animState != obj->oldbase.animState)
	{
		obj->newAnimState = true;				
	}

	//issue any object events that have occured
	if (obj->base.numEvents)
	{
		for (e = 0; e < obj->base.numEvents; e++)
		{
			CL_DoObjectEvent(obj, &obj->base.events[e]);
		}
	}
}


/*==========================

  CL_EndServerFrame

  the server update has been completed.

 ==========================*/

void	CL_EndServerFrame()
{	
	cl.svframecount++;
}


extern cvar_t cl_characterScale;



/*==========================

  CL_EvaluateTrajectoryForRendering

  Interpolates the object along its trajectory and adds
  a tumbling effect as specified in its object file

  this function, as well as code in CL_ObjectUpdated,
  guarantees that oldbase.pos and base.pos specify the
  trajectory segment needed to render trail effects
  for this frame.  in addition, visual.pos will always
  equal base.pos after this function is called, so
  visual.pos and base.pos can be used interchangeably.

 ==========================*/

void	CL_EvaluateTrajectoryForRendering(clientObject_t *obj)
{
	objectData_t	*objdata = CL_ObjectType(obj->visual.type);
	float	randnum;

	if (obj->base.events[0].type == EVENT_DEATH)
	{
		//on death, a projectile stores its final position in base.pos
		M_CopyVec3(obj->base.pos, obj->visual.pos);
	}
	else
	{			
		//evaluate trajectory
		Traj_GetPos(&obj->base.traj, cl.trajectoryTime, obj->visual.pos);
		M_CopyVec3(obj->visual.pos, obj->base.pos);
	}

	//get a 'fake' random number, that will always be the same for this object
	//by including the trajectory start time, it automatically creates a new tumble rotation when it bounces
	randnum = (obj->visual.traj.startTime * obj->visual.index) % 1000;
	randnum /= 1000.0;

	obj->visual.angle[YAW] += randnum * (objdata->yaw.max - objdata->yaw.min) + objdata->yaw.min;
	obj->visual.angle[PITCH] += randnum * (objdata->pitch.max - objdata->pitch.min) + objdata->pitch.min;
	obj->visual.angle[ROLL] += randnum * (objdata->roll.max - objdata->roll.min) + objdata->roll.min;
}


/*==========================

  CL_ProcessObjects

  the main object processing function called every frame.
  here we link objects in to our local collision system for
  playerstate prediction purposes and other client side
  effects.

 ==========================*/

void	CL_ProcessObjects()
{
	int i;	

	PERF_BEGIN;

	if (!cl.numSvObjs)
		return;

	if (cl_lerpObjects.integer)
	{
		if (cl.svframecount > 1)
		{
			cl.objLerp = ((float)(cl.gametime - cl.lastUpdateTime) / (float)cl.serverFrameTime);
			//	corec.Console_DPrintf("objlerp: %f\n", cl.objLerp);

			if (cl.objLerp > 2)
				cl.objLerp = 2;
			else if (cl.objLerp < 0)
				cl.objLerp = 0;
		}
		else
			cl.objLerp = 1;		
	}
	else
	{
		cl.objLerp = 1;		
	}

	cl.trajectoryTime = cl.gametime + cl.serverFrameTime;

//	corec.Console_DPrintf("lerp: %f\n",cl.objLerp);

	//go through all known objects and process them
	for (i = 0; i < cl.numSvObjs; i++)
	{		
		clientObject_t *obj = cl.svObjs[i];
		baseObject_t *base = &obj->base;
		
		if (!base->active)
			continue;
		if (!base->type)
			continue;
		if (obj->inFogOfWar)
			continue;
		if (base->flags & BASEOBJ_NO_RENDER)
			continue;

		if (base->index != cl.clientnum)
		{
			if (!M_CompareVec3(base->pos, obj->oldbase.pos))
			{
				obj->lastPosUpdateTime = cl.gametime;
	//			corec.Console_Printf("pos update\n");
			}

			if (base->flags & BASEOBJ_USE_TRAJECTORY && cl_useTrajectory.integer)
			{
				CL_EvaluateTrajectoryForRendering(obj);
			}
			else
				CL_LerpObjectForRendering(obj);

			if (!(base->flags & BASEOBJ_NO_LINK) || cl_linkHack.integer)
			{
				if (!(M_CompareVec3(base->bmin, zero_vec) && M_CompareVec3(base->bmax, zero_vec)))
				{
				//	if (obj->visual.health || cl_linkHack.integer)
					{							
						if (IsUnitType(base->type) && cl.isCommander) //hack for scaling up characters in commander view
						{
							objectData_t *def = CL_ObjectType(base->type);

							vec3_t temp1,temp2;
							M_CopyVec3(base->bmin, temp1);
							M_CopyVec3(base->bmax, temp2);
							M_MultVec3(base->bmin, def->cmdrScale, base->bmin);
							M_MultVec3(base->bmax, def->cmdrScale, base->bmax);
							corec.World_LinkObject(base);
							M_CopyVec3(temp1, base->bmin);
							M_CopyVec3(temp2, base->bmax);
						}
						else
						{								
							corec.World_LinkObject(&obj->visual);
						}
						obj->linked = true;
					}
				}
			}
			else
			{
				if (!(obj->oldbase.flags & BASEOBJ_NO_LINK))
					corec.World_UnlinkObject(&obj->visual);
			}
		}
		
		//increment animation timers	

		if (obj->base.flags & BASEOBJ_NO_ANIMATE)
		{
			obj->animStateTime = 99999;
			obj->animState2Time = 99999;
		}
		else
		{
			int index;

			//advance animation timers
			if (obj->newAnimState2)
			{
				obj->animState2Time = 0;
				obj->newAnimState2 = false;
			}
			else
				obj->animState2Time += corec.FrameMilliseconds();

			if (obj->newAnimState)
			{
				obj->animStateTime = 0;
				obj->newAnimState = false;
			}
			else
				obj->animStateTime += corec.FrameMilliseconds();

			for (index = 0; index < MAX_STATE_SLOTS; index++)
				obj->stateAnimTime[index] += corec.FrameMilliseconds();
		}

		corec.Sound_MoveSource(base->index, obj->visual.pos, NULL);
		if (base->index < MAX_CLIENTS)
		{
			//weapon sounds play on a separate sound source
			corec.Sound_MoveSource(base->index + 2048, obj->visual.pos, NULL);
		}
	}			

	PERF_END(PERF_PROCESSOBJECTS);
}





/*==========================

  CL_AddEvent

  simulate an object event as if we received it from the server.

 ==========================*/

void	CL_AddEvent(int objnum, byte event, byte param, byte param2)
{
	int numEvents = cl.objects[objnum].base.numEvents;

	if (numEvents < MAX_OBJECT_EVENTS - 1)
	{
		cl.objects[objnum].base.events[numEvents].type = event;
		cl.objects[objnum].base.events[numEvents].param = param;
		cl.objects[objnum].base.events[numEvents].param2 = param2;
		cl.objects[objnum].base.numEvents++;
	}
}

void	CL_InitEvents()
{
	corec.Cvar_Register(&cl_showEvents);
	corec.Cvar_Register(&cl_showAnimEvents);
	corec.Cvar_Register(&cl_cameraJitterScale);
	corec.Cvar_Register(&cl_cameraJitterDuration);
	corec.Cvar_Register(&cl_lerpObjectAngles);
	corec.Cvar_Register(&cl_useTrajectory);

	corec.Cvar_Register(&cl_linkHack);
	corec.Cvar_Register(&cl_bloodThreshhold);
	corec.Cvar_Register(&cl_lerpShift);

	corec.Cvar_Register(&cl_debugUpdates);
}
