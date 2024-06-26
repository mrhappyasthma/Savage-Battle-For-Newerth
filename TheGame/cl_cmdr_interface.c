/*
 * (C) 2003 S2 Games
 *
 * cl_cmdr_interface.c'
 */

#include "client_game.h"

#include "../gui/gui_slider.h"
#include "cl_translator.h"

#define CONTEXTMENU_UNIT_NAME_LENGTH 27

#define	MAX_GOAL_TEXT_LENGTH 64

#define MAX_VISIBLE_STATES 6 //max number of active states shown for the selected unit

cvar_t  unit_progress = { "unit_progress", "0" };

cvar_t	highlightedSelection = { "highlightedSelection", "0" };

static gui_element_t *unit0_model;
static gui_element_t *unit0_name;
static gui_element_t *unit0_health;
static gui_element_t *unit0_type;
static gui_element_t *unit0_kills;
static gui_element_t *unit0_goal;
static gui_element_t *unit0_level;

static gui_element_t *unit0_gold;
static gui_element_t *unit0_loyaltypoints;
static gui_element_t *unit0_effects[MAX_VISIBLE_STATES];
static gui_element_t *unit0_items[MAX_INVENTORY];
static gui_element_t *building0_currentResearchItem;
static gui_element_t *building0_type;
static gui_element_t *building0_research[MAX_CLIENT_QUEUED_RESEARCH];

static ivec2_t  rotatingModelPos, rotatingModelSize;

cvar_t unit_healthpercent = { "unit_healthpercent", "0" };

/*
 * Refresh the Manage Users screen, brought up when you hit <tab>
 */
void	CMDR_RefreshManageUsers()
{
	static int lastNumClients = 99999;
	int i, j, numClients;
	int level, money, loyalty, kills;
	gui_element_t *obj;

	j = 0;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!cl.clients[i].info.active || cl.clients[i].info.team != cl.info->team) // || i == cl.clientnum)
			continue;

		level = cl.clients[i].score.level;
		money = cl.clients[i].score.money;
		loyalty = cl.clients[i].score.loyalty;
		kills = cl.clients[i].score.kills;

		obj = corec.GUI_GetObject(fmt("unit_manage_user%i_name", j));
		if (cl.clients[i].info.clan_id)
			GUI_Label_ShowText(obj, fmt("^clan %i^%s", cl.clients[i].info.clan_id, cl.clients[i].info.name));
		else
			GUI_Label_ShowText(obj, cl.clients[i].info.name);

		obj = corec.GUI_GetObject(fmt("unit_manage_user%i_unit", j));
		if (cl.objects[i].base.active)
			GUI_Label_ShowText(obj, CL_ObjectType(cl.objects[i].visual.type)->description);
		else
			GUI_Label_ShowText(obj, "N/A");

		obj = corec.GUI_GetObject(fmt("unit_manage_user%i_level", j));
		GUI_Label_ShowInt(obj, level);

		obj = corec.GUI_GetObject(fmt("unit_manage_user%i_kills", j));
		GUI_Label_ShowInt(obj, kills);

		obj = corec.GUI_GetObject(fmt("unit_manage_user%i_loyalty", j));
		GUI_Label_ShowInt(obj, loyalty);

		//obj = corec.GUI_GetObject(fmt("unit_manage_user%i_group", j));
		//GUI_Label_ShowInt(obj, 0);

		obj = corec.GUI_GetObject(fmt("unit_manage_user%i_gold", j));
		GUI_Label_ShowInt(obj, money);

		obj = corec.GUI_GetObject(fmt("unit_manage_user%i_officer", j));

		if (CMDR_UnitIsOfficer(i))
		{
			GUI_Button_ShowText(obj, "Demote");
			GUI_Button_Command(obj, fmt("demoteOfficerUnit %i", i));
		}
		else
		{
			GUI_Button_ShowText(obj, "Promote");
			GUI_Button_Command(obj, fmt("promoteToOfficer -1 %i", i));
		}

		corec.GUI_Exec(fmt("show unit_manage_user%i_name; show unit_manage_user%i_unit; show unit_manage_user%i_level; show unit_manage_user%i_kills; show unit_manage_user%i_loyalty; show unit_manage_user%i_group; show unit_manage_user%i_gold; show unit_manage_user%i_gold_icon", j, j, j, j, j, j, j, j));

		if (!cl.objects[i].base.active)
			corec.GUI_Exec(fmt("hide unit_manage_user%i_find", j));
		else
			corec.GUI_Exec(fmt("show unit_manage_user%i_find", j));

		if (!cl.objects[i].base.active || cl.clientnum == CL_TeamUnitToGlobalUnitNum(i))
			corec.GUI_Exec(fmt("hide unit_manage_user%i_officer", j));
		else
			corec.GUI_Exec(fmt("show unit_manage_user%i_officer", j));

		j++;
	}

	numClients = j;

	while (j < 32 && j <= lastNumClients)
	{
		corec.GUI_Exec(fmt("hide unit_manage_user%i_name; hide unit_manage_user%i_unit; hide unit_manage_user%i_level; hide unit_manage_user%i_kills; hide unit_manage_user%i_loyalty; hide unit_manage_user%i_group; hide unit_manage_user%i_officer; hide unit_manage_user%i_gold; hide unit_manage_user%i_gold_icon; hide unit_manage_user%i_find;", j, j, j, j, j, j, j, j, j, j));
		j++;
	}

	lastNumClients = numClients;
}

