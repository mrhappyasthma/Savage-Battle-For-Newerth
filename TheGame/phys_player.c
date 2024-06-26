// (C) 2001 S2 Games

// phys_player.c

// physics which affect a playerstate

#include "game.h"
physicsParams_t p;
phys_out_t *p_out = NULL;
playerState_t *ps = NULL;
inputState_t *in = NULL;
inputState_t *oldin = NULL;
objectData_t *unit = NULL;
vec3_t oldAngles;

//#define PHYS_STEP_HEIGHT		25

//these are named the same as the functions in object_config.c intentionally
//for ease of use, and as a safeguard so that the function headers never get exposed to this module
#define GetObjectByType(type) (p.objectTypeFunc(type))
#define IsCharacterType(type) (GetObjectByType(type)->objclass == OBJCLASS_UNIT && !GetObjectByType(type)->isVehicle)
#define IsUnitType(type) (GetObjectByType(type)->objclass == OBJCLASS_UNIT)
#define IsBuildingType(type) (GetObjectByType(type)->objclass == OBJCLASS_BUILDING)
#define IsWorkerType(type) (GetObjectByType(type)->isWorker && IsUnitType(type))
#define IsMobileType(type) IsUnitType(type)
#define IsItemType(type) (GetObjectByType(type)->objclass == OBJCLASS_ITEM)
#define IsWeaponType(type) (GetObjectByType(type)->objclass == OBJCLASS_WEAPON)
#define IsMeleeType(type) (GetObjectByType(type)->objclass == OBJCLASS_MELEE)
#define IsUpgradeType(type) (GetObjectByType(type)->objclass == OBJCLASS_UPGRADE)


enum
{
	JUMPING,
	DODGING
};

vec3_t up = { 0, 0, 1 };

cvar_t p_sprintBurst =			{ "p_sprintBurst",			"0",	CVAR_TRANSMIT };
cvar_t p_gravity =				{ "p_gravity",				"1",	CVAR_TRANSMIT };
cvar_t p_jumpheight =			{ "p_jumpheight",			"1",	CVAR_TRANSMIT };
cvar_t p_friction =				{ "p_friction",				"1",	CVAR_TRANSMIT };
cvar_t p_speed =				{ "p_speed",				"2.8",	CVAR_TRANSMIT };
cvar_t p_aircontrol =			{ "p_aircontrol",			"1.5",	CVAR_TRANSMIT };
cvar_t p_stepheight =			{ "p_stepheight",			"1",	CVAR_TRANSMIT };
cvar_t p_groundfriction =		{ "p_groundfriction",		"10",	CVAR_TRANSMIT };
cvar_t p_lunge =				{ "p_lunge",				"0",	CVAR_TRANSMIT };		//set to 1 for 'lunge' momentum for certain melee attacks
cvar_t p_minslope =				{ "p_minslope",				"0.7",	CVAR_TRANSMIT };
cvar_t p_attackingSpeed =		{ "p_attackingSpeed",		"0.8",	CVAR_TRANSMIT };
cvar_t p_sprintSpeed =			{ "p_sprintSpeed",			"1.4",	CVAR_TRANSMIT };
cvar_t p_backwardsSpeed =		{ "p_backwardsSpeed",		"0.5",	CVAR_TRANSMIT };
cvar_t p_strafeSpeed =			{ "p_strafeSpeed",			"1",	CVAR_TRANSMIT };
cvar_t p_cooldownRate =			{ "p_cooldownRate",			"0.75",	CVAR_TRANSMIT };
cvar_t p_blockSpeed =			{ "p_blockSpeed",			"0",	CVAR_TRANSMIT };
cvar_t p_autoRunTime =			{ "p_autoRunTime",			"4000",	CVAR_TRANSMIT };
cvar_t p_autoRunSpeed =			{ "p_autoRunSpeed",			"1.8",	CVAR_TRANSMIT };
cvar_t p_slowWeaponAttack =		{ "p_slowWeaponAttack",		"0",	CVAR_TRANSMIT };
cvar_t p_levelHealthBonus =		{ "p_levelHealthBonus",		"0.0",	CVAR_TRANSMIT };
cvar_t p_maxBlockTime =			{ "p_maxBlockTime",			"600",	CVAR_TRANSMIT };
cvar_t p_fallingDamage =		{ "p_fallingDamage",		"1",	CVAR_TRANSMIT };
cvar_t p_fallingDamageSpeed =	{ "p_fallingDamageSpeed",	"50",	CVAR_TRANSMIT };
cvar_t p_landDuration =			{ "p_landDuration",			"100",	CVAR_TRANSMIT };
cvar_t p_jumpStaminaCost =		{ "p_jumpStaminaCost",		"500",	CVAR_TRANSMIT };
cvar_t p_dodgeStaminaCost =		{ "p_dodgeStaminaCost",		"1600",	CVAR_TRANSMIT };
cvar_t p_sprintRegainLandDelay =	{ "p_sprintRegainLandDelay",	"200",	CVAR_TRANSMIT };
cvar_t p_sprintRegainDelay =	{ "p_sprintRegainDelay",	"200",	CVAR_TRANSMIT };
cvar_t p_staminaDepleteSpeed =	{ "p_staminaDepleteSpeed",	"1.5",	CVAR_TRANSMIT };
cvar_t p_sprintRegenSlow =		{ "p_sprintRegenSlow",		"0.2",	CVAR_TRANSMIT };
cvar_t p_sprintRegenFast =		{ "p_sprintRegenFast",		"0.6",	CVAR_TRANSMIT };
cvar_t p_sprintDeadZone =		{ "p_sprintDeadZone",		"1000",	CVAR_TRANSMIT };
cvar_t p_freeflyAccel =			{ "p_freeflyAccel", "10", CVAR_TRANSMIT };
cvar_t p_freeflyAngleLerp =		{ "p_freeflyAngleLerp", "1", CVAR_TRANSMIT };

