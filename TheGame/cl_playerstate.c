// (C) 2001 S2 Games

// cl_playerstate.c

// for handling changes in the playerstate that cause various client-side effects
// we'll use this module for client side prediction later on, too



// client side damage effects

#include "client_game.h"

cvar_t	cl_prediction =				{ "cl_prediction",				"1" };
cvar_t	cl_debugPrediction =		{ "cl_debugPrediction",			"0" };
cvar_t	cl_zlerp =					{ "cl_zlerp",					"8", CVAR_VALUERANGE, 8, 9999 };
cvar_t	cl_shakeFactor =			{ "cl_shakeFactor",				"100" };	//camera shake
cvar_t	cl_showAnimStates =			{ "cl_showAnimStates",			"0" };
cvar_t	cl_viewbob = { "cl_viewbob", "1" };
cvar_t	cl_syncAngles = { "cl_syncAngles", "1" };

extern cvar_t cl_prediction;

void	CL_CameraShake(float duration, float frequency, float x, float y, float z)
{
	if (cl.effects.cameraEffect & CAMFX_JITTER)
	{
		//already doing a camera shake
		if (x < cl.effects.cameraJitter[X] &&
			y < cl.effects.cameraJitter[Y] &&
			z < cl.effects.cameraJitter[Z])
			return;
	}

	cl.effects.cameraEffect |= CAMFX_JITTER;
	//*
	cl.effects.cameraJitter[X] = x; //M_Randnum(-x, x);
	cl.effects.cameraJitter[Y] = y; //M_Randnum(-y, y);
	cl.effects.cameraJitter[Z] = z; //M_Randnum(-z, z);
	cl.effects.cameraJitterDuration = duration;
	cl.effects.cameraJitterTime = duration;
	cl.effects.cameraJitterFrequency = frequency;
	/**/
	//cl.effects.cameraNoiseInitialTime = 5;
	//cl.effects.cameraNoiseTime = 5;
	//cl.effects.cameraNoiseStrength = 2;
}

void	CL_CameraTwang(float duration, float x, float y, float z, bool pingpong)
{
	cl.effects.cameraEffect |= CAMFX_TWANG;

	cl.effects.cameraTwang[X] = x;
	cl.effects.cameraTwang[Y] = y;
	cl.effects.cameraTwang[Z] = z;
	cl.effects.cameraTwangDuration = duration;
	cl.effects.cameraTwangTime = duration;
	cl.effects.cameraTwangPingPong = pingpong;
}


void	CL_QuadOverlay(float time, float red, float green, float blue, float alpha)
{
	cl.effects.overlayColorParam[0] = red;
	cl.effects.overlayColorParam[1] = green;
	cl.effects.overlayColorParam[2] = blue;
	cl.effects.overlayColorParam[3] = alpha;
	cl.effects.overlayTimeParam = cl.effects.overlayTime = time;
	cl.effects.overlayEffect = 1;
}

void	CL_DoQuadOverlay()
{
	float t;

	cl.effects.overlayTime -= corec.FrameSeconds();
 
	if (cl.effects.overlayTimeParam)
		t = cl.effects.overlayTime / cl.effects.overlayTimeParam;
	else
		t = 0;

	cl.effects.overlayColor[0] = t * cl.effects.overlayColorParam[0];
	cl.effects.overlayColor[1] = t * cl.effects.overlayColorParam[1];
	cl.effects.overlayColor[2] = t * cl.effects.overlayColorParam[2];
	cl.effects.overlayColor[3] = t * cl.effects.overlayColorParam[3];
}


void	CL_ApplyCameraNoise(float duration, float strength)
{
}

#define CURVE_INTENSITY	0.1		//lower values are more intense
#define REORIENT_SPEED	1.2

void	CL_ThrowAngles(float duration, float x, float y, float z)
{
	if (!duration)
		return;

	cl.effects.cameraEffect |= CAMFX_THROW_ANGLES;
	cl.effects.cameraAngleParam[X] = x;
	cl.effects.cameraAngleParam[Y] = y;
	cl.effects.cameraAngleParam[Z] = z;
	cl.effects.cameraTimeParam = duration;
	//we do the following to ensure that the calculation of 't' in CL_DoThrowAngles is normalized into the 0..1 range,
	//and also to avoid divide by 0 errors
	cl.effects.cameraTime = duration - (duration * CURVE_INTENSITY);
	cl.effects.cameraNormalize = 1 / (duration * CURVE_INTENSITY);
}

