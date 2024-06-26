// (C) 2002 S2 Games

#include "server_game.h"
#include "sv_aiutil.h"
#include "sv_ai.h"
#include "sv_aigoal.h"
#include "sv_ainav.h"

extern cvar_t sv_unitAngleLerp;
extern cvar_t sv_aiPredict;
extern cvar_t sv_respawnMultiplier;
extern cvar_t sv_respawnNPCInterval;
extern cvar_t sv_respawnNPCs;

extern int numAttackableUnits;
extern int attackableUnits[];
extern int numPeonNames;

//----------------------------------------------------------------------------

void SV_AIClearTarget(serverObject_t *obj, int goal)
{
	if ( obj->ai )
	{
		aiGoalCreateStruct_t aigcs = { 0 };
		aigcs.ai = obj->ai;
		aigcs.goal = AIGOAL_IDLE;

		SV_AI_ExitGoal(obj->ai);

		if ( !obj->ai->destroy )
		{
			SV_AI_EnterGoal(obj->ai, &aigcs);
		}
	}
}

//----------------------------------------------------------------------------

void SV_AITargetPosition(client_t *giver, serverObject_t *obj, float x, float y, int goal)
{
	aiGoalCreateStruct_t aigcs = { 0 };

	if (obj->base.health <= 0 || !obj->base.active || !obj->ai)
		return;

	aigcs.ai = obj->ai;
	aigcs.goal = AIGOAL_GOTO;
	M_CopyVec2(vec2(x, y), aigcs.targetPos);

	SV_AI_ExitGoal(obj->ai);
	SV_AI_EnterGoal(obj->ai, &aigcs);
}

//----------------------------------------------------------------------------

void SV_AITargetObject(client_t *giver, serverObject_t *obj, serverObject_t *target, int goal)
{
	aiGoalCreateStruct_t aigcs = { 0 };

	if (obj->base.health <= 0 || !obj->base.active || !obj->ai || !target)
		return;

	if ( goal == GOAL_MINE )
	{
		aigcs.ai = obj->ai;
		aigcs.targetObj = target;
		aigcs.goal = AIGOAL_MINE;
		SV_AI_ChangeGoal(obj->ai, &aigcs);
	}
	else if ( goal == GOAL_REPAIR || goal == GOAL_CONSTRUCT_BUILDING || goal == GOAL_ENTER_BUILDING )
	{
		aigcs.ai = obj->ai;
		aigcs.targetObj = target;
		aigcs.goal = AIGOAL_CONSTRUCT;
		SV_AI_ChangeGoal(obj->ai, &aigcs);
	}
	else if ( goal == GOAL_ATTACK_OBJECT )
	{
		if (obj->base.team == target->base.team)
		{
			return;
		}

		aigcs.ai = obj->ai;
		aigcs.targetObj = target;
		aigcs.goal = SV_AI_CalculateAttackGoal(obj->ai, target);
		SV_AI_ChangeGoal(obj->ai, &aigcs);
	}
	else if ( goal == GOAL_FOLLOW )
	{
		aigcs.ai = obj->ai;
		aigcs.targetObj = target;
		aigcs.goal = AIGOAL_FOLLOW;
		SV_AI_ChangeGoal(obj->ai, &aigcs);
	}
	else
	{
//		__asm { int 0x03 }
	}

/* $MOVE
	float dist = 0;

	if (obj->base.health <= 0 || !obj->base.active)
		return; 

	if (!obj->base.team)
	{
		if (!GetObjectByType(target->base.type)->npcTarget)
			return;
	}

	AI_PRINTF("ai object %i: clearing goal because we have a new object goal (goal %i)\n", obj->base.index, goal);
	SV_AIClearTarget(obj, GOAL_NONE);

	obj->targetType = TARGTYPE_OBJECT;
	obj->objectTarget = target;
	obj->goal = goal;
	
	if (goal == GOAL_FLEE)
	{
		M_SubVec3(obj->base.pos, target->base.pos, obj->posTarget);
		dist = M_Normalize(obj->posTarget);
		M_MultVec3(obj->posTarget, 50, obj->posTarget);
		M_AddVec3(obj->base.pos, obj->posTarget, obj->posTarget);
	}
	else
	{
		M_CopyVec3(target->base.pos, obj->posTarget);
	}

	SV_AIGetPath(obj, obj->posTarget);

	//if (dist > 50)
	//	SV_AIClearTarget(obj, GOAL_NONE);
*/
}

//----------------------------------------------------------------------------

