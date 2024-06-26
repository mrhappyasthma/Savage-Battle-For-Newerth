/*
 * (C) 2002 S2 Games
 *
 * sv_objects.c
 */

#include "server_game.h"

extern cvar_t p_gravity;
extern cvar_t sv_hitStructureRewardScale;
extern cvar_t sv_buildingDmgScale;

extern cvar_t sv_debug;

extern byte	numPeonNames;

extern void SV_AI_Destroy(struct ai_s* ai);

/*==========================

  SV_AllocObject

  Find a free baseobject and initialize it

 ==========================*/

serverObject_t *SV_AllocObject(byte object_type, int team)
{
  	int obj_num = MAX_CLIENTS;
	serverObject_t *obj;

	if (!GetObjectByType(object_type)->objclass)
	{
		//core.Game_Error("Invalid type given to SV_AllocObject\n");
		cores.Console_Printf("WARNING: Invalid type given to SV_AllocObject\n");
	}

	while (obj_num < MAX_OBJECTS && sl.objects[obj_num].base.active)
	{
		obj_num++;
	}

	if (obj_num >= MAX_OBJECTS)
		core.Game_Error("Server ran out of objects\n");

	if (obj_num > sl.lastActiveObject)
		sl.lastActiveObject = obj_num;
	
	obj = &sl.objects[obj_num];
	memset(obj, 0, sizeof(serverObject_t));
	obj->base.index = obj_num;
	obj->base.active = true;
	obj->base.team = team;
	obj->base.type = object_type;
	SetObjectBounds(&obj->base);
			
	obj->ai = NULL;
	
	return obj;
}


/*==========================

  SV_FillInBaseStats

  Sets the initial state of a characters adjustable stats

 ==========================*/

void	SV_FillInBaseStats(serverObject_t *obj)
{
	objectData_t	*baseData = GetObjectByType(obj->base.type);

	if (!baseData)
		return;

	obj->adjustedStats.buildRate =			baseData->buildRate;
	obj->adjustedStats.repairRate =			baseData->repairRate;
	obj->adjustedStats.maxCarryBonus =		0;
	if (baseData->isMine)
		obj->adjustedStats.fullhealth =		baseData->mineAmount;
	else
		obj->adjustedStats.fullhealth =		baseData->fullHealth;
	obj->adjustedStats.meleeDamageBonus =	0;
	obj->adjustedStats.meleeRangeBonus =	0;
	obj->adjustedStats.maxStamina =			baseData->maxStamina;
	obj->adjustedStats.blockPower =			baseData->blockPower;
	obj->adjustedStats.bldPierce =			baseData->bldPierce;
	obj->adjustedStats.unitPierce =			baseData->unitPierce;
	obj->adjustedStats.siegePierce =		baseData->siegePierce;
}


/*==========================

  SV_SpawnObject

  spawns an object and performs all necessary setup for the object type

  only valid for buildings, pickup items, and AI units / NPCs

 ==========================*/

serverObject_t *SV_SpawnObject(int team, int object_type, vec3_t pos, vec3_t angle)
{
	serverObject_t *obj = NULL;
	objectData_t *cfg = GetObjectByType(object_type);

	if (object_type < 0 || object_type >= MAX_OBJECT_TYPES || !cfg->objclass)
	{
		core.Game_Error(fmt("SV_SpawnObject: invalid object type %i passed to SV_SpawnObject\n", object_type));
	}

	switch(cfg->objclass)
	{
		case OBJCLASS_BUILDING:
			obj = SV_SpawnBuilding((byte)object_type, team, pos, angle);
			break;
		case OBJCLASS_ITEM:
			obj = SV_SpawnPickupItem((byte)object_type, team, pos, angle);
			break;
		case OBJCLASS_UNIT:							
			obj = SV_SpawnAIUnit((byte)object_type, team, pos, angle);			
			break;
		default:
			core.Game_Error("SV_SpawnObject: bad objclass\n");
			break;
	}

	obj->twin = NULL;
	//notify the core engine that we're spawning a new object
	cores.Server_SpawnObject(obj, obj->base.index);

	return obj;
}