//*
//#if 0

#define POING_FACTOR	100

void	CL_DoCameraShake()
{
	float t = ((cl.effects.cameraNoiseInitialTime - cl.effects.cameraNoiseTime) / (cl.effects.cameraNoiseInitialTime ? cl.effects.cameraNoiseInitialTime : 1));
	//float s = cl.effects.cameraNoiseStrength;
	static float pingpong = -1;
	static int dir = 1;

	if (t)
		t = 1/t;
	else
		t = 0;

	pingpong += corec.FrameSeconds() * dir * POING_FACTOR;

	if (pingpong > 1)
	{
		dir = -1;
		pingpong = 1;
	}
	else if (pingpong < -1)
	{
		dir = 1;
		pingpong = -1;
	}

//	cl.effects.angleOffset[0] = (t)*pingpong;
//	cl.effects.angleOffset[1] = (t)*pingpong;
//	cl.effects.angleOffset[2] = (t)*pingpong;

	//cl.effects.angleOffset[0] = (t);
	cl.effects.angleOffset[1] = (t);
	//cl.effects.angleOffset[2] = (t);
	cl.effects.cameraNoiseTime -= corec.FrameSeconds();
}

//#endif
/**/

void	CL_DoThrowAngles()
{
	float t = (1 / ((cl.effects.cameraTimeParam - cl.effects.cameraTime) / cl.effects.cameraTimeParam)) / cl.effects.cameraNormalize;

	cl.effects.cameraNormalize*=REORIENT_SPEED;

	cl.effects.angleOffset[0] += cl.effects.cameraAngleParam[0] * t;
	cl.effects.angleOffset[1] += cl.effects.cameraAngleParam[1] * t;
	cl.effects.angleOffset[2] += cl.effects.cameraAngleParam[2] * t;

	cl.effects.cameraTime -= corec.FrameSeconds();
}




