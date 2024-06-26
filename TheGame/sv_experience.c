/*
 * (C) 2003 S2 Games
 *
 * sv_experience.c'
 */

#include "server_game.h"

// Functions to handle the setup of the experience/level rewards table
//=============================================================================

cvar_t sv_xp_adjust_by_level =		{ "sv_xp_adjust_by_level",		"0.0" };	//reward += reward * adjust_by_level * (target_level - killer_level)
cvar_t sv_xp_allow_level_loss =		{ "sv_xp_allow_level_loss",		"0" };		//will allow an exp penalty to lower the targets level
cvar_t sv_xp_cap_at_level =			{ "sv_xp_cap_at_level",			"0" };		//truncates rewards if the reward raises to a new level
cvar_t sv_xp_max_gain =				{ "sv_xp_max_gain",				".2" };		//percent of next level that can be gained in one reward
cvar_t sv_xp_max_level =			{ "sv_xp_max_level",			"99" };		//stop rewarding xp at this level

cvar_t sv_xp_kill_npc =				{ "sv_xp_kill_npc",				"1" };		// * level
cvar_t sv_xp_kill_worker =			{ "sv_xp_kill_worker",			"2" };		// * level
cvar_t sv_xp_kill_player =			{ "sv_xp_kill_player",			"10" };
cvar_t sv_xp_kill_item =			{ "sv_xp_kill_item",			"0" };		
cvar_t sv_xp_kill_siege =			{ "sv_xp_kill_siege",			"20" };		
cvar_t sv_xp_revive_player =		{ "sv_xp_revive_player",		"5" };
cvar_t sv_xp_heal_player =			{ "sv_xp_heal_player",			"0.0333" };
cvar_t sv_xp_structure_damage =		{ "sv_xp_structure_damage",		".00667" };	//total exp for killing a whole building
cvar_t sv_xp_raze =					{ "sv_xp_raze",					"50" };		//bonus for the last hit
cvar_t sv_xp_die =					{ "sv_xp_die",					"0" };		
cvar_t sv_xp_team_kill =			{ "sv_xp_team_kill",			"0" };		//if sv_teamDamage is one
cvar_t sv_xp_repair =				{ "sv_xp_repair",				"0" };		//for a full repair
cvar_t sv_xp_mine =					{ "sv_xp_mine",					".02" };	//per resource gathered
cvar_t sv_xp_build =				{ "sv_xp_build",				".00333" };	//multiplied by percent of contribution
cvar_t sv_xp_survival =				{ "sv_xp_survival",				"1" };		//points for living through the survival interval
cvar_t sv_xp_survival_interval =	{ "sv_xp_survival_interval",	"30000" };	//survival time period between receiving survival points
cvar_t sv_xp_honor =				{ "sv_xp_honor",				"5" };		//3 kills
cvar_t sv_xp_skill =				{ "sv_xp_skill",				"10" };		//5 kills
cvar_t sv_xp_hero =					{ "sv_xp_hero",					"15" };		//10 kills
cvar_t sv_xp_legend =				{ "sv_xp_legend",				"25" };		//15 kills

cvar_t sv_xp_commander_order_interval =	{ "sv_xp_commander_order_interval",	"5000" };	//amount to award commander for giving an order in 
cvar_t sv_xp_commander_order_given =	{ "sv_xp_commander_order_given",	"1" };		//how often to award order_given and order_followed
cvar_t sv_xp_commander_order_followed =	{ "sv_xp_commander_order_followed",	"1" };		//given to commander when a player follows his order
cvar_t sv_xp_commander_powerup_given =	{ "sv_xp_commander_powerup_given",	"5" };		//for buffing a player
cvar_t sv_xp_commander_structure =		{ "sv_xp_commander_structure",		"20" };		//for building a structure
cvar_t sv_xp_commander_research =		{ "sv_xp_commander_research",		"10" };		//for researching an items/wepaon/unit
cvar_t sv_xp_commander_raze =			{ "sv_xp_commander_raze",			"50" };		//your team destroyed a building
cvar_t sv_xp_commander_gather =			{ "sv_xp_commander_gather",			".001" };	//per resource gathered
cvar_t sv_xp_commander_demolish =		{ "sv_xp_commander_demolish",		"-20" };
cvar_t sv_xp_commander_request_ignore =	{ "sv_xp_commander_request_ignore",	"-1" };

