#include "server_game.h"
#include "sv_aiutil.h"
#include "sv_ai.h"
#include "sv_aigoal.h"
#include "sv_ainav.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(ai_t);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

float SV_AI_GetMeleeRange(ai_t* ai)
{	
	serverObject_t* obj = ai->aics.obj;

	int nextattack = obj->base.animState2 + 1;

	if (!IS_MELEE_ATTACK(nextattack))
		nextattack = AS_MELEE_1;

	return objData[obj->base.type].attacks[nextattack].range;
}

//----------------------------------------------------------------------------

void SV_AI_EnterGoal(ai_t* ai, const aiGoalCreateStruct_t* aigcs)
{
	ai->goal = SV_AIGoal_Create(aigcs);
}

//----------------------------------------------------------------------------

void SV_AI_ExitGoal(ai_t* ai)
{
	if ( ai->goal )
	{
		SV_AIGoal_Destroy(ai->goal);
		ai->goal = NULL;
	}
}

//----------------------------------------------------------------------------

void SV_AI_ChangeGoal(ai_t* ai, const aiGoalCreateStruct_t* aigcs)
{
	SV_AI_ExitGoal(ai);
	SV_AI_EnterGoal(ai, aigcs);
}

//----------------------------------------------------------------------------

void SV_AI_Init(ai_t* ai)
{
	aiNavCreateStruct_t aincs = { 0 };
	aiGoalCreateStruct_t aigcs = { 0 };

	aincs.ai = ai;
	ai->nav = SV_AINav_Create(&aincs);

	aigcs.ai = ai;
	aigcs.goal = AIGOAL_IDLE;
	SV_AI_ChangeGoal(ai, &aigcs);
}

//----------------------------------------------------------------------------

void SV_AI_Term(ai_t* ai)
{
	SV_AI_ExitGoal(ai);

	if ( ai->nav )
	{
		SV_AINav_Destroy(ai->nav);
		ai->nav = NULL;
	}

	SV_AIClearTarget(ai->aics.obj, GOAL_NONE);
}

//----------------------------------------------------------------------------

void SV_AI_Update(ai_t* ai)
{
	serverObject_t* obj = ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);

	if ( ai->destroy )
	{
		SV_AI_Destroy(ai);
		return;
	}

	SV_AIGoal_Update(ai->goal);

	if ( ai->destroy )
	{
		SV_AI_Destroy(ai);
		return;
	}

	SV_AINav_Update(ai->nav);

	if ( SV_AIGoal_IsDone(ai->goal) )
	{
		serverObject_t* obj = ai->aics.obj;

		if ( obj->base.team == 0 && obj->refObject.active && SV_AI_IsAttacking(ai) )
		{
			aiGoalCreateStruct_t aigcs = { 0 };

			aigcs.ai = ai;
			aigcs.goal = AIGOAL_GOTO;
			M_SetVec2(aigcs.targetPos, obj->refObject.pos.position[0], obj->refObject.pos.position[1]);

			SV_AI_ChangeGoal(ai, &aigcs);
		}
		else
		{
			aiGoalCreateStruct_t aigcs = { 0 };

			aigcs.ai = ai;
			aigcs.goal = AIGOAL_IDLE;

			SV_AI_ChangeGoal(ai, &aigcs);
		}
	}

	// if idle, check aggression range

	if ( !SV_AI_IsPeon(ai) && ai->goal->aigcs.goal == AIGOAL_IDLE )
	{
		// if it's time to do an aggression check

		if ( sl.gametime > ai->timeNextAggression )
		{
			// if we meet our chance of aggressiveness

			if ( unit->aggressionChance >= M_Randnum(0, 1) )
			{
				int target;
				float distance;

				#define MAX_ATTACKABLE_UNITS 16
				int attackableUnitsFiltered[MAX_ATTACKABLE_UNITS];
				int numAttackableUnitsFiltered = 0;

				int i;
				int n = MIN(numAttackableUnits, MAX_ATTACKABLE_UNITS);

				for ( i = 0 ; i < n ; ++i )
				{
					if ( GetObjectByType(attackableUnits[i])->npcTarget )
					{
						attackableUnitsFiltered[numAttackableUnitsFiltered++] = attackableUnits[i];
					}
				}
				
				target = SV_FindClosestObjects(attackableUnitsFiltered, numAttackableUnitsFiltered, ai->aics.obj->base.index, -1, M_Randnum(unit->aggressionRange.min, unit->aggressionRange.max), &distance);
				if (target >= 0)
				{
					SV_AITargetObject(NULL, obj, &sl.objects[target], GOAL_ATTACK_OBJECT);
				}
			}

			ai->timeNextAggression = sl.gametime + SV_AI_RandomTime(unit->aggressionInterval.min, unit->aggressionInterval.max);
		}
	}

	if ( ai->destroy )
	{
		SV_AI_Destroy(ai);
		return;
	}
}