void	CL_DoWalkBob()
{/*
	static float lastchange;
	float change;

	float speed = M_GetVec3Length(cl.predictedState.velocity) * 0.0142;
	//corec.Console_Printf("speed:%f\n",speed);
	if (speed < 1)
		speed = 1;
	else if (speed > 1.5)
		speed = 1.5;

	speed = ((speed-1) * 3) + 1;	


	change = sin(cl.effects.walkbobTime * cl_walkbobspeed.value * 2) * cl_walkbobamount.value * speed;

		if (lastchange < 0 && change > 0)
	{
		int r = rand() % 4;
		corec.Cmd_Exec(fmt("play2d /sfx/testfootsteps/stone_%i.wav", r+1));

	}


	cl.effects.posOffset[2] += change*2 > 10 ? 10 : change*2;
	lastchange = change;
	corec.Console_Printf("change*2=%f\n",change*2);
	change = sin(cl.effects.walkbobTime * cl_walkbobspeed.value) * cl_walkbobamount.value * speed;
	cl.effects.posOffset[0] += cl.rightvec[0] * change * 2;
	cl.effects.posOffset[1] += cl.rightvec[1] * change * 2;
	cl.effects.angleOffset[1] += change*0.2;

	cl.effects.walkbobTime += corec.FrameSeconds();	
	*/


	//smooth out changes in speed
	static float playerspeed = 0;
	float lastBobCycle = cl.bobCycle;
	float walkbobspeed;
	float walkbobamount = CL_ObjectType(cl.predictedState.unittype)->bobAmount;

	playerspeed = M_GetVec3Length(cl.predictedState.velocity);
	
	if (IsMovementAnim(cl.predictedState.animState) && !cl.thirdperson && playerspeed > 10 && cl_viewbob.integer)
	{	
		bool sprint;

		if (playerspeed < 100)
		{
			sprint = false;
			walkbobspeed = CL_ObjectType(cl.predictedState.unittype)->bobSpeedRun;
		}
		else
		{
			sprint = true;			
			walkbobspeed = CL_ObjectType(cl.predictedState.unittype)->bobSpeedSprint;
		}

		//set bobcycle based on player speed
		cl.bobCycle = sin(cl.effects.walkbobTime * (walkbobspeed + playerspeed * 0.008));
		cl.bobHalfCycle = sin(cl.effects.walkbobTime * (walkbobspeed + playerspeed * 0.008) * 0.5);
		cl.effects.walkbobTime += corec.FrameSeconds();

		//more up and down movement when walking forward
		if (IsForwardMovementAnim(cl.predictedState.animState))
		{
			if (sprint)
			{				
				cl.effects.walkbobPos[2] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[2], cl.bobCycle*3*walkbobamount);
				cl.effects.walkbobPos[0] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[0], cl.rightvec[0] * cl.bobHalfCycle * 2 * walkbobamount);
				cl.effects.walkbobPos[1] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[1], cl.rightvec[1] * cl.bobHalfCycle * 2 * walkbobamount);
				cl.effects.walkbobAngles[1] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobAngles[1], 0);
			}
			else
			{			
				cl.effects.walkbobPos[2] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[2], cl.bobCycle*2*walkbobamount);
				cl.effects.walkbobPos[0] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[0], cl.rightvec[0] * cl.bobHalfCycle * 1.5 * walkbobamount);
				cl.effects.walkbobPos[1] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[1], cl.rightvec[1] * cl.bobHalfCycle * 1.5 * walkbobamount);
				cl.effects.walkbobAngles[1] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobAngles[1], 0);			
			}
			cl.effects.walkbobAngles[0] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobAngles[0], 0);
		}
		else 
		{
			cl.effects.walkbobPos[2] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[2], cl.bobCycle*1.2);
			cl.effects.walkbobPos[0] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[0], cl.rightvec[0] * cl.bobHalfCycle);
			cl.effects.walkbobPos[1] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobPos[1], cl.rightvec[1] * cl.bobHalfCycle);
			if (IsLeftMovementAnim(cl.predictedState.animState))
				cl.effects.walkbobAngles[1] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobAngles[1], -0.5);
			else if (IsRightMovementAnim(cl.predictedState.animState))
				cl.effects.walkbobAngles[1] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobAngles[1], 0.5);
			else if (IsBackwardMovementAnim(cl.predictedState.animState))
				cl.effects.walkbobAngles[0] = M_ClampLerp(cl.frametime * 10, cl.effects.walkbobAngles[0], 0);
		}

		if (cl.bobCycle - lastBobCycle >= 0)
		{
			if (cl.pendingFootstep)
			{
				CL_FootstepSound(cl.player);
				cl.pendingFootstep = false;
			}			
		}
		else
		{
			cl.pendingFootstep = true;
		}
	}
	else
	{
		float lerp;
		//stop the bobbing		
		cl.bobCycle = M_ClampLerp(cl.frametime * 10, cl.bobCycle, 0);
		cl.bobHalfCycle = M_ClampLerp(cl.frametime * 10, cl.bobHalfCycle, 0);
		lerp = CLAMP(cl.frametime * 10, 0, 1);
		M_LerpVec3(lerp, cl.effects.walkbobPos, zero_vec, cl.effects.walkbobPos);
		M_LerpVec3(lerp, cl.effects.walkbobAngles, zero_vec, cl.effects.walkbobAngles);
		cl.effects.walkbobTime = 0;
	}
	
	M_AddVec3(cl.effects.walkbobPos, cl.effects.posOffset, cl.effects.posOffset);
	M_AddVec3(cl.effects.walkbobAngles, cl.effects.angleOffset, cl.effects.angleOffset);
}


void	CL_DoCameraJitter()
{
	vec3_t offset;
	float t = cl.effects.cameraJitterTime / cl.effects.cameraJitterDuration;

	//corec.Console_Printf("CL: jittering %f\n", t);
/*	offset[0] = cl.effects.cameraJitter[X] * sin(cl.effects.cameraJitterTime * cl.effects.cameraJitterFrequency) * t;
	offset[1] = cl.effects.cameraJitter[Y] * cos(cl.effects.cameraJitterTime * cl.effects.cameraJitterFrequency) * t;
	offset[2] = cl.effects.cameraJitter[Z] * sin(cl.effects.cameraJitterTime * cl.effects.cameraJitterFrequency) * t;
*/
	offset[0] = M_Randnum(-cl.effects.cameraJitter[X], cl.effects.cameraJitter[X]) * t;
	offset[1] = M_Randnum(-cl.effects.cameraJitter[Y], cl.effects.cameraJitter[Y]) * t;
	offset[2] = M_Randnum(-cl.effects.cameraJitter[Z], cl.effects.cameraJitter[Z]) * t;

	M_SetVec3Length(offset, MAX(MAX(cl.effects.cameraJitter[X], cl.effects.cameraJitter[Y]), cl.effects.cameraJitter[Z]) * t);

	//corec.Console_Printf("%f %f %f\n",offset[0],offset[1],offset[2]);

	M_TransformPoint(offset, zero_vec, cl.camera.viewaxis, cl.effects.posOffset);
	/*
	M_MultVec3(cl.camera.viewaxis[RIGHT], offset[RIGHT], cl.effects.posOffset);
	M_PointOnLine(cl.effects.posOffset, cl.camera.viewaxis[FORWARD], offset[FORWARD], cl.effects.posOffset);
	M_PointOnLine(cl.effects.posOffset, cl.camera.viewaxis[UP], offset[UP], cl.effects.posOffset);	
*/
	cl.effects.cameraJitterTime -= corec.FrameSeconds();
}

