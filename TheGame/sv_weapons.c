// (C) 2002 S2 Games

// sv_weapons.c
//=============================================================================

#include "server_game.h"
#include <float.h>

#define	MAX_OBJECT_HITS	16

cvar_t	sv_projRestVelocity =	{ "sv_projRestVelocity",	"35.0",	CVAR_READONLY };
cvar_t	sv_projRestSlope =		{ "sv_projRestSlope",		"0.55",	CVAR_READONLY };

extern cvar_t sv_hitStructureRewardScale;
extern cvar_t sv_teamDamage;
//=============================================================================


/*==========================

  SV_GetMeleeDamage

  Returns adjusted damage for a character, based on character level, upgrades
  powerups, and melee 'charge'

 ==========================*/

int		SV_GetMeleeDamage(serverObject_t *obj, int attacktype, float charge, objectData_t *unit, objectData_t *weapon)
{
	int basedamage = 0, damage, n;
	int add = 0;
	float mul = 1.0;

	basedamage = weapon->damage;

	damage = ((attacktype == AS_MELEE_RELEASE) ? charge : 1.0) * basedamage;

	//adjust for upgrades
	//fixme: have upgrades adjust the stats structure
	for (n = 0; n < MAX_STATE_SLOTS; n++)
	{
		int	statenum = obj->base.states[n];

		if (!statenum)
			continue;

		add += stateData[statenum].damageAdd;
		mul += MAX(0.0, stateData[statenum].damageMult - 1.0);
	}

	add += obj->adjustedStats.meleeDamageBonus;

	damage *= mul;
	damage += add;

	return damage;
}


/*==========================

  SV_ClientMeleeAttack

  Handles a melee attack, including building interactions

 ==========================*/