cvar_t	p_focusMinAngleChange =		{ "p_focusMinAngleChange",	"0.6", CVAR_TRANSMIT };
cvar_t	p_focusMaxAngleChange =		{ "p_focusMaxAngleChange",	"3", CVAR_TRANSMIT };
cvar_t p_focusAngleAmplify = { "p_focusAngleAmplify", "2", CVAR_TRANSMIT };

void	Phys_AddEvent(playerState_t *ps, byte event, byte param, byte param2)
{
	if (ps->numEvents >= MAX_OBJECT_EVENTS)
	{
		core.Console_DPrintf("Phys_AddEvent: too many events for player %p\n", ps);
		return;
	}
	
	ps->events[ps->numEvents].type = event;
	ps->events[ps->numEvents].param = param;
	ps->events[ps->numEvents].param2 = param2;
	ps->numEvents++;
}

void	Phys_ClearEvents(playerState_t *ps)
{
	ps->numEvents = 0;
}


/*==========================

  Phys_GetSurfaceInfo

  determine if we're on the ground, and what kind of surface we're intersecting with if any

 ==========================*/

bool	Phys_GetSurfaceInfo()
{
	traceinfo_t surface;

	vec3_t startpos = { ps->pos[0], ps->pos[1], ps->pos[2] };
	vec3_t targetpos = { ps->pos[0], ps->pos[1], ps->pos[2] - 0.05 };

	p.tracefunc(&surface, startpos, startpos, p.bmin, p.bmax, 0);
	if (surface.fraction < 1)
		ps->insideSurface = surface.flags;
	else
		ps->insideSurface = 0;

	M_ClearVec3(p.ground.normal);
	p.tracefunc(&p.ground, startpos, targetpos, p.bmin, p.bmax, TRACE_PLAYER_MOVEMENT);

	if (p.ground.fraction < 1 && p.ground.normal[2] >= p_minslope.value)
	{
		if (!(ps->flags & PS_ON_GROUND))		//previously off the ground
		{
			Phys_AddEvent(ps, EVENT_JUMP_LAND, 0, 0);
			ps->landTime = in->gametime;
		}

		ps->flags |= PS_ON_GROUND;
		if (ps->velocity[2] < 0)
			ps->velocity[2] = 0;
		return true;	
	}
		
	//core.Console_Printf("slope: %f\n", p.ground.normal[2]);
	ps->flags &= ~PS_ON_GROUND;
	return false;
}

//#define DEBUG_MOVEMENT


//slides velocity along a plane but not exactly, to avoid precision errors
void Phys_ParallelPlane(vec3_t in_vel, vec3_t normal, vec3_t out_vel)
{
	float a;
	int	n;

	a = M_DotProduct(in_vel, normal);
	a += SGN(a)*0.01;

	for (n=0; n<3; n++)
	{
		out_vel[n] = in_vel[n] - normal[n]*a;
	}
}


void Phys_ClampVelocity()
{
	if (ABS(ps->velocity[0]) < 0.01)
		ps->velocity[0] = 0;
	if (ABS(ps->velocity[1]) < 0.01)
		ps->velocity[1] = 0;
	if (ABS(ps->velocity[2]) < 0.01)
		ps->velocity[2] = 0;
}

bool	Phys_IsLungeAttack(int state)
{
	if (!p_lunge.integer)
		return false;
	if (!IS_MELEE_ATTACK(state))
		return false;
	if (!unit->attacks[state].lungeTime)
		return false;

	return true;
}



void	Phys_SetDirectionalAnim(int left, int right, int fwd, int back, int none)
{
	ps->animState = none;

	if (in->movement & MOVE_FORWARD)
	{		
		ps->animState = fwd;
	}	
	else if (in->movement & MOVE_BACKWARD)
	{
		ps->animState = back;
	}
	if (in->movement & MOVE_LEFT)
	{
		ps->animState = left;		
	}
	else if (in->movement & MOVE_RIGHT)
	{
		ps->animState = right;
	}

	return;		//animState should stay at whatever is was on the previous move
}


/*==========================

  Phys_SetMovementAnim

  set the appropriate movement animation

 ==========================*/

void	Phys_SetMovementAnim()
{
	if (ps->animState == AS_RESURRECTED)
		return;

	if (ps->weaponState == AS_BLOCK)
	{
		ps->animState = 0;
		return;
	}

	if (ps->flags & PS_ON_GROUND)
	{
		if (in->gametime - ps->landTime < p_landDuration.integer)
		{
			ps->animState = AS_JUMP_LAND;
			return;
		}
		else
		{
			float vel = M_GetVec2Length(ps->velocity);

			if (vel < 1)
			{
				ps->animState = AS_IDLE;
				return;
			}
			else if (vel < 50)
			{
				Phys_SetDirectionalAnim(AS_WALK_LEFT, AS_WALK_RIGHT, AS_WALK_FWD, AS_WALK_BACK, AS_IDLE);
				return;
			}
			else if (vel < 100)
			{
				Phys_SetDirectionalAnim(AS_RUN_LEFT, AS_RUN_RIGHT, AS_RUN_FWD, AS_RUN_BACK, AS_IDLE);
				return;
			}
			else
			{
				Phys_SetDirectionalAnim(AS_SPRINT_LEFT, AS_SPRINT_RIGHT, AS_SPRINT_FWD, AS_SPRINT_BACK, AS_IDLE);
				return;
			}
		}
	}
	else
	{
		//float vel2d = M_GetVec2Length(ps->velocity);
		float velz = ps->velocity[2];
		float midArc;
		int animBase;

		if (ps->airMovementState == DODGING)
		{
			midArc = 25;
			animBase = AS_DODGE_START_LEFT;
		}
		else
		{
			midArc = p.jumpheight / 2;
			animBase = AS_JUMP_START_LEFT;
		}

		if (velz > 0)
		{
			if (velz > midArc)			//started jump
			{
				Phys_SetDirectionalAnim(animBase, animBase+1, animBase+2, animBase+3, AS_JUMP_UP_START);
				return;			
			}
			else									//slowing down
			{
				Phys_SetDirectionalAnim(animBase+4, animBase+5, animBase+6, animBase+7, AS_JUMP_UP_MID);
				return;
			}
		}
		else
		{
			if (velz > -midArc)			//slowing down
			{
				Phys_SetDirectionalAnim(animBase+4, animBase+5, animBase+6, animBase+7, AS_JUMP_UP_MID);
				return;
			}
			else									//descending
			{
				Phys_SetDirectionalAnim(animBase+8, animBase+9, animBase+10, animBase+11, AS_JUMP_UP_END);
				return;
			}
		}	
	}
}


