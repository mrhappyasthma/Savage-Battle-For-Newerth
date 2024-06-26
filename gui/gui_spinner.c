
#include "../toplayer/tl_shared.h"
#include "../Core/system.h"
#include "gui_spinner.h"

static char *class_name = "spinner";

void	GUI_Spinner_Draw(gui_element_t *obj, int x, int y)
{
	gui_spinner_t *button;
	int w, h, shader;

	button = corec.GUI_GetUserdata(class_name, obj);
	if (!button)
		return;

	x = 0;
	y = 0;
	w = obj->width;
	h = obj->height;

	if (button->parent->down)
	{
			shader = button->parent->down_shader;
	}
	else
	{
		if (button->parent->mouseover)
		{
			shader = button->parent->up_mouseover_shader;
		}
		else
		{
			shader = button->parent->up_shader;
		}

	}

	corec.GUI_SetRGBA(1,1,1, obj->alpha);

	corec.GUI_Quad2d_S(x, y, w, h, shader);
}

void	GUI_Spinner_ChangeValue(gui_element_t *obj, bool inc)
{
	gui_spinner_t *spinner;
	
	float value;

	spinner = corec.GUI_GetUserdata(class_name, obj);
	if (!spinner)
		return;
	
	value = corec.Cvar_GetValue(spinner->variable);

	if (inc)
		value += spinner->step;
	else
		value -= spinner->step;

	if (value < spinner->low)
		value = spinner->low;
	if (value > spinner->hi)
		value = spinner->hi;
		
	corec.Cvar_SetValue(spinner->variable, value);
}

void	GUI_Spinner_MouseDown (gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_spinner_t *spinner;
	
	spinner = corec.GUI_GetUserdata(class_name, obj);
	if (!spinner)
		return;

	GUI_Spinner_ChangeValue(obj, y < obj->height/2);

	spinner->parent->down = true;
	
	corec.GUI_Exec(spinner->parent->down_command);
}

void	GUI_Spinner_MouseUp (gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_spinner_t *spinner;
	
	spinner = corec.GUI_GetUserdata(class_name, obj);
	if (!spinner)
		return;

	spinner->parent->down = false;

	corec.GUI_Exec(spinner->parent->down_command);
}

void	GUI_Spinner_MouseOut(gui_element_t *obj)
{
	gui_spinner_t *spinner;
	spinner = corec.GUI_GetUserdata(class_name, obj);
	if (!spinner)
		return;

	spinner->parent->mouseover = false;
}

void	GUI_Spinner_MouseOver(gui_element_t *obj, int x, int y)
{
	gui_spinner_t *spinner;
	int curtime;
	
	spinner = corec.GUI_GetUserdata(class_name, obj);
	if (!spinner)
		return;

	if (spinner->parent->down) {

		curtime = corec.Milliseconds();

		if (curtime > (spinner->curtime + 100)) {
			GUI_Spinner_ChangeValue(obj, y < obj->height/2);

			spinner->curtime = curtime;
		}
	}
	
	if (!obj->focus)
	{
		spinner->parent->mouseover = true;
	} 
	else 
	{
		spinner->parent->mouseover = false;
	}
}

