// (C) 2003 S2 Games

// metaconfig.c

// commands and structures that define high level game play
//=============================================================================
#include "game.h"

//=============================================================================
// Resources
//=============================================================================
resourceData_t	resourceData[MAX_RESOURCE_TYPES];
char *resourceNames[MAX_RESOURCE_TYPES];
char goodieBag[MAX_OBJECT_NAME_LENGTH] = "";

/*==========================

  GetNumResources

  return number of currently active resources

 ==========================*/

int	GetNumResources()
{
	int count = 0;

	while (stricmp(resourceNames[count], ""))
		count++;

	return count;
}

int		GetResourceByName(char *resourceName)
{
	int index = 0;

	while (index < MAX_RESOURCE_TYPES && resourceNames[index])
	{
		if (stricmp(resourceNames[index], resourceName) == 0)
			return index;
		index++;
	}
	return -1;
}

char	*ResourceName(int index)
{
	if (index >= MAX_RESOURCE_TYPES || index < 0)
		return "UNKNOWN_RESOURCE";

	return resourceData[index].description;
}

/*==========================

  addResource_cmd

  adds a resource <name> to the resource list

 ==========================*/

void	addResource_cmd(int argc, char *argv[])
{
	int index = 0;

	if (argc < 1)
	{
		core.Console_Printf("Usage: cfg_addResource <name> [description] [icon file]\n");
		return;
	}

	//find first free slot
	index = GetNumResources();

	if (index < MAX_RESOURCE_TYPES)
	{
		//set internal name
		strcpy(resourceData[index].name, argv[0]);

		//set descriptive name
		if (argc > 1)
			DynSetString(&resourceData[index].description, argv[1]);
		else
			DynSetString(&resourceData[index].description, argv[0]);	//if none specified, use same as internal

		//set color
		if (argc > 4)
		{
			resourceData[index].red = atof(argv[2]);
			resourceData[index].green = atof(argv[3]);
			resourceData[index].blue = atof(argv[4]);
		}
		
		//set icon file
		if (argc > 5)
			DynSetString(&resourceData[index].icon, argv[5]);
		else
			resourceData[index].icon = NULL;

	}
	else
	{
		core.Console_DPrintf("Couldn't add resource \"%s\". (Too many resources!)\n", argv[0]);
	}
}

/*==========================

  listResources_cmd

  list all the active resources

 ==========================*/

void	listResources_cmd(int argc, char *argv[])
{
	int index = 0;

	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
	{
		if (!stricmp(resourceNames[index], ""))
			break;

		core.Console_Printf("%s\n", resourceNames[index]);
	}
}

//remove a resource
void	delResource_cmd(int argc, char *argv[])
{
	int index = 0;

	if (argc < 1)
	{
		core.Console_Printf("Usage: cfg_delResource <name>\n");
		return;
	}

	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
	{
		if (!stricmp(argv[0], resourceNames[index]))
		{
			DynFree(resourceData[index].name);
			DynFree(resourceData[index].description);
			DynFree(resourceData[index].icon);
			break;
		}
	}

	core.Console_Printf("Couldn't find resource \"%s\"\n", argv[1]);
}


char	*GetGoodieBagName()
{
	return goodieBag;
}

void	setGoodieBag_cmd(int argc, char *argv[])
{
	if (argc < 1)
	{
		core.Console_Printf("Usage: setGoodieBag <objectname>\n");
		return;
	}

	strncpySafe(goodieBag, argv[0], MAX_OBJECT_NAME_LENGTH);
}
//=============================================================================


//=============================================================================
// Races
//=============================================================================
raceData_t	raceData[MAX_RACES];
char		*raceNames[MAX_RACES];
int editRace = 0;

configVar_t	raceData_desc[] =
{
	{ "name",			0,	MAX_NAME_LEN,			"race%i",	NULL,			T_STRING,		offsetof(raceData_t, name),				ALL },
	{ "active",			0,	0,						"",			NULL,			T_DEFAULT,		offsetof(raceData_t, active),			ALL },
	{ "description",	0,	0,						"race%i",	NULL,			T_DYNSTRING,	offsetof(raceData_t, description),		ALL },
	{ "baseBuilding",	0,	0,						"",			NULL,			T_DYNSTRING,	offsetof(raceData_t, baseBuilding),		ALL },
	{ "resources",		0,	MAX_RESOURCE_TYPES,		"clear",	resourceNames,	T_FLAGS,		offsetof(raceData_t, resources),		ALL },
	{ "currency",		0,	MAX_RESOURCE_TYPES,		"gold",		resourceNames,	T_INT,			offsetof(raceData_t, currency),			ALL },
	{ "officerState",	0,	MAX_STATES,				"",			stateNames,		T_INT,			offsetof(raceData_t, officerState),		ALL },
	{ "ammoDrop",		0,	MAX_OBJECT_NAME_LENGTH,	"",			NULL,			T_DYNSTRING,	offsetof(raceData_t, ammoDrop),			ALL },
	{""}
};
//=============================================================================

