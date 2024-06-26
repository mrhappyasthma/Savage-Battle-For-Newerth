// (C) 2001 S2 Games

// sv_events.c

// server side events that happen after physics / combat code

#define LOCAL_TESTING

#include "server_game.h"

cvar_t sv_damagePushScale =		{ "sv_damagePushScale",		"0.35" };

cvar_t sv_radiusTweak =			{ "sv_radiusTweak",			"5.0", CVAR_READONLY };

cvar_t sv_quakeMultiplyer =		{ "sv_quakeMultiplyer",		"3.5" };

cvar_t sv_respawnTime =			{ "sv_respawnTime",			"30000" };
cvar_t sv_useRespawnWindow =	{ "sv_useRespawnWindow",	"1" };
cvar_t sv_fraglimit =			{ "sv_fraglimit",			"20" };
cvar_t sv_teamDamage =			{ "sv_teamDamage",			"0" };

cvar_t sv_maxPush	 =			{ "sv_maxPush",		"500" };
cvar_t sv_hitStunTime =			{ "sv_hitStunTime",			"150" };

cvar_t sv_fogRevealTime =		{ "sv_fogRevealTime",		"5000" };

cvar_t sv_goodieBags =			{ "sv_goodieBags",			"1" };


/*==========================

  SV_GetArmor

  Returns the percent of damage to be negated, based on units settings and any applicable states

 ==========================*/

float	SV_GetArmor(serverObject_t	*obj)
{
	int		n;
	float	armor = 0;
	float	add = 0;
	float	mul = 1.0;

	armor = GetObjectByType(obj->base.type)->armor;

	for (n = 0; n < MAX_STATE_SLOTS; n++)
	{
		if (!obj->base.states[n])
			continue;

		add = MAX(add, stateData[obj->base.states[n]].armorAdd);
		mul *= stateData[obj->base.states[n]].armorMult;
	}

	armor *= mul;
	armor = MAX(armor, add);

	return MIN(1.0, armor);
}


/*==========================

  SV_IsCloaked

  Returns true if a unit is hidden by a special effect
  This function takes into account the targets current state, as well as
  otehr objects that can influence it's state of being hidden

 ==========================*/

bool	SV_IsCloaked(serverObject_t *looker, serverObject_t *target)
{
	int index;
	bool cloaked = false;

	//the object is a detector
	if (GetObjectByType(looker->base.type)->revealHidden)
		return false;

	//check if the unit is cloaked
	for (index = 0; index < MAX_STATE_SLOTS; index++)
	{
		if (!target->base.states[index])
			continue;

		if (stateData[target->base.states[index]].cloak)
		{
			cloaked = true;
			break;
		}
	}

	//check for a cloaked unit being revealed by a detector
	if (cloaked)
	{
		vec3_t	pos1, pos2, vec;

		SV_GetPosition(target->base.index, pos1);

		for (index = 0; index <= sl.lastActiveObject; index++)
		{
			if (!sl.objects[index].base.active)
				continue;

			if (SV_GetTeam(looker->base.index) != SV_GetTeam(index))
				continue;

			if (!GetObjectByType(sl.objects[index].base.type)->revealHidden)
				continue;

			SV_GetPosition(index, pos2);
			M_SubVec3(pos1, pos2, vec);

			if (M_GetVec3Length(vec) <= GetObjectByType(sl.objects[index].base.type)->viewDistance)
			{
				cloaked = false;
				break;
			}
		}
	}

	return cloaked;
}

byte	SV_EncodePositionOnObject(serverObject_t *obj, const vec3_t pos)
{
	byte ret;
	vec3_t dir;
	vec3_t bpos,bext;
	vec3_t bmin,bmax;

	M_AddVec3(obj->base.pos, obj->base.bmin, bmin);
	M_AddVec3(obj->base.pos, obj->base.bmax, bmax);
	M_CalcBoxExtents(bmin,bmax,bpos,bext);
	M_SubVec3(pos, bpos, dir);
	M_Normalize(dir);
	ret = M_NormalToByte(dir);

	return ret;
}