void 	CMDR_RefreshManageUsers_Cmd(int argc, char *argv[])
{
	CMDR_RefreshManageUsers();
}

/*==========================

  CMDR_RefreshSelectionIcons

 ==========================*/

void	CMDR_RefreshSelectionIcons()
{
	gui_element_t *widget, *widget2;
	char *name;
	int i = 0;
	
	if (cl.selection.numSelected > 1)
	{
		corec.GUI_Exec("hide commander_unit_panel");

		for (i = 0; i < cl.selection.numSelected; i++)
		{
			widget = corec.GUI_GetObject(fmt("selection_panel:object%i", i));
			if (widget)
			{
				objectData_t *obj = CL_ObjectType(cl.objects[cl.selection.array[i]].base.type);

				if (cl.selection.array[i] < MAX_CLIENTS)
				{
					name = cl.clients[cl.selection.array[i]].info.name;
				}
				else if (cl.objects[cl.selection.array[i]].base.nameIdx)
					name = "Worker"; // %s", peonNames[cl.objects[cl.selection.array[i]].base.nameIdx]);
				else
					name = obj->description;
				
				corec.GUI_Select(widget);
				corec.Cmd_Exec(fmt("param mouseover_command "
									"\""
										"show grid_popup_panel; "//  #cl_gridPopupFadeTime#; "
										"set grid_popup_name %s; "
										"set highlightedSelection %i; "
										"set grid_popup_description \\\"%s\\\"; "
									"\"", 
									name, 
									i, 
									obj->tooltip)
								);
				corec.Cmd_Exec("param mouseaway_command \"hide grid_popup_panel\"");//  #cl_gridPopupFadeTime#\"");
				corec.Cmd_Exec(fmt("param command \"selectUnits %i\"", cl.selection.array[i]));
				corec.Cmd_Exec(fmt("param up_image %s.tga",	obj->icon));
				corec.Cmd_Exec(fmt("param up_hover_image %s_over.tga",		obj->icon));
				corec.Cmd_Exec(fmt("param down_image %s_down.tga",	obj->icon));
				corec.GUI_Show(widget);
			}
			widget = corec.GUI_GetObject(fmt("selection_panel:object%i_health", i));
			if (widget)
				corec.GUI_Show(widget);
		}
	}
	else
	{
		corec.GUI_Exec("show commander_unit_panel");
	}

	widget = corec.GUI_GetObject(fmt("selection_panel:object%i", i));
	while (widget)
	{
		if (widget)
			corec.GUI_Hide(widget);
		widget2 = corec.GUI_GetObject(fmt("selection_panel:object%i_health", i));
		if (widget2)
			corec.GUI_Hide(widget2);
		i++;
		widget = corec.GUI_GetObject(fmt("selection_panel:object%i", i));
	}
}

/*
 * Draw the single unit closeup info to the right of the selection info box
 *
 */
