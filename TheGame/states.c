// (C) 2003 S2 Games
// states.c

#include "game.h"

//=============================================================================
// States
// These cover powerups and negative effects that can be applied to an object
// with a fixed or indefinate duration.  Includes stat modification, special
// effects and damaging/healing
//=============================================================================
stateData_t	stateData[MAX_STATES];
char		*stateNames[MAX_STATES];
int			editState = 0;

#define MODIFIER_INFO(a)	{#a"Add",	-NO_LIMIT,	NO_LIMIT,	"0",	NULL,	T_FLOAT,	offsetof(stateData_t, a##Add),	ALL	},\
							{#a"Mult",	-NO_LIMIT,	NO_LIMIT,	"1",	NULL,	T_FLOAT,	offsetof(stateData_t, a##Mult),	ALL	},

extern char* dmgFlagNames[];

configVar_t stateData_desc[] =
{
	{"index",		0,	MAX_STATES,			"%i",		NULL,	T_DEFAULT,		offsetof(stateData_t, index),		ALL },
	{"name",		0,	MAX_NAME_LEN,		"state%i",	NULL,	T_STRING,		offsetof(stateData_t, name),		ALL },
	{"active",		0,	0,					"",			NULL,	T_DEFAULT,		offsetof(stateData_t, active),		ALL },
	{"icon",		0,	0,					"",			NULL,	T_DYNSTRING,	offsetof(stateData_t, icon),		ALL },
	{"slot",		0,	MAX_STATE_SLOTS,	"0",		NULL,	T_INT,			offsetof(stateData_t, slot),		ALL },
	{"priority",	0,	NO_LIMIT,			"0",		NULL,	T_INT,			offsetof(stateData_t, priority),	ALL },

	MODIFIER_INFO(damage)
	MODIFIER_INFO(armor)
	MODIFIER_INFO(speed)
	MODIFIER_INFO(jump)
	MODIFIER_INFO(attackSpeed)
	MODIFIER_INFO(health)
	MODIFIER_INFO(regenRate)
	MODIFIER_INFO(regen)

	{"staminaRegenAdjust",	0,	NO_LIMIT,	"0",		NULL,				T_FLOAT,		offsetof(stateData_t, staminaRegenAdjust),	ALL },

	{"damage",			0,	NO_LIMIT,		"0",		NULL,				T_INT,			offsetof(stateData_t, damage),			ALL },
	{"damageFrequency",	0,	NO_LIMIT,		"0",		NULL,				T_INT,			offsetof(stateData_t, damageFrequency),	ALL },
	{"damageFlags",		0,	32,				"clear",	dmgFlagNames,		T_FLAGS,		offsetof(stateData_t, damageFlags),		ALL },
	
	{"cloak",			0,	1,				"0",		NULL,				T_FLOAT,		offsetof(stateData_t, cloak),			ALL },
	{"markEnemies",		0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, markEnemies),		ALL },
	{"lockdownTech",	0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, lockdownTech),	ALL },
	{"splashProtect",	0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, splashProtect),	ALL },

	{"isVulnerable",	0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, isVulnerable),	ALL },

	{"radiusDamage",	0,	NO_LIMIT,		"0",		NULL,				T_INT,			offsetof(stateData_t, radiusDamage),	ALL },
	{"radiusDamageFreq",	0,	NO_LIMIT,	"0",		NULL,				T_INT,			offsetof(stateData_t, radiusDamageFreq),	ALL },
	{"radius",			0,	NO_LIMIT,		"0",		NULL,				T_FLOAT,		offsetof(stateData_t, radius),			ALL },
	{"radiusTargets",	0,	16,				"clear",	TargetFlagNames,	T_FLAGS,		offsetof(stateData_t, radiusTargets),	ALL },
	{"radiusState",		0,	MAX_STATES-1,	"",			stateNames,			T_INT,			offsetof(stateData_t, radiusState),		ALL },

	{"model1",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, model[0]),		ALL },
	{"skin1",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, skin[0]),			ALL },
	{"bone1",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, bone[0]),			ALL },
	{"useCharAnim1",	0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, useCharAnim[0]),	ALL },

	{"model2",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, model[1]),		ALL },
	{"skin2",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, skin[1]),			ALL },
	{"bone2",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, bone[1]),			ALL },
	{"useCharAnim2",	0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, useCharAnim[1]),	ALL },

	{"model3",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, model[2]),		ALL },
	{"skin3",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, skin[2]),			ALL },
	{"bone3",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, bone[2]),			ALL },
	{"useCharAnim3",	0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, useCharAnim[2]),	ALL },

	{"model4",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, model[3]),		ALL },
	{"skin4",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, skin[3]),			ALL },
	{"bone4",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, bone[3]),			ALL },
	{"useCharAnim4",	0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, useCharAnim[3]),	ALL },

	{"model5",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, model[4]),		ALL },
	{"skin5",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, skin[4]),			ALL },
	{"bone5",			0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, bone[4]),			ALL },
	{"useCharAnim5",	0,	1,				"0",		NULL,				T_INT,			offsetof(stateData_t, useCharAnim[4]),	ALL },

	{"singleShader",	0,	0,				"",			NULL,				T_DYNSTRING,	offsetof(stateData_t, singleShader),	ALL },

	{"effect",			0,	MAX_EFFECTS + MAX_EXEFFECTS,	"",			exEffectNames,		T_INT,			offsetof(stateData_t, effect),			ALL },
	{"effectPeriod",	0,	NO_LIMIT,		"",			NULL,				T_INTRANGE,		offsetof(stateData_t, effectPeriod),	ALL },
	{""}
};


