#include "server_game.h"
#include "sv_aiutil.h"
#include "sv_ai.h"
#include "sv_aistate.h"
#include "sv_ainav.h"

#define WORK_INTERVAL 1000		//delay between workers building/repairing/mining

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIState_Init(aiState_t* state)
{
	state->init(state);
}

//----------------------------------------------------------------------------

void SV_AIState_Term(aiState_t* state)
{
	state->term(state);
}

//----------------------------------------------------------------------------

void SV_AIState_Update(aiState_t* state)
{
	state->update(state);
}

//----------------------------------------------------------------------------

bool SV_AIState_IsDone(aiState_t* state)
{
	return state->done;
}

//----------------------------------------------------------------------------

int SV_AIState_GetError(aiState_t* state)
{
	return state->error;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateIdle_Init(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateIdle_Term(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateIdle_Update(aiState_t* state)
{
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateGoto_Init(aiState_t* state)
{
	if ( state->aiscs.targetObj )
	{
		bool building = GetObjectByType(state->aiscs.targetObj->base.type)->objclass == OBJCLASS_BUILDING;
		state->timeRepath = sl.gametime + SV_AI_RandomTime(500, 1500);
		SV_AINav_GotoObject(state->aiscs.ai->nav, state->aiscs.targetObj, false, building);
	}
	else
	{
		SV_AINav_GotoPosition(state->aiscs.ai->nav, state->aiscs.targetPos);
	}
}

//----------------------------------------------------------------------------

void SV_AIStateGoto_Term(aiState_t* state)
{
	SV_AINav_Stop(state->aiscs.ai->nav);
}

//----------------------------------------------------------------------------

void SV_AIStateGoto_Update(aiState_t* state)
{
	if ( SV_AINav_IsArrived(state->aiscs.ai->nav) )
	{
		state->done = true;
		return;
	}
	else if ( state->aiscs.targetObj && (sl.gametime > state->timeRepath) )
	{
		bool building = GetObjectByType(state->aiscs.targetObj->base.type)->objclass == OBJCLASS_BUILDING;
		// $todo: optimize the path?
		// $todo: rand based on distance from object?
		SV_AINav_GotoObject(state->aiscs.ai->nav, state->aiscs.targetObj, false, building);
		state->timeRepath = sl.gametime + SV_AI_RandomTime(500, 1500);
	}
/*

	if ( SV_AINav_IsBlocked(state->aiscs.ai->nav) )
	{

	}
*/
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateChase_Init(aiState_t* state)
{
	state->timeRepath = sl.gametime + SV_AI_RandomTime(500, 1500);

	SV_AINav_GotoObject(state->aiscs.ai->nav, state->aiscs.targetObj, !SV_AI_IsPeon(state->aiscs.ai), false);
}

//----------------------------------------------------------------------------

void SV_AIStateChase_Term(aiState_t* state)
{
	SV_AINav_Stop(state->aiscs.ai->nav);
}

//----------------------------------------------------------------------------

void SV_AIStateChase_Update(aiState_t* state)
{
	serverObject_t* target = state->aiscs.targetObj;
	
	if ( SV_AINav_IsArrived(state->aiscs.ai->nav) )
	{
		SV_AINav_GotoObject(state->aiscs.ai->nav, state->aiscs.targetObj, !SV_AI_IsPeon(state->aiscs.ai), false);
	}
	else if ( sl.gametime > state->timeRepath )
	{
		// $todo: optimize the path?
		// $todo: rand based on distance from object?
		SV_AINav_GotoObject(state->aiscs.ai->nav, state->aiscs.targetObj, !SV_AI_IsPeon(state->aiscs.ai), false);
		state->timeRepath = sl.gametime + SV_AI_RandomTime(500, 1500);
	}
	else
	{
		if ( SV_AI_IsInRange(state->aiscs.ai, target, state->aiscs.chaseRange) && SV_AI_IsVisible(state->aiscs.ai, target) )
		{
			state->done = true;
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateMine_Init(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateMine_Term(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateMine_Update(aiState_t* state)
{
	serverObject_t* obj = state->aiscs.ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);
	serverObject_t* building = state->aiscs.targetObj;

	SV_AINav_FacePos(state->aiscs.ai->nav, building->base.pos);

	if ( sl.gametime >= state->aiscs.ai->timeNextWork )
	{
		state->aiscs.ai->timeNextWork = sl.gametime + WORK_INTERVAL;

		if ( !SV_MineResourcesFromBuilding(obj, state->aiscs.targetObj, unit->mineRate) )
		{
			state->done = true;
			return;
		}

		state->working = true;
	}
	
	if ( state->working )
	{
		obj->base.animState = AS_MINE;
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateConstruct_Init(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateConstruct_Term(aiState_t* state)
{
	SV_AISetSecondaryAnim(state->aiscs.ai->aics.obj, 0);
}

//----------------------------------------------------------------------------

void SV_AIStateConstruct_Update(aiState_t* state)
{
	serverObject_t* obj = state->aiscs.ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);
	serverObject_t* building = state->aiscs.targetObj;

	SV_AINav_FacePos(state->aiscs.ai->nav, building->base.pos);

	if ( sl.gametime >= state->aiscs.ai->timeNextWork )
	{
		state->aiscs.ai->timeNextWork = sl.gametime + WORK_INTERVAL;

		if ( !SV_ConstructBuilding(obj, state->aiscs.targetObj, unit->buildRate) )
		{
			if ( !SV_RepairBuilding(obj, state->aiscs.targetObj, unit->repairRate) )
			{
				state->done = true;
				return;
			}
		}

		state->working = true;
	}

	if ( state->working )
	{
		obj->base.animState = AS_CONSTRUCT;
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateAttackMelee_Init(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateAttackMelee_Term(aiState_t* state)
{
	SV_AISetSecondaryAnim(state->aiscs.ai->aics.obj, 0);
}

//----------------------------------------------------------------------------

void SV_AIStateAttackMelee_Update(aiState_t* state)
{
	ai_t* ai = state->aiscs.ai;
	serverObject_t* obj = ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);
	serverObject_t* enemy = state->aiscs.targetObj;

	SV_AINav_FacePos(state->aiscs.ai->nav, enemy->base.pos);

	// $todo: face target
	// $todo: handle target out of range, handle target dead, handle can't attack
	// $todo: don't do damage if out of range

	if ( IS_MELEE_ATTACK(obj->base.animState2) )
	{
		if ( (sl.gametime > state->timeImpacted) && !state->impacted )
		{
			state->impacted = true;
			SV_DamageTarget(obj, state->aiscs.targetObj, obj->base.pos, 0, unit->attacks[obj->base.animState2].damage, 0);
		}

		if ( (sl.gametime > state->timeAttacked) && !state->attacked )
		{
			state->timeAttack = sl.gametime + SV_AI_RandomTime(unit->attacks[obj->base.animState2].restTime.min, unit->attacks[obj->base.animState2].restTime.max);
			state->attacked = true;
			SV_AISetSecondaryAnim(obj, 0);
		}
	}
	else
	{
		if ( sl.gametime >= state->timeAttack )		//fixme: might want to use nextDecisionTime..but maybe not
		{
			if ( !SV_AI_IsInRange(ai, enemy, SV_AI_GetMeleeRange(ai)) )
			{
				state->error = AISTATE_ERR_TARGETOUTOFRANGE;
				return;
			}

			state->impacted = false;
			state->attacked = false;

			if (!obj->base.animState2)
			{
				SV_AISetSecondaryAnim(obj, AS_MELEE_1);					
			}
			else
			{
				int nextAnim = obj->base.animState2 + 1;
									
				//if (obj->currentAttack > ATTACK_JAB + unit->flurryCount || obj->currentAttack < ATTACK_JAB)
				if (!unit->attacks[obj->base.animState2].impact.min)
				{
					SV_AISetSecondaryAnim(obj, AS_MELEE_1);						
				}
				else
				{
					SV_AISetSecondaryAnim(obj, nextAnim);
				}
			}				

			//if (obj->currentAttack == ATTACK_JAB + (unit->flurryCount))
			//if (!unit->attacks[obj->base.animState2].flurryTime)
			{
				state->timeImpacted = sl.gametime + unit->attacks[obj->base.animState2].impact.min;
				state->timeAttacked = sl.gametime + unit->attacks[obj->base.animState2].time;
			}
	/*		else
			{
				state->timeAttack = sl.gametime + unit->attacks[obj->base.animState2].flurry;
			}
	*/	}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateAttackMissile_Init(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateAttackMissile_Term(aiState_t* state)
{
	SV_AISetSecondaryAnim(state->aiscs.ai->aics.obj, 0);
}

//----------------------------------------------------------------------------

void SV_AIStateAttackMissile_FireProjectile(aiState_t* state)
{
	serverObject_t* obj = state->aiscs.ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);

	vec3_t dir;
	M_SubVec3(state->aiscs.targetObj->base.pos, state->aiscs.ai->aics.obj->base.pos, dir);
	M_Normalize(dir);

	SV_WeaponFire(state->aiscs.ai->aics.obj, dir, GetObjectTypeByName(unit->forceInventory[0]), NULL);
}

//----------------------------------------------------------------------------

void SV_AIStateAttackMissile_Update(aiState_t* state)
{
	ai_t* ai = state->aiscs.ai;
	serverObject_t* obj = ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);
	serverObject_t* enemy = state->aiscs.targetObj;

	SV_AINav_FacePos(state->aiscs.ai->nav, enemy->base.pos);


	// $todo: face target
	// $todo: handle target out of range, handle target dead, handle can't attack
	// $todo: don't do damage if out of range

	if (IS_MELEE_ATTACK(obj->base.animState2))
	{
		float prob = M_Randnum(0,1);

		if (prob < unit->attackProbability
			&& (ai->timeNextWork - sl.gametime) < (unit->attacks[obj->base.animState2].impact.min)
			&& !(state->attacked))
		{
			state->attacked = true;
			//fixme: this should carry on until the very end of the attack, not just the impact time
			SV_AIStateAttackMissile_FireProjectile(state);
			//SV_DamageTarget(obj, state->aiscs.targetObj, obj->base.pos, 0, unit->attacks[obj->base.animState2].damage, 0);
			SV_AISetSecondaryAnim(obj, 0);
		}
	}

	if (sl.gametime >= ai->timeNextWork)		//fixme: might want to use nextDecisionTime..but maybe not
	{
		if ( !(SV_AI_IsInRange(ai, state->aiscs.targetObj, 400.0f) && SV_AI_IsVisible(ai, state->aiscs.targetObj)) )
		{
			state->error = AISTATE_ERR_TARGETOUTOFRANGE;
			return;
		}

		state->attacked = false;

		if (!obj->base.animState2)
		{
			SV_AISetSecondaryAnim(obj, AS_MELEE_1);					
		}
		else
		{
			int nextAnim = obj->base.animState2 + 1;
								
			//if (obj->currentAttack > ATTACK_JAB + unit->flurryCount || obj->currentAttack < ATTACK_JAB)
			if (!unit->attacks[obj->base.animState2].impact.min)
			{
				SV_AISetSecondaryAnim(obj, AS_MELEE_1);						
			}
			else
			{
				SV_AISetSecondaryAnim(obj, nextAnim);
			}
		}				

		//if (obj->currentAttack == ATTACK_JAB + (unit->flurryCount))
		if (!unit->attacks[obj->base.animState2].flurryTime)
			ai->timeNextWork = sl.gametime + unit->attacks[obj->base.animState2].time;
		else
			ai->timeNextWork = sl.gametime + unit->attacks[obj->base.animState2].flurry;
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateAttackPound_Init(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateAttackPound_Term(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateAttackPound_Update(aiState_t* state)
{
	ai_t* ai = state->aiscs.ai;
	serverObject_t* obj = ai->aics.obj;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIStateAttackSuicide_Init(aiState_t* state)
{
	ai_t* ai = state->aiscs.ai;
	serverObject_t* obj = ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);

	state->timeImpacted = sl.gametime + SV_AI_RandomTime(unit->attackSuicideDelay.min, unit->attackSuicideDelay.max);
}

//----------------------------------------------------------------------------

void SV_AIStateAttackSuicide_Term(aiState_t* state)
{
}

//----------------------------------------------------------------------------

void SV_AIStateAttackSuicide_Update(aiState_t* state)
{
	ai_t* ai = state->aiscs.ai;
	serverObject_t* obj = ai->aics.obj;
	objectData_t* unit = GetObjectByType(obj->base.type);

	if ( (sl.gametime > state->timeImpacted) && !state->done )
	{
		SV_Phys_AddEvent(obj, EVENT_ATTACK_SUICIDE, 0, 0);
		SV_DamageRadius(obj, obj->base.pos, obj->base.type, unit->attackSuicideRadius, unit->attackSuicideDamage, DAMAGE_SPLASH);
		SV_AIDamaged(obj, obj, obj->base.pos, 0, obj->base.health, 0);
		state->done = true;
	}
	else if ( !state->done )
	{
		obj->base.animState = AS_SUICIDE;
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiState_t);

//----------------------------------------------------------------------------

typedef struct
{
	int					type;
	SV_AIState_FnInit	init;
	SV_AIState_FnTerm	term;
	SV_AIState_FnInit	update;
}
aiFactoryState_t;

#define AIFACTORYSTATE(g, n) \
	{ AISTATE_##g,	(SV_AIState_FnInit)SV_AIState##n##_Init,	(SV_AIState_FnTerm)SV_AIState##n##_Term,	(SV_AIState_FnUpdate)SV_AIState##n##_Update },

static aiFactoryState_t factories[] = 
{
	AIFACTORYSTATE(IDLE, Idle)
	AIFACTORYSTATE(GOTO, Goto)
	AIFACTORYSTATE(CHASE, Chase)
	AIFACTORYSTATE(MINE, Mine)
	AIFACTORYSTATE(CONSTRUCT, Construct)
	AIFACTORYSTATE(ATTACKMELEE, AttackMelee)
	AIFACTORYSTATE(ATTACKMISSILE, AttackMissile)
	AIFACTORYSTATE(ATTACKPOUND, AttackPound)
	AIFACTORYSTATE(ATTACKSUICIDE, AttackSuicide)
};

//static int num_factories = sizeof(factories)/sizeof(aiFactoryState_t);

//----------------------------------------------------------------------------

aiState_t* SV_AIState_Create(const aiStateCreateStruct_t* aiscs)
{
	aiState_t* state = ALLOCATE(aiState_t);

	state->init = factories[aiscs->state].init;
	state->term = factories[aiscs->state].term;
	state->update = factories[aiscs->state].update;
	state->aiscs = *aiscs;
	state->error = 0;
	state->done = false;
	state->timeAttack = 0;
	state->impacted = false;
	state->timeImpacted = 0;
	state->attacked = false;
	state->timeAttacked = 0;
	state->timeRepath = 0;
	state->working = false;

	SV_AIState_Init(state);

	return state;
}

//----------------------------------------------------------------------------

void SV_AIState_Destroy(aiState_t* state)
{
	SV_AIState_Term(state);

	DEALLOCATE(aiState_t, state);
}
