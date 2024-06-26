/*
 * (C) 2003 S2 Games
 *
 * cl_cmdr_officer.c'
 */

#include "client_game.h"

// console 'ask' command, and the functions that support it
//=============================================================================

cvar_t	answer = { "answer", "",	CVAR_READONLY };


/*==========================

  Ask_IsResearched_Fn

  syntax: ask isResearched <object name>
  returns: Number of instances if the team has researched this item

 ==========================*/

char	*Ask_IsResearched_Fn(int argc, char *argv[])
{
	if (cl.gametype == GAMETYPE_DEATHMATCH)
		return "1";

	if (!argc)
		return "0";

	return fmt("%i", Tech_IsResearched(CL_GetObjectTypeByName(argv[0]), cl.research));
}


/*==========================

  Ask_FileExists_Fn

  syntax: ask fileExists <file name>
  returns: "1" if the file exists, otherwise "0"

 ==========================*/

char	*Ask_FileExists_Fn(int argc, char *argv[])
{
	if (!argc)
		return "0";

	if (core.File_Exists(argv[0]))
		return "1";
	else
		return "0";
}


/*==========================

  Ask_Ammo_Fn

  syntax: ask ammo <slot number>
  returns: amount of ammo remaining for item in <slot number>, otherwise "-1"

 ==========================*/

char	*Ask_Ammo_Fn(int argc, char *argv[])
{
	int slot, ammo;

	if (argc < 1)
		return "-1";

	slot = atoi(argv[0]);
	if (slot < 0 || slot >= MAX_INVENTORY)
		return "-1";

	ammo = cl.predictedState.ammo[slot];

	return fmt("%i", ammo);
}


/*==========================

  Ask_Clip_Fn

  syntax: ask clip <slot number>
  returns: amount of ammo remaining the clip for item in <slot number>

 ==========================*/

char	*Ask_Clip_Fn(int argc, char *argv[])
{
	int slot, clip;

	if (argc < 1)
		return "-1";

	slot = atoi(argv[0]);
	if (slot < 0 || slot >= MAX_INVENTORY)
		return "-1";

	clip = cl.predictedState.clip[slot];

	return fmt("%i", clip);
}


/*==========================

  Ask_IsAvailable_Fn

  syntax: ask isAvailable <object name>
  returns: "1" if object is available for use/pruchase, otherwise "0"

 ==========================*/

char	*Ask_IsAvailable_Fn(int argc, char *argv[])
{
	if (cl.gametype == GAMETYPE_DEATHMATCH)
		return "1";

	if (!argc)
		return "0";

	if (Tech_IsAvailable(CL_GetObjectTypeByName(argv[0]), cl.research))
		return "1";
	else
		return "0";
}


/*==========================

  Ask_IsInInventory_Fn

  syntax: ask isInInventory <object name>
  returns: number of occurances of <object name> in client's inventory

 ==========================*/

char	*Ask_IsInInventory_Fn(int argc, char *argv[])
{
	int index, itemnum, count = 0;

	if (argc < 1)
		return "0";
	
	itemnum = CL_GetObjectTypeByName(argv[0]);
	if (!itemnum)
		return "0";

	for (index = 0; index < MAX_INVENTORY; index++)
	{
		if (cl.predictedState.inventory[index] == itemnum)
			count++;
	}
	return fmt("%i", count);
}


/*==========================

  Ask_Inventory_Fn

  syntax: ask inventorySlot <slot number>
  returns: name of item in inventory slot <slot number>, otherwise ""

 ==========================*/

char	*Ask_Inventory_Fn(int argc, char *argv[])
{
	int slotnum, objnum;

	if (argc < 1)
		return "";
	
	slotnum = atoi(argv[0]);
	if (slotnum < 0 || slotnum >= MAX_INVENTORY)
		return "";

	objnum = cl.predictedState.inventory[slotnum];

	if (objnum < 1)
		return "";

	return fmt("%s", CL_ObjectType(objnum)->name);
}


/*==========================

  Ask_CanPutInInventory_Fn

  syntax: ask canPutInInventory <object type>
  returns: "1" if it is possible to add the object, or the reason you can not

 ==========================*/

