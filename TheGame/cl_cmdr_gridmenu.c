/*
 * (C) 2003 S2 Games
 *
 * cl_cmdr_gridmenu.c'
 *
 * Handles display and updating of the commanders 'grid menu'
 */

#include "client_game.h"

//these are displayed by labels in the grid_popup_panel
cvar_t grid_popup_name =			{"grid_popup_name",			"" };
cvar_t grid_popup_costs =			{"grid_popup_costs",		"" };
cvar_t grid_popup_requirements =	{"grid_popup_requirements",	"" };
cvar_t grid_popup_base =			{"grid_popup_base",			"" };
cvar_t grid_popup_tech =			{"grid_popup_tech",			"" };
cvar_t grid_popup_description =		{"grid_popup_description",	"" };
cvar_t grid_popup_population =		{"grid_popup_population",	"" };

cvar_t	cl_gridPopupFadeTime = { "cl_gridPopupFadeTime", "0" };

//=============================================================================


/*==========================

  CMDR_RefreshGrid

  Updates the grid buttons with the apropriate commands

  ==========================*/

void	CMDR_RefreshGrid()
{
	int	index;
	clientObject_t *obj;
	char *gui_basepath = corec.Cvar_GetString("gui_basepath");

	if (cl.selection.numSelected < 1)
	{
		corec.Cmd_Exec(fmt("exec %s/gridmenus/layout_empty.cfg", gui_basepath));
	}
	else if (cl.selection.numSelected == 1)
	{
		obj = &cl.objects[cl.selection.array[0]];
		corec.Cmd_Exec(fmt("exec %s/%s", gui_basepath, CL_ObjectType(obj->visual.type)->gridmenu));
	}
	else
	{
		obj = &cl.objects[cl.selection.array[0]];
		corec.Cmd_Exec(fmt("exec %s", CL_ObjectType(obj->visual.type)->gridmenu));
		for (index = 0; index < 12; index++)
			corec.Cmd_Exec(fmt("set _grid%i grid%i", index, index));

		for (index = 1; index < cl.selection.numSelected; index++)
		{
			obj = &cl.objects[cl.selection.array[index]];
			corec.Cmd_Exec(fmt("exec %s/%s", gui_basepath, CL_ObjectType(obj->visual.type)->gridmenu));
			corec.Cmd_Exec(fmt("exec %s/gridmenus/ui_mergeselected.cfg", gui_basepath));
		}
	}
	corec.Cmd_Exec(fmt("exec %s/gridmenus/ui_setupgrid.cfg", gui_basepath));

}


/*==========================

  CMDR_RefreshCommanderGrid_Cmd

  Manual call to refresh the grid

 ==========================*/

void	CMDR_RefreshCommanderGrid_Cmd(int argc, char *argv[])
{
	CMDR_RefreshGrid();
}


/*==========================

  CMDR_InitGridMenu

  Called once when the client starts up

 ==========================*/

void	CMDR_InitGridMenu()
{
	//register cvars
	corec.Cvar_Register(&grid_popup_name);
	corec.Cvar_Register(&grid_popup_costs);
	corec.Cvar_Register(&grid_popup_requirements);
	corec.Cvar_Register(&grid_popup_base);
	corec.Cvar_Register(&grid_popup_tech);
	corec.Cvar_Register(&grid_popup_description);
	corec.Cvar_Register(&grid_popup_population);

	corec.Cvar_Register(&cl_gridPopupFadeTime);

	//register commands
	corec.Cmd_Register("refreshCommanderGrid",		CMDR_RefreshCommanderGrid_Cmd);
}
