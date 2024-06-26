// (C) 2001 S2 Games

// trajectory.c

// functions to evaluate trajectory

#include "game.h"

// Traj_GetPos
// Set's <out> to the position indicated by <traj> at <time>
//=============================================================================
void	Traj_GetPos(trajectory_t *traj, int time, vec3_t out)
{
	float	deltatime = (time - traj->startTime) / (float)1000;
	float	acceltime = deltatime * deltatime * 0.5; // 1/2t^2
	vec3_t	result;

	if (!out)
		return;

	//initial possition
	M_CopyVec3(traj->origin, out);

	//velocity
	M_MultVec3(traj->velocity, deltatime, result);
	M_AddVec3(out, result, out);

	//linear acceleration
	M_CopyVec3(traj->velocity, result);
	M_Normalize(result);
	M_MultVec3(result, (acceltime * traj->acceleration), result);
	M_AddVec3(out, result, out);

	//gravity acceleration
	M_SetVec3(result, 0, 0, -1);
	M_MultVec3(result, (acceltime * (traj->gravity * DEFAULT_GRAVITY * PHYSICS_SCALE)), result);
	M_AddVec3(out, result, out);
}

// Traj_GetVelocity
// Set's <out> to a vector representiing the velocity of <traj> at <time>
// return value is the magnitude of <out>
//=============================================================================
float	Traj_GetVelocity(trajectory_t *traj, int time, vec3_t out)
{
	float	deltatime = (time - traj->startTime) / (float)1000;
	vec3_t	vec, result;

	//inintial velocity
	M_CopyVec3(traj->velocity, result);

	//linear acceleration
	M_CopyVec3(traj->velocity, vec);
	M_Normalize(vec);
	M_MultVec3(vec, deltatime * traj->acceleration, vec);
	M_AddVec3(vec, result, result);

	//gravity acceleration
	M_SetVec3(vec, 0, 0, -1);
	M_MultVec3(vec, deltatime * (traj->gravity * DEFAULT_GRAVITY * PHYSICS_SCALE), vec);
	M_AddVec3(vec, result, result);

	if (out)
		M_CopyVec3(result, out);

	return M_GetVec3Length(result);
}