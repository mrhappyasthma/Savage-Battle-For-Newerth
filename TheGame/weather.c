
// (C) 2003 S2 Games

// weather.c

//=============================================================================
// Client side weather effects
//=============================================================================
#include "game.h"


//=============================================================================
//=============================================================================
char *weclassnames[] =
{
	"null",
	"effect",

	""
};

#define NUL	0x01
#define PRT	0x02
#define	BEM	0x04

#define NULX	0x01
#define SNDX	0x02
#define	EMTX	0x04
#define	ATRX	0x08
#define	TRAX	0x10

#define ALL 0xff

weatherEffect_t	weatherEffectData[MAX_WEATHER_EFFECTS];

configVar_t	weatherEffectData_desc[] =
{
	{"name",			0,			MAX_NAME_LEN,	"",			NULL,	T_STRING,		offsetof(weatherEffect_t, name),			ALL },
	{""}
};

configVar_t weatherSubEffectData_desc[] =
{
	{"startZType",		0,			3,				"0",		NULL,	T_INT,			offsetof(weatherSubEffect_t, startZType),		ALL },
	{"startZ",			0,			1000,			"50.0",		NULL,	T_FLOAT,		offsetof(weatherSubEffect_t, startZ),			ALL },
	{"blockHeight",		0,			5000,			"50.0",		NULL,	T_FLOAT,		offsetof(weatherSubEffect_t, blockHeight),	ALL },
	{"blockSize",		0,			5000,			"200.0",	NULL,	T_FLOAT,		offsetof(weatherSubEffect_t, blockSize),		ALL },
	{"numBlocks",		0,			6,				"0",		NULL,	T_INT,			offsetof(weatherSubEffect_t, numBlocks),		ALL },
	{"interval",		0,			100000,			"200.0",	NULL,	T_INT,			offsetof(weatherSubEffect_t, interval),		ALL },
	{"effect",			0,			MAX_NAME_LEN,	"",			NULL,	T_STRING,		offsetof(weatherSubEffect_t, effect),			ALL },
	{"loopingSound",	0,			MAX_NAME_LEN,	"",			NULL,	T_STRING,		offsetof(weatherSubEffect_t, loopingSound),			ALL },
	{"loopingSoundVolume",	0,		1,				"1.0",		NULL,	T_FLOAT,		offsetof(weatherSubEffect_t, loopingSoundVolume),	ALL },
	{""}
};

//=============================================================================


//=============================================================================
//ef* console commands
//=============================================================================
int	weEditCurrent = 0;		//stores index of current particle system
int wfEditCurrent = 0;		//stores the index of the current effect

int		weLookup(char *name)
{
	int idx = 0;

	for (idx = 1; idx < MAX_WEATHER_EFFECTS; idx++)
	{
		if (stricmp(weatherEffectData[idx].name, name) == 0)
		{
			return idx;
		}
	}

	return -1;
}

//=============================================================================
void	weEdit_Cmd(int argc, char *argv[])
{
	int	index = 0;

	if (!stricmp(argv[0], "new"))
	{
		for (index = 1; index < MAX_WEATHER_EFFECTS; index++)
		{
			if (!weatherEffectData[index].name[0])
			{
				weEditCurrent = index;
				break;
			}
		}

		if (index == MAX_WEATHER_EFFECTS)
		{
			core.Console_DPrintf("*** weEdit could not find a new effect! ***\n");
			return;
		}
	}
	else
	{
		if (!argc)
		{
			core.Console_Printf("usage: weEdit <name>\n");
			return;
		}

		for (index = 1; index < MAX_WEATHER_EFFECTS; index++)
		{
			if (stricmp(weatherEffectData[index].name, argv[0]) == 0)
			{
				weEditCurrent = index;
				return;
			}
		}
		core.Console_Printf("weEdit: no weather data found matching the name %s\n", argv[0]);
	}
}

//=============================================================================
void	weList_Cmd(int argc, char *argv[])
{
	//int n;

	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for weather effect #%i: %s\n", weEditCurrent, weatherEffectData[weEditCurrent].name);
	core.Console_Printf("=========================================\n");
	xList(weatherEffectData_desc, countConfigVars(weatherEffectData_desc), &weatherEffectData[weEditCurrent], ALL);

	{
		core.Console_Printf("\nWeather effect %i:\n", wfEditCurrent);
		core.Console_Printf("===================\n");
		xList(weatherSubEffectData_desc, countConfigVars(weatherSubEffectData_desc), &weatherEffectData[weEditCurrent].effects[wfEditCurrent], ALL);
	}
}

