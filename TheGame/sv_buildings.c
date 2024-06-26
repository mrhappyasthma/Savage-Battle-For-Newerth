// (C) 2002 S2 Games

// sv_buildings.c


#include "server_game.h"

extern cvar_t sv_hitStructureRewardScale;
extern cvar_t sv_rangedHitStructureRewardScale;
extern cvar_t sv_repairCost;
extern cvar_t sv_respawnMultiplier;
extern cvar_t sv_buildingInitialHealthPercent;
extern cvar_t sv_placementLeniency;
extern cvar_t sv_minAttackDamageForNotification;

cvar_t	sv_claimableBuildings =			{ "sv_claimableBuildings",		"1", CVAR_CHEAT };
cvar_t	sv_buildingDmgScale =			{ "sv_buildingDmgScale",		"1", CVAR_SERVERINFO | CVAR_VALUERANGE, 0.5, 4 };
cvar_t	sv_minClaimInterval =			{ "sv_minClaimInterval",		"5000", CVAR_CHEAT };
cvar_t	sv_placeBuildingTestScale =		{ "sv_placeBuildingTestScale",	"0.5", CVAR_CHEAT };
cvar_t	sv_musicResetTime =				{ "sv_musicResetTime",			"120000" };


typedef enum
{
	BWARN_UNDER_ATTACK,
	BWARN_HALF_DESTROYED,
	BWARN_NEARLY_DESTROYED,
	BWARN_DESTROYED,
	BWARN_DESTROYED_ALL,		//all buildings of the same type have been destroyed
	BWARN_MINE_LOW,
	BWARN_MINE_EMPTY,
	BWARN_UNDER_SIEGE,
	NUM_BUILDING_WARNINGS
} buildingWarnings_enum;

char *buildingWarnings[] =
{
	"under_attack",
	"half_destroyed",
	"nearly_destroyed",
	"destroyed",
	"destroyed_all",
	"low",
	"empty",
	"under_siege",
	""
};

//=============================================================================



/*==========================

  SV_BuildingWarning

  if the given warning sound is defined for this building, send it

 ==========================*/

extern cvar_t sv_team1race;
extern cvar_t sv_team2race;

void	SV_BuildingWarning(int team, serverObject_t *obj, buildingWarnings_enum warning)
{
	char *teamRace;
	char *opposingRace;
	char *warningString;
	objectData_t *def = GetObjectByType(obj->base.type);	
	int n;

	if (!team)
		return;

	if (team == 1)
	{
		teamRace = sv_team1race.string;
		opposingRace = sv_team2race.string;
	}
	else
	{
		teamRace = sv_team2race.string;
		opposingRace = sv_team1race.string;
	}	

	//make sure we get the base building name so we don't have to define messages for every upgrade
	//don't recurse more than 4 times in case we have messed up object data
	for (n=0; n<4; n++)
	{
		if (GetObjectByName(def->builder[0])->objclass == OBJCLASS_BUILDING)
		{
			def = GetObjectByName(def->builder[0]);
		}
		else
			break;
	}	

	/*
	if (strncmp(fmt("%s_", teamRace), def->name, strlen(teamRace)+1)==0)
	{
		defName = def->name + strlen(teamRace) + 1;
	}
	*/

	if (warning == BWARN_MINE_LOW)
	{
		warningString = fmt("%s_%s_low", teamRace, def->name);
		if (Snd(warningString)[0])
			SV_SendNoticeToTeam(team, NOTICE_ALERT_STRING, obj->base.index, warningString);
	}
	else if (warning == BWARN_MINE_EMPTY)
	{
		warningString = fmt("%s_%s_empty", teamRace, def->name);
		if (Snd(warningString)[0])
			SV_SendNoticeToTeam(team, NOTICE_ALERT_STRING, obj->base.index, warningString);
	}
	else if (warning >= BWARN_UNDER_ATTACK && warning <= BWARN_NEARLY_DESTROYED)
	{		
		//friend
		for (n = warning; n >= BWARN_UNDER_ATTACK; n--)
		{
			warningString = fmt("friend_%s_%s_%s", teamRace, def->name, buildingWarnings[n]);
			if (Snd(warningString)[0])
			{
				SV_SendNoticeToTeam(team, NOTICE_ALERT_STRING, obj->base.index, warningString);
				break;
			}
		}

		//enemy
		for (n = warning; n >= BWARN_UNDER_ATTACK; n--)
		{
			warningString = fmt("enemy_%s_%s_%s", opposingRace, def->name, buildingWarnings[n]);
			if (Snd(warningString)[0])
			{
				SV_SendNoticeToTeam(team^3, NOTICE_ALERT_STRING, obj->base.index, warningString);
				break;
			}
		}
	}
	else if (warning == BWARN_DESTROYED || warning == BWARN_DESTROYED_ALL)
	{
		bool sent = false;

		//friend
		for (n = warning; n >= BWARN_DESTROYED; n--)
		{
			warningString = fmt("friend_%s_%s_%s", teamRace, def->name, buildingWarnings[n]);
			if (Snd(warningString)[0])
			{
				SV_SendNoticeToTeam(team, NOTICE_ALERT_STRING, obj->base.index, warningString);
				sent = true;
				break;
			}
		}

		if (!sent)
		{
			SV_SendNoticeToTeam(team, NOTICE_ALERT_STRING, obj->base.index, fmt("friend_%s_building_destroyed", teamRace));
		}
	
		sent = false;

		//enemy
		for (n = warning; n >= BWARN_DESTROYED; n--)
		{
			warningString = fmt("enemy_%s_%s_%s", opposingRace, def->name, buildingWarnings[n]);
			if (Snd(warningString)[0])
			{
				SV_SendNoticeToTeam(team^3, NOTICE_ALERT_STRING, obj->base.index, warningString);
				sent = true;
				break;
			}
		}

		if (!sent)
		{
			SV_SendNoticeToTeam(team^3, NOTICE_ALERT_STRING, obj->base.index, fmt("enemy_%s_building_destroyed", opposingRace));
		}
	}
	else if (warning == BWARN_UNDER_SIEGE)
	{				
		warningString = fmt("friend_%s_%s_%s", teamRace, def->name, buildingWarnings[BWARN_UNDER_SIEGE]);		
		if (Snd(warningString)[0])
		{			
			SV_SendNoticeToTeam(team, NOTICE_ALERT_STRING, obj->base.index, warningString);			
		}
		else
		{
			warningString = fmt("friend_%s_%s_%s", teamRace, def->name, buildingWarnings[BWARN_UNDER_ATTACK]);
			if (Snd(warningString)[0])
			{
				SV_SendNoticeToTeam(team, NOTICE_ALERT_STRING, obj->base.index, warningString);				
			}
		}
		
		warningString = fmt("enemy_%s_%s_%s", opposingRace, def->name, buildingWarnings[BWARN_UNDER_SIEGE]);
		if (Snd(warningString)[0])
		{
			SV_SendNoticeToTeam(team^3, NOTICE_ALERT_STRING, obj->base.index, warningString);			
		}
		else
		{
			warningString = fmt("enemy_%s_%s_%s", opposingRace, def->name, buildingWarnings[BWARN_UNDER_ATTACK]);
			if (Snd(warningString)[0])
			{
				SV_SendNoticeToTeam(team^3, NOTICE_ALERT_STRING, obj->base.index, warningString);				
			}
		}
	}
}