void	SV_DamagePush(client_t *target, vec3_t origin, int velocity, bool changeZ)
{
	int n;
	vec3_t vec;

	M_SubVec3(target->ps.pos, origin, vec);
	M_Normalize(vec);
	M_MultVec3(vec, velocity, vec);
	if (!changeZ)
		vec[2] = 0;
	else
	{
		vec[2] = MIN(sv_maxPush.value, vec[2]);
		if (vec[2] < -sv_maxPush.value)
			vec[2] = -sv_maxPush.value;
		else if (vec[2] > sv_maxPush.value)
			vec[2] = sv_maxPush.value;
	}

	for (n=0; n<2; n++)
	{
		vec[n] = MIN(sv_maxPush.value, vec[n]);
		if (vec[n] < -sv_maxPush.value)
			vec[n] = -sv_maxPush.value;
		else if (vec[n] > sv_maxPush.value)
			vec[n] = sv_maxPush.value;
	}

	M_AddVec3(target->ps.velocity, vec, target->ps.velocity);

	target->ps.flags |= PS_DAMAGE_PUSH;
}


/*==========================

  SV_CanDamageTarget

 ==========================*/

bool	SV_CanDamageTarget(serverObject_t *attacker, serverObject_t *target, int weapon, int damageFlags)
{	
	bool invincible;
	objectData_t	*targData = GetObjectByType(target->base.type);

	if (!targData->isVulnerable)
		return false;

	if (targData->meleeOnlyVulnerable && !IsMeleeType(weapon))
		return false;

	//team damage rules:
	//  - we can always damage ourself	
	//  - we can never damage non client objects on our own team
	//  - we can damage other clients on our own team if sv_teamDamage is on or we're in setup/warmup mode
	if (attacker->base.team == target->base.team && attacker != target)
	{
		if (!target->client)
			return false;

		if (!sv_teamDamage.integer && sl.status != GAME_STATUS_SETUP && sl.status != GAME_STATUS_WARMUP)
			return false;

	}

	if (target->client)
		invincible = target->client->ps.invincibleTime >= sl.gametime;
	else
		invincible = target->base.flags & BASEOBJ_INVINCIBLE;

	if (invincible)
		return false;

	return true;
}


