// (C) 2003 S2 Games

// gui_main.c

// level editor gui stuff

#include "core.h"

#define GUI_CLIPPING_LAXNESS	30  //bit of a weird hack for expanding clipping constraints on widgets

#define DEFAULT_FADE_TIME		2000

gui_element_t elemlist;
gui_element_t *elemlist_tail = NULL;
gui_element_t *active = NULL;
gui_element_t *focus = NULL;
gui_element_t *selected_object = NULL;
gui_element_t *lastactive = NULL;

bool alt_text_showing = false;
int mouseover_start_time = 0;
bool mousedown[3] = { 0, 0, 0 };

cvar_t gui_hidden =			{ "gui_hidden",			"0"};
cvar_t gui_skipdraw =		{ "gui_skipdraw",		"0"};
cvar_t gui_coordWidth =		{ "gui_coordWidth",		"1024" };
cvar_t gui_coordHeight =	{ "gui_coordHeight",	"768" };
cvar_t gui_screenCoords =	{ "gui_screenCoords",	"0" };	//if 1, gui_coordWidth and gui_coordHeight are ignored and screen coordinates are used
cvar_t gui_staticdepth =	{ "gui_staticdepth",	"0" };	//if 1, panels won't get snapped to the foreground when clicked on
cvar_t gui_basepath =		{ "gui_basepath",		"/gui/standard/" };
cvar_t gui_coordWidthMultiplier =		{ "gui_coordWidthMultiplier",		"1" };
cvar_t gui_coordHeightMultiplier =		{ "gui_coordHeightMultiplier",		"1" };

cvar_t gui_defaultButtonSound = { "gui_defaultButtonSound", "/sound/gui/button_click.wav" };

int offsetx = 0;
int offsety = 0;

typedef struct gui_userdata_s {
	gui_element_t			*object;
	void					*class_name;
	void					*data;
	struct gui_userdata_s	*next;
} gui_userdata_t;

typedef struct gui_classname_s {
	void					*(*create)(gui_element_t *obj, int argc, char *argv[]);
	char					*class_name;
	struct gui_classname_s	*next;
} gui_classname_t;

gui_userdata_t *object_userdata = NULL;
gui_classname_t *object_classes	= NULL;

GHashTable *userdataHash = NULL;

// stuff that makes it all run ----------------------------------------

void	GUI_OffsetCoords(int x, int y)
{	
	offsetx = x;
	offsety = y;
}

void	GUI_ResetButtons()
{
	int i;

	for (i = 0; i < 3; i++)
		mousedown[i] = false;
}
	
bool	GUI_ButtonsDown()
{
	int i;

	for (i = 0; i < 3; i++)
		if (mousedown[i])
			return true;

	return false;
}

bool	GUI_WidgetWantsMouseClicks(mouse_button_enum button, int x, int y, bool down)
{
	if (!focus)
		return false;

	if ((focus->wantsMouseDownClicks && down) ||
		(focus->wantsMouseUpClicks && !down))
	{
		//possibly add a callback for some specialized widgets?
		return true;
	}
	return false;
}

bool	GUI_WidgetWantsKey(int key, bool down)
{
	if (key >= KEY_F1 && key <= KEY_F12)
	{
		if (focus && focus->wantsFunctionKeys)
			return true;
		else
			return false;
	}
	else if (key >= KEY_MOUSEFIRSTBUTTON && key <= KEY_MOUSELASTBUTTON)
	{
		if ((focus && focus->wantsMouseDownClicks && down) ||
			(focus && focus->wantsMouseUpClicks && !down))
			return true;
		else
			return false;
	}

	return true;
}

gui_element_t *GUI_GetWidgetAtXY(int x, int y)
{
	gui_element_t *list;
	bool ret = false;
	gui_panel_t *panel, *panelList_tail;
	gui_objectlist_t *obj;

	if (gui_hidden.integer)
		return false;

	GUI_Coord(&x, &y);

	//do it in last to first order so the last drawn element is the first to be checked
	if (!ret)
	{
		LIST_GETTAIL(panelList_tail, &panelList);

		for (panel=panelList_tail; panel!=&panelList; panel=panel->prev)
		{
			if (!panel->visible)
				continue;

			LIST_GETTAIL(obj, &panel->objects);
			for (; obj!=&panel->objects; obj=obj->prev)
			{
				list = obj->object;
	//		for (list=elemlist.next;list!=&elemlist;list=list->next)
	//		{
				if (GUI_IsVisible(list))
				{			
					if (GUI_IsInteractive(list))
					{
						if (x >= (list->bmin[0] + panel->pos[0]) 
							&& y >= (list->bmin[1] + panel->pos[1]) 
							&& x <= (list->bmax[0] + panel->pos[0]) 
							&& y <= (list->bmax[1] + panel->pos[1]))
						{		
							return list;
						}
					}								
				}
			}
		}
	}
	return NULL;
}

void	GUI_Sound(residx_t sound)
{
	Sound_Play(sound, -1, NULL, 1.0, CHANNEL_GUI, 0);
}