void	GUI_Spinner_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_Spinner_Toggle_Cmd(int argc, char *argv[])
{
	gui_spinner_t *button;
	gui_element_t *obj;

	if (argc < 1)
	{
		corec.Console_Printf("syntax: spinner toggle <panel>:<button>\n");
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

void	GUI_Spinner_Up_Cmd(int argc, char *argv[])
{
	gui_spinner_t *button;
	gui_element_t *obj;

	if (argc < 1)
	{
		corec.Console_Printf("syntax: spinner up <panel>:<button>\n");
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

void	GUI_Spinner_Down_Cmd(int argc, char *argv[])
{
	gui_spinner_t *button;
	gui_element_t *obj;

	if (argc < 1)
	{
		corec.Console_Printf("syntax: spinner down <panel>:<button>\n");
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

void	GUI_Spinner_Variable_Cmd(int argc, char *argv[])
{
	gui_spinner_t *spinner;

	if (argc < 2)
	{	
		corec.Console_Printf("syntax: spinner variable <panel:object> <var_name>\n");
		return;
	}

	spinner = corec.GUI_GetClass(argv[0], class_name);

	if (!spinner)
		return;

	strncpySafe(spinner->variable, argv[1], sizeof(spinner->variable));
}


void	GUI_Spinner_Range_Cmd(int argc, char *argv[])
{
	gui_spinner_t *spinner;

	if (argc < 2)
	{	
		corec.Console_Printf("syntax: spinner range <panel:object> <low> <high>\n");
		return;
	}

	spinner = corec.GUI_GetClass(argv[0], class_name);

	if (!spinner)
		return;

	spinner->low = atof(argv[1]);
	spinner->hi = atof(argv[2]);
}

void GUI_Spinner_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_spinner_t *spinner;

	if (argc < 1)
	{
		corec.Console_Printf("spinner param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   range <low> <high> - the possible range for the value\n");
		corec.Console_Printf("   variable <var>     - the variable to affect\n");
		corec.Console_Printf("   step <float>       - the stepsize of the button\n");
		return;
	}

	spinner = corec.GUI_GetUserdata(class_name, obj);

	if (!spinner)
		return;

	if (strcmp(argv[0], "range") == 0)
	{
		if (argc > 2)
		{
			spinner->low = atof(argv[1]);
			spinner->hi = atof(argv[2]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the range values.\n");
		}
	}
	else if (strcmp(argv[0], "variable") == 0)
	{
		if (argc > 1)
		{
			strncpySafe(spinner->variable, argv[1], sizeof(spinner->variable));			
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the variable name.\n");
		}
	}
	else if (strcmp(argv[0], "step") == 0)
	{
		if (argc > 1)
		{
			spinner->step = (atof(argv[1]) != 0 ? atof(argv[1]) : 1);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify a step value, i.e. -50 or 100.\n");
		}
	}
	else
	{
		GUI_Button_Param_Cmd(obj, argc, argv);
	}
}

void	GUI_Spinner_Destroy(gui_element_t *obj)
{
	gui_spinner_t *spinner;

	spinner = corec.GUI_GetUserdata(class_name, obj);

	if (!spinner)
		return;

	corec.GUI_Free(spinner->variable);
}

void	*GUI_Spinner_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_button_t *parent;
	gui_spinner_t *spinner;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: spinner name x y w h\n");
		return NULL; 
	}

	corec.GUI_SetClass(obj, class_name);

	parent = GUI_Button_Create(obj, argc, argv);

	if (!parent)
	{
		corec.Console_Printf("Spinner error: couldn't instantiate button class.  See error above.\n");
		return NULL; 
	}
	spinner = corec.GUI_Malloc(sizeof (gui_spinner_t));

	if (!spinner)
	{
		corec.Console_Printf("Spinner error: couldn't get enough space to hold spinner\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, spinner);

	spinner->parent = parent;
	
	spinner->step = 100;
	spinner->curtime = 0;
	spinner->speed = 1;

	obj->mousedown = GUI_Spinner_MouseDown;
	obj->mouseout = GUI_Spinner_MouseOut;
	obj->mouseover = GUI_Spinner_MouseOver;
	obj->mouseup = GUI_Spinner_MouseUp;

	obj->param = GUI_Spinner_Param_Cmd;

	return spinner;
}

void	GUI_Spinner_List()
{
	corec.GUI_List_Cmd(1, &class_name);
}

void	GUI_Spinner_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;
	
	if (!argc)
	{
		corec.Console_Printf("spinner <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    param\n");
		corec.Console_Printf("    toggle\n");
		//corec.Console_Printf("    hold\n");
		corec.Console_Printf("    down\n");
		return;
	}

	/*if (strcmp(argv[0], "toggle") == 0)
	{
		GUI_Spinner_Toggle_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "hold") == 0)
	{
		GUI_Spinner_Hold_Cmd(argc-1, &argv[1]);
	} */
	if (strcmp(argv[0], "down") == 0)
	{
		GUI_Spinner_Down_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "list") == 0)
	{
		GUI_Spinner_List();
	}
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			if (obj)
				GUI_Spinner_Param_Cmd(obj, argc-2, &argv[2]);
		}
	} 
	

}

void	GUI_Spinners_Init()
{
	corec.GUI_RegisterClass(class_name, GUI_Spinner_Create);

	corec.Cmd_Register("spinner", GUI_Spinner_Cmd);
}