void	SV_ClientMeleeAttack(serverObject_t *attacker, int attacktype)
{
	vec3_t			forward, right, end;
	inputState_t	*in;
	client_t		*client = &sl.clients[attacker->base.index];
	objectData_t	*unit = &objData[client->ps.unittype], *melee = NULL;
	int				damage;
	traceinfo_t trace;
	vec3_t start;
	int	unlinked[MAX_OBJECTS];
	int	numhits = 0;
	float vscale;
	vec3_t mins, maxs;
	int		ignoreStructs = 0;
	serverObject_t *target;
	bool	alreadyHit;

	//safety checks
	if (!attacker->client)
		return;

	melee = GetObjectByType(client->ps.inventory[client->ps.item]);
	if (!melee)
		return;

	//get adjusted damage
	damage = SV_GetMeleeDamage(attacker, attacktype, client->ps.charge, unit, melee);

	in = &client->input;

	M_CopyVec3(client->ps.pos, start);

	//determine how to scale the trace box:
	//we will have the trace use the players bounds, stretched
	//up or down based on their angle, but never shifted
	//this way you can always hit something right in front of you,
	//even if it's at your feet and you are looking up
	HeadingFromAngles(0, client->ps.angle[YAW], forward, right);
	vscale = (client->ps.angle[PITCH] - 90.0) / 90.0;

	//set the end point of the trace
	M_PointOnLine(start, forward, unit->attacks[attacktype].range + attacker->adjustedStats.meleeRangeBonus, end);

	//and the bounds for the trace
	M_CopyVec3(attacker->base.bmin, mins);
	M_CopyVec3(attacker->base.bmax, maxs);
	if (vscale < 0)
		mins[2] += maxs[2] * vscale;
	else
		maxs[2] += maxs[2] * vscale;

	//hit everything in range (up to 10 objects, just to be safe)
	while (numhits < 10)
	{
		alreadyHit = false;

		cores.World_TraceBoxEx(&trace, start, end, mins, maxs, TRACE_MELEE | ignoreStructs, attacker->base.index);

		//-1 means we hit nothing, MAX_OBJECTS or greater means we hit a static object,
		//which shouldn't be possible, so just bail if for some reason it does happen
		if (trace.index < 0 || trace.index >= MAX_OBJECTS)
			break;
		target = &sl.objects[trace.index];

		//has this been hit before?
		alreadyHit = attacker->hitObjects[trace.index];

		//record the hit, and ensure that this object will not be hit in a subsequent trace
		attacker->hitObjects[trace.index] = true;
		if (target->base.flags & BASEOBJ_WORLDOBJ_REPRESENTS)
		{
			unlinked[numhits] = -1;
			numhits++;
			ignoreStructs = SURF_COMPLEX;
		}
		else
		{
			unlinked[numhits] = trace.index;
			numhits++;
			cores.World_UnlinkObject(&sl.objects[trace.index].base);
		}

		//this has to happen after the hit has been recorded, otherwise the trace would
		//keep bumping something we hit on a previous frame
		if (alreadyHit)
			continue;

		//handle structure interactions
		if (target->base.flags & BASEOBJ_WORLDOBJ_REPRESENTS)
		{
			objectData_t *building = GetObjectByType(target->base.type);

			if (building->isClaimable && target->base.team != attacker->base.team)
				SV_ClaimBuilding(attacker, target);

			if (building->isMine && melee->mineRate)
			{
				SV_MineResourcesFromBuilding(attacker, target, unit->mineRate);
			}
			else if (target->base.team == attacker->base.team)
			{
				if (!SV_ConstructBuilding(attacker, target, attacker->adjustedStats.buildRate))
					SV_RepairBuilding(attacker, target, attacker->adjustedStats.repairRate);
			}
			else
			{
				SV_DamageTarget(attacker, target, trace.endpos, 0, damage, melee->damageFlags);
				if (building->thorndamage)
					SV_DamageTarget(target, attacker, trace.endpos, 0, building->thorndamage, 0);
			}
			continue;
		}

		//deal out damage
		SV_DamageTarget(attacker, target, trace.endpos, client->ps.inventory[client->ps.item], damage, melee->damageFlags);
		SV_GameScriptExecute(attacker, target, client->ps.inventory[client->ps.item], GS_ENTRY_IMPACT);

		if (GetObjectByType(target->base.type)->thorndamage)
			SV_DamageTarget(target, attacker, trace.endpos, 0, GetObjectByType(target->base.type)->thorndamage, 0);
	}

	//re-link what we unlinked
	while (numhits)
	{
		numhits--;
		if (unlinked[numhits] >= 0)
			cores.World_LinkObject(&sl.objects[unlinked[numhits]].base);
	}
}


/*==========================

  SV_Projectile_Impact

  Determine what the projectile hit, and interact with it based on the proejctile's settings
  returns true if trajectory requires further evaluation

 ==========================*/