//this is the main UI loop
//mouse is given in screen coordinates
bool	GUI_CheckMouseAgainstUI(int x, int y)
{
	gui_element_t *widget;

	if (gui_hidden.integer)
		return false;

	if (x < 0)
		x = 0;
	if (x >= Vid_GetScreenW())	
		x = Vid_GetScreenW()-1;
	if (y < 0)
		y = 0;
	if (y >= Vid_GetScreenH())
		y = Vid_GetScreenH()-1;
	
	widget = GUI_GetWidgetAtXY(x, y);

	GUI_Coord(&x, &y);

	if (focus)
	{
		if (widget != focus)
		{
			if (focus->loseFocusOnMouseout
				&& !GUI_ButtonsDown())
			{
				GUI_Unfocus(focus);
			}
		}
	}
	
	if (widget != active)
	{
		if (!GUI_ButtonsDown())
		{
			active = widget;
			mouseover_start_time = 0;
		}
	}
		
	if (lastactive!=active && lastactive)
	{
		if (lastactive->mouseout)
			lastactive->mouseout(lastactive);
		if (lastactive->onMouseOut_cmd)
			GUI_Exec(lastactive->onMouseOut_cmd);
		if (lastactive->mouseout_sound)
			GUI_Sound(lastactive->mouseout_sound);			
		alt_text_showing = false;
		mouseover_start_time = 0;
		if (active && active->mouseenter)
			active->mouseenter(active);
	}

	if (!active)
		return false;
	
	if (mouseover_start_time == 0)
	{
		mouseover_start_time = Host_Milliseconds();
	}
	else if (!alt_text_showing)
	{
		if (active->drawalttext
			&& Host_Milliseconds() - mouseover_start_time > GUI_ALT_TEXT_DELAY_TIME)
		{
			alt_text_showing = true;
		}
	}

	if (active->mouseover)
	{
		active->mouseover(active, 
				x - (active->bmin[0] + active->panel->pos[0]), 
				y - (active->bmin[1] + active->panel->pos[1]));
	}
	if (active->onMouseOver_cmd)
	{
		GUI_Exec(active->onMouseOver_cmd);
	}
	if (active->mouseover_sound)
		GUI_Sound(active->mouseover_sound);

	lastactive = active;

	return (widget != NULL);
}

void	GUI_SetDrawRegion(int x, int y, int w, int h)
{
	GUI_ConvertToScreenCoord(&x, &y);
	GUI_ConvertToScreenCoord(&w, &h);
	Vid_SetDrawRegion(x, y, w, h);
}

void	GUI_Draw()
{
	gui_panel_t *panel;
	gui_objectlist_t *obj;

	OVERHEAD_INIT;

	if (gui_hidden.integer || !panelList.next)
		return;

	for (panel=panelList.next; panel!=&panelList; panel=panel->next)
	{
		gui_panel_t *p;
		ivec2_t min, max;
	
		if (!panel->visible)
			continue;

		//set the draw region to include this panel and all parents
		M_ClearRectI(min, max);

		for (p = panel; p; p=p->parent)
		{
			ivec2_t bmin,bmax;
			bmin[0] = p->bmin[0] + p->pos[0];
			bmin[1] = p->bmin[1] + p->pos[1];
			bmax[0] = p->bmax[0] + p->pos[0];
			bmax[1] = p->bmax[1] + p->pos[1];
			M_AddRectToRectI(bmin, bmax, min, max);
		}
		min[0]-=GUI_CLIPPING_LAXNESS;
		min[1]-=GUI_CLIPPING_LAXNESS;
		max[0]+=GUI_CLIPPING_LAXNESS;
		max[1]+=GUI_CLIPPING_LAXNESS;

		GUI_OffsetCoords(0,0);
		GUI_SetDrawRegion(min[0],
			min[1],
			max[0] - min[0],
			max[1] - min[1]);

		for (obj = panel->objects.next; obj!=&panel->objects; obj=obj->next)
		{
			gui_element_t *elem = obj->object;

			if (GUI_IsVisible(elem) && elem->draw)
			{
				GUI_OffsetCoords(elem->bmin[0] + elem->panel->pos[0], 
					elem->bmin[1] + elem->panel->pos[1]);

				if (elem->noclip)
					Vid_SetDrawRegion(0,0,Vid_GetScreenW(),Vid_GetScreenH());

				if (!gui_skipdraw.integer)
					elem->draw(elem, 
						elem->bmax[0] - elem->bmin[0], 
						elem->bmax[1] - elem->bmin[1]);
				GUI_OffsetCoords(0,0);
				
				if (elem->noclip)
					GUI_SetDrawRegion(min[0],
						min[1],
						max[0] - min[0],
						max[1] - min[1]);
			}
		}
	}

	Vid_SetDrawRegion(0,0,Vid_GetScreenW(),Vid_GetScreenH());

	if (active)
	{
/*		if (!active->panel->visible || !active->visible)
		{
			active = NULL;
			return;
		}
	
		if (active->focus)
		{
			if (active->draw)
			{
				gui_element_t *elem = active;

				GUI_OffsetCoords(elem->bmin[0] + elem->panel->pos[0], 
					elem->bmin[1] + elem->panel->pos[1]);

				elem->draw(elem, 
					elem->bmax[0] - elem->bmin[0], 
					elem->bmax[1] - elem->bmin[1]);

				GUI_OffsetCoords(0,0);
			}
		}
*/
		if (alt_text_showing
			&& active->drawalttext)
		{
			GUI_OffsetCoords(active->bmin[0] + active->panel->pos[0], 
							 active->bmin[1] + active->panel->pos[1]);
			if (!gui_skipdraw.integer)				
				active->drawalttext(active, 
								active->bmax[0] - active->bmin[0], 
								active->bmax[1] - active->bmin[1]);
			GUI_OffsetCoords(0,0);
		}
	}

	GUI_OffsetCoords(0,0);

	/*
	for (list=elemlist_tail;list!=&elemlist;list=list->prev)
	{
		if (GUI_IsVisible(list) && list->draw)
		{
			current_elem = list;
			if (list->panel)
			{
				GUI_OffsetCoords(list->bmin[0] + list->panel->pos[0], 
					list->bmin[1] + list->panel->pos[1]);
			} else {
				GUI_OffsetCoords(list->bmin[0], list->bmin[1]);
			}
			list->draw(list, 
				list->bmax[0] - list->bmin[0], 
				list->bmax[1] - list->bmin[1]);
			GUI_OffsetCoords(0,0);
		}
	}
	*/

	OVERHEAD_COUNT(OVERHEAD_GUI_DRAW);
}

