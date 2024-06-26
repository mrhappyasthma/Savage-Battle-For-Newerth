// (C) 2001 S2 Games

// sv_items.c

#include "server_game.h"
//=============================================================================

extern cvar_t	sv_maxMoney;

cvar_t sv_goodiesForAll = { "sv_goodiesForAll", "1" };		//0 == only one player can pick up the bag
															//1 == the whole team can pick up the bag
															//2 == both teams can pick up the bag
cvar_t sv_goodieDuration =			{ "sv_goodieDuration", "10000" };
cvar_t sv_goodieMaxMoney =			{ "sv_goodieMaxMoney", "1000" };
cvar_t sv_goodieBigMoneyChance =	{ "sv_goodieBigMoneyChance", "30" };	//i.e. 1 in 30 chance
cvar_t sv_goodieAmmoChance =		{ "sv_goodieAmmoChance", "4" };

extern cvar_t	sv_projRestVelocity;
extern cvar_t	sv_projRestSlope;

/*==========================

  SV_MoveItem

  Applys gravity to an item

 ==========================*/

void	SV_MoveItem(serverObject_t *item, bool link)
{
	vec3_t		startPos, targetPos;
	traceinfo_t	trace;
	int			impactTime, maxhits = 0, impactTimeOffset = 0;
	float		dot, remaining = 1.0;
	vec3_t		vel;

	if (item->base.flags & BASEOBJ_ATTACHED_TO_OWNER)
	{
		vec3_t	pos;

		if (!item->owner)
			return;

		SV_GetPosition(item->owner->base.index, item->base.pos);

		if (item->owner->base.health <= 0)
		{
			if (item->kill)
				item->kill(item, item->owner, pos, 0, 0);
			else
				SV_FreeObject(item->base.index);
		}
		return;
	}

	if (!(item->base.flags & BASEOBJ_USE_TRAJECTORY))
	{
		if (!SV_Phys_ObjectOnGround(item, &trace))
		{
			item->base.flags |= BASEOBJ_USE_TRAJECTORY;
			item->base.traj.acceleration = 0;
			item->base.traj.gravity = GetObjectByType(item->base.type)->gravity;
			M_CopyVec3(item->base.pos, item->base.traj.origin);
			item->base.traj.startTime = sl.gametime;
			M_ClearVec3(item->base.traj.velocity);
		}
		else
		{
			return;
		}
	}

	if (link)
		cores.World_UnlinkObject(&item->base);

	while (remaining > 0.0001 && maxhits < 10)
	{
		//evalutate the trajectory
		M_CopyVec3(item->base.pos, startPos);
		Traj_GetPos(&item->base.traj, sl.gametime + impactTimeOffset, targetPos);
		cores.World_TraceBoxEx(&trace, startPos, targetPos, item->base.bmin, item->base.bmax, SURF_CORPSE|SURF_TRANSPARENT_ITEM, (item->owner) ? item->owner->base.index : 0);

		//Bounce
		////////////////
		//rebuild the trajectory structure
		if (trace.fraction < 1.0)
		{
			impactTime = sl.gametime + impactTimeOffset + ((sl.frame_msec - impactTimeOffset) * trace.fraction);
			
			impactTimeOffset = impactTime - sl.gametime;
			
			Traj_GetVelocity(&item->base.traj, impactTime, vel);

			M_CopyVec3(trace.endpos, item->base.traj.origin);
			M_AddVec3(trace.normal, item->base.traj.origin, item->base.traj.origin);
			M_CopyVec3(item->base.traj.origin, item->base.pos);
			
			item->base.traj.startTime = impactTime;
			
			dot = M_DotProduct(vel, trace.normal);
			M_MultVec3(trace.normal, -2 * dot, item->base.traj.velocity);
			M_AddVec3(item->base.traj.velocity, vel, item->base.traj.velocity);
			M_MultVec3(item->base.traj.velocity, GetObjectByType(item->base.type)->bounce, item->base.traj.velocity);

			//let it come to a rest
			if (trace.normal[2] > sv_projRestSlope.value && M_GetVec3Length(item->base.traj.velocity) < sv_projRestVelocity.value)
			{
				item->base.flags &= ~BASEOBJ_USE_TRAJECTORY;
				M_CopyVec3(trace.normal, item->forward);
				break;
			}

			SV_Phys_AddEvent(item, EVENT_BOUNCE, 0, 0);
		}

		remaining -= trace.fraction * remaining;
		maxhits++;	//this is just a safety meassure to assure we don't hit an infinite loop
	}

	M_CopyVec3(trace.endpos, item->base.pos);
	M_CopyVec3(item->base.traj.velocity, item->forward);
	M_Normalize(item->forward);

	if (link)
		cores.World_LinkObject(&item->base);
}