int	SV_Projectile_Impact(serverObject_t *proj, int other, traceinfo_t *trace, int *impactTimeOffset)
{
	serverObject_t	*target = NULL;
	objectData_t	*projectile = GetObjectByType(proj->base.type);
	int		reaction = -1;
	int		damage;

	damage = projectile->damage;
	CHARGEMOD(projectile, MOD_DAMAGE, damage, proj->charge);

	//not the world
	if (other >= 0 && other < MAX_OBJECTS)
	{
		//check if it's a building or a unit
		objectData_t	*targetinfo = GetObjectByType(sl.objects[other].base.type);

		if (targetinfo->objclass == OBJCLASS_UNIT || targetinfo->objclass == OBJCLASS_ITEM)
			reaction = projectile->hitUnit;
		if (targetinfo->objclass == OBJCLASS_BUILDING)
			reaction = projectile->hitBuilding;

		target = &sl.objects[other];
		
		//treat same team objects as world objects
		if (target->base.team == proj->owner->base.team/* && !sv_teamDamage.integer*/)
			reaction = projectile->hitWorld;
	}
	//hit the world
	else
	{
		reaction = projectile->hitWorld;
	}

	if (reaction < 0)
	{
		cores.Console_DPrintf("Hit an unknown object type\n");
		return reaction;
	}

	switch (reaction)
	{
	default:
		cores.Console_DPrintf("Unknown projectile reaction\n");
		break;

	case PROJ_STOP:
		//release any snap to muzzle effects
		proj->base.flags &= ~BASEOBJ_SNAP_TO_MUZZLE;
		proj->base.flags &= ~BASEOBJ_USE_TRAJECTORY;
		M_CopyVec3(trace->normal, proj->forward);
		break;

	//no interaction
	case PROJ_IGNORE:
		break;

	//damage target, expire
	case PROJ_DIE:
	case PROJ_DIE_NO_EFFECT:
		//release any snap to muzzle effects if this isn't a trace weapon
		if (projectile->fuseTime.min > 0)
			proj->base.flags &= ~BASEOBJ_SNAP_TO_MUZZLE;

		//apply direct damage to the target, if there is no radius damage
		if	(damage && target && !projectile->radius.max)
			SV_DamageTarget(proj->owner, target, proj->base.pos, proj->base.type, damage, projectile->damageFlags);
		SV_GameScriptExecute(proj, target, proj->base.type, GS_ENTRY_IMPACT);
		M_CopyVec3(trace->normal, proj->forward);
		break;

	//damage target, continue on trajectory
	case PROJ_PIERCE:
		SV_DamageTarget(proj->owner, target, proj->base.pos, proj->base.type, damage, projectile->damageFlags);
		SV_GameScriptExecute(proj, target, proj->base.type, GS_ENTRY_IMPACT);
		M_CopyVec3(trace->normal, proj->forward);
		break;

	//change trajectory, send bounce event
	case PROJ_BOUNCE_DAMAGE:
		//release any snap to muzzle effects
		proj->base.flags &= ~BASEOBJ_SNAP_TO_MUZZLE;

		if	(damage && target && !projectile->radius.max)
			SV_DamageTarget(proj->owner, target, proj->base.pos, proj->base.type, damage, projectile->damageFlags);
		SV_GameScriptExecute(proj, target, proj->base.type, GS_ENTRY_IMPACT);

	case PROJ_BOUNCE:
		//release any snap to muzzle effects
		proj->base.flags &= ~BASEOBJ_SNAP_TO_MUZZLE;

		if (projectile->bounce > 0)
		{
			int		impactTime = sl.gametime + *impactTimeOffset + ((sl.frame_msec - *impactTimeOffset) * trace->fraction);
			float	dot;
			vec3_t	vel;	//the heading of the projectile when it hit

			//Bounce
			////////////////
			//rebuild the trajectory structure
			*impactTimeOffset = impactTime - sl.gametime;

			Traj_GetVelocity(&proj->base.traj, impactTime, vel);
			
			M_CopyVec3(trace->endpos, proj->base.traj.origin);
			M_AddVec3(trace->normal, proj->base.traj.origin, proj->base.traj.origin);
			M_CopyVec3(proj->base.traj.origin, proj->base.pos);
			
			proj->base.traj.startTime = impactTime;
			
			dot = M_DotProduct(vel, trace->normal);
			M_MultVec3(trace->normal, -2 * dot, proj->base.traj.velocity);
			M_AddVec3(proj->base.traj.velocity, vel, proj->base.traj.velocity);
			M_MultVec3(proj->base.traj.velocity, projectile->bounce, proj->base.traj.velocity);



			//let projectiles come to rest
			if (trace->normal[2] > sv_projRestSlope.value && M_GetVec3Length(proj->base.traj.velocity) < sv_projRestVelocity.value)
			{
				proj->base.flags &= ~BASEOBJ_USE_TRAJECTORY;
				M_CopyVec3(trace->normal, proj->forward);
				return PROJ_STOP;
			}
		}
		SV_Phys_AddEvent(proj, EVENT_BOUNCE, 0, 0);
		break;
	}

	return reaction;
}


/*==========================

  SV_Projectile_Frame

  Advances a projectile based on it's trajectory_t setting, handles interactions

  ==========================*/