//=============================================================================
void	weSave_Cmd(int argc, char *argv[])
{
	char	fname[_MAX_PATH];
	file_t *f;
	int	index = weEditCurrent;
	bool all = false;
	int n;

	if (argc && !stricmp(argv[0], "all"))
	{
		index = 1;
		all = true;
	}

	while (weatherEffectData[index].name[0])
	{
		strcpy(fname, fmt("weather/%s.weather", weatherEffectData[index].name));

		//open it
		f = core.File_Open(fname, "w");
		if (!f)
		{
			core.Console_Printf("Couldn't open %s for writing\n", fname);
			return;
		}

		//write the config
		xSave(f, weatherEffectData_desc, countConfigVars(weatherEffectData_desc), &weatherEffectData[index], "we", ALL);
		
		core.File_Printf(f, "\n//Sub Effects:\n");
		for (n = 0; n < MAX_WEATHER_SUB_EFFECTS; n++)
		{
			core.File_Printf(f, "wfEdit %i\n", n);
			xSave(f, weatherSubEffectData_desc, countConfigVars(weatherSubEffectData_desc), &weatherEffectData[index].effects[n], "wf", ALL);
			core.File_Printf(f, "\n");
		}
		
		core.File_Close(f);
		core.Console_Printf("Wrote %s\n", fname);
		
		if (!all)
			break;

		index++;
	}
}

//=============================================================================
void	weSet_Cmd(int argc, char *argv[])
{
	xSet(weatherEffectData_desc, countConfigVars(weatherEffectData_desc), &weatherEffectData[weEditCurrent], argc, argv);
}




//=============================================================================
// wfSet stuff - weather sub-effects
//=============================================================================
void	wfList_Cmd(int argc, char *argv[])
{
	//int n;

	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for weather sub-effect #%i: %s\n", wfEditCurrent, weatherEffectData[weEditCurrent].effects[wfEditCurrent]);
	core.Console_Printf("=========================================\n");
	{
		core.Console_Printf("\nWeather effect %i:\n", wfEditCurrent);
		core.Console_Printf("===================\n");
		xList(weatherSubEffectData_desc, countConfigVars(weatherSubEffectData_desc), &weatherEffectData[weEditCurrent].effects[wfEditCurrent], ALL);
	}
}

//=============================================================================
void	wfSet_Cmd(int argc, char *argv[])
{
	xSet(weatherSubEffectData_desc, countConfigVars(weatherSubEffectData_desc), &weatherEffectData[weEditCurrent].effects[wfEditCurrent], argc, argv);
}

void    wfEdit_Cmd(int argc, char *argv[])
{
	if (argc < 1)
		return;

	SetInt(&wfEditCurrent, 0, MAX_WEATHER_SUB_EFFECTS - 1, NULL, argv[0]);
}



//=============================================================================
//=============================================================================

/*==========================

  weFileCallback

  Initializes a weather effect file

 ==========================*/

void	weFileCallback(const char *filename, void *userdata)
{
	char fname[_MAX_PATH];

	strcpy(fname, Filename_GetFilename((char*)filename));
	Filename_StripExtension(fname, fname);

	core.Cmd_Exec("weEdit new");
	core.Cmd_Exec(fmt("weset name %s", fname));
	core.Cmd_Exec(fmt("exec /weather/%s", filename));
}

/*==========================

  InitWeather

 ==========================*/

void	InitWeather()
{
	int n, index;

	core.Console_Printf(" * Initializing weather...\n");

	core.Cmd_Register("weEdit",	weEdit_Cmd);
	core.Cmd_Register("weSet",	weSet_Cmd);
	core.Cmd_Register("weList",	weList_Cmd);
	core.Cmd_Register("weSave",	weSave_Cmd);
	core.Cmd_Register("wfEdit",	wfEdit_Cmd);
	core.Cmd_Register("wfSet",	wfSet_Cmd);
	core.Cmd_Register("wfList",	wfList_Cmd);

	memset(weatherEffectData, 0, sizeof(weatherEffect_t) * MAX_WEATHER_EFFECTS);
	for (n = 0; n < MAX_WEATHER_EFFECTS; n++)
	{
		xInit(weatherEffectData_desc, countConfigVars(weatherEffectData_desc), &weatherEffectData[n], n, true);
		for (index = 0; index < MAX_WEATHER_SUB_EFFECTS; index++)
			xInit(weatherSubEffectData_desc, countConfigVars(weatherSubEffectData_desc), &(weatherEffectData[n].effects[index]), index, true);
	}

	core.System_Dir("weather", "*.weather", false, NULL, weFileCallback, NULL);	
}
