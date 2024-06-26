#include "../toplayer/tl_shared.h"
#include "gui_floater.h"

static char *class_name = "floater";

void	GUI_Floater_MouseDown (gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_floater_t *floater;
	floater = corec.GUI_GetUserdata(class_name, obj);
	if (!floater)
		return;

	floater->mousedown = true;
	floater->mousedown_loc[0] = x;
	floater->mousedown_loc[1] = y;
}

void	GUI_Floater_MouseUp(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_floater_t *floater;
	floater = corec.GUI_GetUserdata(class_name, obj);
	if (!floater)
		return;

	floater->mousedown = false;
}

void	GUI_Floater_MouseOver(gui_element_t *obj, int x, int y)
{
	gui_floater_t *floater;
	floater = corec.GUI_GetUserdata(class_name, obj);
	if (!floater)
		return;

	if (floater->mousedown)
	{
		corec.GUI_Panel_MoveRelative(obj->panel, x - floater->mousedown_loc[0], y - floater->mousedown_loc[1]);
		//floater->mousedown_loc[0] = -floater->mousedown_loc[0];
		//floater->mousedown_loc[1] = -floater->mousedown_loc[1];
	} 
}

//floater "name" x y w h floater ["text"]
void	*GUI_Floater_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_graphic_t *parent;
	gui_floater_t *floater;

	if (argc < 4)
	{
		corec.Console_Printf("syntax: floater name x y w h\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);

	parent = GUI_Graphic_Create(obj, argc, argv);

	if (!parent)
	{
		corec.Console_Printf("Floater error: couldn't instantiate graphic class.  See error above.\n");
		return NULL; 
	}
	floater = corec.GUI_Malloc(sizeof (gui_floater_t));

	if (!floater)
	{
		corec.Console_Printf("Floater error: couldn't enough space to hold floater\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, floater);

	floater->parent = parent;

	floater->element = obj;

	//obj->interactive = true;
	obj->mousedown = GUI_Floater_MouseDown;
	obj->mouseup = GUI_Floater_MouseUp;
	obj->mouseover = GUI_Floater_MouseOver;

	floater->mousedown = false;
	floater->parent = NULL;

	return floater;
}

void	GUI_Floater_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("floater <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	} 
	else if (strcmp(argv[0], "param") == 0)
	{
		GUI_Graphic_Cmd(argc, argv);
	}
}

void	GUI_Floaters_Init()
{
	corec.Cmd_Register("floater", GUI_Floater_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_Floater_Create);
}
