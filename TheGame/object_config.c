// (C) 2003 S2 Games

// object_config.c


#include "game.h"

extern	char *eventNames[];
extern	char *animNames[];

char *TargetFlagNames[] =
{
	"enemy",
	"ally",
	"neutral",

	"unit",
	"building",
	"item",
	"world",
	"team",

	"same_type",

	""
};

extern cvar_t p_levelHealthBonus;

char	*objectNames[MAX_OBJECT_TYPES];	//this is generated from the objectData array on init
objectData_t	objData[MAX_OBJECT_TYPES];

bool	IsItemType(int type);
bool	IsWeaponType(int type);
bool	IsMeleeType(int type);
bool	IsUpgradeType(int type);
bool	IsUnitType(int type);
bool	IsCharacterType(int type);
bool	IsBuildingType(int type);
bool	IsMobileType(int type);
bool	IsWorkerType(int type);
int		CanPutInInventory(int unittype, int itemtype);
objectData_t	*GetObjectByType(int index);


void	SetObjectBounds(baseObject_t *obj)
{
	if (IsUnitType(obj->type) || IsItemType(obj->type))
	{
		objectData_t *unit = GetObjectByType(obj->type);
		M_CopyVec3(unit->bmin, obj->bmin);
		M_CopyVec3(unit->bmax, obj->bmax);
	}
	else if (IsBuildingType(obj->type))
	{
		//buildings will have their bmin/bmax set during LinkToWorldObject
		return;
	}
	else
	{
		M_CopyVec3(zero_vec, obj->bmin);
		M_CopyVec3(zero_vec, obj->bmax);
	}
}

bool	ValidateObjectNumber(int n)
{
#if 0

#ifdef SAVAGE_DEMO
	//this just specifies which objects are allowed to change
	if (n > 0 && (
		n <= 48 || 
		n == 96 || 
		n == 97 || 
		n == 101 ||
		n == 102 ||
		n == 104 ||
		n == 109 ||
		n == 114 ||
		n == 115))
		return true;
	else
		return false;
#else	//SAVAGE_DEMO
	return true;
#endif	//SAVAGE_DEMO

#endif

	return true;
}


/*==========================

  listNames

  Prints an array of strings to the console

 ==========================*/

void	listNames(int num, char *names[])
{
	int n;

	for (n = 1; n < num; n ++)
		if (strcmp(names[n], ""))
			core.Console_Printf("#%3i: %s\n", n, names[n]);
}

void	listObjects_Cmd(int argc, char *argv[])
{
	int n;

	for (n = 1; n < MAX_OBJECT_TYPES; n++)
	{
		if (!GetObjectByType(n)->touched)
			break;

		core.Console_Printf("#%3i: %s\n", n, objectNames[n]);
	}
}

void	listEvents_Cmd(int argc, char *argv[])
{
	listNames(NUM_EVENTS, eventNames);
}


//=============================================================================
bool	IsCharacterType(int type)
{
	if (objData[type].objclass == OBJCLASS_UNIT && !objData[type].isVehicle)
		return true;
	return false;
}

bool	IsUnitType(int type)
{
	if (objData[type].objclass == OBJCLASS_UNIT)
		return true;

	return false;
}

bool	IsBuildingType(int type)
{
	if (objData[type].objclass == OBJCLASS_BUILDING)
		return true;

	return false;
}

bool	IsWorkerType(int type)
{
	if (!IsUnitType(type))
		return false;

	return GetObjectByType(type)->isWorker;
}

bool	IsMobileType(int type)
{
	return IsUnitType(type);
}

bool	IsItemType(int type)
{
	if (GetObjectByType(type)->objclass == OBJCLASS_ITEM)
		return true;
	else
		return false;
}

bool	IsWeaponType(int type)
{
	if (objData[type].objclass == OBJCLASS_WEAPON)
		return true;

	return false;
}

bool	IsMeleeType(int type)
{
	if (objData[type].objclass == OBJCLASS_MELEE)
		return true;

	return false;
}

/*==========================

  CanPutInInventory

  Return a set of bit flags representing which slots of a given
  unit's inventory can hold a given item

 ==========================*/

int	CanPutInInventory(int unittype, int itemtype)
{
	int				index, ret = 0;
	objectData_t	*item, *unit;

	item = GetObjectByType(itemtype);
	unit = GetObjectByType(unittype);

	if (!IsUnitType(unittype))
		return ret;

	if (item->playerCost < 0 ||
		unit->race != item->race)
		return ret;

	for (index = 0; index < MAX_INVENTORY; index++)
	{
		if (unit->allowInventory[index] & (1 << item->objclass))
			ret |= 1 << index;
	}

	return ret;
}

int		RegisterVarArray(cvar_t varArray[])
{
	int n = 0;

	while (varArray[n].name[0])
	{
		core.Cvar_Register(&varArray[n]);
		n++;
	}

	return n;
}

void	ClearVarArray(cvar_t varArray[])
{
	int n = 0;

	while (varArray[n].name[0])
	{
		core.Cvar_Set(varArray[n].name, "");
		n++;
	}
}

//=============================================================================
char *objclassNames[] =
{
	"none",
	"weapon",
	"building",
	"unit",
	"item",
	"upgrade",
	"melee",
	""
};

#define	objOffset(n)	offsetof(objectData_t, n)

