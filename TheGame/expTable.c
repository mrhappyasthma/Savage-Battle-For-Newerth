/*
 * (C) 2003 S2 Games
 *
 * exptable.c'
 */

#include "game.h"

// Setup of the experience/level rewards table
//=============================================================================
experienceData_t experienceTable[MAX_RACES][MAX_EXP_LEVEL+1];

configVar_t	experience_desc[] =
{
	{"points",		1,	NO_LIMIT,	"100",						NULL,	T_INT,			offsetof(experienceData_t, points),			ALL },
	{"rewardtext",	0,	NO_LIMIT,	"You have gained a level",	NULL,	T_DYNSTRING,	offsetof(experienceData_t, rewardtext),		ALL },
	{"buildRate",	0,	NO_LIMIT,	"0",						NULL,	T_INT,			offsetof(experienceData_t, buildrate),		ALL },
	{"repairRate",	0,	NO_LIMIT,	"0",						NULL,	T_INT,			offsetof(experienceData_t, repairrate),		ALL },
	{"maxCarry",	0,	NO_LIMIT,	"0",						NULL,	T_INT,			offsetof(experienceData_t, maxcarry),		ALL },
	{"bodyModel",	0,	NO_LIMIT,	"",							NULL,	T_DYNSTRING,	offsetof(experienceData_t, bodymodel),		ALL },
	{"health",		0,	NO_LIMIT,	"0",						NULL,	T_INT,			offsetof(experienceData_t, health),			ALL },
	{"rMeleeModel",	0,	NO_LIMIT,	"",							NULL,	T_DYNSTRING,	offsetof(experienceData_t, rmeleemodel),	ALL },
	{"lMeleeModel",	0,	NO_LIMIT,	"",							NULL,	T_DYNSTRING,	offsetof(experienceData_t, lmeleemodel),	ALL },
	{"damage",		0,	NO_LIMIT,	"0",						NULL,	T_INT,			offsetof(experienceData_t, damage),			ALL },
	{"range",		0,	NO_LIMIT,	"0",						NULL,	T_INT,			offsetof(experienceData_t, range),			ALL },
	{"stamina",		0,	NO_LIMIT,	"0",						NULL,	T_INT,			offsetof(experienceData_t, stamina),		ALL },
	{"blockPower",	0,	NO_LIMIT,	"0",						NULL,	T_FLOAT,		offsetof(experienceData_t, blockpower),		ALL },
	{"bldPierce",	0,	NO_LIMIT,	"0",						NULL,	T_FLOAT,		offsetof(experienceData_t, bldpierce),		ALL },
	{"unitPierce",	0,	NO_LIMIT,	"0",						NULL,	T_FLOAT,		offsetof(experienceData_t, unitpierce),		ALL },
	{"siegePierce",	0,	NO_LIMIT,	"0",						NULL,	T_FLOAT,		offsetof(experienceData_t, siegepierce),	ALL },
	{"warcry",		0,	1,			"1",						NULL,	T_INT,			offsetof(experienceData_t, warcry),			ALL },
	{""}
};

int expEdit = 1;
int	expRace = 1;

void	Exp_Edit_Cmd(int argc, char *argv[])
{
	SetInt(&expEdit, 1, MAX_EXP_LEVEL, NULL, argv[0]);
}

void	Exp_Race_Cmd(int argc, char *argv[])
{
	SetInt(&expRace, FIRST_RACE, LAST_RACE, raceNames, argv[0]);
}

void	Exp_Set_Cmd(int argc, char *argv[])
{
	xSet(experience_desc, countConfigVars(experience_desc), &experienceTable[expRace][expEdit], argc, argv);
}

void	Exp_Save_Cmd(int argc, char *argv[])
{
	int		index;
	file_t	*f;
	char	filename[_MAX_PATH];

	strcpy(filename, fmt("configs/%s_exptable.cfg", raceNames[expRace]));
	f = core.File_Open(filename, "w");
	if (!f)
	{
		core.Console_DPrintf("Could not open %s\n", filename);
		return;
	}

	core.File_Printf(f, "expRace %s\n\n", raceNames[expRace]);
	for (index = 2; index <= MAX_EXP_LEVEL; index++)
	{
		core.File_Printf(f, "expEdit %i\n", index);
		xSave(f, experience_desc, countConfigVars(experience_desc), &experienceTable[expRace][index], "exp", ALL);
		core.File_Printf(f, "//==================\n\n");
	}

	core.File_Close(f);
}

void	Exp_List_Cmd(int argc, char *argv[])
{
	core.Console_Printf("Experience table for %s level %2i:\n", raceNames[expRace], expEdit);
	core.Console_Printf("----------------------------------\n");
	xList(experience_desc, countConfigVars(experience_desc), &experienceTable[expRace][expEdit], ALL);
}

int		Exp_GetTotalPointsForLevel(int race, int level)
{
	int	index;
	int accumulator = 0;

	//validate arguments
	if (race < FIRST_RACE || race > LAST_RACE || 
		level < 2 || level > MAX_EXP_LEVEL)
		return 0;

	for (index = 2; index <= level; index++)
		accumulator += experienceTable[race][index].points;

	return accumulator;
}

float	Exp_GetPercentNextLevel(int race, int level, int exp)
{
	//validate arguments
	if (race < FIRST_RACE || race > LAST_RACE || 
		level < 1 || level > MAX_EXP_LEVEL)
		return 0.0;

	exp -= Exp_GetTotalPointsForLevel(race, level);

	return (exp / (float)experienceTable[race][level + 1].points);
}

void	Exp_Init()
{
	int	index, n;

	core.Cmd_Register("expEdit",	Exp_Edit_Cmd);
	core.Cmd_Register("expRace",	Exp_Race_Cmd);
	core.Cmd_Register("expSet",		Exp_Set_Cmd);
	core.Cmd_Register("expSave",	Exp_Save_Cmd);
	core.Cmd_Register("expList",	Exp_List_Cmd);

	for (n = 0; n < MAX_RACES; n++)
		for (index = 0; index <= MAX_EXP_LEVEL; index++)
			xInit(experience_desc, countConfigVars(experience_desc), &experienceTable[n][index], index, false);

	for (n = 0; n < MAX_RACES; n++)
		core.Cmd_Exec(fmt("exec configs/%s_exptable.cfg", raceNames[n]));
}
