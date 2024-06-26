
// (C) 2003 S2 Games

// misc_shared.c


//these need to match the enum lists in game.h
//

#include "game.h"

cvar_t	showGamePerf = { "showGamePerf", "0" };

char	*gameTypes[] =
{
	"RTSS",
	"DM",
	"LOBBY",
	""
};

char *eventNames[] =
{
	"NONE",

	"wounded",
	"deflected",
	"dazed",
	"falling_damage",

	"jump",
	"jump_land",

	"bounce",
	"stop",

	"mine",
	"dropoff",
	"resource_full",

	"level_up",

	"spawn",
	"deathEvent",					//named this way to prevent conflict with animation name
	"deathEvent(quiet)",

	"anim_retrigger",				//retrigger the animState
	"anim2_retrigger",				//retrigger the animState2

	"task_finished",
	
	"use_item",
	"item_sleep",
	"item_idle",
	"item_activate",
	"fizzle",
	"backfire",

	"pickup_weapon",
	
	"quake",
	"weapon_fire",

	"weapon_hit",

	"goodie_pickup",
	"splodey_death",

	"commander_selected",

	"attack_pound",
	"attack_suicide",

	"powerup",

	"resurrected",

	""
};

char *animNames[] =
{
	"idle",

	"melee_1",
	"melee_2",
	"melee_3",
	"melee_4",
	"alt_melee_1",
	"alt_melee_2",
	"alt_melee_3",
	"alt_melee_4",
	"melee_charge",
	"melee_release",

	"melee_move_1",
	"melee_move_2",
	"melee_move_3",
	"melee_move_4",
	"melee_move_charge",
	"melee_move_release",

	"block",

	"walk_left",
	"walk_right",
	"walk_fwd",
	"walk_back",

	"run_left",
	"run_right",
	"run_fwd",
	"run_back",

	"sprint_left",
	"sprint_right",
	"sprint_fwd",
	"sprint_back",

	"jump_start_left",
	"jump_start_right",
	"jump_start_fwd",
	"jump_start_back",

	"jump_mid_left",
	"jump_mid_right",
	"jump_mid_fwd",
	"jump_mid_back",

	"jump_end_left",
	"jump_end_right",
	"jump_end_fwd",
	"jump_end_back",

	"jump_up_start",
	"jump_up_mid",
	"jump_up_end",

	"jump_land",

	"dodge_start_left",
	"dodge_start_right",
	"dodge_start_fwd",
	"dodge_start_back",

	"dodge_mid_left",
	"dodge_mid_right",
	"dodge_mid_fwd",
	"dodge_mid_back",

	"dodge_end_left",
	"dodge_end_right",
	"dodge_end_fwd",
	"dodge_end_back",

	"crouch_idle",

	"crouch_left",
	"crouch_right",
	"crouch_fwd",
	"crouch_back",

	"walk_with_bag",

	"mine",
	"repair",
	"construct",

	"wounded_left",
	"wounded_right",
	"wounded_fwd",
	"wounded_back",
	
	"death_generic",
	"death_left",
	"death_right",
	"death_fwd",
	"death_back",

	"resurrected",

	"weapon_idle_1",
	"weapon_idle_2",
	"weapon_idle_3",
	"weapon_idle_4",
	"weapon_idle_5",
	"weapon_idle_6",

	"weapon_charge_1",
	"weapon_charge_2",
	"weapon_charge_3",
	"weapon_charge_4",
	"weapon_charge_5",
	"weapon_charge_6",

	"weapon_fire_1",
	"weapon_fire_2",
	"weapon_fire_3",
	"weapon_fire_4",
	"weapon_fire_5",
	"weapon_fire_6",

	"weapon_reload_1",
	"weapon_reload_2",
	"weapon_reload_3",
	"weapon_reload_4",
	"weapon_reload_5",
	"weapon_reload_6",

	"weapon_switch",

	"wpstate_idle",
	"wpstate_switch",
	"wpstate_charge",
	"wpstate_spinup",
	"wpstate_spindown",
	"wpstate_overheat",
	"wpstate_fire",
	"wpstate_backfire",

	"item_sleep",
	"item_active",

	"construct_1",
	"construct_2",
	"construct_3",
	"construct_final",

	"suicide",

	""
};

//string tables
residx_t	msgTable;
residx_t	soundTable;
residx_t	texTable;
residx_t	mdlTable;
residx_t	uiTable;

char	*Snd(const char *id)
{	
	static char buf[2][1024];
	static int marker = 0;
	char *colon;
	char *ret = core.Str_Get(soundTable, id);

	marker = marker ^ 1;
	buf[marker][0] = 0;
	colon = strchr(ret, ':');
	if (colon)
	{
		int r = rand() % atoi(colon+1) + 1;

		strncpySafe(buf[marker], ret, colon-ret+1);		
		strcat(buf[marker], fmt("_%i.wav", r));

		return buf[marker];
	}
	else
		return ret;
}

