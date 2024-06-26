/*
 * (C) 2003 S2 Games
 *
 * sv_gamescript.c'
 */

/*=============================================================================
Server side scripts to handle functionality of objects in game.  Hopefully most
stuff can be done through these eventually, but at the very least this should
help clean up the object files.  This should also be a good way to implement
map triggers.

Script format:
Blank lines are ignored, any other line must start with a special character:
# - Comment
@ - Entry point
! - Instruction

When the script is parsed, instructions are appended to the end of one of
several linked lists.  The last entry point directive determines which list
the current instruction is being added to.  When something in game triggers
an entry point of an object, the game will step through the isntructions in
that list and execute them.

#[comment text] ...
  Ignored, just like a blank line

@<entry point name> [frequency] | [min_frequency max_frequency]
  See gsEntryNamesp[] below for valid entry points.  Not all entry points
  require a frequency, unnecessary frequency specifications are ignored.

!<instruction name> <target> [param] ...
  See gameScriptDictionary[] below for valid commands.  See gsTargetNames[]
  below for valid target names.  Instructions simply serve as a reference
  to a function in the game code, which accepts a target and parameter list
  as it's parameters.  Paramaters are stored in a lnked list of structures
  conatining a float int and char * component, like cvars.
=============================================================================*/

#include "server_game.h"

cvar_t	sv_reviveMoneyReward = { "sv_reviveMoneyReward", "500" };

//=============================================================================
//=============================================================================
char *gsEntryNames[] =
{
	"NULL",

	"pickup",	//item was added to a player's inventory
	"drop",		//item was removed from a player's inventory
	"toss",		//object was just created via a "toss" command
	"attach",	//object was just created via an "attach" command

	"use",		//player used an item from their inventory, or a powerup was cast on a player
	"fizzle",	//object attempted to activate but failed
	"spawn",	//object just entered the world

	"idle",		//object became idle
	"idling",	//an object resting in the world
	"activate",	//delay timer expired, or otherwise manually activated
	"active",	//an object in the world after being activated
	"sleep",	//object became inactive
	"sleeping",	//object is inactive

	"impact",	//projectile hit something
	"fuse",		//a projectile's fuse expired
	"backfire",	//someone held the trigger down too long... oops

	"wounded",	//object took damage
	"die",		//object took fatal damage

	""
};

char *gsTargetNames[] = 
{
	"self",		//object that is acting
	"target",	//object that initiated the action
	"enemy",	//object's current enemy
	"owner",	//object's owner, if any
	"link",		//object's link, if any

	""
};

enum
{
	GS_TESTCASE_ALLY,
	GS_TESTCASE_ENEMY,
	GS_TESTCASE_NEUTRAL,

	GS_TESTCASE_CHARACTER,
	GS_TESTCASE_SIEGE,
	GS_TESTCASE_NPC,

	GS_NUM_TESTCASES
};

char *gsTestCaseNames[] =
{
	"ally",
	"enemy",
	"neutral",

	"character",
	"siege",
	"npc",

	""
};

gameScript_t	*objectScripts[MAX_OBJECT_TYPES][NUM_GS_ENTRIES];

#define GS_ERROR_BUFFER_LEN	256
char	gsErrorBuffer[GS_ERROR_BUFFER_LEN];
//=============================================================================
//=============================================================================


#define GS_FUNC(name)	gameScriptResult_t	GS_##name(serverObject_t *self, int type, serverObject_t *target, gameScriptParam_t *params)
#define GS_ERROR(text)	{strncpySafe(gsErrorBuffer, text, GS_ERROR_BUFFER_LEN); return GS_RESULT_ERROR;}
//=============================================================================
//=============================================================================

