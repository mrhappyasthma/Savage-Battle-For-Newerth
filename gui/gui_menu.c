
#include "../toplayer/tl_shared.h"
#include "gui_menu.h"

static char *class_name = MENU_CLASS;

#define MIN_CLICK_TIME 300

void	GUI_Menu_Draw(gui_element_t *obj, int x, int y)
{
	gui_menu_t *menu;
	int w, h, i, rows, cols;

	menu = corec.GUI_GetUserdata(class_name, obj);

	if (!menu)
	{
		corec.Console_Printf("Error: couldn't get userdata for menu %s\n", obj->name);
		return;
	}

	x = 0;
	y = 0;
	w = obj->width;
	h = obj->height;

	corec.GUI_SetRGBA(menu->bg_r, menu->bg_g, menu->bg_b, obj->alpha);
	corec.GUI_Quad2d_S(0, 0, w, h, corec.GetWhiteShader());

	if (!menu->mouse_down && menu->selection >= 0 && menu->selection < menu->num_items)
	{
		corec.GUI_GetStringRowsCols(menu->items[menu->selection].name, &rows, &cols);
		corec.GUI_SetRGBA(obj->textcolor[0], obj->textcolor[1], obj->textcolor[2], obj->alpha);
		
		corec.GUI_DrawString(1, 1, menu->items[menu->selection].name, 
			obj->char_height, obj->char_height, rows, menu->element->width, corec.GetNicefontShader());
		return;
	}

	for (i=0; i < menu->num_items; i++)
	{
		corec.GUI_GetStringRowsCols(menu->items[i].name, &rows, &cols);
		
		if (i==menu->selection)
			corec.GUI_SetRGBA(menu->highlight_text_r, menu->highlight_text_g, menu->highlight_text_b, obj->alpha);
		else
			corec.GUI_SetRGBA(obj->textcolor[0], obj->textcolor[1], obj->textcolor[2], obj->alpha);
		
		corec.GUI_DrawString(1, i*obj->char_height, menu->items[i].name, 
			obj->char_height, obj->char_height, rows, menu->element->width, corec.GetNicefontShader());		
	}
}

void	GUI_Menu_Exec(gui_menu_t *menu, int x, int y)
{
	corec.GUI_Resize(menu->element, menu->element->width, menu->element->char_height);

	if (x < menu->element->width && x >= 0)
	{
		if (menu->selection >= 0 && menu->selection < menu->num_items)
		{
			corec.GUI_Exec(menu->items[menu->selection].command);
		}
		else
			menu->mouse_down = false;
	}
	else
		menu->mouse_down = false;
}

void	GUI_Menu_MouseDown (gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_menu_t *menu;

	menu = corec.GUI_GetUserdata(class_name, obj);

	if (!menu)
	{
		corec.Console_Printf("Error, we got an object with no userdata.\n");
		return;
	}

	if (menu->mouse_down)
	{
		menu->mouse_down = false;

		GUI_Menu_Exec(menu, x, y);
		return;
	}
	
	menu->mouse_down = true;
	menu->mousedown_time = corec.Milliseconds();

	corec.GUI_Resize(obj, obj->width, (menu->num_items+1) * obj->char_height);
}


void	GUI_Menu_MouseUp (gui_element_t *obj, mouse_button_enum button, int x, int y)
{
	gui_menu_t *menu;

	menu = corec.GUI_GetUserdata(class_name, obj);

	if (!menu)
	{
		corec.Console_Printf("Error, we got an object with no userdata.\n");
		return;
	}

	if (corec.Milliseconds() - menu->mousedown_time < MIN_CLICK_TIME)
		return;
					
	menu->mouse_down = false;

	GUI_Menu_Exec(menu, x, y);
}

/*
void	GUI_Menu_MouseOut(gui_element_t *obj)
{
	gui_menu_t *menu;
	menu = corec.GUI_GetUserdata(class_name, obj);

	if (!menu)
	{
		corec.Console_Printf("Error, we got an object with no userdata.\n");
		return;
	}

	menu->mouse_down = false;
}
*/

void	GUI_Menu_MouseOver(gui_element_t *obj, int x, int y)
{
	gui_menu_t *menu;
	menu = corec.GUI_GetUserdata(class_name, obj);

	if (!menu)
	{
		corec.Console_Printf("Error, we got an object with no userdata.\n");
		return;
	}

	if (menu->mouse_down)
		menu->selection = y / obj->char_height;

	if (menu->selection < 0)
		menu->selection = 0;
	
	if (menu->selection >= menu->num_items)
		menu->selection = menu->num_items-1;
}

void	GUI_Menu_Move(gui_element_t *obj, int x, int y)
{
	corec.GUI_Move(obj, x, y);
}

