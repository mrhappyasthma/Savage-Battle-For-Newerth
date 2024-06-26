#include "server_game.h"
#include "sv_aiutil.h"
#include "sv_ainav.h"
#include "sv_ai.h"

//----------------------------------------------------------------------------

#define AINAV_RESULT_NONE		1
#define AINAV_RESULT_ARRIVED	2
#define AINAV_RESULT_BLOCKED	3

//----------------------------------------------------------------------------

IMPLEMENT_ALLOCATOR(aiNav_t);

//----------------------------------------------------------------------------

void SV_AINav_Init(aiNav_t* nav)
{
}

//----------------------------------------------------------------------------

void SV_AINav_Term(aiNav_t* nav)
{
	if ( nav->path )
	{
		cores.NavRep_PathDestroy(navmesh_small, nav->path);
		nav->path = NULL;
	}
}

//----------------------------------------------------------------------------

void SV_AINav_Update(aiNav_t* nav)
{
	serverObject_t* obj = nav->aincs.ai->aics.obj;
	objectData_t *unit = &objData[obj->base.type];

	// check for repath

	if ( nav->repath && sl.gametime > nav->timeNextRepath )
	{
		if ( nav->object )
		{
			SV_AINav_GotoObject(nav, nav->object, nav->close, nav->blockapprox);
		}
		else
		{
			SV_AINav_GotoPosition(nav, nav->destination);
		}

		if ( nav->repath )
		{
			// we're stuck, there's no way to get where we want to go

			AI_PRINTF("AI is stuck repathing...");
		}
	}

	// update our position in the path struct

	if ( nav->path )
	{
		M_CopyVec2(obj->base.pos, nav->path->pos);
	}

	// dodge?

	if ( SV_AINav_IsDodging(nav) )
	{
		SV_AINav_FacePos(nav, nav->dodgeObj->base.pos);

		// check for waypoint advancement

		if ( M_GetDistanceSqVec2(obj->base.pos, nav->dodgePos) <= MAX(unit->speed * unit->speed, 25) )
		{
			nav->dodge = AINAV_DODGE_NONE;
		}
	}

	// check our blocked status

	if ( nav->blocker )
	{
		if ( nav->blocked < sl.gametime )
		{
			nav->blocker = NULL;
		}
		else
		{
			return;
		}
	}

	// don't update our path if we're dodging

	if ( SV_AINav_IsDodging(nav) ) 
	{
		return;
	}

	// see if we should advance our path

	if ( nav->path )
	{
		bool approx = nav->path->approx;
		// check for waypoint advancement

		if ( M_GetDistanceSqVec2(obj->base.pos, nav->path->waypoint->position) <= MAX(unit->speed * unit->speed, 25) )
		{
			nav->path->waypoint = nav->path->waypoint->next;

			if ( nav->path->waypoint == NULL )
			{
				if ( approx && nav->blockapprox )
				{
					nav->repath = true;
					nav->timeNextRepath = sl.gametime + SV_AI_RandomTime(1000, 2000);

					SV_AINav_Block(nav, obj, nav->timeNextRepath + 100);
				}
				else
				{
					cores.NavRep_PathDestroy(navmesh_small/*obj->navmeshsize*/, nav->path);
					nav->path = NULL;
					nav->object = NULL;
					nav->result = AINAV_RESULT_ARRIVED;
					nav->close = false;
					nav->blockapprox = false;
				}
			}
			else
			{
				nav->bridge = nav->path->waypoint->bridge;
			}
		}
	}

	// see if we should optimize our path 

	if ( nav->path && nav->path->timeNextOptimize < sl.gametime )
	{
		nav->path->timeNextOptimize = sl.gametime + SV_AI_RandomTime(500, 1000);
		cores.NavRep_PathOptimize(navmesh_small/*obj->navmeshsize*/, nav->path);
	}
}

//----------------------------------------------------------------------------