bool	SV_MakeItemSolid(serverObject_t *item)
{
	if (!GetObjectByType(item->base.type)->isSolid)
		return false;

	if (item->base.surfaceFlags & SURF_TRANSPARENT_ITEM && item->base.health > 0)
	{
		traceinfo_t	trace;

		cores.World_TraceBoxEx(&trace, item->base.pos, item->base.pos, item->base.bmin, item->base.bmax, SURF_TERRAIN, item->base.index);
		if (trace.index == -1)
		{
			cores.World_UnlinkObject(&item->base);
			item->base.surfaceFlags = SURF_SOLID_ITEM;
			cores.World_LinkObject(&item->base);
			return true;
		}
		return false;
	}

	return true;
}


/*==========================

  SV_PickupItem

  give the item to a client who ran over it 

 ==========================*/

bool	SV_PickupItem(serverObject_t *itemObj, int clientnum, byte giveItem)
{
	bool gotItem = false;
	playerState_t *ps = &sl.clients[clientnum].ps;
	objectData_t *objdata = GetObjectByType(giveItem);

	//deathmatch pickup scheme (test)

	if (IsWeaponType(giveItem))
	{
		if (ps->inventory[1] != giveItem)
		{
			ps->inventory[1] = giveItem;
			ps->ammo[1] = GetObjectByType(giveItem)->ammoStart;
			gotItem = true;
		}
		else
		{
			if (ps->ammo[1] < GetObjectByType(giveItem)->ammoMax)
			{
				ps->ammo[1] += GetObjectByType(giveItem)->ammoGroup;
				gotItem = true;
			}
		}		
	}

	if (gotItem)
	{
		SV_Phys_AddEvent(sl.clients[clientnum].obj, EVENT_PICKUP_WEAPON, 0, 0);		
		
		itemObj->base.flags |= BASEOBJ_NO_RENDER;
		itemObj->nextPickupTime = sl.gametime + objdata->pickupRespawn;
	}

	return gotItem;
}

#define ITEM_PICKUP_RADIUS	20


/*==========================

  SV_PickupItemFrame

  Called each frame for pickup items

 ==========================*/

void	SV_PickupItemFrame(serverObject_t *item)
{
	//if (itemData->canPickup)
//	{
		int client;

		SV_MoveItem(item, false);

		if (sl.gametime >= item->nextPickupTime)
		{
			item->base.flags &= ~BASEOBJ_NO_RENDER;

			for (client = 0; client < MAX_CLIENTS; client++)
			{
				vec3_t dif;
	
				M_SubVec3(item->base.pos, sl.clients[client].ps.pos, dif);
				if (M_GetVec3Length(dif) <= ITEM_PICKUP_RADIUS)
				{
					byte giveItem;
					objectData_t *itemData = GetObjectByType(item->base.type);
					
					giveItem = GetObjectTypeByName(itemData->pickupGive);

					if (SV_PickupItem(item, client, giveItem))
						break;					
				}
			}
		}
		else
		{
			//don't render pickup items if they can't be picked up
			item->base.flags |= BASEOBJ_NO_RENDER;
			return;
		}
//	}
}

