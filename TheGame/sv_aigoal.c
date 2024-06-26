#include "server_game.h"
#include "sv_aiutil.h"
#include "sv_aistate.h"
#include "sv_aigoal.h"
#include "sv_ainav.h"
#include "sv_ai.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIGoal_ExitState(aiGoal_t* goal)
{
	if ( goal->state )
	{
		SV_AIState_Destroy(goal->state);
		goal->state = NULL;
	}
}

//----------------------------------------------------------------------------

void SV_AIGoal_EnterState(aiGoal_t* goal, const aiStateCreateStruct_t* aiscs)
{
	goal->state = SV_AIState_Create(aiscs);
}

//----------------------------------------------------------------------------

void SV_AIGoal_ChangeState(aiGoal_t* goal, const aiStateCreateStruct_t* aiscs)
{
	SV_AIGoal_ExitState(goal);

	goal->state = SV_AIState_Create(aiscs);
}

//----------------------------------------------------------------------------

void SV_AIGoal_Init(aiGoal_t* goal)
{
	goal->init(goal);
}

//----------------------------------------------------------------------------

void SV_AIGoal_Term(aiGoal_t* goal)
{
	SV_AIGoal_ExitState(goal);

	goal->term(goal);
}

//----------------------------------------------------------------------------

void SV_AIGoal_Update(aiGoal_t* goal)
{
	goal->update(goal);
}

//----------------------------------------------------------------------------

bool SV_AIGoal_Is(aiGoal_t* goal, int goalType)
{
	return goal->aigcs.goal == goalType;
}

//----------------------------------------------------------------------------

bool SV_AIGoal_IsDone(aiGoal_t* goal)
{
	return goal->done;
}

//----------------------------------------------------------------------------

int SV_AIGoal_GetError(aiGoal_t* goal)
{
	return goal->error;
}

//----------------------------------------------------------------------------

serverObject_t* SV_AIGoal_GetObject(aiGoal_t* goal)
{
	return goal->aigcs.targetObj;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIGoalGoto_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_GOTO;
	
	if ( goal->aigcs.targetObj )
	{
		aiscs.targetObj = goal->aigcs.targetObj;
	}
	else
	{
		M_CopyVec2(goal->aigcs.targetPos, aiscs.targetPos);
	}

	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);
}

//----------------------------------------------------------------------------

void SV_AIGoalGoto_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalGoto_Update(aiGoal_t* goal)
{
	SV_AIState_Update(goal->state);

	if ( goal->aigcs.targetObj && !SV_Object_IsValid(goal->aigcs.targetObj) )
	{
		goal->done = true;
		goal->error = AIGOAL_ERR_TARGETINVALID;
	}
	else if ( SV_AIState_IsDone(goal->state) )
	{
		SV_DropoffResources(goal->aigcs.ai->aics.obj, goal->aigcs.targetObj);
		goal->done = true;
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoalFollow_t);

//----------------------------------------------------------------------------

const int GOAL_FOLLOW_TASK_NONE = -1;
const int GOAL_FOLLOW_TASK_FOLLOWING = 0;
const int GOAL_FOLLOW_TASK_WAITING = 1;

//----------------------------------------------------------------------------

void SV_AIGoalFollow_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_CHASE;
	aiscs.targetObj = goal->aigcs.targetObj;
	aiscs.chaseRange = 30.0f;
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

	goal->task = GOAL_FOLLOW_TASK_FOLLOWING;
	goal->timeNextFollow = sl.gametime + SV_AI_RandomTime(1000,2000);
}

//----------------------------------------------------------------------------