void	GUI_ChangeAlpha(gui_element_t *obj, float alpha)
{
	obj->alpha = alpha;
}

void	GUI_Frame()
{
	gui_panel_t *panel;
	gui_objectlist_t *obj;

	OVERHEAD_INIT;

	for (panel=panelList.next; panel!=&panelList; panel=panel->next)
	{
		if (!panel->visible)
			continue;

		for (obj = panel->objects.next; obj!=&panel->objects; obj=obj->next)
		{
			gui_element_t *elem = obj->object;

			switch (elem->fadeType)
			{
				case FADE_IN:
						elem->alpha = MIN(1, LERP((Host_Milliseconds() - elem->fadeStart) / elem->fadeTime, 0, 1));
						if (elem->alpha >= elem->alphaSetting)
						{
							elem->fadeType = FADE_NONE;
							elem->alpha = elem->alphaSetting;
						}
						break;
				case FADE_OUT:
						elem->alpha = MAX(0, LERP((Host_Milliseconds() - elem->fadeStart) / elem->fadeTime, 1, 0));
						if (elem->alpha <= 0)
						{
							elem->fadeType = FADE_NONE;
							elem->alpha = 0;
							GUI_Hide(elem);
						}
						break;
				case FADE_NONE:
						break;
			}
		}
	}

	OVERHEAD_COUNT(OVERHEAD_GUI_FRAME);
}

gui_element_t *GUI_GetNextWidgetInPanel(gui_element_t *widget)
{
	gui_panel_t *panel;
	gui_element_t *elem = NULL;
	gui_objectlist_t *obj;

	panel = widget->panel;

	for (obj = panel->objects.prev; obj!=&panel->objects; obj=obj->prev)
	{
		elem = obj->object;

		if (elem == widget)
			break;
	}
	if (elem != widget
		|| obj->prev == &panel->objects)
	{
		return NULL;
	}
	return obj->prev->object;
}

gui_element_t *GUI_GetNextInteractiveWidgetInPanel(gui_element_t *widget)
{
	gui_element_t *next = GUI_GetNextWidgetInPanel(widget);
	while (next)
	{
		if (next->interactive)
			return next;

		next = GUI_GetNextWidgetInPanel(next);
	}
	return NULL;
}

void	GUI_ActivateNextInteractiveWidget()
{
	gui_element_t *next;
	if (focus)
	{
		next = GUI_GetNextInteractiveWidgetInPanel(focus);
		if (next)
		{
			GUI_Focus(next);
		}
	}
}

void	GUI_ActivateNextInteractiveWidget_Cmd(int argc, char *argv[])
{
	GUI_ActivateNextInteractiveWidget();
}

bool	GUI_SendMouseDown(mouse_button_enum button, int x, int y)
{
	gui_element_t *over;
	
	if (gui_hidden.value)
		return false;

	over = GUI_GetWidgetAtXY(x, y);
	
	GUI_Coord(&x, &y);

	if (over && over != focus)
	{
		if (focus)
			GUI_Unfocus(focus);
		active = over;
	}
	
	if (active)
	{
		//switch this panel to the foreground
		if (active->interactive)
		{
			if (!gui_staticdepth.integer && !active->panel->staticdepth)
				GUI_Panel_BringTreeToFront(active->panel);
			GUI_Focus(active);

			mousedown[button] = true;
		
			if (active->mousedown)
			{
				active->mousedown(active, button,
					x - (active->bmin[0] + active->panel->pos[0]), 
					y - (active->bmin[1] + active->panel->pos[1]));
			}

			if (active)		//active could conceivably become NULL during the mousedown function
			{
				if (active->onMouseDown_cmd)
					GUI_Exec(active->onMouseDown_cmd);
				if (active->mousedown_sound)
					GUI_Sound(active->mousedown_sound);
			}
		}
		
		return true;		
		
	}
	return false;
}

bool	GUI_SendMouseUp(mouse_button_enum button, int x, int y)
{
	if (gui_hidden.value)
		return false;

	GUI_Coord(&x, &y);

	if (active)
	{		
		//GUI_Unfocus(active);

		mousedown[button] = false;
			
		if (active->mouseup)
		{
			active->mouseup(active, button,
				x - (active->bmin[0] + active->panel->pos[0]), 
				y - (active->bmin[1] + active->panel->pos[1]));				
		}
		if (active->onMouseUp_cmd)
			GUI_Exec(active->onMouseUp_cmd);
		if (active->mouseup_sound)
			GUI_Sound(active->mouseup_sound);

		return true;
	}
	return false;
}

void	GUI_ResetFocus()
{
	if (active)
		active->focus = false;
}

void	GUI_AddElement(gui_element_t *elem)
{
	
	elem->next = &elemlist;
	elem->prev = elemlist.prev;
	elem->prev->next = elem;
	elem->next->prev = elem;
	
	elemlist_tail = elem;

	elem->focus = false;
	elem->width = elem->bmax[0] - elem->bmin[0];
	elem->height = elem->bmax[1] - elem->bmin[1];
}

void	GUI_RemoveElement(gui_element_t *elem)
{
	gui_element_t *tmp;
	void *userdata;
	
	elem->next->prev = elem->prev;
	elem->prev->next = elem->next;
	elem->next = NULL;
	elem->prev = NULL;

	while (elem)
	{
		tmp = elem;
		elem = elem->parent;
		userdata = GUI_GetUserdata(tmp->class_name, tmp);
		if (tmp->destroy)
			tmp->destroy(tmp);

		g_hash_table_remove(userdataHash, userdata);
		//free the class data
		GUI_Free(userdata);
		//free the gui_element
		GUI_Free(tmp);
	}

}