void	CL_DoCameraTwang()
{
	static int pingpong = 1;

	if (cl.effects.cameraTwangPingPong)
		pingpong = -pingpong;
	else
		pingpong = 1;
	
	cl.effects.angleOffset[0] += cl.effects.cameraTwang[X] * cl.effects.cameraTwangTime * pingpong;
	cl.effects.angleOffset[1] += cl.effects.cameraTwang[Y] * cl.effects.cameraTwangTime * pingpong;
	cl.effects.angleOffset[2] += cl.effects.cameraTwang[Z] * cl.effects.cameraTwangTime * pingpong; 

	cl.effects.cameraTwangTime -= corec.FrameSeconds();
}




/*==========================

  CL_DamageEffect

 ==========================*/

#define MAX_DAMAGE_SPREAD	35


void	CL_DamageEffect(int damage)
{
	//set up some fairly arbitrary values for scaling the damage effect
	float duration = (float)damage / 5;
	float spread = (float)damage / 3;
	if (spread > MAX_DAMAGE_SPREAD)
		spread = MAX_DAMAGE_SPREAD;
	if (duration > 5)
		duration = 5;
	if (duration < 0.001)
		return;

	CL_ThrowAngles(duration, M_Randnum(0,spread), 0,0);//M_Randnum(-spread,spread), M_Randnum(-spread/2,spread/2));
	CL_QuadOverlay(duration/2, 1, 0, 0, 0.5);
}


/*==========================

  CL_HealEffect

 ==========================*/



void	CL_HealEffect(int damage)
{
	//set up some fairly arbitrary values for scaling the damage effect
	float duration = (float)damage / 5;	
	if (duration > 5)
		duration = 5;
	if (duration < 0.5)
		return;
	
	CL_QuadOverlay(duration/2, 1, 1, 1, 0.5);
}



/*==========================

  CL_PlayerEffects

  player specific effects and interface events

 ==========================*/

