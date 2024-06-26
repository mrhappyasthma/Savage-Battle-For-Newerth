#include "core.h"

gui_panel_t		panelList;
char			*current_panelname = NULL;
#define CURRENT_PANEL panelList.next

#define GUI_SCALE_WIDTH		1 //((float)gui_screenw / 1024)
#define GUI_SCALE_HEIGHT	1 //((float)gui_screenh / 768)

extern int		gui_screenw, gui_screenh;

static char *class_name = "panel";


void	GUI_StaticDepth_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("syntax: staticdepth <0|1>\n");
		return;
	}

	if (LIST_EMPTY(&panelList))
	{
		return;
	}

	CURRENT_PANEL->staticdepth = atoi(argv[0]);
}

void	GUI_Panel_FadeIn(gui_panel_t *panel, float time)
{
	gui_objectlist_t *obj;

	if (!panel->visible)
	{
		panel->visible = true;
		for (obj = panel->objects.next; obj!=&panel->objects; obj=obj->next)
			GUI_ChangeAlpha(obj->object, 0);
	}
	
	for (obj = panel->objects.next; obj!=&panel->objects; obj=obj->next)
	{
		GUI_FadeInObject(obj->object, time);
	}
}

void	GUI_Panel_FadeOut(gui_panel_t *panel, float time)
{
	gui_objectlist_t *obj;

	for (obj = panel->objects.next; obj!=&panel->objects; obj=obj->next)
	{
		if (obj->object)
			GUI_FadeOutObject(obj->object, time);
	}
}

void	GUI_Panel_AddObject(gui_element_t *obj)
{
	gui_objectlist_t *new_object;

	if (LIST_EMPTY(&panelList))
	{
		Console_Printf("No panels exist!  Not adding GUI widget.\n");
		return;
	}

	new_object = (gui_objectlist_t *)GUI_Malloc(sizeof (gui_objectlist_t));

	if (!new_object)
	{
		Console_Printf("Couldn't allocate enough space for this GUI widget.\n");
		return;
	}

	new_object->object = obj;
	LIST_INSERT(&CURRENT_PANEL->objects, new_object);
	CURRENT_PANEL->num_objects++;

	obj->panel = CURRENT_PANEL;

	M_AddRectToRectI(obj->bmin, obj->bmax, CURRENT_PANEL->bmin, CURRENT_PANEL->bmax);

	//Console_DPrintf("Adding widget %s to panel %s\n", obj->name, obj->panel->name);
}

void	GUI_Panel_UpdateWidgetSize(gui_element_t *obj)
{
	M_AddRectToRectI(obj->bmin, obj->bmax, obj->panel->bmin, obj->panel->bmax);
}

void	GUI_Panel_List()
{
	gui_panel_t *tmp_panelList = panelList.next;

	Console_Printf("Current panels:\n");

	while (tmp_panelList != &panelList)
	{
		Console_Printf("  %s\n", tmp_panelList->name);
		tmp_panelList = tmp_panelList->next;
	}
}

gui_panel_t *GUI_GetPanel(char *name)
{
	gui_panel_t *tmp_panelList = panelList.next;

	/*if (current_panelname)
	{
		if (strcmp(name, current_panelname)==0)
			return panelList.next;
	}*/

	while (tmp_panelList 
		   && tmp_panelList != &panelList)
	{
		if (tmp_panelList->valid 
			&& strcmp(tmp_panelList->name, name) == 0)
			return tmp_panelList;
		tmp_panelList = tmp_panelList->next;
	}
	return NULL;
}

//x and y are absolute coords, pivot_x and pivot_y are relative
// if pivot_x and pivot_y are not relative, this will result in terribly wrong calculations
void	GUI_Panel_Move(gui_panel_t *panel, int x, int y)
{
	int i, scaled_x, scaled_y;

	if (!panel)
		return;

	scaled_x = (int)((float)(x + panel->pivot[X]) * GUI_SCALE_WIDTH) - panel->pivot[X];
	scaled_y = (int)((float)(y + panel->pivot[Y]) * GUI_SCALE_HEIGHT) - panel->pivot[Y];

	for (i = 0; i < panel->num_children; i++)
	{
		GUI_Panel_MoveRelative(panel->children[i], scaled_x - panel->pos[X], scaled_y - panel->pos[Y]);
	}

	panel->pos[X] = scaled_x;
	panel->pos[Y] = scaled_y;
}