void GUI_Menu_Select(gui_element_t *obj, char *itemname)
{
	int i;
	gui_menu_t *menu = corec.GUI_GetUserdata(class_name, obj);

    if (!menu)
	        return;

	i = 0;
	while (i < menu->num_items)
	{
		if (strcmp(itemname, menu->items[i].name) == 0)
		{
			menu->selection = i;
			corec.GUI_Exec(menu->items[menu->selection].command);
			return;
		}
		i++;
	}
}

void GUI_Menu_Select_Cmd(int argc, char *argv[])
{
	gui_menu_t *menu;

	if (argc < 2)
	{
		corec.Console_Printf("menu select <panel:object> name\n");
		return;
	}

	menu = corec.GUI_GetClass(argv[0], class_name);

	if (!menu)
		return;

	GUI_Menu_Select(menu->element, argv[1]);

}

char *GUI_Menu_GetValue(gui_element_t *obj)
{
	gui_menu_t *menu = corec.GUI_GetUserdata(class_name, obj);
	if (!menu)
		return "";
	
	if (menu->selection >= 0 && menu->selection < menu->num_items)
		return menu->items[menu->selection].name;

	return "";
}

void GUI_Menu_AddItem(gui_menu_t *menu, const char *itemname, char *command)
{
	int length;
	char *s;

	if (!itemname)
		return;
	
	menu->num_items++;
	menu->items = corec.GUI_ReAlloc(menu->items, menu->num_items*sizeof(gui_menu_t));
	memset(&menu->items[menu->num_items-1], 0, sizeof(gui_menu_t));

	length = corec.GUI_StringWidth(itemname, menu->element->char_height, menu->element->char_height, 1, 1024, corec.GetNicefontShader());
	//if (length < menu->element->width)
	//{
		menu->items[menu->num_items-1].name = corec.GUI_StrDup(itemname);
		if (menu->trimstring[0] && (s = strstr(menu->items[menu->num_items-1].name, menu->trimstring)))
		{
			s[0] = 0;
		}
	/*}
	else 
	{
		menu->items[menu->num_items-1].name = corec.GUI_Malloc(sizeof (char) * MAX_MENU_ITEM_LENGTH);
		strncpy(menu->items[menu->num_items-1].name, itemname, MAX_MENU_ITEM_LENGTH-1);
		menu->items[menu->num_items-1].name[MAX_MENU_ITEM_LENGTH-1] = 0;
	}
	*/

	menu->items[menu->num_items-1].command = corec.GUI_StrDup(command);

	if (length + 4 > menu->element->width)
	{
		corec.GUI_Resize(menu->element, length + 4, menu->element->height);
	}
	
	if (menu->num_items == 1 || menu->selection < 0 || menu->selection >= menu->num_items)
	{
		menu->selection = 0;
	}

	corec.Console_DPrintf("adding option '%s' to menu that will execute %s\n", itemname, command);
}

void GUI_Menu_RemoveItem(gui_menu_t *menu, const char *itemname)
{
	int n;

	if (!itemname)
		return;
	
	for (n=0; n<menu->num_items; n++)
	{
		if (strcmp(menu->items[n].name, itemname)==0)
		{
			corec.GUI_Free(menu->items[n].name);
			corec.GUI_Free(menu->items[n].command);
			memmove(&menu->items[n], &menu->items[n+1], sizeof(gui_menu_t) * (menu->num_items - n));
			menu->num_items--;
			if (menu->num_items > 0)
				menu->items = corec.GUI_ReAlloc(menu->items, menu->num_items*sizeof(gui_menu_t));
			else
				corec.GUI_Free(menu->items);
		}
	}
}

void	GUI_Menu_RemoveAllItems(gui_menu_t *menu)
{
	int n;

	for (n=0; n<menu->num_items; n++)
	{
		corec.GUI_Free(menu->items[n].name);
		corec.GUI_Free(menu->items[n].command);
	}
	corec.GUI_Free(menu->items);
	menu->num_items = 0;
	menu->items = NULL;
}

bool	GUI_Menu_ItemExists(gui_menu_t *menu, const char *item)
{
	int n;

	for (n=0; n<menu->num_items; n++)
	{
		if (menu->items[n].name && strcmp(menu->items[n].name, item)==0)
			return true;
	}
	
	return false;
}

void GUI_Menu_Add_Cmd(int argc, char *argv[])
{
	gui_menu_t *menu;

	if (argc < 3)
	{
		corec.Console_Printf("menu add <panel:object> name command\n");
		return;
	}

	menu = corec.GUI_GetClass(argv[0], class_name);

	if (!menu)
		return;

	GUI_Menu_AddItem(menu, argv[1], argv[2]);
}