void	GUI_UpdateDims()
{
	gui_element_t *elem;

	for (elem = elemlist.next; elem!=&elemlist; elem=elem->next)
	{
		elem->width = elem->bmax[0] - elem->bmin[0];
		elem->height = elem->bmax[1] - elem->bmin[1];
	}
}



// ------ userdata functions ---------------------------------------------------

gui_userdata_t *GUI_AddUserdataNode(gui_userdata_t *list)
{
	gui_userdata_t *node;
	node = (gui_userdata_t *)GUI_Malloc(sizeof (gui_userdata_t));
	node->next = list;
	return node;
}

gui_userdata_t *GUI_FreeUserdata(gui_userdata_t *list)
{
	gui_userdata_t *tmp;

	while (list != NULL)
	{
		tmp = list;
		list = list->next;
		GUI_Free(tmp);
	}
	return NULL;
}

void   *GUI_GetUserdata(char *class_name, gui_element_t *obj)
{
	gui_userdata_t *node;
	gpointer hash_node;
	
	if (!obj)
		return NULL;

	if (obj->userdata)
		return obj->userdata;
		
	hash_node = g_hash_table_lookup(userdataHash, obj);

	node = hash_node;
	if (node && node->class_name && node->data 
		&& strcmp(node->class_name, class_name) == 0)
	{
		return node->data;
	}
	
	node = object_userdata;
	while (node != NULL)
	{
		if (strcmp(node->class_name, class_name) == 0
			&& node->object == obj)
		{
			return node->data;
		}
		node = node->next;
	}
	return NULL;
}

void	*GUI_SetUserdata(char *class_name, gui_element_t *obj, void *user_data)
{
	object_userdata = GUI_AddUserdataNode(object_userdata);
	object_userdata->object = obj;
	object_userdata->class_name = class_name;
	object_userdata->data = user_data;

	g_hash_table_insert(userdataHash, obj, object_userdata);

	if (obj->userdata)
		obj->userdata = NULL;
	else
		obj->userdata = user_data;
	
	return user_data;
}

gui_element_t	*GUI_GetObject(char *hierarchy)
{
	gui_element_t *tmp_obj, *walker;
	gui_panel_t *panel;
	gui_objectlist_t *objects;
	char panel_name[256];
	char *name;
	bool found = false;

	if (!hierarchy)
		return NULL;

	if (!hierarchy[0])
		return NULL;

	if (!strchr(hierarchy, ':'))
	{
		name = hierarchy;
		tmp_obj = NULL;

		for (walker = elemlist.next; walker!=&elemlist; walker = walker->next)
		{
			if (!walker)
				return NULL;

			if (strcmp(walker->name, name) == 0)
			{
				if (found)
				{
					Console_Printf("There is more than one object named '%s'.  You must specify the panel name along with the object name.\n", name);
					return NULL;
				}
				tmp_obj = walker;
				found = true;
			}
		}
		//tmp_obj will be NULL if nothing was found
		return tmp_obj;
	}
	else
	{
		strncpySafe(panel_name, hierarchy, 256);
		panel_name[strchr(hierarchy, ':') - hierarchy] = 0;
		name = strchr(hierarchy, ':')+1;
	}
	
	panel = GUI_GetPanel(panel_name);

	if (!panel)
	{
		Console_Printf("panel %s not found\n", panel_name);
		return NULL;
	}

	for (objects = panel->objects.next; objects != &panel->objects; objects = objects->next)
	{
		if (strcmp(objects->object->name, name) == 0)
			return objects->object;
	}
	Console_Printf("panel %s object %s not found\n", panel_name, name);
	return NULL;
}

void	*GUI_GetClass(char *hierarchy, char *classname)
{
	gui_element_t *obj;

	obj = GUI_GetObject(hierarchy);

	if (!obj)
	{
		Console_Printf("error, couldn't find object named %s\n", hierarchy);
		return NULL;
	}

	/*if (strcmp(obj->class_name, classname) != 0)
	{
		Console_Printf("Class mismatch, %s is not a %s, it is a %s\n", hierarchy, classname, obj->class_name);
		return NULL;	
	}
	*/

	return GUI_GetUserdata(classname, obj);
}





//
//coordinate and scalar conversion functions
//


int	GUI_GetScreenWidth()
{
	if (gui_screenCoords.integer)
	{
		return Vid_GetScreenW();
	}
	else
	{
		return gui_coordWidth.value;
	}
}

int	GUI_GetScreenHeight()
{
	if (gui_screenCoords.integer)
	{
		return Vid_GetScreenH();
	}
	else
	{
		return gui_coordHeight.value;
	}
}

//translate a GUI coordinate to screen coordinate
void	GUI_ConvertToScreenCoord(int *x, int *y)
{
	float fx,fy;

	if (gui_screenCoords.integer)
	{
		*x = *x + offsetx;
		*y = *y + offsety;
		return;
	}

	fx = (*x + offsetx) * (Vid_GetScreenW() / gui_coordWidth.value);
	fy = (*y + offsety) * (Vid_GetScreenH() / gui_coordHeight.value);

	FloatToInt(x, fx);
	FloatToInt(y, fy);
}

//translate a screen coordinate to GUI coordinate
void	GUI_Coord(int *x, int *y)
{
	float fx,fy;

	if (gui_screenCoords.integer)
	{
		*x = *x - offsetx;
		*y = *y - offsety;
		return;		//gui is in screen coordinate mode already
	}

	fx = *x * (gui_coordWidth.value / Vid_GetScreenW());
	fy = *y * (gui_coordHeight.value / Vid_GetScreenH());

	FloatToInt(x, fx - offsetx);
	FloatToInt(y, fy - offsety);
}

//convert two scalar values specified in GUI units to screen units
void	GUI_ScaleToScreen(int *w, int *h)
{
	float fw,fh;

	if (gui_screenCoords.integer)
	{
		return;
	}

	fw = *w * (Vid_GetScreenW() / gui_coordWidth.value);
	fh = *h * (Vid_GetScreenH() / gui_coordHeight.value);

	FloatToInt(w, fw);
	FloatToInt(h, fh);
}