/*==========================

  SV_FreeObject

 ==========================*/

void    SV_FreeObject(int obj_num)
{
	//cores.Console_Printf("SV_FreeObject(%i)\n", obj_num);

	if ( sl.objects[obj_num].ai )
	{
		SV_AI_Destroy(sl.objects[obj_num].ai);
		sl.objects[obj_num].ai = NULL;
	}

	//free the worldobject that represented us, if any
	cores.WO_DeleteObject(obj_num);

	//unlink from collision system if we were linked
	cores.World_UnlinkObject(&sl.objects[obj_num].base);

	cores.Server_FreeObject(&sl.objects[obj_num]);

	sl.objects[obj_num].base.active = false;
	if (obj_num == sl.lastActiveObject)
	{
		while (sl.objects[sl.lastActiveObject].base.active == false)
		{
			--sl.lastActiveObject;
		}
	}

	cores.Console_DPrintf("FreeObject: %i\n", obj_num);
}

void	SV_FreeObjectFrame(serverObject_t *obj)
{
	SV_FreeObject(obj->base.index);
}

int		SV_FindClosestObjects(int *objectTypes, int num_objectTypes, int object, int team, float max_dist, float *actual_dist)
{
	return cores.WOG_FindClosestObjects(objectTypes, num_objectTypes, 0.0f, max_dist, sl.objects[object].base.pos, team, actual_dist);
}

int     SV_FindClosestObject(int objectType, int object, int team, float max_dist, float *actual_dist)
{
	return SV_FindClosestObjects(&objectType, 1, object, team, max_dist, actual_dist);
}

int		SV_FindClosestObjectsToPos(int *objectTypes, int num_objectTypes, vec3_t pos, int team, float max_dist, float *actual_dist)
{
	return cores.WOG_FindClosestObjects(objectTypes, num_objectTypes, 0.0f, max_dist, pos, team, actual_dist);
}

int     SV_FindClosestObjectToPos(int objectType, vec3_t pos, int team, float max_dist, float *actual_dist)
{
	return SV_FindClosestObjectsToPos(&objectType, 1, pos, team, max_dist, actual_dist);
}

int		SV_FindObjectsInRadius(int *objectTypes, int num_objectTypes, float radius, vec3_t pos, int *objects, int numObjects)
{
	return cores.WOG_FindObjectsInRadius(objectTypes, num_objectTypes, radius, pos, objects, numObjects);
}


#define SIDE_BOTTOM 3
#define SIDE_RIGHT 	2
#define SIDE_LEFT 	1
#define SIDE_TOP 	0

void 	SV_GetBoxEdgeEndpoints(const vec3_t pos, const vec3_t axis[3], const vec3_t bmin, const vec3_t bmax, byte side, vec3_t startPos, vec3_t endPos)
{		
	switch (side)
	{
		case SIDE_RIGHT:
			M_SetVec3(startPos, bmax[X], bmax[Y], bmin[Z]);
			M_SetVec3(endPos, 	bmax[X], bmin[Y], bmin[Z]);
			break;
		case SIDE_LEFT:
			M_SetVec3(startPos, bmin[X], bmin[Y], bmin[Z]);
			M_SetVec3(endPos, 	bmin[X], bmax[Y], bmin[Z]);
			break;
		case SIDE_BOTTOM:
			M_SetVec3(startPos, bmin[X], bmin[Y], bmin[Z]);
			M_SetVec3(endPos, 	bmax[X], bmin[Y], bmin[Z]);
			break;
		case SIDE_TOP:
			M_SetVec3(startPos, bmax[X], bmax[Y], bmin[Z]);
			M_SetVec3(endPos, 	bmin[X], bmax[Y], bmin[Z]);
			break;
	}

	//rotate the positions
	M_TransformPoint(startPos, pos, axis, startPos);
	M_TransformPoint(endPos, pos, axis, endPos);
}