/*==========================

  SV_AdjustStatsForCurrentLevel

  Sets the current value for all adjustable stats, based on 

 ==========================*/

void	SV_AdjustStatsForCurrentLevel(serverObject_t *obj, int level)
{
	int index;
	experienceData_t	*xpData;

	if (!obj)
		return;

	SV_FillInBaseStats(obj);

	xpData = experienceTable[SV_GetRace(obj->base.index)];

	for (index = 2; index <= level; index++)
	{
		obj->adjustedStats.buildRate +=			xpData[index].buildrate;
		obj->adjustedStats.repairRate +=		xpData[index].repairrate;
		obj->adjustedStats.maxCarryBonus +=		xpData[index].maxcarry;
		obj->adjustedStats.fullhealth +=		xpData[index].health;
		obj->adjustedStats.meleeDamageBonus +=	xpData[index].damage;
		obj->adjustedStats.meleeRangeBonus +=	xpData[index].range;
		obj->adjustedStats.maxStamina +=		xpData[index].stamina;
		obj->adjustedStats.bldPierce +=			xpData[index].bldpierce;
		obj->adjustedStats.unitPierce +=		xpData[index].unitpierce;
		obj->adjustedStats.siegePierce +=		xpData[index].siegepierce;
		obj->adjustedStats.blockPower +=		xpData[index].blockpower;
		
		obj->adjustedStats.blockPower = MAX(0.0, MIN(1.0, obj->adjustedStats.blockPower));
	}

	//adjust client specific stuff
	if (obj->client)
	{
		float healthpercent;

		healthpercent = obj->client->ps.health / (float)obj->client->ps.fullhealth;
		obj->client->ps.fullhealth = obj->adjustedStats.fullhealth;
		obj->client->ps.health = obj->client->ps.fullhealth * healthpercent;

		obj->client->ps.maxstamina = obj->adjustedStats.maxStamina;
	}
}


/*==========================

  SV_AddExperience

  Adds experience to a player, also handles advancing their level

 ==========================*/