//x and y are given in relative screen coords.  Ugly, but necessary for floaters
void	GUI_Panel_MoveRelative(gui_panel_t *panel, int x, int y)
{
	int i;
	int xx,yy;

	if (!panel)
		return;

	xx = x;
	yy = y;
//	GUI_Coord(&xx, &yy);

	panel->pos[X] += xx;
	panel->pos[Y] += yy;

	for (i = 0; i < panel->num_children; i++)
	{
		GUI_Panel_MoveRelative(panel->children[i], x, y);
	}
}

void	GUI_Panel_Hide(gui_panel_t *panel)
{
	gui_objectlist_t *list;

	if (panel)
	{
		for (list = panel->objects.next; list != &panel->objects; list = list->next)
		{
			if (list->object->visible)
			{
				GUI_Hide(list->object);
				list->object->visible = true;		//keep visible true for when the panel is shown again
			}
		}
		
		panel->visible = false;
	}
}

void	GUI_Panel_Show(gui_panel_t *panel)
{
	gui_objectlist_t *list;

	if (panel)
	{
		for (list = panel->objects.next; list != &panel->objects; list = list->next)
		{
			if (list->object->visible)
			{
				GUI_Show(list->object);
			}
		}

		panel->visible = true;
	}
}

void	GUI_Panel_Focus(gui_panel_t *panel)
{
	if (!panel)
		return;

	GUI_Panel_Show(panel);
	GUI_Panel_BringTreeToFront(panel);
}

//walks up the panels until we hit the farthest parent, then returns it
gui_panel_t *GUI_Panel_GetAncestor(gui_panel_t *panel)
{
	if (!panel)
		return NULL;

	while (panel->parent)
		panel = panel->parent;

	return panel;
}

// Brings the specified panel and all of its children (and all it's children's children, etc.) to the front
void	GUI_Panel_BringToFront(gui_panel_t *panel)
{
	int n;

	if (!panel)
		return;

	LIST_REMOVE(panel);
	LIST_APPEND(&panelList, panel);

	for (n=0; n<panel->num_children; n++)
	{
		GUI_Panel_BringToFront(panel->children[n]);
	}
}

//brings the entire tree belonging to panel to the front
void	GUI_Panel_BringTreeToFront(gui_panel_t *panel)
{
	gui_panel_t *ancestor;

	if (!panel)
		return;

	ancestor = GUI_Panel_GetAncestor(panel);
	GUI_Panel_BringToFront(ancestor);
}

//panel create name x y 
void	*GUI_Panel_Create(gui_element_t *obj, int argc, char *argv[])
{
	gui_panel_t *panel;

	if (argc < 3)
	{
		Console_Printf("syntax: panel name x y [pivot_x] [pivot_y]\n");
		return NULL;
	}

	if (GUI_GetPanel(argv[0]))
	{
		Console_Printf("Panel named '%s' already exists.\n", argv[0]);
		return NULL;
	}

	if (panelList.next != &panelList && !panelList.next->valid)
	{
		panel = panelList.next;
	} 
	else 
	{
		panel  = (gui_panel_t *)GUI_Malloc(sizeof (gui_panel_t));		

		LIST_INSERT(&panelList, panel);
	}

	panel->name = GUI_Malloc(sizeof(char) * (strlen(argv[0])+1));
	strcpy(panel->name, argv[0]);

	panel->num_objects = 0;
	LIST_CLEAR(&panel->objects);
	panel->num_children = 0;
	panel->children = NULL;
	panel->parent = NULL;

	if (argc > 4)
	{
		panel->pivot[X] = atoi(argv[3]); // * GUI_SCALE_WIDTH;
		panel->pivot[Y] = atoi(argv[4]); // * GUI_SCALE_HEIGHT;
	} 
	else 
	{
		panel->pivot[X] = 0;
		panel->pivot[Y] = 0;
	}

	GUI_Panel_Move(panel, atoi(argv[1]), atoi(argv[2]));
		
	panel->visible = true;
	panelList.next->valid = true;

	current_panelname = panel->name;  //so we don't have to type the fully qualified object name

	M_ClearRectI(panel->bmin, panel->bmax);

	//return NULL so widget is used for another class -- we don't need it
	return NULL;
}