/*==========================

  Phys_SetSecondaryAnim

  set the secondary (weapon) animation for the player

 ==========================*/

void	Phys_SetSecondaryAnim()
{
	int animgroup;

	if (!ps->weaponState)
	{
		ps->animState2 = 0;
		return;
	}

	if (IsMeleeType(ps->inventory[ps->item]))
	{
		//melee attack
		if (in->gametime == ps->wpStateStartTime)		//just started the attack
		{
			if (!ps->animState)
			{
				ps->animState2 = ps->weaponState;
				return;
			}
			else
			{
				//pick a different melee anim if we're moving
				switch(ps->weaponState)
				{
					case AS_MELEE_1:
						ps->animState2 = AS_MELEE_MOVE_1;
						return;
					case AS_MELEE_2:
						ps->animState2 = AS_MELEE_MOVE_2;
						return;
					case AS_MELEE_3:
						ps->animState2 = AS_MELEE_MOVE_3;
						return;
					case AS_MELEE_4:
						ps->animState2 = AS_MELEE_MOVE_4;
						return;
					case AS_MELEE_CHARGE:
						ps->animState2 = AS_MELEE_MOVE_CHARGE;
						return;
					case AS_MELEE_RELEASE:
						ps->animState2 = AS_MELEE_MOVE_RELEASE;
						return;
					/*case AS_ALT_MELEE_1:
						ps->animState2 = AS_ALT_MELEE_1;
						return;
					case AS_ALT_MELEE_2:
						ps->animState2 = AS_ALT_MELEE_2;
						return;
					case AS_ALT_MELEE_3:
						ps->animState2 = AS_ALT_MELEE_3;
						return;
					case AS_ALT_MELEE_4:
						ps->animState2 = AS_ALT_MELEE_4;*/
					case AS_BLOCK:
						ps->animState2 = AS_BLOCK;
						return;
					default:
						core.Console_DPrintf("Phys_SetSecondaryAnim: unknown melee state %i\n", ps->weaponState);
						ps->animState2 = ps->weaponState;
						return;
				}
			}
		}

		return;
	}	
	
	animgroup = GetObjectByType(ps->inventory[ps->item])->animGroup - 1;

	if (animgroup < 0 || animgroup > 5)
		animgroup = 0;	

	switch(ps->weaponState)
	{
		case AS_WPSTATE_IDLE:
			ps->animState2 = AS_WEAPON_IDLE_1 + animgroup;
			return;
		case AS_WPSTATE_SWITCH:
			ps->animState2 = AS_WEAPON_SWITCH;
			return;
		case AS_WPSTATE_CHARGE:
			ps->animState2 = AS_WEAPON_CHARGE_1 + animgroup;
			return;
		case AS_WPSTATE_SPINUP:
			ps->animState2 = AS_WEAPON_CHARGE_1 + animgroup;
			return;
		case AS_WPSTATE_SPINDOWN:
			ps->animState2 = AS_WEAPON_FIRE_1 + animgroup;
			return;
		case AS_WPSTATE_OVERHEAT:
			ps->animState2 = AS_WEAPON_IDLE_1 + animgroup;
			return;
		case AS_WPSTATE_FIRE:
			ps->animState2 = AS_WEAPON_FIRE_1 + animgroup;
			return;		
		default:
			core.Console_DPrintf("Phys_SetSecondaryAnim: unhandled weapon state %i\n", ps->weaponState);
			ps->animState2 = 0;
			return;
	}
}

void	Phys_SetDrivingAnimState()
{
	ps->animState2 = 0;
	ps->animState = 0;

	if (in->movement & MOVE_FORWARD)
	{
		ps->animState = AS_WALK_FWD;		
	}
	else if (in->movement & MOVE_BACKWARD)
	{
		ps->animState = AS_WALK_BACK;		
	}

	if (ps->weaponState == AS_WPSTATE_FIRE)
	{
		ps->animState2 = AS_WEAPON_FIRE_1;
	}
	else if (ps->weaponState == AS_WPSTATE_CHARGE)
	{
		ps->animState2 = AS_WEAPON_CHARGE_1;
	}

	return;
}

float	Phys_GetStaminaRegenRate(byte states[MAX_STATE_SLOTS], byte type)
{
	int		n;
	float	ret;

	ret = GetObjectByType(type)->staminaRegenRate;

	for (n = 0; n < MAX_STATE_SLOTS; n++)
	{
		int statenum = states[n];

		if (!statenum)
			continue;

		ret += stateData[statenum].staminaRegenAdjust;
	}

	return ret;
}

