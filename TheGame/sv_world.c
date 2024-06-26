// (C) 2001 S2 Games

// sv_world.c

// various world operations

#include "server_game.h"

extern cvar_t sv_npcs;


/*==========================

  SV_GetDistanceSq

  this returns the squared distance, for speed. 
  If you need the real dist, you need to do the sqrt yourself

 ==========================*/

float   SV_GetDistanceSq(int obj1, int obj2)
{
	vec3_t diff;

	M_SubVec3(sl.objects[obj1].base.pos, sl.objects[obj2].base.pos, diff);
	return (M_DotProduct(diff, diff));
}


/*==========================

	SV_GetHighestSolidPoint

	return the highest Z for a given X and Y

 ==========================*/

float	SV_GetHighestSolidPoint(float x, float y)
{
	vec3_t wayUpThere = { x, y, 99999 };
	vec3_t wayDownHere = { x, y, -99999 };
	traceinfo_t trace;

	cores.World_TraceBox(&trace, wayUpThere, wayDownHere, zero_vec, zero_vec, 0);

	return trace.endpos[Z];
}


/*==========================

  SV_SpawnReferenceObject

 ==========================*/

serverObject_t	*SV_SpawnReferenceObject(referenceObject_t *obj, int type)
{
	int team;
	int lives;

	vec3_t angle = { obj->pos.rotation[X], obj->pos.rotation[Y], obj->pos.rotation[Z] };	
	serverObject_t *ret;

	if (ST_FindState(obj->info, "team"))
	{
		team = atoi(ST_GetState(obj->info, "team"));
		if (team < 0 || team >= MAX_TEAMS)
			team = 0;
	}
	else
	{
		team = 0;
	}

	if (ST_FindState(obj->info, "lives"))
	{
		lives = atoi(ST_GetState(obj->info, "lives"));
	}
	else
	{
		//use the unit info lives
		lives = GetObjectByType(type)->totalLives;
		if (lives <= 0)
			lives = -1;
	}

	cores.Console_DPrintf("spawning reference object on team %i with %i lives\n", team, lives);
	
	ret = SV_SpawnObject(team, type, obj->pos.position, angle);

	ret->refObject = *obj;
	ret->livesLeft = lives;
	
	return ret;
}


/*==========================

  SV_SpawnReferenceObjects

  spawn dynamic objects that were placed down in the level editor.

 ==========================*/

void	SV_SpawnReferenceObjects()
{
	int n;
	int index;

	//spawn any building types that aren't command centers

	if (sl.gametype == GAMETYPE_RTSS)
	{
		index = -1;

		for (n = 0; n < MAX_OBJECT_TYPES; n++)
		{	
			referenceObject_t obj;
			objectData_t	*bld = GetObjectByType(n);

			if (bld->objclass != OBJCLASS_BUILDING || bld->commandCenter)	//command centers don't spawn until a game starts
					continue;

			while(1)
			{	
				index = cores.WO_FindReferenceObject(index+1, GetObjectByType(n)->name, &obj);
				if (index == -1)
					break;
				
				SV_SpawnReferenceObject(&obj, n);
			}
		}

		//spawn NPCs

		if (!sv_npcs.integer)
			return;

		index = -1;

		for (n = 1; n < MAX_OBJECT_TYPES; n++)
		{
			referenceObject_t obj;
			objectData_t *unit = GetObjectByType(n);

			if (unit->objclass != OBJCLASS_UNIT)
				continue;

			while(1)
			{
				serverObject_t	*spawnobj;

				index = cores.WO_FindReferenceObject(index+1, objectNames[n], &obj);
				if (index == -1)
					break;		
				
				spawnobj = SV_SpawnReferenceObject(&obj, n);
			}
		}
	}
	else if (sl.gametype == GAMETYPE_DEATHMATCH)
	{
		referenceObject_t obj;

		//search for spawnpoints (just make sure there is at least one)
		index = cores.WO_FindReferenceObject(0, "spawnpoint", &obj);
		if (index == -1)
			core.Game_Error("Couldn't find any spawnpoints in world for deathmatch game\n");

		index = -1;

		sl.numSpawnPoints = 0;

		while (1)
		{
			index = cores.WO_FindReferenceObject(index+1, "spawnpoint", &obj);
			if (index == -1)
				break;

			M_CopyVec3(obj.pos.position, sl.spawnPoints[sl.numSpawnPoints]);
			sl.spawnPoints[sl.numSpawnPoints][2] += 10;		//fudge factor
			sl.numSpawnPoints++;
			if (sl.numSpawnPoints >= MAX_SPAWNPOINTS)
				break;
		}

		//spawn items
		for (n = 1; n < MAX_OBJECT_TYPES; n++)
		{
			referenceObject_t obj;

			if (objData[n].objclass != OBJCLASS_ITEM)
				continue;

			while(1)
			{
				serverObject_t	*spawnobj;

				index = cores.WO_FindReferenceObject(index+1, objectNames[n], &obj);
				if (index == -1)
					break;

				spawnobj = SV_SpawnReferenceObject(&obj, (byte)n);
			}
		}

	}
}