void SV_AINav_GotoPosition(aiNav_t* nav, const vec2_t target)
{
	serverObject_t* obj = nav->aincs.ai->aics.obj;

	M_CopyVec2(target, nav->destination);
	nav->repath = false;
	nav->result = AINAV_RESULT_NONE;
	nav->object = NULL;
	nav->close = false;
	nav->blockapprox = false;

	if ( nav->path )
	{
		cores.NavRep_PathDestroy(navmesh_small/*obj->navmeshsize*/, nav->path);
		nav->path = NULL;
		// $TODO: might get null path with no result
	}

	if ( obj->subtracted )
	{
		cores.NavRep_CSGAdd(&obj->base);
		obj->subtracted = false;
	}

	nav->path = cores.NavRep_PathCreateToPosition(navmesh_small/*obj->navmeshsize*/, obj->base.pos, nav->bridge, target);
	if ( nav->path )
	{
		pointinfo_t pi;
		cores.World_SampleGround(nav->path->dest[0], nav->path->dest[1], &pi);

		// hack our destination position back into the serverObject
		M_CopyVec2(nav->path->dest, obj->posTarget);
		obj->posTarget[2] = pi.z;
	}

	if ( nav->path )
	{
		nav->path->timeNextOptimize = 0;
	}
	else
	{
		nav->repath = true;
		nav->timeNextRepath = sl.gametime + SV_AI_RandomTime(1000, 2000);
	}
}

//----------------------------------------------------------------------------

void SV_AINav_GotoObject(aiNav_t* nav, serverObject_t* target, bool close, bool blockapprox)
{
	serverObject_t* obj = nav->aincs.ai->aics.obj;

	nav->repath = false;
	nav->result = AINAV_RESULT_NONE;
	nav->object = target;
	nav->close = close;
	nav->blockapprox = blockapprox;

	if ( nav->path )
	{
		cores.NavRep_PathDestroy(navmesh_small/*obj->navmeshsize*/, nav->path);
		nav->path = NULL;
		// $TODO: might get null path with no result
	}

	if ( obj->subtracted )
	{
		cores.NavRep_CSGAdd(&obj->base);
		obj->subtracted = false;
	}

	nav->path = cores.NavRep_PathCreateToObject(navmesh_small/*obj->navmeshsize*/, obj->base.pos, nav->bridge, &target->base, close);
	if ( nav->path )
	{
		pointinfo_t pi;
		cores.World_SampleGround(nav->path->dest[0], nav->path->dest[1], &pi);

		// hack our destination position back into the serverObject
		M_CopyVec2(nav->path->dest, obj->posTarget);
		obj->posTarget[2] = pi.z;
	}

	if ( nav->path )
	{
		nav->path->timeNextOptimize = 0;
	}
	else
	{
		nav->repath = true;
		nav->timeNextRepath = sl.gametime + SV_AI_RandomTime(1000, 2000);
	}
}

//----------------------------------------------------------------------------

bool SV_AINav_IsMoving(aiNav_t* nav)
{
	if ( !nav->path ) return false;
	if ( nav->blocker ) return false;
//	if ( obj->blockedIndex != -1 ) return false;
//	if ( obj->base.animState != AS_WALK_FWD && obj->base.animState != AS_WALK_WITH_BAG ) return false;

	return true;
}

//----------------------------------------------------------------------------

float* SV_AINav_GetMoveTo(aiNav_t* nav)
{
	if ( SV_AINav_IsDodging(nav) )
	{
		return nav->dodgePos;
	}

	if ( nav->path )
	{
		return nav->path->waypoint->position;
	}

	return NULL;
}

//----------------------------------------------------------------------------

void SV_AINav_HandleCollision(aiNav_t* nav, traceinfo_t* collision)
{
	serverObject_t* obj_other;

	obj_other = &sl.objects[collision->index];
	if ( !obj_other ) return;

	// $todo: maybe we should stop dodging if we collide
	if ( SV_AINav_IsDodging(nav) ) return;

	if ( nav->path && nav->object && (obj_other == nav->object) )
	{
		cores.NavRep_PathDestroy(navmesh_small/*obj->navmeshsize*/, nav->path);
		nav->path = NULL;
		nav->object = NULL;
		nav->result = AINAV_RESULT_ARRIVED;
	}
	else if ( obj_other->ai && SV_AI_IsMoving(obj_other->ai) )
	{
		// if other has a path and is not blocked, block one and repathfind the other

		if ( SV_AI_IsHigherCollisionPriority(nav->aincs.ai, obj_other->ai) )
		{
			SV_AINav_Block(obj_other->ai->nav, nav->aincs.ai->aics.obj, sl.gametime + 500);
		}
		else
		{
			SV_AINav_Block(nav, obj_other, sl.gametime + 500);
		}
	}
	else if ( SV_AI_IsMoving(nav->aincs.ai) )
	{
		if ( nav->object ) 
		{
			SV_AINav_GotoObject(nav, nav->object, nav->close, nav->blockapprox);
		}
		else
		{
			SV_AINav_GotoPosition(nav, nav->destination);
		}
	}
}

//----------------------------------------------------------------------------

bool SV_AINav_IsArrived(aiNav_t* nav)
{
	return nav->result == AINAV_RESULT_ARRIVED;
}