void	Phys_Walk()
{
	vec3_t	intentNormalized;
	bool	mvmt = false;
	bool	sprinting = false;
	bool	stunned = false;
	vec3_t	forward, right;
	bool	locked = false;
	//int		movetype = AS_IDLE;
	//objectData_t	*unitData = GetObjectByType(ps->unittype);

	ps->angle[PITCH] = WORD2ANGLE(in->pitch);
	ps->angle[ROLL] = 0;
	ps->angle[YAW] = WORD2ANGLE(in->yaw);

	//view angle modifications
	if (IS_MELEE_ATTACK(ps->weaponState))
	{
		if (GetObjectByType(ps->unittype)->attacks[ps->weaponState].lockAngles)
		{
			M_CopyVec3(oldAngles, ps->angle);
		}		
	}	

	M_CopyVec3(p.forward, forward);
	M_CopyVec3(p.right, right);
	forward[2] = 0; right[2] = 0;	//we only want the 2d component for walking physics

	Phys_GetSurfaceInfo();

	if (!(ps->insideSurface & SURF_PUSH_ZONE))
	{
		if (((ps->stunFlags & STUN_MOVEMENT) && ps->stunTime > in->gametime))		//don't allow movement during a stun
			stunned = true;
		else
			ps->stunFlags &= ~STUN_MOVEMENT;

		M_ClearVec3(ps->intent);

		if (!(in->movement & MOVE_UP))
			ps->flags &= ~PS_JUMP_LOCKED;
		if (!(in->buttons & B_BLOCK) && GetObjectByType(ps->inventory[ps->item])->canDodge)
			ps->flags &= ~PS_BLOCK_LOCKED;
		

		//don't allow movement input during a stun or an attack with a lunge
		if (!stunned && !(Phys_IsLungeAttack(ps->weaponState)))		
		{
			//work out the direction we are intending to move
			if (in->movement & MOVE_FORWARD)
			{
				mvmt = true;
				ps->intent[0] += forward[0];
				ps->intent[1] += forward[1];
			}	
			else if (in->movement & MOVE_BACKWARD)
			{
				mvmt = true;
				ps->intent[0] += -forward[0];
				ps->intent[1] += -forward[1];
			}
			if (in->movement & MOVE_LEFT)
			{
				mvmt = true;
				ps->intent[0] += -right[0];
				ps->intent[1] += -right[1];
			}
			else if (in->movement & MOVE_RIGHT)
			{
				mvmt = true;
				ps->intent[0] += right[0];
				ps->intent[1] += right[1];
			}

			//normalized intent
			M_NormalizeVec2(ps->intent);
			M_CopyVec3(ps->intent, intentNormalized);
		}

		//determine movement speed
		if (!stunned)
		{
			//adjust intend by the player's movement speed for their current direction of travel
			if (in->movement & MOVE_BACKWARD)
				M_MultVec2(ps->intent, p.speed * p_backwardsSpeed.value, ps->intent);
			else if (in->movement & MOVE_LEFT || in->movement & MOVE_RIGHT)
				M_MultVec2(ps->intent, p.speed * p_strafeSpeed.value, ps->intent);
			else
				M_MultVec2(ps->intent, p.speed, ps->intent);

			//modify for holding a weapon
			if (ps->inventory[ps->item])	//weapon slot
				M_MultVec2(ps->intent, GetObjectByType(ps->inventory[ps->item])->speedPenalty, ps->intent);

			//block modifier
			if (ps->weaponState == AS_BLOCK)
				M_MultVec2(ps->intent, p_blockSpeed.value, ps->intent);

			//attack modifier
			if (IS_MELEE_ATTACK(ps->weaponState))
			{
				M_MultVec2(ps->intent, GetObjectByType(ps->unittype)->attacks[ps->weaponState].speed, ps->intent);
				if (GetObjectByType(ps->unittype)->attacks[ps->weaponState].speed == 0)
					locked = true;
			}

			if (ps->animState == AS_RESURRECTED)
				M_ClearVec2(ps->intent);

			//apply sprint modifier
			if (in->buttons & B_SPRINT && mvmt && ps->maxstamina > 0 && GetObjectByType(ps->unittype)->canSprint)
			{
				sprinting = true;
				if (ps->stamina)
				{
					if (p_sprintBurst.integer)
						M_MultVec2(ps->intent, 1 + (((float)ps->stamina / ps->maxstamina) * p_sprintSpeed.value), ps->intent);
					else
						M_MultVec2(ps->intent, p_sprintSpeed.value, ps->intent);

					//deplete sprint power
					ps->stamina -= p.frametime * 1000 * p_staminaDepleteSpeed.value;
					if (ps->stamina < 0)
						ps->stamina = 0;
				}
				ps->walkTime = in->gametime;	//this gets set every frame that we are sprinting
			}
			else
			{
				if ((ps->flags & PS_ON_GROUND) && 
					(in->gametime - ps->landTime > p_sprintRegainLandDelay.integer) &&
					(in->gametime - ps->walkTime > p_sprintRegainDelay.integer))
				{
					//replenish sprint power
					if (ps->stamina <= p_sprintDeadZone.value)
					{
						ps->stamina += p.frametime * 1000 * Phys_GetStaminaRegenRate(ps->states, ps->unittype) * p_sprintRegenSlow.value;
					}
					else
					{
						ps->stamina += p.frametime * 1000 * Phys_GetStaminaRegenRate(ps->states, ps->unittype) * p_sprintRegenFast.value;
					}
					if (ps->stamina > ps->maxstamina)
						ps->stamina = ps->maxstamina;
				}

				if (!GetObjectByType(ps->unittype)->canSprint)
					ps->stamina = 0;
			}

			//adjust speed while attacking
			if (ps->weaponState)
			{
				if (!ps->item || p_slowWeaponAttack.integer)
					M_MultVec2(ps->intent, p_attackingSpeed.value, ps->intent);
			}		
		}
	}

	//focus adjustment
	if (mvmt)
	{
		objectData_t *def = GetObjectByType(ps->inventory[ps->item]);

		//compensate for the recovery rate
		ps->focus -= def->focusRecoverRate *p.frametime;

		if (sprinting)
			ps->focus -= def->focusDegradeRate * p.frametime * p_sprintSpeed.value;
		else
			ps->focus -= def->focusDegradeRate * p.frametime;

		if (ps->focus < def->focusPenalty)
			ps->focus = def->focusPenalty;
	}

	Phys_DoCombat(in, ps);

	if (ps->insideSurface & SURF_PUSH_ZONE)
	{
		//push zones override any other kind of movement
		M_FlipVec3(forward, ps->intent);
		M_MultVec3(ps->intent, p.speed, ps->intent);
	}

	
	if (ps->flags & PS_ON_GROUND && 
		!(ps->flags & PS_DAMAGE_PUSH) && 
		!(ps->insideSurface & SURF_PUSH_ZONE) &&
		ps->animState != AS_RESURRECTED &&
		!locked)
	{
		float lerp;
		bool checkJump = true;

		lerp = p.frametime * p_groundfriction.value;
		if (lerp > 1)
			lerp = 1;
		ps->velocity[0] = LERP(lerp, ps->velocity[0], ps->intent[0]);
 		ps->velocity[1] = LERP(lerp, ps->velocity[1], ps->intent[1]);
		ps->velocity[2] = 0;

		//check for dodge moves first
		if (GetObjectByType(ps->inventory[ps->item])->canDodge)
		{
			if (in->buttons & B_BLOCK && !(ps->flags & PS_BLOCK_LOCKED))
			{
				ps->flags |= PS_BLOCK_LOCKED;

				if (ps->stamina >= p_dodgeStaminaCost.integer && mvmt)
				{				
					ps->velocity[2] += 50;
					ps->velocity[0] = intentNormalized[0] * p.speed * 3;
					ps->velocity[1] = intentNormalized[1] * p.speed * 3;

					ps->flags &= ~PS_ON_GROUND;								

					ps->flags |= PS_BLOCK_LOCKED;

					checkJump = false;

					ps->stamina -= p_dodgeStaminaCost.integer;
					if (ps->stamina < 0)
						ps->stamina = 0;

					ps->airMovementState = DODGING;
				}
			}
		}

		if (checkJump && p.jumpheight > 0 && p.speed > 0.0)
		{
			if (in->movement & MOVE_UP && !(ps->flags & PS_JUMP_LOCKED))
			{
				ps->velocity[2] += p.jumpheight;

				Phys_AddEvent(ps, EVENT_JUMP, 0, 0);

				ps->flags &= ~PS_ON_GROUND;
				ps->flags |= PS_JUMP_LOCKED;
			
				ps->stamina -= p_jumpStaminaCost.integer;
				if (ps->stamina < 0)
					ps->stamina = 0;

				ps->airMovementState = JUMPING;
			}
		}
	}
	else
	{
		float lerp;
		vec3_t vel;

		M_CopyVec2(ps->intent, vel);

		//set intent to the same or greater length as velocity so we don't lose the skiing effect
		M_SetVec2Length(vel, MAX(M_GetVec2Length(ps->intent), M_GetVec2Length(ps->velocity)));

		//move towards the velocity
		lerp = p.frametime * p_aircontrol.value;
		if (lerp > 1)
			lerp = 1;
		ps->velocity[0] = LERP(lerp, ps->velocity[0], vel[0]);
		ps->velocity[1] = LERP(lerp, ps->velocity[1], vel[1]);

		//apply gravity
		ps->velocity[2] -= DEFAULT_GRAVITY * p.gravity * p.frametime;
	}
	
	Phys_ClampVelocity();	

	if (p_out)
	{
		if (ps->flags & PS_ON_GROUND && !(ps->flags & PS_DAMAGE_PUSH) && !(ps->insideSurface & SURF_PUSH_ZONE))
			Phys_SmartSlide(&p, p_out);		//only do stepping motion when we're on the ground
		else
			Phys_Slide(&p, p_out);
	}

	//set the movement animation
	Phys_SetMovementAnim();
	//set secondary anim
	Phys_SetSecondaryAnim();

	ps->flags &= ~PS_DAMAGE_PUSH;
}