void	GUI_Scale(int *w, int *h)
{
	float fw,fh;

	if (gui_screenCoords.integer)
	{
		return;		//gui is in screen coordinate mode already
	}

	fw = *w * (gui_coordWidth.value / Vid_GetScreenW());
	fh = *h * (gui_coordHeight.value / Vid_GetScreenH());

	FloatToInt(w, fw);
	FloatToInt(h, fh);
}

void	GUI_GetPosition(gui_element_t *obj, ivec2_t pos)
{
	pos[X] = obj->bmin[X] + obj->panel->pos[X];
	pos[Y] = obj->bmin[Y] + obj->panel->pos[Y];	
}

void	GUI_GetSize(gui_element_t *obj, ivec2_t size)
{
	size[X] = obj->width;
	size[Y] = obj->height;
}

void	GUI_GetScreenPosition(gui_element_t *obj, ivec2_t pos)
{
	pos[X] = obj->bmin[X] + obj->panel->pos[X];
	pos[Y] = obj->bmin[Y] + obj->panel->pos[Y];
	GUI_ConvertToScreenCoord(&pos[X], &pos[Y]);
}

void	GUI_GetScreenSize(gui_element_t *obj, ivec2_t size)
{
	size[X] = obj->width;
	size[Y] = obj->height;
	GUI_ConvertToScreenCoord(&size[X], &size[Y]);
}

// ------ class name/create functions ---------------------------------------------------

gui_classname_t *GUI_AddClassnameNode(gui_classname_t *list)
{
	gui_classname_t *node;
	node = (gui_classname_t *)Tag_Malloc(sizeof (gui_classname_t), MEM_GUI_CLASSNAMES);
	if (!node)
	{
		Console_Printf("AddClassname error: not enough pool memory to allocate node (size %d).\n", sizeof(gui_classname_t));
		return list;
	}
	node->next = list;
	return node;
}

gui_classname_t *GUI_FreeClassnames(gui_classname_t *list)
{
	Tag_FreeAll(MEM_GUI_CLASSNAMES);
	return NULL;
}

void   *GUI_GetClassname(char *class_name)
{
	gui_classname_t *node = object_classes;

	while (node != NULL)
	{
		if (strcmp(node->class_name, class_name) == 0)
		{
			return node->create;
		}
		node = node->next;
	}
	return NULL;
}

void	*GUI_RegisterClass(char *class_name, void *(*create)(gui_element_t *obj, int argc, char *argv[]))
{
	object_classes = GUI_AddClassnameNode(object_classes);
	object_classes->class_name = class_name;
	object_classes->create = create;
	return create;
}

//send a notification message to all gui elements
void	GUI_Notify(int argc, char *argv[])
{
	gui_panel_t *panel;
	gui_objectlist_t *obj;

	if (!panelList.next)
		return;	

	for (panel=panelList.next; panel!=&panelList; panel=panel->next)
	{
		//if (!panel->visible)
	//		continue;

		for (obj = panel->objects.next; obj!=&panel->objects; obj=obj->next)
		{
			gui_element_t *elem = obj->object;

			//right now we're just sending the notification even if the element is hidden.  this is probably the desired behavior
			if (elem->notify)
				elem->notify(elem, argc, argv);
		}
	}
}

void	GUI_FadeOutObject(gui_element_t *obj, float time)
{
	//the fadeType check is only a hack so we can keep calling "fadeout <widget>" 
	//every frame from a button and get a valid effect
	if (obj->alpha <= 0 || obj->fadeType == FADE_OUT)
		return;
		
	//Console_DPrintf("fading out object\n");
	
	if (time <= 0)
	{
		GUI_Hide(obj);
		return;
	}
		
	//remember it might not be full alpha at this point
	obj->fadeStart = Host_Milliseconds() - obj->alpha / time;
	obj->fadeTime = time + obj->alpha / time;
	obj->fadeType = FADE_OUT;
}

void	GUI_FadeInObject(gui_element_t *obj, float time)
{
	//the fadeType check is only a hack so we can keep calling "fadein <widget>" 
	//every frame from a button and get a valid effect
	//if (obj->alpha >= 1 || obj->fadeType == FADE_IN)
	//	return;

	if (!GUI_IsVisible(obj))
	{
		GUI_Show(obj);
	}
		
	//Console_DPrintf("fading in object\n");
	
	if (time <= 0)
	{
		GUI_Show(obj);
		return;
	}
		
	//remember it might not be full alpha at this point
	obj->fadeStart = Host_Milliseconds() - obj->alpha / time;
	obj->fadeTime = time + obj->alpha / time;
	obj->fadeType = FADE_IN;
}

void	GUI_FadeInObject_Cmd(int argc, char *argv[])
{
	gui_element_t *obj = NULL;
	gui_panel_t *panel = NULL;
	float time = DEFAULT_FADE_TIME;

	if (!argc)
	{
		Console_Printf("syntax: fadein <object> [time]\n");
		return;
	}
	obj = GUI_GetObject(argv[0]);
	if (!obj)
	{
		panel = GUI_GetPanel(argv[0]);
	}

	if (argc > 1)
		time = atoi(argv[1]);

	if (panel)
		GUI_Panel_FadeIn(panel, time);
	else if (obj)
		GUI_FadeInObject(obj, time);
	else
		Console_Printf("Object %s not found.\n", argv[0]);
}