serverObject_t	*SV_SpawnPickupItem(byte objtype, int team, vec3_t pos, vec3_t angle)
{
	serverObject_t *item = SV_AllocObject((byte)objtype, team);

	item->base.fullhealth = GetObjectByType(objtype)->fullHealth;
	SV_FillInBaseStats(item);

	if (!GetObjectByType(objtype)->canPickup)
		core.Game_Error(fmt("SV_SpawnObject: attempt to spawn item that's not a pickup (type %i)\n", objtype));

	M_CopyVec3(pos, item->base.pos);
	M_CopyVec3(angle, item->base.angle);		
				
	item->base.type = objtype;
	item->base.health = 100;
	item->damage = SV_ObjectDamaged;
	item->frame = SV_PickupItemFrame;	
	item->base.surfaceFlags = SURF_TRANSPARENT_ITEM;

	item->base.animState = AS_IDLE;

	cores.World_LinkObject(&item->base);	

	return item;
}


//FIXME: we can use a normal trace to determine pickup now
void	SV_GoodieBagFrame(serverObject_t *item)
{
	int clientnum;
	int pickedUp = -1;
	
	SV_MoveItem(item, /*true*/ false);		//change to true when above fixme is done

	if (item->base.flags & BASEOBJ_ASSIGNED_TO_CLIENT)
	{
		vec3_t dif;
		client_t *client = &sl.clients[item->base.assignedToClient];

		M_SubVec3(item->base.pos, client->ps.pos, dif);
		if (M_GetVec3Length(dif) <= ITEM_PICKUP_RADIUS)
		{
			pickedUp = client->index;
		}
	}
	else
	{
		//fixme: this loop gives an unfair advantage to lower client numbers
		for (clientnum = 0; clientnum < MAX_CLIENTS; clientnum++)
		{
			vec3_t dif;
			client_t *client = &sl.clients[clientnum];

			//don't allow people on the other team to pick it up unless it's set to team 0
			if (item->base.team && item->base.team != client->info.team)
				continue;
			//if (client->ps.money >= sv_maxMoney.integer)
			//	continue;
			
			M_SubVec3(item->base.pos, client->ps.pos, dif);
			if (M_GetVec3Length(dif) <= ITEM_PICKUP_RADIUS)
			{
				pickedUp = client->index;
				break;
			}
		}
	}

	if (pickedUp > -1)
	{
		bool addevent = false;
		client_t *client = &sl.clients[pickedUp];

		if (GetObjectByType(item->base.type)->giveAmmo)
		{
			//hardcoded to use #1 inventory slot for weapon
			if (IsWeaponType(client->ps.inventory[1]))
			{
				objectData_t *weap = GetObjectByType(client->ps.inventory[1]);

				if (client->ps.ammo[1] < weap->ammoMax)
				{
					addevent = true;
					client->ps.ammo[1] += weap->ammoGroup;
					if (client->ps.ammo[1] > weap->ammoMax)
						client->ps.ammo[1] = weap->ammoMax;
				}
			}		
		}
		
		if (GetObjectByType(item->base.type)->giveMana)
		{
			int maxMana = GetObjectByType(client->ps.unittype)->maxMana;

			if (GetObjectByType(client->ps.inventory[1])->useMana)
			{
				if (client->ps.mana < maxMana)
				{
					addevent = true;
					client->ps.mana += GetObjectByType(item->base.type)->giveMana;
					if (client->ps.mana > maxMana)
						client->ps.mana = maxMana;
				}
			}		
		}
		
		if (item->moneyCarried)
		{
			addevent = true;
			SV_GiveMoney(pickedUp, item->moneyCarried, true);
		}

		if (addevent)
		{
			Phys_AddEvent(&client->ps, EVENT_GOODIE_PICKUP, 0, 0);			
		}
		else
		{
			pickedUp = -1;
		}
	}
		
	if (sl.gametime >= item->nextDecisionTime || pickedUp > -1)
	{
		//free me
		SV_FreeObject(item->base.index);
	}

	if (sl.gametime >= item->nextDecisionTime - 3000)
		item->base.flags |= BASEOBJ_EXPIRING;
}