/*==========================

  SV_ClaimBuilding

  ==========================*/

void	SV_ClaimBuilding(serverObject_t *claimer, serverObject_t *building)
{
	int n;
	int other;
	objectData_t *def = GetObjectByType(claimer->base.type);

	if (!sv_claimableBuildings.integer)
		return;

	if (!claimer->base.team)
		return;		//npcs and such can't claim things

	if (claimer->base.team == building->base.team)
		return;		//already claimed for this team

	if (sl.gametime - building->lastClaimTime < sv_minClaimInterval.integer)
		return;		//this was claimed very recently

	if (claimer->base.team)
		other = claimer->base.team ^ 3;
	
	//todo: proper announcer message
	//SV_Announcer(...)

	SV_SendNoticeToTeam(claimer->base.team, NOTICE_BUILDING_CLAIMED, building->base.index, fmt("A %s has been claimed!", GetObjectByType(building->base.type)->description));

	//get rid of a waypoint if they had one on this structure
	for (n=0; n<MAX_CLIENTS; n++)
	{
		client_t *client = &sl.clients[n];
		client_t *giver;

		if (!client->active)
			continue;		

		if (client->waypoint.active && &sl.objects[client->waypoint.object_index] == building)
		{
			giver = client->waypoint.clientnum == -1 ? NULL : &sl.clients[client->waypoint.clientnum];

			if (client->obj == claimer)
			{
				//completed waypoint
				SV_PlayerClearTarget(client->obj, GOAL_NONE);
			}
			else
			{
				//failed waypoint
				SV_PlayerTargetPosition(giver, client->obj, 0, 0, GOAL_NONE);
			}
		}
	}

	if (building->base.team != 0)
	{
		//todo: proper announcer message
		//SV_Announcer(...)

		SV_SendNoticeToTeam(other, NOTICE_BUILDING_STOLEN, building->base.index, fmt("A %s has been stolen!", GetObjectByType(building->base.type)->description));
	}

	building->base.team = claimer->base.team;
	building->lastClaimTime = sl.gametime;
}


/*==========================

  SV_MineResourcesFromBuilding

  returns false if miner can no longer mine from this structure

 ==========================*/

bool	SV_MineResourcesFromBuilding(serverObject_t *miner, serverObject_t *mine, int requestedAmount)
{
	objectData_t *mineData = GetObjectByType(mine->base.type);
	int maxcarry;
	int amountToMine;
	int mineType = mineData->mineType;

	//safety checks
	if (!mineData->isMine)
		return false;

	if (mineType < 0 || mineType >= MAX_RESOURCE_TYPES)
		return false;

	if (mineData->isClaimable)
	{
		if (miner->base.team != mine->base.team)
		{
			SV_ClaimBuilding(miner, mine);
		}
	}

	//determine how much the miner can carry
	maxcarry = objData[miner->base.type].maxResources[mineType] + miner->adjustedStats.maxCarryBonus;
	if (miner->resources[mineType] + requestedAmount > maxcarry)
		amountToMine = maxcarry - miner->resources[mineType];
	else
		amountToMine = requestedAmount;

	//miner is full already
	if (amountToMine <= 0)
		return false;

	//does the mine have enough left?
	if (mine->resources[mineType] < amountToMine)
		amountToMine = mine->resources[mineType];

	//make the transaction
	miner->resources[mineType] += amountToMine;
	mine->resources[mineType] -= amountToMine;
	if (miner->client)
		SV_Phys_AddEvent(miner, EVENT_MINE, 0, 0);

	//check for an empty mine
	if (mine->resources[mineType] <= 0)
	{		
		mine->resources[mineType] = 0;
		SV_DestroyBuilding(mine);
		SV_BuildingWarning(miner->base.team, mine, BWARN_MINE_EMPTY);		//send a building warning here so it goes to the correct team
		return false;
	}
	else if (mine->resources[mineType] < mineData->mineAmount / 5 && !mine->mineWarning)
	{
		//getting low
		SV_BuildingWarning(miner->base.team, mine, BWARN_MINE_LOW);

		mine->mineWarning = true;		//so we won't send it again
	}
	
	//did the transaction fill up the miner?
	if (miner->resources[mineType] >= maxcarry)
	{
		if (miner->client)
			SV_Phys_AddEvent(miner, EVENT_RESOURCE_FULL, 0, 0);
		return false;
	}

	//keep mining!
	return true;
}


/*==========================

  SV_RepairBuilding

  Heals a structure and deducts resources from the team
  Returns true if the repair is successful

 ==========================*/

