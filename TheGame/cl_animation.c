// (C) 2003 S2 Games

// cl_animation.c

// perform animation and animation events

// it would be pretty straightforward to move all this into shared code so the server could
// peform skeletal animation (for accurate hit detection)


#include "client_game.h"
#include <float.h>

cvar_t	cl_bipedHack		= { "cl_bipedHack",			"1" };
vec3_t yaw180[3];

static bool events_only = false;



/*==========================

  CL_GetBoneWorldTransform

 ==========================*/
 
void	CL_GetBoneWorldTransform(const char *boneName, const vec3_t objectPos, const vec3_t objectAxis[3], float scale, skeleton_t *skel, vec3_t pos, vec3_t axis[3])
{
	int index;
	vec3_t p;
	vec3_t a[3];
	vec3_t b[3];
	
	index = corec.Geom_GetBoneIndex(skel, boneName);
	if (index == -1)
	{
		M_CopyVec3(objectPos, pos);
		memcpy(axis, objectAxis, sizeof(vec3_t)*3);
		return;
	}
	
	//axis is assumed to be normalized!

	M_CopyVec3(skel->bones[index].tm_world.pos, p);
	memcpy(a, skel->bones[index].tm_world.axis, sizeof(vec3_t)*3);	

	M_MultiplyAxis(yaw180, a, b);

	//scale the position
	if (scale != 1)
		M_MultVec3(p, scale, p);

	//bring it into world space
	M_TransformPoint(p, objectPos, objectAxis, pos);
	M_MultiplyAxis(b, objectAxis, axis);	

	if (_isnan(pos[0]) || _isnan(pos[1]) || _isnan(pos[2]) || M_CompareVec3(zero_vec, pos))		//HACK
	{
		corec.Console_DPrintf("CL_GetBoneWorldTransform: NAN\n");
		M_CopyVec3(objectPos, pos);
		M_SetVec3(axis[0], 1, 0, 0);
		M_SetVec3(axis[1], 0, 1, 0);
		M_SetVec3(axis[2], 0, 0, 1);
	}
}

/*==========================

  CL_SetBoneAnim

  set animation and handle animation events

 ==========================*/

void	CL_SetBoneAnim(clientObject_t *obj, const char *parentBone, const char *animName, float animTime, int currentTime, int blendTime)
{
	if (events_only)
		corec.Geom_SetBoneAnimNoPose(animName, animTime, currentTime, 0);
	else
		corec.Geom_SetBoneAnim(parentBone, animName, animTime, currentTime, blendTime, 0);

	if (obj->skeleton.gotEvent)
		CL_AnimEvent(obj, obj->skeleton.eventString);
}

void	CL_SetBoneAnimEx(clientObject_t *obj, const char *parentBone, const char *animName, float animTime, int currentTime, int blendTime, int eventChannel)
{
	if (events_only)
		corec.Geom_SetBoneAnimNoPose(animName, animTime, currentTime, eventChannel);
	else
		corec.Geom_SetBoneAnim(parentBone, animName, animTime, currentTime, blendTime, eventChannel);

	if (obj->skeleton.gotEvent)
		CL_AnimEvent(obj, obj->skeleton.eventString);
}



/*==========================

  CL_ImpulseAnim

 ==========================*/

void	CL_ImpulseAnim(clientObject_t *obj, const char *parentBone, int animstate, float animTime)
{
	//corec.Geom_BeginPose(&obj->skeleton, CL_Model(obj->base.type));
	//CL_SetBoneAnim(obj, parentBone, GetAnimName(animstate), animTime, cl.systime, 0);
	//corec.Geom_EndPose();

	obj->overrideAnim = animstate;
	obj->overrideAnimEnd = cl.gametime + 300;
	obj->overrideAnimTime = animTime;
}

/*==========================

	CL_PoseGenericModel

 ==========================*/

void	CL_PoseGenericModel(clientObject_t *obj, residx_t model)
{
	corec.Geom_BeginPose(&obj->skeleton, model);

	if (obj->visual.animState2)
		CL_SetBoneAnim(obj, "", GetAnimName(obj->visual.animState2), obj->animState2Time, cl.systime, 200);
	else
		CL_SetBoneAnim(obj, "", GetAnimName(obj->visual.animState), obj->animStateTime, cl.systime, 200);

	corec.Geom_EndPose();
}


int		CL_BlendTime(int animState)
{
	//return 0;
	switch(animState)
	{
		case AS_JUMP_START_LEFT:
		case AS_JUMP_START_RIGHT:
		case AS_JUMP_START_FWD:
		case AS_JUMP_START_BACK:
		case AS_JUMP_MID_LEFT:
		case AS_JUMP_MID_RIGHT:
		case AS_JUMP_MID_FWD:
		case AS_JUMP_MID_BACK:
		case AS_JUMP_END_LEFT:
		case AS_JUMP_END_RIGHT:
		case AS_JUMP_END_FWD:
		case AS_JUMP_END_BACK:
		case AS_JUMP_UP_START:
		case AS_JUMP_UP_MID:
		case AS_JUMP_UP_END:
		//case AS_JUMP_LAND:
			return 500;
		case AS_WEAPON_FIRE_1:
		case AS_WEAPON_FIRE_2:
		case AS_WEAPON_FIRE_3:
		case AS_WEAPON_FIRE_4:
		case AS_WEAPON_FIRE_5:
		case AS_WEAPON_FIRE_6:
			return 0;
		default:
			return 200;
	}
}