void	GUI_FadeOutObject_Cmd(int argc, char *argv[])
{
	gui_element_t *obj = NULL;
	gui_panel_t *panel = NULL;
	float time = DEFAULT_FADE_TIME;

	if (!argc)
	{
		Console_Printf("syntax: fadeout <object> [time]\n");
		return;
	}
	obj = GUI_GetObject(argv[0]);
	if (!obj)
	{
		panel = GUI_GetPanel(argv[0]);
	}

	if (argc > 1)
		time = atoi(argv[1]);

	if (panel)
		GUI_Panel_FadeOut(panel, time);
	else if (obj)
		GUI_FadeOutObject(obj, time);
	else
		Console_Printf("Object %s not found.\n", argv[0]);
}

// --- Object function prototypes ----------------------------

bool	GUI_IsVisible(gui_element_t *obj)
{
	if (!obj->visible
		|| (obj->panel && !obj->panel->visible)
		)
		return false;

	return true;
}

bool	GUI_IsInteractive(gui_element_t *obj)
{
	return obj->interactive;
}

void	GUI_Exec(char *command)
{
	if (!command)
		return;

	Cmd_Exec(command);
}

char	*GUI_SetClass(gui_element_t *obj, char *class_name)
{
	if (obj->class_name 
		&& strcmp(obj->class_name, "") != 0)
		return obj->class_name;
	strncpy(obj->class_name, class_name, GUI_ELEMENT_CLASS_NAME_LENGTH-1);
	obj->name[GUI_ELEMENT_CLASS_NAME_LENGTH-1] = 0;
	return class_name;
}

char	*GUI_SetName(gui_element_t *obj, char *name)
{
	strncpy(obj->name, name, GUI_ELEMENT_NAME_LENGTH-1);
	obj->name[GUI_ELEMENT_NAME_LENGTH-1] = 0;
	return name;
}

void	GUI_Move(gui_element_t *obj, int x, int y)
{
	if (!obj)
		return;
		
	obj->bmin[0] = x;
	obj->bmin[1] = y;
	obj->bmax[0] = x + obj->width;
	obj->bmax[1] = y + obj->height;

	if (obj->panel)		//objects don't get panel assigned until after their create function
		M_AddRectToRectI(obj->bmin, obj->bmax, obj->panel->bmin, obj->panel->bmax);

	if (obj->onMove_cmd)
		GUI_Exec(obj->onMove_cmd);
}

void	GUI_Resize(gui_element_t *obj, int w, int h)
{
	if (!obj)
		return;
		
	obj->bmax[0] = w + obj->bmin[0];
	obj->bmax[1] = h + obj->bmin[1];

	obj->width = w;
	obj->height = h;

	if (obj->panel)
		GUI_Panel_UpdateWidgetSize(obj);
}

void	GUI_Show(gui_element_t *obj)
{
	if (!obj)
		return;
		
	obj->visible = true;
	GUI_ChangeAlpha(obj, obj->alphaSetting);
	obj->fadeType = FADE_NONE;
	
	if (obj->show)
		obj->show(obj);

	if (obj->onShow_cmd)
		GUI_Exec(obj->onShow_cmd);
}

void	GUI_Hide(gui_element_t *obj)
{
	if (!obj)
		return;
		
	if (focus == obj)
		GUI_Unfocus(obj);

	obj->visible = false;
	obj->fadeType = FADE_NONE;
	obj->alpha = obj->alphaSetting;

	if (obj->hide)
		obj->hide(obj);

	if (obj->onHide_cmd)
		GUI_Exec(obj->onHide_cmd);
}

void	GUI_Unfocus(gui_element_t *obj)
{
	static bool recursion = false;

	if (recursion)
		return;

	if (!obj)
		return;
		
	if (focus != obj)
		return;

	obj->focus = false;
	if (obj->unfocused)
	{
		recursion = true;
		obj->unfocused(obj);
		recursion = false;
	}

	focus = NULL;
}

void	GUI_Focus(gui_element_t *obj)
{
	if (!obj)
		return;
		
	if (focus)
		GUI_Unfocus(focus);

	if (!obj->visible || !obj->panel->visible)
		return;	

	obj->focus = true;
	if (obj->focused)
		obj->focused(obj);
	focus = obj;
}

bool 	GUI_Select(gui_element_t *obj)
{
	if (obj)
	{
		selected_object = obj;
		return true;
	}
	else
	{
		selected_object = NULL;
		return false;
	}
}

void            GUI_Param(gui_element_t *obj, int argc, char *argv[])
{	
	/*
	if (argc < 1)
		return;
	*/

	if (argc && strcmp(argv[0], "interactive") == 0)
	{
		if (argc > 1)
		{
			obj->interactive = atoi(argv[1]);
		}
	}
	if (argc && strcmp(argv[0], "alpha") == 0)
	{
		if (argc > 1)
		{
			obj->alphaSetting = atof(argv[1]);			
			obj->alpha = obj->alphaSetting;
		}
	}
	else if (argc && (strcmp(argv[0], "text_color") == 0 || strcmp(argv[0], "textcolor") == 0))
	{
		if (argc > 3)
		{
			obj->textcolor[0] = atof(argv[1]);
			obj->textcolor[1] = atof(argv[2]);
			obj->textcolor[2] = atof(argv[3]);
		} else {
			Console_Printf("Not enough parameters.  You must specify the R, G, and B values (range [0..1])\n");
		}
	}
	else if (argc && (strcmp(argv[0], "text_height") == 0 || strcmp(argv[0], "char_height") == 0))
	{
		if (argc > 1)
		{
			obj->char_height = MAX(1.0, atof(argv[1]));
		} else {
			Console_Printf("Not enough parameters.  You must specify the text height in pixels.\n");
		}
	}
	else if (argc && (strcmp(argv[0], "mouseover_sound") == 0))
	{
		if (argc > 1)
		{
			obj->mouseover_sound = Res_LoadSound(argv[1]);
		} else {
			Console_Printf("Not enough parameters.  You must specify the sound filename.\n");
		}
	}
	else if (argc && (strcmp(argv[0], "mouseout_sound") == 0))
	{
		if (argc > 1)
		{
			obj->mouseout_sound = Res_LoadSound(argv[1]);
		} else {
			Console_Printf("Not enough parameters.  You must specify the sound filename.\n");
		}
	}
	else if (argc && (strcmp(argv[0], "mousedown_sound") == 0 || strcmp(argv[0], "sound") == 0))
	{
		if (argc > 1)
		{
			obj->mousedown_sound = Res_LoadSound(argv[1]);
		} else {
			Console_Printf("Not enough parameters.  You must specify the sound filename.\n");
		}
	}
	else if (argc && (strcmp(argv[0], "mouseup_sound") == 0))
	{
		if (argc > 1)
		{
			obj->mouseup_sound = Res_LoadSound(argv[1]);
		} else {
			Console_Printf("Not enough parameters.  You must specify the sound filename.\n");
		}
	}

	if (obj->param)
		obj->param(obj, argc, argv);
}