bool	SV_RepairBuilding(serverObject_t *worker, serverObject_t *building, int amount)
{
	byte			buildingtype = building->base.type;
	objectData_t	*bld = GetObjectByType(buildingtype);
	int				fullhealth = building->adjustedStats.fullhealth;
	int				index;

	//not a valid building
	if (!(bld->objclass == OBJCLASS_BUILDING))
		return false;

	//is building specifically flagged to not be repaired?
	if (!bld->canBeRepaired)
		return false;

	//only repair same team structures
	if (building->base.team != worker->base.team)
		return false;

	//does the building even need repairing?
	if (building->base.health >= fullhealth)
		return false;

	if (sv_repairCost.integer)
	{		
		if (!worker->client)		//clients won't cost the team resources when they repair
		{
			//check all costs
			for (index = 0; index < MAX_RESOURCE_TYPES; index++)
			{
				int cost;

				if (bld->cost[index] > 0)
				{
					cost = ceil(bld->cost[index] * amount / (float)fullhealth);
					if (cost > sl.teams[worker->base.team].resources[index])
					{
						cores.Console_Printf("Not enough %s to repair %s.\n", resourceData[index].description, objectNames[buildingtype]);
						return false;
					}
				}
			}

			//we have enough, so now do a pass and deduct
			for (index = 0; index < MAX_RESOURCE_TYPES; index++)
			{
				int cost;

				if (bld->cost[index] > 0)
				{
					cost = ceil(bld->cost[index] * amount / (float)fullhealth);
					sl.teams[worker->base.team].resources[index] -= cost;
				}
			}
		}
	}

	//apply the repair
	building->base.health += amount;
	if (building->base.health > fullhealth)
		building->base.health = fullhealth;

	if (worker->client)
		SV_AddExperience(worker->client, EXPERIENCE_REPAIR, amount, bld->expMult);
	return true;
}


/*==========================

  SV_CancelConstruction

 ==========================*/

void	SV_CancelConstruction(int objindex)
{
	int itemType;

	serverObject_t	*obj = &sl.objects[objindex];
	itemType = obj->itemConstruction;
	obj->itemConstruction = 0;
	obj->itemConstructionAmountLeft = 0;

	SV_SendNoticeToTeam(SV_GetTeam(objindex), NOTICE_CANCEL_RESEARCH, objindex, fmt("%i", itemType));
}


/*==========================

  SV_ConstructObject

 ==========================*/

void	SV_ConstructObject(serverObject_t *builder, int objType)
{
	objectData_t	*def = GetObjectByType(objType);

	builder->itemConstruction = objType;
	builder->itemConstructionAmountLeft = sv_fastTech.integer ? 100 : def->researchTime;

	SV_SendNoticeToTeam(SV_GetTeam(builder->base.index), NOTICE_BEGIN_RESEARCH, builder->base.index, fmt("%i", objType));
}

/*==========================

  SV_ItemConstructionComplete

  Sets up new stuff when a building finishes constructing/researching/upgrading

 ==========================*/

bool	SV_ItemConstructionComplete(int objindex, int itemType)
{
	serverObject_t	*obj = &sl.objects[objindex];
	vec3_t spawnPoint, zerovec = { 0, 0, 0 };
	serverObject_t *item = NULL;
	teamInfo_t *team = &sl.teams[obj->base.team];
	int type;

	if (itemType < 1 || itemType >= MAX_OBJECT_TYPES)
		return false;

	if (Tech_GetEntry(itemType))
	{
		//A researchable item has been completed
		if (Tech_IsResearchType(itemType))				
		{
			SV_GiveUpgradeToTeam(obj->base.team, itemType);
			if (team->commander >= 0 && team->commander < MAX_CLIENTS)
				SV_AddExperience(&sl.clients[team->commander], EXPERIENCE_COMMANDER_RESEARCH, 0, GetObjectByType(itemType)->expMult);
		}
		
		//a worker unit should be spawned
		else if (IsWorkerType(itemType))
		{
			//find a free spot along the perimeter of the structure
			if (SV_GetSpawnPointFromBuilding(obj, spawnPoint, itemType))
			{
				//if that worked, let's spawn our item
				item = SV_SpawnObject(obj->base.team, itemType, spawnPoint, zerovec);						
			}
			else
			{
				cores.Console_DPrintf("Couldn't find any position on the perimeter of the %s free to hold a %s\n", 
									  GetObjectByType(obj->base.type)->description, GetObjectByType(itemType));
				
				//simply returning here will actually cause this function to get called again next frame, which is probably the behavior we want
				return false;
			}
		}

		//a building has upgraded itself
		else if (IsBuildingType(itemType) && obj->base.type != itemType)
		{
			float healthpercent = obj->base.health / (float)(MAX(obj->base.health, obj->adjustedStats.fullhealth));
			teamInfo_t *team = &sl.teams[obj->base.team];			
			bool newbase = false;
			serverObject_t	*newObj = NULL;
			vec3_t pos, angle;

			M_CopyVec3(obj->base.pos, pos);
			M_CopyVec3(obj->base.angle, angle);
			
			//are we upgrading the team's command center?
			if (obj->base.index == team->command_center)
				newbase = true;

			SV_FreeObject(obj->base.index);
			newObj = SV_SpawnObject(team->index, itemType, pos, angle);
			newObj->base.health = newObj->base.fullhealth * healthpercent;
			if (newbase)
				sl.teams[team->index].command_center = newObj->base.index;

			SV_CancelConstruction(newObj->base.index);
		}
	}

	SV_CancelConstruction(obj->base.index);

	SV_UpdateTeamResearch(team->index);

	//see if they have something in the build queue
	while ((type = SV_GetQueuedRequest(obj->base.index)) >= 0)
	{
		if (Tech_IsAvailable(type, team->research))
		{
			SV_PurchaseObject(team->index, type, obj, false);
			break;
		}
		else
		{
			SV_RemoveQueuedRequest(obj->base.index, (byte)type);
		}
	}

	SV_SendNoticeToTeam(team->index, NOTICE_UPGRADECOMPLETE, objindex, "");
	return true;
}


/*==========================

  SV_ConstructBuilding

 ==========================*/