//----------------------------------------------------------------------------

bool SV_AINav_IsBlocked(aiNav_t* nav)
{
	return nav->result == AINAV_RESULT_BLOCKED;
}

//----------------------------------------------------------------------------

void SV_AINav_Block(aiNav_t* nav, serverObject_t* blocker, int blocked)
{
	nav->blocker = blocker;
	nav->blocked = blocked;
}

//----------------------------------------------------------------------------

void SV_AINav_Dodge(aiNav_t* nav, serverObject_t* dodge)
{
	serverObject_t* obj = nav->aincs.ai->aics.obj;
	objectData_t *unit = &objData[obj->base.type];

	vec2_t dir, perp;
	float amount = M_Randnum(unit->dodgeDistance.min, unit->dodgeDistance.max);
	float distance;
	float leftright[2];
	int i;

	if ( SV_AINav_IsBridging(nav) ) return;
	if ( SV_AINav_IsDodging(nav) ) return;
	// $todo: allow dodging without path!
	if ( !nav->path ) return;
	
	M_SubVec2(nav->path->waypoint->position, nav->path->pos, dir);
	distance = M_NormalizeVec2(dir);
	
	if ( distance < amount ) return;

	distance = MIN(20.0f, distance*0.5f);
	M_SetVec2(perp, dir[1], -dir[0]);
	M_MultVec2(dir, distance, dir);

	leftright[0] = M_Randnum(0, 1) < 0.5f ? -1 : +1;
	leftright[1] = -leftright[0];

	for ( i = 0 ; i < 2 ; ++i )
	{
		M_MultVec2(perp, leftright[i]*amount, nav->dodgePos);
		M_AddVec2(nav->path->pos, nav->dodgePos, nav->dodgePos);
		M_AddVec2(dir, nav->dodgePos, nav->dodgePos);

		if ( cores.NavRep_Trace(navmesh_small, nav->path->pos, nav->dodgePos) && 
			 cores.NavRep_Trace(navmesh_small, nav->dodgePos, nav->path->waypoint->position) )
		{
			if ( leftright[0] < 0 )
			{
				nav->dodge = AINAV_DODGE_STRAFELEFT;
				nav->dodgeObj = dodge;
			}
			else
			{
				nav->dodge = AINAV_DODGE_STRAFERIGHT;
				nav->dodgeObj = dodge;
			}

			return;
		}
	}
}

//----------------------------------------------------------------------------

void SV_AINav_Stop(aiNav_t* nav)
{
	if ( nav->path )
	{
		cores.NavRep_PathDestroy(navmesh_small/*obj->navmeshsize*/, nav->path);
		nav->path = NULL;
	}
	
	nav->object = NULL;
	nav->close = false;
	nav->blockapprox = false;
	nav->result = AINAV_RESULT_NONE;
	nav->blocker = NULL;
}

//----------------------------------------------------------------------------

bool SV_AINav_IsDodging(aiNav_t* nav)
{
	return nav->dodge != AINAV_DODGE_NONE;
}

//----------------------------------------------------------------------------

int SV_AINav_GetDodge(aiNav_t* nav)
{
	return nav->dodge;
}

//----------------------------------------------------------------------------

bool SV_AINav_IsBridging(aiNav_t* nav)
{
	return nav->bridge ? true : false;
}

//----------------------------------------------------------------------------

void SV_AINav_FacePos(aiNav_t* nav, vec2_t pos)
{
	nav->face = true;
	M_CopyVec2(pos, nav->facePos);
}

//----------------------------------------------------------------------------

void SV_AINav_FaceDir(aiNav_t* nav, vec2_t dir)
{
	nav->face = true;
	M_CopyVec2(dir, nav->faceDir);
}

//----------------------------------------------------------------------------

aiNav_t* SV_AINav_Create(const aiNavCreateStruct_t* aincs)
{
	aiNav_t* nav = ALLOCATE(aiNav_t);
	nav->aincs = *aincs;
	nav->result = AINAV_RESULT_NONE;
	nav->path = NULL;
	nav->blocker = NULL;
	nav->dodge = AINAV_DODGE_NONE;
	nav->repath = false;
	nav->object = NULL;
	nav->close = false;
	nav->blockapprox = false;
	nav->bridge = NULL;
	SV_AINav_Init(nav);

	return nav;
}

//----------------------------------------------------------------------------

void SV_AINav_Destroy(aiNav_t* nav)
{
	SV_AINav_Term(nav);
	DEALLOCATE(aiNav_t, nav);
}
