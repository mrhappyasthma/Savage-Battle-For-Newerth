#include "../toplayer/tl_shared.h"
#include "gui_swatch.h"

static char *class_name = "swatch";

cvar_t	gui_swatchbar = { "gui_swatchbar", "swatch_bar" };
cvar_t	gui_swatchpanel = { "gui_swatchpanel", "swatch_panel" };
cvar_t	gui_swatchpanel_r_var = { "gui_swatchpanel_r_var", "" };
cvar_t	gui_swatchpanel_g_var = { "gui_swatchpanel_g_var", "" };
cvar_t	gui_swatchpanel_b_var = { "gui_swatchpanel_b_var", "" };
cvar_t	gui_swatchpanel_a_var = { "gui_swatchpanel_a_var", "" };

void	GUI_DrawSwatch(gui_element_t *obj, int x, int y)
{
	gui_swatch_t *swatch;

	swatch = corec.GUI_GetUserdata(class_name, obj);

	if (!swatch)
		return;

	if (swatch->r_var)
		swatch->r = corec.Cvar_GetValue(swatch->r_var);

	if (swatch->g_var)
		swatch->g = corec.Cvar_GetValue(swatch->g_var);

	if (swatch->b_var)
		swatch->b = corec.Cvar_GetValue(swatch->b_var);

	if (swatch->a_var)
		swatch->a = corec.Cvar_GetValue(swatch->a_var);

	corec.GUI_SetRGBA(swatch->r, swatch->g, swatch->b, obj->alpha);
	corec.GUI_ShadowQuad2d_S(	0, 0, 
						obj->width, 
						obj->height, 
						corec.GetWhiteShader());
	corec.GUI_Quad2d_S(0,0, 
				obj->width, 
				obj->height, 
				corec.GetWhiteShader());

	//fixme: add in alpha display
}

void	GUI_MoveSwatch(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void	GUI_Swatch_Variables(gui_element_t *obj, char *r, char *g, char *b, char *a)
{
	gui_swatch_t *swatch;

	swatch = corec.GUI_GetUserdata(class_name, obj);

	if (!swatch)
		return;

	if (swatch->r_var)
		corec.GUI_Free(swatch->r_var);

	if (swatch->g_var)
		corec.GUI_Free(swatch->g_var);

	if (swatch->b_var)
		corec.GUI_Free(swatch->b_var);

	if (swatch->a_var)
		corec.GUI_Free(swatch->a_var);

	if (strcmp(r, "")==0)
	{
		swatch->r = 0;
		swatch->r_var = NULL;
	} 
	else 
	{
		swatch->r_var = corec.GUI_StrDup(r);
		swatch->r = corec.Cvar_GetValue(swatch->r_var);
	}

	if (strcmp(g, "")==0)
	{
		swatch->g = 0;
		swatch->g_var = NULL;
	} 
	else 
	{
		swatch->g_var = corec.GUI_StrDup(g);
		swatch->g = corec.Cvar_GetValue(swatch->g_var);
	}

	if (strcmp(b, "")==0)
	{
		swatch->b = 0;
		swatch->b_var = NULL;
	} 
	else 
	{
		swatch->b_var = corec.GUI_StrDup(b);
		swatch->b = corec.Cvar_GetValue(swatch->b_var);
	}

	if (strcmp(a, "")==0)
	{
		swatch->a = 0;
		swatch->a_var = NULL;
	} 
	else 
	{
		swatch->a_var = corec.GUI_StrDup(a);
		swatch->a = corec.Cvar_GetValue(swatch->a_var);
	}
}

void	GUI_Swatch_Variables_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (argc < 4)
	{	
		corec.Console_Printf("syntax: swatch variable <panel:object> r g b a\n");
		return;
	}

	obj = corec.GUI_GetObject(argv[0]);

	if (!obj)
	{
		corec.Console_Printf("error, couldn't find object named %s\n", argv[0]);
		return;
	}

	if (strcmp(obj->class_name, class_name) != 0)
	{
		corec.Console_Printf("Class mismatch, %s is not a %s, it is a %s\n", argv[0], class_name, obj->class_name);
		return;	
	}

	// we don't actually need the swatch object here
	// swatch = GUI_GetUserdata(class_name, obj);

	GUI_Swatch_Variables(obj, argv[1], argv[2], argv[3], argv[4]);
}

