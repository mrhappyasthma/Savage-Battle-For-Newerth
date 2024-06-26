// (C) 2001 S2 Games

// units.c

// all data specific to in game characters (nomad units, beast units, NPCs)

#include "game.h"
//=============================================================================

int	attackableUnits[MAX_OBJECT_TYPES];
int	numAttackableUnits = 0;
int	allTeamUnits[MAX_OBJECT_TYPES];
int numTeamUnits = 0;

extern objectData_t objData[MAX_OBJECT_TYPES];

void	PostProcessUnits()
{
	int i;
	
	for (i = 1; i < MAX_OBJECT_TYPES; i++)
	{
		if (objData[i].objclass != OBJCLASS_UNIT)
			continue;

		if (objData[i].race == RACE_UNDECIDED)
			continue;

		attackableUnits[numAttackableUnits++] = i;
	}
	for (i = 0; i < MAX_OBJECT_TYPES; i++)
	{
		if (objData[i].objclass == OBJCLASS_BUILDING && objData[i].race == RACE_UNDECIDED)
			continue;

		if (objData[i].objclass == OBJCLASS_NULL)
			continue;

		allTeamUnits[numTeamUnits++] = i;
	}
}
