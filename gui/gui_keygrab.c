#include "../toplayer/tl_shared.h"
#include "gui_keygrab.h"

static char *class_name = "keygrab";

#define KEYGRAB_SPACING_X 2
#define KEYGRAB_SPACING_Y 1

bool	GUI_Keygrab_ReadingInput(gui_element_t *obj)
{
	return (corec.Input_GetCallbackWidget() == obj);
}

/*==========================

  GUI_DrawKeygrab

 ==========================*/

void	GUI_DrawKeygrab(gui_element_t *obj, int x, int y)
{
	gui_keygrab_t *keygrab;
	char	keystr[64], *keyname;
	int		len = 0, index;
	bool	firstkey = true;

	keygrab = corec.GUI_GetUserdata(class_name, obj);

	if (!keygrab)
		return;

	corec.GUI_SetRGBA(1, 1, 1, obj->alpha);
	if (keygrab->border)
		corec.GUI_LineBox2d_S(0, 0, obj->width, obj->height, keygrab->border);

	corec.GUI_SetRGBA(obj->textcolor[0], obj->textcolor[1], obj->textcolor[2], obj->alpha);

	//find keys that are bound to this
	memset(keystr, 0, sizeof(keystr));
	for (index = 0; index < 255; index++)
	{
		//only do the alphabet once
		if (index == 'A')
			index = 'Z' + 1;

		keyname = corec.Input_GetNameFromKey(index);
		
		//skip keys that don't return a valid name
		if (!*keyname)
			continue;

		//check that the binding matches the widgets binding commands
		if (strcmp(corec.Input_GetBindDownCmd(index), keygrab->keyDownCmd))
			continue;
		if (strcmp(corec.Input_GetBindUpCmd(index), keygrab->keyUpCmd))
			continue;

		//add the keyname to display string
		if (len + strlen(keyname) + (firstkey ? 0 : 2) < sizeof(keystr) - 1)
		{
			len += strlen(keyname) + (firstkey ? 0 : 2);
			if (!firstkey)
			{
				strcat(keystr, ", ");
			}
			strcat(keystr, keyname);
			firstkey = false;
		}
	}
	
	//draw input buffer
	corec.GUI_DrawString(KEYGRAB_SPACING_X,
				KEYGRAB_SPACING_Y,
				strupr(keystr),
				obj->char_height,
				obj->char_height,
				1,
				obj->width - (KEYGRAB_SPACING_X*2),
				corec.GetNicefontShader()
				);
}


/*==========================

  GUI_Keygrab_Mousedown

 ==========================*/

void	GUI_Keygrab_Mousedown(gui_element_t *obj, mouse_button_enum button, int x, int y)
{
//	GUI_Keygrab_Activate(obj);
}


/*==========================

  GUI_MoveKeygrab

 ==========================*/

void	GUI_MoveKeygrab(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}


/*==========================

  GUI_Keygrab_Activate

 ==========================*/

void	GUI_Keygrab_Activate(gui_element_t *obj)
{
	gui_keygrab_t *keygrab;

	keygrab = corec.GUI_GetUserdata(class_name, obj);

	if (!keygrab)
		return;
	
	if (!corec.Input_ActivateInputCallback(GUI_Keygrab_InputCallback, false, obj))
	{
		corec.Console_Printf("Keygrab error: Can't get input focus\n");		
		return;
	}

	if (keygrab->panel)
		corec.GUI_Panel_Show(keygrab->panel);

	return;
}


/*==========================

  GUI_Keygrab_Deactivate

 ==========================*/

void	GUI_Keygrab_Deactivate(gui_element_t *obj)
{
	gui_keygrab_t *keygrab;

	keygrab = corec.GUI_GetUserdata(class_name, obj);

	if (!keygrab)
		return;

	corec.Input_DeactivateInputCallback(true);
	corec.GUI_Unfocus(obj);

	if (keygrab->panel)
		corec.GUI_Panel_Hide(keygrab->panel);
}

void	GUI_Keygrab_Hide(gui_element_t *obj)
{
	if (GUI_Keygrab_ReadingInput(obj))
		GUI_Keygrab_Deactivate(obj);
}