void SV_AIGoalFollow_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalFollow_Update(aiGoal_t* goal)
{
	repeat
	{
		if ( !SV_Unit_IsValid(goal->aigcs.targetObj) )
		{
			goal->done = true;
			goal->error = AIGOAL_ERR_TARGETINVALID;

			return;
		}

		SV_AIState_Update(goal->state);

		if ( GOAL_FOLLOW_TASK_FOLLOWING == goal->task )
		{
			if ( SV_AIState_IsDone(goal->state) )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_IDLE;
				aiscs.targetObj = goal->aigcs.targetObj;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->timeNextFollow = sl.gametime + SV_AI_RandomTime(1000, 2000);
				goal->task = GOAL_FOLLOW_TASK_WAITING;
			}
			else
			{
				return;
			}
		}
		else if ( GOAL_FOLLOW_TASK_WAITING == goal->task )
		{
			if ( (sl.gametime > goal->timeNextFollow) && !SV_AI_IsInVisibleRange(goal->aigcs.ai, goal->aigcs.targetObj, 30.0f) )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_CHASE;
				aiscs.targetObj = goal->aigcs.targetObj;
				aiscs.chaseRange = 30.0f;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_FOLLOW_TASK_FOLLOWING;
			}
			else
			{
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoalFlee_t);

//----------------------------------------------------------------------------

const int GOAL_FLEE_TASK_NONE = -1;
const int GOAL_FLEE_TASK_FLEEING = 0;
const int GOAL_FLEE_TASK_WAITING = 1;

//----------------------------------------------------------------------------

void SV_AIGoalFlee_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_CHASE;
	aiscs.targetObj = goal->aigcs.targetObj;
	aiscs.chaseRange = 30.0f;
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

	goal->task = GOAL_FLEE_TASK_FLEEING;
	goal->timeNextFollow = sl.gametime + SV_AI_RandomTime(1000, 2000);
	goal->timeNextFled = sl.gametime + SV_AI_RandomTime(2000, 4000);
}

//----------------------------------------------------------------------------

void SV_AIGoalFlee_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalFlee_Update(aiGoal_t* goal)
{
	repeat
	{
		if ( !SV_Object_IsValid(goal->aigcs.fleeingObj) )
		{
			goal->done = true;
			return;
		}

		if ( !SV_Unit_IsValid(goal->aigcs.targetObj) )
		{
			goal->done = true;
			goal->error = AIGOAL_ERR_TARGETINVALID;

			return;
		}

		if ( SV_AI_IsInVisibleRange(goal->aigcs.ai, goal->aigcs.fleeingObj, 200.0f) )
		{
			goal->timeNextFled = sl.gametime + SV_AI_RandomTime(2000, 4000);
		}

		if ( sl.gametime > goal->timeNextFled )
		{
			goal->done = true;
			return;
		}

		SV_AIState_Update(goal->state);

		if ( GOAL_FLEE_TASK_FLEEING == goal->task )
		{
			if ( SV_AIState_IsDone(goal->state) )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_IDLE;
				aiscs.targetObj = goal->aigcs.targetObj;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->timeNextFollow = sl.gametime + SV_AI_RandomTime(1000, 2000);
				goal->task = GOAL_FLEE_TASK_WAITING;
			}
			else
			{
				return;
			}
		}
		else if ( GOAL_FLEE_TASK_WAITING == goal->task )
		{
			if ( (sl.gametime > goal->timeNextFollow) && !SV_AI_IsInVisibleRange(goal->aigcs.ai, goal->aigcs.targetObj, 30.0f) )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_CHASE;
				aiscs.targetObj = goal->aigcs.targetObj;
				aiscs.chaseRange = 30.0f;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_FLEE_TASK_FLEEING;
			}
			else
			{
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoalConstruct_t);

//----------------------------------------------------------------------------

const int GOAL_CONSTRUCT_TASK_NONE = -1;
const int GOAL_CONSTRUCT_TASK_GOING_TO_CONSTRUCT = 0;
const int GOAL_CONSTRUCT_TASK_CONSTRUCTING = 1;

//----------------------------------------------------------------------------

void SV_AIGoalConstruct_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_GOTO;
	aiscs.targetObj = goal->aigcs.targetObj;
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

	goal->task = GOAL_CONSTRUCT_TASK_GOING_TO_CONSTRUCT;
}

//----------------------------------------------------------------------------

