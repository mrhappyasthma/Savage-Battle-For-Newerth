/*
 * (C) 2002 S2 Games
 *
 * cmdr_requests.c'
 */

#include "client_game.h"

// 
//=============================================================================



/*==========================

  CMDR_ActivateUpgrade_Cmd

  Commander uses this command to apply a state to a player
  Syntax: activateUpgrade <upgradename|upgradenumber> [clientnumber]

 ==========================*/

void	CMDR_ActivateUpgrade_Cmd(int argc, char *argv[])
{
	int type, targetnum = -1;

	//must specify upgrade
	if (argc < 1)
	{
		corec.Console_Printf("No upgrade specified\n");
		return;
	}
	
	//determine upgrade
	SetInt(&type, FIRST_OBJECT_TYPE, LAST_OBJECT_TYPE, cl.objNames, argv[0]);
	if (type < 1 || type > MAX_OBJECT_TYPES)
	{
		corec.Console_Printf("Invalid upgrade\n");
		return;
	}

	//get target
	if (argc < 2)
	{
		if (cl.selection.numSelected != 1 || 
			!IsUnitType(cl.objects[cl.selection.array[0]].visual.type) ||
			cl.selection.array[0] < 0 || cl.selection.array[0] >= MAX_OBJECTS)
		{
			corec.Console_Printf("Must select a valid unit\n");
			return;
		}

		//We're trying to hit the selected unit
		targetnum = cl.selection.array[0];
	}
	else
	{
		//clientnum was specified
		if (!SetInt(&targetnum, 0, MAX_OBJECTS-1, NULL, argv[0]))
		{
			corec.Console_Printf("Invalid client numbers\n");
			return;
		}
	}

	//check availability
	if (Tech_IsResearched(type, cl.research))
		corec.Client_SendMessageToServer(fmt("%s %i %i", CLIENT_COMMANDER_ACTIVATE_MSG, type, targetnum));
	else
		corec.Console_Printf("Upgrade has not been researched.\n");
}

bool	CMDR_ActivateTech()
{
	int objIndex;

	objIndex = CL_GetObjectUnderCursor();

	//filter out any non-hits
	if (objIndex < 0 || objIndex > MAX_OBJECTS)	//didn't hit anything
	{
		if (!(CL_ObjectType(cl.cmdrActiveTech)->useTarget & TARGET_WORLD))
			return false;
	}

	if (cl.objects[objIndex].base.team == cl.info->team)	//friendly unit
	{
		if (!(CL_ObjectType(cl.cmdrActiveTech)->useTarget & TARGET_TEAM))
			return false;
	}
	else if (cl.objects[objIndex].base.team != cl.info->team)	//enemy
	{
		if (!(CL_ObjectType(cl.cmdrActiveTech)->useTarget & TARGET_ENEMY))
			return false;
	}

	if (IsBuildingType(cl.objects[objIndex].base.type) && !(CL_ObjectType(cl.cmdrActiveTech)->useTarget & TARGET_BUILDING))
		return false;
	
	if (IsMobileType(cl.objects[objIndex].base.type) && !(CL_ObjectType(cl.cmdrActiveTech)->useTarget & TARGET_UNIT))
		return false;

	//CL_SendActivateRequest(cl.clientnum, cl.cmdrActiveTech, objIndex);
	return true;
}


/*==========================

  CMDR_InitUpgrades

  Register cvars/commands relevant to comamnder issued powerups

 ==========================*/

void CMDR_InitUpgrades()
{
	corec.Cmd_Register("activateUpgrade",	CMDR_ActivateUpgrade_Cmd);
}