void	CL_PlayerEffects()
{		
	int index;
	int n;

	M_ClearVec3(cl.effects.posOffset);
	M_ClearVec3(cl.effects.angleOffset);

	if (cl.effects.overlayTime <= 0)
	{
		cl.effects.overlayEffect = 0;
	}

	CL_DoWalkBob();

	if (cl.effects.cameraEffect & CAMFX_THROW_ANGLES)
	{
		CL_DoThrowAngles();
		if (cl.effects.cameraTime <= 0)
		{
			cl.effects.cameraEffect &= ~CAMFX_THROW_ANGLES;			
		}
	}

	if (cl.effects.cameraEffect & CAMFX_JITTER)
	{
		CL_DoCameraJitter();
		if (cl.effects.cameraJitterTime <= 0)
			cl.effects.cameraEffect &= ~CAMFX_JITTER;
	}
	else
	{
//		M_ClearVec3(cl.effects.posOffset);
	}

	if (cl.effects.cameraEffect & CAMFX_TWANG)
	{
		CL_DoCameraTwang();
		if (cl.effects.cameraTwangTime <= 0)
			cl.effects.cameraEffect &= ~CAMFX_TWANG;
	}

	switch(cl.effects.overlayEffect)
	{
		case 1:
			CL_DoQuadOverlay();
			break;
	}

	if (cl.status == STATUS_PLAYER)
	{
		//damage effect
		if (cl.oldPlayerState.health - cl.playerstate.health > 0)
		{
			float fullhealth = cl.predictedState.fullhealth;
			if (fullhealth)
				CL_DamageEffect(((cl.oldPlayerState.health - cl.playerstate.health) * 100) / fullhealth);
		}
		else if (cl.playerstate.health - cl.oldPlayerState.health > 0)
		{
			float fullhealth = cl.predictedState.fullhealth;
			if (fullhealth)
				CL_HealEffect(((cl.playerstate.health - cl.oldPlayerState.health) * 100) / fullhealth);
		}
	}


	//money effect
	if (cl.oldPlayerState.score.money != cl.predictedState.score.money)
	{
		//play sound and animation
		if (cl.oldPlayerState.score.money < cl.predictedState.score.money)
		{
			CL_Play2d(Snd("get_gold"), 1.0, CHANNEL_GUI);
			GUI_TextBuffer_Printf(cl.ui_goldgain, fmt("+%i\n", cl.predictedState.score.money - cl.oldPlayerState.score.money), 0);
			//CL_NotifyMessage(fmt("You picked up %i gold", cl.predictedState.score.money - cl.oldPlayerState.score.money), "");
		}
		else
		{
			GUI_TextBuffer_Printf(cl.ui_goldgain, fmt("-%i\n", cl.oldPlayerState.score.money - cl.predictedState.score.money), 0);
		}
		cl.ui_money_icon->startFrame = 0;
		cl.ui_money_icon->curFrame = 0;
		cl.ui_money_icon->endFrame = corec.Res_GetNumTextureFrames(cl.ui_money_icon->shader);
		cl.ui_money_icon->numloops = 1;
		cl.ui_money_icon->freezeFrame = 0;											
	}

	if (cl.status == STATUS_UNIT_SELECT)
	{
		if (memcmp(cl.oldPlayerState.inventory, cl.predictedState.inventory, sizeof(cl.oldPlayerState.inventory)) != 0 ||
			memcmp(cl.oldPlayerState.ammo, cl.predictedState.ammo, sizeof(cl.oldPlayerState.ammo)) != 0)
			CL_Play2d(Snd("spend_gold"), 1.0, CHANNEL_GUI);
	}

	if (cl.oldPlayerState.weaponState == AS_WPSTATE_FIRE && cl.predictedState.weaponState != cl.oldPlayerState.weaponState)
	{
		cl.playedFireEvent = false;
		CL_StopEffect(cl.fireEffectId);
	}


	if (cl.oldPlayerState.item != cl.predictedState.item)
	{
		cl.weaponSwitchTime = cl.gametime;
	}


	/*
	 *   interface events
	 */


	if (cl.oldPlayerState.unittype != cl.predictedState.unittype)
		CL_InterfaceEvent(IEVENT_UNITTYPE);

	//state changes
	for (index = 0; index < MAX_STATE_SLOTS; index++)
	{
		if (cl.oldPlayerState.states[index] != cl.predictedState.states[index])
		{
			CL_InterfaceEvent(IEVENT_STATES);
			break;
		}
	}

	//check for an inventory change
	for (index = 0; index < MAX_INVENTORY; index++)
	{
		if (cl.oldPlayerState.inventory[index] != cl.predictedState.inventory[index])
		{
			CL_InterfaceEvent(IEVENT_INVENTORY);
			break;
		}
	}

	//check for an ammo change
	for (index = 0; index < MAX_INVENTORY; index++)
	{
		if (cl.oldPlayerState.ammo[index] != cl.predictedState.ammo[index])
		{
			CL_InterfaceEvent(IEVENT_AMMO);
			break;
		}
	}

	//playerstate events

	for (n=0; n<cl.predictedState.numEvents; n++)
	{
		objEvent_t *ev = &cl.predictedState.events[n];

		switch(ev->type)
		{
			case EVENT_WEAPON_FIRE:
			{
				//do camera kick
				objectData_t *obj = CL_ObjectType(cl.predictedState.inventory[cl.predictedState.item]);
				CL_CameraTwang(obj->twangTime,obj->twangAmount,0,0,obj->twangPingPong);
				break;
			}
			case EVENT_DROPOFF:
			{
				//print a "you gave the team X resource" message
				//param stores the resource index, param2 stores the amount
				CL_NotifyMessage(fmt("%s %i %s!", GameMsg("dropoff_resources"), ev->param2, ResourceName(ev->param)), NULL);
				break;
			}
			case EVENT_WEAPON_HIT:
			{
				corec.Sound_StopHandle(cl.hitConfirmSound);
				cl.hitConfirmSound = corec.Sound_Play(corec.Res_LoadSound(Snd("weapon_hit")),
								 -1, NULL, 1.0, CHANNEL_AUTO, PRIORITY_HIGH);				
				break;
			}
			default:
				break;
		}
	}
}



/*==========================

  CL_IsPredictableEvent

  returns true if this is an event that can be predicted through CL_PredictPlayerState

 ==========================*/