void	SV_DamageClient(serverObject_t *attacker, client_t *target, vec3_t origin, int weapon, int attackDamage, int damageFlags)
{
	int			attackPush;
	vec3_t		dir;
	bool		blocked = false;
	float		dot2;
	bool		damageSelf;
	bool		doesDamage;
	bool		killsTarget;
	bool		sendWounded = false;
	
	if (!target)
		return;

	if (target->ps.fullhealth <= 0)
		return;

	attackPush = ((400.0 / (float)GetObjectByType(target->obj->base.type)->mass) * attackDamage * sv_damagePushScale.value);
	
	damageSelf = attacker->base.index == target->obj->base.index;

	if (target->ps.health <= 0)
	{
		//already dead...could do some nasty gibbing here (-:
		return;
	}

	//check for avoiding damage completely
	if (target->ps.flags & PS_ON_GROUND)
	{
		if (damageFlags & DAMAGE_NO_GROUND_TARGETS)
			return;
	}
	else
	{
		if (damageFlags & DAMAGE_NO_AIR_TARGETS)
			return;
	}

	//check for self damage
	if (target->obj->base.index == attacker->base.index)
	{		
		if (damageFlags & DAMAGE_SELF_HALF)
			attackDamage *= 0.5;
		if (damageFlags & DAMAGE_SELF_NONE)
			attackDamage = 0;
	}

	doesDamage = SV_CanDamageTarget(attacker, target->obj, weapon, damageFlags) || damageSelf;

	//check for blocking
	if (doesDamage)
	{
		M_SubVec3(origin, target->ps.pos, dir);
		M_Normalize(dir);

		dot2 = dir[0] * target->forward[0] + dir[1] * target->forward[1];

		if (target->ps.weaponState == AS_BLOCK
			&& dot2 > -cos(DEG2RAD((360 - objData[target->obj->base.type].blockArc)/2))
			&& !(damageFlags & DAMAGE_UNBLOCKABLE))
		{
			attackDamage *= (1.0f - target->obj->adjustedStats.blockPower);
			attackPush *= (1.0f - (target->obj->adjustedStats.blockPower / 2.0f));	//block half the push at best
			SV_Phys_AddEvent(target->obj, EVENT_DEFLECTED, SV_EncodePositionOnObject(target->obj, origin), 0);
			SV_Phys_AddEvent(attacker, EVENT_DAZED, SV_EncodePositionOnObject(attacker, origin), 0);
			
			//stun melee attackers when they are blocked
			if (attacker->base.index < MAX_CLIENTS && attacker->client->ps.item == 0)
			{
				attacker->client->ps.stunFlags |= STUN_COMBAT | STUN_MOVEMENT;
				attacker->client->ps.stunTime = sl.gametime + GetObjectByType(target->ps.inventory[target->ps.item])->blockStunTime;
			}

			target->ps.weaponState = AS_IDLE;
			blocked = true;
		}
	}

	killsTarget = (target->ps.health <= attackDamage);
	
	//do knockback before damage so that we can push even if damage is completely blocked
	if (doesDamage && !(damageFlags & DAMAGE_NO_KNOCKBACK))
	{
		vec3_t org = { origin[0],origin[1],origin[2]-5};
	//	if (damageFlags & DAMAGE_SPLASH/* || attackPush > 100*/)		//hack to give vertical velocity for radius damage wps			
	//		org[2] -= 25;
	//	else// if (attackPush > 100)
	//		org[2] -= 12;
	
		SV_DamagePush(target, org, attackPush, true);//killsTarget);
	}

	//if no damage is being done, don't have player react as if they are injured
	if (attackDamage <= 0)
		return;

	target->ps.attacker = attacker->base.index;

	if ((damageFlags & DAMAGE_STRIP_STATES) && doesDamage)
	{
		int	i;

		for (i = 0; i < MAX_STATES; i++)
		{
			if (stateData[i].isVulnerable)
				SV_RemoveStateFromObject(target->obj->base.index, i);
		}
		attackDamage = 0;
		doesDamage = false;
	}

	if (doesDamage)
	{
		target->ps.health -= attackDamage;
		target->obj->base.health = target->ps.health;
		
		//adjust attackDamage to reflect actual damge dealt on a killing blow
		if (target->ps.health < 0)
			attackDamage += target->ps.health;

		//vampire effect
		if (GetObjectByType(weapon)->transferHealth > 0)
		{
			if (attacker->base.index < MAX_CLIENTS)
			{
				attacker->client->ps.health += attackDamage * GetObjectByType(weapon)->transferHealth;
				if (attacker->client->ps.health > attacker->adjustedStats.fullhealth)
					attacker->client->ps.health = attacker->adjustedStats.fullhealth;
				attacker->base.health = attacker->client->ps.health;
			}
			else
			{
				attacker->base.health += attackDamage * GetObjectByType(weapon)->transferHealth;
				if (attacker->base.health > attacker->base.fullhealth)
					attacker->base.health = attacker->base.fullhealth;
			}
		}

		//stamina regain effect
		if (GetObjectByType(weapon)->staminaDrain > 0)
		{
			if (attacker->base.index < MAX_CLIENTS)
			{
				attacker->client->ps.stamina += attackDamage * GetObjectByType(weapon)->staminaDrain;
				if (attacker->client->ps.stamina > attacker->adjustedStats.maxStamina)
					attacker->client->ps.stamina = attacker->adjustedStats.maxStamina;
			}
		}

		if (!blocked)
		{
			if (!(damageFlags & DAMAGE_NO_REACTION))
				sendWounded = true;
			//if (!target->ps.item)	//only cancel attacks in melee combat
			//	target->ps.currentAttack = 0;
		}
		if (!damageSelf)
		{
			if (attacker->client)
			{
				//add to client stats
				attacker->client->stats.playerDamage += attackDamage;						
				//let the originator know they damaged something
				Phys_AddEvent(&attacker->client->ps, EVENT_WEAPON_HIT, 0, 0);	
			}
		}
	}

	if (target->ps.health <= 0)
	{
		//determine the state that killed them
		if (weapon < 0)
			weapon = -target->ps.states[-weapon];
		SV_KillClient(target, attacker, origin, weapon, attackDamage, damageFlags);
	}
	else
	{
		if (sendWounded)
			SV_Phys_AddEvent(target->obj, EVENT_WOUNDED, SV_EncodePositionOnObject(target->obj, origin), (byte)MIN(255, attackDamage));
	}
}