GS_FUNC(Heal)
{
	if (!params)
		GS_ERROR("Not enough parameters");

	if (!target)
		GS_ERROR("Invalid target");

	//don't heal the dead
	if (target->base.health <= 0)
		return GS_RESULT_FAIL;

	if (params->value < 1.0)	//heal a percentage
		target->base.health += params->value * GetObjectByType(target->base.type)->fullHealth;
	else	//heal a fixed amount
		target->base.health += params->integer;

	if (target->base.health > target->adjustedStats.fullhealth)
		target->base.health = target->adjustedStats.fullhealth;

	if (target->client)
	{
		target->client->ps.health = target->base.health;

		if (self)
		{
			serverObject_t *owner = self->owner ? self->owner : self;

			if (owner->client)
			{
				if (target->base.health < target->adjustedStats.fullhealth)	//this isn't quite right but good enough
					SV_AddExperience(owner->client, EXPERIENCE_PLAYER_HEAL, params->integer, 1.0);
			}
		}
	}


	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(GiveMana)
{
	if (!params)
		GS_ERROR("Not enough parameters");

	if (!target)
		GS_ERROR("Invalid target");

	if (!target->client)
		GS_ERROR("Target must be a client");

	if (!GetObjectByType(target->base.type)->maxMana)
		GS_ERROR("Target does not use mana");

	target->client->ps.mana += params->integer;
	if (target->client->ps.mana > GetObjectByType(target->client->ps.unittype)->maxMana)
		target->client->ps.mana = GetObjectByType(target->client->ps.unittype)->maxMana;

	if (target->client->ps.mana < 0)
		target->client->ps.mana = 0;

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(GiveStamina)
{
	int max;

	if (!params)
		GS_ERROR("Not enough parameters");

	if (!target)
		GS_ERROR("Invalid target");

	if (!target->client)
		GS_ERROR("Target must be a client");

	max = target->client->ps.maxstamina;

	target->client->ps.stamina += params->integer;

	if (target->client->ps.stamina > max)
		target->client->ps.stamina = max;

	if (target->client->ps.stamina < 0)
		target->client->ps.stamina = 0;
	
	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(GiveAmmo)
{
	int					index;
	float				mult;
	gameScriptParam_t	*at = params;

	if (!at)
		GS_ERROR("Not enough parameters");

	if (!target)
		GS_ERROR("Invalid target");

	if (!target->client)
		GS_ERROR("Target must be a client");

	mult = at->value;
	at = at->next;
	if (!at)
		GS_ERROR("Not enough parameters");

	for (index = 0; index < MAX_INVENTORY; index++)
	{
		if (!IsWeaponType(target->client->ps.inventory[index]))
			continue;

		if (!stricmp(at->string, "full"))
			target->client->ps.ammo[index] = GetObjectByType(target->client->ps.inventory[index])->ammoMax * mult;
		else if (!stricmp(at->string, "start"))
			target->client->ps.ammo[index] = GetObjectByType(target->client->ps.inventory[index])->ammoStart * mult;
		else if (!stricmp(at->string, "group"))
			target->client->ps.ammo[index] += GetObjectByType(target->client->ps.inventory[index])->ammoGroup * mult;
		else
			target->client->ps.ammo[index] += at->integer * mult;
	}

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Teleport)
{
	gameScriptParam_t	*at = params;
	vec3_t	point;

	if (!at)
		GS_ERROR("Not enough parameters");

	if (!target)
		GS_ERROR("Invalid target");

	if (!stricmp(at->string, "home"))
	{
		SV_GetSpawnPointFromBuilding(&sl.objects[sl.teams[SV_GetTeam(target->base.index)].command_center],
		point, sl.clients[target->base.index].ps.unittype);
	}
	else if (!stricmp(at->string, "here"))
	{
		SV_GetPosition(self->base.index, point);
	}
	else if (!stricmp(at->string, "link"))
	{
		if (!target->link)
			return GS_RESULT_FAIL;

		if (!target->link->base.active)
			return GS_RESULT_FAIL;

		SV_GetSpawnPointAroundObject(target->link->base.type, target->link->base.pos, NULL, target->base.type, point);
	}
	else
	{
		float x, y, z;

		x = at->value;
		at = at->next;
		if (!at)
			GS_ERROR("Not enough parameters");
		y = at->value;
		at = at->next;
		if (!at)
			z = cores.World_CalcMaxZ(
				x + GetObjectByType(target->base.type)->bmin[X],
				y + GetObjectByType(target->base.type)->bmin[Y],
				x + GetObjectByType(target->base.type)->bmax[X],
				y + GetObjectByType(target->base.type)->bmax[Y]) + 10;
		else
			z = at->value;

		M_SetVec3(point, x, y, z);
	}

	if (target->client)
	{
		SV_SpawnPlayerFrom(target->client, sl.teams[target->client->info.team].command_center);
		M_CopyVec3(point, target->client->ps.pos);
	}
	M_CopyVec3(point, target->base.pos);

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(GiveState)
{
	gameScriptParam_t	*at = params;
	int n, statenum = 0, duration;

	if (!at)
		GS_ERROR("Not enough parameters");

	if (!target)
		GS_ERROR("Invalid target");

	for (n = 0; n < MAX_STATES; n++)
	{
		if (!stricmp(at->string, stateData[n].name))
		{
			statenum = n;
			break;
		}
	}
	if (!statenum)
		GS_ERROR("Invalid state");

	at = at->next;
	if (!at)
		GS_ERROR("Not enough parameters");

	duration = at->integer;

	if (!SV_ApplyStateToObject((self) ? self->base.index : target->base.index, target->base.index, statenum, duration))
		return GS_RESULT_FAIL;

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Toss)
{
	gameScriptParam_t	*at = params;
	float	velocity, gravity;
	serverObject_t	*obj;

	if (!at)
		GS_ERROR("Not enough parameters");

	if (!target)
		GS_ERROR("Invalid target");

	velocity = at->value;
	at = at->next;
	if (!at)
		GS_ERROR("Not enough parameters");

	gravity = at->value;

	//spawn it
	obj = SV_AllocObject((byte)type, SV_GetTeam(target->base.index));

	//set the possition
	SV_GetPosition(target->base.index, obj->base.pos);
	
	//arbitrary height
	obj->base.pos[2] += objData[target->base.type].viewheight + GetObjectByType(type)->muzzleOffset[2];
		
	//the toss
	if (target->client)
		M_CopyVec3(target->client->forward, obj->velocity);
	else
		M_CopyVec3(target->forward, obj->velocity);
	M_Normalize(obj->velocity);
	M_MultVec3(obj->velocity, velocity, obj->velocity);

	//bounding box
	M_CopyVec3(GetObjectByType(type)->bmin, obj->base.bmin);
	M_CopyVec3(GetObjectByType(type)->bmax, obj->base.bmax);
	obj->base.surfaceFlags = SURF_TRANSPARENT_ITEM;
	
	M_CopyVec3(obj->base.pos, obj->base.traj.origin);
	obj->base.traj.startTime = sl.gametime;
	obj->base.traj.acceleration = 0.0;
	obj->base.traj.gravity = gravity;
	M_CopyVec3(obj->velocity, obj->base.traj.velocity);
	obj->base.flags |= BASEOBJ_USE_TRAJECTORY | BASEOBJ_SNAP_TO_MUZZLE;
			
	obj->owner = target;
	obj->base.owner = target->base.index;
	obj->base.team = SV_GetTeam(target->base.index);

	obj->base.health = GetObjectByType(type)->fullHealth;
	obj->base.fullhealth = GetObjectByType(type)->fullHealth;
	obj->damage = SV_ItemDamage;
	obj->frame = SV_ItemSleepFrame;
	obj->kill = SV_ItemKill;
	obj->nextDecisionTime = 0;
	obj->base.animState = AS_ITEM_SLEEP;

	cores.Server_SpawnObject(obj, obj->base.index);

	if (GetObjectByType(type)->linkToOwner)
	{
		//destroy client's linked object, if it exists
		if (target->link)
		{
			if (target->link->kill)
				target->link->kill(target->link, NULL, target->link->base.pos, 0, 0);
		}
		target->link = obj;
	}
	
	SV_GameScriptExecute(obj, target, obj->base.type, GS_ENTRY_TOSS);

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Attach)
{
	serverObject_t	*obj;

	if (!target)
		GS_ERROR("Invalid target");

	//spawn it
	obj = SV_AllocObject((byte)type, SV_GetTeam(target->base.index));

	//set the possition
	////
	obj->base.surfaceFlags = SURF_TRANSPARENT_ITEM;
	obj->base.flags |= BASEOBJ_ATTACHED_TO_OWNER;
			
	obj->owner = target;
	obj->base.team = SV_GetTeam(target->base.index);

	obj->base.health = GetObjectByType(type)->fullHealth;
	obj->base.fullhealth = GetObjectByType(type)->fullHealth;
	obj->damage = SV_ItemDamage;
	obj->frame = SV_ItemSleepFrame;
	obj->kill = SV_ItemKill;
	obj->nextDecisionTime = 0;
	obj->base.animState = AS_ITEM_SLEEP;

	cores.Server_SpawnObject(obj, obj->base.index);
	
	SV_GameScriptExecute(obj, target, obj->base.type, GS_ENTRY_ATTACH);

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(GiveStateRadius)
{
	gameScriptParam_t	*at = params;
	float	radius;
	int		n, statenum = 0, duration;

	if (!at)
		GS_ERROR("Not enough parameters");

	if (!target)
		GS_ERROR("Invalid target");

	radius = at->value;
	at = at->next;
	if (!at)
		GS_ERROR("Not enough parameters");

	for (n = 0; n < MAX_STATES; n++)
	{
		if (!stricmp(at->string, stateData[n].name))
		{
			statenum = n;
			break;
		}
	}
	if (!statenum)
		GS_ERROR("Invalid state");

	at = at->next;
	if (!at)
		GS_ERROR("Not enough parameters");

	duration = at->integer;

	SV_ApplyStateToRadius((self) ? self->base.index : target->base.index, radius, statenum, duration);

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Destabilize)
{
	int		index, scanflags = 0, targetflags = 0;
	float	radius;
	vec3_t	origin, pos, dif;
	gameScriptParam_t	*at = params;

	if (!target)
		GS_ERROR("Invalid target");

	if (!at)
		GS_ERROR("Not enough parameters");
	radius = at->value;

	//check for target flags
	at = at->next;
	index = 0;
	while (at)
	{
		for (index = 0; index < 32; index++)
		{
			if (!TargetFlagNames[index][0])
				break;

			if (!stricmp(at->string, TargetFlagNames[index]))
			{
				scanflags |= (1 << index);
				break;
			}
		}
		at = at->next;
	}
	if (scanflags == 0)
		GS_ERROR("No valid targets specified");

	SV_GetPosition(target->base.index, origin);

	for (index = 0; index <= sl.lastActiveObject; index++)
	{
		if (index == target->base.index)
			continue;

		if (!sl.objects[index].base.active)
			continue;

		if (!GetObjectByType(sl.objects[index].base.type)->isVolatile)
			continue;

		if (!sl.objects[index].kill)
			continue;

		//check target flags
		if (sl.objects[index].base.type == type && !(scanflags & TARGET_SAME_TYPE))
			continue;

		if (IsUnitType(sl.objects[index].base.type))
			targetflags |= TARGET_UNIT;
		else if (IsBuildingType(sl.objects[index].base.type))
			targetflags |= TARGET_BUILDING;
		else if (IsItemType(sl.objects[index].base.type))
			targetflags |= TARGET_ITEM;

		if (!SV_GetTeam(index))
			targetflags |= TARGET_NEUTRAL;
		else
			targetflags |= (SV_GetTeam(index) == SV_GetTeam(target->base.index)) ? TARGET_ALLY : TARGET_ENEMY;

		//check for a target match
		if (!(scanflags & targetflags & TARGET_TEAMS) ||
			!(scanflags & targetflags & TARGET_TYPES))
			continue;

		SV_GetPosition(index, pos);
		M_SubVec3(origin, pos, dif);
		if (M_GetVec3Length(dif) > radius)
			continue;

		sl.objects[index].kill(&sl.objects[index], target, origin, type, 0);
	}

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Die)
{
	if (!target)
		GS_ERROR("Invalid target");

	if (target->kill)
		target->kill(target, self, target->base.pos, (self) ? self->base.type : 0, 0);

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(DamageRadius)
{
	float	radius;
	int		damage;
	vec3_t	origin;
	gameScriptParam_t	*at = params;

	if (!target)
		GS_ERROR("Inavlid target");

	if (!at)
		GS_ERROR("Not enough parameters");
	
	radius = at->value;
	at = at->next;
	if (!at)
		GS_ERROR("Not enough parameters");

	damage = at->integer;
	SV_GetPosition(target->base.index, origin);
	SV_DamageRadius(target, origin, type, radius, damage, 0);

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Damage)
{
	int		damage;
	vec3_t	origin;
	gameScriptParam_t	*at = params;

	if (!target)
		GS_ERROR("Inavlid target");

	if (!at)
		GS_ERROR("Not enough parameters");
	damage = at->integer;

	SV_GetPosition(target->base.index, origin);
	SV_DamageTarget(self, target, origin, type, damage, 0);

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(SetState)
{
	int	state, nextstate, duration;
	gameScriptParam_t	*at = params;

	if (!target)
		GS_ERROR("Inavlid target");

	if (!at)
		GS_ERROR("Not enough parameters");

	if (!IsItemType(type))
		GS_ERROR("Only items can use setstate");

	if (!stricmp(at->string, "sleep"))
		state = AS_ITEM_SLEEP;
	else if (!stricmp(at->string, "idle"))
		state = AS_IDLE;
	else if (!stricmp(at->string, "activate"))
		state = AS_ITEM_ACTIVE;
	else
		GS_ERROR("Invalid state");

	at = at->next;
	if (at)
	{
		duration = at->integer;
		at = at->next;
		if (!at)
			GS_ERROR("duration with no next state");

		if (!stricmp(at->string, "sleep"))
			nextstate = AS_ITEM_SLEEP;
		else if (!stricmp(at->string, "idle"))
			nextstate = AS_IDLE;
		else if (!stricmp(at->string, "activate"))
			nextstate = AS_ITEM_ACTIVE;
		else
			GS_ERROR("Invalid next state");
	}
	else
	{
		duration = 0;
		nextstate = state;
	}

	switch (state)
	{
	case AS_ITEM_SLEEP:
		SV_ItemSleep(target, duration, nextstate);
		break;

	case AS_IDLE:
		SV_ItemIdle(target, duration, nextstate);
		break;

	case AS_ITEM_ACTIVE:
		SV_ItemActivate(target, duration, nextstate);
		break;
	}
	
	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Delay)
{
	if (!target)
		GS_ERROR("Invalid target");

	if (!params)
		GS_ERROR("Not enough parameters");

	target->nextDecisionTime = sl.gametime + params->integer;

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Scan)
{
	gameScriptParam_t	*at = params;
	float	radius;
	int		state, index, scanflags = 0;
	vec3_t	origin;

	if (!target)
		GS_ERROR("Invalid target");

	//get radius
	if (!at)
		GS_ERROR("Not enough parameters");

	radius = at->value;

	//determine state to transition to
	at = at->next;
	if (!at)
		GS_ERROR("Not Enough parameters");

	if (!stricmp(at->string, "sleep"))
		state = AS_ITEM_SLEEP;
	else if (!stricmp(at->string, "idle"))
		state = AS_IDLE;
	else if (!stricmp(at->string, "activate"))
		state = AS_ITEM_ACTIVE;
	else
		GS_ERROR("Invalid trigger state");
	
	//check for target flags
	at = at->next;
	index = 0;
	while (at)
	{
		for (index = 0; index < 32; index++)
		{
			if (!TargetFlagNames[index][0])
				break;
			if (!stricmp(at->string, TargetFlagNames[index]))
				scanflags |= (1 << index);
		}
		at = at->next;
	}
	if (scanflags == 0)
		GS_ERROR("No valid targets specified");

	SV_GetPosition(target->base.index, origin);
	for (index = 0; index <= sl.lastActiveObject; index++)
	{
		vec3_t pos, dif;
		float dist;
		serverObject_t	*obj = &sl.objects[index];
		objectData_t	*objdata = GetObjectByType(obj->base.type);
		int targetFlags = 0;

		if (!sl.objects[index].base.active)
			continue;

		//skip self
		if (index == target->base.index)
			continue;

		//check target flags
		if (obj->base.type == type && !(scanflags & TARGET_SAME_TYPE))
			continue;

		if (IsUnitType(obj->base.type))
			targetFlags |= TARGET_UNIT;
		else if (IsBuildingType(obj->base.type))
			targetFlags |= TARGET_BUILDING;

		if (!SV_GetTeam(index))
			targetFlags |= TARGET_NEUTRAL;
		else
			targetFlags |= (SV_GetTeam(index) == SV_GetTeam(target->base.index)) ? TARGET_ALLY : TARGET_ENEMY;

		//check for a target match
		if (!(scanflags & targetFlags & TARGET_TEAMS) ||
			!(scanflags & targetFlags & TARGET_TYPES))
			continue;

		//make sure it's not dead
		if (sl.objects[index].base.health <= 0 && objdata->fullHealth > 0)
			continue;

		SV_GetPosition(index, pos);
		M_SubVec3(origin, pos, dif);

		dist = M_Normalize(dif);

		if (dist <= radius)
		{
			//target->enemy = obj;
			switch (state)
			{
			case AS_ITEM_SLEEP:
				SV_ItemSleep(target, 0, state);
				break;

			case AS_IDLE:
				SV_ItemIdle(target, 0, state);
				break;

			case AS_ITEM_ACTIVE:
				SV_ItemActivate(target, 0, state);
				break;
			}
			break;
		}
	}

	return GS_RESULT_PASS;
}

//=============================================================================

extern cvar_t sv_ressurectTime;

GS_FUNC(Revive)
{
	float	percent;
	gameScriptParam_t	*at = params;

	if (!target)
		GS_ERROR("Inavlid target");

	if (!at)
		GS_ERROR("Not enough parameters");

	percent = at->value;

	if (SV_ReviveCorpse(target->base.index, percent))
	{
		serverObject_t *owner = self->owner ? self->owner : self;		
		if (owner->client)
		{
			owner->client->stats.revives++;

			SV_AddExperience(owner->client, EXPERIENCE_PLAYER_REVIVE, 0, 1.0);

			//give a monetary reward for reviving, not taxed
			SV_GiveMoney(owner->client->index, sv_reviveMoneyReward.integer, false);

			owner->client->ps.invincibleTime = sl.gametime + 5000;  //hack 
		}
	}

	return GS_RESULT_PASS;
}

//=============================================================================

GS_FUNC(Test)
{
	gameScriptParam_t	*at = params;
	int	index = 0, test = -1;

	if (!target)
		GS_ERROR("Invalid target");

	if (!at)
		GS_ERROR("Not enough parameters");

	while (gsTestCaseNames[index][0])
	{
		if (!stricmp(at->string, gsTestCaseNames[index]))
		{
			test = index;
			break;
		}
		index++;
	}

	if (test < 0)
		GS_ERROR("Invalid test case");

	switch (test)
	{
	case GS_TESTCASE_ALLY:
		if (SV_GetTeam(self->base.index) == SV_GetTeam(target->base.index))
			return GS_RESULT_PASS;
		break;

	case GS_TESTCASE_ENEMY:
		if (SV_GetTeam(self->base.index) != SV_GetTeam(target->base.index))
			return GS_RESULT_PASS;
		break;

	case GS_TESTCASE_NEUTRAL:
		if (SV_GetTeam(target->base.index) == 0)
			return GS_RESULT_PASS;
		break;

	case GS_TESTCASE_CHARACTER:
		if (IsCharacterType(target->base.type) && !IsWorkerType(target->base.type))
			return GS_RESULT_PASS;
		break;

	case GS_TESTCASE_SIEGE:
		if (GetObjectByType(target->base.type)->isSiegeWeapon)
			return GS_RESULT_PASS;
		break;

	case GS_TESTCASE_NPC:
		if (IsWorkerType(target->base.type))
			return GS_RESULT_PASS;
		break;
	}

	return GS_RESULT_BREAK;
}

//=============================================================================

GS_FUNC(TestNot)
{
	switch (GS_Test(self, type, target, params))
	{
	case GS_RESULT_PASS:
		return GS_RESULT_BREAK;

	case GS_RESULT_BREAK:
		return GS_RESULT_PASS;

	case GS_RESULT_ERROR:
	default:
		return GS_RESULT_ERROR;
	}		
}

//=============================================================================

GS_FUNC(CheckResult)
{
	return GS_RESULT_CHECK;
}

//=============================================================================

GS_FUNC(Give)
{
	int					objtype = 0;
	gameScriptParam_t	*at = params;

	if (!target)
		GS_ERROR("Inavlid target");

	if (!at)
		GS_ERROR("Not enough parameters");

	if (!target->client)
		GS_ERROR("Target must be a client");

	if (!SetInt(&objtype, 0, MAX_OBJECT_TYPES, objectNames, at->string))
		GS_ERROR("Invalid object type");

	target->client->ps.inventory[target->client->ps.item] = objtype;
	target->client->ps.ammo[target->client->ps.item] = 1;

	return GS_RESULT_PASS;
}

//=============================================================================

gameScriptDefinition_t gameScriptDictionary[] =
{
													// params //
	{ "heal",				GS_Heal },				// <amount>
	{ "givemana",			GS_GiveMana },			// <amount>
	{ "givestamina",		GS_GiveStamina },		// <amount>
	{ "giveammo",			GS_GiveAmmo },			// <multiplier> <amount|"full"|"start"|"group">
	{ "teleport",			GS_Teleport },			// <"home"|"here"|"link">|(<x> <y> [z])
	{ "givestate",			GS_GiveState },			// <state name> <duration>
	{ "toss",				GS_Toss },				// <velocity> <gravity>
	{ "attach",				GS_Attach },			// 
	{ "givestateradius",	GS_GiveStateRadius },	// <radius> <state name> <duration>
	{ "destabilize",		GS_Destabilize },		// <radius> <targetflag> [...]
	{ "die",				GS_Die },				// 
	{ "damage",				GS_Damage },			// <amount>
	{ "damageradius",		GS_DamageRadius },		// <radius> <amount>
	{ "setstate",			GS_SetState },			// <state>  [<duration> <next state>]
	{ "delay",				GS_Delay },				// <duration>
	{ "scan",				GS_Scan },				// <radius> <trigger state> <targetflag> [...]
	{ "revive",				GS_Revive },			// <health percent>
	{ "test",				GS_Test },				// <test case> [param] [...]
	{ "testnot",			GS_TestNot },			// <test case> [param] [...]
	{ "checkresult",		GS_CheckResult },		// 
	{ "give",				GS_Give },				// <object type>

	{ "", NULL }
};
//=============================================================================
//=============================================================================

#define GS_BUFFER_LEN		65536
#define GS_MAX_LINE_LEN		256
#define GS_MAX_TOKEN_LEN	32


/*==========================

  GS_GetToken

 ==========================*/

int	GS_GetToken(char *dest, const char *source, int maxlen)
{
	int n = 0;

	//grab the token
	memset(dest, 0, maxlen);
	while (source[n] != ' ' &&
			source[n] != '\n' &&
			source[n] != '\r' &&
			source[n] != '\t' &&
			source[n] != 0 &&
			n < maxlen)
	{
		dest[n] = source[n];
		n++;
	}

	//eat the trailing white space
	while (n < maxlen &&
			source[n] != 0 &&
			(source[n] == ' ' ||
			source[n] == '\n' ||
			source[n] == '\r' ||
			source[n] == '\t'))
			n++;

	return n;
}


/*==========================

  GS_GetLine

 ==========================*/

bool	GS_GetLine(char *line, int maxlen, const char *buffer, int *pos, int end)
{
	int n = 0;
	memset(line, 0, maxlen);

	if ((*pos) >= end || buffer[*pos] == 0)
		return false;

	//copy characters
	while (buffer[*pos] != '\n' &&
			buffer[*pos] != '\r' &&
			(*pos) < end &&
			n < maxlen)
	{
		line[n] = buffer[*pos];
		(*pos)++;
		n++;
	}

	//move pos past any newline chars
	while (buffer[*pos] && 
		(buffer[*pos] == '\n' || buffer[*pos] == '\r') &&
		*pos < end)
		(*pos)++;

	return true;
}


/*==========================

  SV_GameScriptLoad

 ==========================*/

bool	SV_GameScriptLoad(gameScript_t *script[], const char *filename)
{
	file_t	*file;
	char	buffer[GS_BUFFER_LEN];
	char	line[GS_MAX_LINE_LEN];
	char	token[GS_MAX_TOKEN_LEN];
	int		index, n, offset, last;
	bool	found;
	gameScript_t		**newinst;
	gameScriptParam_t	**newparam;

	//open the file
	file = core.File_Open(filename, "rb");
	if (!file)
		return false;

	index = 0;
	memset(buffer, 0, GS_BUFFER_LEN);
	core.File_Read(buffer, GS_BUFFER_LEN, sizeof(char), file);

	newinst = NULL;
	while(GS_GetLine(line, GS_MAX_LINE_LEN, buffer, &index, GS_BUFFER_LEN))
	{
		//check first character
		switch(line[0])
		{
		case '!':		//adding a command
			offset = 1;

			if (!newinst)
			{
				cores.Console_Printf("Warning: Instruction prior to setting an entry point in %s\n", filename);
				break;
			}

			(*newinst) = cores.Tag_MallocGameScript(sizeof(gameScript_t));
			if (!(*newinst))
			{
				cores.Console_Printf("Warning: Couldn't allocate new instruction\n");
				break;
			}
			(*newinst)->next = NULL;
			(*newinst)->param = NULL;

			//first param (instruction)
			offset += GS_GetToken(token, &line[offset], GS_MAX_TOKEN_LEN);
			found = false;
			n = 0;
			while (gameScriptDictionary[n].name[0])
			{
				if (!stricmp(token, gameScriptDictionary[n].name))
				{
					(*newinst)->functionID = n;
					found = true;
					break;
				}
				n++;
			}

			if (!found)
			{
				cores.Console_Printf("Warning: Unknown instruction '%s' loading %s\n", token, filename);
				cores.Tag_Free(*newinst);
				(*newinst) = NULL;
				break;
			}

			//second param (target)
			offset += GS_GetToken(token, &line[offset], GS_MAX_TOKEN_LEN);
			found = false;
			n = 0;
			while(gsTargetNames[n][0])
			{
				if (!stricmp(token, gsTargetNames[n]))
				{
					(*newinst)->target = n;
					found = true;
					break;
				}
				n++;
			}

			if (!found)
			{
				cores.Console_Printf("Warning: Unknown target '%s' loading %s\n", token, filename);
				cores.Tag_Free(*newinst);
				(*newinst) = NULL;
				break;
			}

			//store the rest of the params
			newparam = &((*newinst)->param);
			while (true)
			{
				last = offset;
				offset += GS_GetToken(token, &line[offset], GS_MAX_TOKEN_LEN);
				if (offset != last)
				{
					int x = 0;

					(*newparam) = cores.Tag_MallocGameScript(sizeof(gameScriptParam_t));
					(*newparam)->next = NULL;
					(*newparam)->flags = 0;
					
					while(true)
					{
						if (token[x] == '%')
							(*newparam)->flags |= GS_PARAM_FLAG_PERCENT;
						else if (token[x] == '=')
							(*newparam)->flags |= GS_PARAM_FLAG_EQUALS;
						else if (token[x] == '*')
							(*newparam)->flags |= GS_PARAM_FLAG_MULTIPLY;
						else
							break;
						x++;
					}

					(*newparam)->string = cores.Tag_MallocGameScript(sizeof(char) * strlen(&(token[x])) + 1);
					strcpy((*newparam)->string, &(token[x]));
					(*newparam)->integer = atoi(&(token[x]));
					(*newparam)->value = atof(&(token[x]));

					newparam = &((*newparam)->next);
					continue;
				}
				break;
			}

			newinst = &((*newinst)->next);
			break;

		case '@':		//changing to a new entry point
			found = false;

			GS_GetToken(token, &line[1], GS_MAX_TOKEN_LEN);

			n = 0;
			while (gsEntryNames[n][0])
			{
				if (!stricmp(token, gsEntryNames[n]))
				{
					newinst = &(script[n]);
					while (*newinst)
						newinst = &(*newinst)->next;
					found = true;
					break;
				}
				n++;
			}

			if (!found)
				cores.Console_Printf("Warning: Unknown entry point '%s' loading %s\n", token, filename);
			break;

		case '#':		//comment, ignore it
			break;

		default:		//print warning
			cores.Console_Printf("Warning: Unknown line marker '%c' loading %s\n", line[0], filename);
			break;
		}
	}

	core.File_Close(file);
	return true;
}
//=============================================================================
//=============================================================================

void	LoadGameScript_Cmd(int argc, char *argv[])
{
	int type, index;

	if (argc < 1)
		return;

	type = GetObjectTypeByName(argv[0]);
	if (!type)
	{
		cores.Console_Printf("Could not find object \"%s\"\n", argv[0]);
		return;
	}

	//free the old script data
	for (index = 0; index < NUM_GS_ENTRIES; index++)
	{
		gameScriptParam_t	*param, *nextparam;
		gameScript_t		*inst, *nextinst;

		inst = objectScripts[type][index];
		while(inst)
		{
			param = inst->param;
			while(param)
			{
				nextparam = param->next;
				cores.Tag_Free(param);
				param = nextparam;
			}

			nextinst = inst->next;
			cores.Tag_Free(inst);
			inst = nextinst;
		}
		objectScripts[type][index] = NULL;
	}

	SV_GameScriptLoad(objectScripts[type], fmt("/objects/scripts/%s.gs", argv[0]));
}


bool	SV_GameScriptExecute(serverObject_t *self, serverObject_t *other, int type, gameScriptEntryPoint_t entry)
{
	gameScript_t	*inst;
	serverObject_t	*target = NULL;
	int	count = 0, result = true;

	if (entry < 0 || entry >= NUM_GS_ENTRIES)
		return false;
	//if (!self || !other)
	//	return false;

	inst = objectScripts[type][entry];

	while (inst)
	{
		count++;
		switch (inst->target)
		{
		case GS_TARGET_TARGET:
			target = other;
			break;

		case GS_TARGET_SELF:
			target = self;
			break;
		
		case GS_TARGET_ENEMY:
			if (self)
				target = self->enemy;
			break;
		
		case GS_TARGET_OWNER:
			if (self)
				target = self->owner;
			break;

		case GS_TARGET_LINK:
			if (other)
				target = other->link;
			break;
		}

		switch (gameScriptDictionary[inst->functionID].function(self, type, target, inst->param))
		{
		case GS_RESULT_PASS:
			result = true;
			break;

		case GS_RESULT_FAIL:
			result = false;
			break;

		case GS_RESULT_BREAK:
			return false;

		case GS_RESULT_ERROR:
			cores.Console_DPrintf("Game script error for %s #%i: %s\n", objectNames[type], count, gsErrorBuffer);
			break;
			
		case GS_RESULT_CHECK:
			if (!result)
				return false;
			break;
		}
		inst = inst->next;
	}

	return result;
}


/*==========================

  SV_InitGameScript

 ==========================*/

void	SV_InitGameScript()
{
	int index;

	cores.Cvar_Register(&sv_reviveMoneyReward);

	cores.Cmd_Register("loadgamescript", LoadGameScript_Cmd);

	cores.Console_Printf("Loading game scripts...\n");
	cores.Tag_FreeGameScript();
	memset(objectScripts, 0, sizeof(gameScript_t*) * MAX_OBJECT_TYPES * NUM_GS_ENTRIES);
	for (index = 0; index < MAX_OBJECT_TYPES; index++)
	{
		if (objData[index].type)
			SV_GameScriptLoad(objectScripts[index], fmt("/objects/scripts/%s.gs", objData[index].name));
	}
}