raceData_t	*GetRaceDataByName(const char *name)
{
	int n;

	for (n = FIRST_RACE; n < MAX_RACES; n++)
	{
		if (stricmp(raceData[n].name, name)==0)
			return &raceData[n];
	}

	return &raceData[0];
}


void	raceEdit_cmd(int argc, char *argv[])
{
	int	index = 0;

	if (argc < 1)
		return;

	if (!stricmp(argv[0], "new"))
	{
		for (index = FIRST_RACE; index < MAX_RACES; index++)
		{
			if (!raceData[index].active)
			{
				editRace = index;
				break;
			}
		}

		if (index > LAST_RACE)
		{
			core.Console_DPrintf("Couldn't add new race.\n");
			return;
		}
	}
	else
	{
		SetInt(&editRace, FIRST_RACE, LAST_RACE, raceNames, argv[0]);
	}
}

void	raceSet_cmd(int argc, char *argv[])
{
	xSet(raceData_desc, countConfigVars(raceData_desc), &raceData[editRace], argc, argv);
}

void	raceList_cmd(int argc, char *argv[])
{
	core.Console_Printf("=========================================\n");
	core.Console_Printf("Data for race #%i: %s\n", editRace, raceNames[editRace]);
	core.Console_Printf("=========================================\n");
	xList(raceData_desc, countConfigVars(raceData_desc), &raceData[editRace], ALL);
}

void	raceActivate_cmd(int argc, char *argv[])
{
	if (editRace < FIRST_RACE || editRace > LAST_RACE)
		return;

	raceData[editRace].active = true;
}
//=============================================================================

//=============================================================================
void	saveConfig_cmd(int argc, char *argv[])
{
	int index;
	file_t *f;

	//open it
	f = core.File_Open("configs/main.cfg", "w");
	if (!f)
	{
		core.Console_DPrintf("Couldn't open configs/main.cfg for writing\n");
		return;
	}

	//write the resources
	core.File_Printf(f, "//============================\n");
	core.File_Printf(f, "// Resources\n");
	core.File_Printf(f, "//\n");
	core.File_Printf(f, "// Syntax: addResource <name> <descriptive name> <red> <green> <blue> <icon file>\n");
	core.File_Printf(f, "// RGB value indicates color associated with resource\n");
	core.File_Printf(f, "//============================\n");
	for (index = 0; index < GetNumResources(); index++)
	{
		core.File_Printf(f, "addResource \"%s\" \"%s\" %.02f %.02f %.02f", resourceNames[index], resourceData[index].description,
																	resourceData[index].red, resourceData[index].green, resourceData[index].blue);
		if (resourceData[index].icon)
			core.File_Printf(f, "\"%s\"", resourceData[index].icon);
		core.File_Printf(f, "\n");
	}
	core.File_Printf(f, "\n");

	//write the races
	core.File_Printf(f, "//============================\n");
	core.File_Printf(f, "// Races\n");
	core.File_Printf(f, "//============================\n");
	for (index = FIRST_RACE; index < MAX_RACES; index++)
	{
		if (!raceData[index].active)
			break;
		core.File_Printf(f, "raceEdit new\n");
		core.File_Printf(f, "raceSet name \"%s\"\n", raceNames[index]);
		xSave(f, raceData_desc, countConfigVars(raceData_desc), &raceData[index], "race", ALL);
		core.File_Printf(f, "raceActivate\n");
		core.File_Printf(f, "\n");
	}

	//save the drops
	core.File_Printf(f, "setGoodieBag %s\n", GetGoodieBagName());

	core.File_Close(f);
	core.Console_Printf("Wrote configs/main.cfg\n");
}
//=============================================================================






voiceMenu_t	voiceMenus[MAX_VOICECHAT_MENUS];