void	SV_AddExperience(client_t *client, experienceRewards_enum rewardType, int param, float multiplier)
{	
	float reward = 0.0;
	int	lvl, accumulator = 0;
	experienceData_t	*xpData;

	//saftey check the client pointer
	if (!client)
		return;

	//get the xp array to use
	xpData = experienceTable[SV_GetRace(client->index)];

	//check for max level
	if (client->ps.score.level >= sv_xp_max_level.integer || client->ps.score.level >= MAX_EXP_LEVEL)
	{
		client->ps.score.level = MIN(sv_xp_max_level.integer, MAX_EXP_LEVEL);
		return;
	}

	//determine reward
	switch (rewardType)
	{
	case EXPERIENCE_NPC_KILL:
		reward = sv_xp_kill_npc.value * param;
		reward += reward * sv_xp_adjust_by_level.value * (param - client->ps.score.level);
		break;

	case EXPERIENCE_WORKER_KILL:
		reward = sv_xp_kill_worker.value * param;
		reward += reward * sv_xp_adjust_by_level.value * (param - client->ps.score.level);
		break;

	case EXPERIENCE_PLAYER_KILL:
		reward = sv_xp_kill_player.value;
		reward += reward * sv_xp_adjust_by_level.value * (param - client->ps.score.level);
		break;	

	case EXPERIENCE_ITEM_KILL:
		reward = sv_xp_kill_item.value;
		reward += reward * sv_xp_adjust_by_level.value * (param - client->ps.score.level);
		break;

	case EXPERIENCE_SIEGE_KILL:
		reward = sv_xp_kill_siege.value;
		break;

	case EXPERIENCE_PLAYER_REVIVE:
		reward = sv_xp_revive_player.value;
		break;

	case EXPERIENCE_PLAYER_HEAL:
		reward = sv_xp_heal_player.value * param;
		break;

	case EXPERIENCE_STRUCTURE_DAMAGE:
		reward = sv_xp_structure_damage.value * param;
		break;
	
	case EXPERIENCE_STRUCTURE_RAZE:
		reward = sv_xp_raze.value;
		break;

	case EXPERIENCE_MINE:
		reward = sv_xp_mine.value * param;
		break;

	case EXPERIENCE_REPAIR:
		reward = sv_xp_repair.value * param;
		break;

	case EXPERIENCE_BUILD:
		reward = sv_xp_build.value * param;
		break;

	case EXPERIENCE_BONUS:
		reward = param;
		break;

	case EXPERIENCE_SURVIVAL:
		reward = sv_xp_survival.value;
		break;

	case EXPERIENCE_COMMANDER_ORDER_GIVEN:
		reward = sv_xp_commander_order_given.value;
		break;

	case EXPERIENCE_COMMANDER_ORDER_FOLLOWED:
		reward = sv_xp_commander_order_followed.value;
		break;

	case EXPERIENCE_COMMANDER_POWERUP:
		reward = sv_xp_commander_powerup_given.value;
		break;

	case EXPERIENCE_COMMANDER_STRUCTURE:
		reward = sv_xp_commander_structure.value;
		break;

	case EXPERIENCE_COMMANDER_RESEARCH:
		reward = sv_xp_commander_research.value;
		break;

	case EXPERIENCE_COMMANDER_RAZE:
		reward = sv_xp_commander_raze.value;
		break;

	case EXPERIENCE_COMMANDER_GATHER:
		reward = sv_xp_commander_gather.value * param;
		break;

	case EXPERIENCE_COMMANDER_DEMOLISH:
		reward = sv_xp_commander_demolish.value;
		break;

	case EXPERIENCE_COMMANDER_IGNORE_REQUEST:
		reward = sv_xp_commander_request_ignore.value;
		break;

	default:
		core.Game_Error("SV_AddExperience(): Invalid experience reward type: %i\n", rewardType);
	}

	//apply multiplier from object setting
	reward *= multiplier;

	//no negative exp
	if (reward <= 0.0)
		return;

	//check for cappping
	if (reward / (float)xpData[client->ps.score.level + 1].points > sv_xp_max_gain.value)
		reward = xpData[client->ps.score.level + 1].points * sv_xp_max_gain.value;

	//add the reward
	client->ps.score.experience += reward;
	client->stats.xpTypes[rewardType] += reward;

	//determine new level
	for (lvl = 2; lvl <= MIN(sv_xp_max_level.integer, MAX_EXP_LEVEL); lvl++)
	{
		accumulator += xpData[lvl].points;

		if (client->ps.score.experience < accumulator)
		{
			lvl--;
			break;
		}
	}	

	//check for a level increase
	if (lvl > client->ps.score.level)
	{
		client->ps.score.level = lvl;
		SV_Phys_AddEvent(client->obj, EVENT_LEVEL_UP, 0, 0);
		SV_ClientEcho(client->index, fmt("%s\n", xpData[lvl].rewardtext));

		if (sv_xp_cap_at_level.integer)
			client->ps.score.experience = accumulator - xpData[lvl].points;
		
		//do other reward stuff here
		SV_AdjustStatsForCurrentLevel(client->obj, lvl);
	}
}


/*==========================

  SV_SubtractExperience

  Remove experience from a player and handle level adjustment

  ==========================*/

void	SV_SubtractExperience(client_t *client, int amount)
{
	int	level, race, floorval, newlevel;
	int	accumulator = 0;
	experienceData_t	*xpData;

	level = client->ps.score.level;
	race = SV_GetRace(client->index);

	xpData = experienceTable[race];

	floorval = Exp_GetTotalPointsForLevel(race, level);

	client->ps.score.experience -= amount;
	
	//lost a level, potentially
	if (client->ps.score.experience < floorval)
	{
		if (sv_xp_allow_level_loss.integer && level > 1)
		{
			//time to drop a lavel
			for (newlevel = 2; newlevel <= MIN(sv_xp_max_level.integer, MAX_EXP_LEVEL); newlevel++)
			{
				accumulator += xpData[newlevel].points;

				if (client->ps.score.experience < accumulator)
				{
					newlevel--;
					break;
				}
			}
			client->ps.score.level = newlevel;
			SV_AdjustStatsForCurrentLevel(client->obj, newlevel);
		}
		else
		{
			//level loss is not allowed or we are level 1, so
			//just bottom out the experience
			client->ps.score.experience = floorval;
		}
	}
}