bool	CL_IsPredictableEvent(byte event)
{
	switch(event)
	{
		case EVENT_JUMP:
		case EVENT_JUMP_LAND:
		case EVENT_WEAPON_FIRE:
			return true;
	}
	return false;
}



/*==========================

  CL_PlayerStateToClientObject

  create a clientObject_t out of a playerState_t for rendering third/first person views

 ==========================*/

void	CL_PlayerStateToClientObject(playerState_t *ps, clientObject_t *obj)
{
	//since we get immediate updates to our score, reflect this in the client info
	cl.clients[cl.clientnum].score = ps->score;

	obj->oldbase = obj->base;

	obj->base.index = cl.clientnum;

	if (cl.status != STATUS_PLAYER || ps->flags & PS_INSIDE_TRANSPORT)
	{
		//don't render us if we're not in the world
		obj->base.active = false;
		obj->visual.active = false;
		obj->oldbase.active = false;
		return;
	}
	M_CopyVec3(ps->pos, obj->base.pos);
	if (ps->health)
	{
		obj->base.angle[0] = cl.predictedState.angle[0];//cl.pitch;
		obj->base.angle[1] = cl.predictedState.angle[1];
		obj->base.angle[2] = cl.predictedState.angle[2];
	}

	obj->base.type = ps->unittype;
	//obj->base.numEvents = ps->numEvents;
	//memcpy(obj->base.events, ps->events, sizeof(obj->base.events));

	obj->base.animState = ps->animState;
	obj->base.animState2 = ps->animState2;
	obj->base.active = true;
	obj->base.health = ps->health;
	memcpy(obj->base.states, ps->states, sizeof(obj->base.states));
	obj->base.team = cl.info->team;
	obj->base.weapon = ps->inventory[ps->item];

	obj->base.flags = 0;
	obj->base.exflags = 0;	

	if (ps->weaponState == AS_WPSTATE_FIRE)
	{
		if (CL_ObjectType(ps->inventory[ps->item])->continuous)
			obj->base.exflags |= BASEOBJEX_FIRING_CONTINUOUS;
	}

	obj->visual = obj->base;

	if (ps->flags & PS_COMMANDER_SELECTED)
		obj->base.flags |= BASEOBJ_COMMANDER_SELECTED;

	if (ps->animState2 != cl.oldPlayerState.animState2)
	{
		obj->newAnimState2 = true;
	}
	if (ps->animState != cl.oldPlayerState.animState)
	{
		obj->newAnimState = true;
	}
	if (ps->numEvents)
	{
		int n;

		for (n=0; n<ps->numEvents; n++)
		{
			if (ps->events[n].type == EVENT_ANIM2_RETRIGGER)
				obj->newAnimState2 = true;
			else if (ps->events[n].type == EVENT_ANIM_RETRIGGER)
				obj->newAnimState = true;
//			else if (ps->events[n].type == EVENT_WEAPON_FIRE && ps->weaponState == WPSTATE_FIRE)
//				obj->newAnimState2 
			else	
				CL_DoObjectEvent(obj, &ps->events[n]);
		}
	}

	if (cl_showAnimStates.integer)
	{
		corec.Console_Printf("animstate: %s\nanimstate2: %s\nwpstate: %s\n", GetAnimName(ps->animState), GetAnimName(ps->animState2), GetAnimName(ps->weaponState));
	}
}


/*

  perform client side prediction by feeding all unacknowledged inputState_t's into
  Phys_AdvancePlayerState(), starting with the last playerState we received from
  the server.  This will give us a new playerstate which is our 'expected' playerstate
  after the unacknowledged inputstates have been processed by the server code.

  

*/
/*void	CL_PredictPlayerState()
{
	int numInputs = corec.Client_GetInputStateBufferNum();
	int n;

	cl.predictedState = cl.playerstate;

	for (n=0; n<numInputs; n++)
	{
		Phys_AdvancePlayerState(corec.Client_GetInputStateBuffer(n), &cl.predictedState, 
	}
	
}
*/


bool	CL_TraceBox(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface)
{
	return corec.World_TraceBox(result, start, end, bmin, bmax, ignoreSurface);
}

