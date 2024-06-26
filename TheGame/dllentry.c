// (C) 2001 S2 Games

// dllentry.c


#include <Windows.h>
#include "game.h"
#include "cl_translator.h"


int initLocale();

SGetText* gettext     = NULL;
SGetText* getstring   = NULL;
static HMODULE s_hLib = NULL;



coreAPI_shared_t core;

/*	This is the main entry point into the game DLL.
	Here the core engine passes its API to us, and we pass
	back our own API. */

int	InitGameDLL(coreAPI_shared_t *core_api_shared)
{

	memcpy(&core, core_api_shared, sizeof(coreAPI_shared_t));

	if (NUM_EVENTS > 64)
	{
		core.Game_Error("NUM_EVENTS > 64\n");
		return DLLTYPE_GAME;
	}

	core.Cvar_AllowCheats();

	//since the game has its own copy of savage_mathlib.c, we need to init this copy
	M_Init();

	if (!initLocale())
		return DLLTYPE_GAME;
  
	//make sure all physics transmit variables get registered
	Phys_Init();
	
	InitMiscShared();

	core.Console_Printf("Initializing game...\n");
	InitEffects();
	InitWeather();
	InitStates();
	InitMetaConfig();
	Exp_Init();

	InitObjectDefinitions();

	PostProcessUnits();
	
	//load object definitions so we can link objects that were placed in the editor to game objects
	InitObjdefReferences();

	core.Cvar_BlockCheats();

	return DLLTYPE_GAME;
}

void	ShutdownGameDLL()
{
	if (NULL != s_hLib)
		FreeLibrary(s_hLib);

	core.Console_DPrintf("ShutdownGameDLL()\n");
}



int initLocale()
{
	s_hLib = LoadLibrary("Lokalizator.dll");

	if (NULL == s_hLib)
	{
		core.Game_Error("Lokalizator.dll is missing!\n");
		return FALSE;
	}

	gettext   = (SGetText*)GetProcAddress(s_hLib,"_gettext@4");
	getstring = (SGetText*)GetProcAddress(s_hLib,"_getstring@4");

	if (NULL == gettext || NULL == getstring)
	{
		core.Game_Error("Lokalizator.dll - wrong version!!!\n");
		return FALSE;
	}

	 return TRUE;
}