/*==========================

  SV_GiveExp_Cmd

  Console command to give a player experience

 ==========================*/

void	SV_GiveExp_Cmd(int argc, char *argv[])
{
	int		client, amount;
	float	tmp = sv_xp_max_gain.value;

	if (!cores.Cvar_GetValue("svr_allowcheats"))
		return;

	//nothing specified, just give a bunch of xp to client 0
	if (argc < 1)
	{
		client = 0;
		amount = 1000;
	}
	//assume argument is amount, target is client 0
	else if (argc < 2)
	{
		client = 0;
		amount = MAX(0, atoi(argv[0]));
	}
	//specify a client and an amount
	else if (argc >= 2)
	{
		client = MAX(0, MIN(MAX_CLIENTS - 1, atoi(argv[0])));
		amount = MAX(0, atoi(argv[1]));
	}

	if (!sl.clients[client].active)
		return;

	cores.Cvar_SetVarValue(&sv_xp_max_gain, 1.0);
	SV_AddExperience(&sl.clients[client], EXPERIENCE_BONUS, 1000, 1.0);
	cores.Cvar_SetVarValue(&sv_xp_max_gain, tmp);
}


void	SV_WriteExpCfg_Cmd(int argc, char *argv[])
{
	cores.Cmd_Exec("writeconfig configs/experience.cfg sv_xp_");
}


/*==========================

  SV_InitExperience

 ==========================*/

void	SV_InitExperience()
{
	cores.Cvar_Register(&sv_xp_adjust_by_level);
	cores.Cvar_Register(&sv_xp_allow_level_loss);
	cores.Cvar_Register(&sv_xp_cap_at_level);
	cores.Cvar_Register(&sv_xp_max_gain);
	cores.Cvar_Register(&sv_xp_max_level);

	cores.Cvar_Register(&sv_xp_kill_npc);
	cores.Cvar_Register(&sv_xp_kill_worker);
	cores.Cvar_Register(&sv_xp_kill_player);
	cores.Cvar_Register(&sv_xp_kill_item);
	cores.Cvar_Register(&sv_xp_kill_siege);
	cores.Cvar_Register(&sv_xp_revive_player);
	cores.Cvar_Register(&sv_xp_heal_player);

	cores.Cvar_Register(&sv_xp_structure_damage);
	cores.Cvar_Register(&sv_xp_raze);
	cores.Cvar_Register(&sv_xp_die);
	cores.Cvar_Register(&sv_xp_team_kill);
	cores.Cvar_Register(&sv_xp_repair);
	cores.Cvar_Register(&sv_xp_mine);
	cores.Cvar_Register(&sv_xp_build);
	cores.Cvar_Register(&sv_xp_survival);
	cores.Cvar_Register(&sv_xp_survival_interval);
	cores.Cvar_Register(&sv_xp_honor);
	cores.Cvar_Register(&sv_xp_skill);
	cores.Cvar_Register(&sv_xp_hero);
	cores.Cvar_Register(&sv_xp_legend);

	cores.Cvar_Register(&sv_xp_commander_order_interval);
	cores.Cvar_Register(&sv_xp_commander_order_given);
	cores.Cvar_Register(&sv_xp_commander_order_followed);
	cores.Cvar_Register(&sv_xp_commander_powerup_given);
	cores.Cvar_Register(&sv_xp_commander_structure);
	cores.Cvar_Register(&sv_xp_commander_research);
	cores.Cvar_Register(&sv_xp_commander_raze);
	cores.Cvar_Register(&sv_xp_commander_gather);
	cores.Cvar_Register(&sv_xp_commander_demolish);
	cores.Cvar_Register(&sv_xp_commander_request_ignore);

	cores.Cmd_Exec("exec /configs/experience.cfg");

	cores.Cmd_Register("giveexp",		SV_GiveExp_Cmd);
	cores.Cmd_Register("writeExpCfg",	SV_WriteExpCfg_Cmd);
}