char	*GameMsg(const char *id)
{
	return core.Str_Get(msgTable, id);
}

char	*Tex(const char *id)
{
	return core.Str_Get(texTable, id);
}

char	*UI(const char *id)
{
	return core.Str_Get(uiTable, id);
}

char	*GetAnimName(int animState)
{
	if (animState < 0 || animState >= NUM_ANIMSTATES)
		return "idle";

	return animNames[animState];
}

char	*Mdl(const char *id)
{
	return core.Str_Get(mdlTable, id);
}

char	*GetEventName(int event)
{
	if (event < 0 || event >= NUM_EVENTS)
		return "NONE";

	return eventNames[event];
}

bool	IsMovementAnim(int animState)
{
	switch(animState)
	{
		case AS_WALK_LEFT:
		case AS_WALK_RIGHT:
		case AS_WALK_BACK:
		case AS_WALK_FWD:
		case AS_RUN_LEFT:
		case AS_RUN_RIGHT:
		case AS_RUN_BACK:
		case AS_RUN_FWD:
		case AS_SPRINT_LEFT:
		case AS_SPRINT_RIGHT:
		case AS_SPRINT_BACK:
		case AS_SPRINT_FWD:
		case AS_WALK_WITH_BAG:
			return true;
		default:
			break;
	}

	return false;
}

bool	IsForwardMovementAnim(int animState)
{
	switch(animState)
	{		
		case AS_WALK_FWD:
		case AS_RUN_FWD:
		case AS_SPRINT_FWD:
		case AS_WALK_WITH_BAG:
			return true;
		default:
			break;
	}

	return false;
}

bool	IsSideMovementAnim(int animState)
{
	switch(animState)
	{		
		case AS_WALK_LEFT:
		case AS_WALK_RIGHT:
		case AS_RUN_LEFT:
		case AS_RUN_RIGHT:
		case AS_SPRINT_LEFT:
		case AS_SPRINT_RIGHT:
			return true;
		default:
			break;
	}

	return false;
}

bool	IsBackwardMovementAnim(int animState)
{
	switch(animState)
	{		
		case AS_WALK_BACK:
		case AS_RUN_BACK:
		case AS_SPRINT_BACK:
			return true;
		default:
			break;
	}

	return false;
}

bool	IsAttackAnim(int animState)
{
	if (animState >= AS_MELEE_1 && animState <= AS_MELEE_MOVE_RELEASE)
		return true;

	if (animState >= AS_ALT_MELEE_1 && animState <= AS_ALT_MELEE_4)
		return true;

	if (animState >= AS_WEAPON_FIRE_1 && animState <= AS_WEAPON_FIRE_4)
		return true;

	if (animState == AS_WPSTATE_FIRE)
		return true;

	return false;	
}

bool	IsLeftMovementAnim(int animState)
{
	switch(animState)
	{		
		case AS_WALK_LEFT:
		case AS_RUN_LEFT:
		case AS_SPRINT_LEFT:
			return true;
		default:
			break;
	}

	return false;
}

bool	IsRightMovementAnim(int animState)
{
	switch(animState)
	{		
		case AS_WALK_RIGHT:
		case AS_RUN_RIGHT:
		case AS_SPRINT_RIGHT:
			return true;
		default:
			break;
	}

	return false;
}

int		StringToGametype(const char *string)
{
	int n = 0;

	while(gameTypes[n][0])
	{
		if (stricmp(string, gameTypes[n]) == 0)
			return n;
		n++;
	}

	return 0;
}

char	*GetGametypeName(int gametype)
{
	if (gametype < 0 || gametype >= NUM_GAMETYPES)
		return "";

	return gameTypes[gametype];
}

double perf_counts[PERF_NUMTYPES];

void	Perf_Count(int perftype, double amount)
{
	if (perftype < 0 || perftype >= PERF_NUMTYPES)
		return;

	perf_counts[perftype] += amount * 1000;
}

void	Perf_ClearServer()
{
}

void	Perf_ClearClient()
{
	perf_counts[PERF_PREDICTION] = 0;
	perf_counts[PERF_PROCESSOBJECTS] = 0;
	perf_counts[PERF_POSEMODEL] = 0;
	perf_counts[PERF_RENDADDOBJECTS] = 0;
	perf_counts[PERF_CLIENTFRAME] = 0;
	perf_counts[PERF_RENDER] = 0;
	perf_counts[PERF_PARTICLES] = 0;
	perf_counts[PERF_DOEFFECT] = 0;
	perf_counts[PERF_SPECIALCHARACTERRENDERING] = 0;
	perf_counts[PERF_SPECIALSTRUCTURERENDERING] = 0;
	perf_counts[PERF_SPECIALPROJECTILERENDERING] = 0;
	perf_counts[PERF_SPECIALITEMRENDERING] = 0;
	perf_counts[PERF_ADDTOSCENE] = 0;
	perf_counts[PERF_ADDWORLDPROPS] = 0;
	perf_counts[PERF_SCR_SAMPLEBRIGHTNESS] = 0;
	perf_counts[PERF_SCR_STATES] = 0;
	perf_counts[PERF_SCR_ATTACHMENTS] = 0;
	perf_counts[PERF_SCR_ARMOR] = 0;
	perf_counts[PERF_SCR_CONTINUOUSBEAM] = 0;
	perf_counts[PERF_SCR_GROUNDMARKS] = 0;
}