/*=======================

  Phys_DriveTrace

  used by Phys_Drive to orient the vehicle axles

 =======================*/

void	Phys_DriveTrace(traceinfo_t *trace, vec3_t center, float forward, float right)
{
	vec3_t startpos,endpos;

	M_PointOnLine(center, p.forward, forward, startpos);
	M_PointOnLine(startpos, p.right, right, startpos);
	startpos[2] += 40;
	endpos[0] = startpos[0];
	endpos[1] = startpos[1];
	endpos[2] = startpos[2] - 200;

	p.tracefunc(trace, startpos, endpos, zero_vec, zero_vec, TRACE_PLAYER_MOVEMENT);
}

/*=======================

  Phys_GetAxleAngle

  transform start and end into our frame of reference and get the euler angle relative to the horizon

 =======================*/

float	Phys_GetAxleAngle(vec3_t start, vec3_t end)
{
	vec3_t v;
	M_SubVec3(end, start, v);
	M_Normalize(v);
	//w[0] = M_DotProduct(p.right, v);
	//w[1] = M_DotProduct(p.forward, v);
	//w[2] = v[2];
	
	//core.Console_Printf("%f %f\n",w[0],
	return RAD2DEG(atan(v[2]));
}

void	Phys_Drive()
{
	int mvmt = 0;
	bool stunned = false;
	vec3_t forward, right;	

	M_CopyVec3(p.forward, forward);
	M_CopyVec3(p.right, right);
	forward[2] = 0; right[2] = 0;	//we only want the 2d component for walking physics

	Phys_GetSurfaceInfo();

	M_ClearVec3(ps->intent);

	if (!ps->weaponState)
	{
		if (in->movement & MOVE_FORWARD)
		{
			mvmt = 1;		
			ps->intent[0] += forward[0];
			ps->intent[1] += forward[1];
		}	
		if (in->movement & MOVE_BACKWARD)
		{
			mvmt = -1;		
			ps->intent[0] += -forward[0];
			ps->intent[1] += -forward[1];
		}

		//turn, don't strafe
		if ((in->movement & MOVE_LEFT) && mvmt)
		{
			ps->angle[YAW] += GetObjectByType(ps->unittype)->turnRate * mvmt;
		}
		if ((in->movement & MOVE_RIGHT) && mvmt)
		{
			ps->angle[YAW] -= GetObjectByType(ps->unittype)->turnRate * mvmt;
		}
		if (ps->angle[YAW] > 360)
			ps->angle[YAW] -= 360;
		if (ps->angle[YAW] < 0)
			ps->angle[YAW] += 360;

		M_NormalizeVec2(ps->intent);	//normalize the 2d component of intent
		if (in->movement & MOVE_BACKWARD)
		{
			M_MultVec2(ps->intent, p.speed * p_backwardsSpeed.value, ps->intent);
		}
		else
		{
			M_MultVec2(ps->intent, p.speed, ps->intent);
		}
	}

	Phys_DoCombat(in, ps);

	
	if (ps->flags & PS_ON_GROUND)
	{
		float lerp;

		if (!stunned)
		{			
			lerp = p.frametime * p_groundfriction.value;
			if (lerp > 1)
				lerp = 1;
			ps->velocity[0] = LERP(lerp, ps->velocity[0], ps->intent[0]);
			ps->velocity[1] = LERP(lerp, ps->velocity[1], ps->intent[1]);
			ps->velocity[2] = 0;
		}
	}
	else
	{
		if (!stunned)
		{
			float lerp;

			//move towards the velocity
			lerp = p.frametime * p_aircontrol.value;
			if (lerp > 1)
				lerp = 1;
			ps->velocity[0] = LERP(lerp, ps->velocity[0], ps->intent[0]);
			ps->velocity[1] = LERP(lerp, ps->velocity[1], ps->intent[1]);
		}

		ps->velocity[2] -= DEFAULT_GRAVITY * p.gravity * p.frametime;
	}
	
	Phys_ClampVelocity();

	if (p_out)
	{
		if (ps->flags & PS_ON_GROUND)
			Phys_SmartSlide(&p, p_out);		//only do stepping motion when we're on the ground
		else
			Phys_Slide(&p, p_out);
	}

	//figure out our pitch and roll
	//if (ps->flags &	PS_ON_GROUND)
	{
		float lerp;
		vec3_t up = {0,0,1};
		float angle = 0;
	
		if (ps->flags & PS_ON_GROUND)
		{
			//compute the orientation of two "axels", in a cross shape passing
			//through the playerstate position

			traceinfo_t backtrace,fronttrace;
			
			Phys_DriveTrace(&backtrace, ps->pos, p.bmin[1], 0);
			Phys_DriveTrace(&fronttrace, ps->pos, p.bmax[1], 0);

			if (backtrace.fraction > 0 && fronttrace.fraction > 0 &&
				backtrace.fraction < 1 && fronttrace.fraction < 1)
			{
				angle = Phys_GetAxleAngle(backtrace.endpos, fronttrace.endpos);
				if (angle > 70)
					angle = 70;
				else if (angle < -70)
					angle = -70;

				lerp = p.frametime * 4;
				if (lerp > 1)
					lerp = 1;

				ps->angle[PITCH] = LERP(lerp, ps->angle[PITCH], angle);
			}
			else
				angle = 0;

			Phys_DriveTrace(&backtrace, ps->pos, 0, p.bmin[0]);
			Phys_DriveTrace(&fronttrace, ps->pos, 0, p.bmax[0]);

			if (backtrace.fraction > 0 && fronttrace.fraction > 0 &&
				backtrace.fraction < 1 && fronttrace.fraction < 1)
			{
				angle = Phys_GetAxleAngle(backtrace.endpos, fronttrace.endpos);
				if (angle > 45)
					angle = 45;
				else if (angle < -45)
					angle = -45;
			}
			else
				angle = 0;

			lerp = p.frametime * 4;
			if (lerp > 1)
				lerp = 1;

			ps->angle[ROLL] = LERP(lerp, ps->angle[ROLL], -angle);
		}
/*		
		if (ps->flags & PS_ON_GROUND)
		{		
			vec3_t startpos,endpos;
			traceinfo_t trace;

			M_PointOnLine(ps->pos, p.right, 5, startpos);
			M_PointOnLine(ps->pos, p.right, -5, endpos);
			//startpos[2] += 10;
			//endpos[2] += 10;

			p.tracefunc(&trace, startpos, endpos, p.bmin, p.bmax, TRACE_PLAYER_MOVEMENT);
			if (trace.fraction < 1 && trace.fraction > 0)
			{
				vec3_t v,w;
				M_SubVec3(trace.endpos, ps->pos, v);
				M_Normalize(v);
				w[0] = M_DotProduct(p.right, v);
				w[1] = M_DotProduct(p.forward, v);
				w[2] = v[2];
				//core.Console_Printf("%f %f\n",w[0],
				angle = RAD2DEG(atan(w[2])) * 0.8;
				if (angle > 25)
					angle = 25;
				else if (angle < -25)
					angle = -25;
			}
			else
			{
				angle = 0;
			}
		}
		else
		{
			angle = 0;
		}
		
		lerp = p.frametime * 2;
		if (lerp > 1)
			lerp = 1;

		ps->angle[ROLL] = LERP(lerp, ps->angle[ROLL], -angle);
*/
	}
/*	else
	{
		ps->angle[ROLL] = 0;
		ps->angle[PITCH] = 0;
	}*/

	//set the appropriate move animation
	Phys_SetDrivingAnimState();
}