void    GUI_RemovePanel(gui_panel_t *panel)
{
	gui_objectlist_t *obj;
	int i, j, num_children;
	
	if (!panel)
		return;

	while ((obj = panel->objects.next) && obj != &panel->objects)
	{
		gui_element_t *elem = obj->object;
	
		GUI_RemoveElement(elem);
		LIST_REMOVE(obj);
		panel->num_objects--;
		GUI_Free(obj);
	}
	panel->objects.object = NULL;

	GUI_Free(panel->name);
	//short circuit the "remove this from the parent panel" crap so we don't waste time with a lot of reallocs and memory moving
	// the children will see that parent->num_children is 0 and won't try to free themselves
	num_children = panel->num_children;
	panel->num_children = 0;
	
	for (i = 0; i < num_children; i++)
	{
		GUI_RemovePanel(panel->children[i]);
		panel->children[i] = NULL;
	}
	
	GUI_Free(panel->children);
	panel->children = NULL;
	
	//find this panel in the parent and remove it
	if (panel->parent)
	{
		for (i = 0; i < panel->parent->num_children; i++)
			if (panel->parent->children[i] == panel)
			{
				//remove this from the parent
				panel->parent->num_children--;
				for (j = 0; j < panel->parent->num_children; j++)
					panel->parent->children[j] = panel->parent->children[j+1];
				panel->parent->children = Tag_Realloc(panel->parent->children, 
													  sizeof(gui_panel_t *) * panel->parent->num_children, MEM_GUI);
			}
	}
	else
	{
		LIST_REMOVE(panel);
	}
	GUI_Free(panel);
}

void	GUI_RemovePanels()
{
	//deactivate the input callback
	Input_DeactivateInputCallback(true);

	GUI_ClearWidgets();
}

//-----  commands

void	GUI_RemovePanels_Cmd(int argc, char *argv[])
{
	GUI_RemovePanels();
}

void	GUI_RemovePanel_Cmd(int argc, char *argv[])
{
	gui_panel_t *panel;

	if (argc < 1)
	{
		Console_Printf("syntax: panel remove <name>\n");
		return;
	}

	panel = GUI_GetPanel(argv[0]);

	if (!panel)
	{
		Console_Printf("Panel '%s' not found.\n", argv[0]);
		return;
	}

	GUI_RemovePanel(panel);
}

void	GUI_MovePanel_Cmd(int argc, char *argv[])
{
	int x, y;
	gui_panel_t *panel;

	if (argc < 3)
	{
		Console_Printf("syntax: panel move <name> [+-]x [+-]y\n");
		return;
	}

	panel = GUI_GetPanel(argv[0]);

	if (!panel)
	{
		Console_Printf("Panel '%s' not found.\n", argv[0]);
		return;
	}

	x = atoi(argv[1]);
	y = atoi(argv[2]);

	/*if (argv[1][0] == '-' ||
		argv[1][0] == '+')
	{
		x = ((int)((float)x * GUI_SCALE_WIDTH) - panel->pos[0]) / GUI_SCALE_WIDTH;
		y = ((int)((float)y * GUI_SCALE_HEIGHT) - panel->pos[1]) / GUI_SCALE_HEIGHT;
	}*/

	GUI_Panel_Move(panel, x, y);
}

void	GUI_HidePanel_Cmd(int argc, char *argv[])
{
	gui_panel_t *panel;

	if (argc < 1)
	{
		Console_Printf("syntax: panel hide <name>\n");
		return;
	}

	panel = GUI_GetPanel(argv[0]);

	if (!panel)
	{
		Console_Printf("Panel '%s' not found.\n", argv[0]);
		return;
	}

	GUI_Panel_Hide(panel);
}

void	GUI_ShowPanel_Cmd(int argc, char *argv[])
{
	gui_panel_t *panel;

	if (argc < 1)
	{
		Console_Printf("syntax: panel show <name>\n");
		return;
	}

	panel = GUI_GetPanel(argv[0]);

	if (!panel)
	{
		Console_Printf("Panel '%s' not found.\n", argv[0]);
		return;
	}

	GUI_Panel_Show(panel);
}

void	GUI_FocusPanel_Cmd(int argc, char *argv[])
{
	gui_panel_t *panel;

	if (argc < 1)
	{
		Console_Printf("syntax: panel focus <name>\n");
		return;
	}

	panel = GUI_GetPanel(argv[0]);

	if (!panel)
	{
		Console_Printf("Panel '%s' not found.\n", argv[0]);
		return;
	}

	GUI_Panel_Focus(panel);
}

