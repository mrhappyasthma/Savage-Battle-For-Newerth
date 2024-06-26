#ifndef SV_AI_H
#define SV_AI_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include "sv_ainav.h"
#include "sv_aigoal.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct aiCreateStruct_t
{
	serverObject_t*		obj;
}
aiCreateStruct_t;

//----------------------------------------------------------------------------

typedef struct ai_s
{
	aiCreateStruct_t	aics;

	struct aiNav_s*		nav;
	struct aiGoal_s*	goal;

	float				priority;
 	int					timeNextWork;
	int					timeNextAggression;

	bool				destroy;

//	serverObject_t*		enemy;
}
ai_t;

//----------------------------------------------------------------------------

ai_t* SV_AI_Create(const aiCreateStruct_t* aics);
void SV_AI_Destroy(ai_t* ai);

//----------------------------------------------------------------------------

void SV_AI_Update(ai_t* ai);

void SV_AI_ChangeGoal(ai_t* ai, const aiGoalCreateStruct_t* aigcs);
void SV_AI_EnterGoal(ai_t* ai, const aiGoalCreateStruct_t* aigcs);
void SV_AI_ExitGoal(ai_t* ai);

bool SV_AI_IsPeon(ai_t* ai);

bool SV_AI_IsAttacking(ai_t* ai);

bool SV_AI_HasResources(ai_t* ai);

bool SV_AI_IsMoving(ai_t* ai);
float* SV_AI_GetMoveTo(ai_t* ai);

bool SV_AI_GetRandomPointCloserTo(ai_t* ai, float range[2], serverObject_t* to, vec2_t point);
bool SV_AI_GetRandomPointFartherFrom(ai_t* ai, float range[2], serverObject_t* from, vec2_t point);

serverObject_t* SV_AI_FindThreat(ai_t* ai);
float SV_AI_GetMeleeRange(ai_t* ai);
int SV_AI_CalculateAttackGoal(ai_t* ai, serverObject_t* target);
bool SV_AI_IsHigherCollisionPriority(ai_t* ai, ai_t* ai_other);

bool SV_AI_IsVisible(ai_t* ai, serverObject_t* target);
bool SV_AI_IsInRange(ai_t* ai, serverObject_t* target, float range);
bool SV_AI_IsInVisibleRange(ai_t* ai, serverObject_t* target, float range);

void SV_AI_HandleCollision(ai_t* ai, traceinfo_t* collision);
void SV_AI_HandleDamage(ai_t* ai, serverObject_t* attacker, int weapon, int attackDamage);
void SV_AI_HandleDeath(ai_t* ai, serverObject_t* attacker, int weapon, int attackDamage);

int SV_AIDamaged(serverObject_t *obj, serverObject_t *attacker, vec3_t pos, int weapon, int attackDamage, int damageFlags);

#endif