void GUI_Swatch_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_swatch_t *swatch;

	if (argc < 1)
	{
		corec.Console_Printf("swatch param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   variables <var1> <var2> <var3> <var4>\n");
		return;
	}

	swatch = corec.GUI_GetUserdata(class_name, obj);

	if (!swatch)
		return;

	if (strcmp(argv[0], "variables") == 0)
	{
		if (argc > 3)
		{
			GUI_Swatch_Variables(obj, argv[1], argv[2], argv[3], argv[4]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the variables\n");
		}
	}
}

void	GUI_Swatch_MouseDown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	char cmd[1024];
	gui_swatch_t *swatch;

	swatch = corec.GUI_GetUserdata(class_name, obj);

	if (!swatch)
		return;

	BPrintf(cmd, 1023, "panel focus %s; panel focus %s; select \"%s:r_slider\"; param variable %s; "
													 "select \"%s:g_slider\"; param variable %s; "
													 "select \"%s:b_slider\"; param variable %s; "
													 "select \"%s:a_slider\"; param variable %s; "
													 "swatch variables generic_swatch %s %s %s;\n",
			gui_swatchbar.string, gui_swatchpanel.string, gui_swatchpanel.string, swatch->r_var, gui_swatchpanel.string, swatch->g_var, gui_swatchpanel.string, swatch->b_var, gui_swatchpanel.string, swatch->a_var, swatch->r_var, swatch->g_var, swatch->b_var);

	corec.GUI_Exec(cmd);
}

void	GUI_Swatch_Destroy(gui_element_t *obj)
{
	gui_swatch_t *swatch;

	swatch = corec.GUI_GetUserdata(class_name, obj);
	if (!swatch)
		return;
	corec.GUI_Free(swatch->r_var);
	corec.GUI_Free(swatch->g_var);
	corec.GUI_Free(swatch->b_var);
	corec.GUI_Free(swatch->a_var);
}

//swatch "name" x y w h r g b
void	*GUI_Swatch_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_swatch_t *swatch;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: create swatch name x y w h\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]) );

	swatch = corec.GUI_Malloc(sizeof (gui_swatch_t));

	if (!swatch)
	{
		corec.Console_Printf("Swatch error: couldn't enough space to hold swatch\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, swatch);
	swatch->element = obj;

	obj->interactive = true;
	
	swatch->r_var = NULL;
	swatch->g_var = NULL;
	swatch->b_var = NULL;
	swatch->a_var = NULL;

	obj->destroy = GUI_Swatch_Destroy;
	obj->draw = GUI_DrawSwatch;
	obj->move = GUI_MoveSwatch;
	obj->param = GUI_Swatch_Param_Cmd;

	obj->mousedown = GUI_Swatch_MouseDown;

	swatch->parent = NULL;

	return swatch;
}

void	GUI_Swatch_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("swatch <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    variables\n");
		corec.Console_Printf("    param\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	} else if (strcmp(argv[0], "variables") == 0)
	{
		GUI_Swatch_Variables_Cmd(argc-1, &argv[1]);
	}
	else if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 1)
		{
			obj = corec.GUI_GetObject(argv[1]);
			GUI_Swatch_Param_Cmd(obj, argc-2, &argv[2]);
		}
	} 
	else 
	{
		corec.Console_Printf("swatch error:  no swatch command %s\n", argv[0]);
	}
}

void	GUI_Swatch_Init()
{
	corec.Cmd_Register("swatch", GUI_Swatch_Cmd);

	corec.GUI_RegisterClass(class_name, GUI_Swatch_Create);

	corec.Cvar_Register(&gui_swatchpanel);
	corec.Cvar_Register(&gui_swatchpanel_r_var);
	corec.Cvar_Register(&gui_swatchpanel_g_var);
	corec.Cvar_Register(&gui_swatchpanel_b_var);
	corec.Cvar_Register(&gui_swatchpanel_a_var);
}
