
#include "../toplayer/tl_shared.h"
#include "gui_togglebutton.h"

cvar_t	gui_button_up_cmd = { "gui_button_up_cmd", "" };
cvar_t	gui_button_down_mouseover = { "gui_button_down_mouseover", "" };

static char *class_name = "togglebutton";

void	GUI_ToggleButton_Draw(gui_element_t *obj, int w, int h)
{
	gui_togglebutton_t *button;
	int x,y;

	x = 0; y = 0;

	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	button->parent->down = corec.Cvar_GetInteger(button->variable);
	
	GUI_Button_Draw(obj, w, h);
}

void	GUI_ToggleButton_MouseDown (gui_element_t *obj, mouse_button_enum mbutton, int x, int y)
{
	gui_togglebutton_t *button;

	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	button->parent->down = !button->parent->down;

	if (button->parent->down)
	{
		corec.GUI_Exec(button->parent->down_command);
	}
	else
	{
		corec.GUI_Exec(button->up_command);
	}

	if (button->variable[0])
	{
		corec.Cvar_SetValue(button->variable, button->parent->down);
	}
}

void	GUI_ToggleButton_MouseOut(gui_element_t *obj)
{
	gui_togglebutton_t *button;
	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	button->parent->mouseover = false;
}

void	GUI_ToggleButton_MouseOver(gui_element_t *obj, int x, int y)
{
	gui_togglebutton_t *button;
	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	if (!obj->focus)
	{
		button->parent->mouseover = true;
	} 
	else 
	{
		button->parent->mouseover = false;
	}
}

void	GUI_ToggleButton_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_ToggleButton_Toggle_Cmd(int argc, char *argv[])
{
	gui_togglebutton_t *button;
	gui_element_t *obj;

	if (argc < 1)
	{
		corec.Console_Printf("syntax: togglebutton toggle <panel>:<button>\n");
		return;
	}
	obj = corec.GUI_GetObject(argv[0]);
	if (!obj)
		return;

	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;
	button->parent->down = !button->parent->down;
}

void	GUI_ToggleButton_Up_Cmd(int argc, char *argv[])
{
	gui_togglebutton_t *button;
	gui_element_t *obj;

	if (argc < 1)
	{
		corec.Console_Printf("syntax: togglebutton up <panel>:<button>\n");
		return;
	}
	obj = corec.GUI_GetObject(argv[0]);
	if (!obj)
		return;

	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;
	button->parent->down = false;
}

void	GUI_ToggleButton_Down_Cmd(int argc, char *argv[])
{
	gui_togglebutton_t *button;
	gui_element_t *obj;

	if (argc < 1)
	{
		corec.Console_Printf("syntax: togglebutton down <panel>:<button>\n");
		return;
	}
	obj = corec.GUI_GetObject(argv[0]);
	if (!obj)
		return;

	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;
	button->parent->down = true;
}