static bool loadingChat = false;
static int currentVoiceMenu = 0;
static int currentVoiceCategory = 0;
static int currentVoiceItem = 0;
static voiceMenu_t *cvm = NULL;


/*==========================

  VoiceDirectory_Cmd

  specify a base directory for voice sounds

 ==========================*/

void	VoiceDirectory_Cmd(int argc, char *argv[])
{
	if (!argc || !loadingChat || !cvm)
		return;

	strncpySafe(cvm->directory, argv[0], sizeof(cvm->directory));
}


/*==========================

  VoiceItem_Cmd

  specify a menu item

 ==========================*/

void	VoiceItem_Cmd(int argc, char *argv[])
{
	voiceItem_t *item;

	if (argc < 2 || !loadingChat || !cvm)
		return;

	currentVoiceItem++;
	if (currentVoiceItem >= MAX_VOICECHAT_ITEMS)
	{
		currentVoiceItem = MAX_VOICECHAT_ITEMS;
		return;
	}

	item = &cvm->categories[currentVoiceCategory].items[currentVoiceItem];

	item->number = atoi(argv[0]);
	strncpySafe(item->desc, argv[1], sizeof(item->desc));	

	cvm->categories[currentVoiceCategory].numItems = currentVoiceItem + 1;
}



/*==========================

  VoiceFlag_Cmd

  set team chat or global here

 ==========================*/

void	VoiceFlag_Cmd(int argc, char *argv[])
{
	int n;
	voiceItem_t *item;

	if (!argc || !loadingChat || currentVoiceCategory == -1 || !cvm)
		return;

	item = &cvm->categories[currentVoiceCategory].items[currentVoiceItem];

	for (n=0; n<argc; n++)
	{
		if (stricmp(argv[n], "global")==0)
		{
			item->flags |= VOICE_GLOBAL;
		}
		else if (stricmp(argv[n], "team")==0)
		{
			item->flags &= ~VOICE_GLOBAL;
		}
	}	
}


/*==========================

  VoiceSound_Cmd

  specify a sound to play and text to print

 ==========================*/

void	VoiceSound_Cmd(int argc, char *argv[])
{
	voiceItem_t *item;

	if (argc < 2 || !loadingChat || currentVoiceCategory == -1 || !cvm)
		return;

	item = &cvm->categories[currentVoiceCategory].items[currentVoiceItem];

	if (item->numSounds >= 4)
		return;	

	strncpySafe(item->vs[item->numSounds].sound, argv[0], sizeof(item->vs[item->numSounds].sound));
	strncpySafe(item->vs[item->numSounds].text, argv[1], sizeof(item->vs[item->numSounds].text));

	item->numSounds++;
}



/*==========================

  VoiceMore_Cmd

  specify a voice chat button that brings up a sub menu (causes sound and text to be ignored)

 ==========================*/

void	VoiceMore_Cmd(int argc, char *argv[])
{
	voiceItem_t *item;

	if (argc < 3 || !loadingChat || !cvm)
		return;

	currentVoiceItem++;
	if (currentVoiceItem >= MAX_VOICECHAT_ITEMS)
	{
		currentVoiceItem = MAX_VOICECHAT_ITEMS;
		return;
	}

	item = &cvm->categories[currentVoiceCategory].items[currentVoiceItem];

	item->number = atoi(argv[0]);
	strncpySafe(item->desc, argv[1], sizeof(item->desc));
	item->more = atoi(argv[2]);

	cvm->categories[currentVoiceCategory].numItems = currentVoiceItem + 1;

}


/*==========================

  VoiceCommander_Cmd

  this voicechat menu belongs to the commander of the specified race

 ==========================*/

void	VoiceCommander_Cmd(int argc, char *argv[])
{
	if (!argc || !cvm)
		return;

	if (!GetRaceDataByName(argv[0])->active)
		return;

	cvm->commander = true;
	cvm->race = GetRaceDataByName(argv[0])->index;
}



/*==========================

  VoiceLoad_Cmd

  load a .voice file

 ==========================*/

void	VoiceLoad_Cmd(int argc, char *argv[])
{
	if (!argc || !loadingChat)
		return;	

	if (currentVoiceMenu >= MAX_VOICECHAT_MENUS)
	{
		core.Console_Printf("Couldn't create voice menu %s\n", argv[0]);
		cvm = NULL;
		return;
	}

	cvm = &voiceMenus[currentVoiceMenu];
	memset(cvm, 0, sizeof(voiceMenu_t));

	strncpySafe(cvm->name, argv[0], sizeof(cvm->name));
			
	core.Cmd_Exec(fmt("exec /configs/voice/%s.voice", argv[0]));

	currentVoiceMenu++;	
	currentVoiceItem = -1;
}

