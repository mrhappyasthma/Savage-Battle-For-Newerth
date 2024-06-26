/*
 * (C) 2002 S2 Games
 *
 * cl_objects.c
 */

#include "client_game.h"

int		CL_GetObjectTypeByName(const char *name)
{
	int index = 1;

	while (index < MAX_OBJECT_TYPES)
	{
		if (stricmp(cl.objData[index]->name, name)==0)
			return index;
		index++; 
	}

	return 0;
}

char *CL_GetGoalString(int goal)
{
	char *goalname = NULL;
	switch (goal)
	{
		case GOAL_NONE:
		case GOAL_COMPLETED:
			goalname = "";
			break;
		case GOAL_REPAIR:
			goalname = "Repair";
			break;
		case GOAL_MINE:
			goalname = "Mine Resources";
			break;
		case GOAL_ATTACK_OBJECT:
			goalname = "Attack";
			break;
		case GOAL_FOLLOW:
			goalname = "Follow";
			break;
		case GOAL_DEFEND:
			goalname = "Defend";
			break;
		case GOAL_REACH_WAYPOINT:
			goalname = "Travel to";
			break;
		case GOAL_CONSTRUCT_BUILDING:
			goalname = "Construct";
			break;
		case GOAL_DROPOFF_RESOURCES:
			goalname = "Drop off Resources";
			break;
		case GOAL_ENTER_BUILDING:
			goalname = "Enter Building";
			break;
		case GOAL_ENTER_TRANSPORT:
			goalname = "Enter Transport";
	  	default:
			goalname = "Unknown";
		  	break;
	}
	return goalname;
}



/*==========================

  CL_PlayGoalSound

  fixme: add beast sounds

 ==========================*/

void CL_PlayGoalSound(int sender, int goal, int objecttype)
{	
	char *snd = "";
	char *rank;

	//if sender is the team's commander or ourself (auto waypoint), use the commander voice, otherwise use the officer voice
	if (cl.teams[cl.info->team].commander == sender || goal == GOAL_DROPOFF_RESOURCES || cl.clients[cl.clientnum].waypoint.fake_waypoint)
	{	
		rank = "cmdr";
	}
	else
	{
		rank = "officer";
	}

	switch (goal)
	{
		case GOAL_NONE:
			snd = CL_RaceSnd(fmt("%s_order_cancel", rank));
			break;
		case GOAL_COMPLETED:
			snd = CL_RaceSnd(fmt("%s_order_complete", rank));
			break;
		case GOAL_MINE:
			snd = CL_RaceSnd(fmt("%s_order_mine", rank));
			break;
		case GOAL_ATTACK_OBJECT:
		{
			objectData_t *def = CL_ObjectType(objecttype);
			char *specific = CL_RaceSnd(fmt("%s_order_attack_%s", rank, def->name));

			if (*specific)
			{
				snd = specific;
			}
			else if (def->isSiegeWeapon)
			{
				snd = CL_RaceSnd(fmt("%s_order_attack_siege", rank));
			}
			else if (def->objclass == OBJCLASS_BUILDING)
			{
				snd = CL_RaceSnd(fmt("%s_order_attack_building", rank));
			}	
			else if (def->objclass == OBJCLASS_UNIT)
			{
				snd = CL_RaceSnd(fmt("%s_order_attack_unit", rank));
			}
			
			if (!snd[0])
			{
				snd = CL_RaceSnd(fmt("%s_order_attack", rank));
			}
			break;
		}
		case GOAL_FOLLOW:
			//snd = CL_RaceSnd("order_follow");
			//break;
			
			//intentional fallthrough
		case GOAL_DEFEND:
		{
			objectData_t *def = CL_ObjectType(objecttype);
			char *specific = CL_RaceSnd(fmt("%s_order_defend_%s", rank, def->name));			

			if (*specific)
			{
				snd = specific;
			}
			else if (def->isSiegeWeapon)
			{
				snd = CL_RaceSnd(fmt("%s_order_defend_siege", rank));
			}
			else if (def->objclass == OBJCLASS_BUILDING)
			{
				snd = CL_RaceSnd(fmt("%s_order_defend_building", rank));
			}	
			else if (def->objclass == OBJCLASS_UNIT)
			{
				snd = CL_RaceSnd(fmt("%s_order_defend_unit", rank));
			}

			if (!snd[0])			
			{
				snd = CL_RaceSnd(fmt("%s_order_defend", rank));
			}
			break;
		}
		case GOAL_REACH_WAYPOINT:
			snd = CL_RaceSnd(fmt("%s_order_move", rank));
			break;
		case GOAL_CONSTRUCT_BUILDING:
			snd = CL_RaceSnd(fmt("%s_order_build", rank));
			break;
		case GOAL_REPAIR:
			snd = CL_RaceSnd(fmt("%s_order_repair", rank));
			break;
		case GOAL_DROPOFF_RESOURCES:
			snd = CL_RaceSnd(fmt("%s_order_dropoff_resources", rank));
			break;
		case GOAL_ENTER_BUILDING:
			snd = CL_RaceSnd(fmt("%s_order_enter_building", rank));
			break;
		case GOAL_ENTER_TRANSPORT:
			snd = CL_RaceSnd(fmt("%s_order_enter_transport", rank));
			break;
	  	default:
			snd = CL_RaceSnd(fmt("%s_order_move", rank));
		  	break;
	}
  
	if (*snd)
		CL_Play2d(snd, 1.0, CHANNEL_GOAL);
}


int CL_GetGoalFromString(char *goaltext)
{
	int goal = 0;

	if (stricmp(goaltext, "mine") == 0)
		goal = GOAL_MINE;
	else if (stricmp(goaltext, "attack") == 0)
		goal = GOAL_ATTACK_OBJECT;
	else if (stricmp(goaltext, "defend") == 0)
		goal = GOAL_DEFEND;
	else if (stricmp(goaltext, "reach_object") == 0)
		goal = GOAL_REACH_WAYPOINT;
	else if (stricmp(goaltext, "follow_object") == 0)
		goal = GOAL_FOLLOW;
//	else if (stricmp(goaltext, "spawn_building") == 0)
//		goal = GOAL_SPAWN_BUILDING;
	else if (stricmp(goaltext, "construct") == 0)
		goal = GOAL_CONSTRUCT_BUILDING;
	else if (stricmp(goaltext, "waypoint") == 0)
		goal = GOAL_REACH_WAYPOINT;
	else if (stricmp(goaltext, "enter") == 0)
		goal = GOAL_ENTER_BUILDING;
	else if (stricmp(goaltext, "enter_transport") == 0)
		goal = GOAL_ENTER_TRANSPORT;
	return goal;
}