void SV_AISetSecondaryAnim(serverObject_t *obj, int anim)
{
	if (obj->base.animState2 == anim)
	{
		//retrigger the animation
		SV_Phys_AddEvent(obj, EVENT_ANIM2_RETRIGGER, 0, 0);
	}

	obj->base.animState2 = anim;
}

/* $MOVE
//----------------------------------------------------------------------------

void	SV_AINoticeObject(serverObject_t *obj, serverObject_t *target)
{
	if (!obj->objectTarget)
		SV_AITargetObject(obj, target, (objData[obj->base.type].isScared) ? GOAL_FLEE : GOAL_ATTACK_OBJECT);
}

//----------------------------------------------------------------------------

#define RUN_AWAY_DIST 1000
#define MAX_PACK_OBJECTS 40
*/


//handles respawning of NPCs
void	SV_NPCDeathFrame(serverObject_t *obj)
{
	if (sl.gametime >= obj->nextDecisionTime)
	{
		//might want to respawn it
		if (obj->refObject.active)		//did the object spawn from a world reference?
		{			
			if (!obj->base.team 
				&& (obj->livesLeft > 0 || obj->livesLeft == -1))
			{	
				serverObject_t *newnpc;
				traceinfo_t trace;
				objectData_t *unit = GetObjectByType(obj->base.type);
				//do a trace to make sure we won't spawn inside anything
				SV_TraceBox(&trace, obj->refObject.pos.position, obj->refObject.pos.position, unit->bmin, unit->bmax, TRACE_PLAYER_MOVEMENT | SURF_TERRAIN);
				if (trace.fraction < 1)
				{
					obj->nextDecisionTime = sl.gametime + 2000;		//wait a couple of seconds before trying to respawn again
					return;
				}

				newnpc = SV_SpawnObject(obj->base.team, obj->base.type, obj->refObject.pos.position, obj->refObject.pos.rotation);
				if (obj->livesLeft > 0)
					newnpc->livesLeft = obj->livesLeft - 1;
				else
					newnpc->livesLeft = -1;
				newnpc->refObject = obj->refObject;
			}
		}

		//free the old object
		SV_FreeObject(obj->base.index);
	}
}


int SV_AIDamaged(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags)
{
	int ret = SV_ObjectDamaged(obj, attacker, pos, weapon, attackDamage, damageFlags);
	
	if ( !ret )
	{
		return 0;
	}

	if ( attacker->client )
	{
		if ( attacker->client->info.team != obj->base.team )
		{
			// perform rewards if we died

			if ( obj->base.health <= 0 )
			{
				if ( sv_goodieBags.integer )
				{
					SV_SpawnGoodieBag(obj, obj->base.pos, attacker->client);
				}
				else
				{
					SV_GiveMoney(attacker->base.index, GetObjectByType(obj->base.type)->killGoldReward, true);
				}
			}

			// credit our attacker with a kill

			if ( obj->base.team > 0 )
			{
				attacker->client->stats.aiUnitDamage += ret;
				if ( obj->base.health <= 0 )
					attacker->client->stats.aiUnitKills++;
			}
			else
			{
				attacker->client->stats.npcDamage += ret;
				if ( obj->base.health <= 0 )
					attacker->client->stats.npcKills++;
			}
		}
	}	

	if ( obj->base.health <= 0 )
	{
		objectData_t* unit = GetObjectByType(obj->base.type);
		
		if ( attacker->client )
		{
			if (SV_GetTeam(obj->base.index) > 0 && SV_GetTeam(obj->base.index) != SV_GetTeam(attacker->base.index))
				SV_AddExperience(attacker->client, EXPERIENCE_WORKER_KILL, unit->level, unit->expMult);
			else
				SV_AddExperience(attacker->client, EXPERIENCE_NPC_KILL, unit->level, unit->expMult);
		}

		AI_PRINTF("ai object %i: clearing goal because we are dead\n", obj->base.index);

		if ( obj->ai ) 
		{
			SV_AI_HandleDamage(obj->ai, attacker, weapon, attackDamage);
			SV_AI_HandleDeath(obj->ai, attacker, weapon, attackDamage);
		}
		
//		SV_AIClearTarget(obj, GOAL_NONE);
		
		obj->base.animState = AS_DEATH_GENERIC;
		obj->base.animState2 = 0;
		
		if ( !obj->base.team )
		{
			int respawntime = unit->respawnTime ? unit->respawnTime : 10000;

			//call deathevent for normal death processing, but set the frame function to SV_NPCDeathFrame so we can respawn it if necessary
			SV_DeathEvent(obj, EVENT_DEATH, SV_EncodePositionOnObject(obj, pos), (byte)MIN(255, attackDamage), sv_respawnMultiplier.value * (unit->respawnTime ? unit->respawnTime : 10000), true);
			if (sv_respawnNPCs.integer)
				obj->frame = SV_NPCDeathFrame;
		}
		else
		{
			//have corpse lie for 10 seconds until disappearing
			SV_DeathEvent(obj, EVENT_DEATH, SV_EncodePositionOnObject(obj, pos), (byte)MIN(255, attackDamage), 10000, true);
		}

		return ret;
	}
	else
	{
		if ( obj->ai ) 
		{
			SV_AI_HandleDamage(obj->ai, attacker, weapon, attackDamage);
		}
	}

	if (!(damageFlags & DAMAGE_NO_REACTION))
		SV_Phys_AddEvent(obj, EVENT_WOUNDED, SV_EncodePositionOnObject(obj, pos), (byte)MIN(255, attackDamage));

	return ret;
}