void GUI_Menu_Remove_Cmd(int argc, char *argv[])
{
	gui_menu_t *menu;

	if (argc < 2)
	{
		corec.Console_Printf("menu remove <panel:object> name\n");
		return;
	}

	menu = corec.GUI_GetClass(argv[0], class_name);

	if (!menu)
		return;

	GUI_Menu_RemoveItem(menu, argv[1]);
}

typedef struct
{
	char *command;
	gui_menu_t *menu;
} menuCallbackHelper_t;

void ItemizeFileCallback(const char *filename, void *data)
{
  	menuCallbackHelper_t *mch = (menuCallbackHelper_t *)data;
	char fname[1024];

	Filename_StripExtension(filename, fname);

	GUI_Menu_AddItem(mch->menu, fname, mch->command);
}

void GUI_Menu_ItemizeFiles_Cmd(int argc, char *argv[])
{
	gui_menu_t *menu;
	menuCallbackHelper_t mch;

	if (argc < 4)
	{
		corec.Console_Printf("menu itemizefiles <panel:object> directory wildcard command\n");
		corec.Console_Printf("got %i args %s   %s\n",argc,argv[0],argv[1]);

		return;
	}

	menu = corec.GUI_GetClass(argv[0], class_name);

	if (!menu)
		return;

	mch.command = argv[3];
	mch.menu = menu;

	corec.System_Dir(argv[1], argv[2], false, NULL, ItemizeFileCallback, &mch);
}

void ItemizeDirCallback(const char *dirname, void *data)
{
  	menuCallbackHelper_t *mch = (menuCallbackHelper_t *)data;
	char *s = StrChrBackwards(dirname, '/');
	s+=1;

	GUI_Menu_AddItem(mch->menu, s, mch->command);
}

void GUI_Menu_ItemizeDirs_Cmd(int argc, char *argv[])
{
	gui_menu_t *menu;
	menuCallbackHelper_t mch;

	if (argc < 3)
	{
		corec.Console_Printf("menu itemizedirs <panel:object> root_dir command\n");
		return;
	}

	menu = corec.GUI_GetClass(argv[0], class_name);

	if (!menu)
		return;

	mch.command = argv[2];
	mch.menu = menu;

	corec.System_Dir(argv[1], "*", false, ItemizeDirCallback, NULL, &mch);
}



void GUI_Menu_Param_Cmd(gui_element_t *obj, int argc, char *argv[])
{
	gui_menu_t *menu;

	if (argc < 1)
	{
		corec.Console_Printf("menu param <panel:object> <var> <value>\n");
		corec.Console_Printf("valid parameters:\n");
		corec.Console_Printf("   bg_color <r> <g> <b>        - sets the background color\n");
		corec.Console_Printf("   text_color <r> <g> <b>      - sets the text color\n");
		corec.Console_Printf("   highlight_color <r> <g> <b> - sets the highlighted text color\n");
		corec.Console_Printf("   char_height <height>          - sets the font width, in pixels\n");
		return;
	}

	menu = corec.GUI_GetUserdata(class_name, obj);

	if (!menu)
		return;

	if (strcmp(argv[0], "bg_color") == 0)
	{
		if (argc > 3)
		{
			menu->bg_r = atof(argv[1]);
			menu->bg_g = atof(argv[2]);
			menu->bg_b = atof(argv[3]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the R, G, and B values (range [0..1])\n");
		}
	}
	else if (strcmp(argv[0], "highlight_color") == 0)
	{
		if (argc > 3)
		{
			menu->highlight_text_r = atof(argv[1]);
			menu->highlight_text_g = atof(argv[2]);
			menu->highlight_text_b = atof(argv[3]);
		} else {
			corec.Console_Printf("Not enough parameters.  You must specify the R, G, and B values (range [0..1])\n");
		}
	}
}

void GUI_Menu_Clear(gui_menu_t *menu)
{
	GUI_Menu_RemoveAllItems(menu);
	menu->selection = -1;
}

void GUI_Menu_Clear_Cmd(int argc, char *argv[])
{
	gui_menu_t *menu;

	if (argc < 1)
	{
		corec.Console_Printf("menu clear <panel:object>\n");
		return;
	}

	menu = corec.GUI_GetClass(argv[0], class_name);

	if (!menu)
		return;

	GUI_Menu_Clear(menu);
}

void	GUI_Menu_Trim(gui_menu_t *menu, char *string)
{
	BPrintf(menu->trimstring, MENU_MAXTRIMSTRING, "%s", string);
}

void GUI_Menu_Trim_Cmd(int argc, char *argv[])
{
	gui_menu_t *menu;

	if (argc < 2)
	{
		corec.Console_Printf("menu trim <panel:object> <string>\n");
		return;
	}

	menu = corec.GUI_GetClass(argv[0], class_name);

	if (!menu)
		return;

	GUI_Menu_Trim(menu, argv[1]);
}

void	GUI_Menu_List()
{
	corec.GUI_List_Cmd(1, &class_name);
}

void	GUI_Menu_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		corec.Console_Printf("menu <command> <args>\n");
		corec.Console_Printf("  commands:\n");
		corec.Console_Printf("    list\n");
		corec.Console_Printf("    add\n");
		corec.Console_Printf("    remove\n");
		corec.Console_Printf("    select\n");
		corec.Console_Printf("    clear\n");
		corec.Console_Printf("    param\n");
		corec.Console_Printf("    itemizefiles\n");
		corec.Console_Printf("    itemizedirs\n");
		return;
	}

	if (strcmp(argv[0], "list") == 0)
	{
		GUI_Menu_List();
	} else 	if (strcmp(argv[0], "select") == 0)
	{
		GUI_Menu_Select_Cmd(argc-1, &argv[1]);
	} else 	if (strcmp(argv[0], "add") == 0)
	{
		GUI_Menu_Add_Cmd(argc-1, &argv[1]);
	} else 	if (strcmp(argv[0], "remove") == 0)
	{
		GUI_Menu_Remove_Cmd(argc-1, &argv[1]);
	} else 	if (strcmp(argv[0], "clear") == 0)
	{
		GUI_Menu_Clear_Cmd(argc-1, &argv[1]);
	} else 	if (strcmp(argv[0], "param") == 0)
	{
		if (argc > 1)
		{
			obj = corec.GUI_GetObject(argv[1]);
			GUI_Menu_Param_Cmd(obj, argc-2, &argv[2]);
		}
	} else if (strcmp(argv[0], "itemizefiles") ==0)
	{
		GUI_Menu_ItemizeFiles_Cmd(argc-1, &argv[1]);
	} else if (strcmp(argv[0], "itemizedirs")==0)
	{
		GUI_Menu_ItemizeDirs_Cmd(argc-1, &argv[1]);
	}
	else if (strcmp(argv[0], "trim")==0)
	{
		GUI_Menu_Trim_Cmd(argc-1, &argv[1]);
	}
}