/*==========================

  Phys_FreeFly

  spectate mode free fly

  ==========================*/

void	Phys_FreeFly()
{
	float lerp;
	vec3_t forward, right;

	if (p_freeflyAngleLerp.value < 1)
	{
		//note: the following only works correctly with cl_syncAngles 0
		//right now the effect can only really work with sync angles on by doing a client side camera
		//lag, which is a limitation of the current system
		ps->angle[PITCH] = M_LerpAngle(p_freeflyAngleLerp.value, ps->angle[PITCH], WORD2ANGLE(in->pitch));
		ps->angle[ROLL] = 0;
		ps->angle[YAW] = M_LerpAngle(p_freeflyAngleLerp.value, ps->angle[YAW], WORD2ANGLE(in->yaw));
	}
	else
	{
		ps->angle[PITCH] = WORD2ANGLE(in->pitch);
		ps->angle[ROLL] = 0;
		ps->angle[YAW] = WORD2ANGLE(in->yaw);
	}

	HeadingFromAngles(ps->angle[PITCH], ps->angle[YAW], forward, right);

	M_ClearVec3(ps->intent);

	if (in->movement & MOVE_FORWARD)
	{
		M_AddVec3(ps->intent, forward, ps->intent);
	}
	if (in->movement & MOVE_LEFT)
	{
		M_SubVec3(ps->intent, right, ps->intent);
	}
	if (in->movement & MOVE_RIGHT)
	{
		M_AddVec3(ps->intent, right, ps->intent);
	}
	if (in->movement & MOVE_BACKWARD)
	{
		M_SubVec3(ps->intent, forward, ps->intent);
	}
	if (in->movement & MOVE_UP)
	{
		M_AddVec3(ps->intent, up, ps->intent);
	}
	if (in->movement & MOVE_DOWN)
	{
		M_SubVec3(ps->intent, up, ps->intent);
	}

	M_SetVec3Length(ps->intent, (in->buttons & B_SPRINT) ? 1000 : 500);

	lerp = p.frametime * p_freeflyAccel.value;
	if (lerp > 1)
		lerp = 1;

	M_LerpVec3(lerp, ps->velocity, ps->intent, ps->velocity);

	Phys_ClampVelocity();

	if (p_out)
		Phys_SmartSlide(&p, p_out);
	
}