void SV_AIGoalConstruct_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalConstruct_Update(aiGoal_t* goal)
{
	repeat
	{
		if ( !SV_Building_IsValid(goal->aigcs.targetObj) )
		{
			goal->done = true;
			goal->error = AIGOAL_ERR_TARGETINVALID;

			return;
		}

		SV_AIState_Update(goal->state);

		if ( GOAL_CONSTRUCT_TASK_GOING_TO_CONSTRUCT == goal->task )
		{
			if ( SV_AI_IsInVisibleRange(goal->aigcs.ai, goal->aigcs.targetObj, 15.0f) || SV_AIState_IsDone(goal->state) )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_CONSTRUCT;
				aiscs.targetObj = goal->aigcs.targetObj;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_CONSTRUCT_TASK_CONSTRUCTING;

				// dropoff any resources we might have

				SV_DropoffResources(goal->aigcs.ai->aics.obj, goal->aigcs.targetObj);
			}
			else
			{
				return;
			}
		}
		else if ( GOAL_CONSTRUCT_TASK_CONSTRUCTING == goal->task )
		{
			// if done mining, goto going to base task
			// else return

			if ( SV_AIState_IsDone(goal->state) )
			{
				goal->done = true;

				return;
			}
			else
			{
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoalMine_t);

//----------------------------------------------------------------------------

const int GOAL_MINE_TASK_NONE = -1;
const int GOAL_MINE_TASK_GOING_TO_MINE = 0;
const int GOAL_MINE_TASK_MINING = 1;
const int GOAL_MINE_TASK_GOING_TO_BASE = 2;

//----------------------------------------------------------------------------

void SV_AIGoalMine_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_GOTO;
	aiscs.targetObj = goal->aigcs.targetObj;
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

	goal->task = GOAL_MINE_TASK_GOING_TO_MINE;
	goal->dropoff = NULL;
}

//----------------------------------------------------------------------------

void SV_AIGoalMine_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalMine_Update(aiGoal_t* goal)
{
	repeat
	{
		if ( !SV_Building_IsValid(goal->aigcs.targetObj) || !GetObjectByType(goal->aigcs.targetObj->base.type)->isMine )
		{
			goal->done = true;
			goal->error = AIGOAL_ERR_TARGETINVALID;

			return;
		}

		if ( (GOAL_MINE_TASK_GOING_TO_BASE == goal->task) && goal->dropoff && !SV_Building_IsValid(goal->dropoff) )
		{
			// our base was invalidated somehow, find a new dropoff

			aiStateCreateStruct_t aiscs = { 0 };

			goal->dropoff = SV_FindNearestDropoffPoint(goal->aigcs.ai->aics.obj);
			if ( !goal->dropoff )
			{
				AI_PRINTF("clearing goal because we are mining and can't find a dropoff\n");
				
				goal->done = true;
				goal->error = AISTATE_ERR_TARGETINVALID;

				return;
			}

			aiscs.ai = goal->aigcs.ai;
			aiscs.state = AISTATE_GOTO;
			aiscs.targetObj = goal->dropoff;
			SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);
		}

		SV_AIState_Update(goal->state);

		if ( GOAL_MINE_TASK_GOING_TO_MINE == goal->task )
		{
			// if arrived at mine, goto mine task
			// else return

			if ( SV_AI_IsInVisibleRange(goal->aigcs.ai, goal->aigcs.targetObj, 5.0f) || SV_AIState_IsDone(goal->state) )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_MINE;
				aiscs.targetObj = goal->aigcs.targetObj;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_MINE_TASK_MINING;
			}
			else
			{
				return;
			}
		}
		else if ( GOAL_MINE_TASK_MINING == goal->task )
		{
			// if done mining, goto going to base task
			// else return

			if ( SV_AIState_IsDone(goal->state) )
			{
				if ( SV_AI_HasResources(goal->aigcs.ai) )
				{
					aiStateCreateStruct_t aiscs = { 0 };

					goal->dropoff = SV_FindNearestDropoffPoint(goal->aigcs.ai->aics.obj);
					if ( !goal->dropoff )
					{
						AI_PRINTF("clearing goal because we are mining and can't find a dropoff\n");
						
						goal->done = true;
						goal->error = AISTATE_ERR_TARGETINVALID;

						return;
					}

					aiscs.ai = goal->aigcs.ai;
					aiscs.state = AISTATE_GOTO;
					aiscs.targetObj = goal->dropoff;
					SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

					goal->task = GOAL_MINE_TASK_GOING_TO_BASE;
				}
				else
				{
					goal->done = true;

					return;
				}
			}
			else
			{
				return;
			}
		}
		else if ( GOAL_MINE_TASK_GOING_TO_BASE == goal->task )
		{
			// if arrived at base, dropoff resources and go back to mine
			// else return

			if ( SV_AIState_IsDone(goal->state) )
			{
				aiStateCreateStruct_t aiscs = { 0 };

				SV_DropoffResources(goal->aigcs.ai->aics.obj, goal->dropoff);
				goal->dropoff = NULL;

				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_GOTO;
				aiscs.targetObj = goal->aigcs.targetObj;

				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_MINE_TASK_GOING_TO_MINE;
			}
			else
			{
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoalAttackMelee_t);

//----------------------------------------------------------------------------

const int GOAL_ATTACKMELEE_TASK_NONE = -1;
const int GOAL_ATTACKMELEE_TASK_CHASING = 0;
const int GOAL_ATTACKMELEE_TASK_ATTACKING = 1;
const int GOAL_ATTACKMELEE_TASK_WAITING = 2;

//----------------------------------------------------------------------------

void SV_AIGoalAttackMelee_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_CHASE;
	aiscs.targetObj = goal->aigcs.targetObj;
	aiscs.chaseRange = SV_AI_GetMeleeRange(goal->aigcs.ai);
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

	goal->task = GOAL_ATTACKMELEE_TASK_CHASING;
}

//----------------------------------------------------------------------------

void SV_AIGoalAttackMelee_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalAttackMelee_Update(aiGoal_t* goal)
{
	serverObject_t* obj = goal->aigcs.ai->aics.obj;
	objectData_t *unit = &objData[obj->base.type];
	ai_t* ai = obj->ai;

	repeat
	{
		if ( !SV_Object_IsValid(goal->aigcs.targetObj) )
		{
			goal->done = true;
			goal->error = AIGOAL_ERR_TARGETINVALID;

			return;
		}

		// $todo: tactic: back off after flurry

		SV_AIState_Update(goal->state);

		if ( GOAL_ATTACKMELEE_TASK_CHASING == goal->task )
		{
			if ( SV_AIState_IsDone(goal->state) )
			{
				// $todo: only attack if close enough!
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_ATTACKMELEE;
				aiscs.targetObj = goal->aigcs.targetObj;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_ATTACKMELEE_TASK_ATTACKING;
			}
			else
			{
				// check to see if we should melee
					
				if ( SV_AI_IsInRange(ai, goal->aigcs.targetObj, SV_AI_GetMeleeRange(ai)) )
				{
					if ( fabs(goal->aigcs.targetObj->base.pos[2] - obj->base.pos[2]) > 30.0f )
					{
						aiStateCreateStruct_t aiscs = { 0 };
						aiscs.ai = goal->aigcs.ai;
						aiscs.state = AISTATE_IDLE;
						SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

						goal->task = GOAL_ATTACKMELEE_TASK_WAITING;
						goal->timeNextChase = sl.gametime + 0.5f;
					}
					else
					{
						aiStateCreateStruct_t aiscs = { 0 };
						aiscs.ai = goal->aigcs.ai;
						aiscs.state = AISTATE_ATTACKMELEE;
						aiscs.targetObj = goal->aigcs.targetObj;
						SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

						goal->task = GOAL_ATTACKMELEE_TASK_ATTACKING;
					}
				}
				else
				{
					// $todo: only check this intermittently, and possibly optimize
					// see if anyone is threatening us

					if ( sl.gametime > goal->timeNextDodge )
					{
						if ( M_Randnum(0, 1) < unit->dodgeChance )
						{
							serverObject_t* threat = SV_AI_FindThreat(goal->aigcs.ai);
							if ( threat )
							{
								// dodge the threat
								SV_AINav_Dodge(goal->aigcs.ai->nav, threat);
							}
						}

						goal->timeNextDodge = sl.gametime + SV_AI_RandomTime(unit->dodgeRestTime.min, unit->dodgeRestTime.max);
					}

					return;
				}
			}
		}
		else if ( GOAL_ATTACKMELEE_TASK_ATTACKING == goal->task )
		{
			if ( sl.gametime > goal->timeNextChase )
			{
				if ( SV_AIState_GetError(goal->state) == AISTATE_ERR_TARGETOUTOFRANGE )
				{
					aiStateCreateStruct_t aiscs = { 0 };
					aiscs.ai = goal->aigcs.ai;
					aiscs.state = AISTATE_CHASE;
					aiscs.targetObj = goal->aigcs.targetObj;
					aiscs.chaseRange = SV_AI_GetMeleeRange(goal->aigcs.ai);
					SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

					goal->task = GOAL_ATTACKMELEE_TASK_CHASING;
					goal->timeNextChase = sl.gametime + 0.1f;
				}
				else if ( fabs(goal->aigcs.targetObj->base.pos[2] - obj->base.pos[2]) > 30.0f )
				{
					aiStateCreateStruct_t aiscs = { 0 };
					aiscs.ai = goal->aigcs.ai;
					aiscs.state = AISTATE_IDLE;
					SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

					goal->task = GOAL_ATTACKMELEE_TASK_WAITING;
					goal->timeNextChase = sl.gametime + 0.5f;
				}
				else
				{
					return;
				}
			}
			else
			{
				return;
			}
		}
		else if ( GOAL_ATTACKMELEE_TASK_WAITING == goal->task )
		{
			if ( sl.gametime > goal->timeNextChase )
			{
				if ( SV_AI_IsInRange(ai, goal->aigcs.targetObj, SV_AI_GetMeleeRange(ai)) )
				{
					if ( fabs(goal->aigcs.targetObj->base.pos[2] - obj->base.pos[2]) > 30.0f )
					{
						return;
					}
					else
					{
						aiStateCreateStruct_t aiscs = { 0 };
						aiscs.ai = goal->aigcs.ai;
						aiscs.state = AISTATE_ATTACKMELEE;
						aiscs.targetObj = goal->aigcs.targetObj;
						SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

						goal->task = GOAL_ATTACKMELEE_TASK_ATTACKING;
					}
				}
				else
				{
					aiStateCreateStruct_t aiscs = { 0 };
					aiscs.ai = goal->aigcs.ai;
					aiscs.state = AISTATE_CHASE;
					aiscs.targetObj = goal->aigcs.targetObj;
					aiscs.chaseRange = SV_AI_GetMeleeRange(goal->aigcs.ai);
					SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

					goal->task = GOAL_ATTACKMELEE_TASK_CHASING;
					goal->timeNextChase = sl.gametime + 0.5f;
				}
			}
			else
			{
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoalAttackMissile_t);

//----------------------------------------------------------------------------

const int GOAL_ATTACKMISSILE_TASK_NONE = -1;
const int GOAL_ATTACKMISSILE_TASK_CHASING = 0;
const int GOAL_ATTACKMISSILE_TASK_ATTACKING = 1;

//----------------------------------------------------------------------------

void SV_AIGoalAttackMissile_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_CHASE;
	aiscs.targetObj = goal->aigcs.targetObj;
	aiscs.chaseRange = 300.0f;
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

	goal->task = GOAL_ATTACKMISSILE_TASK_CHASING;
}

//----------------------------------------------------------------------------

void SV_AIGoalAttackMissile_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalAttackMissile_Update(aiGoal_t* goal)
{
	//serverObject_t* obj = goal->aigcs.ai->aics.obj;
	//objectData_t *unit = &objData[obj->base.type];

	repeat
	{
		if ( !SV_Object_IsValid(goal->aigcs.targetObj) )
		{
			goal->done = true;
			goal->error = AIGOAL_ERR_TARGETINVALID;

			return;
		}

		SV_AIState_Update(goal->state);

		//

		if ( GOAL_ATTACKMISSILE_TASK_CHASING == goal->task )
		{
			if ( SV_AIState_IsDone(goal->state) )
			{
				// $todo: only attack if close enough!
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_ATTACKMISSILE;
				aiscs.targetObj = goal->aigcs.targetObj;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_ATTACKMISSILE_TASK_ATTACKING;
			}
			else
			{
				return;
			}
		}
		else if ( GOAL_ATTACKMISSILE_TASK_ATTACKING == goal->task )
		{
			if ( SV_AIState_GetError(goal->state) == AISTATE_ERR_TARGETOUTOFRANGE )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_CHASE;
				aiscs.targetObj = goal->aigcs.targetObj;
				aiscs.chaseRange = 300.0f;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_ATTACKMISSILE_TASK_CHASING;
			}
			else
			{
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoalAttackPound_t);

//----------------------------------------------------------------------------

const int GOAL_ATTACKPOUND_TASK_NONE = -1;
const int GOAL_ATTACKPOUND_TASK_CHASING = 0;
const int GOAL_ATTACKPOUND_TASK_ATTACKING = 1;

//----------------------------------------------------------------------------

void SV_AIGoalAttackPound_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_CHASE;
	aiscs.targetObj = goal->aigcs.targetObj;
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

	goal->task = GOAL_ATTACKPOUND_TASK_CHASING;
}

//----------------------------------------------------------------------------

void SV_AIGoalAttackPound_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalAttackPound_Update(aiGoal_t* goal)
{
	serverObject_t* obj = goal->aigcs.ai->aics.obj;
	objectData_t *unit = &objData[obj->base.type];
	ai_t* ai = obj->ai;

	repeat
	{
		if ( !SV_Object_IsValid(goal->aigcs.targetObj) )
		{
			goal->done = true;
			goal->error = AIGOAL_ERR_TARGETINVALID;

			return;
		}

		SV_AIState_Update(goal->state);

		if ( GOAL_ATTACKPOUND_TASK_CHASING == goal->task )
		{
			// check to see if we should detonate
			if ( SV_AI_IsInRange(ai, goal->aigcs.targetObj, SV_AI_GetMeleeRange(ai)) )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_ATTACKPOUND;
				aiscs.targetObj = goal->aigcs.targetObj;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_ATTACKPOUND_TASK_ATTACKING;
			}
			else
			{
				if ( SV_AIState_IsDone(goal->state) )
				{
					aiStateCreateStruct_t aiscs = { 0 };
					aiscs.ai = goal->aigcs.ai;
					aiscs.state = AISTATE_CHASE;
					aiscs.targetObj = goal->aigcs.targetObj;
					SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);
				}
				else
				{
					return;
				}
			}
		}
		else if ( GOAL_ATTACKPOUND_TASK_ATTACKING == goal->task )
		{
			if ( SV_AIState_IsDone(goal->state) )
			{
				goal->done = true;
			}

			return;
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoalAttackSuicide_t);

//----------------------------------------------------------------------------

const int GOAL_ATTACKSUICIDE_TASK_NONE = -1;
const int GOAL_ATTACKSUICIDE_TASK_CHASING = 0;
const int GOAL_ATTACKSUICIDE_TASK_ATTACKING = 1;

//----------------------------------------------------------------------------

void SV_AIGoalAttackSuicide_Init(aiGoal_t* goal)
{
	serverObject_t* obj = goal->aigcs.ai->aics.obj;
	objectData_t *unit = &objData[obj->base.type];

	float range = M_Randnum(unit->attackSuicideRange.min, unit->attackSuicideRange.max);
	float rangesqr = range*range;

	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_CHASE;
	aiscs.targetObj = goal->aigcs.targetObj;
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);
	goal->task = GOAL_ATTACKSUICIDE_TASK_CHASING;

	goal->rangesqr = rangesqr;
}

//----------------------------------------------------------------------------

void SV_AIGoalAttackSuicide_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalAttackSuicide_Update(aiGoal_t* goal)
{
	//serverObject_t* obj = goal->aigcs.ai->aics.obj;
	//objectData_t *unit = &objData[obj->base.type];

	repeat
	{
		if ( !SV_Object_IsValid(goal->aigcs.targetObj) )
		{
			goal->done = true;
			goal->error = AIGOAL_ERR_TARGETINVALID;

			return;
		}

		SV_AIState_Update(goal->state);

		if ( GOAL_ATTACKSUICIDE_TASK_CHASING == goal->task )
		{
			float distsqr = M_GetDistanceSqVec2(goal->aigcs.ai->aics.obj->base.pos, goal->aigcs.targetObj->base.pos);

			// check to see if we should detonate
			if ( distsqr < goal->rangesqr )
			{
				aiStateCreateStruct_t aiscs = { 0 };
				aiscs.ai = goal->aigcs.ai;
				aiscs.state = AISTATE_ATTACKSUICIDE;
				aiscs.targetObj = goal->aigcs.targetObj;
				SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);

				goal->task = GOAL_ATTACKSUICIDE_TASK_ATTACKING;
			}
			else
			{
				if ( SV_AIState_IsDone(goal->state) )
				{
					aiStateCreateStruct_t aiscs = { 0 };
					aiscs.ai = goal->aigcs.ai;
					aiscs.state = AISTATE_CHASE;
					aiscs.targetObj = goal->aigcs.targetObj;
					SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);
				}
				else
				{
					return;
				}
			}
		}
		else if ( GOAL_ATTACKSUICIDE_TASK_ATTACKING == goal->task )
		{
			if ( SV_AIState_IsDone(goal->state) )
			{
				goal->done = true;
			}

			return;
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void SV_AIGoalIdle_Init(aiGoal_t* goal)
{
	aiStateCreateStruct_t aiscs = { 0 };
	aiscs.ai = goal->aigcs.ai;
	aiscs.state = AISTATE_IDLE;
	SV_AIGoal_ChangeState((aiGoal_t*)goal, &aiscs);
}

//----------------------------------------------------------------------------

void SV_AIGoalIdle_Term(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------

void SV_AIGoalIdle_Update(aiGoal_t* goal)
{
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiGoal_t);

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct
{
	int					type;
	SV_AIGoal_FnInit	init;
	SV_AIGoal_FnTerm	term;
	SV_AIGoal_FnInit	update;
}
aiFactoryGoal_t;

#define AIFACTORYGOAL(g, n) \
	{ AIGOAL_##g,	(SV_AIGoal_FnInit)SV_AIGoal##n##_Init,	(SV_AIGoal_FnTerm)SV_AIGoal##n##_Term,	(SV_AIGoal_FnUpdate)SV_AIGoal##n##_Update },

static aiFactoryGoal_t factories[] = 
{
	AIFACTORYGOAL(IDLE, Idle)
	AIFACTORYGOAL(GOTO, Goto)
	AIFACTORYGOAL(FOLLOW, Follow)
	AIFACTORYGOAL(FLEE, Flee)
	AIFACTORYGOAL(CONSTRUCT, Construct)
	AIFACTORYGOAL(MINE, Mine)
	AIFACTORYGOAL(ATTACKMELEE, AttackMelee)
	AIFACTORYGOAL(ATTACKMISSILE, AttackMissile)
	AIFACTORYGOAL(ATTACKPOUND, AttackPound)
	AIFACTORYGOAL(ATTACKSUICIDE, AttackSuicide)
};

//static int num_factories = sizeof(factories)/sizeof(aiFactoryGoal_t);

//----------------------------------------------------------------------------

aiGoal_t* SV_AIGoal_Create(const aiGoalCreateStruct_t* aigcs)
{
	aiGoal_t* goal = NULL;

	goal = ALLOCATE(aiGoal_t);
	goal->init = factories[aigcs->goal].init;
	goal->term = factories[aigcs->goal].term;
	goal->update = factories[aigcs->goal].update;
	goal->aigcs = *aigcs;
	goal->state = NULL;
	goal->done = false;
	goal->error = 0;
	goal->task = 0;
	goal->dropoff = NULL;
	goal->timeNextFollow = 0;
	goal->timeNextFled = 0;
	goal->timeNextDodge = 0;
	goal->timeNextChase = 0;

	SV_AIGoal_Init(goal);

	return goal;
}

//----------------------------------------------------------------------------

void SV_AIGoal_Destroy(aiGoal_t* goal)
{
	SV_AIGoal_Term(goal);

	DEALLOCATE(aiGoal_t, goal);
}