void	GUI_Menu_Destroy(gui_element_t *obj)
{
	gui_menu_t *menu;
	
	menu = corec.GUI_GetUserdata(class_name, obj);
	if (!menu)
		return;

	GUI_Menu_RemoveAllItems(menu);
	//that will also free the menu->items array
}

//menu name x y w
void	*GUI_Menu_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_menu_t *menu;
	
	if (argc < 4)
	{
		corec.Console_Printf("syntax: create %s name x y w\n", class_name);
		return NULL;
	}

	corec.GUI_SetClass(obj, class_name);
	corec.GUI_SetName(obj, argv[0]);
	corec.GUI_Move (obj, atoi(argv[1]), atoi(argv[2]) );

	menu = corec.GUI_Malloc(sizeof (gui_menu_t));

	if (!menu)
	{
		corec.Console_Printf("Menu error: couldn't enough space to hold menu\n");
		return NULL; 		
	}

	corec.GUI_SetUserdata(class_name, obj, menu);
	menu->element = obj;

	menu->num_items = 0;
	menu->items = NULL;

	menu->bg_r = 0.1;
	menu->bg_g = 0.1;
	menu->bg_b = 0.1;
	menu->highlight_text_r = 1;
	menu->highlight_text_g = 1;
	menu->highlight_text_b = 1;
	obj->char_height = 16;

	corec.GUI_Resize(obj, atoi(argv[3]), (menu->num_items+1) * obj->char_height);

	obj->destroy = GUI_Menu_Destroy;
	obj->draw = GUI_Menu_Draw;
	obj->idle = NULL;
	obj->move = GUI_Menu_Move;
	obj->mouseup = GUI_Menu_MouseUp;
	obj->mousedown = GUI_Menu_MouseDown;
	//obj->mouseout = GUI_Menu_MouseOut;
	obj->mouseover = GUI_Menu_MouseOver;
	
	obj->param = GUI_Menu_Param_Cmd;

	obj->getvalue = GUI_Menu_GetValue;

	obj->noclip = true;		//don't want drawing constrained when the menu pops up

	menu->mouse_down = false;
	menu->selection = 0;
	menu->trimstring[0] = 0;

	menu->parent = NULL;

	return menu;
}

void	GUI_Menus_Init()
{
	corec.GUI_RegisterClass(class_name, GUI_Menu_Create);

	corec.Cmd_Register("menu", GUI_Menu_Cmd);
}