/*==========================

  SV_SpawnGoodieBag

 ==========================*/

void	SV_SpawnGoodieBag(serverObject_t *target, const vec3_t pos, client_t *attacker)
{
	serverObject_t *goodiebag;
	objectData_t *targetdef = GetObjectByType(target->base.type);
	byte goodietype;
	byte ammopacktype;
	int numspawn;
	int goldtotal = 0;
	int reward;

	if (sv_goodieMaxMoney.integer < 10)
		cores.Cvar_SetVarValue(&sv_goodieMaxMoney, 10);	

	if (!targetdef->killGoldReward)
		return;

	if (rand() % sv_goodieBigMoneyChance.integer == 0)
	{
		//lucky!
		reward = targetdef->killGoldReward * 10;
		if (reward > sv_maxMoney.integer)
			reward = sv_maxMoney.integer;
	}
	else
		reward = targetdef->killGoldReward;

	goodietype = GetObjectTypeByName(GetGoodieBagName());
	ammopacktype = GetObjectTypeByName(raceData[sl.teams[attacker->info.team].race].ammoDrop);
	if (!goodietype || !ammopacktype)
	{
		core.Game_Error(fmt("SV_SpawnGoodieBag: Couldn't find goodie definition (%s)\n", GetGoodieBagName()));
	}

	numspawn = 0;
	while(numspawn < 30)
	{
		vec3_t goodiepos;
		bool spawnAmmo = false;

		int amount = sv_goodieMaxMoney.integer;

		spawnAmmo = (rand() % sv_goodieAmmoChance.integer) == 0;
		
		goodiebag = SV_AllocObject((byte)(spawnAmmo ? ammopacktype : goodietype), attacker->info.team);

		goodiebag->base.angle[2] = rand() % 360;
		goodiebag->base.health = 100;
		goodiebag->base.surfaceFlags = SURF_TRANSPARENT_ITEM;
		goodiebag->damage = SV_ObjectDamaged;
		goodiebag->frame = SV_GoodieBagFrame;
		goodiebag->base.animState = AS_IDLE;

		goodiepos[0] = pos[0];
		goodiepos[1] = pos[1];
		goodiepos[2] = pos[2] + (GetObjectByType(target->base.type)->bmax[Z] / 2.0);
		
		M_CopyVec3(goodiepos, goodiebag->base.pos);
		M_CopyVec3(goodiepos, goodiebag->base.traj.origin);
		
		if (IsBuildingType(target->base.type))
		{
			M_SetVec3(goodiebag->base.traj.velocity, M_Randnum(-200, 200), M_Randnum(-200, 200), M_Randnum(200, 300));
		}
		else
		{
			M_SetVec3(goodiebag->base.traj.velocity, M_Randnum(-100, 100), M_Randnum(-100, 100), M_Randnum(100, 200));
		}

		goodiebag->base.traj.startTime = sl.gametime;
		goodiebag->base.traj.gravity = GetObjectByType(goodietype)->gravity;
		goodiebag->base.flags |= BASEOBJ_USE_TRAJECTORY;

		if (goldtotal + amount > reward)
			amount = reward - goldtotal;

		if (spawnAmmo)
			goodiebag->moneyCarried = 0;
		else
			goodiebag->moneyCarried = amount;
			

		if (attacker->obj == target)
		{
			goodiebag->base.team = (3 ^ attacker->info.team);	//other team
		}
		else if (sv_goodiesForAll.integer == 0)			//available to attacker only
		{
			goodiebag->base.assignedToClient = attacker->index;
			goodiebag->base.team = attacker->info.team;
			goodiebag->base.flags |= BASEOBJ_ASSIGNED_TO_CLIENT;
		}
		else if (sv_goodiesForAll.integer == 1)		//available to the whole team
		{
			goodiebag->base.team = attacker->info.team;
		}
		else										//available to everyone
		{
			goodiebag->base.team = 0;
		}

		goodiebag->nextDecisionTime = sl.gametime + sv_goodieDuration.value;

		if (!numspawn)
		{
			//add the event to the first one we spawn only
			SV_Phys_AddEvent(goodiebag, EVENT_SPAWN, 0, 0);			
		}

		goldtotal += amount;
		if (goldtotal >= reward)
			break;

		numspawn++;
	}
}