/*==========================

  stateEdit_cmd

 ==========================*/

void	stateEdit_cmd(int argc, char *argv[])
{
	int	index = 0;

	if (argc < 1)
		return;

	if (!stricmp(argv[0], "new"))
	{
		for (index = 1; index < MAX_STATES; index++)
		{
			if (!stateData[index].active)
			{
				editState = index;
				break;
			}
		}

		if (index >= MAX_STATES)
		{
			core.Console_DPrintf("Couldn't add new state.\n");
			return;
		}
	}
	else
	{
		SetInt(&editState, 1, MAX_STATES-1, stateNames, argv[0]);
	}
}


/*==========================

  stateSet_cmd

 ==========================*/

void	stateSet_cmd(int argc, char *argv[])
{
	if (xSet(stateData_desc, countConfigVars(stateData_desc), &stateData[editState], argc, argv))
		stateData[editState].active = true;
}


/*==========================

  stateList_cmd

 ==========================*/

void	stateList_cmd(int argc, char *argv[])
{
	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for state #%i: %s\n", editState, stateNames[editState]);
	core.Console_Printf("=========================================\n");
	xList(stateData_desc, countConfigVars(stateData_desc), &stateData[editState], ALL);
}


/*==========================

  stateSave_cmd

 ==========================*/

void	stateSave_cmd(int argc, char *argv[])
{
	char	fname[_MAX_PATH];
	file_t *f;
	int	index = editState;
	bool all = false;

	if (argc && !stricmp(argv[0], "all"))
	{
		index = 1;
		all = true;
	}

	while (stateData[index].active)
	{
		strcpy(fname, fmt("states/%s.state", stateData[index].name));

		//open it
		f = core.File_Open(fname, "w");
		if (!f)
		{
			core.Console_Printf("Couldn't open %s for writing\n", fname);
			return;
		}

		//write the config
		xSave(f, stateData_desc, countConfigVars(stateData_desc), &stateData[index], "state", ALL);
		
		core.File_Close(f);
		core.Console_Printf("Wrote %s\n", fname);
		
		if (!all)
			break;

		index++;
	}
}


/*==========================

  stateFileCallback

 ==========================*/

void	stateFileCallback(const char *filename, void *userdata)
{
	char fname[_MAX_PATH];

	//strcpy(fname, filename);
	strcpy(fname, Filename_GetFilename((char*)filename));
	Filename_StripExtension(fname, fname);

	core.Cmd_Exec("stateedit new");
	core.Cmd_Exec(fmt("stateset name %s", fname));
	core.Cmd_Exec(fmt("exec %s", filename));
}


/*==========================

  stateLoad_Cmd

 ==========================*/

void	stateLoad_Cmd(int argc, char *argv[])
{
	if (argc < 1)
		return;

	stateFileCallback(fmt("%s.state", argv[0]), NULL);
}


/*==========================

  statePut_Cmd

 ==========================*/

void	statePut_Cmd(int argc, char *argv[])
{
	char string[1024];

	if (editState == 0)
	{
		core.Console_Printf("Cannot retrieve data from state 0.\n");
		return;
	}

	if (argc < 2)
		return;

	if (xMakeString(stateData_desc, countConfigVars(stateData_desc), &stateData[editState], argv[0], argv[2], string, 1024))
		core.Cvar_Set(argv[1], string);
	else
		core.Console_Printf("Object setting not found or unsupported by objPut\n");
}


/*==========================

  listStates_Cmd

 ==========================*/

void	listStates_Cmd(int argc, char *argv[])
{
	int n;

	for (n = 1; n < MAX_STATES; n++)
	{
		if (!stateData[n].active)
			break;

		core.Console_Printf("#%3i: %s\n", n, stateNames[n]);
	}
}


/*==========================

  InitStates

 ==========================*/

void	InitStates()
{
	int	index;

	core.Console_Printf(" * Initializing states...\n");

	for (index = 0; index < MAX_STATES; index++)
		stateNames[index] = stateData[index].name;
	for (index = 0; index < MAX_STATES; index++)
		xInit(stateData_desc, countConfigVars(stateData_desc), &stateData[index], index, false);

	//commands
	core.Cmd_Register("stateLoad",		stateLoad_Cmd);
	core.Cmd_Register("stateEdit",		stateEdit_cmd);
	core.Cmd_Register("stateSet",		stateSet_cmd);
	core.Cmd_Register("stateList",		stateList_cmd);
	core.Cmd_Register("stateSave",		stateSave_cmd);
	core.Cmd_Register("listStates",		listStates_Cmd);
	core.Cmd_Register("statePut",		statePut_Cmd);

	//execute config file
	core.Cmd_Exec("exec states/statelist.cfg");
}