/*==========================

  VoiceCategory_Cmd

  define a menu category

 ==========================*/

void	VoiceCategory_Cmd(int argc, char *argv[])
{
	int i;	

	if (!argc || !loadingChat || !cvm)
		return;

	i = atoi(argv[0]);
	if (i < 0 || i >= MAX_VOICECHAT_CATEGORIES)
		return;

	currentVoiceCategory = i;
	currentVoiceItem = -1;

	cvm->categories[i].active = true;
}


/*==========================

  GetNumVoiceMenus

 ==========================*/

int	GetNumVoiceMenus()
{
	return currentVoiceMenu + 1;
}


/*==========================

  GetVoiceMenu

  return the appropriate voice menu based on the given parameters

  make sure to check a NULL return here

 ==========================*/

voiceMenu_t *GetVoiceMenu(const char *name)
{
	int n;

	for (n=0; n<=currentVoiceMenu; n++)
	{
		if (stricmp(voiceMenus[n].name, name)==0)
			return &voiceMenus[n];
	}

	return NULL;
}


/*==========================

  InitVoiceChat

 ==========================*/

void	InitVoiceChat()
{
	core.Console_Printf(" * Initializing voice macros...\n");

	core.Cmd_Register("voiceLoad",		VoiceLoad_Cmd);
	core.Cmd_Register("voiceDirectory", VoiceDirectory_Cmd);
	core.Cmd_Register("voiceItem",		VoiceItem_Cmd);
	core.Cmd_Register("voiceMore",		VoiceMore_Cmd);	
	core.Cmd_Register("voiceCategory",	VoiceCategory_Cmd);
	core.Cmd_Register("voiceCommander", VoiceCommander_Cmd);	
	core.Cmd_Register("voiceSound", VoiceSound_Cmd);
	core.Cmd_Register("voiceFlag", VoiceFlag_Cmd);

	loadingChat = true;
	core.Cmd_Exec("exec configs/voice/voicelist.cfg");	
	loadingChat = false;
}


// Shared gameplay affecting cvars
//=============================================================================
cvar_t g_allUnitsAvailable =	{ "g_allUnitsAvailable",	"0", CVAR_TRANSMIT };
cvar_t g_allWeaponsAvailable =	{ "g_allWeaponsAvailable",	"0", CVAR_TRANSMIT };


//=============================================================================
void	InitMetaConfig()
{
	int	index;

	core.Console_Printf(" * Initializing game parameters...\n");

	//initialize data structures
	//resourceData
	memset(resourceData, 0, sizeof(resourceData));

	for (index = 0; index < MAX_RESOURCE_TYPES; index++)
		resourceNames[index] = resourceData[index].name;

	//raceData
	for (index = 0; index < MAX_RACES; index++)
	{
		xInit(raceData_desc, countConfigVars(raceData_desc), &raceData[index], index, false);
		raceData[index].index = index;
	}
	for (index = 0; index < MAX_RACES; index++)
		raceNames[index] = raceData[index].name;

	//add commands
	core.Cmd_Register("addResource",	addResource_cmd);
	core.Cmd_Register("listResources",	listResources_cmd);
	core.Cmd_Register("delResources",	delResource_cmd);
	core.Cmd_Register("raceEdit",		raceEdit_cmd);
	core.Cmd_Register("raceSet",		raceSet_cmd);
	core.Cmd_Register("raceList",		raceList_cmd);
	core.Cmd_Register("raceActivate",	raceActivate_cmd);
	core.Cmd_Register("saveConfig",		saveConfig_cmd);
	core.Cmd_Register("setGoodieBag",   setGoodieBag_cmd);	

	//game cvars
	core.Cvar_Register(&g_allWeaponsAvailable);
	core.Cvar_Register(&g_allUnitsAvailable);

#ifdef SAVAGE_DEMO
	core.Cmd_Exec("exec configs/main_demo.cfg");
#else	//SAVAGE_DEMO
	core.Cmd_Exec("exec configs/main.cfg");
#endif	//SAVAGE_DEMO

	InitVoiceChat();
}
//=============================================================================