bool	SV_ItemStateTransition(serverObject_t *item)
{
	if (item->nextStateTransition && sl.gametime >= item->nextStateTransition)
	{
		switch (item->nextState)
		{
		case AS_ITEM_SLEEP:
			SV_ItemSleep(item, 0, AS_ITEM_SLEEP);
			return true;

		case AS_IDLE:
			SV_ItemIdle(item, 0, AS_IDLE);
			return true;

		case AS_ITEM_ACTIVE:
			SV_ItemActivate(item, 0, AS_ITEM_ACTIVE);
			return true;
		}
	}

	return false;
}


/*==========================

  SV_ItemActiveFrame

 ==========================*/

void	SV_ItemActiveFrame(serverObject_t *item)
{
	bool	solid;
	objectData_t	*itemData = GetObjectByType(item->base.type);

	solid = SV_MakeItemSolid(item);
	SV_MoveItem(item, solid);

	if (SV_ItemStateTransition(item))
		return;

	if (item->nextDecisionTime && sl.gametime < item->nextDecisionTime)
		return;

	SV_GameScriptExecute(item, item, item->base.type, GS_ENTRY_ACTIVE);
}


/*==========================

  SV_ItemActivate

 ==========================*/

void	SV_ItemActivate(serverObject_t *item, int duration, int nextstate)
{
	objectData_t	*itemData = GetObjectByType(item->base.type);

	SV_Phys_AddEvent(item, EVENT_ITEM_ACTIVATE, 0, 0);
	item->base.animState = AS_ITEM_ACTIVE;
	item->frame = SV_ItemActiveFrame;
	item->nextDecisionTime = sl.gametime;
	item->nextStateTransition = (duration) ? sl.gametime + duration : 0;
	item->nextState = nextstate;

	SV_GameScriptExecute(item, item, item->base.type, GS_ENTRY_ACTIVATE);
}


/*==========================

  SV_ItemIdleFrame

 ==========================*/

void	SV_ItemIdleFrame(serverObject_t *item)
{
	bool	solid;
	objectData_t	*itemData = GetObjectByType(item->base.type);

	solid = SV_MakeItemSolid(item);
	SV_MoveItem(item, solid);
	
	if (SV_ItemStateTransition(item))
		return;

	if (item->nextDecisionTime && sl.gametime < item->nextDecisionTime)
		return;

	SV_GameScriptExecute(item, item, item->base.type, GS_ENTRY_IDLING);
}


/*==========================

  SV_ItemIdle

 ==========================*/

void	SV_ItemIdle(serverObject_t *item, int duration, int nextstate)
{
	objectData_t	*itemData = GetObjectByType(item->base.type);

	SV_Phys_AddEvent(item, EVENT_ITEM_IDLE, 0, 0);
	item->base.animState = AS_IDLE;
	item->frame = SV_ItemIdleFrame;
	item->nextDecisionTime = sl.gametime;
	item->nextStateTransition = (duration) ? sl.gametime + duration : 0;
	item->nextState = nextstate;

	SV_GameScriptExecute(item, item, item->base.type, GS_ENTRY_IDLE);
}


/*==========================

  SV_ItemSleepFrame

 ==========================*/

void	SV_ItemSleepFrame(serverObject_t *item)
{
	bool	solid;

	solid = SV_MakeItemSolid(item);
	SV_MoveItem(item, solid);
	
	if (SV_ItemStateTransition(item))
		return;

	if (item->nextDecisionTime && sl.gametime < item->nextDecisionTime)
		return;

	SV_GameScriptExecute(item, item, item->base.type, GS_ENTRY_SLEEPING);
}