void GUI_ToggleButton_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	char filename[256];
	gui_togglebutton_t *togglebutton;

	if (argc < 1)
	{
		corec.Console_Printf("togglebutton param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   command <command>        - the command to execute when a button is pressed\n");
		corec.Console_Printf("   up_command <command>     - the command to execute when a button is released\n");
		corec.Console_Printf("   down_image <image>       - the image file to use when the button is down (pressed)\n");
		corec.Console_Printf("   down_hover_image <image> - the image file to use when the button is down and the mouse is over it (mouseover)\n");
		corec.Console_Printf("   up_image <image>         - the image file to use when the button is up   (normal)\n");
		corec.Console_Printf("   up_hover_image <image>   - the image file to use when the button is up and the mouse is over it (mouseover)\n");
		corec.Console_Printf("   text <text>              - the text to overlay on top of the button\n");
		corec.Console_Printf("   variable <varname>       - use a cvar to control the toggle/untoggle\n");
		return;
	}

	togglebutton = corec.GUI_GetUserdata(class_name, obj);

	if (!togglebutton)
		return;

	if (strcmp(argv[0], "down_hover_image") == 0)
	{
		if (argc > 1)
		{
			BPrintf(filename, 255, "%s%s", UI_PREFIX, argv[1]);
			filename[255] = 0;

			togglebutton->down_mouseover_shader = corec.Res_LoadShaderEx(filename, SHD_NO_MIPMAPS | SHD_FULL_QUALITY);
		}
		else
		{
			corec.Console_Printf("togglebutton error: you must specify a filename to use with this command.\n");
		}
	}
	else if (strcmp(argv[0], "up_command") == 0)
	{
		if (argc > 1)
		{
			strncpy(togglebutton->up_command, argv[1], GUI_BUTTON_CMD_LENGTH-1);
			togglebutton->up_command[GUI_BUTTON_CMD_LENGTH-1] = 0;
		}
		else
		{
			strcpy(togglebutton->up_command, "");
		}
	}
	else if (strcmp(argv[0], "variable") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(togglebutton->variable, argv[1], sizeof(togglebutton->variable));
						
			togglebutton->parent->down = corec.Cvar_GetInteger(togglebutton->variable);
		}
		else
		{
			corec.Console_Printf("Error: you must specify the variable name\n");
		}
	}
	else
	{
		GUI_Button_Param_Cmd(obj, argc, argv);
		if (togglebutton->parent->down_shader && !togglebutton->down_mouseover_shader)
		{
			togglebutton->down_mouseover_shader = togglebutton->parent->down_shader;
		}
	}
}

void	*GUI_ToggleButton_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_button_t *parent;
	gui_togglebutton_t *togglebutton;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: togglebutton name x y w h\n");
		return NULL; 
	}

	corec.GUI_SetClass(obj, class_name);

	parent = GUI_Button_Create(obj, argc, argv);

	if (!parent)
	{
		corec.Console_Printf("Togglebutton error: couldn't instantiate button class.  See error above.\n");
		return NULL; 
	}
	togglebutton = corec.GUI_Malloc(sizeof (gui_togglebutton_t));

	if (!togglebutton)
	{
		corec.Console_Printf("Togglebutton error: couldn't get enough space to hold togglebutton\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, togglebutton);

	togglebutton->parent = parent;

	obj->mousedown = GUI_ToggleButton_MouseDown;
	obj->mouseout = GUI_ToggleButton_MouseOut;
	obj->mouseover = GUI_ToggleButton_MouseOver;
	obj->mouseup = NULL;
	obj->draw = GUI_ToggleButton_Draw;

	obj->param = GUI_ToggleButton_Param_Cmd;

	togglebutton->down_mouseover_shader = 0;
	strcmp(togglebutton->up_command, "");

	return togglebutton;
}

void	GUI_ToggleButton_List()
{
	corec.GUI_List_Cmd(1, &class_name);
}

void	GUI_ToggleButton_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;
	
	if (!argc)
	{
		corec.Console_Printf("togglebutton <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		corec.Console_Printf("    toggle\n");
		corec.Console_Printf("    up\n");
		corec.Console_Printf("    down\n");
		return;
	}

	if (strcmp(argv[0], "toggle") == 0)
	{
		GUI_ToggleButton_Toggle_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "up") == 0)
	{
		GUI_ToggleButton_Up_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "down") == 0)
	{
		GUI_ToggleButton_Down_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "list") == 0)
	{
		GUI_ToggleButton_List();
	}
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			if (obj)
				GUI_ToggleButton_Param_Cmd(obj, argc-2, &argv[2]);
		}
	} 

}

void	GUI_ToggleButtons_Init()
{
	corec.Cvar_Register(&gui_button_down_mouseover);
	corec.Cvar_Register(&gui_button_up_cmd);

	corec.GUI_RegisterClass(class_name, GUI_ToggleButton_Create);

	corec.Cmd_Register("togglebutton", GUI_ToggleButton_Cmd);
}
