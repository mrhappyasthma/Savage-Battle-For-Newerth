#ifndef SV_AINAV_H
#define SV_AINAV_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define AINAV_DODGE_NONE		1
#define AINAV_DODGE_STRAFELEFT	2
#define AINAV_DODGE_STRAFERIGHT 3

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

typedef struct aiNavCreateStruct_t
{
	struct ai_s*	ai;
}
aiNavCreateStruct_t;

//----------------------------------------------------------------------------

typedef struct aiNav_s
{
	aiNavCreateStruct_t		aincs;

	navpath_t*				path;
	int						result;

	vec2_t					destination;
	serverObject_t*			object;
	bool					close;
	bool					blockapprox;

	serverObject_t*			blocker;
	int						blocked;

	int						dodge;
	serverObject_t*			dodgeObj;
	vec2_t					dodgePos;

	struct navpoly_s*		bridge;

	bool					repath;
	int						timeNextRepath;

	bool					face;
	vec2_t					facePos;
	vec2_t					faceDir;
}
aiNav_t;

//----------------------------------------------------------------------------

aiNav_t* SV_AINav_Create(const aiNavCreateStruct_t* aincs);
void SV_AINav_Destroy(aiNav_t* nav);

//----------------------------------------------------------------------------

void SV_AINav_Update(aiNav_t* nav);

void SV_AINav_HandleCollision(aiNav_t* nav, traceinfo_t* collision);

void SV_AINav_Block(aiNav_t* nav, serverObject_t* blocker, int blocked);
bool SV_AINav_IsBlocked(aiNav_t* nav);

void SV_AINav_GotoPosition(aiNav_t* nav, const vec2_t position);
void SV_AINav_GotoObject(aiNav_t* nav, serverObject_t* obj, bool close, bool blockapprox);
void SV_AINav_Stop(aiNav_t* nav);

bool SV_AINav_IsBridging(aiNav_t* nav);

void SV_AINav_Dodge(aiNav_t* nav, serverObject_t* dodge);
bool SV_AINav_IsDodging(aiNav_t* nav);
int SV_AINav_GetDodge(aiNav_t* nav);

bool SV_AINav_IsMoving(aiNav_t* nav);
float* SV_AINav_GetMoveTo(aiNav_t* nav);
bool SV_AINav_IsArrived(aiNav_t* nav);

void SV_AINav_FacePos(aiNav_t* nav, vec2_t pos);
void SV_AINav_FaceDir(aiNav_t* nav, vec2_t dir);

#endif