/*==========================

  SV_DamageTarget

  anything that does damage should go through here

  ==========================*/

void	SV_DamageTarget(serverObject_t *attacker, serverObject_t *target, vec3_t pos, int weapon, int damage, int damageFlags)
{
	int inflicter = 0;
	float pierce = 1.0;

	if (!target)
		return;

	if (IsWeaponType(weapon) || IsItemType(weapon))
		inflicter =  weapon;
	else if (weapon >= 0)
		inflicter = attacker->base.type;

	if ((target->base.health <= 0 && target->adjustedStats.fullhealth) || !target->base.active)
		return;

	//adjust damage by pierce value
	//this is done before we adjust the attacker, since we want the actual inflicting objects pierce value
	if (IsUnitType(target->base.type))
	{
		if (GetObjectByType(target->base.type)->isSiegeWeapon)
		{
			if (IsUnitType(inflicter))
				pierce = attacker->adjustedStats.siegePierce;
			else
				pierce = GetObjectByType(inflicter)->siegePierce;
		}
		else
		{
			if (IsUnitType(inflicter))
				pierce = attacker->adjustedStats.unitPierce;
			else
				pierce = GetObjectByType(inflicter)->unitPierce;
		}
	}
	else if (IsBuildingType(target->base.type))
	{
		if (IsUnitType(inflicter))
			pierce = attacker->adjustedStats.bldPierce;
		else
			pierce = GetObjectByType(inflicter)->bldPierce;
	}

	damage *= pierce;

	//apply armor
	damage -= (damage * SV_GetArmor(target));

	//cores.Console_DPrintf("SV_DamageTarget(): inflicter is %s, pierce is %f\n", objectNames[inflicter], pierce);

	//if the attacker has an owner, it must be a projectile
	//set the attacker to the object actually initiating the attack
	if (attacker->owner)
	{
		//only pass this to the player that set it if they are still on the team
		if (attacker->base.team == attacker->owner->base.team)
			attacker = attacker->owner;
	}

	//first, determine what kind of object we hit
	if (target->client)
	{
		//we hit another player
		SV_DamageClient(attacker, target->client, pos, weapon, damage, damageFlags);					
	}
	else
	{
		if (SV_CanDamageTarget(attacker, target, weapon, damageFlags))
		{
			//hit another type of object
			if (target->damage)
			{
				target->damage(target, attacker, pos, weapon, damage, damageFlags);
			}
		}
	}

	if (target->numRiders && target->base.health <= 0)
	{
		//make sure we kill anything that's inside, otherwise they'll be left in limbo
	
		int n;

		for (n=0; n<target->numRiders; n++)
		{
			//recursively damage everything
			//set their attacker to the same thing that killed the transport
			SV_DamageTarget(attacker, &sl.objects[target->riders[n]], pos, weapon, 999999, 0);
		}
	}

	//reveal through commander's fog of war for a few seconds
	if (target->base.team)
	{
		if (attacker->base.team != target->base.team)
			attacker->fogRevealTime = sl.gametime + sv_fogRevealTime.integer;
	}
}


/*==========================

  SV_DamageRadius

  Calls SV_DamageTarget for all valid targets within <radius> units of <origin>.
  Also generates 'quake' events (screen shakes)

 ==========================*/