void	SV_Projectile_Frame(serverObject_t *proj)
{
	vec3_t			targetPos, startPos;
	objectData_t	*projectile = GetObjectByType(proj->base.type);
	traceinfo_t		trace;
	int				result = -1;
	int				n, numhits = 0, ignore;
	serverObject_t	*hits[MAX_OBJECT_HITS];
	int				timeshift = 0;
	int				damage;
	int				iterations;
	float			lastfraction;
	int				impactTimeOffset = 0;

	damage = projectile->damage;
	CHARGEMOD(projectile, MOD_DAMAGE, damage, proj->charge);

	//special case for trace weapons
	if (projectile->fuseTime.max == 0)
		timeshift = 1000;

	if (proj->base.flags & BASEOBJ_USE_TRAJECTORY)
	{
		//first thing to unlink is the projectiles owner
		ignore = proj->owner->base.index;
		hits[numhits] = proj->owner;
		cores.World_UnlinkObject(&hits[numhits]->base);
		numhits++;

		//move the projectile
		iterations = 0;
		lastfraction = 0.0;
		do
		{
			//evalutate the trajectory
			M_CopyVec3(proj->base.pos, startPos);
			if (projectile->continuous)
				M_CopyVec3(proj->base.pos, proj->base.traj.origin);
			Traj_GetPos(&proj->base.traj, sl.gametime + sl.frame_msec + timeshift, targetPos);
			if (projectile->continuous)
				M_ClearVec3(proj->base.traj.origin);
			if ( projectile->projectileRadius > 0.0f )
			{
				vec3_t bmin, bmax;
				M_SetVec3(bmin, -projectile->projectileRadius, -projectile->projectileRadius, -projectile->projectileRadius);
				M_SetVec3(bmax, +projectile->projectileRadius, +projectile->projectileRadius, +projectile->projectileRadius);

				cores.World_TraceBoxEx(&trace, startPos, targetPos, bmin, bmax, TRACE_PROJECTILE, ignore);
			}
			else
			{
				cores.World_TraceBoxEx(&trace, startPos, targetPos, zero_vec, zero_vec, TRACE_PROJECTILE, ignore);
			}
			M_SubVec3(targetPos, startPos, proj->forward);
			M_Normalize(proj->forward);
			M_CopyVec3(trace.endpos, proj->base.pos);
			
			//just relocate if there was no impact
			if (trace.fraction == 1.0)
				break;

			if (trace.index >= 0)
				ignore = trace.index;

			//unlink things that we hit (that aren't buildings) so projectile can continue (if it needs to)
			if ((trace.index < MAX_OBJECTS && trace.index >= 0) && !(sl.objects[trace.index].base.flags & BASEOBJ_WORLDOBJ_REPRESENTS) && numhits < MAX_OBJECT_HITS)
			{
				hits[numhits] = &sl.objects[trace.index];
				cores.World_UnlinkObject(&hits[numhits]->base);
				numhits++;
			}

			//handle impacts
			result = SV_Projectile_Impact(proj, trace.index, &trace, &impactTimeOffset);
			
			//projectile is wedged in a corner, stop it
			if (trace.fraction <= 0.0001 && lastfraction <= 0.0001)
			{
				proj->base.flags &= ~BASEOBJ_USE_TRAJECTORY;
				M_CopyVec3(trace.normal, proj->forward);
				result = PROJ_STOP;
			}

			lastfraction = trace.fraction;

			iterations++;
		}
		while (result > PROJ_STOP && iterations <= 10);

		//relink anything that was hit
		for (n = 0; n < numhits; n++)
			cores.World_LinkObject(&hits[n]->base);
	}

	//time to explode?
	if (sl.gametime >= proj->expireTime || result == PROJ_DIE || result == PROJ_DIE_NO_EFFECT)
	{
		//release any snap to muzzle effects
		if (projectile->fuseTime.min > 0)
			proj->base.flags &= ~BASEOBJ_SNAP_TO_MUZZLE;

		if (projectile->radius.max)
			SV_DamageRadius(proj, proj->base.pos, proj->base.type, GETRANGE(projectile->radius), damage, projectile->damageFlags);

		SV_GameScriptExecute(proj, proj, proj->base.type, GS_ENTRY_FUSE);

		if (result != PROJ_DIE_NO_EFFECT)
			SV_DeathEvent(proj, EVENT_DEATH, 0, 0, 1000, true);
		else
			SV_DeathEvent(proj, EVENT_DEATH_QUIET, 0, 0, 1000, true);
	}
}