//this keeps dead bodies from just hanging in the air
//they should fall, and keep the momentum of whatever killed them, but that's all
void	Phys_Dead()
{
	ps->angle[PITCH] = WORD2ANGLE(in->pitch);
	ps->angle[ROLL] = 0;
	ps->angle[YAW] = WORD2ANGLE(in->yaw);

	if (ps->flags & PS_ON_GROUND)
	{
		int n;

	//	ps->velocity[2] = 0;
		float mult = p.frametime * 300;

		for (n=0; n<3; n++)
		{
			if (ps->velocity[n] < 0)
			{
				ps->velocity[n] += mult;
				if (ps->velocity[n] > 0)
					ps->velocity[n] = 0;
			}
			else
			{
				ps->velocity[n] -= mult;
				if (ps->velocity[n] < 0)
					ps->velocity[n] = 0;
			}			
		}
	}
	else
		ps->velocity[2] -= DEFAULT_GRAVITY * p.gravity * p.frametime;	

	Phys_ClampVelocity();
	
		//slight hack to prevent discrepancies between the dead player and the corpse that gets generated by the gamecode
		//ps->pos[0] = (int)ps->pos[0];
		//ps->pos[1] = (int)ps->pos[1];
		//ps->pos[2] = (int)ps->pos[2];
		//ps->angle[0] = BYTE2ANGLE(ANGLE2BYTE(ps->angle[0]));
		//ps->angle[1] = BYTE2ANGLE(ANGLE2BYTE(ps->angle[1]));
		//ps->angle[2] = BYTE2ANGLE(ANGLE2BYTE(ps->angle[2]));	

	if (p_out)
		Phys_Slide(&p, p_out);

	Phys_GetSurfaceInfo();
}


//advance the player state by 1 frame
void	Phys_AdvancePlayerState(playerState_t *playerState, int team, inputState_t *inputState, inputState_t *oldInputState,
								bool (*tracefunc)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface),
								objectData_t *(*objTypeFunc)(int type),
								phys_out_t *phys_out)
{
	float diff;

	Phys_SetupParams(playerState->unittype, team, playerState->states, playerState->pos, playerState->velocity, tracefunc, objTypeFunc, (float)inputState->delta_msec * 0.001, &p);
	

	ps = playerState;
	in = inputState;
	oldin = oldInputState;
	memset(phys_out, 0, sizeof(phys_out_t));
	p_out = phys_out;
	
	ps->inputSequence = in->sequence;

	//set up viewing angles

	M_CopyVec3(ps->angle, oldAngles);

	if (ps->flags & PS_EXTERNAL_ANGLES)		//the server code is setting angles for us
	{
		//modify the inputstate angles
		inputState->pitch = ANGLE2WORD(ps->angle[PITCH]);
		inputState->yaw = ANGLE2WORD(ps->angle[YAW]);
		inputState->roll = ANGLE2WORD(ps->angle[ROLL]);
	}

	if (team)
	{
		objectData_t	*item = GetObjectByType(ps->inventory[ps->item]);

		unit = GetObjectByType(playerState->unittype);		

		if (ps->health <= 0)
		{
			ps->phys_mode = PHYSMODE_DEAD;
			//return;
		}
		else if (!ps->weaponState)		//if we're not doing something with a weapon or melee attack
		{
			//allow the player to select an item in their inventory	
			ps->item = in->item % MAX_INVENTORY;

			if (ps->item)
			{
				if (!ps->inventory[ps->item])
					ps->item = 0;
			}
		}

		//adjust focus based on mouse movement
		diff = fabs(fabs(WORD2ANGLE(inputState->pitch)) - fabs(WORD2ANGLE(oldInputState->pitch)));
		diff +=	fabs(fabs(WORD2ANGLE(inputState->yaw)) - fabs(WORD2ANGLE(oldInputState->yaw)));

		if (diff > p_focusMinAngleChange.value)
		{
			ps->focus -= (diff / p_focusMaxAngleChange.value) * item->focusDegradeRate * p.frametime * p_focusAngleAmplify.value;
			if (ps->focus < item->focusPenalty)
				ps->focus = item->focusPenalty;
		}
		else if (ps->focus < 1.0 && ps->weaponState != AS_WPSTATE_FIRE)
		{
			//recover degraded focus
			ps->focus += item->focusRecoverRate * p.frametime;
			if (ps->focus > 1.0)
				ps->focus = 1.0;
		}	
	}
	else
	{
		ps->focus = 1.0;
		ps->phys_mode = PHYSMODE_FREEFLY;
	}

	HeadingFromAngles(ps->angle[PITCH], ps->angle[YAW], p.forward, p.right);
	p.aircontrol *= /*p.frametime */ PHYSICS_SCALE * p_aircontrol.value;
	p.friction *= /*p.frametime */ PHYSICS_SCALE * p_friction.value;
	p.gravity *= /*p.frametime */ PHYSICS_SCALE * p_gravity.value;
	p.speed *= /*p.frametime */ PHYSICS_SCALE * p_speed.value;
	p.jumpheight *= p_jumpheight.value * PHYSICS_SCALE;

	p.stepheight *= p_stepheight.value;

	switch(ps->phys_mode)
	{
		case PHYSMODE_FREEFLY:	//fly around
			Phys_FreeFly();
			break;
		case PHYSMODE_WALK: //walk around
			Phys_Walk();
			break;
		case PHYSMODE_DRIVE:
			Phys_Drive();
			break;
		case PHYSMODE_DEAD:
			Phys_Dead();
			return;		//don't bother with anything else
	}
}