bool	SV_ConstructBuilding(serverObject_t *worker, serverObject_t *building, int amount)
{
	int				fullhealth;
	objectData_t	*buildingData = GetObjectByType(building->base.type);

	if (!Tech_GetEntry(building->base.type))
		return false;

	if (!(building->base.flags & BASEOBJ_UNDER_CONSTRUCTION))
		return false;

	if (buildingData->linked && !building->twin)
		return false;

	//contribute to the building
	building->itemConstructionAmountLeft -= amount;
	fullhealth = building->base.fullhealth;
	building->base.health += ceil(fullhealth * (amount / (float)MAX(1,buildingData->researchTime)));
	
	if (worker->client)
		SV_AddExperience(worker->client, EXPERIENCE_BUILD, amount, buildingData->expMult);

	if (building->base.health > fullhealth)
		building->base.health = fullhealth;
	if (building->itemConstructionAmountLeft <= 0)
	{
		//announce that we're done
		SV_Phys_AddEvent(worker, EVENT_TASK_FINISHED, 0, 0);
		SV_ItemConstructionComplete(building->base.index, building->base.type);
		//SV_Phys_AddEvent(building, EVENT_BUILDING_COMPLETE, 0, 0);
		if (sl.teams[building->base.team].commander >= 0 && sl.teams[building->base.team].commander < MAX_CLIENTS)
			SV_AddExperience(&sl.clients[sl.teams[building->base.team].commander], EXPERIENCE_COMMANDER_STRUCTURE, 0, buildingData->expMult);

		if (buildingData->linked && !building->twin)
			cores.Server_SendMessage(-1, sl.teams[SV_GetTeam(building->base.index)].commander, fmt("%s %i", SERVER_COMMANDER_PLACE_LINK_MSG, building->base.index));
		return false;
	}

	return true;
}



/*==========================

  SV_DropoffResources

 ==========================*/

bool	SV_DropoffResources(serverObject_t *worker, serverObject_t *building)
{
	bool ret = false;
	int index;
	teamInfo_t *team = &sl.teams[worker->base.team];

	//safety checks
	if (!worker || !building)
		return false;

	//must be a building
	if (!IsBuildingType(building->base.type))
		return false;

	//not a valid dropoff point
	if (!GetObjectByType(building->base.type)->dropoff)
		return false;

	//still under construction
	if (building->base.flags & BASEOBJ_UNDER_CONSTRUCTION)
		return false;

	//must be same team
	if (worker->base.team != building->base.team)
		return false;

	//make the transaction
	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
	{
		if (worker->resources[index] > 0)
		{
			team->resources[index] += worker->resources[index];
			SV_Phys_AddEvent(worker, EVENT_DROPOFF, (byte)index, (byte)worker->resources[index]);
			
			//player experience
			if (worker->client)
				SV_AddExperience(worker->client, EXPERIENCE_MINE, worker->resources[index], 1.0);

			//commander experience
			if (sl.teams[worker->base.team].commander >= 0 && sl.teams[worker->base.team].commander < MAX_CLIENTS)
				SV_AddExperience(&sl.clients[sl.teams[worker->base.team].commander], EXPERIENCE_COMMANDER_GATHER, worker->resources[index], 1.0);

			ret = true;
		}
		worker->resources[index] = 0;
	}

	return ret;
}


/*==========================

  SV_FindNearestDropoffPoint

 ==========================*/

serverObject_t	*SV_FindNearestDropoffPoint(serverObject_t *obj)
{
	int n;
	serverObject_t *candidate = NULL;
	byte team = obj->base.team;
	serverObject_t *other;
	float	bestdist = FAR_AWAY;

	for (n = MAX_CLIENTS; n <= sl.lastActiveObject; n++)
	{

		other = &sl.objects[n];
		if (!other->base.active)
			continue;
		if (other->base.health <= 0)
			continue;
		if (team != other->base.team)
			continue;
		if (!IsBuildingType(other->base.type))
			continue;
		if (other->base.flags & BASEOBJ_UNDER_CONSTRUCTION)
			continue;
		if (GetObjectByType(other->base.type)->dropoff)
		{
			float d;

			d = M_GetDistanceSq(other->base.pos, obj->base.pos);

			if (d < bestdist || !candidate)
			{
				bestdist = d;
				candidate = other;
			}
		}
	}

	return candidate;
}


/*==========================

  SV_DestroyBuilding

 ==========================*/

void	SV_DestroyBuilding(serverObject_t *obj)
{
	objectData_t *def = GetObjectByType(obj->base.index);
	static bool twincall = false;

	SV_CancelConstruction(obj->base.index);

	if (GetObjectByType(obj->base.type)->commandCenter)
	{
		//don't deallocate at all, just play death animation
		SV_Phys_AddEvent(obj, EVENT_DEATH, 0, 0);		
	}
	else
	{
		SV_DeathEvent(obj, EVENT_DEATH, 0, 0, 15000, true);		
		SV_UnlinkFromWorldObject(obj);
	}

	obj->base.animState = AS_DEATH_GENERIC;
	obj->base.animState2 = 0;

	if (obj->twin && !twincall)
	{
		twincall = true;
		SV_DestroyBuilding(obj->twin);
		twincall = false;
	}

	if (!twincall && obj->base.team)
	{	
		int n = 0;
		if (!(obj->base.flags & BASEOBJ_UNDER_CONSTRUCTION))
		{
			//does the team have any of these buildings left?
			for (n = MAX_CLIENTS; n <= sl.lastActiveObject; n++)
			{			
				if (!sl.objects[n].base.active)
					continue;
				if (&sl.objects[n] == obj)
					continue;
				if (sl.objects[n].base.team != obj->base.team)
					continue;
				if (sl.objects[n].base.health <= 0)
					continue;
				if (sl.objects[n].base.flags & BASEOBJ_UNDER_CONSTRUCTION)
					continue;
				if (sl.objects[n].base.type == obj->base.type)
					break;
			}
		}

		if (n > sl.lastActiveObject)
		{
			SV_BuildingWarning(obj->base.team, obj, BWARN_DESTROYED_ALL);
		}
		else
		{
			SV_BuildingWarning(obj->base.team, obj, BWARN_DESTROYED);
		}
	}
}



/*==========================

  SV_BuildingDamaged

 ==========================*/

