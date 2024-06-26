/*
 * (C) 2002 S2 Games
 * cl_contextmenu.c - context-sensitive menu ability
 */

#include "client_game.h"

gui_element_t *options[MAX_CONTEXTMENU_OPTIONS];
gui_panel_t *context_panel;

#define CONTEXTMENU_BORDER_SIZE 20

int free_option_num;
int width;

static void	_ResizeWidgets(int width)
{
	int i;

	for (i = 0; i < MAX_CONTEXTMENU_OPTIONS; i++)
	{
		corec.GUI_Resize(options[i], width, options[i]->height);
	}
}

bool	CL_ContextMenu_ExecOption(int num)
{
	if (num >= 0 && num < MAX_CONTEXTMENU_OPTIONS
		&& options[num]->mousedown)
	{
		options[num]->mousedown(options[num], MOUSE_LBUTTON, 0, 0);
		return true;
	}
	return false;
}

int		CL_ContextMenu_AddOption(char *name, char *command)
{
	if (free_option_num >= MAX_CONTEXTMENU_OPTIONS
		|| !options[free_option_num]
		|| !context_panel)
		return 0;

	corec.GUI_Select(options[free_option_num]);
	corec.GUI_Exec(fmt("param text \"%s\"", name));
	corec.GUI_Exec(fmt("param command \"contextmenu_hide; %s\"", command));

	if (width < options[free_option_num]->width)
	{
		width = options[free_option_num]->width;
		_ResizeWidgets(width + CONTEXTMENU_BORDER_SIZE);
	}
	
	corec.GUI_Show(options[free_option_num]);
	corec.GUI_Panel_Show(context_panel);
	
	free_option_num++;
	return free_option_num-1;
}

void	CL_ContextMenu_Clear()
{
	width = 0;
	free_option_num = 0;
	CL_ContextMenu_Hide();
	_ResizeWidgets(0);
}

void	CL_ContextMenu_Move(int screenx, int screeny)
{
	if (context_panel)
	{
		corec.GUI_Coord(&screenx, &screeny);		//convert the screen coordinate to a GUI coordinate
		corec.GUI_Panel_Move(context_panel, screenx, screeny);
		//corec.GUI_Panel_MoveRelative(context_panel, x - context_panel->pos[X], y - context_panel->pos[Y]);
	}
}
	
void	CL_ContextMenu_Hide()
{
	int i;
	
	for (i = 0; i < MAX_CONTEXTMENU_OPTIONS; i++)
		if (options[i])
			corec.GUI_Hide(options[i]);

	if (context_panel)
		corec.GUI_Panel_Hide(context_panel);
}

void	CL_ContextMenu_Hide_Cmd(int argc, char *argv[])
{
	CL_ContextMenu_Hide();
}

void	CL_ContextMenu_Init()
{
	int i = 0;

	free_option_num = 0;
	
	context_panel = corec.GUI_GetPanel("contextmenu_panel");
	if (!context_panel)
		corec.Console_Printf("Error!  Couldn't find the context menu panel!\n");

	for (i = 0; i < MAX_CONTEXTMENU_OPTIONS; i++)
	{
		options[i] = corec.GUI_GetObject(fmt("contextmenu_panel:option%i_button", i+1));
		if (!options[i])
			corec.Console_Printf("Error!  Couldn't find context menu button #%i (tried contextmenu_panel:option%i_button)\n", i, i+1);
	}

	corec.Cmd_Register("contextmenu_hide", CL_ContextMenu_Hide_Cmd);
}