//----------------------------------------------------------------------------

void SV_AdvanceAIObject(serverObject_t *obj)
{
	physicsParams_t p;
	phys_out_t phys_out;
	vec2_t intent = { 0,0 };
	traceinfo_t groundtrace;//, groundtrace2;
	//objectData_t *unit = &objData[obj->base.type];
	bool onground, fighting;
	vec3_t oldpos;
	vec3_t oldvelocity;
	float *targetpos = NULL;

	obj->base.animState = AS_IDLE;

	if ( !obj->ai ) return;

	SV_AI_Update(obj->ai);

	if ( !obj->ai ) return;

	targetpos = SV_AI_IsMoving(obj->ai) ? SV_AI_GetMoveTo(obj->ai) : NULL;

	fighting = false;
	phys_out.num_collisions = 0;

	M_CopyVec3(obj->base.pos, oldpos);
	M_CopyVec3(obj->velocity, oldvelocity);

	onground = SV_Phys_ObjectOnGround(obj, &groundtrace);
	Phys_SetupParams(obj->base.type, obj->base.team, obj->base.states, obj->base.pos, obj->velocity, SV_TraceBox, GetObjectByType, sl.frame_sec, &p);

	if ( targetpos )
	{
		//get an intended velocity based on the target position and the unit's speed setting
		//however, don't move at all if we're waiting to 'think' again

//		if (sl.gametime >= obj->nextDecisionTime)
		{
			M_SubVec2(targetpos, obj->base.pos, intent);
			M_NormalizeVec2(intent);
			M_MultVec2(intent, p.speed * PHYSICS_SCALE, intent);
		}
	}

	if ( fabs(intent[0]) > .001 && fabs(intent[1]) > .001 )
	{
		M_NormalizeVec2(intent);
		M_MultVec2(intent, p.speed * PHYSICS_SCALE, intent);
	}

	//apply gravity and move

	if ( onground )
	{
		if (obj->velocity[2] < 0)
		{
			obj->velocity[2] = 0;
		}

		obj->velocity[0] = intent[0];
		obj->velocity[1] = intent[1];

		// slide if we have any velocity

		if ( !M_CompareVec3(obj->velocity, zero_vec) )
		{
			Phys_SmartSlide(&p, &phys_out);
		}
	}
	else
	{
		obj->velocity[2] -= p.gravity * PHYSICS_SCALE;

		Phys_Slide(&p, &phys_out);
	}

	// handle all our collisions

	if ( phys_out.num_collisions )
	{
		int collision = 0;
		for ( collision = 0 ; collision < phys_out.num_collisions ; ++collision )
		{
			traceinfo_t *trace;

			trace = &phys_out.collisions[collision];
			if ( !trace || trace->index >= MAX_OBJECTS ) continue;

			SV_AI_HandleCollision(obj->ai, trace);
		}
	}

	//set animState animation and angle

	if ( SV_AI_IsMoving(obj->ai) || obj->ai->nav->face )
	{
		//fixme: lerping should be done client side
		vec3_t direction;
		float anglelerp;

		if ( obj->ai->nav->face )
		{
			anglelerp = sl.frame_sec * sv_unitAngleLerp.value;

			M_SubVec2(obj->ai->nav->facePos, obj->base.pos, direction);
			direction[2] = 0;
			M_NormalizeVec2(direction);
		}
		else
		{
			anglelerp = sl.frame_sec * sv_unitAngleLerp.value;

			M_CopyVec3(obj->velocity, direction);
			direction[2] = 0;
			if (M_CompareVec3(direction, zero_vec))
			{
/*				__asm { int 0x03 }
					switch(obj->targetType)
				{
					case TARGTYPE_LOCATION:
						M_SubVec3(obj->posTarget, obj->base.pos, direction);
						break;
					case TARGTYPE_OBJECT:
						if (obj->objectTarget->base.active)
						{
							M_SubVec3(obj->objectTarget->base.pos, obj->base.pos, direction);
						}
						break;
					default:
						break;
				}
*/
			}
			else
			{
				M_NormalizeVec2(direction);
			}
		}

		if (!M_CompareVec3(direction, zero_vec))
		{
			if (anglelerp > 1)
				anglelerp = 1;
			else if (anglelerp < 0)
				anglelerp = 0;

			obj->base.angle[2] = M_LerpAngle(anglelerp, obj->base.angle[2], (M_GetVec2Angle(direction)));
		}

		if ( SV_AI_IsMoving(obj->ai) )
		{
			if ( SV_AINav_IsDodging(obj->ai->nav) )
			{
				if ( AINAV_DODGE_STRAFELEFT == SV_AINav_GetDodge(obj->ai->nav) )
				{
					obj->base.animState = AS_WALK_LEFT;
				}
				else // if ( AINAV_DODGE_STRAFERIGHT == SV_AINav_GetDodge(obj->ai->nav) )
				{
					obj->base.animState = AS_WALK_RIGHT;
				}
			}
			else
			{
				int r;

				obj->base.animState = AS_WALK_FWD;

				for (r=0; r < MAX_RESOURCE_TYPES; r++)
				{
					if (obj->resources[r])
					{
						obj->base.animState = AS_WALK_WITH_BAG;
						break;
					}
				}
			}
		}
		else
		{
	//		obj->base.animState = AS_IDLE;
		}
	}
	
	if ( !SV_AI_IsMoving(obj->ai) )
	{
		M_ClearVec2(obj->velocity);
	}

	// $todo: figure out better place to put this
	obj->ai->nav->face = false;
}