/*==========================

  SV_GetSiegeFiringDirection

  This function modifies the forward vector to point from the camera origin
  through the cursor's location on screen when the player is spawned as a
  vehicle type unit

 ==========================*/

void	SV_GetSiegeFiringDirection(serverObject_t *owner, const vec3_t dir, vec3_t out)
{
	objectData_t	*objdata = GetObjectByType(owner->base.type);
	vec3_t		origin, camerapos, cursorpos, cameradir, up, right;
	float		offset, fovy;
	traceinfo_t	trace;

	if (owner->base.index >= MAX_CLIENTS)
		return;

//	cores.Console_Printf("%i %i\n", owner->client->aimX, owner->client->aimY);

	M_CopyVec3(owner->base.pos, origin);

	//get world position of camera
	//horizontal component
	offset = -(cos(DEG2RAD(owner->client->ps.angle[PITCH])) * (float)objdata->distOffset);
	M_MultVec3(owner->client->forward, offset, camerapos);
	M_AddVec3(origin, camerapos, camerapos);
	//vertical component
	offset = -sin(DEG2RAD(owner->client->ps.angle[PITCH])) * (float)objdata->distOffset;
	camerapos[2] += objdata->viewheight + offset;
	
	//get world position of the center of the screen
	M_CopyVec3(origin, cursorpos);
	cursorpos[2] += objdata->viewheight;

	//get direction camera is pointing
	M_SubVec3(cursorpos, camerapos, cameradir);
	M_Normalize(cameradir);
	
	//do this before right gets messed with
	M_SetVec3(up, 0, 0, 1);
	M_CrossProduct(dir, up, right);
	M_CrossProduct(right, cameradir, up);

	//vector offset for cursors X coordinate
	//convert cursor offset from pixels to world units
	offset = ((owner->client->aimX - (0x7fff>>1)) / (float)(0x7fff>>1)) * objdata->distOffset;
	M_MultVec3(right, offset, right);
	
	//vector offset for cursors Y coordinate
	M_Normalize(up);
	//convert cursor offset from pixels to world units
	fovy = atan(owner->client->aspect);
	offset = ((owner->client->aimY - (0x7fff>>1)) / -(float)(0x7fff>>1)) * (objdata->distOffset * tan(fovy));
	M_MultVec3(up, offset, up);

	//add cusor offsets 
	M_AddVec3(cursorpos, right, cursorpos);
	M_AddVec3(cursorpos, up, cursorpos);

	//trace from camera through cursors real world coordinates to get target
	M_SubVec3(cursorpos, camerapos, cameradir);
	M_Normalize(cameradir);
	M_MultVec3(cameradir, 10000, cameradir);
	M_AddVec3(cameradir, camerapos, cursorpos);
	cores.World_TraceBox(&trace, camerapos, cursorpos, zero_vec, zero_vec, TRACE_PROJECTILE);

	//get vector from origin to trace hit
	M_CopyVec3(origin, cursorpos);
	cursorpos[2] += objdata->viewheight;
	if (!out)
		return;
	M_SubVec3(trace.endpos, cursorpos, out);
	M_Normalize(out);
}


/*==========================

  SV_WeaponFire

  Uses a wepaon object defintion to spawn a projectile or perform a trace

 ==========================*/

#define USE_SV_MUZZLE_OFFSET