/*==========================

  GUI_Keygrab_Commit

 ==========================*/

void	GUI_Keygrab_Commit(gui_keygrab_t *keygrab, int key)
{
	char	*name;

	name = corec.Input_GetNameFromKey(key);

	if (*name)
	{
		corec.Cmd_Exec(fmt("bind %s \"%s\"", name, keygrab->keyDownCmd));
		corec.Cmd_Exec(fmt("bindup %s \"%s\"", name, keygrab->keyUpCmd));
	}
	corec.GUI_Exec(keygrab->exec_cmd);

	GUI_Keygrab_Deactivate(keygrab->element);
}


/*==========================

  GUI_Keygrab_Abort

 ==========================*/

void	GUI_Keygrab_Abort(gui_keygrab_t *keygrab)
{
	if (keygrab->abort_cmd)
		corec.GUI_Exec(keygrab->abort_cmd);

	GUI_Keygrab_Deactivate(keygrab->element);
}


/*==========================

  GUI_Keygrab_InputCallback

 ==========================*/

bool	GUI_Keygrab_InputCallback(int key, char rawchar, bool down, void *userdata)
{
	gui_keygrab_t *keygrab;

	if (!userdata)
		return false;

	keygrab = corec.GUI_GetUserdata(class_name, (gui_element_t *)userdata);

	if (!keygrab)
		return false;

	if (!down)
		return true;

	if (key == KEY_ESCAPE)
		GUI_Keygrab_Abort(keygrab);
	else
		GUI_Keygrab_Commit(keygrab, key);

	return true;
}


/*==========================

  GUI_Keygrab_Param_Cmd

 ==========================*/

void	GUI_Keygrab_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_keygrab_t *keygrab;
	int oldBorder;
	char	buf[CMD_MAX_LENGTH];

	if (argc < 2)
	{
		corec.Console_Printf("keygrab param <panel:object> <param> <args>\n");
		corec.Console_Printf("  params:\n");
		corec.Console_Printf("    commit_cmd - executed when a key is pressed\n");
		corec.Console_Printf("    abort_cmd  - executed if the grab is aborted\n");
		corec.Console_Printf("    border     - size of the border to draw around the widget\n");
		corec.Console_Printf("    down       - bind for key press\n");
		corec.Console_Printf("    up         - bind for key release\n");
		return;
	}

	keygrab = corec.GUI_GetUserdata(class_name, obj);

	if (!keygrab)
		return;

	if (strcmp(argv[0], "commit_cmd") == 0)
	{
		strncpy(keygrab->exec_cmd, argv[1], CMD_MAX_LENGTH);
	}
	else if (strcmp(argv[0], "abort_cmd") == 0)
	{
		strncpy(keygrab->abort_cmd, argv[1], CMD_MAX_LENGTH);
	}
	else if (strcmp(argv[0], "down") == 0)
	{
		ConcatArgs(&argv[1], argc - 1, buf);
		memset(keygrab->keyDownCmd, 0, CMD_MAX_LENGTH);
		strncpy(keygrab->keyDownCmd, buf, CMD_MAX_LENGTH);
	}
	else if (strcmp(argv[0], "up") == 0)
	{
		ConcatArgs(&argv[1], argc - 1, buf);
		memset(keygrab->keyUpCmd, 0, CMD_MAX_LENGTH);
		strncpy(keygrab->keyUpCmd, buf, CMD_MAX_LENGTH);
	}
	else if (strcmp(argv[0], "border") == 0)
	{
		oldBorder = keygrab->border;
		keygrab->border = atoi(argv[1]);
		corec.GUI_Resize (obj,
			keygrab->element->width + keygrab->border*2 - oldBorder*2 + KEYGRAB_SPACING_Y*2,
			obj->char_height + keygrab->border*2 + KEYGRAB_SPACING_X*2);
	}
}


/*==========================

  GUI_Keygrab_Create

  keygrab "name" x y width height panel

 ==========================*/