void	refresh_UnitCloseupView(int object_id)
{
	GUI_Label_ShowText(unit0_health, fmt("%i / %i", cl.objects[object_id].base.health, cl.objects[object_id].visual.fullhealth));
}

/*
 *	Update the "selection view" box
 *	If no objects selected, draw nothing
 *	If one object selected, show "detail view" (name, gold, kills, deaths, inventory, etc.)
 *	If >1 objects selected, show icons of all of them plus their health
 */
void	refresh_Commander_MultiSelection()
{
	int i, fullhealth;
	gui_element_t *widget;

	//update all the health bars
	for (i = 0; i < cl.selection.numSelected; i++)
	{
		fullhealth = cl.objects[cl.selection.array[i]].visual.fullhealth;
		if (fullhealth > 0)
		{
			widget = corec.GUI_GetObject(fmt("selection_panel:object%i_health", i));
			if (widget)
			{
				GUI_Slider_Value(widget, 
							100 * cl.objects[cl.selection.array[i]].base.health /
							fullhealth);
			}
		}
	}
	corec.GUI_Exec("show selection_panel; hide commander_unit_info_panel; hide commander_building_info_panel; hide commander_generic_info_panel");
}

void	CMDR_UpdateUnitInventory(int unitnum, int numItems, int items[MAX_INVENTORY])
{
	int i = 1;
	residx_t graphic;
	char *fname;
	objectData_t	*objdata;

	if (!cl.selection.numSelected || cl.selection.numSelected > 1 
		|| (cl.selection.numSelected == 1 && cl.selection.array[0] != unitnum))
	{
		while (i < MAX_INVENTORY)
		{
			corec.GUI_Hide(unit0_items[i]);
			i++;
		}
		return;
	}

	//skip item 0
	for (i = 1; i < numItems; i++)
	{
		if (!items[i] && i > 1) //it's okay for them not to have a ranged weapon
			break;
		corec.GUI_Show(unit0_items[i]);
		//GUI_Graphic_ChangeImage(unit0_items[i], corec.Res_LoadShader(fmt("%s.tga", CL_ObjectType(items[i])->icon)));
		objdata = CL_ObjectType(items[i]);
		fname = fmt("%s.tga", objdata->icon);
		graphic = corec.Res_LoadShader(fname);
		GUI_Graphic_ChangeImage(unit0_items[i], graphic);
	}
	while (i < MAX_INVENTORY)
	{
		corec.GUI_Hide(unit0_items[i]);
		i++;
	}
}