void	CL_LerpPlayerState(playerState_t *from, playerState_t *to, playerState_t *out, float amount)
{
	*out = *to;

	M_LerpVec3(amount, from->pos, to->pos, out->pos);
	out->angle[0] = M_LerpAngle(amount, from->angle[0], to->angle[0]);
	out->angle[1] = LERP(amount, from->angle[1], to->angle[1]);			//things can get screwy if we use lerpangle here
	out->angle[2] = M_LerpAngle(amount, from->angle[2], to->angle[2]);
	out->stamina = LERP(amount, from->stamina, to->stamina);
	out->focus = LERP(amount, from->focus, to->focus);
	out->overheatCounter = LERP(amount, from->overheatCounter, to->overheatCounter);
	out->charge = LERP(amount, from->charge, to->charge);
}


void	CL_PredictPlayerState()
{
	int n, curnum;
	unsigned int lastack;
	int deltatime;
	float avgdelta = 0;
	int numinputs = 0;
	phys_out_t out;
	static float last_z = 0;

	PERF_BEGIN;

	if (corec.Client_IsPlayingDemo())
	{
		if (cl_prediction.integer)
		{
			//lerp between the last real playerstate and this one
			CL_LerpPlayerState(&cl.lastRealPS, &cl.playerstate, &cl.predictedState, MIN(cl.objLerp, 1));
		}
		else
		{
			cl.predictedState = cl.playerstate;
		}
	}
	else
	{

		curnum = corec.Client_GetCurrentInputStateNum();	//this frame's input
		lastack = cl.playerstate.inputSequence;				//time of the most recently processed inputstate from the server that we know about

		//start at the most recent player state we have from the server
		//corec.Client_GetRealPlayerState(&cl.predictedState);
		cl.predictedState = cl.playerstate;
		cl.predictedState.numEvents = 0;

		//ok, the cl.predictedState.inputSequence is now BEFORE (or equal to) cl.oldPlayerState.inputSequence.
		//as soon as we process the last unacknowledged input, cl.predictedState.inputSequence will be
		//AFTER cl.oldPlayerState.inputSequence.

		if (cl_prediction.integer)
		{
			//start at the first unacknowledged player input and advance up to the current input
			for (n=curnum - (INPUTSTATE_BUFFER_SIZE-1); n<=curnum; n++)
			{
				inputState_t in;				
				inputState_t lastin;

				corec.Client_GetInputState(n, &in);
				corec.Client_GetInputState(n-1, &lastin);
				
				//if the time of this input is before the newest playerstate time, continue
				if (in.sequence <= lastack)
					continue;
		//		if (in.gametime < cl.gametime - 1500)
		//			continue;
		/*
				if (in.gametime == lastack)
				{
					vec3_t pos_error;
					vec3_t vel_error;

					M_SubVec3(cl.playerstate->pos, 
				}
		*/
				//deltatime = in.gametime - lasttime;
				deltatime = in.delta_msec;
				
				if (cl.predictedState.inputSequence == cl.predictionCompareState.inputSequence)
				{					
					//we can compare the current predicted state with the state we predicted for this inputSequence
					//on the previous client frame, and detect any prediction errors that may have occured
					//since integrating the newest cl.playerstate
					vec3_t errorvec;
					float vel_error = 0;
					float pos_error = 0;
					float angle_error = 0;

					//compare the previously predicted playerstate with the one we just got from the server
					if (!M_CompareVec3(cl.predictionCompareState.pos, cl.predictedState.pos))
					{						
						M_SubVec3(cl.predictionCompareState.pos, cl.predictedState.pos, errorvec);
						pos_error = M_GetVec3Length(errorvec);

						if (cl_debugPrediction.integer)
						{
							corec.Console_Printf("Prediction pos error: %.2f @ %ims\n", pos_error, lastack);
						}
					}
					if (!M_CompareVec3(cl.predictionCompareState.angle, cl.predictedState.angle))
					{						
						M_SubVec3(cl.predictionCompareState.angle, cl.predictedState.angle, errorvec);
						angle_error = M_GetVec3Length(errorvec);

						if (cl_debugPrediction.integer)
						{
							corec.Console_Printf("Prediction angle error: %.2f @ %ims\n", angle_error, lastack);
						}
					}
					if (!M_CompareVec3(cl.predictionCompareState.velocity, cl.predictedState.velocity))
					{
						vec3_t errorvec;

						M_SubVec3(cl.predictionCompareState.velocity, cl.predictedState.velocity, errorvec);
						vel_error = M_GetVec3Length(errorvec);

						if (cl_debugPrediction.integer)
						{
							corec.Console_Printf("Prediction velocity error: %.2f @ %ims\n", vel_error, lastack);
						}
					}
				}				

				/*
				if (deltatime <= 0)
				{
					if (cl_debugPrediction.integer)
					{
						corec.Console_DPrintf("deltatime <= 0\n");
					}
					continue;
				}
				if (deltatime > 500)
				{
					if (cl_debugPrediction.integer)
					{
						corec.Console_DPrintf("deltatime > 500\n");
					}
					//deltatime = 500;
				}
				*/

				//we should only process events that happen on the current input (the last loop)
				cl.predictedState.numEvents = 0;

				if (CL_ObjectType(cl.objects[cl.clientnum].base.type)->isVehicle && !CL_Dead())
				{
					in.yaw = ANGLE2WORD(cl.predictedState.angle[YAW]);
					in.pitch = 0;
				}

				Phys_AdvancePlayerState(&cl.predictedState, cl.info->team, &in, &lastin, CL_TraceBox, _CL_GetObjectByType, &out);

				avgdelta += deltatime;
				numinputs++;				

				/*
				if (cl_debugPrediction.integer)
				{
					corec.Console_DPrintf("deltatime == %i\n", deltatime);
				}
				*/
			}
		}		

		cl.predictionCompareState = cl.predictedState;		//save out this state before we modify it external to the input loop

		if (cl.predictedState.flags & PS_ON_GROUND && last_z && !cl.thirdperson)
		{
			//lerp a bit to smooth out sudden changes in height (steps or sharp edges on slopes)
			cl.predictedState.pos[2] = M_ClampLerp(cl_zlerp.value * cl.frametime, last_z, cl.predictedState.pos[2]);
		}
		last_z = cl.predictedState.pos[2];
	}

	if (cl.playerstate.numEvents)
		corec.Console_DPrintf("numevents: %i\n", cl.playerstate.numEvents);
	//add events to the predicted state that we received from the server this frame	
	for (n=0; n<cl.playerstate.numEvents; n++)
	{
		objEvent_t *ev = &cl.playerstate.events[n];

		if (cl_prediction.integer && !corec.Client_IsPlayingDemo())
		{
			if (CL_IsPredictableEvent(ev->type))
				continue;			//don't add events we would have already predicted
		}
		
		Phys_AddEvent(&cl.predictedState, ev->type, ev->param, ev->param2);
	}
	//clear them so they don't get added again next frame
	cl.playerstate.numEvents = 0;

  	if (numinputs)
		avgdelta /= numinputs;

	PERF_END(PERF_PREDICTION);

	//corec.Console_DPrintf("avgdelta: %f  numinputs:%i\n",avgdelta,numinputs);
}