/*==========================

  SV_ItemSleep

 ==========================*/

void	SV_ItemSleep(serverObject_t *item, int duration, int nextstate)
{
	objectData_t	*itemData = GetObjectByType(item->base.type);

	SV_Phys_AddEvent(item, EVENT_ITEM_SLEEP, 0, 0);
	item->base.animState = AS_ITEM_SLEEP;
	item->frame = SV_ItemSleepFrame;
	item->nextDecisionTime = sl.gametime;
	item->nextStateTransition = (duration) ? sl.gametime + duration : 0;
	item->nextState = nextstate;

	SV_GameScriptExecute(item, item, item->base.type, GS_ENTRY_SLEEP);
}


/*==========================

  SV_ItemDamage

  Called when a vulnerable item takes damage

 ==========================*/

int 	SV_ItemDamage(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags)
{
	objectData_t	*item = GetObjectByType(obj->base.type);

	//see if they're already dead
	if (obj->base.health <= 0 && obj->adjustedStats.fullhealth)
	{
		obj->base.health = 0;
		return 0;
	}

	//todo: check for defense powerups, etc
	obj->base.health -= attackDamage;
	if (attacker->client)
		Phys_AddEvent(&attacker->client->ps, EVENT_WEAPON_HIT, 0, 0);

	if (obj->base.health <= 0 && obj->kill)
	{
		obj->kill(obj, attacker, pos, weapon, damageFlags);
	}
	else
	{
		if (!(damageFlags & DAMAGE_NO_REACTION))
			SV_Phys_AddEvent(obj, EVENT_WOUNDED, SV_EncodePositionOnObject(obj, pos), (byte)MIN(255, attackDamage));
	}

	return attackDamage;
}


/*==========================

  SV_ItemKill

 ==========================*/

void	SV_ItemKill(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int damageFlags)
{
	int				index;
	objectData_t	*item = GetObjectByType(obj->base.type);

	SV_GameScriptExecute(obj, attacker, obj->base.type, GS_ENTRY_DIE);

	//clear any links to this object
	for (index = 0; index < MAX_OBJECTS; index++)
	{
		if (!sl.objects[index].base.active)
			continue;

		if (sl.objects[index].link == obj)
			sl.objects[index].link = NULL;
	}

	if (attacker && attacker->client)
		SV_AddExperience(attacker->client, EXPERIENCE_ITEM_KILL, 0, item->expMult);
	SV_DeathEvent(obj, EVENT_DEATH, 0, 0, item->deathLinger, true);
	SV_UnlinkFromWorldObject(obj);
	obj->base.health = 0;
	obj->base.animState = AS_DEATH_GENERIC;
	if (!item->deathLinger)
		obj->base.flags |= BASEOBJ_NO_RENDER;
}


/*==========================

  SV_UseItem

  This function is called whenever a player uses an item in their inventory, and initiates
  any events that result.

 ==========================*/

void	SV_UseItem(int clientnum, int itemnum)
{
	objectData_t	*itemData = GetObjectByType(itemnum);
	serverObject_t	*player = &sl.objects[clientnum];

	if (itemData->objclass != OBJCLASS_ITEM)
		return;

	if (!itemData->isActivated)
		return;

	SV_Phys_AddEvent(player, EVENT_USE_ITEM, 0, (byte)itemnum);

	SV_GameScriptExecute(player, player, itemnum, GS_ENTRY_USE);
}


/*==========================

  SV_InitItems

  Register any commands/cvars related to items here

 ==========================*/

void	SV_InitItems()
{
	cores.Cvar_Register(&sv_goodiesForAll);
	cores.Cvar_Register(&sv_goodieDuration);
	cores.Cvar_Register(&sv_goodieMaxMoney);
	cores.Cvar_Register(&sv_goodieBigMoneyChance);
	cores.Cvar_Register(&sv_goodieAmmoChance);
}
