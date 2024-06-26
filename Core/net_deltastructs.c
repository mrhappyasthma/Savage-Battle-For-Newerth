// (C) 2001 S2 Games

// net_deltastructs.c

#include "core.h"

//to save a bit of bandwidth, it's a good idea to put the most frequently
//changing fields at the start of a deltaUpdateStruct array, as this will
//reduce the number of update flag bits that need to be sent

//baseObject_t
deltaUpdateStruct_t baseObjectDesc[] = 
{
	{ T_COORD,			offsetof(baseObject_t,	pos[0]) },
	{ T_COORD,			offsetof(baseObject_t,	pos[1]) },
	{ T_COORD,			offsetof(baseObject_t,	pos[2]) },
	{ T_BYTE_ANGLE,		offsetof(baseObject_t,	angle[0]) },
	{ T_BYTE_ANGLE,		offsetof(baseObject_t,	angle[1]) },
	{ T_BYTE_ANGLE,		offsetof(baseObject_t,	angle[2]) },
	{ T_SHORT15,		offsetof(baseObject_t,	owner) },
	{ T_BYTE,			offsetof(baseObject_t,	animState) },
	{ T_BYTE,			offsetof(baseObject_t,	animState2) },
	{ T_INT,			offsetof(baseObject_t,	health) },
	{ T_BYTE,			offsetof(baseObject_t,	weapon) },
	{ T_BYTE,			offsetof(baseObject_t,	percentToComplete) },	
	{ T_BYTE,			offsetof(baseObject_t,	type) },
	{ T_BYTE,			offsetof(baseObject_t,	team) },
	{ T_SHORT15,		offsetof(baseObject_t,	flags) },
	{ T_SHORT15,		offsetof(baseObject_t,	exflags) },
	{ T_BYTE,			offsetof(baseObject_t,	assignedToClient) },
	{ T_COORD,			offsetof(baseObject_t,	traj.origin[0]) },
	{ T_COORD,			offsetof(baseObject_t,	traj.origin[1]) },
	{ T_COORD,			offsetof(baseObject_t,	traj.origin[2]) },
	{ T_COORD,			offsetof(baseObject_t,	traj.velocity[0]) },
	{ T_COORD,			offsetof(baseObject_t,	traj.velocity[1]) },
	{ T_COORD,			offsetof(baseObject_t,	traj.velocity[2]) },
	{ T_FLOAT,			offsetof(baseObject_t,	traj.gravity) },
	{ T_FLOAT,			offsetof(baseObject_t,	traj.acceleration) },
	{ T_INT,			offsetof(baseObject_t,	traj.startTime) },
	{ T_BYTE,			offsetof(baseObject_t,	states[0]) },
	{ T_BYTE,			offsetof(baseObject_t,	states[1]) },
	{ T_BYTE,			offsetof(baseObject_t,	states[2]) },
	{ T_BYTE,			offsetof(baseObject_t,	states[3]) },
	{ T_BYTE,			offsetof(baseObject_t,	states[4]) },
	{ T_BYTE,			offsetof(baseObject_t,	states[5]) },
	{ T_BYTE,			offsetof(baseObject_t,	states[6]) },
	{ T_BYTE,			offsetof(baseObject_t,	states[7]) },
	{ T_INT,			offsetof(baseObject_t,	fullhealth) },
	{ T_INT,			offsetof(baseObject_t,	surfaceFlags) },
	{ T_BYTE,			offsetof(baseObject_t,	nameIdx) },
	
	{ 0, 0 }
};

deltaUpdateStruct_t	inputStateDesc[] = 
{
	{ T_BYTE,			offsetof(inputState_t,	delta_msec) },
	{ T_SHORT,			offsetof(inputState_t,	pitch) },
	{ T_SHORT,			offsetof(inputState_t,	yaw) },
	{ T_BYTE,			offsetof(inputState_t, movement) },
	{ T_BYTE,			offsetof(inputState_t, buttons) },
	{ T_BYTE,			offsetof(inputState_t, item) },
	{ T_SHORT,			offsetof(inputState_t, roll) },
	{ 0, 0 }
};