void	CL_ProcessPlayerState()
{
	int olditem;

	//make sure we modify the item field in the input state before we do the prediction, so weapon switching will work
	corec.Client_ModifyInputStateItem((byte)cl.wishitem);

	//unlink our player bounding box during prediction
	corec.World_UnlinkObject(&cl.player->base);

	olditem = cl.predictedState.item;
	CL_PredictPlayerState();	//move the playerstate ahead to where we think it will be for responsive controls
	if (cl.predictedState.item != olditem)
		cl.wishitem = cl.predictedState.item;

	if (cl_prediction.integer && cl_syncAngles.integer)
	{
		//set the internal mouse angles to whatever we predicted our player state angles to be
		//this allows us to easily change the viewing angles on the server or in the physics code
		//and have the client sync up with it
		corec.Input_SetMouseAngles(cl.predictedState.angle);
	}

	//calc heading and right vector
	HeadingFromAngles(cl.predictedState.angle[PITCH], cl.predictedState.angle[YAW], cl.heading, cl.rightvec);

    CL_PlayerStateToClientObject(&cl.predictedState, cl.player);        //convert to clientobject for rendering and processing certain events
	//only link our bounding box in third person
	if (cl.thirdperson)
		corec.World_LinkObject(&cl.player->base);

	CL_PlayerEffects();	
}


void CL_TestDamage_Cmd(int argc, char *argv[])
{
	if (argc)
		CL_DamageEffect(atoi(argv[0]));
}

void	CL_RegisterTestCommands()
{
	corec.Cmd_Register("testdamage", CL_TestDamage_Cmd);
	corec.Cvar_Register(&cl_prediction);
	corec.Cvar_Register(&cl_debugPrediction);
	corec.Cvar_Register(&cl_zlerp);	
	corec.Cvar_Register(&cl_showAnimStates);
	corec.Cvar_Register(&cl_viewbob);
	corec.Cvar_Register(&cl_syncAngles);
}