/*==========================

  Phys_GetSpeed

  ==========================*/

float	Phys_GetSpeed(byte unittype, int team, byte states[MAX_STATE_SLOTS])
{
	int				n;
	float			speed, add = 0.0, mul = 1.0;
	objectData_t	*obj = GetObjectByType(unittype);

	if (!IsUnitType(unittype))
		return 0.0;

	speed = obj->speed;

	//adjust for upgrades
	for (n = 0; n < MAX_STATE_SLOTS; n++)
	{
		if (states[n])
		{
	 		mul *= stateData[states[n]].speedMult;
			add += stateData[states[n]].speedAdd;
		}
	}

	speed *= mul;
	speed += add;
	return speed;
}


/*==========================

  Phys_GetJumpHeight

 ==========================*/

float	Phys_GetJumpHeight(byte unittype, int team, byte states[MAX_STATE_SLOTS])
{
	int				n;
	float			jump, add = 0.0, mul = 1.0;
	objectData_t	*obj = GetObjectByType(unittype);

	if (!IsUnitType(unittype))
		return 0.0;

	jump = obj->jumpheight;

	//adjust for upgrades
	for (n = 0; n < MAX_STATE_SLOTS; n++)
	{
		if (states[n])
		{
	 		mul *= stateData[states[n]].jumpMult;
			add += stateData[states[n]].jumpAdd;
		}
	}

	jump *= mul;
	jump += add;
	return jump;
}


/*==========================

  Phys_SetupParams

 ==========================*/

void		Phys_SetupParams(byte unittype, int team, byte states[MAX_STATE_SLOTS], vec3_t pos, vec3_t velocity,
					 	bool (*tracefunc)(traceinfo_t *result, const vec3_t start, const vec3_t end, const vec3_t bmin, const vec3_t bmax, int ignoreSurface),
						objectData_t *(*objectTypeFunc)(int type),
						float frametime,
						physicsParams_t *out)
{
	objectData_t *unit = objectTypeFunc(unittype);	

	memset(out, 0, sizeof(*out));

	out->pos = pos;
	out->velocity = velocity;

  	out->frametime = frametime;
	out->tracefunc = tracefunc;
	out->objectTypeFunc = objectTypeFunc;

	if (unittype)
	{
		out->aircontrol = unit->aircontrol;
		out->friction = unit->friction;
		out->gravity = unit->gravity;

		p = *out;		//do this so that Phys_GetJumpHeight() and Phys_GetSpeed() don't barf on GetObjectByType

		if (states)
			out->jumpheight = Phys_GetJumpHeight(unittype, team, states);
		else
			out->jumpheight = unit->jumpheight;

		if (states)
			out->speed = Phys_GetSpeed(unittype, team, states);
		else
			out->speed = unit->speed;
		out->stepheight = unit->stepheight;
		M_CopyVec3(unit->bmin, out->bmin);
		M_CopyVec3(unit->bmax, out->bmax);
	}
	else
	{		
		//used for freefly spectate mode
		out->stepheight = 0;
		M_SetVec3(out->bmin, -10, -10, -10);
		M_SetVec3(out->bmax, 10, 10, 10);
	}
}


/*==========================

  Phys_Init

 ==========================*/

void	Phys_Init()
{
	core.Cvar_Register(&p_gravity);
	core.Cvar_Register(&p_jumpheight);
	core.Cvar_Register(&p_friction);
	core.Cvar_Register(&p_speed);
	core.Cvar_Register(&p_aircontrol);
	core.Cvar_Register(&p_stepheight);	
	core.Cvar_Register(&p_groundfriction);
	core.Cvar_Register(&p_lunge);
	core.Cvar_Register(&p_minslope);
	core.Cvar_Register(&p_attackingSpeed);
	core.Cvar_Register(&p_sprintSpeed);
	core.Cvar_Register(&p_backwardsSpeed);
	core.Cvar_Register(&p_strafeSpeed);
	core.Cvar_Register(&p_cooldownRate);
	core.Cvar_Register(&p_blockSpeed);
	core.Cvar_Register(&p_autoRunTime);
	core.Cvar_Register(&p_autoRunSpeed);
	core.Cvar_Register(&p_slowWeaponAttack);
	core.Cvar_Register(&p_levelHealthBonus);
	core.Cvar_Register(&p_sprintBurst);
	core.Cvar_Register(&p_maxBlockTime);
	core.Cvar_Register(&p_fallingDamage);
	core.Cvar_Register(&p_fallingDamageSpeed);
	core.Cvar_Register(&p_landDuration);
	core.Cvar_Register(&p_jumpStaminaCost);
	core.Cvar_Register(&p_dodgeStaminaCost);
	core.Cvar_Register(&p_sprintRegainLandDelay);
	core.Cvar_Register(&p_sprintRegainDelay);
	core.Cvar_Register(&p_staminaDepleteSpeed);
	core.Cvar_Register(&p_sprintDeadZone);
	core.Cvar_Register(&p_sprintRegenSlow);
	core.Cvar_Register(&p_sprintRegenFast);
	core.Cvar_Register(&p_freeflyAccel);
	core.Cvar_Register(&p_freeflyAngleLerp);

	core.Cvar_Register(&p_focusMinAngleChange);
	core.Cvar_Register(&p_focusMaxAngleChange);
	core.Cvar_Register(&p_focusAngleAmplify);
}