void	SV_DamageRadius(serverObject_t *source, vec3_t origin, int weapon, float radius, int attackDamage, int damageFlags)
{
	int			index, damage, n;
	vec3_t		pos, vec, org, corners[9];
	float		dist;
	traceinfo_t	trace;
	bool skip = false;

	if (!attackDamage)
		return;

	//prevent div by zero
	if (radius < 1)
		radius = 1;

	//shifting the origin of the up slightly helps keep it from geting 'caught' inside objects
	M_AddVec3(origin, vec3(0, 0, sv_radiusTweak.value), org);

 	//test each object
	for (index = 0; index <= sl.lastActiveObject; index++)
	{
		//don't bother attacking yourself
		if (index == source->base.index)
			continue;

		//only active objects
		if (!sl.objects[index].base.active)
			continue;

		//skip things we know we can't damage
		if ((damageFlags & DAMAGE_NO_STRUCTURES) && IsBuildingType(sl.objects[index].base.type))
			continue;

		//get the correct possition
		SV_GetPosition(index, pos);
		
		//make sure the distance is within the radius
		//this is a preliminary check and not entirely acurate, so we error
		//on the side of the hit being a success

		//get distance from origin to target
		M_SubVec3(pos, org,vec);
 		dist = M_GetVec3Length(vec);

		//create a vector whose length is the radius of a sphere that could contain the objects bounding box
		vec[0] = MAX(ABS(sl.objects[index].base.bmin[0]), ABS(sl.objects[index].base.bmax[0]));
		vec[1] = MAX(ABS(sl.objects[index].base.bmin[1]), ABS(sl.objects[index].base.bmax[1]));
		vec[2] = MAX(ABS(sl.objects[index].base.bmin[2]), ABS(sl.objects[index].base.bmax[2]));
		
		//subtract the "bounding sphere" from the distance
		dist -= M_GetVec3Length(vec);
		if (dist < 0)
			dist = 0;
		
		//first check for generating a quake event... this isn't gameplay affecting, so it doesn't need to be precise
		if ((dist <= radius * sv_quakeMultiplyer.value) &&
			index < MAX_CLIENTS &&
			(damageFlags & DAMAGE_QUAKE_EVENT))
			SV_Phys_AddEvent(&sl.objects[index], EVENT_QUAKE, (byte)(127 * (1.0 - (dist/(float)(radius*sv_quakeMultiplyer.value)))), 0);

		//skip this object if it doesn't even pass the preliminary test
		if (dist > radius)
			continue;

		//test for splash protection
		for (n = 0; n < MAX_STATE_SLOTS; n++)
		{
			if (!sl.objects[index].base.states[n])
				continue;

			if (stateData[sl.objects[index].base.states[n]].splashProtect)
			{
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		//set up corners array to make a few extra traces in case the origin trace doesn't work
		//this should solve all but the most extreme cases of oddly shaped strucutres
  		M_CopyVec3(pos, corners[0]);

		{
			int x;

			corners[1][0] = sl.objects[index].base.bmin[0];
			corners[1][1] = sl.objects[index].base.bmin[1];
			corners[1][2] = sl.objects[index].base.bmin[2];

			corners[2][0] = sl.objects[index].base.bmax[0];
			corners[2][1] = sl.objects[index].base.bmin[1];
			corners[2][2] = sl.objects[index].base.bmin[2];

			corners[3][0] = sl.objects[index].base.bmin[0];
			corners[3][1] = sl.objects[index].base.bmax[1];
			corners[3][2] = sl.objects[index].base.bmin[2];

			corners[4][0] = sl.objects[index].base.bmin[0];
			corners[4][1] = sl.objects[index].base.bmin[1];
			corners[4][2] = sl.objects[index].base.bmax[2];

			corners[5][0] = sl.objects[index].base.bmin[0];
			corners[5][1] = sl.objects[index].base.bmax[1];
			corners[5][2] = sl.objects[index].base.bmax[2];

			corners[6][0] = sl.objects[index].base.bmax[0];
			corners[6][1] = sl.objects[index].base.bmin[1];
			corners[6][2] = sl.objects[index].base.bmax[2];

			corners[7][0] = sl.objects[index].base.bmax[0];
			corners[7][1] = sl.objects[index].base.bmax[1];
			corners[7][2] = sl.objects[index].base.bmin[2];

			corners[8][0] = sl.objects[index].base.bmax[0];
			corners[8][1] = sl.objects[index].base.bmax[1];
			corners[8][2] = sl.objects[index].base.bmax[2];

			for (x = 1; x < 9; x++)
				M_AddVec3(corners[x], pos, corners[x]);
		}

		//trace to the object to make sure they aren't behind a wall or something,
		//also the trace distance from damage origin to bounding box edge will
		//determine if the object was within range of the hit
		{
			bool	hit = false;
			int		n;

			for (n = 0; n < 9; n++)
			{
				cores.World_TraceBoxEx(&trace, org, corners[n], zero_vec, zero_vec, SURF_TERRAIN, source->base.index);
				if (trace.index != index)
					continue;

				M_SubVec3(org, trace.endpos, vec);
				dist = M_GetVec3Length(vec);

				if (dist > radius)
					continue;

				hit = true;
				break;
			}

			if (!hit)
				continue;
		}

		if (damageFlags & DAMAGE_NO_FALLOFF)
			damage = attackDamage;
		else
			damage = attackDamage * (1.0 - dist/(float)radius);

		if (damage > 0)
			SV_DamageTarget(source, 
							&sl.objects[trace.index], 
							vec3(origin[0],origin[1],origin[2]-sv_radiusTweak.value-10), 
							weapon, 
							damage, 
							damageFlags | DAMAGE_SPLASH);
	}
}
//=============================================================================


int	SV_CorpseDamage(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags)
{
	SV_Phys_AddEvent(obj, EVENT_WOUNDED, SV_EncodePositionOnObject(obj, pos), (byte)MIN(255, attackDamage));
	return 0;
}

void	SV_CorpseKill(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int damageFlags)
{
}

/*==========================

  SV_GenerateCorpse

  fixme: move into sv_clients

 ==========================*/

serverObject_t	*SV_GenerateCorpse(client_t *client, int animState)
{
	int n;

	serverObject_t	*corpse;
	
	//copy corpse to a new non-client object
	corpse = SV_AllocObject(client->ps.unittype, client->obj->base.team);	
	
	corpse->base.team = client->obj->base.team;
	corpse->base.health = 0;

	corpse->base.numEvents = 0;
	corpse->base.type = client->obj->base.type;
	corpse->base.flags = /*BASEOBJ_NO_LINK |*/ BASEOBJ_NAMEIDX_SPECIFIES_LEVEL;
	corpse->base.flags |= BASEOBJ_NO_ANIMATE;
	corpse->base.nameIdx = client->ps.score.level;
	corpse->base.surfaceFlags = SURF_CORPSE;

	M_CopyVec3( client->obj->base.pos, corpse->base.pos );
	M_CopyVec3( client->obj->base.angle, corpse->base.angle );
	M_CopyVec3( client->obj->base.bmin, corpse->base.bmin);
	M_CopyVec3( client->obj->base.bmax, corpse->base.bmax);
	corpse->base.bmax[Z] *= 0.1;
	
	corpse->base.animState = animState;
	corpse->base.animState2 = 0;

	corpse->damage = SV_CorpseDamage;
	corpse->kill = SV_CorpseKill;

	corpse->frame = SV_PlayerCorpseFrame;
	corpse->nextDecisionTime = sl.gametime + CORPSE_LINGER_TIME;
	M_CopyVec3(client->ps.velocity, corpse->velocity);

	corpse->owner = client->obj;
	corpse->base.owner = client->index;	//send owner across the network, too
	corpse->base.flags |= BASEOBJ_HAS_OWNER;

	corpse->flags = 1;		//fixme

	cores.Server_SpawnObject(corpse, corpse->base.index);
	cores.World_LinkObject(&corpse->base);

	//see if we have spectators, and if so move them over to the corpse
	for (n = 0; n < MAX_CLIENTS; n++)
	{
		if (n == client->index)
			continue;

		if (client->ps.flags & PS_CHASECAM && client->ps.chaseIndex == client->index)
			client->ps.chaseIndex = corpse->base.index;
	}
	
	return corpse;
}

//=============================================================================
#ifdef LOCAL_TESTING
void	SV_DamageMe_Cmd(int argc, char *argv[])
{
	SV_DamageTarget(sl.clients[0].obj, sl.clients[0].obj, sl.clients[0].ps.pos, 0, argc ? atoi(argv[0]) : 35, 0);
}

void	SV_PushBack_Cmd(int argc, char *argv[])
{
	M_AddVec3(vec3(200,200,50), sl.clients[0].ps.velocity, sl.clients[0].ps.velocity);
}

void	SV_Respawn_Cmd(int argc, char *argv[])
{
	if (!SV_ClientIsCommander(0))
		SV_SpawnClient(0, sl.clients[0].ps.pos, sl.clients[0].obj->base.angle);
}

#endif
//=============================================================================

//=============================================================================

/*==========================

  SV_DoOncePerInputStateEvents

  called after every AdvancePlayerState for each client

 ==========================*/

void	SV_DoOncePerInputStateEvents(client_t *client)
{
	if (client->phys_out.doAttack)
	{
		if (client->phys_out.doAttack == AS_WPSTATE_BACKFIRE)
		{
			SV_Phys_AddEvent(client->obj, EVENT_BACKFIRE, 0, client->ps.inventory[client->ps.item]);
			SV_GameScriptExecute(client->obj, client->obj, client->ps.inventory[client->ps.item], GS_ENTRY_BACKFIRE);
		}
		else
			SV_Attack(client->obj, client->phys_out.doAttack);

		client->phys_out.doAttack = 0;
	}
}


/*==========================

  SV_UnitHeal

  Handles regeneration of health

 ==========================*/

void	SV_UnitHeal(int objindex)
{
	client_t	*client;
	serverObject_t	*obj;
	objectData_t *unit;
	int	fullhealth, healamount, healrate;
	int	rateAdd = 0, amountAdd = 0;
	float	rateMul = 1.0, amountMul = 1.0;
	int index;

	if (objindex < 0 || objindex >= MAX_OBJECTS || !sl.objects[objindex].base.active)
		return;
	
	if (objindex < MAX_CLIENTS)
		client = &sl.clients[objindex];
	else
		client = NULL;
	obj = &sl.objects[objindex];

	unit = &objData[(client ? client->ps.unittype : sl.objects[objindex].base.type)];
	fullhealth = obj->base.fullhealth;
	healamount = unit->healAmount;
	healrate = unit->healRate;

	//get adjustments from states
	for (index = 0; index < MAX_STATE_SLOTS; index++)
	{
		int statenum;

		statenum = sl.objects[objindex].base.states[index];
		
		if (!statenum)
			continue;
	
		rateAdd += stateData[statenum].regenRateAdd;
		//rateMul += MAX(0.0, stateData[statenum].regenRateMult - 1.0);
		rateMul *= stateData[statenum].regenRateMult;
		amountAdd += stateData[statenum].regenAdd;
		amountMul += MAX(0.0, stateData[statenum].regenMult - 1.0);
	}

	healamount *= amountMul;
	healamount += amountAdd;
	healrate *= rateMul;
	healrate += rateAdd;
	if (healrate < 0)
		healrate = 0;
	if (healamount <= 0)
		return;

	//apply the healamount if enough time has passed
	if ( client ? (client->ps.health > 0) : (obj->base.health > 0) )
	{
		if ((sl.gametime - obj->lastHeal) >= healrate)
		{
			if (client)
			{
				client->ps.health += healamount;
				if (client->ps.health > fullhealth)
					client->ps.health = fullhealth;
			}
			else
			{
				obj->base.health += healamount;
				if (obj->base.health > fullhealth)
					obj->base.health = fullhealth;
			}
			obj->lastHeal = sl.gametime;
		}
	}
	else
	{
		obj->lastHeal = sl.gametime;
	}
}

void	SV_RegenerateMana(int objindex)
{
	int	index;
	serverObject_t	*obj;
	playerState_t *ps;
	objectData_t	*unit;
	int	rate, amount, max;

	if (objindex < 0 || objindex > MAX_CLIENTS)
		return;

	obj = &sl.objects[objindex];
	ps = &sl.objects[objindex].client->ps;
	unit = GetObjectByType(ps->unittype);

	if (unit->maxMana <= 0)
		return;

	rate = unit->manaRegenRate;
	amount = unit->manaRegenAmount;
	max = unit->maxMana;

	for (index = 0; index < MAX_INVENTORY; index++)
	{
		objectData_t *item;

		if (!ps->inventory[index])
			continue;

		item = GetObjectByType(ps->inventory[index]);
		if (!item->isActivated)
		{
			rate *= item->manaRateMult;
			amount += item->manaRegenAdd;
		}
	}

	if (sl.gametime - obj->lastManaAdd < rate)
		return;

	ps->mana += amount;

	if (ps->mana > max)
		ps->mana = max;

	obj->lastManaAdd = sl.gametime;
}


/*==========================

  SV_DoOncePerFrameEvents

  called once per frame for each client

 ==========================*/

extern cvar_t	sv_xp_survival_interval;

void	SV_DoOncePerFrameEvents(client_t *client)
{
	int n;
	playerState_t *ps = &client->ps;
	
	if (client->ps.status != STATUS_PLAYER)
		return;
	if (GetObjectByType(client->ps.unittype)->objclass != OBJCLASS_UNIT)
		return;

	if (client->nextSurvivalReward <= sl.gametime)
	{
		SV_AddExperience(client, EXPERIENCE_SURVIVAL, 0, 1.0);
		client->nextSurvivalReward = sl.gametime + sv_xp_survival_interval.integer;
	}

	SV_UnitHeal(client->obj->base.index);
	SV_RegenerateMana(client->obj->base.index);

	//respond to events that got added to the playerstate this frame
	for (n=0; n<ps->numEvents; n++)
	{
		objEvent_t *ev = &ps->events[n];

		switch(ev->type)
		{
			case EVENT_RESOURCE_FULL:
				if (!client->obj->goal || client->obj->goal == GOAL_MINE)
				{
					//give them a helper waypoint to let them know where to drop off resources										
					SV_PlayerTargetObject(NULL, client->obj, SV_FindNearestDropoffPoint(client->obj), GOAL_DROPOFF_RESOURCES);
				}
				break;
		}
	}
}


//free an object when gametime reaches obj->nextDecisionTime
void	SV_FreeNextDecision_Frame(serverObject_t *obj)
{
	if (sl.gametime >= obj->nextDecisionTime)
	{
		SV_FreeObject(obj->base.index);
		return;
	}

	M_ClearVec3(obj->velocity);			//fixme: hack for preventing dead bodies from sliding
	M_ClearVec3(obj->base.prVelocity);
}

//attach an event to this object, then free it after 'msec' milliseconds
void	SV_DeathEvent(serverObject_t *obj, byte event, byte param, byte param2, int msec, bool render)
{
	SV_Phys_AddEvent(obj, event, param, param2);
	obj->frame = SV_FreeNextDecision_Frame;
	obj->nextDecisionTime = sl.gametime + MAX(100, msec);

	obj->base.health = 0;

	obj->base.flags |= BASEOBJ_NO_LINK | (render ? 0 : BASEOBJ_MARKED_FOR_DEATH);
	obj->flags |= OBJECT_FLAG_NOT_A_TARGET;

	cores.World_UnlinkObject(&obj->base);
}
/*=============================================================================
=============================================================================*/


void	SV_InitEvents()
{
#ifdef LOCAL_TESTING
	cores.Cmd_Register("dmg", SV_DamageMe_Cmd);
	cores.Cmd_Register("pushback", SV_PushBack_Cmd);
	cores.Cmd_Register("respawn", SV_Respawn_Cmd);
#endif
	cores.Cvar_Register(&sv_damagePushScale);
	
	cores.Cvar_Register(&sv_radiusTweak);

	cores.Cvar_Register(&sv_quakeMultiplyer);

	cores.Cvar_Register(&sv_respawnTime);
	cores.Cvar_Register(&sv_useRespawnWindow);
	cores.Cvar_Register(&sv_fraglimit);
	cores.Cvar_Register(&sv_teamDamage);

	cores.Cvar_Register(&sv_maxPush);
	cores.Cvar_Register(&sv_hitStunTime);

	cores.Cvar_Register(&sv_fogRevealTime);

	cores.Cvar_Register(&sv_goodieBags);
}