void	SV_WeaponFire(serverObject_t *owner, vec3_t dir, int weapontype, vec3_t target)
{
	objectData_t	*weapon = GetObjectByType(weapontype);
	int		count, n;
	float	charge, realcharge;
	float	heat;
	bool	spawn = false;

	//check for a techlockdown
	if (weapon->techType)
	{
		for (n = 0; n < MAX_STATE_SLOTS; n++)
		{
			int	statenum = owner->base.states[n];

			if (!statenum)
				continue;

			if (stateData[statenum].lockdownTech)
			{
				SV_Phys_AddEvent(owner, EVENT_FIZZLE, 0, 0);
				return;
			}
		}
	}

	//check that owner is a player
	if (!owner->client)
	{
		charge = 1.0;
		heat = 0.0;
		//return;
	}
	else
	{
		realcharge = sl.clients[owner->base.index].ps.charge;
		charge = MIN(1.0, realcharge);
		heat = weapon->overheatTime ? (sl.clients[owner->base.index].ps.overheatCounter / (float)weapon->overheatTime) : 0;
	}

	//check that object is a valid weapon
	if (!IsWeaponType(weapontype) && !weapon->useWeaponFire)
	{
		cores.Console_DPrintf("SV_Weapon_Fire failed: [%i]%s is not a weapon\n", weapontype, weapon->name);
		return;
	}

	//determine number of projectiles to fire
	count = GETRANGE(weapon->count);
	CHARGEMOD(weapon, MOD_COUNT, count, charge);

	//fixme: traces should spawn only one object, no matter how many are being done
	//		 they need to store the number of traces to perform somewhere, as well as
	//       send a seed for a random sequence that client/server both use
	for (n = 0; n < count; n++)
	{		
		float			velocity;
		trajectory_t	traj;
		serverObject_t	*newproj;
		int		deathTime;
		float	accuracy = weapon->accuracy;
		vec3_t	up, right, yaw, tempdir, rightadd, upadd, firedir;
#ifdef USE_SV_MUZZLE_OFFSET
		vec3_t muzzleOffset = { weapon->muzzleOffset[0], weapon->muzzleOffset[1], weapon->muzzleOffset[2] };
#else
		vec3_t muzzleOffset = { 0,0,0 };
#endif

		//don't abuse dir!
		M_CopyVec3(dir, tempdir);

		//set firing direction
		if (GetObjectByType(owner->base.type)->isVehicle)
			SV_GetSiegeFiringDirection(owner, tempdir, tempdir);
		M_Normalize(tempdir);
		
		//add variance
		M_CopyVec2(tempdir, yaw);
		yaw[2] = 0;
		M_SetVec3(up, 0, 0, 1);
		M_CrossProduct(yaw, up, right);
		M_Normalize(right);
		M_CrossProduct(tempdir, right, up);
		M_Normalize(up);
		CHARGEMOD(weapon, MOD_ACCURACY, accuracy, charge);
		HEATMOD(weapon, MOD_ACCURACY, accuracy, heat);
		if (owner->base.index < MAX_CLIENTS)
			accuracy *= owner->client->ps.focus;
		//cores.Console_Printf("Accuracy: %f, focus: %f\n", accuracy, owner->client->ps.focus);
		M_MultVec3(right, (1.0 - accuracy) * M_Randnum(-0.1, 0.1), rightadd);
		M_MultVec3(up, (1.0 - accuracy) * M_Randnum(-0.1, 0.1), upadd);
		M_AddVec3(tempdir, upadd, tempdir);
		M_AddVec3(tempdir, rightadd, firedir);
		M_Normalize(firedir);

		//build a trajectory structure

		//origin
		M_CopyVec3(owner->base.pos, traj.origin);
		
		// height of the gun
		traj.origin[2] += objData[owner->base.type].viewheight + muzzleOffset[2];
		
		//sideways offset
		M_CopyVec3(right, tempdir);
		M_Normalize(tempdir);
		M_MultVec3(tempdir, muzzleOffset[0], tempdir);
		M_AddVec3(traj.origin, tempdir, traj.origin);

		//distance forward from players center
		M_CopyVec3(dir, tempdir);
		M_MultVec3(tempdir, muzzleOffset[1], tempdir);
		M_AddVec3(tempdir, traj.origin, traj.origin);
		M_Normalize(tempdir);
	
		//initial time
		traj.startTime = sl.gametime;
		
		//acceleration (linear and gravity)
		traj.acceleration = weapon->accelerate;
		traj.gravity = weapon->gravity;
		
		//velocity
		M_CopyVec3(firedir, traj.velocity);
		velocity = GETRANGE(weapon->velocity);
		CHARGEMOD(weapon, MOD_VELOCITY, velocity, charge);
		HEATMOD(weapon, MOD_VELOCITY, velocity, heat);
		if (velocity < weapon->minVelocity)
			velocity = weapon->minVelocity;
		if (velocity > weapon->maxVelocity && weapon->maxVelocity > 0)
			velocity = weapon->maxVelocity;
		M_MultVec3(traj.velocity, velocity, traj.velocity);
		traj.velocity[Z] -= GETRANGE(weapon->vertVelocity) * up[Z];
		if (weapon->inheritVelocity && owner->client)
			M_AddVec3(traj.velocity, owner->client->ps.velocity, traj.velocity);

		//over ride trajectory if a target position was specified
		if (target && !weapon->fuseTime.max)
		{
			vec3_t	vel;

			M_SubVec3(target, traj.origin, vel);
			M_CopyVec3(vel, traj.velocity);
		}

		//Spawn a new projectile
		newproj = SV_AllocObject((byte)weapontype, owner->base.team);
		newproj->nextDecisionTime = sl.gametime;

		newproj->owner = owner;

		deathTime = GETRANGE(weapon->fuseTime);
		CHARGEMOD(weapon, MOD_FUSETIME, deathTime, charge);
		if (weapon->startFuseAtCharge)
			deathTime -= realcharge * weapon->chargeTime;

		newproj->expireTime = sl.gametime + deathTime;
		
		newproj->base.flags |= BASEOBJ_USE_TRAJECTORY | BASEOBJ_HAS_OWNER;
		newproj->base.team = owner->base.team;
		newproj->base.owner = owner->base.index;		
		//newproj->base.flags |= BASEOBJ_SNAP_TO_MUZZLE;		//snap fast/trace weapons to the muzzle point when rendering
		newproj->frame = SV_Projectile_Frame;
		newproj->charge = charge;
		newproj->chargeFlags = weapon->chargeFlags;

		M_CopyVec3(traj.origin, newproj->base.pos);
		memcpy(&(newproj->base.traj), &traj, sizeof(trajectory_t));
		if (weapon->continuous)
			M_ClearVec3(newproj->base.traj.origin);

		SV_Projectile_Frame(newproj);

		newproj->base.flags |= BASEOBJ_SNAP_TO_MUZZLE;

		//continuous weapons are completely predicted on the client,
		//so we free them immediately before they have a chance
		//to get sent over the network
		if (weapon->continuous)		
			SV_FreeObject(newproj->base.index);
	}

	//lose focus now that the shot has fired
	if (owner->client)
		owner->client->ps.focus = weapon->focusPenalty;
}