void	GUI_HideAllPanels()
{
	gui_panel_t *tmp_panelList = panelList.next;

	Console_DPrintf("Hiding all gui panels\n");
	while (tmp_panelList 
		   && tmp_panelList != &panelList)
	{
		GUI_Panel_Hide(tmp_panelList);
		tmp_panelList = tmp_panelList->next;
	}
}

void	GUI_LinkPanel_Cmd(int argc, char *argv[])
{
	gui_panel_t *panel1, *panel2;

	if (argc < 2)
	{
		Console_Printf("syntax: panel link <panel1> <panel2> [top|bottom|left|right]\n -- links panel1 to panel2");
		return;
	}

	panel1 = GUI_GetPanel(argv[0]);

	if (!panel1)
	{
		Console_Printf("Panel '%s' not found.\n", argv[0]);
		return;
	}

	panel2 = GUI_GetPanel(argv[1]);

	if (!panel2)
	{
		Console_Printf("Panel '%s' not found.\n", argv[1]);
		return;
	}

	panel2->num_children++;
	panel2->children = GUI_ReAlloc(panel2->children, sizeof(gui_panel_t) * panel2->num_children);
	panel2->children[panel2->num_children-1] = panel1;
	panel2->children[panel2->num_children-1]->parent = panel2;

	if (argc > 2)
	{
		if (strcmp(argv[2], "top") == 0)
		{
			panel1->pos[Y] = panel2->pos[Y] - 1 - (panel1->bmax[Y] - panel1->bmin[Y]);
		}
		else if (strcmp(argv[2], "bottom") == 0)
		{
			panel1->pos[Y] = panel2->pos[Y] + 1 + (panel2->bmax[Y] - panel2->bmin[Y]);
		}
		else if (strcmp(argv[2], "left") == 0)
		{
			panel1->pos[X] = panel2->pos[X] - 1 - (panel1->bmax[X] - panel1->bmin[X]);
		}
		else if (strcmp(argv[2], "right") == 0)
		{
			panel1->pos[X] = panel2->pos[X] + 1 + (panel2->bmax[X] - panel2->bmin[X]);
		}
	}
}


void	GUI_Panel_Cmd(int argc, char *argv[])
{
	if (!argc)
	{
		Console_Printf("panel <command> <args>\n");
		Console_Printf("  commands:\n");
		Console_Printf("    list\n");
		Console_Printf("    end\n");
		Console_Printf("    move\n");
		Console_Printf("    show\n");
		Console_Printf("    focus\n");
		Console_Printf("    hide\n");
		Console_Printf("    remove (aka destroy)\n");
		Console_Printf("    link\n");
		return;
	}

	if (strcmp(argv[0], "move") == 0)
	{
		GUI_MovePanel_Cmd(argc-1, &argv[1]);
	} 
	else if (strcmp(argv[0], "remove") == 0)
	{
		GUI_RemovePanel_Cmd(argc-1, &argv[1]);
	}
	else if (strcmp(argv[0], "show") == 0)
	{
		GUI_ShowPanel_Cmd(argc-1, &argv[1]);
	}
	else if (strcmp(argv[0], "focus") ==0)
	{
		GUI_FocusPanel_Cmd(argc-1, &argv[1]);
	}
	else if (strcmp(argv[0], "panel") == 0)
	{
		GUI_FocusPanel_Cmd(argc-1, &argv[1]);
	}
	else if (strcmp(argv[0], "hide") == 0)
	{
		GUI_HidePanel_Cmd(argc-1, &argv[1]);		
	} 
	else if (strcmp(argv[0], "link") == 0)
	{
		GUI_LinkPanel_Cmd(argc-1, &argv[1]);		
	} 
	else if (strcmp(argv[0], "list") == 0)
	{
		GUI_Panel_List();
	}
}

void	GUI_Panels_ChangeRes()
{
	gui_screenw = Vid_GetScreenW();
	gui_screenh = Vid_GetScreenH();
}

void	GUI_Panels_Init()
{
	LIST_CLEAR(&panelList);
//	panelList_tail = &panelList;
	current_panelname = NULL;

	Cmd_Register("panel", GUI_Panel_Cmd);
	Cmd_Register("removepanels", GUI_RemovePanels_Cmd);
	Cmd_Register("staticdepth", GUI_StaticDepth_Cmd);

	GUI_RegisterClass(class_name, GUI_Panel_Create);

	GUI_Panels_ChangeRes();
}