void	refresh_Commander_SingleSelection()
{
	int level, money, loyalty, kills, i = 0, j = 0;
	char *goal;
	float fullhealth;
	int object_id = cl.selection.array[0];

	if (object_id < MAX_CLIENTS)
	{
		GUI_Label_ShowText(unit0_name, cl.clients[object_id].info.name);
	
		if (cl.clients[object_id].waypoint.active)
			goal = getstring(CL_GetGoalString(cl.clients[object_id].waypoint.goal));
		else
			goal = "No task";
		GUI_Label_ShowText(unit0_goal, goal);

		level = cl.clients[object_id].score.level;
		money = cl.clients[object_id].score.money;
		loyalty = cl.clients[object_id].score.loyalty;
		kills = cl.clients[object_id].score.kills;

		GUI_Label_ShowInt(unit0_gold, money);
		GUI_Label_ShowInt(unit0_level, level);
		GUI_Label_ShowInt(unit0_kills, kills);
		GUI_Label_ShowInt(unit0_loyaltypoints, loyalty);

		corec.GUI_Exec("show unit0_loyaltypoints_label; show unit0_kills_label; show unit0_goldicon; show unit0_level_label");
		corec.GUI_Exec("hide selection_panel; hide commander_building_info_panel; show commander_unit_info_panel; show commander_generic_info_panel");
	}
	else if (IsBuildingType(cl.objects[object_id].base.type))
	{
		objectData_t *obj = CL_ObjectType(cl.objects[object_id].itemConstruction);
		
		if (cl.objects[object_id].base.percentToComplete > 0
			&& !(cl.objects[object_id].base.flags & BASEOBJ_UNDER_CONSTRUCTION))
		{
			//GUI_Graphic_ChangeImage(building0_currentResearchItem, corec.Res_LoadShader(fmt("%s.tga", obj->icon)));
			corec.GUI_Exec(fmt("select building0_currentResearchItem; param up_image %s.tga", obj->icon));
			corec.GUI_Show(building0_currentResearchItem);
			corec.GUI_Exec("show building0_currentResearchItem_label");
		}
		else
		{
			corec.GUI_Hide(building0_currentResearchItem);
			corec.GUI_Exec("hide building0_currentResearchItem_label");
		}
		
		GUI_Label_ShowText(building0_type, CL_ObjectType(cl.objects[object_id].visual.type)->description);

		for (i = 0; i < MAX_CLIENT_QUEUED_RESEARCH; i++)
		{
			if (cl.objects[object_id].researchQueue[i] > 0 
				&& i < cl.objects[object_id].numQueuedResearch)
			{
				GUI_Graphic_ChangeImage(building0_research[i], corec.Res_LoadShader(fmt("%s.tga", CL_ObjectType(cl.objects[object_id].researchQueue[i])->icon)));
				corec.GUI_Show(building0_research[i]);
			}
			else
			{
				corec.GUI_Hide(building0_research[i]);
			}
		}
		
		corec.GUI_Exec("show commander_building_info_panel; hide selection_panel; hide commander_unit_info_panel; show commander_generic_info_panel");
	}
	else
	{
		GUI_Label_ShowText(unit0_name, "");
		GUI_Label_ShowText(unit0_goal, "");
		GUI_Label_ShowText(unit0_gold, "");
		GUI_Label_ShowText(unit0_level, "");
		GUI_Label_ShowText(unit0_loyaltypoints, "");
		GUI_Label_ShowText(unit0_kills, "");
		corec.GUI_Exec("hide unit0_loyaltypoints_label; hide unit0_kills_label; hide unit0_goldicon; hide unit0_level_label");
		corec.GUI_Exec("hide selection_panel; hide commander_building_info_panel; show commander_unit_info_panel; show commander_generic_info_panel");

		//GUI_Graphic_ShowInt(unit0_percentComplete, MAX_PERCENTCOMPLETE - cl.objects[object_id].base.percentToComplete);
	}
	GUI_Label_ShowText(unit0_type, CL_ObjectType(cl.objects[object_id].visual.type)->description);

	fullhealth = cl.objects[object_id].visual.fullhealth;
	if (fullhealth > 0)
	{
		corec.Cvar_SetVarValue(&unit_healthpercent, 
						100 * cl.objects[object_id].base.health /
						fullhealth);
	}

	for (i = 1, j = 0; i < MAX_STATES && j < MAX_VISIBLE_STATES; i++)
	{
		if (!stateData[i].active)
			break;
		if (cl.objects[cl.selection.array[0]].base.states[i])
		{
			corec.GUI_Show(unit0_effects[j]);
			GUI_Graphic_ChangeImage(unit0_effects[j], corec.Res_LoadShader(stateData[i].icon));
			j++;
		}
	}
	while (j < MAX_VISIBLE_STATES)
	{
		corec.GUI_Hide(unit0_effects[j]);
		j++;
	}
}

void	refresh_Commander_NoSelection()
{
	gui_element_t *widget;
	int i, j = 0;
	
	GUI_Label_ShowText(unit0_name, "");
	GUI_Label_ShowText(unit0_goal, "");
	GUI_Label_ShowText(unit0_health, "");
	
	while (j < MAX_VISIBLE_STATES)
	{
		corec.GUI_Hide(unit0_effects[j]);
		j++;
	}

	for (i = 0; i < MAX_SELECTABLE_UNITS; i++)
	{
		widget = corec.GUI_GetObject(fmt("selection_panel:object%i_health", i));
		corec.GUI_Hide(widget);
	}
	corec.GUI_Exec("hide commander_unit_info_panel; hide commander_building_info_panel; hide commander_generic_info_panel");
}