//----------------------------------------------------------------------------

void SV_AIFrame(serverObject_t *obj)
{
	cores.World_UnlinkObject(&obj->base);

	SV_AdvanceAIObject(obj);

	cores.World_LinkObject(&obj->base);	

	/*
	if (sv_aiPredict.integer)
	{
		obj->base.flags |= BASEOBJ_PREDICT_POS;
		//set the prVelocity field for AI units getting predicted
		M_SubVec3(obj->base.pos, obj->oldpos, obj->base.prVelocity);
		//normalize it into units per second
		M_MultVec3(obj->base.prVelocity, 1 / MAX(1,sl.frame_sec), obj->base.prVelocity);
	}
	*/

	if ( obj->ai )
	{
		if ( !SV_AI_IsMoving(obj->ai) && !obj->subtracted )
		{
			cores.NavRep_CSGSubtract(&obj->base);
			obj->subtracted = true;
		}
		else if ( SV_AI_IsMoving(obj->ai) && obj->subtracted )
		{
			cores.NavRep_CSGAdd(&obj->base);
			obj->subtracted = false;
		}
	}
}

//----------------------------------------------------------------------------

serverObject_t* SV_SpawnAIUnit(byte type, int team, vec3_t position, vec3_t angle)
{
	pointinfo_t pi;
	aiCreateStruct_t aics;
	serverObject_t *obj = SV_AllocObject(type, team);

	SV_FillInBaseStats(obj);

	aics.obj = obj;
	obj->ai = SV_AI_Create(&aics);
	obj->base.fullhealth = obj->adjustedStats.fullhealth;
	obj->base.health = obj->adjustedStats.fullhealth;
	obj->targetPosition = SV_AITargetPosition;
	obj->targetObject = SV_AITargetObject;
	obj->clearTarget = SV_AIClearTarget;
	obj->frame = SV_AIFrame;
	obj->damage = SV_AIDamaged;
	//	obj->noticeObject = SV_AINoticeObject;

	M_CopyVec3(position, obj->base.pos);
	M_CopyVec3(angle, obj->base.angle);	
	
	cores.World_SampleGround(obj->base.pos[X], obj->base.pos[Y], &pi);
	obj->base.pos[Z] = pi.z;
					
	SV_Phys_AddEvent(obj, EVENT_SPAWN, 0, 0);

	//give it a name
	if ( obj->base.team != TEAM_UNDECIDED && IsWorkerType(type) )
	{
		obj->base.nameIdx = M_Randnum(1, numPeonNames);
	}

	cores.World_LinkObject(&obj->base);

	cores.NavRep_CSGSubtract(&obj->base);
	obj->subtracted = true;

	return obj;
}