char	*Ask_CanPutInInventory_Fn(int argc, char *argv[])
{
	objectData_t	*unit, *obj;
	int				type = 0, index;

	if (argc < 1)
		return "No object type specified";

	if (!SetInt(&type, 1, MAX_OBJECT_TYPES-1, cl.objNames, argv[0]))
		return "Invalid object";

	if (cl.status != STATUS_UNIT_SELECT && !g_allWeaponsAvailable.integer)
		return "You must enter a building to purchase equipment";

	unit = CL_ObjectType(cl.predictedState.unittype);
	obj = CL_ObjectType(type);

	//check for disabled tech
	if (!Tech_IsAvailable(type, cl.research) && !g_allWeaponsAvailable.integer)
		return "This item cannot be constructed";

	//check unit restrictions
	if (!unit->canPurchase)
		return "This unit cannot purchase equipment";

	//check deployment limitations
	if (obj->maxDeployment &&
		cl.deployment[type] >= obj->maxDeployment)
		return fmt("Your team has too many %ss in use", obj->description);

	//test weapon loadout
	if (IsWeaponType(type))
	{
		int wpPoints = obj->weapPointValue;

		for (index = 0; index < MAX_INVENTORY; index++)
			wpPoints += CL_ObjectType(cl.predictedState.inventory[index])->weapPointValue;

		if (wpPoints > unit->maxWeapPoints)
			return "You are holding your maximum weapon capacity";
	}

	//test maxhold count
	if (obj->maxHold)
	{
		int count = 0;

		for (index = 0; index < MAX_INVENTORY; index++)
		{
			if (cl.predictedState.inventory[index] == type &&
				cl.predictedState.ammo[index] >= obj->ammoMax )
				count++;
		}

		if (count >= obj->maxHold)
			return fmt("You can not hold any more %ss", obj->description);
	}

	//look for a slot
	for (index = 0; index < MAX_INVENTORY; index++)
	{
		//test slot compatability
		if (!(unit->allowInventory[index] & (1 << obj->objclass)))
			continue;

		//adding ammo
		if (cl.predictedState.inventory[index] == type &&
			cl.predictedState.ammo[index] < obj->ammoMax)
			return "1";

		//occupied slot
		if (cl.predictedState.inventory[index] &&
			cl.predictedState.inventory[index] != CL_GetObjectTypeByName(unit->forceInventory[index]))
			continue;

		return "1";
	}

	return "Your inventory is full";
}


/*==========================

  Ask_StringsMatch_Fn

  syntax: ask stringsMatch <string1> <string2>
  returns: "1" if strings are identical, otherwise "0"

 ==========================*/

char	*Ask_StringsMatch_Fn(int argc, char *argv[])
{
	if (argc < 2)
		return "";

	if (stricmp(argv[0], argv[1]))
		return "0";
	else
		return "1";
}


/*==========================

  Ask_PlayerState_Fn

  syntax: ask playerState <state slot>
  returns: number of active state (if any) for a given state slot

 ==========================*/

char	*Ask_PlayerState_Fn(int argc, char *argv[])
{
	int	statenum;

	if (argc < 1)
		return "0";

	statenum = atoi(argv[0]);
	if (statenum < 0 || statenum > MAX_STATE_SLOTS)
		return "0";

	return fmt("%i", cl.objects[cl.clientnum].visual.states[statenum]);
}


/*==========================

  Ask_BuildItem_Fn

  syntax: ask buildItem <object type name> <slot number>
  returns: object type name that can be built by <object type name>, otherwise ""

 ==========================*/

char	*Ask_BuildItem_Fn(int argc, char *argv[])
{
	techEntry_t	*tech;
	int	index;

	if (argc < 2)
		return "";

	tech = Tech_GetEntry(CL_GetObjectTypeByName(argv[0]));
	if (!tech)
		return "";

	index = atoi(argv[1]);
	if (index > tech->numItemsToBuild)
		return "";

	if (!tech->itemsToBuild[index])
		return "";

	return cl.objNames[tech->itemsToBuild[index]];
}


/*==========================

  Ask_Selection_Fn

  syntax: ask selection <slot number>
  returns: object index of object in selection <slot number> if it is valid, otherwise "-1"

 ==========================*/

char	*Ask_Selection_Fn(int argc, char *argv[])
{
	int	index;

	if (argc < 1)
		return "-1";

	index = atoi(argv[1]);

	if (cl.selection.numSelected < 1)
		return "-1";

	if (index < 0 || index > MAX_SELECTABLE_UNITS)
		return "-1";

	return fmt("%i", cl.selection.array[index]);
}


/*==========================

  Ask_ObjectType_Fn

  syntax: ask objectType <object index>
  returns: object type name for <object index> if it is valid, otherwise ""

 ==========================*/

char	*Ask_ObjectType_Fn(int argc, char *argv[])
{
	int	index, type;

	if (argc < 1)
		return "";

	index = atoi(argv[0]);
	if (index < 0 || index > MAX_OBJECTS)
		return "";

	type = cl.objects[index].visual.type;

	if (type < 1 || type > MAX_OBJECT_TYPES || cl.objData[type]->objclass == OBJCLASS_NULL)
		return "";
	
	return cl.objNames[type];
}


/*==========================

  Ask_IsOfficer_Fn

  syntax: ask isOfficer <client number>
  returns "1" if client is an officer, "0" otherwise

 ==========================*/

char	*Ask_IsOfficer_Fn(int argc, char *argv[])
{
	int	index;

	if (argc < 1)
		return "";

	index = atoi(argv[0]);
	if (index < 0 || index >= MAX_CLIENTS)
		return "0";

	if (CMDR_UnitIsOfficer(index))
		return "1";
	else
		return "0";
}


/*==========================

  Ask_CurrentDir_Fn

  syntax: ask currentDir
  returns: pathname the engine is currently working in

 ==========================*/