int		SV_BuildingDamaged(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags)
{
	static bool twincall = false;
	int ret, index;
	objectData_t *objdata = GetObjectByType(obj->base.type);

	if (obj->twin && !twincall)
	{
		twincall = true;
		SV_BuildingDamaged(obj->twin, attacker, pos, weapon, attackDamage, damageFlags);
		twincall = false;
	}	

	ret = SV_ObjectDamaged(obj, attacker, pos, weapon, attackDamage, damageFlags);
	
	if (!ret)
		return 0;

	if (objdata->isMine)
	{
		obj->base.health += ret;
		return 0;		//mines can't be damaged
	}
	if (objdata->commandCenter && sl.status != GAME_STATUS_NORMAL)
	{
		obj->base.health += ret;
		return 0;		//can't damage command center in setup mode
	}
	
	if (!(damageFlags & DAMAGE_NO_REACTION))
		SV_Phys_AddEvent(obj, EVENT_WOUNDED, SV_EncodePositionOnObject(obj, pos), (byte)MIN(255, attackDamage));	

	if (obj->base.health <= 0)
	{
		SV_DestroyBuilding(obj);

		for (index = 0; index < MAX_RESOURCE_TYPES; index++)
			SV_GiveResourceToTeam(attacker->base.team, index, objdata->killResourceReward[index]);
		if (sl.teams[attacker->base.team].commander >= 0 && sl.teams[attacker->base.team].commander < MAX_CLIENTS)
			SV_AddExperience(&sl.clients[sl.teams[attacker->base.team].commander], EXPERIENCE_COMMANDER_RAZE, 0, objdata->expMult);
	}

	if (SV_GetTeam(obj->base.index) != SV_GetTeam(attacker->base.index))
	{
		obj->enemy = attacker;
		obj->lastNewEnemyTime = sl.gametime;
	}	

	if (attacker->client)
	{		
		//a player is attacking an enemy structure, give them a small reward on every hit
		if (attacker->client->info.team != obj->base.team && obj->base.team > 0)
		{
			if (weapon)
				SV_GiveMoney(attacker->base.index, attackDamage * sv_rangedHitStructureRewardScale.value, true);
			else
				SV_GiveMoney(attacker->base.index, attackDamage * sv_hitStructureRewardScale.value, true);
			SV_AddExperience(attacker->client, EXPERIENCE_STRUCTURE_DAMAGE, attackDamage, objdata->expMult);
			attacker->client->stats.buildingDamage += ret;
			if (obj->base.health <= 0)
			{
				attacker->client->stats.buildingKills++;
				
				SV_AddExperience(attacker->client, EXPERIENCE_STRUCTURE_RAZE, 0, objdata->expMult);
				if (sv_goodieBags.integer)
					SV_SpawnGoodieBag(obj, obj->base.pos, attacker->client);
			}
		}
	}

	obj->damageAccum += attackDamage;

	//building specific warning messages
	if (sl.gametime >= obj->nextWarningTime && obj->base.health > 0)
	{					
		if (obj->base.team)
		{			
			bool warned = true;

			if (obj->base.health < objdata->fullHealth / 8)
			{
				SV_BuildingWarning(obj->base.team, obj, BWARN_NEARLY_DESTROYED);
				if (sl.teams[obj->base.team].command_center == obj->base.index)
				{
					SV_SetUrgencyLevel(URGENCY_HIGH);
					sl.urgencyResetTime = sl.gametime + sv_musicResetTime.integer;
				}
			}
			else if (obj->base.health < objdata->fullHealth / 2)
			{
				SV_BuildingWarning(obj->base.team, obj, BWARN_HALF_DESTROYED);
				if (sl.teams[obj->base.team].command_center == obj->base.index)
				{
					SV_SetUrgencyLevel(URGENCY_HIGH);
					sl.urgencyResetTime = sl.gametime + sv_musicResetTime.integer;
				}
			}
			else if (obj->damageAccum > 500)
			{
				if (GetObjectByType(attacker->base.type)->isSiegeWeapon)
				{					
					SV_BuildingWarning(obj->base.team, obj, BWARN_UNDER_SIEGE);
					if (sl.teams[obj->base.team].command_center == obj->base.index)
					{
						SV_SetUrgencyLevel(URGENCY_HIGH);
						sl.urgencyResetTime = sl.gametime + sv_musicResetTime.integer;
					}
				}
				else
				{
					SV_BuildingWarning(obj->base.team, obj, BWARN_UNDER_ATTACK);
					if (sl.teams[obj->base.team].command_center == obj->base.index)
					{
						SV_SetUrgencyLevel(URGENCY_HIGH);
						sl.urgencyResetTime = sl.gametime + sv_musicResetTime.integer;
					}

				}
				obj->damageAccum = 0;
			}
			else
				warned = false;

			if (warned)
			{
				//SV_Phys_AddEvent(obj, EVENT_BUILDING_UNDER_ATTACK, 0, 0);
				obj->nextWarningTime = sl.gametime + 20000;
			}
		}
	}		

	return ret;
}


/*==========================

  SV_CommandCenterDamaged

  if a command center gets destroyed, the game is over

  ==========================*/

int 	SV_CommandCenterDamaged(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags)
{
	int ret = SV_BuildingDamaged(obj, attacker, pos, weapon, attackDamage, damageFlags);

	if (obj->base.health <= 0)
	{
		//set the "hero" award
		if (attacker->client && !sl.hero)
		{
			if (attacker->client->info.team != obj->base.team)
				sl.hero = attacker->client;
		}
		
		if (obj->base.team)
			SV_EndGame(obj->base.team ^ 3);
	}

	return ret;
}


/*==========================

  SV_SetupCommandCenter

 ==========================*/

void	SV_SetupCommandCenter(serverObject_t *obj)
{	
	obj->damage = SV_CommandCenterDamaged;
}


/*==========================

  SV_LineOfSite

  Determine if there is a clear line from looker to target

 ==========================*/

bool	SV_LineOfSite(int looker, int target)
{
	traceinfo_t	trace;
	vec3_t		start, end;

	if (!SV_GetPosition(looker, start))
		return false;
	start[Z] += GetObjectByType(sl.objects[looker].base.type)->viewheight;
	
	if (!SV_GetPosition(target, end))
		return false;

	cores.World_TraceBoxEx(&trace, start, end, zero_vec, zero_vec, 0, looker);

	if (trace.index > -1 && trace.index != target)
		return false;

	if (trace.index == -1 && trace.fraction < 1.0)
		return false;

	return true;
}