//playerState_t
deltaUpdateStruct_t	playerStateDesc[] = 
{
	{ T_INT,	offsetof(playerState_t,		inputSequence) },
	{ T_FLOAT,	offsetof(playerState_t,		pos[0]) },
	{ T_FLOAT,	offsetof(playerState_t,		pos[1]) },
	{ T_FLOAT,	offsetof(playerState_t,		pos[2]) },
	{ T_WORD_ANGLE,	offsetof(playerState_t,		angle[0]) },
	{ T_WORD_ANGLE,	offsetof(playerState_t,		angle[1]) },
	{ T_WORD_ANGLE,	offsetof(playerState_t,		angle[2]) },
	{ T_FLOAT,	offsetof(playerState_t,		velocity[0]) },
	{ T_FLOAT,	offsetof(playerState_t,		velocity[1]) },
	{ T_FLOAT,	offsetof(playerState_t,		velocity[2]) },
	{ T_FLOAT,	offsetof(playerState_t,		focus) },
	{ T_BYTE,	offsetof(playerState_t,		animState) },
	{ T_BYTE,	offsetof(playerState_t,		animState2) },	
	{ T_BYTE,	offsetof(playerState_t,		weaponState) },	
	{ T_INT,	offsetof(playerState_t,		stamina) },
	{ T_BYTE,	offsetof(playerState_t,		airMovementState) },
	{ T_INT,	offsetof(playerState_t,		health) },
	{ T_SHORT,	offsetof(playerState_t,		flags) },
	{ T_BYTE,	offsetof(playerState_t,		stunFlags) },
	{ T_INT,	offsetof(playerState_t,		stunTime) },
	{ T_BYTE,	offsetof(playerState_t,		attackFlags) },
	{ T_INT,	offsetof(playerState_t,		landTime) },
	{ T_INT,	offsetof(playerState_t,		walkTime) },
	{ T_FLOAT,	offsetof(playerState_t,		charge) },
	{ T_INT,	offsetof(playerState_t,		overheatCounter) },
	{ T_INT,	offsetof(playerState_t,		wpStateStartTime) },
	{ T_INT,	offsetof(playerState_t,		wpAnimStartTime) },
	{ T_SHORT15,offsetof(playerState_t,		attacker) },
	{ T_BYTE,	offsetof(playerState_t,		item) },
	{ T_BYTE,	offsetof(playerState_t,		phys_mode) },
	{ T_INT,	offsetof(playerState_t,		respawnTime) },
	{ T_INT,	offsetof(playerState_t,		invincibleTime) },
	{ T_INT,	offsetof(playerState_t,		insideSurface) },

	{ T_INT,	offsetof(playerState_t,		score.kills) },
	{ T_INT,	offsetof(playerState_t,		score.deaths) },
	{ T_FLOAT,	offsetof(playerState_t,		score.experience) },
	{ T_INT,	offsetof(playerState_t,		score.level) },
	{ T_INT,	offsetof(playerState_t,		score.money) },
	{ T_INT,	offsetof(playerState_t,		score.loyalty) },
	{ T_INT,	offsetof(playerState_t,		score.ping) },

	{ T_SHORT,	offsetof(playerState_t,		mana) },
	{ T_SHORT,	offsetof(playerState_t,		clip[0]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[1]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[2]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[3]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[4]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[5]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[6]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[7]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[8]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[9]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[10]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[11]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[12]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[13]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[14]) },
	{ T_SHORT,	offsetof(playerState_t,		clip[15]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[0]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[1]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[2]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[3]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[4]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[5]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[6]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[7]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[8]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[9]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[10]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[11]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[12]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[13]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[14]) },
	{ T_SHORT,	offsetof(playerState_t,		ammo[15]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[0]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[1]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[2]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[3]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[4]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[5]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[6]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[7]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[8]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[9]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[10]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[11]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[12]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[13]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[14]) },
	{ T_BYTE,	offsetof(playerState_t,		inventory[15]) },
	{ T_BYTE,	offsetof(playerState_t,		unittype) },
	{ T_INT,	offsetof(playerState_t,		fullhealth) },
	{ T_INT,	offsetof(playerState_t,		maxstamina) },
	{ T_SHORT15,offsetof(playerState_t,		chaseIndex) },
	{ T_FLOAT,	offsetof(playerState_t,		chasePos[0]) },
	{ T_FLOAT,	offsetof(playerState_t,		chasePos[1]) },
	{ T_FLOAT,	offsetof(playerState_t,		chasePos[2]) },
	{ T_BYTE,	offsetof(playerState_t,		status) },
	{ T_BYTE,	offsetof(playerState_t,		statusMessage) },
	
	{ T_BYTE,	offsetof(playerState_t,		states[0]) },
	{ T_BYTE,	offsetof(playerState_t,		states[1]) },
	{ T_BYTE,	offsetof(playerState_t,		states[2]) },
	{ T_BYTE,	offsetof(playerState_t,		states[3]) },
	{ T_BYTE,	offsetof(playerState_t,		states[4]) },
	{ T_BYTE,	offsetof(playerState_t,		states[5]) },
	{ T_BYTE,	offsetof(playerState_t,		states[6]) },
	{ T_BYTE,	offsetof(playerState_t,		states[7]) },
	{ T_INT,	offsetof(playerState_t,		stateExpireTimes[0]) },
	{ T_INT,	offsetof(playerState_t,		stateExpireTimes[1]) },
	{ T_INT,	offsetof(playerState_t,		stateExpireTimes[2]) },
	{ T_INT,	offsetof(playerState_t,		stateExpireTimes[3]) },
	{ T_INT,	offsetof(playerState_t,		stateExpireTimes[4]) },
	{ T_INT,	offsetof(playerState_t,		stateExpireTimes[5]) },
	{ T_INT,	offsetof(playerState_t,		stateExpireTimes[6]) },
	{ T_INT,	offsetof(playerState_t,		stateExpireTimes[7]) },

	{ T_BYTE,	offsetof(playerState_t,		clientnum) },
	
	{ 0, 0 }
};

void	Net_RegisterDeltaStructs()
{
	Net_RegisterDeltaStruct(baseObjectDesc);
	Net_RegisterDeltaStruct(playerStateDesc);
	Net_RegisterDeltaStruct(inputStateDesc);
}