void	CMDR_RefreshSelectedView()
{
	static bool lastSelectionEmpty = false;
	
	if (cl.selection.numSelected <= 0)
	{
		//only call this once when the selection is made empty, no updates are necessary thereafter
		if (!lastSelectionEmpty)
		{
			refresh_Commander_NoSelection();
			lastSelectionEmpty = true;
		}
	}
	else if (cl.selection.numSelected > 1)
	{
	    //more than one object selected
		refresh_Commander_MultiSelection();

		//now draw the closeup info about the currently highlighted selection
		if (highlightedSelection.integer >= cl.selection.numSelected)
			corec.Cvar_SetVarValue(&highlightedSelection, 0);
		refresh_UnitCloseupView(cl.selection.array[highlightedSelection.integer]);
		lastSelectionEmpty = false;
	}
	else
	{
		//only one object selected
		refresh_Commander_SingleSelection();
		refresh_UnitCloseupView(cl.selection.array[0]);
		lastSelectionEmpty = false;
	}
}

void	CMDR_RotatingModelFrame()
{
	if (!cl.isCommander)
		return;

	if (cl.selection.numSelected)
	{
  		if (unit0_model)
		{
			if (unit0_model->visible)
			{
				corec.GUI_GetScreenPosition(unit0_model, rotatingModelPos);
				corec.GUI_GetScreenSize(unit0_model, rotatingModelSize);				
				Rend_Draw3dUnitWindow(cl.objects[cl.selection.array[0]].visual.type, cl.selection.array[0], rotatingModelPos, rotatingModelSize);
			}
		}
	}
}

void	CMDR_InitGUIWidgets()
{
	int i;

	unit0_model 			= corec.GUI_GetObject("unit_closeup_panel:unit0_model");
	if (!unit0_model)
		goto commanderGUIError;

	unit0_name 				= corec.GUI_GetObject("unit0_name");
	if (!unit0_name)
		goto commanderGUIError;

	unit0_health 			= corec.GUI_GetObject("unit0_health");
	if (!unit0_health)
		goto commanderGUIError;

	unit0_gold 				= corec.GUI_GetObject("unit0_gold");
	if (!unit0_gold)
		goto commanderGUIError;

	unit0_loyaltypoints 	= corec.GUI_GetObject("unit0_loyaltypoints");
	if (!unit0_loyaltypoints)
		goto commanderGUIError;

	for (i = 0; i < MAX_VISIBLE_STATES; i++)
	{
		unit0_effects[i] = corec.GUI_GetObject(fmt("unit0_effect%i", i));
		if (!unit0_effects[i])
			goto commanderGUIError;
	}
	
	for (i = 0; i < MAX_INVENTORY; i++)
	{
		unit0_items[i] = corec.GUI_GetObject(fmt("unit0_item%i", i));
		if (!unit0_items[i])
			goto commanderGUIError;
	}
	
	unit0_goal 				= corec.GUI_GetObject("unit0_goal");
	if (!unit0_goal)
		goto commanderGUIError;

	unit0_type 				= corec.GUI_GetObject("unit0_type");
	if (!unit0_type)
		goto commanderGUIError;

	unit0_kills 				= corec.GUI_GetObject("unit0_kills");
	if (!unit0_kills)
		goto commanderGUIError;

	unit0_level 			= corec.GUI_GetObject("unit0_level");
	if (!unit0_level)
		goto commanderGUIError;

	building0_currentResearchItem   = corec.GUI_GetObject("building0_currentResearchItem");
	if (!building0_currentResearchItem)
		goto commanderGUIError;

	building0_type   = corec.GUI_GetObject("building0_type");
	if (!building0_type)
		goto commanderGUIError;

	for (i = 0; i < MAX_CLIENT_QUEUED_RESEARCH; i++)
	{
		building0_research[i] = corec.GUI_GetObject(fmt("building0_item%i", i));
		if (!building0_research[i])
			goto commanderGUIError;
	}
	
	return;

commanderGUIError:
	core.Game_Error("Commander GUI not configured correctly\n");
}

void	CMDR_InitInterface()
{
	corec.Cvar_Register(&unit_progress);
	corec.Cvar_Register(&unit_healthpercent);

	corec.Cvar_Register(&highlightedSelection);
	
	corec.Cmd_Register("refreshManageScreen", 		CMDR_RefreshManageUsers_Cmd);
}