//----------------------------------------------------------------------------

bool SV_AI_GetRandomPointCloserTo(ai_t* ai, float range[2], serverObject_t* to, vec2_t point)
{
	vec2_t dir, perp;
	float distance;

	serverObject_t* obj = ai->aics.obj;

	M_SubVec2(to->base.pos, obj->base.pos, dir);
	distance = M_NormalizeVec2(dir);
	M_SetVec2(perp, dir[1], -dir[0]);

	// try to find a valid point
	{
		repeatn(8)
		{
			float amount;
			vec2_t offset;

			if ( distance > range[1] )
			{
				amount = M_Randnum(range[0], range[1]);
			}
			else if ( distance > range[0] && distance < range[1] )
			{
				amount = M_Randnum(range[0], distance);
			}
			else if ( distance < range[0] )
			{
				return false;
			}

			M_MultVec2(dir, amount, point);
			M_AddVec2(obj->base.pos, point, point); 
			M_MultVec2(perp, M_Randnum(-100,100), offset);
			M_AddVec2(offset, point, point); 

			if ( cores.NavRep_Trace(navmesh_small, point, point) )
			{
				return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------

bool SV_AI_GetRandomPointFartherFrom(ai_t* ai, float range[2], serverObject_t* from, vec2_t point)
{
	vec2_t dir, perp;
	float distance;

	serverObject_t* obj = ai->aics.obj;

	M_SubVec2(from->base.pos, obj->base.pos, dir);
	distance = M_NormalizeVec2(dir);
	M_SetVec2(perp, dir[1], -dir[0]);

	// try to find a valid point
	{
		repeatn(8)
		{
			vec2_t offset;
			float amount;

			amount = -M_Randnum(range[0], range[1]);

			M_MultVec2(dir, amount, point);
			M_AddVec2(obj->base.pos, point, point); 
			M_MultVec2(perp, M_Randnum(-100,100), offset);
			M_AddVec2(offset, point, point); 

			if ( cores.NavRep_Trace(navmesh_small, point, point) )
			{
				return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------

bool SV_AI_HasResources(ai_t* ai)
{
	int index;
	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
	{
		if (ai->aics.obj->resources[index] > 0)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------

bool SV_AI_IsPeon(ai_t* ai)
{
	return ai->aics.obj->base.team != TEAM_UNDECIDED;
}

//----------------------------------------------------------------------------

bool SV_AI_IsAttacking(ai_t* ai)
{
	return (ai->goal->aigcs.goal == AIGOAL_ATTACKMISSILE || 
		    ai->goal->aigcs.goal == AIGOAL_ATTACKMELEE || 
		    ai->goal->aigcs.goal == AIGOAL_ATTACKPOUND ||
		    ai->goal->aigcs.goal == AIGOAL_ATTACKSUICIDE);
}

//----------------------------------------------------------------------------

bool SV_AI_IsIdle(ai_t* ai)
{
	if ( ai )
	{
		if ( ai->goal )
		{
			if ( ai->goal->aigcs.goal == AIGOAL_IDLE )
			{
				return true;
			}
		}
	}
	
	return false;
}

//----------------------------------------------------------------------------

bool SV_AI_IsMoving(ai_t* ai)
{
	return SV_AINav_IsMoving(ai->nav);
}

//----------------------------------------------------------------------------

float* SV_AI_GetMoveTo(ai_t* ai)
{
	return SV_AINav_GetMoveTo(ai->nav);
}

//----------------------------------------------------------------------------

bool SV_AI_IsHigherCollisionPriority(ai_t* ai, ai_t* ai_other)
{	
	serverObject_t* obj = ai->aics.obj;
	serverObject_t* obj_other = ai_other->aics.obj;

	// determine who gets to go. smaller abs of dp of velocity and direction to other unit wins

	vec2_t diff, dir;
	vec2_t diff_other, dir_other;

	// $todo: hmm...
	if ( obj_other->base.animState == AS_WALK_WITH_BAG && obj->base.animState == AS_WALK_FWD ) return false;
	if ( obj->base.animState == AS_WALK_WITH_BAG && obj_other->base.animState == AS_WALK_FWD ) return true;
// $TODO
	if ( SV_AIGoal_GetObject(ai->goal) == obj_other ) return false; // if ( obj->objectTarget == obj_other ) return false;
	if ( SV_AIGoal_GetObject(ai_other->goal) == obj ) return true; //if ( obj_other->objectTarget == obj ) return true;

	M_SubVec2(obj_other->base.pos, obj->base.pos, diff);
	M_NormalizeVec2(diff);
	M_SetVec2(diff_other, -diff[0], -diff[1]);

	M_SetVec2(dir, 0, 1);
	M_RotateVec2(obj->base.angle[2], dir);
	M_SetVec2(dir_other, 0, 1);
	M_RotateVec2(obj_other->base.angle[2], dir_other);

	return fabs(M_DotProduct2(diff, dir)) < fabs (M_DotProduct2(diff_other, dir_other));	
}

//----------------------------------------------------------------------------

bool SV_AI_IsInRange(ai_t* ai, serverObject_t* target, float range)
{
	return M_GetDistanceSqVec2(ai->aics.obj->base.pos, target->base.pos) < range*range;
}

//----------------------------------------------------------------------------

bool SV_AI_IsVisible(ai_t* ai, serverObject_t* target)
{
	serverObject_t* obj = ai->aics.obj;
	traceinfo_t trace;
	vec3_t dir;
	vec3_t endpos;

	M_SubVec3(target->base.pos, obj->base.pos, dir);	
	M_Normalize(dir);
	M_PointOnLine(obj->base.pos, dir, FAR_AWAY, endpos);

	SV_TraceBox(&trace, obj->base.pos, endpos, obj->base.bmin, obj->base.bmax, SURF_TERRAIN | SURF_STATIC);

	if (trace.fraction < 1)
	{
		if (trace.index == target->base.index)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------

bool SV_AI_IsInVisibleRange(ai_t* ai, serverObject_t* target, float range)
{
	serverObject_t* obj = ai->aics.obj;
	traceinfo_t trace;
	vec3_t dir;
	vec3_t endpos;

	M_SubVec3(target->base.pos, obj->base.pos, dir);	
	M_Normalize(dir);
	M_PointOnLine(obj->base.pos, dir, range, endpos);

	SV_TraceBox(&trace, obj->base.pos, endpos, obj->base.bmin, obj->base.bmax, SURF_TERRAIN | SURF_STATIC);

	if (trace.fraction < 1)
	{
		if (trace.index == target->base.index)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------

int SV_AI_CalculateAttackGoal(ai_t* ai, serverObject_t* target)
{
	serverObject_t* obj = ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);
	float attackChances = unit->attackMeleeChance + unit->attackMissileChance + unit->attackPoundChance + unit->attackSuicideChance;
	float chance = M_Randnum(0,attackChances);
	
	if ( unit->attackMeleeChance && chance <= unit->attackMeleeChance )
	{
		return AIGOAL_ATTACKMELEE;
	}

	chance -= unit->attackMeleeChance;
	
	if ( unit->attackMissileChance && chance <= unit->attackMissileChance && (unit->forceInventory[0] && *(unit->forceInventory[0])))
	{
		return AIGOAL_ATTACKMISSILE;
	}
	
	chance -= unit->attackMissileChance;
	
	if ( unit->attackPoundChance && chance <= unit->attackPoundChance )
	{
		return AIGOAL_ATTACKPOUND;
	}
	
	chance -= unit->attackPoundChance;
	
	if ( unit->attackSuicideChance && chance <= unit->attackSuicideChance )
	{
		return AIGOAL_ATTACKSUICIDE;
	}
	
	chance -= unit->attackSuicideChance;

	return AIGOAL_ATTACKMELEE;
}

//----------------------------------------------------------------------------

serverObject_t* SV_AI_FindThreat(ai_t* ai)
{
	int n;
	for (n = 0; n < MAX_CLIENTS; n++)
	{
		client_t *client = &sl.clients[n];
		if ( client->active )
		{
			vec2_t dir;

			M_SubVec2(ai->aics.obj->base.pos, client->obj->base.pos, dir);
			M_NormalizeVec2(dir);

			// $todo: should do line/circle intersection?
			if ( M_DotProduct2(dir, client->forward) > .99f )
			{
				return client->obj;
			}
		}
	}

	return NULL;
}

//----------------------------------------------------------------------------

bool SV_AI_FindFleeToPosition(ai_t* ai, serverObject_t* fleeing, vec2_t position)
{
	float distanceMin = FAR_AWAY;
	int index;
	for (index = 0; index <= sl.lastActiveObject; index++)
	{
		objectData_t* bld = GetObjectByType(sl.objects[index].base.type);
		objectData_t* wpn = &objData[GetObjectTypeByName(bld->forceInventory[0])];

		float distance;

		//only our team
		if (sl.objects[index].base.team != ai->aics.obj->base.team)
			continue;

		//only active objects
		if (!sl.objects[index].base.active)
			continue;

		//only buildings
		if (!IsBuildingType(sl.objects[index].base.type))
			continue;

		// only if it's a weapon
		if (!bld->refreshTime || !bld->scanRange || wpn->objclass != OBJCLASS_WEAPON)
			continue;

		distance = M_GetDistanceSqVec2(ai->aics.obj->base.pos, sl.objects[index].base.pos);
		if ( distance < distanceMin )
		{
			M_CopyVec2(sl.objects[index].base.pos, position);
			distanceMin = distance;
		}
	}

	if ( distanceMin < FAR_AWAY )
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------

serverObject_t* SV_AI_FindFleeToObject(ai_t* ai, serverObject_t* fleeing)
{
	serverObject_t* obj = ai->aics.obj;
	//objectData_t* unit = GetObjectByType(obj->base.type);

	int nearest = -1;
	float nearestDist = FAR_AWAY;
	int i;

	for ( i = 0 ; i < MAX_CLIENTS ; ++i )
	{
		if ( sl.clients[i].info.team == obj->base.team )
		{
			float dist = M_GetDistanceSqVec2(sl.clients[i].obj->base.pos, obj->base.pos);
			if ( dist < nearestDist )
			{
				nearest = i;
				nearestDist = dist;
			}
		}
	}

	if ( -1 != nearest )
	{
		return sl.clients[nearest].obj;
	}
	else
	{
		return NULL;
	}
}

//----------------------------------------------------------------------------

void SV_AI_HandleCollision(ai_t* ai, traceinfo_t* collision)
{
	SV_AINav_HandleCollision(ai->nav, collision);
}

//----------------------------------------------------------------------------

void SV_AI_HandleDamage(ai_t* ai, serverObject_t* attacker, int weapon, int attackDamage)
{
	serverObject_t* obj = ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);

	if ( !SV_AI_IsAttacking(ai) )
	{
		if ( SV_AI_IsPeon(ai) )
		{
			bool flee;

			float aggressionRangeSq = M_Randnum(unit->aggressionRange.min, unit->aggressionRange.max);
			aggressionRangeSq *= aggressionRangeSq;

			// check to see if attacker is close enough for aggression

			if ( M_GetDistanceSqVec2(attacker->base.pos, obj->base.pos) < aggressionRangeSq )
			{
				// check to see if have a pack

				#define MAX_PACK_OBJECTS 16
				int i, numObjects, packSize = 0;
				int objectType = obj->base.type;
				int objects[MAX_PACK_OBJECTS];

				numObjects = SV_FindObjectsInRadius(&objectType, 1, objData[obj->base.type].packRange, obj->base.pos, objects, MAX_PACK_OBJECTS);
				for ( i = 0 ; i < numObjects ; ++i )
				{
					ai_t* ai_other = sl.objects[objects[i]].ai;
					if ( ai_other && !SV_AI_IsAttacking(ai_other) )
					{
						// valid pack member if it's an ai that's not attacking (it is implicitly on our team)
						packSize++;
					}
				}

				if ( packSize >= unit->packSize.min )
				{
					packSize = MIN(packSize, unit->packSize.max);

					for ( i = 0 ; (i < numObjects) && (packSize > 0) ; ++i )
					{
						ai_t* ai_other = sl.objects[objects[i]].ai;
						if ( ai_other && !SV_AI_IsAttacking(ai_other) && (sl.objects[objects[i]].base.team == obj->base.team) )
						{
							// valid pack member if it's an ai that's not attacking and is on our team

							SV_AITargetObject(NULL, ai_other->aics.obj, attacker, GOAL_ATTACK_OBJECT);
							packSize--;
						}
					}

					SV_AITargetObject(NULL, obj, attacker, GOAL_ATTACK_OBJECT);

					return;
				}
			}

			flee = SV_AI_IsIdle(ai);

			if ( flee || (obj->base.health/unit->fullHealth < unit->fleeThreshhold) )
			{
				if ( flee || (M_Randnum(0,1) <= unit->fleeChance) )
				{
					aiGoalCreateStruct_t aigcs = { 0 };

					// find nearest ally to flee to

					aigcs.targetObj = SV_AI_FindFleeToObject(ai, attacker);
			
					if ( aigcs.targetObj )
					{
						// found an ally, flee to him

						aigcs.ai = ai;
						aigcs.goal = AIGOAL_FLEE;
						aigcs.targetObj = aigcs.targetObj;
						aigcs.fleeingObj = attacker;
						SV_AI_ChangeGoal(obj->ai, &aigcs);
					}
					else
					{
						// could not find an ally, flee to a position

						if ( SV_AI_FindFleeToPosition(ai, attacker, aigcs.targetPos) )
						{
							aigcs.ai = ai;
							aigcs.goal = AIGOAL_GOTO;
							SV_AI_ChangeGoal(obj->ai, &aigcs);
						}
					}
				}
			}

			return;
		}
		else
		{
			SV_AITargetObject(NULL, obj, attacker, GOAL_ATTACK_OBJECT);
		}
	}

	if ( attacker != ai->aics.obj )
	{
		#define MAX_PACK_OBJECTS 16
		int i, numObjects, objectType;
		int objects[MAX_PACK_OBJECTS];

		objectType = obj->base.type;

		numObjects = SV_FindObjectsInRadius(&objectType, 1, objData[obj->base.type].packRange, obj->base.pos, objects, MAX_PACK_OBJECTS);
		for ( i = 0 ; i < numObjects ; ++i )
		{
			ai_t* ai_other = sl.objects[objects[i]].ai;
			if ( ai_other && !SV_AI_IsAttacking(ai_other) )
			{
				SV_AITargetObject(NULL, ai_other->aics.obj, attacker, GOAL_ATTACK_OBJECT);
			}
		}
	}
}

//----------------------------------------------------------------------------

void SV_AI_HandleDeath(ai_t* ai, serverObject_t* attacker, int weapon, int attackDamage)
{
	ai->destroy = true;
}

//----------------------------------------------------------------------------

ai_t* SV_AI_Create(const aiCreateStruct_t* aics)
{
	ai_t* ai = ALLOCATE(ai_t);
	ai->aics = *aics;
	ai->goal = NULL;
	ai->nav = NULL;
	ai->timeNextWork = 0;
	ai->timeNextAggression = 0;
	ai->priority = M_Randnum(0.0f, 1.0f);
	ai->destroy = false;

	SV_AI_Init(ai);
	
	return ai;
}

//----------------------------------------------------------------------------

void SV_AI_Destroy(ai_t* ai)
{
	ai->destroy = true;
	SV_AI_Term(ai);
	ai->aics.obj->ai = NULL;
	DEALLOCATE(ai_t, ai);
}