configVar_t	objectData_desc[] =
{
	//common
	{"objclass",		0,	NUM_OBJCLASSES,	"none",		objclassNames,	T_INT,			objOffset(objclass),		ALL	},
	{"name",			0,	MAX_NAME_LEN,	"object%i",	NULL,			T_STRING,		objOffset(name),			ALL	},
	{"description",		0,	0,				"desc %i",	NULL,			T_DYNSTRING,	objOffset(description),		ALL },
	{"race",			0,	MAX_RACES,		"0",		raceNames,		T_INT,			objOffset(race),			ALL },
	{"drawTeamIcon",	0,	1,				"1",		NULL,			T_INT,			objOffset(drawTeamIcon),	ALL },
	{"drawName",		0,	1,				"1",		NULL,			T_INT,			objOffset(drawName),		ALL },
	{"drawHealth",		0,	1,				"1",		NULL,			T_INT,			objOffset(drawHealth),		ALL },
	
	//visual representation of the unit/building
	{"model",	0,			0,				"",			NULL,	T_MODEL,		objOffset(model),		ALL	},
	{"skin",	0,			0,				"",			NULL,	T_DYNSTRING,	objOffset(skin),		ALL	},
	{"shader",	0,			0,				"",			NULL,	T_DYNSTRING,	objOffset(shader),		ALL	},
	{"effect",	0,			MAX_EFFECTS + MAX_EXEFFECTS,	"0",	exEffectNames,	T_INT,	objOffset(effect),	ALL },
	{"scale",	0.1,		NO_LIMIT,		"1",		NULL,	T_FLOAT,		objOffset(scale),		ALL },
	{"yaw",		-NO_LIMIT,	NO_LIMIT,		"0",		NULL,	T_FLOATRANGE,	objOffset(yaw),			ALL },
	{"pitch",	-NO_LIMIT,	NO_LIMIT,		"0",		NULL,	T_FLOATRANGE,	objOffset(pitch),		ALL },
	{"roll",	-NO_LIMIT,	NO_LIMIT,		"0",		NULL,	T_FLOATRANGE,	objOffset(roll),		ALL },
 
	//model to display in first person view
	{"handModel",	0,	0,				"",	NULL,	T_MODEL,		objOffset(handModel),	WPN|ITM },
	{"handSkin",	0,	0,				"",	NULL,	T_DYNSTRING,	objOffset(handSkin),	WPN|ITM },
	{"handEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"0",	exEffectNames,	T_INT,	objOffset(handEffect),	WPN|ITM },

	//visual data for projectiles (or anything else that this object 'spawns')
	{"projectileModel",		0,		0,				"",			NULL,	T_MODEL,		objOffset(projectileModel),		WPN|ITM },
	{"projectileSkin",		0,		0,				"",			NULL,	T_DYNSTRING,	objOffset(projectileSkin),		WPN|ITM },
	{"projectileShader",	0,		0,				"",			NULL,	T_DYNSTRING,	objOffset(projectileShader),	WPN|ITM },
	{"projectileScale",		0.1,	NO_LIMIT,		"1",		NULL,	T_FLOAT,		objOffset(projectileScale),		WPN|ITM },
	{"projectileRadius",	0,		NO_LIMIT,		"0",		NULL,	T_FLOAT,		objOffset(projectileRadius),	WPN|ITM },
	{"projectileEffect",	0,		MAX_EFFECTS + MAX_EXEFFECTS,	"0",	exEffectNames,	T_INT,			objOffset(projectileEffect),	WPN|ITM },
	{"trailEffect",			0,		MAX_EFFECTS + MAX_EXEFFECTS,	"",		exEffectNames,	T_INT,			objOffset(trailEffect),			WPN|ITM },
	{"trailPeriod",			0,		NO_LIMIT,		"200",		NULL,	T_INTRANGE,		objOffset(trailPeriod),			WPN|ITM },	
	{"flybyEffect",			0,		MAX_EFFECTS + MAX_EXEFFECTS,	"",		exEffectNames,	T_INT,			objOffset(flybyEffect),			WPN|ITM },

	//voice chat
	{"voiceMenu",			0,			0,			"human_player_male",				NULL,	T_DYNSTRING,	objOffset(voiceMenu),	UNT },

	//interface information
	{"icon",				0,			0,			"",									NULL,	T_DYNSTRING,	objOffset(icon),				ALL },
	{"selectionIcon",		0,			0,			"",									NULL,	T_DYNSTRING,	objOffset(selectionIcon),		ALL },
	{"mapIcon",				0,			0,			"",									NULL,	T_DYNSTRING,	objOffset(mapIcon),				ALL },
	{"tooltip",				0,			0,			"info %i",							NULL,	T_DYNSTRING,	objOffset(tooltip),				ALL },
	{"selectionSound",		0,			0,			"",									NULL,	T_DYNSTRING,	objOffset(selectionSound),		UNT|BLD },
	{"gridmenu",			0,			0,			"/gridmenus/layout_default.cfg",	NULL,	T_DYNSTRING,	objOffset(gridmenu),			UNT|BLD },
	{"proximityMessage",	0,			0,			"",									NULL,	T_DYNSTRING,	objOffset(proximityMessage),	UNT|BLD|ITM },
	{"proximitySound",		0,			0,			"default",							NULL,	T_DYNSTRING,	objOffset(proximitySound),		UNT|BLD|ITM },
	{"proximity",			0,			NO_LIMIT,	"0",								NULL,	T_FLOAT,		objOffset(proximity),			UNT|BLD|ITM },
	
	//techtree placement
	{"builder1",			0,			0,					"",			NULL,			T_DYNSTRING,	objOffset(builder[0]),			ALL },
	{"builder2",			0,			0,					"",			NULL,			T_DYNSTRING,	objOffset(builder[1]),			ALL },
	{"requirement1",		0,			0,					"",			NULL,			T_DYNSTRING,	objOffset(requirements[0]),		ALL },
	{"requirement2",		0,			0,					"",			NULL,			T_DYNSTRING,	objOffset(requirements[1]),		ALL },
	{"needTechPoints",		0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(needTechPoints),		ALL },
	{"needBasePoints",		0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(needBasePoints),		ALL },
	{"techPointValue",		0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(techPointValue),		ALL },
	{"basePointValue",		0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(basePointValue),		ALL },
	{"alwaysAvailable",		0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(alwaysAvailable),		ALL },
	{"techType",			0,			MAX_RESOURCE_TYPES,	"0",		resourceNames,	T_INT,			objOffset(techType),			ALL },

	//weapons
	{"useWeaponFire",	0,			1,			"0",		NULL,				T_INT,			objOffset(useWeaponFire),	ITM },
	{"ammoName",		0,			0,			"ammo",		NULL,				T_DYNSTRING,	objOffset(ammoName),		WPN|ITM },
	{"velocity",		0,			NO_LIMIT,	"1000",		NULL,				T_FLOATRANGE,	objOffset(velocity),		WPN|ITM },
	{"vertVelocity",	-NO_LIMIT,	NO_LIMIT,	"0",		NULL,				T_FLOATRANGE,	objOffset(vertVelocity),	WPN|ITM },
	{"count",			1,			NO_LIMIT,	"1",		NULL,				T_INTRANGE,		objOffset(count),			WPN|ITM },
	{"accuracy",		0,			1,			"1.0",		NULL,				T_FLOAT,		objOffset(accuracy),		WPN|ITM|BLD },
	{"chargeFlags",		0,			16,			"clear"	,	chargeFlagNames,	T_FLAGS,		objOffset(chargeFlags),		WPN|ITM },
	{"heatFlags",		0,			16,			"clear",	chargeFlagNames,	T_FLAGS,		objOffset(heatFlags),		WPN|ITM },
	{"chargeTime",		0,			NO_LIMIT,	"0",		NULL,				T_INT,			objOffset(chargeTime),		WPN|ITM },
	{"spinupTime",		0,			NO_LIMIT,	"0",		NULL,				T_INT,			objOffset(spinupTime),		WPN|ITM },
	{"spindownTime",	0,			NO_LIMIT,	"0",		NULL,				T_INT,			objOffset(spindownTime),	WPN|ITM },
	{"cooldownTime",	0,			NO_LIMIT,	"0",		NULL,				T_INT,			objOffset(cooldownTime),	WPN|ITM },
	{"overheatTime",	0,			NO_LIMIT,	"0",		NULL,				T_INT,			objOffset(overheatTime),	WPN|ITM },
	{"refreshTime",		0,			NO_LIMIT,	"500",		NULL,				T_INT,			objOffset(refreshTime),		WPN|ITM|BLD },
	{"fireDelay",		0,			NO_LIMIT,	"0",		NULL,				T_INT,			objOffset(fireDelay),		WPN|ITM },
	{"backfireTime",	0,			NO_LIMIT,	"500",		NULL,				T_INT,			objOffset(backfireTime),	WPN|ITM },
	{"ammoMax",			-1,			32677,		"-1",		NULL,				T_INT,			objOffset(ammoMax),			WPN|ITM },
	{"ammoStart",		0,			32677,		"0",		NULL,				T_INT,			objOffset(ammoStart),		WPN|ITM },
	{"ammoGroup",		1,			32677,		"1",		NULL,				T_INT,			objOffset(ammoGroup),		WPN|ITM },
	{"ammoCost",		0,			NO_LIMIT,	"10",		NULL,				T_INT,			objOffset(ammoCost),		WPN|ITM },
	{"twangAmount",		-100,		100,		"2",		NULL,				T_FLOAT,		objOffset(twangAmount),		WPN|ITM	},
	{"twangTime",		-5,			5,			"0.2",		NULL,				T_FLOAT,		objOffset(twangTime),		WPN|ITM	},
	{"twangPingPong",	0,			1,			"1",		NULL,				T_INT,			objOffset(twangPingPong),	WPN|ITM	},
	{"minVelocity",		0,			NO_LIMIT,	"0.0",		NULL,				T_FLOAT,		objOffset(minVelocity),		WPN|ITM	},
	{"maxVelocity",		0,			NO_LIMIT,	"1000",		NULL,				T_FLOAT,		objOffset(maxVelocity),		WPN|ITM	},
	{"gravity",			-NO_LIMIT,	NO_LIMIT,	"1.0",		NULL,				T_FLOAT,		objOffset(gravity),			WPN|ITM	},
	{"bounce",			-NO_LIMIT,	NO_LIMIT,	"0.0",		NULL,				T_FLOAT,		objOffset(bounce),			WPN|ITM	},
	{"accelerate",		-NO_LIMIT,	NO_LIMIT,	"0.0",		NULL,				T_FLOAT,		objOffset(accelerate),		WPN|ITM	},
	{"fuseTime",		0,			NO_LIMIT,	"5000",		NULL,				T_INTRANGE,		objOffset(fuseTime),		WPN|ITM	},
	{"muzzleRightOffset",	-NO_LIMIT,NO_LIMIT,	"2.5",		NULL,				T_FLOAT,		objOffset(muzzleOffset[0]),	WPN|ITM },
	{"muzzleFwdOffset",		-NO_LIMIT,NO_LIMIT,	"3",		NULL,				T_FLOAT,		objOffset(muzzleOffset[1]),	WPN|ITM },
	{"muzzleUpOffset",		-NO_LIMIT,NO_LIMIT,	"-3",		NULL,				T_FLOAT,		objOffset(muzzleOffset[2]),	WPN|ITM },
	{"weapPointValue",	0,			NO_LIMIT,	"0",		NULL,				T_INT,			objOffset(weapPointValue),	WPN|MLE },
	{"startFuseAtCharge",	0,		1,			"0",		NULL,				T_INT,			objOffset(startFuseAtCharge),	WPN|ITM },

	{"rightMeleeModel",			0,	0,	"",			NULL,			T_DYNSTRING,	objOffset(rightMeleeModel),			UNT|MLE },
	{"leftMeleeModel",			0,	0,	"",			NULL,			T_DYNSTRING,	objOffset(leftMeleeModel),			UNT|MLE },
	{"useDefaultMeleeModel",	0,	1,	"1",		NULL,			T_INT,			objOffset(useDefaultMeleeModel),	MLE },
	{"targetCorpse",			0,	1,	"0",		NULL,			T_INT,			objOffset(targetCorpse),			MLE },

	{"hitBuilding",			0,			NUM_PROJ_REACTIONS,	"die",	projReactionNames,	T_INT,			objOffset(hitBuilding),			WPN|ITM	},
	{"hitUnit",				0,			NUM_PROJ_REACTIONS,	"die",	projReactionNames,	T_INT,			objOffset(hitUnit),				WPN|ITM	},
	{"hitWorld",			0,			NUM_PROJ_REACTIONS,	"die",	projReactionNames,	T_INT,			objOffset(hitWorld),			WPN|ITM	},
	{"damage",				-NO_LIMIT,	NO_LIMIT,			"10",	NULL,				T_INT,			objOffset(damage),				WPN|ITM|MLE	},
	{"radius",				0,			NO_LIMIT,			"0",	NULL,				T_FLOATRANGE,	objOffset(radius),				WPN|ITM	},
	{"unitpierce",			0,			NO_LIMIT,			"1.0",	NULL,				T_FLOAT,		objOffset(unitPierce),			WPN|ITM|UNT	},
	{"bldpierce",			0,			NO_LIMIT,			"1.0",	NULL,				T_FLOAT,		objOffset(bldPierce),			WPN|ITM|UNT	},
	{"siegepierce",			0,			NO_LIMIT,			"1.0",	NULL,				T_FLOAT,		objOffset(siegePierce),			WPN|ITM|UNT	},
	{"repeat",				0,			1,					"1",	NULL,				T_INT,			objOffset(repeat),				WPN|ITM },
	{"continuous",			0,			1,					"0",	NULL,				T_INT,			objOffset(continuous),			WPN|ITM },
	{"continuousBeamShader",0,			0,					"",		NULL,				T_DYNSTRING,	objOffset(continuousBeamShader),WPN|ITM },
	{"continuousBeamType",	0,			0,					"0",	NULL,				T_INT,			objOffset(continuousBeamType),	WPN|ITM },
	{"minfov",				10,			90,					"90",	NULL,				T_FLOAT,		objOffset(minfov),				WPN|ITM },
	{"focusPenalty",		0,			1.0,				"1.0",	NULL,				T_FLOAT,		objOffset(focusPenalty),		WPN|ITM },
 	{"focusDegraderate",	0,			NO_LIMIT,			".5",	NULL,				T_FLOAT,		objOffset(focusDegradeRate),	WPN|ITM },
 	{"focusRecoverRate",	0,			NO_LIMIT,			".5",	NULL,				T_FLOAT,		objOffset(focusRecoverRate),	WPN|ITM },
	{"animGroup",			1,			6,					"1",	NULL,				T_INT,			objOffset(animGroup),			WPN|ITM },
	{"inheritVelocity",		0,			1,					"0",	NULL,				T_INT,			objOffset(inheritVelocity),		WPN|ITM },
	{"useAltMelee",			0,			1,					"0",	NULL,				T_INT,			objOffset(useAltMelee),			MLE },
	{"blockStunTime",		0,			NO_LIMIT,			"650",		NULL,			T_INT,			objOffset(blockStunTime),		MLE },
	
	{"cost",				0,			MAX_RESOURCE_TYPES,	"0",			resourceNames,	T_ARRAY,		objOffset(cost),			ALL },
	{"researchTime",		0,			NO_LIMIT,			"0",			NULL,			T_INT,			objOffset(researchTime),	ALL },
	{"playerCost",			-1,			NO_LIMIT,			"0",			NULL,			T_INT,			objOffset(playerCost),		ALL },
	{"attachBone",			0,			0,					"_boneRangedR",	NULL,			T_DYNSTRING,	objOffset(attachBone),		WPN|ITM },
	{"damageFlags",			0,			16,					"clear",		dmgFlagNames,	T_FLAGS,		objOffset(damageFlags),		WPN|MLE|ITM },
	
	{"useMana",			0,	1,			"0",		NULL,				T_INT,		objOffset(useMana),				WPN|ITM|MLE },
	{"manaCost",		0,	NO_LIMIT,	"0",		NULL,				T_INT,		objOffset(manaCost),			WPN|ITM|MLE },
	{"transferHealth",	0,	NO_LIMIT,	"0",		NULL,				T_FLOAT,	objOffset(transferHealth),		WPN|ITM|MLE },
	{"staminaDrain",	0,	NO_LIMIT,	"0",		NULL,				T_FLOAT,	objOffset(staminaDrain),		WPN|ITM|MLE },
	{"speedPenalty",	0,	1,			"1.0",		NULL,				T_FLOAT,	objOffset(speedPenalty),		WPN|ITM|MLE },

	//units
	{"e3noblend",			0,			1,					"0",		NULL,			T_INT,			objOffset(e3noblend),			UNT },
	{"npcTarget",			0,			1,					"1",		NULL,			T_INT,			objOffset(npcTarget),			UNT },
	{"cmdrScale",			0,			NO_LIMIT,			"3",		NULL,			T_FLOAT,		objOffset(cmdrScale),			UNT|ITM },
	{"footstepType",		0,			NO_LIMIT,			"light",	NULL,			T_DYNSTRING,	objOffset(footstepType),		UNT },
	{"bobSpeedRun",			-NO_LIMIT,	NO_LIMIT,			"16",		NULL,			T_FLOAT,		objOffset(bobSpeedRun),			UNT },
	{"bobSpeedSprint",		-NO_LIMIT,	NO_LIMIT,			"20",		NULL,			T_FLOAT,		objOffset(bobSpeedSprint),		UNT },
	{"bobAmount",			-NO_LIMIT,	NO_LIMIT,			"1",		NULL,			T_FLOAT,		objOffset(bobAmount),			UNT },
	{"isWorker",			0,			1,					"0",		NULL,			T_INT,			objOffset(isWorker),			UNT	},
	{"isVehicle",			0,			1,					"0",		NULL,			T_INT,			objOffset(isVehicle),			UNT },
	{"canRide",				0,			1,					"0",		NULL,			T_INT,			objOffset(canRide),				UNT },
	{"canEject",			0,			1,					"0",		NULL,			T_INT,			objOffset(canEject),			UNT },
	{"ejectUnit",			0,			NO_LIMIT,			"",			NULL,			T_DYNSTRING,	objOffset(ejectUnit),			UNT },
	{"maxRiders",			0,			10,					"10",		NULL,			T_INT,			objOffset(maxRiders),			UNT },
	{"blockPower",			0,			1.0,				"1.0",		NULL,			T_FLOAT,		objOffset(blockPower),			UNT	},
	{"blockArc",			0,			360,				"180",		NULL,			T_FLOAT,		objOffset(blockArc),			UNT	},
	{"speed",				0,			NO_LIMIT,			"1",		NULL,			T_FLOAT,		objOffset(speed),				UNT	},
	{"jumpHeight",			0,			NO_LIMIT,			"6",		NULL,			T_FLOAT,		objOffset(jumpheight),			UNT	},
	{"stepHeight",			0,			NO_LIMIT,			"15",		NULL,			T_FLOAT,		objOffset(stepheight),			UNT },
	{"airControl",			0,			NO_LIMIT,			"6",		NULL,			T_FLOAT,		objOffset(aircontrol),			UNT	},
	{"friction",			0,			NO_LIMIT,			"5",		NULL,			T_FLOAT,		objOffset(friction),			UNT	},
	{"viewHeight",			0,			NO_LIMIT,			"0",		NULL,			T_FLOAT,		objOffset(viewheight),			UNT|BLD	},
	{"distOffset",			1,			NO_LIMIT,			"20",		NULL,			T_INT,			objOffset(distOffset),			UNT	},
	{"maxCarry",			0,			MAX_RESOURCE_TYPES,	"",			resourceNames,	T_ARRAY,		objOffset(maxResources),		UNT },
	{"bmin_x",				-NO_LIMIT,	NO_LIMIT,			"-10",		NULL,			T_FLOAT,		objOffset(bmin[0]),				UNT|ITM	},
	{"bmin_y",				-NO_LIMIT,	NO_LIMIT,			"-10",		NULL,			T_FLOAT,		objOffset(bmin[1]),				UNT|ITM	},
	{"bmin_z",				-NO_LIMIT,	NO_LIMIT,			"0",		NULL,			T_FLOAT,		objOffset(bmin[2]),				UNT|ITM	},
	{"bmax_x",				-NO_LIMIT,	NO_LIMIT,			"10",		NULL,			T_FLOAT,		objOffset(bmax[0]),				UNT|ITM	},
	{"bmax_y",				-NO_LIMIT,	NO_LIMIT,			"10",		NULL,			T_FLOAT,		objOffset(bmax[1]),				UNT|ITM	},
	{"bmax_z",				-NO_LIMIT,	NO_LIMIT,			"20",		NULL,			T_FLOAT,		objOffset(bmax[2]),				UNT|ITM	},
	{"killGoldReward",		0,			NO_LIMIT,			"5",		NULL,			T_INT,			objOffset(killGoldReward),		UNT	},
	{"killResourceReward",	0,			MAX_RESOURCE_TYPES,	"",			resourceNames,	T_ARRAY,		objOffset(killResourceReward),	UNT	},
	{"attackProbability",	0,			1,					"1",		NULL,			T_FLOAT,		objOffset(attackProbability),	UNT	},
	{"level",				1,			10,					"1",		NULL,			T_INT,			objOffset(level),				UNT	},
	{"totalLives",			0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(totalLives),			UNT	},
	{"respawnTime",			0,			NO_LIMIT,			"10000",	NULL,			T_FLOAT,		objOffset(respawnTime),			UNT	},
	{"fixedPitch",			-90,		0,					"-45",		NULL,			T_FLOAT,		objOffset(fixedPitch),			UNT },
	{"turnRate",			0,			NO_LIMIT,			"3.0",		NULL,			T_FLOAT,		objOffset(turnRate),			UNT },
	{"maxPopulation",		0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(maxPopulation),		UNT|BLD },
	{"mineRate",			0,			NO_LIMIT,			"5",		NULL,			T_INT,			objOffset(mineRate),			UNT },
	{"repairRate",			0,			NO_LIMIT,			"10",		NULL,			T_INT,			objOffset(repairRate),			UNT },
	{"buildRate",			0,			NO_LIMIT,			"1000",		NULL,			T_INT,			objOffset(buildRate),			UNT },
	{"isScared",			0,			1,					"0",		NULL,			T_INT,			objOffset(isScared),			UNT },
	{"spawnAtStartNum",		0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(spawnAtStartNum),		UNT },
	{"isPassive",			0,			1,					"0",		NULL,			T_INT,			objOffset(isPassive),			UNT },
	{"minAimX",				0,			1,					"0",		NULL,			T_FLOAT,		objOffset(minAimX),				UNT },
	{"maxAimX",				0,			1,					"1",		NULL,			T_FLOAT,		objOffset(maxAimX),				UNT },
	{"minAimY",				0,			1,					"0",		NULL,			T_FLOAT,		objOffset(minAimY),				UNT },
	{"maxAimY",				0,			1,					"1",		NULL,			T_FLOAT,		objOffset(maxAimY),				UNT },
	{"viewDistance",		-NO_LIMIT,	NO_LIMIT,			"900",		NULL,			T_FLOAT,		objOffset(viewDistance),		UNT },
	{"healAmount",			0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(healAmount),			UNT },
	{"healRate",			0,			NO_LIMIT,			"1000",		NULL,			T_INT,			objOffset(healRate),			UNT },
	{"canPurchase",			0,			1,					"1",		NULL,			T_INT,			objOffset(canPurchase),			UNT },
	{"allowFirstPerson",	0,			1,					"1",		NULL,			T_INT,			objOffset(allowFirstPerson),	UNT },
	{"canBlock",			0,			1,					"0",		NULL,			T_INT,			objOffset(canBlock),			MLE },
	{"canDodge",			0,			1,					"0",		NULL,			T_INT,			objOffset(canDodge),			MLE },
	{"isSiegeWeapon",		0,			1,					"0",		NULL,			T_INT,			objOffset(isSiegeWeapon),		UNT },
	{"maxMana",				0,			250,				"0",		NULL,			T_INT,			objOffset(maxMana),				UNT },
	{"manaRegenRate",		0,			NO_LIMIT,			"1000",		NULL,			T_INT,			objOffset(manaRegenRate),		UNT },
	{"manaRegenAmount",		0,			NO_LIMIT,			"0",		NULL,			T_INT,			objOffset(manaRegenAmount),		UNT },
	{"canSprint",			0,			1,					"1",		NULL,			T_INT,			objOffset(canSprint),			UNT },
	{"maxStamina",			0,			NO_LIMIT,			"5000",		NULL,			T_INT,			objOffset(maxStamina),			UNT },
	{"staminaRegenRate",	0,			NO_LIMIT,			"1.0",		NULL,			T_FLOAT,		objOffset(staminaRegenRate),	UNT },		
	{"mass",				1,			NO_LIMIT,			"180",		NULL,			T_FLOAT,		objOffset(mass),				UNT },
	{"maxWeapPoints",		0,			NO_LIMIT,			"1",		NULL,			T_INT,			objOffset(maxWeapPoints),		UNT },
	{"revivable",			0,			1,					"0",		NULL,			T_INT,			objOffset(revivable),			UNT },
	{"isHealer",			0,			1,					"0",		NULL,			T_INT,			objOffset(isHealer),			UNT },

	// npc only
	{"attackMeleeChance",	0,			1,					"0",		NULL,			T_FLOAT,		objOffset(attackMeleeChance),	UNT },
	{"attackMissileChance",	0,			1,					"0",		NULL,			T_FLOAT,		objOffset(attackMissileChance),	UNT },
	{"attackPoundChance",	0,			1,					"0",		NULL,			T_FLOAT,		objOffset(attackPoundChance),	UNT },
	{"attackSuicideChance",	0,			1,					"0",		NULL,			T_FLOAT,		objOffset(attackSuicideChance),	UNT },
	{"attackSuicideDelay",	0,			NO_LIMIT,			"500 1000",	NULL,			T_FLOATRANGE,	objOffset(attackSuicideDelay),	UNT	},
	{"attackSuicideRange",	0,			NO_LIMIT,			"30 60",	NULL,			T_FLOATRANGE,	objOffset(attackSuicideRange),	UNT	},
	{"attackSuicideRadius",	0,			NO_LIMIT,			"60",		NULL,			T_FLOAT,		objOffset(attackSuicideRadius),	UNT },
	{"attackSuicideDamage",	0,			NO_LIMIT,			"100",		NULL,			T_FLOAT,		objOffset(attackSuicideDamage),	UNT },
	{"fleeChance",			0,			1,					"1",		NULL,			T_FLOAT,		objOffset(fleeChance),			UNT },
	{"fleeThreshhold",		0,			1,					"0.5",		NULL,			T_FLOAT,		objOffset(fleeThreshhold),		UNT },
	{"dodgeChance",			0,			1,					"0",		NULL,			T_FLOAT,		objOffset(dodgeChance),			UNT },
	{"dodgeRestTime",		0,			NO_LIMIT,			"0",		NULL,			T_INTRANGE,		objOffset(dodgeRestTime),		UNT	},
	{"dodgeDistance",		0,			NO_LIMIT,			"30 60",	NULL,			T_FLOATRANGE,	objOffset(dodgeDistance),		UNT	},
	{"aggressionChance",	0,			1,					"0",		NULL,			T_FLOAT,		objOffset(aggressionChance),	UNT },
	{"aggressionRange",		0,			NO_LIMIT,			"200 400",	NULL,			T_FLOATRANGE,	objOffset(aggressionRange),		UNT	},
	{"aggressionInterval",	0,			NO_LIMIT,			"500 1000",	NULL,			T_FLOATRANGE,	objOffset(aggressionInterval),	UNT	},
	{"packRange",			0,			NO_LIMIT,			"0",		NULL,			T_FLOAT,		objOffset(packRange),			UNT	},
	{"packSize",			0,			NO_LIMIT,			"0 16",		NULL,			T_INTRANGE,		objOffset(packSize),			UNT	},

	{"fullHealth",			0,	NO_LIMIT,		"100",	NULL,			T_INT,			objOffset(fullHealth),			ALL },
	{"weapon",				0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[0]),	BLD|WPN|ITM },
	{"forceInventory0",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[0]),	UNT },
	{"forceInventory1",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[1]),	UNT },
	{"forceInventory2",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[2]),	UNT },
	{"forceInventory3",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[3]),	UNT },
	{"forceInventory4",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[4]),	UNT },
	{"forceInventory5",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[5]),	UNT },
	{"forceInventory6",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[6]),	UNT },
	{"forceInventory7",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[7]),	UNT },
	{"forceInventory8",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[8]),	UNT },
	{"forceInventory9",		0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[9]),	UNT },
	{"forceInventory10",	0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[10]),	UNT },
	{"forceInventory11",	0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[11]),	UNT },
	{"forceInventory12",	0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[12]),	UNT },
	{"forceInventory13",	0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[13]),	UNT },
	{"forceInventory14",	0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[14]),	UNT },
	{"forceInventory15",	0,	0,				"",		NULL,			T_DYNSTRING,	objOffset(forceInventory[15]),	UNT },
	{"allowInventory0",		0,	NUM_OBJCLASSES,	"+melee",	objclassNames,	T_FLAGS,		objOffset(allowInventory[0]),	UNT	},
	{"allowInventory1",		0,	NUM_OBJCLASSES,	"+weapon",	objclassNames,	T_FLAGS,		objOffset(allowInventory[1]),	UNT	},
	{"allowInventory2",		0,	NUM_OBJCLASSES,	"+item",	objclassNames,	T_FLAGS,		objOffset(allowInventory[2]),	UNT	},
	{"allowInventory3",		0,	NUM_OBJCLASSES,	"+item",	objclassNames,	T_FLAGS,		objOffset(allowInventory[3]),	UNT	},
	{"allowInventory4",		0,	NUM_OBJCLASSES,	"+item",	objclassNames,	T_FLAGS,		objOffset(allowInventory[4]),	UNT	},
	{"allowInventory5",		0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[5]),	UNT	},
	{"allowInventory6",		0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[6]),	UNT	},
	{"allowInventory7",		0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[7]),	UNT	},
	{"allowInventory8",		0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[8]),	UNT	},
	{"allowInventory9",		0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[9]),	UNT	},
	{"allowInventory10",	0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[10]),	UNT	},
	{"allowInventory11",	0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[11]),	UNT	},
	{"allowInventory12",	0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[12]),	UNT	},
	{"allowInventory13",	0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[13]),	UNT	},
	{"allowInventory14",	0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[14]),	UNT	},
	{"allowInventory15",	0,	NUM_OBJCLASSES,	"clear",	objclassNames,	T_FLAGS,		objOffset(allowInventory[15]),	UNT	},
	{"radiusState",			0,	MAX_STATES,		"",		stateNames,		T_INT,			objOffset(radiusState),			BLD },
	{"targetProjectiles",	0,	1,				"0",	NULL,			T_INT,			objOffset(targetProjectiles),	BLD },

	//buildings
	{"isMine",				0,			1,					"0",		NULL,			T_INT,		objOffset(isMine),					BLD	},
	{"isClaimable",			0,			1,					"0",		NULL,			T_INT,		objOffset(isClaimable),				BLD },
	{"mineType",			0,			MAX_RESOURCE_TYPES,	"0",		resourceNames,	T_INT,		objOffset(mineType),				BLD	},
	{"mineAmount",			0,			NO_LIMIT,			"100000",	NULL,			T_INT,		objOffset(mineAmount),				BLD },
	{"generate",			0,			MAX_RESOURCE_TYPES,	"0",		resourceNames,	T_FLAGS,	objOffset(generate),				BLD },
	{"store",				0,			MAX_RESOURCE_TYPES,	"0",		resourceNames,	T_ARRAY,	objOffset(store),					BLD },
	{"dropoff",				0,			1,					"0",		NULL,			T_INT,		objOffset(dropoff),					BLD	},
	{"commandCenter",		0,			1,					"0",		NULL,			T_INT,		objOffset(commandCenter),			BLD	},
	{"spawnFrom",			0,			1,					"0",		NULL,			T_INT,		objOffset(spawnFrom),				BLD|ITM	},
	{"canEnter",			0,			1,					"0",		NULL,			T_INT,		objOffset(canEnter),				BLD },
	{"scanRange",			0,			NO_LIMIT,			"0",		NULL,			T_FLOAT,	objOffset(scanRange),				BLD|WPN	},
	{"healthStage1",		0,			100,				"25",		NULL,			T_INT,		objOffset(healthStages[0]),			BLD },
	{"healthStage2",		0,			100,				"50",		NULL,			T_INT,		objOffset(healthStages[1]),			BLD },
	{"healthStage3",		0,			100,				"75",		NULL,			T_INT,		objOffset(healthStages[2]),			BLD },
	{"healthStage1Effect",	0,			MAX_EFFECTS + MAX_EXEFFECTS,		"",			exEffectNames,	T_INT,		objOffset(healthStageEffects[0]),	BLD },
	{"healthStage2Effect",	0,			MAX_EFFECTS + MAX_EXEFFECTS,		"",			exEffectNames,	T_INT,		objOffset(healthStageEffects[1]),	BLD },
	{"healthStage3Effect",	0,			MAX_EFFECTS + MAX_EXEFFECTS,		"",			exEffectNames,	T_INT,		objOffset(healthStageEffects[2]),	BLD },
	{"healthStage1Period",	0,			NO_LIMIT,			"500",		NULL,			T_INTRANGE,	objOffset(healthStagePeriods[0]),	BLD },
	{"healthStage2Period",	0,			NO_LIMIT,			"500",		NULL,			T_INTRANGE,	objOffset(healthStagePeriods[1]),	BLD },
	{"healthStage3Period",	0,			NO_LIMIT,			"500",		NULL,			T_INTRANGE,	objOffset(healthStagePeriods[2]),	BLD },
	{"linked",				0,			1,					"0",		NULL,			T_INT,		objOffset(linked),					BLD },
	{"selfBuild",			0,			1,					"0",		NULL,			T_INT,		objOffset(selfBuild),				BLD },
	{"canBeRepaired",		0,			1,					"1",		NULL,			T_INT,		objOffset(canBeRepaired),			BLD },
	{"lifeSpan",			0,			NO_LIMIT,			"0",		NULL,			T_INT,		objOffset(lifeSpan),				BLD },
	{"anchorRange",			0,			NO_LIMIT,			"0.0",		NULL,			T_FLOAT,	objOffset(anchorRange),				BLD },
	{"needsAnchor",			0,			1,					"0",		NULL,			T_INT,		objOffset(needsAnchor),				BLD },
	{"expMult",				0,			NO_LIMIT,			"1.0",		NULL,			T_FLOAT,	objOffset(expMult),					ALL },
	{"thornDamage",			0,			NO_LIMIT,			"0",		NULL,			T_INT,		objOffset(thorndamage),				BLD|UNT|ITM },
	{"shiftProjAccel",		-NO_LIMIT,	NO_LIMIT,			"0.0",		NULL,			T_FLOAT,	objOffset(shiftProjAccel),			BLD },
	{"shiftProjGrav",		-NO_LIMIT,	NO_LIMIT,			"0.0",		NULL,			T_FLOAT,	objOffset(shiftProjGrav),			BLD },
	{"alwaysWaypoint",		0,			1,					"0",		NULL,			T_INT,		objOffset(alwaysWaypoint),			ALL },

	//items
	{"giveAmmo",			0,			1,				"0",		NULL,				T_INT,		objOffset(giveAmmo),			ITM },
	{"giveMana",			0,			NO_LIMIT,		"0",		NULL,				T_INT,		objOffset(giveMana),			ITM },
	{"maxHold",				0,			NO_LIMIT,		"1",		NULL,				T_INT,		objOffset(maxHold),			ITM|WPN },
	{"delayEffect",			0,			MAX_EFFECTS + MAX_EXEFFECTS,	"0",		exEffectNames,		T_INT,		objOffset(delayEffect),			ITM },
	{"delayPeriod",			0,			NO_LIMIT,		"500",		NULL,				T_INT,		objOffset(delayPeriod),			ITM },
	{"activeEffect",		0,			MAX_EFFECTS + MAX_EXEFFECTS,	"0",		exEffectNames,		T_INT,		objOffset(activeEffect),		ITM },
	{"activeEffectPeriod",	0,			NO_LIMIT,		"500",		NULL,				T_INT,		objOffset(activeEffectPeriod),	ITM },
	{"isSolid",				0,			1,				"0",		NULL,				T_INT,		objOffset(isSolid),				ITM },
	{"isActivated",			0,			1,				"0",		NULL,				T_INT,		objOffset(isActivated),			ITM },
	{"isVulnerable",		0,			1,				"1",		NULL,				T_INT,		objOffset(isVulnerable),		ITM|UNT|BLD|WPN },
	{"meleeOnlyVulnerable",	0,			1,				"0",		NULL,				T_INT,		objOffset(meleeOnlyVulnerable),	ITM|UNT|BLD },
	{"revealHidden",		0,			1,				"0",		NULL,				T_INT,		objOffset(revealHidden),		ITM|UNT|BLD },
	{"canPickup",			0,			1,				"0",		NULL,				T_INT,		objOffset(canPickup),			ITM },
	{"pickupGive",			0,			MAX_NAME_LEN,	"",			NULL,				T_STRING,	objOffset(pickupGive),			ITM },
	{"pickupRespawn",		0,			NO_LIMIT,		"20000",	NULL,				T_INT,		objOffset(pickupRespawn),		ITM },
	{"manaRegenAdd",		0,			NO_LIMIT,		"0",		NULL,				T_INT,		objOffset(manaRegenAdd),		ITM },
	{"manaRateMult",		0,			NO_LIMIT,		"1.0",		NULL,				T_FLOAT,	objOffset(manaRateMult),		ITM },
	{"isVolatile",			0,			1,				"0",		NULL,				T_INT,		objOffset(isVolatile),			ITM },
	{"maxDeployment",		0,			NO_LIMIT,		"0",		NULL,				T_INT,		objOffset(maxDeployment),		ITM },
	{"deathLinger",			0,			NO_LIMIT,		"5000",		NULL,				T_INT,		objOffset(deathLinger),			ITM },
	{"isSelectable",		0,			1,				"1",		NULL,				T_INT,		objOffset(isSelectable),		ITM|WPN },
	{"linkToOwner",			0,			1,				"0",		NULL,				T_INT,		objOffset(linkToOwner),			ITM },

	//upgrades
	{"duration",		0,	NO_LIMIT,			"0",		NULL,				T_INT,		objOffset(duration),		UPG },
	{"activateCost",	0,	MAX_RESOURCE_TYPES,	"",			resourceNames,		T_ARRAY,	objOffset(activateCost),	UPG },
	{"targetFlags",		0,	16,					"clear",	TargetFlagNames,	T_FLAGS,	objOffset(targetFlags),		UPG|UNT|ITM },

	//event effects
	{"woundedEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_WOUNDED]),				UNT|ITM },
	{"resurrectedEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_RESURRECTED]),			UNT },
	{"deflectedEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_DEFLECTED]),			UNT },
	{"dropoffEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_DROPOFF]),				UNT },
	{"levelUpEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_LEVEL_UP]),				UNT },
	{"spawnEffect",			0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_SPAWN]),				ALL },
	{"deathEffect",			0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_DEATH]),				ALL },
	{"dazedEffect",			0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_DAZED]),				UNT },
	{"weaponFireEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"", exEffectNames,	T_INT,	objOffset(effects[EVENT_WEAPON_FIRE]),			WPN },
	{"mineEffect",			0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_MINE]),					UNT },
	{"dropoffEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_DROPOFF]),				UNT },
	{"bounceEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_BOUNCE]),				WPN },
	{"stopEffect",			0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_STOP]),					WPN },
	{"use_itemEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_USE_ITEM]),				ITM },
	{"activateItemEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_ITEM_ACTIVATE]),		ITM },
	{"sleepItemEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_ITEM_SLEEP]),			ITM },
	{"idleItemEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_ITEM_IDLE]),			ITM },
	{"task_finishedEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_TASK_FINISHED]),		UNT },
	{"resourceFullEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"", exEffectNames,	T_INT,	objOffset(effects[EVENT_RESOURCE_FULL]),		UNT },
	{"goodiePickupEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_GOODIE_PICKUP]),		UNT },
	{"jumpLandEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_JUMP_LAND]),			UNT },
	{"jumpEffect",			0,	MAX_EFFECTS + MAX_EXEFFECTS,	"", exEffectNames,	T_INT,	objOffset(effects[EVENT_JUMP]),					UNT },
	{"splodeyDeathEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_SPLODEY_DEATH]),		UNT },
	{"selectionEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"", exEffectNames,	T_INT,	objOffset(effects[EVENT_COMMANDER_SELECTED]),	UNT },
	{"attackPoundEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"", exEffectNames,	T_INT,	objOffset(effects[EVENT_ATTACK_POUND]),			UNT },
	{"attackSuicideEffect",	0,	MAX_EFFECTS + MAX_EXEFFECTS,	"", exEffectNames,	T_INT,	objOffset(effects[EVENT_ATTACK_SUICIDE]),		UNT },
	{"fizzleEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_FIZZLE]),				UNT|BLD },
	{"backfireEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_BACKFIRE]),				ITM|WPN },
	{"powerupEffect",		0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",	exEffectNames,	T_INT,	objOffset(effects[EVENT_POWERUP]),				UPG	 },

	//looping sounds
	{ "projectileSound",	0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(projectileSound),					ALL },
	{ "constructionSound",	0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(constructionSound),				ALL },

	{ "idleSound",			0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_IDLE]),			ALL },
	{ "walkLeftSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WALK_LEFT]),		ALL },
	{ "walkRightSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WALK_RIGHT]),	ALL },
	{ "walkFwdSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WALK_FWD]),		ALL },
	{ "walkBackSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WALK_BACK]),		ALL },
	{ "runLeftSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_RUN_LEFT]),		ALL },
	{ "runRightSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_RUN_RIGHT]),		ALL },
	{ "runFwdSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_RUN_FWD]),		ALL },
	{ "runBackSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_RUN_BACK]),		ALL },
	{ "sprintLeftSound",	0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_SPRINT_LEFT]),		ALL },
	{ "sprintRightSound",	0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_SPRINT_RIGHT]),		ALL },
	{ "sprintFwdSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_SPRINT_FWD]),		ALL },
	{ "sprintBackSound",	0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_SPRINT_BACK]),		ALL },
	{ "wpIdleSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WPSTATE_IDLE]),		ALL },
	{ "wpChargeSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WPSTATE_CHARGE]),	ALL },
	{ "wpSpinupSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WPSTATE_SPINUP]),	ALL },
	{ "wpOverheatSound",	0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WPSTATE_OVERHEAT]),	ALL },
	{ "wpFiringSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_WPSTATE_FIRE]),		ALL },
	{ "itemSleepSound",		0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_ITEM_SLEEP]),		ALL },
	{ "itemActiveSound",	0,		0,		"",		NULL,	T_DYNSTRING,	objOffset(loopingSounds[AS_ITEM_ACTIVE]),		ALL },
	{ "" }
};


//melee attacks
configVar_t attackData_desc[] = 
{
	{"time",			0,			NO_LIMIT,	"0",		NULL,			T_INT,			offsetof(attackData_t, time),			ALL },
	{"impact",			0,			NO_LIMIT,	"0",		NULL,			T_INTRANGE,		offsetof(attackData_t, impact),			ALL },
	{"lunge",			0,			NO_LIMIT,	"0",		NULL,			T_INT,			offsetof(attackData_t, lunge),			ALL },
	{"lungeTime",		0,			NO_LIMIT,	"0",		NULL,			T_INT,			offsetof(attackData_t, lungeTime),		ALL },
	{"flurry",			0,			NO_LIMIT,	"0",		NULL,			T_INT,			offsetof(attackData_t, flurry),			ALL },
	{"flurryTime",		0,			NO_LIMIT,	"0",		NULL,			T_INT,			offsetof(attackData_t, flurryTime),		ALL },
	{"damage",			-NO_LIMIT,	NO_LIMIT,	"20",		NULL,			T_INT,			offsetof(attackData_t, damage),			ALL },
	{"damageFlags",		0,			16,			"clear",	dmgFlagNames,	T_FLAGS,		offsetof(attackData_t, damageFlags),	ALL },
	{"xLunge",			-NO_LIMIT,	NO_LIMIT,	"0",		NULL,			T_INT,			offsetof(attackData_t, xLunge),			ALL },
	{"yLunge",			-NO_LIMIT,	NO_LIMIT,	"0",		NULL,			T_INT,			offsetof(attackData_t, yLunge),			ALL },
	{"range",			0,			NO_LIMIT,	"0",		NULL,			T_INT,			offsetof(attackData_t, range),			ALL },
	{"horzKick",		-1.0,		1.0,		"0",		NULL,			T_FLOAT,		offsetof(attackData_t, horzKick),		ALL },
	{"vertKick",		-1.0,		1.0,		"0",		NULL,			T_FLOAT,		offsetof(attackData_t, horzKick),		ALL },
	{"speed",			0,			NO_LIMIT,	"1.0",		NULL,			T_FLOAT,		offsetof(attackData_t, speed),			ALL },
	{"lockAngles",		0,			1,			"0",		NULL,			T_INT,			offsetof(attackData_t, lockAngles),		ALL },
	{"restTime",		0,			NO_LIMIT,	"1000",		NULL,			T_INTRANGE,		offsetof(attackData_t, restTime),		ALL	},
	{""}
};

///=============================================================================

int arrayindex = -1;

//=============================================================================
int	countConfigVars(configVar_t *info)
{
	int count = 0;

	if (!info)
		return 0;

	while (info[count].name[0])
		count++;

	return count;
}
//=============================================================================

//=============================================================================
// Set* functions
// These are support functions for xSet that handle the various data types
//=============================================================================
bool	SetInt(int *v, int min, int max, char *nameList[], const char *string)
{
	int		n;
	bool	f = false;	//did the value change?

	if (!string)
		return f;

	//try to set the value based on an index name
	if (nameList)
	{
		for (n = min; n <= max; n++)
		{
			if (!stricmp(nameList[n], ""))
			{
				max = n - 1;
				break;
			}

			if (!stricmp(nameList[n], string))
				break;
		}
		if (n <= max)
		{
			*v = n;
			f = true;
		}
	}

	//couldn't set value by an index name
	if (!f)
	{
		n = atoi(string);

		if ((n < min) || (n > max))
			core.Console_Printf("Valid values are [%i,%i]\n", min, max);
		else
		{
			*v = n;
			f = true;
		}
	}

	if (!f)
	{
		if (nameList && *v >= min && *v <= max)
			core.Console_Printf("Current value: %s\n", nameList[*v]);
		else
			core.Console_Printf("Current value: %i\n", *v);
	}

	return f;
}
//=============================================================================
bool	SetIntEx(int *v, configVar_t *info, int argc, char *argv[])
{
	int		n, min, max;
	bool	f = false;	//did the value change?

	if (argc < 1)
		return f;

	min = info->min;
	max = info->max;

	//try to set the value based on an index name
	if ((char**)info->nameList)
	{
		for (n = min; n < max; n++)
		{
			if (!stricmp( ((char**)info->nameList)[n], argv[0]))
				break;
		}
		if (n < max)
		{
			*v = n;
			f = true;
		}
	}

	//couldn't set value by an index name
	if (!f)
	{
		n = atoi(argv[0]);

		if ((n < min) || (n > max))
			core.Console_Printf("Valid values are [%i,%i]\n", min, max);
		else
		{
			*v = n;
			f = true;
		}
	}

	if (!f)
	{
		if (info->nameList)
			core.Console_Printf("Current value: %s\n", ((char**)info->nameList)[*v]);
		else
			core.Console_Printf("Current value: %i\n", *v);
	}

	return f;
}
//=============================================================================
bool	SetIntRange(intrange *v, int minr, int maxr, int argc, char *argv[])
{
	int		min, max;
	bool	f = false;	//did the value change?

	if (argc > 0)
	{
		min = atoi(argv[0]);

		if (argc > 1)
			max = atoi(argv[1]);
		else
			max = min;

		if ((min < minr) || (max > maxr))
			core.Console_Printf("Valid values are [%i,%i]\n", minr, maxr);
		else if (min > max)
			core.Console_Printf("min value must be less than max value.\n");
		else
		{
			SETRANGE(*v, min, max);
			f = true;
		}
	}

	if (!f)
		core.Console_Printf("Current value: [%i,%i]\n", v->min, v->max);
	return f;
}
//=============================================================================
bool	SetFloat(float *v, float minr, float maxr, int argc, char *argv[])
{
	float	n;
	bool	f = false;

	if (argc > 0)
	{
		n = atof(argv[0]);

		if ((n < minr) || (n > maxr))
			core.Console_Printf("Valid values are [%f,%f]\n", minr, maxr);
		else
		{
			*v = n;
			f = true;
		}
	}

	if (!f)
		core.Console_Printf("Current value: %f\n", *v);
	return f;
}
//=============================================================================
bool	SetFloatRange(floatrange *v, float minr, float maxr, int argc, char *argv[])
{
	float	min, max;
	bool	f = false;

	if (argc > 0)
	{
		min = atof(argv[0]);

		if (argc > 1)
			max = atof(argv[1]);
		else
			max = min;

		if ((min < minr) || (max > maxr))
			core.Console_Printf("Valid values are [%f,%f]\n", minr, maxr);
		else if (min > max)
			core.Console_Printf("min value must be less than max value.\n");
		else
		{
			SETRANGE(*v, min, max);
			f = true;
		}
	}

	if (!f)
		core.Console_Printf("Current value: [%f,%f]\n", v->min, v->max);
	return f;
}
//=============================================================================
bool	SetString(char *v, int maxlen, int argc, char *argv[])
{
	int n = 0;

	if (argc < 1)
		return false;

	n = strlen(argv[0]);

	if (n >= maxlen)
		core.Console_DPrintf("Warning: string too long, truncating...\n");

	strncpySafe(v, argv[0], maxlen);
	return true;
}
//=============================================================================
bool	SetDynString(char **v, int argc, char *argv[])
{
	if (argc < 1)
		return false;

	if (*v)
		DynFree(*v);

	*v = DynAllocString(argv[0]);

	return true;
}
//=============================================================================
void	SetFlags(int *v, char *flagNames[], int argc, char *argv[])
{
	int	onFlags, offFlags, n, x;

	if (!flagNames)
		return;

	if (argc > 0)
	{
		//check for a 'clear' command
		if (!stricmp(argv[0], "clear"))
		{
			//core.Console_Printf("All flags cleared\n");
			*v = 0;
			return;
		}

		//just to be reprical...
		if (!stricmp(argv[0], "all"))
		{
			//core.Console_Printf("All flags set\n");
			*v = 0xffff;
			return;
		}

		onFlags = offFlags = 0;

		//scan the argument list
		for (x = 0; x < argc; x++)
		{
			//search for a sepcified flag
			for (n = 0; n < 32; n++)
			{
				if (!flagNames[n])
					break;
				if (flagNames[n][0] == 0)
					break;

				if (!stricmp(&(argv[x][1]), flagNames[n]))
				{
					if (argv[x][0] == '+')
					{
						onFlags |= (1 << n);
						//core.Console_Printf("Flag %s switched on\n", flagNames[n]);
					}
					else if (argv[x][0] == '-')
					{
						offFlags |= (1 << n);
						//core.Console_Printf("Flag %s switched off\n", flagNames[n]);
					}
				}
			}
		}
		
		*v |= onFlags;
		*v &= ~offFlags;
	}
}

//=============================================================================
bool SetModel(void *data, configVar_t info[], int index, int argc, char *argv[])
{
	char	name[_MAX_PATH];

	if (argc < 1)
		return false;

	//save the path to the model file
	SetDynString((char**)((char*)(data)+info[index].offset), argc, argv);

	Filename_StripExtension(argv[0], name);

	return true;
}
//=============================================================================
//=============================================================================

bool	xSetVar(configVar_t info[], int count, void *data, int index, int argc, char *argv[])
{
	POP_ARG;

	if (info[index].type == T_ARRAY)
	{
		SetInt(&arrayindex, 0, info[index].max, info[index].nameList, argv[0]);
		POP_ARG;
	}

	//no setting parameter, list it
	if (argc < 1)
	{
		xList(&info[index], 1, data, ALL);
		arrayindex = -1;
		return true;
	}
	
	//set the value
	switch (info[index].type)
	{
	case T_INT:
		SetIntEx(INTP_OFFSET(data,info,index), &info[index], argc, argv);
		break;
	case T_INTRANGE:
		SetIntRange(INTRANGEP_OFFSET(data,info,index), info[index].min, info[index].max, argc, argv);
		break;
	case T_FLOAT:
		SetFloat(FLOATP_OFFSET(data,info,index), info[index].min, info[index].max, argc, argv);
		break;
	case T_FLOATRANGE:
		SetFloatRange(FLOATRANGEP_OFFSET(data,info,index), info[index].min, info[index].max, argc, argv);
		break;
	case T_STRING:
		SetString(STRINGP_OFFSET(data,info,index), info[index].max, argc, argv);
		break;
	case T_DYNSTRING:
		SetDynString((char**)((char*)(data) + info[index].offset), argc, argv);
		break;
	case T_FLAGS:
		SetFlags(INTP_OFFSET(data, info, index), info[index].nameList, argc, argv);
		break;
	case T_MODEL:
		SetModel(data, info, index, argc, argv);
		break;
	case T_DEFAULT:
		break;
	case T_ARRAY:
		SetInt( (int*)((char*)(data) + info[index].offset + sizeof(int) * arrayindex), 0, NO_LIMIT, NULL, argv[0]);
		break;
	}

		arrayindex = -1;
	return true;

}
				
//=============================================================================
// Generic versions of Set, List, Init, Save commands
// These can be callesd within custom console command functions to operate on
// data structures that have a configVar_t description.
//=============================================================================

//=============================================================================
// xSet
// Sets a variable within a structure
//
// info : configVar_t array that describes the structure
// count : number of elements in info
// data : pointer to structure that info describes
// argc : (pass from parent function)
// argv : (pass from parent function)
//
// *** This function is cheat protected, since it is the core of editing stuff ***
//
//=============================================================================
bool	xSet(configVar_t info[], int count, void *data, int argc, char *argv[])
{
	int		index;

	if (!core.Cvar_GetValue("svr_allowCheats"))
	{
		core.Console_Printf("The game is currently cheat protected.  To edit game configs, use 'devworld' to load your map.\n");
		return false;
	}

	if (argc < 1)
	{
		core.Console_Printf("Usage: xSet <key> <value>\n");
		return false;
	}

	//seearch for the key word
	for (index = 0; index < count; index++)
	{
		if (info[index].type == T_DEFAULT)
			continue;

		if (!stricmp(argv[0], info[index].name))
			break;
	}

	//didn't find a key by that name
	if (index == count)
		return false;

	return xSetVar(info, count, data, index, argc, argv);
}
//=============================================================================

//=============================================================================

//only supports certain types
bool	xMakeString(configVar_t info[], int count, void *data, const char *field, char *sub, char *out, int maxlen)
{
	int index, n;

	out[0] = 0;

	//search for the key word
	for (index = 0; index < count; index++)
	{
		if (info[index].type == T_DEFAULT)
			continue;
		
		if (!stricmp(field, info[index].name))
			break;
	}

	if (index == count)
		return false;

	switch (info[index].type)
	{
		case T_DEFAULT:
			break;
		case T_INT:
			if (info[index].nameList)
				strncpySafe(out, ((char**)info[index].nameList)[INT_OFFSET(data, info, index)], maxlen);				
			else
				strncpySafe(out, fmt("%i", INT_OFFSET(data, info, index)), maxlen);				
			return true;
		case T_INTRANGE:
			strncpySafe(out, fmt("%i - %i", INTRANGE_OFFSET(data, info, index).min, INTRANGE_OFFSET(data,info,index).max), maxlen);
			return true;
		case T_FLOAT:
			strncpySafe(out, fmt("%f", FLOAT_OFFSET(data, info, index)), maxlen);			
			return true;
		case T_FLOATRANGE:
			strncpySafe(out, fmt("%f - %f", FLOATRANGE_OFFSET(data, info, index).min, FLOATRANGE_OFFSET(data,info,index).max), maxlen);
			return true;
		case T_MODEL:
		case T_DYNSTRING:
			strncpySafe(out, *((char**)((char*)(data) + info[index].offset)), maxlen);			
			return true;
		case T_STRING:
			strncpySafe(out, STRINGP_OFFSET(data, info, index), maxlen);			
			return true;
		case T_ARRAY:
			if (!sub)
				return false;
			if (!SetInt(&n, 0, info[index].max, info[index].nameList, sub))
				return false;
			strncpySafe(out, fmt("%i", (*(int*)((char*)(data) + info[index].offset + sizeof(int) * n)) ), maxlen);
			return true;
		default:
			break;
	}	

	return false;
}


/*==========================

  xList

  Displays the values for all variables in a structure

  info : configVar_t array that describes the structure
  count : number of elements in info
  data : pointer to structure that info describes

 ==========================*/

void	xList(configVar_t info[], int count, void *data, int mask)
{
	int	index, n;

	if (!info || !data)
		return;

	for (index = 0; index < count; index++)
	{
		if (!(info[index].types & mask))
			continue;

		switch (info[index].type)
		{
		case T_DEFAULT:
			break;

		case T_INT:
			if (info[index].nameList)
				core.Console_Printf("%s: %s\n", info[index].name, info[index].nameList[INT_OFFSET(data, info, index)]);
			else
				core.Console_Printf("%s: %i\n", info[index].name, INT_OFFSET(data, info, index));
			break;

		case T_INTRANGE:
			core.Console_Printf("%s: %i - %i\n", info[index].name, INTRANGE_OFFSET(data, info, index).min, INTRANGE_OFFSET(data,info,index).max);
			break;

		case T_FLOAT:
			core.Console_Printf("%s: %f\n", info[index].name, FLOAT_OFFSET(data, info, index));
			break;

		case T_FLOATRANGE:
			core.Console_Printf("%s: %f - %f\n", info[index].name, FLOATRANGE_OFFSET(data, info, index).min, FLOATRANGE_OFFSET(data,info,index).max);
			break;

		case T_MODEL:
		case T_DYNSTRING:
			core.Console_Printf("%s: %s\n", info[index].name, *((char**)((char*)(data) + info[index].offset)));
			break;

		case T_STRING:
			core.Console_Printf("%s: %s\n", info[index].name, STRINGP_OFFSET(data, info, index));
			break;

		case T_FLAGS:
			if (!info[index].nameList)
			{
				core.Console_Printf("ERROR: No names listed for flags!\n");
				break;
			}

			core.Console_Printf("%s: ", info[index].name);
			for (n = 0; n < info[index].max; n++)
			{
				if (!info[index].nameList[n])
					break;
				if (!stricmp(info[index].nameList[n], ""))
					break;
				core.Console_Printf("%c%s ", (INT_OFFSET(data, info, index) & (1 << n)) ? '+' : '-', info[index].nameList[n]);
			}
			core.Console_Printf("\n");
			break;

		case T_ARRAY:
			if (arrayindex == -1)
			{
				for (n = 0; n < info[index].max; n++)
				{
					if (info[index].nameList)
					{
						if ( !stricmp(((char**)info[index].nameList)[n], "") )
							break;
						core.Console_Printf("%s: <%s> %i\n", info[index].name, info[index].nameList[n], (*(int*)((char*)(data) + info[index].offset + sizeof(int) * n)) );
					}
					else
						core.Console_Printf("%s: <%i> %i\n", info[index].name, n, (*(int*)((char*)(data) + info[index].offset + sizeof(int) * n)) );
				}
			}
			else
				core.Console_Printf("%s: %i\n", info[index].name, (*(int*)((char*)(data) + info[index].offset + sizeof(int) * arrayindex)) );
			break;
		}
	}
}

//=============================================================================
void	xSave(file_t	*f, configVar_t info[], int count, void *data, const char *pre, int mask)
{
	int		index, n;

	for (index = 0; index < count; index++)
	{
		//name is derived from the filename, so don't write it in the file
		if (!stricmp(info[index].name, "name"))
			continue;

		if (!(info[index].types & mask))
			continue;

		switch (info[index].type)
		{
		case T_DEFAULT:
			break;
		case T_INT:
			if (info[index].nameList)
				core.File_Printf(f, "%sSet %s \"%s\"\n", pre, info[index].name, info[index].nameList[INT_OFFSET(data, info, index)]);
			else
				core.File_Printf(f, "%sSet %s %i\n", pre, info[index].name, INT_OFFSET(data, info, index));
			break;
		case T_INTRANGE:
			core.File_Printf(f, "%sSet %s %i %i\n", pre, info[index].name, INTRANGE_OFFSET(data, info, index).min, INTRANGE_OFFSET(data, info, index).max);
			break;
		case T_FLOAT:
			core.File_Printf(f, "%sSet %s %f\n", pre, info[index].name, FLOAT_OFFSET(data, info, index));
			break;
		case T_FLOATRANGE:
			core.File_Printf(f, "%sSet %s %f %f\n", pre, info[index].name, FLOATRANGE_OFFSET(data, info, index).min, FLOATRANGE_OFFSET(data, info, index).max);
			break;
		case T_DYNSTRING:
		case T_MODEL:
			core.File_Printf(f, "%sSet %s \"%s\"\n", pre, info[index].name, *((char**)((char*)(data) + info[index].offset)));
			break;
		case T_STRING:
			core.File_Printf(f, "%sSet %s \"%s\"\n", pre, info[index].name, STRINGP_OFFSET(data, info, index));
			break;
		case T_FLAGS:
			core.File_Printf(f, "%sSet %s ", pre, info[index].name);
			for (n = 0; n < info[index].max; n++)
			{
				if (!stricmp(((char**)info[index].nameList)[n], ""))
					break;
				core.File_Printf(f, "%c%s ", (INT_OFFSET(data, info, index) & (1 << n)) ? '+' : '-', ((char**)info[index].nameList)[n]);
			}
			core.File_Printf(f, "\n");
			break;
		case T_ARRAY:
			for (n = 0; n < info[index].max; n++)
			{
				if (info[index].nameList)
				{
					if ( !stricmp(((char**)info[index].nameList)[n], "") )
						break;
					core.File_Printf(f, "%sSet %s %s %i\n", pre, info[index].name, info[index].nameList[n], (*(int*)((char*)(data) + info[index].offset + sizeof(int) * n)) );
				}
				else
					core.File_Printf(f, "%sSet %s %i %i\n", pre, info[index].name, n, (*(int*)((char*)(data) + info[index].offset + sizeof(int) * n)) );
			}
			break;
		}
	}
}

//=============================================================================
void	xInit(configVar_t info[], int count, void *data, int id, bool dynstringsonly)
{
	int		index, a;
	char	*argv[CMD_MAX_ARGS];
	char	def[CMD_MAX_LENGTH];
	char	*at;

	//initialize argv array to null
	memset(argv, 0, sizeof(char*) * CMD_MAX_ARGS);

	for (index = 0; index < count; index++)
	{
		if (info[index].type != T_DYNSTRING &&
			info[index].type != T_MODEL &&
			dynstringsonly)
		{
			continue;
		}

		strcpy(def, fmt(info[index].def, id));
		
		argv[0] = info[index].name;
		argv[1] = def;

		a = 2;
		at = def;

		if (info[index].type != T_DYNSTRING)
		{
			while (strchr(at, ' '))
			{
				at = strchr(at, ' ');
				*at = 0;
				at = argv[a] = at + 1;
				a++;
			}
		}

		if (info[index].type == T_DEFAULT)
		{
			if (!strlen(info[index].def))
			{
				info[index].type = T_INT;
				strcpy(def, fmt("%i", info[index].max));
			}
			else
			{
				info[index].type = T_STRING;
			}
			xSetVar(info, count, data, index, a, argv);
			info[index].type = T_DEFAULT;
		}
		else
		{
			if (info[index].type == T_ARRAY)
			{
				int		n;
				char	d[CMD_MAX_LENGTH];

				strcpy(d, argv[1]);
				//strcpy(argv[2], argv[1]);
				argv[2] = d;
				a++;
				for (n = 0; n < info[index].max; n++)
				{
					if (info[index].nameList)
						if (stricmp(info[index].nameList[n], ""))
							break;

					strcpy(argv[1], fmt("%i", n));
					xSetVar(info, count, data, index, a, argv);
				}
			}
			else
			{
				xSetVar(info, count, data, index, a, argv);
			}
		}
	}
}
//=============================================================================
//=============================================================================


//=============================================================================
//obj* console commands
//=============================================================================
int	objEditCurrent = 0;

//=============================================================================
void	objEdit_Cmd(int argc, char *argv[])
{
	int	last = objEditCurrent;
	int	index = 0;

	if (!stricmp(argv[0], "new"))
	{
		for (index = 1; index < MAX_OBJECT_TYPES; index++)
		{
			if (!objData[index].objclass)
			{
				objEditCurrent = index;
				break;
			}
		}

		if (index == MAX_OBJECT_TYPES)
		{
			core.Console_Printf("*** objEdit could not find a new object! ***\n");
			return;
		}
	}
	else
	{
		SetInt(&objEditCurrent, 1, LAST_OBJECT_TYPE, objectNames, argv[0]);
	}

	if (!ValidateObjectNumber(objEditCurrent))
		objEditCurrent = last;

	if (!objData[objEditCurrent].touched)
	{
		xInit(objectData_desc, countConfigVars(objectData_desc), &objData[objEditCurrent], objEditCurrent, false);
		objData[objEditCurrent].type = objEditCurrent;
		objData[objEditCurrent].touched = true;
	}
}


//=============================================================================
void	objList_Cmd(int argc, char *argv[])
{
	if (objEditCurrent == 0)
	{
		core.Console_Printf("Object 0 is selected.\n");
		return;
	}

	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for object #%i: %s\n", objEditCurrent, objectNames[objEditCurrent]);
	core.Console_Printf("=========================================\n");
	xList(objectData_desc, countConfigVars(objectData_desc), &objData[objEditCurrent], (1<<objData[objEditCurrent].objclass));
}

//=============================================================================
void	objSave_Cmd(int argc, char *argv[])
{
	char	fname[_MAX_PATH];
	file_t *f;
	int n;
	int	index = objEditCurrent;
	bool all = false;

	if (argc && !stricmp(argv[0], "all"))
	{
		index = 1;
		all = true;
	}

	if (objEditCurrent == 0)
	{
		core.Console_Printf("Cannot save object 0.\n");
		return;
	}

	while (objData[index].objclass)
	{
		strcpy(fname, fmt("objects/%s.object", objData[index].name));

		//open it
		f = core.File_Open(fname, "w");
		if (!f)
		{
			core.Console_Printf("Couldn't open %s for writing\n", fname);
			return;
		}

		//write the config
		xSave(f, objectData_desc, countConfigVars(objectData_desc), &objData[index], "obj", (1<<objData[index].objclass));
		
		if (objData[index].objclass == OBJCLASS_UNIT)
		{
			core.File_Printf(f, "\n//Attacks:\n");
			for (n = 1; n < NUM_MELEE_ATTACKS; n++)
			{
				core.File_Printf(f, "atkEdit %s\n", GetAnimName(n));
				xSave(f, attackData_desc, countConfigVars(attackData_desc), &objData[index].attacks[n], "atk", ALL);
				core.File_Printf(f, "\n");
			}
		}

		core.File_Close(f);
		core.Console_Printf("Wrote %s\n", fname);
		
		if (!all)
			break;

		index++;
	}
}



/*==========================

  objPut_Cmd

  syntax: opjPut <field> <cvar>
  sets <cvar> to whatever is contained in <field> of the currently selected object

 ==========================*/

void	objPut_Cmd(int argc, char *argv[])
{
	char string[1024];

	if (objEditCurrent == 0)
	{
		core.Console_Printf("Cannot retrieve data from object 0.\n");
		return;
	}

	if (argc < 2)
		return;

	if (xMakeString(objectData_desc, countConfigVars(objectData_desc), &objData[objEditCurrent], argv[0], argv[2], string, 1024))
		core.Cvar_Set(argv[1], string);
	else
		core.Console_Printf("Object setting not found or unsupported by objPut\n");
}


static bool breakObjectLoop;

void	objBreakLoop_Cmd(int argc, char *argv[])
{
	breakObjectLoop = true;
}

//objloop loops through all objects which match a specified criteria and performs a command
void	objLoop_Cmd(int argc, char *argv[])
{
	int n;
	int inc;
	int includeMode, wrapMode;
	int oldObjEditCurrent = objEditCurrent;
	static bool returnloop = false;
	
	breakObjectLoop = false;

	if (returnloop)
	{
		core.Console_Printf("Nested object loops are not allowed!!\n");
		return;
	}

	if (argc < 5 || (argc % 2 == 0))
	{
		core.Console_Printf("syntax:\n\nobjloop <INCLUDE|EXCLUDE> start_object <WRAP|NOWRAP> <FORWARD|BACKWARD> [obj_settingname1 obj_setting1 ... obj_settingnameX obj_settingX] command\n\n");
		core.Console_Printf("example:\n\nobjloop 1 objclass unit \"objset builder command_center\"\nloops through all objects with objclass \"unit\" and sets isWorker to 1 (i.e. make all units AI units)\n");
		return;
	}

	if (stricmp(argv[0], "INCLUDE")==0)
	{
		includeMode = true;
	}
	else if (stricmp(argv[0], "EXCLUDE")==0)
	{
		includeMode = false;
	}
	else
	{
		core.Console_Printf("Expected either INCLUDE or EXCLUDE for first parameter\n");
		return;
	}

	if (stricmp(argv[2], "WRAP")==0)
	{
		wrapMode = true;
	}
	else if (stricmp(argv[2], "NOWRAP")==0)
	{
		wrapMode = false;
	}
	else
	{
		core.Console_Printf("Expected either WRAP or NOWRAP for third parameter\n");
		return;
	}

	if (stricmp(argv[3], "FORWARD")==0)
	{
		inc = 1;
	}
	else if (stricmp(argv[3], "BACKWARD")==0)
	{
		inc = -1;
	}
	else
	{
		core.Console_Printf("Expected either FORWARD or BACKWARD for fourth parameter\n");
		return;
	}

	SetInt(&objEditCurrent, 1, MAX_OBJECT_TYPES-1, objectNames, argv[1]);
	if (!includeMode)
		objEditCurrent += inc;

	for (n = 1; n < MAX_OBJECT_TYPES; n++, objEditCurrent+=inc)
	{
		char string[1024];
		int a;
		bool performCommand = true;

		if (objEditCurrent >= MAX_OBJECT_TYPES)
		{
			if (wrapMode)
			{
				objEditCurrent = 1;
			}
			else
				break;
		}
		if (objEditCurrent <= 0)
		{
			if (wrapMode)
			{
				objEditCurrent = MAX_OBJECT_TYPES-1;
			}
			else
				break;
		}

		if (!objData[objEditCurrent].objclass)
			continue;

		for (a = 4; a < argc-1; a+=2)
		{
			if (!xMakeString(objectData_desc, countConfigVars(objectData_desc), &objData[objEditCurrent], argv[a], NULL, string, 1024))
			{
				//core.Console_Printf("objLoop object has no variable called %s\n", argv[a]);
				performCommand = false;
				break;
			}

			if (!stricmp(string, argv[a+1])==0)
			{
				performCommand = false;
				break;
			}
		}

		if (performCommand)
		{
			returnloop = true;
			core.Cmd_Exec(argv[argc-1]);
			returnloop = false;
			if (breakObjectLoop)		//objBreakLoop was called during the command
			{				
				break;
			}
		}		
	}

	objEditCurrent = oldObjEditCurrent;	
}

//=============================================================================
void	objSet_Cmd(int argc, char *argv[])
{
	if (objEditCurrent == 0)
	{
		core.Console_Printf("Cannot edit obejct 0.\n");
		return;
	}

	xSet(objectData_desc, countConfigVars(objectData_desc), &objData[objEditCurrent], argc, argv);
}
//=============================================================================
//=============================================================================


//=============================================================================
//atk* console commands
//=============================================================================
int	atkEditCurrent = 0;		//stores index of current attack

//=============================================================================
void	atkEdit_Cmd(int argc, char *argv[])
{
	if (argc < 1)
		return;

	SetInt(&atkEditCurrent, 1, NUM_MELEE_ATTACKS-1, animNames, argv[0]);
}

//=============================================================================
void	atkList_Cmd(int argc, char *argv[])
{
	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for attack #%i: %s of object #%i: %s\n", atkEditCurrent, animNames[atkEditCurrent], objEditCurrent, objectNames[objEditCurrent]);
	core.Console_Printf("=========================================\n");
	xList(attackData_desc, countConfigVars(attackData_desc), &(objData[objEditCurrent].attacks[atkEditCurrent]), ALL);
}

//=============================================================================
void	atkSet_Cmd(int argc, char *argv[])
{
	xSet(attackData_desc, countConfigVars(attackData_desc), &(objData[objEditCurrent].attacks[atkEditCurrent]), argc, argv);
}
//=============================================================================
//=============================================================================

int	GetObjectTypeByName(const char *name)
{
	int index = 1;

	while (/*objData[index].objclass &&*/ index < MAX_OBJECT_TYPES)
	{
		if (!stricmp(objData[index].name, name))
			return index;
		index++;
	}

	return 0;
}

objectData_t	*GetObjectByName(const char *name)
{
	return &objData[GetObjectTypeByName(name)];
}

objectData_t	*GetObjectByType(int index)
{
	if (index < 1 || index >= MAX_OBJECT_TYPES)
		return &objData[0];

	return &objData[index];
}

int	CountActiveObjects()
{
	int index = 1;

	while (objData[index].objclass)
		index++;

	return index - 1;
}

//=============================================================================
// Initialization
//=============================================================================
bool	IsObjectLoaded(char *name)
{
	return (GetObjectByName(name)->objclass);
}

void	objFileCallback(const char *filename, void *userdata)
{
	char fname[_MAX_PATH];

	strcpy(fname, Filename_GetFilename((char*)filename));
	Filename_StripExtension(fname, fname);

	//don't reload an object that was already loaded from objlist.cfg
	if (IsObjectLoaded(fname))
		return;

	core.Cmd_Exec("objedit new");
	core.Cmd_Exec(fmt("objset name %s", fname));
	core.Cmd_Exec(fmt("exec /objects/%s", filename));
}

void	objLoad_Cmd(int argc, char *argv[])
{
	char fname[256];

	if (argc < 1)
		return;

	if (argc > 1)
	{
		int	index;

		strncpySafe(fname, fmt("%s", argv[1]), sizeof(fname));
		if (IsObjectLoaded(fname))
			return;
		
		index = atoi(argv[0]);
		if (index < 1 || index >= MAX_OBJECT_TYPES)
			return;

		core.Cmd_Exec(fmt("objedit %i", index));
		core.Cmd_Exec(fmt("objset name %s", fname));
		core.Cmd_Exec(fmt("exec /objects/%s.object", fname));
	}
	else
	{
		strncpySafe(fname, fmt("%s.object", argv[0]), sizeof(fname));
		objFileCallback(fname, NULL);
	}
}
//=============================================================================
void	InitObjectDefinitions()
{
	int	n = 0, x;

	core.Console_Printf(" * Initializing Objects...\n");

	core.Cmd_Register("objEdit",		objEdit_Cmd);
	core.Cmd_Register("objSet",			objSet_Cmd);
	core.Cmd_Register("objList",		objList_Cmd);
	core.Cmd_Register("objSave",		objSave_Cmd);
	core.Cmd_Register("objLoad",		objLoad_Cmd);
	core.Cmd_Register("objPut",			objPut_Cmd);
	core.Cmd_Register("objLoop",		objLoop_Cmd);
	core.Cmd_Register("objBreakLoop",	objBreakLoop_Cmd);

	core.Cmd_Register("atkEdit",	atkEdit_Cmd);
	core.Cmd_Register("atkSet",		atkSet_Cmd);
	core.Cmd_Register("atkList",	atkList_Cmd);

	core.Cmd_Register("listObjects",	listObjects_Cmd);
	core.Cmd_Register("listEvents",		listEvents_Cmd);

	memset(objData, 0, sizeof(objectData_t) * MAX_OBJECT_TYPES);
	for (n = 0; n < MAX_OBJECT_TYPES; n++)
	{
		xInit(objectData_desc, countConfigVars(objectData_desc), &objData[n], n, false);
		for (x = 0; x < NUM_MELEE_ATTACKS; x++)
			xInit(attackData_desc, countConfigVars(attackData_desc), &(objData[n].attacks[x]), x, false);
	}

	for (n = 0; n < MAX_OBJECT_TYPES; n++)
		objectNames[n] = objData[n].name;

	//load objects listed in objlist.cfg
#ifdef SAVAGE_DEMO
	core.Cmd_Exec("exec objects/objlist_demo.cfg");
#else
	core.Cmd_Exec("exec objects/objlist.cfg");
#endif	//SAVAGE_DEMO
}
//=============================================================================