gui_element_t *GUI_New()
{
	gui_element_t *obj;

	obj = (gui_element_t *)GUI_Malloc(sizeof (gui_element_t));

	memset(obj, 0, sizeof (gui_element_t));

	obj->interactive = true;
	obj->visible = true;
	obj->loseFocusOnMouseout = true;
	obj->alpha = 1.0;
	obj->alphaSetting = 1.0;
	obj->fadeType = FADE_NONE;
	obj->textcolor[0] = 1;
	obj->textcolor[1] = 1;
	obj->textcolor[2] = 1;
	obj->char_height = 14;

	obj->mousedown_sound = Res_LoadSound(gui_defaultButtonSound.string);

	return obj;
}

void	*GUI_Malloc(int size)
{
	char *newmem = Tag_Malloc(size, MEM_GUI);	
	
	return newmem;
}

void	*GUI_ReAlloc(void *ptr, int size)
{
	return Tag_Realloc(ptr, size, MEM_GUI);
}

void	GUI_Free(void *ptr)
{
	if (ptr == NULL)
		Console_DPrintf("warning: NULL free attempt\n");
	else
		Tag_Free(ptr);
}

char	*GUI_StrDup(const char *string)
{
	return Tag_Strdup(string, MEM_GUI);
}

/* -----------------------------------------------------------
 * All the console commands...
 *
 */

void	GUI_Move_Cmd(int argc, char *argv[])
{
	gui_panel_t *panel;
	gui_element_t *obj;
	int x, y;

	if (argc < 3)
	{
		Console_Printf("syntax: move <panel>:<object> [+-]x [+-]y\n");
		return;
	}
	obj = GUI_GetObject(argv[0]);

	x = atoi(argv[1]);
	y = atoi(argv[2]);

	if (!obj)
	{
		panel = GUI_GetPanel(argv[0]);
		if (panel)
			GUI_Panel_Move(panel, x, y);
		return;
	}

	if (argv[1][0] == '-'
		|| argv[1][0] == '+')
	{
		x += obj->bmin[0];
		y += obj->bmin[1];
	}

	GUI_Move(obj, x, y);
}

void	GUI_Resize_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;
	int w, h;

	if (argc < 3)
	{
		Console_Printf("syntax: resize <panel>:<object> w h\n");
		return;
	}
	obj = GUI_GetObject(argv[0]);
	if (!obj)
		return;

	w = atoi(argv[1]);
	h = atoi(argv[2]);

	GUI_Resize(obj, w, h);
}

void	GUI_Show_Cmd(int argc, char *argv[])
{
	gui_panel_t *panel;
	gui_element_t *obj;

	if (!argc)
	{
		Console_Printf("syntax: show <panel>:<object>\n");
		return;
	}
	obj = GUI_GetObject(argv[0]);
	if (!obj)
	{
		panel = GUI_GetPanel(argv[0]);
		if (panel)
			GUI_Panel_Show(panel);
		return;
	}

	GUI_Show(obj);
}

void	GUI_Hide_Cmd(int argc, char *argv[])
{
	gui_panel_t *panel;
	gui_element_t *obj;

	if (!argc)
	{
		Console_Printf("syntax: hide <panel>:<object>\n");
		return;
	}
	obj = GUI_GetObject(argv[0]);
	if (!obj)
	{
		panel = GUI_GetPanel(argv[0]);
		if (panel)
			GUI_Panel_Hide(panel);
		return;

	}

	GUI_Hide(obj);
}

void	GUI_List_Cmd(int argc, char *argv[])
{
	gui_userdata_t *objects;

	Console_Printf("GUI Objects:\n");

	objects = object_userdata;
	while (objects)
	{
		if (!argc 
			|| strstr( objects->object->name, argv[0] ) 
			|| strstr( objects->object->panel->name, argv[0] ) )
		{
			Console_Printf("    %s:%s\n", objects->object->panel->name, objects->object->name);
		}
		objects = objects->next;
	}
}

void	GUI_Create_Cmd(int argc, char *argv[])
{
	static gui_element_t *obj = NULL;
	void *(*create)(gui_element_t *obj, int argc, char *argv[]);

	if (!argc)
	{
		Console_Printf("syntax: create <object> to see parameters for that object\n");
		return;
	}

	create = GUI_GetClassname(argv[0]);
	if (!create)
	{
		Console_Printf("error: object '%s' not found.\n", argv[0]); 
		return;
	}

	if (!obj)
		obj = GUI_New();

	if (create(obj, argc - 1, &argv[1]))
	{
		GUI_Panel_AddObject(obj);
		GUI_AddElement(obj);
		selected_object = obj;
		obj = NULL;
	}
}