void	Perf_Print()
{
	core.Console_Printf("==== Client Frame ====\n\n");
	core.Console_Printf("Prediction ms:            %.2f\n", perf_counts[PERF_PREDICTION]);
	core.Console_Printf("Process obj ms:          +%.2f\n", perf_counts[PERF_PROCESSOBJECTS]);
	core.Console_Printf("Pose model ms:           +%.2f\n", perf_counts[PERF_POSEMODEL]);
	core.Console_Printf("Add render objects ms:   +%.2f\n", perf_counts[PERF_RENDADDOBJECTS]);
	core.Console_Printf("  DoEffect:               %.2f\n", perf_counts[PERF_DOEFFECT]);
	core.Console_Printf("  Character:              %.2f\n", perf_counts[PERF_SPECIALCHARACTERRENDERING]);
	core.Console_Printf("    Sample brightness:    %.2f\n", perf_counts[PERF_SCR_SAMPLEBRIGHTNESS]);
	core.Console_Printf("    States:               %.2f\n", perf_counts[PERF_SCR_STATES]);
	core.Console_Printf("    Armor:                %.2f\n", perf_counts[PERF_SCR_ARMOR]);
	core.Console_Printf("    Attachments:          %.2f\n", perf_counts[PERF_SCR_ATTACHMENTS]);
	core.Console_Printf("    Ground marks:         %.2f\n", perf_counts[PERF_SCR_GROUNDMARKS]);
	core.Console_Printf("    Continuous beam:      %.2f\n", perf_counts[PERF_SCR_CONTINUOUSBEAM]);
	core.Console_Printf("  Structure:              %.2f\n", perf_counts[PERF_SPECIALSTRUCTURERENDERING]);
	core.Console_Printf("  Projectile:             %.2f\n", perf_counts[PERF_SPECIALPROJECTILERENDERING]);
	core.Console_Printf("  Item:                   %.2f\n", perf_counts[PERF_SPECIALITEMRENDERING]);
	core.Console_Printf("  World props:            %.2f\n", perf_counts[PERF_ADDWORLDPROPS]);
	core.Console_Printf("  Add to scene:           %.2f\n", perf_counts[PERF_ADDTOSCENE]);
	core.Console_Printf("Particle logic ms:       +%.2f\n", perf_counts[PERF_PARTICLES]);	
	core.Console_Printf("Render ms:               +%.2f\n", perf_counts[PERF_RENDER]);
	core.Console_Printf("Total client frame ms:   =%.2f\n", perf_counts[PERF_CLIENTFRAME]);	
	core.Console_Printf("\n");
}


/*==========================

  InitMiscShared

  count the eventNames and animNames tables to make sure they match up

 ==========================*/

void	InitMiscShared()
{
	int numstrings;

	numstrings=0;
	while(eventNames[numstrings][0])
		numstrings++;
	
	if (numstrings != NUM_EVENTS)
	{
		core.Game_Error("Event name strings don't match event enum\n");
	}

	numstrings=0;
	while(animNames[numstrings][0])
		numstrings++;

	if (numstrings != NUM_ANIMSTATES)
	{
		core.Game_Error("Anim name strings don't match animState enum\n");
	}

	numstrings=0;
	while(gameTypes[numstrings][0])
		numstrings++;

	if (numstrings != NUM_GAMETYPES)
	{
		core.Game_Error("Game type strings don't match gameType enum\n");
	}

	//load string tables
	soundTable = core.Res_LoadStringTable("stringtables/soundlist.str");
	if (!soundTable)
	{
		core.Game_Error("Couldn't load stringtables/soundlist.str\n");
	}

	texTable = core.Res_LoadStringTable("stringtables/texturelist.str");
	if (!texTable)
	{
		core.Game_Error("Couldn't load stringtables/texturelist.str\n");
	}

	msgTable = core.Res_LoadStringTable("stringtables/messages.str");
	if (!msgTable)
	{
		core.Game_Error("Couldn't load stringtables/messages.str\n");
	}

	mdlTable = core.Res_LoadStringTable("stringtables/modellist.str");
	if (!mdlTable)
	{
		core.Game_Error("Couldn't load stringtables/modellist.str\n");
	}

	uiTable = core.Res_LoadStringTable("stringtables/ui.str");
	if (!uiTable)
	{
		core.Game_Error("Couldn't load stringtables/ui.str\n");
	}

	core.Cvar_Register(&showGamePerf);

//	TodoList();
}

