#ifndef SV_AIGOAL_H
#define SV_AIGOAL_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct aiGoalCreateStruct_t
{
	int				goal;
	struct ai_s*	ai;
	vec2_t			targetPos;
	serverObject_t*	targetObj;

	// flee

	serverObject_t*	fleeingObj;
}
aiGoalCreateStruct_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef void (*SV_AIGoal_FnInit)(struct aiGoal_s* goal); 
typedef void (*SV_AIGoal_FnTerm)(struct aiGoal_s* goal); 
typedef void (*SV_AIGoal_FnUpdate)(struct aiGoal_s* goal); 

//----------------------------------------------------------------------------

typedef struct aiGoal_s
{
	// all goals

	SV_AIGoal_FnInit		init;
	SV_AIGoal_FnTerm		term;
	SV_AIGoal_FnUpdate		update;
	aiGoalCreateStruct_t	aigcs;
	struct aiState_s*		state;
	bool					done;
	int						error;
	
	// follow, build, repair, mine, attackmelee, attackmissile

	int						task;

	// mine

	serverObject_t*			dropoff;

	// flee

	int						timeNextFled;

	// follow, flee

	int						timeNextFollow;

	// attackmelee

	int						timeNextChase;
	int						timeNextDodge;

	// attacksuicide

	float					rangesqr;
}
aiGoal_t;

//----------------------------------------------------------------------------

#define AIGOAL_IDLE				0
#define AIGOAL_GOTO				1
#define AIGOAL_FOLLOW			2
#define AIGOAL_FLEE				3
#define AIGOAL_CONSTRUCT		4
#define AIGOAL_MINE				5
#define AIGOAL_ATTACKMELEE		6
#define AIGOAL_ATTACKMISSILE	7
#define AIGOAL_ATTACKPOUND		8
#define AIGOAL_ATTACKSUICIDE	9

//----------------------------------------------------------------------------

#define AIGOAL_ERR_TARGETINVALID		10

//----------------------------------------------------------------------------

aiGoal_t* SV_AIGoal_Create(const aiGoalCreateStruct_t* aigcs);
void SV_AIGoal_Destroy(aiGoal_t* goal);

bool SV_AIGoal_Is(aiGoal_t* goal, int goalType);

bool SV_AIGoal_IsDone(aiGoal_t* goal);
int SV_AIGoal_GetError(aiGoal_t* goal);

serverObject_t* SV_AIGoal_GetObject(aiGoal_t* goal);

//----------------------------------------------------------------------------

void SV_AIGoal_Update(aiGoal_t* goal);

#endif