char	*Ask_CurrentDir_Fn(int argc, char *argv[])
{
	return core.File_GetCurrentDir();
}



/*==========================

  Ask_IsResearchable_Fn

  syntax: ask isResearchable <object name>
  returns: "1" if item is currently available for research, otherwise "0"

 ==========================*/

char	*Ask_IsResearchable_Fn(int argc, char *argv[])
{
	if (Tech_IsResearchable(CL_GetObjectTypeByName(argv[0]), cl.research))
		return "1";
	else
		return "0";
}


/*==========================

  Ask_IsResearching_Fn

  syntax: ask isResearching <object index>
  returns: Returns current item being researched, otherwise "-1" if index is invalid.

 ==========================*/

char	*Ask_IsResearching_Fn(int argc, char *argv[])
{
	int objindex;

	if (argc < 1)
		return "0";

	objindex = atoi(argv[0]);
	if (objindex < MAX_CLIENTS || objindex >= MAX_OBJECTS)
		return "0";

	if (cl.objects[objindex].itemConstruction)
		return fmt("%i", cl.objects[objindex].itemConstruction);
	else
		return "0";
}


/*==========================

  Ask_IsAlly_Fn

  syntax: ask isAlly <object index>
  returns: Returns "1" if object is an ally, "0" if neutral, "-1" if it's an enemy

 ==========================*/

char	*Ask_IsAlly_Fn(int argc, char *argv[])
{
	int objindex;

	if (argc < 1)
		return "0";

	objindex = atoi(argv[0]);
	if (objindex > MAX_OBJECTS || objindex < 0)
		return "0";

	if (cl.objects[objindex].base.team == cl.info->team)
		return "1";
	else if (cl.objects[objindex].base.team == 0)
		return "0";
	else
		return "-1";
}


/*==========================

  Ask_NumDeployed_Fn

  syntax: ask numDeployed <object type>
  returns: number of items of that type that your team currently has deployed

 ==========================*/

char	*Ask_NumDeployed_Fn(int argc, char *argv[])
{
	int objtype;

	if (argc < 1)
		return "-1";

	objtype = CL_GetObjectTypeByName(argv[0]);
	if (objtype < 0 || objtype >= MAX_OBJECT_TYPES)
		return "-1";

	return fmt("%i", cl.deployment[objtype]);
}


/*==========================

  Ask_X_Fn

  This is here so tha I can test a new ask querey without recompiling :)

 ==========================*/

char	*Ask_X_Fn(int argc, char *argv[])
{
	return "0";
}

typedef struct
{
	char *name;
	char *(*fn)(int argc, char *argv[]);
} askFunction_t;

askFunction_t askTable[] = 
{
	{ "stringsMatch",		Ask_StringsMatch_Fn },
	{ "fileExists",			Ask_FileExists_Fn },
	{ "currentDir",			Ask_CurrentDir_Fn },
	{ "isResearched",		Ask_IsResearched_Fn },
	{ "isAvailable",		Ask_IsAvailable_Fn },
	{ "isResearchable",		Ask_IsResearchable_Fn },
	{ "isResearching",		Ask_IsResearching_Fn },
	{ "ammo",				Ask_Ammo_Fn },
	{ "clip",				Ask_Clip_Fn },
	{ "isInInventory",		Ask_IsInInventory_Fn },
	{ "inventory",			Ask_Inventory_Fn },
	{ "canPutInInventory",	Ask_CanPutInInventory_Fn },
	{ "playerState",		Ask_PlayerState_Fn },
	{ "buildItem",			Ask_BuildItem_Fn },
	{ "selection",			Ask_Selection_Fn },
	{ "objectType",			Ask_ObjectType_Fn },
	{ "isAlly",				Ask_IsAlly_Fn },
	{ "isOfficer",			Ask_IsOfficer_Fn },
	{ "numDeployed",		Ask_NumDeployed_Fn },
	
	{ "X",	Ask_X_Fn },
	{ "" }
};


/*==========================

  CL_Ask_Cmd

  Set's the cvar "asnwer" to a value based on the result of the query
  Syntax: ask <query> [arg0] ... [argX]

 ==========================*/

void	CL_Ask_Cmd(int argc, char *argv[])
{
	int n = 0;	

	if (!argc)
	{
		corec.Console_Printf("Valid queries:\n\n");
		while (askTable[n].name[0])
		{
			corec.Console_Printf(fmt("%s\n", askTable[n].name));
			n++;
		}
		return;
	}

	while (askTable[n].name[0])
	{
		if (stricmp(argv[0], askTable[n].name) == 0)
		{
			corec.Cvar_SetVar(&answer, askTable[n].fn(argc-1, &argv[1]));
			//corec.Console_DPrintf("Answer: %s\n", answer.string);
			return;
		}
		n++;
	}

	corec.Console_Printf("%s is Not a valid query\n", argv[0]);
}


void	CL_InitAsk()
{
	corec.Cmd_Register("ask", CL_Ask_Cmd);
	corec.Cvar_Register(&answer);
}