void	*GUI_Keygrab_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_keygrab_t *keygrab;

	if (argc < 5)
	{
		corec.Console_Printf("syntax: create keygrab name x y width height\n");
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move(obj, atoi(argv[1]), atoi(argv[2]));
	corec.GUI_Resize (obj, atoi(argv[3]), atoi(argv[4]));

	keygrab = corec.GUI_Malloc(sizeof (gui_keygrab_t));
	if (!keygrab)
	{
		corec.Console_Printf("Keygrab error: couldn't enough space to hold keygrab\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, keygrab);
	keygrab->element = obj;

	keygrab->panel = NULL;
	if (argc > 5)
		keygrab->panel = corec.GUI_GetPanel(argv[5]);

	strcpy(keygrab->exec_cmd, "");
	strcpy(keygrab->abort_cmd, "");
	strcpy(keygrab->keyDownCmd, "");
	strcpy(keygrab->keyUpCmd, "");

	obj->char_height = 12;
	keygrab->border = 1;

	obj->interactive = true;
	obj->draw = GUI_DrawKeygrab;
	obj->move = GUI_MoveKeygrab;
	obj->mousedown = GUI_Keygrab_Mousedown;
	obj->focused = GUI_Keygrab_Activate;
	obj->unfocused = GUI_Keygrab_Deactivate;
	obj->loseInput = GUI_Keygrab_Deactivate;
	obj->hide = GUI_Keygrab_Hide;

	obj->param = GUI_Keygrab_Param_Cmd;

	obj->getvalue = NULL;

	obj->loseFocusOnMouseout = false;
	obj->wantsMouseDownClicks = true;
	obj->wantsMouseUpClicks = false;
	obj->wantsFunctionKeys = true;	

	keygrab->parent = NULL;

	return keygrab;
}


/*==========================

  GUI_Keygrab_Abort_Cmd

 ==========================*/

void	GUI_Keygrab_Abort_Cmd(int argc, char *argv[])
{
	gui_keygrab_t *keygrab;

	if (argc < 1)
	{
		corec.Console_Printf("keygrab abort <panel:object>\n");
		return;
	}

	keygrab = corec.GUI_GetClass(argv[0], class_name);

	if (!keygrab)
		return;

	GUI_Keygrab_Abort(keygrab);
}


/*==========================

  GUI_Keygrab_Activate_Cmd

 ==========================*/

void	GUI_Keygrab_Activate_Cmd(int argc, char *argv[])
{
	gui_keygrab_t *keygrab;

	if (argc < 1)
	{
		corec.Console_Printf("keygrab activate <panel:object>\n");
		return;
	}

	keygrab = corec.GUI_GetClass(argv[0], class_name);

	if (!keygrab)
		return;

	corec.GUI_Focus(keygrab->element);
}


/*==========================

  GUI_Keygrab_Cmd

 ==========================*/

void	GUI_Keygrab_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		corec.Console_Printf("keygrab <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list     - lists all the keygrabs\n");
		corec.Console_Printf("    activate - activates a keygrab (causes it to focus and start reading input)\n");
		corec.Console_Printf("    abort    - deactivates a keygrab (causes it to lose focus and stop reading input)\n");
		corec.Console_Printf("    param	   - set parameters for this widget\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		corec.GUI_List_Cmd(1, &class_name);
	} 
	else if (strcmp(argv[0], "activate") == 0)
	{
		GUI_Keygrab_Activate_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "abort") == 0)
	{
		GUI_Keygrab_Abort_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "param") == 0)
	{
		gui_element_t *obj;

		if (argc > 2)
		{
			obj = corec.GUI_GetObject(argv[1]);
			GUI_Keygrab_Param_Cmd(obj, argc-2, &argv[2]);
		}
	} 
	else 
	{
		corec.Console_Printf("keygrab error:  no keygrab command %s\n", argv[0]);
	}
}


/*==========================

  GUI_Keygrab_Init

 ==========================*/

void	GUI_Keygrab_Init()
{
	corec.Cmd_Register("keygrab", GUI_Keygrab_Cmd);	

	corec.GUI_RegisterClass(class_name, GUI_Keygrab_Create);
}
