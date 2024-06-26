#ifndef SV_AISTATE_H
#define SV_AISTATE_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct aiStateCreateStruct_t
{
	int				state;
	struct ai_s*	ai;
	vec2_t			targetPos;
	serverObject_t*	targetObj;

	// chase

	float			chaseRange;
}
aiStateCreateStruct_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef void (*SV_AIState_FnInit)(struct aiState_s* state); 
typedef void (*SV_AIState_FnTerm)(struct aiState_s* state); 
typedef void (*SV_AIState_FnUpdate)(struct aiState_s* state); 

//----------------------------------------------------------------------------

typedef struct aiState_s
{
	SV_AIState_FnInit		init;
	SV_AIState_FnTerm		term;
	SV_AIState_FnUpdate		update;
	aiStateCreateStruct_t	aiscs;
	int						error;
	bool					done;

	// attackmelee, attacksuicide

	int		timeAttack;
	bool	impacted;
	int		timeImpacted;
	bool	attacked;
	int		timeAttacked;

	// chase

	int		timeRepath;

	// construct

	bool	working;
}
aiState_t;

//----------------------------------------------------------------------------

#define AISTATE_IDLE			0
#define AISTATE_GOTO			1
#define AISTATE_CHASE			2
#define AISTATE_MINE			3
#define AISTATE_CONSTRUCT		4
#define AISTATE_ATTACKMELEE		5
#define AISTATE_ATTACKMISSILE	6
#define AISTATE_ATTACKPOUND		7
#define AISTATE_ATTACKSUICIDE	8

//----------------------------------------------------------------------------

#define AISTATE_ERR_TARGETINVALID		10
#define AISTATE_ERR_TARGETOUTOFRANGE	20
#define AISTATE_ERR_TARGETNOTVISIBLE	30

//----------------------------------------------------------------------------

aiState_t* SV_AIState_Create(const aiStateCreateStruct_t* aiscs);
void SV_AIState_Destroy(aiState_t* pState);

bool SV_AIState_IsDone(aiState_t* state);
int SV_AIState_GetError(aiState_t* state);

void SV_AIState_Update(aiState_t* self);

#endif