/*==========================

  SV_BuildingAdvanceResearch

  If a building is in the process of researching/upgrading/producing, advance that here

 ==========================*/

void	SV_BuildingAdvanceResearch(int objindex)
{
	serverObject_t	*building = &sl.objects[objindex];

	//Is something being constructed?
	if (building->itemConstruction)
	{
		if (Tech_HasEntry(building->itemConstruction))
		{
			int researchTime = sv_fastTech.integer ? 100 : MAX(1,GetObjectByType(building->itemConstruction)->researchTime);

			//this is a building under construction
			if (building->itemConstruction == building->base.type)
			{
				if (GetObjectByType(building->itemConstruction)->selfBuild)
				{
					if (!building->lastBuildTime)
						building->lastBuildTime = sl.gametime;

					SV_ConstructBuilding(building, building, sl.gametime - building->lastBuildTime);
					building->lastBuildTime = sl.gametime;
				}

				building->base.flags |= BASEOBJ_UNDER_CONSTRUCTION;
				building->base.percentToComplete = ((float)building->itemConstructionAmountLeft / (float)researchTime) * 100.0;
				return;
			}
			else
			{
				building->itemConstructionAmountLeft -= sl.frame_msec;
				if (building->itemConstructionAmountLeft <= 0)
				{
					SV_ItemConstructionComplete(objindex, building->itemConstruction);
				}
				else
				{
					building->base.percentToComplete = ((float)building->itemConstructionAmountLeft / (float)researchTime) * 100.0;
					return;
				}
			}
		}
	}

	building->base.percentToComplete = 0;
	building->base.flags &= ~BASEOBJ_UNDER_CONSTRUCTION;
}


/*==========================

  SV_BuildingFrame

  Called once each frame for every building

 ==========================*/