//walks around the perimeter of a structure and finds a spot large enough to hold the specified itemType
// instead of taking a serverObject, we pass in the data types, so this can be used by peons calculating where to 
// stand to build a building (where the building doesn't exist yet)
bool	SV_GetSpawnPointAroundObject(byte structureType, vec3_t structurePos, vec3_t structureAngle, int itemType, vec3_t spawnPoint)
{
	traceinfo_t trace;
	float sideProgress;
	byte side;
	vec3_t bmin, bmax, structBmin, structBmax, startPos, endPos;
	vec3_t axis[3] = { { 1,0,0 }, { 0,1,0 }, { 0,0,1 } };
	
	if (!IsUnitType(itemType))
		return false;

	M_CopyVec3(objData[itemType].bmin, bmin);
	M_CopyVec3(objData[itemType].bmax, bmax);
	
	
	
	if (IsBuildingType(structureType))
	{
		cores.Res_GetModelSurfaceBounds(cores.Res_LoadModel(GetObjectByType(structureType)->model), structBmin, structBmax);
		M_MultVec3(structBmin, STRUCTURE_SCALE, structBmin);
		M_MultVec3(structBmax, STRUCTURE_SCALE, structBmax);
		//get rotation axis
		//buildings must use the proper axis because their collision surfaces are complex (not just AABBs)
		M_GetAxis(structureAngle[0], structureAngle[1], structureAngle[2], axis);
	}
	else
	{
		M_CopyVec3(objData[structureType].bmin, structBmin);
		M_CopyVec3(objData[structureType].bmax, structBmax);
	}
	//expand the bounds to account for object size plus some fudge
	structBmin[0] -= (bmax[0]-bmin[0]) + 20;
	structBmax[0] += (bmax[0]-bmin[0]) + 20;
	structBmin[1] -= (bmax[1]-bmin[1]) + 20;
	structBmax[1] += (bmax[1]-bmin[1]) + 20;
	
	for (side = 0; side < 4; side++)
	{
		//get the start and end points for this side of the struct
		SV_GetBoxEdgeEndpoints(structurePos, (const vec3_t *)axis, structBmin, structBmax, side, startPos, endPos);
				
		for (sideProgress = 1; sideProgress > 0; sideProgress -= ITEM_SPAWNPOINT_SEARCH_STRIDE)
		{
			//lerp along this edge to get our test point
			spawnPoint[X] = LERP(sideProgress, startPos[X], endPos[X]);
			spawnPoint[Y] = LERP(sideProgress, startPos[Y], endPos[Y]);
			//make sure we spawn at least as high as the terrain is at this point
			spawnPoint[Z] = MAX(cores.World_CalcMaxZ(spawnPoint[X] + bmin[X], spawnPoint[Y] + bmin[Y], spawnPoint[X] + bmax[X], spawnPoint[Y] + bmax[Y]) + 10, structurePos[2] + 10);
			
			cores.World_TraceBox(&trace, spawnPoint, spawnPoint, bmin, bmax, TRACE_PLAYER_MOVEMENT);
			if (trace.fraction >= 1)
			{
				vec3_t spawnPointDown;
				spawnPointDown[X] = spawnPoint[X];
				spawnPointDown[Y] = spawnPoint[Y];
				spawnPointDown[Z] = spawnPoint[Z] - 1000;

				cores.World_TraceBox(&trace, spawnPoint, spawnPointDown, bmin, bmax, TRACE_PLAYER_MOVEMENT);
				if ((trace.fraction >= 1) || (trace.objectType == STATIC_OBJECT))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool	SV_TestRandomPointAroundObject(byte structureType, vec3_t structurePos, vec3_t structureAngle, byte objectTypeAroundStructure, int side, float sideProgress, vec3_t spawnPoint)
{
	traceinfo_t trace;
	vec3_t bmin, bmax, structBmin, structBmax, startPos, endPos;
	vec3_t axis[3] = { { 1,0,0 }, { 0,1,0 }, { 0,0,1 } };
	
	M_CopyVec3(objData[objectTypeAroundStructure].bmin, bmin);
	M_CopyVec3(objData[objectTypeAroundStructure].bmax, bmax);
	
	cores.Res_GetModelSurfaceBounds(cores.Res_LoadModel(GetObjectByType(structureType)->model), structBmin, structBmax);
	
	if (IsBuildingType(structureType))
	{
		M_MultVec3(structBmin, STRUCTURE_SCALE, structBmin);
		M_MultVec3(structBmax, STRUCTURE_SCALE, structBmax);
		//get rotation axis
		//buildings must use the proper axis because their collision surfaces are complex (not just AABBs)
		M_GetAxis(structureAngle[0], structureAngle[1], structureAngle[2], axis);
	}
	//expand the bounds to account for object size
	structBmin[0] -= (bmax[0]-bmin[0]);
	structBmax[0] += (bmax[0]-bmin[0]);
	structBmin[1] -= (bmax[1]-bmin[1]);
	structBmax[1] += (bmax[1]-bmin[1]);

	//get the start and end points for this side of the struct
	SV_GetBoxEdgeEndpoints(structurePos, (const vec3_t *)axis, structBmin, structBmax, (byte)side, startPos, endPos);
				
	spawnPoint[X] = LERP(sideProgress, startPos[X], endPos[X]);
	spawnPoint[Y] = LERP(sideProgress, startPos[Y], endPos[Y]);
	//get the max height of the terrain here so we won't spawn inside it
	spawnPoint[Z] = cores.World_CalcMaxZ(spawnPoint[X] + bmin[X], spawnPoint[Y] + bmin[Y], spawnPoint[X] + bmax[X], spawnPoint[Y] + bmax[Y]) + 10;
			
	cores.World_TraceBox(&trace, spawnPoint, spawnPoint, bmin, bmax, 0);
	if (trace.fraction >= 1)
		return true;

	return false;
}

bool	SV_GetRandomPointAroundObject(byte structureType, vec3_t structurePos, vec3_t structureAngle, byte objectTypeAroundStructure, vec3_t spawnPoint)
{
	int side, n;
	float sideProgress;

	n=0;
	while (n < 10)
	{
		side = M_Randnum(0,3);
		sideProgress = M_Randnum(0,1);
		if (SV_TestRandomPointAroundObject(structureType, structurePos, structureAngle, objectTypeAroundStructure, side, sideProgress, spawnPoint))
			return true;
		n++;
	}
	return false;
}

void	SV_AdvanceObject(serverObject_t *obj)
{
	M_CopyVec3(obj->base.pos, obj->oldpos);

	//call the objects frame function
	if (obj->frame)
		obj->frame(obj);

	if (obj->base.health < 0)
		obj->base.health = 0;
	else if (obj->base.health > obj->base.fullhealth)
		obj->base.health = obj->base.fullhealth;
}

bool	SV_UnderConstruction(serverObject_t *structure)
{
	if (structure->itemConstruction == structure->base.type)
		return true;
	return false;
}



/*==========================

  SV_ProcessStates

  Updates the states of all objects, removing those that have expired

 ==========================*/

void	SV_ProcessStates()
{
	int objindex, statenum;

	for (objindex = 0; objindex <= sl.lastActiveObject; objindex++)
	{
		serverObject_t	*obj = &sl.objects[objindex];

		if (!obj->base.active)
			continue;

		for (statenum = 0; statenum < MAX_STATE_SLOTS; statenum++)
		{
			if (!obj->base.states[statenum])
				continue;

			//negative clock indicates an indefinate effect
			if (obj->stateExpireTimes[statenum] < 0)
				continue;

			//anything else gets freed if the time is up
			if (obj->stateExpireTimes[statenum] <= sl.gametime)
			{
				obj->base.states[statenum] = 0;
				obj->stateExpireTimes[statenum] = 0;
			}
			//if the state is still active, apply it's effects
			else
			{
				stateData_t	*state = &stateData[obj->base.states[statenum]];

				if (state->radiusState)
					SV_ApplyStateToRadius(objindex, state->radius, state->radiusState, 100);

				if (state->damage && state->damageFrequency && 
					sl.gametime - obj->stateDamageTimes[statenum] > state->damageFrequency)
				{
					vec3_t	pos;

					SV_GetPosition(objindex, pos);
					SV_DamageTarget(&sl.objects[obj->stateInflictors[statenum]], obj, pos, -statenum, state->damage, state->damageFlags);
					obj->stateDamageTimes[statenum] = sl.gametime;
				}

				if (state->radiusDamage && state->radiusDamageFreq &&
					sl.gametime - obj->stateRadiusDamageTime[statenum] > state->radiusDamageFreq)
				{
					vec3_t	pos;

					SV_GetPosition(objindex, pos);
					SV_DamageRadius(obj, pos, 0, state->radius, state->radiusDamage, state->damageFlags);
					obj->stateRadiusDamageTime[statenum] = sl.gametime;
				}
			}
		}

		//synch the clients with any changes that were just made
		if (objindex < MAX_CLIENTS)
		{
			memcpy(sl.clients[objindex].ps.states, obj->base.states, sizeof(sl.clients[objindex].ps.states));
			memcpy(sl.clients[objindex].ps.stateExpireTimes, obj->stateExpireTimes, sizeof(sl.clients[objindex].ps.stateExpireTimes));
		}
	}
}

	
/*==========================

  SV_AdvanceObjects

  This is the main function used for processing non-client objects every frame

 ==========================*/

void 	SV_AdvanceObjects()
{
	int i;
	
	if (sl.status == GAME_STATUS_ENDED)
		return;

	for (i=1; i<MAX_TEAMS; i++)
	{
		sl.teams[i].outpostList = NULL;
	}

	SV_ProcessStates();

	for (i = MAX_CLIENTS; i <= sl.lastActiveObject; i++)
	{
		serverObject_t *object = &sl.objects[i];
		objectData_t	*bld = GetObjectByType(object->base.type);

		if (!object->base.active)
			continue;

		SV_UnitHeal(i);
		SV_AdvanceObject(object);

		//store a list of outposts for each team
		if (bld->spawnFrom && (object->base.index != sl.teams[object->base.team].command_center))
		{
			if (object->base.active && object->base.health > 0 && !(object->base.flags & BASEOBJ_UNDER_CONSTRUCTION) && object->base.team)
			{
				object->nextOutpost = sl.teams[object->base.team].outpostList;
				sl.teams[object->base.team].outpostList = object;
			}
		}
	}

}



int 	SV_ObjectDamaged(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags)
{
	//see if they're already dead
	if (obj->base.health <= 0)
	{
		obj->base.health = 0;
//		obj->base.moveState = AS_DEAD;	//just to make sure
		return 0;
	}

	if (attackDamage<=0)
		return attackDamage;

	//fixme: hack for E3
	if (IsBuildingType(obj->base.type))
	{	

		attackDamage *= sv_buildingDmgScale.value;
		if (attackDamage <= 0)
			attackDamage = 1;
	}

	//check damage flags
	if (obj == attacker)
	{
		if (damageFlags & DAMAGE_SELF_NONE)
			attackDamage = 0;
		else if (damageFlags & DAMAGE_SELF_HALF)
			attackDamage *= 0.5;
	}

	//todo: check for defense powerups, etc
	obj->base.health -= attackDamage;
	
	if (GetObjectByType(weapon)->transferHealth > 0)
	{
		int	xfer;
		
		if (obj->base.health < 0)
			attackDamage += obj->base.health;

		xfer = attackDamage * GetObjectByType(weapon)->transferHealth;

		if (attacker->base.index < MAX_CLIENTS)
		{
			attacker->client->ps.health += xfer;
			if (attacker->client->ps.health > attacker->base.fullhealth)
				attacker->client->ps.health = attacker->base.fullhealth;
			attacker->base.health = attacker->client->ps.health;
		}
		else
		{
			attacker->base.health += xfer;
			if (attacker->base.health > attacker->base.fullhealth)
				attacker->base.health = attacker->base.fullhealth;
		}
	}

	//stamina regain effect
	if (GetObjectByType(weapon)->staminaDrain > 0)
	{
		int xfer;

		if (obj->base.health < 0)
			attackDamage += obj->base.health;

		xfer = attackDamage * GetObjectByType(weapon)->staminaDrain;

		if (attacker->base.index < MAX_CLIENTS)
		{
			attacker->client->ps.stamina += attackDamage * GetObjectByType(weapon)->staminaDrain;
			if (attacker->client->ps.stamina > attacker->adjustedStats.maxStamina)
				attacker->client->ps.stamina = attacker->adjustedStats.maxStamina;
		}
	}

	//FIXME: this shouldn't be here
	if (obj->base.health <= 0)
	{
		obj->base.health = 0;
		//FIXME: hack (cuz this shouldn't be here)
		if (GetObjectByType(obj->base.type)->objclass != OBJCLASS_BUILDING)
		{
			SV_DeathEvent(obj, EVENT_DEATH, 0, 0, 15000, true);
			SV_UnlinkFromWorldObject(obj);
			
			obj->base.animState = AS_DEATH_GENERIC;
			obj->base.animState2 = 0;
		}
	}

	if (attacker->client)
	{
		Phys_AddEvent(&attacker->client->ps, EVENT_WEAPON_HIT, 0, 0);	
	}

	return attackDamage;
}


/*==========================

  SV_CommanderPlaceBuilding

  ==========================*/

serverObject_t *SV_CommanderPlaceBuilding(client_t *client, int type, vec3_t pos, vec3_t angle, bool ignorecost)
{
	char			reason[256];
	int				team = client->info.team;
	serverObject_t	*building;

	if (!IsBuildingType(type))
		return NULL;

	if (!SV_ClientIsCommander(client->index))
		return NULL;

	if (!SV_BuildingCanFit(type, pos, angle, SV_GetTeam(client->index), reason))
		return NULL;
	
	if (!SV_PurchaseObject(team, type, NULL, ignorecost))
		return NULL;

	building = SV_SpawnObject(team, type, pos, angle);
	building->base.flags |= BASEOBJ_UNDER_CONSTRUCTION;
	SV_ConstructObject(building, type);
	return building;
}
	

/*==========================

  SV_LinkToWorldObject

  this function is called for static objects with complex surfaces (like structures), that we want to marry to a world object

 ==========================*/

bool SV_LinkToWorldObject(serverObject_t *obj)
{
	objectPosition_t objpos;
	char *model, *skin;	

	//fix up serverobject coordinate to integer aligned so there are no prediction errors on the client
	obj->base.pos[0] = (int)obj->base.pos[0];
	obj->base.pos[1] = (int)obj->base.pos[1];
	obj->base.pos[2] = (int)obj->base.pos[2];

	//fix up serverobject angles to prevent angular prediction errors
	obj->base.angle[0] = BYTE2ANGLE(ANGLE2BYTE(obj->base.angle[0]));
	obj->base.angle[1] = BYTE2ANGLE(ANGLE2BYTE(obj->base.angle[1]));
	obj->base.angle[2] = BYTE2ANGLE(ANGLE2BYTE(obj->base.angle[2]));

	M_CopyVec3(obj->base.pos, objpos.position);
	M_CopyVec3(obj->base.angle, objpos.rotation);

	objpos.scale = STRUCTURE_SCALE;				//hack to scale down structures

	obj->base.flags |= BASEOBJ_NO_LINK | BASEOBJ_WORLDOBJ_REPRESENTS;

	model = GetObjectByType(obj->base.type)->model;
	skin = GetObjectByType(obj->base.type)->skin;

	if (!cores.WO_CreateObject(-1, model, skin, &objpos, obj->base.index))
		return false;

	return true;
}


/*==========================

  SV_UnlinkFromWorldObject

 ==========================*/

void SV_UnlinkFromWorldObject(serverObject_t *obj)
{
	obj->base.flags &= ~BASEOBJ_WORLDOBJ_REPRESENTS;

	cores.WO_DeleteObject(obj->base.index);
}