/*==========================

  SV_Attack

  Main entry function for all attacks

  ==========================*/

void	SV_Attack(serverObject_t *owner, int wpState)
{
	playerState_t	*ps = &sl.clients[owner->base.index].ps;
	int	type = ps->inventory[ps->item];

	switch(wpState)
	{
	case AS_BLOCK:
		break;

	case AS_MELEE_1:
	case AS_MELEE_2:
	case AS_MELEE_3:
	case AS_MELEE_4:
	case AS_ALT_MELEE_1:
	case AS_ALT_MELEE_2:
	case AS_ALT_MELEE_3:
	case AS_ALT_MELEE_4:
	case AS_MELEE_CHARGE:
	case AS_MELEE_RELEASE:
		SV_ClientMeleeAttack(owner, wpState);
		break;

	case AS_WPSTATE_FIRE:
		if (IsWeaponType(type) || GetObjectByType(type)->useWeaponFire)
			SV_WeaponFire(owner, sl.clients[owner->base.index].forward, type, NULL);
		else if (IsItemType(type))
			SV_UseItem(owner->base.index, type);
		break;

	default:
		break;
	}
}


/*==========================

  SV_InitWeapons

  Register commands/cvars relevant to weapons

 ==========================*/

void	SV_InitWeapons()
{
	cores.Cvar_Register(&sv_projRestVelocity);
	cores.Cvar_Register(&sv_projRestSlope);
}