void	SV_BuildingFrame(serverObject_t *obj)
{
	int				index, dist;
	objectData_t	*bld = GetObjectByType(obj->base.type);
	objectData_t	*wpn = &objData[GetObjectTypeByName(bld->forceInventory[0])];
	vec3_t			scanorigin, origin, pos, dif;
	bool			lockdown = false;

	//mines are indestructable, so use their health to reflect the remaining quantity
	if (bld->isMine)
		obj->base.health = obj->resources[bld->mineType];

	if (sl.status > GAME_STATUS_NORMAL)
		return;

	SV_BuildingAdvanceResearch(obj->base.index);

	//if building is invalid, return
	if (!obj->base.active || obj->base.health <= 0 || !bld->objclass || obj->itemConstructionAmountLeft)
		return;

	//check for a tech lockdown
	for (index = 0; index < MAX_STATE_SLOTS; index++)
	{
		int statenum;

		if (!obj->base.states[index])
			continue;
		
		statenum = obj->base.states[index];
		if (stateData[statenum].lockdownTech && bld->techType)
		{
			lockdown = true;
			break;
		}
	}

	//apply the buildings radius state
	if (bld->radiusState && !lockdown)
		SV_ApplyStateToRadius(obj->base.index, bld->scanRange, bld->radiusState, 100);

	//set origin
	M_CopyVec3(obj->base.pos, scanorigin);
	M_CopyVec3(scanorigin, origin);
	origin[Z] += bld->viewheight;

	if ((bld->shiftProjAccel != 0.0 || bld->shiftProjGrav != 0.0) && !lockdown)
	{
		//check radius
		for (index = MAX_CLIENTS; index <= sl.lastActiveObject; index++)
		{
			//only active objects from other teams
			if (!sl.objects[index].base.active || SV_GetTeam(index) == obj->base.team || sl.objects[index].flags & OBJECT_FLAG_NOT_A_TARGET)
				continue;

			if (!GetObjectByType(sl.objects[index].base.type)->isVulnerable)
				continue;

			//only projectiles
			if (GetObjectByType(sl.objects[index].base.type)->objclass != OBJCLASS_WEAPON /*&&
				GetObjectByType(sl.objects[index].base.type)->objclass != OBJCLASS_ITEM*/)
				continue;

			if (sl.objects[index].hasShifted)
				continue;

			M_CopyVec3(sl.objects[index].base.pos, pos);

			//get dif from potential target origin to potential attacker origin
			M_SubVec3(pos, scanorigin, dif);

			dist = M_Normalize(dif);

			if (dist <= bld->scanRange)
			{
				vec3_t	neworg;

				Traj_GetPos(&sl.objects[index].base.traj, sl.gametime, neworg);
				M_CopyVec3(neworg, sl.objects[index].base.traj.origin);
				M_MultVec3(sl.objects[index].base.traj.velocity, 0.25, sl.objects[index].base.traj.velocity);
				sl.objects[index].base.traj.acceleration += bld->shiftProjAccel;
				sl.objects[index].base.traj.gravity += bld->shiftProjGrav;
				sl.objects[index].base.traj.startTime = sl.gametime;
				sl.objects[index].hasShifted = true;
			}
		}
	}

	//if building is not set up for an attack, return
	if (!bld->refreshTime || !bld->scanRange || wpn->objclass != OBJCLASS_WEAPON)
		return;
	
	if (obj->nextDecisionTime > sl.gametime)
		return;

	//have a target already
	if (obj->enemy)
	{
		bool clearenemy = false;

		//this is just silly...
		if (obj->enemy == obj)
			clearenemy = true;

		if (SV_IsCloaked(obj, obj->enemy))
			clearenemy = true;

		//since it always neutralizes it's target, it should never have one carry over
		if (bld->targetProjectiles)
			clearenemy = true;

		//check a few things first...
		if (obj->enemy->base.health <= 0 || !obj->enemy->base.active || obj->enemy->base.index >= MAX_OBJECTS)
			clearenemy = true;

		if (clearenemy)
			obj->enemy = NULL;
	}

	//make sure it didn't move out of range
	if (obj->enemy && sl.gametime - obj->lastNewEnemyTime > 5000)	//if you attacked, you are the towers enemy for 5 seconds
	{
		//make sure it's still in range
		index = obj->enemy->base.index;
		
		//get dif from potential target origin to potential attacker origin
		SV_GetPosition(index, pos);
		M_SubVec3(pos, scanorigin, dif);

		//this will leave us set up to fire if target is valid, otherwise find a new one
		if (M_Normalize(dif) > bld->scanRange)
			obj->enemy = NULL;
	}
	
	//try to find a new target
	if (!obj->enemy)
	{
		int	newobj = -1, closest = bld->scanRange;

		//check radius
		for (index = 0; index <= sl.lastActiveObject; index++)
		{
			//only active objects from other teams
			if (!sl.objects[index].base.active || SV_GetTeam(index) == obj->base.team || sl.objects[index].flags & OBJECT_FLAG_NOT_A_TARGET)
				continue;

			//only check health for units buildings
			if ((IsCharacterType(sl.objects[index].base.type) || IsBuildingType(sl.objects[index].base.type)) && sl.objects[index].base.health <= 0)
				continue;

			if (!GetObjectByType(sl.objects[index].base.type)->isVulnerable)
				continue;

			//ignore projectiles (unless specifically targeting them)
			if (GetObjectByType(sl.objects[index].base.type)->objclass == OBJCLASS_WEAPON)
			{
				if (!bld->targetProjectiles)
					continue;
			}
			else if (bld->targetProjectiles)
				continue;

			//ignore items
			if(GetObjectByType(sl.objects[index].base.type)->objclass == OBJCLASS_ITEM)
				continue;

			//ignore neutral things (that aren't units)
			if (sl.objects[index].base.team == 0)// && !IsUnitType(sl.objects[index].base.type))
				continue;

			if (SV_IsCloaked(obj, &sl.objects[index]))
				continue;

			SV_GetPosition(index, pos);

			//get dif from potential target origin to potential attacker origin
			M_SubVec3(pos, scanorigin, dif);

			dist = M_Normalize(dif);
			if (dist > closest)
				continue;
			else
			{
				closest = dist;
				newobj = index;
			}
		}

		if (newobj != -1)
			obj->enemy = &sl.objects[newobj];
		else
			obj->enemy = NULL;
	}

	//fire at the target
	if (obj->enemy)
	{
		float	t;
		float	zdif;
		vec3_t	dir, vel;

		index = obj->enemy->base.index;
		//get possition and velocity of target
		if (index < MAX_CLIENTS)
		{
			M_CopyVec3(sl.clients[index].ps.pos, pos);
			M_CopyVec2(sl.clients[index].ps.velocity, dir);
		}
		else
		{
			M_CopyVec3(sl.objects[index].base.pos, pos);
			M_CopyVec2(sl.objects[index].velocity, dir);
		}
		dir[Z] = 0;

		//aim for the center
		pos[Z] += (sl.objects[index].base.bmax[Z] - sl.objects[index].base.bmin[Z]) / 2;

		//get distance from target origin to attacker origin
		M_SubVec2(pos, origin, dif);
		dif[Z] = 0;
		dist = M_Normalize(dif);

		M_MultVec3(dif, GETRANGE(wpn->velocity), vel);
		
		//calculate time based on targets current possition and projectiles specified velocity
		t = dist / MAX(1,(M_GetVec2Length(vel)));
		if (t <= 0)
			t = 1;

		//shift target possition by time
		M_MultVec2(dir, t, dir);
		M_AddVec3(pos, dir, pos);

		//fudge the horizontal velocity to impact the predicted pos
		M_SubVec2(pos, origin, dif);
		dif[Z] = 0;
		dist = M_Normalize(dif);
		M_MultVec3(dif, (dist / t), vel);
		
		//give the projectile a verticle velocity to compensate for gravity
		zdif = pos[Z] - origin[Z];
		vel[Z] = (zdif + (((DEFAULT_GRAVITY * PHYSICS_SCALE * wpn->gravity) * pow(t, 2)) * 0.5)) / t;

		M_Normalize(vel);
		if (GetObjectByType(obj->enemy->base.type)->objclass == OBJCLASS_WEAPON)
			SV_WeaponFire(obj, vel, GetObjectTypeByName(bld->forceInventory[0]), obj->enemy->base.pos);
		else
			SV_WeaponFire(obj, vel, GetObjectTypeByName(bld->forceInventory[0]), NULL);
		obj->nextDecisionTime = sl.gametime + wpn->refreshTime;

		SV_Phys_AddEvent(obj, EVENT_WEAPON_FIRE, 0, (byte)GetObjectTypeByName(bld->forceInventory[0]));

		//if it's a projectile, just eliminate it if there is a valid line of sight
		//(is this cheating?)
		if (GetObjectByType(obj->enemy->base.type)->objclass == OBJCLASS_WEAPON &&
			SV_LineOfSite(obj->base.index, obj->enemy->base.index) && !lockdown)
		{
			if (GetObjectByType(obj->enemy->base.type)->isVulnerable)
			{
				//'damage' the weapons fuse, so it will be destroyed
				sl.objects[obj->enemy->base.index].expireTime -= GetObjectByName(bld->forceInventory[0])->damage;
				SV_Projectile_Frame(obj->enemy);
			}
		}
	}
}


/*==========================

  SV_BuildingCanFit

  ==========================*/