/*==========================

  CL_PosePlayerModel

  mixes upper/lower body animations if necessary

 ==========================*/

void	CL_PosePlayerModel(clientObject_t *obj, residx_t model)
{	
	if (IsCharacterType(obj->base.type))
	{
		corec.Geom_BeginPose(&obj->skeleton, model);

		if (obj->visual.animState && !obj->visual.animState2)
			CL_SetBoneAnim(obj, "", GetAnimName(obj->visual.animState), obj->animStateTime, cl.systime, CL_BlendTime(obj->visual.animState));
		else if (!obj->visual.animState && obj->visual.animState2)
			CL_SetBoneAnim(obj, "", GetAnimName(obj->visual.animState2), obj->animState2Time, cl.systime, CL_BlendTime(obj->visual.animState2));
		else if (!obj->visual.animState && !obj->visual.animState2)
		{
			CL_SetBoneAnim(obj, "", GetAnimName(obj->visual.animState), obj->animStateTime, cl.systime, CL_BlendTime(obj->visual.animState));
		} 
		else
		{		
			//mix upper and lower body

			CL_SetBoneAnimEx(obj, "", GetAnimName(obj->visual.animState), obj->animStateTime, cl.systime, 0, 0);

			//replace the upper body with the secondary animation if one is playing
			if (obj->visual.animState2)		
			{			
				if (cl_bipedHack.integer)
				{	
					//this will make the upper body look exactly as it does in max, but may cause bad twisting at the waist
					int bipidx = corec.Geom_GetBoneIndex(&obj->skeleton, "Bip01");
					vec4_t bippos;

					if (bipidx > -1)
					{
						//end the pose to finalize bone positions
						corec.Geom_EndPose();

						//start another pose so we can mask off the legs and replace the rest of the body with the weapon state animation
						corec.Geom_BeginPose(&obj->skeleton, model);

						M_CopyVec3(obj->skeleton.bones[bipidx].tm.pos, bippos);						

						//mask off the leg bones so they're not affected
						corec.Geom_SetBoneState("Bip01 R Thigh", POSE_MASKED);
						corec.Geom_SetBoneState("Bip01 L Thigh", POSE_MASKED);
						CL_SetBoneAnimEx(obj, "Bip01", GetAnimName(obj->visual.animState2), obj->animState2Time, cl.systime, 0, 1);
						
						//unmask
						//corec.Geom_SetBoneState("Bip01 R Thigh", 0);
						//corec.Geom_SetBoneState("Bip01 L Thigh", 0);

						//move the root to its original position, but keep the rotation
						M_CopyVec3(bippos, obj->skeleton.bones[bipidx].tm.pos);						
					}
				}
				else
				{
//					corec.Geom_RotateBone("Bip01", 0, 0, 0, false);
					//simply apply the upper body animation to the lower spine
					CL_SetBoneAnimEx(obj, "Bip01 Spine", GetAnimName(obj->visual.animState2), obj->animState2Time, cl.systime, 0, 1);

				}
			}
		}		

	//	corec.Geom_RotateBone(&obj->skeleton, "Bip01 Head", 0,0,-obj->visual.angle[PITCH],true);//0, 0, -cl.pitch/3, true);

//		corec.Geom_RotateBone(&obj->skeleton, "Bip01 Spine", 0, 0, -obj->visual.angle[PITCH], true);
		corec.Geom_EndPose();

		
		
	}
	else
	{
		CL_PoseGenericModel(obj, model);
	}
}

/*==========================

  CL_PoseModel

  perform all skeletal animation posing  

 ==========================*/


void	CL_PoseModel(clientObject_t *obj, bool eventsOnly)
{		
	//vec3_t dif;
	residx_t model = CL_Model(obj->visual.type);

	PERF_BEGIN;

	events_only = eventsOnly;

	//BLENDING HACK FOR E3
	if (CL_ObjectType(obj->visual.type)->e3noblend)
	{
		corec.Cvar_Set("geom_blendframes", "0");
	}
/*
	//don't pose things that are far away
	M_SubVec3(obj->visual.pos, cl.camera.origin, dif);
	if (M_GetVec3Length(dif) >= cl.player_farclip)
		return;
*/
	//don't pose things that won't get rendered
	if (obj->base.flags & BASEOBJ_NO_RENDER)
		return;

	if (cl.gametime <= obj->overrideAnimEnd)
	{
		corec.Geom_BeginPose(&obj->skeleton, CL_Model(obj->base.type));
		CL_SetBoneAnim(obj, "", GetAnimName(obj->overrideAnim), obj->overrideAnimTime, cl.systime, 100);
		corec.Geom_EndPose();

		obj->overrideAnimTime += corec.FrameMilliseconds();
		return;
	}

	if (obj->base.index < MAX_CLIENTS && IsCharacterType(obj->base.type))
	{
		CL_PosePlayerModel(obj, model);
	}
	else
	{
		CL_PoseGenericModel(obj, model);
	}

	//BLENDING HACK FOR E3
	if (CL_ObjectType(obj->visual.type)->e3noblend)
	{
		corec.Cvar_Set("geom_blendframes", "1");
	}

	PERF_END(PERF_POSEMODEL);
}


/*==========================

  CL_InitAnimation

 ==========================*/

void	CL_InitAnimation()
{
	corec.Cvar_Register(&cl_bipedHack);
	M_GetAxis(0, 0, 180, yaw180);
}