void	GUI_Destroy_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (!argc)
	{
		Console_Printf("Usage: destroy <widget>\n");
		return;
	}
	
	obj = GUI_GetObject(argv[0]);
	
	if (!obj)
	{
		Console_Printf("Object '%s' not found.\n", argv[0]);
		return;
	}
	
	GUI_RemoveElement(obj);
}

//syntactic sugar / param code
void	GUI_Focus_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (argc < 1)
	{
		Console_Printf("syntax: focus <panel>:<object>\n");
		return;
	}

	obj = GUI_GetObject(argv[0]);

	if (!obj)
		return;

	if (!GUI_Select(obj))
		Console_Printf("focus error:  object %s not found.\n", argv[0]);
	GUI_Focus(obj);
}

//syntactic sugar / param code
void	GUI_Select_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (argc < 1)
	{
		Console_Printf("syntax: select <panel>:<object>\n");
		return;
	}

	obj = GUI_GetObject(argv[0]);

	if (!GUI_Select(obj))
		Console_Printf("select error:  object %s not found.\n", argv[0]);
}

//syntactic sugar / param code
void	GUI_Param_Cmd(int argc, char *argv[])
{
	/*
	if (argc < 2)
	{
		Console_Printf("syntax: param <var> <value>\n");
		return;
	}
	*/

	if (!selected_object)
	{
		Console_Printf("no object selected\n");
		return;
	}

	GUI_Param(selected_object, argc, argv);
}

//event catching
void	GUI_On_Cmd(int argc, char *argv[])
{
	gui_element_t *obj;

	if (argc < 2)
	{
		Console_Printf("syntax: on <event> cmd\n");
		Console_Printf("<event> can be one of the following:\n");
		Console_Printf("mousedown, mouseover, mouseup, mouseout, show, hide, move\n");

		return;
	}

	if (!selected_object)
	{
		Console_Printf("on: no object selected\n");
		return;
	}

	obj = selected_object;	

	if (strcmp(argv[0], "mousedown")==0)
		obj->onMouseDown_cmd = GUI_StrDup(argv[1]);
	else if (strcmp(argv[0], "mouseover")==0)
		obj->onMouseOver_cmd = GUI_StrDup(argv[1]);
	else if (strcmp(argv[0], "mouseup")==0)
		obj->onMouseUp_cmd = GUI_StrDup(argv[1]);
	else if (strcmp(argv[0], "mouseout")==0)
		obj->onMouseOut_cmd = GUI_StrDup(argv[1]);
	else if (strcmp(argv[0], "show")==0)
		obj->onShow_cmd = GUI_StrDup(argv[1]);
	else if (strcmp(argv[0], "hide")==0)
		obj->onHide_cmd = GUI_StrDup(argv[1]);
	else if (strcmp(argv[0], "move")==0)
		obj->onMove_cmd = GUI_StrDup(argv[1]);
	else
	{
		Console_Printf("error: not a valid event type.  Type \"on\" for a list of events\n");
		return;
	}
}

void	GUI_HideAll_Cmd(int argc, char *argv[])
{
	GUI_HideAllPanels();
}

// ------- class init ------------------------------------------

extern char *current_panelname;

void GUI_ClearWidgets()
{
	object_userdata = GUI_FreeUserdata(object_userdata);
	
	if (userdataHash)
	{
		g_hash_table_destroy(userdataHash);
	}
	userdataHash = g_hash_table_new(g_direct_hash, g_direct_equal);

	Tag_FreeAll(MEM_GUI);
	
	elemlist.next = &elemlist;
	elemlist.prev = &elemlist;
	elemlist_tail = &elemlist;
	active = NULL;
	selected_object = NULL;
	lastactive = NULL;
	focus = NULL;

	LIST_CLEAR(&panelList);
	current_panelname = NULL;
}

void	GUI_Reset()
{
	static bool firstrun = true;

	if (!firstrun)
		GUI_RemovePanels();

	GUI_ClearWidgets();
	GUI_Panels_Init();

	Cvar_SetVarValue(&gui_coordWidthMultiplier, (Vid_GetScreenW() / gui_coordWidth.value));
	Cvar_SetVarValue(&gui_coordHeightMultiplier, (Vid_GetScreenH() / gui_coordHeight.value));

	firstrun = false;
}

void GUI_Init()
{
	Cvar_Register(&gui_hidden);
	Cvar_Register(&gui_coordWidth);
	Cvar_Register(&gui_coordHeight);
	Cvar_Register(&gui_screenCoords);
	Cvar_Register(&gui_staticdepth);
	Cvar_Register(&gui_basepath);
	Cvar_Register(&gui_skipdraw);
	Cvar_Register(&gui_coordWidthMultiplier);
	Cvar_Register(&gui_coordHeightMultiplier);
	Cvar_Register(&gui_defaultButtonSound);
	
	object_classes = GUI_FreeClassnames(object_classes);

	GUI_Reset();

	Cmd_Register("move", GUI_Move_Cmd);
	Cmd_Register("show", GUI_Show_Cmd);
	Cmd_Register("hide", GUI_Hide_Cmd);
	Cmd_Register("hideall", GUI_HideAll_Cmd);
	Cmd_Register("resize", GUI_Resize_Cmd);
	Cmd_Register("list", GUI_List_Cmd);
	Cmd_Register("create", GUI_Create_Cmd);
	Cmd_Register("on", GUI_On_Cmd);
	Cmd_Register("notify", GUI_Notify);
	Cmd_Register("param", GUI_Param_Cmd);
	Cmd_Register("select", GUI_Select_Cmd);
	Cmd_Register("focus", GUI_Focus_Cmd);
	Cmd_Register("destroy", GUI_Destroy_Cmd);
	Cmd_Register("fadein", GUI_FadeInObject_Cmd);
	Cmd_Register("fadeout", GUI_FadeOutObject_Cmd);
	Cmd_Register("nextWidget", GUI_ActivateNextInteractiveWidget_Cmd);	
}