bool	SV_BuildingCanFit(int structure_type, vec3_t pos, vec3_t angle, int team, char* reason)
{
	vec3_t bmin,bmax;
	vec3_t orig_bmin, orig_bmax;
	vec3_t navtest_bmin, navtest_bmax;
	vec3_t tracetest_bmin, tracetest_bmax;

	vec3_t axis[3] = { { 1,0,0 }, { 0,1,0 }, { 0,0,1 } };
	objectData_t *bld = GetObjectByType(structure_type);

	//work out the AABB of the rotated structure in world coords
	cores.Res_GetModelSurfaceBounds(cores.Res_LoadModel(GetObjectByType(structure_type)->model), orig_bmin, orig_bmax);
	
	M_MultVec3(orig_bmin, STRUCTURE_SCALE, orig_bmin);
	M_MultVec3(orig_bmax, STRUCTURE_SCALE, orig_bmax);
	M_GetAxis(angle[0], angle[1], angle[2], axis);

	M_MultVec3(orig_bmin, sv_placeBuildingTestScale.value, navtest_bmin);
	M_MultVec3(orig_bmax, sv_placeBuildingTestScale.value, navtest_bmax);
	M_TransformPoint(navtest_bmin, zero_vec, axis, navtest_bmin);
	M_TransformPoint(navtest_bmax, zero_vec, axis, navtest_bmax);
	M_TransformPoint(orig_bmin, zero_vec, axis, tracetest_bmin);
	M_TransformPoint(orig_bmax, zero_vec, axis, tracetest_bmax);

	bmin[2] = orig_bmin[2];
	bmax[2] = orig_bmax[2];
	bmin[0] = MIN(navtest_bmin[0], navtest_bmax[0]);
	bmin[1] = MIN(navtest_bmin[1], navtest_bmax[1]);
	bmax[0] = MAX(navtest_bmin[0], navtest_bmax[0]);
	bmax[1] = MAX(navtest_bmin[1], navtest_bmax[1]);

	M_AddVec3(pos, bmin, navtest_bmin);
	M_AddVec3(pos, bmax, navtest_bmax);

	bmin[0] = MIN(tracetest_bmin[0], tracetest_bmax[0]);
	bmin[1] = MIN(tracetest_bmin[1], tracetest_bmax[1]);
	bmax[0] = MAX(tracetest_bmin[0], tracetest_bmax[0]);
	bmax[1] = MAX(tracetest_bmin[1], tracetest_bmax[1]);

	M_CopyVec3(bmin, tracetest_bmin);
	M_CopyVec3(bmax, tracetest_bmax);

	{
		traceinfo_t trace;		
		/*
		cores.World_SampleGround(pos[X], pos[Y], &pi);
		basez = pi.z;
		*/

		
		cores.World_TraceBox(&trace, pos, pos, tracetest_bmin, tracetest_bmax, SURF_FOLIAGE | SURF_NOT_SOLID | SURF_TERRAIN | SURF_STATIC | SURF_COMPLEX);
		if (trace.fraction < 1)
		{
			strcpy(reason, "Building is intersecting other objects!");
			return false;
		}

		/*
		cores.World_SampleGround(bminw[X], bminw[Y], &pi);
		z1 = pi.z;
		cores.World_SampleGround(bminw[X], bmaxw[Y], &pi);
		z2 = pi.z;
		cores.World_SampleGround(bmaxw[X], bminw[Y], &pi);
		z3 = pi.z;
		cores.World_SampleGround(bmaxw[X], bmaxw[Y], &pi);
		z4 = pi.z;
		if (ABS(basez - z1) > sv_placementLeniency.value
			|| ABS(basez - z2) > sv_placementLeniency.value
			|| ABS(basez - z3) > sv_placementLeniency.value
			|| ABS(basez - z4) > sv_placementLeniency.value)
		{
			strcpy(reason, "Building is on uneven ground!");
			return false;
		}
*/
//		return true;
	}

//	if ( sv_placeBuildingTest.value )
	{

		if ( !cores.NavRep_CanPlaceBuilding(navtest_bmin, navtest_bmax) )
		{
			strcpy(reason, "Building does not fit here!");
			return false;
		}
	}

	if (bld->needsAnchor)
	{
		int n;
		serverObject_t *other;
		float	d;

		for (n = MAX_CLIENTS; n <= sl.lastActiveObject; n++)
		{
			other = &sl.objects[n];
			if (!IsBuildingType(other->base.type))
				continue;
			if (!other->base.active)
				continue;
			if (other->base.health <= 0)
				continue;
			if (team != other->base.team)
				continue;
			if (other->base.flags & BASEOBJ_UNDER_CONSTRUCTION)
				continue;
			if (objData[other->base.type].anchorRange == 0)
				continue;

			d = M_GetDistanceSq(other->base.pos, pos);
			if (d <= (objData[other->base.type].anchorRange * objData[other->base.type].anchorRange))
				break;
		}

		if (n > sl.lastActiveObject)
		{
			strcpy(reason, "Building is too far from another building!");
			return false;
		}
	}

	return true;
}


/*==========================

  SV_SpawnBuilding

  ==========================*/

serverObject_t	*SV_SpawnBuilding(byte structure_type, int team, vec3_t pos, vec3_t angle)
{
	serverObject_t *obj;
	objectData_t	*bld = GetObjectByType(structure_type);
		
	obj = SV_AllocObject(structure_type, team);	

	SV_FillInBaseStats(obj);
	obj->base.fullhealth = obj->adjustedStats.fullhealth;

	obj->damage = SV_BuildingDamaged;

	if (bld->commandCenter)
	{
		SV_SetupCommandCenter(obj);		
	}

	if (team == 0 || bld->commandCenter)
		obj->base.health = obj->base.fullhealth;
	else
		obj->base.health = obj->base.fullhealth * sv_buildingInitialHealthPercent.value;		

	if (bld->isMine)
	{
		obj->resources[bld->mineType] = bld->mineAmount;
	}

	obj->frame = SV_BuildingFrame;
	obj->nextDecisionTime = sl.gametime + 5000;	//short delay before activating
	obj->enemy = NULL;
	
	M_CopyVec3(pos, obj->base.pos);
	M_CopyVec3(angle, obj->base.angle);

	//all buildings get linked to internal static world objects so we can use more complex collision surfaces
	SV_LinkToWorldObject(obj);

	SV_Phys_AddEvent(obj, EVENT_SPAWN, 0, 0);

	//sl.teams[team].command_center = obj->base.index;
	SV_RefreshTeamInfo(team);

	return obj;
}


/*==========================

  SV_InitBuildings

 ==========================*/

void	SV_InitBuildings()
{
	int n = 0;

	cores.Cvar_Register(&sv_claimableBuildings);
	cores.Cvar_Register(&sv_buildingDmgScale);
	cores.Cvar_Register(&sv_minClaimInterval);
	cores.Cvar_Register(&sv_placeBuildingTestScale);
	cores.Cvar_Register(&sv_musicResetTime);

	while(buildingWarnings[n][0])
	{
		n++;
	}

	if (n != NUM_BUILDING_WARNINGS)
		core.Game_Error("Building warning enum doesn't match string list\n");
}
